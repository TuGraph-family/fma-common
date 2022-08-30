/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <atomic>

#include "fma-common/logger.h"
#include "fma-common/timed_task.h"
#include "fma-common/unit_test_utils.h"
#include "fma-common/utils.h"

FMA_SET_TEST_PARAMS(TimedTask, "");

FMA_UNIT_TEST(TimedTask) {
    using namespace fma_common;

    {
        TimedTaskScheduler scheduler;
        std::atomic<int> n(0);
        auto task = scheduler.ScheduleReccurringTask(100, [&n](TimedTask *) {
            FMA_LOG() << "recurring " << n;
            n++;
        });
        while (n < 10) SleepS(0);
        task->Cancel();
        scheduler.WaitTillClear();
    }
    {
        TimedTaskScheduler scheduler;
        for (size_t i = 10; i > 0; i--) {
            scheduler.RunAfterDuration(i * 100, [i](TimedTask *) { FMA_LOG() << i; });
        }
        scheduler.WaitTillClear();
        for (size_t i = 10; i > 0; i--) {
            scheduler.RunAfterDuration(i * 100, [i](TimedTask *) { FMA_LOG() << i; });
        }
    }
    {
        TimedTaskScheduler scheduler;
        std::vector<std::thread> thrs;
        std::vector<double> times(11, 0);
        for (int i = 10; i >= 0; i--) {
            thrs.emplace_back([&scheduler, i, &times]() {
                auto task = scheduler.RunAfterDuration(
                    i * 100, [i, &times](TimedTask *) { times[i] = GetTime(); });
            });
        }
        for (auto &thr : thrs) thr.join();
        scheduler.WaitTillClear();
        for (size_t i = 1; i <= 10; i++) {
            FMA_CHECK_GT(times[i], times[i - 1]);
        }

        std::atomic<int> n(0);
        {
            AutoCancelTimedTask guard(scheduler.ScheduleReccurringTask(100, [&n](TimedTask *) {
                FMA_LOG() << "recurring " << n;
                n++;
            }));
            fma_common::SleepS(1);
        }
        FMA_ASSERT(n >= 9 && n <= 11);
    }
    return 0;
}
