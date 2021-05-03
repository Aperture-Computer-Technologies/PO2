//
// Created by MassiveAtoms on 5/3/21.
//
#include "LPmap3.h"
#include <iostream>
using pair = std::pair<const int, int>;
int main(){
    unsigned int size = 200000;
    LP3<int,int> map{size};
    for (int i = 0; i < size; i++){
        map.insert({i,i});
    }
    // succ lookup, copy
    for (int i = 0; i < size; i++){
        if (map[i] != i){std::cout << "WUT1";}
    }
    // succ lookup, move
    for (int i = 0; i < size; i++){
        if (map[{i}] != i){std::cout << "WUT2";}
    }
    // nosucc lookup
    // succ lookup, move
    for (int i = size; i < 2 * size; i++){
        if (map.count(i)){std::cout << "WUT3";}
    }
    // delete
    for (int i = 0; i < size; i++){
        if (map.erase(i) == 0){std::cout << "WUT4";}
    }

}