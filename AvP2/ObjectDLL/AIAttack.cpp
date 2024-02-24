// ----------------------------------------------------------------------- //
//
// MODULE  : Attack.h
//
// PURPOSE : Implements the Attack goal and state.
//
// CREATED : 6/14/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AIAttack.h"
#include "AI.h"
#include "AITarget.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "CharacterMgr.h"
#include "AINodeMgr.h"
#include "AIActionMisc.h"
#include "AIVolume.h"
#include "SoundButeMgr.h"
#include "AIAnimButeMgr.h"

#ifdef _DEBUG
#define MILD_ATTACK_DEBUG
//#define ATTACK_DEBUG
#endif


static bool WithinWeaponFireRange(const AIWBM_AIWeapon * pWeaponButes, const LTVector & vRelativePos)
{
	if( !pWeaponButes )
		return true;

	return vRelativePos.MagSqr() < pWeaponButes->fMaxFireRange*pWeaponButes->fMaxFireRange;
}

static bool WithinWeaponIdealRange(const AIWBM_AIWeapon * pWeaponButes, const LTVector & vRelativePos)
{
	if( !pWeaponButes )
		return true;

	return vRelativePos.MagSqr() < pWeaponButes->fMaxIdealRange*pWeaponButes->fMaxIdealRange;
}

static bool InsideWeaponIdealRange(const AIWBM_AIWeapon * pWeaponButes, const LTVector & vRelativePos)
{
	if( !pWeaponButes )
		return true;

	return vRelativePos.MagSqr() < pWeaponButes->fMinIdealRange*pWeaponButes->fMinIdealRange;
}

/////////////////////////////////////////////////
//
//     Attack Goal
//
/////////////////////////////////////////////////

AttackGoal::AttackGoal( CAI * pAI, std::string create_name)
	: AICombatGoal(pAI, create_name),
	  m_StrategyApproachTarget(pAI),
	  m_StrategyFollowPath(pAI),
	  m_StrategyRetreat(pAI),
	  m_AttackButes( g_pAIButeMgr->GetAttack(pAI->GetTemplateId()) ),
	  m_pCurCoverNode(LTNULL),
	  m_pLastNodeReached(LTNULL),
	  m_bSentUnreachableCmd(LTFALSE),
	  m_fCost(0.0f),
	  m_bFiredOnApproach(LTFALSE),
	  m_fApproachTime(0.0f)
{
	ASSERT( pAI );
	
	SetupFSM();
}

AttackGoal::~AttackGoal()
{
}


void AttackGoal::SetupFSM()
{

	//
	// Set up the root FSM.
	//

	m_RootFSM.DefineState( Root::eStateWaitForNewTarget, 
		                   m_RootFSM.MakeCallback(*this,&AttackGoal::UpdateWaitForNewTarget),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::StartWaitForNewTarget),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::EndWaitForNewTarget)   );

	m_RootFSM.DefineState( Root::eStateApproachTarget, 
		                   m_RootFSM.MakeCallback(*this,&AttackGoal::UpdateApproachTarget),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::StartApproachTarget),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::EndApproachTarget) );

	m_RootFSM.DefineState( Root::eStateGetLineOfSight, 
		                   m_RootFSM.MakeCallback(*this,&AttackGoal::UpdateGetLineOfSight),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::StartGetLineOfSight),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::EndGetLineOfSight) );

	m_RootFSM.DefineState( Root::eStateRunForCover, 
		                   m_RootFSM.MakeCallback(*this,&AttackGoal::UpdateRunForCover),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::StartRunForCover),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::EndRunForCover) );

	m_RootFSM.DefineState( Root::eStateEngageTarget, 
		                   m_RootFSM.MakeCallback(*this,&AttackGoal::UpdateEngageTarget),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::StartEngageTarget),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::EndEngageTarget) );

	m_RootFSM.DefineState( Root::eStateFear, 
		                   m_RootFSM.MakeCallback(*this,&AttackGoal::UpdateFear),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::StartFear),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::EndFear) );

	m_RootFSM.DefineState( Root::eStateEngageMelee, 
		                   m_RootFSM.MakeCallback(*this,&AttackGoal::UpdateEngageMelee),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::StartEngageMelee),
						   m_RootFSM.MakeCallback(*this,&AttackGoal::EndEngageMelee) );

	m_RootFSM.DefineTransition( Root::eNoTarget, Root::eStateWaitForNewTarget );

	m_RootFSM.SetInitialState( Root::eStateWaitForNewTarget );

#ifdef ATTACK_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&AttackGoal::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&AttackGoal::PrintRootFSMMessage));
#endif
}

#ifndef _FINAL
const char * AttackGoal::GetDescription() const
{ 
	if( GetAIPtr() )
	{
		if( !GetAIPtr()->GetCurrentWeaponButesPtr() )
			return "Attack ***** No Weapon!"; 
	}

	return "Attack";
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void AttackGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	AICombatGoal::Load(hRead);

	*hRead >> m_NewTarget;

	*hRead >> m_RootFSM;
	*hRead >> m_StrategyApproachTarget;
	*hRead >> m_StrategyFollowPath;
	*hRead >> m_StrategyRetreat;

	m_pCurCoverNode = dynamic_cast<CAINodeCover*>( g_pAINodeMgr->GetNode(hRead->ReadDWord()) );
	m_pLastNodeReached = dynamic_cast<CAINodeCover*>( g_pAINodeMgr->GetNode(hRead->ReadDWord()) );
	
	// m_AttackButes is set upon construction.
	*hRead >> m_tmrDodgeDelay;
	*hRead >> m_tmrBackoffDelay;
	*hRead >> m_tmrRunForCoverDelay;
	*hRead >> m_tmrCoverDuration;
	*hRead >> m_tmrCrouchDelay;

	*hRead >> m_bSentUnreachableCmd;

	*hRead >> m_tmrMeleeRepath;
	*hRead >> m_fCost;
	*hRead >> m_bFiredOnApproach;
	*hRead >> m_fApproachTime;
}


void AttackGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	AICombatGoal::Save(hWrite);
	
	*hWrite << m_NewTarget;

	*hWrite << m_RootFSM;
	*hWrite << m_StrategyApproachTarget;
	*hWrite << m_StrategyFollowPath;
	*hWrite << m_StrategyRetreat;

	if( m_pCurCoverNode )
		hWrite->WriteDWord( m_pCurCoverNode->GetID() );
	else
		hWrite->WriteDWord( CAINode::kInvalidID );

	if( m_pLastNodeReached )
		hWrite->WriteDWord( m_pLastNodeReached->GetID() );
	else
		hWrite->WriteDWord( CAINode::kInvalidID );

	// m_AttackButes is set upon construction.
	*hWrite << m_tmrDodgeDelay;
	*hWrite << m_tmrBackoffDelay;
	*hWrite << m_tmrRunForCoverDelay;
	*hWrite << m_tmrCoverDuration;
	*hWrite << m_tmrCrouchDelay;

	*hWrite << m_bSentUnreachableCmd;

	*hWrite << m_tmrMeleeRepath;
	*hWrite << m_fCost;
	*hWrite << m_bFiredOnApproach;
	*hWrite << m_fApproachTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAttack::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT AttackGoal::GetBid()
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
			fBid = fDefaultOther;

			// If we saw the enemy, we need to go to a higher bid.
			if(    m_NewTarget.eReason == NewTarget::SeeEnemy
				|| m_NewTarget.eReason == NewTarget::Damage
				|| m_NewTarget.eReason == NewTarget::HearAllyWeaponFire 
				|| m_NewTarget.eReason == NewTarget::HearBattleCry )
			{
				fBid = fDefaultSeeEnemy;
			}
		}
		else if( GetAIPtr()->HasTarget() )
		{
			fBid = fDefaultOther;
			if( GetAIPtr()->GetTarget().IsMaintained() )
			{
				fBid = fDefaultSeeEnemy;
			}
		}
		else if (HearFullWeaponImpact())
		{
			fBid = fDefaultSeeEnemy;
		}

		// Only enter attack goal if we have a weapon!
		if (fBid > 0.0f)
		{
			if( m_NewTarget.hTarget )
				GetAIPtr()->ChooseBestWeapon(m_NewTarget.vPos);

			if (!GetAIPtr()->GetCurrentWeaponPtr())
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
		if ( fBid == 0.0f && HearPartialWeaponImpact())
		{
			fBid = fDefaultOther;
		}

		// If we don't have a weapon, don't keep attacking!
		if (!GetAIPtr()->GetCurrentWeaponPtr())
			fBid = 0.0f;
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAttack::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT AttackGoal::GetMaximumBid()
{
	return 	g_pAIButeMgr->GetDefaultBid(BID_ATTACKSEEENEMY);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAttack::Start
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AttackGoal::Start()
{
	AICombatGoal::Start();

	ASSERT( GetAIPtr() );
	if( !GetAIPtr() ) return;

	m_StrategyFollowPath.Clear();

	// Initialize the goal.
	if( m_NewTarget.hTarget )
	{
		GetAIPtr()->Target( m_NewTarget.hTarget, m_NewTarget.vPos );
	}
	
	if( GetAIPtr()->HasTarget() )
	{
		GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
	}

	m_tmrWaitForNewTarget.Stop();

	// Look mad!
	GetAIPtr()->SetAnimExpression(EXPRESSION_ANGRY);

	// Be sure we are running.
	GetAIPtr()->Run();


	// Open up and say "Arrrggghhh"
	int nButes = -1;
	if(    m_NewTarget.eReason != NewTarget::HearBattleCry 
		&& m_NewTarget.eReason != NewTarget::HearAllyWeaponFire	)
	{
		nButes = GetAIPtr()->PlayExclamationSound("Attack");
	}
	else
	{
		nButes = GetAIPtr()->PlayExclamationSound("FollowBattleCry");
	}

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

	// Get in the right state.
	if( GetAIPtr()->HasTarget() )
	{
		m_RootFSM.SetState( Root::eStateApproachTarget );
	}
	else
	{
		m_RootFSM.SetState( Root::eStateWaitForNewTarget );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAttack::Update
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AttackGoal::Update()
{
	AICombatGoal::Update(); 

	// Recognize a dead or missing target.
	if( !GetAIPtr()->HasTarget() )
	{
		m_RootFSM.AddMessage(Root::eNoTarget);
	}

	m_RootFSM.Update();

	// Handle a time-out when we don't have a target.
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
//	ROUTINE:	CAIStateAttack::End
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void AttackGoal::End()
{
	AICombatGoal::End();

	m_NewTarget = NewTarget();

	GetAIPtr()->SetAnimExpression(EXPRESSION_NORMAL);
	GetAIPtr()->SetMovementStyle();
	GetAIPtr()->SetFlashlight(LTFALSE);
	GetAIPtr()->ClearAim();
	GetAIPtr()->LookAt( LTNULL );

	if( m_pCurCoverNode )
	{
		m_pCurCoverNode->Unlock();
		m_pCurCoverNode = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::xxxxWaitForNewTarget
//
//	PURPOSE:	What the AI should do when she doesn't have a target.
//
// ----------------------------------------------------------------------- //

void AttackGoal::StartWaitForNewTarget(RootFSM * rootFSM)
{
	GetAIPtr()->SetMovementStyle("Investigate");
	GetAIPtr()->ClearAim();
	return;
}

void AttackGoal::UpdateWaitForNewTarget(RootFSM * rootFSM)
{
	// Just sit and wait for a target.
	if( GetAIPtr()->HasTarget() )
	{
		GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}

	return;
}

void AttackGoal::EndWaitForNewTarget(RootFSM * rootFSM)
{
	GetAIPtr()->SetMovementStyle();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::xxxApproachTarget
//
//	PURPOSE:	What the AI should do to get within firing range of the target
//
// ----------------------------------------------------------------------- //
void  AttackGoal::StartApproachTarget(RootFSM * rootFSM)
{
	m_bSentUnreachableCmd = LTFALSE;
	GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
	GetAIPtr()->SetMovementStyle("Combat");

	const AIBM_ApproachTarget & butes = g_pAIButeMgr->GetApproachTarget(GetAIPtr()->GetTemplateId());

	if (GetAIPtr()->CanUseCover())
	{
		CAINodeCover * pCoverNode = g_pAINodeMgr->FindAdvanceNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());
		if (pCoverNode && m_StrategyFollowPath.Set(pCoverNode) )
		{
			if (m_pCurCoverNode)
				m_pCurCoverNode->Unlock();

			m_pCurCoverNode = pCoverNode;
			m_pCurCoverNode->Lock(GetAIPtr()->GetObject());

			m_fCost = m_pCurCoverNode->GetCost();

			const LTFLOAT fTime = GetApproachTime() * GetRandom(butes.fMinFireTime, butes.fMaxFireTime);
			m_tmrApproach.Start(fTime);
			m_bFiredOnApproach = LTFALSE;
		}
	}

	m_StrategyApproachTarget.Reset();
}

void  AttackGoal::UpdateApproachTarget(RootFSM * rootFSM)
{
	const AIBM_ApproachTarget & butes = g_pAIButeMgr->GetApproachTarget(GetAIPtr()->GetTemplateId());

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

	if (GetAIPtr()->HasDamageFear() || GetAIPtr()->HasOddsFear())
	{
		rootFSM->SetNextState(Root::eStateFear);
		return;
	}

	// If our ideal weapon is a melee weapon, just go to melee attack.
	// This is intentionally done after the advance to cover node stuff.
	const AIWBM_AIWeapon * const pIdealWeaponButes = GetAIPtr()->GetIdealWeaponButesPtr();
	if( pIdealWeaponButes && pIdealWeaponButes->bMeleeWeapon )
	{
		m_RootFSM.SetNextState( Root::eStateEngageMelee );
	}

	const LTVector & vTargetPos = GetAIPtr()->GetTarget().GetPosition();
	const LTVector & vAIPos = GetAIPtr()->GetPosition();
	if (WithinWeaponIdealRange(pIdealWeaponButes, vTargetPos - vAIPos)
		&& GetAIPtr()->GetTarget().IsMaintained())
	{
		// We're within range!  Get ready to rumble!
		rootFSM->SetNextState( Root::eStateGetLineOfSight );
		return;
	}

	// Move towards the target.

	// Are we using Cover?
	if (GetAIPtr()->CanUseCover())
	{
		// Try to get a node if we don't have one yet
		// We don't need to do the standard lock check, since we already verified
		// that the current node is not set
		if (!m_pCurCoverNode)
		{
			CAINodeCover * pCurCoverNode = g_pAINodeMgr->FindAdvanceNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());
			if (pCurCoverNode && m_StrategyFollowPath.Set(pCurCoverNode))
			{
				if( m_pCurCoverNode )
					m_pCurCoverNode->Unlock();

				m_pCurCoverNode = pCurCoverNode;
				m_pCurCoverNode->Lock(GetAIPtr()->GetObject());
				

				m_fCost = m_pCurCoverNode->GetCost();

				const LTFLOAT fTime = GetApproachTime() * GetRandom(butes.fMinFireTime, butes.fMaxFireTime);
				m_tmrApproach.Start(fTime);
				m_bFiredOnApproach = LTFALSE;
			}
		}

		// Got one
		if (m_pCurCoverNode && (m_pCurCoverNode != m_pLastNodeReached))
		{
			if (m_tmrApproach.Stopped() && !m_bFiredOnApproach)
			{
				if (GetRandom(0.0f, 1.0f) < butes.fFireChance)
				{
					FireOnTarget(LTFALSE, LTTRUE);	// no sight or range check
					m_bFiredOnApproach = LTTRUE;
				}
			}

			if( m_StrategyFollowPath.IsDone() )
			{
				// Don't unlock yet because EngageTarget may use this node for crouching

				// We have reached our cover node, start attacking.
				m_pLastNodeReached = m_pCurCoverNode;
				rootFSM->SetNextState( Root::eStateGetLineOfSight );
				return;
			}
			m_StrategyFollowPath.Update();
			return;
		}
	}

	m_StrategyApproachTarget.Update();

	// We want to fire, because we may have an alternate weapon with a different range.
	if(    GetAIPtr()->GetCurrentWeaponPtr()
		&& GetAIPtr()->GetCurrentWeaponButesPtr()
		&& !NeedReload(*GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr()) )
	{
		if( FireOnTarget(LTTRUE) )
			return;
	}


	if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
	}
}

void  AttackGoal::EndApproachTarget(RootFSM * rootFSM)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::xxxGetLineOfSight
//
//	PURPOSE:	What the AI should do to get a solid firing position against the target.
//
// ----------------------------------------------------------------------- //
void  AttackGoal::StartGetLineOfSight(RootFSM * rootFSM)
{
	GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
	GetAIPtr()->SetMovementStyle("Combat");

	m_tmrStuckFire.Stop();

	if (GetAIPtr()->GetLightLevel(GetAIPtr()->GetTarget().GetPosition()) < 0.2f)
		GetAIPtr()->SetFlashlight(LTTRUE);
}

void  AttackGoal::UpdateGetLineOfSight(RootFSM * rootFSM)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	// Eeeek!
	if (GetAIPtr()->HasDamageFear() || GetAIPtr()->HasOddsFear())
	{
		rootFSM->SetNextState(Root::eStateFear);
		return;
	}

	if( m_StrategyApproachTarget.LostTarget() && !GetAIPtr()->NeverLoseTarget() )
	{
		// Clear our old target and wait for a new one.
		GetAIPtr()->Target(LTNULL);
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}

	// If our ideal is a melee weapon, use that!
	const AIWBM_AIWeapon * const pIdealWeaponButes = GetAIPtr()->GetIdealWeaponButesPtr();
	if( pIdealWeaponButes && pIdealWeaponButes->bMeleeWeapon )
	{
		m_RootFSM.SetNextState( Root::eStateEngageMelee );
	}

	// If the target is outside our ideal weapon's ideal range, keep approaching!
	if( !WithinWeaponIdealRange(pIdealWeaponButes, target.GetPosition() - GetAIPtr()->GetPosition()) )
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}


	// Just keep approaching until the target is partially visible.
	ObjectFilterFn ofn = CAI::DefaultFilterFn;
	PolyFilterFn pfn = LTNULL;

	if( GetAIPtr()->CanShootThrough() )
	{
		ofn = CAI::ShootThroughFilterFn;
	}

	if( GetAIPtr()->IsTargetVisibleFromWeapon( ofn, pfn ) )
	{
		const CCharacter * pClipsInto = WillClip( *GetAIPtr(), GetAIPtr()->GetPosition() );
		if( !pClipsInto || !g_pCharacterMgr->AreAllies(*GetAIPtr(),*pClipsInto) )
		{
			// Okay, the target is visible, just make sure we 
			rootFSM->SetNextState(Root::eStateEngageTarget);
			return;
		}
	}

	// If we are stuck, just start shooting!
	if( GetAIPtr()->GetMovePtr()->IsStuck() && GetAIPtr()->GetTarget().IsPartiallyVisible() )
	{
		m_tmrStuckFire.Start( GetRandom(m_AttackButes.fMinStuckFireDuration, m_AttackButes.fMaxStuckFireDuration) );
		rootFSM->SetNextState(Root::eStateEngageTarget);
	}

	
	m_StrategyApproachTarget.Update();


	if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
	}
}

void  AttackGoal::EndGetLineOfSight(RootFSM * rootFSM)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::xxxRunForCover
//
//	PURPOSE:	AI runs for cover.
//
// ----------------------------------------------------------------------- //
void  AttackGoal::StartRunForCover(RootFSM * rootFSM)
{
	GetAIPtr()->LookAt( LTNULL );
	GetAIPtr()->SetMovementStyle("Combat");

	m_tmrCoverDuration.Pause();
}


void  AttackGoal::UpdateRunForCover(RootFSM * rootFSM)
{
	// Go to the cover node.

	// Follow path should have been set when the cover node was stored.
	if(    !m_pCurCoverNode 
		|| m_StrategyFollowPath.IsUnset() 
		|| GetAIPtr()->GetMovePtr()->IsStuck() )
	{
		rootFSM->SetNextState( Root::eStateApproachTarget );
		return;
	}

	
	if( m_StrategyFollowPath.IsDone() )
	{
		GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );

		// Start the hide in cover timer.
		if( m_tmrCoverDuration.Paused() )
		{
			m_tmrCoverDuration.Resume();
		}

		// If we need to reload, now would be a good time.
		if(    GetAIPtr()->GetCurrentWeaponPtr()
			&& GetAIPtr()->GetCurrentWeaponButesPtr()
			&& NeedReload(*GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr()) )
		{
			if( m_pCurCoverNode && m_pCurCoverNode->GetCrouch() )
			{
				if( GetAIPtr()->SetNextAction(CAI::ACTION_CROUCHRELOAD) )
					return;
			}
			else
			{
				if( GetAIPtr()->SetNextAction(CAI::ACTION_RELOAD) )
					return;
			}
		}

		// If the hide in cover timer has run out, start attacking again.
		if( m_tmrCoverDuration.Stopped() )
		{
			// We have reached our cover node and waited long enough, start attacking.
			rootFSM->SetNextState( Root::eStateEngageTarget );
			return;
		}

		// If we are supposed to be crouched, crouch!
		if( m_pCurCoverNode && m_pCurCoverNode->GetCrouch() )
		{
			AIActionCrouch * pCrouchAction = dynamic_cast<AIActionCrouch*>( GetAIPtr()->SetNextAction(CAI::ACTION_CROUCH) );
			if( pCrouchAction )
			{
				pCrouchAction->Init( m_pCurCoverNode );
				return;
			}
		}
	}
	else
	{
		// Keep firing at our target (yep, we're pretty mean and nasty).
		if(    GetAIPtr()->GetCurrentWeaponPtr()
			&& GetAIPtr()->GetCurrentWeaponButesPtr()
			&& !NeedReload(*GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr()) )
		{
			if( FireOnTarget(LTTRUE) )
				return;
		}

		m_StrategyFollowPath.Update();
	}

	if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
	}


}

void  AttackGoal::EndRunForCover(RootFSM * rootFSM)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::xxxEngageTarget
//
//	PURPOSE:	What the AI should do to kill the target (ie. fire her weapon!)
//
// ----------------------------------------------------------------------- //
void  AttackGoal::StartEngageTarget(RootFSM * rootFSM)
{
	GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
	GetAIPtr()->SetMovementStyle("Combat");

	// Reset the dodging.
	m_tmrDodgeDelay.Stop();
	m_tmrBackoffDelay.Stop();
}

void  AttackGoal::UpdateEngageTarget(RootFSM * rootFSM)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	if (GetAIPtr()->HasDamageFear() || GetAIPtr()->HasOddsFear())
	{
		rootFSM->SetNextState(Root::eStateFear);
		return;
	}

	if( !target.IsMaintained() )
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}

	const LTVector vRelativeTargetPos = target.GetPosition() - GetAIPtr()->GetPosition();
	if( !WithinWeaponIdealRange(GetAIPtr()->GetIdealWeaponButesPtr(), vRelativeTargetPos ) )
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}

	if( Melee() )
		return;

	if (GetAIPtr()->CanUseCover() && m_pCurCoverNode)
	{
		// Should we crouch?
		if( m_pCurCoverNode->GetCrouch() )
		{
			if( Crouch() )
				return;
		}

		// if we've advanced out into the open
		if (!m_pCurCoverNode->IsOccupied(GetAIPtr()->GetObject()))
		{
			if (RunForCover())
				return;
		}
	}
	else   // !( m_pCurCoverNode )
	{
		// Possibley run for cover.
		if( RunForCover() )
			return;

		// Possibly dodge.
		if( Dodge() )
			return;

		// Possibly perform a back-off maneuver.
		if( Backoff() )
			return;
		
	}  //if( m_pCurCoverNode )

	// Reload the weapon if we need be.
	if(    GetAIPtr()->GetCurrentWeaponPtr()
		&& GetAIPtr()->GetCurrentWeaponButesPtr()
		&& NeedReload(*GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr()) )
	{
		if( m_pCurCoverNode && m_pCurCoverNode->GetCrouch() )
		{
			if( !m_pCurCoverNode->IsCoverFromTarget(*GetAIPtr()) )
			{
				// Search for another node.
				CAINodeCover * pCoverNode = g_pAINodeMgr->FindDodgeNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());
				if (pCoverNode && m_StrategyFollowPath.Set(pCoverNode) )
				{
					if (m_pCurCoverNode)
						m_pCurCoverNode->Unlock();
					
					m_pCurCoverNode = pCoverNode;
					m_pCurCoverNode->Lock(GetAIPtr()->GetObject());
					m_tmrCoverDuration.Start(GetRandom(m_AttackButes.fMinCoverDuration, m_AttackButes.fMaxCoverDuration));
					m_RootFSM.SetNextState(Root::eStateRunForCover);
					return;
				}
			}

			if( GetAIPtr()->SetNextAction(CAI::ACTION_CROUCHRELOAD) )
				return;
		}
		
		// Okay, no fancy cover node stuff, just do a plain old reload.
		if( GetAIPtr()->SetNextAction(CAI::ACTION_RELOAD) )
			return;
	}

	// Be sure we still have a line of sight.
	ObjectFilterFn ofn = CAI::DefaultFilterFn;
	PolyFilterFn pfn = LTNULL;

	if( GetAIPtr()->CanShootThrough() )
	{
		ofn = CAI::ShootThroughFilterFn;
	}

	if( m_tmrStuckFire.Stopped() && !GetAIPtr()->IsTargetVisibleFromWeapon( ofn, pfn ) )
	{
		rootFSM->SetNextState(Root::eStateGetLineOfSight);
		return;
	}

	// If we are not allowed to turn while firing, we may need to turn right now!
	if( !GetAIPtr()->AllowSnapTurn() )
	{
		const LTVector vAimDir = target.GetPosition() - GetAIPtr()->GetAimFromPosition();

		const LTFLOAT aim_dot_forward = vAimDir.Dot(GetAIPtr()->GetForward());
		if(    aim_dot_forward < 0.0f 
			|| aim_dot_forward*aim_dot_forward < c_fCos45*c_fCos45*vAimDir.MagSqr() )
		{
			const LTVector & my_dims = GetAIPtr()->GetDims();
			if( vAimDir.MagSqr() > my_dims.x*my_dims.x + my_dims.z*my_dims.z )
			{
				// Do the turn.
				AIActionTurnTo * pNewAction = dynamic_cast<AIActionTurnTo*>( GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO) );
				if( pNewAction )
				{
					pNewAction->Init(vAimDir);
					return;
				}
			}
		}
	}


	// Alright already, let's fire our weapon!
	const AIWBM_AIWeapon * pIdealWeaponProfile = GetAIPtr()->GetIdealWeaponButesPtr();
	if( pIdealWeaponProfile )
	{
		const LTVector vTargetRange = ( GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition() );
		const LTFLOAT  fTargetRangeSqr = vTargetRange.MagSqr();

		// Only fire if within range of current weapon.
		if( FireOnTarget(LTFALSE) )
		{
			return;
		}

	} //if( pIdealWeaponProfile )

	// Go to a weapon up position if we aren't doing anything else.
	if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
	}

	return;
}

void  AttackGoal::EndEngageTarget(RootFSM * rootFSM)
{
	m_tmrStuckFire.Stop();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::xxxFear
//
//	PURPOSE:	What the AI should do when they are "afraid".
//
// ----------------------------------------------------------------------- //

void AttackGoal::StartFear(RootFSM * rootFSM)
{
	GetAIPtr()->LookAt( LTNULL );
	GetAIPtr()->SetMovementStyle("Combat");

	//	m_StrategyRetreat.SetFlee(LTTRUE);
	GetAIPtr()->Run();
	m_StrategyRetreat.Start();

	// Turn flashlight off when retreating
	if (GetAIPtr()->IsFlashlightOn())
		GetAIPtr()->SetFlashlight(LTFALSE);
}

void AttackGoal::UpdateFear(RootFSM * rootFSM)
{
	const AIBM_Influence & influence = g_pAIButeMgr->GetInfluence(GetAIPtr()->GetTemplateId());

	if (!GetAIPtr())
		return;

	// Running away because there aren't enough friendly AI's nearby --
	// try to retreat in the direction of the nearest ally
	if (GetAIPtr()->HasOddsFear())
	{
		const CCharacter *pChar = GetClosestAllyPtr(*GetAIPtr());
		if (pChar)
		{
			const LTFLOAT fDistSqr = VEC_DISTSQR(pChar->GetPosition(), GetAIPtr()->GetPosition());
			if (fDistSqr > influence.fCharacterRange * influence.fCharacterRange)
				m_StrategyRetreat.FindFriend(pChar->m_hObject);
			else
				m_StrategyRetreat.FindFriend(LTNULL);
		}
	}

	if (!GetAIPtr()->HasOddsFear() && !GetAIPtr()->HasDamageFear())
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}

	if (m_StrategyRetreat.IsCornered())
	{
		if( GetAIPtr()->GetCurrentWeaponPtr() && GetAIPtr()->GetCurrentWeaponButesPtr() )
		{
			if( NeedReload( *GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr() ) )
			{
				GetAIPtr()->SetNextAction( CAI::ACTION_RELOAD );
				return;
			}
			else
			{
				if( FireOnTarget(LTTRUE) )
				{
					return;
				}
				else
				{
					GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
					return;
				}
			}
		}
	}

	m_StrategyRetreat.Update();
	
	if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
	}

	return;
}

void AttackGoal::EndFear(RootFSM * rootFSM)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::xxxEngageMelee
//
//	PURPOSE:	Melee combat
//
// ----------------------------------------------------------------------- //

void AttackGoal::StartEngageMelee(RootFSM * rootFSM)
{
	GetAIPtr()->LookAt( GetAIPtr()->GetTarget().GetObject() );
	GetAIPtr()->SetMovementStyle("Combat");

	// Be sure combat goal won't switch our weapon on us!
	AllowWeaponSwitch(LTFALSE);
	
	// Reset the pathing.
	m_tmrMeleeRepath.Stop();
}

void AttackGoal::UpdateEngageMelee(RootFSM * rootFSM)
{
	const CAITarget & target = GetAIPtr()->GetTarget();
	const LTVector & my_pos = GetAIPtr()->GetPosition();

	// Be sure we have a weapon.
	const AIWBM_AIWeapon * const pWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();
	if( !pWeaponButes )
	{
		m_RootFSM.SetNextState( Root::eStateApproachTarget );
		return;
	}

	// Possibly leave this state if our ideal weapon is not a melee weapon.
	const AIWBM_AIWeapon * const pIdealWeaponButes = GetAIPtr()->GetIdealWeaponButesPtr();
	if( pIdealWeaponButes && !pIdealWeaponButes->bMeleeWeapon )
	{
		// Leave this state if the target is outside our exit distance.
		if( (target.GetPosition() - my_pos).MagSqr() > m_AttackButes.fExitMeleeDist*m_AttackButes.fExitMeleeDist )
		{
			m_RootFSM.SetNextState( Root::eStateApproachTarget );
		}
	}

	// If we aren't close enough, approach our target.
	if( !WithinWeaponFireRange(pWeaponButes, target.GetPosition() - my_pos) )
	{
		if( !m_StrategyFollowPath.IsSet() || m_tmrMeleeRepath.Stopped() )
		{
			if( !m_StrategyFollowPath.Set(target.GetPosition()) )
				m_StrategyFollowPath.Set( target.GetCharacterPtr() );

			// This is just hardcoded for now.
			m_tmrMeleeRepath.Start( GetRandom(0.1f, 0.3f) );
		}

		m_StrategyFollowPath.Update();

		if( GetAIPtr()->GetMovePtr()->IsStuck() && !GetAIPtr()->GetMovePtr()->IsAvoidingIgnorable() )
		{
			// Firing our melee weapon will move us toward our target while
			// keep us inside the volume system.
			if( GetAIPtr()->BurstDelayExpired() )
				GetAIPtr()->SetNextAction( CAI::ACTION_FIREMELEE );
		}

		return;
	}

	// Fire our melee weapon!
	if( GetAIPtr()->BurstDelayExpired() )
		GetAIPtr()->SetNextAction( CAI::ACTION_FIREMELEE );

	if( GetAIPtr()->CanDoNewAction() && !GetAIPtr()->GetCurrentAction() )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_AIM);
	}
}

void AttackGoal::EndEngageMelee(RootFSM * rootFSM)
{
	AllowWeaponSwitch(LTTRUE);

	GetAIPtr()->ChooseBestWeapon(GetAIPtr()->GetTarget().GetPosition());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::FireOnTarget
//
//	PURPOSE:	Decides and starts the AI firing.
//
// ----------------------------------------------------------------------- //

LTBOOL AttackGoal::FireOnTarget(LTBOOL bIfVisible, LTBOOL bIgnoreRange)
{
	// Don't fire while wall-walking!
	if( GetAIPtr()->GetMovement()->GetCharacterButes()->m_bWallWalk
		&& (GetAIPtr()->GetMovement()->GetControlFlags() & CM_FLAG_WALLWALK) )
		return LTFALSE;

	const LTFLOAT target_range_sqr 
		= ( GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition() ).MagSqr();

	// Only fire if within range of current weapon.
	const AIWBM_AIWeapon * pCurrentWeaponProfile = GetAIPtr()->GetCurrentWeaponButesPtr();
	if((    pCurrentWeaponProfile 
		&& target_range_sqr > pCurrentWeaponProfile->fMinFireRange*pCurrentWeaponProfile->fMinFireRange
		&& target_range_sqr < pCurrentWeaponProfile->fMaxFireRange*pCurrentWeaponProfile->fMaxFireRange )
		|| bIgnoreRange )
	{
		// Don't fire if we need to reload.
		if(    !GetAIPtr()->GetCurrentWeaponPtr() 
			|| NeedReload(*GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr()) )
			return LTFALSE;

		// Only fire if target can be seen.
		LTBOOL bTargetCanBeHit = GetAIPtr()->GetTarget().IsPartiallyVisible();
		if( bTargetCanBeHit || !bIfVisible )
		{
			ObjectFilterFn ofn = CAI::DefaultFilterFn;
			PolyFilterFn pfn = LTNULL;

			if( GetAIPtr()->CanShootThrough() )
			{
				ofn = CAI::ShootThroughFilterFn;
			}

			bTargetCanBeHit = GetAIPtr()->IsTargetVisibleFromWeapon( ofn, pfn );
		}

		if( bTargetCanBeHit )
		{
			// Must have full SeeEnemy stimulation in order to fire at a cloaked enemy
			if (GetAIPtr()->GetTarget().GetCharacterPtr()->GetCloakState() != CLOAK_STATE_INACTIVE)
			{
				if (!GetAIPtr()->GetTarget().IsFullyVisible())
					return LTFALSE;
			}

			if( GetAIPtr()->BurstDelayExpired() )
			{
				if( pCurrentWeaponProfile->bMeleeWeapon )
				{
					GetAIPtr()->SetNextAction( CAI::ACTION_FIREMELEE );
				}
				else
				{
					if (!GetAIPtr()->WillHurtFriend())
						GetAIPtr()->SetNextAction( CAI::ACTION_FIREWEAPON );
				}

				return LTTRUE;
			}
		}
	}

	return LTFALSE;		// did not fire
}

LTFLOAT AttackGoal::GetApproachTime()
{
	const LTFLOAT fSpeed = GetAIPtr()->GetMovement()->GetMaxSpeed(CMS_RUNNING);
	
	if (fSpeed > 0.0f)
		return m_fCost / fSpeed;
	else
		return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::RunForCover
//
//	PURPOSE:	Decides and starts the AI running for cover.
//
// ----------------------------------------------------------------------- //

LTBOOL AttackGoal::RunForCover()
{
	if( !GetAIPtr()->CanUseCover() )
		return LTFALSE;

	if( m_AttackButes.fRunForCoverChance <= 0.0f || m_AttackButes.fMaxRunForCoverDelay <= 0.0f )
		return LTFALSE;

	const CAITarget & target = GetAIPtr()->GetTarget();

	if( m_tmrRunForCoverDelay.On() && m_tmrRunForCoverDelay.Stopped() )
	{
		m_tmrRunForCoverDelay.Stop();

		if( target.IsFacing() )
		{
			if( GetRandom(0.0f,1.0f) < m_AttackButes.fRunForCoverChance )
			{
				CAINodeCover * pCoverNode = g_pAINodeMgr->FindDodgeNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());
				if (pCoverNode && m_StrategyFollowPath.Set(pCoverNode) )
				{
					if (m_pCurCoverNode)
						m_pCurCoverNode->Unlock();
					
					m_pCurCoverNode = pCoverNode;
					m_pCurCoverNode->Lock(GetAIPtr()->GetObject());
					m_tmrCoverDuration.Start( GetRandom( m_AttackButes.fMinCoverDuration, m_AttackButes.fMaxCoverDuration ) );
					m_RootFSM.SetNextState( Root::eStateRunForCover );
					return LTTRUE;
				}
			}
		}
	}

	// This is down here so that the cover delay doesn't start until the dodging is finished.
	if( !m_tmrRunForCoverDelay.On() )
	{
		// Get the dodge delay going again.
		m_tmrRunForCoverDelay.Start( GetRandom(m_AttackButes.fMinRunForCoverDelay, m_AttackButes.fMaxRunForCoverDelay) );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::Dodge
//
//	PURPOSE:	Decides and starts the AI dodging.
//
// ----------------------------------------------------------------------- //

LTBOOL AttackGoal::Dodge()
{
	if( m_AttackButes.fDodgeProbability <= 0.0f || m_AttackButes.fMaxDodgeDelay <= 0.0f )
		return LTFALSE;

	const CAITarget & target = GetAIPtr()->GetTarget();

	if( m_tmrDodgeDelay.On() && m_tmrDodgeDelay.Stopped() )
	{
		m_tmrDodgeDelay.Stop();

		// Only dodge if the target is firing at us.
		if( target.IsAttacking() && target.IsFacing() )
		{
			// Check our chances to dodge.
			if( GetRandom(0.0f,1.0f) < m_AttackButes.fDodgeProbability )
			{
				// Otherwise, just dodge.
				const LTVector & vMyPos = GetAIPtr()->GetPosition();
				const LTVector & vMyRight = GetAIPtr()->GetRight();
				const LTVector & vMyDims = GetAIPtr()->GetDims();
				
				const LTVector & vAttackPoint = target.GetAttackingPoint();


				const bool bDodgeRight = ( vMyRight.Dot( vAttackPoint - vMyPos ) < 0.0f );

				const LTFLOAT fDodgeDist = GetRandom(m_AttackButes.fMinDodgeDist, m_AttackButes.fMaxDodgeDist);
				LTVector vDodgeDest = GetAIPtr()->GetPosition();
				if( bDodgeRight )
				{
					vDodgeDest += vMyRight*fDodgeDist;
				}
				else
				{
					vDodgeDest -= vMyRight*fDodgeDist;
				}

				const CAIVolume * pCurrentVolume = GetAIPtr()->GetCurrentVolumePtr();

				if( pCurrentVolume && pCurrentVolume->ContainsWithNeighbor(vDodgeDest, LTVector(vMyDims.x, 0, vMyDims.z) ) )
				{
					// Check for other AI.
					if( !WillClip( *GetAIPtr(), vMyPos, vDodgeDest ) || WillClip(*GetAIPtr(), vMyPos) )
					{
						// Whew! Now we can dodge!
						AIActionDodge* pDodgeAction = dynamic_cast<AIActionDodge*>(GetAIPtr()->SetNextAction(CAI::ACTION_DODGE));
						if( pDodgeAction )
						{
							pDodgeAction->Init(bDodgeRight, vDodgeDest);
							return LTTRUE;
						}
					}
				}
			}

		} //if( target.IsAttacking() && target.IsFacing() )

	} //if( m_tmrDodgeDelay.On() && m_tmrDodgeDelay.Stopped() )

	// This is down here so that the dodge delay doesn't start until the dodging is finished.
	if( !m_tmrDodgeDelay.On() )
	{
		// Get the dodge delay going again.
		m_tmrDodgeDelay.Start( GetRandom(m_AttackButes.fMinDodgeDelay, m_AttackButes.fMaxDodgeDelay) );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::Backoff
//
//	PURPOSE:	Decides and starts the AI backing off.
//
// ----------------------------------------------------------------------- //

LTBOOL AttackGoal::Backoff()
{
	if( m_AttackButes.fBackoffProbability <= 0.0f || m_AttackButes.fMaxBackoffDelay <= 0.0f )
		return LTFALSE;


	if( m_tmrBackoffDelay.On() && m_tmrBackoffDelay.Stopped() )
	{
		m_tmrBackoffDelay.Stop();

		if( GetRandom(0.0f, 1.0f) < m_AttackButes.fBackoffProbability )
		{
			// Only back-off if the target is too close.
			const CAITarget & target = GetAIPtr()->GetTarget();
			const LTVector vRelativeTargetPos = target.GetPosition() - GetAIPtr()->GetPosition();
			if( InsideWeaponIdealRange(GetAIPtr()->GetIdealWeaponButesPtr(), vRelativeTargetPos) )
			{
				const LTVector & vMyPos = GetAIPtr()->GetPosition();
				const LTVector & vMyForward = GetAIPtr()->GetForward();
				const LTVector & vMyDims = GetAIPtr()->GetDims();
				
				const LTFLOAT  fBackoffDist = GetRandom(m_AttackButes.fMinBackoffDist, m_AttackButes.fMaxBackoffDist);
				const LTVector vBackoffDest = GetAIPtr()->GetPosition() - vMyForward*fBackoffDist;

				const CAIVolume * pCurrentVolume = GetAIPtr()->GetCurrentVolumePtr();

				if( pCurrentVolume && pCurrentVolume->ContainsWithNeighbor(vBackoffDest, LTVector(vMyDims.x, 0, vMyDims.z) ) )
				{
					// Check for other AI.
					if( !WillClip( *GetAIPtr(), vMyPos, vBackoffDest ) || WillClip(*GetAIPtr(), vMyPos) )
					{
						// Whew! Now we can dodge!
						AIActionBackoff* pBackoffAction = dynamic_cast<AIActionBackoff*>(GetAIPtr()->SetNextAction(CAI::ACTION_BACKOFF));
						if( pBackoffAction )
						{
							pBackoffAction->Init(vBackoffDest);
							return LTTRUE;
						}
					}
				}
			}

		} //if( GetRandom(0.0f, 1.0f) < m_AttackButes.fBackoffProbability )

	} //if( m_tmrBackoffDelay.On() && m_tmrBackoffDelay.Stopped() )

	// This is down here so that the dodge delay doesn't start until the dodging is finished.
	if( !m_tmrBackoffDelay.On() )
	{
		// Get the dodge delay going again.
		m_tmrBackoffDelay.Start( GetRandom(m_AttackButes.fMinBackoffDelay, m_AttackButes.fMaxBackoffDelay) );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::Melee
//
//	PURPOSE:	Decides and starts the AI running for cover.
//
// ----------------------------------------------------------------------- //

LTBOOL AttackGoal::Melee()
{
	if( m_AttackButes.strMeleeWeapon.empty() || m_AttackButes.fEnterMeleeDist <= 0.0f )
		return LTFALSE;

	const CAITarget & target = GetAIPtr()->GetTarget();

	// See if the target is inside the enter melee distance.
	if( (target.GetPosition() - GetAIPtr()->GetPosition()).MagSqr() <= m_AttackButes.fEnterMeleeDist*m_AttackButes.fEnterMeleeDist )
	{
		// Get our melee weapon (and immediately set-up our
		GetAIPtr()->SetWeapon( m_AttackButes.strMeleeWeapon.c_str(), true);
		GetAIPtr()->UpdateWeaponAnimation();

		// Switch to a melee attack!
		m_RootFSM.SetNextState( Root::eStateEngageMelee );
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackGoal::Crouch
//
//	PURPOSE:	Decides and starts the AI crouching (does _not_ check if at crouch node!).
//
// ----------------------------------------------------------------------- //

LTBOOL AttackGoal::Crouch()
{
	if (!GetAIPtr()->CanUseCover())
		return LTFALSE;

	if( m_AttackButes.fCrouchChance <= 0.0f || m_AttackButes.fMaxCrouchDelay <= 0.0f )
		return LTFALSE;

	if( m_tmrCrouchDelay.On() && m_tmrCrouchDelay.Stopped() )
	{
		m_tmrCrouchDelay.Stop();

		// Check for cover node validity.
		if( m_pCurCoverNode && !m_pCurCoverNode->IsCoverFromTarget( *GetAIPtr() ) )
		{
			CAINodeCover * pCoverNode = g_pAINodeMgr->FindDodgeNode(GetAIPtr()->GetObject(), GetAIPtr()->GetTarget().GetObject());
			if (pCoverNode && m_StrategyFollowPath.Set(pCoverNode) )
			{
				if (m_pCurCoverNode)
					m_pCurCoverNode->Unlock();
				
				m_pCurCoverNode = pCoverNode;
				m_pCurCoverNode->Lock(GetAIPtr()->GetObject());
				m_tmrCoverDuration.Start( 0.0f );
				m_RootFSM.SetNextState( Root::eStateRunForCover );
				return LTTRUE;
			}

			// If our cover node just doesn't do it, restart the whole
			// process.
			m_RootFSM.SetNextState( Root::eStateApproachTarget );
			return LTTRUE;
		}
		else
		{
			const CAITarget & target = GetAIPtr()->GetTarget();

			// Alright, try to duck.
			if( target.IsFacing() )
			{
				// Check our chances to crouch.
				if( GetRandom(0.0f,1.0f) < m_AttackButes.fCrouchChance )
				{
					// Make sure we're actually at our cover node
					if (m_pCurCoverNode && m_pCurCoverNode->IsOccupied(GetAIPtr()->GetObject()))
					{
						// Duck!
						AIActionCrouch * pCrouchAction = dynamic_cast<AIActionCrouch*>( GetAIPtr()->SetNextAction(CAI::ACTION_CROUCH) );
						if( pCrouchAction )
						{
							// Turn off our shoulder lamp.
							if (GetAIPtr()->IsFlashlightOn())
								GetAIPtr()->SetFlashlight(LTFALSE);
							
							pCrouchAction->Init( m_pCurCoverNode );
							return LTTRUE;
						}
					}
				}
			}
		}
	}

	// This is down here so that the dodge delay doesn't start until the dodging is finished.
	if( !m_tmrCrouchDelay.On() )
	{
		// Get the dodge delay going again.
		m_tmrCrouchDelay.Start( GetRandom(m_AttackButes.fMinCrouchDelay, m_AttackButes.fMaxCrouchDelay) );
	}

	return LTFALSE;

}


#ifdef ATTACK_DEBUG

void AttackGoal::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateWaitForNewTarget : szMessage = "eStateWaitForNewTarget"; break;
		case Root::eStateApproachTarget : szMessage = "eStateApproachTarget"; break;
		case Root::eStateGetLineOfSight : szMessage = "eStateGetLineOfSight"; break;
		case Root::eStateRunForCover : szMessage = "eStateRunForCover"; break;
		case Root::eStateEngageTarget : szMessage = "eStateEngageTarget"; break;
		case Root::eStateFear : szMessage = "eStateFear"; break;
		case Root::eStateEngageMelee : szMessage = "eStateEngageMelee"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Attack::RootFSM is entering state %s.", szMessage );
}

void AttackGoal::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = "Unknown";
	switch( message )
	{
		case Root::eNoTarget : szMessage = "eNoTarget"; break;
		default : szMessage = "Unknown"; break;
	}

	if( fsm.HasTransition( fsm.GetState(), message ) )
	{
		AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Attack::RootFSM is processing %s.", szMessage );
	}
}

#endif

