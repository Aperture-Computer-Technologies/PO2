# PO2 thing

Current sprint:
- [ ] inplementeer een hashmap met de simple api die te zien is in ./simple_api
- [ ] implementeer tests die testen of de hashmap werken
- [ ] implementeer/vind benchmark suite voor hashmap performance test


implementeren doe je zo:
branch van dev, creer een header file met je implementatie in ./hashmap_implementations
en modify de ./test_hashmap_test.cpp zoals het gedaan is voor chaining.
om te builden, maak een target in cmakelists.txt
```cmake
add_executable(yourimplementation-test
${PROJECT_SOURCE_DIR}/hashmap_implementations/yourheader.h
${PROJECT_SOURCE_DIR}/test/hashmap_test.cpp
)
```

# Sagar's hashmaps
Note: <H,K,V> = a simple struct storing the hash, key and value.

1. LPmap. This is standard linear probing. Storing the <H,K,V> in an array. This won't meet at least 1 of the requirements that std::unordered meets. pointer to value invalidates on rehash.
2. LPmap2. Same as LPmap, but now i'm storing <H,K,V*>, with value living on the heap.
3. Nodemap. Uses "specialsauce" container. I'll explain the container later. This map has an array of pointers to <H,K,V>, and <H,K,V> is stored into specialsauce container.
4. Nodemap1b. This is very similar to nodemap. I'm just using it to implement optimisations. I can then benchmark both original nodemap and 1b, so i see if my "optimisation" is indeed faster.
5. Nodemap2. This is actually closer to LPmap than to nodemap. it has an array of pointers to <H,K,V> , and <H,K,V> live on the heap. This was a test to see if I got tangible benefits of using "specialsauce" container over just letting <H,K,V> live on the heap. There are benefits, and cons.

# the requirement LPmap doesn't meet
https://eel.is/c++draft/unord.req
point 8. 
The elements of an unordered associative container are organized into buckets.
Keys with the same hash code appear in the same bucket.
The number of buckets is automatically increased as elements are added to an unordered associative container, so that the average number of elements per bucket is kept below a bound.
**Rehashing** invalidates iterators, changes ordering between elements, and changes which buckets elements appear in, but **does not invalidate pointers or references to elements.**

LPmap doesn't meet that.


# "Specialsauce" container
it's special in the same way this dog is
![](https://preview.redd.it/hcprm17ktpu21.jpg?width=660&auto=webp&s=ea581419f0a16c7f5e6202d08ed40ec70885537c)

I need to ask mr. Winston if there is a data structure that does what i want, since he studied compsci while my compsci knowledge is spotty.

Reason for creating it: I need a (semi-)continous data structure that can resize, but doesn't move around when it resizes. vector can resize, but it allocates a new memory region that can hold it's capacity, moves/copies over it's contents and then destroys the old vector at the old location, thereby invalidating all pointers to it.

I want it, because it **should** have better space locality, thereby reducing cache misses. (it does, when you bench Nodemap and Nodemap2)

To archieve this, i have created the container. Under the hood, it's vector<vector*>. when it increases capacity, it does so by creating a vector on the heap, and then reserving a set size for it.
When something is inserted in the data structure, it does the following:
1. if there are resulting open slots from previous deletions, it inserts in those locations.
2. if there aren't, it checks to see if inserting in the last array will exceed it's reserved capacity, and inserts there if it doesnt.
3. if it will exceed reserved capacity, it creates a new vector on the heap and stores there.

When previous open slots are detected, it tries to pick a slot from an vector with the least ammount of open slots. So theoretically, if you delete slots at random, and try to fill out the vectors with the least open slots, eventually some vectors will become empty, and can then safely be deleted.

Things i should try out to improve it:
1. currently, i'm reserving vector sizes like 128,18,256,512,1024,etc. I should try just doing 512,512,512,512,etc. I think i can remove some of the logic needed for selecting empty slots if i use that.
2. currently, the strategy is to pick empty slots from vectors that have the most actual filled slots. So it picks vectors which are the furthest from deletion. (assuming random deletes). I should try out smallest actually empty/capacity, least deletes, or largest sized vector. 







