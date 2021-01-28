
#include <initializer_list>

class PO2_map {
    virtual int& operator[](const int& k) = 0;  // lookup and if you can, insert
    virtual int& operator[](int&& k) = 0;       // lookup and if you can, insert
    virtual int erase(const int& k) = 0;  // erase, duh
    virtual void reserve(int n) = 0;      // set size of the array
    void insert(std::initializer_list<int> il); // inserts
};


class Cuckoo : public PO2_map {
    // constructor
    // destructor

};


