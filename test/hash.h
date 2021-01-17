
#ifndef HASH_H
#define HASH_H

#include <functional>

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
int sfold(int s, int M) { return (s * 256) % M; }

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
#endif