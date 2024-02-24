// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyFollowPath.cpp
//
// PURPOSE : 
//
// CREATED : 6.20.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIStrategyFollowPath.h"

#include "AI.h"
#include "AINode.h"
#include "AINodeMgr.h"
#include "AIVolume.h"
#include "AIPath.h"
#include "AIPathMgr.h"
#include "Door.h"
#include "DebugLineSystem.h"
#include "AIUtils.h"
#include "CVarTrack.h"
#include "AINodeVolume.h"
#include "AIActionMisc.h"
#include "AIActionMoveTo.h"
#include "AIVolumeMgr.h"

#ifdef _DEBUG
//#define FOLLOW_PATH_DEBUG
#endif


namespace /* unnamed */
{

#ifndef _FINAL
	class ShowPath
	{
	
		const CAIPath & m_Path;
		const LTVector color;
		const LTVector dims;


	public:
		ShowPath( CAIPath & path, const LTVector & new_color = Color::White,
			      const LTVector & node_dims = LTVector(3.0f,3.0f,3.0f) )
			: m_Path(path),
			  color(new_color),
			  dims(node_dims) {}

		friend DebugLineSystem & operator<<(DebugLineSystem & out, const ShowPath & x)
		{
			for( CAIPath::const_WaypointIterator iter = x.m_Path.BeginWaypoints();
			     iter != x.m_Path.EndWaypoints(); ++iter )
			{
				if(    iter->GetInstruction() == CAIPathWaypoint::eInstructionMoveTo 
					|| iter->GetInstruction() == CAIPathWaypoint::eInstructionLadderTo
					|| iter->GetInstruction() == CAIPathWaypoint::eInstructionWallWalkTo )
				{
					out << LineSystem::Box(iter->GetArgumentPosition(), x.dims, x.color);
				 
					if( iter+1 != x.m_Path.EndWaypoints()  )
					{
						if(    (iter+1)->GetInstruction() == CAIPathWaypoint::eInstructionMoveTo 
							|| (iter+1)->GetInstruction() == CAIPathWaypoint::eInstructionLadderTo 
							|| (iter+1)->GetInstruction() == CAIPathWaypoint::eInstructionWallWalkTo )
						{
							out << LineSystem::Arrow( iter->GetArgumentPosition(), (iter+1)->GetArgumentPosition(), x.color);
						}
						else if(  iter+2 != x.m_Path.EndWaypoints()  )
						{
							if(    (iter+2)->GetInstruction() == CAIPathWaypoint::eInstructionMoveTo 
								|| (iter+2)->GetInstruction() == CAIPathWaypoint::eInstructionLadderTo 
								|| (iter+2)->GetInstruction() == CAIPathWaypoint::eInstructionWallWalkTo )
							{
								out << LineSystem::Arrow( iter->GetArgumentPosition(), (iter+2)->GetArgumentPosition(), x.color);
							}

						}
					}
				}
			}

			return out;
		}

	};
#endif

#ifndef _FINAL
	bool ShouldShowPath(HOBJECT hObject)
	{
		ASSERT( g_pLTServer );

#ifdef FOLLOW_PATH_DEBUG
		static CVarTrack vtShowPath(g_pLTServer,"ShowAIPath",1.0f);
#else
		static CVarTrack vtShowPath(g_pLTServer,"ShowAIPath",0.0f);
#endif

		return vtShowPath.GetFloat() > 0.0f && ShouldShow(hObject);
	}
#endif

	ILTMessage & operator<<(ILTMessage & out, CAIStrategyFollowPath::PathSetType & x )
	{
		out.WriteByte(x);
		return out;
	}

	ILTMessage & operator>>(ILTMessage & in, CAIStrategyFollowPath::PathSetType & x )
	{
		x = CAIStrategyFollowPath::PathSetType(in.ReadByte());
		return in;
	}

} //namespace /*unnamed


CAIStrategyFollowPath::CAIStrategyFollowPath(CAI * pAI)
	: CAIStrategy(pAI),
	  m_bExactMovement(LTFALSE),
	  m_bShowingPath(false),
	  m_pCurrentPath(&m_PathA),
	  m_Waypoint(CAINode::kInvalidID),
	  m_nDestNodeID(CAINode::kInvalidID),
	  m_nDestVolumeID(CAIVolume::kInvalidID),
	  m_eSetType(eNotSet)
{
	SetupFSM();
}

CAIStrategyFollowPath::~CAIStrategyFollowPath()
{
	m_pCurrentPath = LTNULL;
}


void CAIStrategyFollowPath::SetupFSM()
{

	//
	// Set up the root FSM.
	//
	m_RootFSM.DefineState( Root::eStateSet, 
		                   m_RootFSM.MakeCallback(*this,&CAIStrategyFollowPath::UpdatePath),
						   m_RootFSM.MakeCallback(*this,&CAIStrategyFollowPath::StartPath),
						   m_RootFSM.MakeCallback(*this,&CAIStrategyFollowPath::EndPath)  );

	m_RootFSM.DefineState( Root::eStateUnset, m_RootFSM.MakeCallback() );

	m_RootFSM.DefineState( Root::eStateDone, 
						   m_RootFSM.MakeCallback(),
						   m_RootFSM.MakeCallback(*this,&CAIStrategyFollowPath::StartDone) );


	m_RootFSM.DefineTransition( Root::eNewDestination, Root::eStateSet );
	m_RootFSM.DefineTransition( Root::eInvalidPath, Root::eStateUnset );

	m_RootFSM.DefineTransition( Root::eStateSet, Root::eReachedEndOfPath, Root::eStateDone );

#ifdef FOLLOW_PATH_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStrategyFollowPath::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStrategyFollowPath::PrintRootFSMMessage));
#endif

	m_RootFSM.SetInitialState( Root::eStateUnset );

	//
	// Set up the path FSM.
	//
	m_PathFSM.DefineState( Path::eStateFindNextWaypoint,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateFindNextWaypoint) );

	m_PathFSM.DefineState( Path::eStateMoveTo,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateMoveTo),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::StartMoveTo) );

	m_PathFSM.DefineState( Path::eStateLadderTo,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateLadderTo),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::StartLadderTo),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::EndLadderTo) );

	m_PathFSM.DefineState( Path::eStateWallWalkTo,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateWallWalkTo),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::StartWallWalkTo) );

	m_PathFSM.DefineState( Path::eStateOpenDoor, 
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateOpenDoor) );

	m_PathFSM.DefineState( Path::eStateJumpTo,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateJumpTo),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::StartJumpTo) );
	
	m_PathFSM.DefineState( Path::eStateWWJumpTo,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateWWJumpTo),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::StartWWJumpTo) );

	m_PathFSM.DefineState( Path::eStateDrop,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateDrop),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::StartDrop) );

	m_PathFSM.DefineState( Path::eStateBezierTo,   
		                   m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::UpdateBezierTo),
				           m_PathFSM.MakeCallback(*this, &CAIStrategyFollowPath::StartBezierTo) );

	m_PathFSM.DefineTransition( Path::eNewPath,				Path::eStateFindNextWaypoint );
	m_PathFSM.DefineTransition( Path::eFinishedWaypoint,	Path::eStateFindNextWaypoint );
	m_PathFSM.DefineTransition( Path::eMoveToWaypoint,		Path::eStateMoveTo );
	m_PathFSM.DefineTransition( Path::eLadderToWaypoint,	Path::eStateLadderTo );
	m_PathFSM.DefineTransition( Path::eWallWalkToWaypoint,	Path::eStateWallWalkTo );
	m_PathFSM.DefineTransition( Path::eJumpToWaypoint,		Path::eStateJumpTo );
	m_PathFSM.DefineTransition( Path::eWWJumpToWaypoint,	Path::eStateWWJumpTo );
	m_PathFSM.DefineTransition( Path::eOpenDoorWaypoint,	Path::eStateOpenDoor );
	m_PathFSM.DefineTransition( Path::eDrop,				Path::eStateDrop );
	m_PathFSM.DefineTransition( Path::eBezierToWaypoint,	Path::eStateBezierTo );


#ifdef FOLLOW_PATH_DEBUG
	m_PathFSM.SetChangingStateCallback(m_PathFSM.MakeChangingStateCallback(*this,&CAIStrategyFollowPath::PrintPathFSMState));
	m_PathFSM.SetProcessingMessageCallback(m_PathFSM.MakeProcessingMessageCallback(*this,&CAIStrategyFollowPath::PrintPathFSMMessage));
#endif

	m_PathFSM.SetInitialState( Path::eStateFindNextWaypoint );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::blah(RootFSM *)
//
//	PURPOSE:	Updates the root state machine
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::Update()
{
	CAIStrategy::Update();

	m_RootFSM.Update();

#ifndef _FINAL
	if( GetAIPtr() && ShouldShowPath(GetAIPtr()->m_hObject) != m_bShowingPath )
	{
		m_bShowingPath = ShouldShowPath(GetAIPtr()->m_hObject);

		if( m_bShowingPath )
		{
			// Draw path (if we have one).
			if( m_pCurrentPath )
			{
				LineSystem::GetSystem(this,"ShowPath")
					<< LineSystem::Clear()
					<< ShowPath(*m_pCurrentPath, Color::Blue );
			}
			else
			{
				m_bShowingPath = false;
			}
		}
		else
		{
			// Clear lines path
			LineSystem::GetSystem(this,"ShowPath")
				<< LineSystem::Clear();
		}
	}
#endif
}

void CAIStrategyFollowPath::Clear()
{
	// Clear out the old messages.
	m_PathFSM.ClearMessages();
	m_RootFSM.ClearMessages();

	m_pCurrentPath->ClearWaypoints();
	
#ifndef _FINAL
	if( m_bShowingPath )
	{
		LineSystem::GetSystem(this,"ShowPath")
				<< LineSystem::Clear();
	}
#endif

	m_bShowingPath = false;

	m_Waypoint = CAIPathWaypoint(CAINode::kInvalidID);

	GetAIPtr()->GetMovePtr()->Clear();
	m_nDestNodeID = CAINode::kInvalidID;
	m_nDestVolumeID = CAIVolume::kInvalidID;

	m_eSetType = eNotSet;

	m_RootFSM.SetState(Root::eStateUnset);
}

const LTVector & CAIStrategyFollowPath::GetWaypointPos() const
{
	static const LTVector zero(0,0,0);

	if( m_Waypoint.IsValid() )
	{
		return m_Waypoint.GetArgumentPosition();
	}

	return zero;
}

bool CAIStrategyFollowPath::IsStuck() const
{
	return GetAIPtr()->GetMovePtr()->IsStuck();
}

LTVector CAIStrategyFollowPath::GetDestinationPos() const
{
	static const LTVector zero(0,0,0);

	const CAIPathWaypoint & end_point = m_pCurrentPath->GetFinalWaypoint();
	if( end_point.IsValid() )
	{
		return end_point.GetArgumentPosition();
	}

	return zero;
}


void  CAIStrategyFollowPath::StartPath(RootFSM * rootFSM)
{
	m_PathFSM.AddMessage(Path::eNewPath);
}

void  CAIStrategyFollowPath::UpdatePath(RootFSM * rootFSM)
{
	m_PathFSM.Update();
}

void  CAIStrategyFollowPath::EndPath(RootFSM * rootFSM)
{
}
	
void  CAIStrategyFollowPath::StartDone(RootFSM * pathFSM)
{
	if( m_nDestNodeID != CAINode::kInvalidID )
	{
		CAINode * pNode = g_pAINodeMgr->GetNode(m_nDestNodeID);

		_ASSERT( pNode );
		if( pNode )
		{
			const LTVector & my_pos = GetAIPtr()->GetPosition();
			const LTVector & my_dims = GetAIPtr()->GetDims();
			const LTVector & node_pos = pNode->GetPos();

			// Only perform the action if we really, truly are on top of the node.
			// Or if it is a wall walk node, as we are not always "on top" of those nodes!
			if( pNode->IsWallWalkOnly()
				|| (   my_pos.x + my_dims.x >= node_pos.x 
				    && my_pos.x - my_dims.x <= node_pos.x
				    && my_pos.z + my_dims.z >= node_pos.z
				    && my_pos.z - my_dims.z <= node_pos.z ) )
			{
				// Send a trigger to self, if a message exists.
				if( !pNode->GetSelfTriggerMsg().empty() )
				{
					SendTriggerMsgToObject( LTNULL, GetAIPtr()->m_hObject, pNode->GetSelfTriggerMsg().c_str() );
				}

				// Send a trigger to an object, if one is specified.
				if( pNode->GetTriggerObject() != LTNULL )
				{
					SendTriggerMsgToObject( GetAIPtr(), pNode->GetTriggerObject(), pNode->GetTriggerMsg().c_str() );
				}
			}
		}
	}
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::UpdateFindNextWaypoint(PathFSM *)
//
//	PURPOSE:	Loads the next waypoint in the path. If there are no more waypoints
//              it notifies RootFSM that end of path has been reached.
//
// ----------------------------------------------------------------------- //

void  CAIStrategyFollowPath::UpdateFindNextWaypoint(PathFSM * pathFSM)
{

	_ASSERT( m_pCurrentPath );

	// Must have a valid path pointer.
	if( !m_pCurrentPath )
	{
		m_RootFSM.AddMessage(Root::eInvalidPath);
		return;
	}

	// Have we reached the end of the path?
	if( !m_pCurrentPath->HasRemainingWaypoints() )
	{
		m_RootFSM.AddMessage(Root::eReachedEndOfPath);
		return;
	}

	// If we are just pathing to a volume, quit when we are in that volume!
	const CAIVolume * pCurrentVolume = GetAIPtr()->GetCurrentVolumePtr();
	if(    m_nDestVolumeID != CAIVolume::kInvalidID 
		&& pCurrentVolume 
		&& pCurrentVolume->GetID() == m_nDestVolumeID )
	{
		m_RootFSM.AddMessage(Root::eReachedEndOfPath);
		return;
	}

	// We still have waypoints to follow, get at it.

	// Pop the next waypoint of the stack (err, I mean path).
	m_Waypoint = m_pCurrentPath->GetCurrentWaypoint();
	m_pCurrentPath->IncrementWaypointIndex();

	const CAINode * pNextNode = g_pAINodeMgr->GetNode( m_Waypoint.GetNodeID() );


	// Interpret the waypoint's instructions.
	switch ( m_Waypoint.GetInstruction() )
	{
		case CAIPathWaypoint::eInstructionMoveTo:
		{
			pathFSM->AddMessage(Path::eMoveToWaypoint);
		}
		break;

		case CAIPathWaypoint::eInstructionLadderTo:
		{
			pathFSM->AddMessage(Path::eLadderToWaypoint);
		}
		break;

		case CAIPathWaypoint::eInstructionOpenDoors:
		{
			pathFSM->AddMessage(Path::eOpenDoorWaypoint);
		}
		break;

		case CAIPathWaypoint::eInstructionWallWalkTo:
		{
			pathFSM->AddMessage(Path::eWallWalkToWaypoint);
		}
		break;

		case CAIPathWaypoint::eInstructionJumpTo:
		{
			pathFSM->AddMessage(Path::eJumpToWaypoint);
		}
		break;

		case CAIPathWaypoint::eInstructionWWJumpTo:
		{
			pathFSM->AddMessage(Path::eWWJumpToWaypoint);
		}
		break;

		case CAIPathWaypoint::eInstructionDrop:
		{
			pathFSM->AddMessage(Path::eDrop);
		}
		break;

		case CAIPathWaypoint::eInstructionBezierTo:
		{
			pathFSM->AddMessage(Path::eBezierToWaypoint);
		}
		break;

		default:
		{
#ifndef _FINAL
			g_pServerDE->CPrint("CAIStrategyFollowPath::Update - unrecognized waypoint instruction");
#endif
			m_RootFSM.AddMessage(Root::eInvalidPath);
		}
		break;
	}
}

void  CAIStrategyFollowPath::RecordReachedWaypoint()
{
	GetAIPtr()->SetNextNode(LTNULL);
	GetAIPtr()->SetLastNodeReached( g_pAINodeMgr->GetNode(m_Waypoint.GetNodeID()) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::UpdateMoveTo
//
//	PURPOSE:	Moves to a point
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::StartMoveTo(PathFSM * pathFSM)
{
	const CAINode * const pDestNode = g_pAINodeMgr->GetNode(m_Waypoint.GetNodeID());
	const CAIVolume * const pDestVolume = pDestNode ? pDestNode->GetContainingVolumePtr() : LTNULL;

	if(    m_pCurrentPath 
		&& m_pCurrentPath->HasRemainingWaypoints()  )
	{
		// Check to see if the next waypoint is in the same volume as the AI.
		const CAIPathWaypoint & next_waypoint = m_pCurrentPath->GetCurrentWaypoint();


		if( GetAIPtr()->GetCurrentVolumePtr() && GetAIPtr()->GetCurrentVolumePtr()->Contains(next_waypoint.GetArgumentPosition()) )
		{
			// Just go to the next waypoint!
			GetAIPtr()->SetNextNode( LTNULL );
			pathFSM->AddMessage(Path::eFinishedWaypoint);

			// Update MoveTo will still be called, so be sure to clear the move strategy!
			GetAIPtr()->GetMovePtr()->Clear();
			return;
		}

		// Only allow movement to overshoot to next destination if the next point is another move-to point.
		if( next_waypoint.GetInstruction() == CAIPathWaypoint::eInstructionMoveTo )
		{
			GetAIPtr()->GetMovePtr()->Set( m_Waypoint.GetArgumentPosition(), next_waypoint.GetArgumentPosition(), m_bExactMovement, pDestVolume, pDestNode );
		}
		else
		{
			GetAIPtr()->GetMovePtr()->Set( m_Waypoint.GetArgumentPosition(), m_bExactMovement, pDestVolume, pDestNode );
		}


#ifdef FOLLOW_PATH_DEBUG
//		AICPrint(GetAIPtr()->m_hObject,"StartMoveTo %s, until %s.",
//			VectorToString(m_Waypoint.GetArgumentPosition()).c_str(),
//			VectorToString(m_pCurrentPath->GetCurrentWaypoint().GetArgumentPosition()).c_str() );
#endif

	}
	else
	{
		GetAIPtr()->GetMovePtr()->Set( m_Waypoint.GetArgumentPosition(), m_bExactMovement, pDestVolume, pDestNode );

#ifdef FOLLOW_PATH_DEBUG
//		AICPrint(GetAIPtr()->m_hObject,"StartMoveTo %s.",
//			VectorToString(m_Waypoint.GetArgumentPosition()).c_str() );
#endif

	}
}

void CAIStrategyFollowPath::UpdateMoveTo(PathFSM * pathFSM)
{
	_ASSERT( GetAIPtr() );

	GetAIPtr()->GetMovePtr()->Update();

	if( GetAIPtr()->GetMovePtr()->IsDone() )
	{
		// No need to call RecordReachedWaypoint(), the move strategy does that already.
		pathFSM->AddMessage(Path::eFinishedWaypoint);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::UpdateLadderTo
//
//	PURPOSE:	Moves to a point
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::StartLadderTo(PathFSM * pathFSM)
{
	LTVector vLadderDest = m_Waypoint.GetArgumentPosition();
	LTVector vExitDest = vLadderDest;

	if(    m_pCurrentPath 
		&& m_pCurrentPath->HasRemainingWaypoints()  )
	{
		const CAIPathWaypoint & next_waypoint = m_pCurrentPath->GetCurrentWaypoint();

		if( next_waypoint.GetInstruction() != CAIPathWaypoint::eInstructionLadderTo )
		{
			vExitDest = next_waypoint.GetArgumentPosition();
		}
	}

	AIActionLadderTo * pNextAction = dynamic_cast<AIActionLadderTo *>( GetAIPtr()->SetNextAction(CAI::ACTION_LADDERTO) );
	if( pNextAction )
	{
		pNextAction->Init( vLadderDest, vExitDest );
	}
}

void CAIStrategyFollowPath::UpdateLadderTo(PathFSM * pathFSM)
{
	// If we reach this point and have no new actions in queue, then
	// we can assume that the jump action succeeded.
	if( GetAIPtr()->CanDoNewAction() )
	{
		RecordReachedWaypoint();
		pathFSM->AddMessage(Path::eFinishedWaypoint);
	}
}

void CAIStrategyFollowPath::EndLadderTo(PathFSM * pathFSM)
{
	GetAIPtr()->StandUp();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::UpdateLadderTo
//
//	PURPOSE:	Moves to a point
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::StartWallWalkTo(PathFSM * pathFSM)
{
	CAINode * const pDestNode = g_pAINodeMgr->GetNode(m_Waypoint.GetNodeID());

	if( pDestNode )
	{
		if( m_Waypoint.GetArgumentNormal() != LTVector(0,0,0) )
		{
			GetAIPtr()->GetMovePtr()->SetWallWalk( m_Waypoint.GetArgumentPosition(), pDestNode->GetNormal(), pDestNode );
		}
		else
		{
			_ASSERT(!"CAIStrategyFollowPath::StartWallWalkTo does not have a normal!");
			GetAIPtr()->GetMovePtr()->SetWallWalk( m_Waypoint.GetArgumentPosition(), GetAIPtr()->GetUp(), pDestNode );
		}
	}
	else
	{
		_ASSERT(!"CAIStrategyFollowPath::StartWallWalkTo does not have a node!");
		GetAIPtr()->GetMovePtr()->SetWallWalk( m_Waypoint.GetArgumentPosition(), GetAIPtr()->GetUp(), pDestNode );
	}


}

void CAIStrategyFollowPath::UpdateWallWalkTo(PathFSM * pathFSM)
{

	UpdateMoveTo(pathFSM);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::*JumpTo
//
//	PURPOSE:	Jumps to a destination
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::StartJumpTo(PathFSM * pathFSM)
{
	const CAINode * pNode = LTNULL;
	
	AIActionJumpTo * pNewAction = dynamic_cast<AIActionJumpTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_JUMPTO) );
	_ASSERT( pNewAction );
	if( pNewAction )
	{
		LTVector vJumpDest = m_Waypoint.GetArgumentPosition();
		vJumpDest.y += GetAIPtr()->GetDims().y;
		pNewAction->Init( vJumpDest );
	}
}

void CAIStrategyFollowPath::UpdateJumpTo(PathFSM * pathFSM)
{
	// If we reach this point and have no new actions in queue, then
	// we can assume that the jump action succeeded.
	if( GetAIPtr()->CanDoNewAction() )
	{
		// If we succeeded.
		if( GetAIPtr()->GetMovement()->GetCharacterVars()->m_bAllowJump )
		{
			RecordReachedWaypoint();
			pathFSM->AddMessage(Path::eFinishedWaypoint);
		}
		else
		{
			// Re-path.
			Repath();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::*WWJumpTo
//
//	PURPOSE:	Jumps to a destination
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::StartWWJumpTo(PathFSM * pathFSM)
{
	const CAINode * pNode = LTNULL;
	
	AIActionWWJumpTo * pNewAction = dynamic_cast<AIActionWWJumpTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_WWJUMPTO) );
	_ASSERT( pNewAction );
	if( pNewAction )
		pNewAction->Init( m_Waypoint.GetArgumentPosition(), m_Waypoint.GetArgumentNormal() );
}

void CAIStrategyFollowPath::UpdateWWJumpTo(PathFSM * pathFSM)
{
	// If we reach this point and have no new actions in queue, then
	// we can assume that the jump action succeeded.
	if( GetAIPtr()->CanDoNewAction() )
	{
		RecordReachedWaypoint();
		pathFSM->AddMessage(Path::eFinishedWaypoint);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::*Drop
//
//	PURPOSE:	The AI stands up.  If he is wall walking, that will cause him to drop.
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::StartDrop(PathFSM * pathFSM)
{
	const CAINode * pNode = LTNULL;
	
	AIActionDrop * pNewAction = dynamic_cast<AIActionDrop*>( GetAIPtr()->SetNextAction(CAI::ACTION_DROP) );
	_ASSERT( pNewAction );
	if( pNewAction )
	{
		// Clear out our node information, we're dropping off the wall!
		GetAIPtr()->SetLastNodeReached( LTNULL );
		GetAIPtr()->SetNextNode( LTNULL );
	}
}

void CAIStrategyFollowPath::UpdateDrop(PathFSM * pathFSM)
{
	// If we reach this point and have no new actions in queue, then
	// we can assume that the jump action succeeded.
	if( GetAIPtr()->CanDoNewAction() )
	{
		// Don't record a reached waypoint, as we didn't really reach anything.
		pathFSM->AddMessage(Path::eFinishedWaypoint);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::*BezierTo
//
//	PURPOSE:	Follows Bezier curve to a destination
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::StartBezierTo(PathFSM * pathFSM)
{

	// Be sure we are wall-walking.
	GetAIPtr()->WallWalk();


	AIActionBezierTo * pNewAction = dynamic_cast<AIActionBezierTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_BEZIERTO) );
	_ASSERT( pNewAction );
	if( pNewAction )
	{
		const LTFLOAT fHeight = GetAIPtr()->GetMovement()->GetDimsSetting(CM_DIMS_CROUCHED).y;

		LTVector vDest = m_Waypoint.GetArgumentPosition();
		vDest += m_Waypoint.GetArgumentNormal()*fHeight;
		pNewAction->Init( vDest, m_Waypoint.GetArgumentNormal(), 
			              m_Waypoint.GetArgumentBezierSrc(), m_Waypoint.GetArgumentBezierDest() );
	}
}

void CAIStrategyFollowPath::UpdateBezierTo(PathFSM * pathFSM)
{
	// If we reach this point and have no new actions in queue, then
	// we can assume that the jump action succeeded.
	if( GetAIPtr()->CanDoNewAction() )
	{
		// This action cannot fail!
		// If we have reached this point, it has (or will soon) succeed.
		RecordReachedWaypoint();
		pathFSM->AddMessage(Path::eFinishedWaypoint);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::UpdateOpenDoors
//
//	PURPOSE:	Open doors
//
// ----------------------------------------------------------------------- //


void CAIStrategyFollowPath::UpdateOpenDoor(PathFSM * pathFSM)
{
	bool bDoorsOpen = true;

	for( CAIPathWaypoint::const_ObjectIterator iter = m_Waypoint.BeginObjectArguments();
	     iter != m_Waypoint.EndObjectArguments(); ++iter )
	{
		HOBJECT hDoor = *iter;
		if ( hDoor )
		{
			Door* pDoor = dynamic_cast<Door*>(g_pServerDE->HandleToObject(hDoor));
			if ( pDoor )
			{
				if(	pDoor->GetState() == DOORSTATE_CLOSED || pDoor->GetState() == DOORSTATE_CLOSING )
				{
					GetAIPtr()->OpenDoor(hDoor);
				}
			
				if ( pDoor->GetState() != DOORSTATE_OPEN )
				{
					bDoorsOpen = false;
				}
			}
		}
	}

	if ( bDoorsOpen )
	{
		RecordReachedWaypoint();
		pathFSM->AddMessage(Path::eFinishedWaypoint);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::Set
//
//	PURPOSE:	Sets the path that we will be following
//
// ----------------------------------------------------------------------- //

LTBOOL CAIStrategyFollowPath::Set(const LTVector& vDestination)
{
	CAIPath * pTestPath = &m_PathA;
	if( pTestPath == m_pCurrentPath )
		pTestPath = &m_PathB;

	const LTBOOL path_found = g_pAIPathMgr->FindPath(GetAIPtr(), vDestination, pTestPath);

	if( path_found ) 
	{
		// We found a path!  Switch to the new path.
		m_pCurrentPath = pTestPath;

		m_eSetType = ePosition;

		m_RootFSM.AddMessage( Root::eNewDestination );

#ifndef _FINAL
		if( GetAIPtr() && ShouldShowPath(GetAIPtr()->m_hObject) )
		{
			m_bShowingPath = true;

			LineSystem::GetSystem(this,"ShowPath")
				<< LineSystem::Clear()
				<< ShowPath(*m_pCurrentPath, Color::Blue );
		}
#endif
	}
	else
	{
		// Go ahead and stop all movement anyhow.
		Clear();
		m_RootFSM.AddMessage( Root::eInvalidPath );
	}

	
	m_RootFSM.Update();

	return path_found;
}

LTBOOL CAIStrategyFollowPath::Set(const CAINode* pDestNode, LTBOOL bWarnIfFail /* = LTFALSE */)
{
	_ASSERT( pDestNode );
	if( !pDestNode ) 
	{
		m_RootFSM.AddMessage(Root::eInvalidPath);
		m_RootFSM.Update();

		return LTFALSE;
	}

	CAIPath * pTestPath = &m_PathA;
	if( pTestPath == m_pCurrentPath )
		pTestPath = &m_PathB;

	const LTBOOL path_found = g_pAIPathMgr->FindPath(GetAIPtr(), pDestNode, pTestPath);

	if( path_found )
	{
		m_nDestNodeID = pDestNode->GetID();

		m_pCurrentPath = pTestPath;

		m_eSetType = eNode;

		m_RootFSM.AddMessage(Root::eNewDestination);

#ifndef _FINAL
		if( GetAIPtr() && ShouldShowPath(GetAIPtr()->m_hObject) )
		{
			m_bShowingPath = true;

			LineSystem::GetSystem(this,"ShowPath")
				<< LineSystem::Clear()
				<< ShowPath(*m_pCurrentPath, Color::Blue );
		}
#endif
	}
	else
	{
		m_RootFSM.AddMessage( Root::eInvalidPath );

#ifndef _FINAL
		if( bWarnIfFail )
		{
			AIErrorPrint(GetAIPtr(), "Could not find path to node \"%s\" at (%.2f,%.2f,%.2f).",
				pDestNode->GetName().c_str(), 
				pDestNode->GetPos().x,pDestNode->GetPos().y,pDestNode->GetPos().z );
		}
#endif		
#ifdef _DEBUG
		// In debug mode, I want to always know about this!
		if( !bWarnIfFail )
		{
//			AICPrint(GetAIPtr(), "(Debug Msg) Did not find path to node \"%s\" at (%.2f,%.2f,%.2f).",
//				pDestNode->GetName().c_str(), 
//				pDestNode->GetPos().x,pDestNode->GetPos().y,pDestNode->GetPos().z );
		}
#endif
	}

	m_RootFSM.Update();

	return path_found;
}

LTBOOL CAIStrategyFollowPath::Set(const CAIVolume* pVolumeDestination)
{
	Clear();

	_ASSERT( pVolumeDestination );
	if( !pVolumeDestination )
	{
		m_RootFSM.AddMessage(Root::eInvalidPath);
		m_RootFSM.Update();
		Clear();
		return LTFALSE;
	}

	CAIPath * pTestPath = &m_PathA;
	if( pTestPath == m_pCurrentPath )
		pTestPath = &m_PathB;

	const LTBOOL path_found = g_pAIPathMgr->FindPath(GetAIPtr(), pVolumeDestination, pTestPath);

	if( path_found )
	{
		m_pCurrentPath = pTestPath;
		
		m_nDestVolumeID = pVolumeDestination->GetID();

		m_eSetType = eVolume;

		m_RootFSM.AddMessage(Root::eNewDestination);

#ifndef _FINAL
		if( GetAIPtr() && ShouldShowPath(GetAIPtr()->m_hObject) )
		{
			m_bShowingPath = true;

			LineSystem::GetSystem(this,"ShowPath")
				<< LineSystem::Clear()
				<< ShowPath(*m_pCurrentPath, Color::Blue );
		}
#endif
	}
	else
	{
		m_RootFSM.AddMessage(Root::eInvalidPath);
	}

	m_RootFSM.Update();

	return path_found;
}

LTBOOL CAIStrategyFollowPath::Set(const CCharacter* pCharDestination)
{
	_ASSERT( pCharDestination );
	if( !pCharDestination )
	{
		m_RootFSM.AddMessage(Root::eInvalidPath);
		m_RootFSM.Update();
		Clear();
		return LTFALSE;
	}

	CAIPath * pTestPath = &m_PathA;
	if( pTestPath == m_pCurrentPath )
		pTestPath = &m_PathB;

	const LTBOOL path_found = g_pAIPathMgr->FindPath(GetAIPtr(), pCharDestination, pTestPath);

	if( path_found )
	{

		m_pCurrentPath = pTestPath;

		m_eSetType = eCharacter;

		m_RootFSM.AddMessage(Root::eNewDestination);

#ifndef _FINAL
		if( GetAIPtr() && ShouldShowPath(GetAIPtr()->m_hObject) )
		{
			m_bShowingPath = true;

			LineSystem::GetSystem(this,"ShowPath")
				<< LineSystem::Clear()
				<< ShowPath(*m_pCurrentPath, Color::Blue );
		}
#endif
	}
	else
	{
		m_RootFSM.AddMessage(Root::eInvalidPath);
	}

	m_RootFSM.Update();

	return path_found;
}

LTBOOL CAIStrategyFollowPath::SetSequintial(CAIVolume* pCurrentVolume, CAIVolume* pPreviousVolume )
{

	CAIPath * pTestPath = &m_PathA;
	if( pTestPath == m_pCurrentPath )
		pTestPath = &m_PathB;

	const LTBOOL path_found = g_pAIPathMgr->FindSequentialPath(GetAIPtr(),pCurrentVolume, pPreviousVolume,pTestPath);

	if( path_found )
	{
		m_pCurrentPath = pTestPath;

		m_eSetType = eSequintial;

		m_RootFSM.AddMessage(Root::eNewDestination);

#ifndef _FINAL
		if( GetAIPtr() && ShouldShowPath(GetAIPtr()->m_hObject) )
		{
			m_bShowingPath = true;

			LineSystem::GetSystem(this,"ShowPath")
				<< LineSystem::Clear()
				<< ShowPath(*m_pCurrentPath, Color::Blue );
		}
#endif
	}
	else
	{
		m_RootFSM.AddMessage(Root::eInvalidPath);
	}
	
	m_RootFSM.Update();

	return path_found;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::Reset
//
//	PURPOSE:	Re-paths.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIStrategyFollowPath::Repath()
{
	switch( m_eSetType )
	{
		case ePosition :
		{
			return Set(GetDestinationPos());
		}
		break;

		case eVolume :
		{
			return Set(g_pAIVolumeMgr->GetVolumePtr(m_nDestVolumeID));
		}
		break;

		case eNode :
		{
			return Set(g_pAINodeMgr->GetNode(m_nDestNodeID), LTTRUE);
		}
		break;

		case eCharacter :
		{
			// Nothing to do but keep trying for
			// that same position.
			return Set(GetDestinationPos());
		}
		break;


		case eSequintial :
		{
			// This just won't repath.
			Clear();
			return LTFALSE;
		}
		break;

		default :
		case eNotSet :
		{
			Clear();
			return LTFALSE;
		}
		break;
	}

	// This section shouldn't be reached.
	ASSERT( 0 );
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!g_pServerDE || !hRead) return;

	CAIStrategy::Load(hRead);

	*hRead >> m_RootFSM;
	*hRead >>  m_PathFSM;

	*hRead >> m_bExactMovement;

	m_bShowingPath = false;

	*hRead >> m_PathA;
	*hRead >> m_PathB;

	const bool bUsingPathA = (hRead->ReadByte() != 0);
	if( bUsingPathA )
		m_pCurrentPath = &m_PathA;
	else
		m_pCurrentPath = &m_PathB;

	*hRead >>  m_Waypoint;

	*hRead >> m_nDestNodeID;
	*hRead >> m_nDestVolumeID;

	*hRead >> m_eSetType;
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyFollowPath::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIStrategyFollowPath::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!g_pServerDE || !hWrite) return;

	CAIStrategy::Save(hWrite);

	*hWrite << m_RootFSM;
	*hWrite << m_PathFSM;

	// GetAIPtr()->GetMovePtr() set at load time.
	*hWrite << m_bExactMovement;

	// m_bShowingPath should not be saved.

	*hWrite << m_PathA;
	*hWrite << m_PathB;
	hWrite->WriteByte( m_pCurrentPath == &m_PathA );

	*hWrite << m_Waypoint;

	*hWrite << m_nDestNodeID;
	*hWrite << m_nDestVolumeID;

	*hWrite << m_eSetType;

}

#ifdef FOLLOW_PATH_DEBUG

void CAIStrategyFollowPath::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateSet : szMessage = "eStateSet"; break;
		case Root::eStateUnset : szMessage = "eStateUnset"; break;
		case Root::eStateDone : szMessage = "eStateDone"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" FollowPath::RootFSM is entering state %s.", szMessage );
}

void CAIStrategyFollowPath::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eNewDestination : szMessage = "eNewDestination"; break;
		case Root::eInvalidPath  : szMessage = "eInvalidPath"; break;
		case Root::eReachedEndOfPath : szMessage = "eReachedEndOfPath"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
				" FollowPath::RootFSM is processing %s.", szMessage );
}



void CAIStrategyFollowPath::PrintPathFSMState(const PathFSM & fsm, Path::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Path::eStateFindNextWaypoint : szMessage = "eStateFindNextWaypoint"; break;
		case Path::eStateMoveTo  : szMessage = "eStateMoveTo"; break;
		case Path::eStateLadderTo  : szMessage = "eStateLadderTo"; break;
		case Path::eStateWallWalkTo  : szMessage = "eStateWallWalkTo"; break;
		case Path::eStateJumpTo  : szMessage = "eStateJumpTo"; break;
		case Path::eStateWWJumpTo  : szMessage = "eStateWWJumpTo"; break;
		case Path::eStateDrop  : szMessage = "eStateDrop"; break;
		case Path::eStateOpenDoor : szMessage = "eStateOpenDoor"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
			" FollowPath::PathFSM is entering state %s.", szMessage );
}

void CAIStrategyFollowPath::PrintPathFSMMessage(const PathFSM & fsm, Path::Message message)
{
	const char * szMessage = 0;
	switch( message )
	{
		case Path::eNewPath : szMessage = "eNewPath"; break;
		case Path::eMoveToWaypoint : szMessage = "eMoveToWaypoint"; break;
		case Path::eLadderToWaypoint : szMessage = "eLadderToWaypoint"; break;
		case Path::eOpenDoorWaypoint : szMessage = "eOpenDoorWaypoint"; break;
		case Path::eWallWalkToWaypoint : szMessage = "eWallWalkToWaypoint"; break;
		case Path::eJumpToWaypoint : szMessage = "eJumpToWaypoint"; break;
		case Path::eWWJumpToWaypoint : szMessage = "eWWJumpToWaypoint"; break;
		case Path::eDrop : szMessage = "eDrop"; break;
		case Path::eFinishedWaypoint : szMessage = "eFinishedWaypoint"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
				" FollowPath::PathFSM is processing %s.", szMessage );
}

#else


void CAIStrategyFollowPath::PrintRootFSMState(const RootFSM & , Root::State )
{
}

void CAIStrategyFollowPath::PrintRootFSMMessage(const RootFSM & , Root::Message )
{
}

void CAIStrategyFollowPath::PrintPathFSMState(const PathFSM & , Path::State )
{
}

void CAIStrategyFollowPath::PrintPathFSMMessage(const PathFSM & , Path::Message )
{
}

#endif
