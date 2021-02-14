#include<bits/stdc++.h>



class cuckoo{
cuckoo();
void reserve();
void insert();
void erase();

private:
int Size = 11;

};
// upper bound on number of elements in our set 
#define Size 11 
  
  
// choices for position 
#define ver 2 
  
// Auxiliary space bounded by a small multiple 
// of Size, minimizing wastage 
int reserve[ver][Size]; 
  
// Array to store possible positions for a key 
int pos[ver]; 
  
/* function to fill hash1 table with dummy value 
 * dummy value: INT_MIN 
 * number of hash1tables: ver */
void init_table() 
{ 
    for (int j=0; j<Size; j++) 
        for (int i=0; i<ver; i++) 
            reserve[i][j] = INT_MIN; 
} 
  
/* return hash1ed value for a key 
 * function: ID of hash1 function according to which 
    key has to hash1ed 
 * key: item to be hash1ed */
int hash1(int function, int key) 
{ 
    switch (function) 
    { 
        case 1: return key%Size; 
        case 2: return (key/Size)%Size; 
    } 
} 
  
/* function to place a key in one of its possible positions 
 * tableID: table in which key has to be placed, also equal 
   to function according to which key must be hash1ed 
 * cnt: number of times function has already been called 
   in order to place the first input key 
 * n: maximum number of times function can be recursively 
   called before stopping and declaring presence of cycle */
void insert(int key, int tableID, int cnt, int n) 
{ 
    /* if function has been recursively called max number 
       of times, stop and declare cycle. R hash1. */
    if (cnt==n) 
    { 
        printf("%d unpositioned\n", key); 
        printf("Cycle present. R hash1.\n"); 
        return; 
    } 
  
    /* calculate and store possible positions for the key. 
     * check if key already present at any of the positions. 
      If YES, return. */
    for (int i=0; i<ver; i++) 
    { 
        pos[i] = hash1(i+1, key); 
        if (reserve[i][pos[i]] == key) 
           return; 
    } 
  
    /* check if another key is already present at the 
       position for the new key in the table 
     * If YES: place the new key in its position 
     * and place the older key in an alternate position 
       for it in the next table */
    if (reserve[tableID][pos[tableID]]!=INT_MIN) 
    { 
        int dis = reserve[tableID][pos[tableID]]; 
        reserve[tableID][pos[tableID]] = key; 
        insert(dis, (tableID+1)%ver, cnt+1, n); 
    } 
    else //else: place the new key in its position 
       reserve[tableID][pos[tableID]] = key; 
} 
  // Erase function
  void erase(int key);




/* function to print hash1 table contents */
void printTable() 
{ 
    printf("Final hash1 tables:\n"); 
  
    for (int i=0; i<ver; i++, printf("\n")) 
        for (int j=0; j<Size; j++) 
            (reserve[i][j]==INT_MIN)? printf("- "): 
                     printf("%d ", reserve[i][j]); 
  
    printf("\n"); 
} 
  
/* function for Cuckoo hash1ing keys 
 * keys[]: input array of keys 
 * n: size of input array */
void cuckoo(int keys[], int n) 
{ 
    // initialize hash1 tables to a dummy value (INT-MIN) 
    // indicating empty position 
    init_table(); 
  
    // start with placing every key at its position in 
    // the first hash1 table according to first hash1 
    // function 
    for (int i=0, cnt=0; i<n; i++, cnt=0) 
        insert(keys[i], 0, cnt, n); 
  
    //print the final hash1 tables 
    printTable(); 
} 
 