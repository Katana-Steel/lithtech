// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyEngageTarget.cpp
//
// PURPOSE : AI strategy class implementations
//
// CREATED : 9.9.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIStrategyEngageTarget.h"
#include "AIButeMgr.h"
#include "AITarget.h"
#include "AIUtils.h"

#ifdef _DEBUG
#include "DebugLineSystem.h"
#endif

CAIStrategyEngageTarget::CAIStrategyEngageTarget(CAI * pAI) 
	: CAIStrategy(pAI),
	  m_StrategyApproachTarget(pAI),
	  m_StrategyRetreat(pAI)
{
}

void CAIStrategyEngageTarget::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAIStrategy::Load(hRead);

	*hRead >> m_StrategyApproachTarget;
	*hRead >> m_StrategyRetreat;
}

void CAIStrategyEngageTarget::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAIStrategy::Save(hWrite);

	*hWrite << m_StrategyApproachTarget;
	*hWrite << m_StrategyRetreat;
}

static void MaintainTarget( LTFLOAT target_range_sqr, const AIWBM_AIWeapon & ideal_weapon_profile, 
						    CAIStrategyApproachTarget * pStrategyApproachTarget )
{
	if( !pStrategyApproachTarget )
		return;

	if( target_range_sqr > ideal_weapon_profile.fMaxIdealRange*ideal_weapon_profile.fMaxIdealRange )
	{
		pStrategyApproachTarget->Update();
	}
}

void CAIStrategyEngageTarget::Update()
{
	ASSERT( GetAIPtr() );
	if( !GetAIPtr() ) return;

	if(    GetAIPtr()->GetCurrentWeaponPtr()
		&& GetAIPtr()->GetCurrentWeaponButesPtr()
		&& NeedReload(*GetAIPtr()->GetCurrentWeaponPtr(), *GetAIPtr()->GetCurrentWeaponButesPtr()) )
	{
		GetAIPtr()->SetNextAction(CAI::ACTION_RELOAD);
		return;
	}

	const AIWBM_AIWeapon * pIdealWeaponProfile = GetAIPtr()->GetIdealWeaponButesPtr();
	if( pIdealWeaponProfile )
	{
		const LTVector vTargetRange = ( GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition() );
		const LTFLOAT  fTargetRangeSqr = vTargetRange.MagSqr();

		const LTVector vFlatTargetRange = (vTargetRange - GetAIPtr()->GetUp()*GetAIPtr()->GetUp().Dot(vTargetRange));
		const LTFLOAT  fFlatTargetRangeSqr = vFlatTargetRange.MagSqr();

		// Keep within a good range of the target.  
		// Using the flat range for this so that they will back up when target is right above them.
		MaintainTarget( fFlatTargetRangeSqr, *pIdealWeaponProfile, &m_StrategyApproachTarget );

		// Don't fire while wall-walking!
		if( GetAIPtr()->GetMovement()->GetCharacterButes()->m_bWallWalk
			&& (GetAIPtr()->GetMovement()->GetControlFlags() & CM_FLAG_WALLWALK) )
			return;

		// Only fire if within range of current weapon.
		const AIWBM_AIWeapon * pCurrentWeaponProfile = GetAIPtr()->GetCurrentWeaponButesPtr();
		if(    pCurrentWeaponProfile 
		    && fTargetRangeSqr > pCurrentWeaponProfile->fMinFireRange*pCurrentWeaponProfile->fMinFireRange
			&& fTargetRangeSqr < pCurrentWeaponProfile->fMaxFireRange*pCurrentWeaponProfile->fMaxFireRange )
		{
			// Only fire if target can be seen.
			if( GetAIPtr()->GetTarget().IsFullyVisible() && GetAIPtr()->BurstDelayExpired() )
			{

				// TODO: remove this workaround and let the FireWeapon action do the aiming
				LTVector vDiff = GetAIPtr()->GetTarget().GetPosition() - GetAIPtr()->GetPosition();
				vDiff.Norm();
				const LTFLOAT fOff = vDiff.Dot(GetAIPtr()->GetAimForward());
				if (fOff > c_fCos30)
				{
					if (!GetAIPtr()->WillHurtFriend())
					{
						GetAIPtr()->SetNextAction( CAI::ACTION_FIREWEAPON );
						return;
					}
				}
			}
		}

	} //if( pIdealWeaponProfile )
}
