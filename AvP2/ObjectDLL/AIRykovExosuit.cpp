// ----------------------------------------------------------------------- //
//
// MODULE  : AIRykovExosuit.cpp
//
// PURPOSE : AIRykovExosuit object - Implementation
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AIRykovExosuit.h"
#include "AIButeMgr.h"
#include "ObjectMsgs.h"
#include "CharacterMgr.h"
#include "AITarget.h"
#include "PlayerObj.h"
#include "AISenseMgr.h"
#include "AINodeMgr.h"
#include "AIActionMisc.h"
#include "SCDefs.h"
#include "AIAttack.h"


#define ADD_GOAL_DEFAULTS(number, group, type, priority) \
		ADD_STRINGPROP_FLAG_HELP(GoalType_##number##, type, (group) | PF_DYNAMICLIST, "Type of goal.") \
		ADD_OBJECTPROP_FLAG_HELP(Link_##number##, "", group, "Link to an object used by the goal (can be null).") \
		ADD_STRINGPROP_FLAG_HELP(Parameters_##number##, "", group, "Other parameters for goal (ie. TARGET=Player).")

BEGIN_CLASS(AIRykovExosuit)

	ADD_STRINGPROP_FLAG(AttributeTemplate, "RykovExosuit", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CharacterType, "RykovExosuit", PF_HIDDEN)
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 255.0f, 0.0f, 0, "Color used for object in DEdit.") 

	ADD_STRINGPROP_FLAG_HELP(Weapon, "<default>", PF_HIDDEN, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, "<default>", PF_HIDDEN, "If AI cannot use primary or secondary weapon, use this weapon.")

	ADD_BOOLPROP_FLAG_HELP(UseCover, LTTRUE, PF_HIDDEN, "AI will use cover nodes if available." )
	ADD_BOOLPROP_FLAG_HELP(NeverLoseTarget,	LTFALSE, PF_HIDDEN, "This should always be true for aliens!" )

	ADD_STRINGPROP_FLAG_HELP(OnHealth50Target, LTFALSE, 0, "Target to send OnHealth50Message to")
	ADD_STRINGPROP_FLAG_HELP(OnHealth50Message, LTFALSE, 0, "Message to send when health is 50%")

//	PROP_DEFINEGROUP(Goals, PF_GROUP5 | PF_HIDDEN)
//		ADD_GOAL_DEFAULTS(1, PF_GROUP5, "<none>", 1000.0f )
//		ADD_GOAL_DEFAULTS(2, PF_GROUP5, "<none>", 150.0f )
	
END_CLASS_DEFAULT_FLAGS(AIRykovExosuit, AIExosuit, NULL, NULL, 0)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRykovExosuit::AIRykovExosuit()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

AIRykovExosuit::AIRykovExosuit()
	: m_pStrategyFollowPath(LTNULL),
	  m_pCurNode(LTNULL),
	  m_hAINodeGroup(LTNULL),
	  m_bRockets(LTFALSE),
	  m_bHealthMsgSent(LTFALSE)
{
	m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
	ASSERT( m_pSeeEnemy );

	m_pHearEnemyFootstep = dynamic_cast<CAISenseHearEnemyFootstep *>(GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyFootstep));
	ASSERT(m_pHearEnemyFootstep);

	m_pHearEnemyWeaponFire = dynamic_cast<CAISenseHearEnemyWeaponFire *>(GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire));
	ASSERT(m_pHearEnemyWeaponFire);

	m_pHearWeaponImpact = dynamic_cast<CAISenseHearWeaponImpact *>(GetSenseMgr().GetSensePtr(CAISenseMgr::HearWeaponImpact));
	ASSERT(m_pHearEnemyWeaponFire);


	SetupFSM();
}

AIRykovExosuit::~AIRykovExosuit()
{
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRykovExosuit::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD AIRykovExosuit::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			// AIExosuit will call our InitialUpdate function!
			DDWORD dwRet = AIExosuit::EngineMessageFn(messageID, pData, fData);

			if (!g_pLTServer) return dwRet;

			// This needs to always be done after AI's initil update (to get strategy move).
			delete m_pStrategyFollowPath;
			m_pStrategyFollowPath = new CAIStrategyFollowPath(this);

			return dwRet;
		}
		break;
	}

	return AIExosuit::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRykovExosuit::ReadProp()
//
//	PURPOSE:	Read DEdit properties
//
// ----------------------------------------------------------------------- //

LTBOOL AIRykovExosuit::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
	if (!g_pLTServer || !pStruct) 
		return LTFALSE;

	AIExosuit::ReadProp(pStruct);

	if ( g_pLTServer->GetPropGeneric( "OnHealth50Target", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_sHealthMsgTarget = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "OnHealth50Message", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_sHealthMsg = genProp.m_String;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIRykovExosuit::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void AIRykovExosuit::PostPropRead(ObjectCreateStruct *pStruct)
{
	AIExosuit::PostPropRead(pStruct);

	if (!g_pLTServer || !pStruct) return;

	// Read our attributes
	if( GetTemplateId() >= 0 )
	{
		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate(GetTemplateId());
	}
}

void AIRykovExosuit::InitialUpdate()
{
	AIExosuit::InitialUpdate();

	AddWindshield();

	Run();
	SetNeverLoseTarget(LTFALSE);
}

void AIRykovExosuit::FirstUpdate()
{
	AIExosuit::FirstUpdate();
}

void AIRykovExosuit::ProcessDamageMsg(const DamageStruct& damage)
{
	const LTFLOAT fTotal = m_damage.GetMaxHitPoints() + m_damage.GetMaxArmorPoints();
	const LTFLOAT fCurrent = m_damage.GetHitPoints() + m_damage.GetArmorPoints();
	
	LTFLOAT fRatio;

	if (fTotal > 0.0f)
		fRatio = fCurrent / fTotal;
	else
		fRatio = 0.0f;

	// Send a trigger command (filled in by DEdit) at 50% health.
	if (!m_bHealthMsgSent)
	{
		if ((fRatio <= 0.5f) && (!m_sHealthMsg.empty()))
		{
			SendTriggerMsgToObjects(this, m_sHealthMsgTarget.c_str(), m_sHealthMsg.c_str());
			m_bHealthMsgSent = LTTRUE;
		}
	}

	// TODO: this is a placeholder until actual weapon usage is defined
	if (!m_bRockets && (fRatio < 0.75f))
	{
		SetWeapon("Exosuit_Rocket", true);
		m_bRockets = LTTRUE;
	}

	if (m_bRockets && (fRatio < 0.50f))
	{
		SetWeapon("Exosuit_Minigun", true);
		m_bRockets = LTFALSE;
	}


	// Give the client info to update the health status bar
	HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_UPDATE_BOSS_HEALTH);
	g_pInterface->WriteToMessageFloat(hMsg, fRatio);
	g_pInterface->EndMessage(hMsg);

	CCharacter::ProcessDamageMsg(damage);

	return;
}

LTBOOL AIRykovExosuit::ProcessCommand(const char*const* pTokens, int nArgs)
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
	
	return AIExosuit::ProcessCommand(pTokens, nArgs);
}


void AIRykovExosuit::Update()
{
	AIExosuit::Update();

//	This is commented out to disable the snipe-ish state machine.  The boss fight
//	has changed spec, so for now Rykov will use a standard Attack goal.
//	m_RootFSM.Update();
}

void AIRykovExosuit::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateFindNode,  
						  m_RootFSM.MakeCallback(*this, &AIRykovExosuit::UpdateFindNode),
						  m_RootFSM.MakeCallback(*this, &AIRykovExosuit::StartFindNode));

	m_RootFSM.DefineState(Root::eStateFindTarget,  
						  m_RootFSM.MakeCallback(*this, &AIRykovExosuit::UpdateFindTarget),
						  m_RootFSM.MakeCallback(*this, &AIRykovExosuit::StartFindTarget));

	m_RootFSM.DefineState(Root::eStateAttack,  
						  m_RootFSM.MakeCallback(*this, &AIRykovExosuit::UpdateAttack),
						  m_RootFSM.MakeCallback(*this, &AIRykovExosuit::StartAttack));

	m_RootFSM.SetState(Root::eStateFindNode);
}

void AIRykovExosuit::StartFindNode(RootFSM * fsm)
{
	// Find a good node, lock it, and go to it.
	// If we don't currently have a node, just find the one closest
	// to us.  Otherwise find the one closest to our target.
	HOBJECT hTarget = LTNULL;
	if( m_pCurNode )
		hTarget = GetTarget().GetObject();
	
	CAINodeSnipe *pSnipeNode = g_pAINodeMgr->FindSnipeNode(m_hObject, hTarget, m_hAINodeGroup);
	if (pSnipeNode && m_pStrategyFollowPath->Set(pSnipeNode, (hTarget == LTNULL) ) )
	{
		if (m_pCurNode)
			m_pCurNode->Unlock();
		
		pSnipeNode->Lock(m_hObject);
		m_pCurNode = pSnipeNode;
	}

	return;
}

void AIRykovExosuit::UpdateFindNode(RootFSM * fsm)
{
	ASSERT(m_pCurNode);
	if (!m_pCurNode)
	{
		fsm->SetNextState(Root::eStateFindTarget);
		return;
	}

	if (m_pStrategyFollowPath->IsDone() || m_pStrategyFollowPath->IsUnset() )
	{
		m_tmrHoldPos.Start(g_pAIButeMgr->GetTemplate(GetTemplateId()).fSnipeTime);
		fsm->SetNextState(Root::eStateFindTarget);
		return;
	}

	m_pStrategyFollowPath->Update();

	return;
}
	
void AIRykovExosuit::StartFindTarget(RootFSM * fsm)
{
}

void AIRykovExosuit::UpdateFindTarget(RootFSM * fsm)
{
	CAISense *pSense = LTNULL;

	if (HasTarget())
	{
		fsm->SetNextState(Root::eStateAttack);
		return;
	}

	// Use all active senses to locate a target (least to most important)
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

	if (pSense)
	{
		Target(pSense->GetStimulus(), pSense->GetStimulusPos());
		fsm->SetNextState(Root::eStateAttack);
		return;
	}

	if (CanDoNewAction())
	{
		if (m_pCurNode && m_pCurNode->GetForward().Dot(GetForward()) < 0.9f)
		{
			AIActionTurnTo * pTurnAction = dynamic_cast<AIActionTurnTo*>(SetNextAction(CAI::ACTION_TURNTO));
			if (pTurnAction)
				pTurnAction->Init(m_pCurNode->GetForward());
		}
	}
}

void AIRykovExosuit::StartAttack(RootFSM * fsm)
{
	return;
}

void AIRykovExosuit::UpdateAttack(RootFSM * fsm)
{
	const CAITarget & target = GetTarget();

	if (target.IsPartiallyVisible())
	{
		const AIWBM_AIWeapon * pWeaponProfile = GetCurrentWeaponButesPtr();
		if (pWeaponProfile)
		{
			const LTFLOAT fRangeSqr = (target.GetPosition() - GetPosition()).MagSqr();
	
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
		m_RootFSM.SetNextState(Root::eStateFindNode);
		return;
	}

	// Invalidate the current node if fired upon
	if (target.IsFacing() && target.IsAttacking() && m_pCurNode)
	{
		// Don't move out of the snipe point too often
		if (g_pAINodeMgr->GetNumSnipeNodes() > 1)
		{
			if (m_tmrHoldPos.Stopped())
			{
				m_RootFSM.SetNextState(Root::eStateFindNode);
				return;
			}
		}
	}

	if (CanDoNewAction() && !GetCurrentAction())
	{
		const CWeapon * pWeapon = GetCurrentWeaponPtr();
		const AIWBM_AIWeapon * pWeaponProfile = GetCurrentWeaponButesPtr();
		if (pWeapon && pWeaponProfile && NeedReload(*pWeapon,*pWeaponProfile))
		{
			SetNextAction(CAI::ACTION_RELOAD);
			return;
		}
		else
		{
			SetNextAction(CAI::ACTION_AIM);
		}
	}

	return;
}


void AIRykovExosuit::FireOnTarget()
{
	if (HasTarget() && BurstDelayExpired())
	{
		SetNextAction(CAI::ACTION_FIREWEAPON);
		return;
	}
}


Goal * AIRykovExosuit::CreateGoal(const char * szGoalName)
{

	ASSERT( szGoalName );

	Goal * pGoal = LTNULL;

	if( stricmp(szGoalName, "ATTACK") == 0 )
	{
		pGoal = new AttackGoal(this, szGoalName);
	}
	else
	{
		pGoal = CAI::CreateGoal(szGoalName);
	}

	return pGoal;

//	return LTNULL;
}


void AIRykovExosuit::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pServerDE || !hWrite) return;
	
	AIExosuit::Save(hWrite,dwSaveFlags);

	*hWrite << m_RootFSM;
	*hWrite << m_tmrHoldPos;

	hWrite->WriteDWord( m_pCurNode ? m_pCurNode->GetID() : CAINode::kInvalidID );

	*hWrite << m_hAINodeGroup;
	*hWrite << m_bRockets;
	*hWrite << m_sHealthMsgTarget;
	*hWrite << m_sHealthMsg;
	*hWrite << m_bHealthMsgSent;
}


void AIRykovExosuit::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pServerDE || !hRead) return;

	AIExosuit::Load(hRead,dwLoadFlags);

	*hRead >> m_RootFSM;
	*hRead >> m_tmrHoldPos;

	const uint32 snipe_node_id = hRead->ReadDWord();
	m_pCurNode = dynamic_cast<CAINodeSnipe*>(g_pAINodeMgr->GetNode( snipe_node_id ));

	*hRead >> m_hAINodeGroup;
	*hRead >> m_bRockets;
	*hRead >> m_sHealthMsgTarget;
	*hRead >> m_sHealthMsg;
	*hRead >> m_bHealthMsgSent;
}
