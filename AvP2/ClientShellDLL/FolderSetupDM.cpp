// FolderSetupDM.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderSetupDM.h"
#include "FolderCommands.h"
#include "MultiplayerMgrDefs.h"
#include "ServerOptionMgr.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
	int kWidth = 0;
	int kSlider = 0;

	enum LocalCommands
	{
		CMD_MAXPLAYERS = FOLDER_CMD_CUSTOM,
		CMD_MAXMARINES,
		CMD_MAXALIENS,
		CMD_MAXPREDATORS,
		CMD_MAXCORPORATES
	};
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSetupDM::CFolderSetupDM()
{

}

//////////////////////////////////////////////////////////////////////

CFolderSetupDM::~CFolderSetupDM()
{

}

//////////////////////////////////////////////////////////////////////
// Build the folder

LTBOOL CFolderSetupDM::Build()
{
	// Some temp variables for this function...
	CLTGUITextItemCtrl* pCtrl = LTNULL;
	CStaticTextCtrl *pStatic = LTNULL;
	CSliderCtrl *pSlider = LTNULL;
	CFrameCtrl *pFrame = LTNULL;
	char szTemp[128];
	HSTRING hTemp;
	LTIntPt pos;
	LTRect rect;
	int nWidth, nHeight;


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	// Setup the title of this folder
	CreateTitle(IDS_TITLE_SETUP_DM);
	UseLogo(LTFALSE);


	// Add the links to other menus...
	AddLink(IDS_GAME, FOLDER_CMD_MP_HOST, IDS_HELP_HOST);
	AddLink(IDS_SETUP, FOLDER_CMD_MP_SETUP, IDS_HELP_SETUP)->Enable(LTFALSE);
#ifdef _DEMO
	AddLink(IDS_MAPS, FOLDER_CMD_MP_MAPS, IDS_HELP_MAPS)->Enable(LTFALSE);
#else
	AddLink(IDS_MAPS, FOLDER_CMD_MP_MAPS, IDS_HELP_MAPS);
#endif
	AddLink(IDS_ADVANCED, FOLDER_CMD_MP_ADVANCED, IDS_HELP_ADVANCED);
	AddLink(IDS_PLAYER, FOLDER_CMD_MP_PLAYER, IDS_HELP_PLAYER);


	// Grab some custom values out of the attributes
	kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST_SETUP_DM, "ColumnWidth");
	kSlider = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST_SETUP_DM, "SliderWidth");


	// -----------------------------------------------------------------
	// Create game description area

	// Now create the setup description field
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_HOST_SETUP_DM, "DescriptionRect");
	nWidth = rect.right - rect.left;
	nHeight = rect.bottom - rect.top;

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, nWidth, nHeight);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rect.left, rect.top), LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, nWidth, GetHelpFont()->GetHeight() + 6);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rect.left, rect.top), LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, nWidth, nHeight, LTFALSE);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rect.left, rect.top), LTFALSE);

	hTemp = g_pLTClient->FormatString(IDS_GAME_DESCRIPTION);
	sprintf(szTemp, "--- %s ---\n", g_pLTClient->GetStringData(hTemp));
	pStatic = CreateStaticTextItem(szTemp, LTNULL, LTNULL, nWidth, nHeight, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, LTIntPt(rect.left + 3, rect.top + 3), LTFALSE);
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_GAME_DESCRIPTION_DM);
	pStatic = CreateStaticTextItem(g_pLTClient->GetStringData(hTemp), LTNULL, LTNULL, nWidth - 20, nHeight - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, LTIntPt(rect.left + 10, rect.top + GetHelpFont()->GetHeight() + 10), LTFALSE);
	g_pLTClient->FreeString(hTemp);


	// -----------------------------------------------------------------
	// Create the controls for this folder

	// Max players
	pSlider = AddSlider(IDS_MAXPLAYERS, IDS_HELP_MAXPLAYERS, kWidth, kSlider, &m_nMaxPlayers);
	pSlider->NotifyOnChange(CMD_MAXPLAYERS, this);
	pSlider->SetSliderRange(2, MAX_CLIENTS);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	AddBlankLine();

	// Max marine players
	pSlider = AddSlider(IDS_MAXMARINES, IDS_HELP_MAXMARINES, kWidth, kSlider, &m_nMaxTeams[Marine]);
	pSlider->NotifyOnChange(CMD_MAXMARINES, this);
	pSlider->SetSliderRange(0, MAX_CLIENTS);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	// Max alien players
	pSlider = AddSlider(IDS_MAXALIENS, IDS_HELP_MAXALIENS, kWidth, kSlider, &m_nMaxTeams[Alien]);
	pSlider->NotifyOnChange(CMD_MAXALIENS, this);
	pSlider->SetSliderRange(0, MAX_CLIENTS);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	// Max predator players
	pSlider = AddSlider(IDS_MAXPREDATORS, IDS_HELP_MAXPREDATORS, kWidth, kSlider, &m_nMaxTeams[Predator]);
	pSlider->NotifyOnChange(CMD_MAXPREDATORS, this);
	pSlider->SetSliderRange(0, MAX_CLIENTS);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	// Max corporate players
	pSlider = AddSlider(IDS_MAXCORPORATES, IDS_HELP_MAXCORPORATES, kWidth, kSlider, &m_nMaxTeams[Corporate]);
	pSlider->NotifyOnChange(CMD_MAXCORPORATES, this);
	pSlider->SetSliderRange(0, MAX_CLIENTS);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	AddBlankLine();

	// Frag limit
	pSlider = AddSlider(IDS_FRAG_LIMIT, IDS_HELP_FRAG_LIMIT, kWidth, kSlider, &m_nFragLimit);
	pSlider->SetSliderRange(0, 80);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	// Score limit
	pSlider = AddSlider(IDS_SCORE_LIMIT, IDS_HELP_SCORE_LIMIT, kWidth, kSlider, &m_nScoreLimit);
	pSlider->SetSliderRange(0, 2000);
	pSlider->SetSliderIncrement(100);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	// Time limit
	pSlider = AddSlider(IDS_TIME_LIMIT, IDS_HELP_TIME_LIMIT, kWidth, kSlider, &m_nTimeLimit);
	pSlider->SetSliderRange(0, 60);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	AddBlankLine();

	// Lifecycle
	CToggleCtrl *pToggle = AddToggle(IDS_LIFECYCLE, IDS_HELP_LIFECYCLE, kWidth, &m_bLifecycle);
	pToggle->SetOnString(IDS_ON);
	pToggle->SetOffString(IDS_OFF);

#ifdef _DEMO
	pToggle->Enable(LTFALSE);
#endif


	// Create the bottom 'back' control
	pCtrl = CreateTextItem(IDS_BACK, FOLDER_CMD_MULTIPLAYER, IDS_HELP_MULTIPLAYER, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl, kBackGroup, m_BackPos);
	UseBack(LTFALSE);

	// Create the Launch button
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"LaunchPos");
	pCtrl = CreateTextItem(IDS_LAUNCH, FOLDER_CMD_LAUNCH, IDS_HELP_LAUNCH, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl, kNextGroup, pos);


	return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Handle commands

uint32 CFolderSetupDM::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_MAXPLAYERS:
		{
			UpdateData(LTTRUE);

//			uint32 nReturn = CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);

			int nTotal = 0;

			for(int i = 0; i < MAX_TEAMS; i++)
				nTotal += (int)m_nMaxTeams[i];

			if(nTotal < m_nMaxPlayers)
			{
				m_nMaxTeams[Marine] += (m_nMaxPlayers - nTotal);
				UpdateData(LTFALSE);
			}

			return 1;
		}

		case CMD_MAXMARINES:
		case CMD_MAXALIENS:
		case CMD_MAXPREDATORS:
		case CMD_MAXCORPORATES:
		{
			UpdateData(LTTRUE);

//			uint32 nReturn = CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);

			int nTotal = 0;

			for(int i = 0; i < MAX_TEAMS; i++)
				nTotal += (int)m_nMaxTeams[i];

			if((nTotal < m_nMaxPlayers) && CBaseFolder::GetSelectedControl())
			{
				CBaseFolder::GetSelectedControl()->OnRight();
				UpdateData(LTFALSE);
			}

			return 1;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Change in focus

void CFolderSetupDM::OnFocus(LTBOOL bFocus)
{
	FocusSetup(bFocus);

	CBaseFolder::OnFocus(bFocus);
}


void CFolderSetupDM::FocusSetup(LTBOOL bFocus)
{
	CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

	if(bFocus)
	{
		m_nMaxPlayers = (int)pOptions->nDM_MaxTotalPlayers;

		for(int i = 0; i < MAX_TEAMS; i++)
			m_nMaxTeams[i] = (int)pOptions->nDM_MaxPlayers[i];

		m_nFragLimit = (int)pOptions->nDM_FragLimit;
		m_nScoreLimit = (int)pOptions->nDM_ScoreLimit;
		m_nTimeLimit = (int)pOptions->nDM_TimeLimit;
		m_bLifecycle = (LTBOOL)pOptions->nDM_Lifecycle;

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData(LTTRUE);

		pOptions->nDM_MaxTotalPlayers = (uint8)m_nMaxPlayers;

		for(int i = 0; i < MAX_TEAMS; i++)
			pOptions->nDM_MaxPlayers[i] = (int)m_nMaxTeams[i];

		pOptions->nDM_FragLimit = (uint8)m_nFragLimit;
		pOptions->nDM_ScoreLimit = (uint16)m_nScoreLimit;
		pOptions->nDM_TimeLimit = (uint8)m_nTimeLimit;
		pOptions->nDM_Lifecycle = (uint8)m_bLifecycle;

		g_pServerOptionMgr->SaveCurrentCfg();
	}
}


//////////////////////////////////////////////////////////////////////

void CFolderSetupDM::Escape()
{
	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
}

