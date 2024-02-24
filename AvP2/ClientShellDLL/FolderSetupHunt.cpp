// FolderSetupHunt.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderSetupHunt.h"
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
		CMD_HUNTERS = FOLDER_CMD_CUSTOM,
		CMD_PREY
	};
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSetupHunt::CFolderSetupHunt()
{

}

//////////////////////////////////////////////////////////////////////

CFolderSetupHunt::~CFolderSetupHunt()
{

}

//////////////////////////////////////////////////////////////////////
// Build the folder

LTBOOL CFolderSetupHunt::Build()
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
	CreateTitle(IDS_TITLE_SETUP_HUNT);
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
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_HOST_SETUP_HUNT, "DescriptionRect");
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

	hTemp = g_pLTClient->FormatString(IDS_GAME_DESCRIPTION_HUNT);
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

	// Frag limit
	pSlider = AddSlider(IDS_FRAG_LIMIT, IDS_HELP_FRAG_LIMIT, kWidth, kSlider, &m_nFragLimit);
	pSlider->SetSliderRange(0, 80);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	// Time limit
	pSlider = AddSlider(IDS_TIME_LIMIT, IDS_HELP_TIME_LIMIT, kWidth, kSlider, &m_nTimeLimit);
	pSlider->SetSliderRange(0, 60);
	pSlider->SetSliderIncrement(5);
	pSlider->SetNumericDisplay(LTTRUE, LTTRUE);

	AddBlankLine();

	// Hunter race
	pCycle = AddCycleItem(IDS_HUNTER_RACE, IDS_HELP_HUNTER_RACE, kWidth - 25, 25, &m_nHunterRace);
	pCycle->NotifyOnChange(CMD_HUNTERS, this);
	pCycle->AddString(IDS_MP_TEAM_ALIENS);
	pCycle->AddString(IDS_MP_TEAM_MARINES);
	pCycle->AddString(IDS_MP_TEAM_PREDATORS);
	pCycle->AddString(IDS_MP_TEAM_CORPORATES);

	// Prey race
	pCycle = AddCycleItem(IDS_PREY_RACE, IDS_HELP_PREY_RACE, kWidth - 25, 25, &m_nPreyRace);
	pCycle->NotifyOnChange(CMD_PREY, this);
	pCycle->AddString(IDS_MP_TEAM_ALIENS);
	pCycle->AddString(IDS_MP_TEAM_MARINES);
	pCycle->AddString(IDS_MP_TEAM_PREDATORS);
	pCycle->AddString(IDS_MP_TEAM_CORPORATES);

	AddBlankLine();

	// Ratio
	pCycle = AddCycleItem(IDS_HUNT_RATIO, IDS_HELP_HUNT_RATIO, kWidth - 25, 25, &m_nRatio);
	pCycle->AddString(IDS_RATIO_1_1);
	pCycle->AddString(IDS_RATIO_1_2);
	pCycle->AddString(IDS_RATIO_1_3);
	pCycle->AddString(IDS_RATIO_1_4);
	pCycle->AddString(IDS_RATIO_1_5);


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

uint32 CFolderSetupHunt::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_HUNTERS:
		case CMD_PREY:
		{
			uint32 nReturn = CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);

			UpdateData(LTTRUE);

			if((m_nHunterRace == m_nPreyRace) && CBaseFolder::GetSelectedControl())
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

void CFolderSetupHunt::OnFocus(LTBOOL bFocus)
{
	FocusSetup(bFocus);

	CBaseFolder::OnFocus(bFocus);
}


void CFolderSetupHunt::FocusSetup(LTBOOL bFocus)
{
	CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

	if(bFocus)
	{
		m_nMaxPlayers = (int)pOptions->nHunt_MaxTotalPlayers;
		m_nFragLimit = (int)pOptions->nHunt_FragLimit;
		m_nTimeLimit = (int)pOptions->nHunt_TimeLimit;

		m_nHunterRace = (int)pOptions->nHunt_HunterRace;
		m_nPreyRace = (int)pOptions->nHunt_PreyRace;
		m_nRatio = (int)(pOptions->nHunt_Ratio - 1);

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData(LTTRUE);

		pOptions->nHunt_MaxTotalPlayers = (uint8)m_nMaxPlayers;
		pOptions->nHunt_FragLimit = (uint8)m_nFragLimit;
		pOptions->nHunt_TimeLimit = (uint8)m_nTimeLimit;

		pOptions->nHunt_HunterRace = (uint8)m_nHunterRace;
		pOptions->nHunt_PreyRace = (uint8)m_nPreyRace;
		pOptions->nHunt_Ratio = (uint8)(m_nRatio + 1);

		g_pServerOptionMgr->SaveCurrentCfg();
	}
}


//////////////////////////////////////////////////////////////////////

void CFolderSetupHunt::Escape()
{
	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
}

