// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAnimation.cpp
//
// PURPOSE : General character animation
//
// CREATED : 8/5/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterAnimation.h"
#include "CharacterFuncs.h"


// ----------------------------------------------------------------------- //

#ifndef _CLIENTBUILD

	#include "CVarTrack.h"
	static CVarTrack g_vtCADebug;
	static CVarTrack g_vtCAError;
#endif

// ----------------------------------------------------------------------- //


#define TRACKERINFO_STATE_INITIALINTERP	0x01
#define TRACKERINFO_STATE_WAITEND		0x02
#define TRACKERINFO_STATE_WAITBREAK		0x04
#define TRACKERINFO_STATE_RANDOMANIM	0x08


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::CharacterAnimation()
//
// PURPOSE:		Set everything to default values
//
// ----------------------------------------------------------------------- //

CharacterAnimation::CharacterAnimation()
{
	m_pInterface = LTNULL;
	m_pModel = LTNULL;

	m_hObject = LTNULL;
	m_nMessageID = -1;

	m_bEnabled = LTTRUE;

	m_pLayers = LTNULL;

	m_fBaseSpeed = 0.0f;

	m_nLastTrackerSet = ANIMATION_BUTE_MGR_INVALID;
	m_nTrackerSet = ANIMATION_BUTE_MGR_INVALID;

	m_bWaiting = LTFALSE;

	m_nAltAnim = INVALID_MODEL_ANIM;
	m_nAltTrackerSet = ANIMATION_BUTE_MGR_INVALID;
	m_bUsingAltAnim = LTFALSE;

	m_nTransTrackerSet = ANIMATION_BUTE_MGR_INVALID;
	m_bUsingTransAnim = LTFALSE;

	m_bFirstSetup = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::~CharacterAnimation()
//
// PURPOSE:		Terminate everything if we didn't call it already
//
// ----------------------------------------------------------------------- //

CharacterAnimation::~CharacterAnimation()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::Init()
//
// PURPOSE:		Setup the main interfaces and pointers
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::Init(INTERFACE *pInterface, HOBJECT hObj, uint32 nMessageID, uint8 nValidTrackers)
{
	if(!pInterface || !hObj) return LTFALSE;

	m_pInterface = pInterface;
	m_pModel = m_pInterface->GetModelLT();

	m_hObject = hObj;
	m_nMessageID = nMessageID;


#ifndef _CLIENTBUILD
	// Init our console variable
	if(!g_vtCADebug.IsInitted())
		g_vtCADebug.Init(m_pInterface, "CADebug", LTNULL, 0.0f);

	if(!g_vtCAError.IsInitted())
		g_vtCAError.Init(m_pInterface, "CAError", LTNULL, 0.0f);
#endif


	// Init some variables in case we're reinitting
	ClearLayers();

	m_nLastTrackerSet = ANIMATION_BUTE_MGR_INVALID;
	m_nTrackerSet = ANIMATION_BUTE_MGR_INVALID;

	m_nAltAnim = INVALID_MODEL_ANIM;
	m_nAltTrackerSet = ANIMATION_BUTE_MGR_INVALID;
	m_bUsingAltAnim = LTFALSE;

	m_nTransTrackerSet = ANIMATION_BUTE_MGR_INVALID;
	m_bUsingTransAnim = LTFALSE;
	m_bFirstSetup = LTFALSE;

	// Make sure all our interfaces got setup properly
	if(!m_pModel || !m_hObject)
		return LTFALSE;


	LTBOOL bFirstTracker = LTTRUE;

	// Create all the trackers for this object
	for(int i = 0; i < ANIM_TRACKER_MAX; i++)
	{
		if(nValidTrackers & (1 << i))
		{
			if(bFirstTracker)
			{
				// Get access to the main tracker
				if(!m_pTrackers[i].m_pTracker)
				{
					m_pModel->GetMainTracker(m_hObject, m_pTrackers[i].m_pTracker);
					m_pTrackers[i].m_pTracker->m_Index = i;
				}

				// Set-up the destination info so everything looks okay.
				m_pModel->GetWeightSet(m_pTrackers[i].m_pTracker, m_pTrackers[i].m_hDestAnim);
				m_pModel->GetCurAnim(m_pTrackers[i].m_pTracker, m_pTrackers[i].m_hAnim);

				bFirstTracker = LTFALSE;
			}
			else
			{
				// Add in a new tracker
				if(!m_pTrackers[i].m_pTracker)
				{
					m_pModel->AddTracker(m_hObject, &m_pTrackers[i].m_Tracker);
					m_pTrackers[i].m_pTracker = &m_pTrackers[i].m_Tracker;
					m_pTrackers[i].m_pTracker->m_Index = i;
				}

				m_pModel->SetLooping(m_pTrackers[i].m_pTracker, LTFALSE);
				m_pModel->SetPlaying(m_pTrackers[i].m_pTracker, LTFALSE);

				HMODELWEIGHTSET hWeights = GetWeightSet("Empty");
				m_pModel->SetWeightSet(m_pTrackers[i].m_pTracker, hWeights);

				m_pModel->SetCurAnim(m_pTrackers[i].m_pTracker, 0);
				m_pModel->ResetAnim(m_pTrackers[i].m_pTracker);

				// Set-up the destination info so everything looks okay.
				m_pTrackers[i].m_hDestAnim = 0;
				m_pTrackers[i].m_hDestWeightSet = hWeights;
			}
		}
	}

	if(m_pTrackers[ANIM_TRACKER_MOVEMENT].m_pTracker)
		m_pModel->SetNormalize(m_pTrackers[ANIM_TRACKER_MOVEMENT].m_pTracker, LTTRUE);


	// Initialize any tracker data we need to
	m_pTrackers[ANIM_TRACKER_RECOIL].m_nIndex = -1;


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::Term()
//
// PURPOSE:		Reset everything so we don't accicentally call something we
//				weren't suppose to
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::Term()
{
	// Remove all the trackers for the object
	if(m_hObject && m_pModel)
	{
		for(int i = 1; i < ANIM_TRACKER_MAX; i++)
		{
			if(m_pTrackers[i].m_pTracker)
			{
				m_pModel->RemoveTracker(m_hObject, &m_pTrackers[i].m_Tracker);
				m_pTrackers[i].m_pTracker = LTNULL;
			}
		}
	}

	m_pInterface = LTNULL;
	m_pModel = LTNULL;
	m_hObject = LTNULL;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::Save()
//
// PURPOSE:		Save information about the character animation
//
// ----------------------------------------------------------------------- //

#ifndef _CLIENTBUILD

void CharacterAnimation::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

 	// m_pInterface set by init.
	// m_pModel set by init.
	// m_hObject set by init.
	// m_nMessageID set by init.


	uint32 nNumLayers = 0;
	{for(AnimStyleLayer *pTempLayer = m_pLayers; pTempLayer; pTempLayer = pTempLayer->pNext)
	{
		++nNumLayers;
	}}

	*hWrite << nNumLayers;

	{for(AnimStyleLayer *pTempLayer = m_pLayers; pTempLayer; pTempLayer = pTempLayer->pNext)
	{
		AnimStyle * pAnimStyle = g_pAnimationButeMgr->GetAnimStyle(pTempLayer->nAnimStyle);
		ASSERT( pAnimStyle );
		if( pAnimStyle )
		{
			*hWrite << pAnimStyle->szName;
			*hWrite << *pTempLayer;
		}
	}}


	{for( TrackerInfo * pTempInfo = m_pTrackers; pTempInfo < m_pTrackers + ANIM_TRACKER_MAX; ++pTempInfo )
	{
		*hWrite << *pTempInfo;
	}}

	*hWrite << m_fBaseSpeed;
	
	*hWrite << m_nLastTrackerSet;
	*hWrite << m_nTrackerSet;

	*hWrite << m_bWaiting;


	*hWrite << m_nAltAnim;
	*hWrite << m_nAltTrackerSet;
	*hWrite << m_bUsingAltAnim;

	*hWrite << m_nTransTrackerSet;
	*hWrite << m_bUsingTransAnim;

	*hWrite << m_bFirstSetup;

}

#endif


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::Load()
//
// PURPOSE:		Load information about the character animation
//
// ----------------------------------------------------------------------- //

#ifndef _CLIENTBUILD

void CharacterAnimation::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

 	// m_pInterface set by init.
	// m_pModel set by init.
	// m_hObject set by init.
	// m_nMessageID set by init.

	ClearLayers();

	uint32 nNumLayers = 0;
	*hRead >> nNumLayers;

	AnimStyleLayer * pTempLayer = LTNULL;
	char szBuf[ANIMATION_BUTES_STRING_SIZE];
	{for(uint32 i = 0; i < nNumLayers; ++i )
	{
		*hRead >> szBuf;

		AddLayer(szBuf,i);

		if( !pTempLayer )
		{
			pTempLayer = m_pLayers;
		}
		else
		{
			pTempLayer = pTempLayer->pNext;
		}

		ASSERT( pTempLayer );
		if( pTempLayer )
		{
			*hRead >> *pTempLayer;
		}
	}}


	{for( TrackerInfo * pTempInfo = m_pTrackers; pTempInfo < m_pTrackers + ANIM_TRACKER_MAX; ++pTempInfo )
	{
		*hRead >> *pTempInfo;
	}}

	*hRead >> m_fBaseSpeed;
	
	*hRead >> m_nLastTrackerSet;
	*hRead >> m_nTrackerSet;

	*hRead >> m_bWaiting;


	*hRead >> m_nAltAnim;
	*hRead >> m_nAltTrackerSet;
	*hRead >> m_bUsingAltAnim;

	*hRead >> m_nTransTrackerSet;
	*hRead >> m_bUsingTransAnim;

	*hRead >> m_bFirstSetup;
}

#endif


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::Update()
//
// PURPOSE:		Handle the current character animation updates
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::Update()
{
	// Make sure all the really important information is valid
	if(!m_pInterface || !m_hObject || !m_bEnabled)
		return;


	// If we're using an alt or trans animation, we want to do things
	// a little different
	if(m_bUsingTransAnim)
	{
		// Update the transition tracker set
		UpdateTrackerSet(m_nTransTrackerSet);

		// Make sure the last tracker set is the trans set so we'll
		// reset after this transition stops
		m_nLastTrackerSet = m_nTransTrackerSet;
	}
	else if(m_bUsingAltAnim)
	{
		if(m_nAltAnim != INVALID_MODEL_ANIM)
		{
			// Update the expression tracker manually
			if((m_pTrackers[ANIM_TRACKER_EXPRESSION].m_nTrackerType != ANIMATION_BUTE_MGR_INVALID) &&
				(m_pTrackers[ANIM_TRACKER_EXPRESSION].m_nTrackerData != ANIMATION_BUTE_MGR_INVALID))
			{
				// Setup different based off of the tracker type
				switch(m_pTrackers[ANIM_TRACKER_EXPRESSION].m_nTrackerType)
				{
					case ANIMATION_BUTE_BASICTRACKER:	UpdateBasicTracker(ANIM_TRACKER_EXPRESSION, m_pTrackers[ANIM_TRACKER_EXPRESSION].m_nTrackerData);	break;
					case ANIMATION_BUTE_RANDOMTRACKER:	UpdateRandomTracker(ANIM_TRACKER_EXPRESSION, m_pTrackers[ANIM_TRACKER_EXPRESSION].m_nTrackerData);	break;
					case ANIMATION_BUTE_AIMINGTRACKER:	UpdateAimingTracker(ANIM_TRACKER_EXPRESSION, m_pTrackers[ANIM_TRACKER_EXPRESSION].m_nTrackerData);	break;
					case ANIMATION_BUTE_INDEXTRACKER:	UpdateIndexTracker(ANIM_TRACKER_EXPRESSION, m_pTrackers[ANIM_TRACKER_EXPRESSION].m_nTrackerData);	break;
				}
			}
		}
		else if(m_nAltTrackerSet != ANIMATION_BUTE_MGR_INVALID)
		{
			// Go through all the trackers and update them!
			UpdateTrackerSet(m_nAltTrackerSet);
		}
	}
	else
	{
		// If our tracker index is different than the last one... we will need
		// to update some specifics in the tracker info structures
		if((m_nLastTrackerSet != m_nTrackerSet) && !m_bWaiting)
		{
			// If our last tracker set was already a transition set, no
			// need to do an initial interpolation!
			LTBOOL bAllowInitialInterp = (   m_nTransTrackerSet == ANIMATION_BUTE_MGR_INVALID
				                          || m_nLastTrackerSet != m_nTransTrackerSet);

			// Play a transition set if one exists
			LTBOOL bTrans = PlayTransTrackerSet(m_nLastTrackerSet, m_nTrackerSet);

			// Do the initial setup work of the tracker set
			if(!bTrans) SetupTrackerSet(m_nTrackerSet, bAllowInitialInterp);

			// Make sure we set them equal so we don't get back in here
			// until we need to
			m_nLastTrackerSet = m_nTrackerSet;
		}
		else
		{
			// Go through all the trackers and update them!
			// This should remain the LastTrackerSet because we could be waiting
			// to switch to the current TrackerSet
			UpdateTrackerSet(m_nLastTrackerSet);
		}
	}

#ifndef _CLIENTBUILD
	if( g_vtCAError.GetFloat() > 0 )
	{
		for( TrackerInfo * pTempInfo = m_pTrackers; pTempInfo < m_pTrackers + ANIM_TRACKER_MAX; ++pTempInfo )
		{
			if(    pTempInfo->m_pTracker 
				&& pTempInfo->m_nTrackerType != ANIMATION_BUTE_MGR_INVALID
				&& pTempInfo->m_nTrackerData != ANIMATION_BUTE_MGR_INVALID )
			{
				if(    pTempInfo->m_hDestAnim == INVALID_MODEL_ANIM 
					|| pTempInfo->m_hDestWeightSet == INVALID_MODEL_WEIGHTSET )
				{
					g_pInterface->CPrint("Anim Error!! Object: %s Tracker: %d DestAnim: '%s' invalid %s.", 
						m_pInterface->GetObjectName(m_hObject), pTempInfo - m_pTrackers, pTempInfo->m_szDestAnim, 
						pTempInfo->m_hDestAnim == INVALID_MODEL_ANIM ? "animation" : "weightset" );
				}
			}
		}
	}
#endif

}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::HandleStringKey()
//
// PURPOSE:		Handle string keys from the model ABC
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::HandleStringKey(ArgList* pArgList, uint8 nTracker)
{
	// Get the first parameter
	char* pKey = pArgList->argv[0];
	if(!pKey) return;


	LTBOOL bBreakKey = LTFALSE;


	// This key gets sent automatically from Lithtech
	if(!_stricmp(pKey, "LTAnim_End"))
	{
		// Special cases for certain trackers

		if(nTracker == ANIM_TRACKER_RECOIL)
		{
			// Make sure we go back to a non-recoil animation
			m_pTrackers[nTracker].m_nIndex = -1;
		}
		else if(m_pTrackers[nTracker].m_nTrackerType == ANIMATION_BUTE_RANDOMTRACKER)
		{
			// Set the random tracker up to return to the base animation
			if(m_pTrackers[nTracker].m_nState & TRACKERINFO_STATE_RANDOMANIM)
			{
				m_pTrackers[nTracker].m_fRandTime = -1.0f;
			}
		}

		bBreakKey = LTTRUE;
	}
	else if(!_stricmp(pKey, "Anim_Break"))
	{
		bBreakKey = LTTRUE;
	}


	// ----------------------------------------------------------------------- //
	// If we hit an appropriate break key...
	if(bBreakKey)
	{
		// This handles the transition animations
		if(m_bUsingTransAnim)
		{
			if(nTracker == ANIM_TRACKER_MOVEMENT)
			{
				// Stop the animation so we'll go back to the next tracker set
				StopAnim(LTFALSE);
				m_nLastTrackerSet = m_nTransTrackerSet;

				// Now send a message telling we hit a break in the trans animation
				SendAnimationMessage(m_hObject, nTracker, LTFALSE, ANIMMSG_TRANS_ANIM_BREAK);
			}

#ifndef _CLIENTBUILD
			if((int)g_vtCADebug.GetFloat() & 0x04)
				g_pInterface->CPrint("CAnim::HandleStringKey!!  Object: %s  TransAnim '%s' Hit Break!  Tracker: %d  Key: %s", m_pInterface->GetObjectName(m_hObject), m_pTrackers[nTracker].m_szAnim, nTracker, pKey);
#endif
		}
		// If we're using an alternate animation, we want to do things
		// a little different
		else if(m_bUsingAltAnim)
		{
			if(m_nAltAnim != INVALID_MODEL_ANIM)
			{
				if(nTracker == ANIM_TRACKER_MOVEMENT)
				{
					// See if we're looping this animation...
					// If we aren't looping... go ahead and stop the animation
					LTBOOL bLooping = (m_pModel->GetLooping(m_pTrackers[nTracker].m_pTracker) == LT_YES);

					if(!bLooping)
						StopAnim(LTFALSE);

					// Now send a message telling we hit a break in the alt animation
					SendAnimationMessage(m_hObject, nTracker, bLooping, ANIMMSG_ALT_ANIM_BREAK);


#ifndef _CLIENTBUILD
					if((int)g_vtCADebug.GetFloat() & 0x04)
						g_pInterface->CPrint("CAnim::HandleStringKey!!  Object: %s  AltAnim '%s' Hit Break!  Key: %s", m_pInterface->GetObjectName(m_hObject), m_pTrackers[nTracker].m_szAnim, pKey);
#endif
				}
			}
		}
		else
		{
			// Send a message to the object letting it know that a tracker has hit a break point
			LTBOOL bLooping = (m_pModel->GetLooping(m_pTrackers[nTracker].m_pTracker) == LT_YES);
			SendAnimationMessage(m_hObject, nTracker, bLooping, ANIMMSG_ANIM_BREAK);

			// If we're waiting and trying to go to a new tracker set...
			if(m_bWaiting && ((m_nLastTrackerSet != m_nTrackerSet) || !bLooping))
			{
				// Clear this trackers wait flags
				m_pTrackers[nTracker].m_nState &= ~(TRACKERINFO_STATE_WAITEND | TRACKERINFO_STATE_WAITBREAK);

				// See if any of the other trackers are still waiting
				m_bWaiting = LTFALSE;

				// Go through each tracker data
				for(int i = 0; i < ANIM_TRACKER_MAX; i++)
				{
					if((i < ANIMATION_BUTE_MAX_TRACKERS) && m_pTrackers[i].m_pTracker)
					{
						if( (m_pTrackers[i].m_nState & TRACKERINFO_STATE_WAITEND) ||
							(m_pTrackers[i].m_nState & TRACKERINFO_STATE_WAITBREAK))
						{
							m_bWaiting = LTTRUE;
						}
					}
				}
			}

#ifndef _CLIENTBUILD
			if((int)g_vtCADebug.GetFloat() & 0x04)
				g_pInterface->CPrint("CAnim::HandleStringKey!!  Object: %s  Anim '%s' Hit Break!  Tracker: %d  Key: %s  Waiting: %s", m_pInterface->GetObjectName(m_hObject), m_pTrackers[nTracker].m_szAnim, nTracker, pKey, m_bWaiting ? "TRUE" : "FALSE");
#endif
		}
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::PlayAnim()
//
// PURPOSE:		Handles playing an alternate full body animation
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::PlayAnim(HMODELANIM hAnim, LTBOOL bLoop, LTBOOL bRestartAnim)
{
	//  Sanity
	if(hAnim == INVALID_ANI) return LTFALSE;

	// Turn on the alt animation flag
	m_bUsingAltAnim = LTTRUE;
	m_nAltAnim = hAnim;
	m_nAltTrackerSet = ANIMATION_BUTE_MGR_INVALID;

	// If the animation didn't actually exist... make sure we don't do anything
	if(m_nAltAnim == INVALID_MODEL_ANIM)
	{
		m_bUsingAltAnim = LTFALSE;
		return LTFALSE;
	}


	// Clear out all the trackers
	for(int i = 0; i < ANIM_TRACKER_MAX; i++)
	{
		if((i != ANIM_TRACKER_MOVEMENT) && (i != ANIM_TRACKER_EXPRESSION))
		{
			if((i < ANIMATION_BUTE_MAX_TRACKERS) && m_pTrackers[i].m_pTracker)
			{
				ClearTracker(i, -1);
			}
		}
	}


	// Now go ahead and play the animation
	LTAnimTracker *pAnimTrk = m_pTrackers[ANIM_TRACKER_MOVEMENT].m_pTracker;

	m_pModel->SetAllowTransition(pAnimTrk, LTTRUE);
	m_pModel->SetTimeScale(pAnimTrk, 1.0f);

	m_pModel->SetWeightSet(pAnimTrk, GetWeightSet("Full", 0));
	m_pModel->SetCurAnim(pAnimTrk, m_nAltAnim);
	
	if( bRestartAnim )
		m_pModel->ResetAnim(pAnimTrk);

	m_pModel->SetLooping(pAnimTrk, bLoop);
	m_pModel->SetPlaying(pAnimTrk, LTTRUE);


#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x01)
		g_pInterface->CPrint("CAnim::PlayAnim!!  Object: %s  Anim: %d  Looping: %s", m_pInterface->GetObjectName(m_hObject), hAnim, bLoop ? "TRUE" : "FALSE");
#endif


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::PlayAnim()
//
// PURPOSE:		Handles playing an alternate full body animation
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::PlayAnim(const char *szAnim, LTBOOL bLoop, LTBOOL bRestartAnim)
{
	return PlayAnim(GetAnim(szAnim), bLoop, bRestartAnim);
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::PlayTrackerSet()
//
// PURPOSE:		Handles playing an alternate tracker set
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::PlayTrackerSet(const char *szTrackerSet)
{
	// Turn on the alt animation flag
	m_bUsingAltAnim = LTTRUE;
	m_nAltAnim = INVALID_MODEL_ANIM;
	m_nAltTrackerSet = g_pAnimationButeMgr->GetTrackerSetIndex(szTrackerSet);

	// Now go ahead and setup the tracker set
	if(m_nAltTrackerSet == ANIMATION_BUTE_MGR_INVALID)
	{
		m_bUsingAltAnim = LTFALSE;
		return LTFALSE;
	}

	SetupTrackerSet(m_nAltTrackerSet, LTTRUE);

	if( !HasValidAnims() )
	{
		m_bUsingAltAnim = LTFALSE;
		return LTFALSE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::StopAnim()
//
// PURPOSE:		Handles stopping the alternate anim or tracker set
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::StopAnim(LTBOOL bUpdate)
{
	// Turn off the alt animation flag
	m_bUsingAltAnim = LTFALSE;
	m_bUsingTransAnim = LTFALSE;

	// Set the last tracker set to something invalid to make sure that
	// everything gets put back to the current tracker set
	m_nLastTrackerSet = ANIMATION_BUTE_MGR_INVALID;
	m_bWaiting = LTFALSE;

	// Now go ahead and update to make sure we set the animation
	if(bUpdate) Update();
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::MovementAllowed()
//
// PURPOSE:		Checks to see if movement is recommended for this tracker set
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::MovementAllowed(LTBOOL bLastTracker)
{
	// Get a pointer to the tracker set structure
	TrackerSet *pSet = g_pAnimationButeMgr->GetTrackerSet(bLastTracker ? m_nLastTrackerSet : m_nTrackerSet);
	if(!pSet) return LTFALSE;

	return pSet->bMovementAllowed;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::SetBaseSpeed()
//
// PURPOSE:		Handles changing the animation speeds
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::SetBaseSpeed(LTFLOAT fSpeed)
{
	// Only set the speeds if this value is different than the current
	if(fSpeed != m_fBaseSpeed)
	{
		// Save the new value
		m_fBaseSpeed = fSpeed;


		// Go through each tracker and update
		for(int i = 0; i < ANIM_TRACKER_MAX; i++)
		{
			if((i < ANIMATION_BUTE_MAX_TRACKERS) && m_pTrackers[i].m_pTracker)
			{
				// Check all the data to make sure we can do this
				if( (m_pTrackers[i].m_nTrackerType != ANIMATION_BUTE_MGR_INVALID) &&
					(m_pTrackers[i].m_nTrackerData != ANIMATION_BUTE_MGR_INVALID))
				{
					LTFLOAT fSpeed = 0.0f;

					// Setup different based off of the tracker type
					switch(m_pTrackers[i].m_nTrackerType)
					{
						case ANIMATION_BUTE_BASICTRACKER:
						{
							BasicTracker *pTrk = g_pAnimationButeMgr->GetBasicTracker(m_pTrackers[i].m_nTrackerData);
							fSpeed = pTrk->fAnimSpeed;
							break;
						}
						case ANIMATION_BUTE_RANDOMTRACKER:
						{
							RandomTracker *pTrk = g_pAnimationButeMgr->GetRandomTracker(m_pTrackers[i].m_nTrackerData);
							fSpeed = pTrk->fAnimSpeed;
							break;
						}
						case ANIMATION_BUTE_AIMINGTRACKER:
						{
							AimingTracker *pTrk = g_pAnimationButeMgr->GetAimingTracker(m_pTrackers[i].m_nTrackerData);
							fSpeed = pTrk->fAnimSpeed;
							break;
						}
						case ANIMATION_BUTE_INDEXTRACKER:
						{
							IndexTracker *pTrk = g_pAnimationButeMgr->GetIndexTracker(m_pTrackers[i].m_nTrackerData);
							fSpeed = pTrk->fAnimSpeed;
							break;
						}
					}

					// Set the speed
					LTFLOAT fScale = (!m_fBaseSpeed || !fSpeed) ? 1.0f : (fSpeed / m_fBaseSpeed);
					m_pModel->SetTimeScale(m_pTrackers[i].m_pTracker, fScale);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::AddLayer()
//
// PURPOSE:		Creates a new animation layer and adds it to the end
//				of the stack
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::AddLayer(const char *szStyle, int nID)
{
	// Make sure the info is valid
	if(!szStyle || !g_pAnimationButeMgr)
		return;

	// Find the style index
	int nIndex = g_pAnimationButeMgr->GetAnimStyleIndex(szStyle);


	if(nIndex != ANIMATION_BUTE_MGR_INVALID)
	{
		// Create the new layer
		AnimStyleLayer *pNewLayer = new AnimStyleLayer;
		pNewLayer->nLayerID = nID;
		pNewLayer->nAnimStyle = nIndex;
		pNewLayer->szReference[0] = 0;


		// Now add it to the stack
		if(!m_pLayers)
		{
			m_pLayers = pNewLayer;
		}
		else
		{
			AnimStyleLayer *pTempLayer = m_pLayers;

			while(pTempLayer->pNext)
				pTempLayer = pTempLayer->pNext;

			pTempLayer->pNext = pNewLayer;
		}
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::ClearLayers()
//
// PURPOSE:		Destroy all the layer information
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::ClearLayers()
{
	// If we're already cleared... just return
	if(!m_pLayers) return;

	AnimStyleLayer *pTempLayer = m_pLayers;

	while(pTempLayer)
	{
		AnimStyleLayer *pDelLayer = pTempLayer;
		pTempLayer = pTempLayer->pNext;

		delete pDelLayer;
	}

	m_pLayers = LTNULL;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::SetLayerReference()
//
// PURPOSE:		Sets the reference name for a particular animation layer
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::SetLayerReference(int nID, const char *szRef)
{
	// If we don't have any layers... just return
	if(!m_pLayers) return LTFALSE;

	AnimStyleLayer *pTempLayer = m_pLayers;

	while(pTempLayer)
	{
		if(pTempLayer->nLayerID == nID)
		{
			// If they're equal already... then we'll just forget
			// about doing anything else
			if(!_stricmp(pTempLayer->szReference, szRef))
				return LTTRUE;

			SAFE_STRCPY(pTempLayer->szReference, szRef);
			break;
		}

		pTempLayer = pTempLayer->pNext;
	}

	// Just abort if we couldn't find the layer.
	if( !pTempLayer ) return LTFALSE;

	// Now update the current tracker set
	m_nTrackerSet = g_pAnimationButeMgr->GetTrackerSetFromLayers(m_pLayers);

#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x01)
		g_pInterface->CPrint("CAnim::SetLayerReference!!  Object: %s  ID: %d  Ref: %s  TrackerSet: %d \"%s\"", 
			m_pInterface->GetObjectName(m_hObject), nID, szRef, m_nTrackerSet,
			g_pAnimationButeMgr->GetTrackerSet(m_nTrackerSet)->szName );
#endif

	// Now go ahead and update to make sure we set the animation
	Update();

	return (m_nTrackerSet != ANIMATION_BUTE_MGR_INVALID) && HasValidAnims();
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::GetLayerReference()
//
// PURPOSE:		Sets the reference name for a particular animation layer
//
// ----------------------------------------------------------------------- //

const char* CharacterAnimation::GetLayerReference(int nID)
{
	// If we don't have any layers... just return
	if(!m_pLayers) return LTNULL;

	AnimStyleLayer *pTempLayer = m_pLayers;

	while(pTempLayer)
	{
		if(pTempLayer->nLayerID == nID)
		{
			return pTempLayer->szReference;
		}

		pTempLayer = pTempLayer->pNext;
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::GetTrackerInfo()
//
// PURPOSE:		Get a pointer to the info struct
//
// ----------------------------------------------------------------------- //

TrackerInfo* CharacterAnimation::GetTrackerInfo(int nTInfo)
{
	// Make sure our index is within range
	if((nTInfo < 0) || (nTInfo >= ANIM_TRACKER_MAX))
		return LTNULL;

	return &m_pTrackers[nTInfo];
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::GetAnimLength()
//
// PURPOSE:		Gets the length of the current animation on this tracker
//
// ----------------------------------------------------------------------- //

uint32 CharacterAnimation::GetAnimLength(int nTInfo)
{
	// Make sure our index is within range
	if((nTInfo < 0) || (nTInfo >= ANIM_TRACKER_MAX))
		return 0;

	uint32 nLength;
	HMODELANIM hAnim;

	m_pModel->GetCurAnim(m_pTrackers[nTInfo].m_pTracker, hAnim);

	if(hAnim == INVALID_MODEL_ANIM)
		return 0;

	m_pModel->GetCurAnimLength(m_pTrackers[nTInfo].m_pTracker, nLength);

	return nLength;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::GetAnimTime()
//
// PURPOSE:		Gets the time of the current animation on this tracker
//
// ----------------------------------------------------------------------- //

uint32 CharacterAnimation::GetAnimTime(int nTInfo)
{
	// Make sure our index is within range
	if((nTInfo < 0) || (nTInfo >= ANIM_TRACKER_MAX))
		return 0;

	uint32 nTime;
	HMODELANIM hAnim;

	m_pModel->GetCurAnim(m_pTrackers[nTInfo].m_pTracker, hAnim);

	if(hAnim == INVALID_MODEL_ANIM)
		return 0;

	m_pModel->GetCurAnimTime(m_pTrackers[nTInfo].m_pTracker, nTime);

	return nTime;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::GetAnimTime()
//
// PURPOSE:		Gets the time of the current animation on this tracker
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::HasValidAnims()
{
	{for( TrackerInfo * pTempInfo = m_pTrackers; pTempInfo < m_pTrackers + ANIM_TRACKER_MAX; ++pTempInfo )
	{
		// Only check valid trackers.
		if(    pTempInfo->m_pTracker 
		    && pTempInfo->m_nTrackerType != ANIMATION_BUTE_MGR_INVALID
			&& pTempInfo->m_nTrackerData != ANIMATION_BUTE_MGR_INVALID )
		{
			// Invalid if the destination animation or destination weight set is invalid.
			if(    pTempInfo->m_hDestAnim == INVALID_MODEL_ANIM 
				|| pTempInfo->m_hDestWeightSet == INVALID_MODEL_WEIGHTSET )
				return LTFALSE;
		}
	}}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::SetupTrackerSet()
//
// PURPOSE:		This will init the tracker info to this set
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::SetupTrackerSet(int nTrackerSet, LTBOOL bAllowInitialInterp)
{
	// Make sure this is a valid tracker set
	if(nTrackerSet == ANIMATION_BUTE_MGR_INVALID)
		return;

	// Get a pointer to the tracker set structure
	TrackerSet *pSet = g_pAnimationButeMgr->GetTrackerSet(nTrackerSet);
	if(!pSet) return;


	// Clear our waiting variable so that it can be set only if needed
	m_bWaiting = LTFALSE;


#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x01)
		g_pInterface->CPrint("CAnim::SetupTrackerSet!!  ---------------------------- Begin ----------------------------");
#endif


	// Go through each tracker data
	for(int i = 0; i < ANIM_TRACKER_MAX; i++)
	{
		if((i < ANIMATION_BUTE_MAX_TRACKERS) && m_pTrackers[i].m_pTracker)
		{
			// Check all the data to make sure we can do this
			if((pSet->nTrackerTypes[i] != ANIMATION_BUTE_MGR_INVALID) &&
				(pSet->nTrackerData[i] != ANIMATION_BUTE_MGR_INVALID))
			{
				// Setup different based off of the tracker type
				switch(pSet->nTrackerTypes[i])
				{
					case ANIMATION_BUTE_BASICTRACKER:	SetupBasicTracker(i, pSet->nTrackerData[i], bAllowInitialInterp);	break;
					case ANIMATION_BUTE_RANDOMTRACKER:	SetupRandomTracker(i, pSet->nTrackerData[i], bAllowInitialInterp);	break;
					case ANIMATION_BUTE_AIMINGTRACKER:	SetupAimingTracker(i, pSet->nTrackerData[i], bAllowInitialInterp);	break;
					case ANIMATION_BUTE_INDEXTRACKER:	SetupIndexTracker(i, pSet->nTrackerData[i], bAllowInitialInterp);	break;
				}

				// Set the tracker info
				m_pTrackers[i].m_nTrackerType = pSet->nTrackerTypes[i];
				m_pTrackers[i].m_nTrackerData = pSet->nTrackerData[i];
			}
			else
			{
				// Set this tracker to the appropriate clear values
				ClearTracker(i, nTrackerSet);
			}


			// ----------------------------------------------------------------------- //
			// Set our waiting value to true if we need to
			if(!m_bUsingTransAnim)
			{
				if( (m_pTrackers[i].m_nState & TRACKERINFO_STATE_WAITEND) ||
					(m_pTrackers[i].m_nState & TRACKERINFO_STATE_WAITBREAK))
				{
					m_bWaiting = LTTRUE;
				}
			}
		}
	}

	m_bFirstSetup = LTFALSE;

#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x01)
		g_pInterface->CPrint("CAnim::SetupTrackerSet!!  Object: %s  TrackerSet: %d  Waiting: %s", m_pInterface->GetObjectName(m_hObject), nTrackerSet, m_bWaiting ? "TRUE" : "FALSE");
#endif
}

// ----------------------------------------------------------------------- //

void CharacterAnimation::SetupBasicTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp)
{
	// Get the tracker data structure
	BasicTracker *pTrk = g_pAnimationButeMgr->GetBasicTracker(nData);
	LTAnimTracker *pAnimTrk = m_pTrackers[nInfo].m_pTracker;


	// Store the destination info for later reference
	strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szAnim);
	m_pTrackers[nInfo].m_hDestAnim = INVALID_MODEL_ANIM;
	m_pTrackers[nInfo].m_hDestWeightSet = INVALID_MODEL_WEIGHTSET;

	// Get the animation we want to set
	m_pTrackers[nInfo].m_hDestAnim = GetAnim(pTrk->szAnim);
	if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) return;


	// Find the weight set to use
	m_pTrackers[nInfo].m_hDestWeightSet = GetWeightSet(pTrk->szWeightSet);
	if(m_pTrackers[nInfo].m_hDestWeightSet == INVALID_MODEL_WEIGHTSET) return;


	// We're good to setup everything... so go ahead
	LTBOOL bTrans = !(pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) || (bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP));
	m_pModel->SetAllowTransition(pAnimTrk, bTrans);

	LTFLOAT fScale = (!m_fBaseSpeed || !pTrk->fAnimSpeed) ? 1.0f : (pTrk->fAnimSpeed / m_fBaseSpeed);
	m_pModel->SetTimeScale(pAnimTrk, fScale);

	m_pModel->SetWeightSet(pAnimTrk, m_pTrackers[nInfo].m_hDestWeightSet);
	m_pModel->SetCurAnim(pAnimTrk, m_pTrackers[nInfo].m_hDestAnim);

	m_pModel->SetLooping(pAnimTrk, (pTrk->nFlags & ANIMATION_BUTE_FLAG_LOOPING));
	m_pModel->SetPlaying(pAnimTrk, LTTRUE);

	// Randomize the starting time of our very first animation.
	if( m_bFirstSetup )
	{
		uint32 nAnimLength = 0;
		m_pModel->GetCurAnimLength(pAnimTrk, nAnimLength);

		if( nAnimLength > 1 )
		{
			m_pModel->SetCurAnimTime(pAnimTrk, GetRandom(0,nAnimLength));
		}
	}

	// Store the current info for later reference
	strcpy(m_pTrackers[nInfo].m_szAnim, pTrk->szAnim);
	m_pTrackers[nInfo].m_hAnim = m_pTrackers[nInfo].m_hDestAnim;
	m_pTrackers[nInfo].m_hWeightSet = m_pTrackers[nInfo].m_hDestWeightSet;


	// Setup the state
	m_pTrackers[nInfo].m_nState = 0;

	if((pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) && bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP))
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_INITIALINTERP;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_END)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITEND;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_BREAK)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITBREAK;


#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x02)
		g_pInterface->CPrint("CAnim::SetupBasicTracker!!  Object: %s  Tracker: %d  BasicTracker: %d  %s", m_pInterface->GetObjectName(m_hObject), nInfo, nData,
		  (m_pTrackers[nInfo].m_nState & TRACKERINFO_STATE_INITIALINTERP) ? "InitialInterp" : "" );
#endif
}

// ----------------------------------------------------------------------- //

void CharacterAnimation::SetupRandomTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp)
{
	// Get the tracker data structure
	RandomTracker *pTrk = g_pAnimationButeMgr->GetRandomTracker(nData);
	LTAnimTracker *pAnimTrk = m_pTrackers[nInfo].m_pTracker;

	// Pick an initial animation.
	ASSERT(pTrk->nNumInitAnim > 0);
	if( pTrk->nNumInitAnim <= 0 ) return;
	int nAnim = GetRandom(0, pTrk->nNumInitAnim - 1);

	// Store the destination info for later reference
	strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szInitAnim[nAnim]);
	m_pTrackers[nInfo].m_hDestAnim = INVALID_MODEL_ANIM;
	m_pTrackers[nInfo].m_hDestWeightSet = INVALID_MODEL_WEIGHTSET;

	// Get the animation we want to set
	m_pTrackers[nInfo].m_hDestAnim = GetAnim(pTrk->szInitAnim[nAnim]);

	// If that failed, go to our first animation.
	if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) 
	{
		nAnim = 0;
		strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szInitAnim[nAnim]);
		m_pTrackers[nInfo].m_hDestAnim = GetAnim(pTrk->szInitAnim[nAnim]);

		// If that fails, get of here!
		if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) 
		{
			return;
		}
	}

	// Find the weight set to use
	m_pTrackers[nInfo].m_hDestWeightSet = GetWeightSet(pTrk->szWeightSet);
	if(m_pTrackers[nInfo].m_hDestWeightSet == INVALID_MODEL_WEIGHTSET) return;


	// We're good to setup everything... so go ahead
	LTBOOL bTrans = !(pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) || (bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP));
	m_pModel->SetAllowTransition(pAnimTrk, bTrans);

	LTFLOAT fScale = (!m_fBaseSpeed || !pTrk->fAnimSpeed) ? 1.0f : (pTrk->fAnimSpeed / m_fBaseSpeed);
	m_pModel->SetTimeScale(pAnimTrk, fScale);

	m_pModel->SetWeightSet(pAnimTrk, m_pTrackers[nInfo].m_hDestWeightSet);
	m_pModel->SetCurAnim(pAnimTrk, m_pTrackers[nInfo].m_hDestAnim);

	m_pModel->SetLooping(pAnimTrk, (pTrk->nFlags & ANIMATION_BUTE_FLAG_LOOPING));
	m_pModel->SetPlaying(pAnimTrk, LTTRUE);

	// Randomize the starting time of our very first animation.
	if( m_bFirstSetup )
	{
		uint32 nAnimLength = 0;
		m_pModel->GetCurAnimLength(pAnimTrk, nAnimLength);

		if( nAnimLength > 1 )
		{
			m_pModel->SetCurAnimTime(pAnimTrk, GetRandom(0,nAnimLength));
		}
	}

	// Store the current info for later reference
	strcpy(m_pTrackers[nInfo].m_szAnim, m_pTrackers[nInfo].m_szDestAnim);
	m_pTrackers[nInfo].m_hAnim = m_pTrackers[nInfo].m_hDestAnim;
	m_pTrackers[nInfo].m_hWeightSet = m_pTrackers[nInfo].m_hDestWeightSet;


	// Setup the random information
	m_pTrackers[nInfo].m_fRandTime = m_pInterface->GetTime();
	m_pTrackers[nInfo].m_fRandDelay = GetRandom(pTrk->fMinDelay, pTrk->fMaxDelay);


	// Setup the state
	m_pTrackers[nInfo].m_nState = 0;

	if((pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) && bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP))
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_INITIALINTERP;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_END)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITEND;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_BREAK)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITBREAK;


#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x02)
		g_pInterface->CPrint("CAnim::SetupRandomTracker!!  Object: %s  Tracker: %d  RandomTracker: %d  %s", m_pInterface->GetObjectName(m_hObject), nInfo, nData,
		  (m_pTrackers[nInfo].m_nState & TRACKERINFO_STATE_INITIALINTERP) ? "InitialInterp" : "" );
#endif
}

// ----------------------------------------------------------------------- //

void CharacterAnimation::SetupAimingTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp)
{
	// Get the tracker data structure
	AimingTracker *pTrk = g_pAnimationButeMgr->GetAimingTracker(nData);
	LTAnimTracker *pAnimTrk = m_pTrackers[nInfo].m_pTracker;

	// See if we should use the high or low animation
	LTBOOL bHigh = (LTBOOL)(m_pTrackers[nInfo].m_fAimingRange >= 0.0f);

	// Store the destination info for later reference
	strcpy(m_pTrackers[nInfo].m_szDestAnim, bHigh ? pTrk->szHighAnim : pTrk->szLowAnim);
	m_pTrackers[nInfo].m_hDestAnim = INVALID_MODEL_ANIM;
	m_pTrackers[nInfo].m_hDestWeightSet = INVALID_MODEL_WEIGHTSET;


	m_pTrackers[nInfo].m_hDestAnim = GetAnim(bHigh ? pTrk->szHighAnim : pTrk->szLowAnim);
	if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) return;


	// Calculate the appropriate weight set name
	int nRange = 0;
	
	if(bHigh)
		nRange = (int)(m_pTrackers[nInfo].m_fAimingRange * (LTFLOAT)(pTrk->nRange));
	else
		nRange = (int)((1.0f + m_pTrackers[nInfo].m_fAimingRange) * (LTFLOAT)(pTrk->nRange));

	if(!pTrk->bAscending) nRange = pTrk->nRange - nRange;

	m_pTrackers[nInfo].m_nIndex = bHigh ? nRange : -nRange;

	char szWeightSet[64];
	sprintf(szWeightSet, pTrk->szWeightSet, nRange);


	// Find the weight set to use
	m_pTrackers[nInfo].m_hDestWeightSet = GetWeightSet(szWeightSet);
	if(m_pTrackers[nInfo].m_hDestWeightSet == INVALID_MODEL_WEIGHTSET) return;


	// We're good to setup everything... so go ahead
	LTBOOL bTrans = !(pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) || ( bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP));
	m_pModel->SetAllowTransition(pAnimTrk, bTrans);

	LTFLOAT fScale = (!m_fBaseSpeed || !pTrk->fAnimSpeed) ? 1.0f : (pTrk->fAnimSpeed / m_fBaseSpeed);
	m_pModel->SetTimeScale(pAnimTrk, fScale);

	m_pModel->SetWeightSet(pAnimTrk, m_pTrackers[nInfo].m_hDestWeightSet);
	m_pModel->SetCurAnim(pAnimTrk, m_pTrackers[nInfo].m_hDestAnim);

	m_pModel->SetLooping(pAnimTrk, (pTrk->nFlags & ANIMATION_BUTE_FLAG_LOOPING));
	m_pModel->SetPlaying(pAnimTrk, LTTRUE);

	// Randomize the starting time of our very first animation.
	if( m_bFirstSetup )
	{
		uint32 nAnimLength = 0;
		m_pModel->GetCurAnimLength(pAnimTrk, nAnimLength);

		if( nAnimLength > 1 )
		{
			m_pModel->SetCurAnimTime(pAnimTrk, GetRandom(0,nAnimLength));
		}
	}

	// Store the current info for later reference
	strcpy(m_pTrackers[nInfo].m_szAnim, bHigh ? pTrk->szHighAnim : pTrk->szLowAnim);
	m_pTrackers[nInfo].m_hAnim = m_pTrackers[nInfo].m_hDestAnim;
	m_pTrackers[nInfo].m_hWeightSet = m_pTrackers[nInfo].m_hDestWeightSet;

	m_pTrackers[nInfo].m_fLastAimingRange = m_pTrackers[nInfo].m_fAimingRange;
	m_pTrackers[nInfo].m_nLastIndex = m_pTrackers[nInfo].m_nIndex;

	// Setup the state
	m_pTrackers[nInfo].m_nState = 0;

	if((pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) && bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP))
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_INITIALINTERP;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_END)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITEND;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_BREAK)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITBREAK;


#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x02)
		g_pInterface->CPrint("CAnim::SetupAimingTracker!!  Object: %s  Tracker: %d  AimingTracker: %d  %s", m_pInterface->GetObjectName(m_hObject), nInfo, nData,
			  (m_pTrackers[nInfo].m_nState & TRACKERINFO_STATE_INITIALINTERP) ? "InitialInterp" : "" );
#endif
}

// ----------------------------------------------------------------------- //

void CharacterAnimation::SetupIndexTracker(int nInfo, int nData, LTBOOL bAllowInitialInterp)
{
	// Get the tracker data structure
	IndexTracker *pTrk = g_pAnimationButeMgr->GetIndexTracker(nData);
	LTAnimTracker *pAnimTrk = m_pTrackers[nInfo].m_pTracker;

	// Set the index to a "safe" value so that if
	// the update fails, it can just return.
	const int nDesiredIndex = m_pTrackers[nInfo].m_nIndex;
	m_pTrackers[nInfo].m_nIndex = -1;

	// Find the appropriate index
	int nIndex = -1;

	for(int i = 0; i < pTrk->nNumIndex; i++)
	{
		if(pTrk->nIndex[i] == nDesiredIndex)
		{
			nIndex = i;
			break;
		}
	}

	if(nIndex == -1) return;

	// Store the destination info for later reference
	strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szAnim[nIndex]);
	m_pTrackers[nInfo].m_hDestAnim = INVALID_MODEL_ANIM;
	m_pTrackers[nInfo].m_hDestWeightSet = INVALID_MODEL_WEIGHTSET;


	// Get the animation we want to set
	m_pTrackers[nInfo].m_hDestAnim = GetAnim(pTrk->szAnim[nIndex]);
	if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) return;


	// Find the weight set to use
	m_pTrackers[nInfo].m_hDestWeightSet = GetWeightSet(pTrk->szWeightSet[nIndex]);
	if(m_pTrackers[nInfo].m_hDestWeightSet == INVALID_MODEL_WEIGHTSET) return;


	// We're good to setup everything... so go ahead
	LTBOOL bTrans = !(pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) || (bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP));
	m_pModel->SetAllowTransition(pAnimTrk, bTrans);

	LTFLOAT fScale = (!m_fBaseSpeed || !pTrk->fAnimSpeed) ? 1.0f : (pTrk->fAnimSpeed / m_fBaseSpeed);
	m_pModel->SetTimeScale(pAnimTrk, fScale);

	m_pModel->SetWeightSet(pAnimTrk, m_pTrackers[nInfo].m_hDestWeightSet);
	m_pModel->SetCurAnim(pAnimTrk, m_pTrackers[nInfo].m_hDestAnim);

	m_pModel->SetLooping(pAnimTrk, (pTrk->nFlags & ANIMATION_BUTE_FLAG_LOOPING));
	m_pModel->SetPlaying(pAnimTrk, LTTRUE);

	// Randomize the starting time of our very first animation.
	if( m_bFirstSetup )
	{
		uint32 nAnimLength = 0;
		m_pModel->GetCurAnimLength(pAnimTrk, nAnimLength);

		if( nAnimLength > 1 )
		{
			m_pModel->SetCurAnimTime(pAnimTrk, GetRandom(0,nAnimLength));
		}
	}

	// Store the current info for later reference
	strcpy(m_pTrackers[nInfo].m_szAnim, pTrk->szAnim[nIndex]);
	m_pTrackers[nInfo].m_hAnim = m_pTrackers[nInfo].m_hDestAnim;
	m_pTrackers[nInfo].m_hWeightSet = m_pTrackers[nInfo].m_hDestWeightSet;
	m_pTrackers[nInfo].m_nIndex = nDesiredIndex;
	m_pTrackers[nInfo].m_nLastIndex = m_pTrackers[nInfo].m_nIndex;


	// Setup the state
	m_pTrackers[nInfo].m_nState = 0;

	if((pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP) && bAllowInitialInterp && (pTrk->nFlags & ANIMATION_BUTE_FLAG_INITINTERP))
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_INITIALINTERP;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_END)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITEND;

	if(pTrk->nFlags & ANIMATION_BUTE_FLAG_BREAK)
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_WAITBREAK;


#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x02)
		g_pInterface->CPrint("CAnim::SetupIndexTracker!!  Object: %s  Tracker: %d  IndexTracker: %d  %s", m_pInterface->GetObjectName(m_hObject), nInfo, nData,
		  (m_pTrackers[nInfo].m_nState & TRACKERINFO_STATE_INITIALINTERP) ? "InitialInterp" : "" );

#endif
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::UpdateTrackerSet()
//
// PURPOSE:		This will update the tracker info using this set
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::UpdateTrackerSet(int nTrackerSet)
{
	// Make sure this is a valid tracker set
	if(nTrackerSet == ANIMATION_BUTE_MGR_INVALID)
		return;


	// Go through each tracker data
	for(int i = 0; i < ANIM_TRACKER_MAX; i++)
	{
		if((i < ANIMATION_BUTE_MAX_TRACKERS) && m_pTrackers[i].m_pTracker)
		{
			// Check all the data to make sure we can do this
			if((m_pTrackers[i].m_nTrackerType != ANIMATION_BUTE_MGR_INVALID) &&
				(m_pTrackers[i].m_nTrackerData != ANIMATION_BUTE_MGR_INVALID))
			{
				// Setup different based off of the tracker type
				switch(m_pTrackers[i].m_nTrackerType)
				{
					case ANIMATION_BUTE_BASICTRACKER:	UpdateBasicTracker(i, m_pTrackers[i].m_nTrackerData);	break;
					case ANIMATION_BUTE_RANDOMTRACKER:	UpdateRandomTracker(i, m_pTrackers[i].m_nTrackerData);	break;
					case ANIMATION_BUTE_AIMINGTRACKER:	UpdateAimingTracker(i, m_pTrackers[i].m_nTrackerData);	break;
					case ANIMATION_BUTE_INDEXTRACKER:	UpdateIndexTracker(i, m_pTrackers[i].m_nTrackerData);	break;
				}
			}


			// ----------------------------------------------------------------------- //
			// Update the state data

			// Check for the initial interpolation state
			if(m_pTrackers[i].m_nState & TRACKERINFO_STATE_INITIALINTERP)
			{
				// The LT define for NOT_INTERPOLATING is 0xFFFF
				if(m_pTrackers[i].m_pTracker->m_InterpolationMS == 0xFFFF)
				{
					m_pModel->SetAllowTransition(m_pTrackers[i].m_pTracker, LTFALSE);
					m_pTrackers[i].m_nState &= ~TRACKERINFO_STATE_INITIALINTERP;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //

void CharacterAnimation::UpdateBasicTracker(int nInfo, int nData)
{

}

// ----------------------------------------------------------------------- //

void CharacterAnimation::UpdateRandomTracker(int nInfo, int nData)
{
	// Get the tracker data structure
	RandomTracker *pTrk = g_pAnimationButeMgr->GetRandomTracker(nData);
	LTAnimTracker *pAnimTrk = m_pTrackers[nInfo].m_pTracker;


	// See if we need to reset to our base animation
	if(m_pTrackers[nInfo].m_nState & TRACKERINFO_STATE_RANDOMANIM)
	{
		if(m_pTrackers[nInfo].m_fRandTime == -1.0f)
		{
			// Pick an initial animation.
			ASSERT(pTrk->nNumInitAnim > 0);
			if( pTrk->nNumInitAnim <= 0 ) return;
			int nAnim = GetRandom(0, pTrk->nNumInitAnim - 1);

			// Store the destination info for later reference
			strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szInitAnim[nAnim]);
			m_pTrackers[nInfo].m_hDestAnim = INVALID_MODEL_ANIM;

			// Find the animation.
			m_pTrackers[nInfo].m_hDestAnim = GetAnim(pTrk->szInitAnim[nAnim]);

			// If that failed, go to our first animation.
			if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) 
			{
				nAnim = 0;
				strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szInitAnim[nAnim]);
				m_pTrackers[nInfo].m_hDestAnim = GetAnim(pTrk->szInitAnim[nAnim]);
			}

			// Set the animation and looping values
			m_pModel->SetAllowTransition(pAnimTrk, !(pTrk->nFlags & ANIMATION_BUTE_FLAG_NOINTERP));
			m_pModel->SetCurAnim(pAnimTrk, m_pTrackers[nInfo].m_hDestAnim);
			m_pModel->SetLooping(pAnimTrk, (pTrk->nFlags & ANIMATION_BUTE_FLAG_LOOPING));

			// Copy over the current animation information
			strcpy(m_pTrackers[nInfo].m_szAnim, m_pTrackers[nInfo].m_szDestAnim);
			m_pTrackers[nInfo].m_hAnim = m_pTrackers[nInfo].m_hDestAnim;

			// Get some new random times to use
			m_pTrackers[nInfo].m_fRandTime = m_pInterface->GetTime();
			m_pTrackers[nInfo].m_fRandDelay = GetRandom(pTrk->fMinDelay, pTrk->fMaxDelay);
			m_pTrackers[nInfo].m_nState &= ~TRACKERINFO_STATE_RANDOMANIM;
		}
	}
	else
	{
		// See if we're ready to play a random animation
		if(m_pInterface->GetTime() < (m_pTrackers[nInfo].m_fRandTime + m_pTrackers[nInfo].m_fRandDelay))
			return;


		// Pick a random animation to play
		if(pTrk->nNumRandomAnim <= 0) 
		{
			// Setup so that the main animation is re-started
			m_pTrackers[nInfo].m_fRandTime = -1.0f;
			m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_RANDOMANIM;
			return;
		}

		int nAnim = GetRandom(0, pTrk->nNumRandomAnim - 1);


		// Get the animation we want to set
		HMODELANIM hAnim = GetAnim(pTrk->szRandomAnim[nAnim]);
		if(hAnim == INVALID_MODEL_ANIM) 
		{
			// Setup so that the main animation is re-started
			m_pTrackers[nInfo].m_fRandTime = -1.0f;
			m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_RANDOMANIM;
			return;
		}

		// Store the destination info for later reference 
		// (this is done after the animation check because a bad random
		//  animation is resolved by restarting the main animation).
		strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szRandomAnim[nAnim]);
		m_pTrackers[nInfo].m_hDestAnim = hAnim;

		// We're good to setup everything... so go ahead
		m_pModel->SetAllowTransition(pAnimTrk, LTTRUE);
		m_pModel->SetCurAnim(pAnimTrk, hAnim);
		m_pModel->SetLooping(pAnimTrk, LTFALSE);


		// Store the current info for later reference
		strcpy(m_pTrackers[nInfo].m_szAnim, m_pTrackers[nInfo].m_szDestAnim);
		m_pTrackers[nInfo].m_hAnim = hAnim;


		// Setup the random information
		m_pTrackers[nInfo].m_nState |= TRACKERINFO_STATE_RANDOMANIM;


#ifndef _CLIENTBUILD
		if((int)g_vtCADebug.GetFloat() & 0x02)
			g_pInterface->CPrint("CAnim::UpdateRandomTracker!!  Object: %s  Tracker: %d  RandomTracker: %d  RandAnim: %s", m_pInterface->GetObjectName(m_hObject), nInfo, nData, pTrk->szRandomAnim[nAnim]);
#endif
	}

	// Randomize the starting time of our very first animation.
	if( m_bFirstSetup )
	{
		uint32 nAnimLength = 0;
		m_pModel->GetCurAnimLength(pAnimTrk, nAnimLength);

		if( nAnimLength > 1 )
		{
			m_pModel->SetCurAnimTime(pAnimTrk, GetRandom(0,nAnimLength));
		}
	}

}

// ----------------------------------------------------------------------- //

void CharacterAnimation::UpdateAimingTracker(int nInfo, int nData)
{
	// Get the tracker data structure
	AimingTracker *pTrk = g_pAnimationButeMgr->GetAimingTracker(nData);
	LTAnimTracker *pAnimTrk = m_pTrackers[nInfo].m_pTracker;


	// If our aiming value hasn't changed... then just leave
	if( (m_pTrackers[nInfo].m_fAimingRange == m_pTrackers[nInfo].m_fLastAimingRange) ||
		(m_pTrackers[nInfo].m_nState & TRACKERINFO_STATE_INITIALINTERP) ||
		(m_nLastTrackerSet != m_nTrackerSet))
		return;


	// See if we should use the high or low animation
	LTBOOL bHigh = (LTBOOL)(m_pTrackers[nInfo].m_fAimingRange >= 0.0f);

	// Calculate the appropriate weight set name
	int nRange = 0;
	
	if(bHigh)
		nRange = (int)(m_pTrackers[nInfo].m_fAimingRange * (LTFLOAT)(pTrk->nRange));
	else
		nRange = (int)((1.0f + m_pTrackers[nInfo].m_fAimingRange) * (LTFLOAT)(pTrk->nRange));

	if(!pTrk->bAscending) nRange = pTrk->nRange - nRange;

	// Store this range in our index value, negative will indicate low aiming.
	m_pTrackers[nInfo].m_nIndex = bHigh ? nRange : -nRange;

	// If our aiming index value hasn't changed... then just leave
	if( m_pTrackers[nInfo].m_nIndex == m_pTrackers[nInfo].m_nLastIndex )
		return;

	// Store the destination info for later reference
	strcpy(m_pTrackers[nInfo].m_szDestAnim, bHigh ? pTrk->szHighAnim : pTrk->szLowAnim);
	m_pTrackers[nInfo].m_hDestAnim = INVALID_MODEL_ANIM;
	m_pTrackers[nInfo].m_hDestWeightSet = INVALID_MODEL_WEIGHTSET;

	if( 0 != strcmp(m_pTrackers[nInfo].m_szDestAnim,m_pTrackers[nInfo].m_szAnim) )
		m_pTrackers[nInfo].m_hDestAnim = GetAnim(bHigh ? pTrk->szHighAnim : pTrk->szLowAnim);
	else
		m_pTrackers[nInfo].m_hDestAnim = m_pTrackers[nInfo].m_hAnim;

	if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) return;

	char szWeightSet[64];
	sprintf(szWeightSet, pTrk->szWeightSet, nRange);


	// Find the weight set to use
	m_pTrackers[nInfo].m_hDestWeightSet = GetWeightSet(szWeightSet);
	if(m_pTrackers[nInfo].m_hDestWeightSet == INVALID_MODEL_WEIGHTSET) return;


	// We're good to setup everything... so go ahead
	if( m_pTrackers[nInfo].m_hDestWeightSet != m_pTrackers[nInfo].m_hWeightSet )
		m_pModel->SetWeightSet(pAnimTrk, m_pTrackers[nInfo].m_hDestWeightSet);

	if( m_pTrackers[nInfo].m_hDestAnim != m_pTrackers[nInfo].m_hAnim )
	{
		// Set the new animation at the same time offset
		// as the old animation.
		uint32 nCurTime = 0;
		m_pModel->GetCurAnimTime(pAnimTrk, nCurTime);

		m_pModel->SetCurAnim(pAnimTrk, m_pTrackers[nInfo].m_hDestAnim);

		uint32 nNewLength = 0;
		m_pModel->GetCurAnimLength(pAnimTrk, nNewLength);

		if( nNewLength > nCurTime )
			m_pModel->SetCurAnimTime(pAnimTrk,nCurTime);
	}


	// Store the current info for later reference
	strcpy(m_pTrackers[nInfo].m_szAnim, bHigh ? pTrk->szHighAnim : pTrk->szLowAnim);
	m_pTrackers[nInfo].m_hAnim = m_pTrackers[nInfo].m_hDestAnim;
	m_pTrackers[nInfo].m_hWeightSet = m_pTrackers[nInfo].m_hDestWeightSet;
	m_pTrackers[nInfo].m_fLastAimingRange = m_pTrackers[nInfo].m_fAimingRange;
	m_pTrackers[nInfo].m_nLastIndex = m_pTrackers[nInfo].m_nIndex;
}

// ----------------------------------------------------------------------- //

void CharacterAnimation::UpdateIndexTracker(int nInfo, int nData)
{
	// Get the tracker data structure
	IndexTracker *pTrk = g_pAnimationButeMgr->GetIndexTracker(nData);
	LTAnimTracker *pAnimTrk = m_pTrackers[nInfo].m_pTracker;


	// If our aiming value hasn't changed... then just leave
	if(m_pTrackers[nInfo].m_nIndex == m_pTrackers[nInfo].m_nLastIndex)
		return;

	// Set the index to a "safe" value so that if
	// the update fails, it can just return.
	const int nDesiredIndex = m_pTrackers[nInfo].m_nIndex;
	m_pTrackers[nInfo].m_nIndex = -1;

	// Find the appropriate index
	int nIndex = -1;

	for(int i = 0; i < pTrk->nNumIndex; i++)
	{
		if(pTrk->nIndex[i] == nDesiredIndex)
		{
			nIndex = i;
			break;
		}
	}

	if(nIndex == -1) return;

	// Store the destination info for later reference
	strcpy(m_pTrackers[nInfo].m_szDestAnim, pTrk->szAnim[nIndex]);
	m_pTrackers[nInfo].m_hDestAnim = INVALID_MODEL_ANIM;
	m_pTrackers[nInfo].m_hDestWeightSet = INVALID_MODEL_WEIGHTSET;

	// Get the animation we want to set
	m_pTrackers[nInfo].m_hDestAnim = GetAnim(pTrk->szAnim[nIndex]);
	if(m_pTrackers[nInfo].m_hDestAnim == INVALID_MODEL_ANIM) return;


	// Find the weight set to use
	m_pTrackers[nInfo].m_hDestWeightSet = GetWeightSet(pTrk->szWeightSet[nIndex]);
	if(m_pTrackers[nInfo].m_hDestWeightSet == INVALID_MODEL_WEIGHTSET) return;


	// We're good to setup everything... so go ahead
	m_pModel->SetWeightSet(pAnimTrk, m_pTrackers[nInfo].m_hDestWeightSet);
	m_pModel->SetCurAnim(pAnimTrk, m_pTrackers[nInfo].m_hDestAnim);

	// Store the current info for later reference
	strcpy(m_pTrackers[nInfo].m_szAnim, pTrk->szAnim[nIndex]);
	m_pTrackers[nInfo].m_hAnim = m_pTrackers[nInfo].m_hDestAnim;
	m_pTrackers[nInfo].m_hWeightSet = m_pTrackers[nInfo].m_hDestWeightSet;
	m_pTrackers[nInfo].m_nIndex = nDesiredIndex;
	m_pTrackers[nInfo].m_nLastIndex = m_pTrackers[nInfo].m_nIndex;


#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x02)
		g_pInterface->CPrint("CAnim::UpdateIndexTracker!!  Object: %s  Tracker: %d  IndexTracker: %d  Index: %d  IndexAnim: %s", m_pInterface->GetObjectName(m_hObject), nInfo, nData, nIndex, pTrk->szAnim[nIndex]);
#endif
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::ClearTracker()
//
// PURPOSE:		This clears out a tracker in the set
//
// ----------------------------------------------------------------------- //

void CharacterAnimation::ClearTracker(int nInfo, int nTrackerSet)
{
	// Get a pointer to the tracker set structure
	TrackerSet *pSet = g_pAnimationButeMgr->GetTrackerSet(nTrackerSet);


	// Set this tracker to the appropriate clear values
	m_pModel->SetAllowTransition(m_pTrackers[nInfo].m_pTracker, LTTRUE);

	HMODELWEIGHTSET hSet = GetWeightSet(pSet ? pSet->szClearWeightSet: "Empty", 0);
	m_pModel->SetWeightSet(m_pTrackers[nInfo].m_pTracker, hSet);

	HMODELANIM hAnim = GetAnim(pSet ? pSet->szClearAnim : "Dims_Default", 0);
	m_pModel->SetCurAnim(m_pTrackers[nInfo].m_pTracker, hAnim);


	// Make sure it's playing, otherwise there'll be problems in the interpolation
	m_pModel->SetLooping(m_pTrackers[nInfo].m_pTracker, LTTRUE);
	m_pModel->SetPlaying(m_pTrackers[nInfo].m_pTracker, LTTRUE);


	// Set the tracker info
	strcpy(m_pTrackers[nInfo].m_szDestAnim, pSet ? pSet->szClearAnim : "Cleared");
	m_pTrackers[nInfo].m_hDestAnim = hAnim;
	m_pTrackers[nInfo].m_hDestWeightSet = hSet;

	strcpy(m_pTrackers[nInfo].m_szAnim, pSet ? pSet->szClearAnim : "Cleared");
	m_pTrackers[nInfo].m_hAnim = hAnim;
	m_pTrackers[nInfo].m_hWeightSet = hSet;

	m_pTrackers[nInfo].m_nTrackerType = ANIMATION_BUTE_MGR_INVALID;
	m_pTrackers[nInfo].m_nTrackerData = ANIMATION_BUTE_MGR_INVALID;
	m_pTrackers[nInfo].m_nState = 0;

}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::GetAnim()
//
// PURPOSE:		Get an anim ID from a given name
//
// ----------------------------------------------------------------------- //

HMODELANIM CharacterAnimation::GetAnim(const char *szName, HMODELANIM hDefault)
{
	HMODELANIM hAnim;

	// Search for an override animation first
	char szBuffer[64];
	sprintf(szBuffer, "*%s", szName);

	hAnim = m_pInterface->GetAnimIndex(m_hObject, szBuffer);

	// If the override animation doesn't exist... then get the original
	if(hAnim == INVALID_MODEL_ANIM)
	{
		hAnim = m_pInterface->GetAnimIndex(m_hObject, const_cast<char*>(szName));
	}

	// Return the index or the default
	return ((hAnim == INVALID_MODEL_ANIM) ? hDefault : hAnim);
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::GetWeightSet()
//
// PURPOSE:		Get a weight set ID from a given name
//
// ----------------------------------------------------------------------- //

HMODELWEIGHTSET CharacterAnimation::GetWeightSet(const char *szName, HMODELWEIGHTSET hDefault)
{
	HMODELWEIGHTSET hWeightSet;
	m_pModel->FindWeightSet(m_hObject, const_cast<char*>(szName), hWeightSet);

	// Return the index or the default
	return ((hWeightSet == INVALID_MODEL_WEIGHTSET) ? hDefault : hWeightSet);
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::SendAnimationMessage()
//
// PURPOSE:		Send animation information to the object
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::SendAnimationMessage(HOBJECT hObject, uint8 nTracker, LTBOOL bLooping, uint8 nInfo)
{
	// If the message ID isn't valid... then the object just won't get info messages
	if(m_nMessageID != -1)
	{
		HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(LTNULL, hObject, m_nMessageID);
		g_pLTServer->WriteToMessageByte(hWrite, nTracker);
		g_pLTServer->WriteToMessageByte(hWrite, bLooping);
		g_pLTServer->WriteToMessageByte(hWrite, nInfo);
		g_pLTServer->EndMessage(hWrite);

		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterAnimation::PlayTransTrackerSet()
//
// PURPOSE:		Determines if there's a transition animation and plays it
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterAnimation::PlayTransTrackerSet(int nFromSet, int nToSet)
{
	// Find the transition tracker set
	m_bUsingTransAnim = LTTRUE;
	m_nTransTrackerSet = g_pAnimationButeMgr->GetTransitionSet(nFromSet, nToSet);

	if(m_nTransTrackerSet == ANIMATION_BUTE_MGR_INVALID)
	{
		m_bUsingTransAnim = LTFALSE;
		return LTFALSE;
	}

#ifndef _CLIENTBUILD
	if((int)g_vtCADebug.GetFloat() & 0x01)
		g_pInterface->CPrint("CAnim::PlayTransTrackerSet!!  Object: %s  From: %d  To: %d  Trans: %d %s", 
			m_pInterface->GetObjectName(m_hObject), nFromSet, nToSet, m_nTransTrackerSet,
			g_pAnimationButeMgr->GetTrackerSet(m_nTransTrackerSet)->szName );
#endif

	// Setup the transition animation playing
	SetupTrackerSet(m_nTransTrackerSet, LTTRUE);

	if( !HasValidAnims() )
	{
		m_bUsingTransAnim = LTFALSE;
		return LTFALSE;
	}

	return LTTRUE;
}


