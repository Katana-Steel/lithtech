// ----------------------------------------------------------------------- //
//
// MODULE  : SharedMultiplayerDefs.h
//
// PURPOSE : Common definitions for multiplayer
//
// CREATED : 3/27/01
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_MULTIPLAYER_DEFS_H__
#define __SHARED_MULTIPLAYER_DEFS_H__

// ----------------------------------------------------------------------- //

#ifndef __GAME_TYPE_H__
#include "GameType.h"
#endif

// ----------------------------------------------------------------------- //

#ifdef _DEMO
	#define GAMESPY_GAME_NAME				"avp2demo"
	#define GAMESPY_SECRET_KEY				"Df3M6Z"
#else
	#define GAMESPY_GAME_NAME				"avp2"
	#define GAMESPY_SECRET_KEY				"Df3M6Z"
#endif

// ----------------------------------------------------------------------- //

#define MAX_CLIENTS						16
#define MAX_TEAMS						4

#define MAX_MP_CFG_NAME_LENGTH			64
#define MAX_MP_HOST_NAME_LENGTH			64
#define MAX_MP_CLIENT_NAME_LENGTH		32
#define MAX_MP_LEVEL_NAME_LENGTH		64
#define MAX_MP_SCORE_NAME_LENGTH		32
#define MAX_MP_PASSWORD_LENGTH			16

#define MAX_MP_LEVELS					32
#define MAX_MP_SCORING_CLASSES			32

// ----------------------------------------------------------------------- //

#define	MP_END_CONDITION_FRAGS			0x01
#define	MP_END_CONDITION_SCORE			0x02
#define	MP_END_CONDITION_TIME			0x04
#define MP_END_CONDITION_ROUNDS			0x08

#define MP_END_CONDITION_ALL			0xFF

// ----------------------------------------------------------------------- //

#define MP_CLIENT_UPDATE_CLIENTDATA		0x01
#define MP_CLIENT_UPDATE_PING			0x02
#define MP_CLIENT_UPDATE_FRAGS			0x04
#define MP_CLIENT_UPDATE_SCORE			0x08
#define MP_CLIENT_UPDATE_CONFRAGS		0x10
#define MP_CLIENT_UPDATE_LIVES			0x20

#define MP_CLIENT_UPDATE_ALL			0xFF

// ----------------------------------------------------------------------- //
// Messages from the server

#define MP_SERV_MSG_KILLED_HIMSELF		1
#define MP_SERV_MSG_KILLED_SOMEONE		2
#define MP_SERV_MSG_KILLED_TEAMMATE		3

#define MP_PICKUP_MSG_CANT_USE_WEAPON	10
#define MP_PICKUP_MSG_CANT_USE_AMMO		11

#define MP_SERVER_CONTROLLER			20
#define MP_SERVER_CONTROLLER_PW_INVAL	21
#define MP_SERVER_COMMAND_INVALID		22
#define MP_SERVER_CONTROLLER_NOT_DEDICATED	23
#define MP_SERVER_CONTROLLER_REDUND		24
#define MP_SERVER_QUEEN_SP_ERROR		25

#define MP_SC_INVALID					26
#define MP_SC_BOUNDS					27

#define MP_CLIENT_RENAMED				30
#define MP_AUTO_TEAM_SWITCH				31
#define MP_TEAM_NOT_AVAILABLE			32
#define MP_SUBSTATE_CHANGED				33
#define MP_CLIENT_CHEATING				34
#define MP_NAME_CHANGE					35

#define MP_HUNT_MUTATE_SWITCH			40
#define MP_HUNT_MUTATE_HUNTER			41
#define MP_HUNT_MUTATE_PREY				42

#define MP_SURVIVOR_MUTATE				50
#define MP_SURVIVOR_INTERM_TIME			51

#define MP_EVAC_COUNTDOWN				60
#define MP_EVAC_COMPLETE				61
#define MP_EVAC_CANCEL					62
#define MP_EVAC_STATE					63

#define MP_QUEEN_MUTATE					70
#define MP_QUEEN_DEATH					71

#define MP_SC_ALIEN_LIFECYCLE			72
#define MP_SC_QUEEN						73
#define MP_SC_EXOSUIT					74
#define MP_SC_PRED_MASK					75
#define MP_SC_FF						76
#define MP_SC_LOC_DAM					77
#define MP_SC_CLASS_WEAPONS				78
#define MP_SC_FN						79

// ----------------------------------------------------------------------- //
// Observing states

#define MP_OBSERVE_ON					0
#define MP_OBSERVE_OFF					1

#define MP_OBSERVE_PREV					10
#define MP_OBSERVE_NEXT					11
#define MP_OBSERVE_RANDOM				12

#define MP_OBSERVE_POINT				20

#define MP_OBSERVE_CHANGE_MODE			30

#define MP_OBSERVE_MODE_NORMAL			0
#define MP_OBSERVE_MODE_START_GAME		1
#define MP_OBSERVE_MODE_WAIT_START_GAME	2
#define MP_OBSERVE_MODE_WAIT_NEXT_ROUND	3
#define MP_OBSERVE_MODE_WAIT_TEAMS		4

// ----------------------------------------------------------------------- //
// Server states

#define MPMGR_STATE_PLAYING				0
#define MPMGR_STATE_NEXT_LEVEL_VERIFY	1
#define MPMGR_STATE_LOAD_LEVEL_VERIFY	2

// Sub states

#define MPMGR_SUBSTATE_NONE				0

#define MPMGR_SUBSTATE_SURVIVOR_TAG		10
#define MPMGR_SUBSTATE_SURVIVOR_CUT		11
#define MPMGR_SUBSTATE_SURVIVOR_ROUND	12
#define MPMGR_SUBSTATE_SURVIVOR_INTERM	13

#define MPMGR_SUBSTATE_TM_GOING			20
#define MPMGR_SUBSTATE_TM_ROUND			21

// ----------------------------------------------------------------------- //
// Next level states

#define MPMGR_NEXT_LEVEL_RESET			0
#define MPMGR_NEXT_LEVEL_IN_PROGRESS	1
#define MPMGR_NEXT_LEVEL_ACCEPTED		2

// ----------------------------------------------------------------------- //
// Misc gameplay values

#define MPMGR_SURVIVOR_INTERM_TIME		6.0f

// ----------------------------------------------------------------------- //
// Default IP port for multiplayer games.

#define DEFAULT_PORT					27888
#define DEFAULT_BANDWIDTH				16000

// ----------------------------------------------------------------------- //

class MPGameInfo
{
	public:

		MPGameInfo()
		{
			memset(this, 0, sizeof(MPGameInfo));
			strcpy(m_szName, "Aliens vs. Predator 2");
			strcpy(m_szPassword, "AVP2");
			strcpy(m_szSCPassword, "AVP2");
			m_nPort = DEFAULT_PORT;
			m_nBandwidth = DEFAULT_BANDWIDTH;

			m_fSpeedScale = 1.0f;
			m_fPowerupRespawnScale = 1.0f;
			m_fDamageScale = 1.0f;

			m_bLocationDamage = LTTRUE;
			m_bFriendlyFire = LTTRUE;
			m_bFriendlyNames = LTFALSE;
			m_bMaskLoss = LTTRUE;
			m_bClassWeapons = LTTRUE;
			m_nExosuit = 0;
			m_nQueenMolt = 0;
		}

		// Basic game information
		char		m_szName[MAX_MP_HOST_NAME_LENGTH];		// The name of the server
		char		m_szPassword[MAX_MP_PASSWORD_LENGTH];	// The game password
		char		m_szSCPassword[MAX_MP_PASSWORD_LENGTH];	// The server control password
		GameType	m_nGameType;							// The current game type
		LTBOOL		m_bLocked;								// Use the password?
		LTBOOL		m_bSCLocked;							// Use the server control password?
		LTBOOL		m_bDedicated;							// Is it a dedicated server?
		LTBOOL		m_bLANConnection;						// Is this purely a LAN connection game?
		uint16		m_nPort;								// Port to put the game on.
		uint32		m_nBandwidth;							// The max server bandwidth setting


		// Game options
		uint8		m_nEndConditionFlags;					// The conditions for switching levels

		uint8		m_nPlayerLimit;							// Max number of players in the game
		uint8		m_nTeamLimits[MAX_TEAMS];				// Max number of players on each team

		uint8		m_nFragLimit;							// The max frags before a level switch
		uint16		m_nScoreLimit;							// The max score before a level switch
		LTFLOAT		m_fTimeLimit;							// The max time (in seconds) before a level switch
		uint8		m_nRoundLimit;							// The max number of rounds to play

		LTBOOL		m_bLifecycle;							// Is the alien lifecycle option on?


		// Level information
		char		m_szLevels[MAX_MP_LEVELS][MAX_MP_LEVEL_NAME_LENGTH];
		uint8		m_nNumLevels;							// The number of levels in the list


		// Game type specific options
		uint8		m_nHunt_HunterRace;						// The race that the Hunters play
		uint8		m_nHunt_PreyRace;						// The race that the Prey play
		uint8		m_nHunt_Ratio;							// The ratio of hunters to prey

		uint8		m_nSurv_SurvivorRace;					// The race that the Survivors play
		uint8		m_nSurv_MutateRace;						// The race that the Mutants play

		uint8		m_nTM_DefenderRace;						// The race that the Defending team plays
		uint8		m_nTM_DefenderLives;					// The number of lives the Defenders get
		uint8		m_nTM_AttackerRace;						// The race that the Attacking team plays
		uint8		m_nTM_AttackerLives;					// The number of lives the Attackers get


		// Advanced options
		LTFLOAT		m_fSpeedScale;
		LTFLOAT		m_fPowerupRespawnScale;
		LTFLOAT		m_fDamageScale;

		LTBOOL		m_bLocationDamage;
		LTBOOL		m_bFriendlyFire;
		LTBOOL		m_bFriendlyNames;
		LTBOOL		m_bMaskLoss;
		LTBOOL		m_bClassWeapons;
		uint8		m_nExosuit;
		uint8		m_nQueenMolt;


		// Scoring
		uint32		m_nScoreClasses[MAX_MP_SCORING_CLASSES];
		uint8		m_nScoreValues[MAX_MP_SCORING_CLASSES];
};

// ----------------------------------------------------------------------- //

class MPClientData
{
	public:

		MPClientData()
		{
			memset(this, 0, sizeof(MPClientData));
			strcpy(m_szName, "Player");
			strcpy(m_szPassword, "AVP2");
		}

		char		m_szName[MAX_MP_CLIENT_NAME_LENGTH];	// The name the client has typed in for his player setup
		char		m_szPassword[MAX_MP_PASSWORD_LENGTH];	// The password used to try and join the game

		uint8		m_nCharacterClass;						// The character class (marine, corporate, alien, predator)
		uint8		m_nCharacterSet[MAX_TEAMS];				// The character set index from CharacterBute.txt
		LTBOOL		m_bIgnoreTaunts;						// Does this client wish to ignore taunts?
		LTBOOL		m_bServerControl;						// Does this client have server control?

		// Internal use only...
		LTBOOL		m_bAutoTeamSwitched;					// Did this player get switched to another team automatically?
};

// ----------------------------------------------------------------------- //

// Game guid.
extern DGUID AVP2GUID;

// Message ID's sent from the servershell to the serverapp.
enum ServerApp
{
	SERVERAPP_ADDCLIENT,
	SERVERAPP_REMOVECLIENT,
	SERVERAPP_SHELLUPDATE,
	SERVERAPP_PRELOADWORLD,
	SERVERAPP_POSTLOADWORLD,
	SERVERAPP_CONSOLEMESSAGE,
};

// Message ID's sent from the serverapp to the servershell.
enum ServerShell
{
	SERVERSHELL_INIT,
	SERVERSHELL_NEXTWORLD,
	SERVERSHELL_SETWORLD,
	SERVERSHELL_MESSAGE,
};


#endif

