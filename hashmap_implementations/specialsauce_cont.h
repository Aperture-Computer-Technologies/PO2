//
// Created by MassiveAtoms on 3/6/21.
//

#ifndef PO2_SPECIALSAUCE_CONT_H
#define PO2_SPECIALSAUCE_CONT_H
#include <algorithm>
#include <memory>
#include <numeric>
#include <tuple>
#include <vector>

/*
 *  * this specialized construct does the following:
 * 1. it doesn't reallocate anything, even if it's size increases.
 * 2. be a mostly continuous memory region, to have better cache locality
 * than just allocating space in the heap ans storing their pointers.
 * My reasoning is that if i just do new KVpair and store it's pointers, KVpairs will be spread
 * all over the heap, meaning worse cache locality.
 */
template <typename T>
class Cont {
  public:
    typedef T ValueType;
    Cont();
    Cont(int siz);
    //    Cont(const Cont& c) = delete; // copy constructor
    //    Cont(Cont<T>&& c); // move constructor
    //    ~Cont();

    T* insert(T& elem);
    void remove(T* elem);
    void reserve(int size);
    //    Cont& operator=(Cont&& c); // move assignment operatr

  private:
    T* empty_slot_selector();
    int n_empty;
    vector<std::shared_ptr<vector<T>>> store;
    vector<vector<T*>> empty_slots;
    vector<int> segment_ends;
    vector<int> actual_store_sizes;
};

template <typename T>
Cont<T>::Cont(int size)
    : store{std::shared_ptr<vector<T>>(new vector<T>)},
      empty_slots{{}},
      segment_ends{size},
      n_empty{0},
      actual_store_sizes{{0}}
{
    store.back()->reserve(size);
}
template <typename T>
Cont<T>::Cont() : Cont{Cont<T>(251)}
{
}

// template <typename T>
// Cont<T>::Cont(Cont<T>&& c)
//    : n_empty{c.n_empty},
//      store{c.store},
//      empty_slots{c.empty_slots},
//      segment_ends{c.segment_ends},
//      actual_store_sizes{c.actual_store_sizes}
//{
//    c.n_empty =0;
//    c.store.clear();
//    c.empty_slots.clear();
//    c.segment_ends.clear();
//    c.actual_store_sizes.clear();
//}
// template <typename T>
// Cont<T>::~Cont()
//{
//    for (auto& x : store) {
//        delete x;
//    }
//}

// template <typename T>
// Cont<T>& Cont<T>::operator=(Cont<T>&& c){
//    if (this != &c){
//        n_empty = c.n_empty;
//        c.n_empty = 0;
//        store = c.store;
//        c.store.clear();
//        empty_slots = c.empty_slots;
//        c.empty_slots.clear();
//        segment_ends = c.segment_ends;
//        actual_store_sizes = c.actual_store_sizes;
//        c.segment_ends.clear();
//        c.actual_store_sizes.clear();
//
//    }
//    return *this;
//}

template <typename T>
T* Cont<T>::insert(T& elem)
{
    T* adress;
    if (n_empty) {
        adress = empty_slot_selector();
        *adress = elem;
        return adress;
    }
    // check if vector is in danger of reallocation
    bool is_full = (store.back()->size() == segment_ends.back());
    if (is_full) {
        int size = std::accumulate(segment_ends.begin(), segment_ends.end(), 0);
        store.push_back(std::shared_ptr<vector<T>>(new vector<T>));
        store.back()->reserve(size);
        actual_store_sizes.push_back(0);
        empty_slots.push_back({});
        segment_ends.push_back(size);
    }
    actual_store_sizes.back()++;
    // no danger of reallocation
    store.back()->push_back(elem);
    return &(store.back()->back());
}
template <typename T>
void Cont<T>::remove(T* elem)
{
    for (int i = 0; i < store.size(); i++) {
        T* begin = store[i]->data();
        if (elem >= begin && elem <= begin + store[i]->size()) {
            empty_slots[i].push_back(elem);
            n_empty++;
            actual_store_sizes[i]--;
            if (!actual_store_sizes[i] && actual_store_sizes.size() > 1) {
                actual_store_sizes.erase(actual_store_sizes.begin() + i);
                store.erase(store.begin() + i);
                n_empty -= empty_slots[i].size();
                empty_slots.erase(empty_slots.begin() + i);
                segment_ends.erase(segment_ends.begin() + i);
            }
            return;
        }
    }
}
template <typename T>
T* Cont<T>::empty_slot_selector()
{
    int pos = 0;
    int max = 0;
    for (int i = 0; i < actual_store_sizes.size(); i++) {
        if (empty_slots[i].empty()) {
            continue;
        }
        if (actual_store_sizes[i] > max) {
            pos = i;
            max = actual_store_sizes[i];
        }
    }
    T* empty = empty_slots[pos].back();
    empty_slots[pos].pop_back();
    n_empty--;
    actual_store_sizes[pos] += 1;
    return empty;
}
template <typename T>
void Cont<T>::reserve(int size)
{
    int current = std::accumulate(segment_ends.begin(), segment_ends.end(), 0);
    if (size <= current) {
        return;
    }
    store.push_back(new vector<T>{});
    store.back()->reserve(size - current);
    actual_store_sizes.push_back(0);
    empty_slots.push_back({});
    segment_ends.push_back(size - current);
}

#endif  // PO2_SPECIALSAUCE_CONT_H
