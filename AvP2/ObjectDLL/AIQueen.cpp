// ----------------------------------------------------------------------- //
//
// MODULE  : AIQueen.cpp
//
// PURPOSE : AIQueen object - Implementation
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIQueen.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "SimpleAIAlien.h"
#include "AIAlien.h"
#include "CharacterMgr.h"
#include "AITarget.h"
#include "PlayerObj.h"
#include "AISenseMgr.h"
#include "AINodeMgr.h"
#include "AIActionMisc.h"
#include "AINodeAlarm.h"
#include "SCDefs.h"
#include "SoundButeMgr.h"
#include "ServerSoundMgr.h"

BEGIN_CLASS(AIQueen)

	ADD_STRINGPROP_FLAG(AttributeTemplate, "Queen", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CharacterType, "Queen", PF_HIDDEN)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 

	ADD_STRINGPROP_FLAG_HELP(Weapon, "<default>", PF_HIDDEN, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary or secondary weapon, use this weapon.")

	ADD_BOOLPROP_FLAG_HELP(UseCover, LTTRUE, PF_HIDDEN, "AI will use cover nodes if available." )
	ADD_BOOLPROP_FLAG_HELP(NeverLoseTarget,	LTTRUE,	PF_HIDDEN, "This should always be true for aliens!" )

	PROP_DEFINEGROUP(Goals, PF_GROUP5 | PF_HIDDEN)
	
	ADD_LONGINTPROP_FLAG_HELP(NumSummons, 1, 0, "Number of times the Queen will run to an Alarm node to summon additional AIs")

END_CLASS_DEFAULT_FLAGS(AIQueen, CAI, NULL, NULL, 0)


// ----------------------------------------------------------------------- //
// Static functions
// ----------------------------------------------------------------------- //
static bool WithinWeaponIdealRange(const AIWBM_AIWeapon * pWeaponButes, const LTVector & vRelativePos)
{
	if( !pWeaponButes )
		return true;

	return vRelativePos.MagSqr() < pWeaponButes->fMaxIdealRange*pWeaponButes->fMaxIdealRange;
}

static bool WithinEnemyWeaponFireRange(const AIWBM_AIWeapon * pWeaponButes, const LTVector & vRelativePos)
{
	if( !pWeaponButes )
		return true;

	return vRelativePos.MagSqr() < pWeaponButes->fMaxFireRange*pWeaponButes->fMaxFireRange;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIQueen::AIQueen()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIQueen::AIQueen()
	: m_pCurNode(LTNULL),
	  m_pStrategyApproachTarget(LTNULL),
	  m_pStrategyFollowPath(LTNULL),
	  m_nSummons(0),
	  m_fDamageChunk(0.0f),
	  m_hLastTarget(LTNULL),
	  m_nRandomSoundBute(0)
{	
	// Default the character to the Alien
	SetCharacter(g_pCharacterButeMgr->GetSetFromModelType("Queen"));

	// Set up sense pointers
	m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
	ASSERT( m_pSeeEnemy );
	
	m_pHearEnemyFootstep = dynamic_cast<CAISenseHearEnemyFootstep*>(GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyFootstep));
	ASSERT( m_pHearEnemyFootstep );
	
	m_pHearEnemyWeaponFire = dynamic_cast<CAISenseHearEnemyWeaponFire*>(GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire));
	ASSERT( m_pHearEnemyWeaponFire );
	
	m_pHearWeaponImpact = dynamic_cast<CAISenseHearWeaponImpact *>(GetSenseMgr().GetSensePtr(CAISenseMgr::HearWeaponImpact));
	ASSERT(m_pHearWeaponImpact);

	SetupFSM();
	m_RootFSM.SetState(Root::eStateWaitForNewTarget);
}

AIQueen::~AIQueen()
{
	if (m_pStrategyApproachTarget)
		delete m_pStrategyApproachTarget;
	if (m_pStrategyFollowPath)
		delete m_pStrategyFollowPath;

	HMESSAGEWRITE hMsg;

	// remove health bar
	hMsg = g_pInterface->StartMessage(LTNULL, SCM_SHOW_BOSS_HEALTH);
	g_pInterface->WriteToMessageByte(hMsg, LTFALSE);
	g_pInterface->EndMessage(hMsg);

	// reset it
	hMsg = g_pInterface->StartMessage(LTNULL, SCM_UPDATE_BOSS_HEALTH);
	g_pInterface->WriteToMessageFloat(hMsg, 1.0f);
	g_pInterface->EndMessage(hMsg);
}


LTBOOL AIQueen::IgnoreDamageMsg(const DamageStruct& damage)
{
	if (!g_pLTServer) 
		return LTTRUE;

	if( damage.eType == DT_ACID)
	{
		return LTTRUE;
	}

	return CAI::IgnoreDamageMsg(damage);
}

void AIQueen::ProcessDamageMsg(const DamageStruct& damage)
{
	const LTFLOAT fTotal = m_damage.GetMaxHitPoints() + m_damage.GetMaxArmorPoints();
	const LTFLOAT fCurrent = m_damage.GetHitPoints() + m_damage.GetArmorPoints();
	
	LTFLOAT fRatio;
	if (fTotal > 0.0f)
		fRatio = fCurrent / fTotal;
	else
		fRatio = 0.0f;

	// Give the client info to update the health status bar
	HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_UPDATE_BOSS_HEALTH);
	g_pInterface->WriteToMessageFloat(hMsg, fRatio);
	g_pInterface->EndMessage(hMsg);

	// Run and get help
	if ( m_nSummons && (fRatio < (m_fDamageChunk * m_nSummons)) )
	{
		m_RootFSM.SetNextState(Root::eStateRunToNode);
		--m_nSummons;
	}

	CCharacter::ProcessDamageMsg(damage);
}

LTBOOL AIQueen::ProcessCommand(const char*const* pTokens, int nArgs)
{
	if (nArgs == 1)
	{
		if (0 == _stricmp("SHOWHEALTH",pTokens[0]))
		{
			HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_SHOW_BOSS_HEALTH);
			g_pInterface->WriteToMessageByte(hMsg, LTTRUE);
			g_pInterface->EndMessage(hMsg);
			return LTTRUE;
		}
		else if (0 == stricmp("HIDEHEALTH",pTokens[0]))
		{
			HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_SHOW_BOSS_HEALTH);
			g_pInterface->WriteToMessageByte(hMsg, LTFALSE);
			g_pInterface->EndMessage(hMsg);
			return LTTRUE;
		}
	}
	
	return CAI::ProcessCommand(pTokens,nArgs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIQueen::ReadProp()
//
//	PURPOSE:	Read DEdit Properties
//
// ----------------------------------------------------------------------- //
LTBOOL AIQueen::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData)
		return LTFALSE;

	CAI::ReadProp(pData);

	if ( g_pLTServer->GetPropGeneric("NumSummons", &genProp) == LT_OK)
		m_nSummons = genProp.m_Long;

	if (m_nSummons > 0)
		m_fDamageChunk = 1.0f / (m_nSummons + 1);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIQueen::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void AIQueen::PostPropRead(ObjectCreateStruct *pStruct)
{
	CAI::PostPropRead(pStruct);

	if (!g_pLTServer || !pStruct) return;

	// Read our attributes
	if( GetTemplateId() >= 0 )
	{
		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate(GetTemplateId());
	}
}

void AIQueen::InitialUpdate()
{
	CAI::InitialUpdate();

	Run();

	m_pStrategyApproachTarget = new CAIStrategyApproachTarget(this);
	m_pStrategyFollowPath = new CAIStrategyFollowPath(this);

	SetNeverLoseTarget(LTTRUE);
}

void AIQueen::Update()
{
	CAI::Update();

	if (m_nRandomSoundBute > 0 && m_tmrRandomSound.Stopped())
	{
		if (m_RootFSM.GetState()== Root::eStateEngageTarget)
			PlayExclamationSound("RandomAttack");
	}


	m_RootFSM.Update();
}

CMusicMgr::EMood AIQueen::GetMusicMoodModifier()
{
	switch( m_RootFSM.GetState() )
	{
		case Root::eStateWaitForNewTarget:
			return CMusicMgr::eMoodAmbient;
			break;

		default:
		case Root::eStateApproachTarget:
		case Root::eStateGetLineOfSight:
		case Root::eStateEngageTarget:
		case Root::eStateRunToNode:
		case Root::eStateSummon:
			return CMusicMgr::eMoodHostile;
			break;
	};


	return CMusicMgr::eMoodNone;
}

void AIQueen::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateWaitForNewTarget, 
		                  m_RootFSM.MakeCallback(*this, &AIQueen::UpdateWaitForNewTarget),
						  m_RootFSM.MakeCallback(*this, &AIQueen::StartWaitForNewTarget),
						  m_RootFSM.MakeCallback(*this, &AIQueen::EndWaitForNewTarget));

	m_RootFSM.DefineState(Root::eStateApproachTarget, 
		                  m_RootFSM.MakeCallback(*this, &AIQueen::UpdateApproachTarget),
						  m_RootFSM.MakeCallback(*this, &AIQueen::StartApproachTarget));

	m_RootFSM.DefineState(Root::eStateGetLineOfSight, 
		                  m_RootFSM.MakeCallback(*this, &AIQueen::UpdateGetLineOfSight),
						  m_RootFSM.MakeCallback(*this, &AIQueen::StartGetLineOfSight));

	m_RootFSM.DefineState(Root::eStateEngageTarget, 
		                  m_RootFSM.MakeCallback(*this, &AIQueen::UpdateEngageTarget),
						  m_RootFSM.MakeCallback(*this, &AIQueen::StartEngageTarget));

	m_RootFSM.DefineState(Root::eStateRunToNode,
						  m_RootFSM.MakeCallback(*this, &AIQueen::UpdateRunToNode),
						  m_RootFSM.MakeCallback(*this, &AIQueen::StartRunToNode));

	m_RootFSM.DefineState(Root::eStateSummon,
						  m_RootFSM.MakeCallback(*this, &AIQueen::UpdateSummon),
						  m_RootFSM.MakeCallback(*this, &AIQueen::StartSummon));

	m_RootFSM.SetInitialState( Root::eStateWaitForNewTarget );
}


void AIQueen::StartWaitForNewTarget(RootFSM * rootFSM)
{
	SetMovementStyle("Investigate");
	return;
}

void AIQueen::UpdateWaitForNewTarget(RootFSM * rootFSM)
{
	// Just sit and wait for a target.
	if( HasTarget() )
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}

	if (m_pSeeEnemy->GetStimulation() >= 1.0f)
		Target(m_pSeeEnemy->GetStimulus());
	else if (m_pHearEnemyFootstep->GetStimulation() >= 1.0f)
		Target(m_pHearEnemyFootstep->GetStimulus());
	else if (m_pHearWeaponImpact->GetStimulation() >= 1.0f)
		Target(m_pHearWeaponImpact->GetStimulus());
	else if (m_pHearEnemyWeaponFire->GetStimulation() >= 1.0f)
		Target(m_pHearEnemyWeaponFire->GetStimulus());
		
	return;
}

void AIQueen::EndWaitForNewTarget(RootFSM * rootFSM)
{
	// Battle cry - once per target
	if ((GetTarget().GetObject() == LTNULL) || (GetTarget().GetObject() != m_hLastTarget))
	{
		m_hLastTarget = GetTarget().GetObject();

		const int nButes = PlayExclamationSound("Attack");
		if( nButes >= 0 )
		{
			// Tell the other AI's of our battle cry!
			const SoundButes & sound_bute = g_pSoundButeMgr->GetSoundButes(nButes);
			for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs(); iter != g_pCharacterMgr->EndAIs(); ++iter )
			{
				(*iter)->GetSenseMgr().BattleCry(sound_bute, this);
			}
		}
	}

	// Set-up to play a random attack sound.
	m_nRandomSoundBute = -1;
	CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(GetCharacter()), "RandomAttack" );
	if( !strSoundButes.IsEmpty() )
	{
		m_nRandomSoundBute = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);

		if( m_nRandomSoundBute > 0 )
		{
			const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(m_nRandomSoundBute);
			m_tmrRandomSound.Start(GetRandom(butes.m_fMinDelay, butes.m_fMaxDelay));
		}
	}
}

void AIQueen::StartApproachTarget(RootFSM * rootFSM)
{
	m_pStrategyApproachTarget->Reset();
}

void AIQueen::UpdateApproachTarget(RootFSM * rootFSM)
{
	if( m_pStrategyApproachTarget->LostTarget() && !NeverLoseTarget() )
	{
		// Clear our old target and wait for a new one.
		Target(LTNULL);
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}

	const CAITarget & target = GetTarget();
	const LTVector & vTargetPos = target.GetPosition();
	const LTVector & vMyPos = GetPosition();
	if( WithinEnemyWeaponFireRange(GetIdealWeaponButesPtr(), vTargetPos - vMyPos) )
	{
		// We're within range!  Get ready to rumble!
		rootFSM->SetNextState( Root::eStateGetLineOfSight );
		return;
	}


	// Well, nothing to do but keep charging at the target!
	m_pStrategyApproachTarget->Update();
}

void AIQueen::StartGetLineOfSight(RootFSM * rootFSM)
{
}

void AIQueen::UpdateGetLineOfSight(RootFSM * rootFSM)
{
	const CAITarget & target = GetTarget();

	if( !target.IsValid() )
	{
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}

	if( !WithinWeaponIdealRange(GetIdealWeaponButesPtr(), target.GetPosition() - GetPosition()) )
	{
		rootFSM->SetNextState(Root::eStateApproachTarget);
		return;
	}


	// Just keep approaching until the target is partially visible.
	if( target.IsPartiallyVisible() )
	{
		rootFSM->SetNextState(Root::eStateEngageTarget);
		return;
	}

	m_pStrategyApproachTarget->Update();
}

void AIQueen::StartEngageTarget(RootFSM * rootFSM)
{
}

void AIQueen::UpdateEngageTarget(RootFSM * rootFSM)
{
	const CAITarget & target = GetTarget();

	if( !WithinEnemyWeaponFireRange(GetIdealWeaponButesPtr(), target.GetPosition() - GetPosition()) )
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
	if( BurstDelayExpired() && CanDoNewAction() )
	{
		SetNextAction(CAI::ACTION_FIREMELEE);
	}

	return;
}

void AIQueen::StartRunToNode(RootFSM * rootFSM)
{
	// find a good node, lock it, and go to it
	CAINodeAlarm *pAlarmNode = g_pAINodeMgr->FindAlarmNode(m_hObject);
	if (pAlarmNode)
	{
		if (m_pCurNode)
			m_pCurNode->Unlock();
		
		pAlarmNode->Lock(m_hObject);
		m_pCurNode = pAlarmNode;
		
		if (!m_pStrategyFollowPath->IsSet())
		{
			Run();
			m_pStrategyFollowPath->Set(pAlarmNode, LTTRUE);
		}
	}
	else
	{
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}

	return;
}

void AIQueen::UpdateRunToNode(RootFSM * rootFSM)
{
	if (!m_pCurNode)
	{
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}

	if (m_pStrategyFollowPath->IsDone())
	{
		// Summon our friends
		rootFSM->SetNextState(Root::eStateSummon);
		return;
	}

	m_pStrategyFollowPath->Update();

	return;
}

void AIQueen::StartSummon(RootFSM * rootFSM)
{
	GetAnimation()->PlayAnim("Scream");
	GetAnimation()->Update();

	return;
}

void AIQueen::UpdateSummon(RootFSM * rootFSM)
{
	if (!m_pCurNode)
	{
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}

	if (!GetAnimation()->UsingAltAnim())
	{
		if (m_pCurNode->GetAlarmObject() != LTNULL)
		{
			LTVector vDir;
			vDir = GetTarget().GetPosition() - GetPosition();

			AIActionTurnTo * pNextAction = dynamic_cast<AIActionTurnTo *>(SetNextAction(CAI::ACTION_TURNTO));
			if (pNextAction)
				pNextAction->Init(vDir);

			SendTriggerMsgToObject(this, m_pCurNode->GetTriggerObject(), m_pCurNode->GetTriggerMsg().c_str());
//			SendTriggerMsgToObject(this, m_pCurNode->GetAlarmObject(), m_pCurNode->GetAlarmMsg());
		}

		m_pCurNode->Unlock();
		rootFSM->SetNextState(Root::eStateWaitForNewTarget);
		return;
	}
}


Goal * AIQueen::CreateGoal(const char * szGoalName)
{
	return LTNULL;
}


void AIQueen::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	CAI::Save(hWrite,dwSaveFlags);

	*hWrite << m_RootFSM;

	if (m_pStrategyApproachTarget)
		*hWrite << *m_pStrategyApproachTarget;
	if (m_pStrategyFollowPath)
		*hWrite << *m_pStrategyFollowPath;

	hWrite->WriteDWord(m_pCurNode ? m_pCurNode->GetID() : CAINodeAlarm::kInvalidID );
	
	*hWrite << m_nSummons;
	*hWrite << m_fDamageChunk;
	*hWrite << m_hLastTarget;
	*hWrite << m_nRandomSoundBute;
	*hWrite << m_tmrRandomSound;
}


void AIQueen::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	CAI::Load(hRead,dwLoadFlags);

	*hRead >> m_RootFSM;

	delete m_pStrategyApproachTarget;
	m_pStrategyApproachTarget = new CAIStrategyApproachTarget(this);
	*hRead >> *m_pStrategyApproachTarget;

	delete m_pStrategyFollowPath;
	m_pStrategyFollowPath = new CAIStrategyFollowPath(this);
	*hRead >> *m_pStrategyFollowPath;

	const uint32 cur_node_id = hRead->ReadDWord();
	if (cur_node_id != CAINodeAlarm::kInvalidID )
		m_pCurNode = (CAINodeAlarm *)g_pAINodeMgr->GetNode(cur_node_id);

	*hRead >> m_nSummons;
	*hRead >> m_fDamageChunk;
	*hRead >> m_hLastTarget;
	*hRead >> m_nRandomSoundBute;
	*hRead >> m_tmrRandomSound;
}
