// ----------------------------------------------------------------------- //
//
// MODULE  : CAIStrategyMove.cpp (was AIMovement.h)
//
// PURPOSE : 
//
// CREATED : 6.20.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIStrategyMove.h"
#include "AI.h"
#include "CharacterMovement.h"
#include "ObstacleMgr.h"
#include "AIVolume.h"
#include "AINode.h"
#include "AITarget.h"
#include "DebugLineSystem.h"
#include "AIUtils.h"
#include "CVarTrack.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "AIButeMgr.h"
#include "AIActionMoveTo.h"
#include "AIActionMisc.h"

#include <limits>
#include <algorithm>

#ifdef _DEBUG
#define MILD_MOVE_DEBUG
//#define STRATEGY_MOVE_DEBUG
#endif



namespace /* unnamed */
{
	const LTFLOAT g_fPathingFrequency = 0.5f;
	const LTFLOAT g_fMinTolerance = 1.0f;

#ifndef _FINAL
	bool ShouldShowMove(HOBJECT hObject)
	{
		ASSERT( g_pLTServer );

#ifdef STRATEGY_MOVE_DEBUG
		static CVarTrack vtShowMove(g_pLTServer,"ShowAIMove",1.0f);
#else
		static CVarTrack vtShowMove(g_pLTServer,"ShowAIMove",0.0f);
#endif

		return vtShowMove.GetFloat() > 0.0f && ShouldShow(hObject);
	}
#endif

	// The ManhattanDistance is the distance between two points if
	// you had to follow a grid line (had to walk along the city blocks, cute huh?).
	// If you had to walk from the center of 
	LTFLOAT ManhattanDistance(const LTVector & v1,const LTVector & v2)
	{
		return (LTFLOAT)(fabs(v1.x - v2.x) + fabs(v1.y - v2.y) + fabs(v1.z - v2.z));
	}

	bool OvershotDest( const LTVector & vDest,
 					   const LTVector & vPos, 
					   const LTVector & vLastPos,
					   const LTVector & vStartPos )
	{
		const LTVector vMovementDir = vDest - vStartPos;

		return    (vDest - vPos).Dot(vMovementDir) <= 0.0f
			   && (vDest - vLastPos).Dot(vMovementDir) >= 0.0f;
	}

	bool NearDest( const LTVector & vDest, const LTVector & vPos, LTFLOAT fEpsilon)
	{
		return  (vDest - vPos).MagSqr() < fEpsilon*fEpsilon;
	}

	LTVector FitIntoVolume( const CAIVolume & volume, const LTVector & vOriginalDest, const LTVector & vDims, bool * pbFits )
	{
		LTVector vResult = vOriginalDest;

		// Assume it will work!
		if( pbFits )
		{
			*pbFits = LTTRUE;
		}

		if( volume.ContainsWithNeighbor(vOriginalDest, vDims ) )
		{
			return vResult;
		}

		// vPullBackDelta will be the amount to add to the result to try to get it to fit.
		LTVector vPullBackDelta = -vDims*0.5f;

		// Determine the direction we should go.
		if( vOriginalDest.x - volume.GetCenter().x < 0.0f )
		{
			vPullBackDelta.x = -vPullBackDelta.x;
		}

		if( vOriginalDest.z - volume.GetCenter().z < 0.0f )
		{
			vPullBackDelta.z = -vPullBackDelta.z;
		}


		int nMaxIterations = 5;
		while(nMaxIterations > 0 )
		{
			vResult += vPullBackDelta;

			if( volume.ContainsWithNeighbor(vResult, vDims) )
			{
				return vResult;
			}

			--nMaxIterations;
		}

		_ASSERT( nMaxIterations <= 0 );
		
		// If we have reached this point, we can't figure out how to get the point to fit inside the volume,
		// so just give up!
		if( pbFits )
		{
			*pbFits = LTFALSE;
		}

		return vOriginalDest;
	}


} //namespace /* unnamed */







CAIStrategyMove::CAIStrategyMove(CAI * pAI)
		: CAIStrategy(pAI),

		  m_pMovement(pAI ? pAI->GetMovement() : LTNULL),
		  m_pObstacleMgr( new ObstacleMgr() ),

		  m_vDest(0,0,0),
		  m_vNextDest(0,0,0),
		  m_bExactMovement(LTFALSE),
		  m_vNormal(0,0,0),
		  m_pDestNode(LTNULL),
		  m_pDestVolume(LTNULL),
		  m_bCheckNextDest(false),

		  m_bMovedLaterally(false),
		  m_bAvoidingObstacle(false),
		  m_bMightOvershoot(false),
		  m_bFitsInVolume(true),
		  m_vLastPosition(0,0,0),
		  m_fNextCheckTime(0),
		  m_vStartPosition(0,0,0),
		
		  m_nNumCorrections(0),
		  // m_tmrCorrection
		  m_vCorrectionDir(0,0,0),

		  m_pLastAction(LTNULL),
		  m_vLastActionGoal(0,0,0),
		  m_vLastActionPos(0,0,0),
		  m_vStartingPos(0,0,0),

		  m_vLastUsedUp(0,0,0),
		  m_vFacingDir(0,0,0),

		  m_Butes( pAI->GetTemplateId() >= 0 ? g_pAIButeMgr->GetMovement( pAI->GetTemplateId() ) : AIBM_Movement() )
{
	SetupFSM();

	m_pObstacleMgr->SetPathingFrequency( m_Butes.fAvoidancePathFrequency );
}

CAIStrategyMove::~CAIStrategyMove()
{
	delete m_pObstacleMgr;
	m_pObstacleMgr = 0;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyMove::SetPathingFrequency
//
//	PURPOSE:	Sets the pathing frequency of obstacle manager.
//
// ----------------------------------------------------------------------- //

void  CAIStrategyMove::SetPathingFrequency(LTFLOAT val)
{
	m_pObstacleMgr->SetPathingFrequency(val);
}

void CAIStrategyMove::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateMoveTo,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::UpdateMoveTo),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::StartMoveTo ) );

	m_RootFSM.DefineState( Root::eStateWallWalkTo,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::UpdateWallWalkTo),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::StartWallWalkTo ) );

	m_RootFSM.DefineState( Root::eStateDone,  
			               m_RootFSM.MakeCallback()); 
//						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::StopMoving) );

	m_RootFSM.DefineState( Root::eStateStuck,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::UpdateStuck),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::StartStuck)   );

	m_RootFSM.DefineState( Root::eStateMoveToCorrection, 
			               m_RootFSM.MakeCallback(*this, &CAIStrategyMove::UpdateMoveToCorrection),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::StartMoveToCorrection) );

	m_RootFSM.DefineState( Root::eStateWallWalkCorrection, 
			               m_RootFSM.MakeCallback(*this, &CAIStrategyMove::UpdateWWCorrection),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::StartWWCorrection),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyMove::EndWWCorrection)  );

	// Default Transitions.
	m_RootFSM.DefineTransition( Root::eStateMoveTo, Root::eNewDest,       Root::eStateMoveTo );
	m_RootFSM.DefineTransition( Root::eStateMoveTo, Root::eNewWWDest,     Root::eStateWallWalkTo );
	m_RootFSM.DefineTransition( Root::eStateMoveTo, Root::eReachedDest,   Root::eStateDone );
	m_RootFSM.DefineTransition( Root::eStateMoveTo, Root::eNotMoving,     Root::eStateMoveToCorrection );

	m_RootFSM.DefineTransition( Root::eStateWallWalkTo, Root::eNewDest,       Root::eStateMoveTo );
	m_RootFSM.DefineTransition( Root::eStateWallWalkTo, Root::eNewWWDest,     Root::eStateWallWalkTo );
	m_RootFSM.DefineTransition( Root::eStateWallWalkTo, Root::eReachedDest,   Root::eStateDone );
	m_RootFSM.DefineTransition( Root::eStateWallWalkTo, Root::eNotMoving,     Root::eStateWallWalkCorrection );
	
	m_RootFSM.DefineTransition( Root::eStateStuck, Root::eNewDest,       Root::eStateMoveTo );
	m_RootFSM.DefineTransition( Root::eStateStuck, Root::eNewWWDest,     Root::eStateWallWalkTo );
	// Don't respond to Root::eReachedDest
	// Don't respond to Root::eNotMoving

	m_RootFSM.DefineTransition( Root::eStateDone, Root::eNewDest,       Root::eStateMoveTo );
	m_RootFSM.DefineTransition( Root::eStateDone, Root::eNewWWDest,     Root::eStateWallWalkTo );
	// Don't respond to Root::eReachedDest
	// Don't respond to Root::eNotMoving

	m_RootFSM.DefineTransition( Root::eStateMoveToCorrection, Root::eNewDest,       Root::eStateMoveTo );
	m_RootFSM.DefineTransition( Root::eStateMoveToCorrection, Root::eNewWWDest,     Root::eStateWallWalkTo );
	m_RootFSM.DefineTransition( Root::eStateMoveToCorrection, Root::eReachedDest,   Root::eStateDone );
	m_RootFSM.DefineTransition( Root::eStateMoveToCorrection, Root::eNotMoving,     Root::eStateMoveToCorrection );

	m_RootFSM.DefineTransition( Root::eStateWallWalkCorrection, Root::eNewDest,       Root::eStateWallWalkCorrection );
	m_RootFSM.DefineTransition( Root::eStateWallWalkCorrection, Root::eNewWWDest,     Root::eStateWallWalkCorrection );
	m_RootFSM.DefineTransition( Root::eStateWallWalkCorrection, Root::eReachedDest,   Root::eStateDone );
	m_RootFSM.DefineTransition( Root::eStateWallWalkCorrection, Root::eNotMoving,     Root::eStateStuck );


	m_RootFSM.DefineTransition( Root::eClear,         Root::eStateDone );
	m_RootFSM.DefineTransition( Root::eStuck,		  Root::eStateStuck );
	
	m_RootFSM.SetInitialState( Root::eStateDone );


#ifdef STRATEGY_MOVE_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStrategyMove::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStrategyMove::PrintRootFSMMessage));
#endif

}


void CAIStrategyMove::Update()
{
	m_RootFSM.Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyMove::MoveTo
//
//	PURPOSE:	Sets the destination point, AI will move to within tolerance radius
//              of that point.  If tolerance is less than zero, it will be 
//              set to within movement_speed*update_time on each update.
//
// ----------------------------------------------------------------------- //
void CAIStrategyMove::Set(const LTVector& vDest, LTBOOL bExactMovement, const CAIVolume * pDestVolume, const CAINode * pDestNode)
{
	m_vDest = vDest;
	m_pDestVolume = pDestVolume;
	m_pDestNode = pDestNode;
	m_bExactMovement = bExactMovement;

	GetAIPtr()->SetNextNode(m_pDestNode);

	if (!m_pDestVolume)
	{
		m_pDestVolume = g_pAIVolumeMgr->FindContainingVolume(m_vDest);
	}

	m_bCheckNextDest = false;

	if( m_tmrMinAvoidIgnoreableDelay.Stopped() )
		m_pObstacleMgr->AvoidIgnorable(true);

#ifndef _FINAL
	if( ShouldShowMove(m_pMovement->GetObject()) )
	{
		LineSystem::GetSystem(this,"ShowSet") 
			<< LineSystem::Clear()
			<< LineSystem::Box( vDest, LTVector(5.0f,5.0f,5.0f), Color::Red )
			<< LineSystem::Line( m_vDest, vDest, Color::Red )
			<< LineSystem::Box( m_vDest, LTVector(15.0f,15.0f,15.0f), Color::Red );
	}
#endif

	m_RootFSM.AddMessage(Root::eNewDest);
	Update();
}

void CAIStrategyMove::Set(const LTVector& vDest, const LTVector & vNextDest, LTBOOL bExactMovement, const CAIVolume * pDestVolume, const CAINode * pDestNode )
{
	m_vDest = vDest;
	m_pDestNode = pDestNode;
	m_pDestVolume = pDestVolume;
	m_bExactMovement = bExactMovement;

	GetAIPtr()->SetNextNode(m_pDestNode);

	if (!m_pDestVolume)
	{
		m_pDestVolume = g_pAIVolumeMgr->FindContainingVolume(m_vDest);
	}

	m_vNextDest = vNextDest;
	m_bCheckNextDest = true;

	if( m_tmrMinAvoidIgnoreableDelay.Stopped() )
		m_pObstacleMgr->AvoidIgnorable(true);

	m_RootFSM.AddMessage(Root::eNewDest);
	Update();
}

void CAIStrategyMove::SetWallWalk(const LTVector & vDest, const LTVector & vNormal, const CAINode * pDestNode)
{
	m_vDest = vDest;
	m_pDestNode = pDestNode;
	m_pDestVolume = LTNULL;
	m_bExactMovement = LTTRUE;
	m_bCheckNextDest = false;

	GetAIPtr()->SetNextNode(m_pDestNode);

	if( m_tmrMinAvoidIgnoreableDelay.Stopped() )
		m_pObstacleMgr->AvoidIgnorable(true);

	const LTVector & my_pos = GetAIPtr()->GetPosition();
	const LTVector & my_up  = GetAIPtr()->GetUp();

	LTVector toward_goal = (vDest - my_pos);
	toward_goal -= my_up*my_up.Dot(toward_goal);
	toward_goal.Norm();		

	if( toward_goal.MagSqr() > 0.9f )
	{
		m_vFacingDir = toward_goal;
	}

	m_vNormal = vNormal;

	m_RootFSM.AddMessage(Root::eNewWWDest);
	Update();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyMove::Is*
//
//	PURPOSE:	Raw state queries
//
// ----------------------------------------------------------------------- //

bool  CAIStrategyMove::IsSet() const 
{ 
	return m_RootFSM.GetState() == Root::eStateMoveTo
		|| m_RootFSM.GetState() == Root::eStateMoveToCorrection 
		|| m_RootFSM.GetState() == Root::eStateWallWalkTo
		|| m_RootFSM.GetState() == Root::eStateWallWalkCorrection;  
}

bool  CAIStrategyMove::IsDone() const 
{
	return m_RootFSM.GetState() == Root::eStateDone; 
}

bool  CAIStrategyMove::IsStuck() const 
{ 
	return m_RootFSM.GetState() == Root::eStateStuck;
}

void CAIStrategyMove::Clear()
{ 
	GetAIPtr()->SetNextNode(LTNULL);

	m_RootFSM.ClearMessages();
	m_RootFSM.SetState(Root::eStateDone);
}

LTBOOL  CAIStrategyMove::IsAvoidingIgnorable() const 
{ 
	if( m_pObstacleMgr )
		return  m_pObstacleMgr->IsAvoidingIgnorable();

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	State:	MoveTo
//
//	PURPOSE:	Does ground based movement to a destination.
//
// ----------------------------------------------------------------------- //

void CAIStrategyMove::StartMoveTo(RootFSM * fsm)
{

	// Be sure the destination fits into the volume.
	m_bFitsInVolume = true;
	if( m_pDestVolume )
	{
		const LTVector & my_dims = GetAIPtr()->GetDims();
		const LTVector & my_up = GetAIPtr()->GetUp();
		const LTVector volume_projected_goal = m_vDest + my_up*my_up.Dot(m_pDestVolume->GetCenter() - m_vDest);

		const LTVector vOriginalDest = m_vDest;

		m_vDest = FitIntoVolume( *m_pDestVolume, volume_projected_goal, LTVector(my_dims.x,0,my_dims.z), &m_bFitsInVolume );
	}


	m_pObstacleMgr->SetGoal(m_vDest, m_pMovement);
	
	m_nNumCorrections = 0;
	m_bMovedLaterally = false;   // setting this false will cause the movement checks to reset.

	if( GetAIPtr()->GetButes()->m_bWallWalk )
	{
		GetAIPtr()->StandUp();
	}

	m_bAvoidingObstacle = false;
	m_bMightOvershoot = false;


	m_pLastAction = LTNULL;
	m_vLastActionGoal = LTVector(0,0,0);
	m_vLastActionPos = LTVector(0,0,0);
	m_vStartingPos = GetAIPtr()->GetPosition();

}

void CAIStrategyMove::UpdateMoveTo(RootFSM * fsm)
{
	//
	// Set-up some constants.
	//

#ifdef AIMOVEMENT_TIMING
//		LTCounter full_counter;
//		g_pServerDE->StartCounter(&full_counter);
#endif
	

	ASSERT( m_pMovement );
	if( !m_pMovement ) return;

	const LTVector & my_pos = m_pMovement->GetPos();
	const LTVector & my_up = m_pMovement->GetUp();
	const LTVector & my_forward = m_pMovement->GetForward();
	const LTVector & my_right = m_pMovement->GetRight();
	const LTVector & my_dims = m_pMovement->GetObjectDims();
	
	// This may be over-ridden by obstacle avoidance.
	LTVector goal = m_vDest;
	LTVector projected_goal = goal + my_up*( my_up.Dot(my_pos - goal) );

	//
	// If we aren't moving and we don't need exact movement,
	// then being within our dims of the goal is good enough!
	//
	if( !m_bMovedLaterally && !m_bExactMovement )
	{
		if( NearDest( projected_goal, my_pos, my_dims.x*1.1f ) )
		{
			GetAIPtr()->SetNextNode(LTNULL);
			GetAIPtr()->SetLastNodeReached( m_pDestNode );
			fsm->AddMessage(Root::eReachedDest);
			return;
		}
	}


	//
	// Check to see if we've reached the destination.
	//
	if( !m_pLastAction || m_pLastAction != GetAIPtr()->GetCurrentAction() || m_pLastAction->IsDone() || !m_bFitsInVolume )
	{
		if(     NearDest( projected_goal, my_pos, 1.0f )
			|| (!m_bFitsInVolume && NearDest(projected_goal, my_pos, my_dims.z*1.1f) )
			|| (m_bMightOvershoot && OvershotDest(projected_goal,my_pos,m_vLastActionPos, m_vStartingPos) ) )
		{
			if( m_bExactMovement )
			{
				GetAIPtr()->Stop();
				GetAIPtr()->SetNextAction(CAI::ACTION_IDLE);
			}

			GetAIPtr()->SetNextNode(LTNULL);
			GetAIPtr()->SetLastNodeReached( m_pDestNode );
			fsm->AddMessage(Root::eReachedDest);
			return;
		}
	}

	// Wait until we can do a new action.
	if( !GetAIPtr()->CanDoNewAction() )
	{
		return; 
	}

	// Check for being stuck on geometry.
/*	if( m_bMovedLaterally && !GetAIPtr()->IsRechargingExosuit() )
	{
		// Engine caps frame time at 0.2f.  So if the frame time is that
		// or higher, weird things are happening (like the level is loading).
		const LTFLOAT fFrameTime = pMovement->GetCharacterVars()->m_fFrameTime;
		if( fFrameTime < 0.2f )
			m_fNextCheckTime -= fFrameTime;

		if( m_fNextCheckTime < 0.0f )
		{
			if( m_vLastPosition.Equals(my_pos,0.1f) )
			{
#ifdef MILD_MOVE_DEBUG
				AICPrint(GetAIPtr(), "Stuck while moving to (%.2f,%.2f,%.2f), at (%.2f,%.2f,%.2f).",
					m_vDest.x,m_vDest.y,m_vDest.z,
					my_pos.x,my_pos.y,my_pos.z );
#endif			
				fsm->AddMessage(Root::eNotMoving);
				return;
			}

			m_fNextCheckTime = 0.4f; //PLH TODO : bute this?
			m_vLastPosition = my_pos;
		}
		
		m_bMovedLaterally = false;  // This will be set to true if moved forward/reverse or strafe left/right.
	}
	else
	{
		m_fNextCheckTime = 0.4f; //PLH TODO : bute this?
		m_vLastPosition = my_pos;

		m_bMovedLaterally = false;  // This will be set to true if moved forward/reverse or strafe left/right.
	}

*/

#ifndef _FINAL
	if( ShouldShowMove(m_pMovement->GetObject() ) )
	{
		LineSystem::GetSystem(this,"Goal")
			<< LineSystem::Clear()
			<< LineSystem::Arrow( my_pos, projected_goal, Color::Yellow )
			<< LineSystem::Box( projected_goal, LTVector(3.0,3.0,3.0), Color::Yellow )
			<< LineSystem::Box( m_vDest, LTVector(2.5,2.5,2.5), Color::Yellow)
			<< LineSystem::Line( projected_goal, m_vDest, Color::Yellow );
		
		if( m_bCheckNextDest )
		{
			LineSystem::GetSystem(this,"Goal")
				<< LineSystem::Box( m_vNextDest, LTVector(2.5,2.5,2.5), Color::Purple)
				<< LineSystem::Arrow( m_vDest, m_vNextDest, Color::Purple );
		}
	}
#endif

	//
	// Let the obstacle avoidance change our goals.
	//
	LTVector next_goal = m_bCheckNextDest ? m_vNextDest : projected_goal;

	// Note that m_bAvoidingObstacle is only set if an obstacle is between
	// ourselves and the goal!  If the obstacle is on the goal, our
	// goal will get changed but m_bAvoidingObstacle will stay false.
	m_bAvoidingObstacle = m_pObstacleMgr->Update(&goal, &next_goal, m_pMovement );
	projected_goal = goal + my_up*( my_up.Dot(my_pos - goal) );
	
	// If we can't get around our obstacles, be stuck.
	if( m_pObstacleMgr->IsStuck() )
	{
		fsm->AddMessage(Root::eStuck);

#ifndef _FINAL
		if( ShouldShowMove(m_pMovement->GetObject()) )
		{
			LineSystem::GetSystem(this,"MoveToDest")
				<< LineSystem::Clear()
				<< LineSystem::Line( projected_goal, my_pos, Color::Red );
		}
#endif

		return;
	}

	//
	// If one of our actions is running, and obstacle avoidance hasn't 
	// changed our goal, then let the movement action keep running.
	//
	if(    m_pLastAction 
		&& m_pLastAction == GetAIPtr()->GetCurrentAction() 
		&& m_vLastActionGoal.Equals(goal,1.0f) )
	{
		return;
	}

#ifndef _FINAL
	if( ShouldShowMove(m_pMovement->GetObject()) )
	{
		LineSystem::GetSystem(this,"MoveToDest")
			<< LineSystem::Clear()
			<< LineSystem::Line( projected_goal, my_pos, Color::Green );

		LineSystem::GetSystem(this,"ShowMove") 
			<< LineSystem::Clear();
	}
#endif

	//
	// Smooth path by walking to furthest point in volume.
	//
	// This _must_ be done after obstacle avoidance but before movement.  If done after movement,
	// the movement code will jerk the AI to a point that may be discarded.
	if( m_bCheckNextDest )
	{
		if(    GetAIPtr()->GetCurrentVolumePtr() 
			&& GetAIPtr()->GetCurrentVolumePtr()->Contains(m_vNextDest) )
		{
			// The next waypoint is in the same volume, just go for it.
			fsm->AddMessage(Root::eReachedDest);
			return;
		}
		else
		{
			// If the line drawn from the AI to the goal to the next goal
			// is straight, just head for the next goal.  This helps
			// volume to volume transitions.
			LTVector local_goal_pos = goal - my_pos;
			local_goal_pos.y = 0;

			LTVector vNextGoalDir = m_vNextDest - goal;
			vNextGoalDir.y = 0;
			LTVector vPerp = LTVector(-vNextGoalDir.z,0,vNextGoalDir.x);  // == vNextGoalDir.Cross(LTVector(0,1,0))
			vPerp.Norm();

			if( fabs(vPerp.Dot(local_goal_pos)) < g_fMinTolerance )
			{
				GetAIPtr()->SetNextNode(LTNULL);
				GetAIPtr()->SetLastNodeReached( m_pDestNode );
				fsm->AddMessage(Root::eReachedDest);
				return;
			}
		}
	}

	// Consider ourselves there if we are right on top of the goal that 
	// the avoidance algorithm is returning when it doesn't say we are
	// avoiding something.  This happens when another object is blocking our
	// destination.
	if( !m_bExactMovement && !m_bAvoidingObstacle )
	{
		if( goal.Equals(my_pos, 1.0f) )
		{
			GetAIPtr()->SetNextNode(LTNULL);
			GetAIPtr()->SetLastNodeReached( m_pDestNode );
			fsm->AddMessage(Root::eReachedDest);
			return;
		}
	}
	//
	// Set-up some useful constants.
	//

	// Lowering the forward dot product will cause the AI's to spend more
	// time turning before they start to move.
	const LTFLOAT fMinForwardDotProduct = (LTFLOAT)cos( MATH_DEGREES_TO_RADIANS(45) );


	// Get the goal in our relative coordinates, ignoring up/down offsets.
	const LTVector rel_proj_goal = projected_goal - my_pos;
	const LTFLOAT rel_proj_goal_mag = rel_proj_goal.Mag();

	//
	// Face the correct direction.
	//
	LTVector desired_facing = rel_proj_goal;
	desired_facing.Norm();


	if( !(desired_facing.MagSqr() > 0.99f && desired_facing.MagSqr() < 1.01f) )
	{
		desired_facing = my_forward;
	}

	// Turn to our desired facing.
	if( desired_facing.Dot(my_forward) > m_Butes.fCosMaxSnapTurnAngle )
	{
		GetAIPtr()->SnapTurn(desired_facing);
	}
	else
	{
		AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO) );
		if( pNewAction )
		{
			pNewAction->Init(desired_facing);

			m_pLastAction = LTNULL;
			return;
		}
	}

	
	// Be sure we don't leave our leash.
	// This restricts our movement so that we can move if the goal is outside our leash and it takes us
	// farther away from our leash.

	// 01/25/01 - MarkS
	// This was causing infinite recursion in some situations
	// It's already handled in AIPathMgr.cpp line 436 so I've removed it here.
//	if( !GetAIPtr()->WithinLeash( projected_goal ) && rel_proj_goal.Dot(GetAIPtr()->GetLeashPoint() - my_pos) < 0.0f )
//	{
//		m_RootFSM.AddMessage(Root::eStuck);
//		return;
//	}


	
	//
	// Do actual movement.
	//

	// Record our new facing. (It will be different if fully_turned is true.
	const LTVector & facing_forward = m_pMovement->GetForward();
	const LTVector & facing_up = m_pMovement->GetUp();

	// Determine our predicted offset and set-up the variables
	// needed to determine if we should move forward/backward and strafe left/right.

	// This action time is a best guess as to how long the AIActionMoveForward can
	// run.  If the AI's seem to be overshooting their goal and moving backward
	// to get back to the goal,  then this value should be increased.
	const LTFLOAT fActionTime = 2.0f;

	const LTFLOAT fMaxVel = m_pMovement->GetMaxSpeed((m_pMovement->GetCharacterVars()->m_dwFlags & CM_FLAG_RUN) ? CMS_RUNNING : CMS_WALKING);

	// The predicted offset is increased by 10% to make sure that it is
	// an over-estimate.  An under-estimate will cause us to overshoot when
	// we may not want to.
	const LTVector predicted_offset = facing_forward*fMaxVel*fActionTime*1.1f;
	const LTFLOAT  poffset_mag = predicted_offset.Mag();

	// This will be used in forward/reverse and strafe movements.
	LTVector rel_next_goal = next_goal - my_pos;
	rel_next_goal-= facing_up*(facing_up.Dot(rel_next_goal));

	LTVector rnext_goal_dir = rel_next_goal - predicted_offset;
	rnext_goal_dir.Norm();
	const LTFLOAT rnextgdir_dot_poffset = rnext_goal_dir.Dot(predicted_offset);

	// Forward movement.
	const LTFLOAT facing_dot_poffset = facing_forward.Dot(predicted_offset);
	const LTFLOAT facing_dot_rpgoal = facing_forward.Dot(rel_proj_goal);

	// Now move forward only if we are facing the goal within our forward facing range arc.
	// PLH 2-9-01  -- This should not be needed becuase all this is wrapped in the turn
	//    code above, but I expect to be asked to make the AI move without facing 
	//    the direction they are trying to go, so I'll need this.  
	//     I predict it will happen by 2-25-01.
	// PLH 2-28-01 -- I was wrong.  Things are looking good with the "do not move until your
	//           facing the desired direction" algorithm.  So this first conditional can
	//           can probably be removed.
	// PLH 9-17-01 -- This needs to be here so that AI's will not constantly try to move
	//                to a point they are right on top of (due to floating point error, 
	//                if the AI is right on top of the goal, facing_dot_rpgoal might by slightly
	//                above zero while rel_proj_goal_mag will be slightly less (or at) zero).
	if( (float)fabs(facing_dot_rpgoal - fMinForwardDotProduct*rel_proj_goal_mag) >= 0.001f )
	{
		// Go ahead and use a full forward if we have a next destination, 
		// and the next destination is in the same direction,
		// and we won't overshoot the next goal.
/*		if(  (    m_bCheckNextDest 
			   && rnextgdir_dot_poffset > m_Butes.fMinOvershootDotProduct*poffset_mag
			   && g_pAIVolumeMgr->FindContainingVolume(my_pos + predicted_offset, my_dims) ) )
//				|| !m_bExactMovement )
		{
			if( !dynamic_cast<AIActionMoveForward *>( GetAIPtr()->GetCurrentAction() ) )
			{
				AIActionMoveForward * pNewAction = dynamic_cast<AIActionMoveForward*>( GetAIPtr()->SetNextAction(CAI::ACTION_MOVEFORWARD) );
				if( pNewAction )
				{
					m_bMovedLaterally = true;
					m_pLastAction     = pNewAction;
					m_vLastActionGoal = goal;
					m_vLastActionPos  = my_pos;

					m_bMightOvershoot = true;

					if( ShouldShowMove(m_pMovement->GetObject()) )
					{
						LineSystem::GetSystem(this,"ShowMove") 
							<< LineSystem::Clear()
							<< LineSystem::Arrow( my_pos + my_up*20.0f, my_pos + predicted_offset + my_up*20.0f, Color::White);
					}
					return;
				}
			}
			else
			{
				// Just keep using our current move forward action.
				m_bMovedLaterally = true;
				m_pLastAction     = GetAIPtr()->GetCurrentAction();
				m_vLastActionGoal = goal;
				m_vLastActionPos  = my_pos;

				m_bMightOvershoot = true;
				return;
			}
		}
		else
*/		{
			AIActionMoveTo * pNewAction = dynamic_cast<AIActionMoveTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_MOVETO) );
			if( pNewAction )
			{
				pNewAction->Init( my_pos + rel_proj_goal );
			
				m_bMovedLaterally = true;
				m_pLastAction     = pNewAction;
				m_vLastActionGoal = goal;
				m_vLastActionPos  = my_pos;

				m_bMightOvershoot = false;

#ifndef _FINAL
				if( ShouldShowMove(m_pMovement->GetObject()) )
				{
					LineSystem::GetSystem(this,"ShowMove") 
						<< LineSystem::Clear()
						<< LineSystem::Arrow( my_pos + my_up*20.0f, my_pos + rel_proj_goal + my_up*20.0f, Color::Blue);
				}
#endif
				return;
			}

		}
	

	} //if( facing_dot_rpgoal > fMinForwardDotProduct*rel_proj_goal_mag )


#ifdef AIMOVEMENT_TIMING
//		const uint32 ticks = g_pServerDE->EndCounter(&full_counter);
//		AICPrint(GetAIPtr()->m_hObject," Movement code took %d ticks.", ticks);
#endif
				
} //void CAIStrategyMove::UpdateMoveTo(RootFSM * fsm)

void CAIStrategyMove::StopMoving(RootFSM * fsm)
{
	// This may be called on start-up, m_pMovement may truly be null.
	//ASSERT( m_pMovement );
	if( !m_pMovement ) return;

	m_pObstacleMgr->SetGoal( m_pMovement->GetPos(), m_pMovement );
}

// ----------------------------------------------------------------------- //
//
//	State:	WallWalkTo
//
//	PURPOSE:	Wall walks to a destination with a normal.
//
// ----------------------------------------------------------------------- //

void CAIStrategyMove::StartWallWalkTo(RootFSM * fsm)
{
	m_vStartPosition = GetAIPtr()->GetPosition();
	const LTFLOAT dest_dist = ManhattanDistance(m_vStartPosition,m_vDest);
	const LTFLOAT speed = GetAIPtr()->GetMovement()->GetMaxSpeed();
	m_fMoveToTime =  speed > 0 ? 2.5f*dest_dist/speed : 5.0f;

	m_pLastAction = LTNULL;
	m_vLastActionGoal = LTVector(0,0,0);
	m_vLastActionPos = LTVector(0,0,0);
	m_vStartingPos = GetAIPtr()->GetPosition();

	GetAIPtr()->WallWalk();
}


void CAIStrategyMove::UpdateWallWalkTo(RootFSM * fsm)
{
	if( GetAIPtr()->GetInjuredMovement() )
	{
		fsm->AddMessage(Root::eStuck);
		return;
	}

	m_bMovedLaterally = false; // Make sure stuck checking doesn't kick in!

	const LTVector & my_pos = m_pMovement->GetPos();
	const LTVector & my_up = m_pMovement->GetUp();
	const LTVector & my_forward = m_pMovement->GetForward();
	const LTVector & my_dims = m_pMovement->GetObjectDims();

	// FrameTime is capped at 0.2f in the engine.  So if the frame
	// time is that high, we can probably avoid doing this check.
	m_fMoveToTime -= m_pMovement->GetCharacterVars()->m_fFrameTime;

	if( m_fMoveToTime < 0.0f )
	{
#ifdef MILD_MOVE_DEBUG
#ifndef _DEMO
		AICPrint(GetAIPtr(), "Couldn't wall walk to (%.2f,%.2f,%.2f) in time, clipping from (%.2f,%.2f,%.2f).",
			m_vDest.x,m_vDest.y,m_vDest.z,
			my_pos.x,my_pos.y,my_pos.z );
#endif
#endif			
		fsm->AddMessage(Root::eNotMoving);
		return;
	}

	_ASSERT( m_vNormal.MagSqr() > 0.0f );



	// Check to see if we have reached our destination.

	// If we are within our radius of the destination, we're there!
	const LTFLOAT fNearEpsilon = my_dims.y*1.5f;
	if( NearDest( m_vDest, my_pos - my_up*my_dims.y, fNearEpsilon) )
	{
		GetAIPtr()->SetNextNode(LTNULL);
		GetAIPtr()->SetLastNodeReached( m_pDestNode );
		fsm->AddMessage(Root::eReachedDest);
		return;
	}

	// This is needed if the action is set to interruptable.
	if( m_pLastAction && m_pLastAction == GetAIPtr()->GetCurrentAction() )
	{
		return;
	}

#ifndef _FINAL
	if( ShouldShowMove(m_pMovement->GetObject() )  )
	{
		LineSystem::GetSystem(this,"Goal")
			<< LineSystem::Clear()
			<< LineSystem::Arrow( my_pos, my_pos + my_up*my_dims.y*2.0f, Color::Blue )
			<< LineSystem::Arrow( my_pos,my_pos + m_vFacingDir*50.0f, Color::Yellow )
			<< LineSystem::Box( my_pos + m_vFacingDir*50.0f, LTVector(3.0,3.0,3.0), Color::Yellow )
			<< LineSystem::Box( m_vDest, LTVector(2.5,2.5,2.5), Color::Cyan)
			<< LineSystem::Arrow( m_vDest, m_vDest + m_vNormal*100.0f, Color::Cyan )
			<< LineSystem::Line( m_vDest,my_pos, Color::Cyan );
	}
#endif

	// Always crouch.
	GetAIPtr()->WallWalk();

	// Be sure we don't leave our leash.
//	if( !GetAIPtr()->WithinLeash( goal ) && (m_vDest - my_pos).Dot(GetAIPtr()->GetLeashPoint() - my_pos) < 0.0f )
//	{
//		m_RootFSM.AddMessage(Root::eStuck);
//		return;
//	}

	// Move forward.

	AIActionWWForward * pNewAction = dynamic_cast<AIActionWWForward*>( GetAIPtr()->SetNextAction(CAI::ACTION_WWFORWARD) );
	if( pNewAction )
	{
		pNewAction->Init(m_vDest);

		// This stuff shouldn't be necessary, as AIActionWWForward is non-interruptable.
		m_pLastAction     = pNewAction;
		m_vLastActionGoal = m_vDest;
		m_vLastActionPos  = my_pos;

#ifndef _FINAL
		if( ShouldShowMove(m_pMovement->GetObject()) )
		{
			LineSystem::GetSystem(this,"ShowMove") 
				<< LineSystem::Arrow( my_pos + my_up*20.0f, my_pos + my_forward*100.0f, Color::White);
		}
#endif
		return;
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	State:	MoveToCorrect
//
//	PURPOSE:	Tries to correct when not moving (moves a perpindicular distance for awhile).
//
// ----------------------------------------------------------------------- //
	
void CAIStrategyMove::StartMoveToCorrection(RootFSM * fsm)
{
	const LTVector & my_pos = GetAIPtr()->GetPosition();

//	if( !GetAIPtr()->WithinLeash( m_vDest ) && (m_vDest - my_pos).Dot(GetAIPtr()->GetLeashPoint() - my_pos) < 0.0f )
//	{
//		m_RootFSM.AddMessage(Root::eStuck);
//		return;
//	}

	m_tmrCorrection.Start( GetRandom(m_Butes.fMinCorrectionTime,m_Butes.fMaxCorrectionTime) );

	m_vCorrectionDir = (m_vDest - m_vLastPosition).Cross(m_pMovement->GetUp());
	if( GetRandom(0,1) ) 
		m_vCorrectionDir *= -1.0f;

	const uint32 m_nMaxCorrections = 3;

	if( ++m_nNumCorrections > m_nMaxCorrections )
	{
		m_RootFSM.AddMessage(Root::eStuck);
	}
}

void CAIStrategyMove::UpdateMoveToCorrection(RootFSM * fsm)
{
	if( m_tmrCorrection.Stopped() )
	{
		// Reset the stuck checking code.
		m_bMovedLaterally = false;
		m_vLastPosition = m_pMovement->GetPos();
		m_fNextCheckTime = 0.4f; //PLH TODO : bute this?

		m_RootFSM.AddMessage(Root::eNewDest);
	}

	GetAIPtr()->Turn(m_vCorrectionDir);

	GetAIPtr()->Forward();

#ifndef _FINAL
	if( GetAIPtr() && ShouldShowMove(GetAIPtr()->m_hObject) )
	{
		const LTVector my_pos = m_pMovement->GetPos();
		LineSystem::GetSystem(this,"ShowMove") 
			<< LineSystem::Clear()
			<< LineSystem::Line( my_pos, my_pos + m_vCorrectionDir*100.0f, Color::Green );
	}
#endif
}

// ----------------------------------------------------------------------- //
//
//	State:	WWCorrection  (WallWalkCorrection)
//
//	PURPOSE:	Does the wall walking correction (clips to point!).
//
// ----------------------------------------------------------------------- //

void CAIStrategyMove::StartWWCorrection(RootFSM * fsm)
{
//	const LTVector & my_pos = GetAIPtr()->GetPosition();
//	if( !GetAIPtr()->WithinLeash( m_vDest ) && (m_vDest - my_pos).Dot(GetAIPtr()->GetLeashPoint() - my_pos) < 0.0f )
//	{
//		m_RootFSM.AddMessage(Root::eStuck);
//		return;
//	}

	_ASSERT( m_vNormal.MagSqr() > 0.5f && m_vNormal.MagSqr() < 1.4f );
	GetAIPtr()->ClipTo(m_vDest + m_vNormal*GetAIPtr()->GetMovement()->GetDimsSetting(CM_DIMS_WALLWALK).y*1.1f, m_vNormal);
}


void CAIStrategyMove::UpdateWWCorrection(RootFSM * fsm)
{
	GetAIPtr()->WallWalk();
	if( m_pMovement->GetMovementState() != CMS_SPECIAL )
	{
		GetAIPtr()->SetNextNode(LTNULL);
		GetAIPtr()->SetLastNodeReached( m_pDestNode );
		fsm->AddMessage(Root::eReachedDest);
	}

}

void CAIStrategyMove::EndWWCorrection(RootFSM * fsm)
{
}


// ----------------------------------------------------------------------- //
//
//	State:	WWCorrection  (WallWalkCorrection)
//
//	PURPOSE:	Does the wall walking correction (clips to point!).
//
// ----------------------------------------------------------------------- //
void CAIStrategyMove::StartStuck(RootFSM * fsm)
{
	if( m_tmrMinAvoidIgnoreableDelay.Stopped() )
		m_tmrMaxStuckDuration.Start(m_Butes.fMaxStuckDuration);
}

void CAIStrategyMove::UpdateStuck(RootFSM * fsm)
{
#ifndef _FINAL
	if( ShouldShowMove(m_pMovement->GetObject()) )
	{
		LineSystem::GetSystem(this,"ShowMove") 
			<< LineSystem::Box( GetAIPtr()->GetPosition(), GetAIPtr()->GetDims()*1.5f, Color::Red );
	}
#endif

	if( m_tmrMaxStuckDuration.Stopped() )
	{
		m_pObstacleMgr->AvoidIgnorable(false);
	}
	m_tmrMinAvoidIgnoreableDelay.Start(3.0f);

	LTVector vTemp = m_vDest;

	m_pObstacleMgr->Update(&vTemp, LTNULL, m_pMovement);
	if (!m_pObstacleMgr->IsStuck())
		fsm->AddMessage(Root::eNewDest);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyMove::Load
//
//	PURPOSE:	Restores the Movement
//
// ----------------------------------------------------------------------- //

void CAIStrategyMove::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAIStrategy::Load(hRead);

	// m_pMovement set in init.
	*hRead >> *m_pObstacleMgr;

	*hRead >> m_RootFSM;
	

	*hRead >> m_vDest;
	*hRead >> m_vNextDest;
	*hRead >> m_bExactMovement;
	*hRead >> m_vNormal;
	const uint32 nDestNodeID = hRead->ReadDWord();
	m_pDestNode = g_pAINodeMgr->GetNode( nDestNodeID );

	const uint32 dest_vol_id = hRead->ReadDWord();
	m_pDestVolume = g_pAIVolumeMgr->GetVolumePtr(dest_vol_id);

	*hRead >> m_bCheckNextDest;

	*hRead >> m_bMovedLaterally;
	*hRead >> m_bAvoidingObstacle;
	*hRead >> m_bMightOvershoot;
	*hRead >> m_bFitsInVolume;
	*hRead >> m_vLastPosition;
	*hRead >> m_fNextCheckTime;
	*hRead >> m_vStartPosition;
	*hRead >> m_fMoveToTime;
	
	*hRead >> m_tmrMaxStuckDuration;
	*hRead >> m_tmrMinAvoidIgnoreableDelay;

	*hRead >> m_nNumCorrections;
	*hRead >> m_tmrCorrection;
	*hRead >> m_vCorrectionDir;

	if( hRead->ReadByte() )
	{
		m_pLastAction = GetAIPtr()->GetCurrentAction();
	}
	else
	{
		m_pLastAction = LTNULL;
	}
	*hRead >> m_vLastActionGoal;
	*hRead >> m_vLastActionPos;
	*hRead >> m_vStartingPos;

	*hRead >> m_vLastUsedUp;
	*hRead >> m_vFacingDir;

	*hRead >> m_Butes.fAvoidancePathFrequency;
	*hRead >> m_Butes.fCosMaxSnapTurnAngle;
	*hRead >> m_Butes.fMaxCorrectionTime;
	*hRead >> m_Butes.fMaxStuckDuration;
	*hRead >> m_Butes.fMinCorrectionTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStrategyMove::Save
//
//	PURPOSE:	Saves the Movement
//
// ----------------------------------------------------------------------- //
void CAIStrategyMove::Save(HMESSAGEREAD hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAIStrategy::Save(hWrite);

	// m_pMovement set in init.
	*hWrite << *m_pObstacleMgr;

	*hWrite << m_RootFSM;
	

	*hWrite << m_vDest;
	*hWrite << m_vNextDest;
	*hWrite << m_bExactMovement;
	*hWrite << m_vNormal;
	hWrite->WriteDWord( m_pDestNode ? m_pDestNode->GetID() : CAINode::kInvalidID );

	hWrite->WriteDWord( m_pDestVolume ? m_pDestVolume->GetID() : CAIVolume::kInvalidID );

	*hWrite << m_bCheckNextDest;

	*hWrite << m_bMovedLaterally;
	*hWrite << m_bAvoidingObstacle;
	*hWrite << m_bMightOvershoot;
	*hWrite << m_bFitsInVolume;
	*hWrite << m_vLastPosition;
	*hWrite << m_fNextCheckTime;
	*hWrite << m_vStartPosition;
	*hWrite << m_fMoveToTime;
		
	*hWrite << m_tmrMaxStuckDuration;
	*hWrite << m_tmrMinAvoidIgnoreableDelay;

	*hWrite << m_nNumCorrections;
	*hWrite << m_tmrCorrection;
	*hWrite << m_vCorrectionDir;

	hWrite->WriteByte( m_pLastAction != LTNULL && m_pLastAction == GetAIPtr()->GetCurrentAction() );
	*hWrite << m_vLastActionGoal;
	*hWrite << m_vLastActionPos;
	*hWrite << m_vStartingPos;

	*hWrite << m_vLastUsedUp;
	*hWrite << m_vFacingDir;

	*hWrite << m_Butes.fAvoidancePathFrequency;
	*hWrite << m_Butes.fCosMaxSnapTurnAngle;
	*hWrite << m_Butes.fMaxCorrectionTime;
	*hWrite << m_Butes.fMaxStuckDuration;
	*hWrite << m_Butes.fMinCorrectionTime;
}



#ifdef STRATEGY_MOVE_DEBUG

void CAIStrategyMove::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateMoveTo : szMessage = "eStateMoveTo"; break;
		case Root::eStateWallWalkTo : szMessage = "eStateWallWalkTo"; break;
		case Root::eStateStuck : szMessage = "eStateStuck"; break;
		case Root::eStateDone : szMessage = "eStateDone"; break;
		case Root::eStateMoveToCorrection : szMessage = "eStateMoveToCorrection"; break;
		case Root::eStateWallWalkCorrection : szMessage = "eStateWallWalkCorrection"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Move::RootFSM is entering state %s.", szMessage );
}

void CAIStrategyMove::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eNewDest : szMessage = "eNewDest"; break;
		case Root::eNewWWDest  : szMessage = "eNewWWDest"; break;
		case Root::eReachedDest : szMessage = "eReachedDest"; break;
		case Root::eNotMoving : szMessage = "eNotMoving"; break;
		case Root::eStuck : szMessage = "eStuck"; break;
		case Root::eClear : szMessage = "eClear"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
				" Move::RootFSM is processing %s.", szMessage );
	if( !fsm.HasTransition(fsm.GetState(),message ) )
	{
		AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL, " Move::RootFSM has no transition for that message.");
	}
}

#else // !defined(STRATEGY_MOVE_DEBUG)

void CAIStrategyMove::PrintRootFSMState(const RootFSM & , Root::State )
{
}

void CAIStrategyMove::PrintRootFSMMessage(const RootFSM & , Root::Message )
{
}

#endif // STRATEGY_MOVE_DEBUG
