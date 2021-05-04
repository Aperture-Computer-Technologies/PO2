# PO2 thing


## Profiling
with perf:
```
sudo perf record -e cache-misses,branch-misses ./LP-profiling
sudo report
```

with gproftools:
```
don't forget to link profiling to the target
env CPUPROFILE=LP-profiling.prof ./LP-profiling 
pprof ./LP-profiling ./LP-profiling.prof 
```

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

1. LPmap. This is standard linear probing. Storing the <H,K,V*> in an array.
2. Nodemap. a vector<<K,V*,H>>, en std::deque<V>


### update:
1. found out what the weird spikes in all my inserts were. I should be reserving 
size * loadfactor for the vectors that store info when hashmap.reserve is called.
   
# Things I want to try out

See if there's any performance to be gained by not just inserting at the back of deque.
1. alternate front and back insertion
2. just front insertion
3. alternate n front, n back



