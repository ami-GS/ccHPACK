#include <stdint.h>
#include <vector>
#include <string>
#include "HPACK.h"
#include "hpack_table.h"

uint16_t
encode_int(uint8_t* &dst, uint32_t I, uint8_t N) {
    if (I < (1 << N)-1) {
        dst = new uint8_t[1];
        *dst = I;
        return 1;
    }

    I -= (1 << N)-1;
    uint16_t i;
    uint32_t tmpI = I;
    for (i = 1; tmpI >= 128; i++) {
        tmpI = tmpI >> 7;
    } // check length
    
    dst = new uint8_t[i+1];
    *dst =  (1 << N) - 1;
    uint8_t j = 1;
    for (; I >= 128; j++) {
        *(dst+j) = (I & 0x7f) | 0x80;
        I = I >> 7;
    }
    *(dst+j) = I;

    return i+1;
}

int64_t
hpack_encode(uint8_t* buf, std::vector<header> headers, bool from_sTable, bool from_dTable, bool is_huffman, Table* table, int dynamic_table_size) {
    uint16_t len;
    uint8_t* d_table_size;
    if (dynamic_table_size != -1) {
        len = encode_int(d_table_size, dynamic_table_size, 5);
        *d_table_size |= 0x20;
        //delete d_table_size;
    }
    for (header h : headers) {
        int index;
        bool match = table->find_header(index, h);
        if (from_sTable && match) {
            uint8_t index_len = 8;
            uint8_t mask = 0;
            uint8_t* content;
            if (from_dTable) {
                index_len = 7;
                mask = 0x80;
            } else {
                content = table->pack_string(h.second, is_huffman); // temporally
            }
            uint8_t* intRep;
            len = encode_int(intRep, index, index_len);
            *intRep |= mask;
            // append gub and intRep
        } else if (from_sTable && !match && index > 0) {
            uint8_t index_len = 4;
            uint8_t mask = 0;
            if (from_dTable) {
                index_len = 6;
                mask = 0x40;
                table->add_header(h);
            }
        }
    }
    return 0;
}


uint32_t
decode_int(uint8_t* buf, uint8_t N) {
    uint32_t I = *buf & ((1 << N) - 1);
    if (I == (1 << N) -1) {
        int M = 0;
        do {
            I += (*(++buf) & 0x7f) << M;            
            M += 7;
        }
        while (*buf & 0x80);
    }
    return I;
}

std::vector< header >
hpack_decode(uint8_t* buf, Table* table) {
    std::vector< header > headers;
    while (*buf != '\0') {
        bool isIndexed = 0;
        bool isIncremental = 0;
        uint32_t index;
        if ((*buf & 0xe0) == 0x20) {
            // 7/3 Header table Size Update
            uint32_t dst = decode_int(buf, 5);
            table->set_dynamic_table_size(dst);
        }

        if ((*buf & 0x80) > 0)  {
            // 7.1 Indexwd Header Field
            if ((*buf & 0x7f) == 0) {
                // error
            }
            l = decode_int(index, ++buf, 7);
            isIndexed = true;
        } else {
            if ((*buf & 0xc0) == 0x40) {
                // 7.2.1 Literal Header Field with Incremental Indexing
                l = decode_int(index, ++buf, 6);
                isIncremental = true;
            } else if ((*buf & 0xf0) == 0xf0) {
                l = decode_int(index, ++buf, 4);
            } else {
                l = decode_int(index, ++buf, 4);
            }
        }
        index = decode_int(++buf, nLen);
        header h = table->parse_header(index, buf, isIndexed);
        //buf += l;
        if (isIncremental) {
            table->add_header(h);
        }
        headers.push_back(h);
    }
    return headers;
}
