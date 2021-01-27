# PO2 thing

Current sprint:
- [ ] inplementeer een hashmap met de simple api die te zien is in ./simple_api
- [ ] implementeer tests die testen of de hashmap werken
- [ ] implementeer/vind benchmark suite voor hashmap performance test


implementeren doe je zo:
branch van dev, creer een folder "yourhashmapname_hashmap"
om te builden, maak een target in cmakelists.txt

```cmake
add_executable(exec_name
${PROJECT_SOURCE_DIR}/yourhashmapname_hashmap/yourfilenames.h
${PROJECT_SOURCE_DIR}/yourhashmapname_hashmap/filename.cpp
)
```

tests zijn er nog niet, maar wanneer ze er zijn, maak een target die je hashmap header en test file includen.