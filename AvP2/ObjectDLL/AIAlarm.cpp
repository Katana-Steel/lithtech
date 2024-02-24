// ----------------------------------------------------------------------- //
//
// MODULE  : AIAlarm.cpp
//
// PURPOSE : Implements the Alarm goal and state.
//
// CREATED : 12/06/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AIAlarm.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "AIVolume.h"
#include "AINodeMgr.h"
#include "GameServerShell.h"

#include "AIActionMisc.h"


/////////////////////////////////////////////////
//
//     Alarm State
//
/////////////////////////////////////////////////

 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAlarm::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

CAIStateAlarm::CAIStateAlarm(CAI * pAI)
	: CAIState(pAI),
	  m_StrategySetFacing(pAI,true),
	  m_StrategyFollowPath(pAI),
  	  m_pCurNode(NULL)
{
	_ASSERT(pAI);

	SetupFSM();
}

bool CAIStateAlarm::HandleParameter(const char * const * pszArgs, int nArgs)
{
	m_hAINodeGroup = LTNULL;

	if (0 == stricmp(*pszArgs, "NG"))
	{
		++pszArgs;
		--nArgs;

		if (nArgs == 1)
		{
			// Find the AINodeGroup (if any)
			if( *pszArgs  && **pszArgs )
			{
				ObjArray<HOBJECT,1> objArray;
				if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>(*pszArgs), objArray ) )
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

void CAIStateAlarm::Start()
{
	CAIState::Start();

	GetAIPtr()->SetMovementStyle();

	m_StrategyFollowPath.Clear();
	m_RootFSM.SetState( Root::eStateFind );
}

void CAIStateAlarm::Update()
{
	CAIState::Update();

	if(!GetAIPtr())
	{
#ifdef _DEBUG
		g_pInterface->CPrint("WARNING: pAI invalid");
#endif
		return;
	}

	m_RootFSM.Update();
}

void CAIStateAlarm::End()
{
	if (m_pCurNode)
	{
		m_pCurNode->Unlock();
		m_pCurNode = LTNULL;
	}

	CAIState::End();
}

void CAIStateAlarm::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateFind,
						   m_RootFSM.MakeCallback(*this, &CAIStateAlarm::UpdateFind),
						   m_RootFSM.MakeCallback(*this, &CAIStateAlarm::StartFind));

	m_RootFSM.DefineState( Root::eStateSwitch,
						   m_RootFSM.MakeCallback(*this, &CAIStateAlarm::UpdateSwitch),
						   m_RootFSM.MakeCallback(*this, &CAIStateAlarm::StartSwitch));

	m_RootFSM.DefineState( Root::eStateDone, 
						   m_RootFSM.MakeCallback(*this, &CAIStateAlarm::UpdateDone));

	// Default Transitions.
	m_RootFSM.DefineTransition( Root::eStimulus, Root::eStateFind );
	m_RootFSM.DefineTransition( Root::eAtNode, Root::eStateSwitch );
	m_RootFSM.DefineTransition( Root::eDone, Root::eStateDone);

#ifdef STATE_ALARM_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStateAlarm::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStateAlarm::PrintRootFSMMessage));
#endif

}


void CAIStateAlarm::StartFind(RootFSM * fsm)
{
	if (!GetAIPtr())
		return;

	CAINodeAlarm *pAlarmNode = g_pAINodeMgr->FindAlarmNode(GetAIPtr()->GetObject(), m_hAINodeGroup);
	if (pAlarmNode && m_StrategyFollowPath.Set(pAlarmNode, LTTRUE) )
	{
		if (m_pCurNode)
			m_pCurNode->Unlock();
		
		pAlarmNode->Lock(GetAIPtr()->GetObject());
		m_pCurNode = pAlarmNode;
		
		GetAIPtr()->Run();
		m_StrategySetFacing.Face(pAlarmNode->GetPos());
	}
	else
	{
		fsm->AddMessage(Root::eDone);
		return;
	}

	return;
}

void CAIStateAlarm::UpdateFind(RootFSM * fsm)
{
	if (!m_pCurNode || m_StrategyFollowPath.IsUnset() )
	{
		m_RootFSM.AddMessage(Root::eDone);
		return;
	}

	if (m_StrategyFollowPath.IsDone())
	{
		m_RootFSM.AddMessage(Root::eAtNode);
		return;
	}

	m_StrategyFollowPath.Update();

	return;
}

void CAIStateAlarm::StartSwitch(RootFSM * fsm)
{
	if (!GetAIPtr())
		return;

	GetAIPtr()->GetAnimation()->PlayAnim("Push_Button");
	GetAIPtr()->GetAnimation()->Update();

	return;
}

void CAIStateAlarm::UpdateSwitch(RootFSM * fsm)
{
	if (!GetAIPtr())
		return;

	// get oriented with the node's direction
	GetAIPtr()->Turn(m_pCurNode->GetForward());

	if (!GetAIPtr()->GetAnimation()->UsingAltAnim())
	{
		if( m_pCurNode->GetAlarmObject() != LTNULL )
		{
			LTVector vDir;
			vDir = GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition();

			AIActionTurnTo * pNextAction = dynamic_cast<AIActionTurnTo *>(GetAIPtr()->SetNextAction(CAI::ACTION_TURNTO));
			if (pNextAction)
				pNextAction->Init(vDir);

//			m_StrategySetFacing.FaceTarget();
			SendTriggerMsgToObject(GetAIPtr(), m_pCurNode->GetAlarmObject(), m_pCurNode->GetAlarmMsg());
		}

		fsm->AddMessage(Root::eDone);
		return;
	}
}

void CAIStateAlarm::UpdateDone(RootFSM * fsm)
{
	m_StrategySetFacing.Update();

	return;
}

LTBOOL CAIStateAlarm::IsDone()
{
	return (m_RootFSM.GetState() == Root::eStateDone);
}


//------------------------------------------------------------------
// Debugging stuff
//------------------------------------------------------------------
#ifdef STATE_ALARM_DEBUG

void CAIStateAlarm::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateFind : szMessage = "eStateFind"; break;
		case Root::eStateSwitch : szMessage = "eStateSwitch"; break;
		case Root::eStateDone : szMessage = "eStateDone"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Alarm::RootFSM is entering state %s.", szMessage );
}

void CAIStateAlarm::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{
	const char * szMessage = 0;
	switch( message )
	{
		case Root::eStimulus : szMessage = "eStimulus"; break;
		case Root::eAtNode : szMessage = "eAtNode"; break;
		case Root::eDone : szMessage = "Done"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" Alarm::RootFSM is processing %s.", szMessage );
}

#else

void CAIStateAlarm::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
}

void CAIStateAlarm::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{
}

#endif
//------------------------------------------------------------------

void CAIStateAlarm::Load(HMESSAGEREAD hRead)
{
	uint32 dwID;

	_ASSERT(hRead);

	CAIState::Load(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_StrategySetFacing;
	*hRead >> m_StrategyFollowPath;
	*hRead >> dwID;
	m_pCurNode = (CAINodeAlarm *)g_pAINodeMgr->GetNode(dwID);
	*hRead >> m_hAINodeGroup;
}

void CAIStateAlarm::Save(HMESSAGEREAD hWrite)
{
	uint32 dwID;

	_ASSERT(hWrite);

	CAIState::Save(hWrite);

	*hWrite << m_RootFSM;
	*hWrite << m_StrategySetFacing;
	*hWrite << m_StrategyFollowPath;
	dwID = m_pCurNode ? m_pCurNode->GetID() : CAINode::kInvalidID;
	*hWrite << dwID;
	*hWrite << m_hAINodeGroup;
}
		
/////////////////////////////////////////////////
//
//     Alarm Goal
//
/////////////////////////////////////////////////


AlarmGoal::AlarmGoal( CAI * pAI, std::string create_name )
	: AICombatGoal(pAI, create_name ),
	  m_pSeeDeadBody(NULL),
	  alarmState(pAI)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AlarmGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool AlarmGoal::HandleParameter(const char * const * pszArgs, int nArgs)
{
	return alarmState.HandleParameter(pszArgs,nArgs);
}

void AlarmGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	AICombatGoal::Load(hRead);

	*hRead >> alarmState;
}


void AlarmGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	AICombatGoal::Save(hWrite);

	*hWrite << alarmState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAlarm::GetBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT AlarmGoal::GetBid()
{
	const LTFLOAT fDefault = g_pAIButeMgr->GetDefaultBid(BID_ALARM);
	LTFLOAT fBid = 0.0f;

	CAINodeAlarm *pAlarmNode = LTNULL;
	
	// If the goal is not yet active
	if (!IsActive())
	{
		// ... and the alarm is not on
		if (!g_pGameServerShell->GetAlarm())
		{
			if( GetAIPtr()->HasTarget() || FindNewTarget().hTarget )
			{
				fBid = fDefault;
			}
		}
	}
	else
	{
		// Goal is active but the Alarm hasn't been triggered yet
		if (!g_pGameServerShell->GetAlarm())
			fBid = fDefault;
	}

	// activate only if an alarm node is available, and we haven't locked one already
	if (fBid > 0.0f)
	{
		if (GetAIPtr())
		{
			if (!alarmState.HasNode())
			{
				pAlarmNode = g_pAINodeMgr->FindAlarmNode(GetAIPtr()->GetObject(), alarmState.GetNodeGroup());
				if (!pAlarmNode)
					fBid = 0.0f;
			}
		}
	}

	return fBid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateAlarm::GetMaximumBid
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
LTFLOAT AlarmGoal::GetMaximumBid()
{
	return g_pAIButeMgr->GetDefaultBid(BID_ALARM);
}
void AlarmGoal::Start()
{
	AICombatGoal::Start();

	alarmState.Start();
}

void AlarmGoal::End()
{
	AICombatGoal::End();

	alarmState.End();
}
