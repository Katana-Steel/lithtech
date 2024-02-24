// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackMelee.cpp
//
// PURPOSE : AIGoalAttackMelee implementation
//
// CREATED : 10/09/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackMelee.h"
#include "AIGoalMgr.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AnimatorPlayer.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackMelee, kGoal_AttackMelee);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackMelee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackMelee::CAIGoalAttackMelee()
{
	m_eWeaponType = kAIWeap_Melee;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackMelee::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackMelee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAttackMelee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackMelee::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackMelee::ActivateGoal()
{
	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackMelee::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAttackMelee::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		if( !m_pGoalMgr->IsCurGoal( this ) )
		{
			if( !g_pAIVolumeMgr->StraightPathExists( m_pAI, 
				m_pAI->GetPosition(), 
				pSenseRecord->vLastStimulusPos, 
				m_pAI->GetVerticalThreshold() * 2.f, 
				AIVolume::kVolumeType_Ladder | 
				AIVolume::kVolumeType_Stairs | 
				AIVolume::kVolumeType_Ledge | 
				AIVolume::kVolumeType_JumpUp | 
				AIVolume::kVolumeType_JumpOver | 
				AIVolume::kVolumeType_AmbientLife |
				AIVolume::kVolumeType_Teleport,
				m_pAI->GetLastVolume() ) )
			{
				return LTFALSE;
			}
		}

		return LTTRUE;
	}

	return LTFALSE;
}

