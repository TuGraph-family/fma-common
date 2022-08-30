/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/cache_aligned_vector.h"
#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/unit_test_utils.h"

FMA_SET_TEST_PARAMS(CacheAlignedVector, "");

FMA_UNIT_TEST(CacheAlignedVector) {
    fma_common::StaticCacheAlignedVector<int, 1> v1;
    int x = 10;
    fma_common::StaticCacheAlignedVector<std::string, 1> v2;
    int y = 10;
    FMA_CHECK_EQ((uint64_t)&v1[0] % fma_common::FMA_CACHE_LINE_SIZE, 0);
    FMA_CHECK_EQ((uint64_t)&v1[0] % fma_common::FMA_CACHE_LINE_SIZE, 0);
    FMA_CHECK_EQ(x + y, 20);
    return 0;
}
