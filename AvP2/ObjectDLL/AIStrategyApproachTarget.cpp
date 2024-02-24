// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyApproachTarget.cpp
//
// PURPOSE : 
//
// CREATED : 6.20.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIStrategyApproachTarget.h"
#include "AI.h"
#include "AITarget.h"
#include "AIButeMgr.h"

#ifdef _DEBUG
//#define STRATEGY_APPROACH_TARGET_DEBUG
//#include "DebugLineSystem.h"
//#include "AIUtils.h"
//#include "AIVolume.h"
#endif

static LTVector AdjustedTargetPos( const CAITarget & target, const LTVector & vAIDims)
{
	const CCharacter * pChar = target.GetCharacterPtr();
	if( !pChar )
		return target.GetPosition();

	return target.GetPosition() - pChar->GetUp().Dot(pChar->GetDims() - vAIDims);
}

CAIStrategyApproachTarget::CAIStrategyApproachTarget(CAI * pAI)
	: CAIStrategy(pAI),
	  m_StrategyFollowPath(pAI),
	  m_fMinRepathTime(0.0f),
	  m_fMaxRepathTime(0.0f),
	  m_vLastKnownPos(LTVector(0,0,0))
{
	SetupFSM();

	const AIBM_ApproachTarget & butes = g_pAIButeMgr->GetApproachTarget( pAI->GetTemplateId() );
	m_fMinRepathTime = butes.fMinRepath;
	m_fMaxRepathTime = butes.fMaxRepath;

	m_RootFSM.SetState( Root::eStateTargetVisible );
}


void CAIStrategyApproachTarget::Update()
{
	CAIStrategy::Update();

	if (!GetAIPtr() || !GetAIPtr()->HasTarget()) 
	{
		m_RootFSM.AddMessage(Root::eTargetInvalid);
	}

	m_RootFSM.Update();
}

void CAIStrategyApproachTarget::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateTargetVisible,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyApproachTarget::UpdateTargetVisible),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyApproachTarget::StartTargetVisible) );

	m_RootFSM.DefineState( Root::eStateLastTargetPosition,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyApproachTarget::UpdateLastTargetPosition),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyApproachTarget::StartLastTargetPosition) );

	m_RootFSM.DefineState( Root::eStateNextVolume,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyApproachTarget::UpdateNextVolume),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyApproachTarget::StartNextVolume) );

	m_RootFSM.DefineState( Root::eStateTargetLost,  
						   m_RootFSM.MakeCallback() );

	// Default Transitions.
	m_RootFSM.DefineTransition( Root::eStateLastTargetPosition, Root::eCanSeeTarget,      Root::eStateTargetVisible );
	m_RootFSM.DefineTransition( Root::eStateNextVolume,         Root::eCanSeeTarget,      Root::eStateTargetVisible );
	m_RootFSM.DefineTransition( Root::eStateTargetLost,         Root::eCanSeeTarget,      Root::eStateTargetVisible );

	m_RootFSM.DefineTransition( Root::eStateTargetVisible,      Root::eCannotSeeTarget,   Root::eStateLastTargetPosition );

	m_RootFSM.DefineTransition( Root::eStateLastTargetPosition, Root::eAtDesiredLocation, Root::eStateNextVolume );
	m_RootFSM.DefineTransition( Root::eStateNextVolume,	        Root::eAtDesiredLocation, Root::eStateTargetLost );

	m_RootFSM.DefineTransition( Root::eStateLastTargetPosition, Root::eCannotReachDesiredLocation,  Root::eStateTargetLost );
	m_RootFSM.DefineTransition( Root::eStateNextVolume,         Root::eCannotReachDesiredLocation,  Root::eStateTargetLost );
	m_RootFSM.DefineTransition( Root::eStateTargetLost,         Root::eCannotReachDesiredLocation,  Root::eStateTargetLost );

	m_RootFSM.DefineTransition( Root::eTargetInvalid, Root::eStateTargetLost );

#ifdef STRATEGY_APPROACH_TARGET_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStrategyApproachTarget::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStrategyApproachTarget::PrintRootFSMMessage));
#endif

}

void CAIStrategyApproachTarget::StartTargetVisible(RootFSM * fsm)
{
	if(    !GetAIPtr() 
		|| !GetAIPtr()->HasTarget() )
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}

	m_tmrRepath.Stop();
}

void CAIStrategyApproachTarget::UpdateTargetVisible(RootFSM *)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	// Check for a new position if we can see our target.
	if (!target.IsMaintained())
	{
		m_RootFSM.AddMessage(Root::eCannotSeeTarget);
	}

	if( m_tmrRepath.Stopped() || m_StrategyFollowPath.IsDone() || m_StrategyFollowPath.IsUnset() )
	{
		// Just go to target's last volume if they are not currently in a volume.
		m_vLastKnownPos = target.GetPosition();
		if( !m_StrategyFollowPath.Set( m_vLastKnownPos ) )
		{
			m_StrategyFollowPath.Set( target.GetCharacterPtr() );
		}

		m_tmrRepath.Start( GetRandom(m_fMinRepathTime, m_fMaxRepathTime) );
	}

	m_StrategyFollowPath.Update();
}

void CAIStrategyApproachTarget::StartLastTargetPosition(RootFSM *)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	if( !m_StrategyFollowPath.Set( m_vLastKnownPos ) )
	{
		m_RootFSM.AddMessage(Root::eCannotReachDesiredLocation);
//		m_StrategyFollowPath.Set( target.GetCharacterPtr() );
	}
}


void CAIStrategyApproachTarget::UpdateLastTargetPosition(RootFSM *)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	if (target.IsMaintained())
	{
		m_RootFSM.AddMessage(Root::eCanSeeTarget);
	}

	m_StrategyFollowPath.Update();

	if( m_StrategyFollowPath.IsDone() )
	{
		m_RootFSM.AddMessage(Root::eAtDesiredLocation);
		return;
	}
	else if( !m_StrategyFollowPath.IsSet() )
	{
		m_RootFSM.AddMessage(Root::eCannotReachDesiredLocation);
		return;
	}
}

void CAIStrategyApproachTarget::StartNextVolume(RootFSM *)
{
	m_StrategyFollowPath.SetSequintial( GetAIPtr()->GetCurrentVolumePtr(), GetAIPtr()->GetLastVolumePtr() );
}

	
void CAIStrategyApproachTarget::UpdateNextVolume(RootFSM *)
{
	m_StrategyFollowPath.Update();

	if( m_StrategyFollowPath.IsDone() )
	{
		m_RootFSM.AddMessage(Root::eAtDesiredLocation);
		return;
	}
	else if( !m_StrategyFollowPath.IsSet() )
	{
		m_RootFSM.AddMessage(Root::eCannotReachDesiredLocation);
		return;
	}
}


void CAIStrategyApproachTarget::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAIStrategy::Load(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_StrategyFollowPath;

	*hRead >> m_tmrRepath;
	*hRead >> m_fMinRepathTime;
	*hRead >> m_fMaxRepathTime;
	*hRead >> m_vLastKnownPos;
}

void CAIStrategyApproachTarget::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAIStrategy::Save(hWrite);

	*hWrite << m_RootFSM;
	*hWrite << m_StrategyFollowPath;

	*hWrite << m_tmrRepath;
	*hWrite << m_fMinRepathTime;
	*hWrite << m_fMaxRepathTime;
	*hWrite << m_vLastKnownPos;
}


#ifdef STRATEGY_APPROACH_TARGET_DEBUG

void CAIStrategyApproachTarget::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateTargetVisible : szMessage = "eStateTargetVisible"; break;
		case Root::eStateLastTargetPosition : szMessage = "eStateLastTargetPosition"; break;
		case Root::eStateNextVolume : szMessage = "eStateNextVolume"; break;
		case Root::eStateTargetLost : szMessage = "eStateTargetLost"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" ApproachTarget::RootFSM is entering state %s.", szMessage );
}

void CAIStrategyApproachTarget::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{

	const char * szMessage = 0;
	switch( message )
	{
		case Root::eCanSeeTarget : szMessage = "eCanSeeTarget"; break;
		case Root::eCannotSeeTarget  : szMessage = "eCannotSeeTarget "; break;
		case Root::eAtDesiredLocation : szMessage = "eAtDesiredLocation"; break;
		case Root::eCannotReachDesiredLocation : szMessage = "eCannotReachDesiredLocation"; break;
		case Root::eTargetInvalid : szMessage = "eTargetInvalid"; break;
		default : szMessage = "Unknown"; break;
	}

	if( fsm.HasTransition( fsm.GetState(), message ) )
	{
		AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
					" ApproachTarget::RootFSM is processing %s.", szMessage );
	}
}

#endif
