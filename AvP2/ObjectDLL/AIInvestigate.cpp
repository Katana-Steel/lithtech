// ----------------------------------------------------------------------- //
//
// MODULE  : AIInvestigate.cpp
//
// PURPOSE : Implements the Investigate goal and state.
//
// CREATED : 9/05/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AIInvestigate.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "AIVolume.h"
#include "CharacterMgr.h"
#include "AIActionMisc.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "AINodeVolume.h"
#include "BodyProp.h"
#include "ServerSoundMgr.h"
#include "CharacterFuncs.h"
#include "ParseUtils.h"
#include "AIAnimButeMgr.h"

#ifdef _DEBUG
//#define INVESTIGATE_DEBUG
#endif

extern const char * g_szInvestigate;  // This is found in AI.cpp.

ILTMessage & operator<<(ILTMessage & out, InvestigateGoal::SuspectType eSuspectType)
{
	out.WriteByte(eSuspectType);
	return out;
}

ILTMessage & operator>>(ILTMessage & in, InvestigateGoal::SuspectType eSuspectType)
{
	eSuspectType = InvestigateGoal::SuspectType(in.ReadByte());
	return in;
}
		
/////////////////////////////////////////////////
//
//     Investigate Goal
//
/////////////////////////////////////////////////


InvestigateGoal::InvestigateGoal( CAI * pAI, std::string create_name )
	: Goal(pAI, create_name ),
	  m_pSeeDeadBody(NULL),
	  m_pHearEnemyWeaponFire(NULL),
	  m_pHearAllyWeaponFire(NULL),
	  m_pHearTurretWeaponFire(NULL),
	  m_pHearAllyPain(NULL),
	  m_pHearDeath(NULL),
	  m_pHearWeaponImpact(NULL),
	  m_pHearEnemyFootstep(NULL),
	  m_pHearEnemyTaunt(NULL),
	  m_pSeeBlood(NULL),
	  m_pSeeCloaked(NULL),
	  m_butes( g_pAIButeMgr->GetInvestigate( pAI ? pAI->GetTemplateId() : 0) ),
  	  m_StrategyFollowPath(pAI),
	  m_pSense(LTNULL),
	  m_vDest(0,0,0),
	  m_eSuspectType(Nothing),
	  // m_tmrDelay
	  m_bTriggered(LTFALSE),
	  m_vTriggeredDest(0,0,0),
	  m_pDeadBody(LTNULL)
{
	ASSERT( pAI );

	if( pAI )
	{
		m_pSeeDeadBody = dynamic_cast<CAISenseSeeDeadBody*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeDeadBody));
		ASSERT( m_pSeeDeadBody );

		m_pHearEnemyFootstep = dynamic_cast<CAISenseHearEnemyFootstep*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyFootstep));
		ASSERT( m_pHearEnemyFootstep );

		m_pHearEnemyTaunt = dynamic_cast<CAISenseHearEnemyTaunt*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyTaunt));
		ASSERT( m_pHearEnemyTaunt );

		m_pHearEnemyWeaponFire = dynamic_cast<CAISenseHearEnemyWeaponFire*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire));
		ASSERT( m_pHearEnemyWeaponFire );

		m_pHearAllyWeaponFire = dynamic_cast<CAISenseHearAllyWeaponFire*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyWeaponFire));
		ASSERT( m_pHearAllyWeaponFire );

		m_pHearTurretWeaponFire = dynamic_cast<CAISenseHearTurretWeaponFire*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearTurretWeaponFire));
		ASSERT( m_pHearTurretWeaponFire );

		m_pHearAllyPain = dynamic_cast<CAISenseHearAllyPain*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyPain));
		ASSERT( m_pHearAllyPain );

		m_pHearDeath = dynamic_cast<CAISenseHearDeath*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearDeath));
		ASSERT( m_pHearDeath );

		m_pHearWeaponImpact = dynamic_cast<CAISenseHearWeaponImpact*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearWeaponImpact));
		ASSERT( m_pHearWeaponImpact );

		m_pSeeEnemyFlashlight = dynamic_cast<CAISenseSeeEnemyFlashlight*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemyFlashlight));
		ASSERT( m_pSeeEnemyFlashlight );

		m_pSeeBlood = dynamic_cast<CAISenseSeeBlood*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeBlood));
		ASSERT( m_pSeeBlood );
		
		m_pSeeCloaked = dynamic_cast<CAISenseSeeCloaked*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeCloaked));
		ASSERT( m_pSeeCloaked );
	}

	SetupFSM();
}

void InvestigateGoal::SetupFSM()
{
	// State definitions.

	m_RootFSM.DefineState( Root::eStateSuspicious,  
						   m_RootFSM.MakeCallback(*this, &InvestigateGoal::UpdateSuspicious),
						   m_RootFSM.MakeCallback(*this, &InvestigateGoal::StartSuspicious) );

	m_RootFSM.DefineState( Root::eStateMoveToStimulus,  
						   m_RootFSM.MakeCallback(*this, &InvestigateGoal::UpdateMoveToStimulus),
						   m_RootFSM.MakeCallback(*this, &InvestigateGoal::StartMoveToStimulus) );

	m_RootFSM.DefineState( Root::eStateSearch,  
						   m_RootFSM.MakeCallback(*this, &InvestigateGoal::UpdateSearch),
						   m_RootFSM.MakeCallback(*this, &InvestigateGoal::StartSearch) );

	m_RootFSM.DefineState( Root::eStateDone,  
						   m_RootFSM.MakeCallback() );


	// Transitions

	m_RootFSM.DefineTransition( Root::eStateSuspicious, Root::eFinished, Root::eStateDone );
	m_RootFSM.DefineTransition( Root::eStateSuspicious, Root::eStimulus, Root::eStateMoveToStimulus );
	// ignore Root::eStimulus
	// ignore Root::eFail (it can't fail!).

	m_RootFSM.DefineTransition( Root::eStateMoveToStimulus, Root::eFinished, Root::eStateSearch );
	m_RootFSM.DefineTransition( Root::eStateMoveToStimulus, Root::eStimulus, Root::eStateMoveToStimulus );
	m_RootFSM.DefineTransition( Root::eStateMoveToStimulus, Root::eFail, Root::eStateDone );


	m_RootFSM.DefineTransition( Root::eStateSearch, Root::eFinished, Root::eStateDone );
	m_RootFSM.DefineTransition( Root::eStateSearch, Root::eStimulus, Root::eStateMoveToStimulus );
	// ignore Root::eFail (it can't fail!).

	m_RootFSM.DefineTransition( Root::eStateDone, Root::eStimulus, Root::eStateSuspicious );
	// ignore Root::eFinished (it can't finish).
	// ignore Root::eFail (it can't fail!).


	m_RootFSM.SetInitialState( Root::eStateDone );

#ifdef INVESTIGATE_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&InvestigateGoal::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&InvestigateGoal::PrintRootFSMMessage));
#endif

}

bool InvestigateGoal::HandleCommand(const char * const * pTokens, int nArgs)
{
	if( 0 == stricmp( *pTokens, g_szInvestigate ) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			ParseLocation result(*pTokens, *GetAIPtr());

			if( !result.error.empty() )
			{
#ifndef _FINAL
				AIErrorPrint(GetAIPtr(), "%s command did not understand location \"%s\".",
					g_szInvestigate, *pTokens);
#endif
			}
			else
			{
				m_bTriggered = LTTRUE;
				m_vTriggeredDest = result.vPosition;
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(GetAIPtr(), "Given command %s with %d arguments.", g_szInvestigate, nArgs);
			AIErrorPrint(GetAIPtr(), "Usage : %s (# # #) or %s node_name or %s object_name", g_szInvestigate, g_szInvestigate, g_szInvestigate );
		}
#endif
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::IsRunToSense
//
//	PURPOSE:	Returns true if this sense is one in which the AI should run
//				to the location and look around.
//
// ----------------------------------------------------------------------- //

void InvestigateGoal::SetInvestigateAnimSet(LTBOOL bOn)
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	if (bOn)
	{
		GetAIPtr()->SetMovementStyle("Investigate");
	}
	else
	{
		GetAIPtr()->SetMovementStyle();
	}
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::IsRunToSense
//
//	PURPOSE:	Returns true if this sense is one in which the AI should run
//				to the location and look around.
//
// ----------------------------------------------------------------------- //

#ifndef _FINAL
const char * InvestigateGoal::GetDescription() const 
{ 
	switch( GetSenseIndex(m_pSense) )
	{
	case 0 :
		return "Investigate HearEnemyWeaponFire";
		break;
	case 1 :
		return "Investigate HearAllyWeaponFire";
		break;
	case 2 :
		return "Investigate HearWeaponImpact";
		break;
	case 3 :
		return "Investigate HearEnemyFootstep";
		break;
	case 4 :
		return "Investigate SeeDeadBody";
		break;
	case 5 :
		return "Investigate SeeEnemyFlashlight";
		break;
	case 6 :
		return "Investigate SeeBlood";
		break;
	case 7 :
		return "Investigate SeeCloaked";
		break;
	case 8 :
		return "Investigate HearAllyPain";
		break;
	case 9 :
		return "Investigate HearDeath";
		break;
	case 10 :
		return "Investigate HearEnemyTaunt";
		break;
	case 11 :
		return "Investigate HearTurretWeaponFire";
		break;
	}

	return "Investigate";
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::IsRunToSense
//
//	PURPOSE:	Returns true if this sense is one in which the AI should run
//				to the location and look around.
//
// ----------------------------------------------------------------------- //

LTBOOL InvestigateGoal::IsRunToSense(const CAISense * pSense)
{
	return    pSense == m_pSeeDeadBody 
		   || pSense == m_pHearAllyWeaponFire
		   || pSense == m_pHearTurretWeaponFire
		   || pSense == m_pHearAllyPain
		   || pSense == m_pHearDeath;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::GetActiveSensePtr
//
//	PURPOSE:	Returns a pointer to the sense with the highest stimulation level
//				If all senses checked report 0.0f, then it returns NULL
//
// ----------------------------------------------------------------------- //
CAISense * InvestigateGoal::GetActiveSensePtr()
{
	CAISense *pSense = NULL;

	// Other senses take precedence if the intensity is higher
	if (m_pSeeBlood && m_pSeeBlood->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pSeeBlood->GetStimulation()))
			pSense = m_pSeeBlood;
	}

	if (m_pSeeCloaked && m_pSeeCloaked->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pSeeCloaked->GetStimulation()))
			pSense = m_pSeeCloaked;
	}

	if (m_pHearAllyPain && m_pHearAllyPain->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearAllyPain->GetStimulation()))
			pSense = m_pHearAllyPain;
	}

	if (m_pHearDeath && m_pHearDeath->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearDeath->GetStimulation()))
			pSense = m_pHearDeath;
	}

	if( m_pHearWeaponImpact && m_pHearWeaponImpact->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearWeaponImpact->GetStimulation()))
			pSense = m_pHearWeaponImpact;
	}

	if (m_pHearEnemyWeaponFire && m_pHearEnemyWeaponFire->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearEnemyWeaponFire->GetStimulation()))
			pSense = m_pHearEnemyWeaponFire;
	}

	if (m_pHearAllyWeaponFire && m_pHearAllyWeaponFire->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearAllyWeaponFire->GetStimulation()))
			pSense = m_pHearAllyWeaponFire;
	}

	if (m_pHearTurretWeaponFire && m_pHearTurretWeaponFire->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearTurretWeaponFire->GetStimulation()))
			pSense = m_pHearTurretWeaponFire;
	}

	if (m_pHearEnemyFootstep && m_pHearEnemyFootstep->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearEnemyFootstep->GetStimulation()))
			pSense = m_pHearEnemyFootstep;
	}
	
	if (m_pHearEnemyTaunt && m_pHearEnemyTaunt->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pHearEnemyTaunt->GetStimulation()))
			pSense = m_pHearEnemyTaunt;
	}

	if (m_pSeeEnemyFlashlight && m_pSeeEnemyFlashlight->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pSeeEnemyFlashlight->GetStimulation()))
			pSense = m_pSeeEnemyFlashlight;
	}

	if (m_pSeeDeadBody && m_pSeeDeadBody->HasPartialStimulus() )
	{
		if (!pSense || (pSense->GetStimulation() <= m_pSeeDeadBody->GetStimulation()))
			pSense = m_pSeeDeadBody;
	}

	if (!pSense || !pSense->HasPartialStimulus() )
		return NULL;

	return pSense;
}


int  InvestigateGoal::GetSenseIndex(CAISense * pSense) const
{
	if( !m_pSense )
		return -1;
	else if( m_pSense == m_pHearEnemyWeaponFire )
		return 0;
	else if( m_pSense == m_pHearAllyWeaponFire )
		return 1;
	else if( m_pSense == m_pHearWeaponImpact )
		return 2;
	else if( m_pSense == m_pHearEnemyFootstep )
		return 3;
	else if( m_pSense == m_pSeeDeadBody )
		return 4;
	else if( m_pSense == m_pSeeEnemyFlashlight )
		return 5;
	else if( m_pSense == m_pSeeBlood )
		return 6;
	else if( m_pSense == m_pSeeCloaked )
		return 7;
	else if( m_pSense == m_pHearAllyPain )
		return 8;
	else if( m_pSense == m_pHearDeath )
		return 9;
	else if( m_pSense == m_pHearEnemyTaunt )
		return 10;
	else if( m_pSense == m_pHearTurretWeaponFire )
		return 11;

	_ASSERT( 0 );

	return -1;
}

CAISense * InvestigateGoal::GetIndexedSense(int nIndex) const
{
	switch (nIndex )
	{
	case 0 :
		return m_pHearEnemyWeaponFire;
		break;
	case 1 :
		return m_pHearAllyWeaponFire;
		break;
	case 2 :
		return m_pHearWeaponImpact;
		break;
	case 3 :
		return m_pHearEnemyFootstep;
		break;
	case 4 :
		return m_pSeeDeadBody;
		break;
	case 5 :
		return m_pSeeEnemyFlashlight;
		break;
	case 6 :
		return m_pSeeBlood;
		break;
	case 7 :
		return m_pSeeCloaked;
		break;
	case 8 :
		return m_pHearAllyPain;
		break;
	case 9 :
		return m_pHearDeath;
		break;
	case 10 :
		return m_pHearEnemyTaunt;
		break;
	case 11 :
		return m_pHearTurretWeaponFire;
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT InvestigateGoal::GetBid()
{
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(BID_INVESTIGATE);
	LTFLOAT fBid = 0.0f;

	if( !IsActive() )
	{
		if( m_bTriggered )
		{
			fBid = fDefault;
		}
		else if( m_tmrHoldOff.Stopped() && GetActiveSensePtr() )
		{
			fBid = fDefault;
		}
	}
	else
	{
		// This goal must run to completion.
		if( m_RootFSM.GetState() != Root::eStateDone )
		{
			fBid = fDefault;
		}
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::GetBidModifier
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT InvestigateGoal::GetMaximumBid()
{
	return g_pAIButeMgr->GetDefaultBid(BID_INVESTIGATE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::Start,Update,End
//
//	PURPOSE:	These are called by the goal system.
//
// ----------------------------------------------------------------------- //

void InvestigateGoal::Start()
{
	Goal::Start();

	m_StrategyFollowPath.Clear();

	GetAIPtr()->SetAnimExpression(EXPRESSION_CURIOUS);

	m_eSuspectType = Nothing;

	CAISense * pSense = GetActiveSensePtr();
	if( pSense )
	{
		if( pSense == m_pSeeCloaked )
		{
			m_eSuspectType = Cloaked;
			GetAIPtr()->PlayExclamationSound("SuspectCloaked");
		}
		else if( pSense == m_pHearEnemyFootstep )
		{
			if( pSense->GetStimulus() )
			{
				if( IsAlien(pSense->GetStimulus()) )
				{
					m_eSuspectType = AlienFootstep;
					GetAIPtr()->PlayExclamationSound("SuspectAlienFootstep");
				}
				else if( IsPredator(pSense->GetStimulus()) )
				{
					m_eSuspectType = PredatorFootstep;
					GetAIPtr()->PlayExclamationSound("SuspectPredatorFootstep");
				}
				else if( IsHuman(pSense->GetStimulus()) || IsSynthetic(pSense->GetStimulus()) )
				{
					m_eSuspectType = HumanFootstep;
					GetAIPtr()->PlayExclamationSound("SuspectHumanFootstep");
				}
			}
		}
		else if( pSense == m_pHearEnemyTaunt )
		{
			if( pSense->GetStimulus() )
			{
				if( IsAlien(pSense->GetStimulus()) )
				{
					m_eSuspectType = AlienTaunt;
					GetAIPtr()->PlayExclamationSound("SuspectAlienTaunt");
				}
				else if( IsPredator(pSense->GetStimulus()) )
				{
					m_eSuspectType = PredatorTaunt;
					GetAIPtr()->PlayExclamationSound("SuspectPredatorTaunt");
				}
				else if( IsHuman(pSense->GetStimulus()) || IsSynthetic(pSense->GetStimulus()) )
				{
					m_eSuspectType = HumanTaunt;
					GetAIPtr()->PlayExclamationSound("SuspectHumanTaunt");
				}
			}
		}
		else if( pSense == m_pSeeEnemyFlashlight )
		{
			m_eSuspectType = Flashlight;
			GetAIPtr()->PlayExclamationSound("SuspectFlashlight");
		}
		else if( pSense == m_pSeeDeadBody )
		{
			m_pDeadBody = dynamic_cast<BodyProp *>( g_pLTServer->HandleToObject( pSense->GetStimulus() ) );
			if( m_pDeadBody )
			{
				if( IsAlien(m_pDeadBody->GetCharacter()) )
				{
					m_eSuspectType = AlienBody;
					GetAIPtr()->PlayExclamationSound("SuspectAlienBody");
				}
				else if( IsPredator(m_pDeadBody->GetCharacter()) )
				{
					m_eSuspectType = PredatorBody;
					GetAIPtr()->PlayExclamationSound("SuspectPredatorBody");
				}
				else if( IsHuman(m_pDeadBody->GetCharacter()) || IsSynthetic(m_pDeadBody->GetCharacter()) )
				{
					m_eSuspectType = HumanBody;
					GetAIPtr()->PlayExclamationSound("SuspectHumanBody");
				}
			}
		}
		else if( pSense == m_pHearEnemyWeaponFire )
		{
			m_eSuspectType = EnemyWeaponFire;
			GetAIPtr()->PlayExclamationSound("SuspectEnemyWeaponFire");
		}
		else if( pSense == m_pHearAllyPain )
		{
			m_eSuspectType = AllyPain;
			GetAIPtr()->PlayExclamationSound("SuspectAllyPain");
		}
		else if( pSense == m_pHearDeath )
		{
			m_eSuspectType = DeathCry;
			GetAIPtr()->PlayExclamationSound("SuspectDeathCry");
		}

	}

	m_tmrDelay.Start( GetRandom(m_butes.fMinAnimDelay, m_butes.fMaxAnimDelay) );

	// Possibly play an initial animation.
	const AIAnimSet* pAttackSet = g_pAIAnimButeMgr->GetAnimSetPtr( GetAIPtr()->GetButes()->m_szAIAnimType, "Investigate");
	if( pAttackSet && (pAttackSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
	{
		if( GetAIPtr()->AllowSnapTurn() && pSense )
		{
			GetAIPtr()->SnapTurn( pSense->GetStimulusPos() - GetAIPtr()->GetPosition() );
		}

		AIActionPlayAnim* pPlayAnim = dynamic_cast<AIActionPlayAnim*>( GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM) );
		if( pPlayAnim )
		{
			pPlayAnim->Init(pAttackSet->GetRandomAnim());
		}
	}

	m_RootFSM.SetState( Root::eStateSuspicious );
}

void InvestigateGoal::Update() 
{ 
	Goal::Update(); 

	if( m_tmrDelay.On() && m_tmrDelay.Stopped() )
	{
		SetInvestigateAnimSet(LTTRUE);
		m_tmrDelay.Stop();
	}

	m_RootFSM.Update();
}


void InvestigateGoal::End()
{
	Goal::End();

	if( m_RootFSM.GetState() == Root::eStateDone )
	{
		m_tmrHoldOff.Start( GetRandom( m_butes.fMinHoldOffDelay, m_butes.fMaxHoldOffDelay ) );

		// If the current state is already state done, that
		// means we didn't find anything, so go ahead and
		// play our "Ohh, I guess it was nothing." sound.
		switch( m_eSuspectType )
		{
			case Cloaked :
				GetAIPtr()->PlayExclamationSound("SuspectCloakedFail");
				break;

			case AlienFootstep :
				GetAIPtr()->PlayExclamationSound("SuspectAlienFootstepFail");
				break;

			case PredatorFootstep :
				GetAIPtr()->PlayExclamationSound("SuspectPredatorFootstepFail");
				break;

			case HumanFootstep :
				GetAIPtr()->PlayExclamationSound("SuspectHumanFootstepFail");
				break;

			case AlienTaunt :
				GetAIPtr()->PlayExclamationSound("SuspectAlienTauntFail");
				break;

			case PredatorTaunt :
				GetAIPtr()->PlayExclamationSound("SuspectPredatorTauntFail");
				break;

			case HumanTaunt :
				GetAIPtr()->PlayExclamationSound("SuspectHumanTauntFail");
				break;

			case Flashlight :
				GetAIPtr()->PlayExclamationSound("SuspectFlashlightFail");
				break;

			case AlienBody :
				GetAIPtr()->PlayExclamationSound("SuspectAlienBodyFail");
				break;

			case PredatorBody :
				GetAIPtr()->PlayExclamationSound("SuspectPredatorBodyFail");
				break;

			case HumanBody :
				GetAIPtr()->PlayExclamationSound("SuspectHumanBodyFail");
				break;

			case EnemyWeaponFire :
				GetAIPtr()->PlayExclamationSound("SuspectEnemyWeaponFireFail");
				break;

			case AllyPain :
				GetAIPtr()->PlayExclamationSound("SuspectAllyPainFail");
				break;

			case DeathCry :
				GetAIPtr()->PlayExclamationSound("SuspectDeathCryFail");
				break;

			default :
			case Nothing:
				// do nothing
				break;
		}
	}
	else
	{
		// Be sure we are in the done state so that we can re-start neatly.
		m_RootFSM.SetState( Root::eStateDone );
	}

	m_pSense = LTNULL;
	m_bTriggered = LTFALSE;

	SetInvestigateAnimSet(LTFALSE);

	GetAIPtr()->SetAnimExpression(EXPRESSION_NORMAL);

	// This may not be wise.  If the flashlight flashes
	// on and off between investigate and attack, you
	// should remove this and put it at the start of
	// all other goals.
	// But I think the updates happen fast enough that
	// the attack goal will turn it back on right
	// away.
	GetAIPtr()->SetFlashlight(LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	STATE:	Suspicious
//	
//	PURPOSE:	AI will stay where they are, but look around.
//
// ----------------------------------------------------------------------- //

void InvestigateGoal::StartSuspicious(RootFSM * fsm)
{
	if (!GetAIPtr())
		return;

	const LTFLOAT fSuspiciousDur = m_tmrDelay.GetDuration() + GetRandom(m_butes.fMinSuspiciousDur, m_butes.fMaxSuspiciousDur);
	m_tmrSuspicious.Start(fSuspiciousDur);	// bute this with a range

	// Be sure we are facing the stimulus.
	m_pSense = GetActiveSensePtr();
	if( m_pSense || m_bTriggered )
	{
		if( m_pSense )
			m_vDest = m_pSense->GetStimulusPos();
		else
			m_vDest = m_vTriggeredDest;

		if( AIActionTurnTo * pTurnAction = dynamic_cast<AIActionTurnTo *>(GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO)) )
		{
			pTurnAction->Init(m_vDest - GetAIPtr()->GetPosition());
			return;
		}
	}
}

void InvestigateGoal::UpdateSuspicious(RootFSM *)
{

	// If the sense goes to full stimulation, go search that
	// location.  Unless it is a weapon impact!
	m_pSense = GetActiveSensePtr();
	if (m_pSense )
	{
		// Go search if the stimulation rises to 1.0

		LTFLOAT fLevel = m_pSense->GetStimulation();


		if (fLevel >= 1.0f || IsRunToSense(m_pSense) || m_tmrSuspicious.Stopped() )
		{
			m_RootFSM.AddMessage(Root::eStimulus);
			return;
		}

		// if the level reaches 0, then pSense will come back NULL
	}

	// A triggered sense should always move to the triggered destination
	// if there is not another sense.
	if ( m_bTriggered )
	{
		m_RootFSM.AddMessage(Root::eStimulus);
		return;
	}

	// anything else, we just stay in this state or switch to another goal

	if (m_tmrSuspicious.Stopped() && !m_pSense )
	{
		m_RootFSM.AddMessage(Root::eFinished);
		return;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	STATE:	MoveToStimulus
//	
//	PURPOSE:	AI is getting closer to the stimulus (in order to search).
//
// ----------------------------------------------------------------------- //


void InvestigateGoal::StartMoveToStimulus(RootFSM * fsm)
{
	GetAIPtr()->Walk();

	m_pSense = GetActiveSensePtr();

	// If we have a sense, we need to figure out how to move to it.
	if( m_pSense )
	{
		// Special movement and sound if we suspect a predator
		if (m_pSense == m_pSeeCloaked)
		{
			if( m_eSuspectType != Cloaked )
				GetAIPtr()->PlayExclamationSound("SuspectCloaked");
		}

		// Start pathing to the location of the stimulation.
		
		// The location may have changed since we became suspiciuos.
		if( m_pSense->GetStimulation() )
			m_vDest = m_pSense->GetStimulusPos();

		// Move directly to some senses.
		if(  IsRunToSense(m_pSense) )
		{
			GetAIPtr()->Run();

			// Be sure we are the only ones to run to this body prop.
			if( m_pSense == m_pSeeDeadBody && m_pSense->GetStimulation() )
			{
				BodyProp* pBodyProp = dynamic_cast<BodyProp*>( g_pLTServer->HandleToObject( m_pSense->GetStimulus() ) );
				if( pBodyProp )
				{
					g_pCharacterMgr->RemoveDeadBody( pBodyProp );
				}
			}

			// Try to actually reach the body.
			if( m_StrategyFollowPath.Set(m_vDest) )
			{
				return;
			}
		}
	}
	// for some reason there is no stimulus.
	else if( !m_bTriggered )
	{
		_ASSERT( 0 );
		m_RootFSM.AddMessage(Root::eFail);
		return;
	}

	// At this point, if the sense was triggered m_vDest should be the same as m_vTriggeredDest.
	// If it isn't, please figure out why rather than just forcing it to be so.
	_ASSERT( !m_bTriggered || (m_vDest == m_vTriggeredDest) );

	const CAIVolume * pNearestVolume = g_pAIVolumeMgr->FindContainingVolume( m_vDest );
	
	if( !pNearestVolume || !m_StrategyFollowPath.Set(pNearestVolume) )
	{
		// If we can't path to that location, path to
		// the nearest volume.

		LTFLOAT fNearestVolumeDistSqr = FLT_MAX;

		for( CAIVolumeMgr::VolumeList::const_iterator iter = g_pAIVolumeMgr->BeginVolumes();
		     iter != g_pAIVolumeMgr->BeginVolumes(); ++iter )
		{
			const CAIVolume & currentVolume = *iter;
			const CAIVolumeNode * const pVolumeNode = dynamic_cast<const CAIVolumeNode *>( g_pAINodeMgr->GetNode(currentVolume.GetNodeID()) );

			// Be sure we can use this volume.
			if( pVolumeNode && GetAIPtr()->CanUseNode(*pVolumeNode) )
			{
				// See if this volume is closer.
				const LTFLOAT fCurrentVolumeDistSqr = (pVolumeNode->GetPos() - m_vDest).MagSqr();
				if(    !pNearestVolume 
					|| fCurrentVolumeDistSqr < fNearestVolumeDistSqr )
				{
					// It is closer, now can we path to it?
					if( m_StrategyFollowPath.Set(&currentVolume) )
					{
						// Alright, we have a new nearest volume!
						pNearestVolume = &currentVolume;
						fNearestVolumeDistSqr = fCurrentVolumeDistSqr;
					}
				}
			}

		} // for( CAIVolumeMgr::VolumeList::const_iterator ......

		// If we didn't find a nearest volume, just search around where we are.!
		if( !m_StrategyFollowPath.IsSet() )
		{
			m_RootFSM.AddMessage(Root::eFinished);
			return;
		}

	} //if( !m_StrategyFollowPath.Set(pNearestVolume) )

}

void InvestigateGoal::UpdateMoveToStimulus(RootFSM *)
{
	CAISense *pSense = GetActiveSensePtr();
	if (pSense && (pSense != m_pSense))
	{
		m_RootFSM.AddMessage(Root::eStimulus);
		return;
	}

	if( m_StrategyFollowPath.IsDone() )
	{
		m_RootFSM.AddMessage(Root::eFinished);
		return;
	}

	m_StrategyFollowPath.Update();

	return;
}

// ----------------------------------------------------------------------- //
//
//	STATE:	Search
//	
//	PURPOSE:	AI is at (or near) stimulus location.  Have a look around.
//
// ----------------------------------------------------------------------- //

void InvestigateGoal::StartSearch(RootFSM * fsm)
{
	// for some reason there is no stimulus, so drop to suspicious
	if (!m_pSense && !m_bTriggered)
	{
		m_RootFSM.AddMessage(Root::eFinished);
		return;
	}

	// The location may have changed.
	if( m_pSense && m_pSense->HasPartialStimulus() )
		m_vDest = m_pSense->GetStimulusPos();

	// Turn on flashlight if it is dark.
	if (GetAIPtr()->GetLightLevel(m_vDest) < 0.2f)
		GetAIPtr()->SetFlashlight(LTTRUE);

	m_tmrSearch.Start( GetRandom(m_butes.fMinSearchDur, m_butes.fMaxSearchDur) );

	if( AIActionTurnTo * pTurnAction = dynamic_cast<AIActionTurnTo *>(GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO)) )
	{
		// This is required for a dead body because it gets removed from the dead body 
		// list immediately after an AI has seen it.  This means GetStimulusPos will 
		// no longer return anything, so we use the stored value instead.
		if (m_pDeadBody && (m_pSense == m_pSeeDeadBody))
		{
			m_vDest = m_pDeadBody->GetDeathDir();
		}

		pTurnAction->Init(m_vDest - GetAIPtr()->GetPosition());
	}

}

void InvestigateGoal::UpdateSearch(RootFSM *)
{
	CAISense *pSense = GetActiveSensePtr();
	if (pSense && (pSense != m_pSense))
	{
		m_RootFSM.AddMessage(Root::eStimulus);
		return;
	}

	if (m_tmrSearch.Stopped() && !pSense )
	{
		m_RootFSM.AddMessage(Root::eFinished);
		return;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InvestigateGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void InvestigateGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);

	Goal::Load(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_StrategyFollowPath;

	*hRead >> m_tmrSearch;
	*hRead >> m_tmrSuspicious;

	const int nSenseIndex = int(hRead->ReadDWord());
	m_pSense = GetIndexedSense(nSenseIndex);

	*hRead >> m_vDest;

	*hRead >> m_eSuspectType;
	*hRead >> m_tmrDelay;

	*hRead >> m_bTriggered;
	*hRead >> m_vTriggeredDest;
}

void InvestigateGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);

	Goal::Save(hWrite);

	*hWrite << m_RootFSM;
	*hWrite << m_StrategyFollowPath;

	*hWrite << m_tmrSearch;
	*hWrite << m_tmrSuspicious;

	hWrite->WriteDWord( GetSenseIndex(m_pSense) );

	*hWrite << m_vDest;

	*hWrite << m_eSuspectType;
	*hWrite << m_tmrDelay;

	*hWrite << m_bTriggered;
	*hWrite << m_vTriggeredDest;
}

//------------------------------------------------------------------
// Debugging stuff
//------------------------------------------------------------------
#ifdef INVESTIGATE_DEBUG

void InvestigateGoal::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateSuspicious : szMessage = "eStateSuspicious"; break;
		case Root::eStateMoveToStimulus : szMessage = "eStateMoveToStimulus"; break;
		case Root::eStateSearch : szMessage = "eStateSearch"; break;
		case Root::eStateDone : szMessage = "eStateDone"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Investigate::RootFSM is entering state %s.", szMessage );
}

void InvestigateGoal::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eStimulus : szMessage = "eStimulus"; break;
		case Root::eFinished : szMessage = "eFinished"; break;
		case Root::eFail : szMessage = "eFail"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Investigate::RootFSM is processing %s.", szMessage );
}

#endif
