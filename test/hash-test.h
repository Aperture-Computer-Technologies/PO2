
#ifndef HASHTEST_H
#define HASHTEST_H

#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include <boost/math/distributions/chi_squared.hpp>
#include "random.h"

double chi_squared_test_rand(std::function<size_t(int, int)> hasher, int size) {
  using std::vector;
  int total_keys = 2 * size;
  vector<int> bins(size);
  for (int i = 0; i < total_keys; i++) {
    bins[hasher(gen_int(), size)]++;
  }
  double top2 = std::accumulate(bins.begin(), bins.end(), 0,
                                [&](auto& acc, auto i) { return ((i - 20) * (i - 20)) + acc; }) /20;
    boost::math::chi_squared c2d(65535.0);
    std::cout << boost::math::cdf(c2d, top2) << "\n";


  double top = std::accumulate(bins.begin(), bins.end(), 0,
                               [&](auto& acc, auto i) { return (i * (i + 1) / 2.0) + acc; });
  double bottom = (total_keys / (2.0 * size)) * (total_keys + 2 * size - 1);
  return top / bottom;
}

double chi_squared_test_iter(std::function<size_t(int, int)> hasher, int size) {
  using std::vector;
  int total_keys = 20 * size;
  vector<int> bins(size);
  for (int i = 0; i < total_keys; i++) {
    bins[hasher(gen_int(), size)]++;
  }

  double top = std::accumulate(bins.begin(), bins.end(), 0,
                               [&](auto& acc, auto i) { return (i * (i + 1) / 2.0) + acc; });
  double bottom = (total_keys / (2.0 * size)) * (total_keys + 2 * size - 1);
  return top / bottom;
}

#endif