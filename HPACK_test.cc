#include "HPACK.h"
#include "gtest/gtest.h"

TEST(encode_intTest, Positive) {
    uint8_t* dst[100];
    EXPECT_EQ(1, encode_int(dst, 1, 1));
    EXPECT_EQ(2, encode_int(dst, 16, 4));
}
