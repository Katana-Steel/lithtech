// ----------------------------------------------------------------------- //
//
// MODULE  : AIRetreat.cpp
//
// PURPOSE : Implements the Retreat goal and state.
//
// CREATED : 9/05/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AIRetreat.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "AIActionMisc.h"

/////////////////////////////////////////////////
//
//     Retreat Goal
//
/////////////////////////////////////////////////

RetreatGoal::RetreatGoal( CAI * pAI, std::string create_name )
	: AICombatGoal(pAI, create_name ),
	  m_StrategyRetreat(pAI),
	  m_StrategySetFacing(pAI)
{
	ASSERT( pAI );

	if( pAI )
	{
		m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
		ASSERT( m_pSeeEnemy );
	}

	m_StrategyRetreat.SetFlee(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RetreatGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void RetreatGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	AICombatGoal::Load(hRead);

	*hRead >> m_StrategyRetreat;
	*hRead >> m_StrategySetFacing;
	*hRead >> m_hTarget;
}


void RetreatGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	AICombatGoal::Save(hWrite);
	
	*hWrite << m_StrategyRetreat;
	*hWrite << m_StrategySetFacing;
	*hWrite << m_hTarget;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateRetreat::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT RetreatGoal::GetBid()
{
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(BID_RETREAT);
	LTFLOAT fBid = 0.0f;

	// If we are already active, stay active!
	if( IsActive() && GetAIPtr()->HasTarget() )
	{
		fBid = fDefault;
	}
	// Retreat if this AI is badly injured and can see its enemy
	else if( m_pSeeEnemy && m_pSeeEnemy->GetStimulation() > 0.0f )
	{
		{
			fBid = fDefault;
			m_hTarget = m_pSeeEnemy->GetStimulus();
		}
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateRetreat::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT RetreatGoal::GetMaximumBid()
{
	return g_pAIButeMgr->GetDefaultBid(BID_RETREAT);
}

void RetreatGoal::Start()
{
	AICombatGoal::Start();
	GetAIPtr()->Target( m_hTarget );
	GetAIPtr()->SetMovementStyle();
	GetAIPtr()->Run();
	GetAIPtr()->SetAnimExpression(EXPRESSION_SCARED);

	m_StrategyRetreat.Start();
}

void RetreatGoal::End()
{
	AICombatGoal::End();
	GetAIPtr()->SetAnimExpression(EXPRESSION_NORMAL);

	m_hTarget = LTNULL;
}

void RetreatGoal::Update()
{
	AICombatGoal::Update();

	m_StrategyRetreat.Update();
	m_StrategySetFacing.Update();

	if( m_StrategyRetreat.IsCornered() && !dynamic_cast<AIActionPlayAnim*>( GetAIPtr()->GetCurrentAction() ) )
	{
		AIActionPlayAnim * pPlayAnim = dynamic_cast<AIActionPlayAnim *>(GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM));
		if (pPlayAnim)
		{
			m_StrategySetFacing.FaceTarget();
			pPlayAnim->Init("Stand_Cower", AIActionPlayAnim::FLAG_FCTRG | AIActionPlayAnim::FLAG_LOOP );

		}
	}
	else
	{
		m_StrategySetFacing.Clear();
	}
}
