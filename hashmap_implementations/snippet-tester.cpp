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
#include "LPmap3.h"
using namespace std;
template <typename K, typename V>
ostream& operator<<(ostream& os, pair<K, V> obj)
{
    os << "{" << obj.first << ", " << obj.second << "}\n";
    return os;
}

int main()
{
    LP3<int, int> testo;
    testo.insert({5, 6});
    testo.insert({7, 8});
    auto it = testo.begin();
    auto it2 = testo.end();
    auto it3 = testo.cbegin();
    auto it4 = testo.cend();
}
