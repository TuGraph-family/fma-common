/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once
#include "fma-common/logger.h"

namespace fma_common {
#define DBG() FMA_DBG()
#define LOG() FMA_LOG()
#define WARN() FMA_WARN()
#define ERR() FMA_ERR()
#define CHECK(pred) FMA_CHECK(pred)
#define CHECK_EQ(a, b) FMA_CHECK_EQ(a, b)
#define CHECK_NEQ(a, b) FMA_CHECK_NEQ(a, b)
#define CHECK_LT(a, b) FMA_CHECK_LT(a, b)
#define CHECK_LE(a, b) FMA_CHECK_LE(a, b)
#define CHECK_GT(a, b) FMA_CHECK_GT(a, b)
#define CHECK_GE(a, b) FMA_CHECK_GE(a, b)
#define ASSERT(pred) FMA_ASSERT(pred)
#define DBG_ASSERT(pred) FMA_DBG_ASSERT(pred)
#define DBG_CHECK(pred) FMA_DBG_CHECK(pred)
#define DBG_CHECK_EQ(a, b) FMA_DBG_CHECK_EQ(a, b)
#define DBG_CHECK_NEQ(a, b) FMA_DBG_CHECK_NEQ(a, b)
#define DBG_CHECK_LT(a, b) FMA_DBG_CHECK_LT(a, b)
#define DBG_CHECK_LE(a, b) FMA_DBG_CHECK_LE(a, b)
#define DBG_CHECK_GT(a, b) FMA_DBG_CHECK_GT(a, b)
#define DBG_CHECK_GE(a, b) FMA_DBG_CHECK_GE(a, b)
#define EXIT() FMA_EXIT()
}  // namespace fma_common
