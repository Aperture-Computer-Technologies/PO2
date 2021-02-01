// Created by MassiveAtoms on 1/31/21.
//
#include <immintrin.h>
#include <nmmintrin.h>

#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>
#include <random>

using namespace std;
// randomnes gen
static std::mt19937 generator(INT32_MAX - 2120);
static std::uniform_int_distribution<int> random_int_distr(0, 255);
int gen_int() { return random_int_distr(generator); }

int main()
{
    array<char, 8> hashes;
    generate(hashes.begin(), hashes.end(), gen_int);
    for (int i = 0; i < 8; i++) {
        char tofind = hashes[i];
        cout << i << ":" << get_pos(hashes, tofind) << "\n";
    }
}