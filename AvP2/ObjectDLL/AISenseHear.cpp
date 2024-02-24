// ----------------------------------------------------------------------- //
//
// MODULE  : AISense.cpp
//
// PURPOSE : Sense information implementation
//
// CREATED : 07.23.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AISenseHear.h"
#include "AI.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AIButeMgr.h"
#include "FXButeMgr.h"
#include "CVarTrack.h"
#include "SoundButeMgr.h"
#include "BodyProp.h"

#include <algorithm>

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define SEE_ENEMY_DEBUG
//#define SENSE_DEBUG
//#define HEAR_DEBUG
#endif

#ifndef _FINAL
static bool ShouldShowHear(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

#ifdef HEAR_DEBUG
	static CVarTrack vtShowHear(g_pLTServer,"ShowAIHear",1.0f);
#else
	static CVarTrack vtShowHear(g_pLTServer,"ShowAIHear",0.0f);
#endif

	return (vtShowHear.GetFloat() > 0.0f || ShouldShowAllSenses() ) && ShouldShow(hObject);
}
#endif

void CAISenseHear::SetAttributes(const AIBM_HearSense & butes)
{
	CAISense::SetAttributes(butes);

	m_fMaxNoiseDistanceSqr = butes.fMaxNoiseDistance*butes.fMaxNoiseDistance;
}

void CAISenseHear::StimulateInstance(const SoundProps & sound, CAISenseInstance * pInstance)
{

	ASSERT( pInstance );
	if( !pInstance )
		return;

	// If the sound has a zero volume, we're not supposed to hear it.
	if( sound.fVolume == 0.0f )
		return;

	// Get the distance to the noise.  
	// The noise distance is modified by the sound volume.  
	// A volume of 1.0 means the sound is as far away as it should be.
	// A lower volume will make the sound seem further away.
	// A higher volume will make the sound seem closer.
	// A negative volume means to not scale be distance.
	LTFLOAT fNoiseDistanceSqr = ( sound.vLocation - GetAIPtr()->GetMovement()->GetPos() ).MagSqr();
	
	if( sound.fVolume > 0.0f )
		fNoiseDistanceSqr /= sound.fVolume*sound.fVolume;
	else if( sound.fVolume < 0.0f )
		fNoiseDistanceSqr = 0.0f;

	// Now that we have a modified distance, check to see if the sound
	// is still within range.
	if ( fNoiseDistanceSqr < m_fMaxNoiseDistanceSqr )
	{
		// m_fMaxNoiseDistanceSqr should always be greater than zero!  We're 
		// dividing by it!!
		_ASSERT( m_fMaxNoiseDistanceSqr > 0.0f );

		// Yes, the sqrt is important.  Without it you get a quadratic behaviour which
		// feels funny when you play the game (it feels like they can hear you all of a sudden
		// rather than as you gradually get closer).
		if( m_fMaxNoiseDistanceSqr > 0.0f )
		{
			LTFLOAT fStimulus = 1.0f - (float)sqrt(fNoiseDistanceSqr/m_fMaxNoiseDistanceSqr);
			fStimulus *= GetAlertRateModifier();

			pInstance->Stimulate( fStimulus, sound.vLocation );
		}

#ifndef _FINAL
		if( ShouldShowHear(GetAIPtr()->m_hObject) )
		{
			ASSERT( m_fMaxNoiseDistanceSqr != 0.0f );
			AICPrint(GetAIPtr()->m_hObject, " %s sense for %s stimulation : %f adding : %f",
				GetDescription(),
				g_pServerDE->GetObjectName(pInstance->GetStimulus()),
				pInstance->GetStimulation(), 
				1.0f - sqrt(fNoiseDistanceSqr/m_fMaxNoiseDistanceSqr) );
		}
#endif

	}
	else
	{
#ifndef _FINAL
		if( ShouldShowHear(GetAIPtr()->m_hObject) )
		{
			AICPrint(GetAIPtr()->m_hObject, " %s sense for %s stimulation : %f with no additions.",
				GetDescription(),
				g_pServerDE->GetObjectName(pInstance->GetStimulus()),
				pInstance->GetStimulation() );
		}
#endif
	}
}

void CAISenseHear::Save(HMESSAGEWRITE hWrite)
{
	CAISense::Save(hWrite);

	hWrite->WriteFloat(m_fLastCheckTime - g_pServerDE->GetTime());
	*hWrite << m_fMaxNoiseDistanceSqr;
}

void CAISenseHear::Load(HMESSAGEREAD hRead)
{
	CAISense::Load(hRead);

	m_fLastCheckTime = hRead->ReadFloat() + g_pServerDE->GetTime();
	*hRead >> m_fMaxNoiseDistanceSqr;
}



//--------------------------
// CAISenseHearEnemyFootstep
//--------------------------

void CAISenseHearEnemyFootstep::Footstep(const CharFootstepSoundInfo & info, const CCharacter * pChar)
{
	if( !pChar )
		return;

	CAISenseInstance * pInstance = GetInstance(pChar->m_hObject);

	if( !pInstance )
		return;


	SoundProps sound_prop;
	sound_prop.fVolume = info.fVolume;
	sound_prop.vLocation = info.vLocation;

	StimulateInstance(sound_prop,pInstance);
}

bool CAISenseHearEnemyFootstep::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreEnemies(*pChar,*GetAIPtr());
}

void CAISenseHearEnemyFootstep::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearEnemyFootstepSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);

	CAISenseHear::SetAttributes(butes);
}


//--------------------------
// CAISenseHearWeaponFire
//--------------------------

void CAISenseHearWeaponFire::WeaponFire(const CharFireInfo & info, const CCharacter * pChar)
{
	if( !pChar )
		return;

	CAISenseInstance * pInstance = GetInstance(pChar->m_hObject);

	if( !pInstance )
		return;


	SoundProps sound_prop;

	sound_prop.fVolume = 0.0f;    // Assume shot was fired out of range.
	sound_prop.vLocation = info.vFiredPos;
	
	
	// Check to see if AI was within gun's sound range.
	const BARREL * pBarrel = g_pWeaponMgr->GetBarrel(info.nBarrelId);

	if( pBarrel ) 
	{
		// If the AI is within the weapon's sound radius, set the sound volume to non-zero.
		const LTFLOAT fWeaponFireNoiseDistanceSqr = pBarrel->fFireSoundRadius*pBarrel->fFireSoundRadius;
		if( (GetAIPtr()->GetMovement()->GetPos() - sound_prop.vLocation).MagSqr() < fWeaponFireNoiseDistanceSqr )
		{
			// A negative volume means don't account for distance in determining the 
			// stimulation level.
			sound_prop.fVolume = -1.0f;
		}
	}

	StimulateInstance(sound_prop,pInstance);
}

//--------------------------
// CAISenseHearEnemyWeaponFire
//--------------------------
bool CAISenseHearEnemyWeaponFire::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreEnemies(*pChar,*GetAIPtr());
}

void CAISenseHearEnemyWeaponFire::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearEnemyWeaponFireSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

//--------------------------
// CAISenseHearAllyWeaponFire
//--------------------------
bool CAISenseHearAllyWeaponFire::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreAllies(*pChar,*GetAIPtr());
}

void CAISenseHearAllyWeaponFire::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearAllyWeaponFireSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

//--------------------------
// CAISenseHearTurretWeaponFire
//--------------------------
bool CAISenseHearTurretWeaponFire::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreEnemies(*pChar,*GetAIPtr());
}

void CAISenseHearTurretWeaponFire::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearTurretWeaponFireSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

//--------------------------
// CAISenseHearAllyPain
//--------------------------
bool CAISenseHearAllyPain::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreAllies(*pChar,*GetAIPtr());
}

void CAISenseHearAllyPain::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearAllyPainSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

void CAISenseHearAllyPain::Pain(const SoundButes & info, const CCharacter * pChar)
{
	if( !pChar )
		return;

	CAISenseInstance * pInstance = GetInstance(pChar->m_hObject);

	if( !pInstance )
		return;

	SoundProps sound_prop;

	sound_prop.fVolume = 0.0f;
	sound_prop.vLocation = pChar->GetPosition();
	
	if ((GetAIPtr()->GetMovement()->GetPos() - sound_prop.vLocation).MagSqr() < info.m_fOuterRad*info.m_fOuterRad)
	{
		sound_prop.fVolume = -1.0f;
	}


	StimulateInstance(sound_prop,pInstance);
}

//--------------------------
// CAISenseHearAllyBattleCry
//--------------------------
bool CAISenseHearAllyBattleCry::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreAllies(*pChar,*GetAIPtr());
}

void CAISenseHearAllyBattleCry::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearAllyBattleCrySense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

void CAISenseHearAllyBattleCry::BattleCry(const SoundButes & info, const CCharacter * pChar)
{
	if( !pChar )
		return;

	CAISenseInstance * pInstance = GetInstance(pChar->m_hObject);

	if( !pInstance )
		return;

	SoundProps sound_prop;

	sound_prop.fVolume = 0.0f;
	sound_prop.vLocation = pChar->GetPosition();
	
	if ((GetAIPtr()->GetMovement()->GetPos() - sound_prop.vLocation).MagSqr() < info.m_fOuterRad*info.m_fOuterRad)
	{
		sound_prop.fVolume = -1.0f;
	}


	StimulateInstance(sound_prop,pInstance);
}

//--------------------------
// CAISenseHearEnemyTaunt
//--------------------------
bool CAISenseHearEnemyTaunt::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreEnemies(*pChar,*GetAIPtr());
}

void CAISenseHearEnemyTaunt::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearEnemyTauntSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

void CAISenseHearEnemyTaunt::BattleCry(const SoundButes & info, const CCharacter * pChar)
{
	if( !pChar )
		return;

	CAISenseInstance * pInstance = GetInstance(pChar->m_hObject);

	if( !pInstance )
		return;

	SoundProps sound_prop;

	sound_prop.fVolume = 0.0f;
	sound_prop.vLocation = pChar->GetPosition();
	
	if ((GetAIPtr()->GetMovement()->GetPos() - sound_prop.vLocation).MagSqr() < info.m_fOuterRad*info.m_fOuterRad)
	{
		sound_prop.fVolume = -1.0f;
	}


	StimulateInstance(sound_prop,pInstance);
}

//--------------------------
// CAISenseHearDeath
//--------------------------

void CAISenseHearDeath::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearDeathSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

void CAISenseHearDeath::Death(const SoundButes & info, const BodyProp * pBody)
{
	if( !pBody )
		return;

	SoundProps sound_prop;

	sound_prop.fVolume = 0.0f;
	g_pLTServer->GetObjectPos(pBody->m_hObject,&sound_prop.vLocation);
	
	const LTFLOAT fDistSqr = (GetAIPtr()->GetMovement()->GetPos() - sound_prop.vLocation).MagSqr();
	if ( fDistSqr < info.m_fOuterRad*info.m_fOuterRad && fDistSqr < GetDistanceSqr() )
	{

		sound_prop.fVolume = -1.0f;

		
		// If the instance for the body does not exist (and it likely doesn't,
		// as the death sound gets called during the body set-up), then
		// we need to make sure we add an instance.
		CAISenseInstance & instance = GetInstanceList()[pBody->m_hObject];


		StimulateInstance(sound_prop,&instance);
	}
}

void CAISenseHearDeath::UpdateAllInstances()
{
	LTVector vBodyPos;

	// Remove all instances which point to a bodyprop that no longer exists or is not in the list.
	{for( InstanceList::iterator iter = GetInstanceList().begin();
		  iter != GetInstanceList().end(); ++iter )
	{
		CAISenseInstance  & instance = iter->second;
		if( !instance.GetStimulus().GetHOBJECT() )
		{
			// The prop has been removed by the engine.
			RemoveInstance(iter);
		}
		else
		{
			// The check to see if the prop is in the bodyprop list.
			const BodyProp * pBodyProp = dynamic_cast<BodyProp*>( g_pLTServer->HandleToObject(instance.GetStimulus()) );
			if(    pBodyProp 
				&& g_pCharacterMgr->EndDeadBodies() == std::find(g_pCharacterMgr->BeginDeadBodies(), g_pCharacterMgr->EndDeadBodies(), pBodyProp) )
			{
				RemoveInstance(iter);
			}
		}
	}}

	// Add and remove instances as appropriate (characters moving in and out
	// of range).
	for (CCharacterMgr::DeadBodyIterator iter = g_pCharacterMgr->BeginDeadBodies();
	iter != g_pCharacterMgr->EndDeadBodies(); ++iter)
	{
		const BodyProp * pBodyProp = *iter;
		if (!pBodyProp)
			continue;

		g_pInterface->GetObjectPos(pBodyProp->m_hObject, &vBodyPos);
		LTFLOAT fSeparationSqr = VEC_DISTSQR(GetAIPtr()->GetPosition(), vBodyPos);
		
		const InstanceList::iterator list_location = GetInstanceList().find(pBodyProp->m_hObject);
		
		if (fSeparationSqr < GetDistanceSqr())		
		{
			// Add an instance if a character has entered our detection range
			// and isn't being watched yet.
			if (GetInstanceList().end() == list_location)
			{
				GetInstanceList()[pBodyProp->m_hObject] = CAISenseInstance(pBodyProp->m_hObject, GetAccumRate(), GetDecayRate(), GetPartialLevel());
			}
		}
		else
		{
			// Remove an instance if the character is out of range
			// and the stimulation has dropped to zero.
			if (GetInstanceList().end() != list_location)
			{
				if (list_location->second.GetStimulation() <= 0.0f)
				{
					RemoveInstance(list_location);
				}
			}
		}
	}
	
	// Update all instances. 
	
	LTFLOAT fHighestStimulation = 0.0f;
	
	for (InstanceList::iterator iter2 = GetInstanceList().begin();
	iter2 != GetInstanceList().end(); ++iter2)
	{
		CAISenseInstance  & instance = iter2->second;

		UpdateStimulation(&instance);
		instance.Update();
		
		if (instance.GetStimulation() > fHighestStimulation)
		{
			fHighestStimulation = instance.GetStimulation();
			m_hStimulus = instance.GetStimulus();
			m_vStimulusPos = instance.GetPos();
		}
	}
	
#ifdef SENSE_DEBUG
	AICPrint( m_pAI->m_hObject, "Stimulus : %s at (%.2f,%.2f,%.2f)",
		g_pLTServer->GetObjectName( m_hStimulus ), 
		m_vStimulusPos.x,m_vStimulusPos.y,m_vStimulusPos.z );
#endif
	
}

//--------------------------
// CAISenseHearWeaponImpact
//--------------------------

void CAISenseHearWeaponImpact::WeaponImpact(const CharImpactInfo & info, const CCharacter * pChar)
{
	if( !pChar )
		return;

	CAISenseInstance * pInstance = GetInstance(pChar->m_hObject);

	if( !pInstance )
		return;

	SoundProps sound_prop;

	sound_prop.fVolume = 0.0f;
	sound_prop.vLocation = info.vImpactPos;
	
	// Get the ammo type
	const AMMO * pAmmo = g_pWeaponMgr->GetAmmo(info.nAmmoId);
	
	if( pAmmo )
	{
		LTFLOAT fImpactNoiseDistSqr = pAmmo->pImpactFX?(LTFLOAT)pAmmo->pImpactFX->nAISoundRadius:0.0F;
		fImpactNoiseDistSqr *= fImpactNoiseDistSqr;

		if ((GetAIPtr()->GetMovement()->GetPos() - sound_prop.vLocation).MagSqr() < fImpactNoiseDistSqr)
		{
			sound_prop.fVolume = -1.0f;
		}
	}


	StimulateInstance(sound_prop,pInstance);
}

bool CAISenseHearWeaponImpact::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreEnemies(*pChar,*GetAIPtr());
}

void CAISenseHearWeaponImpact::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_HearWeaponImpactSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	CAISenseHear::SetAttributes(butes);
}

