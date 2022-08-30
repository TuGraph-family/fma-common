/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once
#include <atomic>

namespace fma_common {
template <typename T>
T AtomicLoad(const std::atomic<T>& d) {
    return d.load(std::memory_order_acquire);
}

template <typename T, typename D>
void AtomicStore(std::atomic<T>& d, const D& new_value) {
    d.store(new_value, std::memory_order_release);
}

template <typename T>
T AtomicFetchInc(std::atomic<T>& d) {
    return d.fetch_add(1, std::memory_order_acq_rel);
}

template <typename T>
T AtomicFetchDec(std::atomic<T>& d) {
    return d.fetch_sub(1, std::memory_order_acq_rel);
}
}  // namespace fma_common
