//
// Created by MassiveAtoms on 4/7/21.
//
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

#include "LPmap.h"
#include "LPmap2.h"
using namespace std;
template <typename K, typename V>
ostream& operator<<(ostream& os, pair<K, V> obj)
{
    os << "{" << obj.first << ", " << obj.second << "}\n";
    return os;
}

int main()
{
    LP<int, int> test;
    test.insert({1, 2});
    test.insert({4, 5});
    test.insert({6, 8});
    test.erase(4);
    for (auto it = test.begin(); it != test.end(); it++) {
        cout << *(it) << "\n";
        it->second = 1000;
    }
    for (auto it = test.begin(); it != test.end(); it++) {
        cout << *(it) << "\n";
    }
    auto it = test.begin();

    cout << sizeof(it) << ", " << sizeof(int*);
}
