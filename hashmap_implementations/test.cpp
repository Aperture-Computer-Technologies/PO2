#include <iostream>
#include <vector>

#include "./nodemap.h"

using namespace std;
int main()
{
    int totest = 100;
    Cont<int> container(10);
    vector<int> der_pointers;
    vector<int*> contpointers;
    cout << "inserting\n";
    for (int i = 0; i < totest; i++) {
        contpointers.push_back(container.insert(i));
        der_pointers.push_back(i);
    }
    cout << "deleting\n";

    for (int i = 0; i < totest; i++) {
        if (i % 2 || i < 11) {
            container.remove(contpointers[i]);
            contpointers[i] = nullptr;
            cout << i << ", ";
        }

    }
    cout << "\nreinserting\n";
    for (int i = 0; i < totest; i++) {
        if (i % 2) {
            continue;
        }
        contpointers[i] = container.insert( i);
        der_pointers[i] =  i;
        cout << i << ": " << *contpointers[i] << ", " << der_pointers[i] << "\n";
    }
    cout << "checkjing\n";

    for (int i = 0; i < totest; i++) {
        if (!contpointers[i]){
            continue;
        }
        if (*contpointers[i] != der_pointers[i]) {
            cout << der_pointers[i] << ", " << *contpointers[i] << "\n";
            auto a = *contpointers[i];
        }
    }
}