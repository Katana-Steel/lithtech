// ----------------------------------------------------------------------------------------------
// 
// ServerOptionsMgr.cpp
// 
// ----------------------------------------------------------------------------------------------


#include "stdafx.h"
#include "butemgr.h"
#include "ServerOptions.h"
#include "GameType.h"

#include <iostream>			// For input and output
#include <fstream>			// For the files

#ifdef _WIN32
	#include <io.h>				// Find first, find next, etc.
#endif

namespace
{
	const uint32 kMinLANPort = 27800;
	const uint32 kMaxLANPort = 27900;
	const uint32 kPortMod = 8;
}

// ----------------------------------------------------------------------------------------------

CServerOptions::CServerOptions()
{
	int i;
	SAFE_STRCPY(szCfgName, ".Default");


	// Basic game information
	SAFE_STRCPY(szServerName, "Aliens vs. Predator 2");
	SAFE_STRCPY(szPassword, "AVP2");
	SAFE_STRCPY(szSCPassword, "AVP2");

	nGameType			= MP_DM;
	nPortID				= DEFAULT_PORT;
	nBandwidth			= DEFAULT_BANDWIDTH;
	bLocked				= LTFALSE;
	bDedicated			= LTFALSE;
	bLANConnection		= LTFALSE;


	// Advanced options
	fSpeed				= 1.0f;
	fRespawn			= 1.0f;
	fDamage				= 1.0f;

	bLocationDamage		= LTTRUE;
	bFriendlyFire		= LTTRUE;
	bFriendlyNames		= LTFALSE;
	bMaskLoss			= LTTRUE;
	bClassWeapons		= LTFALSE;
	nExosuit			= 0;
	nQueenMolt			= 0;


	// Deathmatch
	nDM_MaxTotalPlayers = MAX_CLIENTS / 2;

	for(i = 0; i < MAX_TEAMS; i++)
		nDM_MaxPlayers[i] = MAX_CLIENTS / 4;

	nDM_FragLimit		= 20;
	nDM_ScoreLimit		= 500;
	nDM_TimeLimit		= 15;
	nDM_Lifecycle		= 0;


	// Team Deathmatch
	nTeam_MaxTotalPlayers = MAX_CLIENTS / 2;

	for(i = 0; i < MAX_TEAMS; i++)
		nTeam_MaxPlayers[i] = MAX_CLIENTS / 4;

	nTeam_FragLimit		= 20;
	nTeam_ScoreLimit	= 500;
	nTeam_TimeLimit		= 15;
	nTeam_Lifecycle		= 0;


	// Hunt
	nHunt_MaxTotalPlayers = MAX_CLIENTS / 2;
	nHunt_FragLimit		= 20;
	nHunt_TimeLimit		= 15;
	nHunt_HunterRace	= Predator;
	nHunt_PreyRace		= Marine;
	nHunt_Ratio			= 2;


	// Survivor
	nSurv_MaxTotalPlayers = MAX_CLIENTS / 2;
	nSurv_ScoreLimit	= 1500;
	nSurv_RoundLimit	= 5;
	nSurv_TimeLimit		= 5;
	nSurv_SurvivorRace	= Marine;
	nSurv_MutateRace	= Alien;


	// Overrun
	nOver_MaxTotalPlayers = MAX_CLIENTS / 2;
	nOver_RoundLimit = 5;
	nOver_TimeLimit = 5;
	nOver_DefenderRace = Corporate;
	nOver_DefenderLives = 1;
	nOver_AttackerRace = Alien;
	nOver_AttackerLives = 2;


	// Evac
	nEvac_MaxTotalPlayers = MAX_CLIENTS / 2;
	nEvac_RoundLimit = 10;
	nEvac_TimeLimit = 2;
	nEvac_DefenderRace = Marine;
	nEvac_DefenderLives = 1;
	nEvac_AttackerRace = Alien;
	nEvac_AttackerLives = 3;
}


// ----------------------------------------------------------------------------------------------

void CServerOptions::SaveToBute(CButeMgr& buteMgr)
{
	if(!_stricmp(szCfgName, ".Default"))
		return;

	StringList::iterator iter;
	char szBuffer[32];
	int i;


	CleanServerName();

	// Basic game information
	buteMgr.SetString("Settings","ServerName",szServerName);
	buteMgr.SetString("Settings","Password",szPassword);
	buteMgr.SetString("Settings","SCPassword",szSCPassword);
	buteMgr.SetInt("Settings","Locked",bLocked);
	buteMgr.SetInt("Settings","Dedicated",bDedicated);
	buteMgr.SetInt("Settings","LANConnection",bLANConnection);
	buteMgr.SetInt("Settings","GameType",nGameType);
	buteMgr.SetInt("Settings","PortID",nPortID);
	buteMgr.SetInt("Settings","Bandwidth",nBandwidth);


	// Advanced options
	buteMgr.SetFloat("Preferences","Speed",fSpeed);
	buteMgr.SetFloat("Preferences","RespawnSpeed",fRespawn);
	buteMgr.SetFloat("Preferences","DamageScale",fDamage);

	buteMgr.SetInt("Preferences","LocationDamage",bLocationDamage);
	buteMgr.SetInt("Preferences","FriendlyFire",bFriendlyFire);
	buteMgr.SetInt("Preferences","FriendlyNames",bFriendlyNames);
	buteMgr.SetInt("Preferences","MaskLoss",bMaskLoss);
	buteMgr.SetInt("Preferences","ClassWeapons",bClassWeapons);
	buteMgr.SetInt("Preferences","Exosuit",nExosuit);
	buteMgr.SetInt("Preferences","QueenMolt",nQueenMolt);

	// Iterate over the scoring classes and save them out.
	ScoringClassHashMap::iterator iterScoring = m_hashScoring.begin( );
	i = 0;
	while( iterScoring != m_hashScoring.end( ))
	{
		// Create the attribute string for the scoring class.
		sprintf( szBuffer, "Class%d", i );

		// Set the scoring class attribute.
		buteMgr.SetInt( "Scoring", szBuffer, iterScoring->first );

		// Create the attribute string for the score.
		sprintf( szBuffer, "Score%d", i );

		// Set the score.
		buteMgr.SetInt( "Scoring", szBuffer, iterScoring->second );

		// Advance our counter.
		i++;
		iterScoring++;
	}


	// Deathmatch settings
	buteMgr.SetInt("Deathmatch","MaxTotalPlayers",nDM_MaxTotalPlayers);

	for(i = 0; i < MAX_TEAMS; i++)
	{
		sprintf(szBuffer,"MaxPlayers%01d",i);
		buteMgr.SetInt("Deathmatch",szBuffer,nDM_MaxPlayers[i]);
	}

	buteMgr.SetInt("Deathmatch","FragLimit",nDM_FragLimit);
	buteMgr.SetInt("Deathmatch","ScoreLimit",nDM_ScoreLimit);
	buteMgr.SetInt("Deathmatch","TimeLimit",nDM_TimeLimit);
	buteMgr.SetInt("Deathmatch","Lifecycle",nDM_Lifecycle);

	iter = sDM_Maps.begin();
	i = 0;

	while(iter != sDM_Maps.end() && i < MAX_MP_LEVELS)
	{
		std::string str = *iter;

		sprintf(szBuffer,"Map%02d",i);
		buteMgr.SetString("Deathmatch",szBuffer,str.c_str());
		iter++;
		i++;
	}


	// Team Deathmatch settings
	buteMgr.SetInt("TeamDeathmatch","MaxTotalPlayers",nTeam_MaxTotalPlayers);

	for(i = 0; i < MAX_TEAMS; i++)
	{
		sprintf(szBuffer,"MaxPlayers%01d",i);
		buteMgr.SetInt("TeamDeathmatch",szBuffer,nTeam_MaxPlayers[i]);
	}

	buteMgr.SetInt("TeamDeathmatch","FragLimit",nTeam_FragLimit);
	buteMgr.SetInt("TeamDeathmatch","ScoreLimit",nTeam_ScoreLimit);
	buteMgr.SetInt("TeamDeathmatch","TimeLimit",nTeam_TimeLimit);
	buteMgr.SetInt("TeamDeathmatch","Lifecycle",nTeam_Lifecycle);

	iter = sTeam_Maps.begin();
	i = 0;

	while(iter != sTeam_Maps.end() && i < MAX_MP_LEVELS)
	{
		std::string str = *iter;

		sprintf(szBuffer,"Map%02d",i);
		buteMgr.SetString("TeamDeathmatch",szBuffer,str.c_str());
		iter++;
		i++;
	}


	// Hunt settings
	buteMgr.SetInt("Hunt","MaxTotalPlayers",nHunt_MaxTotalPlayers);
	buteMgr.SetInt("Hunt","FragLimit",nHunt_FragLimit);
	buteMgr.SetInt("Hunt","TimeLimit",nHunt_TimeLimit);
	buteMgr.SetInt("Hunt","HunterRace",nHunt_HunterRace);
	buteMgr.SetInt("Hunt","PreyRace",nHunt_PreyRace);
	buteMgr.SetInt("Hunt","HunterRatio",nHunt_Ratio);

	iter = sHunt_Maps.begin();
	i = 0;

	while(iter != sHunt_Maps.end() && i < MAX_MP_LEVELS)
	{
		std::string str = *iter;

		sprintf(szBuffer,"Map%02d",i);
		buteMgr.SetString("Hunt",szBuffer,str.c_str());
		iter++;
		i++;
	}


	// Survivor settings
	buteMgr.SetInt("Survivor","MaxTotalPlayers",nSurv_MaxTotalPlayers);
	buteMgr.SetInt("Survivor","ScoreLimit",nSurv_ScoreLimit);
	buteMgr.SetInt("Survivor","RoundLimit",nSurv_RoundLimit);
	buteMgr.SetInt("Survivor","TimeLimit",nSurv_TimeLimit);
	buteMgr.SetInt("Survivor","SurvivorRace",nSurv_SurvivorRace);
	buteMgr.SetInt("Survivor","MutateRace",nSurv_MutateRace);

	iter = sSurv_Maps.begin();
	i = 0;

	while(iter != sSurv_Maps.end() && i < MAX_MP_LEVELS)
	{
		std::string str = *iter;

		sprintf(szBuffer,"Map%02d",i);
		buteMgr.SetString("Survivor",szBuffer,str.c_str());
		iter++;
		i++;
	}


	// Overrun Survivor settings
	buteMgr.SetInt("Overrun","MaxTotalPlayers",nOver_MaxTotalPlayers);
	buteMgr.SetInt("Overrun","RoundLimit",nOver_RoundLimit);
	buteMgr.SetInt("Overrun","TimeLimit",nOver_TimeLimit);
	buteMgr.SetInt("Overrun","DefenderRace",nOver_DefenderRace);
	buteMgr.SetInt("Overrun","DefenderLives",nOver_DefenderLives);
	buteMgr.SetInt("Overrun","AttackerRace",nOver_AttackerRace);
	buteMgr.SetInt("Overrun","AttackerLives",nOver_AttackerLives);

	iter = sOver_Maps.begin();
	i = 0;

	while(iter != sOver_Maps.end() && i < MAX_MP_LEVELS)
	{
		std::string str = *iter;

		sprintf(szBuffer,"Map%02d",i);
		buteMgr.SetString("Overrun",szBuffer,str.c_str());
		iter++;
		i++;
	}


	// Evac settings
	buteMgr.SetInt("Evac","MaxTotalPlayers",nEvac_MaxTotalPlayers);
	buteMgr.SetInt("Evac","RoundLimit",nEvac_RoundLimit);
	buteMgr.SetInt("Evac","TimeLimit",nEvac_TimeLimit);
	buteMgr.SetInt("Evac","DefenderRace",nEvac_DefenderRace);
	buteMgr.SetInt("Evac","DefenderLives",nEvac_DefenderLives);
	buteMgr.SetInt("Evac","AttackerRace",nEvac_AttackerRace);
	buteMgr.SetInt("Evac","AttackerLives",nEvac_AttackerLives);

	iter = sEvac_Maps.begin();
	i = 0;

	while(iter != sEvac_Maps.end() && i < MAX_MP_LEVELS)
	{
		std::string str = *iter;

		sprintf(szBuffer,"Map%02d",i);
		buteMgr.SetString("Evac",szBuffer,str.c_str());
		iter++;
		i++;
	}
}

// ----------------------------------------------------------------------------------------------

void CServerOptions::LoadFromBute(CButeMgr& buteMgr)
{
// Don't need this here.  Since a bute file
// wouldn't be saved out if it was still "default", then
// we'd never have the chance to load it.
//	if (stricmp(szCfgName,".Default") == 0)
//		return;

	CString str;
	char szBuffer[32];
	int i;


	// Basic game information
	str = buteMgr.GetString("Settings","ServerName",szServerName);
	SAFE_STRCPY(szServerName,str);
	CleanServerName();

	str = buteMgr.GetString("Settings","Password",szPassword);
	SAFE_STRCPY(szPassword,str);
	str = buteMgr.GetString("Settings","SCPassword",szSCPassword);
	SAFE_STRCPY(szSCPassword,str);
	bLocked = buteMgr.GetInt("Settings","Locked",bLocked);
	bDedicated = buteMgr.GetInt("Settings","Dedicated",bDedicated);
	bLANConnection = buteMgr.GetInt("Settings","LANConnection",bLANConnection);
	nGameType = buteMgr.GetInt("Settings","GameType",nGameType);
	nPortID = buteMgr.GetInt("Settings","PortID",nPortID);
	nBandwidth = buteMgr.GetInt("Settings","Bandwidth",nBandwidth);


	// Advanced options
	fSpeed = buteMgr.GetFloat("Preferences","Speed",fSpeed);
	fRespawn = buteMgr.GetFloat("Preferences","RespawnSpeed",fRespawn);
	fDamage = buteMgr.GetFloat("Preferences","DamageScale",fDamage);

	bLocationDamage = buteMgr.GetInt("Preferences","LocationDamage",bLocationDamage);
	bFriendlyFire = buteMgr.GetInt("Preferences","FriendlyFire",bFriendlyFire);
	bFriendlyNames = buteMgr.GetInt("Preferences","FriendlyNames",bFriendlyNames);
	bMaskLoss = buteMgr.GetInt("Preferences","MaskLoss",bMaskLoss);
	bClassWeapons = buteMgr.GetInt("Preferences","ClassWeapons",bClassWeapons);
	nExosuit = buteMgr.GetInt("Preferences","Exosuit",nExosuit);
	nQueenMolt = buteMgr.GetInt("Preferences","QueenMolt",nQueenMolt);

	// Load the class scoring.  Keep loading until we fail to get another class/score
	// set.  This will signal the end of the list.
	uint32 nScoreClass = 0;
	i = 0;
	while( 1 )
	{
		// Create the attribute string for getting the class index.
		sprintf(szBuffer, "Class%d", i );

		// Get the next scoring class.
		nScoreClass = buteMgr.GetInt( "Scoring", szBuffer, 0 );
		
		// If we failed to get a scoring class, then stop iterating, there are no more.
		if( !buteMgr.Success( ))
			break;

		// Create the attribute string for getting the score.
		sprintf( szBuffer, "Score%d", i );

		// Get the next score.
		m_hashScoring[nScoreClass] = buteMgr.GetInt( "Scoring", szBuffer, 0 );

		// Advance our counter.
		i++;
	}


	// Deathmatch settings
	nDM_MaxTotalPlayers = buteMgr.GetInt("Deathmatch","MaxTotalPlayers",nDM_MaxTotalPlayers);

	if(nDM_MaxTotalPlayers > 16)
		nDM_MaxTotalPlayers = 16;

	for(i = 0; i < MAX_TEAMS; i++)
	{
		sprintf(szBuffer,"MaxPlayers%01d",i);
		nDM_MaxPlayers[i] = buteMgr.GetInt("Deathmatch",szBuffer,nDM_MaxPlayers[i]);
	}

	nDM_FragLimit = buteMgr.GetInt("Deathmatch","FragLimit",nDM_FragLimit);
	nDM_ScoreLimit = buteMgr.GetInt("Deathmatch","ScoreLimit",nDM_ScoreLimit);
	nDM_TimeLimit = buteMgr.GetInt("Deathmatch","TimeLimit",nDM_TimeLimit);
	nDM_Lifecycle = buteMgr.GetInt("Deathmatch","Lifecycle",nDM_Lifecycle);

	i = 0;
	sprintf(szBuffer,"Map%02d",i);

	while(i < MAX_MP_LEVELS && buteMgr.Exist("Deathmatch",szBuffer))
	{
		str = buteMgr.GetString("Deathmatch",szBuffer,"");
		sDM_Maps.push_back((LPCSTR)str);

		i++;
		sprintf(szBuffer,"Map%02d",i);
	}


	// Team Deathmatch settings
	nTeam_MaxTotalPlayers = buteMgr.GetInt("TeamDeathmatch","MaxTotalPlayers",nTeam_MaxTotalPlayers);

	for(i = 0; i < MAX_TEAMS; i++)
	{
		sprintf(szBuffer,"MaxPlayers%01d",i);
		nTeam_MaxPlayers[i] = buteMgr.GetInt("TeamDeathmatch",szBuffer,nTeam_MaxPlayers[i]);
	}

	nTeam_FragLimit = buteMgr.GetInt("TeamDeathmatch","FragLimit",nTeam_FragLimit);
	nTeam_ScoreLimit = buteMgr.GetInt("TeamDeathmatch","ScoreLimit",nTeam_ScoreLimit);
	nTeam_TimeLimit = buteMgr.GetInt("TeamDeathmatch","TimeLimit",nTeam_TimeLimit);
	nTeam_Lifecycle = buteMgr.GetInt("TeamDeathmatch","Lifecycle",nTeam_Lifecycle);

	i = 0;
	sprintf(szBuffer,"Map%02d",i);

	while(i < MAX_MP_LEVELS && buteMgr.Exist("TeamDeathmatch",szBuffer))
	{
		str = buteMgr.GetString("TeamDeathmatch",szBuffer,"");
		sTeam_Maps.push_back((LPCSTR)str);

		i++;
		sprintf(szBuffer,"Map%02d",i);
	}


	// Hunt settings
	nHunt_MaxTotalPlayers = buteMgr.GetInt("Hunt","MaxTotalPlayers",nHunt_MaxTotalPlayers);
	nHunt_FragLimit = buteMgr.GetInt("Hunt","FragLimit",nHunt_FragLimit);
	nHunt_TimeLimit = buteMgr.GetInt("Hunt","TimeLimit",nHunt_TimeLimit);
	nHunt_HunterRace = buteMgr.GetInt("Hunt","HunterRace",nHunt_HunterRace);
	nHunt_PreyRace = buteMgr.GetInt("Hunt","PreyRace",nHunt_PreyRace);
	nHunt_Ratio = buteMgr.GetInt("Hunt","HunterRatio",nHunt_Ratio);

	i = 0;
	sprintf(szBuffer,"Map%02d",i);

	while(i < MAX_MP_LEVELS && buteMgr.Exist("Hunt",szBuffer))
	{
		str = buteMgr.GetString("Hunt",szBuffer,"");
		sHunt_Maps.push_back((LPCSTR)str);

		i++;
		sprintf(szBuffer,"Map%02d",i);
	}


	// Survivor settings
	nSurv_MaxTotalPlayers = buteMgr.GetInt("Survivor","MaxTotalPlayers",nSurv_MaxTotalPlayers);
	nSurv_ScoreLimit = buteMgr.GetInt("Survivor","ScoreLimit",nSurv_ScoreLimit);
	nSurv_RoundLimit = buteMgr.GetInt("Survivor","RoundLimit",nSurv_RoundLimit);
	nSurv_TimeLimit = buteMgr.GetInt("Survivor","TimeLimit",nSurv_TimeLimit);
	nSurv_SurvivorRace = buteMgr.GetInt("Survivor","SurvivorRace",nSurv_SurvivorRace);
	nSurv_MutateRace = buteMgr.GetInt("Survivor","MutateRace",nSurv_MutateRace);

	i = 0;
	sprintf(szBuffer,"Map%02d",i);

	while(i < MAX_MP_LEVELS && buteMgr.Exist("Survivor",szBuffer))
	{
		str = buteMgr.GetString("Survivor",szBuffer,"");
		sSurv_Maps.push_back((LPCSTR)str);

		i++;
		sprintf(szBuffer,"Map%02d",i);
	}


	// Overrun settings
	nOver_MaxTotalPlayers = buteMgr.GetInt("Overrun","MaxTotalPlayers",nOver_MaxTotalPlayers);
	nOver_RoundLimit = buteMgr.GetInt("Overrun","RoundLimit",nOver_RoundLimit);
	nOver_TimeLimit= buteMgr.GetInt("Overrun","TimeLimit",nOver_TimeLimit);
	nOver_DefenderRace = buteMgr.GetInt("Overrun","DefenderRace",nOver_DefenderRace);
	nOver_DefenderLives = buteMgr.GetInt("Overrun","DefenderLives",nOver_DefenderLives);
	nOver_AttackerRace = buteMgr.GetInt("Overrun","AttackerRace",nOver_AttackerRace);
	nOver_AttackerLives = buteMgr.GetInt("Overrun","AttackerLives",nOver_AttackerLives);

	i = 0;
	sprintf(szBuffer,"Map%02d",i);

	while(i < MAX_MP_LEVELS && buteMgr.Exist("Overrun",szBuffer))
	{
		str = buteMgr.GetString("Overrun",szBuffer,"");
		sOver_Maps.push_back((LPCSTR)str);

		i++;
		sprintf(szBuffer,"Map%02d",i);
	}


	// Evac settings
	nEvac_MaxTotalPlayers = buteMgr.GetInt("Evac","MaxTotalPlayers",nEvac_MaxTotalPlayers);
	nEvac_RoundLimit = buteMgr.GetInt("Evac","RoundLimit",nEvac_RoundLimit);
	nEvac_TimeLimit = buteMgr.GetInt("Evac","TimeLimit",nEvac_TimeLimit);
	nEvac_DefenderRace = buteMgr.GetInt("Evac","DefenderRace",nEvac_DefenderRace);
	nEvac_DefenderLives = buteMgr.GetInt("Evac","DefenderLives",nEvac_DefenderLives);
	nEvac_AttackerRace = buteMgr.GetInt("Evac","AttackerRace",nEvac_AttackerRace);
	nEvac_AttackerLives = buteMgr.GetInt("Evac","AttackerLives",nEvac_AttackerLives);

	i = 0;
	sprintf(szBuffer,"Map%02d",i);

	while(i < MAX_MP_LEVELS && buteMgr.Exist("Evac",szBuffer))
	{
		str = buteMgr.GetString("Evac",szBuffer,"");
		sEvac_Maps.push_back((LPCSTR)str);

		i++;
		sprintf(szBuffer,"Map%02d",i);
	}
}

// ----------------------------------------------------------------------------------------------

void CServerOptions::FillInHostInfo(StartGameRequest &request, MPGameInfo &gi)
{
	// Now fill in information specific to game types
	StringList *sMaps;
	int i;

	switch(nGameType)
	{
		case MP_DM:			sMaps = &(sDM_Maps);	break;
		case MP_TEAMDM:		sMaps = &(sTeam_Maps);	break;
		case MP_HUNT:		sMaps = &(sHunt_Maps);	break;
		case MP_SURVIVOR:	sMaps = &(sSurv_Maps);	break;
		case MP_OVERRUN:	sMaps = &(sOver_Maps);	break;
		case MP_EVAC:		sMaps = &(sEvac_Maps);	break;
		default:			sMaps = NULL;			break;
	}

	if (!sMaps)
	{
		return;
	}

	// Set the game type.
	gi.m_nGameType.Set( nGameType );
	gi.m_nGameType.SetDifficulty( DIFFICULTY_PC_MEDIUM );

	// Fill in the basic game information
	if(strlen(szServerName) > 0)
	{
		strncpy(gi.m_szName, szServerName, MAX_MP_HOST_NAME_LENGTH);
		strncpy(request.m_HostInfo.m_sName, szServerName, MAX_MP_HOST_NAME_LENGTH);
	}
	else
	{
		strncpy(gi.m_szName, "Aliens vs. Predator 2", MAX_MP_HOST_NAME_LENGTH);
		strncpy(request.m_HostInfo.m_sName, "Aliens vs. Predator 2", MAX_MP_HOST_NAME_LENGTH);
	}

	if(strlen(szPassword) > 0)
	{
		strncpy(gi.m_szPassword, szPassword, MAX_MP_PASSWORD_LENGTH);
		strncpy(request.m_HostInfo.m_sPassword, szPassword, MAX_MP_PASSWORD_LENGTH);
	}
	else
	{
		strncpy(gi.m_szPassword, "AVP2", MAX_MP_PASSWORD_LENGTH);
		strncpy(request.m_HostInfo.m_sPassword, "AVP2", MAX_MP_PASSWORD_LENGTH);
	}

	if(strlen(szSCPassword) > 0)
	{
		strncpy(gi.m_szSCPassword, szSCPassword, MAX_MP_PASSWORD_LENGTH);
	}
	else
	{
		strncpy(gi.m_szSCPassword, "AVP2", MAX_MP_PASSWORD_LENGTH);
	}

	gi.m_nPort = (uint16)nPortID;
	request.m_HostInfo.m_Port = nPortID;
	gi.m_nBandwidth = nBandwidth;

	gi.m_bLocked = bLocked;
	gi.m_bDedicated = bDedicated;
	gi.m_bLANConnection = bLANConnection;


#ifdef _DEMO
	gi.m_nNumLevels = 1;

#ifdef _WIN32
	strcpy(gi.m_szLevels[0], "Worlds\\Multi\\DM\\DMDemo_ALesserFate");
	strcpy(request.m_WorldName, "Worlds\\Multi\\DM\\DMDemo_ALesserFate");
#else
	strcpy(gi.m_szLevels[0], "Worlds/Multi/DM/DMDemo_ALesserFate");
	strcpy(request.m_WorldName, "Worlds/Multi/DM/DMDemo_ALesserFate");
#endif

#else
	// Fill in the map names
	std::string str = *(sMaps->begin());
	strcpy(request.m_WorldName, str.c_str());


	StringList slTemp = *sMaps;
	StringList::iterator iter = slTemp.begin();

	gi.m_nNumLevels = 0;

	while(iter != slTemp.end() && gi.m_nNumLevels < MAX_MP_LEVELS)
	{
		std::string str = *iter;

		if(!str.empty())
		{
			strcpy(gi.m_szLevels[gi.m_nNumLevels++], str.c_str());
		}

		iter++;
	}
#endif


	// Fill in the advanced options
	gi.m_fSpeedScale = fSpeed;
	gi.m_fPowerupRespawnScale = fRespawn;
	gi.m_fDamageScale = fDamage;

	gi.m_bLocationDamage = bLocationDamage;
	gi.m_bFriendlyFire = bFriendlyFire;
	gi.m_bFriendlyNames = bFriendlyNames;
	gi.m_bMaskLoss = bMaskLoss;
	gi.m_bClassWeapons = bClassWeapons;
	gi.m_nExosuit = nExosuit;
	gi.m_nQueenMolt = nQueenMolt;


	// Fill in the scoring classes
	ScoringClassHashMap::iterator iterScoring = m_hashScoring.begin( );
	i = 0;
	while( iterScoring != m_hashScoring.end( ))
	{
		// Fill in the scoring class data.
		gi.m_nScoreClasses[i] = iterScoring->first;
		gi.m_nScoreValues[i] = iterScoring->second;

		// Go to next scoring class.
		iterScoring++;
		i++;
	}


	// Now do the game type specific stuff (other than the maps above)
	switch(nGameType)
	{
		case MP_DM:
		{
			request.m_HostInfo.m_dwMaxPlayers = nDM_MaxTotalPlayers;
			gi.m_nPlayerLimit = nDM_MaxTotalPlayers;

			for(i = 0; i < MAX_TEAMS; i++)
				gi.m_nTeamLimits[i] = nDM_MaxPlayers[i];

			gi.m_nFragLimit = nDM_FragLimit;
			gi.m_nScoreLimit = nDM_ScoreLimit;
			gi.m_fTimeLimit = (LTFLOAT)nDM_TimeLimit * 60.0f;

			gi.m_bLifecycle = nDM_Lifecycle;

			break;
		}

		case MP_TEAMDM:
		{
			request.m_HostInfo.m_dwMaxPlayers = nTeam_MaxTotalPlayers;
			gi.m_nPlayerLimit = nTeam_MaxTotalPlayers;

			for(i = 0; i < MAX_TEAMS; i++)
				gi.m_nTeamLimits[i] = nTeam_MaxPlayers[i];

			gi.m_nFragLimit = nTeam_FragLimit;
			gi.m_nScoreLimit = nTeam_ScoreLimit;
			gi.m_fTimeLimit = (LTFLOAT)nTeam_TimeLimit * 60.0f;

			gi.m_bLifecycle = nTeam_Lifecycle;

			break;
		}

		case MP_HUNT:
		{
			request.m_HostInfo.m_dwMaxPlayers = nHunt_MaxTotalPlayers;
			gi.m_nPlayerLimit = nHunt_MaxTotalPlayers;
			gi.m_nHunt_HunterRace = nHunt_HunterRace;
			gi.m_nHunt_PreyRace = nHunt_PreyRace;
			gi.m_nHunt_Ratio = nHunt_Ratio;

			gi.m_nFragLimit = nHunt_FragLimit;
			gi.m_fTimeLimit = (LTFLOAT)nHunt_TimeLimit * 60.0f;

			break;
		}

		case MP_SURVIVOR:
		{
			request.m_HostInfo.m_dwMaxPlayers = nSurv_MaxTotalPlayers;
			gi.m_nPlayerLimit = nSurv_MaxTotalPlayers;
			gi.m_nSurv_SurvivorRace = nSurv_SurvivorRace;
			gi.m_nSurv_MutateRace = nSurv_MutateRace;

			gi.m_nScoreLimit = nSurv_ScoreLimit;
			gi.m_nRoundLimit = nSurv_RoundLimit;
			gi.m_fTimeLimit = (LTFLOAT)nSurv_TimeLimit * 60.0f;

			break;
		}

		case MP_OVERRUN:
		{
			request.m_HostInfo.m_dwMaxPlayers = nOver_MaxTotalPlayers;
			gi.m_nPlayerLimit = nOver_MaxTotalPlayers;

			gi.m_nRoundLimit = nOver_RoundLimit;
			gi.m_fTimeLimit = (LTFLOAT)nOver_TimeLimit * 60.0f;

			gi.m_nTM_DefenderRace = nOver_DefenderRace;
			gi.m_nTM_DefenderLives = nOver_DefenderLives;
			gi.m_nTM_AttackerRace = nOver_AttackerRace;
			gi.m_nTM_AttackerLives = nOver_AttackerLives;

			break;
		}

		case MP_EVAC:
		{
			request.m_HostInfo.m_dwMaxPlayers = nEvac_MaxTotalPlayers;
			gi.m_nPlayerLimit = nEvac_MaxTotalPlayers;

			gi.m_nRoundLimit = nEvac_RoundLimit;
			gi.m_fTimeLimit = (LTFLOAT)nEvac_TimeLimit * 60.0f;

			gi.m_nTM_DefenderRace = nEvac_DefenderRace;
			gi.m_nTM_DefenderLives = nEvac_DefenderLives;
			gi.m_nTM_AttackerRace = nEvac_AttackerRace;
			gi.m_nTM_AttackerLives = nEvac_AttackerLives;

			break;
		}
	}
}

// ----------------------------------------------------------------------------------------------

LTBOOL CServerOptions::MapListEmpty()
{
	StringList *sMaps;

	switch(nGameType)
	{
		case MP_DM:			sMaps = &(sDM_Maps);	break;
		case MP_TEAMDM:		sMaps = &(sTeam_Maps);	break;
		case MP_HUNT:		sMaps = &(sHunt_Maps);	break;
		case MP_SURVIVOR:	sMaps = &(sSurv_Maps);	break;
		case MP_OVERRUN:	sMaps = &(sOver_Maps);	break;
		case MP_EVAC:		sMaps = &(sEvac_Maps);	break;
	}

	return (sMaps->empty() || (sMaps->begin())->empty());
}

uint8 CServerOptions::GetScoring( uint32 nScoringClass )
{
	// Find the class entry.
	ScoringClassHashMap::iterator iter = m_hashScoring.find( nScoringClass );

	// Check if we don't have that entry.
	if( iter == m_hashScoring.end( ))
	{
		return eDefaultScoringValue;
	}

	return iter->second;
}

void CServerOptions::SetScoring( uint32 nScoringClass, uint8 nScoring )
{
	// Apply the new scoring value.
	m_hashScoring[nScoringClass] = nScoring;
}


uint32 GetValidLANPort(uint32 nPortID)
{
	uint32 nLANPort = nPortID;
	if (nLANPort < kMinLANPort)
		nLANPort = kMinLANPort;
	if (nLANPort > kMaxLANPort)
		nLANPort = kMaxLANPort;

	nLANPort -= kMinLANPort;
	nLANPort = kMinLANPort + kPortMod * (nLANPort/kPortMod);

	return nLANPort;

}


void CServerOptions::CleanServerName()
{
	
	//GameSpy chokes on backslashes in server names, so convert 'em to normal slashes
	char *szTemp = strchr(szServerName,'\\');
	LTBOOL bChange = LTFALSE;
	while (szTemp)
	{
		*szTemp = '/';
		szTemp = strchr(szServerName,'\\');
	}

}
