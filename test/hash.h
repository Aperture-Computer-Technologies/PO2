
#ifndef HASH_H
#define HASH_H

#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>

size_t example(int key, int size) { return key % size; }

size_t dumb_bitmask(int key, int size)
{
    int ans = key >> 21;
    if (ans >= size) {
        return ans >> 1;
    }
    else {
        return ans;
    }
}  // Use folding on a string, summed 4 bytes at a time
size_t sfold(int key, int size)
{
    size_t x = key * 256;
    if (x < 0) {
        x = x * -1;
    }
    x = x % size;
    return x;
}

unsigned int PJWHash(const char* str, unsigned int length)
{
    const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
    const unsigned int ThreeQuarters = BitsInUnsignedInt * 0.75;
    const unsigned int OneEighth = BitsInUnsignedInt / 8;
    const unsigned int HighBits = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
    unsigned int hash = 0;
    unsigned int test = 0;
    unsigned int i = 0;

    for (i = 0; i < length; ++str, ++i) {
        hash = (hash << OneEighth) + (*str);

        if ((test = hash & HighBits) != 0) {
            hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
        }
    }

    return hash;
}

size_t blender_internal(size_t x)
{
    x = ((x & 0xffff000000000000) >> 48) | ((x & 0x0000ffff00000000) >> 16) | ((x & 0x00000000ffff0000) << 16)
        | ((x & 0x000000000000ffff) << 48);
    x = ((x & 0xff000000ff000000) >> 24) | ((x & 0x00ff000000ff0000) >> 8) | ((x & 0x0000ff000000ff00) << 8)
        | ((x & 0x000000ff000000ff) << 24);
    x = ((x & 0xf000f000f000f000) >> 12) | ((x & 0x0f000f000f000f00) >> 4) | ((x & 0x00f000f000f000f0) << 4)
        | ((x & 0x000f000f000f000f) << 12);
    x = ((x & 0xc0c0c0c0c0c0c0c0) >> 6) | ((x & 0x3030303030303030) >> 2) | ((x & 0x0c0c0c0c0c0c0c0c) << 2)
        | ((x & 0x0303030303030303) << 6);
    x = ((x & 0x8888888888888888) >> 3) | ((x & 0x4444444444444444) >> 1) | ((x & 0x2222222222222222) << 1)
        | ((x & 0x1111111111111111) << 3);
    return x;
}

size_t blender(int key, size_t n) { return blender_internal(key); };
#endif