
#ifndef HASHTEST_H
#define HASHTEST_H

#include <algorithm>
#include <boost/math/distributions/chi_squared.hpp>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include <bitset>

#include "random.h"

/**
 * Chi squared test as described at
 * http://staffwww.fullcoll.edu/aclifton/cs133/assignment5.html
 *
 * @param std::function<size_t(int,int)> hashfunction, int size
 * @return double probability that it is not random. So the closer the result is to 0, the better
 */
double chi_squared_test_rand(std::function<size_t(int, int)> hasher, int size) {
    int total_keys = 20 * size;
    std::vector<int> bins(size);
    for (int i = 0; i < total_keys; i++) {
        bins[hasher(gen_int(), size)]++;
    }
    double variance
        = std::accumulate(bins.begin(), bins.end(), 0.0,
                          [&](auto& acc, auto i) { return ((i - 20) * (i - 20)) + acc; })
          / 20;
    boost::math::chi_squared chi_sq_distr(size - 1);
    double prob = boost::math::cdf(chi_sq_distr, variance);
    return prob;
}

/**
 * Chi squared test as described at
 * https://en.wikipedia.org/wiki/Hash_function#Testing_and_measurement
 *
 * @param std::function<size_t(int,int)> hashfunction, int size
 * @return something between 0.95 and 1.05 if the hashfunction is uniform
 */
double chi_squared_test_wiki(std::function<size_t(int, int)> hasher, int size) {
    int total_keys = 20 * size;
    std::vector<int> bins(size);
    for (int i = 0; i < total_keys; i++) {
        bins[hasher(gen_int(), size)]++;
    }
    double top = std::accumulate(bins.begin(), bins.end(), 0,
                                 [&](auto& acc, auto i) { return (i * (i + 1) / 2.0) + acc; });
    double bottom = (total_keys * (total_keys + 2 * size - 1)) / (2 * size);
    return top / bottom;
}

int bit_histogram(std::function<size_t(int, int)> hasher, int size) {
    int total_keys = 20 * size;
    std::vector<int> bins(size);
    for (int i = 0; i < total_keys; i++) {
        bins[hasher(gen_int(), size)]++;
    }
    std::vector<std::bitset<32>> bits_bins(size);
    std::transform(bins.begin(), bins.end(), bits_bins.begin(), [&](const int a){return std::bitset<32>(a);});
    // need to count number of bits

    return 1;
}

#endif