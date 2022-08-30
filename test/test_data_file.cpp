/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/data_file.h"
#include "fma-common/logging.h"
#include "fma-common/unit_test_utils.h"

using namespace fma_common;

FMA_UNIT_TEST(DataFile) {
    DataFileWriter<int, void> writer("test.bin", 1024, 1);
    DataFileWriter<int, int> writer2("t2.bin", 1, 1);
    return 0;
}
