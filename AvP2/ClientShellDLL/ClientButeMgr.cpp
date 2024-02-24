// ----------------------------------------------------------------------- //
//
// MODULE  : ClientButeMgr.cpp
//
// PURPOSE : ClientButeMgr implementation - Client-side attributes
//
// CREATED : 2/02/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientButeMgr.h"

#define CBMGR_CHEATS_TAG				"Cheats"
#define CBMGR_CHEATS_ATTRIBUTE_NAME		"Cheat"

#define CBMGR_GAME_TAG					"Game"
#define CBMGR_CAMERA_TAG				"Camera"
#define CBMGR_LEVEL_TAG					"Level"
#define CBMGR_REVERB_TAG				"Reverb"
#define CBMGR_WEATHER_TAG				"Weather"
#define CBMGR_INTERFACE_TAG				"Interface"

#define CBMGR_DEBUGKEYS_TAG				 "DebugKey"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_NAME	 "Name"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_KEY	 "Key"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_STRING "String"
#define CBMGR_DEBUGKEYS_ATTRIBUTE_TITLE  "Title"

#define CBMGR_REVERB_TAG				"Reverb"


#define CBMGR_ALIENAURA_TAG	"AlienAura"

#define CBMGR_PREDCOLOR_KEY "PredatorColor"
#define CBMGR_ALIENCOLOR_KEY "AlienColor"
#define CBMGR_HUMANCOLOR_KEY "HumanColor"
#define CBMGR_SELHUMANCOLOR_KEY "SelHumanColor"

#define CBMGR_DEADPREDCOLOR_KEY "DeadPredatorColor"
#define CBMGR_DEADALIENCOLOR_KEY "DeadAlienColor"
#define CBMGR_DEADHUMANCOLOR_KEY "DeadHumanColor"
#define CBMGR_DEADSELHUMANCOLOR_KEY "DeadSelHumanColor"

#define CBMGR_FUNCTIONKEYS_TAG "FunctionKeys"

#define CBMGR_QUICKSAVE_KEY		"QuickSave"
#define CBMGR_QUICKLOAD_KEY		"QuickLoad"
#define CBMGR_CINEMATICSKIP_KEY	"CinematicSkip"
#define CBMGR_ESCAPE_KEY		"Escape"
#define CBMGR_PAUSE_KEY			"Pause"

static char s_aTagName[30];
static char s_aAttName[100];

CClientButeMgr* g_pClientButeMgr = DNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::CClientButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CClientButeMgr::CClientButeMgr()
{
	m_nNumCheatAttributes	= 0;
	m_nNumLevels			= 0;
	m_nNumDebugKeys			= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::~CClientButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientButeMgr::~CClientButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

DBOOL CClientButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	if (g_pClientButeMgr || !szAttributeFile) return DFALSE;
	if (!Parse(pInterface, szAttributeFile)) return DFALSE;


	// Set up global pointer...

	g_pClientButeMgr = this;


	// Determine how many cheat attributes there are...

	m_nNumCheatAttributes = 0;
	sprintf(s_aAttName, "%s%d", CBMGR_CHEATS_ATTRIBUTE_NAME, m_nNumCheatAttributes);

	while (m_buteMgr.Exist(CBMGR_CHEATS_TAG, s_aAttName))
	{
		m_nNumCheatAttributes++;
		sprintf(s_aAttName, "%s%d", CBMGR_CHEATS_ATTRIBUTE_NAME, m_nNumCheatAttributes);
	}

	// Determine how many levels there are...

	m_nNumLevels = 0;
	sprintf(s_aTagName, "%s%d", CBMGR_LEVEL_TAG, m_nNumLevels);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_nNumLevels++;
		sprintf(s_aTagName, "%s%d", CBMGR_LEVEL_TAG, m_nNumLevels);
	}

	// Determine how many debug key attributes there are...

	m_nNumDebugKeys = 0;
	sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, m_nNumDebugKeys);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_nNumDebugKeys++;
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, m_nNumDebugKeys);
	}

	m_aNumDebugLevels = DebugLevels(m_nNumDebugKeys,0);
	for( DebugLevels::iterator iter = m_aNumDebugLevels.begin(); iter != m_aNumDebugLevels.end(); ++iter )
	{
		const int nDebugKey = (iter - m_aNumDebugLevels.begin());
		int & nNumLevels = *iter;

		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugKey );
		sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_STRING, nNumLevels );

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			nNumLevels++;
			sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_STRING, nNumLevels );
		}
	}

	m_vPredAuraColor = m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_PREDCOLOR_KEY);
	m_vAlienAuraColor = m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_ALIENCOLOR_KEY);
	m_vHumanAuraColor =  m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_HUMANCOLOR_KEY);
	m_vSelHumanAuraColor =  m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_SELHUMANCOLOR_KEY);

	m_vDeadPredAuraColor = m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_DEADPREDCOLOR_KEY);
	m_vDeadAlienAuraColor = m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_DEADALIENCOLOR_KEY);
	m_vDeadHumanAuraColor = m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_DEADHUMANCOLOR_KEY);
	m_vDeadSelHumanAuraColor = m_buteMgr.GetVector(CBMGR_ALIENAURA_TAG, CBMGR_DEADSELHUMANCOLOR_KEY);


	m_nQuickSaveKey = m_buteMgr.GetInt(CBMGR_FUNCTIONKEYS_TAG, CBMGR_QUICKSAVE_KEY, VK_F6);
	m_nQuickLoadKey = m_buteMgr.GetInt(CBMGR_FUNCTIONKEYS_TAG, CBMGR_QUICKLOAD_KEY, VK_F7);
	m_nCinematicSkipKey = m_buteMgr.GetInt(CBMGR_FUNCTIONKEYS_TAG, CBMGR_CINEMATICSKIP_KEY, VK_SPACE);
	m_nEscapeKey = m_buteMgr.GetInt(CBMGR_FUNCTIONKEYS_TAG, CBMGR_ESCAPE_KEY, VK_ESCAPE);
	m_nPauseKey = m_buteMgr.GetInt(CBMGR_FUNCTIONKEYS_TAG, CBMGR_PAUSE_KEY, VK_PAUSE);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CClientButeMgr::Term()
{
	g_pClientButeMgr = DNULL;
}


/////////////////////////////////////////////////////////////////////////////
//
//	C H E A T  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCheat()
//
//	PURPOSE:	Get the cheat specified by the number
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetCheat(DBYTE nCheatNum)
{
	CString str;
	if (nCheatNum < m_nNumCheatAttributes)
	{
		sprintf(s_aAttName, "%s%d", CBMGR_CHEATS_ATTRIBUTE_NAME, nCheatNum);
		str = m_buteMgr.GetString(CBMGR_CHEATS_TAG, s_aAttName);
	}

	return str;
}


/////////////////////////////////////////////////////////////////////////////
//
//	D E B U G Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetNumDebugLevels()
//
//	PURPOSE:	Get the number of debug levels for this key.
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetNumDebugLevels(DBYTE nDebugNum) const
{
	if( nDebugNum < m_aNumDebugLevels.size() )
	{
		return m_aNumDebugLevels[nDebugNum];
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugName()
//
//	PURPOSE:	Get the key associated with this debug level
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetDebugName(DBYTE nDebugNum)
{
	CString str;
	if (nDebugNum < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugNum);
		str = m_buteMgr.GetString(s_aTagName, CBMGR_DEBUGKEYS_ATTRIBUTE_NAME);
	}

	return str;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugKey()
//
//	PURPOSE:	Get the key associated with this debug level
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetDebugKey(DBYTE nDebugNum)
{
	int nResult = -1;

	if (nDebugNum < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugNum);
		nResult = m_buteMgr.GetInt(s_aTagName, CBMGR_DEBUGKEYS_ATTRIBUTE_KEY, -1 );
	}

	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugString()
//
//	PURPOSE:	Get the console string associated with this debug level
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetDebugString(DBYTE nDebugNum, DBYTE nDebugLevel)
{
	CString str;

	if (nDebugNum < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugNum);

		if( nDebugLevel < GetNumDebugLevels(nDebugNum) )
		{
			sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_STRING, nDebugLevel);
			str = m_buteMgr.GetString(s_aTagName, s_aAttName);
		}
	}

	return str;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetDebugTitle()
//
//	PURPOSE:	Get the message string associated with this debug level
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetDebugTitle(DBYTE nDebugNum, DBYTE nDebugLevel)
{
	CString str;

	if (nDebugNum < m_nNumDebugKeys)
	{
		sprintf(s_aTagName, "%s%d", CBMGR_DEBUGKEYS_TAG, nDebugNum);

		if( nDebugLevel < GetNumDebugLevels(nDebugNum) )
		{
			sprintf(s_aAttName, "%s%d", CBMGR_DEBUGKEYS_ATTRIBUTE_TITLE, nDebugLevel);
			str = m_buteMgr.GetString(s_aTagName, s_aAttName, CString(""));
		}
	}

	return str;
}

/////////////////////////////////////////////////////////////////////////////
//
//	G A M E  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetGameAttributeInt()
//
//	PURPOSE:	Get a game attribute as an int
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetGameAttributeInt(char* pAttribute)
{
	if (!pAttribute) return 0;

	return m_buteMgr.GetInt(CBMGR_GAME_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetGameAttributeFloat()
//
//	PURPOSE:	Get a game attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetGameAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_GAME_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetGameAttributeString()
//
//	PURPOSE:	Get a game attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetGameAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_GAME_TAG, pAttribute);
}


/////////////////////////////////////////////////////////////////////////////
//
//	C A M E R A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCameraAttributeInt()
//
//	PURPOSE:	Get a camera attribute as an int
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetCameraAttributeInt(char* pAttribute)
{
	if (!pAttribute) return 0;

	return m_buteMgr.GetInt(CBMGR_CAMERA_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCameraAttributeFloat()
//
//	PURPOSE:	Get a camera attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetCameraAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_CAMERA_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetCameraAttributeString()
//
//	PURPOSE:	Get a camera attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetCameraAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_CAMERA_TAG, pAttribute);
}


/////////////////////////////////////////////////////////////////////////////
//
//	L E V E L  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetLevelAttributeInt()
//
//	PURPOSE:	Get a level attribute as an int
//
// ----------------------------------------------------------------------- //

int CClientButeMgr::GetLevelAttributeInt(DBYTE nLevelNum, char* pAttribute)
{
	if (nLevelNum < 0 || nLevelNum > m_nNumLevels || !pAttribute) return 0;

	sprintf(s_aTagName, "%s%d", CBMGR_LEVEL_TAG, nLevelNum);
	return m_buteMgr.GetInt(s_aTagName, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetLevelAttributeFloat()
//
//	PURPOSE:	Get a level attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetLevelAttributeFloat(DBYTE nLevelNum, char* pAttribute)
{
	if (nLevelNum < 0 || nLevelNum > m_nNumLevels || !pAttribute) return 0.0f;

	sprintf(s_aTagName, "%s%d", CBMGR_LEVEL_TAG, nLevelNum);
	return (float) m_buteMgr.GetDouble(s_aTagName, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetLevelAttributeString()
//
//	PURPOSE:	Get a level attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetLevelAttributeString(DBYTE nLevelNum, char* pAttribute)
{
	CString str;
	if (nLevelNum < 0 || nLevelNum > m_nNumLevels || !pAttribute) return str;

	sprintf(s_aTagName, "%s%d", CBMGR_LEVEL_TAG, nLevelNum);
	return m_buteMgr.GetString(s_aTagName, pAttribute);
}



/////////////////////////////////////////////////////////////////////////////
//
//	R E V E R B  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetReverbAttributeFloat()
//
//	PURPOSE:	Get a reverb attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetReverbAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_REVERB_TAG, pAttribute);
}




/////////////////////////////////////////////////////////////////////////////
//
//	W E A T H E R  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetWeatherAttributeFloat()
//
//	PURPOSE:	Get a weather attribute as a float
//
// ----------------------------------------------------------------------- //

float CClientButeMgr::GetWeatherAttributeFloat(char* pAttribute)
{
	if (!pAttribute) return 0.0f;

	return (float) m_buteMgr.GetDouble(CBMGR_WEATHER_TAG, pAttribute);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetWeatherAttributeString()
//
//	PURPOSE:	Get a weather attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetWeatherAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_WEATHER_TAG, pAttribute);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientButeMgr::GetInterfaceAttributeString()
//
//	PURPOSE:	Get a interface attribute as a string
//
// ----------------------------------------------------------------------- //

CString CClientButeMgr::GetInterfaceAttributeString(char* pAttribute)
{
	CString str;
	if (!pAttribute) return str;

	return m_buteMgr.GetString(CBMGR_INTERFACE_TAG, pAttribute);
}
