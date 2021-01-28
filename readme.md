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
