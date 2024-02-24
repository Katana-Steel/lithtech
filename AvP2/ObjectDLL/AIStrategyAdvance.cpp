// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyAdvance.cpp
//
// PURPOSE : This strategy causes the AI to move to the neighboring volume
//			 furthest from his target.  This continues until the enemy no 
//			 longer has LOS, or the AI is cornered.
//
// CREATED : 8.15.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AI.h"
#include "AIStrategyAdvance.h"
#include "AITarget.h"
#include "AIVolume.h"
#include "AINodeVolume.h"

#include "AINodeMgr.h"
#include "AIUtils.h"

#ifdef STRATEGY_ADVANCE_DEBUG
#include "DebugLineSystem.h"
#include "AIUtils.h"
#endif

CAIStrategyAdvance::CAIStrategyAdvance(CAI * pAI)
	: CAIStrategy(pAI),
	  m_StrategyFollowPath(pAI),
	  m_pCurNode(LTNULL),
	  m_bCover(LTFALSE)
{
	SetupFSM();
}

void CAIStrategyAdvance::Update()
{
	CAIStrategy::Update();

	if(!GetAIPtr())
	{
#ifndef _FINAL
		g_pInterface->CPrint("WARNING: pAI invalid");
#endif
		return;
	}

	m_RootFSM.Update();
}

void CAIStrategyAdvance::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateHiding,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyAdvance::UpdateHiding),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyAdvance::StartHiding) );

	m_RootFSM.DefineState( Root::eStateRunning,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyAdvance::UpdateRunning),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyAdvance::StartRunning) );

	m_RootFSM.DefineState( Root::eStateCornered,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyAdvance::UpdateCornered),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyAdvance::StartCornered) );

	// Default Transitions.
	m_RootFSM.DefineTransition(Root::eTargetInvalid, Root::eStateHiding);

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateRunning, Root::eTargetCannotSee, Root::eStateHiding);
	m_RootFSM.DefineTransition(Root::eStateRunning, Root::eCornered, Root::eStateCornered);

	m_RootFSM.DefineTransition(Root::eStateHiding, Root::eNotCornered, Root::eStateRunning);
	m_RootFSM.DefineTransition(Root::eStateHiding, Root::eCanSeeTarget, Root::eStateRunning);

	m_RootFSM.DefineTransition(Root::eStateCornered, Root::eNotCornered, Root::eStateRunning);

#ifdef STRATEGY_ADVANCE_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStrategyAdvance::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStrategyAdvance::PrintRootFSMMessage));
#endif

	m_RootFSM.SetInitialState( Root::eStateRunning );

}

void CAIStrategyAdvance::SetPath()
{
	CAINodeCover * pCover;
	LTVector vPos, vTarget;

	if (!GetAIPtr())
		return;

	if (!GetAIPtr()->GetTarget().IsValid())
		return;

	if (m_StrategyFollowPath.IsSet())
		return;

	// HACK
	// Path is not set, which means we've arrived at a destination
	// If the destination was a cover node, then the cover flag is set
	// This flag forces the AI to pause at the destination before resuming movement
	if (m_bCover && m_tmrCover.Stopped())
	{
		if (m_pCurNode && m_pCurNode->GetCrouch())
		{
			if (m_pCurNode->IsOccupied(GetAIPtr()->GetObject()))
			{
				GetAIPtr()->Crouch();
			}
		}

		m_tmrCover.Start(3.0f);
		m_bCover = LTFALSE;
	}

	GetAIPtr()->StandUp();

	pCover = g_pAINodeMgr->FindAdvanceNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());
	if( pCover && m_StrategyFollowPath.Set(pCover) )
	{
		if (m_pCurNode)
			m_pCurNode->Unlock();
		
		m_bCover = LTTRUE;
		pCover->Lock(GetAIPtr()->GetObject());

		m_pCurNode = pCover;
	}
	else
	{
		m_StrategyFollowPath.Set(GetAIPtr()->GetTarget().GetPosition());
		m_bCover = LTFALSE;
	}
	
	return;
}

void CAIStrategyAdvance::StartRunning(RootFSM * fsm)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget() )
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}
}

void CAIStrategyAdvance::UpdateRunning(RootFSM *)
{
	DVector vIntersect;

	if(!GetAIPtr() || !GetAIPtr()->HasTarget()) 
	{
		m_RootFSM.AddMessage(Root::eTargetInvalid);
		return;
	}

	const CAITarget & target = GetAIPtr()->GetTarget();

	// Can the target see me?

	if (!m_StrategyFollowPath.IsSet())
	{
		++g_cIntersectSegmentCalls;
		if (!TestLOSIgnoreCharacters(GetAIPtr()->GetObject(), target.GetObject()))
		{
			m_RootFSM.AddMessage(Root::eTargetCannotSee);
			return;
		}
	}

// Need a crouch timer before implementing this part
/*
	if ((m_pCurNode && !m_pCurNode->GetCrouch()) || (!m_pCurNode))
	{
		GetAIPtr()->StandUp();
	}
*/
	m_StrategyFollowPath.Update();
	SetPath();
}

void CAIStrategyAdvance::StartHiding(RootFSM * fsm)
{
	// TODO: bute this
	m_tmrHide.Start(0.5f);
}

void CAIStrategyAdvance::UpdateHiding(RootFSM * fsm)
{
	if (!GetAIPtr() || !GetAIPtr()->HasTarget())
	{
		// No target = no reason to run
		return;
	}

	const CAITarget & target = GetAIPtr()->GetTarget();
	
	// Check for a new position if we can see our target.
	if( target.IsPartiallyVisible() )
	{
		m_RootFSM.AddMessage(Root::eCanSeeTarget);
		return;
	}
	
	// TODO: add hearing and smell checks in here
	
	// Target has gone undetected for awhile, so move for the heck of it
	if (m_tmrHide.Stopped())
	{
		m_StrategyFollowPath.Set(target.GetPosition());
		m_bCover = LTFALSE;
		m_RootFSM.AddMessage(Root::eNotCornered);
	}

	return;
}

void CAIStrategyAdvance::StartCornered(RootFSM * fsm)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget() )
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}
}

void CAIStrategyAdvance::UpdateCornered(RootFSM * fsm)
{
	LTVector vIntersect;
	LTFLOAT fEnemyToIntersect, fEnemyToMe, fTemp;
	CAIVolume *pCurVol = NULL; 

	if (!GetAIPtr() || !GetAIPtr()->HasTarget())
	{
		m_RootFSM.AddMessage(Root::eTargetInvalid);
		return;
	}

	const CAITarget & target = GetAIPtr()->GetTarget();
	pCurVol = GetAIPtr()->GetCurrentVolumePtr();

	if (pCurVol)
	{
		fTemp = 0.0f;
		for(CAIVolume::NeighborNodeList::iterator iter = pCurVol->BeginNeighborNodes(); iter != pCurVol->EndNeighborNodes(); ++iter )
		{
			const CAIVolumeNeighborNode * pNode = dynamic_cast<const CAIVolumeNeighborNode *>( g_pAINodeMgr->GetNode(*iter) );
			if( pNode )
			{
				vIntersect = pNode->GetPos();
				
				fEnemyToIntersect = VEC_DISTSQR(vIntersect, target.GetPosition());
				fEnemyToMe = VEC_DISTSQR(target.GetPosition(), GetAIPtr()->GetPosition());
				
				// if there is a good retreat path, then we're no longer cornered
				if (fEnemyToIntersect > fEnemyToMe)
				{
					m_RootFSM.AddMessage(Root::eNotCornered);
					return;
				}
			}
		}
	}

	// No place to go
	// TODO: Add attack code here

	return;
}

void CAIStrategyAdvance::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAIStrategy::Load(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_StrategyFollowPath;
	*hRead >> m_tmrHide;
	*hRead >> m_bCover;
	*hRead >> m_tmrCover;

	uint32 dwID = hRead->ReadDWord();
	m_pCurNode = dynamic_cast<CAINodeCover*>( g_pAINodeMgr->GetNode(dwID) );
}

void CAIStrategyAdvance::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAIStrategy::Save(hWrite);

	*hWrite << m_RootFSM;
	*hWrite << m_StrategyFollowPath;
	*hWrite << m_tmrHide;
	*hWrite << m_bCover;
	*hWrite << m_tmrCover;

	hWrite->WriteDWord( m_pCurNode ? m_pCurNode->GetID() : CAINode::kInvalidID );
}


#ifdef STRATEGY_ADVANCE_DEBUG

void CAIStrategyAdvance::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateTargetVisible : szMessage = "eStateTargetVisible"; break;
		case Root::eStateLastTargetPosition : szMessage = "eStateLastTargetPosition"; break;
		case Root::eStateNextVolume : szMessage = "eStateNextVolume"; break;
		case Root::eStateTargetLost : szMessage = "eStateTargetLost"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Retreat::RootFSM is entering state %s.", szMessage );
}

void CAIStrategyAdvance::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eCanSeeTarget : szMessage = "eCanSeeTarget"; break;
		case Root::eCannotSeeTarget  : szMessage = "eCannotSeeTarget "; break;
		case Root::eAtDesiredLocation : szMessage = "eAtDesiredLocation"; break;
		case Root::eCannotReachDesiredLocation : szMessage = "eCannotReachDesiredLocation"; break;
		default : szMessage = "Unknown"; break;
	}

	if( fsm.HasTransition( fsm.GetState(), message ) )
	{
		AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Retreat::RootFSM is processing %s.", szMessage );
	}
}

#endif
