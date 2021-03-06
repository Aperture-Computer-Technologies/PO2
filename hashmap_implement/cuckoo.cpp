#include "./cuckoo_hashing.h" 
/* driver function */
int main() 
{ 
    /* following array doesn't have any cycles and 
       hence  all keys will be inserted without any 
       rehashing */
    int keys_1[] = {20, 50, 53, 75, 100, 67, 105, 
                    3, 36, 39}; 
  
    int n = sizeof(keys_1)/sizeof(int); 
  
    cuckoo(keys_1, n); 
  
    /* following array has a cycle and hence we will 
       have to rehash to position every key */
    int keys_2[] = {20, 50, 53, 75, 100, 67, 105, 
                    3, 36, 39, 6}; 
  
    int m = sizeof(keys_2)/sizeof(int); 
  
    cuckoo(keys_2, m); 
  
    return 0; 
} 