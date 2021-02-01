#include <algorithm>
#include <iostream>
#include <vector>

#include "./../tools/random.h"
#include "immintrin.h"  // for AVX
#include "nmmintrin.h"  // for SSE4.2

using std::cerr;
using std::cout;
using std::vector;
/*
these tests won't actually use the passed hashmap, we just need the type passed
*/
template <class CustomHashmap>
bool creation(CustomHashmap h)
{
    CustomHashmap testmap;
    return true;
}

template <class CustomHashmap>
bool creation_reserve(CustomHashmap h)
{
    CustomHashmap testmap;
    testmap.reserve(500);
    return true;
}
// technically, we're also testing access
// if insert and access happen to be wrong in just the right ways
// this test may be able to pass
template <class CustomHashmap>
bool single_insert(CustomHashmap a)
{
    CustomHashmap testmap;
    int key = gen_int();
    testmap.insert({key, key + 1});
    if (testmap[key] != key + 1) {
        return false;
    }
    return true;
}

template <class CustomHashmap>
bool single_insert_with_reserve(CustomHashmap a)
{
    CustomHashmap testmap;
    testmap.reserve(1);
    int key = gen_int();
    testmap.insert({key, key + 1});
    if (testmap[key] != key + 1) {
        return false;
    }
    return true;
}

// technically, we're also testing access
// if insert and access happen to be wrong in just the right ways
// this test may be able to pass
template <class CustomHashmap>
bool insert_n(CustomHashmap b, int n)
{
    vector<int> keys(n);
    CustomHashmap testmap;
    std::generate(keys.begin(), keys.end(), gen_int);
    for (const int &x : keys) {
        testmap.insert({x, x + 1});
    }
    for (const int &x : keys) {
        if (testmap[x] != x + 1) {
            return false;
        }
    }

    return true;
}

template <class CustomHashmap>
bool insert_n_with_reserve(CustomHashmap b, int n)
{
    vector<int> keys(n);
    CustomHashmap testmap;
    testmap.reserve(n);
    std::generate(keys.begin(), keys.end(), gen_int);
    for (const int &x : keys) {
        testmap.insert({x, x + 1});
    }
    for (const int &x : keys) {
        if (testmap[x] != x + 1) {
            return false;
        }
    }

    return true;
}

// if you key x exists, inserting key x again should not change the current
// value
template <class CustomHashmap>
bool insert_existing(CustomHashmap a)
{
    CustomHashmap testmap;
    int key = gen_int();
    testmap.insert({key, key + 1});
    testmap.insert({key, key + 2});
    if (testmap[key] != key + 1) {
        return false;
    }
    return true;
}

// It's ok if this doesn't work, since we said that operator[] didn't have to be
// able to insert now
//
template <class CustomHashmap>
bool overwrite_existing(CustomHashmap a)
{
    CustomHashmap testmap;
    int key = gen_int();
    testmap.insert({key, key + 1});
    testmap[key] = key + 2;
    if (testmap[key] != key + 2) {
        return false;
    }
    return true;
}

// should return default init value, in this case 0
template <class CustomHashmap>
bool access_nonexising(CustomHashmap h)
{
    CustomHashmap testmap;
    int key{gen_int()};
    testmap.insert({key, key});
    if (testmap[key + 1] != 0) {
        return false;
    }
    return true;
}
template <class CustomHashmap>
bool single_delete(CustomHashmap h)
{
    CustomHashmap testmap;
    int key{gen_int()};
    testmap.insert({key, key});
    testmap.erase(key);
    if (testmap[key] != 0) {
        return false;
    }
    return true;
}

template <class CustomHashmap>
bool delete_n(CustomHashmap b, int n)
{
    vector<int> keys(n);
    CustomHashmap testmap;
    std::generate(keys.begin(), keys.end(), gen_int);
    for (const int &x : keys) {
        testmap.insert({x, x + 1});
    }
    std::shuffle(keys.begin(), keys.end(), generator);
    for (const int &x : keys) {
        testmap.erase(x);
    }
    for (const int &x : keys) {
        if (testmap[x] != 0) {
            return false;
        }
    }
    return true;
}

template <class CustomHashmap>
bool delete_n_with_reserve(CustomHashmap b, int n)
{
    vector<int> keys(n);
    CustomHashmap testmap;
    testmap.reserve(n);
    std::generate(keys.begin(), keys.end(), gen_int);
    for (const int &x : keys) {
        testmap.insert({x, x + 1});
    }
    std::shuffle(keys.begin(), keys.end(), generator);
    for (const int &x : keys) {
        testmap.erase(x);
    }
    for (const int &x : keys) {
        if (testmap[x] != 0) {
            return false;
        }
    }
    return true;
}

template <class CustomHashmap>
void hashmap_test_suite(CustomHashmap h)
{
    if (creation(h)) {
        std::cout << "creation Passed\n";
    }
    else {
        cerr << "creation Failed!\n";
    }

    if (creation_reserve(h)) {
        std::cout << "Creation with reserve Passed\n";
    }
    else {
        cerr << "Creation with reserve  Failed!\n";
    }
    if (single_insert(h)) {
        std::cout << "Single insert Passed\n";
    }
    else {
        cerr << "Single insert Failed!\n";
    }
    if (single_insert_with_reserve(h)) {
        std::cout << "Single insert with reserve Passed\n";
    }
    else {
        cerr << "Single insert with reserve Failed!\n";
    }
    if (insert_n(h, 10)) {
        std::cout << "inserting 10 keys Passed\n";
    }
    else {
        cerr << "inserting 10 keys  Failed!\n";
    }
    if (insert_n(h, 1000)) {
        std::cout << "inserting 1000 keys Passed\n";
    }
    else {
        cerr << "inserting 1000 keys  Failed!\n";
    }
    if (insert_n_with_reserve(h, 10)) {
        std::cout << "Inserting 10 with reserve Passed\n";
    }
    else {
        cerr << "Inserting 10 with reserve  Failed!\n";
    }
    if (insert_n_with_reserve(h, 1000)) {
        std::cout << "Inserting 1000 with reserve Passed\n";
    }
    else {
        cerr << "Inserting 1000 with reserve  Failed!\n";
    }
    if (insert_existing(h)) {
        std::cout << "Inserting existing key Passed\n";
    }
    else {
        cerr << "Inserting existing key Failed!\n";
    }
    if (overwrite_existing(h)) {
        std::cout << "Overwriting existing Passed\n";
    }
    else {
        cerr << "Overwriting existing Failed!, but that's ok for now.\n";
    }
    if (access_nonexising(h)) {
        std::cout << "Access nonexisting Passed\n";
    }
    else {
        cerr << "Access nonexisting Failed!\n";
    }
    if (single_delete(h)) {
        std::cout << "Single Delete Passed\n";
    }
    else {
        cerr << "Single Delete Failed!\n";
    }
    if (delete_n(h, 10)) {
        std::cout << "Delete 10 keys Passed\n";
    }
    else {
        cerr << "Delete 10 keys  Failed!\n";
    }
    if (delete_n(h, 1000)) {
        std::cout << "Delete 1000 keys Passed\n";
    }
    else {
        cerr << "Delete 1000 keys  Failed!\n";
    }
    if (delete_n_with_reserve(h, 10)) {
        std::cout << "reserve+Delete Passed\n";
    }
    else {
        cerr << "Reserve+Delete Failed!\n";
    }
}