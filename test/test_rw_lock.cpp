/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/logging.h"
#include "fma-common/rw_lock.h"
#include "fma-common/unit_test_utils.h"

using namespace fma_common;

FMA_UNIT_TEST(RWLock) {
    size_t n = 3;
    size_t nr = 10;
    size_t nw = 10;
    size_t nsleep = 10;

    SpinningRWLock spinning_rw_lock;
    { AutoReadLock<SpinningRWLock> r_lock(spinning_rw_lock); }
    { AutoWriteLock<SpinningRWLock> w_lock(spinning_rw_lock); }
    RWLock rw_lock;
    { AutoReadLock<RWLock> r_lock(rw_lock); }
    { AutoWriteLock<RWLock> w_lock(rw_lock); }
    TLSRWLock tls_lock;
    {
        AutoReadLock<TLSRWLock> r_lock(tls_lock);
        r_lock.Unlock();
        tls_lock.ReadLock();
        tls_lock.ReadLock();
        tls_lock.ReadUnlock();
        tls_lock.ReadUnlock();
        AutoWriteLock<TLSRWLock> w_lock(tls_lock);
        w_lock.Unlock();
    }
    {
        FMA_LOG() << "Testing TLSRWLock dead lock";
        // test read-write dead lock
        // t1 read, t2 write, t1 read
        std::thread reader([&]() {
            AutoReadLock<TLSRWLock> l1(tls_lock);
            SleepUs(500000);
            AutoReadLock<TLSRWLock> l2(tls_lock);
        });
        std::thread writer([&]() {
            SleepUs(200000);
            AutoWriteLock<TLSRWLock> l(tls_lock);
        });
        reader.join();
        writer.join();
    }
    {
        std::atomic<int64_t> readers(0);
        std::atomic<int64_t> writers(0);

        for (size_t i = 0; i < n; i++) {
            FMA_LOG() << i << "th run";
            std::vector<std::thread> rthrs;
            static unsigned int seed = 0;
            for (size_t i = 0; i < nr; i++) {
                rthrs.emplace_back([i, nsleep, &tls_lock, &readers, &writers]() {
                    for (size_t j = 0; j < nsleep; j++) {
                        AutoReadLock<TLSRWLock> l(tls_lock);
                        FMA_LOG() << "Reader " << i << " reading " << j;
                        readers++;
                        SleepUs(rand_r(&seed) % 100);
                        readers--;
                        FMA_CHECK_EQ(writers, 0);
                    }
                });
            }
            std::vector<std::thread> wthrs;
            for (size_t i = 0; i < nw; i++) {
                wthrs.emplace_back([i, nsleep, &tls_lock, &readers, &writers]() {
                    for (size_t j = 0; j < nsleep; j++) {
                        AutoWriteLock<TLSRWLock> l(tls_lock);
                        FMA_LOG() << "Writer " << i << " writing " << j;
                        writers++;
                        SleepUs(rand_r(&seed) % 100);
                        writers--;
                        FMA_CHECK_EQ(writers, 0);
                        FMA_CHECK_EQ(readers, 0);
                    }
                });
            }
            for (auto &t : rthrs) t.join();
            for (auto &t : wthrs) t.join();
            FMA_CHECK_EQ(writers, 0);
            FMA_CHECK_EQ(readers, 0);
        }
    }
    return 0;
}
