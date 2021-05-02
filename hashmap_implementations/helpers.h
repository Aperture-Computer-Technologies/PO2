//
// Created by MassiveAtoms on 3/4/21.
//

#ifndef PO2_HELPERS_H
#define PO2_HELPERS_H
#include <vector>
/*
 * just a quick way to get the next prime size that's almost double, added to a namespace
 * so i'm not poluting my environment.
 */
using std::vector;
#include "../tools/random.h"
namespace helper {
    static vector<size_t> prime_sizes
        = {127,        251,        479,        911,        1733,      3299,      6269,       11923,      22669,
           43093,      81883,      155579,     295601,     561667,    1067179,   2027659,    3852553,    7319857,
           13907737,   26424707,   50206957,   95393219,   143731457, 179424989, 224367413,  280465301,  350916677,
           373588249,  467886691,  573259913,  717266647,  776531999, 971057303, 1190495191, 1400305763, 1611624473,
           1824261979, 2252945627, 2685457727, 3340200581, 4000846897};

    size_t next_prime(const size_t& n)
    {
        size_t next = n;
        for (const int x : prime_sizes) {
            if (x > n) {
                next = x;
                break;
            }
        }
        return next;
    }
}  // namespace helper

#endif  // PO2_HELPERS_H
