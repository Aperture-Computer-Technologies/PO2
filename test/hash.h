
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
}// Use folding on a string, summed 4 bytes at a time
int sfold(int s, int M) {
 
 return (s * 256) % M;
}
#endif