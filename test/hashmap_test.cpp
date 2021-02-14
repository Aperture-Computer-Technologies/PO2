#include "./hashmap_test.h"
#include <unordered_map>
#include "./../hashmap_implementations/chaining.h"
#include "./../hashmap_implement/cuckoo_hashing_weis_Api.h"
#include <functional>
int main(){
    hashmap_test_suite(std::unordered_map<int, int> {});
    cout << "--------Sagar's Chaining map\n";
    hashmap_test_suite(CuckooHashTable<int,hash<int>>{});

}