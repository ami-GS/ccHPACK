#include "HPACK.h"
#include "hpack_table.h"
#include "gtest/gtest.h"
#include <string.h>
#include "picojson/picojson.h"
#include <fstream>
#include <iostream>
#include <iterator>


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

TEST(encodeTest, NormalTest) {
    std::ifstream ifs("hpack-test-case/haskell-http2-naive/story_00.json");
    if (ifs.fail()) {
        std::cerr  << "fail to open" << std::endl;
    }
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    picojson::value v;
    std::string err = picojson::parse(v, str);
    if (! err.empty()) {
        std::cerr << err << std::endl;
    }

    picojson::object obj = v.get<picojson::object>();
    picojson::array arr = obj["cases"].get<picojson::array>();
    picojson::array::iterator it_seqno;
    for (it_seqno = arr.begin(); it_seqno != arr.end(); it_seqno++) {
        picojson::object obj_in = it_seqno->get<picojson::object>();
        std::string wire = obj_in["wire"].to_str();
        picojson::array json_headers = obj_in["headers"].get<picojson::array>();
        picojson::array::iterator it_headers;
        std::vector<header> ans_headers;

        for (it_headers = json_headers.begin(); it_headers != json_headers.end(); it_headers++) {
            picojson::object content = it_headers->get<picojson::object>();
            picojson::object::iterator it = content.begin();
            ans_headers.push_back(header(it->first, it->second.to_str()));
        }
        Table* table = new Table();
        uint8_t dst[2000];
        int64_t len = hpack_encode(dst, ans_headers, false, false, false, table, -1);
        //EXPECT_TRUE(0 == std::memcmp(dst, wire, len));
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
