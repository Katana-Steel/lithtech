// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.cpp
//
// PURPOSE : MissionMgr implementation - Controls attributes of all 
//			 missions
//
// CREATED : 07/26/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MissionMgr.h"
#include "CommonUtilities.h"

#define MISSION_BUTES_TAG				"Mission"

#define	MISSION_BUTE_NAME				"Name"
#define	MISSION_BUTE_DESCRIPTION		"NextDesc"
#define	MISSION_BUTE_NEXT_LEVEL			"NextLevel"
#define	MISSION_BUTE_RESET_HEALTH_ARMOR	"ResetArmorAndHealth"
#define	MISSION_BUTE_FADE_TIME			"ScreenFadeTime"


const char* const CAMPAIGN_TAGS[3]		=
{
	"CampaignAlien",
	"CampaignMarine",
	"CampaignPredator",
};

#define	CAMPAIGN_BUTE_NAME				"MissionName"
#define	CAMPAIGN_BUTE_DESC				"MissionDesc"
#define	CAMPAIGN_BUTE_LEVEL				"MissionStart"

static char s_aTagName[30];
static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];


void MISSIONSTART::Init(CButeMgr & buteMgr, char* aTagName, int nNum)
{
	sprintf(s_aAttName,"%s%d",CAMPAIGN_BUTE_NAME,nNum);
	m_nNameId = buteMgr.GetInt(aTagName, s_aAttName, 0);

	sprintf(s_aAttName,"%s%d",CAMPAIGN_BUTE_DESC,nNum);
	m_nDescId = buteMgr.GetInt(aTagName, s_aAttName, 0);

	sprintf(s_aAttName,"%s%d",CAMPAIGN_BUTE_LEVEL,nNum);
	SAFE_STRCPY(m_szWorld, buteMgr.GetString(s_aTagName, s_aAttName, ""));
}


void CAMPAIGN::Init(CButeMgr & buteMgr, char* aTagName)
{
	m_nNumMissions = 0;
	sprintf(s_aAttName,"%s%d",CAMPAIGN_BUTE_LEVEL,m_nNumMissions);
	while (m_nNumMissions < MAX_CAMPAIGN_MISSIONS && buteMgr.Exist(aTagName,s_aAttName))
	{
		m_Missions[m_nNumMissions].Init(buteMgr,aTagName,m_nNumMissions);
		m_nNumMissions++;
		sprintf(s_aAttName,"%s%d",CAMPAIGN_BUTE_LEVEL,m_nNumMissions);
	}

}


CMissionMgr* g_pMissionMgr = DNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::CMissionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMissionMgr::CMissionMgr()
{
	m_nNumSets	= 0;;
	m_pButeSets	= LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::~CMissionMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CMissionMgr::~CMissionMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

DBOOL CMissionMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pMissionMgr || !szAttributeFile) return DFALSE;
	if (!Parse(pInterface, szAttributeFile)) return DFALSE;

	// Set up global pointer...

	g_pMissionMgr = this;

	// Calculate the number of mission attribute sets
	m_nNumSets = 0;
	sprintf(s_aTagName, "%s0", MISSION_BUTES_TAG);

	while(m_buteMgr.Exist(s_aTagName))
	{
		m_nNumSets++;
		sprintf(s_aTagName, "%s%d", MISSION_BUTES_TAG, m_nNumSets);
	}

	m_pButeSets = new MissionButes[m_nNumSets];
	ASSERT( m_pButeSets );
	if( m_pButeSets )
	{
		for( int i = 0; i < m_nNumSets; ++i )
		{
			LoadMissionButes(i, m_pButeSets[i]);
			m_pButeSets[i].m_nId = i;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		strcpy(s_aTagName,CAMPAIGN_TAGS[i]);
		m_Campaigns[i].Init(m_buteMgr,s_aTagName);
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CMissionMgr::Term()
{
	g_pMissionMgr = DNULL;
	
	if(m_pButeSets)
	{
		delete[] m_pButeSets;
		m_pButeSets = LTNULL;
	}
}

//*********************************************************************************

void CMissionMgr::LoadMissionButes(int nSet, MissionButes &butes)
{
	SAFE_STRCPY(butes.m_szName,			GetStringAttrib(nSet, MISSION_BUTE_NAME));

	butes.m_nNextDesc = GetIntAttrib(nSet, MISSION_BUTE_DESCRIPTION);

	SAFE_STRCPY(butes.m_szNextLevel,	GetStringAttrib(nSet, MISSION_BUTE_NEXT_LEVEL));

	butes.m_bResetHealthAndArmor = (DBOOL)GetIntAttrib(nSet, MISSION_BUTE_RESET_HEALTH_ARMOR);

	butes.m_fScreenFadeTime = (LTFLOAT)GetDoubleAttrib(nSet, MISSION_BUTE_FADE_TIME);
}


//*********************************************************************************

const MissionButes & CMissionMgr::GetMissionButes(char *szName) const
{
	int nSet = GetSetFromName(szName);

	ASSERT( nSet >= 0 && nSet < m_nNumSets );

	if(nSet != -1)
		return m_pButeSets[nSet];

	return m_pButeSets[0];
}

//*********************************************************************************

int CMissionMgr::GetSetFromName(char *szName) const
{
	// Find out which character we're looking for
	for(int i = 0; i < m_nNumSets; i++)
	{
		if(!stricmp(szName, GetMissionName(i)))
			return i;
	}

	g_pInterface->CPrint("CMissionMgr: Could not find set %s! Defaulting to index zero!", szName);

	return 0;
}

//*********************************************************************************

int CMissionMgr::GetSetFromNextWorld(char *szNextWorld) const
{
	// Find out which character we're looking for
	for(int i = 0; i < m_nNumSets; i++)
	{
		if(!stricmp(szNextWorld, GetNextLevelName(i)))
			return i;
	}

	return -1;
}

//*********************************************************************************

CString CMissionMgr::GetStringAttrib(int nSet, char *szAttrib)
{
	if(m_nNumSets <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", MISSION_BUTES_TAG, nSet);
	return m_buteMgr.GetString(s_aTagName, szAttrib);
}

//*********************************************************************************

DVector CMissionMgr::GetVectorAttrib(int nSet, char *szAttrib)
{
	if(m_nNumSets <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", MISSION_BUTES_TAG, nSet);
	return m_buteMgr.GetVector(s_aTagName, szAttrib);
}

//*********************************************************************************

CPoint CMissionMgr::GetPointAttrib(int nSet, char *szAttrib)
{
	if(m_nNumSets <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", MISSION_BUTES_TAG, nSet);
	return m_buteMgr.GetPoint(s_aTagName, szAttrib);
}

//*********************************************************************************

int CMissionMgr::GetIntAttrib(int nSet, char *szAttrib)
{
	if(m_nNumSets <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", MISSION_BUTES_TAG, nSet);
	return m_buteMgr.GetInt(s_aTagName, szAttrib);
}

//*********************************************************************************

double CMissionMgr::GetDoubleAttrib(int nSet, char *szAttrib)
{
	if(m_nNumSets <= nSet || nSet < 0)
		nSet = 0;

	sprintf(s_aTagName, "%s%d", MISSION_BUTES_TAG, nSet);
	return m_buteMgr.GetDouble(s_aTagName, szAttrib);
}

//*********************************************************************************
//
// CMissionButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use MissionMgr
//
//*********************************************************************************

#ifndef _CLIENTBUILD  // Server-side only

// Plugin statics

DBOOL CMissionButeMgrPlugin::sm_bInitted = DFALSE;
CMissionMgr CMissionButeMgrPlugin::sm_ButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT	CMissionButeMgrPlugin::PreHook_EditStringList(
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
		if(!g_pMissionMgr)
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, MISSION_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, MISSION_DEFAULT_FILE);
#endif
			sm_ButeMgr.SetInRezFile(LTFALSE);
			sm_ButeMgr.Init(g_pLTServer, szFile);
			sm_bInitted = LTTRUE;
		}
	}

	// Add an entry for each character...
	uint32 nNumMissions = g_pMissionMgr->GetNumSets();

	for (uint32 i = 0; i < nNumMissions; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		if((*pcStrings) + 1 < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], g_pMissionMgr->GetMissionName(i).GetBuffer(0));
		}
	}

	return LT_UNSUPPORTED;
}


#endif //_CLIENTBUILD
