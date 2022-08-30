/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <ctime>
#include "fma-common/date.h"
#include "fma-common/logger.h"
#include "fma-common/unit_test_utils.h"

FMA_SET_TEST_PARAMS(Date, "");

FMA_UNIT_TEST(Date) {
    // using namespace date;
    // year_month_day tp{year(2019), month(2), day(29)};
    // auto sd = sys_days(tp);
    // FMA_LOG() << std::chrono::duration_cast<std::chrono::seconds>(sd.time_since_epoch()).count();
    tm t;
    t.tm_year = 2019;
    t.tm_mon = 2;
    t.tm_mday = 29;
    FMA_LOG() << t.tm_year << t.tm_mon << t.tm_mday;

    return 0;
}
