//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Retrieves attributes from the AnimationButes.txt
//*********************************************************************************
// File:		AnimationButeMgr.cpp
// Created:		Aug 5, 2000
// Updated:		Aug 5, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#include "stdafx.h"
#include "AnimationButeMgr.h"

//*********************************************************************************

#define ANIMSTYLE_BUTE_TAG					"AnimStyle_"

#define ANIMSTYLE_BUTE_REFERENCE			"Reference"
#define ANIMSTYLE_BUTE_TRACKERSETFRAG		"TrackerSetFrag"
#define ANIMSTYLE_BUTE_ALTTRACKERSETFRAG	"AltTrackerSetFrag"

//*********************************************************************************

#define TRACKERSET_BUTE_TAG					"TrackerSet_"

#define TRACKERSET_BUTE_CLEARANIM			"ClearAnim"
#define TRACKERSET_BUTE_CLEARWEIGHTSET		"ClearWeightSet"
#define TRACKERSET_BUTE_TRACKERDATA			"TrackerData"
#define TRACKERSET_BUTE_MOVEMENTALLOWED		"MovementAllowed"

//*********************************************************************************

#define BASICTRACKER_BUTE_TAG				"BasicTracker_"

#define BASICTRACKER_BUTE_ANIM				"Anim"
#define BASICTRACKER_BUTE_WEIGHTSET			"WeightSet"
#define BASICTRACKER_BUTE_ANIMSPEED			"AnimSpeed"
#define BASICTRACKER_BUTE_FLAGS				"Flags"

//*********************************************************************************

#define RANDOMTRACKER_BUTE_TAG				"RandomTracker_"

#define RANDOMTRACKER_BUTE_WEIGHTSET		"WeightSet"
#define RANDOMTRACKER_BUTE_ANIMSPEED		"AnimSpeed"
#define RANDOMTRACKER_BUTE_FLAGS			"Flags"
#define RANDOMTRACKER_BUTE_INITANIM			"InitAnim"
#define RANDOMTRACKER_BUTE_RANDANIM			"RandAnim"
#define RANDOMTRACKER_BUTE_MINDELAY			"MinDelay"
#define RANDOMTRACKER_BUTE_MAXDELAY			"MaxDelay"

//*********************************************************************************

#define AIMINGTRACKER_BUTE_TAG				"AimingTracker_"

#define AIMINGTRACKER_BUTE_HIGHANIM			"HighAnim"
#define AIMINGTRACKER_BUTE_LOWANIM			"LowAnim"
#define AIMINGTRACKER_BUTE_WEIGHTSET		"WeightSet"
#define AIMINGTRACKER_BUTE_RANGE			"Range"
#define AIMINGTRACKER_BUTE_ASCENDING		"Ascending"
#define AIMINGTRACKER_BUTE_ANIMSPEED		"AnimSpeed"
#define AIMINGTRACKER_BUTE_FLAGS			"Flags"

//*********************************************************************************

#define INDEXTRACKER_BUTE_TAG				"IndexTracker_"

#define INDEXTRACKER_BUTE_INDEX				"Index"
#define INDEXTRACKER_BUTE_ANIM				"Anim"
#define INDEXTRACKER_BUTE_WEIGHTSET			"WeightSet"
#define INDEXTRACKER_BUTE_ANIMSPEED			"AnimSpeed"
#define INDEXTRACKER_BUTE_FLAGS				"Flags"

//*********************************************************************************

#define TRANSITION_BUTE_TAG					"Transition_"

#define TRANSITION_BUTE_FROM				"From"
#define TRANSITION_BUTE_TO					"To"
#define TRANSITION_BUTE_TRANS				"Trans"

//*********************************************************************************

CAnimationButeMgr* g_pAnimationButeMgr = LTNULL;

//*********************************************************************************

static char s_aTagName[ANIMATION_BUTES_STRING_SIZE];
static char s_aAttribName[ANIMATION_BUTES_STRING_SIZE];

//*********************************************************************************
// t_AnimStyleLayer Load/Save
//*********************************************************************************
#ifndef _CLIENTBUILD

ILTMessage & operator<<(ILTMessage & out, t_AnimStyleLayer & layer)
{
	out << layer.nLayerID;
	out << layer.nAnimStyle;

	uint32 nStringLength = strlen(layer.szReference) + 1;
	out << nStringLength;
	out.WriteString(layer.szReference);

	return out;
}

ILTMessage & operator>>(ILTMessage & in, t_AnimStyleLayer & layer)
{
	in >> layer.nLayerID;
	in >> layer.nAnimStyle;

	uint32 nStringLength = 0;
	in >> nStringLength;

	ASSERT( nStringLength < ANIMATION_BUTES_STRING_SIZE );

	in.ReadStringFL(layer.szReference, nStringLength);

	return in;
}

#endif

//*********************************************************************************
// Construction/Destruction
//*********************************************************************************

CAnimationButeMgr::CAnimationButeMgr()
{
	m_pAnimStyles = LTNULL;
	m_pTrackerSets = LTNULL;
	m_pBasicTrackers = LTNULL;
	m_pRandomTrackers = LTNULL;
	m_pAimingTrackers = LTNULL;
	m_pIndexTrackers = LTNULL;
	m_pTransitions = LTNULL;
}

CAnimationButeMgr::~CAnimationButeMgr()
{
	Term();
}


//*********************************************************************************
//
//	ROUTINE:	CAnimationButeMgr::Init()
//	PURPOSE:	Init mgr
//
//*********************************************************************************

bool CAnimationButeMgr::CountTags(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) return true;

	// Get access to the bute mgr
	CAnimationButeMgr *pButeMgr = (CAnimationButeMgr*)pData;

	if(!_strnicmp(szTagName, ANIMSTYLE_BUTE_TAG, sizeof(ANIMSTYLE_BUTE_TAG) - 1))
		++pButeMgr->m_nNumAnimStyles;
	else if(!_strnicmp(szTagName, TRACKERSET_BUTE_TAG, sizeof(TRACKERSET_BUTE_TAG) - 1))
		++pButeMgr->m_nNumTrackerSets;
	else if(!_strnicmp(szTagName, BASICTRACKER_BUTE_TAG, sizeof(BASICTRACKER_BUTE_TAG) - 1))
		++pButeMgr->m_nNumBasicTrackers;
	else if(!_strnicmp(szTagName, RANDOMTRACKER_BUTE_TAG, sizeof(RANDOMTRACKER_BUTE_TAG) - 1))
		++pButeMgr->m_nNumRandomTrackers;
	else if(!_strnicmp(szTagName, AIMINGTRACKER_BUTE_TAG, sizeof(AIMINGTRACKER_BUTE_TAG) - 1))
		++pButeMgr->m_nNumAimingTrackers;
	else if(!_strnicmp(szTagName, INDEXTRACKER_BUTE_TAG, sizeof(INDEXTRACKER_BUTE_TAG) - 1))
		++pButeMgr->m_nNumIndexTrackers;
	else if(!_strnicmp(szTagName, TRANSITION_BUTE_TAG, sizeof(TRANSITION_BUTE_TAG) - 1))
		++pButeMgr->m_nNumTransitions;

	return true;
}

//-------------------------------------------------------------------------------//

struct LoadButesInfo
{
	CAnimationButeMgr *pButeMgr;
	char szTag[ANIMATION_BUTES_STRING_SIZE];
};

bool CAnimationButeMgr::LoadButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) return true;

	LoadButesInfo *bInfo = (LoadButesInfo*)pData;
	CAnimationButeMgr *pButeMgr = (CAnimationButeMgr*)bInfo->pButeMgr;
	char *szTag = (char*)bInfo->szTag;

	if(!strnicmp(szTagName, bInfo->szTag, strlen(bInfo->szTag)))
	{
		if(!stricmp(bInfo->szTag, ANIMSTYLE_BUTE_TAG))
			pButeMgr->LoadAnimStyle(szTagName);
		else if(!stricmp(bInfo->szTag, TRACKERSET_BUTE_TAG))
			pButeMgr->LoadTrackerSet(szTagName);
		else if(!stricmp(bInfo->szTag, BASICTRACKER_BUTE_TAG))
			pButeMgr->LoadBasicTracker(szTagName);
		else if(!stricmp(bInfo->szTag, RANDOMTRACKER_BUTE_TAG))
			pButeMgr->LoadRandomTracker(szTagName);
		else if(!stricmp(bInfo->szTag, AIMINGTRACKER_BUTE_TAG))
			pButeMgr->LoadAimingTracker(szTagName);
		else if(!stricmp(bInfo->szTag, INDEXTRACKER_BUTE_TAG))
			pButeMgr->LoadIndexTracker(szTagName);
		else if(!stricmp(bInfo->szTag, TRANSITION_BUTE_TAG))
			pButeMgr->LoadTransition(szTagName);
	}

	return true;
}

//-------------------------------------------------------------------------------//

LTBOOL CAnimationButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pAnimationButeMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Reinit our count variables
	m_nNumAnimStyles = 0;
	m_nNumTrackerSets = 0;
	m_nNumBasicTrackers = 0;
	m_nNumRandomTrackers = 0;
	m_nNumAimingTrackers = 0;
	m_nNumIndexTrackers = 0;
	m_nNumTransitions = 0;


	// Set up global pointer...
	g_pAnimationButeMgr = this;


	// Create our arrays
	m_buteMgr.GetTags(CountTags, this);

	m_pAnimStyles = new AnimStyle[m_nNumAnimStyles];
	m_pTrackerSets = new TrackerSet[m_nNumTrackerSets];
	m_pBasicTrackers = new BasicTracker[m_nNumBasicTrackers];
	m_pRandomTrackers = new RandomTracker[m_nNumRandomTrackers];
	m_pAimingTrackers = new AimingTracker[m_nNumAimingTrackers];
	m_pIndexTrackers = new IndexTracker[m_nNumIndexTrackers];
	m_pTransitions = new Transition[m_nNumTransitions];


	// Fill in the attributes
	LoadButesInfo bInfo;
	bInfo.pButeMgr = this;

	// Load the basic trackers
	m_nNumBasicTrackers = 0;
	strcpy(bInfo.szTag, BASICTRACKER_BUTE_TAG);
	m_buteMgr.GetTags(LoadButes, &bInfo);

	// Load the random trackers
	m_nNumRandomTrackers = 0;
	strcpy(bInfo.szTag, RANDOMTRACKER_BUTE_TAG);
	m_buteMgr.GetTags(LoadButes, &bInfo);

	// Load the aiming trackers
	m_nNumAimingTrackers = 0;
	strcpy(bInfo.szTag, AIMINGTRACKER_BUTE_TAG);
	m_buteMgr.GetTags(LoadButes, &bInfo);

	// Load the index trackers
	m_nNumIndexTrackers = 0;
	strcpy(bInfo.szTag, INDEXTRACKER_BUTE_TAG);
	m_buteMgr.GetTags(LoadButes, &bInfo);

	// Load the tracker sets
	m_nNumTrackerSets = 0;
	strcpy(bInfo.szTag, TRACKERSET_BUTE_TAG);
	m_buteMgr.GetTags(LoadButes, &bInfo);

	// Load the anim styles
	m_nNumAnimStyles = 0;
	strcpy(bInfo.szTag, ANIMSTYLE_BUTE_TAG);
	m_buteMgr.GetTags(LoadButes, &bInfo);

	// Load the transition set information
	m_nNumTransitions = 0;
	strcpy(bInfo.szTag, TRANSITION_BUTE_TAG);
	m_buteMgr.GetTags(LoadButes, &bInfo);



	// Free up butemgr's memory and what-not.
	m_buteMgr.Term();

	return LTTRUE;
}


//*********************************************************************************
//
//	ROUTINE:	CAnimationButeMgr::Term()
//	PURPOSE:	Clean up.
//
//*********************************************************************************

void CAnimationButeMgr::Term()
{
	g_pAnimationButeMgr = LTNULL;

	// Delete the anim styles
	if(m_pAnimStyles)
	{
		for(int i = 0; i < m_nNumAnimStyles; i++)
		{
			if(m_pAnimStyles[i].szReference)
			{
				delete[] m_pAnimStyles[i].szReference;
				m_pAnimStyles[i].szReference = LTNULL;
			}

			if(m_pAnimStyles[i].szTrackerFrag)
			{
				delete[] m_pAnimStyles[i].szTrackerFrag;
				m_pAnimStyles[i].szTrackerFrag = LTNULL;
			}

			if(m_pAnimStyles[i].szAltTrackerFrag)
			{
				delete[] m_pAnimStyles[i].szAltTrackerFrag;
				m_pAnimStyles[i].szAltTrackerFrag = LTNULL;
			}
		}

		delete[] m_pAnimStyles;
		m_pAnimStyles = LTNULL;
		m_nNumAnimStyles = 0;
	}

	// Delete the tracker sets
	if(m_pTrackerSets)
	{
		delete[] m_pTrackerSets;
		m_pTrackerSets = LTNULL;
		m_nNumTrackerSets = 0;
	}

	// Delete the basic trackers
	if(m_pBasicTrackers)
	{
		delete[] m_pBasicTrackers;
		m_pBasicTrackers = LTNULL;
		m_nNumBasicTrackers = 0;
	}

	// Delete the random trackers
	if(m_pRandomTrackers)
	{
		for(int i = 0; i < m_nNumRandomTrackers; i++)
		{
			delete[] m_pRandomTrackers[i].szInitAnim;
			m_pRandomTrackers[i].szInitAnim = LTNULL;

			delete[] m_pRandomTrackers[i].szRandomAnim;
			m_pRandomTrackers[i].szRandomAnim = LTNULL;
		}

		delete[] m_pRandomTrackers;
		m_pRandomTrackers = LTNULL;
		m_nNumRandomTrackers = 0;
	}

	// Delete the aiming trackers
	if(m_pAimingTrackers)
	{
		delete[] m_pAimingTrackers;
		m_pAimingTrackers = LTNULL;
		m_nNumAimingTrackers = 0;
	}

	// Delete the index trackers
	if(m_pIndexTrackers)
	{
		for(int i = 0; i < m_nNumIndexTrackers; i++)
		{
			if(m_pIndexTrackers[i].nIndex)
			{
				delete[] m_pIndexTrackers[i].nIndex;
				m_pIndexTrackers[i].nIndex = LTNULL;
			}

			if(m_pIndexTrackers[i].szAnim)
			{
				delete[] m_pIndexTrackers[i].szAnim;
				m_pIndexTrackers[i].szAnim = LTNULL;
			}

			if(m_pIndexTrackers[i].szWeightSet)
			{
				delete[] m_pIndexTrackers[i].szWeightSet;
				m_pIndexTrackers[i].szWeightSet = LTNULL;
			}
		}

		delete[] m_pIndexTrackers;
		m_pIndexTrackers = LTNULL;
		m_nNumIndexTrackers = 0;
	}

	// Delete the transitions
	if(m_pTransitions)
	{
		delete[] m_pTransitions;
		m_pTransitions = LTNULL;
		m_nNumTransitions = 0;
	}
}


//*********************************************************************************
//
//	ROUTINE:	CAnimationButeMgr::GetAnimStyleIndex()
//	PURPOSE:	Find the AnimStyle with the name passed in and return the index
//
//*********************************************************************************

int CAnimationButeMgr::GetAnimStyleIndex(const char *szName)
{
	IndexTable::const_iterator found_style = m_AnimStyleTable.find(szName);

	if( found_style != m_AnimStyleTable.end() )
	{
		return found_style->second;
	}

	return ANIMATION_BUTE_MGR_INVALID;
}


//*********************************************************************************
//
//	ROUTINE:	CAnimationButeMgr::GetTrackerSetIndex()
//	PURPOSE:	Find the TrackerSet with the name passed in and return the index
//
//*********************************************************************************

int CAnimationButeMgr::GetTrackerSetIndex(const char *szName)
{
	IndexTable::const_iterator found_style = m_TrackerSetTable.find(szName);

	if( found_style != m_TrackerSetTable.end() )
	{
		return found_style->second;
	}

	return ANIMATION_BUTE_MGR_INVALID;
}


//*********************************************************************************
//
//	ROUTINE:	CAnimationButeMgr::GetTrackerSetFromLayers()
//	PURPOSE:	Calculates a tracker set name for the anim layers and gets the index
//
//*********************************************************************************

int CAnimationButeMgr::GetTrackerSetFromLayers(AnimStyleLayer *pLayer)
{
	// Make sure the layer is ok
	if(!pLayer) return ANIMATION_BUTE_MGR_INVALID;

	// Create the buffer of the tracker set name
	char szTrackerSet[ANIMATION_BUTES_STRING_SIZE];
	sprintf(szTrackerSet, "TrackerSet");

	AnimStyleLayer *pTempLayer = pLayer;
	char szTemp[ANIMATION_BUTES_STRING_SIZE];
	int nNumRefs;

	// Alternate frag variables
	char szAltTrackerSet[ANIMATION_BUTES_STRING_SIZE];
	char szAltTemp[ANIMATION_BUTES_STRING_SIZE];
	LTBOOL bAlt = LTFALSE;


	// Go through each layer and append to the tracker set name
	while(pTempLayer)
	{
		if((pLayer->nAnimStyle != ANIMATION_BUTE_MGR_INVALID) && pTempLayer->szReference[0])
		{
			nNumRefs = m_pAnimStyles[pTempLayer->nAnimStyle].nNumReference;
			szTemp[0] = 0;
			szAltTemp[0] = 0;

			// Find the reference for this layer and fill in the tracker frag
			for(int i = 0; i < nNumRefs; i++)
			{
				if(!_stricmp(m_pAnimStyles[pTempLayer->nAnimStyle].szReference[i], pTempLayer->szReference))
				{
					SAFE_STRCPY(szTemp, m_pAnimStyles[pTempLayer->nAnimStyle].szTrackerFrag[i]);
					SAFE_STRCPY(szAltTemp, m_pAnimStyles[pTempLayer->nAnimStyle].szAltTrackerFrag[i]);
					break;
				}
			}

			// If we couldn't find the reference... something is not setup right
			if(!szTemp[0])
				return ANIMATION_BUTE_MGR_INVALID;

			if(szAltTemp[0])
			{
				strcpy(szAltTrackerSet, szTrackerSet);
				sprintf(szTrackerSet, "%s_%s", szTrackerSet, szTemp);
				sprintf(szAltTrackerSet, "%s_%s", szAltTrackerSet, szAltTemp);
				bAlt = LTTRUE;
			}
			else
			{
				sprintf(szTrackerSet, "%s_%s", szTrackerSet, szTemp);
				sprintf(szAltTrackerSet, "%s_%s", szAltTrackerSet, szTemp);
			}
		}

		pTempLayer = pTempLayer->pNext;
	}


	// Return the appropriate index if it can find it
	int nTrackerSet = GetTrackerSetIndex(szTrackerSet);

	if(bAlt && (nTrackerSet == ANIMATION_BUTE_MGR_INVALID))
		nTrackerSet = GetTrackerSetIndex(szAltTrackerSet);

	return nTrackerSet;
}


//*********************************************************************************
//
//	ROUTINE:	CAnimationButeMgr::GetTransitionSet()
//	PURPOSE:	Gets the transition tracker set from one tracker set to another
//
//*********************************************************************************

int CAnimationButeMgr::GetTransitionSet(int nFrom, int nTo)
{
	for(int i = 0; i < m_nNumTransitions; i++)
	{
		if((m_pTransitions[i].nFromTrackerSet == nFrom) && (m_pTransitions[i].nToTrackerSet == nTo))
			return m_pTransitions[i].nTransTrackerSet;
	}

	return ANIMATION_BUTE_MGR_INVALID;
}


//*********************************************************************************
//
//	ROUTINE:	CAnimationButeMgr::CreateHashValue()
//	PURPOSE:	This is just hash value creation function to speed up searching
//
//*********************************************************************************

uint16 CAnimationButeMgr::CreateHashValue(const char *szStr)
{
	uint16 nIndex = -1;
	char *pChar = const_cast<char*>(szStr);

	while(*(pChar))
	{
		nIndex = (nIndex + 397) + *(pChar);
		++pChar;
	}

	return (nIndex % 0xFFFF);
}


//*********************************************************************************
//
//	ATTRIBUTE LOADING FUNCTIONS
//
//*********************************************************************************

void CAnimationButeMgr::LoadAnimStyle(const char *szTagName)
{
	// Get a pointer to the approriate bute
	AnimStyle *pButes = &m_pAnimStyles[m_nNumAnimStyles++];


	// Load in the butes
	SAFE_STRCPY(pButes->szName, szTagName);
	m_AnimStyleTable[pButes->szName] = (m_nNumAnimStyles - 1);


	// Count the number of references
	pButes->nNumReference = 0;
	pButes->szReference = LTNULL;
	pButes->szTrackerFrag = LTNULL;
	pButes->szAltTrackerFrag = LTNULL;

	sprintf(s_aAttribName, "%s%d", ANIMSTYLE_BUTE_REFERENCE, pButes->nNumReference);

	while(Exist(szTagName, s_aAttribName))
	{
		pButes->nNumReference++;
		sprintf(s_aAttribName, "%s%d", ANIMSTYLE_BUTE_REFERENCE, pButes->nNumReference);
	}

	// Create the array
	if(pButes->nNumReference > 0)
	{
		pButes->szReference = new char[pButes->nNumReference][ANIMATION_BUTES_STRING_SIZE];
		pButes->szTrackerFrag = new char[pButes->nNumReference][ANIMATION_BUTES_STRING_SIZE];
		pButes->szAltTrackerFrag = new char[pButes->nNumReference][ANIMATION_BUTES_STRING_SIZE];

		for(int i = 0; i < pButes->nNumReference; i++)
		{
			CString cStr;

			sprintf(s_aAttribName, "%s%d", ANIMSTYLE_BUTE_REFERENCE, i);
			cStr = GetString(szTagName, s_aAttribName);

			if(cStr.IsEmpty())
			{
				SAFE_STRCPY(pButes->szReference[i], "\0");
			}
			else
			{
				SAFE_STRCPY(pButes->szReference[i], cStr);
			}

			sprintf(s_aAttribName, "%s%d", ANIMSTYLE_BUTE_TRACKERSETFRAG, i);
			SAFE_STRCPY(pButes->szTrackerFrag[i], GetString(szTagName, s_aAttribName));

			sprintf(s_aAttribName, "%s%d", ANIMSTYLE_BUTE_ALTTRACKERSETFRAG, i);
			SAFE_STRCPY(pButes->szAltTrackerFrag[i], GetString(szTagName, s_aAttribName));
		}
	}
}

//-------------------------------------------------------------------------------//

void CAnimationButeMgr::LoadTrackerSet(const char *szTagName)
{
	// Get a pointer to the approriate bute
	TrackerSet *pButes = &m_pTrackerSets[m_nNumTrackerSets++];


	// Load in the butes
	SAFE_STRCPY(pButes->szName, szTagName);

	m_TrackerSetTable[pButes->szName] = (m_nNumTrackerSets - 1);


	// Load in the clear animation and tracker set
	SAFE_STRCPY(pButes->szClearAnim, GetString(szTagName, TRACKERSET_BUTE_CLEARANIM));
	SAFE_STRCPY(pButes->szClearWeightSet, GetString(szTagName, TRACKERSET_BUTE_CLEARWEIGHTSET));


	char szTemp[ANIMATION_BUTES_STRING_SIZE];

	for(int i = 0; i < ANIMATION_BUTE_MAX_TRACKERS; i++)
	{
		// Init the data to invalid...
		pButes->nTrackerTypes[i] = ANIMATION_BUTE_MGR_INVALID;
		pButes->nTrackerData[i] = ANIMATION_BUTE_MGR_INVALID;


		// Fill in the attribute name to search for
		sprintf(s_aAttribName, "%s%d", TRACKERSET_BUTE_TRACKERDATA, i);


		if(Exist(szTagName, s_aAttribName))
		{
			SAFE_STRCPY(szTemp, GetString(szTagName, s_aAttribName));

			// Search all the types of trackers
			if(!_strnicmp(szTemp, BASICTRACKER_BUTE_TAG, sizeof(BASICTRACKER_BUTE_TAG) - 1))
			{
				for(int j = 0; j < m_nNumBasicTrackers; j++)
				{
					if(!_stricmp(m_pBasicTrackers[j].szName, szTemp))
					{
						pButes->nTrackerTypes[i] = ANIMATION_BUTE_BASICTRACKER;
						pButes->nTrackerData[i] = j;
						break;
					}
				}
			}
			else if(!_strnicmp(szTemp, RANDOMTRACKER_BUTE_TAG, sizeof(RANDOMTRACKER_BUTE_TAG) - 1))
			{
				for(int j = 0; j < m_nNumRandomTrackers; j++)
				{
					if(!_stricmp(m_pRandomTrackers[j].szName, szTemp))
					{
						pButes->nTrackerTypes[i] = ANIMATION_BUTE_RANDOMTRACKER;
						pButes->nTrackerData[i] = j;
						break;
					}
				}
			}
			else if(!_strnicmp(szTemp, AIMINGTRACKER_BUTE_TAG, sizeof(AIMINGTRACKER_BUTE_TAG) - 1))
			{
				for(int j = 0; j < m_nNumAimingTrackers; j++)
				{
					if(!_stricmp(m_pAimingTrackers[j].szName, szTemp))
					{
						pButes->nTrackerTypes[i] = ANIMATION_BUTE_AIMINGTRACKER;
						pButes->nTrackerData[i] = j;
						break;
					}
				}
			}
			else if(!_strnicmp(szTemp, INDEXTRACKER_BUTE_TAG, sizeof(INDEXTRACKER_BUTE_TAG) - 1))
			{
				for(int j = 0; j < m_nNumIndexTrackers; j++)
				{
					if(!_stricmp(m_pIndexTrackers[j].szName, szTemp))
					{
						pButes->nTrackerTypes[i] = ANIMATION_BUTE_INDEXTRACKER;
						pButes->nTrackerData[i] = j;
						break;
					}
				}
			}

#ifdef _DEBUG
			// Report an error if we could not find the tracker set.
			if( pButes->nTrackerData[i] == ANIMATION_BUTE_MGR_INVALID )
			{
				if( g_pInterface )
				{
					g_pInterface->CPrint("AnimationButeMgr Error!!  %s refers to the unknown tracker set \"%s\".",
						szTagName, szTemp);
				}
			}
#endif
		}
	}


	pButes->bMovementAllowed = (LTBOOL)GetInt(szTagName, TRACKERSET_BUTE_MOVEMENTALLOWED);
}

//-------------------------------------------------------------------------------//

void CAnimationButeMgr::LoadBasicTracker(const char *szTagName)
{
	// Get a pointer to the approriate bute
	BasicTracker *pButes = &m_pBasicTrackers[m_nNumBasicTrackers++];


	// Load in the butes
	SAFE_STRCPY(pButes->szName, szTagName);


	SAFE_STRCPY(pButes->szAnim, GetString(szTagName, BASICTRACKER_BUTE_ANIM));
	SAFE_STRCPY(pButes->szWeightSet, GetString(szTagName, BASICTRACKER_BUTE_WEIGHTSET));
	pButes->fAnimSpeed = (LTFLOAT)GetDouble(szTagName, BASICTRACKER_BUTE_ANIMSPEED);
	pButes->nFlags = GetInt(szTagName, BASICTRACKER_BUTE_FLAGS);
}

//-------------------------------------------------------------------------------//

void CAnimationButeMgr::LoadRandomTracker(const char *szTagName)
{
	// Get a pointer to the approriate bute
	RandomTracker *pButes = &m_pRandomTrackers[m_nNumRandomTrackers++];


	// Load in the butes
	SAFE_STRCPY(pButes->szName, szTagName);


	SAFE_STRCPY(pButes->szWeightSet, GetString(szTagName, RANDOMTRACKER_BUTE_WEIGHTSET));
	pButes->fAnimSpeed = (LTFLOAT)GetDouble(szTagName, RANDOMTRACKER_BUTE_ANIMSPEED);
	pButes->nFlags = GetInt(szTagName, RANDOMTRACKER_BUTE_FLAGS);


	// Count the number of init animations
	pButes->nNumInitAnim = 0;
	pButes->szInitAnim = LTNULL;
	sprintf(s_aAttribName, "%s%d", RANDOMTRACKER_BUTE_INITANIM, pButes->nNumInitAnim);

	while(Exist(szTagName, s_aAttribName))
	{
		pButes->nNumInitAnim++;
		sprintf(s_aAttribName, "%s%d", RANDOMTRACKER_BUTE_INITANIM, pButes->nNumInitAnim);
	}


	// Create the array
	if(pButes->nNumInitAnim > 0)
	{
		pButes->szInitAnim = new char[pButes->nNumInitAnim][ANIMATION_BUTES_STRING_SIZE];

		for(int i = 0; i < pButes->nNumInitAnim; i++)
		{
			sprintf(s_aAttribName, "%s%d", RANDOMTRACKER_BUTE_INITANIM, i);
			SAFE_STRCPY(pButes->szInitAnim[i], GetString(szTagName, s_aAttribName));
		}
	}

	// Count the number of random animations
	pButes->nNumRandomAnim = 0;
	pButes->szRandomAnim = LTNULL;
	sprintf(s_aAttribName, "%s%d", RANDOMTRACKER_BUTE_RANDANIM, pButes->nNumRandomAnim);

	while(Exist(szTagName, s_aAttribName))
	{
		pButes->nNumRandomAnim++;
		sprintf(s_aAttribName, "%s%d", RANDOMTRACKER_BUTE_RANDANIM, pButes->nNumRandomAnim);
	}


	// Create the array
	if(pButes->nNumRandomAnim > 0)
	{
		pButes->szRandomAnim = new char[pButes->nNumRandomAnim][ANIMATION_BUTES_STRING_SIZE];

		for(int i = 0; i < pButes->nNumRandomAnim; i++)
		{
			sprintf(s_aAttribName, "%s%d", RANDOMTRACKER_BUTE_RANDANIM, i);
			SAFE_STRCPY(pButes->szRandomAnim[i], GetString(szTagName, s_aAttribName));
		}
	}


	pButes->fMinDelay = (LTFLOAT)GetDouble(szTagName, RANDOMTRACKER_BUTE_MINDELAY);
	pButes->fMaxDelay = (LTFLOAT)GetDouble(szTagName, RANDOMTRACKER_BUTE_MAXDELAY);
}

//-------------------------------------------------------------------------------//

void CAnimationButeMgr::LoadAimingTracker(const char *szTagName)
{
	// Get a pointer to the approriate bute
	AimingTracker *pButes = &m_pAimingTrackers[m_nNumAimingTrackers++];


	// Load in the butes
	SAFE_STRCPY(pButes->szName, szTagName);


	SAFE_STRCPY(pButes->szHighAnim, GetString(szTagName, AIMINGTRACKER_BUTE_HIGHANIM));
	SAFE_STRCPY(pButes->szLowAnim, GetString(szTagName, AIMINGTRACKER_BUTE_LOWANIM));
	SAFE_STRCPY(pButes->szWeightSet, GetString(szTagName, AIMINGTRACKER_BUTE_WEIGHTSET));
	pButes->nRange = GetInt(szTagName, AIMINGTRACKER_BUTE_RANGE);
	pButes->bAscending = (LTBOOL)GetInt(szTagName, AIMINGTRACKER_BUTE_ASCENDING);
	pButes->fAnimSpeed = (LTFLOAT)GetDouble(szTagName, AIMINGTRACKER_BUTE_ANIMSPEED);
	pButes->nFlags = GetInt(szTagName, AIMINGTRACKER_BUTE_FLAGS);
}

//-------------------------------------------------------------------------------//

void CAnimationButeMgr::LoadIndexTracker(const char *szTagName)
{
	// Get a pointer to the approriate bute
	IndexTracker *pButes = &m_pIndexTrackers[m_nNumIndexTrackers++];


	// Load in the butes
	SAFE_STRCPY(pButes->szName, szTagName);


	// Count the number of indexes
	pButes->nNumIndex = 0;
	pButes->nIndex = LTNULL;
	pButes->szAnim = LTNULL;
	pButes->szWeightSet = LTNULL;
	sprintf(s_aAttribName, "%s%d", INDEXTRACKER_BUTE_INDEX, pButes->nNumIndex);

	while(Exist(szTagName, s_aAttribName))
	{
		pButes->nNumIndex++;
		sprintf(s_aAttribName, "%s%d", INDEXTRACKER_BUTE_INDEX, pButes->nNumIndex);
	}


	// Create the array
	if(pButes->nNumIndex > 0)
	{
		pButes->nIndex = new int[pButes->nNumIndex];
		pButes->szAnim = new char[pButes->nNumIndex][ANIMATION_BUTES_STRING_SIZE];
		pButes->szWeightSet = new char[pButes->nNumIndex][ANIMATION_BUTES_STRING_SIZE];

		for(int i = 0; i < pButes->nNumIndex; i++)
		{
			sprintf(s_aAttribName, "%s%d", INDEXTRACKER_BUTE_INDEX, i);
			pButes->nIndex[i] = GetInt(szTagName, s_aAttribName);

			sprintf(s_aAttribName, "%s%d", INDEXTRACKER_BUTE_ANIM, i);
			SAFE_STRCPY(pButes->szAnim[i], GetString(szTagName, s_aAttribName));

			sprintf(s_aAttribName, "%s%d", INDEXTRACKER_BUTE_WEIGHTSET, i);
			SAFE_STRCPY(pButes->szWeightSet[i], GetString(szTagName, s_aAttribName));
		}
	}


	pButes->fAnimSpeed = (LTFLOAT)GetDouble(szTagName, INDEXTRACKER_BUTE_ANIMSPEED);
	pButes->nFlags = GetInt(szTagName, INDEXTRACKER_BUTE_FLAGS);
}

//-------------------------------------------------------------------------------//

void CAnimationButeMgr::LoadTransition(const char *szTagName)
{
	// Get a pointer to the approriate bute
	Transition *pButes = &m_pTransitions[m_nNumTransitions++];


	// Load in the butes
	SAFE_STRCPY(pButes->szName, szTagName);


	pButes->nFromTrackerSet = GetTrackerSetIndex(GetString(szTagName, TRANSITION_BUTE_FROM));
	pButes->nToTrackerSet = GetTrackerSetIndex(GetString(szTagName, TRANSITION_BUTE_TO));
	pButes->nTransTrackerSet = GetTrackerSetIndex(GetString(szTagName, TRANSITION_BUTE_TRANS));
}

