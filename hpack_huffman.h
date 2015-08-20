#ifndef HPACK_HUFFMAN_H_
#define HPACK_HUFFMAN_H_

#include <stdint.h>

typedef struct {
    uint32_t code;
    uint8_t bitLen;
} huffman_code;

struct Node {
    Node *left, *right;
    uint32_t code;
};

#endif // HPACK_HUFFMAN_H_
