#include <stdint.h>

uint8_t* encode_int(uint32_t I, uint8_t N) {
    if (I < (1 << N)-1) {
        uint8_t* buf = new uint8_t[1];
        *buf = I;
        return buf;
    }

    I -= (1 << N)-1;
    int i;
    uint32_t tmpI = I;
    for (i = 1; tmpI >= 128; i++) {
        tmpI = tmpI >> 7;
    } // check length
    
    uint8_t* buf = new uint8_t[i];
    *(buf++) = (1 << N)-1;
    while (I >= 128) {
        *(buf++) = (I & 0x7f) | 0x80;
        I = I >> 7;
    }
    *buf = I;
    
    return buf;
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
