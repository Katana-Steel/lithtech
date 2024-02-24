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
#include "AISense.h"
#include "AI.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AIButeMgr.h"
#include "CVarTrack.h"

#include <algorithm>

#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define SEE_ENEMY_DEBUG
//#define SENSE_DEBUG
//#define HEAR_DEBUG
#endif


#ifndef _FINAL
bool ShouldShowAllSenses()
{
	ASSERT( g_pLTServer );

	static CVarTrack vtShowSenses(g_pLTServer,"ShowSenses",0.0f);

	return vtShowSenses.GetFloat() > 0.0f;

}
#endif

CAISenseInstance::CAISenseInstance( const LTSmartLink & hStimulus /* = LTNULL */,
									LTFLOAT fAccumRate /* = 0.0f */, 
									LTFLOAT fDecayRate /* = 0.0f */,
									LTFLOAT fPartialLevel /* = 0.0f */ )
	: m_hStimulus(hStimulus),
	  m_fAccumRate(fAccumRate),
	  m_fDecayRate(fDecayRate),
	  m_fPartialLevel(fPartialLevel),

	  m_fStimulation(0.0f),
	  m_fLastStimulation(0.0f),
	  m_fStimulationAccum(0.0f),
	  m_bFullyStimulate(false),
	  m_fLastUpdateTime(g_pLTServer ? g_pLTServer->GetTime() : 0.0f),
	  m_vPos(0,0,0)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseInstance::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAISenseInstance::Save(LMessage * pMsg)
{
	if (!pMsg) return;

	*pMsg << m_hStimulus;
	*pMsg << m_fAccumRate;
	*pMsg << m_fDecayRate;
	*pMsg << m_fPartialLevel;

	*pMsg << m_fStimulation;
	*pMsg << m_fLastStimulation;
	*pMsg << m_fStimulationAccum;
	*pMsg << m_bFullyStimulate;

	pMsg->WriteFloat(m_fLastUpdateTime - g_pLTServer->GetTime() );

	*pMsg << m_vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAISenseInstance::Load(LMessage * pMsg)
{
	if (!pMsg) return;

	*pMsg >> m_hStimulus;
	*pMsg >> m_fAccumRate;
	*pMsg >> m_fDecayRate;
	*pMsg >> m_fPartialLevel;

	*pMsg >> m_fStimulation;
	*pMsg >> m_fLastStimulation;
	*pMsg >> m_fStimulationAccum;
	*pMsg >> m_bFullyStimulate;

	m_fLastUpdateTime = pMsg->ReadFloat() + g_pLTServer->GetTime();

	*pMsg >> m_vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AgeStimulation
//
//	PURPOSE:	Manage stimulation level (add accumulations and decay).
//
// ----------------------------------------------------------------------- //

static inline LTFLOAT AgeStimulation( LTFLOAT fCurrentStimulation, LTFLOAT fAddedStimulations,  
									  LTFLOAT fTimeElapsed,
									  LTFLOAT fAccumRate,        LTFLOAT fDecayRate )
{
	_ASSERT( fAddedStimulations >= 0.0f );

	// Add the new stimulations.

	if( fAddedStimulations > 0.0f )
	{
		if( fAccumRate >= 0.0f )
			fCurrentStimulation += fAccumRate*fAddedStimulations*fTimeElapsed;
		else
			fCurrentStimulation += fAddedStimulations;
	}
	else
	{
		if( fDecayRate >= 0.0f )
			fCurrentStimulation -= fDecayRate*fTimeElapsed;
		else
			fCurrentStimulation = 0.0f;
	}

	// Cap the stimulation to between 0 and 1.
	if( fCurrentStimulation < 0.0f ) fCurrentStimulation = 0.0f;
	if( fCurrentStimulation > 1.0f ) fCurrentStimulation = 1.0f;

	return fCurrentStimulation;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseInstance::Update
//
//	PURPOSE:	Update the sense level.
//
// ----------------------------------------------------------------------- //

void CAISenseInstance::Update()
{
	const LTFLOAT fTime = g_pLTServer->GetTime();

	m_fLastStimulation = m_fStimulation;

	if( !m_bFullyStimulate )
	{
		m_fStimulation = AgeStimulation( m_fStimulation,
			                             m_fStimulationAccum,
										 fTime - m_fLastUpdateTime,
										 m_fAccumRate, m_fDecayRate);
#ifdef SENSE_DEBUG
//		g_pLTServer->CPrint( "Simulation : %f, Accum : %f, TimeElapsed : %f, AccumRate : %f, DecayRate : %f ",
//							 m_fStimulation, m_fStimulationAccum, fTimeElapsed, m_fAccumRate, m_fDecayRate);
#endif

	}
	else
	{
		m_fStimulation = 1.0f; 

#ifdef SENSE_DEBUG
//		g_pLTServer->CPrint("Fully stimulated.");
#endif
	}

	m_fStimulationAccum = 0.0f;
	m_fLastUpdateTime = fTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::~CAISense
//
//	PURPOSE:	Cleanup
//
// ----------------------------------------------------------------------- //
CAISense::~CAISense()
{
	if (m_hstr1stTriggerObject)
		FREE_HSTRING(m_hstr1stTriggerObject);

	if (m_hstr1stTriggerMessage)
		FREE_HSTRING(m_hstr1stTriggerMessage);

	if (m_hstrTriggerObject)
		FREE_HSTRING(m_hstrTriggerObject);

	if (m_hstrTriggerMessage)
		FREE_HSTRING(m_hstrTriggerMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Init
//
//	PURPOSE:	Init the sense
//
// ----------------------------------------------------------------------- //

void CAISense::Init(const CAI* pAI)
{
	m_pAI = pAI;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::SetAttributes
//
//	PURPOSE:	Sets base sense attributes
//
// ----------------------------------------------------------------------- //

void CAISense::SetAttributes(const AIBM_Sense & sense_data)
{
	m_fDistanceSqr = sense_data.fRange*sense_data.fRange;
	m_fAccumRate = (sense_data.fIncreaseStimulationTime != 0.0f) ? 
		  1.0f / sense_data.fIncreaseStimulationTime 
		: 0.0f;
	m_fDecayRate = (sense_data.fDecreaseStimulationTime != 0.0f) ? 
		  1.0f / sense_data.fDecreaseStimulationTime
		: 0.0f;
	m_fPartialLevel = sense_data.fPartialLevel;

	m_fAlertRateModifier = sense_data.fAlertRateModifier;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::RemoveInstance
//
//	PURPOSE:	Use this to safely remove an instance
//
// ----------------------------------------------------------------------- //
void CAISense::RemoveInstance(InstanceList::iterator list_location)
{
	if( list_location == m_Instances.end() )
		return;

	if( list_location->first == m_hStimulus )
	{
		m_hStimulus =  LTNULL;
		m_vStimulusPos = LTVector(0,0,0);
	}

	m_Instances.erase(list_location);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::GetInstance
//
//	PURPOSE:	If an instance for the named stimulus exists, a pointer to it will be returned.
//				Returns null if it does not exist.
//
// ----------------------------------------------------------------------- //

CAISenseInstance * CAISense::GetInstance(HOBJECT hStimulus)
{
	const InstanceList::iterator list_location = m_Instances.find(hStimulus);

	if( list_location == m_Instances.end() )
		return LTNULL;

	return &list_location->second;

}

const CAISenseInstance * CAISense::GetInstance(HOBJECT hStimulus) const
{
	const InstanceList::const_iterator list_location = m_Instances.find(hStimulus);

	if( list_location == m_Instances.end() )
		return LTNULL;

	return &list_location->second;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Update
//
//	PURPOSE:	Add, remove, and update the sense instances.
//
// ----------------------------------------------------------------------- //

void CAISense::Update()
{
	if( !m_bPropEnabled )
		return;

	if( m_bEnabled )
	{
		m_bDisabled = false;

		// After the first stimulation, this will be set to null.
		if( m_bWaitingFor1stTrigger && m_hstr1stTriggerMessage )
		{
			if (GetStimulation() >= 1.0f)
			{
				m_bWaitingFor1stTrigger = LTFALSE;
				SendTriggerMsgToObjects(const_cast<CAI *>(GetAIPtr()), m_hstr1stTriggerObject, m_hstr1stTriggerMessage);
				
#ifdef SENSE_DEBUG
				AICPrint(m_hObject, "First full stimulation");
#endif
			}
		}
		else if( m_hstrTriggerMessage )
		{
			if (GetStimulation() >= 1.0f)
			{
				SendTriggerMsgToObjects(const_cast<CAI *>(GetAIPtr()), m_hstrTriggerObject, m_hstrTriggerMessage);
			}
		}

		UpdateAllInstances();

#ifdef SENSE_DEBUG
		AICPrint( m_pAI->m_hObject, "Stimulus : %s at (%.2f,%.2f,%.2f)",
							 g_pLTServer->GetObjectName( m_hStimulus ), 
							 m_vStimulusPos.x,m_vStimulusPos.y,m_vStimulusPos.z );
#endif

	}
	else if( !m_bDisabled )
	{
		m_bDisabled = true;
		m_Instances.clear();
	}

} //void CAISense::Update()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::GetStimulation
//
//	PURPOSE:	Returns the stimulation level of a stimulus.
//
// ----------------------------------------------------------------------- //
LTFLOAT CAISense::GetStimulation() const
{
	if( !m_bPropEnabled )
		return 0.0f;

	m_bEnabled = true;
	
	if( m_hStimulus )
		return GetStimulation(m_hStimulus);

	return 0.0f;
}

LTFLOAT CAISense::GetStimulation(HOBJECT hStimulus) const
{
	if( !m_bPropEnabled )
		return 0.0f;

	m_bEnabled = true;

	const InstanceList::const_iterator list_location = m_Instances.find(hStimulus);

	if( list_location == m_Instances.end() )
		return 0.0f;

	return list_location->second.GetStimulation();
}
		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::GetProperties
//
//	PURPOSE:	Gets attributes for the sense out of the butes file
//
// ----------------------------------------------------------------------- //

void CAISense::GetProperties(GenericProp* pgp)
{

	if ( g_pLTServer->GetPropGeneric((char*)GetPropertiesEnabledString(), pgp ) == LT_OK )
	{
		if ( pgp->m_String[0] )
			m_bPropEnabled = (pgp->m_Bool == LTTRUE);
	}

	if ( g_pLTServer->GetPropGeneric((char*)GetPropertiesDistanceString(), pgp ) == LT_OK )
	{
		if ( pgp->m_String[0] )
		{
			m_fDistanceSqr = pgp->m_Float*pgp->m_Float;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::UpdateAllInstances
//
//	PURPOSE:	This is set up to allow other senses to override the 
//				instance management.
//
// ----------------------------------------------------------------------- //

void CAISense::UpdateAllInstances()
{
		// Remove all instances which point to an object that has been removed by the engine
		// or is non-visible (was setting AI's "active 0" does this).
		{for( InstanceList::iterator iter = m_Instances.begin();
			  iter != m_Instances.end(); ++iter )
		{
			CAISenseInstance  & instance = iter->second;
			if( !instance.GetStimulus().GetHOBJECT() || !(FLAG_VISIBLE & g_pLTServer->GetObjectFlags( iter->first )) )
			{
				RemoveInstance(iter);
			}
		}}

		// Add and remove instances as appropriate (characters moving in and out
		// of range).

		{for( CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters();
			  iter != g_pCharacterMgr->EndCharacters(); ++iter )
		{
			const CCharacter * pCharacter = *iter;
			const InstanceList::iterator list_location = m_Instances.find(pCharacter->m_hObject);

			if(    pCharacter == GetAIPtr() 
				|| IgnoreCharacter( *iter )
			    || pCharacter->IsDead()
				|| (g_pLTServer->GetObjectState(pCharacter->m_hObject) & OBJSTATE_INACTIVE) )
			{
				// Be sure to remove the instance if it exists.
				// This needs to be done because character's can change
				// alliance (hence IgnoreCharacter can change) and
				// die will an instance for them exists.
				if( m_Instances.end() !=  list_location )
				{
					RemoveInstance(list_location);
				}
			}
			else
			{
				// Otherwise, we have a potential instance.
				const LTFLOAT fSeparationSqr  = (GetAIPtr()->GetPosition() - pCharacter->GetPosition()).MagSqr();
				
				if(   fSeparationSqr < m_fDistanceSqr )
				{
					// Add an instance if a character has entered our detection range
					// and isn't being watched yet.
					if( m_Instances.end() ==  list_location )
					{
						m_Instances[pCharacter->m_hObject] = CAISenseInstance( pCharacter->m_hObject, m_fAccumRate, m_fDecayRate, m_fPartialLevel );
					}
				}
				else
				{
					// Remove an instance if the character is out of range
					// and the stimulation has dropped to zero.
					if( m_Instances.end() != list_location )
					{
						if( list_location->second.GetStimulation() <= 0.0f )
						{
							RemoveInstance(list_location);
						}
					}
				}
			}
			
		}} //{for(CharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters()...}

		// Update all instances. 

		LTFLOAT fHighestStimulation = 0.0f;

		{for( InstanceList::iterator iter = m_Instances.begin();
			  iter != m_Instances.end(); ++iter )
		{
			CAISenseInstance  & instance = iter->second;

			UpdateStimulation(&instance);
			instance.Update();

			if( instance.GetStimulation() > fHighestStimulation )
			{
				fHighestStimulation = instance.GetStimulation();

				m_hStimulus = instance.GetStimulus();
				m_vStimulusPos = instance.GetPos();
			}
		}}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Save
//
//	PURPOSE:	Save the sense
//
// ----------------------------------------------------------------------- //

void CAISense::Save(HMESSAGEWRITE hWrite)
{
	hWrite->WriteDWord(m_Instances.size());
	for( InstanceList::iterator iter = m_Instances.begin();
	     iter != m_Instances.end(); ++iter )
	{
		hWrite->WriteObject(iter->first);
		*hWrite << iter->second;
	}

		
	*hWrite << m_bPropEnabled;
	*hWrite << m_bEnabled;
	*hWrite << m_bDisabled;
	*hWrite << m_fAccumRate;
	*hWrite << m_fDecayRate;
	*hWrite << m_fPartialLevel;
	*hWrite << m_fAlertRateModifier;
	*hWrite << m_fDistanceSqr;

	*hWrite << m_hStimulus;
	*hWrite << m_vStimulusPos;

	*hWrite << m_bWaitingFor1stTrigger;
	g_pLTServer->WriteToMessageHString(hWrite, m_hstr1stTriggerObject);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstr1stTriggerMessage);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrTriggerObject);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrTriggerMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISense::Load
//
//	PURPOSE:	Restore the sense
//
// ----------------------------------------------------------------------- //

void CAISense::Load(HMESSAGEREAD hRead)
{
	m_Instances.clear();

	const int num_instances = hRead->ReadDWord();
	for( int i = 0; i < num_instances; ++i )
	{
		const HOBJECT hObj = hRead->ReadObject();
		*hRead >> m_Instances[hObj];
	}

	
	*hRead >> m_bPropEnabled;
	*hRead >> m_bEnabled;
	*hRead >> m_bDisabled;
	*hRead >> m_fAccumRate;
	*hRead >> m_fDecayRate;
	*hRead >> m_fPartialLevel;
	*hRead >> m_fAlertRateModifier;
	*hRead >> m_fDistanceSqr;

	*hRead >> m_hStimulus;
	*hRead >> m_vStimulusPos;

	*hRead >> m_bWaitingFor1stTrigger;
	m_hstr1stTriggerObject = g_pLTServer->ReadFromMessageHString(hRead);
	m_hstr1stTriggerMessage = g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrTriggerObject = g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrTriggerMessage = g_pLTServer->ReadFromMessageHString(hRead);
}




