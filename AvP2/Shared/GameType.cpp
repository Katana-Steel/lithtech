//*********************************************************************************
//---------------------------------------------------------------------------------
// Project:		Alien vs. Predator 2
// Purpose:		
//---------------------------------------------------------------------------------
// File:		GameType.cpp
// Created:		Jun. 5, 2000
// Updated:		Jun. 5, 2000
// Author:		Andy Mattingly
//---------------------------------------------------------------------------------
//*********************************************************************************

#include "stdafx.h"
#include "GameType.h"

//*********************************************************************************
// These are used for DEdit property values

const int	g_nPropGameTypes = 10;
char		*g_pPropGameTypes[g_nPropGameTypes] =
{
	"ALL",
	"(SP) ALL",
	"(SP) STORY",
	"(MP) ALL",
	"(MP) DM",
	"(MP) TEAMDM",
	"(MP) HUNT",
	"(MP) SURVIVOR",
	"(MP) OVERRUN",
	"(MP) EVAC",
};

int			g_pPropGameTypesEnum[g_nPropGameTypes] =
{
	GT_ALL,
	SP_ALL,
	SP_STORY,
	MP_ALL,
	MP_DM,
	MP_TEAMDM,
	MP_HUNT,
	MP_SURVIVOR,
	MP_OVERRUN,
	MP_EVAC,
};

//---------------------------------------------------------------------------------

const int	g_nDescDifficulty = 3;
char		*g_pDescDifficulty[g_nDescDifficulty] =
{
	"PC - Easy",
	"PC - Medium",
	"PC - Hard",
};

//*********************************************************************************
// These are used for server game description

const int	g_nDescGameTypes = 7;
char		*g_pDescGameTypes[g_nDescGameTypes] =
{
	"Story Mode",
	"DM",
	"Team DM",
	"Hunt",
	"Survivor",
	"Overrun",
	"Evac",
};

int			g_pDescGameTypesEnum[g_nDescGameTypes] =
{
	SP_STORY,
	MP_DM,
	MP_TEAMDM,
	MP_HUNT,
	MP_SURVIVOR,
	MP_OVERRUN,
	MP_EVAC,
};

//*********************************************************************************

t_GameType::t_GameType()
{
	m_nType = SP_STORY;
	m_nDifficulty = DIFFICULTY_PC_MEDIUM;
	m_bStartMission = LTFALSE;
}

//*********************************************************************************

t_GameType::t_GameType(int nType, int nDiff)
{
	if((nType < 0) || (nType >= g_nDescGameTypes))
		m_nType = SP_STORY;
	else
		m_nType = nType;

	if((nDiff < 0) || (nDiff >= g_nDescDifficulty))
		m_nDifficulty = DIFFICULTY_PC_MEDIUM;
	else
		m_nDifficulty = nDiff;
}

//*********************************************************************************

LTBOOL t_GameType::IsSinglePlayer()
{
	if(m_nType == SP_STORY)			return LTTRUE;

	return LTFALSE;
}

//*********************************************************************************

LTBOOL t_GameType::IsMultiplayer()
{
	if(m_nType == MP_DM)			return LTTRUE;
	if(m_nType == MP_TEAMDM)		return LTTRUE;
	if(m_nType == MP_HUNT)			return LTTRUE;
	if(m_nType == MP_SURVIVOR)		return LTTRUE;
	if(m_nType == MP_OVERRUN)		return LTTRUE;
	if(m_nType == MP_EVAC)			return LTTRUE;

	return LTFALSE;
}

//*********************************************************************************

LTBOOL t_GameType::IsTeamMode()
{
	if(m_nType == MP_TEAMDM)		return LTTRUE;
	if(m_nType == MP_OVERRUN)		return LTTRUE;
	if(m_nType == MP_EVAC)			return LTTRUE;

	return LTFALSE;
}

//*********************************************************************************

void t_GameType::GetDescription(char *szDesc, int nSize)
{
	// Make sure the buffer is valid
	if(!szDesc || (nSize < 1)) return;

	// Make sure the current type is within bounds
	if((m_nType < 0) || (m_nType >= g_nDescGameTypes))
	{
		strcpy(szDesc, "Unknown");
		return;
	}

	// Copy the description string
	strncpy(szDesc, g_pDescGameTypes[m_nType], nSize);
}

//*********************************************************************************

LTBOOL t_GameType::SetTypeFromProp(char *szProp)
{
	// Make sure the string is valid
	if(!szProp) return LTFALSE;

	// Go through each description and see if it's equal
	for(int i = 0; i < g_nPropGameTypes; i++)
	{
		if(_stricmp(g_pPropGameTypes[i], szProp) == 0)
		{
			m_nType = g_pPropGameTypesEnum[i];
			return LTTRUE;
		}
	}

	return LTFALSE;
}

//*********************************************************************************

LTBOOL t_GameType::SetTypeFromDesc(char *szDesc)
{
	// Make sure the string is valid
	if(!szDesc) return LTFALSE;

	// Go through each description and see if it's equal
	for(int i = 0; i < g_nDescGameTypes; i++)
	{
		if(_stricmp(g_pDescGameTypes[i], szDesc) == 0)
		{
			m_nType = g_pDescGameTypesEnum[i];
			return LTTRUE;
		}
	}

	return LTFALSE;
}
