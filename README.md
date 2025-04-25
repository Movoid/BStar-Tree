# A B\*-Tree implementation.

This is a simple, educational B\*-Tree implementation based on B+-Tree, not B-Tree.  
C-style, not STL.

- [x] `insert` with preemptive split
- [x] `erase` with preemptive merge
- [x] Duplicate keys support
- [x] `find`
- [ ] Range query
- [ ] Cpp-style

Not cache optimized.

```text
Benchmark:

SCALE = 100000000
FLOOR = 145

B-star insert
B-star erase
B-star find
Insert time: 12.0652 s
Erase time:  44.024 s
Find time:  9.14769 s
std::map insert
std::map erase
std::map find
Insert time: 128.22 s
Erase time:  114.343 s
Find time:  6.20423 s
```
