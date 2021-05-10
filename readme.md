# PO2 hashmap

# What is this?
This project delivers `LP3`, a replacement for `std::unordered_map` that has better performance
across the board except lookups where the key and value types are equal to or smaller than 32 bit integers.
It is a map based on linear probing.

# Design goals
This project started as a design project for college, where I chose to create an STL compatible hashmap that performs 
better than std::unordered_map at lookups. It also had to be as close to a drop in replacement for unordered_map as possible,
emulating its behaviour (outside of performance, of course) as best as it could.

# How to use
Just copy `LPmap3.h`, `fastmod3.h`, and `plf_colony.h` to a folder, 
and include `"./path/to/location/LPmap3.h"` where you normally would include `<unordered_map>`

Where you would use
```C++
#include <unordered_map>
// ........

std::unordered_map<int, int> my_map;

```
you now use
```C++

#include "path/to/LPmap3.h"
// ........

LP3<int, int> my_map;


```

# Differences from std::unordered_map
Like the design goals mention, it has to be as close as possible to std::unordered_map. Unfortunately, 
I was not able to archive full compatibility with it.
The following things differ from std::unordered_map:
1. There are no iterators to buckets `begin(size_t bucket_n)`, cbegin, end and cend
   member functions. The default map allows you to iterate over the buckets where each element
   has the same hash. Since this map doesn't use seperate chaining, buckets have 1 element. Instead of 
   implementing it with slightly differing behaviour, which might result in bugs, I chose not to implement it.
2. There are no merge and extract member functions. unordered_map, being node based, can easily transfer ownership
   without invalidating references.  This map, not being one, can't do it as easily without heavy redesign.

# benchmarks
To Be Added Later

# C++ versions and compilers supported
It compiles on clang 11 and 12, and GCC 10 and 11. 
It passes all tests when compiling for C++11, C++14, C++17 and C++20, although
most of the dev work has been done with C++17 as the target.

# Support
This map is mostly a proof of concept, and is provided as is. I don't foresee myself working on this in the future.
I probably will respond to issues and bug reports, but it will never be a priority.
The test suite is not close to exhaustive, so take that however you will.

# Licence
This project is licensed under GPLv3.
Also, I am not sure where to put this, but plf_colony, one of the dependencies
is licenced under zlib, which require me to state changes made to it.
I added my bucket interface as a friend class to plf::colony::iterator

## contributer specific stuff
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
## TODO
- [ ] Decrease bucket size by removing as much redundant info from colony's iterator
- [ ] Write more comperhensive test suite
- [ ] look for more performance improvements
- [ ] find better after hashing function