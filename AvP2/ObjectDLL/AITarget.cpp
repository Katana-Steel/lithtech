// ----------------------------------------------------------------------- //
//
// MODULE  : AITarget.cpp
//
// PURPOSE : Target information implementation
//
// CREATED : 05.10.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AITarget.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "AIUtils.h"
#include "CVarTrack.h"
#include "DebugLineSystem.h"
#include "CharacterMgr.h"

#ifndef _FINAL
static bool ShouldShowTarget(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

	static CVarTrack vtShowAITarget(g_pLTServer,"ShowAITarget",0.0f);

	return vtShowAITarget.GetFloat() > 0.0f && ShouldShow(hObject);
}
#endif

const LTFLOAT c_fMinLagRatioSqr = 0.5f;
const LTFLOAT c_fMaxLagRatioSqr = 0.5f;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Save
//
//	PURPOSE:	Saves our data
//
// ----------------------------------------------------------------------- //
void CAITarget::Init(const CAI* pAI)
{
	ASSERT(pAI);
	if( !pAI ) return;

	m_pAI = pAI;

	const CAISenseMgr & sense_mgr = m_pAI->GetSenseMgr();

	const CAISenseSeeEnemy * pSense = dynamic_cast<const CAISenseSeeEnemy*>(sense_mgr.GetSensePtr( CAISenseMgr::SeeEnemy ));

	ASSERT( pSense );
	if( !pSense ) return;

	m_pSeeEnemySense = pSense;
}

void CAITarget::GetAttributes( int nTemplateId )
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_Targeting & butes = g_pAIButeMgr->GetTargeting(nTemplateId);

	m_fMinFullSightStimulation = butes.fFullSighting;
	m_fMaxFireCheckTime = butes.fMaxFiredCheck;
	m_fMinAttackDistanceSqr = butes.fMinAttackingDist*butes.fMinAttackingDist;
	m_fMaintainTargetDuration = butes.fMaintainDuration;
	m_fMaintainCloakedTargetDuration = butes.fMaintainCloakedDuration;
	m_fMaintainCloakedTargetChance = butes.fMaintainCloakedChance;
	m_fMaintainTargetChance = butes.fMaintainChance;

	m_fStillAttackingDuration = butes.fStillAttackingDuration;

	m_fIncreaseLagRate = butes.fIncreaseLagTime > 0.0f ? 1.0f/butes.fIncreaseLagTime : 0.0f;
	m_fDecreaseLagRate = butes.fDecreaseLagTime > 0.0f ? 1.0f/butes.fDecreaseLagTime : 0.0f;
	m_fIdleLagRate = butes.fIdleLagTime > 0.0f ? 1.0f/butes.fIdleLagTime : 0.0f;
	m_fMaxLag = butes.fMaxLag;
	m_fMinLag = butes.fMinLag;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Save
//
//	PURPOSE:	Saves our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Save(HMESSAGEWRITE hWrite)
{
	if ( !g_pServerDE || !hWrite ) return;

	// m_pAI set by Init.
	// m_pSeeEnemySense set by Init.

	*hWrite << m_fMinFullSightStimulation;
	*hWrite << m_fMaxFireCheckTime;
	*hWrite << m_fMinAttackDistanceSqr;
	*hWrite << m_fStillAttackingDuration;
	*hWrite << m_fIncreaseLagRate;
	*hWrite << m_fDecreaseLagRate;
	*hWrite << m_fIdleLagRate;
	*hWrite << m_fMaxLag;
	*hWrite << m_fMinLag;

	*hWrite << m_bPartiallyVisible;
	*hWrite << m_bFullyVisible;
	*hWrite << m_hTarget;

	*hWrite << m_vLag;

	*hWrite << m_vLastPosition;
	*hWrite << m_vLastVelocity;
	hWrite->WriteFloat(m_fLastSpottingTime - g_pLTServer->GetTime());

	hWrite->WriteFloat(m_fLastFireTime - g_pLTServer->GetTime());
	*hWrite << m_tmrStillAttacking;
	*hWrite << m_vAttackingPoint;

	*hWrite << m_bTouching;
	*hWrite << m_bAimAtHead;

	*hWrite << m_fMaintainTargetDuration;
	*hWrite << m_fMaintainTargetChance;

	*hWrite << m_fMaintainCloakedTargetDuration;
	*hWrite << m_fMaintainCloakedTargetChance;

	*hWrite << m_tmrMaintainTarget;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Load
//
//	PURPOSE:	Loads our data
//
// ----------------------------------------------------------------------- //

void CAITarget::Load(HMESSAGEREAD hRead)
{
	if ( !g_pServerDE || !hRead ) return;

	// m_pAI set by Init.
	// m_pSeeEnemySense set by Init.

	*hRead >> m_fMinFullSightStimulation;
	*hRead >> m_fMaxFireCheckTime;
	*hRead >> m_fMinAttackDistanceSqr;
	*hRead >> m_fStillAttackingDuration;
	*hRead >> m_fIncreaseLagRate;
	*hRead >> m_fDecreaseLagRate;
	*hRead >> m_fIdleLagRate;
	*hRead >> m_fMaxLag;
	*hRead >> m_fMinLag;

	*hRead >> m_bPartiallyVisible;
	*hRead >> m_bFullyVisible;
	*hRead >> m_hTarget;

	*hRead >> m_vLag;

	*hRead >> m_vLastPosition;
	*hRead >> m_vLastVelocity;
	m_fLastSpottingTime = hRead->ReadFloat() + g_pLTServer->GetTime();

	m_fLastFireTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_tmrStillAttacking;
	*hRead >> m_vAttackingPoint;

	*hRead >> m_bTouching;
	*hRead >> m_bAimAtHead;

	*hRead >> m_fMaintainTargetDuration;
	*hRead >> m_fMaintainTargetChance;

	*hRead >> m_fMaintainCloakedTargetDuration;
	*hRead >> m_fMaintainCloakedTargetChance;

	*hRead >> m_tmrMaintainTarget;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::Update
//
//	PURPOSE:	Performs target info updates
//
// ----------------------------------------------------------------------- //
void CAITarget::Update()
{
	// 
	// Update visibility.  If target is not visible, don't do anything
	// further (keep old values).
	//

#ifndef _FINAL
	if( ShouldShowTarget(m_pAI->m_hObject) )
	{
		LineSystem::GetSystem(this,"ShowTarget")
			<< LineSystem::Clear();

		if( m_hTarget )
		{
			LTVector vTargetPos;
			g_pLTServer->GetObjectPos(m_hTarget,&vTargetPos);
			LineSystem::GetSystem(this,"ShowTarget")
				<< LineSystem::Arrow(m_pAI->GetPosition(),vTargetPos, Color::Yellow)
				<< LineSystem::Box(m_vLastPosition, LTVector(50,50,50), Color::Green)
				<< LineSystem::Line(m_pAI->GetPosition(),m_vLastPosition, Color::Green);
		}
	}
#endif

	// Assume target is not visible.
	m_bPartiallyVisible = false;
	m_bFullyVisible = false;

	// GetCharacter will check that we have an active handle.
	const CCharacter * pCharacter = GetCharacterPtr();
	if( !pCharacter ) return;

	// Be sure we don't keep attacking a dead target!
	if ( pCharacter->IsDead() || (g_pLTServer->GetObjectState(pCharacter->m_hObject) & OBJSTATE_INACTIVE) ) 
	{
		SetObject(LTNULL);
		return;
	}


	// If the target has become and ally somehow, stop targeting them!
	if( g_pCharacterMgr->AreAllies(*pCharacter,*m_pAI) )
	{
		SetObject(LTNULL);
		return;
	}

	// Be sure we have a pointer to our sense.
	ASSERT( m_pSeeEnemySense );
	if( !m_pSeeEnemySense ) return;

	// Check that the sense is tracking our target,
	//  if it isn't then the target isn't visible.
	const LTFLOAT stimulation = m_pSeeEnemySense->GetStimulation(pCharacter->m_hObject);

	LTFLOAT fMaintainDuration, fMaintainChance;
	if (GetCharacterPtr() && (GetCharacterPtr()->GetCloakState() != CLOAK_STATE_INACTIVE))
	{
		fMaintainDuration = m_fMaintainCloakedTargetDuration;
		fMaintainChance = m_fMaintainCloakedTargetChance;
	}
	else
	{
		fMaintainDuration = m_fMaintainTargetDuration;
		fMaintainChance = m_fMaintainTargetChance;
	}


	// Check for any visiblity.
	// If not visible, there is still a chance of following the target
	// using the maintain target timer.
	if( stimulation <= 0.0f )
	{
		if( !m_pAI->NeverLoseTarget() )
		{
			// If the timer has been turned off, don't do anything further.
			if( !m_tmrMaintainTarget.On() )
			{
				return;
			}
			else if( m_tmrMaintainTarget.Stopped() )
			{
				// The timer has just finished, either re-start it 
				// or stop it.
				if( GetRandom(0.0f,1.0f) < fMaintainChance )
				{
					m_tmrMaintainTarget.Start( fMaintainDuration );
				}
				else
				{
					// Stop() turns the timer off.
					// ( On() will return false after this point).
					m_tmrMaintainTarget.Stop();
					return;
				}
			}
		}
		else
		{
			// Just keep restarting the maintain target timer to show that
			// the target is maintained!
			m_tmrMaintainTarget.Start( fMaintainDuration );
		}
	}
	else
	{
		m_bPartiallyVisible = true;
		m_tmrMaintainTarget.Start( fMaintainDuration );
	}

	const LTVector & my_forward = m_pAI->GetForward();
	const LTVector & my_right = m_pAI->GetRight();
	const LTVector & my_up = m_pAI->GetUp();
	const LTVector & my_pos = m_pAI->GetPosition();
	const LTVector & my_dims = m_pAI->GetDims();

	const LTVector vApproxWeaponPos = m_pAI->GetAimFromPosition() + my_forward*my_dims.z;
	const LTFLOAT fTargetDistSqr = (m_vLastPosition - vApproxWeaponPos).MagSqr();

	LTFLOAT fMinLag = m_fMinLag;
	if( fMinLag*fMinLag > fTargetDistSqr*c_fMinLagRatioSqr )
	{
		fMinLag = (LTFLOAT)sqrt(fTargetDistSqr*c_fMinLagRatioSqr);
		if( fMinLag <= 0.0f )
			fMinLag = 0.1f;
	}

	LTFLOAT fMaxLag = m_fMaxLag;
	if( fMaxLag*fMaxLag > fTargetDistSqr*c_fMaxLagRatioSqr )
	{
		fMaxLag = (LTFLOAT)sqrt(fTargetDistSqr*c_fMaxLagRatioSqr);

		if(fMaxLag <= 0.0f )
			fMaxLag = 0.1f;
	}

	// Only do our "idle" lag if the lag is below 
	// the minimum lag.
	const LTFLOAT fRightLeftLag = m_vLag.Dot(my_right);
	const LTFLOAT fUpDownLag = m_vLag.Dot(my_up);
	if(    fabs(fRightLeftLag) < fMinLag 
		&& fabs(fRightLeftLag) < fMinLag  )
	{
		_ASSERT( fMinLag > 0.0f );

		const LTFLOAT fRightLeftPercent = fRightLeftLag / fMinLag;
		if( fRightLeftPercent > 0.1f )
		{
			m_vLag += my_right*GetRandom(-(2.0f-fRightLeftPercent)*fMinLag,fRightLeftPercent*fMinLag)*(m_fIdleLagRate + m_fDecreaseLagRate)*m_pAI->GetUpdateDelta();
		}
		else if( fRightLeftPercent < -0.1f )
		{
			m_vLag += my_right*GetRandom(fRightLeftPercent*fMinLag,(2.0f + fRightLeftPercent)*fMinLag)*(m_fIdleLagRate + m_fDecreaseLagRate)*m_pAI->GetUpdateDelta();
		}
		else
		{
			m_vLag += my_right*GetRandom(-fMinLag,fMinLag)*(m_fIdleLagRate + m_fDecreaseLagRate)*m_pAI->GetUpdateDelta();
		}

		const LTFLOAT fUpDownPercent = fUpDownLag / fMinLag;
		if( fUpDownPercent > 0.1f )
		{
			m_vLag += my_up*GetRandom(-(2.0f - fUpDownPercent)*fMinLag,fUpDownPercent*fMinLag)*(m_fIdleLagRate + m_fDecreaseLagRate)*m_pAI->GetUpdateDelta();
		}
		else if( fUpDownPercent < -0.1f )
		{
			m_vLag += my_up*GetRandom(fUpDownPercent*fMinLag,(2.0f + fUpDownPercent)*fMinLag)*(m_fIdleLagRate + m_fDecreaseLagRate)*m_pAI->GetUpdateDelta();
		}
		else
		{
			m_vLag += my_up*GetRandom(-fMinLag,fMinLag)*(m_fIdleLagRate + m_fDecreaseLagRate)*m_pAI->GetUpdateDelta();
		}

		m_vLag.x = Clamp(m_vLag.x,-fMinLag,fMinLag);
		m_vLag.y = Clamp(m_vLag.y,-fMinLag,fMinLag);
		m_vLag.z = Clamp(m_vLag.z,-fMinLag,fMinLag);
	}
	else
	{
		m_vLag *= Max(0.0f, 1.0f - m_fDecreaseLagRate*m_pAI->GetUpdateDelta() );
	}

	if( stimulation >= m_fMinFullSightStimulation )
	{
		m_bFullyVisible = true;
	}

	// So the target is either visible or being maintained, go ahead and update
	// its information.


	// Update the moving target variance (called "lag").
	if( m_vLag.MagSqr() < fMaxLag*fMaxLag )
	{
		// The lag should increase like (excluding idle lag effects).
		// if( 	m_fDecreaseLagRate*fDeltaTime < 1.0f )
		// {
		//		m_vLag = m_vLag*(1.0f - m_fDecreaseLagRate*fDeltaTime);
		// } 
		// else 
		// {
		//		m_vLag = LTVector(0,0,0);
		// }
		//	
		// m_vLag += vRandomDirection * vChangePos / fDeltaTime * (m_fIncreaseLagRate + m_fDecreaseLagRate) * fDeltaTime;
		// 
		// Below is slightly optimized (hopefully!).

		const LTFLOAT fDeltaTime = (g_pLTServer->GetTime() - m_fLastSpottingTime);
		const LTVector vDir = my_right*GetRandom(0.0f,1.0f) + my_up*GetRandom(0.0f,1.0f);
		// vDir.Norm(); // Not needed.

		// vVariance is the equivalent of vRandomDirection*vChangePos.
		const LTVector vVariance = vDir*vDir.Dot(m_vLastPosition - pCharacter->GetMovement()->GetPos());  
		

		// This is done above so that the lag will decrease if the target is partially visible.
		// const LTFLOAT fDeltaTime = g_pLTServer->GetTime() - m_fLastSpottingTime;
		// m_vLag *= std_min(0.0f,(1.0f - m_fDecreaseLagRate*fDeltaTime));

		m_vLag += vVariance*(m_fIncreaseLagRate + m_fDecreaseLagRate)*fDeltaTime;

		m_vLag.x = Clamp(m_vLag.x,-fMaxLag,fMaxLag);
		m_vLag.y = Clamp(m_vLag.y,-fMaxLag,fMaxLag);
		m_vLag.z = Clamp(m_vLag.z,-fMaxLag,fMaxLag);
	}

	// Update the target's position and velocity.
	m_vLastPosition = pCharacter->GetMovement()->GetPos();
	m_vLastVelocity = pCharacter->GetMovement()->GetVelocity();
	m_fLastSpottingTime = g_pLTServer->GetTime();



	//
	// Check to see if target is attacking us.
	// If they are shooting near us, they are attacking us!
	//

	const CharFireInfo & info =	pCharacter->GetLastFireInfo();

	// Be sure they have shot recently.
	if( info.fTime > m_fLastFireTime && info.fTime + m_fMaxFireCheckTime < g_pServerDE->GetTime() )
	{
		m_fLastFireTime = info.fTime;

		// The weapon must be able to reach us.
		BARREL * pBarrelData = g_pWeaponMgr->GetBarrel(info.nBarrelId);
		if(    pBarrelData 
			&& (m_pAI->GetPosition() - info.vFiredPos).MagSqr() < LTFLOAT(pBarrelData->nRange*pBarrelData->nRange) )
		{
			// See if the shot passed close to us.
			// Uses closest distance of line of fire to our position
			// to determine if they are shooting "at" us.

			const LTVector vFiredDir = info.vFiredDir;


			LTVector vNearestPoint 
				= info.vFiredPos 
				+ vFiredDir*(((m_pAI->GetPosition() - info.vFiredPos).Dot(vFiredDir))/vFiredDir.Dot(vFiredDir));

			if ( (vNearestPoint - m_pAI->GetPosition()).MagSqr() < m_fMinAttackDistanceSqr )
			{
				m_tmrStillAttacking.Start(m_fStillAttackingDuration);

				m_vAttackingPoint = vNearestPoint;

			}
		}
	}



	// Clear out one shot flags
	m_bTouching = false;

} //void CAITarget::Update()


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::HandleTouch
//
//	PURPOSE:	Sets touch appropriately
//
// ----------------------------------------------------------------------- //
void CAITarget::HandleTouch(HOBJECT hObj)
{
	if ( hObj && hObj == m_hTarget )
	{
		m_bTouching = true;
	}
}

bool CAITarget::IsFacing() const
{
	if( !GetCharacterPtr() ) 
	{
		// Does not seem to be needed. Causes crash: JKE 10/30
//		ASSERT( 0 );
		return false;
	}

	// Is the target facing me?
	LTVector vToAI = m_pAI->GetPosition() - GetPosition();
	vToAI.Norm();

	// 07/10/00 - MarkS: 
	// TODO: Change this to use the field of view, instead of .5
	if (vToAI.Dot(GetCharacterPtr()->GetForward()) > 0.5f)
		return true;

	return false;
}

LTVector CAITarget::GetTargetOffset() const
{
	const CCharacter * pCharTarget = GetCharacterPtr();

	if( pCharTarget )
	{
		if( m_bAimAtHead )
			return pCharTarget->GetHeadOffset();

		return pCharTarget->GetChestOffset();
	}

	return LTVector(0,0,0);
}

void CAITarget::SetObject(const LTSmartLink & target, const LTVector & vLastKnownPos /* = LTVector(FLT_MAX,FLT_MAX,FLT_MAX) */)
{
	m_hTarget = target; 

	if( !m_hTarget )
		return;

	// Record the target's last known position.
	if( vLastKnownPos.x < FLT_MAX )
	{
		m_vLastPosition = vLastKnownPos;
	}
	else
	{
		g_pLTServer->GetObjectPos(m_hTarget,&m_vLastPosition);
	}

	// Be sure our last spotting time is reset to some reasonable value.
	m_fLastSpottingTime = g_pLTServer->GetTime();


	// Set-up our mis-aiming.
	LTFLOAT fMaxLag = m_fMaxLag;
	const LTFLOAT fTargetDistSqr = (m_vLastPosition - m_pAI->GetPosition()).MagSqr();
	if( fMaxLag*fMaxLag > fTargetDistSqr*c_fMaxLagRatioSqr )
	{
		fMaxLag = (LTFLOAT)sqrt(fTargetDistSqr*c_fMaxLagRatioSqr);

		if(fMaxLag <= 0.0f )
			fMaxLag = 0.1f;
	}

	m_vLag = LTVector( GetRandom(-fMaxLag, fMaxLag),
					   GetRandom(-fMaxLag, fMaxLag),
					   GetRandom(-fMaxLag, fMaxLag) );
}
