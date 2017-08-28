# udgcd (UnDirected Graph Cycle Detection)

C++ wrapper over Boost Graph Library (aka BGL), provides a mean to detect cycles in a planar undirected graph.

For example, with the following graph generated by the included sample program:

![alt](
https://github.com/skramm/udgcd/blob/master/sample1_2.png "sample graph")

This code will give you the three cycles as three sets of vertices.
These are sorted with the smallest vertex in first position, and such as the second vertices is always smaller than the last one (see [notes](#s_notes)):
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
- UPDATE 2017-08-28: at present, output is redundant: in some case, can give more cycles than it actually should. See [here](#s_issues).
After some investigating and coding, this issue is actually pretty bad, thus I would recommend *not to use* this code until I find some fix.
- UPDATE 2017/04: now works with user bundled properties for the edges, [see here](#note_bp).
- beta. Not extensively tested, but provides sample application code.
- WARNING: not selled as being "optimal", a lot of things can probably being optimized.
- Works for graphs holding unconnected sub-graphs.
- Modern C++ design'ed (RAII).
- Fairly generic, should be suited for pretty much all types of undirected graphs, as long as you can [order the vertices](#s_notes).
- Build upon [example](http://www.boost.org/doc/libs/1_58_0/libs/graph/example/undirected_dfs.cpp) taken from BGL manual
(also check [this page](http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/undirected_dfs.html)).
Got some help from [sehe](http://stackoverflow.com/a/43481372/193789).
- Released under the Boost licence.
- Intended audience: Any C++ app having a graph cycle detection issue and whose licence is compatible with the Boost licence.

### Usage:
 <a name="s_usage"></a>

 1. Add `#include "udgcd.hpp"` in your application (all-in-one file).
 1. Create your graph ([check this](#s_notes)).
 1. Build your graph (add vertices and edges, see BGL manual or the provided samples).
 1. Call `udgcd::FindCycles()`. It will return a set of paths that are cycles (1):

 `auto cycles = udgcd::FindCycles<my_graph_type,my_vertex_type>( graph );`
 - Just to check, you may call `udgcd::PrintPaths()` to print out the cycles:

   `udgcd::PrintPaths( std::cout, cycles, "cycles" );`
 - Done !

(1) FYI, the type of the returned value is actually
`std::vector<std::vector<my_vertex_t>>` but with C+11, you don't really need that information...

See included samples.

### Building & installing:
 <a name="s_build"></a>

- header only, no build. Provided as a single file (the other files are useless for basic user).
- To build & run the provided sample code, just use `make run`, no other dependency than BGL.
(tested with boost 1.54, let me know if you discover any inconsistency with later releases.)

##### Installing
Just fetch the file `udgcd.hpp` above and store it where you want. Or use the provided target of makefile (if you clone the whole repo):
`make install` (might require `sudo`).
This will copy the file in `/usr/local/include/`

##### Build options:
 - The provided makefile is not requested to use the library, as it is "header-only", but might help. It has the following targets:
  - `make` (no targets) : builds the included demos
  - `make run` : builds and runs all the included demos
  - `make dox` : builds the doxygen reference file (needs doxygen installed...)

To run a single demo, run `bin/sample_X`.

 - If the symbol UDGCD_PRINT_STEPS is defined at build time, then different steps will be printed on `std::cout` (useful only for debugging purposes).
 For the provided samples, this can be done by passing option `PRINT_STEPS=Y` to make.

If Graphviz/dot is installed, the demo samples will render the generated graphs into svg images in the `obj` folder.


### Issues:
 <a name="s_issues"></a>

 - At present, this code requires a static allocated variable (done automatically by compiler, as it is templated).
 Thus it is **not** thread safe, neither can it handle multiple graphs simultaneously.
 - A bug has been confirmed: more cycles are been computed than necessary, see https://github.com/skramm/udgcd/issues/1
. Currently under investigation...

### How does it work ?
 <a name="s_inside"></a>

Three steps are involved: first we need to check if there **is** at least one cycle. Is this is true, we explore the graph to find it/them.

- The first step is done by a Depth First Search (DFS), with  [boost::undirected_dfs()](http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/undirected_dfs.html)
with passing a visitor of class `CycleDetector`, inherited from
[boost::dfs_visitor](http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/dfs_visitor.html).
This object holds a set of vertices that are part of an edge on which `back_edge()` is called.
If this happens, it means that a cycle *has* been encountered.

- The second step is done by exploring recursively the graph, by starting from each of the vertices that have been identified as part of a "back edge".

- The third steps does a lot of post-processing. At present, we have all the possibles paths and we need to filter-out many of them.
 The steps are:
 1. remove symmetrical paths (i.e. : 1-2-3 is the same as 1-3-2)
 1. remove duplicate paths  (i.e. : 1-2-3 is the same as 2-3-1)
 1. remove non-chordless cycles (see [WP page](https%3A%2F%2Fen.wikipedia.org%2Fwiki%2FCycle_%28graph_theory%29%23Chordless_cycles))
 1. remove composite cycles (WIP, NOT DONE YET, SEE https://github.com/skramm/udgcd/issues/1)

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
- https://stackoverflow.com/a/43481372/193789

  ### Notes
  <a name="s_notes"></a>

- In the output vector, the paths are sorted, so you need to have the `<` operator defined for the vertices. Sorting is done such as:
 - The smallest element is in first position;
 - The element in second position is smaller than the last one.

 As an example, say you have a raw cycle expressed as
   `6-2-1-4`, it is released as `1-2-6-4` (and not `1-4-6-2`).

<a name="note_bp"></a>
  #### User properties & graph type

You can use whatever edge and vertices types, the coloring needed by the algorithm is handled by providing color maps as external properties. So if you have no special needs on vertices and edge properties, you can use something as trivial as this:
```C++
    typedef boost::adjacency_list<
	   boost::vecS,
	   boost::vecS,
	   boost::undirectedS
	> graph_t;
```
But you can also have some user properties, defines as bundled properties. For example:
```C++
struct my_Vertex
{
	int         v1;
	std::string v2;
	float       v3;
};
struct my_Edge
{
		int         e1;
		std::string e2;
};
```
Then your graph type definition will become:
```C++
	typedef boost::adjacency_list<
		boost::vecS,                   // edge container
		boost::vecS,                   // vertex container
		boost::undirectedS,            // type of graph
		my_Vertex,                   // vertex type
		my_Edge
		> graph_t;
```
