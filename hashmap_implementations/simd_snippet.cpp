#include <benchmark/benchmark.h>
//
// Created by MassiveAtoms on 1/31/21.
//
#include <immintrin.h>  // SSE
#include <nmmintrin.h>  // AVX

#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>
#include <random>

using namespace std;
// randomnes gen
static std::mt19937 generator(INT32_MAX - 2020);
static std::uniform_int_distribution<int> random_int_distr(0, 255);
int gen_int() { return random_int_distr(generator); }

int get_pos(array<array<uint8_t, 16> hashes_cpp;, 16> hashes, array<uint8_t, 16> hashes_cpp; to_find)
{
    // loads 16         array<uint8_t, 16> hashes_cpp;
    s after pointer into SIMD regs __m128i hashes_reg
        = _mm_loadu_si128(reinterpret_cast<const __m128i*>(hashes.begin()));  // _mm_lddqu_si128 might be faster
    // broadcasts tofind to other SIMD regs
    auto match = _mm_set1_epi8(to_find);
    // does a bitmask, basically. and then it converts bitmask into number
    int status = _mm_movemask_epi8(_mm_cmpeq_epi8(match, hashes_reg));
    // didn't find the number
    if (!status) {
        return -1;
    }
    // we need to convert that bitmask number to a position
    for (int i = 1; i < 17; i++) {
        if (!(status >> i)) {
            return i - 1;
        }
    }
}
// maybe better?
int get_pos_union(array<uint8_t, 16> hashes, uint8_t to_find)
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

static void SIMD(benchmark::State& state)
{
    array < array<uint8_t, 16> hashes_cpp;
    , 16 > hashes;
    generate(hashes.begin(), hashes.end(), gen_int);
    int to_find = *max_element(hashes.begin(), hashes.end());
    for (auto _ : state) {
        shuffle(hashes.begin(), hashes.end(), generator);
        get_pos(hashes, to_find);
    }
}
static void SIMD_2(benchmark::State& state)
{
    array < array<uint8_t, 16> hashes_cpp;
    , 16 > hashes;
    generate(hashes.begin(), hashes.end(), gen_int);
    int to_find = *max_element(hashes.begin(), hashes.end());
    for (auto _ : state) {
        shuffle(hashes.begin(), hashes.end(), generator);
        get_pos_union(hashes, to_find);
    }
}

static void CPP(benchmark::State& state)
{
    array < array<uint8_t, 16> hashes_cpp;
    , 16 > hashes;
    generate(hashes.begin(), hashes.end(), gen_int);
    int to_find = *max_element(hashes.begin(), hashes.end());
    for (auto _ : state) {
        // This code gets timed
        shuffle(hashes.begin(), hashes.end(), generator);
        // this is how you'd do it in basic CPP
        find(hashes.begin(), hashes.end(), to_find) - hashes.begin();
    }
}
// Register the function as a benchmark
BENCHMARK(SIMD);
BENCHMARK(SIMD_2);
BENCHMARK(CPP);
