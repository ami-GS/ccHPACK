#include <stdint.h>
#include <vector>
#include <string>
#include "HPACK.h"
#include "hpack_table.h"

int64_t
encode_int(uint8_t* dst, uint32_t I, uint8_t N) {
    if (I < (1 << N)-1) {
        *dst = I;
        return 1;
    }

    I -= (1 << N)-1;
    *dst =  (1 << N) - 1;
    uint64_t i = 1;
    for (; I >= 128; i++) {
        *(dst+i) = (I & 0x7f) | 0x80;
        I = I >> 7;
    }
    *(dst+(i++)) = I;

    return i;
}

int64_t
hpack_encode(uint8_t* buf, std::vector<header> headers, bool from_sTable, bool from_dTable, bool is_huffman, Table* table, int dynamic_table_size) {
    uint64_t len;
    uint8_t d_table_size[100];
    uint8_t intRep[100];
    if (dynamic_table_size != -1) {
        len = encode_int(d_table_size, dynamic_table_size, 5);
        *d_table_size |= 0x20;
        // memcopy
        buf += len;
    }
    for (header h : headers) {
        int index;
        bool match = table->find_header(index, h);
        if (from_sTable && match) {
            if (from_dTable) {
                len = encode_int(intRep, index, 7);
                *intRep |= 0x80;
                // memcopy
            } else {
                len = encode_int(intRep, index, 4);
                // memcopy
                buf += len;
                len = table->pack_string(buf, h.second, is_huffman);
                buf += len;
            }
        } else if (from_sTable && !match && index > 0) {
            if (from_dTable) {
                len = encode_int(intRep, index, 6);
                *intRep |= 0x40;
                // memcopy
                table->add_header(h);
            } else {
                len = encode_int(intRep, index, 4);
                // memcopy
                buf += len;
                len = table->pack_string(buf, h.second, is_huffman);
                buf += len;
            }
        } else {
            uint8_t prefix = 0x00; // if buf is initialized by ZERO, no need.
            if (from_dTable) {
                prefix = 0x40;
                table->add_header(h);
            }
            *(buf++) = prefix;
            len = table->pack_string(buf, h.first, is_huffman);
            buf += len;
            len = table->pack_string(buf, h.second, is_huffman);
            buf += len;
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
        uint32_t index;
        if ((*buf & 0xe0) == 0x20) {
            // 7/3 Header table Size Update
            uint32_t dst = decode_int(buf, 5);
            table->set_dynamic_table_size(dst);
        }

        uint8_t nLen = 0;
        if ((*buf & 0x80) > 0)  {
            // 7.1 Indexwd Header Field
            if ((*buf & 0x7f) == 0) {
                // error
            }
            nLen = 7;
            isIndexed = true;
        } else {
            if ((*buf & 0xc0) == 0x40) {
                // 7.2.1 Literal Header Field with Incremental Indexing
                nLen = 6;
            } else {
                // when buf[cursor]&0xf0 == 0xf0
                // 7.2.2 Literal Header Field without Indexing
                // else
                // 7.2.3 Literal Header Field never Indexed
                nLen = 4;
            }
        }
        index = decode_int(++buf, nLen);
        header h = table->parse_header(index, buf, isIndexed);
        if (nLen == 6) {
            table->add_header(h);
        }
        headers.push_back(h);
    }
    return headers;
}
