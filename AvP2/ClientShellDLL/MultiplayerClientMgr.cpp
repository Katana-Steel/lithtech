// -------------------------------------------------------------------- //
//
// MODULE  : MultiplayerClientMgr.cpp
//
// PURPOSE : A client-side multiplayer statistics manager
//
// CREATED : 3/27/01
//
// -------------------------------------------------------------------- //


#include "stdafx.h"
#include "MultiplayerClientMgr.h"
#include "GameClientShell.h"
#include "LithFontMgr.h"
#include "VarTrack.h"
#include "MsgIDs.h"

#include "ClientRes.h"
#include "MessageMgr.h"
#include "CommandIDs.h"

#include "CharacterButeMgr.h"
#include "ClientButeMgr.h"

#include "ServerOptionMgr.h"

#include "ProfileMgr.h"

// -------------------------------------------------------------------- //

#define SD_STATE_NONE				0
#define SD_STATE_RAMPIN				1
#define SD_STATE_DISPLAYED			2
#define SD_STATE_RAMPOUT			3

// -------------------------------------------------------------------- //

#define STAT_NA						0
#define STAT_SERVER					1
#define STAT_GAMETYPE				2
#define STAT_LEVEL					3
#define STAT_FRAGLIMIT				4
#define STAT_SCORELIMIT				5
#define STAT_TIMEREMAINING			6
#define STAT_PLAYERNAME				7
#define STAT_PING					8
#define STAT_FRAGS					9
#define STAT_SCORE					10
#define STAT_TIMEPLAYING			11
#define STAT_DEATHMATCH				12
#define STAT_TEAM_DEATHMATCH		13
#define STAT_TEAMNAME				14
#define STAT_NEXTLEVEL				15
#define STAT_VERIFY					16
#define STAT_WAITING				17
#define STAT_LOADING				18
#define STAT_TOTALPLAYERS			19
#define STAT_UNKNOWN				20
#define STAT_PLAYER					21
#define STAT_SELECTION				22
#define STAT_PLAYERINFO				23
#define STAT_ALIEN_HUNT				24
#define STAT_MARINE_SURVIVOR		25
#define STAT_OVERRUN				26
#define STAT_EVAC					27
#define STAT_ROUND					28
#define STAT_SURVIVOR_TAG			29
#define STAT_SURVIVOR_CUT			30
#define STAT_RANDOM					31
#define STAT_OBSERVE_POINT			32
#define STAT_RESPAWN				33
#define STAT_START_GAME				34
#define STAT_WAIT_NEXT_ROUND		35
#define STAT_WAIT_START_GAME		36
#define STAT_WAIT_TEAMS				37
#define STAT_LIVES					38
#define STAT_AUTOSTART				39
#define STAT_ROUNDSTART				40
#define STAT_SUMMARY				41
#define STAT_LEVELSTART				42
#define STAT_DEDICATED				43
#define STAT_CLIP_TITLE				44
#define STAT_CLIP_INFO1				45
#define STAT_CLIP_INFO2				46
#define STAT_SURVIVORS				47
#define STAT_SURVIVED				48
#define STAT_EVACS					49
#define STAT_EXTRA12				50
#define STAT_EXTRA13				51
#define STAT_EXTRA14				52
#define STAT_EXTRA15				53
#define STAT_EXTRA16				54
#define STAT_EXTRA17				55
#define STAT_EXTRA18				56
#define STAT_EXTRA19				57

// -------------------------------------------------------------------- //

#define STRDATA(hStr)				(g_pLTClient->GetStringData(hStr))

// -------------------------------------------------------------------- //

#define EVAC_OVERLAY_ALPHA_NEAR		1000.0f
#define EVAC_OVERLAY_ALPHA_FAR		5000.0f

// -------------------------------------------------------------------- //

#define HEADER_FRAGS				0x01
#define HEADER_SCORE				0x02
#define HEADER_TIME					0x04
#define HEADER_ROUNDS				0x08
#define HEADER_PLAYERS				0x10
#define HEADER_LIVES				0x20

// -------------------------------------------------------------------- //

HLTCOLOR g_clrSpecies[MAX_TEAMS + 1] =
{
	SETRGB_T(255, 96, 96),		// Alien
	SETRGB_T(100, 100, 255),	// Marine
	SETRGB_T(0, 255, 0),		// Predator
	SETRGB_T(200, 200, 255),	// Corporate
	SETRGB_T(96, 96, 96)		// Observer
};

// -------------------------------------------------------------------- //

VarTrack g_vtScoreDisplayRampIn;
VarTrack g_vtScoreDisplayRampOut;
VarTrack g_vtScoreDisplayColorCycle;

VarTrack g_vtMultiplayerSounds;
VarTrack g_vtMultiplayerSoundDelay;

VarTrack g_vtConFragsLow;
VarTrack g_vtConFragsMed;
VarTrack g_vtConFragsHigh;


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::MultiplayerClientMgr()
//
// PURPOSE:		Initial setup
//
// -------------------------------------------------------------------- //

MultiplayerClientMgr::MultiplayerClientMgr()
{
	// Initialize any variables that need it
	m_szServer[0] = 0;
	m_szLevel[0] = 0;

	m_nFragLimit = 0;
	m_nScoreLimit = 0;
	m_nTimeRemaining = 0;
	m_nRoundLimit = 0;

	m_fSpeedScale = 1.0f;
	m_bClassWeapons = LTFALSE;
	m_nExosuit = 0;

	m_nClients = 0;

	m_nFragLeader = -1;
	m_nScoreLeader = -1;

	m_nTeamFragLeader = -1;
	m_nTeamScoreLeader = -1;

	m_nServerController = -1;

	m_nCurrentRound = 1;

	m_hBackground = LTNULL;
	m_hCheckOn = LTNULL;
	m_hCheckOff = LTNULL;
	m_hStar	= LTNULL;
	m_hFireLow = LTNULL;
	m_hFireMed = LTNULL;
	m_hFireHigh = LTNULL;

	m_pFontSmall = LTNULL;
	m_pFontMedium = LTNULL;
	m_pFontLarge = LTNULL;
	m_pFontHuge = LTNULL;
	m_pFontMammoth = LTNULL;

	m_pTitleFont = LTNULL;
	m_pHeaderFont = LTNULL;
	m_pTextFont = LTNULL;

	m_fStartTime = 0.0f;
	m_nDisplayState = SD_STATE_NONE;
	m_fDisplayRamp = 0.0f;

	m_hScreen = LTNULL;
	m_nScreenWidth = 0;
	m_nScreenHeight = 0;

	m_fColorCycle = 0.0f;

	m_bStatsDisplay = LTFALSE;
	m_bPlayerDisplay = LTFALSE;
	m_bSummaryDisplay = LTFALSE;
	m_bRoundDisplay = LTFALSE;
	m_bRulesDisplay = LTFALSE;
	m_bLoadingDisplay = LTFALSE;
	m_bObserveDisplay = LTFALSE;
	m_bClipModeDisplay = LTFALSE;

	memset(m_szInfoString, 0, 256);
	m_hInfoStringColor = 0;

	memset(m_szWarningString, 0, 128);
	m_hWarningStringColor = 0;
	m_fWarningStringTimer = -1.0f;
	m_fWarningStringDelay = 0.0f;

	m_fLastSoundTime = 0.0f;
	m_hEvacSound = LTNULL;

	m_bDrawEvacLocation = 0;
	m_vEvacLocation.Init();

	m_nWaitSeconds = 0xFF;

	int i, j;

	for(i = 0; i < MAX_TEAMS + 1; i++)
		m_szTeams[i] = LTNULL;

	for(i = 0; i < NUM_STATS_STRINGS; i++)
		m_szStats[i] = LTNULL;

	for(i = 0; i < SR_MAX_ITEMS; i++)
		m_szSRules[i] = LTNULL;

	for(i = 0; i < MAX_TEAMS; i++)
	{
		for(j = 0; j < 32; j++)
		{
			m_szCharList[i][j] = LTNULL;
			m_nCharList[i][j] = -1;
		}

		m_nCharCount[i] = 0;
	}

	m_nTimeLimit			= 0;					
	m_nHunt_HunterRace		= 0;					
	m_nHunt_PreyRace		= 0;					
	m_nHunt_Ratio			= 0;					
													
	m_nSurv_SurvivorRace	= 0;					
	m_nSurv_MutateRace		= 0;					
													
	m_nTM_DefenderRace		= 0;					
	m_nTM_DefenderLives		= 0;					
	m_nTM_AttackerRace		= 0;					
	m_nTM_AttackerLives		= 0;					

	m_bLifecycle			= LTFALSE;				

	m_fPowerupRespawnScale	= 1.0f;					
	m_fDamageScale			= 1.0f;					

	m_bLocationDamage		= LTTRUE;				
	m_bFriendlyFire			= LTFALSE;				
	m_bFriendlyNames		= LTFALSE;				
	m_bMaskLoss				= LTTRUE;				
	m_bCustScoring			= LTFALSE;	
	m_nBandwidth			= 0;
	m_nQueenMolt			= 0;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::~MultiplayerClientMgr()
//
// PURPOSE:		Mass Destruction!! Bwah ha ha!
//
// -------------------------------------------------------------------- //

MultiplayerClientMgr::~MultiplayerClientMgr()
{
	Term();
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Init()
//
// PURPOSE:		Setup the multiplayer manager
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::Init()
{
	// Create a transparent color
	HLTCOLOR hTransColor = SETRGB_T(0, 0, 0);

	// Create the background
	if(!m_hBackground)
	{
		m_hBackground = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\background.pcx");
		g_pLTClient->FillRect(m_hBackground, LTNULL, SETRGB(16, 16, 16));
		g_pLTClient->OptimizeSurface(m_hBackground, hTransColor);
		g_pLTClient->SetSurfaceAlpha(m_hBackground, 0.7f);
	}

	if(!m_hCheckOn)
	{
		m_hCheckOn = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\check-on.pcx");
		g_pLTClient->OptimizeSurface(m_hCheckOn, hTransColor);
		g_pLTClient->SetSurfaceAlpha(m_hCheckOn, 0.99f);
	}

	if(!m_hCheckOff)
	{
		m_hCheckOff = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\check-off.pcx");
		g_pLTClient->OptimizeSurface(m_hCheckOff, hTransColor);
		g_pLTClient->SetSurfaceAlpha(m_hCheckOff, 0.99f);
	}

	if(!m_hStar)
	{
		m_hStar = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\star.pcx");
		g_pLTClient->OptimizeSurface(m_hStar, hTransColor);
		g_pLTClient->SetSurfaceAlpha(m_hStar, 0.99f);
	}

	if(!m_hFireLow)
	{
		m_hFireLow = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\firelow.pcx");
		g_pLTClient->OptimizeSurface(m_hFireLow, hTransColor);
		g_pLTClient->SetSurfaceAlpha(m_hFireLow, 0.99f);
	}

	if(!m_hFireMed)
	{
		m_hFireMed = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\firemed.pcx");
		g_pLTClient->OptimizeSurface(m_hFireMed, hTransColor);
		g_pLTClient->SetSurfaceAlpha(m_hFireMed, 0.99f);
	}

	if(!m_hFireHigh)
	{
		m_hFireHigh = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\firehigh.pcx");
		g_pLTClient->OptimizeSurface(m_hFireHigh, hTransColor);
		g_pLTClient->SetSurfaceAlpha(m_hFireHigh, 0.99f);
	}


	// Create the font
	LITHFONTCREATESTRUCT lfcs;
	lfcs.szFontName = "Arial";
	lfcs.bBold = LTTRUE;

	if(!m_pFontSmall)
	{
		lfcs.nWidth = 7;
		lfcs.nHeight = 12;

		m_pFontSmall = CreateLithFont(g_pLTClient, &lfcs, LTTRUE);
	}

	if(!m_pFontMedium)
	{
		lfcs.nWidth = 8;
		lfcs.nHeight = 15;

		m_pFontMedium = CreateLithFont(g_pLTClient, &lfcs, LTTRUE);
	}

	if(!m_pFontLarge)
	{
		lfcs.nWidth = 10;
		lfcs.nHeight = 18;

		m_pFontLarge = CreateLithFont(g_pLTClient, &lfcs, LTTRUE);
	}

	if(!m_pFontHuge)
	{
		lfcs.nWidth = 13;
		lfcs.nHeight = 22;

		m_pFontHuge = CreateLithFont(g_pLTClient, &lfcs, LTTRUE);
	}

	if(!m_pFontMammoth)
	{
		lfcs.nWidth = 16;
		lfcs.nHeight = 29;

		m_pFontMammoth = CreateLithFont(g_pLTClient, &lfcs, LTTRUE);
	}


	// Initialize any console variables
	if(!g_vtScoreDisplayRampIn.IsInitted())
		g_vtScoreDisplayRampIn.Init(g_pLTClient, "ScoreDisplayRampIn", LTNULL, 0.15f);

	if(!g_vtScoreDisplayRampOut.IsInitted())
		g_vtScoreDisplayRampOut.Init(g_pLTClient, "ScoreDisplayRampOut", LTNULL, 0.1f);

	if(!g_vtScoreDisplayColorCycle.IsInitted())
		g_vtScoreDisplayColorCycle.Init(g_pLTClient, "ScoreDisplayColorCycle", LTNULL, 0.5f);

	if(!g_vtMultiplayerSounds.IsInitted())
		g_vtMultiplayerSounds.Init(g_pLTClient, "MultiplayerSounds", LTNULL, 1.0f);

	if(!g_vtMultiplayerSoundDelay.IsInitted())
		g_vtMultiplayerSoundDelay.Init(g_pLTClient, "MultiplayerSoundDelay", LTNULL, 10.0f);

	if(!g_vtConFragsLow.IsInitted())
		g_vtConFragsLow.Init(g_pLTClient, "ConFragsLow", LTNULL, 3.0f);

	if(!g_vtConFragsMed.IsInitted())
		g_vtConFragsMed.Init(g_pLTClient, "ConFragsMed", LTNULL, 4.0f);

	if(!g_vtConFragsHigh.IsInitted())
		g_vtConFragsHigh.Init(g_pLTClient, "ConFragsHigh", LTNULL, 5.0f);


	// Fill in the team names
	if(!m_szTeams[0]) m_szTeams[0] = g_pLTClient->FormatString(IDS_MP_TEAM_ALIENS);
	if(!m_szTeams[1]) m_szTeams[1] = g_pLTClient->FormatString(IDS_MP_TEAM_MARINES);
	if(!m_szTeams[2]) m_szTeams[2] = g_pLTClient->FormatString(IDS_MP_TEAM_PREDATORS);
	if(!m_szTeams[3]) m_szTeams[3] = g_pLTClient->FormatString(IDS_MP_TEAM_CORPORATES);
	if(!m_szTeams[4]) m_szTeams[4] = g_pLTClient->FormatString(IDS_MP_TEAM_OBSERVERS);

	// Fill in the stat strings
	for(int i = 0; i < NUM_STATS_STRINGS; i++)
	{
		if(!m_szStats[i]) m_szStats[i] = g_pLTClient->FormatString(IDS_MP_STAT_MIN + i + 1);
	}

	// Fill in rules strings...
	InitRulesStrings();

	// Now create all the character strings for the player selection
	BuildClassLists();


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::InitRulesString()
//
// PURPOSE:		Init the rules strings...
//
// -------------------------------------------------------------------- //
void MultiplayerClientMgr::InitRulesStrings()
{
	if(!m_szSRules[SR_GAME_SETTINGS])	m_szSRules[SR_GAME_SETTINGS] = g_pLTClient->FormatString(IDS_HOST_SETTINGS);
	if(!m_szSRules[SR_MAX_PLAYERS])		m_szSRules[SR_MAX_PLAYERS] = g_pLTClient->FormatString(IDS_MAXPLAYERS);
	if(!m_szSRules[SR_MAX_MARINES])		m_szSRules[SR_MAX_MARINES] = g_pLTClient->FormatString(IDS_MAXMARINES);
	if(!m_szSRules[SR_MAX_ALIENS])		m_szSRules[SR_MAX_ALIENS] = g_pLTClient->FormatString(IDS_MAXALIENS);
	if(!m_szSRules[SR_MAX_PREDATORS])	m_szSRules[SR_MAX_PREDATORS] = g_pLTClient->FormatString(IDS_MAXPREDATORS);
	if(!m_szSRules[SR_MAX_CORPORATES])	m_szSRules[SR_MAX_CORPORATES] = g_pLTClient->FormatString(IDS_MAXCORPORATES);
	if(!m_szSRules[SR_FRAG_LIMIT])		m_szSRules[SR_FRAG_LIMIT] = g_pLTClient->FormatString(IDS_FRAG_LIMIT);
	if(!m_szSRules[SR_SCORE_LIMIT])		m_szSRules[SR_SCORE_LIMIT] = g_pLTClient->FormatString(IDS_SCORE_LIMIT);
	if(!m_szSRules[SR_TIME_LIMIT])		m_szSRules[SR_TIME_LIMIT] = g_pLTClient->FormatString(IDS_TIME_LIMIT);
	if(!m_szSRules[SR_ROUND_LIMIT])		m_szSRules[SR_ROUND_LIMIT] = g_pLTClient->FormatString(IDS_ROUND_LIMIT);
	if(!m_szSRules[SR_LIFECYCLE])		m_szSRules[SR_LIFECYCLE] = g_pLTClient->FormatString(IDS_LIFECYCLE);
	if(!m_szSRules[SR_HUNTER_RACE])		m_szSRules[SR_HUNTER_RACE] = g_pLTClient->FormatString(IDS_HUNTER_RACE);
	if(!m_szSRules[SR_PREY_RACE])		m_szSRules[SR_PREY_RACE] = g_pLTClient->FormatString(IDS_PREY_RACE);
	if(!m_szSRules[SR_HUNT_RATIO])		m_szSRules[SR_HUNT_RATIO] = g_pLTClient->FormatString(IDS_HUNT_RATIO);
	if(!m_szSRules[SR_SURVIVOR_RACE])	m_szSRules[SR_SURVIVOR_RACE] = g_pLTClient->FormatString(IDS_SURVIVOR_RACE);
	if(!m_szSRules[SR_MUTATE_RACE])		m_szSRules[SR_MUTATE_RACE] = g_pLTClient->FormatString(IDS_MUTATE_RACE);
	if(!m_szSRules[SR_DEFENDER_RACE])	m_szSRules[SR_DEFENDER_RACE] = g_pLTClient->FormatString(IDS_DEFENDER_RACE);
	if(!m_szSRules[SR_DEFENDER_LIVES])	m_szSRules[SR_DEFENDER_LIVES] = g_pLTClient->FormatString(IDS_DEFENDER_LIVES);
	if(!m_szSRules[SR_ATTACKER_RACE])	m_szSRules[SR_ATTACKER_RACE] = g_pLTClient->FormatString(IDS_ATTACKER_RACE);
	if(!m_szSRules[SR_ATTACKER_LIVES])	m_szSRules[SR_ATTACKER_LIVES] = g_pLTClient->FormatString(IDS_ATTACKER_LIVES);
	if(!m_szSRules[SR_ROUND_TIME])		m_szSRules[SR_ROUND_TIME] = g_pLTClient->FormatString(IDS_ROUND_TIME);
	if(!m_szSRules[SR_EVAC_RACE])		m_szSRules[SR_EVAC_RACE] = g_pLTClient->FormatString(IDS_EVAC_RACE);
	if(!m_szSRules[SR_EVAC_LIVES])		m_szSRules[SR_EVAC_LIVES] = g_pLTClient->FormatString(IDS_EVAC_LIVES);
	if(!m_szSRules[SR_EVAC_TIME])		m_szSRules[SR_EVAC_TIME] = g_pLTClient->FormatString(IDS_EVAC_TIME);
	if(!m_szSRules[SR_ADV_SETTINGS])	m_szSRules[SR_ADV_SETTINGS] = g_pLTClient->FormatString(IDS_HOST_PREFERENCES);
	if(!m_szSRules[SR_DAMAGE_RATE])		m_szSRules[SR_DAMAGE_RATE] = g_pLTClient->FormatString(IDS_OPT_PLAYER_DAMAGE);
	if(!m_szSRules[SR_LOC_DAMAGE])		m_szSRules[SR_LOC_DAMAGE] = g_pLTClient->FormatString(IDS_OPT_LOC_DAMAGE);
	if(!m_szSRules[SR_FRIENDLY_FIRE])	m_szSRules[SR_FRIENDLY_FIRE] = g_pLTClient->FormatString(IDS_OPT_FRIENDLY_FIRE);
	if(!m_szSRules[SR_FRIENDLY_NAMES])	m_szSRules[SR_FRIENDLY_NAMES] = g_pLTClient->FormatString(IDS_OPT_FRIENDLY_NAMES);
	if(!m_szSRules[SR_PRED_MASK_LOSS])	m_szSRules[SR_PRED_MASK_LOSS] = g_pLTClient->FormatString(IDS_OPT_PRED_MASK_LOSS);
	if(!m_szSRules[SR_CLASS_WEAPONS])	m_szSRules[SR_CLASS_WEAPONS] = g_pLTClient->FormatString(IDS_OPT_CLASS_WEAPONS);
	if(!m_szSRules[SR_EXOSUIT])			m_szSRules[SR_EXOSUIT] = g_pLTClient->FormatString(IDS_OPT_EXOSUIT);
	if(!m_szSRules[SR_QUEEN])			m_szSRules[SR_QUEEN] = g_pLTClient->FormatString(IDS_OPT_QUEEN);
	if(!m_szSRules[SR_CUSTOM_SCORING])	m_szSRules[SR_CUSTOM_SCORING] = g_pLTClient->FormatString(IDS_HOST_SCORING);
	if(!m_szSRules[SR_OPT_RESPAWN])		m_szSRules[SR_OPT_RESPAWN] = g_pLTClient->FormatString(IDS_OPT_RESPAWN);
	if(!m_szSRules[SR_OPT_RUNSPEED])	m_szSRules[SR_OPT_RUNSPEED] = g_pLTClient->FormatString(IDS_OPT_RUNSPEED);
	if(!m_szSRules[SR_BANDWIDTH])		m_szSRules[SR_BANDWIDTH] = g_pLTClient->FormatString(IDS_SERVER_BANDWIDTH);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Term()
//
// PURPOSE:		Free up all the multiplayer manager resources
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Term()
{
	// Get rid of the background... blah, who wants it anyway!
	if(m_hBackground)
	{
		g_pLTClient->DeleteSurface(m_hBackground);
		m_hBackground = LTNULL;
	}

	if(m_hCheckOn)
	{
		g_pLTClient->DeleteSurface(m_hCheckOn);
		m_hCheckOn = LTNULL;
	}

	if(m_hCheckOff)
	{
		g_pLTClient->DeleteSurface(m_hCheckOff);
		m_hCheckOff = LTNULL;
	}

	if(m_hStar)
	{
		g_pLTClient->DeleteSurface(m_hStar);
		m_hStar = LTNULL;
	}

	if(m_hFireLow)
	{
		g_pLTClient->DeleteSurface(m_hFireLow);
		m_hFireLow = LTNULL;
	}

	if(m_hFireMed)
	{
		g_pLTClient->DeleteSurface(m_hFireMed);
		m_hFireMed = LTNULL;
	}

	if(m_hFireHigh)
	{
		g_pLTClient->DeleteSurface(m_hFireHigh);
		m_hFireHigh = LTNULL;
	}


	// Sounds
	if(m_hEvacSound)
	{
		g_pLTClient->KillSound(m_hEvacSound);
		m_hEvacSound = LTNULL;
	}


	// Free the team strings
	for(int i = 0; i < MAX_TEAMS + 1; i++)
	{
		if(m_szTeams[i])
		{
			g_pLTClient->FreeString(m_szTeams[i]);
			m_szTeams[i] = LTNULL;
		}
	}

	for(i = 0; i < NUM_STATS_STRINGS; i++)
	{
		if(m_szStats[i])
		{
			g_pLTClient->FreeString(m_szStats[i]);
			m_szStats[i] = LTNULL;
		}
	}

	for(i = 0; i < SR_MAX_ITEMS; i++)
	{
		if(m_szSRules[i])
		{
			g_pLTClient->FreeString(m_szSRules[i]);
			m_szSRules[i] = LTNULL;
		}
	}

	// Make the fonts go 'poof!'
	FreeLithFont(m_pFontSmall);
	FreeLithFont(m_pFontMedium);
	FreeLithFont(m_pFontLarge);
	FreeLithFont(m_pFontHuge);
	FreeLithFont(m_pFontMammoth);

	m_pTitleFont = LTNULL;
	m_pHeaderFont = LTNULL;
	m_pTextFont = LTNULL;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::AddClient()
//
// PURPOSE:		Handle the passed in message if necessary
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::AddClient(uint32 nClientID, MPClientMgrData &pData)
{
	// See if this client is already on the list
	int nIndex = FindClient(nClientID);

	// If this client was not found... then find a spot for him
	if(nIndex == -1)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!m_ClientData[i].m_nClientID)
			{
				nIndex = i;
				break;
			}
		}
	}

	// If we're still invalid... just return, we're out of spots
	if(nIndex == -1) return LTFALSE;


	// Otherwise, copy over our info
	memcpy(&m_ClientData[nIndex], &pData, sizeof(MPClientMgrData));
	m_nClients++;


	if(g_pGameClientShell->GetGameType()->IsMultiplayer())
	{
		// Pick a random message
		int nMsg = GetRandom(IDS_MP_CLIENT_JOINED_MIN, IDS_MP_CLIENT_JOINED_MAX);

		HSTRING hStr = g_pLTClient->FormatString(nMsg, m_ClientData[nIndex].m_szName);
		g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
		g_pLTClient->FreeString(hStr);
	}


	// Clear the stat charts
	ResetStatCharts();


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::RemoveClient()
//
// PURPOSE:		Handle the passed in message if necessary
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::RemoveClient(uint32 nClientID)
{
	// See if this client is on the list
	int nIndex = FindClient(nClientID);

	// If this client was not found... then find a spot for him
	if(nIndex == -1) return LTFALSE;


	if(g_pGameClientShell->GetGameType()->IsMultiplayer())
	{
		// Pick a random message
		int nMsg = GetRandom(IDS_MP_CLIENT_EXITED_MIN, IDS_MP_CLIENT_EXITED_MAX);

		HSTRING hStr = g_pLTClient->FormatString(nMsg, m_ClientData[nIndex].m_szName);
		g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
		g_pLTClient->FreeString(hStr);
	}


	// Otherwise, clear out our data
	memset(&m_ClientData[nIndex], 0, sizeof(MPClientMgrData));
	m_nClients--;


	// Make sure the leader stats get updated
	UpdateLeaders();


	// Clear the stat charts
	ResetStatCharts();


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::FindClient()
//
// PURPOSE:		Handle the passed in message if necessary
//
// -------------------------------------------------------------------- //

int MultiplayerClientMgr::FindClient(uint32 nClientID)
{
	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_nClientID == nClientID)
		{
			return i;
		}
	}

	return -1;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::FindClient()
//
// PURPOSE:		Handle the passed in message if necessary
//
// -------------------------------------------------------------------- //

uint32 MultiplayerClientMgr::FindClientID(char *szName)
{
	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_nClientID)
		{
			if(!_stricmp(m_ClientData[i].m_szName, szName))
			{
				return m_ClientData[i].m_nClientID;
			}
		}
	}

	return 0;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ClearClientData()
//
// PURPOSE:		Clears all the current client data
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ClearClientData()
{
	memset(&m_ClientData, 0, MAX_CLIENTS * sizeof(MPClientMgrData));
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ClearTeamData()
//
// PURPOSE:		Clears all the current team data
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ClearTeamData()
{
	memset(&m_TeamData, 0, MAX_TEAMS * sizeof(MPClientMgrTeamData));
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ClearVerification()
//
// PURPOSE:		Clears all the current verification values
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::ClearVerification()
{
	LTBOOL bNeededClearing = LTFALSE;

	// Go through the list and look for this guy
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_nClientID)
		{
			if(m_ClientData[i].m_bVerified)
				bNeededClearing = LTTRUE;

			m_ClientData[i].m_bVerified = LTFALSE;
		}
	}

	return bNeededClearing;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage()
//
// PURPOSE:		Handle the passed in message if necessary
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage(uint8 nMessage, HMESSAGEREAD hRead)
{
	switch(nMessage)
	{
		case MID_MPMGR_GAME_INFO:
		{
			return OnMessage_GameInfo(nMessage, hRead);
		}

		case MID_MPMGR_ADD_CLIENT:
		{
			MPClientMgrData data;

			hRead->ReadStringFL(data.m_szName, MAX_MP_CLIENT_NAME_LENGTH);
			hRead->ReadByteFL(data.m_nCharacterClass);
			hRead->ReadByteFL(data.m_nCharacterSet);

			hRead->ReadDWordFL(data.m_nClientID);

			data.m_nPing = 0;
			data.m_nFrags = 0;
			data.m_nConsecutiveFrags = 0;
			data.m_nScore = 0;

			data.m_nLives = 0;

			// Add this guy onto the list
			AddClient(data.m_nClientID, data);

			return LTTRUE;
		}

		case MID_MPMGR_REMOVE_CLIENT:
		{
			uint32 nClientID;
			hRead->ReadDWordFL(nClientID);

			// Take this client off the list
			RemoveClient(nClientID);

			return LTTRUE;
		}

		case MID_MPMGR_UPDATE_CLIENT:
		{
			return OnMessage_UpdateClient(nMessage, hRead);
		}

		case MID_MPMGR_UPDATE_TEAM:
		{
			return OnMessage_UpdateTeam(nMessage, hRead);
		}

		case MID_MPMGR_TIME:
		{
			hRead->ReadWordFL(m_nTimeRemaining);

			if(m_nTimeRemaining <= 15)
			{
				char szTemp[64];
				sprintf(szTemp, "%s%d:%.2d", STRDATA(m_szStats[STAT_TIMEREMAINING]), m_nTimeRemaining / 60, m_nTimeRemaining % 60);

				SetWarningString(szTemp, LTTRUE, 1.25f, SETRGB(32, 32, 32));
			}

			return LTTRUE;
		}

		case MID_MPMGR_ROUND:
		{
			hRead->ReadByteFL(m_nCurrentRound);

			char szTemp[64];
			sprintf(szTemp, "%s (%d/%d)", STRDATA(m_szStats[STAT_ROUND]), m_nCurrentRound, m_nRoundLimit);

			SetWarningString(szTemp, LTTRUE, 3.0f, SETRGB(32, 32, 32));

			return LTTRUE;
		}

		case MID_MPMGR_MESSAGE:
		case MID_MPMGR_TEAM_MESSAGE:
		case MID_MPMGR_PRIVATE_MESSAGE:
		{
			return OnMessage_Message(nMessage, hRead);
		}

		case MID_MPMGR_SERVER_MESSAGE:
		{
			return OnMessage_ServerMessage(nMessage, hRead);
		}

		case MID_MPMGR_NEXT_LEVEL:
		{
			return OnMessage_NextLevel(nMessage, hRead);
		}

		case MID_MPMGR_LOAD_LEVEL:
		{
			// Make sure the loading display gets on screen
			ShowLoadingDisplay(LTTRUE);
			Update();

			// Tell the server that we've verified
			HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_MPMGR_LOAD_LEVEL);
			g_pLTClient->EndMessage(hMessage);

			return LTTRUE;
		}

		case MID_MPMGR_CHANGE_PLAYER:
		{
			return OnMessage_ChangePlayer(nMessage, hRead);
		}

		case MID_MPMGR_OBSERVE_MODE:
		{
			uint8 nCommand;
			hRead->ReadByteFL(nCommand);

			if(nCommand == MP_OBSERVE_ON)
			{
				uint32 nClientID;
				hRead->ReadDWordFL(nClientID);

				// See if this client is on the list
				int nIndex = FindClient(nClientID);

				if(nIndex != -1)
				{
					uint32 nLocalID;
					g_pLTClient->GetLocalClientID(&nLocalID);

					if(nLocalID == m_ClientData[nIndex].m_nClientID)
					{
						ShowObserveDisplay(LTTRUE);
						ShowClipModeDisplay(LTFALSE);
					}

					m_ClientData[nIndex].m_bObserving = LTTRUE;
				}
			}
			else if(nCommand == MP_OBSERVE_POINT)
			{
				uint8 nPoint, nMode;
				hRead->ReadByteFL(nPoint);
				hRead->ReadByteFL(nMode);

				// Get this client's local ID
				uint32 nLocalID;
				g_pLTClient->GetLocalClientID(&nLocalID);

				int nIndex = FindClient(nLocalID);

				if(nIndex != -1)
				{
					m_ClientData[nIndex].m_nObservePoint = nPoint;
					m_ClientData[nIndex].m_nObserveMode = nMode;
				}
			}

			return LTTRUE;
		}

		case MID_MPMGR_RESPAWN:
		{
			uint32 nClientID;
			hRead->ReadDWordFL(nClientID);

			// See if this client is on the list
			int nIndex = FindClient(nClientID);

			if(nIndex != -1)
			{
				uint32 nLocalID;
				g_pLTClient->GetLocalClientID(&nLocalID);

				if(nLocalID == m_ClientData[nIndex].m_nClientID)
				{
					ShowObserveDisplay(LTFALSE);
					ShowClipModeDisplay(LTFALSE);
				}

				m_ClientData[nIndex].m_bObserving = LTFALSE;
			}

			return LTTRUE;
		}

		case MID_MPMGR_TAUNT:
		{
			uint32 nClientID;
			hRead->ReadDWordFL(nClientID);

			// Find the object of this client and play a sound from him
			g_pGameClientShell->PlayTauntSound(nClientID);

			return LTTRUE;
		}

		case MID_MPMGR_WAIT_SECONDS:
		{
			hRead->ReadByteFL(m_nWaitSeconds);
			return LTTRUE;
		}
	}

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMouseMove()
//
// PURPOSE:		Sets the current mouse cursor position
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMouseMove(int x, int y)
{
	m_ptCursor.x = x;
	m_ptCursor.y = y;

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnLButtonDown()
//
// PURPOSE:		Handles a left mouse button down event
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnLButtonDown(int x, int y)
{
	// Make sure we're not in the menus or something...
	if(g_pGameClientShell->GetInterfaceMgr()->GetGameState() != GS_PLAYING)
		return LTFALSE;


	if(!m_bPlayerDisplay && !m_bObserveDisplay || m_bClipModeDisplay)
		return LTFALSE;


	if(m_ptChart.x != -1)
	{
		// Set the current class and set
		if(m_bPlayerDisplay)
		{
			m_nCurrentClass = m_ptChart.x;
			m_nCurrentSet = m_nCharList[m_ptChart.x][m_ptChart.y - 2];
		}
		else if(m_bObserveDisplay)
		{
			if(m_ptChart.y == 2)
			{
				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_MPMGR_OBSERVE_MODE);

				if(m_ptChart.x == 0)
					hMessage->WriteByte(MP_OBSERVE_PREV);
				else if(m_ptChart.x == 1)
					hMessage->WriteByte(MP_OBSERVE_RANDOM);
				else if(m_ptChart.x == 2)
					hMessage->WriteByte(MP_OBSERVE_NEXT);

				g_pLTClient->EndMessage(hMessage);
			}
			else if((m_ptChart.x == 1) && (m_ptChart.y == 4))
			{
				// Get this client's local ID
				uint32 nLocalID;
				g_pLTClient->GetLocalClientID(&nLocalID);

				int nIndex = FindClient(nLocalID);

				if(nIndex != -1)
				{
					switch(m_ClientData[nIndex].m_nObserveMode)
					{
						case MP_OBSERVE_MODE_NORMAL:
						{
							HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_RESPAWN);
							g_pLTClient->EndMessage(hMsg);
							break;
						}

						case MP_OBSERVE_MODE_START_GAME:
						{
							HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_START_GAME);
							g_pLTClient->EndMessage(hMsg);
							break;
						}
					}
				}
			}
		}

		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\playerselect.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}
	else
	{
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\playerselect_na.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnKeyDown()
//
// PURPOSE:		Handle key down events
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnCommandOn(int command)
{
	// Make sure we're not in the menus or something...
	if(g_pGameClientShell->GetInterfaceMgr()->GetGameState() != GS_PLAYING)
		return LTFALSE;

	switch(command)
	{
		case COMMAND_ID_MESSAGE:
		case COMMAND_ID_TEAMMESSAGE:
		{
			// We want to return FALSE here so that GameClientShell will allow other
			// modules a chance at handling the command even if the control is taken over
			// by the MultiplayerClientMgr
			return LTFALSE;
		}

		case COMMAND_ID_PLAYERINFO:
		{
			if(g_pGameClientShell->GetGameType()->IsMultiplayer())
			{
				int nType = g_pGameClientShell->GetGameType()->Get();

				// don't allow this in hunt or survivor mode.
				if(nType == MP_HUNT || nType == MP_SURVIVOR)
					return LTTRUE;

				// Get this client's local ID
				uint32 nLocalID;
				g_pLTClient->GetLocalClientID(&nLocalID);

				int nIndex = FindClient(nLocalID);

				if(nIndex != -1)
				{
					LTBOOL bIsOverrunOrEvac = nType == MP_OVERRUN || nType == MP_EVAC;

					if(g_pGameClientShell->IsPlayerDead() && !m_ClientData[nIndex].m_bObserving && !bIsOverrunOrEvac)
					{
						// Tell the server to put us in observe mode
						HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_MPMGR_OBSERVE_MODE);
						hMessage->WriteByte(MP_OBSERVE_ON);
						g_pLTClient->EndMessage(hMessage);
					}
					else
					{
						ShowPlayerDisplay(!m_bPlayerDisplay, command);
					}
				}
			}

			return LTTRUE;
		}

		case COMMAND_ID_FIRING:
		case COMMAND_ID_ALT_FIRING:
		case COMMAND_ID_NEXT_VISIONMODE:
		{
			// Handle Clip-Mode specific commands if in clip-mode...

			if (m_bClipModeDisplay)
			{
				if (COMMAND_ID_FIRING == command)
				{
					SetObservePoint(MP_OBSERVE_NEXT);
				}
				else if (COMMAND_ID_ALT_FIRING == command)
				{
					SetObservePoint(MP_OBSERVE_PREV);
				}
				else
				{
					SetObservePoint(MP_OBSERVE_RANDOM);
				}
			}
		}
		break;
	}

	// If input is not allowed... return TRUE so nothing else can screw with the controls
	if(!AllowMovementControl())
		return LTTRUE;

	// Allow other modules a shot at it...
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnKeyDown()
//
// PURPOSE:		Handle key down events
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnCommandOff(int command)
{
	// Make sure we're not in the menus or something...
	if(g_pGameClientShell->GetInterfaceMgr()->GetGameState() != GS_PLAYING)
		return LTFALSE;

	switch(command)
	{
		case COMMAND_ID_MESSAGE:
		case COMMAND_ID_TEAMMESSAGE:
			// We want to return FALSE here so that GameClientShell will allow other
			// modules a chance at handling the command even if the control is taken over
			// by the MultiplayerClientMgr
			return LTFALSE;

		case COMMAND_ID_PLAYERINFO:
			return LTTRUE;
	}

	// If input is not allowed... return TRUE so nothing else can screw with the controls
	if(!AllowMovementControl())
		return LTTRUE;

	// Allow other modules a shot at it...
	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnKeyDown()
//
// PURPOSE:		Handle key down events
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnKeyDown(int key, int rep)
{
	// Make sure we're not in the menus or something...
	if(g_pGameClientShell->GetInterfaceMgr()->GetGameState() != GS_PLAYING)
		return LTFALSE;

	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);

	int nIndex = FindClient(nLocalID);
	if(nIndex == -1) return LTFALSE;


	// If we're in the next level verify mode... then check for verify button
	if(m_bSummaryDisplay)
	{
		if(key == g_pClientButeMgr->GetCinematicSkipKey())
		{
//			if(!m_ClientData[nIndex].m_bVerified)
//			{
//				// Send a verification to the server
//				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_MPMGR_NEXT_LEVEL);
//				g_pLTClient->EndMessage(hMessage);
//			}

			return LTTRUE;
		}
	}
	else if(m_ClientData[nIndex].m_bObserving && !m_bClipModeDisplay)
	{
		// Tell the server we want a new observe point...
		if(key == g_pClientButeMgr->GetCinematicSkipKey())
		{
			SetObservePoint(MP_OBSERVE_RANDOM);
		}
		else if(key == VK_LEFT)
		{
			SetObservePoint(MP_OBSERVE_PREV);
		}
		else if(key == VK_RIGHT)
		{
			SetObservePoint(MP_OBSERVE_NEXT);
		}
	}

	return LTFALSE;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::SetObservePoint()
//
// PURPOSE:		Change the observe point
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::SetObservePoint(uint8 nObserveState)
{
	// Make sure observe state is valid...

	if (MP_OBSERVE_PREV != nObserveState && 
		MP_OBSERVE_NEXT != nObserveState &&
		MP_OBSERVE_RANDOM != nObserveState) return LTFALSE;

	// Can only change observe points when in observe display or clip mode display...

	if (!m_bObserveDisplay && !m_bClipModeDisplay) return LTFALSE;

	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_MPMGR_OBSERVE_MODE);
	hMessage->WriteByte(nObserveState);
	g_pLTClient->EndMessage(hMessage);

	g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\playerselect.wav", SOUNDPRIORITY_MISC_MEDIUM);

	return LTTRUE;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnKeyUp()
//
// PURPOSE:		Handle key up events
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnKeyUp(int nKey)
{
	// Make sure we're not in the menus or something...
	if(g_pGameClientShell->GetInterfaceMgr()->GetGameState() != GS_PLAYING)
		return LTFALSE;

	return LTFALSE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Update()
//
// PURPOSE:		Update the multiplayer manager
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::Update()
{
	// Turn off the evac display by default
	LTBOOL bDrawEvacLocation = LTFALSE;

	// Update the display state and ramp values
	LTBOOL bDisplayControl = (m_bStatsDisplay || m_bPlayerDisplay || m_bSummaryDisplay || m_bRoundDisplay || m_bLoadingDisplay || m_bRulesDisplay);
	UpdateDisplay(bDisplayControl);

	// Make sure we're using the right formatting values
	UpdateScreenResolution();

	// Update the cycling color ramp value
	UpdateColorCycle();


	// Draw the info string to the center of the screen if it exists..
	if(m_szInfoString[0])
	{
		LITHFONTDRAWDATA dd;
		dd.dwFlags = LTF_DRAW_SOLID;
		dd.byJustify = LTF_JUSTIFY_CENTER;
		dd.hColor = m_hInfoStringColor;

		m_pHeaderFont->Draw(m_hScreen, m_szInfoString, &dd, m_nScreenWidth / 2, (m_nScreenHeight / 2) + 25);
	}


	// Draw everything to the screen based off the game type
	if(m_nDisplayState != SD_STATE_NONE)
	{
		// Calculate an offset of the header to draw from
		int nHeaderOffset = (m_pHeaderFont->Height() * 9);

		// Draw the appropriate screen
		if(m_bLoadingDisplay)
		{
			Draw_Background();
			Draw_Header_Loading();
		}
		else if(m_bSummaryDisplay)
		{
			uint8 nFlags = 0;
			uint8 nFlags2 = 0;
			int yOffset = 0;

			switch(g_pGameClientShell->GetGameType()->Get())
			{
				case MP_DM:			nFlags = HEADER_FRAGS | HEADER_SCORE; nFlags2 = nFlags; break;
				case MP_TEAMDM:		nFlags = HEADER_FRAGS | HEADER_SCORE; nFlags2 = nFlags; yOffset = (m_pTextFont->Height() * 7); break;
				case MP_HUNT:		nFlags = HEADER_FRAGS; nFlags2 = nFlags; break;
				case MP_SURVIVOR:	nFlags = HEADER_SCORE | HEADER_ROUNDS; nFlags2 = nFlags; break;
				case MP_OVERRUN:	nFlags = HEADER_SCORE | HEADER_ROUNDS; nFlags2 = nFlags; yOffset = (m_pTextFont->Height() * 7); break;
				case MP_EVAC:		nFlags = HEADER_SCORE | HEADER_ROUNDS; nFlags2 = nFlags & ~HEADER_SCORE; yOffset = (m_pTextFont->Height() * 7); break;
			}

			Draw_Background();
			Draw_Header(nFlags);

			if(g_pGameClientShell->GetGameType()->IsTeamMode())
				Draw_Stats_TeamDM(LTIntPt(80, nHeaderOffset), nFlags);

			Draw_Stats_DM(LTIntPt(20, nHeaderOffset + yOffset), nFlags2, LTTRUE);
			Draw_Footer_Summary();
		}
		else if(m_bRoundDisplay)
		{
			Draw_Stats_Round();
		}
		else if(m_bPlayerDisplay)
		{
			Draw_Background();
			Draw_Header(HEADER_PLAYERS);
			Draw_Player_Info(LTIntPt(0, nHeaderOffset));

			GameType gt = *g_pGameClientShell->GetGameType();

			if((gt != MP_HUNT) && (gt != MP_SURVIVOR))
				Draw_Player_Selection();
		}
		else if(m_bStatsDisplay)
		{
			uint8 nFlags = 0;
			uint8 nFlags2 = 0;
			int yOffset = 0;

			switch(g_pGameClientShell->GetGameType()->Get())
			{
				case MP_DM:			nFlags = HEADER_FRAGS | HEADER_SCORE | HEADER_TIME; nFlags2 = nFlags; break;
				case MP_TEAMDM:		nFlags = HEADER_FRAGS | HEADER_SCORE | HEADER_TIME; yOffset = (m_pTextFont->Height() * 7); nFlags2 = nFlags; break;
				case MP_HUNT:		nFlags = HEADER_FRAGS | HEADER_TIME; nFlags2 = nFlags; break;
				case MP_SURVIVOR:	nFlags = HEADER_SCORE | HEADER_ROUNDS | HEADER_TIME; nFlags2 = nFlags; break;
				case MP_OVERRUN:	nFlags = HEADER_SCORE | HEADER_LIVES | HEADER_ROUNDS | HEADER_TIME; nFlags2 = nFlags; yOffset = (m_pTextFont->Height() * 7); break;
				case MP_EVAC:		nFlags = HEADER_SCORE | HEADER_LIVES | HEADER_ROUNDS | HEADER_TIME; nFlags2 = nFlags & ~HEADER_SCORE; yOffset = (m_pTextFont->Height() * 7); break;
			}

			Draw_Background();
			Draw_Header(nFlags);

			// see if we are drawing the server rules or just the
			// normal score display
			if(m_bRulesDisplay)
			{
				// rules it is!
				Draw_Server_Rules(LTIntPt(0, nHeaderOffset));
			}
			else
			{
				// draw the score...
				if(g_pGameClientShell->GetGameType()->IsTeamMode())
					Draw_Stats_TeamDM(LTIntPt(80, nHeaderOffset), nFlags);

				Draw_Stats_DM(LTIntPt(20, nHeaderOffset + yOffset), nFlags2);
			}
		}
		else if(m_bRulesDisplay)
		{
			uint8 nFlags = 0;
			uint8 nFlags2 = 0;
			int yOffset = 0;

			switch(g_pGameClientShell->GetGameType()->Get())
			{
				case MP_DM:			nFlags = HEADER_FRAGS | HEADER_SCORE | HEADER_TIME; nFlags2 = nFlags; break;
				case MP_TEAMDM:		nFlags = HEADER_FRAGS | HEADER_SCORE | HEADER_TIME; yOffset = (m_pTextFont->Height() * 7); nFlags2 = nFlags; break;
				case MP_HUNT:		nFlags = HEADER_FRAGS | HEADER_TIME; nFlags2 = nFlags; break;
				case MP_SURVIVOR:	nFlags = HEADER_SCORE | HEADER_ROUNDS | HEADER_TIME; nFlags2 = nFlags; break;
				case MP_OVERRUN:	nFlags = HEADER_SCORE | HEADER_LIVES | HEADER_ROUNDS | HEADER_TIME; nFlags2 = nFlags; yOffset = (m_pTextFont->Height() * 7); break;
				case MP_EVAC:		nFlags = HEADER_SCORE | HEADER_LIVES | HEADER_ROUNDS | HEADER_TIME; nFlags2 = nFlags & ~HEADER_SCORE; yOffset = (m_pTextFont->Height() * 7); break;
			}

			Draw_Background();
			Draw_Header(nFlags);

			Draw_Server_Rules(LTIntPt(0, nHeaderOffset));
		}
	}
	else
	{
		// The observe display servers a little different purpose than the others... so do it here

		if(m_bObserveDisplay)
		{
			Draw_Observe_Selection();
		}
		else if(m_bClipModeDisplay)
		{
			Draw_ClipMode_Selection();
		}
		else
		{
			// Do the warning text display
			Draw_Warning_String();

			// Set the variable to turn on the overlay
			bDrawEvacLocation = LTTRUE;
		}
	}


	if(g_pGameClientShell->GetGameType()->Get() == MP_EVAC)
	{
		// Toggle on or off the evac display
		g_pGameClientShell->GetVisionModeMgr()->SetEvacMode(bDrawEvacLocation && m_bDrawEvacLocation);

		UpdateEvacOverlay();
	}


	return bDisplayControl;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowStatsDisplay()
//
// PURPOSE:		Turn on or off the stats display
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowStatsDisplay(LTBOOL bOn)
{
	if(!m_bLoadingDisplay && !m_bSummaryDisplay && !m_bPlayerDisplay)
	{
		if(!m_bStatsDisplay && bOn)
		{
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\stats_on.wav", SOUNDPRIORITY_MISC_MEDIUM);
		}
		else if(m_bStatsDisplay && !bOn)
		{
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\stats_off.wav", SOUNDPRIORITY_MISC_MEDIUM);
		}

		m_bStatsDisplay = bOn;
	}
	else
	{
		m_bStatsDisplay = LTFALSE;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowRulesDisplay()
//
// PURPOSE:		Turn on or off the rules display
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowRulesDisplay(LTBOOL bOn)
{
	if(!m_bLoadingDisplay && !m_bSummaryDisplay && !m_bPlayerDisplay && !m_bStatsDisplay)
	{
		if(!m_bRulesDisplay && bOn)
		{
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\stats_on.wav", SOUNDPRIORITY_MISC_MEDIUM);
		}
		else if(m_bRulesDisplay && !bOn)
		{
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\stats_off.wav", SOUNDPRIORITY_MISC_MEDIUM);
		}

		m_bRulesDisplay = bOn;
	}
	else
	{
		m_bRulesDisplay = LTFALSE;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowPlayerDisplay()
//
// PURPOSE:		Turn on or off the player display
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowPlayerDisplay(LTBOOL bOn, int command)
{
	if(!m_bLoadingDisplay && !m_bSummaryDisplay)
	{
		if(!m_bPlayerDisplay && bOn)
		{
			g_pInterfaceMgr->UseCursor(LTTRUE);
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\playerinfo_on.wav", SOUNDPRIORITY_MISC_MEDIUM);

			// Init some variables needed for the screen...
			uint32 nLocalID;
			g_pLTClient->GetLocalClientID(&nLocalID);

			int nIndex = FindClient(nLocalID);
			if(nIndex != -1)
			{
				m_nCurrentClass = m_ClientData[nIndex].m_nCharacterClass;
				m_nCurrentSet = m_ClientData[nIndex].m_nCharacterSet;
			}
			else
			{
				m_nCurrentClass = MAX_TEAMS;
				m_nCurrentSet = -1;
			}
		}
		else if(m_bPlayerDisplay && !bOn)
		{
			if(!m_bObserveDisplay)
				g_pInterfaceMgr->UseCursor(LTFALSE);

			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\playerinfo_off.wav", SOUNDPRIORITY_MISC_MEDIUM);

			// Decide if we've changed characters or not...
			uint32 nLocalID;
			g_pLTClient->GetLocalClientID(&nLocalID);

			int nIndex = FindClient(nLocalID);
			if(nIndex != -1)
			{
				if(m_ClientData[nIndex].m_nCharacterSet != m_nCurrentSet)
				{
					// Send the message to change players...
					HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_MPMGR_CHANGE_PLAYER);
					hMessage->WriteByte(m_nCurrentSet);
					g_pLTClient->EndMessage(hMessage);
				}
			}
		}

		m_bPlayerDisplay = bOn;
	}
	else
	{
		m_bPlayerDisplay = LTFALSE;
	}

	// set the rules flag
	m_bRulesDisplay = (command == COMMAND_ID_SERVERINFO);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowSummaryDisplay()
//
// PURPOSE:		Turn on or off the summary display
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowSummaryDisplay(LTBOOL bOn)
{
	if(!m_bLoadingDisplay)
	{
		if(!m_bSummaryDisplay && bOn)
		{
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\summary_on.wav", SOUNDPRIORITY_MISC_MEDIUM);

			// Turn off the cursor
			if(g_pGameClientShell->GetInterfaceMgr()->GetGameState() != GS_FOLDER)
				g_pInterfaceMgr->UseCursor(LTFALSE);

			// Clear the stat screens so they get reformatted
			ResetStatCharts();

			// Turn any warning strings off
			SetWarningString(LTNULL);
		}

		m_bSummaryDisplay = bOn;
	}
	else
	{
		m_bSummaryDisplay = LTFALSE;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowRoundDisplay()
//
// PURPOSE:		Turn on or off the round display
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowRoundDisplay(LTBOOL bOn)
{
	if(!m_bLoadingDisplay && !m_bSummaryDisplay)
	{
		if(!m_bRoundDisplay && bOn)
		{
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\summary_on.wav", SOUNDPRIORITY_MISC_MEDIUM);

			// Turn any warning strings off
			SetWarningString(LTNULL);
		}

		m_bRoundDisplay = bOn;
	}
	else
	{
		m_bRoundDisplay = LTFALSE;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowLoadingDisplay()
//
// PURPOSE:		Turn on or off the loading display
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowLoadingDisplay(LTBOOL bOn)
{
	if(!m_bLoadingDisplay && bOn)
	{
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\loading_on.wav", SOUNDPRIORITY_MISC_MEDIUM);

		// Turn any warning strings off
		SetWarningString(LTNULL);
	}

	m_bLoadingDisplay = bOn;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowObserveDisplay()
//
// PURPOSE:		Turn on or off the observe display
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowObserveDisplay(LTBOOL bOn)
{
	if(!m_bObserveDisplay && bOn)
	{
		g_pInterfaceMgr->UseCursor(LTTRUE);
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\observe_on.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}
	else if(m_bObserveDisplay && !bOn)
	{
		g_pInterfaceMgr->UseCursor(LTFALSE);
	}

	m_bObserveDisplay = bOn;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ShowClipModeDisplay()
//
// PURPOSE:		Turn on or off the clip-mode display (NOTE: this
//				should only be used inside of the PS_OBSERVE state
//				when the observe display isn't displayed).
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ShowClipModeDisplay(LTBOOL bOn)
{
	if(!m_bClipModeDisplay && bOn)
	{
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\clipmode_on.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}

	m_bClipModeDisplay = bOn;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::SetInfoString()
//
// PURPOSE:		Update the info string text...
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::SetInfoString(const char *szString, HLTCOLOR hColor)
{
	if(!szString || !szString[0])
		m_szInfoString[0] = 0;
	else
	{
		strncpy(m_szInfoString, szString, 255);
		m_hInfoStringColor = hColor;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::SetInfoString()
//
// PURPOSE:		Update the info string text...
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::SetInfoString(uint32 nClientID)
{
	// Find this client...
	int nIndex = FindClient(nClientID);
	if(nIndex == -1)
		return;


	// This bit of code finds the index into the character class list... so we can display
	// that information next to the character's name
	int nElement = -1;

	for(int i = 0; i < m_nCharCount[m_ClientData[nIndex].m_nCharacterClass]; i++)
	{
		if(m_nCharList[m_ClientData[nIndex].m_nCharacterClass][i] == m_ClientData[nIndex].m_nCharacterSet)
		{
			nElement = i;
			break;
		}
	}

	// Now fill in the info string with his info...
	sprintf(m_szInfoString, "%s (%s)", m_ClientData[nIndex].m_szName, (nElement == -1) ? STRDATA(m_szStats[STAT_UNKNOWN]) : STRDATA(m_szCharList[m_ClientData[nIndex].m_nCharacterClass][nElement]));
	m_hInfoStringColor = g_clrSpecies[m_ClientData[nIndex].m_nCharacterClass];
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::SetWarningString()
//
// PURPOSE:		Update the warning string text...
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::SetWarningString(const char *szString, LTBOOL bForce, LTFLOAT fDelay, HLTCOLOR hColor)
{
	// Check to see if we just want to clear it out...
	if(!szString)
	{
		m_szWarningString[0] = 0;
		m_fWarningStringTimer = -1.0f;

		return;
	}

	// Check to see if we can update the string yet...
	if((m_fWarningStringTimer == -1.0f) || bForce)
	{
		strncpy(m_szWarningString, szString, 127);
		m_hWarningStringColor = hColor;

		m_fWarningStringTimer = g_pLTClient->GetTime();
		m_fWarningStringDelay = fDelay;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::ResetStatCharts()
//
// PURPOSE:		Clear all the stat charts so they're get reset
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::ResetStatCharts()
{
	m_StatChart1.Clear();
	m_StatChart2.Clear();
	m_StatChart3.Clear();
	m_StatChart4.Clear();
	m_StatChart5.Clear();
	m_StatChart6.Clear();
	m_StatChart7.Clear();
	m_StatChart8.Clear();
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::GetSortedClientList()
//
// PURPOSE:		Get a list of indexes for a sorted client list
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::GetSortedClientList(int *nList, int nListSize, int nFlags)
{
	// Return if the parameters are invalid
	if(!nList || (nListSize < 1)) return;


	int16 nCheck = -10000;
	int16 nMax = 10000;
	int16 nTemp;

	int i, nIndex = 0;
	LTBOOL bSet;


	// Clear the list to start with...
	for(i = 0; i < nListSize; i++)
		nList[i] = -1;


	while(1)
	{
		// Reset our values...
		bSet = LTFALSE;
		nCheck = -10000;


		// Now find the next score to check for...
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_nClientID)
			{
				nTemp = (nFlags & HEADER_FRAGS) ? m_ClientData[i].m_nFrags : m_ClientData[i].m_nScore;

				if((nTemp > nCheck) && (nTemp < nMax))
				{
					nCheck = nTemp;
					bSet = LTTRUE;
				}
			}
		}


		// If we didn't set it... then we've go nobody left...
		if(!bSet)
			return;


		// Set our max to the value we found
		nMax = nCheck;


		// Now go through the list again and add in clients that have this score
		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_ClientData[i].m_nClientID)
			{
				nTemp = (nFlags & HEADER_FRAGS) ? m_ClientData[i].m_nFrags : m_ClientData[i].m_nScore;

				if(nTemp == nMax)
				{
					nList[nIndex++] = i;

					if(nIndex >= nListSize)
						return;
				}
			}
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::GetSortedTeamList()
//
// PURPOSE:		Get a list of indexes for a sorted team list
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::GetSortedTeamList(int *nList, int nListSize, int nFlags)
{
	// Return if the parameters are invalid
	if(!nList || (nListSize < 1)) return;


	int16 nCheck = -10000;
	int16 nMax = 10000;
	int16 nTemp;

	int i, nIndex = 0;
	LTBOOL bSet;


	// Clear the list to start with...
	for(i = 0; i < nListSize; i++)
		nList[i] = -1;


	while(1)
	{
		// Reset our values...
		bSet = LTFALSE;
		nCheck = -10000;


		// Now find the next score to check for...
		for(i = 0; i < MAX_TEAMS; i++)
		{
			nTemp = (nFlags & HEADER_FRAGS) ? m_TeamData[i].m_nFrags : m_TeamData[i].m_nScore;

			if((nTemp > nCheck) && (nTemp < nMax))
			{
				nCheck = nTemp;
				bSet = LTTRUE;
			}
		}


		// If we didn't set it... then we've go nobody left...
		if(!bSet)
			return;


		// Set our max to the value we found
		nMax = nCheck;


		// Now go through the list again and add in clients that have this score
		for(i = 0; i < MAX_TEAMS; i++)
		{
			nTemp = (nFlags & HEADER_FRAGS) ? m_TeamData[i].m_nFrags : m_TeamData[i].m_nScore;

			if(nTemp == nMax)
			{
				nList[nIndex++] = i;

				if(nIndex >= nListSize)
					return;
			}
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Background()
//
// PURPOSE:		Draw the background, and update the resolution variables
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Background(LTBOOL bNoDraw)
{
	// Draw the background
	if(!bNoDraw)
		g_pLTClient->ScaleSurfaceToSurface(m_hScreen, m_hBackground, &LTRect(0, 0, m_nScreenWidth, m_nScreenHeight), LTNULL);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Header()
//
// PURPOSE:		Draw the stat header
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Header(uint8 nHeaderFlags)
{
	// Setup how to draw the font for this header
	LITHFONTDRAWDATA dd;
	dd.dwFlags = LTF_DRAW_SOLID;
	dd.byJustify = LTF_JUSTIFY_CENTER;
	dd.hColor = SETRGB_T(192, 192, 192);


	// Fill in a string buffer with the header text
	char szBuffer[256];
	char szDesc[64];
	char szMisc[64];

	char szTemp[32];


	// Clear out the misc information string
	memset(szMisc, 0, 64);

	// Fill in the frags
	if(nHeaderFlags & HEADER_FRAGS)
	{
		if(m_nFragLimit)
			sprintf(szTemp, "%s%d", STRDATA(m_szStats[STAT_FRAGLIMIT]), m_nFragLimit);
		else
			sprintf(szTemp, "%s%s", STRDATA(m_szStats[STAT_FRAGLIMIT]), STRDATA(m_szStats[STAT_NA]));

		if(szMisc[0])
			strcat(szMisc, "  ");

		strcat(szMisc, szTemp);
	}

	// Fill in the score
	if(nHeaderFlags & HEADER_SCORE)
	{
		if(m_nScoreLimit)
			sprintf(szTemp, "%s%d", STRDATA(m_szStats[STAT_SCORELIMIT]), m_nScoreLimit);
		else
			sprintf(szTemp, "%s%s", STRDATA(m_szStats[STAT_SCORELIMIT]), STRDATA(m_szStats[STAT_NA]));

		if(szMisc[0])
			strcat(szMisc, "  ");

		strcat(szMisc, szTemp);
	}

	// Fill in the time
	if(nHeaderFlags & HEADER_TIME)
	{
		if(m_nTimeRemaining)
			sprintf(szTemp, "%s%d:%.2d", STRDATA(m_szStats[STAT_TIMEREMAINING]), m_nTimeRemaining / 60, m_nTimeRemaining % 60);
		else
			sprintf(szTemp, "%s%s", STRDATA(m_szStats[STAT_TIMEREMAINING]), STRDATA(m_szStats[STAT_NA]));

		if(szMisc[0])
			strcat(szMisc, "  ");

		strcat(szMisc, szTemp);
	}

	// Fill in the player count
	if(nHeaderFlags & HEADER_PLAYERS)
	{
		sprintf(szTemp, "%s (%d/%d)", STRDATA(m_szStats[STAT_TOTALPLAYERS]), m_nClients, m_nPlayerLimit);

		if(szMisc[0])
			strcat(szMisc, "  ");

		strcat(szMisc, szTemp);
	}

	// Fill in the rounds
	if(nHeaderFlags & HEADER_ROUNDS)
	{
		sprintf(szTemp, "%s (%d/%d)", STRDATA(m_szStats[STAT_ROUND]), m_nCurrentRound, m_nRoundLimit);

		if(szMisc[0])
			strcat(szMisc, "  ");

		strcat(szMisc, szTemp);
	}


	switch(g_pGameClientShell->GetGameType()->Get())
	{
		case MP_DM:			strcpy(szDesc, STRDATA(m_szStats[STAT_DEATHMATCH]));		break;
		case MP_TEAMDM:		strcpy(szDesc, STRDATA(m_szStats[STAT_TEAM_DEATHMATCH]));	break;
		case MP_HUNT:		strcpy(szDesc, STRDATA(m_szStats[STAT_ALIEN_HUNT]));		break;
		case MP_SURVIVOR:	strcpy(szDesc, STRDATA(m_szStats[STAT_MARINE_SURVIVOR]));	break;
		case MP_OVERRUN:	strcpy(szDesc, STRDATA(m_szStats[STAT_OVERRUN]));			break;
		case MP_EVAC:		strcpy(szDesc, STRDATA(m_szStats[STAT_EVAC]));				break;
	}

	sprintf(szBuffer, "%s%s\n%s%s\n%s%s\n\n%s",
			STRDATA(m_szStats[STAT_SERVER]), m_szServer,
			STRDATA(m_szStats[STAT_GAMETYPE]), szDesc,
			STRDATA(m_szStats[STAT_LEVEL]), m_szLevel,
			szMisc);


	// Get the current position to draw it at
	uint32 tX, tY;
	m_pHeaderFont->GetTextExtents(szBuffer, &dd, tX, tY);
	int y = -(int)tY + (int)(m_fDisplayRamp * (m_pHeaderFont->Height() * 7));

	// Draw it!
	m_pHeaderFont->Draw(m_hScreen, szBuffer, &dd, m_nScreenWidth / 2, y);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Header_Loading()
//
// PURPOSE:		Draw the loading text
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Header_Loading()
{
	// Setup how to draw the font for this header
	LITHFONTDRAWDATA dd;
	dd.dwFlags = LTF_DRAW_SOLID;
	dd.byJustify = LTF_JUSTIFY_CENTER;
	dd.hColor = SETRGB_T(192, 192, 192);


	// Fill in a string buffer with the loading text
	char szBuffer[256];
	sprintf(szBuffer, "%s '%s'...", STRDATA(m_szStats[STAT_LOADING]), m_szNextLevel);


	// Draw it!
	m_pHeaderFont->Draw(m_hScreen, szBuffer, &dd, m_nScreenWidth / 2, m_nScreenHeight / 2);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Stats_DM()
//
// PURPOSE:		Draw the stats for deathmatch
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Stats_DM(LTIntPt pt, uint8 nHeaderFlags, LTBOOL bDrawChecks)
{
	// Setup the table to draw everything
	if(!m_StatChart1.IsInitted())
	{
		m_StatChart1.Init(7, MAX_CHART_ROWS, pt, m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart1.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart1.SetRowSize(-1, m_pTextFont->Height());

		m_StatChart1.SetColumnSize(-1, 75);
		m_StatChart1.SetColumnSize(0, 20);
		m_StatChart1.SetColumnSize(1, 20);
		m_StatChart1.SetColumnSize(2, 20);
		m_StatChart1.SetColumnSize(3, m_nScreenWidth - 375);

		// Set the justification of the columns
		m_StatChart1.SetHJustify(-1, -1, CHART_JUSTIFY_RIGHT);
		m_StatChart1.SetHJustify(3, -1, CHART_JUSTIFY_LEFT);

		// Setup the header row
		strcpy(m_StatChart1.GetElement(3, 0)->m_szStr, STRDATA(m_szStats[STAT_PLAYERNAME]));

		int nColumn = 6;

		if(nHeaderFlags & HEADER_LIVES)
		{
			strcpy(m_StatChart1.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_LIVES]));
			nColumn--;
		}

		if(nHeaderFlags & HEADER_SCORE)
		{
			if(g_pGameClientShell->GetGameType()->Get() == MP_OVERRUN)
				strcpy(m_StatChart1.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_SURVIVED]));
			else
				strcpy(m_StatChart1.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_SCORE]));

			nColumn--;
		}

		if(nHeaderFlags & HEADER_FRAGS)
		{
			strcpy(m_StatChart1.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_FRAGS]));
			nColumn--;
		}

		strcpy(m_StatChart1.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_PING]));


		m_StatChart1.SetBGColor(-1, 0, SETRGB(48, 48, 48));
		m_StatChart1.SetDrawBG(-1, 0, LTTRUE);
		m_StatChart1.SetDrawBG(0, 0, LTFALSE);
		m_StatChart1.SetDrawBG(1, 0, LTFALSE);
		m_StatChart1.SetDrawBG(2, 0, LTFALSE);

		// The bitmaps need to be moved up some...
		m_StatChart1.SetOffset(0, -1, LTIntPt(0, -2));
		m_StatChart1.SetOffset(1, -1, LTIntPt(0, -2));
		m_StatChart1.SetOffset(2, -1, LTIntPt(0, -2));
	}


	// Set the upper left corner again in case it's changed
	m_StatChart1.SetPt(pt);


	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);


	// Get a list of clients sorted by score
	int nList[MAX_CLIENTS];
	GetSortedClientList(nList, MAX_CLIENTS, (int)nHeaderFlags);


	// Go through the clients and fill in the table
	int nRow = 2;
	int x;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		x = nList[i];

		if((x != -1) && m_ClientData[x].m_nClientID)
		{
			// Grab the default color for this client...
			HLTCOLOR hColor = m_ClientData[x].m_bObserving ? g_clrSpecies[4] : g_clrSpecies[m_ClientData[x].m_nCharacterClass];


			// If this is our player data... draw a background behind us
			if(m_ClientData[x].m_nClientID == nLocalID)
			{
				m_StatChart1.SetDrawBG(-1, nRow, LTTRUE);
				m_StatChart1.SetDrawBG(0, nRow, LTFALSE);
				m_StatChart1.SetDrawBG(1, nRow, LTFALSE);
				m_StatChart1.SetDrawBG(2, nRow, LTFALSE);
			}
			else
			{
				m_StatChart1.SetDrawBG(-1, nRow, LTFALSE);
			}

			// Cycle the color of the leader text
			if(m_nFragLeader == x || m_nScoreLeader == x)
			{
				LTFLOAT r = (LTFLOAT)(255 - GETR(hColor)) * m_fColorCycle;
				LTFLOAT g = (LTFLOAT)(255 - GETG(hColor)) * m_fColorCycle;
				LTFLOAT b = (LTFLOAT)(255 - GETB(hColor)) * m_fColorCycle;

				hColor = SETRGB(GETR(hColor) + (int)r, GETG(hColor) + (int)g, GETB(hColor) + (int)b);

				m_StatChart1.SetColor(-1, nRow, hColor);
			}
			else
				m_StatChart1.SetColor(-1, nRow, hColor);


			// This bit of code finds the index into the character class list... so we can display
			// that information next to the character's name
			int nElement = -1;

			for(int j = 0; j < m_nCharCount[m_ClientData[x].m_nCharacterClass]; j++)
			{
				if(m_nCharList[m_ClientData[x].m_nCharacterClass][j] == m_ClientData[x].m_nCharacterSet)
				{
					nElement = j;
					break;
				}
			}

			// Fill in the player data
			sprintf(m_StatChart1.GetElement(3, nRow)->m_szStr, "%s (%s)", m_ClientData[x].m_szName, (nElement == -1) ? STRDATA(m_szStats[STAT_UNKNOWN]) : STRDATA(m_szCharList[m_ClientData[x].m_nCharacterClass][nElement]));

			int nColumn = 6;

			if(nHeaderFlags & HEADER_LIVES)
			{
				sprintf(m_StatChart1.GetElement(nColumn, nRow)->m_szStr, "%d", m_ClientData[x].m_nLives);
				nColumn--;
			}

			if(nHeaderFlags & HEADER_SCORE)
			{
				sprintf(m_StatChart1.GetElement(nColumn, nRow)->m_szStr, "%d", m_ClientData[x].m_nScore);
				nColumn--;
			}

			if(nHeaderFlags & HEADER_FRAGS)
			{
				sprintf(m_StatChart1.GetElement(nColumn, nRow)->m_szStr, "%d", m_ClientData[x].m_nFrags);
				nColumn--;
			}

			sprintf(m_StatChart1.GetElement(nColumn, nRow)->m_szStr, "%d", m_ClientData[x].m_nPing);


			// Set all the icons to blank...
			m_StatChart1.GetElement(2, nRow)->m_hBitmap = LTNULL;
			m_StatChart1.GetElement(1, nRow)->m_hBitmap = LTNULL;
			m_StatChart1.GetElement(0, nRow)->m_hBitmap = LTNULL;


			// Now update all of them with the proper icons
			int nIconColumn = 2;

			// Set the check mark bitmaps if we're supposed to
//			if(bDrawChecks)
//			{
//				m_StatChart1.GetElement(nIconColumn, nRow)->m_hBitmap = m_ClientData[x].m_bVerified ? m_hCheckOn : m_hCheckOff;
//				nIconColumn--;
//			}

			// Set the bitmap for server controller if this is the right client
			if(m_ClientData[x].m_nConsecutiveFrags >= (int8)g_vtConFragsLow.GetFloat())
			{
				if(m_ClientData[x].m_nConsecutiveFrags >= (int8)g_vtConFragsHigh.GetFloat())
					m_StatChart1.GetElement(nIconColumn, nRow)->m_hBitmap = m_hFireHigh;
				else if(m_ClientData[x].m_nConsecutiveFrags >= (int8)g_vtConFragsMed.GetFloat())
					m_StatChart1.GetElement(nIconColumn, nRow)->m_hBitmap = m_hFireMed;
				else
					m_StatChart1.GetElement(nIconColumn, nRow)->m_hBitmap = m_hFireLow;

				nIconColumn--;
			}

			// Set the bitmap for server controller if this is the right client
			if(m_ClientData[x].m_nClientID == m_nServerController)
			{
				m_StatChart1.GetElement(nIconColumn, nRow)->m_hBitmap = m_hStar;
				nIconColumn--;
			}



			nRow++;
		}
	}


	// Draw the chart
	m_StatChart1.Draw(m_hScreen);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Stats_TeamDM()
//
// PURPOSE:		Draw the stats for team deathmatch
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Stats_TeamDM(LTIntPt pt, uint8 nHeaderFlags)
{
	// Setup the table to draw everything
	if(!m_StatChart2.IsInitted())
	{
		m_StatChart2.Init(3, 6, pt, m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart2.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart2.SetRowSize(-1, m_pTextFont->Height());

		m_StatChart2.SetColumnSize(-1, 75);
		m_StatChart2.SetColumnSize(0, m_nScreenWidth - 300);

		// Set the justification of the columns
		m_StatChart2.SetHJustify(-1, -1, CHART_JUSTIFY_RIGHT);
		m_StatChart2.SetHJustify(0, -1, CHART_JUSTIFY_LEFT);

		// Setup the header row
		strcpy(m_StatChart2.GetElement(0, 0)->m_szStr, STRDATA(m_szStats[STAT_TEAMNAME]));

		int nColumn = 2;

		if(nHeaderFlags & HEADER_SCORE)
		{
			if(g_pGameClientShell->GetGameType()->Get() == MP_OVERRUN)
				strcpy(m_StatChart2.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_SURVIVORS]));
			else if(g_pGameClientShell->GetGameType()->Get() == MP_EVAC)
				strcpy(m_StatChart2.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_EVACS]));
			else
				strcpy(m_StatChart2.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_SCORE]));

			nColumn--;
		}

		if(nHeaderFlags & HEADER_FRAGS)
		{
			strcpy(m_StatChart2.GetElement(nColumn, 0)->m_szStr, STRDATA(m_szStats[STAT_FRAGS]));
			nColumn--;
		}

		m_StatChart2.SetBGColor(-1, 0, SETRGB(48, 48, 48));
		m_StatChart2.SetDrawBG(-1, 0, LTTRUE);
	}


	// Set the upper left corner again in case it's changed
	m_StatChart2.SetPt(pt);


	// Get a list of clients sorted by score
	int nList[MAX_TEAMS];
	GetSortedTeamList(nList, MAX_TEAMS, (int)nHeaderFlags);


	// Go through the clients and fill in the table
	int nRow = 2;
	int x;

	for(int i = 0; i < MAX_TEAMS; i++)
	{
		x = nList[i];

		if((x != -1) && (m_nTeamLimits[x] > 0))
		{
			if(m_nTeamFragLeader == x || m_nTeamScoreLeader == x)
			{
				HLTCOLOR hColor = g_clrSpecies[x];
				LTFLOAT r = (LTFLOAT)(255 - GETR(hColor)) * m_fColorCycle;
				LTFLOAT g = (LTFLOAT)(255 - GETG(hColor)) * m_fColorCycle;
				LTFLOAT b = (LTFLOAT)(255 - GETB(hColor)) * m_fColorCycle;

				hColor = SETRGB(GETR(hColor) + (int)r, GETG(hColor) + (int)g, GETB(hColor) + (int)b);

				m_StatChart2.SetColor(-1, nRow, hColor);
			}
			else
				m_StatChart2.SetColor(-1, nRow, g_clrSpecies[x]);

			sprintf(m_StatChart2.GetElement(0, nRow)->m_szStr, "%s", STRDATA(m_szTeams[x]));

			int nColumn = 2;

			if(nHeaderFlags & HEADER_SCORE)
			{
				sprintf(m_StatChart2.GetElement(nColumn, nRow)->m_szStr, "%d", m_TeamData[x].m_nScore);
				nColumn--;
			}

			if(nHeaderFlags & HEADER_FRAGS)
			{
				sprintf(m_StatChart2.GetElement(nColumn, nRow)->m_szStr, "%d", m_TeamData[x].m_nFrags);
				nColumn--;
			}

			nRow++;
		}
	}


	// Draw the chart
	m_StatChart2.Draw(m_hScreen);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Server_Rules()
//
// PURPOSE:		Draw the server rules
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Server_Rules(LTIntPt pt)
{
	// Setup the table to draw everything
	if(!m_StatChart8.IsInitted())
	{
		// Set some temp vars
		char szTemp[256];
		szTemp[0] = 0;

		// Set some default strings
		HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
		HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);

		HSTRING hTeams[MAX_TEAMS];
		hTeams[0] = g_pLTClient->FormatString(IDS_MP_TEAM_ALIENS);
		hTeams[1] = g_pLTClient->FormatString(IDS_MP_TEAM_MARINES);
		hTeams[2] = g_pLTClient->FormatString(IDS_MP_TEAM_PREDATORS);
		hTeams[3] = g_pLTClient->FormatString(IDS_MP_TEAM_CORPORATES);

		// Calculate the bandwidth
		HSTRING	hBandwidth;
		if(m_nBandwidth <= 15999)
			hBandwidth = g_pLTClient->FormatString(IDS_CONNECT_VSLOW);
		else if (m_nBandwidth <= 31999)
			hBandwidth = g_pLTClient->FormatString(IDS_CONNECT_SLOW);
		else if (m_nBandwidth <= 999999)
			hBandwidth = g_pLTClient->FormatString(IDS_CONNECT_MEDIUM);
		else if (m_nBandwidth <= 9999999)
			hBandwidth = g_pLTClient->FormatString(IDS_CONNECT_FAST);
		else
			hBandwidth = g_pLTClient->FormatString(IDS_CONNECT_VFAST);

		
		
		// Init the chart
		int nRows = 14;  // 3 headers, 2 gaps and 9 advanced items....

		switch(g_pGameClientShell->GetGameType()->Get())
		{
			case MP_DM:			nRows += 9;	break;
			case MP_TEAMDM:		nRows += 9;	break;
			case MP_HUNT:		nRows += 6;	break;
			case MP_SURVIVOR:	nRows += 6;	break;
			case MP_OVERRUN:	nRows += 7;	break;
			case MP_EVAC:		nRows += 7;	break;
		}

		// figure out our x pos
		pt.x = (m_nScreenWidth>>1) - 200;
		m_StatChart8.Init(2, nRows, pt, m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart8.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart8.SetRowSize(-1, m_pTextFont->Height());

		m_StatChart8.SetColumnSize(-1, 200);

		// Set the justification of the columns
		m_StatChart8.SetHJustify(-1, -1, CHART_JUSTIFY_RIGHT);
		m_StatChart8.SetHJustify(0, -1, CHART_JUSTIFY_LEFT);

		// Setup the bandwidth row 
		strcpy(m_StatChart8.GetElement(0, 0)->m_szStr, STRDATA(m_szSRules[SR_BANDWIDTH]));
		strcpy(m_StatChart8.GetElement(1, 0)->m_szStr, STRDATA(hBandwidth));

		// Setup the header row for standard settings
		strcpy(m_StatChart8.GetElement(0, 2)->m_szStr, STRDATA(m_szSRules[SR_GAME_SETTINGS]));

		// Setup the standard settings based on game type
		nRows = 3;
		switch(g_pGameClientShell->GetGameType()->Get())
		{
			case MP_TEAMDM:
			case MP_DM:
			{
				strcpy(m_StatChart8.GetElement(0, 3)->m_szStr, STRDATA(m_szSRules[SR_MAX_PLAYERS]));
				sprintf(szTemp, "%d", m_nPlayerLimit);
				strcpy(m_StatChart8.GetElement(1, 3)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 4)->m_szStr, STRDATA(m_szSRules[SR_MAX_MARINES]));
				sprintf(szTemp, "%d", m_nTeamLimits[Marine]);
				strcpy(m_StatChart8.GetElement(1, 4)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 5)->m_szStr, STRDATA(m_szSRules[SR_MAX_ALIENS]));
				sprintf(szTemp, "%d", m_nTeamLimits[Alien]);
				strcpy(m_StatChart8.GetElement(1, 5)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 6)->m_szStr, STRDATA(m_szSRules[SR_MAX_PREDATORS]));
				sprintf(szTemp, "%d", m_nTeamLimits[Predator]);
				strcpy(m_StatChart8.GetElement(1, 6)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 7)->m_szStr, STRDATA(m_szSRules[SR_MAX_CORPORATES]));
				sprintf(szTemp, "%d", m_nTeamLimits[Corporate]);
				strcpy(m_StatChart8.GetElement(1, 7)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 8)->m_szStr, STRDATA(m_szSRules[SR_FRAG_LIMIT]));
				sprintf(szTemp, "%d", m_nFragLimit);
				strcpy(m_StatChart8.GetElement(1, 8)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 9)->m_szStr, STRDATA(m_szSRules[SR_SCORE_LIMIT]));
				sprintf(szTemp, "%d", m_nScoreLimit);
				strcpy(m_StatChart8.GetElement(1, 9)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 10)->m_szStr, STRDATA(m_szSRules[SR_TIME_LIMIT]));
				sprintf(szTemp, "%d %s", m_nTimeLimit, "mins");
				strcpy(m_StatChart8.GetElement(1, 10)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 11)->m_szStr, STRDATA(m_szSRules[SR_LIFECYCLE]));
				strcpy(m_StatChart8.GetElement(1, 11)->m_szStr, m_bLifecycle?STRDATA(hOn):STRDATA(hOff));

				nRows += 9;
				break;
			}

			case MP_HUNT:
			{
				strcpy(m_StatChart8.GetElement(0, 3)->m_szStr, STRDATA(m_szSRules[SR_MAX_PLAYERS]));
				sprintf(szTemp, "%d", m_nPlayerLimit);
				strcpy(m_StatChart8.GetElement(1, 3)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 4)->m_szStr, STRDATA(m_szSRules[SR_HUNTER_RACE]));
				strcpy(m_StatChart8.GetElement(1, 4)->m_szStr, STRDATA(hTeams[m_nHunt_HunterRace]));

				strcpy(m_StatChart8.GetElement(0, 5)->m_szStr, STRDATA(m_szSRules[SR_PREY_RACE]));
				strcpy(m_StatChart8.GetElement(1, 5)->m_szStr, STRDATA(hTeams[m_nHunt_PreyRace]));

				strcpy(m_StatChart8.GetElement(0, 6)->m_szStr, STRDATA(m_szSRules[SR_HUNT_RATIO]));
				sprintf(szTemp, "%d", m_nHunt_Ratio);
				strcpy(m_StatChart8.GetElement(1, 6)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 7)->m_szStr, STRDATA(m_szSRules[SR_FRAG_LIMIT]));
				sprintf(szTemp, "%d", m_nFragLimit);
				strcpy(m_StatChart8.GetElement(1, 7)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 8)->m_szStr, STRDATA(m_szSRules[SR_TIME_LIMIT]));
				sprintf(szTemp, "%d %s", m_nTimeLimit, "mins");
				strcpy(m_StatChart8.GetElement(1, 8)->m_szStr, szTemp);

				nRows += 6;
				break;
			}

			case MP_SURVIVOR:
			{
				strcpy(m_StatChart8.GetElement(0, 3)->m_szStr, STRDATA(m_szSRules[SR_MAX_PLAYERS]));
				sprintf(szTemp, "%d", m_nPlayerLimit);
				strcpy(m_StatChart8.GetElement(1, 3)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 4)->m_szStr, STRDATA(m_szSRules[SR_SURVIVOR_RACE]));
				strcpy(m_StatChart8.GetElement(1, 4)->m_szStr, STRDATA(hTeams[m_nSurv_SurvivorRace]));

				strcpy(m_StatChart8.GetElement(0, 5)->m_szStr, STRDATA(m_szSRules[SR_MUTATE_RACE]));
				strcpy(m_StatChart8.GetElement(1, 5)->m_szStr, STRDATA(hTeams[m_nSurv_MutateRace]));

				strcpy(m_StatChart8.GetElement(0, 6)->m_szStr, STRDATA(m_szSRules[SR_SCORE_LIMIT]));
				sprintf(szTemp, "%d", m_nScoreLimit);
				strcpy(m_StatChart8.GetElement(1, 6)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 7)->m_szStr, STRDATA(m_szSRules[SR_ROUND_LIMIT]));
				sprintf(szTemp, "%d", m_nRoundLimit);
				strcpy(m_StatChart8.GetElement(1, 7)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 8)->m_szStr, STRDATA(m_szSRules[SR_ROUND_TIME]));
				sprintf(szTemp, "%d %s", m_nTimeLimit, "mins");
				strcpy(m_StatChart8.GetElement(1, 8)->m_szStr, szTemp);

				nRows += 6;
				break;
			}

			case MP_OVERRUN:
			{
				strcpy(m_StatChart8.GetElement(0, 3)->m_szStr, STRDATA(m_szSRules[SR_MAX_PLAYERS]));
				sprintf(szTemp, "%d", m_nPlayerLimit);
				strcpy(m_StatChart8.GetElement(1, 3)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 4)->m_szStr, STRDATA(m_szSRules[SR_DEFENDER_RACE]));
				strcpy(m_StatChart8.GetElement(1, 4)->m_szStr, STRDATA(hTeams[m_nTM_DefenderRace]));

				strcpy(m_StatChart8.GetElement(0, 5)->m_szStr, STRDATA(m_szSRules[SR_DEFENDER_LIVES]));
				sprintf(szTemp, "%d", m_nTM_DefenderLives);
				strcpy(m_StatChart8.GetElement(1, 5)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 6)->m_szStr, STRDATA(m_szSRules[SR_ATTACKER_RACE]));
				strcpy(m_StatChart8.GetElement(1, 6)->m_szStr, STRDATA(hTeams[m_nTM_AttackerRace]));

				strcpy(m_StatChart8.GetElement(0, 7)->m_szStr, STRDATA(m_szSRules[SR_ATTACKER_LIVES]));
				sprintf(szTemp, "%d", m_nTM_AttackerLives);
				strcpy(m_StatChart8.GetElement(1, 7)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 8)->m_szStr, STRDATA(m_szSRules[SR_ROUND_LIMIT]));
				sprintf(szTemp, "%d", m_nRoundLimit);
				strcpy(m_StatChart8.GetElement(1, 8)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 9)->m_szStr, STRDATA(m_szSRules[SR_ROUND_TIME]));
				sprintf(szTemp, "%d %s", m_nTimeLimit, "mins");
				strcpy(m_StatChart8.GetElement(1, 9)->m_szStr, szTemp);

				nRows += 7;
				break;
			}

			case MP_EVAC:
			{
				strcpy(m_StatChart8.GetElement(0, 3)->m_szStr, STRDATA(m_szSRules[SR_MAX_PLAYERS]));
				sprintf(szTemp, "%d", m_nPlayerLimit);
				strcpy(m_StatChart8.GetElement(1, 3)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 4)->m_szStr, STRDATA(m_szSRules[SR_DEFENDER_RACE]));
				strcpy(m_StatChart8.GetElement(1, 4)->m_szStr, STRDATA(hTeams[m_nTM_DefenderRace]));

				strcpy(m_StatChart8.GetElement(0, 5)->m_szStr, STRDATA(m_szSRules[SR_DEFENDER_LIVES]));
				sprintf(szTemp, "%d", m_nTM_DefenderLives);
				strcpy(m_StatChart8.GetElement(1, 5)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 6)->m_szStr, STRDATA(m_szSRules[SR_ATTACKER_RACE]));
				strcpy(m_StatChart8.GetElement(1, 6)->m_szStr, STRDATA(hTeams[m_nTM_AttackerRace]));

				strcpy(m_StatChart8.GetElement(0, 7)->m_szStr, STRDATA(m_szSRules[SR_ATTACKER_LIVES]));
				sprintf(szTemp, "%d", m_nTM_AttackerLives);
				strcpy(m_StatChart8.GetElement(1, 7)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 8)->m_szStr, STRDATA(m_szSRules[SR_ROUND_LIMIT]));
				sprintf(szTemp, "%d", m_nRoundLimit);
				strcpy(m_StatChart8.GetElement(1, 8)->m_szStr, szTemp);

				strcpy(m_StatChart8.GetElement(0, 9)->m_szStr, STRDATA(m_szSRules[SR_ROUND_TIME]));
				sprintf(szTemp, "%d %s", m_nTimeLimit, "mins");
				strcpy(m_StatChart8.GetElement(1, 9)->m_szStr, szTemp);

				nRows += 7;
				break;
			}
		}

		// Setup the header for advanced settings
		strcpy(m_StatChart8.GetElement(0, nRows + 1)->m_szStr, STRDATA(m_szSRules[SR_ADV_SETTINGS]));

		// Setup the advanced options
		strcpy(m_StatChart8.GetElement(0, nRows + 2)->m_szStr, STRDATA(m_szSRules[SR_DAMAGE_RATE]));
		sprintf(szTemp, "%d%%", (int)(m_fDamageScale * 100.0f));
		strcpy(m_StatChart8.GetElement(1, nRows + 2)->m_szStr, szTemp);

		strcpy(m_StatChart8.GetElement(0, nRows + 3)->m_szStr, STRDATA(m_szSRules[SR_LOC_DAMAGE]));
		strcpy(m_StatChart8.GetElement(1, nRows + 3)->m_szStr, m_bLocationDamage?STRDATA(hOn):STRDATA(hOff));

		strcpy(m_StatChart8.GetElement(0, nRows + 4)->m_szStr, STRDATA(m_szSRules[SR_FRIENDLY_FIRE]));
		strcpy(m_StatChart8.GetElement(1, nRows + 4)->m_szStr, m_bFriendlyFire?STRDATA(hOn):STRDATA(hOff));

		strcpy(m_StatChart8.GetElement(0, nRows + 5)->m_szStr, STRDATA(m_szSRules[SR_FRIENDLY_NAMES]));
		strcpy(m_StatChart8.GetElement(1, nRows + 5)->m_szStr, m_bFriendlyNames?STRDATA(hOff):STRDATA(hOn));

		strcpy(m_StatChart8.GetElement(0, nRows + 6)->m_szStr, STRDATA(m_szSRules[SR_PRED_MASK_LOSS]));
		strcpy(m_StatChart8.GetElement(1, nRows + 6)->m_szStr, m_bMaskLoss?STRDATA(hOn):STRDATA(hOff));

		strcpy(m_StatChart8.GetElement(0, nRows + 7)->m_szStr, STRDATA(m_szSRules[SR_CLASS_WEAPONS]));
		strcpy(m_StatChart8.GetElement(1, nRows + 7)->m_szStr, m_bClassWeapons?STRDATA(hOn):STRDATA(hOff));

		strcpy(m_StatChart8.GetElement(0, nRows + 8)->m_szStr, STRDATA(m_szSRules[SR_EXOSUIT]));
		sprintf(szTemp, "%d", m_nExosuit);
		strcpy(m_StatChart8.GetElement(1, nRows + 8)->m_szStr, szTemp);

		strcpy(m_StatChart8.GetElement(0, nRows + 9)->m_szStr, STRDATA(m_szSRules[SR_QUEEN]));
		sprintf(szTemp, "%d", m_nQueenMolt);
		strcpy(m_StatChart8.GetElement(1, nRows + 9)->m_szStr, szTemp);

		strcpy(m_StatChart8.GetElement(0, nRows + 10)->m_szStr, STRDATA(m_szSRules[SR_CUSTOM_SCORING]));
		HSTRING hCustom = m_bCustScoring ? g_pLTClient->FormatString(IDS_HOST_CUSTOM) : g_pLTClient->FormatString(IDS_HOST_DEFAULT);
		strcpy(m_StatChart8.GetElement(1, nRows + 10)->m_szStr, STRDATA(hCustom));
		g_pLTClient->FreeString(hCustom);

		// Create the borders
		m_StatChart8.SetBGColor(-1, 2, SETRGB(0, 32, 128));
		m_StatChart8.SetDrawBG(-1, 2, LTTRUE);

		m_StatChart8.SetBGColor(-1, nRows + 1, SETRGB(0, 32, 128));
		m_StatChart8.SetDrawBG(-1, nRows + 1, LTTRUE);

		if(m_nBandwidth <= 4000)
			m_StatChart8.SetBGColor(-1, 0, SETRGB(255, 0, 0));
		else if (m_nBandwidth <= 16000)
			m_StatChart8.SetBGColor(-1, 0, SETRGB(225, 75, 25));
		else if (m_nBandwidth <= 32000)
		{
			m_StatChart8.SetBGColor(-1, 0, SETRGB(255, 255, 0));
			m_StatChart8.SetColor(-1, 0, SETRGB(0, 0, 0));
		}
		else if (m_nBandwidth <= 1000000)
		{
			m_StatChart8.SetBGColor(-1, 0, SETRGB(0, 255, 255));
			m_StatChart8.SetColor(-1, 0, SETRGB(0, 0, 0));
		}
		else
		{
			m_StatChart8.SetBGColor(-1, 0, SETRGB(0, 255, 0));
			m_StatChart8.SetColor(-1, 0, SETRGB(0, 0, 0));
		}
		m_StatChart8.SetDrawBG(-1, 0, LTTRUE);



		// Clean up
		g_pLTClient->FreeString(hTeams[0]);
		g_pLTClient->FreeString(hTeams[1]);
		g_pLTClient->FreeString(hTeams[2]);
		g_pLTClient->FreeString(hTeams[3]);

		g_pLTClient->FreeString(hBandwidth);

		g_pLTClient->FreeString(hOn);
		g_pLTClient->FreeString(hOff);
	}

	// Set the upper left corner again in case it's changed
	pt.x = (m_nScreenWidth>>1) - 200;
	m_StatChart8.SetPt(pt);

	// Draw the chart
	m_StatChart8.Draw(m_hScreen);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Stats_Round()
//
// PURPOSE:		Draw the stats for round transitions
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Stats_Round()
{
	// Setup the table to draw everything
	if(!m_StatChart6.IsInitted())
	{
		int nRows = MAX_CLIENTS * 2;

		m_StatChart6.Init(6, nRows, LTIntPt(0, 0), m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart6.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart6.SetRowSize(-1, m_pTextFont->Height());
		m_StatChart6.SetRowSize(0, m_pHeaderFont->Height() + 6);
		m_StatChart6.SetRowSize(nRows - 1, 5);

		m_StatChart6.SetColumnSize(0, 5);
		m_StatChart6.SetColumnSize(1, 15);
		m_StatChart6.SetColumnSize(2, (m_nScreenWidth / 2) - 140);
		m_StatChart6.SetColumnSize(3, 100);
		m_StatChart6.SetColumnSize(4, 15);
		m_StatChart6.SetColumnSize(5, 5);

		// Set the justification of the columns
		m_StatChart6.SetHJustify(-1, -1, CHART_JUSTIFY_LEFT);
		m_StatChart6.SetHJustify(3, -1, CHART_JUSTIFY_RIGHT);

		int nHeaderRow = 2;

		if(g_pGameClientShell->GetGameType()->IsTeamMode())
		{
			// Setup the header row
			strcpy(m_StatChart6.GetElement(2, nHeaderRow)->m_szStr, STRDATA(m_szStats[STAT_TEAMNAME]));

			if(g_pGameClientShell->GetGameType()->Get() == MP_OVERRUN)
				strcpy(m_StatChart6.GetElement(3, nHeaderRow)->m_szStr, STRDATA(m_szStats[STAT_SURVIVORS]));
			else if(g_pGameClientShell->GetGameType()->Get() == MP_EVAC)
				strcpy(m_StatChart6.GetElement(3, nHeaderRow)->m_szStr, STRDATA(m_szStats[STAT_EVACS]));
			else
				strcpy(m_StatChart6.GetElement(3, nHeaderRow)->m_szStr, STRDATA(m_szStats[STAT_SCORE]));

			m_StatChart6.SetBGColor(-1, nHeaderRow, SETRGB(48, 48, 48));
			m_StatChart6.SetDrawBG(-1, nHeaderRow, LTTRUE);

			nHeaderRow = 9;
		}

		// Setup the header row
		strcpy(m_StatChart6.GetElement(2, nHeaderRow)->m_szStr, STRDATA(m_szStats[STAT_PLAYERNAME]));

		if(g_pGameClientShell->GetGameType()->Get() == MP_OVERRUN)
			strcpy(m_StatChart6.GetElement(3, nHeaderRow)->m_szStr, STRDATA(m_szStats[STAT_SURVIVED]));
		else if(g_pGameClientShell->GetGameType()->Get() == MP_EVAC)
			strcpy(m_StatChart6.GetElement(3, nHeaderRow)->m_szStr, " ");
		else
			strcpy(m_StatChart6.GetElement(3, nHeaderRow)->m_szStr, STRDATA(m_szStats[STAT_SCORE]));

		m_StatChart6.SetBGColor(-1, nHeaderRow, SETRGB(48, 48, 48));
		m_StatChart6.SetDrawBG(-1, nHeaderRow, LTTRUE);


		// Create the borders
		m_StatChart6.SetBGColor(-1, 0, SETRGB(0, 32, 128));
		m_StatChart6.SetDrawBG(-1, 0, LTTRUE);

		m_StatChart6.SetBGColor(-1, nRows - 1, SETRGB(0, 32, 128));
		m_StatChart6.SetDrawBG(-1, nRows - 1, LTTRUE);

		m_StatChart6.SetBGColor(0, -1, SETRGB(0, 32, 128));
		m_StatChart6.SetDrawBG(0, -1, LTTRUE);

		m_StatChart6.SetBGColor(5, -1, SETRGB(0, 32, 128));
		m_StatChart6.SetDrawBG(5, -1, LTTRUE);

		// Set the upper left corner again in case it's changed
		LTIntPt ptDims = m_StatChart6.GetChartDims(-1, -1);
		m_StatChart6.SetPt(LTIntPt((m_nScreenWidth - ptDims.x) / 2, (m_nScreenHeight - ptDims.y) / 2));
	}


	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);


	// Go through the teams and fill in the table
	int nRow = 4;
	int x;


	if(g_pGameClientShell->GetGameType()->IsTeamMode())
	{
		// Get a list of clients sorted by score
		int nTList[MAX_TEAMS];
		GetSortedTeamList(nTList, MAX_TEAMS, 0);


		// Go through the clients and fill in the table
		for(int i = 0; i < MAX_TEAMS; i++)
		{
			x = nTList[i];

			if((x != -1) && (m_nTeamLimits[x] > 0))
			{
				// Grab the default color for this client...
				HLTCOLOR hColor = g_clrSpecies[x];

				// Cycle the color of the leader text
				if(m_nTeamFragLeader == x || m_nTeamScoreLeader == x)
				{
					LTFLOAT r = (LTFLOAT)(255 - GETR(hColor)) * m_fColorCycle;
					LTFLOAT g = (LTFLOAT)(255 - GETG(hColor)) * m_fColorCycle;
					LTFLOAT b = (LTFLOAT)(255 - GETB(hColor)) * m_fColorCycle;

					hColor = SETRGB(GETR(hColor) + (int)r, GETG(hColor) + (int)g, GETB(hColor) + (int)b);
				}

				m_StatChart6.SetColor(2, nRow, hColor);
				m_StatChart6.SetColor(3, nRow, hColor);


				// Fill in the player data
				sprintf(m_StatChart6.GetElement(2, nRow)->m_szStr, "%s", STRDATA(m_szTeams[x]));
				sprintf(m_StatChart6.GetElement(3, nRow)->m_szStr, "%d", m_TeamData[x].m_nScore);

				nRow++;
			}
		}

		nRow = 11;
	}
	

	// Get a list of clients sorted by score
	int nList[MAX_CLIENTS];
	GetSortedClientList(nList, MAX_CLIENTS, 0);


	// Go through the clients and fill in the table
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		x = nList[i];

		if((x != -1) && m_ClientData[x].m_nClientID)
		{
			// Grab the default color for this client...
			HLTCOLOR hColor = m_ClientData[x].m_bObserving ? g_clrSpecies[4] : g_clrSpecies[m_ClientData[x].m_nCharacterClass];


			// If this is our player data... draw a background behind us
			if(m_ClientData[x].m_nClientID == nLocalID)
			{
				m_StatChart6.SetDrawBG(1, nRow, LTTRUE);
				m_StatChart6.SetDrawBG(2, nRow, LTTRUE);
				m_StatChart6.SetDrawBG(3, nRow, LTTRUE);
				m_StatChart6.SetDrawBG(4, nRow, LTTRUE);
			}
			else
			{
				m_StatChart6.SetDrawBG(1, nRow, LTFALSE);
				m_StatChart6.SetDrawBG(2, nRow, LTFALSE);
				m_StatChart6.SetDrawBG(3, nRow, LTFALSE);
				m_StatChart6.SetDrawBG(4, nRow, LTFALSE);
			}

			// Cycle the color of the leader text
			if(m_nFragLeader == x || m_nScoreLeader == x)
			{
				LTFLOAT r = (LTFLOAT)(255 - GETR(hColor)) * m_fColorCycle;
				LTFLOAT g = (LTFLOAT)(255 - GETG(hColor)) * m_fColorCycle;
				LTFLOAT b = (LTFLOAT)(255 - GETB(hColor)) * m_fColorCycle;

				hColor = SETRGB(GETR(hColor) + (int)r, GETG(hColor) + (int)g, GETB(hColor) + (int)b);
			}

			m_StatChart6.SetColor(2, nRow, hColor);
			m_StatChart6.SetColor(3, nRow, hColor);


			// This bit of code finds the index into the character class list... so we can display
			// that information next to the character's name
			int nElement = -1;

			for(int j = 0; j < m_nCharCount[m_ClientData[x].m_nCharacterClass]; j++)
			{
				if(m_nCharList[m_ClientData[x].m_nCharacterClass][j] == m_ClientData[x].m_nCharacterSet)
				{
					nElement = j;
					break;
				}
			}

			// Fill in the player data
			sprintf(m_StatChart6.GetElement(2, nRow)->m_szStr, "%s", m_ClientData[x].m_szName);

			if(g_pGameClientShell->GetGameType()->Get() != MP_EVAC)
				sprintf(m_StatChart6.GetElement(3, nRow)->m_szStr, "%d", m_ClientData[x].m_nScore);

			nRow++;
		}
	}


	// Draw the background
	LTIntPt pt = m_StatChart6.GetPt();
	LTIntPt ptDims = m_StatChart6.GetChartDims(-1, -1);

	g_pLTClient->ScaleSurfaceToSurface(m_hScreen, m_hBackground, &LTRect(pt.x, pt.y, pt.x + ptDims.x, pt.y + ptDims.y), LTNULL);


	// Draw the chart
	m_StatChart6.Draw(m_hScreen);


	// Get the current position to draw it at
	LITHFONTDRAWDATA dd;
	dd.dwFlags = LTF_DRAW_SOLID;
	dd.byJustify = LTF_JUSTIFY_CENTER;
	dd.hColor = SETRGB_T(224, 224, 224);

	char szBuffer[256];
	sprintf(szBuffer, "%s %d %s", STRDATA(m_szStats[STAT_ROUND]), m_nCurrentRound, STRDATA(m_szStats[STAT_SUMMARY]));

	// Draw it!
	m_pHeaderFont->Draw(m_hScreen, szBuffer, &dd, m_nScreenWidth / 2, pt.y + 3);


	// Draw the wait seconds...
	sprintf(szBuffer, "%s%d", STRDATA(m_szStats[STAT_ROUNDSTART]), m_nWaitSeconds);

	int y = (int)m_nScreenHeight - (m_pTextFont->Height() * 2);
	m_pTextFont->Draw(m_hScreen, szBuffer, &dd, m_nScreenWidth / 2, y);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Footer_Summary()
//
// PURPOSE:		Draw the standard summary footer information
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Footer_Summary()
{
	// Setup how to draw the font for this header
	LITHFONTDRAWDATA dd;
	dd.dwFlags = LTF_DRAW_SOLID;
	dd.byJustify = LTF_JUSTIFY_CENTER;
	dd.hColor = SETRGB_T(224, 224, 224);


	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);

	int nIndex = FindClient(nLocalID);
	if(nIndex == -1) return;


	// Fill in a string buffer with the header text
	char szBuffer[256];

	sprintf(szBuffer, "%s%s\n%s%d",
			STRDATA(m_szStats[STAT_NEXTLEVEL]), m_szNextLevel,
			STRDATA(m_szStats[STAT_LEVELSTART]), m_nWaitSeconds);

/*	sprintf(szBuffer, "%s%s\n%s\n\n%s%d",
			STRDATA(m_szStats[STAT_NEXTLEVEL]), m_szNextLevel,
			m_ClientData[nIndex].m_bVerified ? STRDATA(m_szStats[STAT_WAITING]) : STRDATA(m_szStats[STAT_VERIFY]),
			STRDATA(m_szStats[STAT_LEVELSTART]), m_nWaitSeconds);
*/

	// Get the current position to draw it at
	uint32 tX, tY;
	m_pTextFont->GetTextExtents(szBuffer, &dd, tX, tY);
	int y = (int)m_nScreenHeight - (int)(tY) - (m_pTextFont->Height() * 2);

	// Draw it!
	m_pTextFont->Draw(m_hScreen, szBuffer, &dd, m_nScreenWidth / 2, y);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Player_Info()
//
// PURPOSE:		Draws the current player information
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Player_Info(LTIntPt pt)
{
	// Setup how to draw the font for this header
	LITHFONTDRAWDATA dd;
	dd.dwFlags = LTF_DRAW_SOLID;
	dd.byJustify = LTF_JUSTIFY_CENTER;
	dd.hColor = SETRGB_T(192, 192, 192);


	// Now draw the help text at the bottom of the screen
	dd.hColor = SETRGB_T(255, 255, 255);
	int y = (int)m_nScreenHeight - (int)(m_fDisplayRamp * (m_pTextFont->Height() * 2));
	m_pTextFont->Draw(m_hScreen, STRDATA(m_szStats[STAT_PLAYERINFO]), &dd, m_nScreenWidth / 2, y);


	// Setup the table to draw everything
	if(!m_StatChart3.IsInitted())
	{
		m_StatChart3.Init(4, MAX_CHART_ROWS, pt, m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart3.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart3.SetRowSize(-1, m_pTextFont->Height() * 2);
		m_StatChart3.SetRowSize(0, m_pTextFont->Height());
		m_StatChart3.SetRowSize(1, m_pTextFont->Height());

		m_StatChart3.SetColumnSize(-1, m_nScreenWidth / 4);

		// Set the justification
		m_StatChart3.SetHJustify(-1, -1, CHART_JUSTIFY_CENTER);

		m_StatChart3.SetBGColor(-1, 0, SETRGB(48, 48, 48));
	}


	// Set the upper left corner again in case it's changed
	m_StatChart3.SetPt(pt);


	// Clear the strings and background drawing
	m_StatChart3.ClearString(-1, -1);
	m_StatChart3.SetDrawBG(-1, -1, LTFALSE);
	m_StatChart3.SetDrawBG(-1, 0, LTTRUE);


	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);


	// Make some total values
	int nTeamPlayers[MAX_TEAMS + 1] = { 0, 0, 0, 0, 0 };
	uint8 nColumn, nRow;

	// Go through the list and add up some totals to print out
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_nClientID)
		{
			uint8 nClass = (m_ClientData[i].m_nClientID == nLocalID) ? m_nCurrentClass : m_ClientData[i].m_nCharacterClass;
			uint8 nSet = (m_ClientData[i].m_nClientID == nLocalID) ? m_nCurrentSet : m_ClientData[i].m_nCharacterSet;

			// Default to the unknown players
			nColumn = MAX_TEAMS;

			if((nClass >= 0) && (nClass < MAX_TEAMS))
				nColumn = nClass;

			// Increment the number of players in this category
			nTeamPlayers[nColumn]++;
			nRow = nTeamPlayers[nColumn] + 1;


			// This bit of code finds the index into the character class list... so we can display
			// that information next to the character's name
			int nElement = -1;

			for(int j = 0; j < m_nCharCount[nClass]; j++)
			{
				if(m_nCharList[nClass][j] == nSet)
				{
					nElement = j;
					break;
				}
			}

			// Set the name of this player
			sprintf(m_StatChart3.GetElement(nColumn, nRow)->m_szStr, "%s\n(%s)", m_ClientData[i].m_szName, (nElement == -1) ? STRDATA(m_szStats[STAT_UNKNOWN]) : STRDATA(m_szCharList[nClass][nElement]));
			m_StatChart3.SetColor(nColumn, nRow, m_ClientData[i].m_bObserving ? g_clrSpecies[MAX_TEAMS] : g_clrSpecies[nColumn]);

			// If this is our player... draw a background behind us
			if(m_ClientData[i].m_nClientID == nLocalID)
				m_StatChart3.GetElement(nColumn, nRow)->m_bDrawBG = LTTRUE;
			else
				m_StatChart3.GetElement(nColumn, nRow)->m_bDrawBG = LTFALSE;
		}
	}


	// Setup the header row
	sprintf(m_StatChart3.GetElement(0, 0)->m_szStr, "%s (%d/%d)", STRDATA(m_szTeams[0]), nTeamPlayers[0], m_nTeamLimits[0]);
	sprintf(m_StatChart3.GetElement(1, 0)->m_szStr, "%s (%d/%d)", STRDATA(m_szTeams[1]), nTeamPlayers[1], m_nTeamLimits[1]);
	sprintf(m_StatChart3.GetElement(2, 0)->m_szStr, "%s (%d/%d)", STRDATA(m_szTeams[2]), nTeamPlayers[2], m_nTeamLimits[2]);
	sprintf(m_StatChart3.GetElement(3, 0)->m_szStr, "%s (%d/%d)", STRDATA(m_szTeams[3]), nTeamPlayers[3], m_nTeamLimits[3]);
//	sprintf(m_StatChart3.GetElement(4, 0)->m_szStr, "%s (%d)", STRDATA(m_szTeams[4]), nTeamPlayers[4], nTeamPlayers[4]);


	// Draw the chart
	m_StatChart3.Draw(m_hScreen);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Player_Selection()
//
// PURPOSE:		Draws the player selection interface
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Player_Selection()
{
	int i, j;

	// Setup the table to draw everything
	if(!m_StatChart4.IsInitted())
	{
		m_StatChart4.Init(4, MAX_CHART_ROWS, LTIntPt(0, 0), m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart4.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart4.SetRowSize(-1, m_pTextFont->Height());
		m_StatChart4.SetColumnSize(-1, m_nScreenWidth / 4);

		// Set the justification
		m_StatChart4.SetHJustify(-1, -1, CHART_JUSTIFY_CENTER);
		m_StatChart4.SetHJustify(1, 0, CHART_JUSTIFY_RIGHT);
		m_StatChart4.SetHJustify(2, 0, CHART_JUSTIFY_LEFT);

		// Setup the header row
		sprintf(m_StatChart4.GetElement(1, 0)->m_szStr, "%s ", STRDATA(m_szStats[STAT_PLAYER]));
		sprintf(m_StatChart4.GetElement(2, 0)->m_szStr, "%s:", STRDATA(m_szStats[STAT_SELECTION]));

		m_StatChart4.SetBGColor(-1, 0, SETRGB(48, 48, 48));
		m_StatChart4.SetDrawBG(-1, 0, LTTRUE);


		// Go through the entire list of characters and fill in their names
		for(i = 0; i < MAX_TEAMS; i++)
		{
			for(j = 0; j < m_nCharCount[i]; j++)
			{
				if(m_szCharList[i][j])
				{
					strcpy(m_StatChart4.GetElement(i, j + 2)->m_szStr, STRDATA(m_szCharList[i][j]));
				}
			}
		}

		// Set the upper left corner again in case it's changed
		LTIntPt ptDims = m_StatChart4.GetChartDims(0, 10);
		m_StatChart4.SetPt(LTIntPt(0, m_nScreenHeight - ptDims.y));
	}


	// Make some total values
	int nTeamPlayers[MAX_TEAMS] = { 0, 0, 0, 0 };

	// Go through the list and add up some totals to print out
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_nClientID)
			nTeamPlayers[m_ClientData[i].m_nCharacterClass]++;
	}

	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);
	int nIndex = FindClient(nLocalID);


	// Go through the entire list of characters and set their colors
	for(i = 0; i < MAX_TEAMS; i++)
	{
		for(j = 0; j < m_nCharCount[i]; j++)
		{
			if(m_szCharList[i][j])
			{
				if((nIndex == -1) || (nTeamPlayers[i] < m_nTeamLimits[i]) || (i == m_ClientData[nIndex].m_nCharacterClass))
					m_StatChart4.SetColor(i, j + 2, g_clrSpecies[i]);
				else
					m_StatChart4.SetColor(i, j + 2, g_clrSpecies[MAX_TEAMS]);
			}
		}
	}


	// Set the background to not draw on any of the elements on rows 2 and higher
	for(i = 2; i < MAX_CHART_ROWS; i++)
		m_StatChart4.SetDrawBG(-1, i, LTFALSE);


	// Now set the background of our current character to some other highlight
	if(m_nCurrentClass < MAX_TEAMS)
	{
		i = m_nCurrentClass;

		for(j = 0; j < m_nCharCount[i]; j++)
		{
			if(m_nCharList[i][j] == m_nCurrentSet)
			{
				m_StatChart4.SetDrawBG(i, j + 2, LTTRUE);
				m_StatChart4.SetBGColor(i, j + 2, SETRGB(64, 64, 0));
				break;
			}
		}
	}


	// Get the current cursor positions
	LTIntPt chartPt = m_StatChart4.GetElementAtPt(m_ptCursor);
	LTBOOL bChanged = ((chartPt.x != m_ptChart.x) || (chartPt.y != m_ptChart.y));
	LTBOOL bValid = LTFALSE;

	m_ptChart = chartPt;


	// Now if the cursor point was over a valid element... set it's background to draw...
	if((m_ptChart.x != -1) && (m_ptChart.y > 1))
	{
		if(m_StatChart4.GetElement(m_ptChart.x, m_ptChart.y)->m_szStr[0] != 0)
		{
			if((nIndex == -1) || (nTeamPlayers[m_ptChart.x] < m_nTeamLimits[m_ptChart.x]) || (m_ptChart.x == m_ClientData[nIndex].m_nCharacterClass))
			{
				m_StatChart4.SetDrawBG(m_ptChart.x, m_ptChart.y, LTTRUE);
				m_StatChart4.SetBGColor(m_ptChart.x, m_ptChart.y, SETRGB(96, 96, 96));

				bValid = LTTRUE;

				// Play a highlighted sound if we've changed
				if(bChanged)
				{
					g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\playerselect_h.wav", SOUNDPRIORITY_MISC_MEDIUM);
				}
			}
		}
	}

	if(!bValid)
	{
		m_ptChart.x = -1;
		m_ptChart.y = -1;
	}


	// Draw the chart
	m_StatChart4.Draw(m_hScreen);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Observe_Selection()
//
// PURPOSE:		Draws the observe point selection interface
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Observe_Selection()
{
	// Setup the table to draw everything
	if(!m_StatChart5.IsInitted())
	{
		m_StatChart5.Init(3, 7, LTIntPt(0, 0), m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart5.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart5.SetRowSize(-1, m_pTextFont->Height());

		m_StatChart5.SetColumnSize(0, 100);
		m_StatChart5.SetColumnSize(1, (int)((LTFLOAT)m_nScreenWidth * 0.4f));
		m_StatChart5.SetColumnSize(2, 100);

		// Set the justification
		m_StatChart5.SetHJustify(-1, -1, CHART_JUSTIFY_CENTER);

		// Setup the header row
		sprintf(m_StatChart5.GetElement(0, 2)->m_szStr, "<<<");
		sprintf(m_StatChart5.GetElement(1, 2)->m_szStr, STRDATA(m_szStats[STAT_RANDOM]));
		sprintf(m_StatChart5.GetElement(2, 2)->m_szStr, ">>>");
		sprintf(m_StatChart5.GetElement(1, 5)->m_szStr, STRDATA(m_szStats[STAT_CLIP_INFO1]));

		m_StatChart5.SetBGColor(-1, 0, SETRGB(48, 48, 48));
		m_StatChart5.SetDrawBG(-1, 0, LTTRUE);


		// Set the upper left corner again in case it's changed
		LTIntPt ptDims = m_StatChart5.GetChartDims(3, 8);
		m_StatChart5.SetPt(LTIntPt((m_nScreenWidth - ptDims.x) / 2, m_nScreenHeight - (int)(ptDims.y * 1.2f)));
	}


	// Turn off the background on all selectable elements... they'll get turned back
	// on in the code below if it's needed.
	m_StatChart5.SetDrawBG(-1, 2, LTFALSE);
	m_StatChart5.SetDrawBG(-1, 4, LTFALSE);


	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);
	int nIndex = FindClient(nLocalID);
	LTBOOL bExtraCommand;

	sprintf(m_StatChart5.GetElement(1, 0)->m_szStr, "%s #%d", STRDATA(m_szStats[STAT_OBSERVE_POINT]), m_ClientData[nIndex].m_nObservePoint + 1);

	if(nIndex != -1)
	{
		switch(m_ClientData[nIndex].m_nObserveMode)
		{
			case MP_OBSERVE_MODE_NORMAL:
			{
				sprintf(m_StatChart5.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_RESPAWN]));
				m_StatChart5.SetBGColor(1, 4, SETRGB(0, 0, 48));
				m_StatChart5.SetDrawBG(1, 4, LTTRUE);
				bExtraCommand = LTTRUE;

				m_nWaitSeconds = 0xFF;
				break;
			}

			case MP_OBSERVE_MODE_START_GAME:
			{
				sprintf(m_StatChart5.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_START_GAME]));
				m_StatChart5.SetBGColor(1, 4, SETRGB(0, 0, 48));
				m_StatChart5.SetDrawBG(1, 4, LTTRUE);
				bExtraCommand = LTTRUE;
				break;
			}

			case MP_OBSERVE_MODE_WAIT_START_GAME:
			{
				sprintf(m_StatChart5.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_WAIT_START_GAME]));
				bExtraCommand = LTFALSE;
				break;
			}

			case MP_OBSERVE_MODE_WAIT_NEXT_ROUND:
			{
				sprintf(m_StatChart5.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_WAIT_NEXT_ROUND]));
				bExtraCommand = LTFALSE;

				m_nWaitSeconds = 0xFF;
				break;
			}

			case MP_OBSERVE_MODE_WAIT_TEAMS:
			{
				sprintf(m_StatChart5.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_WAIT_TEAMS]));
				bExtraCommand = LTFALSE;

				m_nWaitSeconds = 0xFF;
				break;
			}
		}
	}


	// Set the wait seconds based of the value
	if(m_nWaitSeconds == 0xFF)
	{
		sprintf(m_StatChart5.GetElement(1, 6)->m_szStr, " ");
	}
	else
	{
		sprintf(m_StatChart5.GetElement(1, 6)->m_szStr, "%s%d", STRDATA(m_szStats[STAT_AUTOSTART]), m_nWaitSeconds);

		// [KLS] 9/23/2001 Make sure this gets re-initialized...
		if (m_nWaitSeconds == 0)
		{
			m_nWaitSeconds = 0xFF;
		}
	}


	// Get the current cursor positions
	LTIntPt chartPt = m_StatChart5.GetElementAtPt(m_ptCursor);
	LTBOOL bChanged = ((chartPt.x != m_ptChart.x) || (chartPt.y != m_ptChart.y));
	LTBOOL bValid = LTFALSE;

	m_ptChart = chartPt;

	// Now if the cursor point was over a valid element... set it's background to draw...
	if((m_ptChart.x != -1) && ((m_ptChart.y == 2) || (bExtraCommand && (m_ptChart.y == 4))))
	{
		if(m_StatChart5.GetElement(m_ptChart.x, m_ptChart.y)->m_szStr[0] != 0)
		{
			m_StatChart5.SetBGColor(m_ptChart.x, m_ptChart.y, SETRGB(96, 96, 96));
			m_StatChart5.SetDrawBG(m_ptChart.x, m_ptChart.y, LTTRUE);

			bValid = LTTRUE;

			// Play a highlighted sound if we've changed
			if(bChanged && (m_ptChart.x != -1))
			{
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\playerselect_h.wav", SOUNDPRIORITY_MISC_MEDIUM);
			}
		}
	}

	if(!bValid)
	{
		m_ptChart.x = -1;
		m_ptChart.y = -1;
	}


	// Draw the chart
	LTIntPt ptDims = m_StatChart5.GetChartDims(3, 7);
	int nHalfWidth = (m_nScreenWidth - ptDims.x) / 2;

	g_pLTClient->ScaleSurfaceToSurface(m_hScreen, m_hBackground, &LTRect(nHalfWidth, m_nScreenHeight - (int)(ptDims.y * 1.2f), m_nScreenWidth - nHalfWidth, m_nScreenHeight), LTNULL);
	m_StatChart5.Draw(m_hScreen);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_ClipMode_Selection()
//
// PURPOSE:		Draws the clip mode selection interface
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_ClipMode_Selection()
{
	// Setup the table to draw everything
	if(!m_StatChart7.IsInitted())
	{
		m_StatChart7.Init(3, 7, LTIntPt(0, 0), m_hBackground);

		// Set the font of the entire chart to the same thing
		m_StatChart7.SetFont(-1, -1, m_pTextFont);

		// Set the size of the columns
		m_StatChart7.SetRowSize(-1, m_pTextFont->Height());

		m_StatChart7.SetColumnSize(0, 100);
		m_StatChart7.SetColumnSize(1, (int)((LTFLOAT)m_nScreenWidth * 0.4f));
		m_StatChart7.SetColumnSize(2, 100);

		// Set the justification
		m_StatChart7.SetHJustify(-1, -1, CHART_JUSTIFY_CENTER);

		// Setup the header row
	    sprintf(m_StatChart7.GetElement(1, 0)->m_szStr, STRDATA(m_szStats[STAT_CLIP_TITLE]));
		sprintf(m_StatChart7.GetElement(1, 2)->m_szStr, STRDATA(m_szStats[STAT_CLIP_INFO2]));

		m_StatChart7.SetBGColor(-1, 0, SETRGB(48, 48, 48));
		m_StatChart7.SetDrawBG(-1, 0, LTTRUE);

		// Set the upper left corner again in case it's changed
		LTIntPt ptDims = m_StatChart7.GetChartDims(3, 8);
		m_StatChart7.SetPt(LTIntPt((m_nScreenWidth - ptDims.x) / 2, m_nScreenHeight - (int)(ptDims.y * 1.2f)));
	}


	// Turn off the background on all selectable elements... they'll get turned back
	// on in the code below if it's needed.
	m_StatChart7.SetDrawBG(-1, 2, LTFALSE);
	m_StatChart7.SetDrawBG(-1, 4, LTFALSE);


	// Get this client's local ID
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);
	int nIndex = FindClient(nLocalID);


	if(nIndex != -1)
	{
		switch(m_ClientData[nIndex].m_nObserveMode)
		{
			case MP_OBSERVE_MODE_WAIT_START_GAME:
			{
				sprintf(m_StatChart7.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_WAIT_START_GAME]));
				break;
			}

			case MP_OBSERVE_MODE_WAIT_NEXT_ROUND:
			{
				sprintf(m_StatChart7.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_WAIT_NEXT_ROUND]));
				m_nWaitSeconds = 0xFF;
				break;
			}

			case MP_OBSERVE_MODE_WAIT_TEAMS:
			{
				sprintf(m_StatChart7.GetElement(1, 4)->m_szStr, STRDATA(m_szStats[STAT_WAIT_TEAMS]));
				m_nWaitSeconds = 0xFF;
				break;
			}
		}
	}


	// Set the wait seconds based of the value
	if(m_nWaitSeconds == 0xFF)
	{
		sprintf(m_StatChart7.GetElement(1, 6)->m_szStr, " ");
	}
	else
	{
		sprintf(m_StatChart7.GetElement(1, 6)->m_szStr, "%s%d", STRDATA(m_szStats[STAT_AUTOSTART]), m_nWaitSeconds);
	
		// [KLS] 9/23/2001 Make sure this gets re-initialized...
		if (m_nWaitSeconds == 0)
		{
			m_nWaitSeconds = 0xFF;
		}
	}


	// Draw the chart
	LTIntPt ptDims = m_StatChart7.GetChartDims(3, 7);
	int nHalfWidth = (m_nScreenWidth - ptDims.x) / 2;

	g_pLTClient->ScaleSurfaceToSurface(m_hScreen, m_hBackground, &LTRect(nHalfWidth, m_nScreenHeight - (int)(ptDims.y * 1.2f), m_nScreenWidth - nHalfWidth, m_nScreenHeight), LTNULL);
	m_StatChart7.Draw(m_hScreen);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::Draw_Warning_String()
//
// PURPOSE:		Draws the warning string
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::Draw_Warning_String()
{
	// Make sure the string is valid...
	if(m_szWarningString[0] && (m_fWarningStringTimer > -1.0f))
	{
		// If the screen is fading... extend the life warning string timer
		if(g_pGameClientShell->GetInterfaceMgr()->IsFadingIn() || g_pGameClientShell->GetInterfaceMgr()->IsFadingOut() || !g_pGameClientShell->IsInWorld())
		{
			m_fWarningStringTimer = g_pLTClient->GetTime();
		}


		// Setup the color...
		HLTCOLOR hColor = m_hWarningStringColor;
		LTFLOAT r = (LTFLOAT)(255 - GETR(hColor)) * m_fColorCycle;
		LTFLOAT g = (LTFLOAT)(255 - GETG(hColor)) * m_fColorCycle;
		LTFLOAT b = (LTFLOAT)(255 - GETB(hColor)) * m_fColorCycle;

		hColor = SETRGB(GETR(hColor) + (int)r, GETG(hColor) + (int)g, GETB(hColor) + (int)b);

		// How to draw the font
		LITHFONTDRAWDATA dd;
		dd.dwFlags = LTF_DRAW_SOLID;
		dd.byJustify = LTF_JUSTIFY_CENTER;
		dd.hColor = hColor;


		uint32 x, y;
		m_pTitleFont->GetTextExtents(m_szWarningString, &dd, x, y);

		int nHalfX = (int)x / 2;
		int nEdge = 15;
		int nLine = 3;

		LTIntPt pt(m_nScreenWidth / 2, (int)((LTFLOAT)m_nScreenHeight * 0.75f));

		LTIntPt ptEdgeTL(pt.x - nHalfX - nEdge, pt.y - nEdge);
		LTIntPt ptEdgeBR(pt.x + nHalfX + nEdge, pt.y + y + nEdge);
		HLTCOLOR hEdgeColor = SETRGB(0, 32, 128);


		// Draw a backdrop so the text is easier to read...
		g_pLTClient->ScaleSurfaceToSurface(m_hScreen, m_hBackground, &LTRect(ptEdgeTL.x, ptEdgeTL.y, ptEdgeBR.x, ptEdgeBR.y), LTNULL);

		g_pLTClient->ScaleSurfaceToSurfaceSolidColor(m_hScreen, m_hBackground, &LTRect(ptEdgeTL.x, ptEdgeTL.y, ptEdgeBR.x, ptEdgeTL.y + nLine), LTNULL, 0, hEdgeColor);
		g_pLTClient->ScaleSurfaceToSurfaceSolidColor(m_hScreen, m_hBackground, &LTRect(ptEdgeTL.x, ptEdgeTL.y + nLine, ptEdgeTL.x + nLine, ptEdgeBR.y - nLine), LTNULL, 0, hEdgeColor);
		g_pLTClient->ScaleSurfaceToSurfaceSolidColor(m_hScreen, m_hBackground, &LTRect(ptEdgeBR.x - nLine, ptEdgeTL.y + nLine, ptEdgeBR.x, ptEdgeBR.y - nLine), LTNULL, 0, hEdgeColor);
		g_pLTClient->ScaleSurfaceToSurfaceSolidColor(m_hScreen, m_hBackground, &LTRect(ptEdgeTL.x, ptEdgeBR.y - nLine, ptEdgeBR.x, ptEdgeBR.y), LTNULL, 0, hEdgeColor);


		// Draw the text
		m_pTitleFont->Draw(m_hScreen, m_szWarningString, &dd, pt.x, pt.y);

		if(g_pLTClient->GetTime() >= (m_fWarningStringTimer + m_fWarningStringDelay))
			m_fWarningStringTimer = -1.0f;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::UpdateDisplay()
//
// PURPOSE:		Sets up the display state and ramp values
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::UpdateDisplay(LTBOOL bOn)
{
	// Do the initial test for a change in the On setting
	if(bOn)
	{
		if(m_nDisplayState == SD_STATE_NONE)
		{
			m_nDisplayState = SD_STATE_RAMPIN;
			m_fStartTime = g_pLTClient->GetTime();
		}
	}
	else
	{
		if(m_nDisplayState == SD_STATE_DISPLAYED)
		{
			m_nDisplayState = SD_STATE_RAMPOUT;
			m_fStartTime = g_pLTClient->GetTime();
		}
	}

	// Now update the display ramp value...
	switch(m_nDisplayState)
	{
		case SD_STATE_NONE:
		{
			m_fDisplayRamp = 0.0f;
			break;
		}

		case SD_STATE_RAMPIN:
		{
			m_fDisplayRamp = (g_pLTClient->GetTime() - m_fStartTime) / g_vtScoreDisplayRampIn.GetFloat();
			m_fDisplayRamp = (m_fDisplayRamp > 1.0f) ? 1.0f : cosf(1.0f - m_fDisplayRamp);

			if(m_fDisplayRamp >= 1.0f)
			{
				m_fDisplayRamp = 1.0f;
				m_nDisplayState = SD_STATE_DISPLAYED;
			}

			break;
		}

		case SD_STATE_DISPLAYED:
		{
			m_fDisplayRamp = 1.0f;
			break;
		}

		case SD_STATE_RAMPOUT:
		{
			m_fDisplayRamp = 1.0f - ((g_pLTClient->GetTime() - m_fStartTime) / g_vtScoreDisplayRampOut.GetFloat());

			if(m_fDisplayRamp <= 0.0f)
			{
				m_fDisplayRamp = 0.0f;
				m_nDisplayState = SD_STATE_NONE;
			}

			break;
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::UpdateScreenResolution()
//
// PURPOSE:		Updates certain values about the screen resolution
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::UpdateScreenResolution()
{
	// Get the screen dimensions
	uint32 nWidth, nHeight;
	m_hScreen = g_pLTClient->GetScreenSurface();
	g_pLTClient->GetSurfaceDims(m_hScreen, &nWidth, &nHeight);

	if(m_nScreenWidth != nWidth || m_nScreenHeight != nHeight)
	{
		// Save these values
		m_nScreenWidth = nWidth;
		m_nScreenHeight = nHeight;


		// Update the header and text fonts
		if(m_nScreenWidth <= 800)
		{
			m_pTitleFont = m_pFontLarge;
			m_pHeaderFont = m_pFontMedium;
			m_pTextFont = m_pFontSmall;
		}
		else if(m_nScreenWidth <= 1024)
		{
			m_pTitleFont = m_pFontHuge;
			m_pHeaderFont = m_pFontLarge;
			m_pTextFont = m_pFontMedium;
		}
		else
		{
			m_pTitleFont = m_pFontMammoth;
			m_pHeaderFont = m_pFontHuge;
			m_pTextFont = m_pFontLarge;
		}


		// Clear the stat charts because they'll need to be reset
		ResetStatCharts();
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::UpdateColorCycle()
//
// PURPOSE:		Updates the color cycle ramp value
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::UpdateColorCycle()
{
	// Calculate the color cycle
	LTFLOAT fTime = g_pLTClient->GetTime();
	m_fColorCycle = fTime / g_vtScoreDisplayColorCycle.GetFloat();
	m_fColorCycle = fTime - ((LTFLOAT)((int)m_fColorCycle) * g_vtScoreDisplayColorCycle.GetFloat());
	m_fColorCycle /= g_vtScoreDisplayColorCycle.GetFloat();

	if(m_fColorCycle > 0.5f)
		m_fColorCycle = 1.0f - ((m_fColorCycle - 0.5f) / 0.5f);
	else
		m_fColorCycle /= 0.5f;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::UpdateLeaders()
//
// PURPOSE:		Sets the players who lead in frags or score
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::UpdateLeaders()
{
	// Determine which method to use based off the game type
	if(g_pGameClientShell->GetGameType()->IsTeamMode())
		UpdateLeaders_TeamDM();
	else
		UpdateLeaders_DM();
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::UpdateLeaders_DM()
//
// PURPOSE:		Sets the players who lead in frags or score
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::UpdateLeaders_DM()
{
	// Setup some variables to use
	int nMaxFrags = -1000000;
	int nMaxScore = -1000000;

	int nNumFragLeaders = 0;
	int nNumScoreLeaders = 0;

	int nCurFragLeader = -1;
	int nCurScoreLeader = -1;


	// If we don't have enough clients for a leader... than just clear them and return
	if(m_nClients < 2)
	{
		m_nFragLeader = -1;
		m_nScoreLeader = -1;
		return;
	}


	// Go through all the available clients and find the max frags and score
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_nClientID)
		{
			if(m_ClientData[i].m_nFrags > nMaxFrags)
			{
				nMaxFrags = m_ClientData[i].m_nFrags;
				nCurFragLeader = i;
				nNumFragLeaders = 1;
			}
			else if(m_ClientData[i].m_nFrags == nMaxFrags)
			{
				nNumFragLeaders++;
			}

			if(m_ClientData[i].m_nScore > nMaxScore)
			{
				nMaxScore = m_ClientData[i].m_nScore;
				nCurScoreLeader = i;
				nNumScoreLeaders = 1;
			}
			else if(m_ClientData[i].m_nScore == nMaxScore)
			{
				nNumScoreLeaders++;
			}
		}
	}


	// If we have one leader for frags and score...
	if((nNumFragLeaders == 1) && (nNumScoreLeaders == 1) && (nCurFragLeader == nCurScoreLeader))
	{
		if((m_nFragLeader != nCurFragLeader) || (m_nScoreLeader != nCurScoreLeader))
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LEADER_ALL, m_ClientData[nCurFragLeader].m_szName);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			m_nFragLeader = nCurFragLeader;
			m_nScoreLeader = nCurScoreLeader;
		}
	}
	else
	{
		// If we have a new leader for frags
		if((nNumFragLeaders == 1) && (m_nFragLeader != nCurFragLeader))
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LEADER_FRAGS, m_ClientData[nCurFragLeader].m_szName);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			m_nFragLeader = nCurFragLeader;
		}

		// If we have a new leader for score
		if((nNumScoreLeaders == 1) && (m_nScoreLeader != nCurScoreLeader))
		{
			LTBOOL bDrawMessage = LTTRUE;

			if(g_pGameClientShell->GetGameType()->Get() == MP_SURVIVOR)
			{
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(nMaxScore - m_ClientData[i].m_nScore == 1)
						bDrawMessage = LTFALSE;
				}
			}

			if(bDrawMessage)
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LEADER_SCORE, m_ClientData[nCurScoreLeader].m_szName);
				g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);

				m_nScoreLeader = nCurScoreLeader;
			}
		}
	}


	// If there is a tie for anything... clear the leaders
	if(nNumFragLeaders > 1)
	{
		m_nFragLeader = -1;
	}

	if(nNumScoreLeaders > 1)
	{
		m_nScoreLeader = -1;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::UpdateLeaders_TeamDM()
//
// PURPOSE:		Sets the players who lead in frags or score
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::UpdateLeaders_TeamDM()
{
	// Setup some variables to use
	int nMaxFrags = -1000000;
	int nMaxScore = -1000000;

	int nNumFragLeaders = 0;
	int nNumScoreLeaders = 0;

	int nCurFragLeader = -1;
	int nCurScoreLeader = -1;


	// Make some total values for the teams
	LTBOOL bTeamExists[MAX_TEAMS] = { LTFALSE, LTFALSE, LTFALSE, LTFALSE };
	int nTeams = 0;


	// Go through the list and check what teams exist
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_ClientData[i].m_nClientID)
		{
			if(!bTeamExists[m_ClientData[i].m_nCharacterClass])
			{
				bTeamExists[m_ClientData[i].m_nCharacterClass] = LTTRUE;
				nTeams++;
			}
		}
	}


	// If we don't have enough teams for a leader... than just clear them and return
	if(nTeams < 2)
	{
		m_nTeamFragLeader = -1;
		m_nTeamScoreLeader = -1;
		return;
	}


	// Go through all the teams and find the max frags and score
	for(i = 0; i < MAX_TEAMS; i++)
	{
		if(bTeamExists[i])
		{
			if(m_TeamData[i].m_nFrags > nMaxFrags)
			{
				nMaxFrags = m_TeamData[i].m_nFrags;
				nCurFragLeader = i;
				nNumFragLeaders = 1;
			}
			else if(m_TeamData[i].m_nFrags == nMaxFrags)
			{
				nNumFragLeaders++;
			}

			if(m_TeamData[i].m_nScore > nMaxScore)
			{
				nMaxScore = m_TeamData[i].m_nScore;
				nCurScoreLeader = i;
				nNumScoreLeaders = 1;
			}
			else if(m_TeamData[i].m_nScore == nMaxScore)
			{
				nNumScoreLeaders++;
			}
		}
	}


	// If we have one leader for frags and score...
	if((nNumFragLeaders == 1) && (nNumScoreLeaders == 1) && (nCurFragLeader == nCurScoreLeader))
	{
		if((m_nTeamFragLeader != nCurFragLeader) || (m_nTeamScoreLeader != nCurScoreLeader))
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LEADER_ALL, STRDATA(m_szTeams[nCurFragLeader]));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			m_nTeamFragLeader = nCurFragLeader;
			m_nTeamScoreLeader = nCurScoreLeader;
		}
	}
	else
	{
		// If we have a new leader for frags
		if((nNumFragLeaders == 1) && (m_nTeamFragLeader != nCurFragLeader))
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LEADER_FRAGS, STRDATA(m_szTeams[nCurFragLeader]));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			m_nTeamFragLeader = nCurFragLeader;
		}

		// If we have a new leader for score
		if((nNumScoreLeaders == 1) && (m_nTeamScoreLeader != nCurScoreLeader))
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LEADER_SCORE, STRDATA(m_szTeams[nCurScoreLeader]));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			m_nTeamScoreLeader = nCurScoreLeader;
		}
	}


	// If there is a tie for anything... clear the leaders
	if(nNumFragLeaders > 1)
	{
		m_nTeamFragLeader = -1;
	}

	if(nNumScoreLeaders > 1)
	{
		m_nTeamScoreLeader = -1;
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::HandleSubState()
//
// PURPOSE:		Sets a warning string to the current sub state description
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::HandleSubState(uint8 nSubState)
{
	LTBOOL bRoundDisplay = LTFALSE;

	switch(nSubState)
	{
		case MPMGR_SUBSTATE_SURVIVOR_TAG:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_SURVIVOR_TAG);
			SetWarningString(STRDATA(hStr), LTTRUE, 5.0f, SETRGB(32, 32, 32));
			g_pLTClient->FreeString(hStr);

			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\survivor_tag.wav", SOUNDPRIORITY_MAX);

			break;
		}

		case MPMGR_SUBSTATE_SURVIVOR_CUT:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_SURVIVOR_CUT);
			SetWarningString(STRDATA(hStr), LTTRUE, 5.0f, SETRGB(32, 32, 32));
			g_pLTClient->FreeString(hStr);

			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\survivor_cutthroat.wav", SOUNDPRIORITY_MAX);

			break;
		}

		case MPMGR_SUBSTATE_SURVIVOR_ROUND:
		case MPMGR_SUBSTATE_TM_ROUND:
		{
			bRoundDisplay = LTTRUE;
			break;
		}
	}

	ShowRoundDisplay(bRoundDisplay);
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::UpdateEvacOverlay()
//
// PURPOSE:		Update the overlay of Evac mode
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::UpdateEvacOverlay()
{
	// Make sure the overlay is facing the correct direction
	HOBJECT hPlayer = g_pGameClientShell->GetPlayerMovement()->GetObject();

	if(hPlayer)
	{
		LTRotation rRot;
		LTVector vPos, vDir, vR, vU, vF;
		LTFLOAT fDist;

		// Find out what direction we ARE facing
		g_pLTClient->GetObjectRotation(hPlayer, &rRot);
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

		// Figure out what direction we NEED to be facing
		g_pLTClient->GetObjectPos(hPlayer, &vPos);
		vDir = m_vEvacLocation - vPos;
		fDist = vDir.Mag();

		// We only want the X/Z plane... so clear out the Ys
		vDir.y = 0.0f;
		vDir.Norm();

		if(!vF.x && !vF.z)
		{
			vF = vDir;
		}
		else
		{
			vF.y = 0.0f;
			vF.Norm();
		}

		vU = LTVector(0.0f, 1.0f, 0.0f);
		vR = vU.Cross(vF);

		// Now determine a rotation given these values
		LTFLOAT fFDot, fRDot;
		fFDot = vDir.Dot(vF);
		fRDot = vDir.Dot(vR);

		if(fFDot > 1.0f) fFDot = 1.0f;
		else if(fFDot < -1.0f) fFDot = -1.0f;

		LTFLOAT fAngle = (LTFLOAT)acos(fFDot);
		if(fRDot < 0.0f) fAngle = -fAngle;

		rRot.Init();
		g_pMathLT->EulerRotateZ(rRot, fAngle);

		// Calculate the alpha we want to display it at...
		LTFLOAT fAlpha;

		if(fDist <= EVAC_OVERLAY_ALPHA_NEAR)
		{
			fAlpha = 0.1f + (m_fColorCycle * 0.9f);
		}
		else if(fDist <= EVAC_OVERLAY_ALPHA_FAR)
		{
			fAlpha = 1.0f - ((fDist - EVAC_OVERLAY_ALPHA_NEAR) / (EVAC_OVERLAY_ALPHA_FAR - EVAC_OVERLAY_ALPHA_NEAR) * 0.9f);
		}
		else
		{
			fAlpha = 0.1f;
		}

		// Set the values
		g_pGameClientShell->GetVisionModeMgr()->SetEvacRotation(rRot);
		g_pGameClientShell->GetVisionModeMgr()->SetEvacAlpha(fAlpha);
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage_GameInfo()
//
// PURPOSE:		
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage_GameInfo(uint8 nMessage, HMESSAGEREAD hRead)
{
	// Clear out all the current data...
	ClearClientData();
	ClearTeamData();


	// Read in all the current game information
	hRead->ReadStringFL(m_szServer, MAX_MP_HOST_NAME_LENGTH);
	hRead->ReadStringFL(m_szLevel, MAX_MP_LEVEL_NAME_LENGTH);

	uint8 nGameType;
	hRead->ReadByteFL(nGameType);
	g_pGameClientShell->GetGameType()->Set((int)nGameType);

	hRead->ReadByteFL(m_nFragLimit);
	hRead->ReadWordFL(m_nScoreLimit);
	hRead->ReadWordFL(m_nTimeRemaining);

	// Record the time limit for posterity
	m_nTimeLimit = m_nTimeRemaining;

	// Convert the seconds into minutes
	m_nTimeLimit /= 60;

	hRead->ReadByteFL(m_nRoundLimit);
	hRead->ReadByteFL(m_nCurrentRound);

	uint8 nSubState;
	hRead->ReadByteFL(nSubState);
	HandleSubState(nSubState);

	hRead->ReadFloatFL(m_fSpeedScale);

	uint8 nClassWeapons;
	hRead->ReadByteFL(nClassWeapons);
	m_bClassWeapons = (LTBOOL)nClassWeapons;

	hRead->ReadByteFL(m_nExosuit);

	//read in the queen molt perameter
	hRead->ReadByteFL(m_nQueenMolt);

	// Apply bandwidth throttle here...
	m_nBandwidth = hRead->ReadDWord();
	g_pProfileMgr->ApplyBandwidthThrottle();

	hRead->ReadByteFL(m_nPlayerLimit);

	for(uint8 i = 0; i < MAX_TEAMS; i++)
	{
		hRead->ReadByteFL(m_nTeamLimits[i]);
		m_TeamData[i].m_nFrags = (int16)hRead->ReadWord();
		m_TeamData[i].m_nScore = (int16)hRead->ReadWord();
	}

	uint8 nClients;
	hRead->ReadByteFL(nClients);

	// Clear the current number of clients because that'll get reset in the loop below
	m_nClients = 0;

	MPClientMgrData data;

	for(i = 0; i < nClients; i++)
	{
		hRead->ReadStringFL(data.m_szName, MAX_MP_CLIENT_NAME_LENGTH);
		hRead->ReadByteFL(data.m_nCharacterClass);
		hRead->ReadByteFL(data.m_nCharacterSet);

		hRead->ReadDWordFL(data.m_nClientID);

		hRead->ReadWordFL(data.m_nPing);
		data.m_nFrags = (int16)hRead->ReadWord();
		data.m_nConsecutiveFrags = (int8)hRead->ReadByte();
		data.m_nScore = (int16)hRead->ReadWord();
		data.m_nLives = (int8)hRead->ReadByte();

		data.m_bVerified = (LTBOOL)hRead->ReadByte();

		data.m_bObserving = (LTBOOL)hRead->ReadByte();

		// Add this guy onto the list
		AddClient(data.m_nClientID, data);
	}

	hRead->ReadDWordFL(m_nServerController);


	// Read the evac zone location if it's an Evac game
	if(nGameType == MP_EVAC)
	{
		m_vEvacLocation = hRead->ReadVector();
	}
	else
	{
		// Make sure any old overlays go away.
		g_pGameClientShell->GetVisionModeMgr()->SetEvacMode(LTFALSE);
	}

	// Now read in some more goodies for our rulez display
	*hRead >> m_nHunt_HunterRace;							// The race that the Hunters play
	*hRead >> m_nHunt_PreyRace;								// The race that the Prey play
	*hRead >> m_nHunt_Ratio;								// The ratio of hunters to prey
																		
	*hRead >> m_nSurv_SurvivorRace;							// The race that the Survivors play
	*hRead >> m_nSurv_MutateRace;							// The race that the Mutants play
																		
	*hRead >> m_nTM_DefenderRace;							// The race that the Defending team plays
	*hRead >> m_nTM_DefenderLives;							// The number of lives the Defenders get
	*hRead >> m_nTM_AttackerRace;							// The race that the Attacking team plays
	*hRead >> m_nTM_AttackerLives;							// The number of lives the Attackers get

	*hRead >> m_bLifecycle;									// Is the alien lifecycle option on?

	*hRead >> m_fPowerupRespawnScale;						// Powerup spawn scale
	*hRead >> m_fDamageScale;								// Damage scale

	*hRead >> m_bLocationDamage;							// Location damage?
	*hRead >> m_bFriendlyFire;								// Friendly fire?
	*hRead >> m_bFriendlyNames;								// Friendly fire?
	*hRead >> m_bMaskLoss;									// Can pred lose his mask?

	*hRead >> m_bCustScoring;								// Is there custom scoring?		

	// Reinit some variables
	m_fStartTime = 0.0f;
	m_nDisplayState = SD_STATE_NONE;
	m_fDisplayRamp = 0.0f;

	m_bStatsDisplay = LTFALSE;
	m_bPlayerDisplay = LTFALSE;
	m_bSummaryDisplay = LTFALSE;
	m_bLoadingDisplay = LTFALSE;
	m_bObserveDisplay = LTFALSE;
	m_bClipModeDisplay = LTFALSE;


	// Clear the stat charts
	ResetStatCharts();


	// Clear the current message queue
	g_pMessageMgr->Clear();


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage_UpdateClient()
//
// PURPOSE:		Update one or more stats of a particular client
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage_UpdateClient(uint8 nMessage, HMESSAGEREAD hRead)
{
	uint32 nClientID;
	uint8 nFlags;
	LTBOOL bUpdateLeaders = LTFALSE;

	hRead->ReadDWordFL(nClientID);
	hRead->ReadByteFL(nFlags);

	int nIndex = FindClient(nClientID);
	if(nIndex == -1) return LTFALSE;


	// -----------------------------------------------------
	// Update the basic data about the client

	if(nFlags & MP_CLIENT_UPDATE_CLIENTDATA)
	{
		hRead->ReadStringFL(m_ClientData[nIndex].m_szName, MAX_MP_CLIENT_NAME_LENGTH);
		hRead->ReadByteFL(m_ClientData[nIndex].m_nCharacterClass);
		hRead->ReadByteFL(m_ClientData[nIndex].m_nCharacterSet);
	}


	// -----------------------------------------------------
	// Update the ping display of this client

	if(nFlags & MP_CLIENT_UPDATE_PING)
	{
		hRead->ReadWordFL(m_ClientData[nIndex].m_nPing);
	}


	// -----------------------------------------------------
	// Update the frag display of this client

	if(nFlags & MP_CLIENT_UPDATE_FRAGS)
	{
		m_ClientData[nIndex].m_nFrags = (int16)hRead->ReadWord();
		bUpdateLeaders = LTTRUE;

		if(!g_pGameClientShell->GetGameType()->IsTeamMode())
		{
			// If this player has frags that are 3 or less from the max... print a warning
			if((m_nFragLimit > 0) && (m_ClientData[nIndex].m_nFrags >= (m_nFragLimit - 3)))
			{
				// This bit of code finds the index into the character class list... so we can display
				// that information next to the character's name
				int nElement = -1;

				for(int j = 0; j < m_nCharCount[m_ClientData[nIndex].m_nCharacterClass]; j++)
				{
					if(m_nCharList[m_ClientData[nIndex].m_nCharacterClass][j] == m_ClientData[nIndex].m_nCharacterSet)
					{
						nElement = j;
						break;
					}
				}


				char szTemp[64];
				sprintf(szTemp, "%s (%s) -- %s (%d/%d)",
					m_ClientData[nIndex].m_szName,
					(nElement == -1) ? STRDATA(m_szStats[STAT_UNKNOWN]) : STRDATA(m_szCharList[m_ClientData[nIndex].m_nCharacterClass][nElement]),
					STRDATA(m_szStats[STAT_FRAGS]),
					m_ClientData[nIndex].m_nFrags,
					m_nFragLimit);

				SetWarningString(szTemp, LTFALSE, 5.0f, g_clrSpecies[m_ClientData[nIndex].m_nCharacterClass]);
			}
		}
	}


	// -----------------------------------------------------
	// Update the con frag display of this client

	if(nFlags & MP_CLIENT_UPDATE_CONFRAGS)
	{
		m_ClientData[nIndex].m_nConsecutiveFrags = (int8)hRead->ReadByte();
	}


	// -----------------------------------------------------
	// Update the score display of this client

	if(nFlags & MP_CLIENT_UPDATE_SCORE)
	{
		m_ClientData[nIndex].m_nScore = (int16)hRead->ReadWord();
		bUpdateLeaders = LTTRUE;

		if(!g_pGameClientShell->GetGameType()->IsTeamMode())
		{
			// If this player has a score that is 50 pts or less from the max... print a warning
			if((m_nScoreLimit > 0) && (m_ClientData[nIndex].m_nScore >= (m_nScoreLimit - 50)))
			{
				// This bit of code finds the index into the character class list... so we can display
				// that information next to the character's name
				int nElement = -1;

				for(int j = 0; j < m_nCharCount[m_ClientData[nIndex].m_nCharacterClass]; j++)
				{
					if(m_nCharList[m_ClientData[nIndex].m_nCharacterClass][j] == m_ClientData[nIndex].m_nCharacterSet)
					{
						nElement = j;
						break;
					}
				}

				char szTemp[64];
				sprintf(szTemp, "%s (%s) -- %s (%d/%d)",
					m_ClientData[nIndex].m_szName,
					(nElement == -1) ? STRDATA(m_szStats[STAT_UNKNOWN]) : STRDATA(m_szCharList[m_ClientData[nIndex].m_nCharacterClass][nElement]),
					STRDATA(m_szStats[STAT_SCORE]),
					m_ClientData[nIndex].m_nScore,
					m_nScoreLimit);

				SetWarningString(szTemp, LTFALSE, 5.0f, g_clrSpecies[m_ClientData[nIndex].m_nCharacterClass]);
			}
		}
	}


	// -----------------------------------------------------
	// Update the con frag display of this client

	if(nFlags & MP_CLIENT_UPDATE_LIVES)
	{
		m_ClientData[nIndex].m_nLives = (int8)hRead->ReadByte();
	}


	// -----------------------------------------------------
	// Update the leader stats if needed

	if(bUpdateLeaders)
		UpdateLeaders();


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage_UpdateTeam()
//
// PURPOSE:		Update one or more stats of a particular team
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage_UpdateTeam(uint8 nMessage, HMESSAGEREAD hRead)
{
	uint8 nTeam;
	uint8 nFlags;
	LTBOOL bUpdateLeaders = LTFALSE;

	hRead->ReadByteFL(nTeam);
	hRead->ReadByteFL(nFlags);

	if(nTeam >= MAX_TEAMS) return LTFALSE;


	// -----------------------------------------------------
	// Update the frag display of this client

	if(nFlags & MP_CLIENT_UPDATE_FRAGS)
	{
		m_TeamData[nTeam].m_nFrags = (int16)hRead->ReadWord();
		bUpdateLeaders = LTTRUE;

		if(g_pGameClientShell->GetGameType()->IsTeamMode())
		{
			// If this team has frags that are 3 or less from the max... print a warning
			if((m_nFragLimit > 0) && (m_TeamData[nTeam].m_nFrags >= (m_nFragLimit - 3)))
			{
				char szTemp[64];
				sprintf(szTemp, "%s -- %s (%d/%d)",
					STRDATA(m_szTeams[nTeam]),
					STRDATA(m_szStats[STAT_FRAGS]),
					m_TeamData[nTeam].m_nFrags,
					m_nFragLimit);

				SetWarningString(szTemp, LTFALSE, 5.0f, g_clrSpecies[nTeam]);
			}
		}
	}


	// -----------------------------------------------------
	// Update the score display of this client

	if(nFlags & MP_CLIENT_UPDATE_SCORE)
	{
		m_TeamData[nTeam].m_nScore = (int16)hRead->ReadWord();
		bUpdateLeaders = LTTRUE;

		if(g_pGameClientShell->GetGameType()->IsTeamMode())
		{
			// If this team has a score that is 50 pts or less from the max... print a warning
			if((m_nScoreLimit > 0) && (m_TeamData[nTeam].m_nScore >= (m_nScoreLimit - 50)))
			{
				char szTemp[64];
				sprintf(szTemp, "%s -- %s (%d/%d)",
					STRDATA(m_szTeams[nTeam]),
					STRDATA(m_szStats[STAT_SCORE]),
					m_TeamData[nTeam].m_nScore,
					m_nScoreLimit);

				SetWarningString(szTemp, LTFALSE, 5.0f, g_clrSpecies[nTeam]);
			}
		}
	}


	// -----------------------------------------------------
	// Update the leader stats if needed

	if(bUpdateLeaders)
		UpdateLeaders();


	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage_Message()
//
// PURPOSE:		
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage_Message(uint8 nMessage, HMESSAGEREAD hRead)
{
	// Retrieve the string from the message, play the chat sound, and display the message
	char szMessage[256];
	uint32 nFrom;

	hRead->ReadDWordFL(nFrom);
	hRead->ReadStringFL(szMessage, sizeof(szMessage));

	// If the nFrom value is -1, then it's a message from the dedicated server
	if(nFrom == -1)
	{
		HLTCOLOR hColor = SETRGB(255, 255, 0);

		g_pMessageMgr->AddMessage(szMessage, hColor, STRDATA(m_szStats[STAT_DEDICATED]), hColor);
		g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\chat_recieve.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}
	else
	{
		// Find the client it was from
		int nIndex = FindClient(nFrom);

		if(nIndex != -1)
		{
			HLTCOLOR hColor;

			if(nMessage == MID_MPMGR_MESSAGE)
				hColor = SETRGB(255, 255, 255);
			else if(nMessage == MID_MPMGR_TEAM_MESSAGE)
				hColor = SETRGB(64, 64, 255);
			else if(nMessage == MID_MPMGR_PRIVATE_MESSAGE)
				hColor = SETRGB(255, 64, 64);

			HLTCOLOR hClientColor = m_ClientData[nIndex].m_bObserving ? g_clrSpecies[4] : g_clrSpecies[m_ClientData[nIndex].m_nCharacterClass];

			g_pMessageMgr->AddMessage(szMessage, hColor, m_ClientData[nIndex].m_szName, hClientColor);
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\chat_recieve.wav", SOUNDPRIORITY_MISC_MEDIUM);
		}
	}

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage_ServerMessage()
//
// PURPOSE:		
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage_ServerMessage(uint8 nMessage, HMESSAGEREAD hRead)
{
	// Get this client's local ID so we'll know if it's about him
	uint32 nLocalID;
	g_pLTClient->GetLocalClientID(&nLocalID);


	// Get the type of message we want to post
	uint8 nType;
	hRead->ReadByteFL(nType);

	switch(nType)
	{
		case MP_SERV_MSG_KILLED_HIMSELF:
		{
			uint32 nID;
			hRead->ReadDWordFL(nID);

			int nIndex = FindClient(nID);

			if(nIndex != -1)
			{
				// Pick a random message
				if(nLocalID == nID)
				{
					int nMsg = GetRandom(IDS_MP_KILLED_YOURSELF_MIN, IDS_MP_KILLED_YOURSELF_MAX);

					HSTRING hStr = g_pLTClient->FormatString(nMsg);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}
				else
				{
					int nMsg = GetRandom(IDS_MP_KILLED_HIMSELF_MIN, IDS_MP_KILLED_HIMSELF_MAX);

					HSTRING hStr = g_pLTClient->FormatString(nMsg, m_ClientData[nIndex].m_szName);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}

				// Play a sound to let people know what happened...
/*				if(g_vtMultiplayerSounds.GetFloat() &&
					(g_pLTClient->GetTime() > (m_fLastSoundTime + g_vtMultiplayerSoundDelay.GetFloat())))
				{
					char szSound[128];
					sprintf(szSound, "sounds\\multiplayer\\killedself_%d.wav", GetRandom(0, 9));
					g_pClientSoundMgr->PlaySoundLocal(szSound, SOUNDPRIORITY_MISC_MEDIUM);

					m_fLastSoundTime = g_pLTClient->GetTime();
				}
*/			}

			break;
		}

		case MP_SERV_MSG_KILLED_SOMEONE:
		case MP_SERV_MSG_KILLED_TEAMMATE:
		{
			uint32 nID1, nID2;
			hRead->ReadDWordFL(nID1);
			hRead->ReadDWordFL(nID2);

			int nIndex1 = FindClient(nID1);
			int nIndex2 = FindClient(nID2);

			if((nIndex1 != -1) && (nIndex2 != -1))
			{
				// Pick a random message
				if(nLocalID == nID1)
				{
					int nMsg = GetRandom(IDS_MP_YOU_KILLED_MIN, IDS_MP_YOU_KILLED_MAX);

					HSTRING hStr = g_pLTClient->FormatString(nMsg, m_ClientData[nIndex2].m_szName);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}
				else if(nLocalID == nID2)
				{
					int nMsg = GetRandom(IDS_MP_KILLED_BY_MIN, IDS_MP_KILLED_BY_MAX);

					HSTRING hStr = g_pLTClient->FormatString(nMsg, m_ClientData[nIndex1].m_szName);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}
				else
				{
					int nMsg = GetRandom(IDS_MP_KILLED_MIN, IDS_MP_KILLED_MAX);

					HSTRING hStr = g_pLTClient->FormatString(nMsg, m_ClientData[nIndex1].m_szName, m_ClientData[nIndex2].m_szName);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}

				// Play a sound to let people know what happened...
/*				if(g_vtMultiplayerSounds.GetFloat() &&
					(g_pLTClient->GetTime() > (m_fLastSoundTime + g_vtMultiplayerSoundDelay.GetFloat())))
				{
					char szSound[128];
					sprintf(szSound, "sounds\\multiplayer\\killed_%d.wav", GetRandom(0, 9));
					g_pClientSoundMgr->PlaySoundLocal(szSound, SOUNDPRIORITY_MISC_MEDIUM);

					m_fLastSoundTime = g_pLTClient->GetTime();
				}
*/			}

			break;
		}

		case MP_PICKUP_MSG_CANT_USE_WEAPON:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_CANT_USE_WEAPON);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_PICKUP_MSG_CANT_USE_AMMO:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_CANT_USE_AMMO);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SERVER_CONTROLLER_PW_INVAL:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_SC_PW);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SERVER_CONTROLLER_REDUND:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_SC_REDUND);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SC_ALIEN_LIFECYCLE:
		{
			LTBOOL bOn = (LTBOOL)hRead->ReadByte();

			// Set some default strings
			HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
			HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LIFECYCLE, bOn?g_pLTClient->GetStringData(hOn):g_pLTClient->GetStringData(hOff));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);
			g_pLTClient->FreeString(hOn);
			g_pLTClient->FreeString(hOff);

			break;
		}

		case MP_SC_PRED_MASK:
		{
			LTBOOL bOn = (LTBOOL)hRead->ReadByte();

			// Set some default strings
			HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
			HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_PREDMASK, bOn?g_pLTClient->GetStringData(hOn):g_pLTClient->GetStringData(hOff));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);
			g_pLTClient->FreeString(hOn);
			g_pLTClient->FreeString(hOff);

			break;
		}

		case MP_SC_FF:
		{
			LTBOOL bOn = (LTBOOL)hRead->ReadByte();

			// Set some default strings
			HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
			HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_FRIENDLY_FIRE, bOn?g_pLTClient->GetStringData(hOn):g_pLTClient->GetStringData(hOff));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);
			g_pLTClient->FreeString(hOn);
			g_pLTClient->FreeString(hOff);

			break;
		}

		case MP_SC_FN:
		{
			LTBOOL bOn = (LTBOOL)hRead->ReadByte();

			// Set some default strings
			HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
			HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_FRIENDLY_NAMES, bOn?g_pLTClient->GetStringData(hOn):g_pLTClient->GetStringData(hOff));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);
			g_pLTClient->FreeString(hOn);
			g_pLTClient->FreeString(hOff);

			break;
		}

		case MP_SC_LOC_DAM:
		{
			LTBOOL bOn = (LTBOOL)hRead->ReadByte();

			// Set some default strings
			HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
			HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LOC_DAMAGE, bOn?g_pLTClient->GetStringData(hOn):g_pLTClient->GetStringData(hOff));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);
			g_pLTClient->FreeString(hOn);
			g_pLTClient->FreeString(hOff);

			break;
		}

		case MP_SC_CLASS_WEAPONS:
		{
			LTBOOL bOn = (LTBOOL)hRead->ReadByte();

			// Set some default strings
			HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
			HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_CLASS_WEAPONS, bOn?g_pLTClient->GetStringData(hOn):g_pLTClient->GetStringData(hOff));
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);
			g_pLTClient->FreeString(hOn);
			g_pLTClient->FreeString(hOff);

			break;
		}

		case MP_SC_QUEEN:
		{
			uint8 nValue = hRead->ReadByte();

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_QUEEN, nValue);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SC_EXOSUIT:
		{
			uint8 nValue = hRead->ReadByte();

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_EXOSUIT, nValue);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));

			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SC_INVALID:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_INVALID);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SC_BOUNDS:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_BOUNDS);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SERVER_CONTROLLER_NOT_DEDICATED:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_SC_NONDED);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SERVER_QUEEN_SP_ERROR:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_QUEEN_ERROR);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_SERVER_CONTROLLER:
		{
			uint32 nID;
			uint8 bNewController;

			hRead->ReadDWordFL(nID);
			hRead->ReadByteFL(bNewController);

			int nIndex = FindClient(nID);

			if(nIndex != -1)
			{
				int nMsg = bNewController ? IDS_MP_SERVER_CONTROL_CHANGE : IDS_MP_SERVER_CONTROL;

				// Supress this message in SP
				if(!g_pGameClientShell->GetGameType()->IsSinglePlayer())
				{
					HSTRING hStr = g_pLTClient->FormatString(nMsg, m_ClientData[nIndex].m_szName);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}

				if(bNewController)
					m_nServerController = nID;
			}
			else
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_MP_NOSC);
				g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);
			}

			break;
		}

		case MP_SERVER_COMMAND_INVALID:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_SERVER_COMMAND_INVALID);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			break;
		}

		case MP_CLIENT_RENAMED:
		{
			char szBuffer[MAX_MP_CLIENT_NAME_LENGTH];
			hRead->ReadStringFL(szBuffer, MAX_MP_CLIENT_NAME_LENGTH);

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_RENAMED, szBuffer);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\servermsg_alert.wav", SOUNDPRIORITY_MISC_MEDIUM);

			break;
		}

		case MP_AUTO_TEAM_SWITCH:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_AUTO_TEAM_SWITCH);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\servermsg_alert.wav", SOUNDPRIORITY_MISC_MEDIUM);

			break;
		}

		case MP_TEAM_NOT_AVAILABLE:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_TEAM_NOT_AVAILABLE);
			g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
			g_pLTClient->FreeString(hStr);

			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\servermsg_alert.wav", SOUNDPRIORITY_MISC_MEDIUM);

			break;
		}

		case MP_SUBSTATE_CHANGED:
		{
			uint8 nSubState;
			hRead->ReadByteFL(nSubState);
			HandleSubState(nSubState);

			break;
		}

		case MP_CLIENT_CHEATING:
		{
			uint32 nID;
			hRead->ReadDWordFL(nID);

			int nIndex = FindClient(nID);

			if(nIndex != -1)
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_MP_CHEATING, m_ClientData[nIndex].m_szName);
				g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);

				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\servermsg_alert.wav", SOUNDPRIORITY_MISC_MEDIUM);
			}

			break;
		}

		case MP_NAME_CHANGE:
		{
			uint32 nID;
			hRead->ReadDWordFL(nID);

			char szBuffer[MAX_MP_CLIENT_NAME_LENGTH];
			hRead->ReadStringFL(szBuffer, MAX_MP_CLIENT_NAME_LENGTH);

			int nIndex = FindClient(nID);

			if(nIndex != -1)
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_MP_NAME_CHANGE, m_ClientData[nIndex].m_szName, szBuffer);
				g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);

				strncpy(m_ClientData[nIndex].m_szName, szBuffer, MAX_MP_CLIENT_NAME_LENGTH);

				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\servermsg_alert.wav", SOUNDPRIORITY_MISC_MEDIUM);
			}

			break;
		}

		case MP_HUNT_MUTATE_SWITCH:
		{
			uint32 nID1, nID2;
			hRead->ReadDWordFL(nID1);
			hRead->ReadDWordFL(nID2);

			int nIndex1 = FindClient(nID1);
			int nIndex2 = FindClient(nID2);

			if((nIndex1 != -1) && (nIndex2 != -1))
			{
				if(nLocalID == nID1)
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_MUTATE_HUNTER);
					SetWarningString(STRDATA(hStr), LTTRUE, 3.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
				else if(nLocalID == nID2)
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_MUTATE_PREY);
					SetWarningString(STRDATA(hStr), LTTRUE, 3.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
				else
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_MUTATE_SWITCH, m_ClientData[nIndex1].m_szName, m_ClientData[nIndex2].m_szName);
					SetWarningString(STRDATA(hStr), LTTRUE, 6.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
			}

			break;
		}

		case MP_HUNT_MUTATE_HUNTER:
		case MP_HUNT_MUTATE_PREY:
		{
			uint32 nID1;
			hRead->ReadDWordFL(nID1);

			int nIndex1 = FindClient(nID1);

			if(nIndex1 != -1)
			{
				int nRes = (nType == MP_HUNT_MUTATE_HUNTER) ? IDS_MP_MUTATE_HUNTER_SELECT : IDS_MP_MUTATE_PREY_SELECT;

				HSTRING hStr = g_pLTClient->FormatString(nRes, m_ClientData[nIndex1].m_szName);
				SetWarningString(STRDATA(hStr), LTTRUE, 3.0f, SETRGB(32, 32, 32));
				g_pLTClient->FreeString(hStr);
			}

			break;
		}

		case MP_SURVIVOR_MUTATE:
		{
			uint32 nID1;
			hRead->ReadDWordFL(nID1);

			int nIndex1 = FindClient(nID1);

			if(nIndex1 != -1)
			{
				if(nLocalID == nID1)
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_MUTATING);
					SetWarningString(STRDATA(hStr), LTTRUE, 3.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
				else
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_IS_MUTATING, m_ClientData[nIndex1].m_szName);
					SetWarningString(STRDATA(hStr), LTTRUE, 5.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
			}

			break;
		}

		case MP_QUEEN_MUTATE:
		{
			uint32 nID1;
			hRead->ReadDWordFL(nID1);

			int nIndex1 = FindClient(nID1);

			if(nIndex1 != -1)
			{
				if(nLocalID == nID1)
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_QUEEN_MUTATING);
					SetWarningString(STRDATA(hStr), LTTRUE, 3.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
				else
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_IS_QUEEN_MUTATING, m_ClientData[nIndex1].m_szName);
					SetWarningString(STRDATA(hStr), LTTRUE, 3.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
			}

			break;
		}

		case MP_QUEEN_DEATH:
		{
			uint32 nID1;
			hRead->ReadDWordFL(nID1);

			int nIndex1 = FindClient(nID1);

			if(nIndex1 != -1)
			{
				// tell everyone about the death of the queen...
				// no need to tell the dead queen, she already knows...
				if(nLocalID != nID1)
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_QUEEN_DEATH, m_ClientData[nIndex1].m_szName);
					SetWarningString(STRDATA(hStr), LTTRUE, 3.0f, SETRGB(32, 32, 32));
					g_pLTClient->FreeString(hStr);
				}
			}

			break;
		}

		case MP_SURVIVOR_INTERM_TIME:
		{
			int8 nTime = (int8)hRead->ReadByte();

			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_SURVIVOR_TAG_ENDING, nTime);
			SetWarningString(STRDATA(hStr), LTTRUE, 2.0f, SETRGB(32, 32, 32));
			g_pLTClient->FreeString(hStr);

			LTFLOAT fPitch = 0.05f + ((MPMGR_SURVIVOR_INTERM_TIME - (LTFLOAT)nTime) / 5.0f);
			g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\survivor_count.wav", SOUNDPRIORITY_MAX, PLAYSOUND_CTRL_PITCH, SMGR_DEFAULT_VOLUME, fPitch);

			break;
		}

		case MP_EVAC_COUNTDOWN:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_EVAC_COUNTDOWN);
			SetWarningString(STRDATA(hStr), LTTRUE, 5.0f, SETRGB(32, 32, 32));
			g_pLTClient->FreeString(hStr);

			if(m_hEvacSound)
			{
				g_pLTClient->KillSound(m_hEvacSound);
				m_hEvacSound = LTNULL;
			}

			m_hEvacSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\evac_count.wav", SOUNDPRIORITY_MAX, PLAYSOUND_GETHANDLE, SMGR_DEFAULT_VOLUME);
			break;
		}

		case MP_EVAC_COMPLETE:
		{
			if(m_hEvacSound)
			{
				g_pLTClient->KillSound(m_hEvacSound);
				m_hEvacSound = LTNULL;
			}

			m_hEvacSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\evac_complete.wav", SOUNDPRIORITY_MAX, PLAYSOUND_GETHANDLE, SMGR_DEFAULT_VOLUME);
			break;
		}

		case MP_EVAC_CANCEL:
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_MP_EVAC_CANCEL);
			SetWarningString(STRDATA(hStr), LTTRUE, 5.0f, SETRGB(32, 32, 32));
			g_pLTClient->FreeString(hStr);

			if(m_hEvacSound)
			{
				g_pLTClient->KillSound(m_hEvacSound);
				m_hEvacSound = LTNULL;
			}

			m_hEvacSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\evac_cancel.wav", SOUNDPRIORITY_MAX, PLAYSOUND_GETHANDLE, SMGR_DEFAULT_VOLUME);
			break;
		}

		case MP_EVAC_STATE:
		{
			m_bDrawEvacLocation = (LTBOOL)hRead->ReadByte();
			break;
		}
	}

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage_NextLevel()
//
// PURPOSE:		
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage_NextLevel(uint8 nMessage, HMESSAGEREAD hRead)
{
	uint8 nNextLevelState = (LTBOOL)hRead->ReadByte();

	switch(nNextLevelState)
	{
		case MPMGR_NEXT_LEVEL_RESET:
		{
			// Reset the verifications
			LTBOOL bNeededClearing = ClearVerification();

			// Set the next level name and make sure we're in summary display mode
			hRead->ReadStringFL(m_szNextLevel, MAX_MP_LEVEL_NAME_LENGTH);
			ShowSummaryDisplay(LTTRUE);

			if(bNeededClearing)
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\verify_clear.wav", SOUNDPRIORITY_MISC_MEDIUM);
			else
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\verify.wav", SOUNDPRIORITY_MISC_MEDIUM);

			break;
		}

		case MPMGR_NEXT_LEVEL_IN_PROGRESS:
		{
			// Set the next level name and make sure we're in summary display mode
			hRead->ReadStringFL(m_szNextLevel, MAX_MP_LEVEL_NAME_LENGTH);
			ShowSummaryDisplay(LTTRUE);

			break;
		}

		case MPMGR_NEXT_LEVEL_ACCEPTED:
		{
			uint32 nID;
			hRead->ReadDWordFL(nID);

			// Update the display to show this character has verified
			int nIndex = FindClient(nID);

			if(nIndex != -1)
			{
				g_pClientSoundMgr->PlaySoundLocal("Sounds\\Interface\\verify.wav", SOUNDPRIORITY_MISC_MEDIUM);
				m_ClientData[nIndex].m_bVerified = LTTRUE;
			}

			break;
		}
	}

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::OnMessage_ChangePlayer()
//
// PURPOSE:		
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::OnMessage_ChangePlayer(uint8 nMessage, HMESSAGEREAD hRead)
{
	// Retrieve the string from the message, play the chat sound, and display the message
	uint32 nFrom;
	uint8 nClass, nSet, bDisplay;

	hRead->ReadDWordFL(nFrom);
	hRead->ReadByteFL(nClass);
	hRead->ReadByteFL(nSet);
	hRead->ReadByteFL(bDisplay);

	// Find the client it was from
	int nIndex = FindClient(nFrom);

	if(nIndex != -1)
	{
		LTBOOL bChangedTeams = (m_ClientData[nIndex].m_nCharacterClass != nClass);

		m_ClientData[nIndex].m_nCharacterClass = nClass;
		m_ClientData[nIndex].m_nCharacterSet = nSet;

		if(bDisplay)
		{
			if(bChangedTeams)
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_MP_CHANGE_TEAM, m_ClientData[nIndex].m_szName, STRDATA(m_szTeams[nClass]));
				g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);
			}
			else
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_MP_CHANGE_PLAYER, m_ClientData[nIndex].m_szName);
				g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);
			}
		}

		// Reset the player info chart so it will get refreshed
		ResetStatCharts();
	}

	return LTTRUE;
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::BuildClassLists()
//
// PURPOSE:		
//
// -------------------------------------------------------------------- //

void MultiplayerClientMgr::BuildClassLists()
{
	int i, j;

	// Reinitialize the lists
	for(i = 0; i < MAX_TEAMS; i++)
	{
		for(j = 0; j < 32; j++)
		{
			if(m_szCharList[i][j])
				g_pLTClient->FreeString(m_szCharList[i][j]);

			m_szCharList[i][j] = LTNULL;
			m_nCharList[i][j] = -1;
		}

		m_nCharCount[i] = 0;
	}


	// How many sets do we have to go through for this?
	int nSets = g_pCharacterButeMgr->GetNumSets();

	CharacterClass eClass;
	Species eSpec;
	LTBOOL bValid;


	// Setup through them one by one
	for(i = 0; i < nSets; i++)
	{
		CString str = g_pCharacterButeMgr->GetMultiplayerModel(i);

		// This could be a potential multiplayer character if the model is a valid *.abc string...
		if(!str.IsEmpty() && str.Find(".abc") >= 0)
		{
			eClass = g_pCharacterButeMgr->GetClass(i);
			bValid = LTTRUE;

			switch(eClass)
			{
				case ALIEN:			eSpec = Alien;				break;
				case MARINE:		eSpec = Marine;				break;
				case PREDATOR:		eSpec = Predator;			break;
				case CORPORATE:		eSpec = Corporate;			break;
				default:			bValid = LTFALSE;			break;
			}

			if(bValid)
			{
				m_szCharList[eSpec][m_nCharCount[eSpec]] = g_pLTClient->FormatString(g_pCharacterButeMgr->GetDisplayName(i));
				m_nCharList[eSpec][m_nCharCount[eSpec]] = i;
				m_nCharCount[eSpec]++;
			}
		}
	}
}


// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::IsFriendly()
//
// PURPOSE:		
//
// -------------------------------------------------------------------- //

LTBOOL MultiplayerClientMgr::IsFriendly(uint32 nClientID)
{
	// See if this client is on the list
	int nIndex = FindClient(nClientID);

	// If this client was not found... return false
	if(nIndex == -1)
	{
		return LTFALSE;
	}

	switch(g_pGameClientShell->GetGameType()->Get())
	{
		case MP_DM:			return LTFALSE;
		case MP_TEAMDM:
		case MP_HUNT:
		case MP_SURVIVOR:
		case MP_OVERRUN:
		case MP_EVAC:	
			{
				// Get this client's local ID
				uint32 nLocalID;
				g_pLTClient->GetLocalClientID(&nLocalID);

				int nLocalIndex = FindClient(nLocalID);

				if(nLocalIndex != -1)
					return m_ClientData[nIndex].m_nCharacterClass == m_ClientData[nLocalIndex].m_nCharacterClass; 
			}
	}

	return LTFALSE;
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	MultiplayerClientMgr::GetCharacterClass()
//
// PURPOSE:		Find a clients character class
//
// -------------------------------------------------------------------- //

uint8 MultiplayerClientMgr::GetCharacterClass(uint32 nClientID)
{
	// See if this client is on the list
	int nIndex = FindClient(nClientID);

	// If this client was not found... 
	if(nIndex == -1) return 0;

	return m_ClientData[nIndex].m_nCharacterClass;
}

