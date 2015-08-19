#ifndef HPACK_HUFFMAN_H_
#define HPACK_HUFFMAN_H_


typedef struct huffman_code {
    code uint32_t;
    bitlen uint8_t;
};

struct Node {
    Node* left, right;
    code uint32_t;
};

#endif // HPACK_HUFFMAN_H_
