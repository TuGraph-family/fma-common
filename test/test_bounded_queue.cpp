/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <thread>

#include "fma-common/bounded_queue.h"
#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/unit_test_utils.h"
#include "fma-common/utils.h"

FMA_SET_TEST_PARAMS(BoundedQueue, "");

FMA_UNIT_TEST(BoundedQueue) {
    using namespace std;
    using namespace fma_common;
    double sleep_time = 0.1;
    Configuration config;
    config.Add(sleep_time, "sleepTime", true).Comment("Time to sleep for each task");
    config.Parse(argc, argv);
    config.Finalize();

    BoundedQueue<int> queue(3);
    std::thread push([&]() {
        for (int i = 0; i < 10; i++) {
            double t1 = GetTime();
            queue.Push(std::move(i));
            double t2 = GetTime();
            if (i >= 4) {
                ASSERT(t2 - t1 > 0.8 * sleep_time && t2 - t1 < 2 * sleep_time)
                    << "Queue didn't block properly";
                LOG() << "Blocked for " << t2 - t1 << " seconds";
            }
            LOG() << "pushed " << i;
        }
    });

    std::thread pop([&]() {
        int d = 0;
        int i = 0;
        while (queue.PopFront(d)) {
            ASSERT(d == i++) << "Pop sequence is wrong, expecting " << i << " but got " << d;
            LOG() << "popped " << d;
            LOG() << "pop sleeping for " << sleep_time << "s";
            SleepS(sleep_time);
        }
    });

    push.join();
    queue.WaitEmpty();
    queue.EndQueue();
    pop.join();
    return 0;
}
