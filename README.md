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

SCALE = 10000000
FLOOR = 135

B-star insert
B-star erase
B-star find
Insert time: 0.238839 s
Erase time:  0.392262 s
Find time:  0.00920421 s
std::map insert
std::map erase
std::map find
Insert time: 2.46519 s
Erase time:  0.649413 s
Find time:  0.00101235 s
```
