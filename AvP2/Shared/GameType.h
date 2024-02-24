//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		
//*********************************************************************************
// File:		GameType.h
// Created:		Jun. 5, 2000
// Updated:		Jun. 5, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#ifndef __GAME_TYPE_H__
#define __GAME_TYPE_H__

//*********************************************************************************

#include "ltbasedefs.h"

//*********************************************************************************

enum
{
	MP_ALL = -3,
	SP_ALL = -2,
	GT_ALL = -1,
	SP_STORY = 0,
	MP_DM,
	MP_TEAMDM,
	MP_HUNT,
	MP_SURVIVOR,
	MP_OVERRUN,
	MP_EVAC,
};

//*********************************************************************************

enum
{
	DIFFICULTY_PC_EASY = 0,
	DIFFICULTY_PC_MEDIUM,
	DIFFICULTY_PC_HARD,
	DIFFICULTY_PC_INSANE
};

//*********************************************************************************

typedef struct t_GameType
{
	public:
		// Construction functions
		t_GameType::t_GameType();
		t_GameType::t_GameType(int nType, int nDiff = DIFFICULTY_PC_MEDIUM);

		// Operator overloads
		LTBOOL		operator == (const int t)			const { return (m_nType == t); }
		LTBOOL		operator != (const int t)			const { return (m_nType != t); }
		LTBOOL		operator == (const t_GameType t)	const { return (m_nType == t.Get()); }
		LTBOOL		operator != (const t_GameType t)	const { return (m_nType != t.Get()); }

		// Set the type
		int			Get()								const { return m_nType; }
		void		Set(int t)							{ m_nType = t; }

		// Set the difficulty
		int			GetDifficulty()						const { return m_nDifficulty; }
		void		SetDifficulty(int d)				{ m_nDifficulty = d; }

		// Determine if this is from a start mission.
		LTBOOL		IsStartMission() const { return m_bStartMission; }
		void		SetStartMission(LTBOOL bVal) { m_bStartMission = bVal; }

		// Helper functions to determine game type
		LTBOOL		IsSinglePlayer();
		LTBOOL		IsMultiplayer();
		LTBOOL		IsTeamMode();



		// Helper functions to retrieve string descriptions
		void		GetDescription(char *szDesc, int nSize);

		// Helper functions to set the game type
		LTBOOL		SetTypeFromProp(char *szProp);
		LTBOOL		SetTypeFromDesc(char *szDesc);

	private:
		// Data members
		int			m_nType;
		int			m_nDifficulty;
		LTBOOL		m_bStartMission;

}	GameType;

//*********************************************************************************

extern const int	g_nPropGameTypes;
extern char			*g_pPropGameTypes[];

extern const int	g_nDescGameTypes;
extern char			*g_pDescGameTypes[];
extern int			g_pDescGameTypesEnum[];

extern const int	g_nDescDifficulty;
extern char			*g_pDescDifficulty[];

//*********************************************************************************

#endif //__GAME_TYPE_H__
