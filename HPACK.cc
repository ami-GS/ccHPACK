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
    if (dynamic_table_size != -1) {
    }
    return 0;
}



uint64_t decode_int(uint32_t &I, uint8_t* buf, uint8_t N) {
    uint8_t* start = buf;
    I = *buf & ((1 << N) - 1);
    if (I == (1 << N) -1) {
        int M = 0;
        do {
            I += (*(++buf) & 0x7f) << M;            
            M += 7;
        }
        while (*buf & 0x80);
    }
    return buf - start + 1;
}

std::vector< header >
hpack_decode(uint8_t* buf, Table* table) {
    uint32_t loc = 0;
    std::vector< header > headers;
    while (*buf != '\0') {
        bool isIndexed = 0;
        bool isIncremental = 0;
        uint32_t index;
        uint64_t l = 0;
        if ((*buf & 0xe0) == 0x20) {
            // 7/3 Header table Size Update
            uint32_t dst = 0;
            l = decode_int(++dst, buf, 5);
            table->set_dynamic_table_size(dst);
            //buf += l;
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
        //buf += l;
        header h = table->parse_header(index, buf, isIndexed);
        //buf += l;
        if (isIncremental) {
            table->add_header(h);
        }
        headers.push_back(h);
    }
    return headers;
}

