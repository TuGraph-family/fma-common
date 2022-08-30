/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once
#include <map>

#include "fma-common/logger.h"

namespace fma_common {
// a log device that selects log device according to log levels
class LeveledLogDevice : public LogDevice {
    std::vector<std::shared_ptr<LogDevice>> devices_;

 public:
    explicit LeveledLogDevice(const std::map<LogLevel, std::shared_ptr<LogDevice>>& devices) {
        if (devices.empty()) throw std::runtime_error("Devices cannot be empty.");
        size_t n_levels = LogLevel::_LL_LAST_INVALID;
        devices_.resize(n_levels);
        size_t l = 0;
        for (auto it = devices.begin(); it != devices.end(); it++) {
            while (l <= it->first) {
                devices_[l] = it->second;
                l++;
            }
        }
        while (l < n_levels) {
            devices_[l] = devices.rbegin()->second;
            l++;
        }
    }

    virtual void WriteLine(const char* p, size_t s, LogLevel level) {
        devices_[level]->WriteLine(p, s, level);
    }

    virtual void Flush() {
        for (auto& d : devices_) d->Flush();
    }
};
}  // namespace fma_common
