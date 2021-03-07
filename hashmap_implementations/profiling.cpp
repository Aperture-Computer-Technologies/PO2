//
// Created by MassiveAtoms on 3/7/21.
//
#include <algorithm>
#include <vector>

#include "./../tools/random.h"
#include "nodemap1b.h"
using namespace std;

int main()
{
    vector<int> keys(10000);
    generate(keys.begin(), keys.end(), gen_integer);

    Nodemap1b<int, int> tester(20000);
    for (const int& x : keys) {
        tester.insert({x, x});
    }
}
