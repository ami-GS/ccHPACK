#ifndef HPACK_H_
#define HPACK_H_

#include <stdint.h>

uint16_t encode_int(uint8_t* &dst, uint32_t I, uint8_t N);
uint32_t decode_int(uint8_t* buf, uint8_t N);

#endif // HPACK_H_
