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

const static std::string TestCases[] = {
    "hpack-test-case/haskell-http2-naive/",
    "hpack-test-case/haskell-http2-naive-huffman/",
    "hpack-test-case/haskell-http2-static/",
    "hpack-test-case/haskell-http2-static-huffman/",
    "hpack-test-case/haskell-http2-linear/",
    "hpack-test-case/haskell-http2-linear-huffman/",
};

const static std::string out_tmp_file = "filename.txt";

std::vector<std::string>
read_json_files(const std::string testcase) {
    std::string call_str = "ls " + testcase + " > " + out_tmp_file;
    int len = testcase.length();
    char* call = new char[len+1];
    memcpy(call, call_str.c_str(), len+1);
    system(call);
    delete [] call;
    std::ifstream fnames(out_tmp_file);
    if (fnames.fail()) {
        std::cerr  << "fail to open" << out_tmp_file << std::endl;
    }
    std::string field;
    std::vector<std::string> jsons;
    while (std::getline(fnames, field)) {
        jsons.push_back(field);
    }
    return jsons;
}

bool
read_json_as_pico(picojson::value& v, const std::string path) {
    std::ifstream ifs(path);

    if (ifs.fail()) {
        std::cerr  << "fail to open" << std::endl;
        return false;
    }
    std::string str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());
    std::string err = picojson::parse(v, str);
    if (! err.empty()) {
        std::cerr << err << std::endl;
        return false;
    }
    return true;
}

bool
read_header_wire(std::vector<header>& ans_headers, std::string& wire, picojson::array::iterator it_seqno) {
    picojson::object obj_in = it_seqno->get<picojson::object>();
    wire = obj_in["wire"].to_str();
    picojson::array json_headers = obj_in["headers"].get<picojson::array>();
    picojson::array::iterator it_headers;

    for (it_headers = json_headers.begin(); it_headers != json_headers.end(); it_headers++) {
        picojson::object content = it_headers->get<picojson::object>();
        picojson::object::iterator it = content.begin();
        ans_headers.push_back(header(it->first, it->second.to_str()));
    }
    return true;
}

bool
wire2byte(uint8_t *wire_byte, const std::string wire) {
    for (int i = 0; i < wire.length(); i += 2) {
        char tmp_c[3];
        wire.copy(tmp_c, 2, i);
        tmp_c[2] = '\0';
        std::string tmp_s = std::string(tmp_c);
        *(wire_byte+i/2) = (uint8_t)std::stoi(tmp_s, nullptr, 16);
    }
    return true;
}

TEST(encodeTest, NormalTest) {
    for (const std::string testcase : TestCases) {
        std::vector<std::string> jsons = read_json_files(testcase);

        bool from_header = std::string::npos != testcase.find("linear", 0);
        bool from_static = from_header || std::string::npos != testcase.find("static", 0);
        bool is_huffman = std::string::npos != testcase.find("huffman", 0);
        std::cout << testcase << " " << from_header << from_static << is_huffman << std::endl;

        Table* table = new Table();
        for (std::string json_file : jsons) {
            picojson::value v;
            bool err = read_json_as_pico(v, testcase + json_file);
            if (!err) {
            }

            picojson::object obj = v.get<picojson::object>();
            picojson::array arr = obj["cases"].get<picojson::array>();
            picojson::array::iterator it_seqno;
            for (it_seqno = arr.begin(); it_seqno != arr.end(); it_seqno++) {
                std::string wire;
                std::vector<header> ans_headers;
                err = read_header_wire(ans_headers, wire, it_seqno);
                if (!err) {
                }

                uint8_t dst[20000];
                int64_t len = hpack_encode(dst, ans_headers,from_static,
                                           from_header, is_huffman, table, -1);

                uint8_t *wire_byte = new uint8_t[wire.length()/2];
                err = wire2byte(wire_byte, wire);
                if (!err) {
                }

                EXPECT_EQ(wire.length()/2, len);
                EXPECT_TRUE(0 == std::memcmp(dst, wire_byte, len));
                delete [] wire_byte;
            }
        }
        delete table;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
