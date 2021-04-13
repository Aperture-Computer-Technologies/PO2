#include "./hashmap_test.h"

#include <deque>
#include <unordered_map>

#include "./../hashmap_implementations/Cuckoo.h"
#include "./../hashmap_implementations/LPmap.h"
#include "./../hashmap_implementations/LPmap2.h"
#include "./../hashmap_implementations/Nodemap.h"

/*
 * hashmap test suite targets.
 * Just add your header, and add the lines below for your test
 */

int main()
{
    //        hashmap_test_suite(std::unordered_map<int, int>{});
    //
    //        cout << "--------Sagar's node3 map\n";
    //        hashmap_test_suite(Nodemap<int, int>{});
    cout << "--------Sagar's LP map\n";
    hashmap_test_suite(LP<int, int>{});
    //    cout << "--------Sagar's LP2 map\n";
    //    hashmap_test_suite(LP2<int, int>{});

    //    cout << "--------Sagar's Cuckoo map\n";
    //    hashmap_test_suite(Cuckoo<int, int>{});
}