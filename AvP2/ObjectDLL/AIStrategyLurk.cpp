// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyLurk.cpp
//
// PURPOSE : 
//
// CREATED : 7.10.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIStrategyLurk.h"
#include "AI.h"
#include "AITarget.h"
#include "AIVolume.h"
#include "AIButeMgr.h"

#ifdef _DEBUG
#define STRATEGY_LURK_DEBUG
#endif

#ifdef STRATEGY_LURK_DEBUG
#include "DebugLineSystem.h"
#include "AIUtils.h"
#endif

CAIStrategyLurk::CAIStrategyLurk()
{
	SetupFSM();
}

void CAIStrategyLurk::Init(CAI * pAI)
{
	ASSERT( pAI );
	if( !pAI ) return;

	CAIStrategy::Init(pAI);
	m_StrategyFollowPath.Init(pAI);
	m_StrategyApproachTarget.Init(pAI);

	m_RootFSM.SetState( Root::eStateSearch );
}


void CAIStrategyLurk::Update()
{
	CAIStrategy::Update();

	if(!GetAIPtr())
	{
		g_pInterface->CPrint("WARNING: pAI invalid");
		return;
	}

	m_RootFSM.Update();

}

void CAIStrategyLurk::SetupFSM()
{
	m_RootFSM.DefineState( Root::eStateSearch,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::UpdateSearch),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::StartSearch) );

	m_RootFSM.DefineState( Root::eStateAdvance,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::UpdateAdvance),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::StartAdvance) );

	m_RootFSM.DefineState( Root::eStateRetreat,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::UpdateRetreat),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::StartRetreat) );

	m_RootFSM.DefineState( Root::eStateAttack,  
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::UpdateAttack),
						   m_RootFSM.MakeCallback(*this, &CAIStrategyLurk::StartAttack) );

	// Default Transitions.
	m_RootFSM.DefineTransition( Root::eCannotSeeTarget, Root::eStateSearch );
	m_RootFSM.DefineTransition( Root::eTargetInvalid, Root::eStateSearch );

	// Specific Transitions
	m_RootFSM.DefineTransition( Root::eStateSearch, Root::eCanSeeTarget, Root::eStateAdvance );

	m_RootFSM.DefineTransition( Root::eStateAdvance, Root::eTargetFacing, Root::eStateRetreat );
	m_RootFSM.DefineTransition( Root::eStateAdvance, Root::eInRange, Root::eStateAttack );

	m_RootFSM.DefineTransition( Root::eStateRetreat, Root::eTargetNotFacing, Root::eStateAdvance );

	m_RootFSM.DefineTransition( Root::eStateAttack, Root::eOutOfRange, Root::eStateAdvance );



#ifdef STRATEGY_LURK_DEBUG
	m_RootFSM.SetChangingStateCallback(m_RootFSM.MakeChangingStateCallback(*this,&CAIStrategyLurk::PrintRootFSMState));
	m_RootFSM.SetProcessingMessageCallback(m_RootFSM.MakeProcessingMessageCallback(*this,&CAIStrategyLurk::PrintRootFSMMessage));
#endif

}

void CAIStrategyLurk::StartSearch(RootFSM * fsm)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget() )
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}
}

void CAIStrategyLurk::UpdateSearch(RootFSM *)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	if (target.IsPartiallyVisible() == true)
		m_RootFSM.AddMessage(Root::eCanSeeTarget);

	// TODO: Add code to move around (search)
}

void CAIStrategyLurk::StartAdvance(RootFSM * fsm)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget())
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}
}

void CAIStrategyLurk::UpdateAdvance(RootFSM *)
{
	DVector vToAI;
	DVector vEnemyForward;
	LTFLOAT fDot;
	
	const CAITarget & target = GetAIPtr()->GetTarget();

	// Is the target still visible?
	if (target.IsPartiallyVisible() != true)
	{
		m_RootFSM.AddMessage(Root::eCannotSeeTarget);
		return;
	}

	// Is the target facing me?
	vEnemyForward = target.GetCharacterPtr()->GetForward();

	vToAI = GetAIPtr()->GetPosition() - target.GetPosition(); 
	vToAI.Norm();

	// 07/10/00 - MarkS: 
	// TODO: Change this to use the field of view, instead of .5
	fDot = vToAI.Dot(vEnemyForward);
	if (fDot > 0.5f)
		m_RootFSM.AddMessage(Root::eTargetFacing);

	
	// Is the target close enough to attack?
	const AIWBM_Template * pWeaponProfile = GetAIPtr()->GetCurrentWeaponButesPtr();
	if( pWeaponProfile )
	{
		const LTFLOAT target_range_sqr 
			= ( GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition() ).MagSqr();

		// Advance if out of range
		if(target_range_sqr > pWeaponProfile->MaxIdealRange*pWeaponProfile->MaxIdealRange)
			ApproachTarget();
		else
			m_RootFSM.AddMessage(Root::eInRange);
	}
}

void CAIStrategyLurk::StartRetreat(RootFSM * fsm)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget())
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}
}

void CAIStrategyLurk::UpdateRetreat(RootFSM *)
{
	DVector vToAI;

	const CAITarget & target = GetAIPtr()->GetTarget();

	// Is the target still visible?
	if (target.IsPartiallyVisible() != true)
	{
		m_RootFSM.AddMessage(Root::eCannotSeeTarget);
		return;
	}

	// Is the target facing me?
	vToAI = GetAIPtr()->GetPosition() - target.GetPosition() ;
	vToAI.Norm();

	// 07/10/00 - MarkS: 
	// TODO: Change this to use the field of view, instead of .5
	if (vToAI.Dot(target.GetCharacterPtr()->GetForward()) <= 0.5f)
		m_RootFSM.AddMessage(Root::eTargetNotFacing);

	DVector vIntersect, vTemp;
	LTFLOAT fMagSqr, fTemp;
	CAIVolume *pCurVol = NULL; 
	const CAIVolume *pDestVol = NULL;

	pCurVol = GetAIPtr()->GetCurrentVolumePtr();
	const CAIVolume::NeighborList &lstNeighbors = pCurVol->GetNeighborList();

	pDestVol = *GetAIPtr()->GetCurrentVolumePtr()->BeginNeighbors();

	// We have volume issues
	if (!pCurVol || !pDestVol)
		m_RootFSM.AddMessage(Root::eCannotSeeTarget);
	
	for(CAIVolume::NeighborInfoIterator iter = pCurVol->BeginNeighborInfo(); iter != pCurVol->EndNeighborInfo(); ++iter )
	{
		vIntersect = (*iter).GetConnectionPos();

		fTemp = (vIntersect - target.GetPosition()).MagSqr();
		if (fMagSqr > fTemp)
		{
			fMagSqr = fTemp;

			if ((*iter).GetVolumePtr())
				pDestVol = (*iter).GetVolumePtr();
		}
	}

	// Got a volume, let's move to it
	m_StrategyFollowPath.Update();
	m_StrategyFollowPath.Set(pDestVol);
}

void CAIStrategyLurk::StartAttack(RootFSM * fsm)
{
	if(!GetAIPtr() || !GetAIPtr()->HasTarget())
	{
		fsm->AddMessage(Root::eTargetInvalid);
		return;
	}
}

void CAIStrategyLurk::UpdateAttack(RootFSM *)
{
	const CAITarget & target = GetAIPtr()->GetTarget();

	// Is the target still visible?
	if (target.IsPartiallyVisible() != true)
	{
		m_RootFSM.AddMessage(Root::eCannotSeeTarget);
		return;
	}
}



void CAIStrategyLurk::ApproachTarget()
{
	m_StrategyApproachTarget.Update();
}

#ifdef STRATEGY_LURK_DEBUG

void CAIStrategyLurk::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
	const char * szMessage = 0;
	switch( state )
	{
		case Root::eStateSearch : szMessage = "eStateSearch"; break;
		case Root::eStateAdvance : szMessage = "eStateAdvance"; break;
		case Root::eStateRetreat : szMessage = "eStateRetreat"; break;
		case Root::eStateAttack : szMessage = "eStateAttacks"; break;
		default : szMessage = "Unknown"; break;
	}

	AICPrint(GetAIPtr() ? GetAIPtr()->m_hObject : LTNULL,
		" Lurk::RootFSM is entering state %s.", szMessage );
}

void CAIStrategyLurk::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
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

void CAIStrategyLurk::PrintRootFSMState(const RootFSM & fsm, Root::State state)
{
}

void CAIStrategyLurk::PrintRootFSMMessage(const RootFSM & fsm, Root::Message message)
{
}

#endif