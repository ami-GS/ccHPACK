#ifndef HPACK_HUFFMAN_H_
#define HPACK_HUFFMAN_H_

#define NULL_NODE (Node*)0
#include <stdint.h>
#include <string>

typedef struct {
    uint32_t code;
    uint8_t bitLen;
} huffman_code;

struct Node {
    Node(Node* lp, Node* rp, uint32_t c) : left(lp), right(rp), code(c) {};
    ~Node();
    Node *left, *right;
    uint32_t code;
};

class HuffmanTree {
    Node* root;
    void delete_node(Node* node);
public:
    HuffmanTree();
    ~HuffmanTree(); // delete alocated tree
    uint16_t encode(uint8_t* &dst, std::string content);
};

#endif // HPACK_HUFFMAN_H_
