//
// Created by MassiveAtoms on 4/7/21.
//
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

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
    LP2<int, int> test;
    test.insert({1, 2});
    test.insert({4, 5});
    auto a = test.begin();
    cout << *a;  // operator<< for pair<K,V> added but not shown
    (*a).second = 5000;
    //    cout << *(++a);
    cout << test[4];
}
