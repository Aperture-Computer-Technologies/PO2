//
// Created by MassiveAtoms on 3/4/21.
//

#ifndef PO2_HELPERS_H
#define PO2_HELPERS_H
#include <vector>

using std::vector;
#include "../tools/random.h"
namespace helper {
    vector<int> prime_sizes
        = {127,    251,    479,     911,     1733,    3299,    6269,     11923,    22669,    43093,    81883,   155579,
           295601, 561667, 1067179, 2027659, 3852553, 7319857, 13907737, 26424707, 50206957, 95393219, 18124717};

    size_t next_prime(const int& n)
    {
        for (const int x : prime_sizes) {
            if (x > n) {
                size_t t = x;
                return t;
            }
        }
    }
}  // namespace helper

#endif  // PO2_HELPERS_H
