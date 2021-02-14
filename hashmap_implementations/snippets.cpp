//
#include <bits/stdint-intn.h>
#include <immintrin.h>  // SSE
#include <nmmintrin.h>  // AVX

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

using namespace std;

// randomnes gen
static std::mt19937 generator(INT32_MAX - 2120);
static std::uniform_int_distribution<int> random_int_distr(0, 255);
uint8_t gen_int() { return random_int_distr(generator); }

template <typename arraylike>
void print_debug(const arraylike& iter, string name)
{
    string filler = "               ";
    name += filler.substr(0, 16 - name.size());
    cout << name << "[";
    for (const auto& x : iter) {
        cout << setfill('0') << setw(2) << hex << +x << ",";
    }
    cout << dec << "]\n";
}

int get_pos(array<uint8_t, 16> hashes, uint8_t to_find)
{
    // union, so we can input array, and handle it as m128i
    union {
        __m128i hashes_reg;
        array<uint8_t, 16> hashes_cpp;
    };
    hashes_cpp = hashes;

    // broadcasts tofind to other SIMD regs
    __m128i mask = _mm_set1_epi8(to_find);

    union {
        array<uint8_t, 16> iter;
        __m128i res;
        // find way to have 128bit int, so you can check if zero for item not in arr before itering
    };
    // does a bitmask, basically.
    res = _mm_cmpeq_epi8(mask, hashes_reg);
    int counter = 0;
    for (const auto& x : iter) {
        if (x == 255) {
            return counter;
        }
        counter += 1;
    }
    return -1;
};

int main()
{
    array<uint8_t, 16> hashes;
    generate(hashes.begin(), hashes.end(), gen_int);
    for (int i = 0; i < 16; i++) {
        int find = hashes[i];
        int pos = get_pos(hashes, find);
        cout << i << ":" << pos << "\n";
    }
}