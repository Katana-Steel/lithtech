// ----------------------------------------------------------------------- //
//
// MODULE  : AISenseMgr.cpp
//
// PURPOSE : Manage all available senses
//
// CREATED : 09.15.2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AI.h"
#include "AISenseMgr.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AIButeMgr.h"
#include "Turret.h"

#include <algorithm>

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define SEE_ENEMY_DEBUG
//#define SENSE_DEBUG
//#define HEAR_DEBUG
//#define SENSE_PROFILE
#endif


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Init
//
//	PURPOSE:	Init the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Init(CAI* pAI)
{
	m_pAI = pAI;

	for ( int iSense = 0 ; iSense < MaxSenses ; iSense++ )
	{
		m_apSenses[iSense]->Init(m_pAI);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::CAISenseMgr
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

CAISenseMgr::CAISenseMgr()
	: m_pAI(LTNULL),
	  m_bEnabled(true)
{
	m_apSenses[SeeEnemy] = &m_SenseSeeEnemy;
	m_apSenses[SeeEnemyFlashlight] = &m_SenseSeeEnemyFlashlight;
	m_apSenses[SeeDeadBody] = &m_SenseSeeDeadBody;
	m_apSenses[SeeBlood] = &m_SenseSeeBlood;
	m_apSenses[SeeCloaked] = &m_SenseSeeCloaked;
	m_apSenses[HearEnemyFootstep] = &m_SenseHearEnemyFootstep;
	m_apSenses[HearEnemyWeaponFire] = &m_SenseHearEnemyWeaponFire;
	m_apSenses[HearAllyWeaponFire] = &m_SenseHearAllyWeaponFire;
	m_apSenses[HearTurretWeaponFire] = &m_SenseHearTurretWeaponFire;
	m_apSenses[HearAllyPain] = &m_SenseHearAllyPain;
	m_apSenses[HearAllyBattleCry] = &m_SenseHearAllyBattleCry;
	m_apSenses[HearEnemyTaunt] = &m_SenseHearEnemyTaunt;
	m_apSenses[HearDeath] = &m_SenseHearDeath;
	m_apSenses[HearWeaponImpact] = &m_SenseHearWeaponImpact;
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Update
//
//	PURPOSE:	Updates the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Update()
{
	if ( !m_bEnabled ) return;

#ifdef SENSE_PROFILE
	LTCounter sense_mgr_counter;
	g_pLTServer->StartCounter(&sense_mgr_counter);
#endif
	
	m_apSenses[SeeEnemy]->Update();
#ifdef SENSE_PROFILE
	const uint32 see_enemy_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "SeeEnemy took %d ticks.", see_enemy_ticks );
#endif

	m_apSenses[SeeEnemyFlashlight]->Update();
#ifdef SENSE_PROFILE
	const uint32 see_enemy_flashlight_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "SeeEnemyFlashlight took %d ticks.", see_enemy_flashlight_ticks - see_enemy_ticks );
#endif

	m_apSenses[SeeDeadBody]->Update();
#ifdef SENSE_PROFILE
	const uint32 see_dead_body_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "SeeDeadBody took %d ticks.", see_dead_body_ticks - see_enemy_flashlight_ticks );
#endif

	m_apSenses[SeeBlood]->Update();
#ifdef SENSE_PROFILE
	const uint32 see_blood_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "SeeBlood took %d ticks.", see_blood_ticks - see_dead_body_ticks);
#endif

	m_apSenses[SeeCloaked]->Update();
#ifdef SENSE_PROFILE
	const uint32 see_cloaked_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "SeeCloaked took %d ticks.", see_cloaked_ticks - see_blood_ticks);
#endif

	m_apSenses[HearEnemyFootstep]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_footstep_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearEnemyFootstep took %d ticks.", hear_enemy_footstep_ticks - see_cloaked_ticks );
#endif

	m_apSenses[HearEnemyWeaponFire]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_EnemyWeaponFire_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearEnemyWeaponFire took %d ticks.", hear_enemy_EnemyWeaponFire_ticks - hear_enemy_footstep_ticks );
#endif

	m_apSenses[HearAllyWeaponFire]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_AllyWeaponFire_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearAllyWeaponFire took %d ticks.", hear_enemy_AllyWeaponFire_ticks - hear_enemy_EnemyWeaponFire_ticks );
#endif

	m_apSenses[HearTurretWeaponFire]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_TurretWeaponFire_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearTurretWeaponFire took %d ticks.", hear_enemy_TurretWeaponFire_ticks - hear_enemy_AllyWeaponFire_ticks );
#endif

	m_apSenses[HearAllyPain]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_AllyPain_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearAllyPain took %d ticks.", hear_enemy_AllyPain_ticks - hear_enemy_TurretWeaponFire_ticks );
#endif

	m_apSenses[HearAllyBattleCry]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_AllyBattleCry_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearAllyBattleCry took %d ticks.", hear_enemy_AllyBattleCry_ticks - hear_enemy_AllyPain_ticks );
#endif

	m_apSenses[HearEnemyTaunt]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_EnemyTaunt_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearEnemyTaunt took %d ticks.", hear_enemy_EnemyTaunt_ticks - hear_enemy_AllyBattleCry_ticks );
#endif

	m_apSenses[HearDeath]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_enemy_Death_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearDeath took %d ticks.", hear_enemy_Death_ticks - hear_enemy_AllyBattleCry_ticks );
#endif

	m_apSenses[HearWeaponImpact]->Update();
#ifdef SENSE_PROFILE
	const uint32 hear_weaponimpact_ticks = g_pLTServer->EndCounter(&sense_mgr_counter);
	AICPrint(m_pAI, "HearWeaponImpact took %d ticks.", hear_weaponimpact_ticks - hear_enemy_Death_ticks);
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::AutoDisableSenses
//
//	PURPOSE:	Calls AutoDisable on all senses.
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::AutoDisableSenses()
{
	for (int iSense = 0; iSense < MaxSenses; iSense++)
	{
		m_apSenses[iSense]->AutoDisable();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::GetAttributes
//
//	PURPOSE:	Gets the attributes for various senses
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::GetAttributes(int nTemplateID)
{
	for (int iSense = 0; iSense < MaxSenses; iSense++)
	{
		m_apSenses[iSense]->GetAttributes(nTemplateID);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::GetProperties
//
//	PURPOSE:	Gets the properties for various senses
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::GetProperties(GenericProp* pgp)
{
	for (int iSense = 0; iSense < MaxSenses; iSense++)
	{
		m_apSenses[iSense]->GetProperties(pgp);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::WeaponFire
//
//	PURPOSE:	Called to stimulate Hear[Ally,Enemy]WeaponFire
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::WeaponFire(const CharFireInfo & info, const CCharacter * pChar)
{
	if( m_SenseHearEnemyWeaponFire.IsEnabled() && !m_SenseHearEnemyWeaponFire.IgnoreCharacter(pChar) )
	{
		m_SenseHearEnemyWeaponFire.WeaponFire(info, pChar);
	}

	if( m_SenseHearAllyWeaponFire.IsEnabled() && !m_SenseHearAllyWeaponFire.IgnoreCharacter(pChar) )
	{
		m_SenseHearAllyWeaponFire.WeaponFire(info, pChar);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::WeaponFire
//
//	PURPOSE:	Called to stimulate Hear[Ally,Enemy]WeaponFire
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::TurretWeaponFire(const CharFireInfo & info, const CCharacter * pTurretTarget)
{
	if( m_SenseHearTurretWeaponFire.IsEnabled() && !m_SenseHearTurretWeaponFire.IgnoreCharacter(pTurretTarget) )
	{
		m_SenseHearTurretWeaponFire.WeaponFire(info, pTurretTarget);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Pain
//
//	PURPOSE:	Called to stimulate Hear[Ally,Enemy]Pain
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::Pain(const SoundButes & info, const CCharacter * pChar)
{
	if( m_SenseHearAllyPain.IsEnabled() && pChar != m_pAI && !m_SenseHearAllyPain.IgnoreCharacter(pChar) )
	{
		m_SenseHearAllyPain.Pain(info, pChar);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::BattleCry
//
//	PURPOSE:	Called to stimulate Hear[Ally,Enemy]BattleCry
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::BattleCry(const SoundButes & info, const CCharacter * pChar)
{
	if( m_SenseHearAllyBattleCry.IsEnabled() && pChar != m_pAI && !m_SenseHearAllyBattleCry.IgnoreCharacter(pChar) )
	{
		m_SenseHearAllyBattleCry.BattleCry(info, pChar);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Taunt
//
//	PURPOSE:	Called to stimulate Hear[Ally,Enemy]Taunt
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::Taunt(const SoundButes & info, const CCharacter * pChar)
{
	if( m_SenseHearEnemyTaunt.IsEnabled() && pChar != m_pAI && !m_SenseHearEnemyTaunt.IgnoreCharacter(pChar) )
	{
		m_SenseHearEnemyTaunt.BattleCry(info, pChar);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Pain
//
//	PURPOSE:	Called to stimulate Hear[Ally,Enemy]Pain
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::Death(const SoundButes & info, const BodyProp * pBody)
{
	if( m_SenseHearDeath.IsEnabled() )
	{
		m_SenseHearDeath.Death(info, pBody);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Pain
//
//	PURPOSE:	Called to stimulate Hear[Ally,Enemy]Pain
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::TouchCharacter(const CCharacter & character)
{
	if( m_SenseSeeEnemy.IsEnabled() )
	{
		m_SenseSeeEnemy.Touched(character);
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::WeaponImpact
//
//	PURPOSE:	Called to stimulate HearWeaponImpact
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::WeaponImpact(const CharImpactInfo & info, const CCharacter * pChar)
{
	if( m_SenseHearWeaponImpact.IsEnabled() && pChar != m_pAI && !m_SenseHearWeaponImpact.IgnoreCharacter(pChar) )
	{
		m_SenseHearWeaponImpact.WeaponImpact(info,pChar);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Footstep
//
//	PURPOSE:	Called to stimulate HearEnemyFootstep
//
// ----------------------------------------------------------------------- //
void CAISenseMgr::Footstep(const CharFootstepSoundInfo & info, const CCharacter * pChar)
{
	if( m_SenseHearEnemyFootstep.IsEnabled() && pChar != m_pAI && !m_SenseHearEnemyFootstep.IgnoreCharacter(pChar) )
	{
		m_SenseHearEnemyFootstep.Footstep(info,pChar);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Save
//
//	PURPOSE:	Save the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_bEnabled;

	*hWrite << m_SenseSeeEnemy;
	*hWrite << m_SenseSeeEnemyFlashlight;
	*hWrite << m_SenseSeeDeadBody;
	*hWrite << m_SenseSeeBlood;
	*hWrite << m_SenseSeeCloaked;
	*hWrite << m_SenseHearEnemyFootstep;
	*hWrite << m_SenseHearEnemyWeaponFire;
	*hWrite << m_SenseHearAllyWeaponFire;
	*hWrite << m_SenseHearTurretWeaponFire;
	*hWrite << m_SenseHearAllyPain;
	*hWrite << m_SenseHearAllyBattleCry;
	*hWrite << m_SenseHearEnemyTaunt;
	*hWrite << m_SenseHearDeath;
	*hWrite << m_SenseHearWeaponImpact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseMgr::Load
//
//	PURPOSE:	Load the SenseMgr
//
// ----------------------------------------------------------------------- //

void CAISenseMgr::Load(HMESSAGEREAD hRead)
{
	*hRead >> m_bEnabled;

	*hRead >> m_SenseSeeEnemy;
	*hRead >> m_SenseSeeEnemyFlashlight;
	*hRead >> m_SenseSeeDeadBody;
	*hRead >> m_SenseSeeBlood;
	*hRead >> m_SenseSeeCloaked;
	*hRead >> m_SenseHearEnemyFootstep;
	*hRead >> m_SenseHearEnemyWeaponFire;
	*hRead >> m_SenseHearAllyWeaponFire;
	*hRead >> m_SenseHearTurretWeaponFire;
	*hRead >> m_SenseHearAllyPain;
	*hRead >> m_SenseHearAllyBattleCry;
	*hRead >> m_SenseHearEnemyTaunt;
	*hRead >> m_SenseHearDeath;
	*hRead >> m_SenseHearWeaponImpact;
}
