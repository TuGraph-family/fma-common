/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/logger.h"
#include "fma-common/unit_test_utils.h"
#include "fma-common/logging.h"

FMA_SET_TEST_PARAMS(LevelLog, "");

FMA_UNIT_TEST(LevelLog) {
    // test err info in different log level

    using namespace fma_common;
    fma_common::Logger::Get().SetFormatter(std::make_shared<TimedLogFormatter>());
    fma_common::Logger::Get().SetLevel(LogLevel::LL_ERROR);
    FMA_LOG() << "This is Error level" << "log format";
    ::fma_common::LoggerStream(::fma_common::Logger::Get(), LogLevel::LL_ERROR)
    << "This is Error level" << "log format";
    fma_common::Logger::Get().SetLevel(LogLevel::LL_INFO);
    FMA_LOG() << "This is Info level" << "log format";
    ::fma_common::LoggerStream(::fma_common::Logger::Get(), LogLevel::LL_ERROR)
    << "This is Info level" << "output format";
    fma_common::Logger::Get().SetLevel(LogLevel::LL_DEBUG);
    FMA_LOG() << "This is Debug level" << "log format";
    ::fma_common::LoggerStream(::fma_common::Logger::Get(), LogLevel::LL_ERROR)
    << "This is Debug level" << "log format";

    std::ostringstream header;
    header << "\n"
    << "**********************************************************************" << "\n"
        << "*                  TuGraph Graph Database v"
        << "*" << "\n"
        << "*                                                                    *" << "\n"
        << "*    Copyright(C) 2018-2021 Ant Group. All rights reserved.          *" << "\n"
        << "*                                                                    *" << "\n"
        << "**********************************************************************" << "\n"
        << "Server is configured with the following parameters:\n";
    FMA_LOG() << header.str();

    return 0;
}
