#include "HPACK.h"
#include "hpack_table.h"
#include "gtest/gtest.h"
#include <string.h>
#include "picojson/picojson.h"
#include <fstream>
#include <iostream>
#include <iterator>

using namespace picojson;

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
    EXPECT_EQ(2, decode_int(dst, data[0], 1));
    EXPECT_EQ(1, dst);
    EXPECT_EQ(2, decode_int(dst, data[1], 4));
    EXPECT_EQ(16, dst);
}

const static std::string TestCases[] = {
    "hpack-test-case/haskell-http2-naive/",
    "hpack-test-case/haskell-http2-naive-huffman/",
    "hpack-test-case/haskell-http2-static/",
    "hpack-test-case/haskell-http2-static-huffman/",
    "hpack-test-case/haskell-http2-linear/",
    "hpack-test-case/haskell-http2-linear-huffman/",
    "hpack-test-case/go-hpack/",
    "hpack-test-case/nghttp2/",
    "hpack-test-case/nghttp2-16384-4096/",
    "hpack-test-case/nghttp2-change-table-size/",
    "hpack-test-case/node-http2-hpack/",
};

const static std::string out_tmp_file = "filename.txt";

bool
read_json_files(std::vector<std::string> &jsons, const std::string testcase) {
    std::string call_str = "ls " + testcase + " > " + out_tmp_file;
    int len = call_str.length();
    char call[70];
    memcpy(call, call_str.c_str(), len+1);
    system(call);
    std::ifstream fnames(out_tmp_file);
    if (fnames.fail()) {
        std::cerr  << "fail to open" << out_tmp_file << std::endl;
        return false;
    }

    std::string field;
    while (std::getline(fnames, field)) {
        jsons.push_back(field);
    }

    return true;
}

bool
read_json_as_pico(value& v, const std::string path) {
    std::ifstream ifs(path);

    if (ifs.fail()) {
        std::cerr  << "fail to open" << std::endl;
        return false;
    }
    std::string str((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());
    std::string err = parse(v, str);
    if (! err.empty()) {
        std::cerr << err << std::endl;
        return false;
    }
    return true;
}

bool
read_sequence(int& table_size, std::vector<header>& ans_headers, std::string& wire, array::iterator it_seqno) {
    object obj_in = it_seqno->get<object>();
    if (obj_in["header_table_size"].to_str() != "null") {
        table_size = (int)std::stoi(obj_in["header_table_size"].to_str());
    }
    wire = obj_in["wire"].to_str();
    array json_headers = obj_in["headers"].get<array>();
    array::iterator it_headers;

    for (it_headers = json_headers.begin(); it_headers != json_headers.end(); it_headers++) {
        object content = it_headers->get<object>();
        object::iterator it = content.begin();
        ans_headers.push_back(header(it->first, it->second.to_str()));
    }
    return true;
}

bool
wire2byte(uint8_t *wire_byte, const std::string wire) {
    int len = wire.length();
    for (int i = 0; i < len; i += 2) {
        *(wire_byte+i/2) = (uint8_t)std::stoi(wire.substr(i, 2).c_str(), nullptr, 16);
    }
    return true;
}

void
detect_testcase_type(bool &from_header, bool &from_static,
                     bool &is_huffman,const std::string testcase) {
    from_header = std::string::npos != testcase.find("linear", 0);
    from_static = from_header || std::string::npos != testcase.find("static", 0);
    if (std::string::npos != testcase.find("haskell", 0)) {
        is_huffman = std::string::npos != testcase.find("huffman", 0);
    } else {
        is_huffman = true;
    }
}

void
print_wires(const uint8_t* expect, uint64_t e_len, const uint8_t* actual, uint64_t a_len) {
    std::cout << "Expect" << std::endl;
    for (int i = 0; i < e_len; i++) {
        printf("%02x", *(expect+i));
    }
    std::cout << std::endl;
    std::cout << "Actual" << std::endl;
    for (int i = 0; i < a_len; i++) {
        printf("%02x", *(actual+i));
    }
    std::cout << std::endl << std::endl;
}

TEST(HPACKeTest, NormalTest) {
    for (const std::string testcase : TestCases) {
        std::vector<std::string> jsons;
        bool err = read_json_files(jsons, testcase);
        if (!err) {
        }

        bool from_header, from_static, is_huffman;
        detect_testcase_type(from_header, from_static, is_huffman, testcase);
        std::cout << testcase << " " << from_header << from_static << is_huffman << std::endl;

        Table* encode_table = new Table();
        Table* decode_table = new Table();
        for (std::string json_file : jsons) {
            value v;
            err = read_json_as_pico(v, testcase + json_file);
            if (!err) {
            }

            object obj = v.get<object>();
            array arr = obj["cases"].get<array>();
            array::iterator it_seqno = arr.begin();
            for (int seqno = 0; it_seqno != arr.end(); seqno++, it_seqno++) {
                int table_size = -1;
                std::string wire;
                std::vector<header> expect_headers;
                err = read_sequence(table_size, expect_headers, wire, it_seqno);
                if (!err) {
                }
                uint8_t *expect_wire = new uint8_t[wire.length()/2];
                err = wire2byte(expect_wire, wire);
                if (*expect_wire == 0x0e) {
                    *expect_wire = 0x08;
                }
                if (!err) {
                }

                uint8_t actual_wire[20000];
                int64_t len = hpack_encode(actual_wire, expect_headers, from_static,
                                           from_header, is_huffman, encode_table, table_size);

                int wire_assert = std::memcmp(actual_wire, expect_wire, len);
                if (wire_assert != 0) {
                    std::cout << testcase << json_file << "  seqno: " << seqno << std::endl;
                    print_wires(expect_wire, wire.length()/2, actual_wire, len);
                    for (header head : expect_headers) {
                        std::cout << head.first << " " << head.second << std::endl;
                    }
                }
                ASSERT_EQ(wire.length()/2, len);
                ASSERT_TRUE(0 == wire_assert);

                std::vector<header> actual_headers;
                int64_t cursor = hpack_decode(actual_headers, expect_wire, decode_table, wire.length()/2);
                ASSERT_EQ(cursor, wire.length()/2);
                ASSERT_EQ(actual_headers.size(), expect_headers.size());
                for (int i = 0; i < actual_headers.size(); i++) {
                    ASSERT_EQ(actual_headers[i], expect_headers[i]);
                }

                delete [] expect_wire;
            }
        }
        delete encode_table;
        delete decode_table;
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
