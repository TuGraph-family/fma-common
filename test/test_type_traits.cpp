/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <functional>

#include "fma-common/type_traits.h"
#include "fma-common/unit_test_utils.h"

FMA_SET_TEST_PARAMS(TypeTraits, "");

using namespace fma_common;

class Foo {
 public:
    void Bar(int i, double j) { FMA_LOG() << i << ", " << j; }
};

FMA_UNIT_TEST(TypeTraits) {
    Foo f;
    _detail::ApplyTuple(std::bind(&Foo::Bar, &f, std::placeholders::_1, std::placeholders::_2),
                        std::make_tuple<int, double>(1, 2.3));

    return 0;
}
