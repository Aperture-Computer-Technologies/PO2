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
    vector<vector<int>> keystorage;
    vector<vector<int>> valuestorage;
    size_t hasher(int key);
    int size;
};

chaining::chaining()
{
    keystorage.reserve(8);
    valuestorage.reserve(8);
    size = 8;
}

size_t chaining::hasher(int key) { return key % size; }
int& chaining::operator[](const int& key)
{
    size_t bucket = hasher(key);
    auto b_location = std::find(keystorage[bucket].begin(), keystorage[bucket].end(), key);
    int ofset = (b_location - keystorage[bucket].begin());
    if (b_location == keystorage[bucket].end()) {  // if key doesn't exist
        valuestorage[bucket].push_back(0);
        keystorage[bucket].push_back(key);
    }
    int& x = valuestorage[bucket][ofset];
    return x;
}

void chaining::insert(std::initializer_list<int> list)
{
    int key = *list.begin();
    int val = *(list.begin() + 1);
    int& x = operator[](key);
    x = val;
}
void chaining::erase(int key)
{
    size_t bucket = hasher(key);
    auto b_location = std::find(keystorage[bucket].begin(), keystorage[bucket].end(), key);
    int offset = (b_location - keystorage[bucket].begin());
    if (b_location == keystorage[bucket].end()) {
        return;
    }
    else {
        keystorage[bucket].erase(b_location);
        valuestorage[bucket].erase(valuestorage[bucket].begin() + offset);
        return;
    }
}
