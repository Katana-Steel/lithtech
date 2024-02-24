// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostDemolitionOptions.cpp
//
// PURPOSE : Interface screen for hosting multi player demolition games
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostDemolitionOptions.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "ResShared.h"

namespace
{
	const int kMaxScoreLimit = 200;
	const int kMaxTimeLimit = 60;
	const int kMaxRunSpeed = 150;
	const int kMaxRounds = 20;

	const uint32 CMD_TEAM1 = CMD_CUSTOM+1;
	const uint32 CMD_TEAM2 = CMD_CUSTOM+2;
}

extern uint8 g_nCurTeam;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostDemolitionOptions::CScreenHostDemolitionOptions()
{
	m_nMaxPlayers = 8;
	m_nRunSpeed = 130;
	m_nRounds = 2;
	m_bFriendlyFire = LTFALSE;

	m_nFragScore = 1;

	m_bWeaponsStay = true;

	m_pMaxPlayers = NULL;
}


CScreenHostDemolitionOptions::~CScreenHostDemolitionOptions()
{
}

// Build the screen
LTBOOL CScreenHostDemolitionOptions::Build()
{
	int kColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST_DE_OPTIONS,"ColumnWidth");
	int kSlider = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST_DE_OPTIONS,"SliderWidth");

	CreateTitle(IDS_TITLE_HOST_OPTIONS);

	m_pTeam1 = AddTextItem(IDS_TEAM_1_OPTIONS, CMD_TEAM1, IDS_HELP_TEAM_1);
	m_pTeam2 = AddTextItem(IDS_TEAM_2_OPTIONS, CMD_TEAM2, IDS_HELP_TEAM_2);

	/*
	LTIntPt tmp = m_nextPos;

	LTIntPt pos = m_pTeam1->GetBasePos();
	pos.x += kColumn;
	m_pTeam1Name = AddTextItem("<team one>",0,0,pos,LTTRUE);

	pos = m_pTeam2->GetBasePos();
	pos.x += kColumn;
	m_pTeam2Name = AddTextItem("<team two>",0,0,pos,LTTRUE);

	m_nextPos = tmp;
	*/
	
	m_pMaxPlayers = AddSlider(IDS_MAX_PLAYERS, IDS_MAX_PLAYERS_HELP, kColumn, kSlider, -1, &m_nMaxPlayers);
	m_pMaxPlayers->SetSliderRange(2, 16);
	m_pMaxPlayers->SetSliderIncrement(1);
	m_pMaxPlayers->SetNumericDisplay(LTTRUE);


	char szYes[16];
	char szNo[16];
	FormatString(IDS_YES,szYes,sizeof(szYes));
	FormatString(IDS_NO,szNo,sizeof(szNo));

	CLTGUIToggle* pToggle = AddToggle(IDS_FRIENDLY_FIRE,IDS_FRIENDLY_FIRE_HELP,kColumn,&m_bFriendlyFire);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

	CLTGUISlider*	pSlider = AddSlider(IDS_RUN_SPEED, IDS_RUN_SPEED_HELP, kColumn, kSlider, -1, &m_nRunSpeed);
	pSlider->SetSliderRange(100, kMaxRunSpeed);
	pSlider->SetSliderIncrement(10);
	pSlider->SetNumericDisplay(LTTRUE);

	pSlider = AddSlider( IDS_ROUNDS, IDS_ROUNDS_HELP, kColumn, kSlider, -1, &m_nRounds );
	pSlider->SetSliderRange( 2, kMaxRounds );
	pSlider->SetSliderIncrement( 2 );
	pSlider->SetNumericDisplay( LTTRUE );

	pSlider = AddSlider(IDS_FRAG_SCORE, IDS_FRAG_SCORE_HELP, kColumn, kSlider, -1, &m_nFragScore);
	pSlider->SetSliderRange(0,3);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE);

	pToggle = AddToggle(IDS_WEAPONSSTAY, IDS_WEAPONSSTAY_HELP, kColumn,&m_bWeaponsStay);
	pToggle->SetOnString(szYes);
	pToggle->SetOffString(szNo);

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

// Change in focus
void    CScreenHostDemolitionOptions::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{
		m_nMaxPlayers = (int)pProfile->m_ServerGameOptions.GetDemolition().m_nMaxPlayers;
		m_nRunSpeed = (int)pProfile->m_ServerGameOptions.GetDemolition().m_nRunSpeed;
		m_nRounds = (int)pProfile->m_ServerGameOptions.GetDemolition().m_nRounds;
		m_bFriendlyFire = pProfile->m_ServerGameOptions.GetDemolition().m_bFriendlyFire;

		m_nFragScore = (int)pProfile->m_ServerGameOptions.GetDemolition().m_nFragScore;
/*
		m_pTeam1Name->SetString(pProfile->m_ServerGameOptions.GetDemolition().m_sTeamName[0].c_str());
		m_pTeam2Name->SetString(pProfile->m_ServerGameOptions.GetDemolition().m_sTeamName[1].c_str());
*/
		m_pMaxPlayers->Enable(!g_pPlayerMgr->IsPlayerInWorld());

		m_bWeaponsStay = pProfile->m_ServerGameOptions.GetDemolition().m_bWeaponsStay;

        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		pProfile->m_ServerGameOptions.GetDemolition().m_nMaxPlayers = (uint8)m_nMaxPlayers;
		pProfile->m_ServerGameOptions.GetDemolition().m_nRunSpeed = (uint8)m_nRunSpeed;
		pProfile->m_ServerGameOptions.GetDemolition().m_nRounds = (uint8)m_nRounds;
		pProfile->m_ServerGameOptions.GetDemolition().m_bFriendlyFire = !!m_bFriendlyFire;
		pProfile->m_ServerGameOptions.GetDemolition().m_bWeaponsStay = !!m_bWeaponsStay;

		pProfile->m_ServerGameOptions.GetDemolition().m_nFragScore = (uint8)m_nFragScore;
		
		pProfile->Save();

		if (g_pPlayerMgr->IsPlayerInWorld())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_MULTIPLAYER_OPTIONS );
			pProfile->m_ServerGameOptions.GetDemolition().Write(cMsg);
		    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}

	}
	CBaseScreen::OnFocus(bFocus);
}



uint32 CScreenHostDemolitionOptions::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_TEAM1:
		{
			g_nCurTeam = 0;
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_TEAM);
		} break;

	case CMD_TEAM2:
		{
			g_nCurTeam = 1;
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_TEAM);
		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

