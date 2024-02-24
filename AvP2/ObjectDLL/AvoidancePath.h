// ----------------------------------------------------------------------- //
//
// MODULE  : AvoidancePath.h
//
// PURPOSE : AI Obstacle avoidance pathing system
//
// CREATED : 5/12/00
//
// ----------------------------------------------------------------------- //

#ifndef __AVOIDANCEPATH_H__
#define __AVOIDANCEPATH_H__


#include <vector>
#include <list>
#include <float.h>

#include "ltsmartlink.h"
#include "ObstacleMgr.h"  // for class Obstacle

class CAI;


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------




class ObstacleWaypoint
{
	friend ILTMessage & operator<<(ILTMessage & out, /*const*/ ObstacleWaypoint & x);
	friend ILTMessage & operator>>(ILTMessage & in, ObstacleWaypoint & x);

public :
	typedef std::vector<ObstacleWaypoint*> NeighborList;
	typedef NeighborList::iterator		   NeighborIterator;

	typedef float CostType;

public :

	ObstacleWaypoint(const Obstacle & new_obstacle, const LTVector & new_offset, bool new_isSolid = true, bool new_isGoal = false)
		: obstacle(new_obstacle),
		  offset(new_offset) ,

		  isGoal(new_isGoal),
		  isSolid(new_isSolid),
		  isInVolumes(false),

		  pathDist(FLT_MAX),
		  heuristic(0.0f),
		  prevPathPoint(LTNULL),
		  inClosed(false),
		  inOpen(false)
	{
		UpdatePosition();
	}

	LTVector GetPosition() const { return obstacle.position + offset; }
	const LTVector & GetObstaclePosition() const { return obstacle.position; }
	const Obstacle & GetObstacle() const { return obstacle; }
	
	void UpdatePosition()
	{
		obstacle.Update();
	}

	bool IsGoal() const { return isGoal; }
	void SetIsGoal( bool val ) { isGoal = val; }

	bool IsSolid() const { return isSolid; }
	void SetIsSolid( bool val ) { isSolid = val; }

	bool IsContainedByVolumes() const { return isInVolumes; }
	void UpdateContainedByVolumes(const LTVector & dims);

	bool IsIgnorable() const { return (obstacle.ignorable == LTTRUE); }

	void ClearNeighbors()
	{
		neighbors.clear();
	}

	void AddNeighbor(ObstacleWaypoint * waypoint_ptr)
	{
		ASSERT( waypoint_ptr );
		if( waypoint_ptr )
			neighbors.push_back(waypoint_ptr);
	}

	NeighborIterator BeginNeighbors() { return neighbors.begin(); }
	NeighborIterator EndNeighbors()	  { return neighbors.end(); }

	NeighborList::const_iterator BeginNeighbors() const { return neighbors.begin(); }
	NeighborList::const_iterator EndNeighbors() const { return neighbors.end(); }

	ObstacleWaypoint * GetPrevPathPoint() const { return prevPathPoint; }
	float  GetWeight() const { return pathDist+heuristic; }
	float  GetCost() const { return pathDist; }

	bool InClosed() const { return inClosed; }
	bool InOpen()   const { return inOpen;   }

	void SetInClosed()	  { inClosed = true; inOpen = false; }
	void SetInOpen()      { inClosed = false; inOpen = true; }

	void SetPrevNode(ObstacleWaypoint * val) { prevPathPoint = val; }
	void SetCost(float val) { pathDist = val; }
	void SetHeuristic(float val) { heuristic = val; }

	void ResetPathing() { prevPathPoint = LTNULL; pathDist = FLT_MAX; heuristic = 0.0f; inClosed = false; inOpen = false; }

private :

	Obstacle obstacle;
	LTVector offset;

	bool    isGoal;
	bool	isSolid;
	bool	isInVolumes;  // True if contained by some volume.


	// Used for pathing algorithm.

	NeighborList neighbors;

	float  pathDist;
	float  heuristic;
	ObstacleWaypoint * prevPathPoint;

	bool  inClosed;
	bool  inOpen;
};



class AvoidancePath
{
	friend ILTMessage & operator<<(ILTMessage & out, /*const*/ AvoidancePath & x);
	friend ILTMessage & operator>>(ILTMessage & in, AvoidancePath & x);

public :


	typedef std::list<ObstacleWaypoint> WayPointList;
	typedef std::vector<ObstacleWaypoint*>		PathHeap;


	AvoidancePath();

	void Update(HOBJECT hMover, const LTVector & my_dims, LTBOOL bIncludeIgnorable);
	void CheapUpdate();

	void SetGoal(const LTVector & new_goal, const LTSmartLink & hAIPathee);

	bool GetWaypoint( LTVector * goal, LTVector * next_goal,
		              const LTVector & my_pos, const LTVector & my_dims,
					  const LTVector & my_up );


	void AddObstacle( const Obstacle & obstacle_src,
		              LTFLOAT my_radius, 
					  LTVector my_right, const LTVector & my_up, LTVector my_forward );
	bool HasObstacle( const Obstacle & obstacle ) const;

	bool IsStuck() const { return isStuck; }

private :

	LTVector goal;
	bool  hasGoal;
	bool  isStuck;

	WayPointList  waypoints;

	PathHeap	  pathHeap;
};


#endif //__AVOIDANCEPATH_H__
