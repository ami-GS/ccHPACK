#ifndef HPACK_H_
#define HPACK_H_

#include <stdint.h>

uint8_t* encode_int(uint32_t I, uint8_t N);
uint64_t decode_int(uint32_t& I, uint8_t* buf, uint8_t N);

#endif // HPACK_H_
