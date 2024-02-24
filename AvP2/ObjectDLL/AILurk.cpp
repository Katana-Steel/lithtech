// ----------------------------------------------------------------------- //
//
// MODULE  : Chase.h
//
// PURPOSE : Implements the Lurk goal and state.
//
// CREATED : 8/10/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AILurk.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "AIVolume.h"
#include "CharacterMgr.h"
#include "SoundButeMgr.h"

#ifdef _DEBUG
//#define STATE_LURK_DEBUG
#endif

/////////////////////////////////////////////////
//
//     Lurk State
//
/////////////////////////////////////////////////
static bool InWeaponRange(const AIWBM_AIWeapon * pWeaponProfile, LTFLOAT fTargetRangeSqr)
{
	if( pWeaponProfile )
	{
		if(fTargetRangeSqr <= pWeaponProfile->fMaxIdealRange*pWeaponProfile->fMaxIdealRange)
			return LTTRUE;
	}

	return LTFALSE;
}
 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateLurk::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

CAIStateLurk::CAIStateLurk(CAI * pAI)
	: CAIState(pAI),
	  m_pSeeEnemy(LTNULL),
	  m_StrategyApproachTarget(pAI),
	  m_StrategyRetreat(pAI),
	  m_bIsCornered(LTFALSE),
	  m_fLurkAttackRange(0.0f),
	  m_bWasAttacked(LTFALSE)
{
	ASSERT( pAI );
	if( pAI )
	{
		m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
		ASSERT( m_pSeeEnemy );
	}

	m_fLurkAttackRange = g_pAIButeMgr->GetTemplate(GetAIPtr()->GetTemplateId()).fLurkRange;

	SetupFSM();
}

CAIStateLurk::Root::State CAIStateLurk::GetNextState(LTBOOL bCanSeeTarget)
{
	const CAI &ai = *GetAIPtr();
	const LTFLOAT fLurkRangeSqr = m_fLurkAttackRange*m_fLurkAttackRange;

	typedef CAIStateLurk::Root State;

	if( !ai.HasTarget() )
	{
		return State::eStateLostTarget;
	}

	const CAITarget & target = ai.GetTarget();
	const LTFLOAT fTargetRangeSqr = (target.GetPosition() - ai.GetPosition()).MagSqr();

	if (target.IsFacing())
	{
		if (target.IsAttacking())
			m_bWasAttacked = LTTRUE;

		if ((fTargetRangeSqr <= fLurkRangeSqr) || m_bWasAttacked)
		{
			if( InWeaponRange(ai.GetCurrentWeaponButesPtr(), fTargetRangeSqr) )
				return State::eStateAttack;
			else
				return State::eStateAdvance;
		}
		else
		{
			if (!TestLOSIgnoreCharacters(ai.GetObject(), target.GetObject()))
				return State::eStateHide;
			else
				return State::eStateRetreat;
		}
	}
	else
	{
		if( fTargetRangeSqr <= fLurkRangeSqr )
		{
			if( InWeaponRange(ai.GetCurrentWeaponButesPtr(), fTargetRangeSqr) )
				return State::eStateAttack;
		}

		return State::eStateAdvance;
	}

	return State::eStateLostTarget;
}


void CAIStateLurk::Start()
{
	CAIState::Start();

	ASSERT( GetAIPtr() );
	if( !GetAIPtr() ) return;

	GetAIPtr()->SetMovementStyle("Combat");

	// flashlight is always off
	if (GetAIPtr()->IsFlashlightOn())
		GetAIPtr()->SetFlashlight(LTFALSE);

	m_StrategyRetreat.Start();
}

void CAIStateLurk::Update()
{
	CAIState::Update();

	if(!GetAIPtr())
	{
#ifdef _DEBUG
		g_pInterface->CPrint("WARNING: pAI invalid");
#endif
		return;
	}

	LTBOOL bCanSeeTarget = LTFALSE;
	if( GetAIPtr()->HasTarget() )
	{
		HOBJECT hTarget = GetAIPtr()->GetTarget().GetObject();
		if( m_pSeeEnemy->GetStimulation(hTarget) > 0.75f )
		{
			GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
			bCanSeeTarget = LTTRUE;
		}
	}

	Root::State eNextState = GetNextState(bCanSeeTarget);
	if( m_RootFSM.GetState() != eNextState )
	{
		m_RootFSM.SetNextState(eNextState);
	}


	m_RootFSM.Update();
}

void CAIStateLurk::End()
{
	GetAIPtr()->SetMovementStyle();
	m_RootFSM.SetState(Root::eStateLostTarget);
	GetAIPtr()->LookAt( LTNULL );
}

void CAIStateLurk::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateHide,  
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::UpdateHide),
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::StartHide) );

	m_RootFSM.DefineState( Root::eStateAdvance,  
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::UpdateAdvance),
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::StartAdvance) );

	m_RootFSM.DefineState( Root::eStateRetreat,  
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::UpdateRetreat),
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::StartRetreat),
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::EndRetreat)  );

	m_RootFSM.DefineState( Root::eStateAttack,  
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::UpdateAttack),
						   m_RootFSM.MakeCallback(*this, &CAIStateLurk::StartAttack) );

	m_RootFSM.DefineState( Root::eStateLostTarget, 
		                   m_RootFSM.MakeCallback() );


	m_RootFSM.SetInitialState( Root::eStateLostTarget );

	// StateLostTarget has no transitions.

#ifdef STATE_LURK_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStateLurk::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStateLurk::PrintRootFSMMessage));
#endif

}

void CAIStateLurk::StartHide(RootFSM * fsm)
{
	m_bIsCornered = LTFALSE;
}

void CAIStateLurk::UpdateHide(RootFSM *)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	GetAIPtr()->SnapTurn( target.GetPosition() - GetAIPtr()->GetPosition() );

	return;
}

void CAIStateLurk::StartAdvance(RootFSM * fsm)
{
	GetAIPtr()->Walk();
	m_StrategyApproachTarget.Reset();
}

void CAIStateLurk::UpdateAdvance(RootFSM *)
{
	m_StrategyApproachTarget.Update();
	return;
}

void CAIStateLurk::StartRetreat(RootFSM * fsm)
{
	GetAIPtr()->Run();
	m_StrategyRetreat.Start();
}

void CAIStateLurk::UpdateRetreat(RootFSM *)
{
	m_StrategyRetreat.Update();

	m_bIsCornered = m_StrategyRetreat.IsCornered();
}

void CAIStateLurk::EndRetreat(RootFSM * fsm)
{
}

void CAIStateLurk::StartAttack(RootFSM * fsm)
{
	GetAIPtr()->Run();

	// Make our attack sound
	const int nButes = GetAIPtr()->PlayExclamationSound("Attack");
	if( nButes >= 0 )
	{
		// Tell the other AI's of our battle cry!
		const SoundButes & sound_bute = g_pSoundButeMgr->GetSoundButes(nButes);
		for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs(); iter != g_pCharacterMgr->EndAIs(); ++iter )
		{
			(*iter)->GetSenseMgr().BattleCry(sound_bute,GetAIPtr());
		}
	}

	m_bIsCornered = LTFALSE;
}

void CAIStateLurk::UpdateAttack(RootFSM *)
{
	if( !GetAIPtr()->GetCurrentWeaponButesPtr() )
	{
		return;
	}

	if (GetAIPtr()->GetCurrentWeaponButesPtr()->bMeleeWeapon)
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_FIREMELEE);
	}
	else
	{
		if( GetAIPtr()->GetCurrentWeaponPtr() && GetAIPtr()->GetCurrentWeaponButesPtr() )
		{
			if( NeedReload( *GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr() ) )
			{
				GetAIPtr()->SetNextAction(CAI::ACTION_RELOAD);
				return;
			}
		
			if (!GetAIPtr()->WillHurtFriend())
			{
				GetAIPtr()->SetNextAction(CAI::ACTION_FIREWEAPON);
				return;
			}
		}

		if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
		{
			GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
		}
	}
	return;
}




//------------------------------------------------------------------
// Debugging stuff
//------------------------------------------------------------------
#ifdef STATE_LURK_DEBUG

void CAIStateLurk::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateHide : szMessage = "eStateHide"; break;
		case Root::eStateAdvance : szMessage = "eStateAdvance"; break;
		case Root::eStateRetreat : szMessage = "eStateRetreat"; break;
		case Root::eStateAttack : szMessage = "eStateAttack"; break;
		case Root::eStateLostTarget : szMessage = "eStateLostTarget"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Lurk::RootFSM is entering state %s.", szMessage );
}

void CAIStateLurk::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eCanSeeTarget : szMessage = "eCanSeeTarget"; break;
		case Root::eCannotSeeTarget  : szMessage = "eInRange "; break;
		case Root::eInRange : szMessage = "eAtDesiredLocation"; break;
		case Root::eOutOfRange : szMessage = "eOutOfRange"; break;
		case Root::eTargetFacing : szMessage = "eTargetFacing"; break;
		case Root::eTargetNotFacing : szMessage = "eTargetNotFacing"; break;
		case Root::eTargetInvalid : szMessage = "eTargetInvalid"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Lurk::RootFSM is processing %s.", szMessage );
}

#else

void CAIStateLurk::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
}

void CAIStateLurk::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{
}

#endif
//------------------------------------------------------------------

void CAIStateLurk::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);

	CAIState::Load(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_StrategyApproachTarget;
	*hRead >> m_StrategyRetreat;
	*hRead >> m_bIsCornered;
	*hRead >> m_fLurkAttackRange;
	*hRead >> m_bWasAttacked;
}

void CAIStateLurk::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);

	CAIState::Save(hWrite);

	*hWrite << m_RootFSM;
	*hWrite << m_StrategyApproachTarget;
	*hWrite << m_StrategyRetreat;
	*hWrite << m_bIsCornered;
	*hWrite << m_fLurkAttackRange;
	*hWrite << m_bWasAttacked;
}


/////////////////////////////////////////////////
//
//     Lurk Goal
//
/////////////////////////////////////////////////


LurkGoal::LurkGoal( CAI * pAI, std::string create_name )
	: AICombatGoal(pAI, create_name ),
	  lurkState(pAI)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LurkGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool LurkGoal::HandleParameter(const char * const * pszArgs, int nArgs)
{
	return lurkState.HandleParameter(pszArgs,nArgs);
}

void LurkGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	AICombatGoal::Load(hRead);

	*hRead >> lurkState;
	*hRead >> m_hTarget;
}


void LurkGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	AICombatGoal::Save(hWrite);
	
	*hWrite << lurkState;
	*hWrite << m_hTarget;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateLurk::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT LurkGoal::GetBid()
{
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(BID_LURK);
	LTFLOAT fBid = 0.0f;

	// If we have a target, keep lurking!
	if( GetAIPtr()->HasTarget() )
	{
		return fDefault;
	}

	if (!IsActive())
	{
		NewTarget new_target = FindNewTarget();
		if( new_target.hTarget )
		{
			if(    new_target.eReason == NewTarget::SeeEnemy
				|| new_target.eReason == NewTarget::Damage   )
			{
				fBid = fDefault;
				m_hTarget = new_target.hTarget;
			}
		}
	}
	else
	{
		if (GetAIPtr()->HasTarget() && !lurkState.LostTarget())
			fBid = fDefault;
		else
			fBid = 0.0f;
	}

/*
	else if( hearEnemyFootstep && hearEnemyFootstep->GetStimulation() == 1.0f )
	{
		valid = true;
		m_hTarget = hearEnemyFootstep->GetStimulus();
	}
	else if( hearWeaponFire && hearEnemyFootstep->GetStimulation() == 1.0f )
	{
		valid = true;
		m_hTarget = hearWeaponFire->GetStimulus();
	}
*/		 

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateLurk::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT LurkGoal::GetMaximumBid()
{
	return g_pAIButeMgr->GetDefaultBid(BID_LURK);
}

void LurkGoal::Start()
{
	AICombatGoal::Start();

	GetAIPtr()->Target( m_hTarget );

	lurkState.Start();
}

void LurkGoal::End()
{
	AICombatGoal::End();

	m_hTarget = LTNULL;

	lurkState.End();
}
