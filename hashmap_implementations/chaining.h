#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

using std::cout;
using std::vector;

class chaining {
  public:
    chaining();
    ~chaining(){};
    void reserve(int size);
    void insert(std::initializer_list<int> init_list);
    int& operator[](const int& k);
    // int& operator[](int&& k);
    void erase(int key);

  private:
    vector<vector<int>> keystore;
    vector<vector<int>> valuestore;
    size_t hasher(int key);
    int size;
    int buckets;
    void auto_rehash();
    int max_loadfactor = 10;
};

chaining::chaining()
{
    keystore.reserve(8);
    valuestore.reserve(8);
    buckets = 8;
    size = 0;
}

size_t chaining::hasher(int key) { return key % buckets; }
int& chaining::operator[](const int& key)
{
    size_t bucket = hasher(key);
    auto b_location = std::find(keystore[bucket].begin(), keystore[bucket].end(), key);
    int ofset = (b_location - keystore[bucket].begin());
    if (b_location == keystore[bucket].end()) {  // if key doesn't exist
        valuestore[bucket].push_back(0);
        keystore[bucket].push_back(key);
        size++;
    }
    int& x = valuestore[bucket][ofset];
    return x;
}

void chaining::insert(std::initializer_list<int> list)
{
    int key = *list.begin();
    int val = *(list.begin() + 1);
    int& x = operator[](key);
    x = val;
    size++;
    if (size / buckets > max_loadfactor) {
        auto_rehash();
    }
}
void chaining::erase(int key)
{
    size_t bucket = hasher(key);
    auto b_location = std::find(keystore[bucket].begin(), keystore[bucket].end(), key);
    int offset = (b_location - keystore[bucket].begin());
    if (b_location == keystore[bucket].end()) {
        return;
    }
    else {
        keystore[bucket].erase(b_location);
        valuestore[bucket].erase(valuestore[bucket].begin() + offset);
        return;
    }
}

void chaining::auto_rehash()
{
    int new_buckets = buckets * 2;
    vector<vector<int>> new_keystore;
    vector<vector<int>> neW_valuestore;
    new_keystore.reserve(new_buckets);
    neW_valuestore.reserve(new_buckets);
    for (int buck = 0; buck < keystore.size(); buck++) {
        for (int offset = 0; offset < keystore[buck].size(); offset++) {
            const int& key = keystore[buck][offset];
            const int& val = valuestore[buck][offset];
            const int newbucket = hasher(key);
            new_keystore[newbucket].push_back(key);
            neW_valuestore[newbucket].push_back(val);
        }
    }
    keystore = new_keystore;
    valuestore = neW_valuestore;
    buckets = new_buckets;
}

void chaining::reserve(int n)
{
    int number_buckets = (n / max_loadfactor) + 1;
    keystore = vector<vector<int>>();
    for (auto& x : keystore) {
        x = vector<int>(max_loadfactor);
    }
    valuestore = vector<vector<int>>();
    for (auto& x : valuestore) {
        x = vector<int>(max_loadfactor);
    }
}