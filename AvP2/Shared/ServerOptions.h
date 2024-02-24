// ServerOptions.h: interface for the CServerOptionMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SERVEROPTIONS_H
#define SERVEROPTIONS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MultiplayerMgrDefs.h"
#include <hash_map>
#include "Globals.h"
#include "ClientServerShared.h"

class CButeMgr;
class StartGameRequest;

uint32	GetValidLANPort(uint32 nPortID);

class CServerOptions
{
	public:

	enum
	{
		eDefaultScoringValue = 25
	};

	public:

		CServerOptions();

		void		SaveToBute(CButeMgr& buteMgr);
		void		LoadFromBute(CButeMgr& buteMgr);

		void		FillInHostInfo(StartGameRequest &request, MPGameInfo &gi);
		LTBOOL		MapListEmpty();

		uint8		GetScoring( uint32 nScoringClass );
		void		SetScoring( uint32 nScoringClass, uint8 nScoring );

		void		CleanServerName();

		char		szCfgName[MAX_MP_CFG_NAME_LENGTH];

		// -------------------------------------------------------------------------
		// Basic game information
		char		szServerName[MAX_MP_HOST_NAME_LENGTH];
		char		szPassword[MAX_MP_PASSWORD_LENGTH];
		char		szSCPassword[MAX_MP_PASSWORD_LENGTH];
		uint8		nGameType;
		uint32		nPortID;
		uint32		nBandwidth;
		LTBOOL		bLocked;
		LTBOOL		bDedicated;
		LTBOOL		bLANConnection;

		// -------------------------------------------------------------------------
		// Advanced options
		LTFLOAT		fSpeed;
		LTFLOAT		fRespawn;
		LTFLOAT		fDamage;

		LTBOOL		bLocationDamage;
		LTBOOL		bFriendlyFire;
		LTBOOL		bFriendlyNames;
		LTBOOL		bMaskLoss;
		LTBOOL		bClassWeapons;
		uint8		nExosuit;
		uint8		nQueenMolt;

		// Defines a hash map of class id's to scorings.
		typedef std::hash_map< uint32, uint8 > ScoringClassHashMap;
		ScoringClassHashMap m_hashScoring;

		// -------------------------------------------------------------------------
		// Game specific settings below this point...

		// Deathmatch
		uint8		nDM_MaxTotalPlayers;
		uint8		nDM_MaxPlayers[MAX_TEAMS];
		uint8		nDM_FragLimit;
		uint16		nDM_ScoreLimit;
		uint8		nDM_TimeLimit;
		uint8		nDM_Lifecycle;
		StringList	sDM_Maps;

		// Team Deathmatch
		uint8		nTeam_MaxTotalPlayers;
		uint8		nTeam_MaxPlayers[MAX_TEAMS];
		uint8		nTeam_FragLimit;
		uint16		nTeam_ScoreLimit;
		uint8		nTeam_TimeLimit;
		uint8		nTeam_Lifecycle;
		StringList	sTeam_Maps;

		// Hunt
		uint8		nHunt_MaxTotalPlayers;
		uint8		nHunt_FragLimit;
		uint8		nHunt_TimeLimit;
		uint8		nHunt_HunterRace;
		uint8		nHunt_PreyRace;
		uint8		nHunt_Ratio;
		StringList	sHunt_Maps;

		// Survivor
		uint8		nSurv_MaxTotalPlayers;
		uint16		nSurv_ScoreLimit;
		uint8		nSurv_RoundLimit;
		uint8		nSurv_TimeLimit;
		uint8		nSurv_SurvivorRace;
		uint8		nSurv_MutateRace;
		StringList	sSurv_Maps;

		// Overrun
		uint8		nOver_MaxTotalPlayers;
		uint8		nOver_RoundLimit;
		uint8		nOver_TimeLimit;
		uint8		nOver_DefenderRace;
		uint8		nOver_DefenderLives;
		uint8		nOver_AttackerRace;
		uint8		nOver_AttackerLives;
		StringList	sOver_Maps;

		// Evac
		uint8		nEvac_MaxTotalPlayers;
		uint8		nEvac_RoundLimit;
		uint8		nEvac_TimeLimit;
		uint8		nEvac_DefenderRace;
		uint8		nEvac_DefenderLives;
		uint8		nEvac_AttackerRace;
		uint8		nEvac_AttackerLives;
		StringList	sEvac_Maps;
};




#endif	// SERVEROPTIONS_H
