/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

/*!
 * \file    fma-common\predefs.h.
 *
 * \brief   Declares the pre-defined variables.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <string>
#include <vector>

#include "fma-common/utils.h"

namespace fma_common {
/*!
 * \fn  inline const std::string& HDFS_CMD()
 *
 * \brief   Hdfs command to use. It must be directly callable. So if hdfs
 *          is not in PATH, you must specify the full path to it.
 *
 * \return  A reference to a const std::string.
 */
inline const std::string& HDFS_CMD() {
    static const std::string hdfs = "hdfs dfs ";
    return hdfs;
}

#ifdef _WIN32
inline const std::string& SUPRESS_OUTPUT() {
    static const std::string r = " >NUL 2>&1";
    // static const std::string r = " >NUL";
    return r;
}
#else
static inline const std::string& SUPRESS_OUTPUT() {
    static const std::string r = " >/dev/null 2>&1";
    return r;
}
#endif
}  // namespace fma_common
