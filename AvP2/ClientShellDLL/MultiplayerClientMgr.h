// -------------------------------------------------------------------- //
//
// MODULE  : MultiplayerClientMgr.h
//
// PURPOSE : A client-side multiplayer statistics manager
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //

#ifndef __MULTIPLAYER_CLIENT_MGR_H__
#define __MULTIPLAYER_CLIENT_MGR_H__

// -------------------------------------------------------------------- //

#include "MultiplayerMgrDefs.h"
#include "MPClientMgrChart.h"
#include "ClientRes.h"

// -------------------------------------------------------------------- //

#define NUM_STATS_STRINGS			(IDS_MP_STAT_MAX - IDS_MP_STAT_MIN - 1)

enum eServerRules
{
	SR_GAME_SETTINGS = 0,
	SR_MAX_PLAYERS,
	SR_MAX_MARINES,
	SR_MAX_ALIENS,
	SR_MAX_PREDATORS,
	SR_MAX_CORPORATES,
	SR_FRAG_LIMIT,
	SR_SCORE_LIMIT,
	SR_TIME_LIMIT,
	SR_ROUND_LIMIT,
	SR_LIFECYCLE,
	SR_HUNTER_RACE,
	SR_PREY_RACE,
	SR_HUNT_RATIO,
	SR_SURVIVOR_RACE,
	SR_MUTATE_RACE,
	SR_DEFENDER_RACE,
	SR_DEFENDER_LIVES,
	SR_ATTACKER_RACE,
	SR_ATTACKER_LIVES,
	SR_ROUND_TIME,
	SR_EVAC_RACE,
	SR_EVAC_LIVES,
	SR_EVAC_TIME,
	SR_ADV_SETTINGS,
	SR_DAMAGE_RATE,
	SR_LOC_DAMAGE,
	SR_FRIENDLY_FIRE,
	SR_FRIENDLY_NAMES,
	SR_PRED_MASK_LOSS,
	SR_CLASS_WEAPONS,
	SR_EXOSUIT,
	SR_QUEEN,
	SR_CUSTOM_SCORING,
	SR_OPT_RESPAWN,
	SR_OPT_RUNSPEED,
	SR_BANDWIDTH,
	SR_MAX_ITEMS
};

// -------------------------------------------------------------------- //

struct MPClientMgrData
{
	MPClientMgrData()
	{
		memset(this, 0, sizeof(MPClientMgrData));
	}

	char			m_szName[MAX_MP_CLIENT_NAME_LENGTH];				// The name of this client
	uint8			m_nCharacterClass;
	uint8			m_nCharacterSet;

	uint32			m_nClientID;										// The Lithtech client ID value

	uint16			m_nPing;											// The current ping of this client
	int16			m_nFrags;											// The current number of frags this client has
	int8			m_nConsecutiveFrags;								// The number of consecutive frags this client has
	int16			m_nScore;											// The current score for this client

	int8			m_nLives;											// The number of lives this client has

	LTBOOL			m_bVerified;										// Has this client verified?

	LTBOOL			m_bObserving;										// Is this client in observe mode?
	uint8			m_nObservePoint;									// The current observe point
	uint8			m_nObserveMode;										// The current observe mode
};

// -------------------------------------------------------------------- //

struct MPClientMgrTeamData
{
	MPClientMgrTeamData()
	{
		memset(this, 0, sizeof(MPClientMgrTeamData));
	}

	int16			m_nFrags;											// The current number of frags this client has
	int16			m_nScore;											// The current score for this client
};

// -------------------------------------------------------------------- //

class MultiplayerClientMgr
{
	public:

		// -------------------------------------------------------------------- //
		// Construction and destruction

		MultiplayerClientMgr();
		~MultiplayerClientMgr();

		LTBOOL		Init();
		void		Term();


		// -------------------------------------------------------------------- //
		// Client control functions

		LTBOOL		AddClient(uint32 nClientID, MPClientMgrData &pData);
		LTBOOL		RemoveClient(uint32 nClientID);

		int			FindClient(uint32 nClientID);
		uint32		FindClientID(char *szName);

		void		ClearClientData();
		void		ClearTeamData();
		LTBOOL		ClearVerification();
		uint8		GetCharacterClass(uint32 nClientId);


		// -------------------------------------------------------------------- //
		// Message control functions
		LTBOOL		OnPeerToPeerAuth(uint32 nClientID, HMESSAGEREAD hRead);
		LTBOOL		OnMessage(uint8 nMessage, HMESSAGEREAD hRead);

		LTBOOL		OnMouseMove(int x, int y);
		LTBOOL		OnLButtonDown(int x, int y);

		LTBOOL		OnCommandOn(int command);
		LTBOOL		OnCommandOff(int command);

		LTBOOL		OnKeyDown(int key, int rep);
		LTBOOL		OnKeyUp(int nKey);


		// -------------------------------------------------------------------- //
		// Manager control functions

		LTBOOL		Update();

		void		ShowStatsDisplay(LTBOOL bOn);
		void		ShowRulesDisplay(LTBOOL bOn);
		void		ShowPlayerDisplay(LTBOOL bOn, int command);
		void		ShowSummaryDisplay(LTBOOL bOn);
		void		ShowRoundDisplay(LTBOOL bOn);
		void		ShowLoadingDisplay(LTBOOL bOn);
		void		ShowObserveDisplay(LTBOOL bOn);
		void		ShowClipModeDisplay(LTBOOL bOn);

		void		SetInfoString(const char *szString, HLTCOLOR hColor = SETRGB(96, 0, 0));
		void		SetInfoString(uint32 nClientID);

		void		SetWarningString(const char *szString, LTBOOL bForce = LTFALSE, LTFLOAT fDelay = 5.0f, HLTCOLOR hColor = SETRGB(128, 128, 128));

		LTBOOL		SetObservePoint(uint8 nObserveState);


		// -------------------------------------------------------------------- //
		// Data retrieval functions

		uint8		GetFragLimit()					{ return m_nFragLimit; }
		uint16		GetScoreLimit()					{ return m_nScoreLimit; }
		uint16		GetTimeRemaining()				{ return m_nTimeRemaining; }

		LTFLOAT		GetSpeedScale()					{ return m_fSpeedScale; }

		LTBOOL		AllowMovementControl()			{ return (!m_bPlayerDisplay && !m_bSummaryDisplay && !m_bRoundDisplay && !m_bLoadingDisplay && !m_bObserveDisplay && !m_bClipModeDisplay && ! m_bRulesDisplay); }
		LTBOOL		AllowClipMovementControl()		{ return (m_bClipModeDisplay && !m_bPlayerDisplay); }
		LTBOOL		AllowMouseControl()				{ return (!m_bPlayerDisplay && !m_bObserveDisplay); }
		LTBOOL		IsRulesDisplay()				{ return m_bRulesDisplay; }

		LTBOOL		IsPlayerDisplay() const			{ return m_bPlayerDisplay; }
		LTBOOL		IsLoadingDisplay() const		{ return m_bLoadingDisplay; }
		LTBOOL		IsClassWeapons()				{ return m_bClassWeapons; }
		uint8		GetMaxExosuitsAllowed()			{ return m_nExosuit; }
		uint8		GetQueenMoltLimit()				{ return m_nQueenMolt; }
		LTBOOL		ShowFriendlyNamesOnly()			{ return m_bFriendlyNames; }
		uint32		GetServerBandwidth()			{ return m_nBandwidth; }


		// -------------------------------------------------------------------- //
		// Display control functions

		void		ResetStatCharts();

		// -------------------------------------------------------------------- //
		// General Utilities

		LTBOOL		IsFriendly(uint32 nClientID);

	private:


		// -------------------------------------------------------------------- //
		// Client helper functions

		void		GetSortedClientList(int *nList, int nListSize, int nFlags);
		void		GetSortedTeamList(int *nList, int nListSize, int nFlags);


		// -------------------------------------------------------------------- //
		// Stat display functions

		void		Draw_Background(LTBOOL bNoDraw = LTFALSE);

		void		Draw_Header(uint8 nHeaderFlags);
		void		Draw_Header_Loading();

		void		Draw_Stats_DM(LTIntPt pt, uint8 nHeaderFlags, LTBOOL bDrawChecks = LTFALSE);
		void		Draw_Stats_TeamDM(LTIntPt pt, uint8 nHeaderFlags);
		void		Draw_Stats_Round();
		void 		Draw_Server_Rules(LTIntPt pt);

		void		Draw_Footer_Summary();

		void		Draw_Player_Info(LTIntPt pt);
		void		Draw_Player_Selection();

		void		Draw_Observe_Selection();
		void		Draw_ClipMode_Selection();
		void		Draw_Warning_String();


		// -------------------------------------------------------------------- //
		// Misc update functions

		void		UpdateDisplay(LTBOOL bOn);
		void		UpdateScreenResolution();
		void		UpdateColorCycle();

		void		UpdateLeaders();
		void		UpdateLeaders_DM();
		void		UpdateLeaders_TeamDM();

		void		HandleSubState(uint8 nSubState);

		void		UpdateEvacOverlay();


		// -------------------------------------------------------------------- //
		// Message helper functions

		LTBOOL		OnMessage_GameInfo(uint8 nMessage, HMESSAGEREAD hRead);
		LTBOOL		OnMessage_UpdateClient(uint8 nMessage, HMESSAGEREAD hRead);
		LTBOOL		OnMessage_UpdateTeam(uint8 nMessage, HMESSAGEREAD hRead);
		LTBOOL		OnMessage_Message(uint8 nMessage, HMESSAGEREAD hRead);
		LTBOOL		OnMessage_ServerMessage(uint8 nMessage, HMESSAGEREAD hRead);
		LTBOOL		OnMessage_NextLevel(uint8 nMessage, HMESSAGEREAD hRead);
		LTBOOL		OnMessage_ChangePlayer(uint8 nMessage, HMESSAGEREAD hRead);


		// -------------------------------------------------------------------- //
		// Misc setup functions

		void		BuildClassLists();
		void		InitRulesStrings();

	private:

		// The data that we don't want people screwing around with directly

		char			m_szServer[MAX_MP_HOST_NAME_LENGTH];			// The name of the server
		char			m_szLevel[MAX_MP_LEVEL_NAME_LENGTH];			// The name of the current level

		uint8			m_nPlayerLimit;									// Max number of players in the game
		uint8			m_nTeamLimits[MAX_TEAMS];						// Max number of players on each team

		uint8			m_nFragLimit;									// The server frag limit
		uint16			m_nScoreLimit;									// The server score limit
		uint16			m_nTimeRemaining;								// The server time remaining
		uint8			m_nRoundLimit;									// The server round limit

		uint16			m_nTimeLimit;									// The server time limit
		uint8			m_nHunt_HunterRace;								// The race that the Hunters play
		uint8			m_nHunt_PreyRace;								// The race that the Prey play
		uint8			m_nHunt_Ratio;									// The ratio of hunters to prey
																		
		uint8			m_nSurv_SurvivorRace;							// The race that the Survivors play
		uint8			m_nSurv_MutateRace;								// The race that the Mutants play
																		
		uint8			m_nTM_DefenderRace;								// The race that the Defending team plays
		uint8			m_nTM_DefenderLives;							// The number of lives the Defenders get
		uint8			m_nTM_AttackerRace;								// The race that the Attacking team plays
		uint8			m_nTM_AttackerLives;							// The number of lives the Attackers get

		LTBOOL			m_bLifecycle;									// Is the alien lifecycle option on?

		LTFLOAT			m_fPowerupRespawnScale;							// Powerup spawn scale
		LTFLOAT			m_fDamageScale;									// Damage scale

		LTBOOL			m_bLocationDamage;								// Location damage?
		LTBOOL			m_bFriendlyFire;								// Friendly fire?
		LTBOOL			m_bFriendlyNames;								// Display only friendly names?
		LTBOOL			m_bMaskLoss;									// Can pred lose his mask?
		LTBOOL			m_bCustScoring;									// Is there custom scoring?		
		
		LTFLOAT			m_fSpeedScale;									// The scale value for player run speeds
		LTBOOL			m_bClassWeapons;								// Are class weapons enabled?
		uint8			m_nExosuit;										// Max number of Exosuits allowed?
		uint8			m_nQueenMolt;									// Number of kills to molt into a queen

		uint32			m_nBandwidth;									// The server bandwidth setting

		uint8			m_nClients;										// The current number of clients
		MPClientMgrData	m_ClientData[MAX_CLIENTS];						// The data for each client
		MPClientMgrTeamData	m_TeamData[MAX_TEAMS];						// The data for each team

		int				m_nFragLeader;									// The client who leads in frags
		int				m_nScoreLeader;									// The client who leads in score

		int				m_nTeamFragLeader;								// The team who leads in frags
		int				m_nTeamScoreLeader;								// The team who leads in score

		uint32			m_nServerController;							// The client ID who controls the server

		uint8			m_nCurrentRound;								// The current round of the game (ding ding ding!)


		// Data members for interface display

		char			m_szNextLevel[MAX_MP_LEVEL_NAME_LENGTH];		// The name of the next level

		HSURFACE		m_hBackground;									// The background surface
		HSURFACE		m_hCheckOn;										// A check mark bitmap
		HSURFACE		m_hCheckOff;									// A check mark bitmap
		HSURFACE		m_hStar;										// A star bitmap
		HSURFACE		m_hFireLow;										// A fire bitmap
		HSURFACE		m_hFireMed;										// A fire bitmap
		HSURFACE		m_hFireHigh;									// A fire bitmap

		LithFont		*m_pFontSmall;									// The font to draw with
		LithFont		*m_pFontMedium;									// The font to draw with
		LithFont		*m_pFontLarge;									// The font to draw with
		LithFont		*m_pFontHuge;									// The font to draw with
		LithFont		*m_pFontMammoth;								// The font to draw with

		LithFont		*m_pTitleFont;									// Pointer to the appropriate font to use for titles
		LithFont		*m_pHeaderFont;									// Pointer to the appropriate font to use for headers
		LithFont		*m_pTextFont;									// Pointer to the appropriate font to use for text

		LTFLOAT			m_fStartTime;									// A time reference for the drawing code
		uint8			m_nDisplayState;								// The current state of the display
		LTFLOAT			m_fDisplayRamp;									// The ramp value for the current display

		HSURFACE		m_hScreen;										// The handle to the screen surface
		uint32			m_nScreenWidth;									// The current screen width
		uint32			m_nScreenHeight;								// The current screen height

		LTFLOAT			m_fColorCycle;									// A color cycle ramp

		LTBOOL			m_bStatsDisplay;								// Show the stats display?
		LTBOOL			m_bPlayerDisplay;								// Show the player display?
		LTBOOL			m_bSummaryDisplay;								// Show the summary display?
		LTBOOL			m_bRoundDisplay;								// Show the round display?
		LTBOOL			m_bRulesDisplay;								// Show the rules display?
		LTBOOL			m_bLoadingDisplay;								// Show the loading display?
		LTBOOL			m_bObserveDisplay;								// Show the observe display?
		LTBOOL			m_bClipModeDisplay;								// Show the clip mode display?

		MPClientMgrChart	m_StatChart1;								// The stats for this game
		MPClientMgrChart	m_StatChart2;								// The stats for this game
		MPClientMgrChart	m_StatChart3;								// The stats for this game
		MPClientMgrChart	m_StatChart4;								// The stats for this game
		MPClientMgrChart	m_StatChart5;								// The stats for this game
		MPClientMgrChart	m_StatChart6;								// The stats for this game
		MPClientMgrChart	m_StatChart7;								// The stats for this game
		MPClientMgrChart	m_StatChart8;								// The stats for this game

		char			m_szInfoString[256];							// The info string to print in the middle of the screen
		HLTCOLOR		m_hInfoStringColor;								// Color to draw the info string

		char			m_szWarningString[128];							// The warning string to print in the middle of the screen
		HLTCOLOR		m_hWarningStringColor;							// Color to draw the warning string
		LTFLOAT			m_fWarningStringTimer;							// The timer for the warning string
		LTFLOAT			m_fWarningStringDelay;							// The delay for the warning string


		// Sound information

		LTFLOAT			m_fLastSoundTime;								// Last time a special multiplayer sound was played
		HLTSOUND		m_hEvacSound;									// The sound stating the current evacuation state


		// Misc variables for specific game types

		LTBOOL			m_bDrawEvacLocation;							// Tells whether to draw the pointer or not
		LTVector		m_vEvacLocation;								// The position of the Evac zone

		uint8			m_nWaitSeconds;									// The number of seconds we're waiting for


		// Data used only for the player selection functionality

		LTIntPt			m_ptCursor;										// The current mouse cursor position
		LTIntPt			m_ptChart;										// The last player selection that was highlighted

		uint8			m_nCurrentClass;								// The currently selected character class
		uint8			m_nCurrentSet;									// The currently selected character set


		// Strings that are frequently used...

		HSTRING			m_szTeams[MAX_TEAMS + 1];						// The names of the available teams
		HSTRING			m_szStats[NUM_STATS_STRINGS];					// The miscellaneous stats strings
		HSTRING			m_szSRules[SR_MAX_ITEMS];						// The miscellaneous server rules strings

		HSTRING			m_szCharList[MAX_TEAMS][32];
		int				m_nCharList[MAX_TEAMS][32];
		int				m_nCharCount[4];
};

// -------------------------------------------------------------------- //

#endif

