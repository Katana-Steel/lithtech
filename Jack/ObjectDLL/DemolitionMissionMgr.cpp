// ----------------------------------------------------------------------- //
//
// MODULE  : DemolitionMissionMgr.h
//
// PURPOSE : Definition of class to handle demolition missions
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "DemolitionMissionMgr.h"
#include "TeamMgr.h"
#include "MissionButeMgr.h"
#include "PlayerObj.h"
#include "ServerSaveLoadMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::CDemolitionMissionMgr
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CDemolitionMissionMgr::CDemolitionMissionMgr()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::~CDemolitionMissionMgr
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CDemolitionMissionMgr::~CDemolitionMissionMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

bool CDemolitionMissionMgr::Init()
{
	if( !CServerMissionMgr::Init() )
		return false;

	// Add the teams to the team mgr...

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );

	uint8 nNumTeams = sgo.GetDemolition().m_nNumTeams;

	for( uint8 i = 0; i < nNumTeams; ++i )
	{
		CTeamMgr::Instance().AddTeam( sgo.GetDemolition().m_sTeamName[i].c_str(),
									  sgo.GetDemolition().m_nTeamModel[i] );
	}

	return !!CTeamMgr::Instance().GetNumTeams();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::Update
//
//	PURPOSE:	Frame update.  Check for end level condition...
//
// ----------------------------------------------------------------------- //

void CDemolitionMissionMgr::Update()
{
	// Check the teams to see if any of them have reached the score limit...
	
	if( m_ServerSettings.m_nScoreLimit > 0 && m_fStartTime >= 0.0f )
	{
		for( uint8 i = 0; i < CTeamMgr::Instance().GetNumTeams(); ++i )
		{
			CTeam *pTeam = CTeamMgr::Instance().GetTeam( i );
			if( pTeam && (pTeam->GetScore() >= m_ServerSettings.m_nScoreLimit) )
			{
				CTeamMgr::Instance( ).WonRound( i );
				g_pLTServer->CPrint( "ScoreLimit reached." );
				NextRound();
			}
		}
	}


	CServerMissionMgr::Update( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::StartGame
//
//	PURPOSE:	Start the game.
//
// --------------------------------------------------------------------------- //

bool CDemolitionMissionMgr::StartGame(ILTMessage_Read& msg )
{
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = sgo.GetDemolition().m_bFriendlyFire;
	sms.m_bWeaponsStay = sgo.GetDemolition().m_bWeaponsStay;
	sms.m_nRunSpeed = sgo.GetDemolition().m_nRunSpeed;
	sms.m_nScoreLimit = 0;
	sms.m_nTimeLimit = 0;
	sms.m_nRounds = ( sgo.GetDemolition().m_nRounds & 1 ) ? sgo.GetDemolition().m_nRounds + 1 : 
		sgo.GetDemolition().m_nRounds;
	sms.m_nRounds = Max( sms.m_nRounds, ( uint8 )2 );
	sms.m_nFragScore = sgo.GetDemolition().m_nFragScore;

	SetServerSettings(sms);

	if( !g_pMissionButeMgr->Init( MISSION_DM_FILE ))
		return false;

	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::LevelStarted()
//
//	PURPOSE:	Called when the clients have finished loading and have started
//				playing a level
//
// ----------------------------------------------------------------------- //

void CDemolitionMissionMgr::LevelStarted()
{
	CServerMissionMgr::LevelStarted( );

	// Check if this is a new level, or just another round.
	if( m_bNewMission )
	{
		CTeamMgr::Instance( ).NewLevel( );
	}
	else
	{
		CTeamMgr::Instance( ).NewRound( );
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleMultiplayerOptions
//
//	PURPOSE:	Handle updating the host options
//
// --------------------------------------------------------------------------- //

bool CDemolitionMissionMgr::HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg )
{

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	sgo.GetDemolition().Read(&msg);

	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = sgo.GetDemolition().m_bFriendlyFire;
	sms.m_nRunSpeed = sgo.GetDemolition().m_nRunSpeed;
	sms.m_nScoreLimit = 0;
	sms.m_nTimeLimit = 0;
	sms.m_nRounds = sgo.GetDemolition().m_nRounds;
	sms.m_nFragScore = sgo.GetDemolition().m_nFragScore;
	sms.m_bWeaponsStay = sgo.GetDemolition( ).m_bWeaponsStay;

	SetServerSettings(sms);

	return true;

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started.
//
// --------------------------------------------------------------------------- //

bool CDemolitionMissionMgr::FinishExitLevel( )
{
	TRACE( "CDemolitionMissionMgr::FinishExitLevel:\n" );

	// No longer waiting for exitlevel messages back from client.
	m_bExitingLevel = false;

	// Clear out the playertracker.
	m_PlayerTracker.Term( );

	// Tell the players to handle an exit level.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		pPlayerObj->HandleExit( true );
		iter++;
	}

	// Load new level with no restoring of keepalives or save games.
	if( !g_pServerSaveLoadMgr->LoadNewLevel( m_sCurrentWorldName ))
		return false;

	// Unpause the server now that we're done switching levels.
	g_pGameServerShell->PauseGame( LTFALSE );	

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDemolitionMissionMgr::EndGame
//
//	PURPOSE:	End the game.
//
// --------------------------------------------------------------------------- //

bool CDemolitionMissionMgr::EndGame( )
{
	// Call base.
	if( !CServerMissionMgr::EndGame( ))
		return false;

	// Can't have custom level or no campaign in deathmatch.
	if( m_bCustomLevel || m_Campaign.size( ) == 0 )
	{
		ASSERT( !"CDemolitionMissionMgr::EndGame:  Can't have custom level or no campaign in doomsday." );
		return false;
	}

	// Start from the beginning.
	m_nCurrentLevel = 0;
	m_nCurCampaignIndex = 0;
	m_nCurrentMission = m_Campaign[m_nCurCampaignIndex];
	m_bNewMission = true;

	// Get the worldname from the mission/level indices.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, m_nCurrentLevel );
	if( !pszFilename )
		return false;

	// Exit to the level.
	if( !ExitLevel( pszFilename, false ))
		return false;

	return true;
}


