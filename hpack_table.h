#ifndef HPACK_TABLE_H_
#define HPACK_TABLE_H_

#include <stdint.h>
#include <utility>
#include <string>
#include "hpack_huffman.h"

typedef std::pair<std::string, std::string> header;

struct RingTable {
    header h;
    RingTable *nxt, *pre;
};

class Table {
    RingTable *head, *tail;
    HuffmanTree *huffman;
    uint32_t entry_size;
    uint32_t entry_num;
    uint32_t dynamic_table_size;
public:
    Table();
    ~Table();
    bool find_header(int &index, const header h);
    void delete_last_entry();
    void add_header(const header h);
    void set_dynamic_table_size(uint32_t size);
    std::string parse_string(const uint8_t* buf);
    int64_t pack_string(uint8_t* buf, const std::string content, bool to_huffman);
    header get_header(uint32_t index);
    header parse_header(uint32_t index, const uint8_t* buf, bool isIndexed);
};

#endif // HPACK_TABLE_H_
