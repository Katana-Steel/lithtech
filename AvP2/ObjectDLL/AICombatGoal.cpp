// AICombatGoal
//
// Make this a parent to combat-related goals
// Handles AI reactions (dodging, target acquisition, etc)

#include "stdafx.h"
#include "AICombatGoal.h"
#include "AI.h"
#include "AITarget.h"
#include "AISenseMgr.h"
#include "AIButeMgr.h"
#include "CharacterMgr.h"
#include "Attachments.h"

#ifdef _DEBUG
//#define AI_WEAPON_DEBUG
#endif

// Initialization
AICombatGoal::AICombatGoal(CAI * pAI, std::string sCreateName)
	: Goal(pAI, sCreateName),
	  m_fTopDamage(0),
	  m_fLastDamageTime(-1.0f),
	  m_bAllowWeaponSwitch(LTTRUE),
	  m_fSwitchWeaponDelay(0),
	  m_fSwitchWeaponChance(0)
{
	ASSERT( pAI );
	if( pAI )
	{
		m_pSeeEnemy = dynamic_cast<CAISenseSeeEnemy*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy));
		ASSERT( m_pSeeEnemy );

		m_pHearEnemyFootstep = dynamic_cast<CAISenseHearEnemyFootstep*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyFootstep));
		ASSERT( m_pHearEnemyFootstep );

		m_pHearEnemyWeaponFire = dynamic_cast<CAISenseHearEnemyWeaponFire*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire));
		ASSERT( m_pHearEnemyWeaponFire );

		m_pHearAllyBattleCry = dynamic_cast<CAISenseHearAllyBattleCry*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyBattleCry));
		ASSERT( m_pHearAllyBattleCry );

		m_pHearAllyWeaponFire = dynamic_cast<CAISenseHearAllyWeaponFire*>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyWeaponFire));
		ASSERT( m_pHearAllyWeaponFire );

		m_pHearWeaponImpact = dynamic_cast<CAISenseHearWeaponImpact *>(pAI->GetSenseMgr().GetSensePtr(CAISenseMgr::HearWeaponImpact));
		ASSERT(m_pHearWeaponImpact);

		m_tmrTargetDelay.Stop();

		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate( pAI->GetTemplateId() );

		m_fSwitchWeaponDelay = butes.fSwitchWeaponDelay;
		m_fSwitchWeaponChance = butes.fSwitchWeaponChance;
	}

	return;
}

void AICombatGoal::Update()
{
	ASSERT(GetAIPtr());
	if (!GetAIPtr())
		return;

	// Call our parent's update.
	Goal::Update();


	// Find best target
	NewTarget new_target = FindNewTarget();
	if( new_target.hTarget )
	{
		GetAIPtr()->Target( new_target.hTarget, new_target.vPos );
	}

	// Be sure the target is valid.
	const CAITarget & target = GetAIPtr()->GetTarget();
	if( !target.IsValid() )
		return;

	// Choose best weapon.
	if( m_bAllowWeaponSwitch )
	{
		if( target.IsValid() && m_tmrSwitchWeapon.Stopped() )
		{
			m_tmrSwitchWeapon.Start(m_fSwitchWeaponDelay);

			if( GetRandom(0.0f,1.0f) < m_fSwitchWeaponChance )
			{
				const AIWBM_AIWeapon * const pOldWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();
				
				GetAIPtr()->ChooseBestWeapon( (target.GetPredictedPosition(GetAIPtr()->GetUpdateDelta()*2.0f) - GetAIPtr()->GetPosition()).MagSqr() );

				const AIWBM_AIWeapon * const pCurrentWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();

				if( pCurrentWeaponButes && pCurrentWeaponButes != pOldWeaponButes )
				{
					// Reset the alternate weapon timer.
					m_tmrAltWeapon.Start( GetRandom(pCurrentWeaponButes->fAltWeaponMinDelay,pCurrentWeaponButes->fAltWeaponMaxDelay) );

#ifdef AI_WEAPON_DEBUG
					AICPrint(GetAIPtr(),"Choosing better weapon, from \"%s\" to \"%s\".",
						pOldWeaponButes ? pOldWeaponButes->Name.c_str() : "none",
						GetAIPtr()->GetCurrentWeaponButesPtr() ? GetAIPtr()->GetCurrentWeaponButesPtr()->Name.c_str() : "None");
#endif
				}

			}
		}
	}

	// Possible switch to our alternate weapon.
	const AIWBM_AIWeapon * pCurrentWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();
	if( pCurrentWeaponButes && m_tmrAltWeapon.Stopped() )
	{
		// Check for chance to use alternate weapon.
		if( GetRandom(0.0f,1.0f) < pCurrentWeaponButes->fAltWeaponChance )
		{
			const AIWBM_AIWeapon * const pOldWeaponButes = pCurrentWeaponButes;

			// Change our weapon!
			GetAIPtr()->SetWeapon(pCurrentWeaponButes->AltWeaponName.c_str(), true);
			pCurrentWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();

			// Be sure we still have a weapon, if not go back to the old one.
			if(    !pCurrentWeaponButes 
				|| pOldWeaponButes == pCurrentWeaponButes 
				|| GetAIPtr()->OutOfAmmo() )
			{
				GetAIPtr()->SetWeapon(pOldWeaponButes->Name.c_str(), true);

				pCurrentWeaponButes = GetAIPtr()->GetCurrentWeaponButesPtr();

#ifdef AI_WEAPON_DEBUG
				AICPrint(GetAIPtr(),"Returned to weapon \"%s\".",
					pOldWeaponButes ? pOldWeaponButes->Name.c_str() : "None");
#endif
			}
			else
			{

#ifdef AI_WEAPON_DEBUG
				AICPrint(GetAIPtr(),"Choose alt weapon \"%s\".",
					pCurrentWeaponButes ? pCurrentWeaponButes->Name.c_str() : "None");
#endif
			}

		} //if( GetRandom(0.0f,1.0f) < pCurrentWeaponButes->fSwitchWeaponChance )

		// Reset the alternate weapon timer.
		if( pCurrentWeaponButes )
		{
			// Be sure we don't switch weapons too quickly.
			// Just use a little more than the max burst delay.
			m_tmrAltWeapon.Start( GetRandom(pCurrentWeaponButes->fAltWeaponMinDelay,pCurrentWeaponButes->fAltWeaponMaxDelay) );
		}

	}

	return;
}

/*
void AICombatGoal::UpdateDodge()
{
	const AIBM_Dodging & dodgeBute = g_pAIButeMgr->GetDodging(GetAIPtr()->GetTemplateId());
	const CAITarget & target = GetAIPtr()->GetTarget();

	// if there was a weapon impact sound
	if (m_pHearWeaponImpact->GetStimulation() == 1.0f)
	{
		// and it was really really close to us - get a move on!
		if (dodgeBute.fDodgeImpactDist < m_pHearWeaponImpact->GetDistance())
		{
			if (target.IsPartiallyVisible())
			{
				m_StrategyDodgeTarget.Start();
			}
		}
	}
	// otherwise, dodge if the target is attacking and facing our way
	else if (target.IsAttacking())
	{
		if (target.IsFacing())
		{
			m_StrategyDodgeTarget.Start();
		}
	}

	// Perform any requested dodging.
	m_StrategyDodgeTarget.Update();
}
*/

void AICombatGoal::End()
{
	Goal::End();

	m_hTopDamager = LTNULL;
	m_fTopDamage = 0.0f;

//	m_StrategyDodgeTarget.Stop();
}

AICombatGoal::NewTarget AICombatGoal::FindNewTarget()
{
	// Some handy constants.
	const AIBM_Targeting & bute = g_pAIButeMgr->GetTargeting(GetAIPtr()->GetTemplateId());
	const CAITarget & target = GetAIPtr()->GetTarget();

	// No target, so try to find one
	if (target.IsValid() == false)
	{
		if (m_pSeeEnemy->GetStimulation() >= 1.0f )
		{
			return NewTarget(m_pSeeEnemy->GetStimulus(), m_pSeeEnemy->GetStimulusPos(), NewTarget::SeeEnemy);
		}

		if (m_pHearEnemyFootstep->GetStimulation() >= 1.0f )
		{
			return NewTarget(m_pHearEnemyFootstep->GetStimulus(), m_pHearEnemyFootstep->GetStimulusPos(), NewTarget::OtherSense);
		}

		if (m_pHearEnemyWeaponFire->GetStimulation() >= 1.0f )
		{
			return NewTarget( m_pHearEnemyWeaponFire->GetStimulus(), m_pHearEnemyWeaponFire->GetStimulusPos(), NewTarget::OtherSense );
		} 
		
		if (m_pHearAllyBattleCry->GetStimulation() >= 1.0f )
		{
			// Only ally weapon fire from CCharacter's will be dealt with (eg. Turrets are ignored!).
			// m_pHearAllyBattleCry can only handle CCharacter weapon fire, 
			// so this should always be true.
			const CCharacter* pFirer = 
				dynamic_cast<const CCharacter*>( g_pLTServer->HandleToObject(m_pHearAllyBattleCry->GetStimulus()) );

			if( pFirer )
			{
				// If it is friendly fire, target their target.
				// This can only be known for AI's.  I wish I 
				// could determine the player's target so that 
				// player friendly AI would be more helpful.
				const CAI * pAIFirer = dynamic_cast<const CAI*>(pFirer);
				if( pAIFirer && pAIFirer->HasTarget())
				{
					if( pAIFirer->GetTarget().GetObject() )
					{
						return NewTarget(pAIFirer->GetTarget().GetObject(),pAIFirer->GetTarget().GetPosition(), NewTarget::HearBattleCry);
					}
				}
			}

		} //if (m_pHearAllyBattleCry->GetStimulation() > 0.0f )

		if (m_pHearAllyWeaponFire->GetStimulation() >= 1.0f )
		{
			// Only ally weapon fire from CCharacter's will be dealt with (eg. Turrets are ignored!).
			// m_pHearAllyWeaponFire can only handle CCharacter weapon fire, 
			// so this should always be true.
			const CCharacter* pFirer = 
				dynamic_cast<const CCharacter*>( g_pLTServer->HandleToObject(m_pHearAllyWeaponFire->GetStimulus()) );

			if( pFirer )
			{
				// If it is friendly fire, target their target.
				// This can only be known for AI's.  I wish I 
				// could determine the player's target so that 
				// player friendly AI would be more helpful.
				const CAI * pAIFirer = dynamic_cast<const CAI*>(pFirer);
				if( pAIFirer && pAIFirer->HasTarget())
				{
					if( pAIFirer->GetTarget().GetObject() )
					{
						return NewTarget(pAIFirer->GetTarget().GetObject(),pAIFirer->GetTarget().GetPosition(), NewTarget::HearAllyWeaponFire);
					}
				}
			}

		} //if (m_pHearAllyWeaponFire->GetStimulation() > 1.0f )

		if (m_pHearWeaponImpact->GetStimulation() >= 1.0f )
		{
			return NewTarget( m_pHearWeaponImpact->GetStimulus(), m_pHearWeaponImpact->GetStimulusPos(), NewTarget::OtherSense );
		}

	} //if (target.IsValid() == false)

	// If damaged by someone else, consider switching targets
	if (m_tmrTargetDelay.Stopped() || target.IsValid() == false )
	{
		if( GetAIPtr()->GetDestructible()->GetLastDamageTime() > m_fLastDamageTime )
		{
			m_fLastDamageTime = GetAIPtr()->GetDestructible()->GetLastDamageTime();
			HOBJECT hDamager = GetAIPtr()->GetDestructible()->GetLastDamager();

			if (hDamager && hDamager != GetAIPtr()->GetTarget().GetObject() && IsMoreDangerous(hDamager))
			{
				LTVector vPos;
				g_pInterface->GetObjectPos(hDamager, &vPos);

				m_tmrTargetDelay.Start(bute.fTargetDelay);

				return NewTarget(hDamager, vPos, NewTarget::Damage);
			}
		}
	}

	return NewTarget();
}

LTBOOL  AICombatGoal::HearFullWeaponImpact() const
{
	return m_pHearWeaponImpact->GetStimulation() >= 1.0f;
}

LTBOOL	AICombatGoal::HearPartialWeaponImpact() const
{
	return m_pHearWeaponImpact->HasPartialStimulus();
}

// IsMoreDangerous
//
// Records the attacker with the highest damage capability so far.
// Returns LTTRUE if the top attacker gets updated.
LTBOOL AICombatGoal::IsMoreDangerous(HOBJECT hDamager)
{
	CCharacter * pDamager;
	CHumanAttachments * pAttach;
	CAttachmentWeapon * pAttachedWeapon;
	LTFLOAT fDamage;

	if (!hDamager)
		return LTFALSE;

	pDamager = dynamic_cast<CCharacter *>(g_pInterface->HandleToObject(hDamager));
	if (pDamager)
	{
		// Friends are not dangerous...
		if (g_pCharacterMgr->AreAllies(*GetAIPtr(), *pDamager))
			return LTFALSE;

		pAttach = dynamic_cast<CHumanAttachments*>(pDamager->GetAttachments());
		if(!pAttach) 
			return LTFALSE;

		pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());
		if (pAttachedWeapon)
		{
			const AMMO * pAmmo = pAttachedWeapon->GetWeapons()->GetCurWeapon()->GetAmmoData();
			fDamage = LTFLOAT( std::max( pAmmo->nInstDamage, pAmmo->nAreaDamage ) );
			fDamage = std::max( fDamage, pAmmo->fProgDamage*pAmmo->fProgDamageDuration );
		}
		else
		{
			fDamage = GetAIPtr()->GetDestructible()->GetLastDamage();
		}

		// No damager was recorded, so copy it over and exit
		if (!m_hTopDamager)
		{
			m_hTopDamager = hDamager;
			m_fTopDamage = fDamage;
			return LTTRUE;
		}

		// If the damage is greater, then update with the new data
		// Also update if the attacker is the same, because he may have changed weapons
		if ((fDamage > m_fTopDamage) || (hDamager == m_hTopDamager))
		{
			// This attack is stronger, so return this target
			m_fTopDamage = fDamage;
			m_hTopDamager = hDamager;
			return LTTRUE;
		}
	}

	// No change
	return LTFALSE;
}


void AICombatGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT( hRead );

	Goal::Load(hRead);

//	*hRead >> m_StrategyDodgeTarget;

	*hRead >> m_hTopDamager;
	*hRead >> m_fTopDamage;
	*hRead >> m_fLastDamageTime;
	*hRead >> m_tmrTargetDelay;

	*hRead >> m_bAllowWeaponSwitch;
	*hRead >> m_tmrSwitchWeapon;
	*hRead >> m_fSwitchWeaponDelay;
	*hRead >> m_fSwitchWeaponChance;

	*hRead >> m_tmrAltWeapon;
}

void AICombatGoal::Save(HMESSAGEWRITE hWrite)
{
	_ASSERT( hWrite );

	Goal::Save(hWrite);

//	*hWrite << m_StrategyDodgeTarget;

	*hWrite << m_hTopDamager;
	*hWrite << m_fTopDamage;
	*hWrite << m_fLastDamageTime;
	*hWrite << m_tmrTargetDelay;

	*hWrite << m_bAllowWeaponSwitch;
	*hWrite << m_tmrSwitchWeapon;
	*hWrite << m_fSwitchWeaponDelay;
	*hWrite << m_fSwitchWeaponChance;

	*hWrite << m_tmrAltWeapon;
}
