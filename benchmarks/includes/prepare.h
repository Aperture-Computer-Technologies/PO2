
#ifndef PREPARE_H
#define PREPARE_H

#include <string>

// hashmaps and hash
//#include <boost/unordered_map.hpp>
#include <unordered_map>

using std::string;

// this is the prepare function, again using template
// this is for all maps which only need this. Below are the ones that
// need something different
template <class T>
void prepare(T &map, int size)
{
    map.reserve(size+10000);
}

#endif /* TESTS_H */