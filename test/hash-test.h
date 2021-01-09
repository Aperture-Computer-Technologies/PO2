
#ifndef HASHTEST_H
#define HASHTEST_H

#include <algorithm>
#include <bitset>
#include <boost/math/distributions/chi_squared.hpp>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

#include "../external/json.hpp"
#include "random.h"

/**
 * Chi squared test as described at
 * http://staffwww.fullcoll.edu/aclifton/cs133/assignment5.html
 * @param std::function<size_t(int,int)> hashfunction, int size, bool iterative
 * is false
 * @return double probability that it is not random. So the closer the result is
 * to 0, the better
 */
double chi_squared_test_rand(std::function<size_t(int, int)> hasher, int size, bool iter = false)
{
    int total_keys = 20 * size;
    std::vector<int> bins(size);
    for (int i = 0; i < total_keys; i++) {
        if (iter) {
            bins[hasher(i, size)]++;
        }
        else {
            bins[hasher(gen_int(), size)]++;
        }
    }

    double variance
        = std::accumulate(bins.begin(), bins.end(), 0.0, [&](auto &acc, auto i) { return ((i - 20) * (i - 20)) + acc; })
          / 20;
    boost::math::chi_squared chi_sq_distr(size - 1);
    double prob = boost::math::cdf(chi_sq_distr, variance);
    return prob;
}

/**
 * Chi squared test as described at
 * https://en.wikipedia.org/wiki/Hash_function#Testing_and_measurement
 *
 * @param std::function<size_t(int,int)> hashfunction, int size, bool iterative = false
 * @return something between 0.95 and 1.05 if the hashfunction is uniform
 */
double chi_squared_test_wiki(std::function<size_t(int, int)> hasher, int size, bool iter = false)
{
    int total_keys = 20 * size;
    std::vector<int> bins(size);
    for (int i = 0; i < total_keys; i++) {
        if (iter) {
            bins[hasher(i, size)]++;
        }
        else {
            bins[hasher(gen_int(), size)]++;
        }
    }
    double top
        = std::accumulate(bins.begin(), bins.end(), 0, [&](auto &acc, auto i) { return (i * (i + 1) / 2.0) + acc; });
    double bottom = (total_keys * (total_keys + 2 * size - 1)) / (2 * size);
    return top / bottom;
}

/**
 * a bit histogram test. This is checking how often the bits on the output side
 * land on one given random input. Returns a vector with the number of times it
 * has been 1. It __should__ return the chance that that number is random,
 * probably with the binominal distr but i need to check on stats stuff first
 * @param std::function<size_t(int,int)> hashfunction, int size, bool iterative
 * = false
 * @return
 */
std::vector<int> bit_histogram(std::function<size_t(int, int)> hasher, int size, bool iter = false)
{
    int total_keys = 20 * size;
    std::vector<int> count(32);
    for (int i = 0; i < total_keys; i++) {
        std::bitset<32> bits;
        if (iter) {
            bits = hasher(i, size);
        }
        else {
            bits = hasher(gen_int(), size);
        }
        for (int j = 0; j < 32; j++) {
            count[j] += bits[j];
        }
    }

    return count;
}
/**
 * Inserts into buckets, returns a vector with the number of items in a bucket
 * this will be used for plotting purposes (skiena 6.3, etc) and others
 * @param std::function<size_t(int,int)> hashfunction, int size, bool iterative
 * = false
 * @return
 */
std::vector<int> inserter(std::function<size_t(int, int)> hasher, int size, bool iter = false)
{
    int total_keys = 20 * size;
    std::vector<int> bins(size);
    for (int i = 0; i < total_keys; i++) {
        if (iter) {
            bins[hasher(i, size)]++;
        }
        else {
            bins[hasher(gen_int(), size)]++;
        }
    }
    return bins;
}

/**
 * benchmarks the speed of the hashfunction
 * @param std::function<size_t(int,int)> hashfunction
 * @return double representing number of nanosecs on average
 */
double bench(std::function<size_t(int, int)> hasher, int size)
{
    using namespace std::chrono;
    time_point<steady_clock> start = steady_clock::now();
    for (int i = 0; i < size; i++) {
        if (hasher(i, size) == -1) {
            std::cout << "WHAT THE FUCK \n";
        }
    }
    time_point<steady_clock> end = steady_clock::now();
    auto duration = duration_cast<nanoseconds>(end - start);
    return duration.count() / size;
}

/**
 * incorporates all the previous tests into one suite and saves a json file with
 * the results
 * @param std::function<size_t(int,int)> hashfunction, name
 * @return
 */
void hash_test_suite(std::function<size_t(int, int)> hasher, std::string name)
{
    int prime = 1009;
    int two = 1024;
    nlohmann::json result;
    result["name"] = name;
    std::cout << "testing hash function:" << name << std::endl;
    std::cout << "benchmarking time\n";
    result["avg_time"] = bench(hasher, 50000);

    std::cout << "doing chi squared tests \n";
    result["chi_wiki_prime_iter"] = chi_squared_test_wiki(hasher, prime, true);
    result["chi_wiki_prime_rand"] = chi_squared_test_wiki(hasher, prime, false);
    result["chi_wiki_two_iter"] = chi_squared_test_wiki(hasher, two, true);
    result["chi_wiki_two_rand"] = chi_squared_test_wiki(hasher, two, false);
    result["chi_other_prime_iter"] = chi_squared_test_rand(hasher, prime, true);
    result["chi_other_prime_rand"] = chi_squared_test_rand(hasher, prime, false);
    result["chi_other_two_iter"] = chi_squared_test_rand(hasher, two, true);
    result["chi_other_two_rand"] = chi_squared_test_rand(hasher, two, false);

    std::cout << "doing bit histograms\n";
    result["bithistogram_prime_iter"] = bit_histogram(hasher, prime, true);
    result["bithistogram_prime_rand"] = bit_histogram(hasher, prime, false);
    result["bithistogram_two_iter"] = bit_histogram(hasher, two, true);
    result["bithistogram_two_rand"] = bit_histogram(hasher, two, false);
    std::cout << result.dump(2);
    std::cout << "doing raw inserts\n";
    result["raw_prime_iter"] = inserter(hasher, prime, true);
    result["raw_prime_rand"] = inserter(hasher, prime, false);
    result["raw_two_iter"] = inserter(hasher, two, true);
    result["raw_two_rand"] = inserter(hasher, two, false);
    std::ofstream outputfile("./test/hashes.json", std::ios_base::app);
    outputfile << result << "\n";
}

#endif