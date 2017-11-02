This is a C++ implementation of the Tiered Vector data structure. It is the basis of the experimental results presented in the paper "Fast Dynamic Arrays" published at the ESA 17 conference which can be found [here](https://arxiv.org/abs/1711.00275).

It is provided as an open-source MIT-licensed C++ library, and is not intended for production use.


The data structure intended use is for large dynamic sequences.
Tiered vector provides `O(n^e)` time complexity for update (insert/remove) operations
and constant-time acces operations.
`STL vector` provides `O(n)` time update and constant-time access
while dynamic self-balancing trees have `O(log n)` time complexity for all
operations.


When working with sequences of ~10^8 elements in practice, this implementation   
provides a speedup of ~ 10.000x for update operations compared to
`STL vector` at the cost of a ~ 1/2x slow down for access.

Compared to the red-black tree based `STL multiset`, the speedup is ~40x for access
at the cost of a ~1/10x slowdown on insert.
Dynamic trees usually have a large memory overhead.
Compared to `STL vector`, `STL multiset` uses ~ 10x more memory for ~10^8 elements. The tiered vector only has an overhead of ~1% compared to `STL vector` (see [publication](https://arxiv.org/abs/1711.00275) for more details and comparisons).

# Build

### Makefile

* `make example`: build the example binary bin/example which compares the time taken to insert 100.000 elements in an tiered vector and a standard vector.
See the file  example.cpp for more info

### Compiler flags

The implementation supports a series of optimizations that can be enabled/disabled
through compiler flags:

|#|Flag| Explanation | Effect |
|---|---|---|---|
| 1 |  | No optimizations, normal pointer based tree structure |
| 2 | PPACK | Like 1 but with child pointers and child offsets co-located in memory | fewer memory probes / operation 
| 3 | PFREE | Use a heap-like tree representation (array instead of pointers) | fewer memory propes / operation, however memory overhead  is linear in # of elements* |
| 4 | PFREE, COMPACT | Like 3 but with word-level packing of vertex offsets  | less space for tree structure => better cache utilization |
| 5 | ARRAY LEVEL | Like 3 but with lazy allocation of leaves | memory overhead sublinear in # of elements* |
| 6 | ARRAY LEVEL PACK | Like 5 but pack the element pointer and the offset of a leaf in a single word | one less memory probe / operation |

*We note that the complexity analysis is only true given the assumption that
the structure is always at most a constant fraction from being full.
In this implementation, the container's maximum size must be specified
at compile time, and it does not support growing dynammically.

Severel experiments have been carried out to compare the optimizations.
They all provide some sort of trade-off between space usage,
memory probes / operation and instructions / opertaion.

When working with sequences of ~10^8 elements, 
the structure given by row 3 outperforms the others.
However, this structure uses extra space linear in the number of elements.
The structure given by row 6 is insignificantly slower
but only uses extra space sublinear in the number of elements.
Thus, this is the structure we recommend.
See the [publication](https://arxiv.org/abs/1711.00275) for more details and comparisons.

# Use

The tiered vector stores a sequence of elements that are ordered in
a strict linear sequence and accessed by their position in this sequence.

The height of the tree underlying the data structure and the width of the leaves
has an effect on the performance of the data structure.

In our experiments we have obtained the best results with a height of 4,
a leaf width of 512 and the remaining widths as powers of 2.
See the [publication](https://arxiv.org/abs/1711.00275) for more details and comparisons.

## Recomended configuration

Build with compiler flags: `-D ARRAY -D LEVEL -D PACK` (see the compiler flags section)
and the configuration

`Tiered<int, LayerItr<LayerEnd, Layer<x ,Layer<y, Layer<z, Layer<512>>>>>> tiered;`

where `x, y` and `z` are powers of two chosen such that the maximum capacity suits the requirements.

## Interface

The interface tiered vector resembles that of `STL vector`.

| | |
| --- | --- |
| size | Return the number of elements in the tiered vector |
| insert(i, x) | Insert element x after the element at position i |
| remove(i) | Remove the element at position i |
| operator[i] | Return a reference to the element at position i |

# Example 

A 3-tiered vector with a maximum capacity of 64^3 = 262144:
```c++
#include <iostream>
#include "sequence.h"

using namespace Seq;

int main() {

    Tiered<int, LayerItr<LayerEnd, Layer<64, Layer<64, Layer<64>>>>> tiered;

    tiered.insert(0, 0);
    tiered.insert(0, 1);
    tiered.insert(0, 2);
    tiered.insert(0, 3);
    tiered.insert(0, 4);

    for(int i = 0; i < tiered.size; i++) {
        std::cout << tiered[i] <<  " ";
    }

    return 0;

}
```

Output: `4 3 2 1 0`




# License 

MIT License

Copyright (c) 2017 Mikko Berggren Ettienne

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
