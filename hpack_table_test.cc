#include "HPACK.h"
#include "hpack_table.h"
#include "gtest/gtest.h"

TEST(find_header_test, NormalTest) {
    Table *table = new Table();
    int num_test = 4;
    header test_headers[] = {
        header("", ""),
        header(":authority", ""),
        header(":authority", "ami-GS"),
        header("hhh", "ddd"),
    };
    int expect_index[] = {-1, 1, 1, -1};
    bool expect_match[] = {false, true, false, false};

    for (int i = 0; i < num_test; i++) {
        int actual_index = 0;
        bool actual_match = false;
        actual_match = table->find_header(actual_index, test_headers[i]);
        ASSERT_EQ(expect_match[i], actual_match);
        ASSERT_EQ(expect_index[i], actual_index);
    }
    delete table;
}

TEST(get_header_test, NormalTest) {
    Table *table = new Table();
    int num_test = 2;
    int test_indices[] = {1, 61};
    header expect_headers[] = {
        header(":authority", ""),
        header("www-authenticate", ""),
    };

    for (int i = 0; i < num_test; i++) {
        header actual_header = table->get_header(test_indices[i]);
        ASSERT_EQ(expect_headers[i], actual_header);
    }
    delete table;
}
