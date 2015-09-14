#ifndef HPACK_H_
#define HPACK_H_

#include <stdint.h>
#include <vector>
#include "hpack_table.h"

int64_t encode_int(uint8_t* dst, uint32_t I, uint8_t N);
int64_t hpack_encode(uint8_t* buf, const std::vector<header> headers, bool fromsTable, bool from_dTable, bool is_huffman, Table* table, int dynamic_table_size);
uint32_t decode_int(const uint8_t* buf, uint8_t N);
std::vector<header> hpack_decode(uint8_t* buf, Table* table);

#endif // HPACK_H_
