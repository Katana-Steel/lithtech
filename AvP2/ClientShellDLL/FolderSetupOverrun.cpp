// FolderSetupOverrun.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderSetupOverrun.h"
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
		CMD_DEFENDERS = FOLDER_CMD_CUSTOM,
		CMD_ATTACKERS
	};
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSetupOverrun::CFolderSetupOverrun()
{

}

//////////////////////////////////////////////////////////////////////

CFolderSetupOverrun::~CFolderSetupOverrun()
{

}

//////////////////////////////////////////////////////////////////////
// Build the folder

LTBOOL CFolderSetupOverrun::Build()
{
	// Some temp variables for this function...
	CLTGUITextItemCtrl* pCtrl = LTNULL;
	CStaticTextCtrl *pStatic = LTNULL;
	CSliderCtrl *pSlider = LTNULL;
	CCycleCtrl *pCycle = LTNULL;
	CFrameCtrl *pFrame = LTNULL;
	char szTemp[128];
	HSTRING hTemp;
	LTIntPt pos;
	LTRect rect;
	int nWidth, nHeight;


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	// Setup the title of this folder
	CreateTitle(IDS_TITLE_SETUP_OVERRUN);
	UseLogo(LTFALSE);


	// Add the links to other menus...
	AddLink(IDS_GAME, FOLDER_CMD_MP_HOST, IDS_HELP_HOST);
	AddLink(IDS_SETUP, FOLDER_CMD_MP_SETUP, IDS_HELP_SETUP)->Enable(LTFALSE);
	AddLink(IDS_MAPS, FOLDER_CMD_MP_MAPS, IDS_HELP_MAPS);
	AddLink(IDS_ADVANCED, FOLDER_CMD_MP_ADVANCED, IDS_HELP_ADVANCED);
	AddLink(IDS_PLAYER, FOLDER_CMD_MP_PLAYER, IDS_HELP_PLAYER);


	// Grab some custom values out of the attributes
	kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST_SETUP_DM, "ColumnWidth");
	kSlider = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST_SETUP_DM, "SliderWidth");


	// -----------------------------------------------------------------
	// Create game description area

	// Now create the setup description field
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_HOST_SETUP_OVERRUN, "DescriptionRect");
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

	hTemp = g_pLTClient->FormatString(IDS_GAME_DESCRIPTION_OVERRUN);
	pStatic = CreateStaticTextItem(g_pLTClient->GetStringData(hTemp), LTNULL, LTNULL, nWidth - 20, nHeight - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, LTIntPt(rect.left + 10, rect.top + GetHelpFont()->GetHeight() + 10), LTFALSE);
	g_pLTClient->FreeString(hTemp);


	// -----------------------------------------------------------------
	// Create the controls for this folder

	// Max players
	pSlider = AddSlider(IDS_MAXPLAYERS, IDS_HELP_MAXPLAYERS, kWidth, kSlider, &m_nMaxPlayers);
	pSlider->SetSliderRange(2, MAX_CLIENTS);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	AddBlankLine();

	// Round limit
	pSlider = AddSlider(IDS_ROUND_LIMIT, IDS_HELP_ROUND_LIMIT, kWidth, kSlider, &m_nRoundLimit);
	pSlider->SetSliderRange(1, 25);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	// Time limit
	pSlider = AddSlider(IDS_ROUND_TIME, IDS_HELP_ROUND_TIME, kWidth, kSlider, &m_nTimeLimit);
	pSlider->SetSliderRange(0, 30);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	AddBlankLine();

	// Defender race
	pCycle = AddCycleItem(IDS_DEFENDER_RACE, IDS_HELP_DEFENDER_RACE, kWidth - 25, 25, &m_nDefenderRace);
	pCycle->NotifyOnChange(CMD_DEFENDERS, this);
	pCycle->AddString(IDS_MP_TEAM_ALIENS);
	pCycle->AddString(IDS_MP_TEAM_MARINES);
	pCycle->AddString(IDS_MP_TEAM_PREDATORS);
	pCycle->AddString(IDS_MP_TEAM_CORPORATES);

	// Defender lives
	pSlider = AddSlider(IDS_DEFENDER_LIVES, IDS_HELP_DEFENDER_LIVES, kWidth, kSlider, &m_nDefenderLives);
	pSlider->SetSliderRange(1, 20);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	AddBlankLine();

	// Attacker race
	pCycle = AddCycleItem(IDS_ATTACKER_RACE, IDS_HELP_ATTACKER_RACE, kWidth - 25, 25, &m_nAttackerRace);
	pCycle->NotifyOnChange(CMD_ATTACKERS, this);
	pCycle->AddString(IDS_MP_TEAM_ALIENS);
	pCycle->AddString(IDS_MP_TEAM_MARINES);
	pCycle->AddString(IDS_MP_TEAM_PREDATORS);
	pCycle->AddString(IDS_MP_TEAM_CORPORATES);

	// Attacker lives
	pSlider = AddSlider(IDS_ATTACKER_LIVES, IDS_HELP_ATTACKER_LIVES, kWidth, kSlider, &m_nAttackerLives);
	pSlider->SetSliderRange(1, 20);
	pSlider->SetSliderIncrement(1);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);



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

uint32 CFolderSetupOverrun::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_DEFENDERS:
		case CMD_ATTACKERS:
		{
			uint32 nReturn = CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);

			UpdateData(LTTRUE);

			if((m_nDefenderRace == m_nAttackerRace) && CBaseFolder::GetSelectedControl())
			{
				CBaseFolder::GetSelectedControl()->OnRight();
			}

			return nReturn;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////
// Change in focus

void CFolderSetupOverrun::OnFocus(LTBOOL bFocus)
{
	FocusSetup(bFocus);

	CBaseFolder::OnFocus(bFocus);
}


void CFolderSetupOverrun::FocusSetup(LTBOOL bFocus)
{
	CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

	if(bFocus)
	{
		m_nMaxPlayers = (int)pOptions->nOver_MaxTotalPlayers;
		m_nRoundLimit = (int)pOptions->nOver_RoundLimit;
		m_nTimeLimit = (int)pOptions->nOver_TimeLimit;

		m_nDefenderRace = (int)pOptions->nOver_DefenderRace;
		m_nDefenderLives = (int)pOptions->nOver_DefenderLives;
		m_nAttackerRace = (int)pOptions->nOver_AttackerRace;
		m_nAttackerLives = (int)pOptions->nOver_AttackerLives;

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData(LTTRUE);

		pOptions->nOver_MaxTotalPlayers = (uint8)m_nMaxPlayers;
		pOptions->nOver_RoundLimit = (uint8)m_nRoundLimit;
		pOptions->nOver_TimeLimit = (uint8)m_nTimeLimit;

		pOptions->nOver_DefenderRace = (uint8)m_nDefenderRace;
		pOptions->nOver_DefenderLives = (uint8)m_nDefenderLives;
		pOptions->nOver_AttackerRace = (uint8)m_nAttackerRace;
		pOptions->nOver_AttackerLives = (uint8)m_nAttackerLives;

		g_pServerOptionMgr->SaveCurrentCfg();
	}
}


//////////////////////////////////////////////////////////////////////

void CFolderSetupOverrun::Escape()
{
	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
}

