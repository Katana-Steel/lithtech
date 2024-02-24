// ----------------------------------------------------------------------- //
//
// MODULE  : ObstacleMgr.cpp
//
// PURPOSE : 
//
// CREATED : 4/17/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObstacleMgr.h"
#include "AI.h"
#include "PlayerObj.h"
#include "CharacterHitBox.h"
#include "AIPath.h"
#include "Prop.h"
#include "Explosion.h"
#include "AvoidancePath.h"
#include "BodyProp.h"
#include "CharacterFuncs.h"

#include <algorithm>


#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#include "AIUtils.h"
//#define OBSTACLE_DEBUG
//#define DETECTION_DEBUG
#endif




namespace /*unnamed*/
{
	const LTFLOAT g_fIntersectionEpsilon = 0.05f;
	const LTFLOAT  g_fFindObjectRadius = 1000.0f;

	LTVector FindMovingBoxMax( const LTVector & start_pos,
							   const LTVector & end_pos,
							   const LTVector & dims )
	{
		return LTVector( std::max(start_pos.x + dims.x, end_pos.x + dims.x),
						 std::max(start_pos.y + dims.y, end_pos.y + dims.y),
						 std::max(start_pos.z + dims.z, end_pos.z + dims.z) );
	}

	LTVector FindMovingBoxMin( const LTVector & start_pos,
							   const LTVector & end_pos,
							   const LTVector & dims )
	{
		return LTVector( std::min(start_pos.x - dims.x, end_pos.x - dims.x),
						 std::min(start_pos.y - dims.y, end_pos.y - dims.y),
						 std::min(start_pos.z - dims.z, end_pos.z - dims.z) );
	}

	bool BoxIntersection( const LTVector & pos1, const LTVector & dims1, 
		                   const LTVector & pos2, const LTVector & dims2  )
	{
		_ASSERT( dims1.x >= 0.0f && dims1.y >= 0.0f && dims1.z >= 0.0f );
		_ASSERT( dims2.x >= 0.0f && dims2.y >= 0.0f && dims2.z >= 0.0f );

		return (   (float)fabs(pos1.x - pos2.x) < dims1.x + dims2.x - g_fIntersectionEpsilon
				// && (float)fabs(pos1.y - pos2.y) < dims1.y + dims2.y - g_fIntersectionEpsilon 			   
				&& (float)fabs(pos1.z - pos2.z) < dims1.z + dims2.z - g_fIntersectionEpsilon );
	}

	LTVector diag_mult(const LTVector & diag_matrix, const LTVector & vector)
	{
		return LTVector( diag_matrix.x * vector.x, 
						 diag_matrix.y * vector.y, 
						 diag_matrix.z * vector.z );
	}


	void FillPotentialObstacleList( ObstacleMgr::ObstacleList * potentials_ptr,
								    const LTVector & goal, const LTVector & position, const LTVector & dims, 
									HOBJECT handle_to_self, LTBOOL bAvoidAcidPools);

	template< class ContainerType >
	bool ClearLine( const LTVector & start,
					const LTVector & end,
					const LTVector & dimensions,
					const ContainerType & obstacle_list,
					const Obstacle ** obstacle_ptr = LTNULL)
	{
#ifdef DETECTION_DEBUG
		LineSystem::GetSystem("Detection") << LineSystem::Clear()
									   << LineSystem::Box(start, LTVector(5.0f,5.0f,5.0f), Color::Red)
									   << LineSystem::Box(end, LTVector(5.0f,5.0f,5.0f), Color::Red);

#endif
		LTVector goal_dir = end - start;
		goal_dir.Norm();
		const LTVector path_incr = diag_mult(dimensions,goal_dir);
		const float path_incr_mag_sqr = path_incr.MagSqr();
		const float max_steps = path_incr_mag_sqr != 0.0f 
						? (float)sqrt((end - start).MagSqr() / path_incr.MagSqr())
						: 0.0f;
			                                              

		float num_steps = 0.0f;
		LTVector current_path_point = start;

		// Walk along our projected path, checking for a collision against all items in the
		// potential colliders list.
		while( num_steps < max_steps )
		{
#ifdef DETECTION_DEBUG
			LineSystem::GetSystem("Detection") << LineSystem::Box( current_path_point, dimensions, Color::Blue );
#endif
			for( typename ContainerType::const_iterator iter = obstacle_list.begin();
				 iter != obstacle_list.end(); ++iter )
			{
				if( iter->IntersectsBox(current_path_point,dimensions) )
				{
					if( obstacle_ptr ) *obstacle_ptr = &iter->GetObstacle();
					return false;
				}
			}
			
			current_path_point += path_incr;
			++num_steps;
		}

		return true;
	}

	#if defined(OBSTACLE_DEBUG)
	#define IMPLEMENT_SHOWPATH
	#endif

	#ifdef IMPLEMENT_SHOWPATH					
	template< class PathList >
	void ShowPath(const PathList & path, LTVector last_pos, const LTVector & goal, DebugLineSystem & debugLineSystem, const LTVector & color);
	#endif

};  //namespace /*unnamed*/


ObstacleMgr::ObstacleMgr()
		: m_pAvoidancePath(new AvoidancePath()),
		  m_fPathingFrequency(0.0f),
		  // m_tmrPathingFrequency
		  m_bAvoidIgnorable(LTTRUE)
		  // potentialObstacleList
{
}


ObstacleMgr::~ObstacleMgr()
{
	delete m_pAvoidancePath;
	m_pAvoidancePath = LTNULL;
}

void ObstacleMgr::SetGoal( const LTVector & goal, const CharacterMovement * pMovement )
{
	_ASSERT(pMovement);
	if( !pMovement ) return;

	const LTVector my_up = pMovement->GetUp();
	const LTVector my_pos = pMovement->GetPos();

	m_pAvoidancePath->SetGoal(goal, LTSmartLink(pMovement->GetObject()));
	m_tmrPathingFrequency.Stop();
}

bool ObstacleMgr::IsStuck() const
{
	return m_pAvoidancePath->IsStuck();
}

// Returns true if the goal_ptr is set to a obstacle avoidance waypoint.
bool ObstacleMgr::Update( LTVector * goal_ptr, LTVector * next_goal_ptr, const CharacterMovement * pMovement )
{
	_ASSERT( goal_ptr );
	if( !goal_ptr ) return false;

	const LTVector & my_right = pMovement->GetRight();
	const LTVector & my_up = pMovement->GetUp();
	const LTVector & my_forward = pMovement->GetForward();
	const LTVector & my_pos = pMovement->GetPos();
	const LTVector & my_dims = pMovement->GetObjectDims();

	if( m_tmrPathingFrequency.Stopped() )
	{
		// Start the timer for the next full update.
		m_tmrPathingFrequency.Start(m_fPathingFrequency);

		// Find all objects around us. 
		potentialObstacleList.clear();
		FillPotentialObstacleList(&potentialObstacleList,*goal_ptr,my_pos,my_dims, pMovement->GetObject(), IsAlien(pMovement->GetCharacterButes()) );


		// 
		// Find if any potential colliders really do hit us and set-up a path around
		// the first one that does.
		//

/*		const Obstacle * current_obstacle = 0;

		if( !ClearLine( my_pos, *goal_ptr, my_dims, potentialObstacleList, &current_obstacle) )
		{
			_ASSERT( current_obstacle );

			if( current_obstacle )
			{
				m_pAvoidancePath->AddObstacle(*current_obstacle,pMovement->GetObject(),my_dims, my_up);
			}

		}
*/

		//
		// Add all the obstacles to our avoidance engine.
		//
		const LTFLOAT my_radius = (float)sqrt(my_dims.x*my_dims.x + my_dims.z*my_dims.z);
		for( ObstacleList::const_iterator iter = potentialObstacleList.begin(); iter != potentialObstacleList.end(); ++iter )
		{
			m_pAvoidancePath->AddObstacle(*iter, my_radius, my_right, my_up, my_forward );
		}


		// Okay, now re-build the node connection list (even if we didn't 
		// find any new obstacles, the old ones may have broken or gained
		// new connections because of obstacle movement).
		m_pAvoidancePath->Update(pMovement->GetObject(), my_dims, m_bAvoidIgnorable);

	}
	else
	{
		// Just adjust our pathing points to move along with the
		// obstacles they are attached to.
		m_pAvoidancePath->CheapUpdate();
	}

	// Store the old values

	// This does a full re-pathing.  Don't worry, A* is cheap.
	bool avoiding_obstacle = m_pAvoidancePath->GetWaypoint(goal_ptr, next_goal_ptr, my_pos, my_dims, my_up );

#ifdef OBSTACLE_DEBUG
	LineSystem::GetSystem(pMovement->GetObject(),"ObstacleDebug")
		<< LineSystem::Box(*goal_ptr, LTVector(3.0f,3.0f,3.0f),Color::Blue );
#endif

	if( m_pAvoidancePath->IsStuck() )
	{
#ifdef OBSTACLE_DEBUG
		LineSystem::GetSystem(pMovement->GetObject(), "ObstacleDebug") 
			<< LineSystem::Clear() << LineSystem::Box(my_pos,my_dims,Color::Red );
#endif //OBSTACLE_DEBUG
	}

	return avoiding_obstacle;
} //void ObstacleMgr::Update( LTVector * goal_ptr )


ILTMessage & operator<<(ILTMessage & out, /*const*/ ObstacleMgr & x)
{
	out << *x.m_pAvoidancePath;

	out << x.m_fPathingFrequency;
	out << x.m_tmrPathingFrequency;

	out << x.m_bAvoidIgnorable;

	// potentialObstacleList is temporary storage, no need to save it.

	return out;
}

ILTMessage & operator>>(ILTMessage & in, ObstacleMgr & x)
{
	in >> *x.m_pAvoidancePath;

	in >> x.m_fPathingFrequency;
	in >> x.m_tmrPathingFrequency;

	in >> x.m_bAvoidIgnorable;

	// potentialObstacleList is temporary storage, no need to save it.
	
	return in;
}

ILTMessage & operator<<(ILTMessage & out, /*const*/ Obstacle & x)
{	
	out.WriteObject(x.objectHandle);
	out << x.position;
	out << x.radius;
	out << x.height;
	out << x.ignorable;

	return out;
}

ILTMessage & operator>>(ILTMessage & in, Obstacle & x)
{	
	x.objectHandle = in.ReadObject();

	in >> x.position;
	in >> x.radius;
	in >> x.height;
	in >> x.ignorable;

	return in;
}

namespace /* unnamed */
{


void FillPotentialObstacleList(ObstacleMgr::ObstacleList * potentials_ptr,
							   const LTVector & goal, const LTVector & position, const LTVector & dims,
							   HOBJECT self_handle, LTBOOL bAvoidAcidPools )
{
#ifdef OBSTACLE_DEBUG
	LineSystem::GetSystem("PotentialObstacles") << LineSystem::Clear();
#endif

	ObjectList * pObstacleList = g_pServerDE->FindObjectsTouchingSphere( const_cast<LTVector*>(&position), g_fFindObjectRadius );
	
	// Add any new objects to the list of potential collisions.
	_ASSERT( pObstacleList->m_nInList != 0 );
	_ASSERT( pObstacleList->m_pFirstLink->m_pNext != pObstacleList->m_pFirstLink );


	// The movement_box is the larger box which circumscribes the box at its start
	// and end position.
	const LTVector movement_box_max = FindMovingBoxMax(position,goal,dims);
	const LTVector movement_box_pos  = position + (goal - position) / 2.0f;
	const LTVector movement_box_dims = movement_box_max - movement_box_pos;

	const LTFLOAT fSelfRadius = (float)sqrt(dims.x*dims.x + dims.z*dims.z);
	const LTFLOAT fSelfHeight = dims.y;

#ifdef OBSTACLE_DEBUG
	LineSystem::GetSystem("PotentialObstacles") << LineSystem::Box(movement_box_pos,movement_box_dims,Color::Purple);
#endif

	for( ObjectLink * pIter = pObstacleList->m_pFirstLink; pIter; pIter = pIter->m_pNext)
	{
		if(    pIter->m_hObject != self_handle 
			&& (g_pServerDE->GetObjectFlags(pIter->m_hObject) & FLAG_SOLID) )
		{
			//
			// Determine if we want to avoid this obstacle.
			//
			LTBOOL bAvoidObstacle = LTFALSE;
			LTBOOL bIgnorable = LTFALSE;

			if( const CCharacter * const pChar = dynamic_cast<const CCharacter*>( g_pServerDE->HandleToObject(pIter->m_hObject) ) )
			{
				// Try to avoid, but can ignore, characters.
				bAvoidObstacle = LTTRUE;
				bIgnorable = LTTRUE;

				if( dynamic_cast<const CPlayerObj*>( pChar ) )
				{
					// Can't ignore the player!
					bIgnorable = LTFALSE;
				}
			}
			else if( const Prop * const pProp = dynamic_cast<const Prop*>( g_pServerDE->HandleToObject(pIter->m_hObject) ) )
			{
				// Must avoid props.
				bAvoidObstacle = LTTRUE;
				bIgnorable = LTFALSE;

				if( const BodyProp * const pBodyProp = dynamic_cast<const BodyProp*>( pProp ) )
				{
					// Can ignore body props.
					bIgnorable = LTTRUE;

					if( !bAvoidAcidPools || !pBodyProp->HasAcidPool() )
					{
						// Don't avoid non-acid pool body props.
						bAvoidObstacle = LTTRUE;
					}
					else
					{
						// The body prop has an acid pool, so try to avoid it.
						bAvoidObstacle = LTTRUE;
					}
				}
			}
			else if( const Explosion * const pExplosion = dynamic_cast<const Explosion*>( g_pServerDE->HandleToObject(pIter->m_hObject) ) )
			{
				// Avoid explosions!
				bAvoidObstacle = LTTRUE;
				bIgnorable = LTFALSE;
			}

			// If we do want to avoid this obstacle, check to see if we really need to.
			if( bAvoidObstacle ) 
			{
				LTVector vDims,vPos;
				g_pServerDE->GetObjectDims( pIter->m_hObject,&vDims );
				g_pServerDE->GetObjectPos(  pIter->m_hObject, &vPos );

#ifdef OBSTACLE_DEBUG
//			g_pServerDE->CPrint("Object %s has dims %s.",
//				g_pServerDE->GetObjectName(pIter->m_hObject),
//				VectorToString(vDims).c_str() );
#endif

				// If the obstacle does intersects our movement_box put it in potential obstacles.
				if( BoxIntersection( movement_box_pos,  movement_box_dims,
									 vPos, vDims )   )
				{
					// Store the new obstacle.
					const LTFLOAT fObstacleRadius = (float)sqrt(vDims.x*vDims.x + vDims.z*vDims.z);
					const LTFLOAT fObstacleHeight = vDims.y;
					potentials_ptr->push_back( Obstacle(pIter->m_hObject,vPos,fObstacleRadius + fSelfRadius, fObstacleHeight + fSelfHeight, bIgnorable) );
				}

			} // if(    pPlayerObj || .... )

		} //if(    pIter->m_hObject != self_handle && .... )
	}

	// Tell the engine to clean its list up.
	g_pServerDE->RelinquishList(pObstacleList);
	pObstacleList = LTNULL;
} //void FillPotentialObstacleList(.........)

}; //namespace /* unnamed */


