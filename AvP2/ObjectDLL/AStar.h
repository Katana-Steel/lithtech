
// ----------------------------------------------------------------------- //
//
// MODULE  : AStar.h
//
// PURPOSE : A templatized A* pathing algorithm.
//
// CREATED : 7/3/00
//
// ----------------------------------------------------------------------- //


#ifndef __ASTAR_H__
#define __ASTAR_H__

/*  Here are some suggested classes for use with AStar::FindPath.

class PathNode
{
public :
	typedef std::vector<PathNode*> NeighborList;
	typedef NeighborList::iterator  NeighborIterator;

	typedef float CostType;
public :

	// isGoal, inClosed, and inOpen are the only value which need
	// to be set for path finding starts.
	PathNode_Suggestion(bool is_goal = false)
		: isGoal(is_goal),
		  cost(FLT_MAX),  // this doesn't need to be set to FLT_MAX, it's just the most intelligent value for no previous point.
		  heuristic(0.0f),
		  inClosed(false),
		  inOpen(false)   {}

	void  SetCost(float val) { cost = val; }
	void  SetInClosed()      { inOpen = false; inClosed = true;  }
	void  SetInOpen()        { inOpen = true;  inClosed = false; }
	void  SetHeuristic(float val) { heuristic = val; }
	void  SetPrevNode(PathNode_Suggestion * prevNode)

	bool  IsGoal() const { return isGoal; }
	float GetCost() const { return cost; }
	float GetWeight()   const { return cost + heuristic; }
	
	bool  InClosed() const { return inClosed; }
	bool  InOpen()   const { return inOpen;   }


	NeighborIterator BeginNeighbors() { return neighbors.begin(); }
	NeighborIterator EndNeighbors()   { return neighbors.end();   }


private :

	bool  isGoal;

	float cost;
	float heuristic;

	bool  inOpen;
	bool  inClosed;
	PathNode * prevPathPoint;

	NeighborList neighbors;
};

class FindPathNodeCost
{
public:

	float operator()(const PathNode & x, const PathNode & y)
	{
		return (x.GetPosition() - y.GetPosition()).MagSqr();
	}
};

class FindPathNodeHeuristic
{
	const LTVector goal;

public:

	FindPathNodeHeuristic(const LTVector & new_goal)
		: goal(new_goal) {}

	float operator()(const PathNode & x) const
	{
		return (x.GetPosition() - goal).MagSqr();
	}
};
	

*/

#include <algorithm>

namespace AStar
{

	// These are a few utility functions for FindPath().
	template< typename PathNode >
	class CompareWeights
	{
	public:

		bool operator()(const PathNode * lhs, const PathNode * rhs) const
		{
			return lhs->GetWeight() > rhs->GetWeight();
		}
	};
	

	
	// Finds the shortest (least cost) path from the starting_node to a node marked goal 
	// (notice that you can have more than one goal node).  If the heuristic always 
	// underestimates (or exactly estimates) the distance from a node to the goal (or all goals),
	// then the shortest path is guaranteed to be returned.
	//
	// The goal node reached is returned and will be pointing to the previous node on the path.
	//
	// If a path to a goal node could not be found, the last examined path is returned.
	//
	// Check that the node returned is a goal node to see if a path was found.
	
	template< typename PathNode, typename DetermineCost, typename DetermineHeuristic, typename HeapContainer>
	PathNode * FindPath( PathNode & starting_node, 
							  DetermineCost determine_cost,
							  DetermineHeuristic determine_heuristic,
							  HeapContainer & path_heap,
  							  typename PathNode::CostType max_search_cost )
	{

		PathNode * last_examined_node = LTNULL;

		// path_heap is the "Open" set.
		path_heap.clear();

#ifdef ASTAR_DEBUG
		uint32 max_heap_size = 0;
		uint32 num_visitations = 0;
		uint32 num_nodes_opened = 0;
		uint32 num_nodes_reheaped = 0;
#endif

		// Put the starting point into the open set, with appropriate starting values.
		starting_node.SetCost(0.0f);
		starting_node.SetHeuristic( determine_heuristic(starting_node) );
		path_heap.push_back( &starting_node );

#ifdef ASTAR_DEBUG
		++num_visitations;
		++num_nodes_opened;
#endif

		// This is the main loop, path_heap will grow within this loop.
		while( !path_heap.empty() )
		{
			ASSERT( path_heap.front() != LTNULL );

#ifdef ASTAR_DEBUG
			if( path_heap.size() > max_heap_size ) max_heap_size = path_heap.size();
#endif				
			PathNode & current = *path_heap.front();
			last_examined_node = &current;

			// Extract the top-most element in the open set, this is the element with
			// the least weight.  weight = (cost to get to a point) + (heuristic cost from point to goal).
			std::pop_heap( path_heap.begin(), path_heap.end(), AStar::CompareWeights<PathNode>() );
			path_heap.pop_back();

			// Here is how we "put" the current in the closed set.  It needn't really
			// be put into a container, it just needs to be marked as processed.
			current.SetInClosed();

			// If the node we just extracted is a goal node, we have our path!
			if( current.IsGoal() )
			{
#ifdef ASTAR_DEBUG
				g_pLTServer->CPrint( "%d nodes opened, %d nodes reheaped, %d visitations.  Max heap size was %d.",
					num_nodes_opened, num_nodes_reheaped, num_visitations, max_heap_size);
#endif
				return &current;
			}

			// Go through this node's neighbors. 
			// If a neighboring node is in neither closed set nor open set, 
			// or if it costs less to reach the neighboring node by going through 
			// the current node, then record the current path as the cheapest
			// way to reach the neighbor and throw neighbor in the open set.
			// However, if the new cost of including the neighbor goes over max_search_cost,
			// just ignore the neighbor.

			for( typename PathNode::NeighborIterator iter = current.BeginNeighbors();
			iter != current.EndNeighbors(); ++iter )
			{
				ASSERT( *iter );
				PathNode & neighbor = *(*iter);

				_ASSERT( determine_cost(current,neighbor) >= 0);

#ifdef ASTAR_DEBUG
				++num_visitations;
#endif

				const typename PathNode::CostType new_cost = current.GetCost() + determine_cost(current,neighbor);
				if( new_cost < max_search_cost )
				{

					if(  (!neighbor.InOpen() && !neighbor.InClosed()) 
					   || new_cost < neighbor.GetCost()   )
					{

						// Either the neighbor has  not been processed (is in neither Open nor Closed),
						// or we just found a cheaper path to the neighbor.
						// So record this as the best node path to get to neighbor.

						neighbor.SetCost(new_cost);
						neighbor.SetHeuristic( determine_heuristic(neighbor) );
						neighbor.SetPrevNode( &current );
						
						if( !neighbor.InOpen() )
						{
							// New element needs to be put into heap (Open set).
							path_heap.push_back( &neighbor );
							std::push_heap( path_heap.begin(), path_heap.end(), AStar::CompareWeights<PathNode>() );
							neighbor.SetInOpen();
							
#ifdef ASTAR_DEBUG
							++num_nodes_opened;
#endif
						}
						else
						{
							// Node is already in open set, and so it needs to be "re-heaped".
							// This operation will _only_ work if the element decrease in value (if
							// using operator>() for a comparator).
							// In other words, only if the node needs to move up the heap.
							// Other wise this will break the heap and you would have to use std::make_heap()
							// on the whole list.

							typename HeapContainer::iterator open_iter = std::find(path_heap.begin(),path_heap.end(), &neighbor);
							ASSERT( open_iter != path_heap.end() );
							std::push_heap( path_heap.begin(), open_iter + 1, AStar::CompareWeights<PathNode>() );

#ifdef ASTAR_DEBUG
							++num_nodes_reheaped;
#endif
						}
					}

				} //if(    new_cost < max_search_cost )

			} //for( PathNode::NeighborIterator iter = current.BeginNeighbors();... )

		} //while( !path_heap.empty() )

		// If this point is reached, a path wasn't found. Are you sure you
		// have a node marked as goal?  If you are, then there is no possible
		// way to get from the starting node to the goal node by only
		// passing through the neighboring nodes.

#ifdef ASTAR_DEBUG
		g_pLTServer->CPrint( "%d nodes opened, %d nodes reheaped, %d visitations.  Max heap size was %d.",
			num_nodes_opened, num_nodes_reheaped, num_visitations, max_heap_size);
#endif

		return last_examined_node;

	} //template<...> PathNode * FindPath( PathNode & starting_node, ....)



} //namespace AStar


#endif  //__ASTAR_H__
