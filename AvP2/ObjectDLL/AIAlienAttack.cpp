// ----------------------------------------------------------------------- //
//
// MODULE  : AIAlienAttack.h
//
// PURPOSE : Implements the Melee Attack goal and state.
//
// CREATED : 8/1/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIAlienAttack.h"
#include "AI.h"
#include "AITarget.h"
#include "AIButeMgr.h"
#include "AINodeMgr.h"
#include "AISenseMgr.h"
#include "Weapon.h"
#include "MsgIDs.h"
#include "RCMessage.h"
#include "CharacterFuncs.h"
#include "AIActionMisc.h"
#include "AIAnimButeMgr.h"
#include "AINodeWallWalk.h"
#include "SoundButeMgr.h"
#include "CharacterMgr.h"
#include "AIVolumeMgr.h"

#ifdef _DEBUG
//#include "AIUtils.h"
//#define ALIEN_ATTACK_DEBUG
#endif 

static bool WithinWeaponFireRange(const AIWBM_AIWeapon * pWeaponButes, const AIBM_AlienAttack & attack_butes, const LTVector & vRelativePos)
{
	if( !pWeaponButes )
	{
		if( attack_butes.fPounceChance > 0.0f )
		{
			return vRelativePos.MagSqr() < attack_butes.fPounceMaxRangeSqr;
		}

		if( attack_butes.fFacehugChance > 0.0f )
		{
			return vRelativePos.MagSqr() < attack_butes.fFacehugMaxRangeSqr;
		}

		return true;
	}

	return vRelativePos.MagSqr() < pWeaponButes->fMaxFireRange*pWeaponButes->fMaxFireRange;
}

static bool DoLungeAttack(const CAI & ai, LTFLOAT fLungeChance, LTFLOAT fLungeMinRangeSqr, LTFLOAT fLungeMaxRangeSqr, const LTVector & vTargetPos, HOBJECT hTarget )
{
	if( GetRandom(0.0f,1.0f) < fLungeChance )
	{
		// Be sure we are within lunging range
		const LTFLOAT fTargetRangeSqr = (ai.GetPosition() - vTargetPos ).MagSqr();
		if(    fTargetRangeSqr >= fLungeMinRangeSqr 
			&& fTargetRangeSqr <  fLungeMaxRangeSqr  )
		{
			// Do line intersect to be sure we are clear to lunge
			if( ai.IsObjectVisible( CAI::DefaultFilterFn, LTNULL, 
									  ai.GetPosition(), hTarget, fTargetRangeSqr*1.1f, LTFALSE) )
			{
				// Be sure we don't get so lost that we will never find our way back again.
				const CAIVolume * pAIVol = ai.GetCurrentVolumePtr();

				// Only pounce if we are in a volume.
				if( pAIVol )
				{
					const CAIVolume * pTargetVol = ai.GetTarget().GetCharacterPtr() ? 
						  ai.GetTarget().GetCharacterPtr()->GetCurrentVolumePtr()
						: LTNULL;

					if( pTargetVol )
					{
						// Only pounce if we can path to the volume.  This
						// way we won't pounce across disconnected volumes.
						const LTFLOAT fMaxVolumePath = 3.0f*(pAIVol->GetCenter() - pTargetVol->GetCenter()).Mag();
						return g_pAIVolumeMgr->FindPath(vTargetPos, pTargetVol, pAIVol, fMaxVolumePath);
					}
					else
					{
						// Don't worry about if the target is in a volume or not.
						return true;
					}
				}
			}
		}
	}

	return false;
}


AlienAttackGoal::AlienAttackGoal( CAI * pAI, std::string create_name )
	: AICombatGoal(pAI,create_name),
	  m_StrategyApproachTarget(pAI),
	  m_StrategyFollowPath(pAI),
	  m_bDropped(LTFALSE),
	  m_bSentUnreachableCmd(LTFALSE),
	  // m_tmrPounce
	  // m_tmrFacehug
	  m_pAdvanceNode(LTNULL),
	  m_Butes( g_pAIButeMgr->GetAlienAttack(pAI->GetTemplateId()) )
{
	_ASSERT(pAI);
	if ( !pAI ) return;

	SetupFSM();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::SetupFSM
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AlienAttackGoal::SetupFSM()
{
	//
	// Set up the root FSM.
	//

	m_RootFSM.DefineState( Root::eStateWaitForNewTarget, 
		                   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::UpdateWaitForNewTarget),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::StartWaitForNewTarget),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::EndWaitForNewTarget)   );

	m_RootFSM.DefineState( Root::eStateDisplay, 
		                   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::UpdateDisplay),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::StartDisplay),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::EndDisplay)   );

	m_RootFSM.DefineState( Root::eStateApproachTarget, 
		                   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::UpdateApproachTarget),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::StartApproachTarget),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::EndApproachTarget) );

	m_RootFSM.DefineState( Root::eStateGetLineOfSight, 
		                   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::UpdateGetLineOfSight),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::StartGetLineOfSight),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::EndGetLineOfSight) );

	m_RootFSM.DefineState( Root::eStateEngageTarget, 
		                   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::UpdateEngageTarget),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::StartEngageTarget),
						   m_RootFSM.MakeCallback(*this,&AlienAttackGoal::EndEngageTarget) );

	m_RootFSM.DefineTransition( Root::eNewTarget,    Root::eStateApproachTarget );
	m_RootFSM.DefineTransition( Root::eEngageTarget, Root::eStateEngageTarget );

	m_RootFSM.SetInitialState( Root::eStateWaitForNewTarget );

#ifdef ALIEN_ATTACK_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&AlienAttackGoal::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&AlienAttackGoal::PrintRootFSMMessage));
#endif

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::HandleParameter
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
bool AlienAttackGoal::HandleParameter(const char * const * pszArgs, int nArgs) 
{ 
	if( nArgs > 0 )
	{
		if( 0 == stricmp(pszArgs[0],"DisplayAnim") )
		{
			if( nArgs > 1 )
			{
				m_strDisplayAnim = pszArgs[1];
			}
#ifndef _FINAL
			else
			{
				AIErrorPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
							" Alien Attack goal given parameter \"%s\" without a second argument!",
							pszArgs[0]);
				AIErrorPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
							"   Format : \"%s\" ( message )", "DisplayAnim" );
			}
#endif
			return true;
		}
	}

	return AICombatGoal::HandleParameter(pszArgs,nArgs); 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT AlienAttackGoal::GetMaximumBid()
{
	return 	g_pAIButeMgr->GetDefaultBid(BID_ATTACKSEEENEMY);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT AlienAttackGoal::GetBid()
{
	const LTFLOAT fDefaultSeeEnemy = g_pAIButeMgr->GetDefaultBid(BID_ATTACKSEEENEMY);
	const LTFLOAT fDefaultOther = g_pAIButeMgr->GetDefaultBid(BID_ATTACKOTHER);

	LTFLOAT fBid = 0.0f;

	if( GetAIPtr()->NeverLoseTarget() )
	{
		if( GetAIPtr()->HasTarget() )
		{
			return fDefaultSeeEnemy;
		}
	}

	if( !IsActive() )
	{
		m_NewTarget = FindNewTarget();
		if( m_NewTarget.hTarget )
		{
			// If using display, only respond to damage or see enemy!
			if( !m_strDisplayAnim.empty() )
				fBid = fDefaultOther;

			// If we saw the enemy, we need to go to a higher bid.
			if(    m_NewTarget.eReason == NewTarget::SeeEnemy
				|| m_NewTarget.eReason == NewTarget::Damage )
			{
				fBid = fDefaultSeeEnemy;
			}
		}
		else if( GetAIPtr()->HasTarget() )
		{
			// If using display, only respond to damage or see enemy!
			if( !m_strDisplayAnim.empty() )
				fBid = fDefaultOther;

			if( GetAIPtr()->GetTarget().IsMaintained() )
			{
				fBid = fDefaultSeeEnemy;
			}
		}
		else if (!m_strDisplayAnim.empty() && HearFullWeaponImpact())
		{
			fBid = fDefaultSeeEnemy;
		}

		// Only enter attack goal if we have a weapon!
		if (fBid > 0.0f)
		{
			if( m_NewTarget.hTarget )
				GetAIPtr()->ChooseBestWeapon(m_NewTarget.vPos);

			if (  !GetAIPtr()->GetCurrentWeaponPtr() 
				&& m_Butes.fPounceChance <= 0.0f 
				&& m_Butes.fFacehugChance <= 0.0f )
				fBid = 0.0f;
		}
	}
	else
	{
		if ( GetAIPtr()->HasTarget() )
			fBid = fDefaultSeeEnemy;
		else
		{
			if( m_tmrWaitForNewTarget.On() && m_tmrWaitForNewTarget.Stopped() )
				fBid = 0.0f;
			else
				fBid = fDefaultOther;
		}


		// Keep alive for as long as we hear weapon impacts.
		// This allows us to dodge and act like we're
		// in combat when we might be getting shot at!
		if ( fBid == 0.0f && HearPartialWeaponImpact() )
		{
			fBid = fDefaultOther;
		}

		// If we don't have a weapon, don't keep attacking!
		if (  !GetAIPtr()->GetCurrentWeaponPtr() 
			&& m_Butes.fPounceChance <= 0.0f 
			&& m_Butes.fFacehugChance <= 0.0f )
			fBid = 0.0f;
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::Start
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AlienAttackGoal::Start()
{
	AICombatGoal::Start();

	m_StrategyFollowPath.Clear();

	if( m_NewTarget.hTarget )
	{
		GetAIPtr()->Target( m_NewTarget.hTarget, m_NewTarget.vPos );
	}

	if( GetAIPtr()->HasTarget() )
	{
		GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
	}
	
	m_tmrWaitForNewTarget.Stop();

	GetAIPtr()->SetAnimExpression(EXPRESSION_ANGRY);
	GetAIPtr()->SetMovementStyle("Combat");
	GetAIPtr()->Run();

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

	// Possibly play an initial animation.
	const AIAnimSet* pAttackSet = g_pAIAnimButeMgr->GetAnimSetPtr( GetAIPtr()->GetButes()->m_szAIAnimType, "Attack");
	if( pAttackSet && (pAttackSet->m_fPlayChance > GetRandom(0.0f,1.0f))  )
	{
		AIActionPlayAnim* pPlayAnim = dynamic_cast<AIActionPlayAnim*>( GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM) );
		if( pPlayAnim )
		{
			pPlayAnim->Init(pAttackSet->GetRandomAnim(), AIActionPlayAnim::FLAG_FCTRG);
		}
	}

	m_pAdvanceNode = LTNULL;

	if( GetAIPtr()->HasTarget() )
	{
/*  The Alien has a bad habit of playing an idle animation between its idlescript and this goal.
		if( !m_strDisplayAnim.empty() )
		{
			m_RootFSM.SetState( Root::eStateDisplay );
		}
		else
*/		{
			m_RootFSM.SetState( Root::eStateApproachTarget );
		}
	}
	else
	{
		m_RootFSM.SetState( Root::eStateWaitForNewTarget );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::Update
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void AlienAttackGoal::Update()
{
	AICombatGoal::Update(); 

	// Attempt any special attacks.
	if( GetAIPtr()->CanDoNewAction() )
	{
		const CAITarget & target = GetAIPtr()->GetTarget();

		// Check for pouncing.
		if( m_Butes.fPounceChance > 0.0f )
		{
			const bool bTargetInVolumes = !target.GetCharacterPtr() || target.GetCharacterPtr()->GetCurrentVolumePtr();
			if(    m_tmrPounce.Stopped() || !bTargetInVolumes )
			{
				m_tmrPounce.Start( GetRandom(m_Butes.fMinPounceDelay, m_Butes.fMaxPounceDelay) );

				LTFLOAT fMinPounceRange = m_Butes.fPounceMinRangeSqr;
				if( !bTargetInVolumes )
				{
					if( GetAIPtr()->GetIdealWeaponButesPtr() )
					{
						fMinPounceRange = GetAIPtr()->GetIdealWeaponButesPtr()->fMaxIdealRange - 1.0f;
					}
					else
					{
						fMinPounceRange = 0.0f;
					}
				}

				if( DoLungeAttack( *GetAIPtr(), m_Butes.fPounceChance, fMinPounceRange, m_Butes.fPounceMaxRangeSqr, target.GetPosition(), target.GetObject() ) )
				{
					// Let the pouncing commence.
					if( GetAIPtr()->SetNextAction(CAI::ACTION_POUNCE) )
					{
						// Assume we will be within range to start fighting.
						m_RootFSM.SetNextState( Root::eStateEngageTarget );
						return;
					}
				}
			}

		}

		// Check for facehugging.
		if( target.GetCharacterPtr() && target.GetCharacterPtr()->GetButes()->m_bCanBeFacehugged )
		{
			if( m_Butes.fFacehugChance > 0.0f )
			{
				const bool bTargetInVolumes = !target.GetCharacterPtr() || target.GetCharacterPtr()->GetCurrentVolumePtr();
				if(    m_tmrFacehug.Stopped() || !bTargetInVolumes )
				{
					m_tmrFacehug.Start( GetRandom(m_Butes.fMinFacehugDelay, m_Butes.fMaxFacehugDelay) );

					LTFLOAT fMinFacehugRange = m_Butes.fFacehugMinRangeSqr;
					if( !bTargetInVolumes )
					{
						if( GetAIPtr()->GetIdealWeaponButesPtr() )
						{
							fMinFacehugRange = GetAIPtr()->GetIdealWeaponButesPtr()->fMaxIdealRange - 1.0f;
						}
						else
						{
							fMinFacehugRange = 0.0f;
						}
					}

					if( DoLungeAttack( *GetAIPtr(), m_Butes.fFacehugChance, fMinFacehugRange, m_Butes.fFacehugMaxRangeSqr, target.GetPosition(), target.GetObject() ) )
					{
						// Let the pouncing commence.
						if( GetAIPtr()->SetNextAction(CAI::ACTION_FACEHUG) )
						{
							// Assume we will be within range to start fighting.
							m_RootFSM.SetNextState( Root::eStateEngageTarget );
							return;
						}
					}
				}
			}
		}

	} //if( GetAIPtr()->CanDoNewAction() )

	m_RootFSM.Update();

	if( !GetAIPtr()->HasTarget() && !m_tmrWaitForNewTarget.On() )
	{
		// PLH TODO : bute this.
		m_tmrWaitForNewTarget.Start( GetRandom(5.0f, 12.0f) );
	}

	if( m_tmrWaitForNewTarget.On() && GetAIPtr()->HasTarget() )
	{
		m_tmrWaitForNewTarget.Stop();
	}
		
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::End
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AlienAttackGoal::End()
{
	AICombatGoal::End();

	m_NewTarget = NewTarget();

	GetAIPtr()->LookAt( LTNULL );
	GetAIPtr()->SetAnimExpression(EXPRESSION_NORMAL);
	GetAIPtr()->SetMovementStyle();

	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::Load
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AlienAttackGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT( hRead );

	AICombatGoal::Load(hRead);

	*hRead >> m_tmrWaitForNewTarget;
	*hRead >> m_NewTarget;

	*hRead >> m_RootFSM;
	
	*hRead >> m_StrategyApproachTarget;
	*hRead >> m_StrategyFollowPath;

	*hRead >> m_bDropped;
	*hRead >> m_bSentUnreachableCmd;

	*hRead >> m_tmrPounce;
	*hRead >> m_tmrFacehug;

	const uint32 nAdvanceNodeID = hRead->ReadDWord();
	m_pAdvanceNode = dynamic_cast<CAINodeWallWalk*>(g_pAINodeMgr->GetNode(nAdvanceNodeID));

	*hRead >> m_strDisplayAnim;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AlienAttackGoal::Save(HMESSAGEWRITE hWrite)
{
	_ASSERT( hWrite );

	AICombatGoal::Save(hWrite);

	*hWrite << m_tmrWaitForNewTarget;
	*hWrite << m_NewTarget;

	*hWrite << m_RootFSM;
	
	*hWrite << m_StrategyApproachTarget;
	*hWrite << m_StrategyFollowPath;

	*hWrite << m_bDropped;
	*hWrite << m_bSentUnreachableCmd;

	*hWrite << m_tmrPounce;
	*hWrite << m_tmrFacehug;

	hWrite->WriteDWord( m_pAdvanceNode ? m_pAdvanceNode->GetID() : CAINode::kInvalidID );

	*hWrite << m_strDisplayAnim;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::FindBetterAdvanceNode
//
//	PURPOSE:	This finds a better advance node by unlocking 
//				the current one and doing a search.  If it comes up with
//				another node, that must be a better one, and it sets up
//				the pathing to move to it.  If it comes up with the
//				same node, then there isn't a better one and it clears
//				out the current one.
//
// ----------------------------------------------------------------------- //

void AlienAttackGoal::FindBetterAdvanceNode()
{
	if( !m_pAdvanceNode )
		return;

	m_pAdvanceNode->Unlock();

	// Try to find another advance node.
	CAINodeWallWalk * pAdvanceNode = g_pAINodeMgr->FindWWAdvanceNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());

	if( pAdvanceNode && m_pAdvanceNode != pAdvanceNode &&  m_StrategyFollowPath.Set(pAdvanceNode) )
	{
		m_pAdvanceNode = pAdvanceNode;
		m_pAdvanceNode->Lock(GetAIPtr()->m_hObject);
	}
	else
	{
		// The node we found was the same as our old one (or unreachable), so there 
		// isn't a better node to use.  Just charge at our target.
		m_pAdvanceNode = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::xxxxWaitForNewTarget
//
//	PURPOSE:	What the AI should do when she doesn't have a target.
//
// ----------------------------------------------------------------------- //

void AlienAttackGoal::StartWaitForNewTarget(RootFSM * rootFSM)
{
	GetAIPtr()->SetMovementStyle("Investigate");
	return;
}

void AlienAttackGoal::UpdateWaitForNewTarget(RootFSM * rootFSM)
{
	// Just sit and wait for a target.
	if( GetAIPtr()->HasTarget() )
	{
		GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
		rootFSM->AddMessage(Root::eNewTarget);
		return;
	}

	return;
}

void AlienAttackGoal::EndWaitForNewTarget(RootFSM * rootFSM)
{
	GetAIPtr()->SetMovementStyle("Combat");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::xxxxWaitForNewTarget
//
//	PURPOSE:	What the AI should do when she doesn't have a target.
//
// ----------------------------------------------------------------------- //

void AlienAttackGoal::StartDisplay(RootFSM * rootFSM)
{
	const LTVector & vDesiredFacing = (GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition());

	GetAIPtr()->SnapTurn(vDesiredFacing);

	return;
}

void AlienAttackGoal::UpdateDisplay(RootFSM * rootFSM)
{
	if( GetAIPtr()->CanDoNewAction() )
	{
		AIActionPlayAnim * pNewAction = dynamic_cast<AIActionPlayAnim*>( GetAIPtr()->SetNextAction(CAI::ACTION_PLAYANIM) );
		if( pNewAction )
		{
			pNewAction->Init(m_strDisplayAnim.c_str(), AIActionPlayAnim::FLAG_FCTRG);
			m_RootFSM.SetNextState( Root::eStateApproachTarget );
			return;
		}
	}

	return;
}

void AlienAttackGoal::EndDisplay(RootFSM * rootFSM)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::xxxApproachTarget
//
//	PURPOSE:	What the AI should do to get within firing range of the target
//
// ----------------------------------------------------------------------- //
void  AlienAttackGoal::StartApproachTarget(RootFSM * rootFSM)
{
	m_StrategyApproachTarget.Reset();
	m_bDropped = LTFALSE;
	m_bSentUnreachableCmd = LTFALSE;

	// Try to find an advance node.
	if( GetAIPtr()->CanUseCover() )
	{
		if( m_pAdvanceNode )
		{
			m_pAdvanceNode->Unlock();
			m_pAdvanceNode = LTNULL;
		}

		// Try to find an advance node.
		CAINodeWallWalk * pAdvanceNode = g_pAINodeMgr->FindWWAdvanceNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());

		if( pAdvanceNode && m_StrategyFollowPath.Set(pAdvanceNode) )
		{
			m_pAdvanceNode = pAdvanceNode;
			m_pAdvanceNode->Lock(GetAIPtr()->m_hObject);
		}
	}
}


void  AlienAttackGoal::UpdateApproachTarget(RootFSM * rootFSM)
{
	if( m_StrategyApproachTarget.LostTarget() )
	{
		if( !GetAIPtr()->NeverLoseTarget() )
		{
			// Clear our old target and wait for a new one.
			GetAIPtr()->Target(LTNULL);
			rootFSM->SetNextState(Root::eStateWaitForNewTarget);
			return;
		}
		else if( !m_bSentUnreachableCmd )
		{
			m_bSentUnreachableCmd = LTTRUE;

			const std::string & command = GetAIPtr()->GetUnreachableTargetCmd();
			if( !command.empty() )
			{
				SendTriggerMsgToObject(GetAIPtr(), GetAIPtr()->m_hObject, command.c_str());
			}
		}
	}	


	const CAITarget & target = GetAIPtr()->GetTarget();
	const LTVector & vTargetPos = target.GetPosition();
	const LTVector & vMyPos = GetAIPtr()->GetPosition();
	if( WithinWeaponFireRange(GetAIPtr()->GetIdealWeaponButesPtr(), m_Butes, vTargetPos - vMyPos) )
	{
		// We're within range!  Get ready to rumble!
		rootFSM->SetNextState( Root::eStateGetLineOfSight );
		return;
	}

	// Possibly drop on our target.
	if( !m_bDropped )
	{
		if( GetAIPtr()->GetMovement()->GetControlFlags() & CM_FLAG_WALLWALK )
		{
			const LTVector & vMyDims = GetAIPtr()->GetDims();
			if(    fabs(vMyPos.x - vTargetPos.x) < vMyDims.x*2.0f
				&& fabs(vMyPos.z - vTargetPos.z) < vMyDims.z*2.0f 
				&& vMyPos.y > vTargetPos.y )
			{
				if( GetAIPtr()->SetNextAction(CAI::ACTION_DROP) )
				{
					// Be sure we only do this once.
					m_bDropped = LTTRUE;
					return;
				}
			}
		}
	}


	// Manage our advance node.
	if( m_pAdvanceNode )
	{
		// Stop pathing if we are done.
		if( m_StrategyFollowPath.IsDone() || m_StrategyFollowPath.IsUnset() || m_StrategyFollowPath.IsStuck() )
		{
			FindBetterAdvanceNode();
		}
		else
		{
			// Abandon our advance node if the the node is beyond our target.
			const LTVector vAIToTarget = GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition();
			const LTVector vAIToNode = m_pAdvanceNode->GetPos() - GetAIPtr()->GetPosition();

			if( vAIToTarget.Dot(vAIToNode) < 0.0f )
			{
				FindBetterAdvanceNode();
			}
		}
	}


	if( m_pAdvanceNode )
	{
		// Keep on path to our advance node.
		m_StrategyFollowPath.Update();
		return;
	}
	else
	{
		// Well, nothing to do but keep charging at the target!
		m_StrategyApproachTarget.Update();

		// If we get stuck, just start clawing!
		if( GetAIPtr()->GetMovePtr()->IsStuck() && !GetAIPtr()->GetMovePtr()->IsAvoidingIgnorable() )
		{
			// Firing our melee weapon will move us toward our target while
			// keep us inside the volume system.
			GetAIPtr()->SetNextAction( CAI::ACTION_FIREMELEE );
		}

	}
}

void  AlienAttackGoal::EndApproachTarget(RootFSM * rootFSM)
{
	if( m_pAdvanceNode )
	{
		m_pAdvanceNode->Unlock();
		m_pAdvanceNode = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::xxxGetLineOfSight
//
//	PURPOSE:	What the AI should do to get a solid firing position against the target.
//
// ----------------------------------------------------------------------- //
void  AlienAttackGoal::StartGetLineOfSight(RootFSM * rootFSM)
{
}

void  AlienAttackGoal::UpdateGetLineOfSight(RootFSM * rootFSM)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	if( !target.IsValid() )
	{
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}

	if( !WithinWeaponFireRange(GetAIPtr()->GetIdealWeaponButesPtr(), m_Butes, target.GetPosition() - GetAIPtr()->GetPosition()) )
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}


	// Just keep approaching until the target is partially visible.

	if( target.IsPartiallyVisible() )
	{
		rootFSM->AddMessage(Root::eEngageTarget);
		return;
	}

	m_StrategyApproachTarget.Update();

	// If we get stuck, just start clawing!
	if( GetAIPtr()->GetMovePtr()->IsStuck() && !GetAIPtr()->GetMovePtr()->IsAvoidingIgnorable() )
	{
		// Firing our melee weapon will move us toward our target while
		// keep us inside the volume system.
		GetAIPtr()->SetNextAction( CAI::ACTION_FIREMELEE );
	}

}

void  AlienAttackGoal::EndGetLineOfSight(RootFSM * rootFSM)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlienAttackGoal::xxxEngageTarget
//
//	PURPOSE:	What the AI should do to kill the target (ie. fire her weapon!)
//
// ----------------------------------------------------------------------- //
void  AlienAttackGoal::StartEngageTarget(RootFSM * rootFSM)
{
}

void  AlienAttackGoal::UpdateEngageTarget(RootFSM * rootFSM)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	if( !WithinWeaponFireRange(GetAIPtr()->GetIdealWeaponButesPtr(), m_Butes, target.GetPosition() - GetAIPtr()->GetPosition()) )
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}

	if( !target.IsPartiallyVisible() )
	{
		rootFSM->SetNextState(Root::eStateGetLineOfSight);
		return;
	}

	// Do the clawing or biting (or tail swiping).
	if( GetAIPtr()->GetCurrentWeaponPtr() )
	{
		if( GetAIPtr()->BurstDelayExpired() )
			GetAIPtr()->SetNextAction( CAI::ACTION_FIREMELEE );
	}
	else
	{
		// If we don't have a weapon, just keep approaching our target.
		const LTFLOAT fTargetRangeSqr = (target.GetPosition() - GetAIPtr()->GetPosition()).MagSqr();
		LTFLOAT fIdealRangeSqr = 0.0f;
		LTFLOAT fMinRangeSqr = -1.0f;
		if( m_Butes.fFacehugChance > 0.0f  )
		{
			fIdealRangeSqr = (m_Butes.fFacehugMaxRangeSqr + m_Butes.fFacehugMinRangeSqr)*0.25f;
			fMinRangeSqr = m_Butes.fFacehugMinRangeSqr*1.01f;
		}
		else if( m_Butes.fPounceChance > 0.0f )
		{
			fIdealRangeSqr = (m_Butes.fPounceMaxRangeSqr + m_Butes.fPounceMinRangeSqr)*0.25f;
			fMinRangeSqr = m_Butes.fPounceMinRangeSqr*1.01f;
		}
			
		if( fTargetRangeSqr > fIdealRangeSqr )
		{
			m_StrategyApproachTarget.Update();
		}
		else 
		{
			// Facehug or pounce if target is within 10% of our minimum range.
			if( m_Butes.fFacehugChance > 0.0f  )
			{
				if( m_tmrFacehug.Stopped() && fTargetRangeSqr < m_Butes.fFacehugMinRangeSqr*1.01f )
				{
					m_tmrFacehug.Start( GetRandom(m_Butes.fMinFacehugDelay, m_Butes.fMaxFacehugDelay) );
					GetAIPtr()->SetNextAction(CAI::ACTION_FACEHUG);
					return;
				}
			}
			else if( m_Butes.fPounceChance > 0.0f )
			{
				if( m_tmrPounce.Stopped() && fTargetRangeSqr < m_Butes.fPounceMinRangeSqr*1.01f )
				{
					m_tmrPounce.Start( GetRandom(m_Butes.fMinPounceDelay, m_Butes.fMaxPounceDelay) );
					GetAIPtr()->SetNextAction(CAI::ACTION_POUNCE);
					return;
				}
			}
		}


	}

	return;
}

void  AlienAttackGoal::EndEngageTarget(RootFSM * rootFSM)
{
	GetAIPtr()->GetAnimation()->SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
//
// Dumb Alien attack
//
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


LTFLOAT DumbAlienAttackGoal::GetBid()
{
	if( GetAIPtr()->GetTarget().IsValid() )
	{
		return g_pAIButeMgr->GetDefaultBid(BID_ATTACKSEEENEMY);
	}

	return 0.0f;
}

LTFLOAT DumbAlienAttackGoal::GetMaximumBid()
{
	return g_pAIButeMgr->GetDefaultBid(BID_ATTACKSEEENEMY);
}

void DumbAlienAttackGoal::Start()
{
	AICombatGoal::Start();

	GetAIPtr()->SetMovementStyle("Combat");

	GetAIPtr()->SetNextAction(CAI::ACTION_IDLE);

	if( GetAIPtr()->HasTarget() )
	{
		GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
	}

	// Always wall-walk.
	GetAIPtr()->Run();

}

void DumbAlienAttackGoal::Update()
{
	AICombatGoal::Update();

	// Wait for the idle action to kick in.
	// Is this needed?
	if( GetAIPtr()->GetCurrentAction() )
		return;

	// Can't really do anything if we don't have a target.
	if( !GetAIPtr()->GetTarget().IsValid() )
		return;

	// These two variables will have their y values removed after some other
	// constants are determined.
	LTVector vTargetPos = GetAIPtr()->GetTarget().GetPosition();
	LTVector vMyPos = GetAIPtr()->GetPosition();
	
	const LTFLOAT fTargetRangeSqr = (vTargetPos - vMyPos).MagSqr();
	const LTFLOAT fMyPosY = vMyPos.y;
	const LTFLOAT fTargetPosY = vTargetPos.y;

	vTargetPos.y = vMyPos.y = 0.0f;

	const LTVector vMyUp = GetAIPtr()->GetUp();
	const LTVector vMyRight = GetAIPtr()->GetRight();
	const LTVector vMyForward = GetAIPtr()->GetForward();

	const AIWBM_AIWeapon * pCurrentWeaponProfile = GetAIPtr()->GetCurrentWeaponButesPtr();
	if(    pCurrentWeaponProfile 
		&& fTargetRangeSqr > pCurrentWeaponProfile->fMinFireRange*pCurrentWeaponProfile->fMinFireRange
		&& fTargetRangeSqr < pCurrentWeaponProfile->fMaxFireRange*pCurrentWeaponProfile->fMaxFireRange )
	{
		GetAIPtr()->StandUp();

		if( GetAIPtr()->BurstDelayExpired() )
		{
			GetAIPtr()->SetNextAction( CAI::ACTION_FIREMELEE );
			return;
		}
	}
	else
	{
		GetAIPtr()->WallWalk();
	}

	GetAIPtr()->SnapTurn( vTargetPos - vMyPos );

	if(    GetAIPtr()->WithinLeash(GetAIPtr()->GetPosition())
		|| vMyForward.Dot(GetAIPtr()->GetLeashPoint() - GetAIPtr()->GetPosition()) > 0.0f )
	{
		GetAIPtr()->Forward();
	}
}

void DumbAlienAttackGoal::End()
{
	AICombatGoal::End();

	GetAIPtr()->LookAt( LTNULL );
	GetAIPtr()->SetMovementStyle();

	// Always wall-walk.
	GetAIPtr()->Run();

}

#ifdef ALIEN_ATTACK_DEBUG

void AlienAttackGoal::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateWaitForNewTarget : szMessage = "eStateWaitForNewTarget"; break;
		case Root::eStateApproachTarget : szMessage = "eStateApproachTarget"; break;
		case Root::eStateGetLineOfSight : szMessage = "eStateGetLineOfSight"; break;
		case Root::eStateEngageTarget : szMessage = "eStateEngageTarget"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Attack::RootFSM is entering state %s.", szMessage );
}

void AlienAttackGoal::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = "Unknown";
/*	switch( message )
	{
		default : szMessage = "Unknown"; break;
	}
*/
	if( fsm.HasTransition( fsm.GetState(), message ) )
	{
		AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Attack::RootFSM is processing %s.", szMessage );
	}
}

#endif
