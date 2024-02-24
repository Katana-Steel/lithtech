//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Retrieves attributes from the SpawnButes.txt
//*********************************************************************************
// File:		SpawnButeMgr.cpp
// Created:		Aug 28, 2000
// Updated:		Aug 28, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#include "stdafx.h"
#include "SpawnButeMgr.h"

//*********************************************************************************

#define SPAWN_BUTES_TAG						"SpawnObject"

#define SPAWN_BUTE_NAME						"Name"
#define SPAWN_BUTE_CLASS					"ClassName"
#define SPAWN_BUTE_PROPS					"Props"
#define SPAWN_BUTE_CACHE_CHARACTER			"CacheCharacter"
#define SPAWN_BUTE_CACHE_WEAPON				"CacheWeapons"

//*********************************************************************************

CSpawnButeMgr* g_pSpawnButeMgr = LTNULL;

//*********************************************************************************

static char s_aTagName[30];
static char s_aAttName[30];

//*********************************************************************************
// Construction/Destruction
//*********************************************************************************

CSpawnButeMgr::CSpawnButeMgr()
{
	m_nNumSpawnButes = 0;
	m_pSpawnButes = LTNULL;
}

CSpawnButeMgr::~CSpawnButeMgr()
{
	Term();
}

//*********************************************************************************
//
//	ROUTINE:	CSpawnButeMgr::Init()
//	PURPOSE:	Init mgr
//
//*********************************************************************************

LTBOOL CSpawnButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pSpawnButeMgr || !szAttributeFile) return LTFALSE;
	if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...
	g_pSpawnButeMgr = this;


	// Calculate the number of spawn attribute sets
	m_nNumSpawnButes = 0;
	sprintf(s_aTagName, "%s0", SPAWN_BUTES_TAG);

	while(m_buteMgr.Exist(s_aTagName))
	{
		m_nNumSpawnButes++;
		sprintf(s_aTagName, "%s%d", SPAWN_BUTES_TAG, m_nNumSpawnButes);
	}

	// Create the spawn attribute list
	m_pSpawnButes = new SpawnButes[m_nNumSpawnButes];
	ASSERT( m_pSpawnButes );
	if( m_pSpawnButes )
	{
		for( int i = 0; i < m_nNumSpawnButes; ++i )
		{
			LoadSpawnButes(i, m_pSpawnButes[i]);
		}
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

	return LTTRUE;
}


//*********************************************************************************
//
//	ROUTINE:	CSpawnButeMgr::Term()
//	PURPOSE:	Clean up.
//
//*********************************************************************************

void CSpawnButeMgr::Term()
{
	g_pSpawnButeMgr = LTNULL;

	if(m_pSpawnButes)
	{
		delete[] m_pSpawnButes;
		m_pSpawnButes = LTNULL;
	}
}

//*********************************************************************************

int CSpawnButeMgr::GetSpawnButesFromName(char *szName) const
{
	// Find out which character we're looking for
	for(int i = 0; i < m_nNumSpawnButes; i++)
	{
		if(!stricmp(szName, m_pSpawnButes[i].m_szName))
			return i;
	}

#ifndef _FINAL
	g_pInterface->CPrint("SpawnButeMgr: Could not find set %s! Defaulting to index zero!", szName);
#endif

	return -1;
}

//*********************************************************************************

const SpawnButes & CSpawnButeMgr::GetSpawnButes(char *szName) const
{
	int nSet = GetSpawnButesFromName(szName);

	ASSERT( nSet >= 0 && nSet < m_nNumSpawnButes );

	return m_pSpawnButes[nSet];
}

//*********************************************************************************

void CSpawnButeMgr::LoadSpawnButes(int nSet, SpawnButes &butes)
{
	SAFE_STRCPY(butes.m_szName, GetStringAttrib(nSet, SPAWN_BUTE_NAME));
	butes.m_szClass = GetStringAttrib(nSet, SPAWN_BUTE_CLASS);

	butes.m_szCharacterClass = GetStringAttrib(nSet, SPAWN_BUTE_CACHE_CHARACTER, "");
	butes.m_szCharacterClass = GetStringAttrib(nSet, SPAWN_BUTE_CACHE_WEAPON, "");

	int nProps = 0;
	CString szTemp;

	sprintf(s_aTagName, "%s%d", SPAWN_BUTES_TAG, nSet);
	sprintf(s_aAttName, "%s%d", SPAWN_BUTE_PROPS, nProps);

	while(m_buteMgr.Exist(s_aTagName, s_aAttName))
	{
		szTemp = GetStringAttrib(nSet, s_aAttName);

		butes.m_szProps += szTemp;
		butes.m_szProps += "; ";

		nProps++;
		sprintf(s_aAttName, "%s%d", SPAWN_BUTE_PROPS, nProps);
	}
}

//*********************************************************************************

CString CSpawnButeMgr::GetStringAttrib(int nSet, const char *szAttrib)
{
	if(m_nNumSpawnButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", SPAWN_BUTES_TAG, nSet);
	return GetString(s_aTagName, szAttrib);
}

CString CSpawnButeMgr::GetStringAttrib(int nSet, const char *szAttrib, const char * szDefault)
{
	if(m_nNumSpawnButes <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", SPAWN_BUTES_TAG, nSet);
	return GetString(s_aTagName, szAttrib, szDefault);
}

//*********************************************************************************
//
// CSpawnButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
//*********************************************************************************

#ifndef _CLIENTBUILD  // Server-side only

// Plugin statics

LTBOOL CSpawnButeMgrPlugin::sm_bInitted = LTFALSE;
CSpawnButeMgr CSpawnButeMgrPlugin::sm_ButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpawnButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT	CSpawnButeMgrPlugin::PreHook_EditStringList(
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
		if(!g_pSpawnButeMgr)
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, SPAWN_BUTES_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, SPAWN_BUTES_DEFAULT_FILE);
#endif

			sm_ButeMgr.SetInRezFile(LTFALSE);
			sm_ButeMgr.Init(g_pLTServer, szFile);
			sm_bInitted = LTTRUE;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpawnButeMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

void CSpawnButeMgrPlugin::PopulateStringList(char* const * aszStrings, uint32* pcStrings, 
	const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return;

	// Add an entry for each weapon...

	uint32 dwNumSets = sm_ButeMgr.GetNumSpawnButes();
	SpawnButes puButes;

	for(uint32 i = 0; i < dwNumSets; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		puButes = sm_ButeMgr.GetSpawnButes(i);
		uint32 dwNameLen = strlen(puButes.m_szName);

		if(puButes.m_szName[0] && (dwNameLen < cMaxStringLength) && ((*pcStrings) + 1 < cMaxStrings))
		{
			// Just use the pickup name...
			strcpy(*aszStrings, puButes.m_szName);
			++aszStrings;
			++(*pcStrings);
		}
	}
}


#endif //_CLIENTBUILD
