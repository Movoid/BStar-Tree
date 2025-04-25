# A B\*-Tree implementation.

This is a simple, educational B\*-Tree implementation based on B+-Tree, not B-Tree.  
C-style, not STL.

- [x] `insert` with preemptive split
- [x] `erase` with preemptive merge
- [x] Duplicate keys support
- [x] `find`
- [x] Range query
- [ ] Cpp-style

Not cache optimized.

```text
Benchmark:

# g++ -O2 ./bstar_tester.cpp -o ./_output.run      

SCALE = 20000000
FLOOR = 145

B-star insert
B-star erase
B-star find
Insert time: 0.509264 s
Erase time:  0.850953 s
Find time:  0.0176606 s
std::map insert
std::map erase
std::map find
Insert time: 4.88365 s
Erase time:  1.42912 s
Find time:  0.00418141 s
```
