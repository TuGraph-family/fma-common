/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <vector>

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/fma_stream.h"
#include "fma-common/unit_test_utils.h"

using namespace fma_common;

FMA_UNIT_TEST(DD) {
    Configuration config;
    int loop = 1;
    int nf = 16;
    std::string path = "./";
    config.Add(loop, "loop", true).Comment("Number of loops");
    config.Add(nf, "n_files", true).Comment("Number of files");
    config.Add(path, "path").Comment("Path prefix");
    config.Parse(argc, argv);

    for (int i = 0; i < loop; i++) {
        std::vector<OutputFmaStream> ofs(nf);
        for (int j = 0; j < nf; j++) {
            ofs[j].Open(path + std::to_string(i * nf + j));
            LOG() << i << " " << j;
        }
    }

    return 0;
}
