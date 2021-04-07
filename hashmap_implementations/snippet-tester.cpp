//
// Created by MassiveAtoms on 4/7/21.
//
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <vector>

#include "LPmap.h"

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
    //    cout << pair<int,int>{5,6};
    //    *test.begin().first = 600;
    auto a = test.begin();
    (*a).second = 5000;
    cout << *a << "\n";
    cout << test[4];
}