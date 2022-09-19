/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "fma-common/atomic_ops.h"
#include "fma-common/cache_aligned_vector.h"
#include "fma-common/thread_id.h"
#include "fma-common/utils.h"

namespace fma_common {
class LockUpgradeConflictException : public std::runtime_error {
 public:
    LockUpgradeConflictException()
        : std::runtime_error("Lock upgrade failed. Probably another writer is already writing?") {}
};

// RWLock using thread-local storage.
// Multi-reader, multi-writer, read optimized, write prioritized RWLock.
// Read locks are kept in thread-local storage, so reading is optimal.
// Readers wait for writers as long as there is any writer, so write is prioritized.
// NOTE: ReadLock/ReadUnlock pairs must be used in the same thread since we keep the
// lock info in TLS.
// Allows re-entry.
class TLSRWLock {
    std::atomic<int64_t> n_writers_;
    StaticCacheAlignedVector<std::atomic<int64_t>, FMA_MAX_THREADS> reader_locks_;

    DISABLE_COPY(TLSRWLock);
    DISABLE_MOVE(TLSRWLock);

 public:
    TLSRWLock() : n_writers_(0) {
        for (size_t i = 0; i < reader_locks_.size(); i++) AtomicStore(reader_locks_[i], 0);
    }

    void ReadLock(int tid) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);
        while (true) {
            // if re-entry, dont' wait for writers to avoid deadlock
            if (AtomicFetchInc(reader_locks_[tid]) > 0) return;
            // otherwise, wait for writers if necessary
            if (AtomicLoad(n_writers_) == 0) break;
            // there is writer, release read lock and wait
            AtomicFetchDec(reader_locks_[tid]);
            while (AtomicLoad(n_writers_)) std::this_thread::yield();
        }
    }

    void ReadLock() { ReadLock(GetMyThreadId()); }

    void ReadUnlock(int tid) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);
        int64_t r = AtomicFetchDec(reader_locks_[tid]);
        FMA_DBG_CHECK_GE(r, 1);
    }

    void ReadUnlock() { ReadUnlock(GetMyThreadId()); }

    void WriteLock(int tid) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);

        while (AtomicFetchInc(n_writers_) != 0) {
            // there are already some writer, wait until it is done
            AtomicFetchDec(n_writers_);
            // If this thread already holds a read lock, we must release the read lock
            // so that the writer can continue. But simply releasing the read lock is
            // not a good idea since this thread is expected to hold the read lock and
            // then upgrade. So we have to throw an exception so the caller can wind back
            // everything and retry.
            if (AtomicLoad(reader_locks_[tid]) != 0)
                throw LockUpgradeConflictException();
            while (AtomicLoad(n_writers_)) std::this_thread::yield();
            // no writer now, try again
            continue;
        }
        // I am the first writer, check for readers
        for (size_t i = 0; i < reader_locks_.size(); i++) {
            if (tid == (int)i) continue;  // upgrade from read to write lock automatically
            // wait for each reader
            while (AtomicLoad(reader_locks_[i])) std::this_thread::yield();
        }
    }

    // get an exclusive write lock
    // if the calling thread already has read locks, they are ignored and
    // the write lock will be obtained if other threads does not have locks.
    void WriteLock() { WriteLock(GetMyThreadId()); }

    void WriteUnlock(int tid = GetMyThreadId()) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);
        int64_t r = AtomicFetchDec(n_writers_);
        FMA_DBG_CHECK_GE(r, 1);
    }
};

class LockInterruptedException : public std::runtime_error {
 public:
    LockInterruptedException() : std::runtime_error("Lock interrupted.") {}
};

// a TLSRWLock that takes an extra function to determine if we should interrupt
// the locking process. The function is used when trying to get a lock. If the
// function returns true, lock is not obtained and ReadLock() or WriteLock() returns
// false.
template <typename ShouldInterruptFuncT>
class InterruptableTLSRWLock {
    std::atomic<int64_t> n_writers_;
    std::atomic<int> curr_writer_;
    StaticCacheAlignedVector<std::atomic<int64_t>, FMA_MAX_THREADS> reader_locks_;
    ShouldInterruptFuncT should_interrupt_;

    DISABLE_COPY(InterruptableTLSRWLock);
    DISABLE_MOVE(InterruptableTLSRWLock);

    void ThrowIfShouldIntertupt() {
        if (should_interrupt_ && should_interrupt_()) throw LockInterruptedException();
    }
 public:
    explicit InterruptableTLSRWLock(const ShouldInterruptFuncT& interrupt = ShouldInterruptFuncT())
        : n_writers_(0), curr_writer_(-1), should_interrupt_(interrupt) {
        for (size_t i = 0; i < reader_locks_.size(); i++) AtomicStore(reader_locks_[i], 0);
    }

    void ReadLock(int tid) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);
        while (true) {
            // if re-entry, we already have the lock
            if (AtomicFetchInc(reader_locks_[tid]) > 0) return;
            // otherwise, wait for writers if necessary
            // we might already have write lock, in this case just return
            if (AtomicLoad(n_writers_) == 0 || AtomicLoad(curr_writer_) == tid) return;
            // there is writer, release read lock and wait
            AtomicFetchDec(reader_locks_[tid]);
            while (AtomicLoad(n_writers_)) {
                ThrowIfShouldIntertupt();
                std::this_thread::yield();
            }
        }
    }

    void ReadLock() { ReadLock(GetMyThreadId()); }

    void ReadUnlock(int tid) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);
        int64_t r = AtomicFetchDec(reader_locks_[tid]);
        FMA_DBG_CHECK_GE(r, 1);
    }

    void ReadUnlock() { ReadUnlock(GetMyThreadId()); }

    void WriteLock(int tid) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);
        while (AtomicFetchInc(n_writers_) != 0) {
            // if we already have the lock
            if (_F_UNLIKELY(AtomicLoad(curr_writer_) == tid)) break;
            // there is already some other writer, wait until it is done
            AtomicFetchDec(n_writers_);
            if (_F_UNLIKELY(AtomicLoad(reader_locks_[tid]) > 0)) {
                // Trying to upgrade lock while another thread has write lock.
                // This causes deadlock if we wait, so need to abort here.
                throw LockUpgradeConflictException();
            }
            while (AtomicLoad(n_writers_)) {
                ThrowIfShouldIntertupt();
                std::this_thread::yield();
            }
            // no writer now, try again
        }
        AtomicStore(curr_writer_, tid);
        // I am the first writer, check for readers
        for (size_t i = 0; i < reader_locks_.size(); i++) {
            if (tid == (int)i) continue;  // upgrade from read to write lock
            // wait for each reader
            while (AtomicLoad(reader_locks_[i])) {
                if (_F_UNLIKELY(should_interrupt_ && should_interrupt_())) {
                    // release write lock
                    AtomicFetchDec(n_writers_);
                    AtomicStore(curr_writer_, -1);
                    throw LockInterruptedException();
                }
                std::this_thread::yield();
            }
        }
    }

    void WriteLock() { WriteLock(GetMyThreadId()); }

    void WriteUnlock(int tid) {
        FMA_DBG_CHECK_LT(tid, FMA_MAX_THREADS);
        AtomicStore(curr_writer_, -1);
        int64_t r = AtomicFetchDec(n_writers_);
        if (r != 1) AtomicStore(curr_writer_, tid);
        FMA_DBG_CHECK_GE(r, 1);
    }

    void WriteUnlock() { WriteUnlock(GetMyThreadId()); }
};

template <typename LockT>
class InterruptableTLSAutoReadLock {
 public:
    typedef LockT LockType;

 private:
    LockT& lock_;
    int tid_;
    bool locked_;
    DISABLE_COPY(InterruptableTLSAutoReadLock);
    DISABLE_MOVE(InterruptableTLSAutoReadLock);

 public:
    InterruptableTLSAutoReadLock(LockT& l, int tid) : lock_(l), tid_(tid) {
        locked_ = lock_.ReadLock(tid_);
        if (_F_UNLIKELY(!locked_)) throw LockInterruptedException();
    }

    ~InterruptableTLSAutoReadLock() {
        if (_F_UNLIKELY(!locked_)) return;
        lock_.ReadUnlock(tid_);
    }

    void Unlock() {
        if (locked_) {
            lock_.ReadUnlock(tid_);
            locked_ = false;
        }
    }

    void Lock() {
        if (locked_) return;
        locked_ = lock_.ReadLock(tid_);
        if (_F_UNLIKELY(!locked_)) throw LockInterruptedException();
    }
};

template <typename LockT>
class InterruptableTLSAutoWriteLock {
 public:
    typedef LockT LockType;

 private:
    LockT& lock_;
    int tid_;
    bool locked_;
    DISABLE_COPY(InterruptableTLSAutoWriteLock);
    DISABLE_MOVE(InterruptableTLSAutoWriteLock);

 public:
    InterruptableTLSAutoWriteLock(LockT& l, int tid) : lock_(l), tid_(tid) {
        locked_ = lock_.WriteLock(tid_);
        if (_F_UNLIKELY(!locked_)) throw LockInterruptedException();
    }

    ~InterruptableTLSAutoWriteLock() {
        if (_F_UNLIKELY(!locked_)) return;
        lock_.WriteUnlock(tid_);
    }

    void Unlock() {
        if (locked_) {
            lock_.WriteUnlock(tid_);
            locked_ = false;
        }
    }

    void Lock() {
        if (locked_) return;
        locked_ = lock_.WriteLock(tid_);
        if (_F_UNLIKELY(!locked_)) throw LockInterruptedException();
    }
};

// Single-writer, multi-reader lock using spining
// no-reentry allowed, otherwise might cause dead-lock, e.g. in the following case:
// t1: read_lock    t2: write_lock    t1: read_lock
// t2 is waiting for n_readers to become 0, while t1 is waiting for writing to become false
class SpinningRWLock {
    std::atomic<size_t> n_readers;
    std::atomic<bool> writing;

    DISABLE_COPY(SpinningRWLock);
    DISABLE_MOVE(SpinningRWLock);

 public:
    SpinningRWLock() : n_readers(0), writing(false) {}

    void ReadLock() {
        while (true) {
            n_readers++;
            if (!writing) return;
            n_readers--;
            while (writing) std::this_thread::yield();
        }
    }

    void WriteLock() {
        bool f = false;
        while (!writing.compare_exchange_strong(f, true, std::memory_order_seq_cst)) {
            std::this_thread::yield();
            f = false;
        }
        while (n_readers) std::this_thread::yield();
    }

    void ReadUnlock() { n_readers--; }

    void WriteUnlock() { writing = false; }
};

// Single-writer, multi-reader lock
// no-reentry allowed, otherwise might cause dead-lock, e.g. in the following case:
// t1: read_lock    t2: write_lock    t1: read_lock
// t2 is waiting for n_readers to become 0, while t1 is waiting for writing to become false
class RWLock {
    std::mutex mutex_;
    std::condition_variable cv_;
    size_t n_readers_;
    bool writing_;

    DISABLE_COPY(RWLock);
    DISABLE_MOVE(RWLock);

 public:
    RWLock() : mutex_(), cv_(), n_readers_(0), writing_(false) {}

    void ReadLock() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() { return !writing_; });
        n_readers_++;
    }

    void WriteLock() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() { return !n_readers_ && !writing_; });
        writing_ = true;
    }

    void ReadUnlock() {
        std::unique_lock<std::mutex> lock(mutex_);
        n_readers_--;
        cv_.notify_all();
    }

    void WriteUnlock() {
        std::unique_lock<std::mutex> lock(mutex_);
        writing_ = false;
        cv_.notify_all();
    }
};

template <typename LockT>
class AutoReadLock {
    LockT& l_;
    bool locked_;

    DISABLE_COPY(AutoReadLock);

 public:
    explicit AutoReadLock(LockT& l) : l_(l) {
        l_.ReadLock();
        locked_ = true;
    }

    AutoReadLock(AutoReadLock&& rhs) : l_(rhs.l_), locked_(rhs.locked_) { rhs.locked_ = false; }

    AutoReadLock& operator=(AutoReadLock&&) = delete;

    ~AutoReadLock() {
        if (locked_) l_.ReadUnlock();
    }

    void Unlock() {
        l_.ReadUnlock();
        locked_ = false;
    }
};

template <typename LockT>
class AutoWriteLock {
    LockT& l_;
    bool locked_;

    DISABLE_COPY(AutoWriteLock);

 public:
    explicit AutoWriteLock(LockT& l) : l_(l) {
        l_.WriteLock();
        locked_ = true;
    }

    AutoWriteLock(AutoWriteLock&& rhs) : l_(rhs.l_), locked_(rhs.locked_) { rhs.locked_ = false; }

    AutoWriteLock& operator=(AutoWriteLock&&) = delete;

    ~AutoWriteLock() {
        if (locked_) l_.WriteUnlock();
    }

    void Unlock() {
        l_.WriteUnlock();
        locked_ = false;
    }
};

class TLSAutoReadLock {
    TLSRWLock& l_;
    int tid_;

    DISABLE_COPY(TLSAutoReadLock);
    DISABLE_MOVE(TLSAutoReadLock);

 public:
    TLSAutoReadLock(TLSRWLock& l, int tid) : l_(l), tid_(tid) { l_.ReadLock(tid_); }

    ~TLSAutoReadLock() { l_.ReadUnlock(tid_); }
};

class TLSAutoWriteLock {
    TLSRWLock& l_;
    int tid_;

    DISABLE_COPY(TLSAutoWriteLock);
    DISABLE_MOVE(TLSAutoWriteLock);

 public:
    TLSAutoWriteLock(TLSRWLock& l, int tid) : l_(l), tid_(tid) { l_.WriteLock(tid_); }

    ~TLSAutoWriteLock() { l_.WriteUnlock(tid_); }
};
}  // namespace fma_common
