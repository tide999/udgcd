// Copyright Sebastien Kramm 2016-2020
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

/**
\file udgcd.hpp
\brief UnDirected Graph Cycle Detection. Finds all the cycles inside an undirected graph.

Home page: https://github.com/skramm/udgcd

Inspired from http://www.boost.org/doc/libs/1_58_0/libs/graph/example/undirected_dfs.cpp


See file README.md
*/

#ifndef HG_UDGCD_HPP
#define HG_UDGCD_HPP

#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <boost/graph/connected_components.hpp>

#include <boost/dynamic_bitset.hpp>       // needed ! Allows bitwise operations on dynamic size boolean vectors

#ifdef UDGCD_DEV_MODE
	#include <iostream>
	#define COUT if(1) std::cout
	#define PRINT_FUNCTION std::cout << "*** start function " <<  __FUNCTION__ << "()\n"
	#define PRINT_FUNCTION_2 if(1) std::cout << "*** start function " <<  __FUNCTION__ << "()"
	#define UDGCD_PRINT_STEPS
	#define PRINT_DIFF( step, v_after, v_before ) std::cout << step << ": REMOVAL OF " << v_before.size() - v_after.size() << " cycles\n"
#else
	#define PRINT_FUNCTION
	#define PRINT_FUNCTION_2 if(0) std::cout
	#define COUT if(0) std::cout
	#define PRINT_DIFF(a,b,c) ;
#endif

/// All the provided code is in this namespace
namespace udgcd {

//-------------------------------------------------------------------------------------------
template<typename T>
void
PrintVector( std::ostream& f, const std::vector<T>& vec )
{
	for( const auto& elem : vec )
		f << elem << "-";
	f << "\n";
}

/// Additional helper function, can be used to print the cycles found
template<typename T>
void
PrintPaths( std::ostream& f, const std::vector<std::vector<T>>& v_paths, const char* msg=0 )
{
	static int iter=0;
	f << "Paths (" << iter++ << "): nb=" << v_paths.size();
	if( msg )
		f << ": " << msg;
	f << "\n";

	for( size_t i=0; i<v_paths.size(); i++ )
	{
		f << " - " << i << ": ";
		PrintVector( f, v_paths[i] );
	}
}
//-------------------------------------------------------------------------------------------
/// holds private types and functions, unneeded to use this library
namespace priv {

/// holds a path as a binary vector.
/**
For a graph of \f$n\f$ vertices, its size needs to be \f$ n.(n-1)/2 \f$

Example: for the path 1-3-4 on a graph of 5 vertices (0 - 4), the vector will have a size of 10 elements:

\verbatim
edge:    0  0  0  0  1  1  1  2  2  3
         1  2  3  4  2  3  4  3  4  4
--------------------------------------
vector:  0  0  0  0  0  1  1  0  0  1
\endverbatim
*/
typedef boost::dynamic_bitset<> BinaryPath;

//-------------------------------------------------------------------------------------------
/// Counts the number of bits to 1
size_t
CountOnes( const BinaryPath& vec )
{
/*	size_t nb_ones = 0;
	for( size_t i=0; i<vec.size(); i++ )
		if( vec[i] )
			nb_ones++;
	return nb_ones;*/
	return vec.count(); // works with boost::dynamic_bitset
}
//-------------------------------------------------------------------------------------------
/// Print vector of bits
template<typename T>
void
PrintBitVector( std::ostream& f, const T& vec )
{
	for( size_t i=0; i<vec.size(); i++ )
	{
		f << vec[i];
		if( !((i+1)%4) && i != vec.size()-1 )
			f << '.';
	}
	f << ": #=" << CountOnes( vec ) << "\n";
}

template<typename T>
void
printBitMatrix( std::ostream& f, const T& mat, std::string msg )
{
	f << "Matrix " << msg << ", nbLines=" << mat.size() << "\n";
    for( auto line: mat )
    {
		f << " | ";
		for( size_t i=0; i<line.size(); i++ )
		{
			f << line[i];
			if( !((i+1)%4) && i != line.size()-1 )
				f << '.';
		}
		f << " |\n";
	}
}

//-------------------------------------------------------------------------------------------
/// Print vector of vectors of bits
template<typename T>
void
PrintBitVectors( std::ostream& f, const T& vec )
{
	f << "Binary vectors for each paths:\n";
	for( size_t i=0; i<vec.size(); i++ )
	{
		f << i << ": ";
		PrintBitVector( f, vec[i] );
	}
//	f << "\n";
}
//-------------------------------------------------------------------------------------------
/// Private, don't use.
/**
Recursive function, explores edges connected to \c v1 until we find a cycle

\warning Have to be sure there \b is a cycle, else infinite recursion !
*/
template <class Vertex, class Graph>
bool
explore(
	const Vertex& v1,                            ///< the starting vertex we want to explore
	const Graph&  g,
	std::vector<std::vector<Vertex>>& vv_paths,
	std::vector<std::vector<Vertex>>& v_cycles, ///< this is where we store the paths that have cycles
	int depth = 0
) {
	PRINT_FUNCTION_2 << " depth=" << depth << " v1=" << v1 << "\n";

	++depth;
	static int max_depth = std::max( depth, max_depth );
	assert( vv_paths.size()>0 );

//	typename boost::graph_traits<Graph>::out_edge_iterator ei, ei_end;
//	boost::tie(ei, ei_end) = out_edges( v1, g );

//	COUTP << "nb of edges = " << ei_end - ei << "\n";
//	size_t edge_idx = 0;

//	std::vector<Vertex> src_path = vv_paths[vv_paths.size()-1];
	std::vector<Vertex> src_path = vv_paths.back();

#ifdef UDGCD_DEV_MODE
	COUT << "src_path: "; PrintVector( std::cout, src_path );
//	int iter=0;
//	auto tmp = boost::out_edges( v1, g );
//	size_t nbedges = tmp.second - tmp.first;
#endif

	bool found = false;
//	for( ; ei != ei_end; ++ei ) //, ++edge_idx )         // iterating on all the output edges
	for( auto oei = boost::out_edges( v1, g ); oei.first != oei.second; ++oei.first ) //, ++edge_idx )         // iterating on all the output edges
	{
		bool b = false;
		Vertex v2a = boost::source(*oei.first, g);
		Vertex v2b = boost::target(*oei.first, g);
#ifdef UDGCD_DEV_MODE
//		COUT << ++iter << '/' << nbedges << " - v1=" << v1 << ": connected edges v2a=" << v2a << " v2b=" << v2b << "\n";
#endif
		if( v2b == v1 && v2a == src_path[0] ) // we just found the edge that we started on, so no need to finish the current iteration, just move on.
			continue;

		std::vector<Vertex> newv(src_path);  // create new path from source
		bool AddNode = true;
		if( newv.size() > 1 )
			if( newv[ newv.size()-2 ] == v2b )
				AddNode = false;

		if( AddNode )
		{
			if( std::find( newv.cbegin(), newv.cend(), v2b ) != newv.cend() )
			{
				newv.push_back( v2b );
				COUT << "*** FOUND CYCLE: "; PrintVector( std::cout, newv );
				v_cycles.push_back( newv );
				return true;
			}
			else
			{
				newv.push_back( v2b );         // else add'em and continue
				COUT << "  -adding vector ";  for( const auto& vv:newv )	COUT << vv << "-"; COUT << "\n";
				vv_paths.push_back( newv );
				b = explore( v2b, g, vv_paths, v_cycles, depth );
			}
		}
		if( b )
			found = true;
	}
	return found;
}
//-------------------------------------------------------------------------------------------
#if 0
/// Private, don't use.
/**
 Remove twins : vector that are the same, but in reverse order
*/
template<typename T>
std::vector<std::vector<T>>
RemoveOppositePairs( const std::vector<std::vector<T>>& v_cycles )
{
	PRINT_FUNCTION;
	assert( v_cycles.size() );

	std::vector<std::vector<T>> out;      // output vector
	out.reserve( v_cycles.size() );       // preallocate memory

	std::vector<bool> flags( v_cycles.size(), true ); // some flags to keep track of which elements are reversed

	for( size_t i=0; i<v_cycles.size()-1; ++i )
	{
		if( flags[i] )
		{
			out.push_back( v_cycles[i] );                        //  1 - add current vector into output

#ifdef UDGCD_DEV_MODE
			COUT << "-Considering path " << i << ":  "; PrintVector( std::cout, v_cycles[i] );
#endif
			std::vector<T> rev = v_cycles[i];                       // step 1: build a reversed copy of the current vector
			std::reverse( rev.begin(), rev.end() );
			for( size_t j=i+1; j<v_cycles.size(); ++j )                  // step 2: parse the rest of the list, and check
				if( flags[j] && rev == v_cycles[j] )                     // if similar, then
				{
//					out.push_back( v_cycles[i] );                        //  1 - add current vector into output
					flags[j] = false;                                 //  2 -  invalidate the reversed one
#ifdef UDGCD_DEV_MODE
					COUT << " -> discarding path " << j << ":  "; PrintVector( std::cout, v_cycles[j] );
#endif
				}
		}
	}
// adding last one, if not discarded

	if( flags.back() )
		out.push_back( v_cycles.back() );

	return out;
}
#endif
//-------------------------------------------------------------------------------------------
template<typename T>
void
putSmallestElemFirst( std::vector<T>& vec )
{
	auto it = std::min_element( vec.begin(), vec.end() );     // rotate so that smallest is first
	std::rotate( vec.begin(), it, vec.end() );
}
//-------------------------------------------------------------------------------------------
#if 0
/// Private, don't use.
/**
Helper function for RemoveIdentical()

Given an input vector "DABCD", it will return "ABCD" (removal of duplicate element, and first element is the smallest)
*/
template<typename T>
std::vector<T>
GetSortedTrimmed( const std::vector<T>& v_in )
{
	assert( v_in.front() == v_in.back() ); // check that this is a cycle
	assert( v_in.size() > 2 );             // a (complete) cycle needs to be at least 3 vertices long

	std::vector<T> v_out( v_in.size() - 1 );                      // Trim: remove
	std::copy( v_in.cbegin(), v_in.cend()-1, v_out.begin() );     //  last element

	putSmallestElemFirst( v_out );

	if( v_out.back() < v_out[1] )                     // if we have 1-4-3-2, then
	{
		std::reverse( v_out.begin(), v_out.end() );   // we transform it into 2-3-4-1
		putSmallestElemFirst( v_out );                // and put smallest first: 1-2-3-4
	}
	return v_out;
}
#endif
//-------------------------------------------------------------------------------------------
/// Removes the parts that are not part of the cycle.
/**
Example:
- in: 1-2-3-4-5-3
- out: 3-4-5
*/
template<typename T>
std::vector<T>
findTrueCycle( const std::vector<T>& cycle )
{
	PRINT_FUNCTION;
//	COUT << "in: "; PrintVector( std::cout, cycle );
	assert( cycle.size() > 2 ); // 3 or more nodes
	if( cycle.size() == 3 )     // if 3 nodes, just return the input path
		return cycle;

	std::vector<T> out;
	out.reserve( cycle.size() );

	bool done = false;
	for( size_t i=0; i<cycle.size()-1 && !done; ++i )
	{
		const T& n1 = cycle[i];
		for( size_t j=i+2; j<cycle.size() && !done; ++j )
		{
			const T& n2 = cycle[j];
			if( n1 == n2 )
			{
				out.resize( j-i );
				std::copy( std::begin(cycle) + i, std::begin(cycle) + j, std::begin(out) );
				done = true;
			}
		}
	}
	putSmallestElemFirst( out );   // if we have 4-3-2-1, then transform into 1-4-3-2

	if( out.back() < out[1] )                     // if we have 1-4-3-2, then
	{
		std::reverse( out.begin(), out.end() );   // we transform it into 2-3-4-1
		putSmallestElemFirst( out );                // and put smallest first: 1-2-3-4
	}

//	COUT << "out: "; PrintVector( std::cout, out );
	return out;
}
//-------------------------------------------------------------------------------------------
/// Removes for each cycle the parts that are not part of the cycle.
/**
Example:
- in: 1-2-3-4-5-3
- out: 3-4-5
*/
template<typename T>
std::vector<std::vector<T>>
cleanCycles( const std::vector<std::vector<T>>& v_cycles )
{
	PRINT_FUNCTION;
	assert( v_cycles.size() );

	std::vector<std::vector<T>> out;
	out.reserve( v_cycles.size() );

	for( const auto& cycle: v_cycles )
	{
		auto newcy = findTrueCycle( cycle );
		if( std::find( std::begin(out), std::end(out), newcy ) == std::end(out) )     // add to output vector only if not already present
			out.push_back( newcy );
	}
	return out;
}
//-------------------------------------------------------------------------------------------
#if 0
/// Private, don't use.
/**
Remove identical strings that are the same up to the starting point
It also sorts the paths by rotating them so that the node of smallest index is first
*/
template<typename T>
std::vector<std::vector<T>>
RemoveIdentical( const std::vector<std::vector<T>>& v_cycles )
{
	PRINT_FUNCTION;
	assert( v_cycles.size() );

	if( v_cycles.size() == 1 )                                   // if single path in input, then we justs add it, after trimming/sorting
	{
		std::vector<std::vector<T>> out( 1, GetSortedTrimmed( v_cycles[0] ) );
		return out;
	}

	std::vector<std::vector<T>> out( v_cycles.size() );
	for( size_t i=0; i<v_cycles.size(); i++ )            // 1 - fill output vector with sorted/trimmed paths
		out[i] = GetSortedTrimmed( v_cycles[i] );

	std::sort( out.begin(), out.end() );                 // 2 - sort
	out.erase(                                           // 3 - erase the ones that are
		std::unique( out.begin(), out.end() ),           //  consecutive duplicates
		out.end()
	);

	return out;
}
#endif

//-------------------------------------------------------------------------------------------
#if 0
/// Returns true if vertices \c v1 and \c v2 are connected by an edge
/**
http://www.boost.org/doc/libs/1_59_0/libs/graph/doc/IncidenceGraph.html#sec:out-edges
*/
template<typename vertex_t, typename graph_t>
bool
AreConnected( const vertex_t& v1, const vertex_t& v2, const graph_t& g )
{
	auto pair_edge = boost::out_edges( v1, g );                      // get iterator range on edges
	for( auto it = pair_edge.first; it != pair_edge.second; ++it )
		if( v2 == boost::target( *it, g ) )
			return true;
	return false;
}
//-------------------------------------------------------------------------------------------
/// Return true if cycle is chordless
/**
See
- https://en.wikipedia.org/wiki/Cycle_(graph_theory)#Chordless_cycles

Quote:
<BLOCKQUOTE>
"A chordless cycle in a graph, also called a hole or an induced cycle, is a cycle such that
no two vertices of the cycle are connected by an edge that does not itself belong to the cycle."
</BLOCKQUOTE>
*/
template<typename vertex_t, typename graph_t>
bool
IsChordless( const std::vector<vertex_t>& path, const graph_t& g )
{
	if( path.size() < 4 ) // no need to test if less than 4 vertices
		return true;

	for( size_t i=0; i<path.size()-3; ++i )
	{
		for( size_t j=i+2; j<path.size()-1; ++j )

		if( AreConnected( path[i], path[j], g ) )
			return false;
	}
	return true;
}
//-------------------------------------------------------------------------------------------
/// Third step, remove non-chordless cycles
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
RemoveNonChordless( const std::vector<std::vector<vertex_t>>& v_in, const graph_t& g )
{
	PRINT_FUNCTION;

	std::vector<std::vector<vertex_t>> v_out;
	v_out.reserve( v_in.size() ); // to avoid unnecessary memory reallocations and copies
    for( const auto& cycle: v_in )
		if( IsChordless( cycle, g ) )
			v_out.push_back( cycle );
	return v_out;
}
#endif
//-------------------------------------------------------------------------------------------
/// holds two vertices, used in RemoveRedundant()
template <typename vertex_t>
struct VertexPair
{
	vertex_t v1,v2;
	VertexPair() {}
	VertexPair( vertex_t va, vertex_t vb ): v1(va), v2(vb)
	{
		if( v2<v1 )
			std::swap( v1, v2 );
	}
	// 2-3 is smaller than 2-4
	friend bool operator < ( const VertexPair& vp_a, const VertexPair& vp_b )
	{
		if( vp_a.v1 < vp_b.v1 )
			return true;
		if( vp_a.v1 == vp_b.v1 )
			return ( vp_a.v2 < vp_b.v2 );
		return false;
	}

#ifdef UDGCD_DEV_MODE
	friend std::ostream& operator << ( std::ostream& s, const VertexPair& vp )
	{
		s << '(' << vp.v1 << '-' << vp.v2 << ')';
		return s;
	}
#endif
};
//-------------------------------------------------------------------------------------------
template<typename vertex_t>
void
PrintSet( const std::set<VertexPair<vertex_t>>& set_edges, std::string msg )
{
	std::cout << "set: " << msg << '\n';
	for( const auto& e: set_edges )
		std::cout << e << '-';
	std::cout << '\n';
}
//-------------------------------------------------------------------------------------------
#if 0
/// Post-process step: removes paths (cycles) that are redundant (i.e. that can be deduced/build from the others)
/**
arg is not const, because it gets sorted here.
\bug bug here, see RemoveRedundant2
*/
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
RemoveRedundant( std::vector<std::vector<vertex_t>>& v_in, const graph_t& g )
{
	PRINT_FUNCTION;

	std::vector<std::vector<vertex_t>> v_out;
	v_out.reserve( v_in.size() ); // to avoid unnecessary memory reallocations and copies

	std::set<VertexPair<vertex_t>> set_edges;

/// preliminary sorting by length, so we keep the shortest paths
	std::sort(
		std::begin(v_in),
		std::end(v_in),
		[]                                                                       // lambda
		( const std::vector<vertex_t>& vv1, const std::vector<vertex_t>& vv2 )
		{ return vv1.size() < vv2.size(); }
	);
	PrintPaths( std::cout, v_in, "After sorting" );

/// enumerate the cycles and store each edge in set. Add cycle to output only if new edge detected
    for( const auto& cycle: v_in )
    {
//		PrintPath( std::cout, cycle );
//		PrintSet( set_edges, "start" );
		bool newEdgeFound(false);
		for( size_t i=0; i<cycle.size(); ++i )
		{
			VertexPair<vertex_t> vp( (i==0?cycle[cycle.size()-1]:cycle[i-1]), cycle[i] );
			auto rv = set_edges.insert( vp );
			if( rv.second == true )  // if insertion took place, it means the edge wasn't already there
				newEdgeFound = true;
//			std::cout << "i=" << i << " vp=" << vp << " found=" << newEdgeFound << '\n';
		}
		if( newEdgeFound )
		{
//			std::cout << " => Adding current cycle to output vector\n";
			v_out.push_back( cycle );
		}
	}
	return v_out;
}
#endif
//-------------------------------------------------------------------------------------------
/// Builds the binary vector \c binvect associated to the cycle \c cycle.
/// The index map \c idx_map is used to fetch the index in binary vector from the two vertices indexes
/**
- sample input: 1-3-5

*/
template<typename vertex_t>
void
buildBinaryVector(
	const std::vector<vertex_t>& cycle,    ///< input cycle
	BinaryPath&                  binvect,  ///< output binary vector (must be allocated)
	const std::vector<size_t>&   idx_map ) ///< reference index map, see how it is build in \ref BuildBinaryVectors() and \ref BuildBinaryIndexMap(()
{
	for( size_t i=0; i<cycle.size(); ++i )
	{
		VertexPair<vertex_t> vp( (i==0?cycle[cycle.size()-1]:cycle[i-1]), cycle[i] );
		size_t idx = idx_map[vp.v1] + vp.v2 - 1;
		assert( idx < binvect.size() );
		binvect[idx] = 1;
	}
//	PrintVector( std::cout, binvect );
}
//-------------------------------------------------------------------------------------------
/// Build table of series $y_n = y_{n-1}+N-n-1$
/**
This is needed to build the binary vector associated with a path, see \ref buildBinaryVector()

For example, if 6 vertices (indexes 0 to 5), then there are 6*(6-1)/2 = 15 possible edges:
\verbatim
        0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
edges:  01-02-03-04-05-12-13-14-15-23-24-25-34-35-45
\endverbatim

This function builds a vector of 6-1=5 elements:
\verbatim
0-4-7-9-10
\endverbatim
With those values, we get the index of a given edge in the binary vector with the formulae:
\f$ idx = idx_map[v1] + v2 - 1 \f$
with \f$ v1 \f$ being the smallest vertex and \f$ v2 \f$ the other one.

*/
std::vector<size_t>
BuildBinaryIndexMap( size_t nbVertices )
{
	std::vector<size_t> idx_map( nbVertices-1 );
	idx_map[0] = 0;
	for( size_t i=1;i<nbVertices-1; i++ )
		idx_map[i] = idx_map[i-1] + nbVertices - i - 1;
	return idx_map;
}
//-------------------------------------------------------------------------------------------
/// Builds all the binary vectors for all the cycles
template<typename vertex_t>
void
buildBinaryVectors(
	const std::vector<std::vector<vertex_t>>& v_cycles,       ///< input cycles
	std::vector<BinaryPath>&                  v_binvect,      ///< output vector of binary vectors
	size_t                                    nbVertices )    ///< nb of vertices of the graph
{
	PRINT_FUNCTION;
	assert( v_cycles.size() == v_binvect.size() );

	size_t nbCombinations = nbVertices * (nbVertices-1) / 2;
//	std::cout << "nbCombinations=" << nbCombinations << '\n';

	std::vector<size_t> idx_map = BuildBinaryIndexMap( nbVertices );
	std::cout << "idx_map: "; PrintVector( std::cout, idx_map );

    for( auto& binvect: v_binvect )
		binvect.resize( nbCombinations );

	for( size_t i=0; i<v_cycles.size(); i++ )
		buildBinaryVector( v_cycles[i], v_binvect[i], idx_map );
}

using RevBinMap = std::vector<std::pair<size_t,size_t>>;

//-------------------------------------------------------------------------------------------
/// Builds an index map giving from an index in the binary vector the indexes of the two vertices
/// that are connected. See \ref convertBC2VC()
/**
Example for n=5 (=> size=10)
\verbatim
0 | 0 1
1 | 0 2
2 | 0 3
3 | 0 4
4 | 1 2
5 | 1 3
6 | 1 4
7 | 2 3
8 | 2 4
9 | 2 5
\endverbatim
*/
RevBinMap
buildReverseBinaryMap( size_t nb_vertices )
{
	PRINT_FUNCTION;
	size_t nb_combinations = nb_vertices*(nb_vertices-1)/2;
	RevBinMap out( nb_combinations );
	size_t v1 = 0;
	size_t v2 = 1;
	for( size_t i=0; i<nb_combinations; ++i )
	{
		if( v2 == nb_vertices )
		{
			v1++;
			v2 = v1 + 1;
		}
		out[i].first  = v1;
		out[i].second = v2++;
//		COUT << i << ": (" << out[i].first << " - " << out[i].second << ")\n";
	}
	return out;
}
//-------------------------------------------------------------------------------------------
/// Convert, for a given graph, a Binary Cycle (BC) \c v_in to a Vertex Cycle (VC)
/**
Algorithm:
-using rev_map, that gives for a given index in the binary vector the two corresponding vertices

* step 1: build a map, giving for each vertex, the next one

FOR each element in BV with value 1:
 - fetch the two values v1 and v2 in rev_map
 - if( flag[v1] is false, set flag[v1] to true
   else vertex_map[v1] = v2

 - if( flag[v2] is false, set flag[v2] to true
   else vertex_map[v2] = v1

* step 2: parse the map, and add the vertices to the output vector

\todo The downside of this approach is that we need to build before the \c rev_map, that can be pretty big...
Maybe we can find a better way ?
*/
template<typename vertex_t>
std::vector<vertex_t>
convertBC2VC(
	const BinaryPath& v_in,         ///< input binary path
	size_t            nbVertices,   ///< nb of vertices of the graph
	const RevBinMap&  rev_map       ///< required map, has to be build before, see buildReverseBinaryMap()
)
{
	assert( v_in.size() == nbVertices * (nbVertices-1) / 2 );
	assert( v_in.size() == rev_map.size() );

//	PRINT_FUNCTION;
//	PrintBitVector( std::cout, v_in );

	std::vector<vertex_t> v_out;
	std::vector<vertex_t> flag( nbVertices, false );
	std::map<vertex_t,vertex_t> vertex_map;

// step 1: build map from binary vector
	{
		bool firstone(true);
		for( size_t i=0; i<v_in.size(); ++i )
		{
			if( v_in[i] == 1 )
			{

				vertex_t v1 = rev_map[i].first;
				vertex_t v2 = rev_map[i].second;
				if( firstone )
				{
					firstone = false;
					v_out.push_back( v1 );
					v_out.push_back( v2 );
				}
				assert( v1 < nbVertices );
				assert( v2 < nbVertices );
//				COUT << i << ": v1=" << v1 << " v2=" << v2 << " flag[v1]=" << flag.at(v1) << " flag[v2]=" << flag.at(v2) << "\n";

				if( !flag.at(v1) )
					flag.at(v1) = true;
				else
					vertex_map[v1] = v2;

				if( !flag.at(v2) )
					flag.at(v2) = true;
				else
					vertex_map[v2] = v1;
			}
		}
	}
#if 0
	COUT << "v_out: " << v_out[0] << "-" << v_out[1] << "\n";
	COUT << "VERTEX MAP:\n";
	for( const auto& vp: vertex_map )
		COUT << vp.first << "-" << vp.second << "\n";
#endif

// step 2: extract vertices from map
//	COUT << "step2:\n";
	vertex_t current = v_out[1];
	auto nbOnes = v_in.count()-2;
	do
	{
		v_out.push_back( vertex_map[current] );
//		COUT << "ADDING " << vertex_map[current] << "\n";
		current = vertex_map[current];
	}
	while( --nbOnes );

	return v_out;
}
//-------------------------------------------------------------------------------------------
/// Gaussian binary elimination
/**
Input: a binary matrix
output: a reduced matrix

- assumes no identical rows
*/
std::vector<BinaryPath>
gaussianElim( const std::vector<BinaryPath>& mat )
{
	assert( mat.size() > 1 );
	auto m_in = mat; // copy so we can edit it (arg is const)

	std::vector<BinaryPath> m_out;
	size_t col = 0;
	size_t nb_rows = mat.size();
	size_t nb_cols = mat[0].size();
	size_t c=0;
	bool done = false;

//	printBitMatrix( std::cout, m_in, "m_in INITIAL" );

	std::vector<bool> tag(nb_rows,false);
	do
	{
		std::cout << "\n* start iter " << ++c << ", current col=" << col
			<< " #tagged lines = " << std::count( tag.begin(),tag.end(), true ) << "\n";

		bool found = false;
		for( size_t row=0; row<nb_rows; row++ )                // search for first row with a 1 in current column
		{
			std::cout << "considering line " << row << "\n";
			if( tag[row] == false && m_in[row][col] == 1 )    // AND not tagged
			{
				found = true;
				std::cout << "row: " << row << ": found 1 in col " << col << "\n";
				m_out.push_back( m_in.at(row) );
				tag[row] = true;
				if( row < nb_rows-1 )
				{
					for( size_t i=row+1; i<nb_rows; i++ )      // search for all following rows that have a 1 in that column
					{
						if( tag[i] == false )                  // AND that are not tagged.
							if( m_in[i][col] == 1 )            // it there is, we XOR them with initial line
							{
								auto res = m_in[i] ^ m_in[row];
								m_in[i] = res;
							}
					}
				}
				std::cout << "BREAK loop\n";
				break;
			}
		}
		if( !found )
		{
			std::cout << "no row with 1 in col " << col << ", done!\n";
//			done = true;
		}
		std::cout << "switch to next col\n";
		col++;
		if( col == nb_cols )
		{
			std::cout << "All columns done, end\n";
			done = true;
		}
		if( std::find(tag.begin(),tag.end(), false ) == tag.end() )
		{
			std::cout << "All lines tagged, end\n";
			done = true;
		}

		printBitMatrix( std::cout, m_in, "m_in" );
		printBitMatrix( std::cout, m_out, "m_out" );
	}
	while( !done );
	return m_out;
}
//-------------------------------------------------------------------------------------------
#if 0
/// Post-process step: removes paths (cycles) that are redundant (i.e. that can be deduced/build from the others)
/**
arg is not const, because it gets sorted here.
*/
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
RemoveRedundant2( std::vector<std::vector<vertex_t>>& v_in, const graph_t& g )
{
	PRINT_FUNCTION;

/// IMPORTANT: the code below assumes we have at least 3 cycles, so lets exit right away if not !
	if( v_in.size() < 3 )
		return v_in;

	std::vector<std::vector<vertex_t>> v_out;
	v_out.reserve( v_in.size() ); // to avoid unnecessary memory reallocations and copies

/// preliminary sorting by length, so we keep the shortest paths first
	std::sort(
		std::begin(v_in),
		std::end(v_in),
		[]                                                                       // lambda
		( const std::vector<vertex_t>& vv1, const std::vector<vertex_t>& vv2 )
		{ return vv1.size() < vv2.size(); }
	);
	PrintPaths( std::cout, v_in, "After sorting" );

/// build for each cycle its associated binary vector
	std::vector<BinaryPath> v_binvect( v_in.size() );  // one binary vector per cycle
	size_t nb_vertices = boost::num_vertices(g);
	buildBinaryVectors( v_in, v_binvect, boost::num_vertices(g) );

	auto rev_map = buildReverseBinaryMap( nb_vertices );

#ifdef UDGCD_DEV_MODE
	PrintBitVectors( std::cout, v_binvect );
	COUT << "Comparing XORed paths 2 by 2\n";
#endif

/// build the composed cycles by taking each pair of cycle, XOR it,
/// and compare to the others
	boost::dynamic_bitset<> v_removals( v_in.size() );  // sets all to 0, 1 means it will be removed
	size_t counter1(0);
	size_t nbRemovals(0);
	size_t nbNonChordless(0);

    for( size_t i=0; i<v_in.size()-1; i++ )
		for( size_t j=i+1; j<v_in.size(); j++ )
		{
			counter1++;
			std::cout << "\n* i=" << i << " j=" << j << "\n";
			PrintVector( std::cout, v_in[i] ); PrintBitVector( std::cout, v_binvect[i] );
			PrintVector( std::cout, v_in[j] ); PrintBitVector( std::cout, v_binvect[j] );
			auto res = v_binvect[i] ^ v_binvect[j];
			COUT << "p" << i << " EXOR p" << j << "="; PrintBitVector( std::cout, res );
			auto xored_path = convertBC2VC<vertex_t>( res, nb_vertices, rev_map );
			PrintVector( std::cout, xored_path );

#if 0
			if( xored_path.size() > 3 )
				if( !IsChordless( xored_path, g ) )
				{
					nbNonChordless++;
					COUT << " => is NOT chordless!\n";
				}
#endif

			for( size_t k=0; k<v_in.size(); k++ )
				if( k != i && k != j )
				{
//					std::cout << "compare to elem " << k << ": "; PrintVector( std::cout, v_in[k] ); PrintBitVector( std::cout, v_binvect[k] );
					if( v_removals[k] == 0 )        // if already to 1, no need to check
						if( v_removals[j] == 0 )
					{
						auto maxsize = std::max( v_in[i].size(), v_in[j].size() );
						if( v_in[k].size() >= maxsize )
						{
//							std::cout << " -size=" << v_in[k].size() << " >= " << maxsize << '\n';
							if( res == v_binvect[k] )
							{
								std::cout << "compare to elem " << k << ": "; PrintVector( std::cout, v_in[k] ); PrintBitVector( std::cout, v_binvect[k] );
								v_removals[k] = 1; std::cout << " remove it!\n";
								nbRemovals++;
							}
						}
					}
				}
		}

	std::cout << "* counter1=" << counter1 << "\n";
	std::cout << "* Nb NonChordless=" << nbNonChordless << "\n";

	std::cout << "Nbremovals=" << nbRemovals << '\n';
    for( size_t i=0; i<v_in.size(); i++ )        // finally, copy all the correct
    {
		COUT << "Elem " << i;
		if( v_removals[i] == 0 )                 // cycles to output vector
		{
			v_out.push_back( v_in[i] );
			COUT << ": keep\n";
		}
		else
			COUT << "=" << v_removals[i] << " : remove\n";
	}

	return v_out;
}
#endif
//-------------------------------------------------------------------------------------------
/// Post-process step: removes cycles based on Gaussian Elimination
/**
arg is not const, because it gets sorted here.
*/
template<typename vertex_t, typename graph_t>
std::vector<std::vector<vertex_t>>
removeRedundant3( std::vector<std::vector<vertex_t>>& v_in, const graph_t& g )
{
	PRINT_FUNCTION;

// IMPORTANT: the code below assumes we have at least 3 cycles, so lets exit right away if not !
	if( v_in.size() < 3 )
		return v_in;

	std::vector<std::vector<vertex_t>> v_out;
	v_out.reserve( v_in.size() ); // to avoid unnecessary memory reallocations and copies

// build for each cycle its associated binary vector
	std::vector<BinaryPath> v_binvect( v_in.size() );  // one binary vector per cycle
	buildBinaryVectors( v_in, v_binvect, boost::num_vertices(g) );
	printBitMatrix( std::cout, v_binvect, "removeRedundant3(): input binary matrix" );

	std::vector<BinaryPath> v_bpaths = gaussianElim( v_binvect );

	printBitMatrix( std::cout, v_bpaths, "removeRedundant3(): output binary matrix" );

// convert back binary cycles to vertex-based cycles, using a reverse map
	size_t nb_vertices = boost::num_vertices(g);

	auto rev_map = buildReverseBinaryMap( nb_vertices );
	for( auto bcycle: v_bpaths )
	{
		auto cycle = convertBC2VC<vertex_t>( bcycle, nb_vertices, rev_map );
		v_out.push_back( cycle );
	}
	return v_out;
}
//-------------------------------------------------------------------------------------------
} // namespace priv

//-------------------------------------------------------------------------------------------
/// Cycle detector for an undirected graph
/**
Passed by value as visitor to \c boost::undirected_dfs()

See http://www.boost.org/doc/libs/1_58_0/libs/graph/doc/undirected_dfs.html
*/
template <typename vertex_t>
struct CycleDetector : public boost::dfs_visitor<>
{
	template<typename T1, typename T2>
	friend std::vector<std::vector<T2>> FindCycles( T1& g );

	public:
		CycleDetector()
		{
			v_source_vertex.clear();
		}
		bool cycleDetected() const { return !v_source_vertex.empty(); }
		template <class Edge, class Graph>
		void back_edge( Edge e, const Graph& g )     // is invoked on the back edges in the graph.
		{
			vertex_t vs = boost::source(e, g);
			vertex_t vt = boost::target(e, g);
	#ifdef UDGCD_PRINT_STEPS
			std::cout << " => CYCLE DETECTED! vs=" << vs << " vt=" << vt << "\n";
	#endif
			if(                                                                                                // add vertex to
				std::find( v_source_vertex.cbegin(), v_source_vertex.cend(), vs ) == v_source_vertex.cend()    // the starting point list
				&&                                                                                             // only if both are
				std::find( v_source_vertex.cbegin(), v_source_vertex.cend(), vt ) == v_source_vertex.cend()    // not already inside
			)
				v_source_vertex.push_back( vs );
		}
	private:
		static std::vector<vertex_t> v_source_vertex;
};


/// static var instanciation
template<class T>
std::vector<T> CycleDetector<T>::v_source_vertex;

//-------------------------------------------------------------------------------------------
// USELESS ?
#if 0
/// Returns nb of cycles of the graph
template<typename graph_t>
size_t
NbCycles( const graph_t& g )
{
	std::vector<size_t> component( boost::num_vertices( g ) );
	auto nb_cc = boost::connected_components( g, &component[0] );
	return boost::num_edges(g) -  boost::num_vertices(g) + nb_cc;
}


} // namespace priv
#endif

//-------------------------------------------------------------------------------------------
/// Main user interface: just call this function to get the cycles inside your graph
/**
Returns a vector of cycles that have been found in the graph
*/
template<typename graph_t, typename vertex_t>
std::vector<std::vector<vertex_t>>
FindCycles( graph_t& g )
{
	if( boost::num_vertices(g) < 3 || boost::num_edges(g) < 3 )
		return std::vector<std::vector<vertex_t>>();

	CycleDetector<vertex_t> cycleDetector;

// vertex color map
	std::vector<boost::default_color_type> vertex_color( boost::num_vertices(g) );
	auto idmap = boost::get( boost::vertex_index, g );
	auto vcmap = make_iterator_property_map( vertex_color.begin(), idmap );

// edge color map
	std::map<typename graph_t::edge_descriptor, boost::default_color_type> edge_color;
	auto ecmap = boost::make_assoc_property_map( edge_color );

// step 1: do a DFS
	boost::undirected_dfs( g, cycleDetector, vcmap, ecmap, 0 );

	if( !cycleDetector.cycleDetected() )             // if no detection,
		return std::vector<std::vector<vertex_t>>(); //  return empty vector, no cycles found


	std::vector<std::vector<vertex_t>> v_cycles;     // else, get the cycles.



// search paths only starting from vertices that were registered as source vertex
	for( const auto& vi: cycleDetector.v_source_vertex )
	{
		COUT << "\n * Start exploring from source vertex " << vi << "\n";
		std::vector<std::vector<vertex_t>> v_paths;
		std::vector<vertex_t> newv(1, vi ); // start by one of the filed source vertex
		v_paths.push_back( newv );
		priv::explore( vi, g, v_paths, v_cycles );    // call of recursive function

#ifdef UDGCD_PRINT_STEPS
//	std::cout << "considering vertex " << vi << ": v_cycle size=" << v_cycles.size() << '\n';
//	PrintPaths( std::cout, v_cycles, "temp" );
#endif

	}

#ifdef UDGCD_PRINT_STEPS
	PrintPaths( std::cout, v_cycles, "Raw cycles" );
#endif

// post process 0: cleanout the cycles by removing steps that are not part of the cycle
	std::vector<std::vector<vertex_t>> v_cycles0 = priv::cleanCycles( v_cycles );
#ifdef UDGCD_PRINT_STEPS
	PrintPaths( std::cout, v_cycles0, "After cleanCycles()" );
#endif



// post process 1: remove the paths that are identical but reversed
/*	std::vector<std::vector<vertex_t>> v_cycles2 = RemoveOppositePairs( v_cycles0 );
#ifdef UDGCD_PRINT_STEPS
	PrintPaths( std::cout, v_cycles2, "After removal of symmetrical cycles" );
#endif
*/

// post process 2: remove twin paths
/*
	std::vector<std::vector<vertex_t>> v_cycles3 = RemoveIdentical( v_cycles2 );
#ifdef UDGCD_PRINT_STEPS
	PrintPaths( std::cout, v_cycles3, "After removal of identical cycles" );
#endif
*/

// post process 3: remove non-chordless cycles
//	std::vector<std::vector<vertex_t>> v_cycles4 = priv::RemoveNonChordless( v_cycles0, g );
//	PRINT_DIFF( "STEP 3", v_cycles4, v_cycles0 );
#ifdef UDGCD_PRINT_STEPS
//	PrintPaths( std::cout, v_cycles4, "After removal of non-chordless cycles" );
#endif

//std::vector<std::vector<vertex_t>> v_cycles5;
//size_t diff = 0;
//do {
// post process 4:
//	std::vector<std::vector<vertex_t>> v_cycles5 = priv::RemoveRedundant2( v_cycles4, g );
//	v_cycles5 = RemoveRedundant2( v_cycles4, g );
//	PRINT_DIFF( "STEP 4", v_cycles5, v_cycles4 );
#ifdef UDGCD_PRINT_STEPS
//	PrintPaths( std::cout, v_cycles5, "After removal of composed cycles" );
#endif
//	diff = v_cycles4.size() - v_cycles5.size();
//	v_cycles4 = v_cycles5;
//}
//while( diff != 0 );

// post process 3: remove non-chordless cycles
//	std::vector<std::vector<vertex_t>> v_cycles6 = RemoveNonChordless( v_cycles5, g );
#ifdef UDGCD_PRINT_STEPS
//	PrintPaths( std::cout, v_cycles6, "After removal of non-chordless cycles" );
#endif


	auto v_cycles5 = priv::removeRedundant3( v_cycles0, g );
#ifdef UDGCD_PRINT_STEPS
	PrintPaths( std::cout, v_cycles5, "After removeRedundant3()" );
#endif

	return v_cycles5;
}
//-------------------------------------------------------------------------------------------
namespace priv {

/// Recursive function, will iterate in graph and return true
/**
End condition
 - we found the initial node as \c next
 - we cannot find in all the linked nodes the next node (the one that is after \c idx_curr in \c cycle)
*/
template<typename vertex_t, typename graph_t>
bool
checkNextNode(
	const std::vector<vertex_t>& cycle,  ///< the cycle we are exploring
	size_t idx_curr,                     ///< current vertex index in above vector
	const graph_t& g                     ///< graph
)
{
	vertex_t start = cycle[0];
	vertex_t curr  = cycle[idx_curr];
	vertex_t next  = cycle[idx_curr==cycle.size()-1 ? 0 : idx_curr+1];
	assert( cycle.size() > 2 );

	for(                                             // iterate over edges
		auto it_pair = boost::out_edges( curr, g );  // of current vertex
		it_pair.first != it_pair.second;
		++it_pair.first
	)
	{
		auto vt = boost::target( *it_pair.first, g );
		if( idx_curr > 1 )
		{
			if( vt == start )    // if we meet the initial vertex
				return true;     // then, it is indeed a cycle!
		}
		if( vt == next )      // if we meet the next node, then we re-enter function
		{
			bool b = checkNextNode( cycle, ++idx_curr, g );
			if( b )                        // stop condition
				return true;
			break; // if if have found "next", we must not iterate on following nodes!
		}
	}
	return false;
}

/// Returns true if \c cycle is correct
/**
This function is only there for checking purposes
*/
template<typename vertex_t, typename graph_t>
bool
isACycle( const std::vector<vertex_t>& cycle, const graph_t& g )
{
//	PRINT_FUNCTION;
	if( cycle.size() > boost::num_vertices(g) )
		return false;

	return checkNextNode( cycle, 0, g );
}

//-------------------------------------------------------------------------------------------
/// Returns true if all cycles in \c v_in are makes sure
/**
This function is only there for checking purposes
*/
template<typename vertex_t, typename graph_t>
bool
checkCycles( const std::vector<std::vector<vertex_t>>& v_in, const graph_t& g )
{
	PRINT_FUNCTION;
	bool res = true;
	for( auto cycle: v_in )
	{
#if 0
		res = res & isACycle( cycle, g );
#else
		bool b = isACycle( cycle, g );
		if( !b )
		{
			std::cout << "Error, computed cycle not a cycle:\n";
			PrintVector( std::cout, cycle );
		}
		res = res & b;
#endif
	}
	return res;
}

} // namespace priv end
//-------------------------------------------------------------------------------------------

} // udgcd namespace end

#endif // HG_UDGCD_HPP
