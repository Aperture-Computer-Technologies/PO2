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
#include "deprecated/LPmap2.h"
using namespace std;
template <typename K, typename V>
ostream& operator<<(ostream& os, pair<K, V> obj)
{
    os << "{" << obj.first << ", " << obj.second << "}\n";
    return os;
}

int main()
{}
