// ----------------------------------------------------------------------- //
//
// MODULE  : Snipe.cpp
//
// PURPOSE : Implements the Snipe goal and state.
//
// CREATED : 6/14/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AISnipe.h"
#include "AI.h"
#include "AITarget.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "CharacterMgr.h"
#include "AINodeMgr.h"
#include "AIActionMisc.h"

#ifdef _DEBUG
#include "AIUtils.h"
//#define SNIPE_DEBUG
#endif
 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateSnipe::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

CAIStateSnipe::CAIStateSnipe(CAI * pAI)
	: CAIState(pAI),
	  m_StrategyFollowPath(pAI),
	  m_pCurNode(NULL),
	  m_nNumNodes(0),
	  m_hAINodeGroup(LTNULL)
{
	_ASSERT(pAI);
	if ( !pAI ) return;

	m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
	ASSERT( m_pSeeEnemy );

	m_pHearEnemyFootstep = dynamic_cast<CAISenseHearEnemyFootstep *>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyFootstep));
	ASSERT(m_pHearEnemyFootstep);

	m_pHearEnemyWeaponFire = dynamic_cast<CAISenseHearEnemyWeaponFire *>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire));
	ASSERT(m_pHearEnemyWeaponFire);

	m_pHearWeaponImpact = dynamic_cast<CAISenseHearWeaponImpact *>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearWeaponImpact));
	ASSERT(m_pHearEnemyWeaponFire);

	m_pSeeEnemyFlashlight = dynamic_cast<CAISenseSeeEnemyFlashlight *>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemyFlashlight));
	ASSERT(m_pSeeEnemyFlashlight);

	
	SetupFSM();
}


bool CAIStateSnipe::HandleParameter(const char * const * pszArgs, int nArgs)
{

	if (0 == stricmp(*pszArgs, "NG"))
	{
		++pszArgs;
		--nArgs;

		m_hAINodeGroup = LTNULL;

		if (nArgs == 1)
		{
			// Find the AINodeGroup (if any)
			if( *pszArgs && **pszArgs )
			{
				ObjArray<HOBJECT,1> objArray;
				if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( *pszArgs ), objArray ) )
				{
					if( objArray.NumObjects() > 0 )
					{
						m_hAINodeGroup = objArray.GetObject(0);
					}
				}
				else
				{
#ifndef _FINAL
					g_pLTServer->CPrint("Could not find an appropriate AINodeGroup object");
#endif
				}
			}
		}
	}

	return true;
}


void CAIStateSnipe::Start()
{
	CAIState::Start();

	ASSERT( GetAIPtr() );
	if( !GetAIPtr() ) return;

	GetAIPtr()->Run();
	m_StrategyFollowPath.Clear();

	m_nNumNodes = g_pAINodeMgr->GetNumSnipeNodes();

	// Unlock the our current node so we "find" it again.
	// Otherwise the AI will go to a new node everytime this
	// goal is entered.
	if( m_pCurNode )
	{
		m_pCurNode->Unlock();
		m_pCurNode = LTNULL;
	}

	// flashlight is always off
	if (GetAIPtr()->IsFlashlightOn())
		GetAIPtr()->SetFlashlight(LTFALSE);

	GetAIPtr()->SetMovementStyle("Combat");
	m_RootFSM.SetState(Root::eStateFindNode);
}

void CAIStateSnipe::Update()
{
	CAIState::Update();

	m_RootFSM.Update();
}

void CAIStateSnipe::End()
{
	GetAIPtr()->SetMovementStyle();
	GetAIPtr()->ClearAim();
}

void CAIStateSnipe::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateFindNode,  
						  m_RootFSM.MakeCallback(*this, &CAIStateSnipe::UpdateFindNode),
						  m_RootFSM.MakeCallback(*this, &CAIStateSnipe::StartFindNode));

	m_RootFSM.DefineState(Root::eStateFindTarget,  
						  m_RootFSM.MakeCallback(*this, &CAIStateSnipe::UpdateFindTarget),
						  m_RootFSM.MakeCallback(*this, &CAIStateSnipe::StartFindTarget));

	m_RootFSM.DefineState(Root::eStateAttack,  
						  m_RootFSM.MakeCallback(*this, &CAIStateSnipe::UpdateAttack),
						  m_RootFSM.MakeCallback(*this, &CAIStateSnipe::StartAttack),
						  m_RootFSM.MakeCallback(*this, &CAIStateSnipe::EndAttack) );

	// Default Transitions
	m_RootFSM.DefineTransition(Root::eNoTarget, Root::eStateFindTarget);
	m_RootFSM.DefineTransition(Root::eCannotSeeTarget, Root::eStateFindNode);

	// Specific Transitions
	m_RootFSM.DefineTransition(Root::eStateFindNode, Root::eAtNode, Root::eStateFindTarget);
	m_RootFSM.DefineTransition(Root::eStateFindTarget, Root::eHasTarget, Root::eStateAttack);
	m_RootFSM.DefineTransition(Root::eStateAttack, Root::eUnderFire, Root::eStateFindNode);

#ifdef SNIPE_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStateSnipe::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStateSnipe::PrintRootFSMMessage));
#endif

}

void CAIStateSnipe::StartFindNode(RootFSM * fsm)
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	// Find a good node, lock it, and go to it.
	// If we don't currently have a node, just find the one closest
	// to us.  Otherwise find the one closest to our target.
	HOBJECT hTarget = LTNULL;
	if( m_pCurNode )
		hTarget = GetAIPtr()->GetTarget().GetObject();

	CAINodeSnipe *pSnipeNode = g_pAINodeMgr->FindSnipeNode(GetAIPtr()->GetObject(), hTarget, m_hAINodeGroup);
	if (pSnipeNode && m_StrategyFollowPath.Set(pSnipeNode, (hTarget == LTNULL) ) )
	{
		if (m_pCurNode)
			m_pCurNode->Unlock();
		
		pSnipeNode->Lock(GetAIPtr()->GetObject());
		m_pCurNode = pSnipeNode;
	}

	return;
}

void CAIStateSnipe::UpdateFindNode(RootFSM * fsm)
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	if (m_StrategyFollowPath.IsDone() || m_StrategyFollowPath.IsUnset() )
	{
		m_tmrHoldPos.Start(g_pAIButeMgr->GetTemplate(GetAIPtr()->GetTemplateId()).fSnipeTime);
		fsm->AddMessage(Root::eAtNode);
		return;
	}

	m_StrategyFollowPath.Update();

	return;
}
	
void CAIStateSnipe::StartFindTarget(RootFSM * fsm)
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;
}

void CAIStateSnipe::UpdateFindTarget(RootFSM * fsm)
{
	CAISense *pSense = LTNULL;

	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	if (GetAIPtr()->HasTarget())
	{
		fsm->AddMessage(Root::eHasTarget);
		return;
	}

	// Use all active senses to locate a target (least to most important)
	if (m_pSeeEnemyFlashlight && m_pSeeEnemyFlashlight->GetStimulation() >= 1.0f)
		pSense = m_pSeeEnemyFlashlight;
	if (m_pHearEnemyWeaponFire && m_pHearEnemyWeaponFire->GetStimulation() >= 1.0f)
		pSense = m_pHearEnemyWeaponFire;
	if (m_pHearWeaponImpact && m_pHearWeaponImpact->GetStimulation() >= 1.0f)
		pSense = m_pHearWeaponImpact;
	if (m_pSeeEnemy && m_pSeeEnemy->GetStimulation() >= 1.0f)
		pSense = m_pSeeEnemy;
	
	// This ranks highest because it only happens when someone is very close.
	// We want to kill people who sneak up on us.
	if (m_pHearEnemyFootstep && m_pHearEnemyFootstep->GetStimulation() >= 1.0f)
		pSense = m_pHearEnemyFootstep;

	if (pSense && (GetAIPtr()))
	{
		GetAIPtr()->Target(pSense->GetStimulus(), pSense->GetStimulusPos());
		fsm->AddMessage(Root::eHasTarget);
		return;
	}
/*
	else
	{
		// TODO: This is badness (see AI.cpp ln 4241).  Need a way to clear the target.
		GetAIPtr()->Target(LTNULL);
		fsm->AddMessage(Root::eNoTarget);

		// also causes the max_recursion to get hit
	}
*/

	if( GetAIPtr()->CanDoNewAction() )
	{
		if( m_pCurNode && m_pCurNode->GetForward().Dot(GetAIPtr()->GetForward()) < 0.9f )
		{
			AIActionTurnTo * pTurnAction = dynamic_cast<AIActionTurnTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO) );
			if( pTurnAction )
				pTurnAction->Init( m_pCurNode->GetForward() );
		}
	}
}

void CAIStateSnipe::StartAttack(RootFSM * fsm)
{
	// Turn to face our target.
	if( GetAIPtr()->HasTarget() )
	{
		GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );

		const LTVector vAimDir = GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetAimFromPosition();

		const LTFLOAT aim_dot_forward = vAimDir.Dot(GetAIPtr()->GetForward());
		if(    !GetAIPtr()->AllowSnapTurn()
			|| aim_dot_forward < 0.0f 
			|| aim_dot_forward*aim_dot_forward < c_fCos35*c_fCos35*vAimDir.MagSqr() )
		{
			if( GetAIPtr()->CanDoNewAction() )
			{
				AIActionTurnTo * pTurnAction = dynamic_cast<AIActionTurnTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO) );
				if( pTurnAction )
				{
					pTurnAction->Init( vAimDir );
					return;
				}
			}
		}
		else
		{
			GetAIPtr()->SnapTurn(vAimDir);
		}
	}

	return;
}

void CAIStateSnipe::UpdateAttack(RootFSM * fsm)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	if (target.IsPartiallyVisible())
	{
		const AIWBM_AIWeapon * pWeaponProfile = GetAIPtr()->GetCurrentWeaponButesPtr();
		if (pWeaponProfile)
		{
			const LTFLOAT fRangeSqr = (target.GetPosition() - GetAIPtr()->GetPosition()).MagSqr();
	
			// Only fire if within range.
			if (fRangeSqr > pWeaponProfile->fMinFireRange*pWeaponProfile->fMinFireRange
				&& fRangeSqr < pWeaponProfile->fMaxFireRange*pWeaponProfile->fMaxFireRange)
			{
				// Only fire if target can be seen.
				if (target.IsFullyVisible())
				{
					FireOnTarget();
				}
			}
		}
	}
	else
	{
		// No target visible for awhile, so try moving to a new node
		if (m_tmrHoldPos.Stopped())
		{
			m_RootFSM.AddMessage(Root::eCannotSeeTarget);
			return;
		}
	}

	// Invalidate the current node if fired upon
	if (target.IsFacing() && target.IsAttacking() && m_pCurNode)
	{
		// can't move between nodes unless we have more than 1
		if (m_nNumNodes > 1)
		{
			// Don't move out of the snipe point too often
			if (m_tmrHoldPos.Stopped())
			{
				m_RootFSM.AddMessage(Root::eUnderFire);
				return;
			}
		}
	}

	if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
	{
		const CWeapon * pWeapon = GetAIPtr()->GetCurrentWeaponPtr();
		const AIWBM_AIWeapon * pWeaponProfile = GetAIPtr()->GetCurrentWeaponButesPtr();
		if( pWeapon && pWeaponProfile && NeedReload(*pWeapon,*pWeaponProfile) )
		{
			GetAIPtr()->SetNextAction(CAI::ACTION_RELOAD);
			return;
		}
		else
		{
			GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
		}
	}

	return;
}


void CAIStateSnipe::FireOnTarget()
{
	if (GetAIPtr()->HasTarget() && GetAIPtr()->BurstDelayExpired() )
	{
		// Must have full SeeEnemy stimulation in order to fire at a cloaked enemy
		if (GetAIPtr()->GetTarget().GetCharacterPtr()->GetCloakState() != CLOAK_STATE_INACTIVE)
		{
			if (!GetAIPtr()->GetTarget().IsFullyVisible())
				return;
		}

		if (!GetAIPtr()->WillHurtFriend())
		{
			GetAIPtr()->SetNextAction( CAI::ACTION_FIREWEAPON );
			return;
		}
	}
}

void CAIStateSnipe::EndAttack(RootFSM * fsm)
{
	GetAIPtr()->LookAt( LTNULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateSnipe::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIStateSnipe::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);

	CAIState::Load(hRead);

	*hRead >> m_RootFSM;

	// m_pSeeEnemy is set in the constructor.
	*hRead >> m_StrategyFollowPath;

	const uint32 snipe_node_id = hRead->ReadDWord();
	m_pCurNode = dynamic_cast<CAINodeSnipe*>(g_pAINodeMgr->GetNode( snipe_node_id ));
	*hRead >> m_nNumNodes;
	*hRead >> m_tmrHoldPos;
	*hRead >> m_hAINodeGroup;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIStateSnipe::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);

	CAIState::Save(hWrite);

	*hWrite << m_RootFSM;
	
	// m_pSeeEnemy is set in the constructor.
	*hWrite << m_StrategyFollowPath;

	hWrite->WriteDWord( m_pCurNode ? m_pCurNode->GetID() : CAINode::kInvalidID );
	*hWrite << m_nNumNodes;
	*hWrite << m_tmrHoldPos;
	*hWrite << m_hAINodeGroup;
}




#ifdef SNIPE_DEBUG


void CAIStateSnipe::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateFindNode : szMessage = "eStateFindNode"; break;
		case Root::eStateFindTarget : szMessage = "eStateFindTarget"; break;
		case Root::eStateAttack : szMessage = "eStateTargetVisible"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Move::RootFSM is entering state %s.", szMessage );
}

void CAIStateSnipe::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	if( fsm.HasTransition(fsm.GetState(), message) )
	{
		const char * szMessage = 0;
		switch( message )
		{
			case Root::eAtNode : szMessage = "eAtNode"; break;
			case Root::eHasTarget  : szMessage = "eHasTarget"; break;
			case Root::eUnderFire : szMessage = "eUnderFire"; break;
			case Root::eNoTarget : szMessage = "eNoTarget"; break;
			default : szMessage = "Unknown"; break;
		}

		AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Move::RootFSM is processing %s.", szMessage );
	}
}

#endif







/////////////////////////////////////////////////
//
//     Snipe Goal
//
/////////////////////////////////////////////////

SnipeGoal::SnipeGoal( CAI * pAI, std::string create_name )
	: AICombatGoal(pAI, create_name),
	  snipeState(pAI),
	  //m_hTarget
	  m_vTargetPos(0,0,0),
	  m_fMinTargetRangeSqr(0.0f)
{
	ASSERT( pAI );

	if( pAI )
	{
		m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
		ASSERT( m_pSeeEnemy );
	}
}

SnipeGoal::~SnipeGoal()
{
}

bool SnipeGoal::HandleParameter(const char * const * pszArgs, int nArgs)
{
	if( 0 == stricmp(*pszArgs,"MinRange") )
	{
		++pszArgs;
		--nArgs;

		if( nArgs == 1 )
		{
			m_fMinTargetRangeSqr = (LTFLOAT)atof(*pszArgs);
			m_fMinTargetRangeSqr *= m_fMinTargetRangeSqr;
		}
		else
		{
#ifndef _FINAL
			g_pLTServer->CPrint("AI \"%s\" given snipe goal with invalid AttackRange value.");
#endif
		}

		return true;
	}

	return snipeState.HandleParameter(pszArgs,nArgs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SnipeGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void SnipeGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	Goal::Load(hRead);

	*hRead >> snipeState;
	// m_pSeeEnemy saved by AI
	// m_pHearEnemyFootstep saved by AI
	// m_pHearEnemyWeaponFire saved by AI
	*hRead >> m_hTarget;
	*hRead >> m_vTargetPos;
	*hRead >> m_fMinTargetRangeSqr;
}


void SnipeGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	Goal::Save(hWrite);
	
	*hWrite << snipeState;
	// m_pSeeEnemy set at construction
	// m_pHearEnemyFootstep set at construction
	// m_pHearEnemyWeaponFire set at construction
	*hWrite << m_hTarget;
	*hWrite << m_vTargetPos;
	*hWrite << m_fMinTargetRangeSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateSnipe::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT SnipeGoal::GetBid()
{
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(BID_SNIPE);
	LTFLOAT fBid = 0.0f;

	// Gotta have nodes to use them
	if (g_pAINodeMgr->GetNumSnipeNodes() > 0)
	{
		fBid = fDefault;
	}

	// Invalidate if the target is too close.
	if((fBid > 0.0f) && GetAIPtr()->HasTarget())
	{
		const LTFLOAT fTargetRangeSqr = (GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition()).MagSqr();

		if (m_fMinTargetRangeSqr < fTargetRangeSqr)
			fBid = fDefault;
		else
			fBid = 0.0f;
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateSnipe::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT SnipeGoal::GetMaximumBid()
{
	return g_pAIButeMgr->GetDefaultBid(BID_SNIPE);
}

void SnipeGoal::Start()
{
	AICombatGoal::Start();
	
	// This goal can be started without a target.
	if( m_hTarget )
	{
		GetAIPtr()->Target( m_hTarget, m_vTargetPos );
	}

	snipeState.Start();
}

void SnipeGoal::End()
{
	AICombatGoal::End();

	m_hTarget = LTNULL;
	m_vTargetPos = LTVector(0,0,0);

	snipeState.End();
}
