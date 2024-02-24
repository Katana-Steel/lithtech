// ----------------------------------------------------------------------- //
//
// MODULE  : AvoidancePath.cpp
//
// PURPOSE : 
//
// CREATED : 5/12/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AvoidancePath.h"
#include "AI.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AStar.h"

#include <algorithm>


#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define AVOIDANCEPATH_DEBUG
#endif

namespace  /* unnamed */
{

	const LTFLOAT g_fGoalEpsilonSqr = 1.0f;
	const LTFLOAT g_fPruneRadiusSqr = 1500.0f*1500.0f;
	const LTFLOAT g_fDistanceEpsilon = 0.01f;

	const LTVector xz_edges[4] =
	{
		LTVector(1.0f,0.0f,1.0f),	//0
		LTVector(-1.0f,0.0f,1.0f),	//1
		LTVector(-1.0f,0.0f,-1.0f),	//2
		LTVector(1.0f,0.0f,-1.0f),	//3
	};

	LTVector FindMovingBoxMax( const LTVector & start_pos,
							   const LTVector & end_pos,
							   const LTVector & dims )
	{
		return LTVector( std::max(start_pos.x + dims.x, end_pos.x + dims.x),
						 std::max(start_pos.y + dims.y, end_pos.y + dims.y),
						 std::max(start_pos.z + dims.z, end_pos.z + dims.z) );
	}

	bool BoxIntersection( const LTVector & pos1, const LTVector & dims1, 
		                   const LTVector & pos2, const LTVector & dims2  )
	{
		_ASSERT( dims1.x >= 0.0f && dims1.y >= 0.0f && dims1.z >= 0.0f );
		_ASSERT( dims2.x >= 0.0f && dims2.y >= 0.0f && dims2.z >= 0.0f );

		return (   (float)fabs(pos1.x - pos2.x) < dims1.x + dims2.x 
				// && (float)fabs(pos1.y - pos2.y) < dims1.y + dims2.y  			   
				&& (float)fabs(pos1.z - pos2.z) < dims1.z + dims2.z  );
	}

	template< class ObstacleList >
	bool ClearMovementBox( const LTVector & start, const LTVector & end, const LTVector & dims, HOBJECT hMover,
						   const typename ObstacleList::const_iterator & exclude1, 
						   const typename ObstacleList::const_iterator & exclude2, 
						   LTBOOL bIncludeIgnorable,
						   const ObstacleList & obstacles )
	{
		// The movement_box is the larger box which circumscribes the box at its start
		// and end position.
		const LTVector movement_box_max = FindMovingBoxMax(start,end,dims);
		const LTVector movement_box_pos  = start + (end - start) * 0.5f;
		const LTVector movement_box_dims = movement_box_max - movement_box_pos;

		_ASSERT( movement_box_dims.x >= 0.0f );
		_ASSERT( movement_box_dims.y >= 0.0f );
		_ASSERT( movement_box_dims.z >= 0.0f );

		// Go through each obstacle in the potential obstacle
		// list.  If the movement box intersects with an obstacle,
		// than the we don't have clear movement.
		for( typename ObstacleList::const_iterator iter = obstacles.begin();
			 iter != obstacles.end(); ++iter )
		{
			// Exclude the ignorables, if so requested.
			if( bIncludeIgnorable || !iter->IsIgnorable() )
			{
				// Exclude any non-solid obstacles.
				if( iter->IsSolid() )
				{
					// Allow for two other exclusions (usually
					// the obstacles being avoided by the waypoints
					// which give us the start and end points).
					if( iter != exclude1 && iter != exclude2 )
					{
						// Finally, check to see if the movement box intersects
						// with this obstacle.
						if( iter->IntersectsBox(movement_box_pos,  movement_box_dims) )
						{
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	// Closest approach to a line segment.
	// pos should be relative to the start of the line segment.
	LTFLOAT LineSegmentDistSqr( const LTVector & segment, const LTVector & pos)
	{
		const LTFLOAT segment_length_sqr = segment.MagSqr();

		if( segment_length_sqr <= g_fDistanceEpsilon )
		{
			return pos.MagSqr();
		}

		const LTFLOAT t = segment.Dot(pos)/segment_length_sqr;
		
		
		if( t < 0.0f )
		{
			// The point is before the start point, so just
			// return the distance from the start point.
			return pos.MagSqr();
		}
		else if( t > 1.0f )
		{
			// The point is beyond the end point, so just
			// return the distance from the end point.
			return (pos - segment).MagSqr();
		}

		// Actually return the nearest distance from the point
		// to the line.
		return (pos - segment*t).MagSqr();
	}



	template< class ObstacleList >
	bool ClearMovementCylinder( const LTVector & start, const LTVector & end, 
								const typename ObstacleList::const_iterator & exclude1, 
								const typename ObstacleList::const_iterator & exclude2, 
								LTBOOL bIncludeIgnorable,
								const ObstacleList & obstacles )
	{
		HOBJECT hLastObstacle = LTNULL;

		LTVector vXZStart(start.x, 0.0f, start.z);
		LTVector vXZPath = end - start;
		vXZPath.y = 0.0f;

		LTFLOAT vYDiff = (float)fabs(start.y - end.y);

		// Go through each obstacle in the potential obstacle
		// list.  If the obstacle comes within avoidance radius 
		// plus obstacle's radius of the path, than the we don't 
		// have clear movement.
		for( typename ObstacleList::const_iterator iter = obstacles.begin();
			 iter != obstacles.end(); ++iter )
		{
			// Exclude the ignorables, if so requested.
			if( bIncludeIgnorable || !iter->IsIgnorable() )
			{
				// Exclude any non-solid obstacles.
				if( iter->IsSolid() )
				{
					// Allow for two other exclusions (usually
					// the obstacles being avoided by the waypoints
					// which give us the start and end points).
					if( iter != exclude1 && iter != exclude2 )
					{
						// The waypoints are added sequintially, with 4 per obstacle.
						// So just checking if the last obstacle was the same as this
						// one will exclude 3 of the waypoints that don't need to be
						// checked against!
						const HOBJECT hObstacle = iter->GetObstacle().objectHandle;
						if( !hObstacle || hObstacle != hLastObstacle )
						{
							hLastObstacle = hObstacle;


							const LTVector & vObjectPos = iter->GetObstaclePosition();
							
							// Check the height.
							const LTFLOAT fMinHeight = iter->GetObstacle().height;
							if(    (float)fabs(start.y - vObjectPos.y) < fMinHeight
								|| (float)fabs(end.y - vObjectPos.y) < fMinHeight )
							{
								// Finally, check to see if the movement box intersects
								// with this obstacle.
								const LTFLOAT  fMinRadius = iter->GetObstacle().radius;
								const LTVector vXZObjectPos(vObjectPos.x, 0.0f, vObjectPos.z);
								if( LineSegmentDistSqr(vXZPath, vXZObjectPos - vXZStart) < fMinRadius*fMinRadius )
								{
									return false;
								}
							}
						}
					}
				}
			}
		}

		return true;
	}


	class ObstacleWaypointHeuristic
	{
		const LTVector goal;

	public:

		ObstacleWaypointHeuristic(const LTVector & new_goal)
			: goal(new_goal) {}

		float operator()(const ObstacleWaypoint & w1) const
		{
			const LTFLOAT fXDiff = (w1.GetPosition().x - goal.x);
			const LTFLOAT fZDiff = (w1.GetPosition().z - goal.z);
				
			return (float)sqrt( fXDiff*fXDiff + fZDiff*fZDiff );
		}
	};

	class ObstacleWaypointCost
	{
	public:

		float operator()(const ObstacleWaypoint & w1, const ObstacleWaypoint & w2) const
		{
			const LTFLOAT fXDiff = (w1.GetPosition().x - w2.GetPosition().x);
			const LTFLOAT fZDiff = (w1.GetPosition().z - w2.GetPosition().z);
				
			return (float)sqrt( fXDiff*fXDiff + fZDiff*fZDiff );
		}
	};

	class ObstacleHandleEquals
	{
		const HOBJECT & m_hObject;
	public:
		ObstacleHandleEquals(const HOBJECT & hObject)
			: m_hObject(hObject) {}

		bool operator()(const Obstacle & obstacle)
		{
			return obstacle.objectHandle == m_hObject;
		}

		bool operator()(const ObstacleWaypoint & obstacle_waypoint)
		{
			return obstacle_waypoint.GetObstacle().objectHandle == m_hObject;
		}
	};

	LTVector diag_mult(const LTVector & diag_matrix, const LTVector & vector)
	{
		return LTVector( diag_matrix.x * vector.x, 
						 diag_matrix.y * vector.y, 
						 diag_matrix.z * vector.z );
	}

#ifdef AVOIDANCEPATH_DEBUG
	struct WayPointNetwork
	{
		typedef AvoidancePath::WayPointList WayPointList;

		const WayPointList & waypoints;
		const LTVector goal;

		const LTVector color;
		const LTVector dims;

		WayPointNetwork( const WayPointList & new_waypoints,
						 const LTVector & new_goal,
						 const LTVector & new_color = Color::White,
						 const LTVector & new_dims = LTVector(3.0f,3.0f,3.0f) )
			: waypoints(new_waypoints),
			  goal(new_goal),
			  color(new_color),
			  dims(new_dims) {}
	};

	DebugLineSystem & operator<<(DebugLineSystem & out, const WayPointNetwork & network)
	{

		for( WayPointNetwork::WayPointList::const_iterator iter = network.waypoints.begin();
			 iter != network.waypoints.end(); ++iter )
		{
			out << LineSystem::Box( iter->GetPosition(), network.dims, network.color );
			out << LineSystem::Box( iter->GetObstaclePosition(), 
				                    LTVector(iter->GetObstacle().radius,iter->GetObstacle().radius,iter->GetObstacle().radius), 
								    network.color*0.5f );
			 
/*			for( ObstacleWaypoint::NeighborList::const_iterator neighbor = iter->BeginNeighbors();
				 neighbor != iter->EndNeighbors(); ++neighbor )
			{
				out << LineSystem::Arrow( iter->GetPosition(), (*neighbor)->GetPosition(), network.color);
			}
*/		}

		 out << LineSystem::Box(network.goal + LTVector(0.0f,network.dims.y*(1.0f + 2.0f/3.0f),0.0f), network.dims*2.0f/3.0f, network.color)
			 << LineSystem::Box(network.goal + LTVector(0.0f,network.dims.y*2.0f,0.0f), network.dims/3.0f, network.color);

		return out;
	}

	class ShowPath
	{
	
		const ObstacleWaypoint & m_Path;
		const LTVector color;
		const LTVector dims;


	public:
		ShowPath( const ObstacleWaypoint & path, const LTVector & new_color = Color::White,
			      const LTVector & node_dims = LTVector(3.0f,3.0f,3.0f) )
			: m_Path(path),
			  color(new_color),
			  dims(node_dims) {}

		friend DebugLineSystem & operator<<(DebugLineSystem & out, const ShowPath & x)
		{
			const ObstacleWaypoint * pLastPoint = LTNULL;
			const ObstacleWaypoint * pCurrentPoint = &x.m_Path;

			if( pCurrentPoint )
			{
				const LTVector vPos = pCurrentPoint->GetPosition();

				out << LineSystem::Box(vPos + LTVector(0.0f,x.dims.y*(1.0f + 2.0f/3.0f),0.0f), x.dims*2.0f/3.0f, x.color)
				    << LineSystem::Box(vPos + LTVector(0.0f,x.dims.y*2.0f,0.0f), x.dims/3.0f, x.color);
			}

			while( pCurrentPoint )
			{
				out << LineSystem::Box(pCurrentPoint->GetPosition(), x.dims, x.color);

				if( pLastPoint )
				{
					out << LineSystem::Line( pCurrentPoint->GetPosition(), pLastPoint->GetPosition(), x.color);
				}

				pLastPoint = pCurrentPoint;
				pCurrentPoint = pCurrentPoint->GetPrevPathPoint();
			}

			return out;
		}

	};

#endif


} //namespace  /* unnamed */


void ObstacleWaypoint::UpdateContainedByVolumes(const LTVector & dims)
{ 
	isInVolumes = ( LTNULL != g_pAIVolumeMgr->FindContainingVolume( GetPosition(), dims ) ); 
}




AvoidancePath::AvoidancePath()
	: goal(0.0f,0.0f,0.0f),
	  hasGoal(false),
	  isStuck(false)
{
}


void AvoidancePath::SetGoal(const LTVector & new_goal, const LTSmartLink & hAI)
{
	if( !hasGoal || (goal - new_goal).MagSqr() > g_fGoalEpsilonSqr  )  // had problems with single bit errors in floating point compare
	{
		pathHeap.clear();
		isStuck = false;
		hasGoal = true;

		goal = new_goal;

		WayPointList::iterator iter = waypoints.begin();

		// The first entry in waypoints is always our starting point.
		if( iter != waypoints.end() )
		{
			*iter = ObstacleWaypoint( Obstacle(hAI, LTFALSE), LTVector(0.0f,0.0f,0.0f), false);
			iter->UpdatePosition();

			++iter;
		}
		else
		{
			// We need to add the element.
			waypoints.push_back( ObstacleWaypoint(Obstacle(hAI, LTFALSE), LTVector(0.0f,0.0f,0.0f), false) );
			waypoints.back().UpdatePosition();

			// be sure the next entry is added as well.
			iter = waypoints.end();
		}


		// The goal needs to be in our list also.  ObstacleWaypoint is set-up to 
		// ignore a null handle.
		if( iter != waypoints.end() )
		{
			*iter = ObstacleWaypoint(Obstacle(LTNULL,LTFALSE), goal, false, true);
		}
		else
		{
			waypoints.push_back( ObstacleWaypoint( Obstacle(LTNULL,LTFALSE), goal, false, true) );
		}

#ifdef AVOIDANCEPATH_DEBUG
		LineSystem::GetSystem(this,"Obstacles") << LineSystem::Clear();
#endif

	}
}


void AvoidancePath::CheapUpdate()
{
	_ASSERT( !waypoints.empty() );
	if( waypoints.empty() ) return;

	// Update the position of each node.
	// This does not update if the node is contained by a volume.  This might cause
	// AI's to fall off cliffs, as they may move to a point that was inside the volumes
	// and that point may move (as its object moves closer to the edge) outside the volumes.
	for( WayPointList::iterator iter = waypoints.begin(); iter != waypoints.end(); ++iter)
	{
		iter->UpdatePosition();
	}
}


void AvoidancePath::Update(HOBJECT hMover, const LTVector & my_dims, LTBOOL bIncludeIgnorable)
{
	_ASSERT( !waypoints.empty() );
	if( waypoints.empty() ) return;

	LTVector vMoverPos;
	g_pLTServer->GetObjectPos(hMover,&vMoverPos);

	// Update position, remove neighbor info for each way point, 
	//    and remove point if not in an AIVolume.

	const WayPointList::iterator start_iter = waypoints.begin();
	const WayPointList::iterator goal_iter = ++waypoints.begin();
	{for( WayPointList::iterator iter = waypoints.begin(); iter != waypoints.end(); )
	{
		iter->ClearNeighbors();
		iter->UpdatePosition();

		// See if we should remove the obstacle.
		bool bRemoveObstacle = false;
		if(    iter != start_iter 
			&& iter != goal_iter )
		{
			// Remove the waypoint if it does not have an object associated with it,
			// (the object was removed).
			if( !(iter->GetObstacle().objectHandle.GetHOBJECT()) )
			{
				bRemoveObstacle = true;
			}
			else
			{
				
				bRemoveObstacle = !(FLAG_SOLID & g_pLTServer->GetObjectFlags(iter->GetObstacle().objectHandle)); 
			}

			// Remove the waypoint if the the position is outside our pruning radius.
			if( !bRemoveObstacle && (iter->GetPosition() - vMoverPos).MagSqr() > g_fPruneRadiusSqr )
			{
				bRemoveObstacle = true;
			}
		}

		// Either remove the obstacle, or update it!
		if( bRemoveObstacle )
		{
			WayPointList::iterator remove_iter = iter++;
			waypoints.erase(remove_iter);
		}
		else
		{
			// Set the way-points of a solid obstacle sitting on the goal as our new goal,
			// so that we at least get as close as possible.
			if( iter->IsSolid() )
			{
				iter->SetIsGoal( iter->GetObstacle().CoversPoint(goal) );

				// Now that we may have another goal point, check to see if
				// anyone else is sitting on us.
				if( iter->IsGoal() )
				{
					const LTVector vNewGoalPos = iter->GetPosition();

					{for( WayPointList::iterator others = goal_iter; others != waypoints.end(); ++others)
					{
						if( others != iter && others->IsSolid() && !others->IsGoal() )
						{
							others->SetIsGoal( others->GetObstacle().CoversPoint(vNewGoalPos) );
						}
					}}
				}
			}

			// Determine if we are in the volume system.
			if( iter != start_iter && iter != goal_iter )
				iter->UpdateContainedByVolumes( LTVector(my_dims.x,0.0f,my_dims.z) );

			++iter;
		}
	}}


	// Update neighbor list of each way point.  If a point is outside all volumes (and is not the start or goal points),
	// it will not have any neighbors.  
	{for( WayPointList::iterator iter = waypoints.begin(); iter != waypoints.end(); ++iter)
	{
		if( bIncludeIgnorable || !iter->IsIgnorable() )
		{
			if(    iter == start_iter 
				|| iter == goal_iter
				|| iter->IsContainedByVolumes()  )
			{
				// Start iterating across the rest of the list.  Be careful not to go past
				// the end of list!
				WayPointList::iterator potential_neighbor = iter;
				if( iter != waypoints.end() ) ++potential_neighbor;

				for(  ; potential_neighbor != waypoints.end(); ++potential_neighbor )
				{
					// Only connect to a node if it is in the volume system and the AI can
					// can move from current position (at iter) to the potential_neighbor's position without
					// colliding with any obstacles in the waypoint list.

					// We still want to connect to ignorable waypoints, because we
					// may need to access one of them.

					if(    potential_neighbor == start_iter 
						|| potential_neighbor == goal_iter
						|| potential_neighbor->IsContainedByVolumes()  )
						
					{
//						if( ClearMovementBox( iter->GetPosition(), potential_neighbor->GetPosition(), 
//											   my_dims, hMover, iter, potential_neighbor, bIncludeIgnorable, waypoints ) )

						if( ClearMovementCylinder( iter->GetPosition(), potential_neighbor->GetPosition(),
							                       iter, potential_neighbor, bIncludeIgnorable, waypoints ) )
						{
							// Make the connection two-way.
							iter->AddNeighbor(&(*potential_neighbor));
							potential_neighbor->AddNeighbor(&(*iter));
						}
					}


				} //for(  ; potential_neighbor != waypoints.end(); ++potential_neighbor )

			}

		} // if( bIncludeIgnorable || !iter->IsIgnorable() )

	}} //{for( WayPointList::iterator iter = waypoints.begin(); iter != waypoints.end(); ++iter)

}

// Returns true if avoiding an obstacle (or stuck).  False if the goal_ptr is a goal.
// Note that if an obstacle is sitting on a goal, the goal_ptr is a goal, but not the original goal. 
// Hence, AvoidancePath will say they are reaching the goal if it is a way-point of an object
// sitting on the goal.
bool AvoidancePath::GetWaypoint( LTVector * goal_ptr, LTVector * next_goal_ptr,
								 const LTVector & my_pos, const LTVector & my_dims, 
								 const LTVector & my_up )
{
	_ASSERT( !waypoints.empty() );

	isStuck = false;

	if( waypoints.empty() )
		return true;


#ifdef AVOIDANCEPATH_DEBUG
	LineSystem::GetSystem(this,"ShowPath").SetMaxLines(500);
	LineSystem::GetSystem(this,"ShowPath") << LineSystem::Clear();
#endif

#ifdef AVOIDANCEPATH_DEBUG
	LineSystem::GetSystem(this,"ShowPath") << WayPointNetwork(waypoints, *goal_ptr, Color::Purple);
#endif

	{for( WayPointList::iterator iter = waypoints.begin(); iter != waypoints.end(); ++iter )
	{
		iter->ResetPathing();
	}}

	const LTFLOAT fMaxSearchCost = 50000.0f*50000.0f;
	ObstacleWaypoint * path 
		= AStar::FindPath(waypoints.front(),ObstacleWaypointCost(),ObstacleWaypointHeuristic(*goal_ptr), pathHeap, fMaxSearchCost);

	if( path && path->IsGoal() )
	{
		// We have found our path.
#ifdef AVOIDANCEPATH_DEBUG
		LineSystem::GetSystem(this,"ShowPath") 
				<< LineSystem::Box( path->GetPosition(), LTVector(3.0f,3.0f,3.0f), Color::White )
				<< LineSystem::Line( *goal_ptr, path->GetPosition() );
#endif
		
		// Walk back along the path until we hit the point before our own start point.
		// (The check for non-null previous point is just an error check, it should never happen!).
		ObstacleWaypoint * waypoint = path;
		ObstacleWaypoint * next_waypoint = LTNULL;

		while(   waypoint->GetPrevPathPoint() != &waypoints.front() 
			  && waypoint->GetPrevPathPoint() != 0 )
		{
#ifdef AVOIDANCEPATH_DEBUG
//			LineSystem::GetSystem(this,"ShowPath") 
//				<< LineSystem::Line( waypoint->GetPosition(), waypoint->GetPrevPathPoint()->GetPosition(), Color::White );
#endif
			next_waypoint = waypoint;
			waypoint = waypoint->GetPrevPathPoint();

#ifdef AVOIDANCEPATH_DEBUG
//			LineSystem::GetSystem(this,"ShowPath") 
//				<< LineSystem::Box( waypoint->GetPosition(), LTVector(3.0f,3.0f,3.0f), Color::White );
#endif
		}


		_ASSERT( waypoint->GetPrevPathPoint() != 0 );

		// Get the goal_ptr and project it to be level with the 
		// character if it is a new position.
		const LTVector old_goal = *goal_ptr;
		*goal_ptr = waypoint->GetPosition();
		if( *goal_ptr != old_goal ) 
			*goal_ptr += my_up*( my_up.Dot(my_pos - *goal_ptr) );

		if( next_goal_ptr && next_waypoint )
		{
			// Do the same for next_goal_ptr.
			const LTVector old_next_goal = *next_goal_ptr;
			*next_goal_ptr = next_waypoint->GetPosition();
			if( old_next_goal != *next_goal_ptr )
				*next_goal_ptr += my_up*( my_up.Dot(my_pos - *next_goal_ptr) );
		}

		return (waypoint != path);

	} //if( current_point )

		
	// If we made it here, we couldn't path to goal_ptr.  We're stuck!
	// So we'll just move to the closest node we can.

#ifdef AVOIDANCEPATH_DEBUG
//	LineSystem::GetSystem(this,"ShowPath") << WayPointNetwork(waypoints, goal_ptr, Color::Yellow);
	
	if(path)
		LineSystem::GetSystem(this,"ShowPath") << ShowPath(*path,  Color::Red);

	g_pLTServer->CPrint("AI can't avoidance path, moving to nearest node.");
#endif

	// Only be stuck if we aren't clipping into something.
//	if( waypoints.front().BeginNeighbors() != waypoints.front().EndNeighbors() )
//	{
//		isStuck = true;
//#ifdef AVOIDANCEPATH_DEBUG
//		g_pLTServer->CPrint("AI is stuck.");
//#endif
//	}

	// check this with a box intersection instead of the "empty list" check
	const LTVector vStartPos = waypoints.front().GetObstacle().position;
	LTBOOL bIntersect = LTFALSE;

	const ObstacleWaypoint * pNearestWaypoint = LTNULL;
	LTFLOAT fNearestDistSqr = FLT_MAX;
	// this is a bizarre way to safely get to the second element of the list.
	WayPointList::const_iterator goal_iter = waypoints.begin();  
	if( goal_iter != waypoints.end() && ++goal_iter != waypoints.end() )
	{
		for( WayPointList::const_iterator iter = goal_iter; iter != waypoints.end() && !bIntersect; ++iter )
		{
			if (!iter->IsSolid() || !iter->IsContainedByVolumes() )
				continue;

			if ( iter->GetObstacle().CoversPoint(vStartPos) )
				bIntersect = LTTRUE;
		}
	}

	// If the boxes intersect then the AI is clipping with the obstacle.  When this
	// happens, we don't want the AI to be "stuck" or he will never be able to move.
	// If false, then the path is blocked / unable to be completed, so the AI is stuck.
	if (!bIntersect)
	{
		isStuck = true;
#ifdef AVOIDANCEPATH_DEBUG
		g_pLTServer->CPrint("AI is stuck.");
#endif
	}
	else
	{
		// Our start point isn't connected to any other point. 
		// This can only happen if we are clipping into something.
		// Try to get out of this bad, bad situation by moving
		// to the nearest waypoint.
			
		// Find the closest node.  The first node is our own position, so we need to exclude that one.
		const ObstacleWaypoint * pNearestWaypoint = LTNULL;
		LTFLOAT fNearestDistSqr = FLT_MAX;
		// this is a bizarre way to safely get to the second element of the list.
		WayPointList::const_iterator goal_iter = waypoints.begin();  
		if( goal_iter != waypoints.end() && ++goal_iter != waypoints.end() )
		{
			for( WayPointList::const_iterator iter = goal_iter; iter != waypoints.end(); ++iter )
			{
				// Only consider waypoints inside volumes, we don't want to fall off a cliff!
				if(    iter == goal_iter 
					|| iter->IsContainedByVolumes() )
				{
					const LTFLOAT fCurrentDistSqr = (iter->GetPosition() - my_pos).MagSqr();
					if(    !pNearestWaypoint 
						|| fCurrentDistSqr < fNearestDistSqr )
					{
						pNearestWaypoint = &(*iter);
						fNearestDistSqr = fCurrentDistSqr;
					}
				}
			}
		}

		// So did we find a node?  If so, go to that node, otherwise be stuck!
		if( pNearestWaypoint )
		{
			if( goal_ptr )
			{
				const LTVector old_goal = *goal_ptr;
				*goal_ptr = pNearestWaypoint->GetPosition();
				if( *goal_ptr != old_goal ) 
					*goal_ptr += my_up*( my_up.Dot(my_pos - *goal_ptr) );
			}
		}
		else
		{
			// I don't think it is possible to reach this point, you should always
			// have at least the goal node to go towards.
			isStuck = true;
#ifdef AVOIDANCEPATH_DEBUG
			g_pLTServer->CPrint("AI is stuck.");
#endif
		}
	}

	return true;
} //bool AvoidancePath::GetWaypoint( .... )


bool AvoidancePath::HasObstacle(const Obstacle & obstacle) const
{
	return   waypoints.end() != std::find_if( waypoints.begin(), waypoints.end(), ObstacleHandleEquals(obstacle.objectHandle) );
}



void AvoidancePath::AddObstacle( const Obstacle & obstacle_src, LTFLOAT my_radius,
								 LTVector my_right, const LTVector & my_up, LTVector my_forward )
{
	// Just ignore it if we already have that obstacle.
	if( HasObstacle(obstacle_src) ) return;

#ifdef AVOIDANCEPATH_DEBUG
	LineSystem::GetSystem(this,"Obstacles") << LineSystem::Box( obstacle_src.position, LTVector(obstacle_src.radius,obstacle_src.radius,obstacle_src.radius), Color::Green );
#endif

	// If the up vector is not along the y-axis, just use the world's x-z axis for
	// right and forward.
	if( (float)fabs(my_up.y - 1.0f) > 0.01f )
	{
		my_right = LTVector(1.0f,0.0f,0.0f);
		my_forward = LTVector(0.0f,0.0f,1.0f);
	}

	// Add the new path points.
	// The points are placed randomly out so that two AI's will not 
	// just sit at each other's avoidance point and wait for their partner
	// to move.
	const LTFLOAT fMinScale = 1.05f;
	const LTFLOAT fMaxScale = 1.15f;

	waypoints.push_back( ObstacleWaypoint( obstacle_src, (my_right + my_forward)*obstacle_src.radius*GetRandom(fMinScale, fMaxScale) ) );
	waypoints.push_back( ObstacleWaypoint( obstacle_src, (my_right - my_forward)*obstacle_src.radius*GetRandom(fMinScale, fMaxScale) ) );

	waypoints.push_back( ObstacleWaypoint( obstacle_src, (-my_right + my_forward)*obstacle_src.radius*GetRandom(fMinScale, fMaxScale) ) );
	waypoints.push_back( ObstacleWaypoint( obstacle_src, (-my_right - my_forward)*obstacle_src.radius*GetRandom(fMinScale, fMaxScale) ) );

}


ILTMessage & operator<<(ILTMessage & out, /*const*/ AvoidancePath & x)
{
	out << x.goal;
	out << x.hasGoal;
	out << x.isStuck;

	out.WriteWord(x.waypoints.size());
	for(AvoidancePath::WayPointList::iterator iter = x.waypoints.begin();
	    iter != x.waypoints.end(); ++iter )
	{
		out << *iter;
	}

	// pathHeap is temporary storage, it doesn't need to be saved.

	return out;
}

ILTMessage & operator>>(ILTMessage & in, AvoidancePath & x)
{
	in >> x.goal;
	in >> x.hasGoal;
	in >> x.isStuck;

	const int waypoint_size = in.ReadWord();
	for(int i = 0; i < waypoint_size; ++i)
	{
		ObstacleWaypoint temp(Obstacle(LTNULL,LTFALSE),LTVector(0.0f,0.0f,0.0f));
		in >> temp;
		x.waypoints.push_back(temp);
	}
	// The path was not saved. Currently re-pathing occurs every update
	// so there isn't a problem.

	// pathHeap is temporary storage, it isn't saved.

	return in;
}


ILTMessage & operator<<(ILTMessage & out, /*const*/ ObstacleWaypoint & x)
{
	out << x.obstacle;
	out << x.offset;

	out << x.isGoal;
	out << x.isSolid;
	out << x.isInVolumes;


	// Pathing information is not saved :

	// neighbors
	// pathDist;
	// heuristic;
	// prevPathPoint;
	// inClosed;

	return out;
}

ILTMessage & operator>>(ILTMessage & in, ObstacleWaypoint & x)
{
	in >> x.obstacle;
	in >> x.offset;

	in >> x.isGoal;
	in >> x.isSolid;
	in >> x.isInVolumes;


	// Pathing information was not saved. :

	// neighbors
	// pathDist;
	// heuristic;
	// prevPathPoint;
	// inClosed;

	return in;
}

namespace /* unnamed */
{
	

#ifdef IMPLEMENT_SHOWPATH
template< class PathList >
void ShowPath(const PathList & path, LTVector last_pos, const LTVector & goal, DebugLineSystem & debugLineSystem, const LTVector & color)
{

	for( PathList::const_reverse_iterator iter = path.rbegin(); iter != path.rend(); ++iter )
	{
		debugLineSystem  << LineSystem::Line( last_pos, iter->GetPosition(), color )
						 << LineSystem::Box( iter->GetPosition(), LTVector(3.0f,3.0f,3.0f), color );
		last_pos = iter->GetPosition();
	}

	debugLineSystem << LineSystem::Line( last_pos, goal, color );
}
#endif

}  //namespace /* unnamed */
