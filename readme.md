# PO2 thing

Current sprint:
- [ ] inplementeer een hashmap met de simple api die te zien is in ./simple_api
- [ ] implementeer/vind benchmark suite voor hashmap performance test
- [ ] implementatie moet met templates zijn

implementeren doe je zo:
branch van dev, creer een header file met je implementatie in ./hashmap_implementations en modify de
./test/hashmap_test.cpp zoals het gedaan is voor LPmap. om te builden, maak een target in cmakelists.txt
```cmake
add_executable(yourimplementation-test
        ${PROJECT_SOURCE_DIR}/hashmap_implementations/yourheader.h
        ${PROJECT_SOURCE_DIR}/test/hashmap_test.cpp
        )
```

# Sagar's hashmaps

Note: <H,K,V> = a simple struct storing the hash, key and value.

1. LPmap. This is standard linear probing. Storing the <H,K,V> in an array. This won't meet at least 1 of the
   requirements that std::unordered meets. pointer to value invalidates on rehash.
2. LPmap2. Same as LPmap, but now i'm storing <H,K,V*>, with value living on the heap.
3. LPmap3. This is the same as LPmap 2, but V is stored in a deque<V> instead of the heap.
4. Nodemap. Uses "specialsauce" container. I'll explain the container later. This map has an array of pointers to <
   H,K,V>, and <H,K,V> is stored into specialsauce container.
5. Nodemap2. This is actually closer to LPmap than to nodemap. it has an array of pointers to <H,K,V> , and <H,K,V> live
   on the heap. This was a test to see if I got tangible benefits of using "specialsauce" container over just letting <
   H,K,V> live on the heap. Nodemap is indeed faster than NM2.
6. Nodemap3. Same as Nodemap, but <K,V,H> live in a deque instead of specialsauce container.

Currently, if you look at ./benchmarks/graphs notebook, you'll see Nodemap1b. That's just Nodemap3. 1b became 3 when it
turned out it was just better than Nodemap, without any of the maintainence costs.

Of these 6 hashmaps, Nodemap3 and LPmap3 are worth pursuiing. LPmap doesn't meet a requirement, so that one is out.
LPmap slower than LPmap3. Nodemap has too high a maintainence cost and it's slower than 3, and Nodemap2 was just a test
and it's slower.

To give a quick summary of rel. perf. between my 2 best options:
LP3 is faster than NM3 for the following:

1. int inserts.
2. int succesful lookups
3. int Deletes.
4. String succesful lookups
5. string unsuccesful lookups.
6. String deletes

NP3 is faster for the following:

1. int succesful lookups
2. string inserts
3. string deletes.

No, I can't explain it yet. I expected that LP3 would always be faster for inserts and deletes, and on par with NM3 for
lookups. I have no idea why succ lookups do worse for ints.

# things I want to try out

See if there's any performance to be gained by not just inserting at the back of deque.

1. alternate front and back insertion
2. just front insertion
3. alternate n front, n back

# the requirement LPmap doesn't meet

https://eel.is/c++draft/unord.req
point 8. The elements of an unordered associative container are organized into buckets. Keys with the same hash code
appear in the same bucket. The number of buckets is automatically increased as elements are added to an unordered
associative container, so that the average number of elements per bucket is kept below a bound.
**Rehashing** invalidates iterators, changes ordering between elements, and changes which buckets elements appear in,
but **does not invalidate pointers or references to elements.**

LPmap doesn't meet that.




# "Specialsauce" container

## Update: std::deque is almost exactly what i want from specialsauce, so here's goodbye to specialsauce.
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

1. currently, the strategy is to pick empty slots from vectors that have the most actual filled slots. So it picks
   vectors which are the furthest from deletion. (assuming random deletes). I should try out smallest actually
   empty/capacity, least deletes, or largest sized vector. 







