//*********************************************************************************
//*********************************************************************************
// Project:		Alien vs. Predator 2
// Purpose:		Retrieves attributes from the ObjectiveButes.txt
//*********************************************************************************
// File:		SoundButeMgr.cpp
// Created:		December 21, 2000
// Updated:		December 21, 2000
// Author:		Andy Kaplan
//*********************************************************************************
//*********************************************************************************

#include "stdafx.h"
#include "ObjectivesButeMgr.h"

//*********************************************************************************

#define OBJECTIVE_BUTES_TAG				"Objective"

#define OBJECTIVE_BUTE_NAME				"Name"
#define OBJECTIVE_BUTE_STRING			"DisplayedString"

//*********************************************************************************

CObjectiveButeMgr* g_pObjectiveButeMgr = LTNULL;

//*********************************************************************************

static char s_aTagName[30];

//*********************************************************************************
// Construction/Destruction
//*********************************************************************************

CObjectiveButeMgr::CObjectiveButeMgr()
{
	m_nNumObjectiveButes = 0;
	m_pObjectiveButes = LTNULL;
}

CObjectiveButeMgr::~CObjectiveButeMgr()
{
	Term();
}

//*********************************************************************************
//
//	ROUTINE:	CObjectiveButeMgr::Init()
//	PURPOSE:	Init Objective mgr
//
//*********************************************************************************

LTBOOL CObjectiveButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pObjectiveButeMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...
	g_pObjectiveButeMgr = this;

	// Calculate the number of pickup attribute sets
	m_nNumObjectiveButes = 0;
	sprintf(s_aTagName, "%s0", OBJECTIVE_BUTES_TAG);

	while(Exist(s_aTagName))
	{
		m_nNumObjectiveButes++;
		sprintf(s_aTagName, "%s%d", OBJECTIVE_BUTES_TAG, m_nNumObjectiveButes);
	}

	// Create the sound attribute list
	m_pObjectiveButes = new ObjectiveButes[m_nNumObjectiveButes];
	ASSERT( m_pObjectiveButes );
	if( m_pObjectiveButes )
	{
		for( int i = 0; i < m_nNumObjectiveButes; ++i )
		{
			LoadObjectiveButes(i, m_pObjectiveButes[i]);
		}
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

	return LTTRUE;
}


//*********************************************************************************
//
//	ROUTINE:	CObjectiveButeMgr::Term()
//	PURPOSE:	Clean up.
//
//*********************************************************************************

void CObjectiveButeMgr::Term()
{
	g_pObjectiveButeMgr = LTNULL;

	if(m_pObjectiveButes)
	{
		delete[] m_pObjectiveButes;
		m_pObjectiveButes = LTNULL;
	}
}

//*********************************************************************************

int CObjectiveButeMgr::GetObjectiveIdFromName(const char *szName) const
{
	// Find out which character we're looking for
	for(int i = 0; i < m_nNumObjectiveButes; i++)
	{
		if(!stricmp(szName, m_pObjectiveButes[i].m_szName))
			return i;
	}

	g_pInterface->CPrint("ObjectiveButeMgr: Could not find set %s! Defaulting to index zero!", szName);

	return -1;
}

//*********************************************************************************

const ObjectiveButes & CObjectiveButeMgr::GetObjectiveButes(const char *szName) const
{
	int nSet = GetObjectiveIdFromName(szName);

	if( nSet >= 0 && nSet < m_nNumObjectiveButes )
		return m_pObjectiveButes[nSet];

	return m_pObjectiveButes[0];
}

//*********************************************************************************

void CObjectiveButeMgr::LoadObjectiveButes(int nSet, ObjectiveButes &butes)
{
	SAFE_STRCPY(butes.m_szParent, GetStringAttrib(nSet, "Parent"));
	SAFE_STRCPY(butes.m_szName, GetStringAttrib(nSet, OBJECTIVE_BUTE_NAME));
	butes.m_nObjectiveText = GetIntAttrib(nSet, OBJECTIVE_BUTE_STRING);
}

//*********************************************************************************

CString CObjectiveButeMgr::GetStringAttrib(int nSet, char *szAttrib)
{
	if(m_nNumObjectiveButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", OBJECTIVE_BUTES_TAG, nSet);
	return GetString(s_aTagName, szAttrib);
}

//*********************************************************************************

DVector CObjectiveButeMgr::GetVectorAttrib(int nSet, char *szAttrib)
{
	if(m_nNumObjectiveButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", OBJECTIVE_BUTES_TAG, nSet);
	return GetVector(s_aTagName, szAttrib);
}

//*********************************************************************************

CPoint CObjectiveButeMgr::GetPointAttrib(int nSet, char *szAttrib)
{
	if(m_nNumObjectiveButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", OBJECTIVE_BUTES_TAG, nSet);
	return GetPoint(s_aTagName, szAttrib);
}

//*********************************************************************************

int CObjectiveButeMgr::GetIntAttrib(int nSet, char *szAttrib)
{
	if(m_nNumObjectiveButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", OBJECTIVE_BUTES_TAG, nSet);
	return GetInt(s_aTagName, szAttrib);
}

//*********************************************************************************

double CObjectiveButeMgr::GetDoubleAttrib(int nSet, char *szAttrib)
{
	if(m_nNumObjectiveButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", OBJECTIVE_BUTES_TAG, nSet);
	return GetDouble(s_aTagName, szAttrib);
}

//*********************************************************************************
//
// CObjectiveButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use ObjectiveMgr
//
//*********************************************************************************

#ifndef _CLIENTBUILD  // Server-side only

// Plugin statics

DBOOL CObjectiveButeMgrPlugin::sm_bInitted = DFALSE;
CObjectiveButeMgr CObjectiveButeMgrPlugin::sm_ButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT	CObjectiveButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if (!sm_bInitted)
	{
		char szFile[256];

		// Create the bute mgr if necessary
		if(!g_pObjectiveButeMgr)
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, OBJECTIVE_BUTES_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, OBJECTIVE_BUTES_DEFAULT_FILE);
#endif
			sm_ButeMgr.SetInRezFile(LTFALSE);
			sm_ButeMgr.Init(g_pLTServer, szFile);
			sm_bInitted = LTTRUE;
		}
	}

	// Add an entry for each objective...
	uint32 nNumObjectives = g_pObjectiveButeMgr->GetNumObjectiveButes();

	typedef std::vector<std::string> StringList;
	StringList List;

	for (uint32 i = 0; i < nNumObjectives; i++)
	{
		List.push_back(g_pObjectiveButeMgr->GetObjectiveName(i));
	}
	std::sort(List.begin(), List.end());

	for( StringList::iterator iter = List.begin(); iter != List.end(); ++iter )
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		if((*pcStrings) + 1 < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], (*iter).c_str());
		}
	}
	return LT_UNSUPPORTED;
}


#endif //_CLIENTBUILD
