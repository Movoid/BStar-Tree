# A B\*-Tree implementation.

This is a simple, educational B\*-Tree implementation based on B+-Tree, not B-Tree.  
C-style, not STL.

- [x] `insert` with preemptive split
- [x] `erase` with preemptive merge
- [x] `find`
- [x] Range query
- [ ] Cpp-style

> NO DUPLICATED KEY ORIGINALLY SUPPORTED,  
> but val_type can be a `std::vector` or some container else.  

> Lack of error handling.  

```benchmark

$ g++ -O2 ./bstar_tester.cpp  -o ./_output.run && ./_output.run

std::map insert
std::map find
test output 49999995000000
std::map erase
Insert time: 8.04212 s
Erase time:  7.31826 s
Find time:  8.52046 s
B-star insert
B-star find
test output 49999995000000
B-star erase
Insert time: 3.01029 s
Erase time:  2.92722 s
Find time:  2.77145 s

```