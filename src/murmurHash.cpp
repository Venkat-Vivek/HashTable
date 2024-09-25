#include "murmurHash.h"

uint32_t rotateLeft(uint32_t x, int r) {
    return (x << r) | (x >> (32 - r));
}

uint32_t mixing(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

uint32_t murmurHash(const char* key, int len, uint32_t seed) {
    const uint8_t *dataPtr = reinterpret_cast<const uint8_t*>(key);
    int nblocks = len / 4;

    uint32_t h = seed;
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;

    const uint32_t *blocks = reinterpret_cast<const uint32_t*>(key);
    for (int i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = rotateLeft(k, 15);
        k *= c2;
        h ^= k;
        h = rotateLeft(h, 13);
        h = h * 5 + 0xe6546b64;
    }

    const uint8_t *tail = dataPtr + nblocks * 4;
    uint32_t k1 = 0;

    switch (len % 4) {
        case 3:
            k1 ^= tail[2] << 16;
        case 2:
            k1 ^= tail[1] << 8;
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = rotateLeft(k1, 15);
            k1 *= c2;
            h ^= k1;
            break;
    }

    h ^= len;
    return mixing(h);
}