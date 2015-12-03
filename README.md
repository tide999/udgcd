# udgcd (UnDirected Graph Cycle Detection)

C++ wrapper over Boost Graph Library (aka as BGL), provides a mean to detect cycles in a planar undirected graph.

For example, with the following graph generated by the included sample program:

![alt](
https://github.com/skramm/udgcd/blob/master/sample1_2.png "sample graph")

This code will give you the three cycles as three sets of vertices. These are sorted with the smallest vertex in first position, and such as the second vertices is always smaller than the last one (see [notes](#s_notes)):
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
- [Design](#s_inside)
- [References](#s_ref)
- [Notes](#s_notes)


### Status
 <a name="s_stat"></a>

- Alpha : CURRENTLY IN DEVELOPMENT. Not extensively tested, but provides sample application code.
- Works for graphs holding unconnected sub-graphs.
- Modern C++ design'ed (RAII).
- Fairly generic, should be suited for pretty much all types of undirected graphs, as long as you can [order the vertices](#s_notes).
- Released under the Boost licence.

### Usage:
 <a name="s_usage"></a>

 - Include the file `udgcd.hpp` in your application.
 - Build your graph (add vertices and edges, see BGL manual)
 - Call `udgcd::FindCycles<your_graph_type,your_vertex_type>( graph )`.
 It will return a set of paths that are cycles.
 - Just to check, you may call `udgcd::PrintPaths()` to print out the cycles.
 - Done !

See included samples.

### Building & installing:
 <a name="s_build"></a>

- header only, no build.
- To build & run the provided sample code, just use `make run`, no other dependency than BGL.
(tested with 1.54)

##### Installing
Just copy the file `udgcd.hpp` where you want, or use the provided target of makefile:
`sudo make install`

##### Build options:
 - if UDGCD_PRINT_STEPS is defined, then different steps will be printed on `std::cout` (useful only for debugging purposes).
 For the provided samples, this can be done by passing option `PRINT_STEPS=Y` to make.

If Graphviz/dot is installed, the demo samples will render the generated graphs into png images in the `obj` folder.

### Issues:
 <a name="s_issues"></a>

 - At present, this code requires a static allocated variable (done automatically by compiler, as it is templated)
 Thus it is not thread safe, neither can it handle multiple graphs simultaneously.


### How does it work ?
 <a name="s_inside"></a>

Three steps are involved: first we need to check if there **is** at least one cycle. Is this is true, we explore the graph to find it/them.

- The first step is done by calling [boost::undirected_dfs()](http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/undirected_dfs.html)
with passing a visitor of class `LoopDetector`, inherited from
[boost::dfs_visitor](http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/dfs_visitor.html).
This object holds a set of vertices that are part of an edge on which `back_edge()` is called.
This means that a cycle has been encountered.

- The second step is done once the DFS has been done. We explore recursively the graph, by starting from each of the vertices that have been identified as part of a "back edge".

- The third steps makes sure no cycle are duplicate in the output set.

### References
 <a name="s_ref"></a>

- BGL: http://www.boost.org/doc/libs/1_59_0/libs/graph/doc
- [wikipedia.org Graph page](https://en.wikipedia.org/wiki/Graph_%28mathematics%29#Undirected_graph)
- https://en.wikipedia.org/wiki/Cycle_basis
- https://en.wikipedia.org/wiki/Cycle_%28graph_theory%29

#### Unsorted links on topic
- http://www.geometrictools.com/Documentation/MinimalCycleBasis.pdf
- http://math.stackexchange.com/questions/8140/
- http://cstheory.stackexchange.com/questions/21154/
- http://stackoverflow.com/questions/16782898/
- http://stackoverflow.com/questions/1607124/
- http://stackoverflow.com/questions/4023310/
- http://stackoverflow.com/questions/16782898/
- http://www.lamsade.dauphine.fr/~poc/IMG/pdf/Amaldi.pdf
- https://people.mpi-inf.mpg.de/~mehlhorn/ftp/CycleBasisImpl.pdf
- https://www.euro-online.org/gom2008/abstracts/gom08-amaldi.pdf

 ### Notes
  <a name="s_notes"></a>

- In the output vector, the paths are sorted, so you need to have the `<` operator defined for the vertices. Sorting is done such as:
 - The smallest element is in first position;
 - The element in second position is smaller than the last one.

 As an example, say you have a raw loop expressed as
   `6-2-1-4`, it is released as `1-2-6-4` (and not `1-4-6-2`).
