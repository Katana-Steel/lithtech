// ----------------------------------------------------------------------- //
//
// MODULE  : Chase.h
//
// PURPOSE : Implements the Chase goal and state.
//
// CREATED : 5/26/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AIChase.h"
#include "AI.h"
#include "AITarget.h"
#include "AISenseMgr.h"

/////////////////////////////////////////////////
//
//     Chase State
//
/////////////////////////////////////////////////


 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateChase::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

CAIStateChase::CAIStateChase(CAI * pAI)
	: CAIState(pAI),
	  m_StrategyApproachTarget(pAI)
{
	ASSERT(pAI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateChase::Start
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAIStateChase::Start()
{
	CAIState::Start();

	GetAIPtr()->SetMovementStyle();
	GetAIPtr()->Run();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateChase::Update
//
//	PURPOSE:	Update of the state
//
// ----------------------------------------------------------------------- //

void CAIStateChase::Update()
{
	CAIState::Update();

	m_StrategyApproachTarget.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateChase::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIStateChase::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);

	CAIState::Load(hRead);

	*hRead >> m_StrategyApproachTarget;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStateChase::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIStateChase::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);

	CAIState::Save(hWrite);

	*hWrite << m_StrategyApproachTarget;
}




/////////////////////////////////////////////////
//
//     Chase Goal
//
/////////////////////////////////////////////////


ChaseGoal::ChaseGoal( CAI * pAI, std::string create_name )
	: Goal(pAI, create_name ),
	  chaseState(pAI)
{
	ASSERT( pAI );

	if( pAI )
	{
		seeEnemy = dynamic_cast<CAISenseSeeEnemy*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
		ASSERT( seeEnemy );

		hearEnemyFootstep = dynamic_cast<CAISenseHearEnemyFootstep*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyFootstep));
		ASSERT( hearEnemyFootstep );

		hearEnemyWeaponFire = dynamic_cast<CAISenseHearEnemyWeaponFire*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire));
		ASSERT( hearEnemyWeaponFire );
	}
}

bool ChaseGoal::HandleParameter(const char * const * pszArgs, int nArgs)
{
	return chaseState.HandleParameter(pszArgs,nArgs);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChaseGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //



void ChaseGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	Goal::Load(hRead);

	*hRead >> chaseState;
	// seeEnemy saved by AI
	// hearEnemyFootstep saved by AI
	// hearEnemyWeaponFire saved by AI
	*hRead >> m_hTarget;
}


void ChaseGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	Goal::Save(hWrite);
	
	*hWrite << chaseState;
	// seeEnemy set at construction
	// hearEnemyFootstep set at construction
	// hearEnemyWeaponFire set at construction
	*hWrite << m_hTarget;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateChase::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT ChaseGoal::GetBid()
{
	const LTFLOAT fDefault = GetMaximumBid();
	LTFLOAT fBid = 0.0f;

	if( GetAIPtr()->HasTarget() )
		return fDefault;

	if( seeEnemy && seeEnemy->GetStimulation() == 1.0f )
	{
		fBid = fDefault;
		m_hTarget = seeEnemy->GetStimulus();
	}
	else if( hearEnemyFootstep && hearEnemyFootstep->GetStimulation() == 1.0f )
	{
		fBid = fDefault;
		m_hTarget = hearEnemyFootstep->GetStimulus();
	}
	else if( hearEnemyWeaponFire && hearEnemyFootstep->GetStimulation() == 1.0f )
	{
		fBid = fDefault;
		m_hTarget = hearEnemyWeaponFire->GetStimulus();
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateChase::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT ChaseGoal::GetMaximumBid()
{
	return 65.0f;
}


void ChaseGoal::Start()
{
	Goal::Start();
	GetAIPtr()->Target( m_hTarget );

	chaseState.Start();
}

void ChaseGoal::End()
{
	Goal::End();

	m_hTarget = LTNULL;

	chaseState.End();
}


