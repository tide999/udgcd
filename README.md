# udgld (UnDirected Graph Loop Detection)

C++ wrapper over Boost Graph Library (aka as BGL), provides a mean to detect loops in undirected graphs.

For example, with the following graph generated by the included sample program:

![alt](
https://github.com/skramm/udgld/blob/master/sample1_2.png "sample graph")

This code will give you the three loops as three sets of vertices, sorted with smallest vertex in first position:
```
1-6-5-4-3-2-
1-6-5-4-13-14-7-3-2-
3-7-14-13-4-
```
### Summary
- [Status](#s_stat)
- [Usage](#s_usage)
- [Building & installing](#s_build)
- [Issues](#s_issues)
- [Insides](#s_inside)
- [References](#s_ref)


### Status
 <a name="s_stat"></a>

- Not extensively tested, but provides sample application code (that works).
- Works for graphs holding unconnected sub-graphs
- Modern C++ design'ed (RAII).
- Fairly generic, should be suited for pretty much all types of undirected graphs.
- Released under the Boost licence.

### Usage:
 <a name="s_usage"></a>

 - Include the file `udgld.hpp` in your application.
 - Build your graph (add vertices and edges, see BGL manual)
 - Call `udgld::FindLoops<your_graph_type,your_vertex_type>( graph )`.
 It will return a set of paths that are loops.
 - Just to check, you may call `udgld::PrintPaths()` to print out the loops.
 - Done !

See included samples.

### Building & installing:
 <a name="s_build"></a>

- header only, no build.
- To build & run the provided sample code, just use `make run`, no other dependency than BGL.
(tested with 1.54)

##### Installing
Just copy the file `udgld.hpp` where you want, or use the provided `install` target of makefile:

 ```
 sudo make install
 ```

##### Build options:
 - if UDGLD_PRINT_STEPS is defined, then different steps will be printed on `std::cout` (useful only for debugging purposes). See makefile.

### Issues:
 <a name="s_issues"></a>

 - At present, this code requires a static allocated variable (done automatically by compiler, as it is templated)
 Thus it is not thread safe, neither can it handle multiple graphs simultaneously.


### How does it work ?
 <a name="s_inside"></a>

Three steps are involved: first we need to check if there **is** at least one loop. Is this is true, we explore the graph to find it/them.

- The first step is done by calling [boost::undirected_dfs()](http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/undirected_dfs.html)
with passing a visitor of class `LoopDetector`, inherited from
[boost::dfs_visitor](http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/dfs_visitor.html).
This object holds a set of vertices that are part of an edge on which `back_edge()` is called.
This means that a loop has been encountered.

- The second step is done once the DFS has been done. We explore recursively the graph, by starting from each of the vertices that have been identified as part of a "back edge".

- The third steps makes sure no loops are duplicate in the output set.

### References
 <a name="s_ref"></a>

 - BGL: http://www.boost.org/doc/libs/1_59_0/libs/graph/doc
 - [wikipedia.org Graph page](https://en.wikipedia.org/wiki/Graph_%28mathematics%29#Undirected_graph)
