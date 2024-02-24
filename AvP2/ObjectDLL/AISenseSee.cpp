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
#include "AISenseSee.h"
#include "AI.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AIButeMgr.h"
#include "BodyProp.h"
#include "TrailNode.h"
#include "DebugLineSystem.h"
#include "CVarTrack.h"

#include <algorithm>

extern LTFLOAT fMinCloakingAlpha;  // In Character.cpp
extern LTFLOAT fMaxCloakingAlpha;  // In Character.cpp

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define SEE_ENEMY_DEBUG
//#define SENSE_DEBUG
//#define HEAR_DEBUG
#endif


//----------------------
// CAISenseSee
//----------------------

#ifndef _FINAL
static bool ShouldShowSee(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

#ifdef SEE_DEBUG
	static CVarTrack vtShowSee(g_pLTServer,"ShowAISee",1.0f);
#else
	static CVarTrack vtShowSee(g_pLTServer,"ShowAISee",0.0f);
#endif

	return (vtShowSee.GetFloat() > 0.0f || ShouldShowAllSenses() ) && ShouldShow(hObject);
}
#endif

LTBOOL CAISenseSee::IsVisible(CAISenseInstance * pInstance)
{
	// Instead of looking right at the center of the target, we look
	// at random points along the bounding box.  If an object
	// is partially visible, they will only be seen some of the time.

	if (!pInstance)
		return LTFALSE;

	HOBJECT hStimulus = pInstance->GetStimulus();

	LTVector vDims;
	g_pServerDE->GetObjectDims(hStimulus, &vDims);

	LTFLOAT fX = vDims.x * GetRandom(-1.0f, 1.0f);
	LTFLOAT fY = vDims.y * GetRandom(-1.0f, 1.0f);
	LTFLOAT fZ = vDims.z * GetRandom(-1.0f, 1.0f);

	LTVector vPosition;
	g_pServerDE->GetObjectPos(hStimulus, &vPosition);

	vPosition.x += fX;
	vPosition.y += fY;
	vPosition.z += fZ;

	// Update the point

	LTFLOAT fDistanceSqr;
	LTBOOL bVisible; 
	LTFLOAT fDp;
	LTVector vDir;

	const LTBOOL bUseFOV = LTTRUE;

	if ( GetAIPtr()->CanSeeThrough() )
	{
		bVisible = GetAIPtr()->IsObjectPositionVisibleFromEye( CAI::SeeThroughFilterFn, LTNULL, 
															   hStimulus, vPosition, GetDistanceSqr(), bUseFOV, 
															   &fDistanceSqr, &fDp, &vDir, &m_vPointColor);
	}
	else
	{
		bVisible = GetAIPtr()->IsObjectPositionVisibleFromEye( CAI::SeeFilterFn, LTNULL,
			                                                   hStimulus, vPosition, GetDistanceSqr(), bUseFOV, 
															   &fDistanceSqr, &fDp, &vDir, &m_vPointColor );
	}

	
#ifndef _FINAL
	if( ShouldShowSee(GetAIPtr()->m_hObject) )
	{
		LineSystem::GetSystem(GetAIPtr(),GetDescription())
			<< LineSystem::Clear()
			<< LineSystem::Line(GetAIPtr()->GetEyePosition(),vPosition, bVisible ? Color::Green : Color::Red );

		LineSystem::GetSystem(GetAIPtr(),"EyeForward")
			<< LineSystem::Clear()
			<< LineSystem::Line(GetAIPtr()->GetEyePosition(), 
			                    GetAIPtr()->GetEyePosition() + GetAIPtr()->GetEyeForward()*500.0f );
	}
#endif

	return bVisible;
}

void CAISenseSee::UpdateStimulation(CAISenseInstance * pInstance)
{
	HOBJECT hStimulus;
	LTBOOL bVisible;
	LTVector vPosition;
	LTFLOAT fDistSqr;

	hStimulus = pInstance->GetStimulus();
	g_pServerDE->GetObjectPos(hStimulus, &vPosition);

	bVisible = IsVisible(pInstance);

	if ( bVisible )
	{ 
		LTFLOAT fRateMultiplier = 1.0f;

		// Dimmer light will cause the AI to take longer to see her target.  Any light with a strength (0.0 - 1.0)
		// above m_fLightMax is considered full strength.  
		if( m_fLightMax > 0.0f )
		{
			LTFLOAT light_strength = std::max(m_vPointColor.x, std::max(m_vPointColor.y, m_vPointColor.z) )/255.0f;
			
			// if the spot is illuminated by a flashlight, set max light strength
			for (CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters();
			iter != g_pCharacterMgr->EndCharacters(); ++iter)
			{
				CCharacter *pChar = *iter;
				if (!pChar)
					continue;

				if (pChar->IsFlashlightOn())
				{
					fDistSqr = (pChar->GetFlashlightPos() - pInstance->GetPos()).MagSqr();
					if (fDistSqr < 250.0f * 250.0f)	// see ClientShellDLL\Flashlight.cpp
						light_strength = 1.0f;
				}
			}

			// Enforce 0.0 to 1.0 range
			m_fLightMin = LTCLAMP(m_fLightMin, 0.0f, 1.0f);
			m_fLightMax = LTCLAMP(m_fLightMax, 0.0f, 1.0f);

			// Min must be <= than max
			if (m_fLightMax < m_fLightMin)
				m_fLightMin = m_fLightMax;

			if (light_strength < m_fLightMin)
				light_strength = m_fLightMin;
			else if( light_strength < m_fLightMax )
				light_strength = light_strength / m_fLightMax;  // already verified m_fLightMax > 0 up above
			else
				light_strength = 1.0f;

			
			fRateMultiplier *= light_strength;

#ifndef _FINAL
			if( ShouldShowSee(GetAIPtr()->m_hObject) )
			{
				AICPrint(GetAIPtr()->m_hObject, "Color is (%.2f,%.2f,%.2f) and ceiling is %f yeilding %f",
					m_vPointColor.x, m_vPointColor.y, m_vPointColor.z, 
					m_fLightMin, light_strength );
			}
#endif
		}

		fRateMultiplier *= GetRateMultiplier(hStimulus, vPosition);

		fRateMultiplier *= GetAlertRateModifier();

		if( fRateMultiplier > 0.0f )
			pInstance->Stimulate(fRateMultiplier,vPosition);

#ifndef _FINAL
		if( ShouldShowSee(GetAIPtr()->m_hObject) )
		{
			AICPrint(GetAIPtr()->m_hObject, " %s for %s stimulation : %f adding : %f",
				GetDescription(),
				g_pServerDE->GetObjectName(hStimulus),
				pInstance->GetStimulation(), 
				fRateMultiplier );
		}
#endif
	}
#ifndef _FINAL
	else if( ShouldShowSee(GetAIPtr()->m_hObject) )
	{
			AICPrint(GetAIPtr()->m_hObject, " %s for %s stimulation : %f with no additions.",
				GetDescription(),
				g_pServerDE->GetObjectName(hStimulus),
				pInstance->GetStimulation());
	}
#endif
}

void CAISenseSee::Save(HMESSAGEWRITE hWrite)
{
	CAISense::Save(hWrite);

	*hWrite << m_fLightMin;
	*hWrite << m_fLightMax;
	*hWrite << m_vPointColor;

}

void CAISenseSee::Load(HMESSAGEREAD hRead)
{
	CAISense::Load(hRead);

	*hRead >> m_fLightMin;
	*hRead >> m_fLightMax;
	*hRead >> m_vPointColor;
}

//----------------------
// CAISenseSeeEnemy
//----------------------

bool CAISenseSeeEnemy::IgnoreCharacter(const CCharacter * pChar)
{
	return !g_pCharacterMgr->AreEnemies(*pChar,*GetAIPtr());
}

void CAISenseSeeEnemy::Touched(const CCharacter & character)
{
	if( !IgnoreCharacter(&character) )
	{
		CAISenseInstance * pInstance = GetInstance(character.m_hObject);
		if( pInstance )
		{
			pInstance->FullyStimulate();
		}
	}
}

LTFLOAT CAISenseSeeEnemy::GetRateMultiplier(HOBJECT hStimulus, const LTVector & vStimulus)
{
	LTVector vStimulusDir = (vStimulus - GetAIPtr()->GetEyePosition());
	const LTFLOAT fDistanceSqr = vStimulusDir.MagSqr();
	
	vStimulusDir.Norm();

	// Look at how fInterpolation factor is used below to understand why it is set
	// between 0 and 1.

	// fInterpolationFactor starts out assuming we are at less than the short time distance.
	LTFLOAT fInterpolationFactor = 0.0f;
	if( fDistanceSqr > m_fShortTimeDistanceSqr )
	{
		// We are at greater than the short time distance,
		// determine if we are less than the long time distance.
		if( fDistanceSqr < m_fLongTimeDistanceSqr )
		{
			// We are between short time distance and long time distance.
			// Interpolate between the two times.
			fInterpolationFactor = (fDistanceSqr - m_fShortTimeDistanceSqr)/(m_fLongTimeDistanceSqr - m_fShortTimeDistanceSqr);
		}
		else
		{
			// We are above the long time distance, so just use the long time.
			fInterpolationFactor = 1.0f;
		}
	}

	_ASSERT( fInterpolationFactor >= 0.0f && fInterpolationFactor <= 1.0f );
	
	// The rate multiplier needs to be set to 1/(reaction time).  Reaction time is between the short time and long
	// time.  fInterpolationFactor decides where between the short distance and long distance we are (in above code).
	// If fInterpolationFactor is 0.0f, the rate multiplier will be 1.0f.  
	// If fInterpolationFactor is 1.0f, the rate multiplier will be (short time)/(long time).  Because the 
	//	the rate is set to 1/(short time), a multiplier of (short time)/(long time) will change it to 1/(long time).
	const LTFLOAT fInverseRateMultiplier = (1.0f - fInterpolationFactor) + fInterpolationFactor*m_fLongDivShortTime;
	LTFLOAT fRateMultiplier = fInverseRateMultiplier > 0.0f ? 1.0f/fInverseRateMultiplier : 0.0f;

	// Calculate angular effects (greater angle means slower stimulation rate).

	if( m_fCosFOV < m_fCosStartFOV )
	{
		const LTFLOAT fDirDotForward = vStimulusDir.Dot(GetAIPtr()->GetEyeForward());
		if(  fDirDotForward < m_fCosStartFOV )
		{
			if( fDirDotForward <= m_fCosFOV )
				return 0.0f;

			fRateMultiplier *= (fDirDotForward - m_fCosFOV) / (m_fCosStartFOV - m_fCosFOV);
		}
	}	

	// The vertical angle is the reverse of the FOV angle, so it needs
	// to use the sine of the angle.  It really does need to use sine, 
	// it doesn't feel right to use the cosine.  Using the sine
	// makes it work very similar to the FOV.
	if( m_fSinVerticalBlind < m_fSinStartVerticalBlind )
	{
		const LTFLOAT fDirDotUp = vStimulusDir.Dot(GetAIPtr()->GetAimUp());

		if( fDirDotUp > 0.0f )
		{
			// This determines the sine of the angle using sin^2 + cos^2 = 1 .....ain't trig great?
			LTFLOAT fSinOfDirAndUp = -1.0f;
			if( fDirDotUp*fDirDotUp < 1.0f )
			{
				fSinOfDirAndUp = (LTFLOAT)sqrt(1.0f - fDirDotUp*fDirDotUp);
			}
			else
			{
				fSinOfDirAndUp = 0.0f;
			}


			if( fSinOfDirAndUp < m_fSinStartVerticalBlind )
			{
				if( fSinOfDirAndUp <= m_fSinVerticalBlind )
					return 0.0f;

				fRateMultiplier *= (fDirDotUp - m_fSinStartVerticalBlind) / (m_fSinVerticalBlind - m_fSinStartVerticalBlind);
			}
		}
	}

	// Handle cloaked targets.

	// Reduce visibility ranges vs. cloaked Predators
	LTFLOAT r_unused, g_unused, b_unused, a;
	a = 1.0f;

	if( hStimulus )
		g_pInterface->GetObjectColor(hStimulus, &r_unused, &g_unused, &b_unused, &a);

	if( m_fCloakSensitivity > 0.0f && a < 1.0f )
	{
		if( fMaxCloakingAlpha > fMinCloakingAlpha )
		{
			LTFLOAT scaled_a = (a - fMinCloakingAlpha)/(fMaxCloakingAlpha - fMinCloakingAlpha);

			// Incorporate the alpha value of the target.
			fRateMultiplier *= Clamp(scaled_a* m_fCloakSensitivity, m_fMinAlphaCloakAffect, m_fMaxAlphaCloakAffect);
		}
	}

	// Add a distance fall off as well so that the AI's won't see a "distant" cloaked target.
	if( m_fCloakFalloffMinDist > 0.0f && a < 1.0f )
	{
		const LTFLOAT fDifference = LTFLOAT(sqrt(fDistanceSqr)) - m_fCloakFalloffMinDist;
		if( fDifference > -0.001f )
		{
			if( fDifference > m_fCloakFalloffRange )
			{
				fRateMultiplier = 0.0f;
			}
			else
			{
				fRateMultiplier *= ( 1.0f - (fDifference/m_fCloakFalloffRange) );
			}
		}
	}

	return fRateMultiplier;
}

void CAISenseSeeEnemy::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_SeeEnemySense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);
	
	CAISense::SetAttributes(butes);

	m_fShortTimeDistanceSqr = butes.fShortTimeDistance*butes.fShortTimeDistance;
	m_fLongTimeDistanceSqr = butes.fLongTimeDistance*butes.fLongTimeDistance;
	m_fLightMin = butes.fLightMin;
	m_fLightMax = butes.fLightMax;

	if( butes.fIncreaseStimulationTime != 0.0f )
		m_fLongDivShortTime = butes.fLongIncreaseStimulationTime/butes.fIncreaseStimulationTime;

	m_fCloakSensitivity = butes.fCloakSensitivity;
	m_fCloakFalloffMinDist = butes.fCloakFalloffMinDist;
	m_fCloakFalloffRange = butes.fCloakFalloffMaxDist - butes.fCloakFalloffMinDist;
	if( m_fCloakFalloffRange < 0.0f ) 
		m_fCloakFalloffRange = 0.0f;

	m_fMinAlphaCloakAffect = butes.fCloakMinAlphaAffect;
	m_fMaxAlphaCloakAffect = butes.fCloakMaxAlphaAffect;

	m_fCosFOV = (float)cos( butes.fFOV*0.5f );
	m_fCosStartFOV = (float)cos( butes.fStartFOV*0.5f );
	
	m_fSinVerticalBlind = (float)sin( butes.fVerticalBlind*0.5f );
	m_fSinStartVerticalBlind = (float)sin( butes.fStartVerticalBlind*0.5f );
}


void CAISenseSeeEnemy::Save(HMESSAGEWRITE hWrite)
{
	CAISense::Save(hWrite);

	*hWrite << m_fShortTimeDistanceSqr;
	*hWrite << m_fLongTimeDistanceSqr;
	*hWrite << m_fLongDivShortTime;

	*hWrite << m_fCloakSensitivity;
	*hWrite << m_fCloakFalloffMinDist;
	*hWrite << m_fCloakFalloffRange;
	*hWrite << m_fMinAlphaCloakAffect;
	*hWrite << m_fMaxAlphaCloakAffect;


	*hWrite << m_fCosFOV;
	*hWrite << m_fCosStartFOV;
	
	*hWrite << m_fSinVerticalBlind;
	*hWrite << m_fSinStartVerticalBlind;
}

void CAISenseSeeEnemy::Load(HMESSAGEREAD hRead)
{
	CAISense::Load(hRead);

	*hRead >> m_fShortTimeDistanceSqr;
	*hRead >> m_fLongTimeDistanceSqr;
	*hRead >> m_fLongDivShortTime;

	*hRead >> m_fCloakSensitivity;
	*hRead >> m_fCloakFalloffMinDist;
	*hRead >> m_fCloakFalloffRange;
	*hRead >> m_fMinAlphaCloakAffect;
	*hRead >> m_fMaxAlphaCloakAffect;

	*hRead >> m_fCosFOV;
	*hRead >> m_fCosStartFOV;
	
	*hRead >> m_fSinVerticalBlind;
	*hRead >> m_fSinStartVerticalBlind;
}

//----------------------
// CAISenseSeeDeadBody
//----------------------

void CAISenseSeeDeadBody::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_SeeDeadBodySense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);

	CAISenseSeeDeadBody::SetAttributes(butes);
}

void CAISenseSeeDeadBody::UpdateAllInstances()
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

		//		if( pCharacter == GetAIPtr() )
		//			continue;
		
		//		if( IgnoreCharacter( *iter ) )
		//			continue;
		
		// Add code to determine whether or not this was a friend
		
		
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

//----------------------
// CAISenseSeeBlood
//----------------------

void CAISenseSeeBlood::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_SeeBloodSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);

	CAISenseSeeBlood::SetAttributes(butes);
}

void CAISenseSeeBlood::UpdateAllInstances()
{
	LTVector vBloodPos;

	// Remove all instances which point to an object that has been removed.
	{for( InstanceList::iterator iter = GetInstanceList().begin();
		  iter != GetInstanceList().end(); ++iter )
	{
		CAISenseInstance  & instance = iter->second;
		if( !instance.GetStimulus().GetHOBJECT() )
		{
			RemoveInstance(iter);
		}
	}}

	// Add and remove instances as appropriate (characters moving in and out
	// of range).
	for (CCharacterMgr::TrailNodeIterator iter = g_pCharacterMgr->BeginTrailNodes();
	iter != g_pCharacterMgr->EndTrailNodes(); ++iter)
	{
		const TrailNode * pTrailNode = *iter;
		
		//		if( pCharacter == GetAIPtr() )
		//			continue;
		
		//		if( IgnoreCharacter( *iter ) )
		//			continue;
		
		// Add code to determine whether or not this was a friend
		
		
		g_pInterface->GetObjectPos(pTrailNode->m_hObject, &vBloodPos);
		LTFLOAT fSeparationSqr = VEC_DISTSQR(GetAIPtr()->GetPosition(), vBloodPos);
		
		const InstanceList::iterator list_location = GetInstanceList().find(pTrailNode->m_hObject);
		
		if (fSeparationSqr < GetDistanceSqr())		
		{
			// Add an instance if a character has entered our detection range
			// and isn't being watched yet.
			if (GetInstanceList().end() == list_location)
			{
				GetInstanceList()[pTrailNode->m_hObject] = CAISenseInstance(pTrailNode->m_hObject, GetAccumRate(), GetDecayRate(), GetPartialLevel());
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

//---------------------------
// CAISenseSeeEnemyFlashlight
//---------------------------

void CAISenseSeeEnemyFlashlight::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_SeeEnemyFlashlightSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);

	CAISenseSeeEnemyFlashlight::SetAttributes(butes);
}

bool CAISenseSeeEnemyFlashlight::IgnoreCharacter(const CCharacter *pChar)
{
	return !g_pCharacterMgr->AreEnemies(*pChar,*GetAIPtr());
}

LTBOOL CAISenseSeeEnemyFlashlight::IsVisible(CAISenseInstance *pInstance)
{
	CCharacter *pChar = NULL;
	LTVector vAIPos, vLightPos;
	LTFLOAT fRadiusSqr;

	// TODO: remove this and bute -- this is just for testing.
	fRadiusSqr = 10.0f * 10.0f;

	if (!m_tmrIntersect.Stopped())
	{
		if (m_bWasVisible)
			return LTTRUE;
		else
			return LTFALSE;
	}

	if (pInstance)
	{
		pChar = dynamic_cast<CCharacter *>(g_pInterface->HandleToObject(pInstance->GetStimulus()));
		if (pChar)
		{
			if (pChar->IsFlashlightOn())
			{
				m_tmrIntersect.Start(1.0f);

				vAIPos = GetAIPtr()->GetPosition();
				vLightPos = pChar->GetFlashlightPos();

				IntersectQuery iQuery;
				IntersectInfo iInfo;
			
				iQuery.m_From	= vAIPos;
				iQuery.m_To		= vLightPos;
			
				iQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;
			
				++g_cIntersectSegmentCalls;
				if (g_pServerDE->IntersectSegment(&iQuery, &iInfo))
				{
					LTFLOAT fDistSqr = VEC_DISTSQR(iInfo.m_Point, vLightPos);
					if (fDistSqr <= fRadiusSqr)
					{
						m_bWasVisible = LTTRUE;
						return LTTRUE;
					}
					else
					{
						m_bWasVisible = LTFALSE;
					}
				}
			}


			vAIPos = GetAIPtr()->GetPosition();

			// iterate through active flares
			for (CCharacter::FlareIterator iter = pChar->BeginFlares();
				 iter != pChar->EndFlares(); ++iter)
			{
				const CFlare *pFlare = *iter;
				if (!pFlare)
					continue;

				g_pLTServer->GetObjectPos(pFlare->m_hObject, &vLightPos);

				IntersectQuery iQuery;
				IntersectInfo iInfo;
			
				iQuery.m_From	= vAIPos;
				iQuery.m_To		= vLightPos;
			
				iQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;
			
				++g_cIntersectSegmentCalls;
				if (g_pServerDE->IntersectSegment(&iQuery, &iInfo))
				{
					m_bWasVisible = LTTRUE;
					return LTTRUE;
				}
			}
		}
	}

	return LTFALSE;
//	return CAISenseSee::IsVisible(pInstance);
}


//----------------------
// CAISenseSeeCloaked
//----------------------

bool CAISenseSeeCloaked::IgnoreCharacter(const CCharacter * pChar)
{
	// Ignore everything except cloaked enemies
	return ((pChar->GetCloakState() == CLOAK_STATE_INACTIVE) || 
		(!g_pCharacterMgr->AreEnemies(*pChar, *GetAIPtr())));
}

void CAISenseSeeCloaked::GetAttributes(int nTemplateID)
{
	ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return;

	const AIBM_SeeCloakedSense & butes = g_pAIButeMgr->GetSense(nTemplateID,this);

	CAISenseSeeCloaked::SetAttributes(butes);
}

void CAISenseSeeCloaked::UpdateAllInstances()
{
	// If this AI is a Predator or Alien, then don't bother using this sense,
	// since their vision is unaffected by cloaking.
	if ((GetAIPtr()->GetCharacterClass() == PREDATOR) ||
		(GetAIPtr()->GetCharacterClass() == ALIEN))
		return;

	CAISenseSee::UpdateAllInstances();
}
