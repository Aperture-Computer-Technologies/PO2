#ifndef NODEMAP_H
#define NODEMAP_H

#include <algorithm>
#include <numeric>
#include <vector>
using std::vector;

constexpr int INT_MAX = 2147483647;
namespace helper2 {
    std::vector<int> prime_sizes
        = {127,    251,    479,     911,     1733,    3299,    6269,     11923,    22669,    43093,    81883,   155579,
           295601, 561667, 1067179, 2027659, 3852553, 7319857, 13907737, 26424707, 50206957, 95393219, 18124717};

    size_t next_prime(const int& n)
    {
        for (const int x : prime_sizes) {
            if (x > n) {
                size_t t = x;
                return t;
            }
        }
    }
}  // namespace helper

/*
 * specialized, don't use for anything outside of this hashmap
 */
template <typename T>
class Cont {
  public:
    typedef T ValueType;
    Cont();
    Cont(int siz);
    T* insert(T elem);
    void remove(T* elem);

  private:
    T* empty_slot_selector();
    int n_empty;
    vector<vector<T>*> store = vector<vector<T>*>(1);
    vector<vector<T*>> empty_slots;
    vector<T> segment_ends;
    vector<int> actual_store_sizes;

};


template <typename T>
Cont<T>::Cont(int size): store{new vector<T>{}}, empty_slots{{}}, segment_ends{size}, n_empty{0}, actual_store_sizes{{0}}
{
    store.back()->reserve(size);

}
template <typename T>
Cont<T>::Cont(): Cont{Cont<T>(251)}
{
}

template <typename T>
T* Cont<T>::insert(T elem)
{
    T* adress;
    if (n_empty){
        adress = empty_slot_selector();
        *adress = elem;
        return adress;
    }
    // check if vector is in danger of reallocation
    bool is_full = (store.back()->size() == segment_ends.back());
    if (is_full){
        int size = std::accumulate(segment_ends.begin(), segment_ends.end(), 0);
        store.push_back(new vector<T>{});
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
    for (int i = 0; i < store.size(); i++){
        T* begin = store[i]->data();
        if (elem >= begin && elem <= begin + store[i]->size()){
            empty_slots[i].push_back(elem);
            n_empty++;
            actual_store_sizes[i]--;
            if (!actual_store_sizes[i]){
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
    int index = 0;
    int max = 0;
    for (int i = 0; i < actual_store_sizes.size(); i++){
        if (empty_slots[i].empty()){
            continue;
        }
        if (actual_store_sizes[i] > max){
            index = i;
            max = actual_store_sizes[i];
        }
    }
    T* empty = empty_slots[index].back();
    empty_slots[index].pop_back();
    n_empty--;
    actual_store_sizes[index]+= 1;
    return empty;
}

#endif
