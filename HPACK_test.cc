#include "HPACK.h"
#include "gtest/gtest.h"
#include <string.h>

TEST(encode_intTest, NormalTest) {
    uint8_t dst[10];
    uint8_t expect[][5] = {
        {0x01, 0x00},
        {0x0f, 0x01},
        //{0x1f, 0xa1, 0x8d, 0xb7, 0x01},
    };
    uint64_t len = encode_int(dst, 1, 1);
    EXPECT_EQ(2, len);
    EXPECT_TRUE(0 == std::memcmp(dst, expect[0], len));
    len = encode_int(dst, 16, 4);
    EXPECT_EQ(2, len);
    EXPECT_TRUE(0 == std::memcmp(dst, expect[1], len));
    /*
      len = encode_int(dst, 3000000, 5);
      EXPECT_EQ(5, len);
      EXPECT_TRUE(0 == std::memcmp(dst, expect[1], len));
    */
}

TEST(decode_intTest, NormalTest) {
    uint32_t dst = 0;
    uint8_t data[][5] = {
        {0x01, 0x00},
        {0x0f, 0x01},
    };
    EXPECT_EQ(1, decode_int(data[0], 1));
    EXPECT_EQ(16, decode_int(data[1], 4));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
