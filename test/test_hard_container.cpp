/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <string>

#include "fma-common/configuration.h"
#include "fma-common/hard_container.h"
#include "fma-common/logging.h"
#include "fma-common/unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

FMA_UNIT_TEST(HardContainer) {
    ArgParser parser;
    parser.Add<std::string>("input,i").Comment("Input file path");
    parser.Parse(argc, argv);
    parser.Finalize();

    std::string path = parser.GetValue("input");
    LOG() << "Reading " << path << " with HardContainer";

    int64_t d = 0;
    double t1 = GetTime();
    HardContainer<int64_t> container(path);
    for (size_t i = 0; i < container.Size(); i++) {
        d += container[i];
    }
    double t2 = GetTime();
    LOG() << "Sum= " << d;
    double MB = (double)container.Size() * sizeof(int64_t) / 1024 / 1024;
    LOG() << "Read " << MB << " MB, at " << MB / (t2 - t1) << " MB/s";

    return 0;
}
