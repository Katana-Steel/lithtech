// ----------------------------------------------------------------------- //
//
// MODULE  : AICower.cpp
//
// PURPOSE : Implements the Cower goal and state.
//
// CREATED : 9/05/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AICower.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "AIVolume.h"
#include "AIActionMisc.h"
#include "AINodeMgr.h"
#include "CharacterMgr.h"
#include "AITarget.h"
#include "AIAnimButeMgr.h"
#include "ServerSoundMgr.h"
#include "SoundButeMgr.h"

#ifdef _DEBUG
//#define COWER_DEBUG
#endif

ILTMessage & operator<<(ILTMessage & out, CowerGoal::CowerAnimState & x)
{
	out.WriteByte(x);
	return out;
}

ILTMessage & operator>>(ILTMessage & in, CowerGoal::CowerAnimState & x)
{
	x = CowerGoal::CowerAnimState(in.ReadByte());
	return in;
}

/////////////////////////////////////////////////
//
//     Cower Goal
//
/////////////////////////////////////////////////


CowerGoal::CowerGoal( CAI * pAI, LTBOOL bUseRetreat, std::string create_name )
	: Goal(pAI, create_name ),
	  m_butes( g_pAIButeMgr->GetCower(pAI ? pAI->GetTemplateId() : 0) ),
	  m_pSeeEnemy(LTNULL),
	  m_pHearEnemyWeaponFire(LTNULL),
	  m_pHearWeaponImpact(LTNULL),
	  m_pSeeDeadBody(LTNULL),
	  m_pHearAllyBattleCry(LTNULL),

	  // m_RootFSM

	  m_StrategySetFacing(pAI),
	  m_StrategyFollowPath(pAI),
	  m_StrategyRetreat(pAI),

	  m_pCurNode(LTNULL),

	  m_bValid(LTFALSE),
	  // m_hAINodeGroup

	  // m_tmrCowerDuration
	  // m_tmrSafe

	  // m_hTarget
	  m_vRelTargetPos(0,0,0),
	  m_vInitTargetPos(0,0,0),

	  m_bUseRetreat(bUseRetreat),

	  m_CowerAnimState(Cower_Start),

	  // m_strNotifyMsg
	  // m_NotifiedList
	  // m_tmrNotifyDelay

	  m_nRunSoundBute(-1)
	  // m_tmrRunSound

{
	ASSERT( pAI );

	if( pAI )
	{
		m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
		ASSERT( m_pSeeEnemy );

		m_pHearEnemyWeaponFire = dynamic_cast<CAISenseHearEnemyWeaponFire*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire));
		ASSERT( m_pHearEnemyWeaponFire );

		m_pHearWeaponImpact = dynamic_cast<CAISenseHearWeaponImpact*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearWeaponImpact));
		ASSERT( m_pHearWeaponImpact );

		m_pSeeDeadBody = dynamic_cast<CAISenseSeeDeadBody*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeDeadBody));
		ASSERT( m_pSeeDeadBody );

		m_pHearAllyBattleCry = dynamic_cast<CAISenseHearAllyBattleCry*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyBattleCry));
		ASSERT( m_pHearAllyBattleCry );
		
	}

	m_StrategyRetreat.SetFlee(LTTRUE);

	SetupFSM();


	CString strRunSoundButes = g_pServerSoundMgr->GetSoundSetName( 
		g_pCharacterButeMgr->GetGeneralSoundDir(GetAIPtr()->GetCharacter()), 
		m_bUseRetreat ? "RetreatRun" : "CowerRun" );

	if( !strRunSoundButes.IsEmpty() )
	{
		m_nRunSoundBute = g_pSoundButeMgr->GetSoundSetFromName(strRunSoundButes);

		if( m_nRunSoundBute > 0 )
		{
			const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(m_nRunSoundBute);
			m_tmrRunSound.Start( butes.m_fMinDelay );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal::HandleParameter
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CowerGoal::HandleParameter(const char * const * pszArgs, int nArgs)
{
	if (0 == stricmp(*pszArgs, "NG"))
	{
		m_hAINodeGroup = LTNULL;

		++pszArgs;
		--nArgs;

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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal::PlayCowerAnim
//
//	PURPOSE:	Keeps the AI cowering.
//
// ----------------------------------------------------------------------- //

void CowerGoal::PlayCowerAnim()
{
	if (!GetAIPtr())
		return;

	// Be sure we are facing our target!
	if( GetAIPtr()->AllowSnapTurn() )
	{
		GetAIPtr()->SnapTurn( m_vRelTargetPos );
	}

	if( GetAIPtr()->CanDoNewAction() )
	{
		switch( m_CowerAnimState )
		{
		
			case Cower_Idle :
			{
				// Are we finished with Cower_Idle? 
				if( !GetAIPtr()->GetAnimation()->UsingAltAnim() )
				{
					// Do our Cower animation.
					if( StartAnimCower() )
					{
						m_CowerAnimState = Cower;
					}
					else
					{
						// For some reason it didn't play,
						// try our CowerIdle instead.
						StartAnimCowerIdle();
						m_CowerAnimState = Cower_Idle;
					}
				}
			}
			break;

			case Cower_Start :
			case Cower :
			{
				// Are we finished with Cower_Idle? 
				if( !GetAIPtr()->GetAnimation()->UsingAltAnim() )
				{
					// Do our Cower_Idle animation.
					if( StartAnimCowerIdle() )
					{
						m_CowerAnimState = Cower_Idle;
					}
					else
					{
						// For some reason it didn't play,
						// try our CowerIdle instead.
						StartAnimCower();
						m_CowerAnimState = Cower;
					}
				}
			}
			break;

		} //switch( m_CowerAnimState )

	} // if( GetAIPtr()->CanDoNewAction() )

	return;
}

bool CowerGoal::StartAnimCower()
{ 
	const bool bTargetNear = m_vRelTargetPos.MagSqr() < m_butes.fRunDistSqr;
	const char * const szCowerSet = bTargetNear ? "CloseCower" : "Cower";

	const AIAnimSet* pSet = g_pAIAnimButeMgr->GetAnimSetPtr( GetAIPtr()->GetButes()->m_szAIAnimType, szCowerSet);
	if( pSet && (pSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
	{
		AIActionPlayAnim* pPlayAnim = dynamic_cast<AIActionPlayAnim*>( GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM) );
		if( pPlayAnim )
		{
			pPlayAnim->Init( pSet->GetRandomAnim(), AIActionPlayAnim::FLAG_INTER );
			return true;
		}
	}

	return false;
}

bool CowerGoal::StartAnimCowerIdle()
{ 
	const bool bTargetNear = m_vRelTargetPos.MagSqr() < m_butes.fRunDistSqr;
	const char * const szCowerSet = bTargetNear ? "CloseCowerIdle" : "CowerIdle";

	const AIAnimSet* pSet = g_pAIAnimButeMgr->GetAnimSetPtr( GetAIPtr()->GetButes()->m_szAIAnimType, szCowerSet);
	if( pSet && (pSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
	{
		AIActionPlayAnim* pPlayAnim = dynamic_cast<AIActionPlayAnim*>( GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM) );
		if( pPlayAnim )
		{
			pPlayAnim->Init( pSet->GetRandomAnim(), AIActionPlayAnim::FLAG_INTER );
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal::SetupFSM
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CowerGoal::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateRetreat,
						   m_RootFSM.MakeCallback(*this, &CowerGoal::UpdateRetreat),
						   m_RootFSM.MakeCallback(*this, &CowerGoal::StartRetreat));

	m_RootFSM.DefineState( Root::eStateFindNode,
						   m_RootFSM.MakeCallback(*this, &CowerGoal::UpdateFindNode),
						   m_RootFSM.MakeCallback(*this, &CowerGoal::StartFindNode));

	m_RootFSM.DefineState( Root::eStateCower,
						   m_RootFSM.MakeCallback(*this, &CowerGoal::UpdateCower),
						   m_RootFSM.MakeCallback(*this, &CowerGoal::StartCower));

	m_RootFSM.DefineState( Root::eStateSafe,
						   m_RootFSM.MakeCallback(*this, &CowerGoal::UpdateSafe),
						   m_RootFSM.MakeCallback(*this, &CowerGoal::StartSafe));

	// Default Transitions.
	m_RootFSM.DefineTransition( Root::eAtNode, Root::eStateCower );
	m_RootFSM.DefineTransition( Root::eSafe, Root::eStateSafe );

	if( m_bUseRetreat )
		m_RootFSM.DefineTransition( Root::eRun, Root::eStateRetreat );
	else
		m_RootFSM.DefineTransition( Root::eRun, Root::eStateFindNode );


#ifdef COWER_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CowerGoal::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CowerGoal::PrintRootFSMMessage));
#endif

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal::GetActiveSensePtr
//
//	PURPOSE:	Returns a pointer to the sense with the highest stimulation level
//				If all senses checked report 0.0f, then it returns NULL
//
// ----------------------------------------------------------------------- //
CAISense * CowerGoal::GetActiveSensePtr()
{
	CAISense *pSense = NULL;

	// at least one sense is required for Cower to work
	if (!m_pHearAllyBattleCry)
		return NULL;

	pSense = m_pHearAllyBattleCry;

	// Other senses take precedence if the intensity is higher
	if (m_pHearEnemyWeaponFire)
	{
		if (pSense->GetStimulation() < m_pHearEnemyWeaponFire->GetStimulation())
			pSense = m_pHearEnemyWeaponFire;
	}

	if (m_pHearWeaponImpact)
	{
		if (pSense->GetStimulation() < m_pHearWeaponImpact->GetStimulation())
			pSense = m_pHearWeaponImpact;
	}
	
	if (m_pSeeDeadBody)
	{
		if (pSense->GetStimulation() < m_pSeeDeadBody->GetStimulation())
			pSense = m_pSeeDeadBody;
	}

	if (m_pSeeEnemy)
	{
		if (pSense->GetStimulation() < m_pSeeEnemy->GetStimulation())
			pSense = m_pSeeEnemy;
	}

	if (!pSense->HasPartialStimulus())
		return NULL;

	return pSense;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal:: FindNode 
//
//	PURPOSE:	finds a node to cower at
//
// ----------------------------------------------------------------------- //

void CowerGoal::StartFindNode(RootFSM * fsm)
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	m_tmrRunSound.Stop();

	// find a good node, lock it, and go to it
	CAINodeCover *pCowerNode = g_pAINodeMgr->FindCowerNode(GetAIPtr()->GetObject(), m_hTarget);
	if (pCowerNode && m_StrategyFollowPath.Set(pCowerNode) )
	{
		if (m_pCurNode)
			m_pCurNode->Unlock();
		
		pCowerNode->Lock(GetAIPtr()->GetObject());
		m_pCurNode = pCowerNode;
	}
	else
	{
		m_RootFSM.AddMessage(Root::eAtNode);
		return;
	}

	return;
}

void CowerGoal::UpdateFindNode(RootFSM * fsm)
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	GetAIPtr()->Run();

	if (m_StrategyFollowPath.IsDone())
	{
		m_RootFSM.AddMessage(Root::eAtNode);
		return;
	}

	// If we get stuck, just cower where we are!
	if (m_StrategyFollowPath.IsStuck())
	{
		if( m_pCurNode )
		{
			m_pCurNode->Unlock();
			m_pCurNode = LTNULL;
		}

		m_RootFSM.AddMessage(Root::eAtNode);
		return;
	}

	m_StrategyFollowPath.Update();

	if( m_nRunSoundBute > 0 && m_tmrRunSound.Stopped() )
	{
		GetAIPtr()->PlayExclamationSound( m_bUseRetreat ? "RetreatRun" : "CowerRun" );

		if( GetAIPtr()->IsPlayingVoicedSound() )
			m_tmrRunSound.Start();
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal:: Retreat 
//
//	PURPOSE:	uses the retreat strategy to get away.
//
// ----------------------------------------------------------------------- //

void CowerGoal::StartRetreat(RootFSM * fsm)
{
	m_StrategyRetreat.Start();
}

void CowerGoal::UpdateRetreat(RootFSM * fsm)
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	m_StrategyRetreat.Update();

	if( m_StrategyRetreat.IsCornered()  )
	{
		m_RootFSM.AddMessage(Root::eAtNode);
		return;
	}

	if( m_nRunSoundBute > 0 && m_tmrRunSound.Stopped() )
	{
		GetAIPtr()->PlayExclamationSound( m_bUseRetreat ? "RetreatRun" : "CowerRun" );

		if( GetAIPtr()->IsPlayingVoicedSound() )
			m_tmrRunSound.Start();
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal:: Cower
//
//	PURPOSE:	does the cowering.
//
// ----------------------------------------------------------------------- //

void CowerGoal::StartCower(RootFSM * fsm)
{
	if (!GetAIPtr())
		return;

	// Turn to face our tormentor.
	if( m_hTarget )
	{
		AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo *>( GetAIPtr()->SetNextAction( CAI::ACTION_TURNTO ) );
		if( pNewAction )
		{
			pNewAction->Init( m_vRelTargetPos );
		}
	}

	m_tmrCowerDuration.Start( GetRandom( m_butes.fMinCowerDuration, m_butes.fMaxCowerDuration ) );

	m_CowerAnimState = Cower_Start;

	return;
}

void CowerGoal::UpdateCower(RootFSM * fsm)
{
	if( !GetAIPtr()->CanDoNewAction() )
		return;

	if( m_tmrCowerDuration.Stopped() )
	{
		if( m_vRelTargetPos.MagSqr() > m_butes.fSafeDistSqr )
		{
			// Maybe we're safe now...
			m_RootFSM.AddMessage(Root::eSafe);
			return;
		}
		
		if( m_vRelTargetPos.MagSqr() > m_butes.fRunDistSqr )
		{
			if( GetRandom(0.0f,1.0f) < m_butes.fRunChance )
			{
				// Look for a new node.
				m_RootFSM.AddMessage(Root::eRun);
				return;
			}
			else
			{
				m_tmrCowerDuration.Start( GetRandom( m_butes.fMinCowerDuration, m_butes.fMaxCowerDuration ) );
			}
		}
	}


	PlayCowerAnim();

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal:: Safe
//
//	PURPOSE:	waits to make sure we are safe.
//
// ----------------------------------------------------------------------- //
void CowerGoal::StartSafe(RootFSM * fsm)
{
	m_tmrSafe.Start( GetRandom(m_butes.fMinSafeDuration, m_butes.fMaxSafeDuration) );
	
	return;
}

void CowerGoal::UpdateSafe(RootFSM * fsm)
{
	// Run if we see anything!
	if (CAISense *pSense = GetActiveSensePtr() )
	{
		if (pSense->GetStimulation() > 0.0f)
		{
			m_RootFSM.AddMessage(Root::eRun);
			return;
		}
	}

	// If all's been quiet for awhile - safe to come out
	if( m_bValid && m_tmrSafe.Stopped() )
	{
		m_bValid = LTFALSE;
	}


	// Keep cowering while we're waiting 
	// to end our goal.
	PlayCowerAnim();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CowerGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CowerGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	Goal::Load(hRead);

	// don't load senses.

	*hRead >> m_RootFSM;

	*hRead >> m_StrategySetFacing;
	*hRead >> m_StrategyFollowPath;
	*hRead >> m_StrategyRetreat;

	const uint32 nCurNodeID = hRead->ReadDWord();
	m_pCurNode = dynamic_cast<CAINodeCover*>(g_pAINodeMgr->GetNode( nCurNodeID ));

	*hRead >> m_bValid;
	*hRead >> m_hAINodeGroup;

	*hRead >> m_tmrCowerDuration;
	*hRead >> m_tmrSafe;

	*hRead >> m_hTarget;
	*hRead >> m_vRelTargetPos;
	*hRead >> m_vInitTargetPos;

	*hRead >> m_bUseRetreat;

	*hRead >> m_CowerAnimState;

	*hRead >> m_strNotifyMsg;
	
	m_NotifiedList.clear();

	*hRead >> m_tmrNotifyDelay;

	*hRead >> m_nRunSoundBute;
	*hRead >> m_tmrRunSound;
}


void CowerGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	Goal::Save(hWrite);
	
	*hWrite << m_RootFSM;

	*hWrite << m_StrategySetFacing;
	*hWrite << m_StrategyFollowPath;
	*hWrite << m_StrategyRetreat;

	if( m_pCurNode )
		hWrite->WriteDWord(m_pCurNode->GetID());
	else
		hWrite->WriteDWord(CAINode::kInvalidID);

	*hWrite << m_bValid;
	*hWrite << m_hAINodeGroup;

	*hWrite << m_tmrCowerDuration;
	*hWrite << m_tmrSafe;

	*hWrite << m_hTarget;
	*hWrite << m_vRelTargetPos;
	*hWrite << m_vInitTargetPos;

	*hWrite << m_bUseRetreat;

	*hWrite << m_CowerAnimState;

	*hWrite << m_strNotifyMsg;

	//m_NotifiedList will be reset on load.

	*hWrite << m_tmrNotifyDelay;

	*hWrite << m_nRunSoundBute;
	*hWrite << m_tmrRunSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateCower::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT CowerGoal::GetBid()
{
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(m_bUseRetreat ? BID_RETREAT : BID_COWER);
	LTFLOAT fBid = 0.0f;


	if( !m_bValid )
	{
		if( m_pSeeEnemy && m_pSeeEnemy->GetStimulation() >= 1.0f )
		{
			fBid = fDefault;
			m_hTarget = m_pSeeEnemy->GetStimulus();
			m_vInitTargetPos = m_pSeeEnemy->GetStimulusPos();
		}
		else if (m_pSeeDeadBody && m_pSeeDeadBody->GetStimulation() >= 1.0f)
		{
			fBid = fDefault;
			m_hTarget = m_pSeeDeadBody->GetStimulus();
			m_vInitTargetPos = m_pSeeDeadBody->GetStimulusPos();
		}
		else if (m_pHearWeaponImpact && m_pHearWeaponImpact->GetStimulation() >= 1.0f)
		{
			fBid = fDefault;
			m_hTarget = m_pHearWeaponImpact->GetStimulus();
			m_vInitTargetPos = m_pHearWeaponImpact->GetStimulusPos();
		}
		else if (m_pHearEnemyWeaponFire && m_pHearEnemyWeaponFire->GetStimulation() >= 1.0f)
		{
			fBid = fDefault;
			m_hTarget = m_pHearEnemyWeaponFire->GetStimulus();
			m_vInitTargetPos = m_pHearEnemyWeaponFire->GetStimulusPos();
		}
		else if (m_pHearAllyBattleCry && m_pHearAllyBattleCry->GetStimulation() >= 1.0f)
		{
			const CCharacter* pFirer = 
				dynamic_cast<const CCharacter*>( g_pLTServer->HandleToObject(m_pHearAllyBattleCry->GetStimulus()) );

			if( pFirer )
			{
				const CAI * pAIFirer = dynamic_cast<const CAI*>(pFirer);
				if( pAIFirer && pAIFirer->HasTarget())
				{
					fBid = fDefault;
					m_hTarget = pAIFirer->GetTarget().GetObject();
					m_vInitTargetPos = pAIFirer->GetTarget().GetPosition();
				}
			}
		}
		else if( GetAIPtr()->HasTarget() )
		{
			fBid = fDefault;
			m_hTarget = GetAIPtr()->GetTarget().GetObject();
			m_vInitTargetPos = GetAIPtr()->GetTarget().GetPosition();
		}
/*  If you want to respond to damage, use this.  You will also need
	to deal with GetActiveSensePtr.

		else if( GetAIPtr()->GetDestructible()->GetLastDamageTime() > m_fLastDamageTime )
		{
			m_fLastDamageTime = GetAIPtr()->GetDestructible()->GetLastDamageTime();
			HOBJECT hDamager = GetAIPtr()->GetDestructible()->GetLastDamager();
			const CCharacter * pCharDamager = dynamic_cast<const CCharacter *>( g_pLTServer->HandleToObject(hDamager) );
			if (hDamager && && pCharDamager && !g_pCharacterMgr->AreAllies(*GetAIPtr(), *pCharDamager) )
			{
				fBid = fDefault;
				m_hTarget = hDamager;
				m_vInitiTargetPos = pCharDamager->GetPosition();
			}
		}
*/

	}
	else
	{
		fBid = fDefault;
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateCower::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTFLOAT CowerGoal::GetMaximumBid()
{
	return g_pAIButeMgr->GetDefaultBid(m_bUseRetreat ? BID_RETREAT : BID_COWER);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateCower::Start/Update/End
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CowerGoal::Start()
{
	Goal::Start();

	m_StrategyFollowPath.Clear();

	GetAIPtr()->Run();

	if( m_bUseRetreat )
	{
		GetAIPtr()->Target( m_hTarget );
	}
	else
	{
		// The non-retreating AI will do not
		// need to do "is visible" checks, so 
		// keep the target null to make sure they 
		// don't.
		GetAIPtr()->Target(LTNULL);
	}

	GetAIPtr()->SetAnimExpression(EXPRESSION_SCARED);
	
	GetAIPtr()->PlayExclamationSound(m_bUseRetreat ? "RetreatSpot" : "CowerSpot");

	m_RootFSM.SetState( m_bUseRetreat ? Root::eStateRetreat : Root::eStateFindNode );

	m_bValid = TRUE;

	m_CowerAnimState = Cower_Start;

	m_NotifiedList.clear();
	m_strNotifyMsg = "inv ";
	m_strNotifyMsg += VectorToString(m_vInitTargetPos);

	m_tmrNotifyDelay.Stop();


}

void CowerGoal::End()
{
	Goal::End();

	m_hTarget = LTNULL;
	m_bValid = FALSE;

	GetAIPtr()->SetAnimExpression(EXPRESSION_NORMAL);

	if( m_pCurNode )
	{
		m_pCurNode->Unlock();
		m_pCurNode = LTNULL;
	}
}


void CowerGoal::Update()
{
	Goal::Update(); 
	
	if(!GetAIPtr())
	{
#ifdef _DEBUG
		g_pInterface->CPrint("WARNING: pAI invalid");
#endif
		return;
	}

	// Update our relative target position.
	if( m_hTarget )
	{
		g_pLTServer->GetObjectPos(m_hTarget, &m_vRelTargetPos);
		m_vRelTargetPos -= GetAIPtr()->GetPosition();
	}

	// Notify any AI's that entered our radius.
	if( m_tmrNotifyDelay.Stopped() )
	{
		m_tmrNotifyDelay.Start( m_butes.fNotifyDelay );

		for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs(); iter != g_pCharacterMgr->EndAIs(); ++iter )
		{
			AIList::iterator found_ai = m_NotifiedList.find(*iter);
			if( found_ai == m_NotifiedList.end() && g_pCharacterMgr->AreAllies(*GetAIPtr(), *(*iter)) )
			{
				const LTVector vRelPos = (*iter)->GetPosition() - GetAIPtr()->GetPosition();

				if( vRelPos.MagSqr() < m_butes.fNotifyDistSqr )
				{
					if( TestLOSIgnoreCharacters((*iter)->m_hObject, GetAIPtr()->m_hObject) )
					{
						SendTriggerMsgToObject(GetAIPtr(), (*iter)->m_hObject, m_strNotifyMsg.c_str());
						m_NotifiedList.insert(*iter);
					}
				}
			}
		}
	}

	// Okay, now let's update the cowering!
	m_RootFSM.Update();
}


//------------------------------------------------------------------
// Debugging stuff
//------------------------------------------------------------------
#ifdef COWER_DEBUG

void CowerGoal::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateFindNode : szMessage = "eStateFindNode"; break;
		case Root::eStateRetreat : szMessage = "eStateRetreat"; break;
		case Root::eStateCower : szMessage = "eStateCower"; break;
		case Root::eStateSafe : szMessage = "eStateSafe"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Cower::RootFSM is entering state %s.", szMessage );
}

void CowerGoal::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eAtNode : szMessage = "eAtNode"; break;
		case Root::eRun : szMessage = "eRun"; break;
		case Root::eSafe : szMessage = "eSafe"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Cower::RootFSM is processing %s.", szMessage );
}

#endif
