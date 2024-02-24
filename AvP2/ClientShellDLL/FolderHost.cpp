// FolderHost.cpp: implementation of the CFolderHost class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderHost.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "MultiplayerMgrDefs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	int kWidth = 0;
	int kSlider = 0;

	enum LocalCommands
	{
		CMD_NAME = FOLDER_CMD_CUSTOM,
		CMD_NAMECHANGE,
		CMD_PASS,
		CMD_SCPASS,
		CMD_PASSCHANGE,
		CMD_TYPE_CHANGE,
		CMD_PASSCHECKON,
		CMD_PASSCHECKOFF,
	};
}

CFolderHost::CFolderHost()
{
	m_pNameGroup = LTNULL;
	m_pEdit = LTNULL;
	m_pLabel = LTNULL;

	m_pPassGroup = LTNULL;
	m_pPassEdit = LTNULL;
	m_pPassLabel = LTNULL;

	m_pSCPassGroup = LTNULL;
	m_pSCPassEdit = LTNULL;
	m_pSCPassLabel = LTNULL;

	m_pSetup = LTNULL;
	m_pAdvanced = LTNULL;

	m_pPassCheck = LTNULL;

	m_szOldName[0] = 0;
	m_szOldPass[0] = 0;

	m_bUsePassword = LTFALSE;

	m_nType = 0;
	m_nMaxPlayers = 8;
}

CFolderHost::~CFolderHost()
{

}


// Build the folder
LTBOOL CFolderHost::Build()
{
	// Some temp variables for this function...
	CLTGUITextItemCtrl* pCtrl = LTNULL;
	CStaticTextCtrl *pStatic = LTNULL;
	CFrameCtrl *pFrame = LTNULL;
	char szTemp[128];
	HSTRING hTemp;
	LTIntPt pos;
	LTRect rect;
	int nWidth, nHeight;


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;


	// Create the title of this menu...
	CreateTitle(IDS_TITLE_HOST);
	UseLogo(LTTRUE);


	// Add the links to other menus...
	AddLink(IDS_GAME, FOLDER_CMD_MP_HOST, IDS_HELP_HOST)->Enable(LTFALSE);
	AddLink(IDS_SETUP, FOLDER_CMD_MP_SETUP, IDS_HELP_SETUP);
#ifdef _DEMO
	AddLink(IDS_MAPS, FOLDER_CMD_MP_MAPS, IDS_HELP_MAPS)->Enable(LTFALSE);
#else
	AddLink(IDS_MAPS, FOLDER_CMD_MP_MAPS, IDS_HELP_MAPS);
#endif
	AddLink(IDS_ADVANCED, FOLDER_CMD_MP_ADVANCED, IDS_HELP_ADVANCED);
	AddLink(IDS_PLAYER, FOLDER_CMD_MP_PLAYER, IDS_HELP_PLAYER);

	// Grab some custom values out of the attributes
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_HOST, "ColumnWidth"))
		kWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST, "ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_HOST, "SliderWidth"))
		kSlider = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST, "SliderWidth");


	// Create the server name control
	m_pEdit = CreateEditCtrl("", CMD_NAME, IDS_HELP_SERVER_NAME, m_Options.szServerName, 32);
	m_pEdit->EnableCursor();
//	m_pEdit->NotifyOnChange(CMD_NAMECHANGE, this);

	m_pLabel = CreateTextItem(IDS_SERVER_NAME, CMD_NAME, IDS_HELP_SERVER_NAME);

	int nGroupWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST, "NameGroupWidth");

	m_pNameGroup = CreateGroup(nGroupWidth, m_pLabel->GetHeight(), IDS_HELP_SERVER_NAME);
    m_pNameGroup->AddControl(m_pLabel, LTIntPt(0, 0), LTTRUE);
    m_pNameGroup->AddControl(m_pEdit, LTIntPt(kWidth - 25, 0), LTFALSE);
	AddFreeControl(m_pNameGroup);


	// Create the Game Type control
	CCycleCtrl *pCycle = AddCycleItem(IDS_TYPE, IDS_HELP_TYPE, kWidth - 25, 25, &m_nType);
	pCycle->NotifyOnChange(CMD_TYPE_CHANGE, this);
	pCycle->AddString(IDS_MP_STAT_DM);
	pCycle->AddString(IDS_MP_STAT_TEAMDM);
#ifndef _DEMO
	pCycle->AddString(IDS_MP_STAT_HUNT);
	pCycle->AddString(IDS_MP_STAT_SURVIVOR);
	pCycle->AddString(IDS_MP_STAT_OVERRUN);
	pCycle->AddString(IDS_MP_STAT_EVAC);
#endif


	// Create the password control
	m_pPassEdit = CreateEditCtrl("", CMD_PASS, IDS_HELP_PASSWORD, m_Options.szPassword, 12);
	m_pPassEdit->SetInputMode(CLTGUIEditCtrl::kInputAlphaNumeric);
	m_pPassEdit->EnableCursor();
//	m_pPassEdit->NotifyOnChange(CMD_PASSCHANGE, this);

	m_pPassLabel = CreateTextItem(IDS_PASSWORD, CMD_PASS, IDS_HELP_PASSWORD);

	nGroupWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_HOST, "PasswordGroupWidth");

	m_pPassGroup = CreateGroup(nGroupWidth, m_pPassLabel->GetHeight(), IDS_HELP_PASSWORD);
	m_pPassGroup->AddControl(m_pPassLabel, LTIntPt(0, 0), LTTRUE);
	m_pPassGroup->AddControl(m_pPassEdit, LTIntPt(kWidth - 25, 0), LTFALSE);
	AddFreeControl(m_pPassGroup);

	m_pSCPassEdit = CreateEditCtrl("", CMD_SCPASS, IDS_HELP_SCPASSWORD, m_Options.szSCPassword, 12);
	m_pSCPassEdit->SetInputMode(CLTGUIEditCtrl::kInputAlphaNumeric);
	m_pSCPassEdit->EnableCursor();

	m_pSCPassLabel = CreateTextItem(IDS_SCPASSWORD, CMD_SCPASS, IDS_HELP_SCPASSWORD);

	m_pSCPassGroup = CreateGroup(nGroupWidth, m_pSCPassLabel->GetHeight(), IDS_HELP_SCPASSWORD);
	m_pSCPassGroup->AddControl(m_pSCPassLabel, LTIntPt(0, 0), LTTRUE);
	m_pSCPassGroup->AddControl(m_pSCPassEdit, LTIntPt(kWidth - 25, 0), LTFALSE);
	AddFreeControl(m_pSCPassGroup);

	// Lock button
	m_pPassCheck = new CBitmapCtrl;
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "PassCheckPos");
	AddFixedControl(m_pPassCheck, kLinkGroup, pos);


	// Create a control to determine whether this is a dedicated server or not
	CToggleCtrl *pToggle = CreateToggle(IDS_DEDICATED,IDS_HELP_DEDICATED,kWidth,&m_Options.bDedicated);
	pToggle->SetOnString(IDS_YES);
	pToggle->SetOffString(IDS_NO);

//	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID, "DedicatedPos");
//	AddFixedControl(pToggle, kLinkGroup, pos);
	AddFreeControl(pToggle);


	// Add the button for custom configurations...
	pCtrl = CreateTextItem(IDS_CONFIG, FOLDER_CMD_MP_CONFIG, IDS_HELP_CONFIG);
	AddFreeControl(pCtrl);


	// Now create the setup description field
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_HOST, "SetupTextRect");
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

	hTemp = g_pLTClient->FormatString(IDS_HOST_SETTINGS);
	sprintf(szTemp, "--- %s ---\n", g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);

	pStatic = CreateStaticTextItem(szTemp, LTNULL, LTNULL, nWidth, nHeight, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, LTIntPt(rect.left + 3, rect.top + 3), LTFALSE);

	m_pSetup = CreateStaticTextItem("<>", LTNULL, LTNULL, nWidth - 20, nHeight - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());
	AddFixedControl(m_pSetup, kNoGroup, LTIntPt(rect.left + 10, rect.top + GetHelpFont()->GetHeight() + 10), LTFALSE);


	// Now create the advanced description field
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_HOST, "AdvancedTextRect");
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

	hTemp = g_pLTClient->FormatString(IDS_HOST_PREFERENCES);
	sprintf(szTemp, "--- %s ---\n", g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);

	pStatic = CreateStaticTextItem(szTemp, LTNULL, LTNULL, nWidth, nHeight, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, LTIntPt(rect.left + 3, rect.top + 3), LTFALSE);

	m_pAdvanced = CreateStaticTextItem("<>", LTNULL, LTNULL, nWidth - 20, nHeight - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());
	AddFixedControl(m_pAdvanced, kNoGroup, LTIntPt(rect.left + 10, rect.top + GetHelpFont()->GetHeight() + 10), LTFALSE);



	// Create the bottom 'back' control
	pCtrl = CreateTextItem(IDS_BACK, FOLDER_CMD_MULTIPLAYER, IDS_HELP_MULTIPLAYER, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl, kBackGroup, m_BackPos);
	UseBack(LTFALSE);

	// Create the Launch button
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"LaunchPos");
	pCtrl = CreateTextItem(IDS_LAUNCH, FOLDER_CMD_LAUNCH, IDS_HELP_LAUNCH, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl, kLinkGroup, pos);


	return LTTRUE;
}


uint32 CFolderHost::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_NAME:
		{
			if (GetCapture())
			{
				char *szText = m_pEdit->GetText();

				if(strlen(szText) < 1)
					m_pEdit->SetText(m_szOldName);

				char *szTemp = strchr(szText,'\\');
				LTBOOL bChange = LTFALSE;
				while (szTemp)
				{
					bChange = LTTRUE;
					*szTemp = '/';
					szTemp = strchr(szText,'\\');
				}

				SetCapture(LTNULL);
				m_pEdit->SetColor(m_hNormalColor, m_hNormalColor, m_hNormalColor);
				m_pEdit->Select(LTFALSE);
				m_pLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(m_szOldName, m_pEdit->GetText());
				SetCapture(m_pEdit);
				m_pEdit->SetColor(m_hSelectedColor, m_hSelectedColor, m_hSelectedColor);
				m_pEdit->Select(LTTRUE);
				m_pLabel->Select(LTFALSE);
			}

			break;
		}

		case CMD_PASS:
		{
			if (GetCapture())
			{
				char *szText = m_pPassEdit->GetText();

				if(strlen(szText) < 1)
					m_pPassEdit->SetText(m_szOldPass);

				SetCapture(LTNULL);
				m_pPassEdit->SetColor(m_hNormalColor, m_hNormalColor, m_hNormalColor);
				m_pPassEdit->Select(LTFALSE);
				m_pPassLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(m_szOldPass, m_pPassEdit->GetText());
				SetCapture(m_pPassEdit);
				m_pPassEdit->SetColor(m_hSelectedColor, m_hSelectedColor, m_hSelectedColor);
				m_pPassEdit->Select(LTTRUE);
				m_pPassLabel->Select(LTFALSE);
			}

			break;
		}

		case CMD_SCPASS:
		{
			if (GetCapture())
			{
				char *szText = m_pSCPassEdit->GetText();

				if(strlen(szText) < 1)
					m_pSCPassEdit->SetText(m_szOldSCPass);

				SetCapture(LTNULL);
				m_pSCPassEdit->SetColor(m_hNormalColor, m_hNormalColor, m_hNormalColor);
				m_pSCPassEdit->Select(LTFALSE);
				m_pSCPassLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(m_szOldSCPass, m_pSCPassEdit->GetText());
				SetCapture(m_pSCPassEdit);
				m_pSCPassEdit->SetColor(m_hSelectedColor, m_hSelectedColor, m_hSelectedColor);
				m_pSCPassEdit->Select(LTTRUE);
				m_pSCPassLabel->Select(LTFALSE);
			}

			break;
		}

		case CMD_PASSCHECKON:
		{
			SetPassCheck(LTFALSE);
			break;
		}

		case CMD_PASSCHECKOFF:
		{
			SetPassCheck(LTTRUE);
			break;
		}

		case FOLDER_CMD_MP_SETUP:
		{
			FocusSetup(LTFALSE);

			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
		}

		case CMD_TYPE_CHANGE:
		{
			UpdateData(LTTRUE);

			switch(m_nType)
			{
				case 0:		m_Options.nGameType = MP_DM;		break;
				case 1:		m_Options.nGameType = MP_TEAMDM;	break;
				case 2:		m_Options.nGameType = MP_HUNT;		break;
				case 3:		m_Options.nGameType = MP_SURVIVOR;	break;
				case 4:		m_Options.nGameType = MP_OVERRUN;	break;
				case 5:		m_Options.nGameType = MP_EVAC;		break;
			}

			CServerOptions *pOpt = g_pServerOptionMgr->GetCurrentCfg();
			(*pOpt) = m_Options;
			g_pServerOptionMgr->SaveCurrentCfg();

			// Redo the setup string
			std::string szSetup;
			BuildSetupString(szSetup);
			m_pSetup->SetString((char *)szSetup.c_str());

			break;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
}

void CFolderHost::Escape()
{
	if(GetCapture() == m_pEdit)
	{
		SetCapture(LTNULL);
		strcpy(m_Options.szServerName, m_szOldName);
		m_pEdit->UpdateData(LTFALSE);
		m_pEdit->SetColor(m_hNormalColor, m_hNormalColor, m_hNormalColor);
		m_pEdit->Select(LTFALSE);
		m_pLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else if(GetCapture() == m_pPassEdit)
	{
		SetCapture(LTNULL);
		strcpy(m_Options.szPassword, m_szOldPass);
		m_pPassEdit->UpdateData(LTFALSE);
		m_pPassEdit->SetColor(m_hNormalColor, m_hNormalColor, m_hNormalColor);
		m_pPassEdit->Select(LTFALSE);
		m_pPassLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else if(GetCapture() == m_pSCPassEdit)
	{
		SetCapture(LTNULL);
		strcpy(m_Options.szSCPassword, m_szOldSCPass);
		m_pSCPassEdit->UpdateData(LTFALSE);
		m_pSCPassEdit->SetColor(m_hNormalColor, m_hNormalColor, m_hNormalColor);
		m_pSCPassEdit->Select(LTFALSE);
		m_pSCPassLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else
	{
		g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
		g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
	}
}

LTBOOL CFolderHost::OnEnter()
{
	if(GetCapture() == m_pEdit)
	{
		char *szText = m_pEdit->GetText();

		if(strlen(szText) < 1)
			m_pEdit->SetText(m_szOldName);
	}
	else if(GetCapture() == m_pPassEdit)
	{
		char *szText = m_pPassEdit->GetText();

		if(strlen(szText) < 1)
			m_pPassEdit->SetText(m_szOldPass);
	}
	else if(GetCapture() == m_pSCPassEdit)
	{
		char *szText = m_pSCPassEdit->GetText();

		if(strlen(szText) < 1)
			m_pSCPassEdit->SetText(m_szOldSCPass);
	}
	return CBaseFolder::OnEnter();
}

LTBOOL	CFolderHost::OnLButtonUp(int x, int y)
{
	if (GetCapture())
	{
		OnEnter();
		return LTTRUE;
	}
	return CBaseFolder::OnLButtonUp(x,y);
}

LTBOOL	CFolderHost::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		OnEnter();
		return LTTRUE;
	}
	return CBaseFolder::OnRButtonUp(x,y);
}


void CFolderHost::OnFocus(LTBOOL bFocus)
{
	FocusSetup(bFocus);

	CBaseFolder::OnFocus(bFocus);

	UseLogo(LTFALSE);
}


void CFolderHost::FocusSetup(LTBOOL bFocus)
{
	if(bFocus)
	{
		m_Options = *(g_pServerOptionMgr->GetCurrentCfg());

		switch(m_Options.nGameType)
		{
			case MP_DM:			m_nType = 0;	break;
			case MP_TEAMDM:		m_nType = 1;	break;
			case MP_HUNT:		m_nType = 2;	break;
			case MP_SURVIVOR:	m_nType = 3;	break;
			case MP_OVERRUN:	m_nType = 4;	break;
			case MP_EVAC:		m_nType = 5;	break;
		}


		std::string szSetup;
		BuildSetupString(szSetup);
		m_pSetup->SetString((char *)szSetup.c_str());

		std::string szAdvanced;
		BuildAdvancedString(szAdvanced);
		m_pAdvanced->SetString((char *)szAdvanced.c_str());

		SetPassCheck(m_Options.bLocked);

		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData(LTTRUE);

		switch(m_nType)
		{
			case 0:		m_Options.nGameType = MP_DM;		break;
			case 1:		m_Options.nGameType = MP_TEAMDM;	break;
			case 2:		m_Options.nGameType = MP_HUNT;		break;
			case 3:		m_Options.nGameType = MP_SURVIVOR;	break;
			case 4:		m_Options.nGameType = MP_OVERRUN;	break;
			case 5:		m_Options.nGameType = MP_EVAC;		break;
		}

		m_Options.bLocked = m_bUsePassword;

		CServerOptions *pOpt = g_pServerOptionMgr->GetCurrentCfg();
		(*pOpt) = m_Options;

		g_pServerOptionMgr->SaveCurrentCfg();
	}
}


void CFolderHost::BuildSetupString(std::string &szSetup)
{
	CServerOptions *pOpt = g_pServerOptionMgr->GetCurrentCfg();
	char szTemp[256];
	szTemp[0] = 0;

	szSetup = "";

	HSTRING hMaxPlayers = g_pLTClient->FormatString(IDS_MAXPLAYERS);
	HSTRING hMaxMarines = g_pLTClient->FormatString(IDS_MAXMARINES);
	HSTRING hMaxAliens = g_pLTClient->FormatString(IDS_MAXALIENS);
	HSTRING hMaxPredators = g_pLTClient->FormatString(IDS_MAXPREDATORS);
	HSTRING hMaxCorporates = g_pLTClient->FormatString(IDS_MAXCORPORATES);
	HSTRING hFragLimit = g_pLTClient->FormatString(IDS_FRAG_LIMIT);
	HSTRING hScoreLimit = g_pLTClient->FormatString(IDS_SCORE_LIMIT);
	HSTRING hTimeLimit = g_pLTClient->FormatString(IDS_TIME_LIMIT);
	HSTRING hRoundLimit = g_pLTClient->FormatString(IDS_ROUND_LIMIT);
	HSTRING hLifecycle = g_pLTClient->FormatString(IDS_LIFECYCLE);
	HSTRING hHunterRace = g_pLTClient->FormatString(IDS_HUNTER_RACE);
	HSTRING hPreyRace = g_pLTClient->FormatString(IDS_PREY_RACE);
	HSTRING hHuntRatio = g_pLTClient->FormatString(IDS_HUNT_RATIO);
	HSTRING hSurvivorRace = g_pLTClient->FormatString(IDS_SURVIVOR_RACE);
	HSTRING hMutateRace = g_pLTClient->FormatString(IDS_MUTATE_RACE);
	HSTRING hDefenderRace = g_pLTClient->FormatString(IDS_DEFENDER_RACE);
	HSTRING hDefenderLives = g_pLTClient->FormatString(IDS_DEFENDER_LIVES);
	HSTRING hAttackerRace = g_pLTClient->FormatString(IDS_ATTACKER_RACE);
	HSTRING hAttackerLives = g_pLTClient->FormatString(IDS_ATTACKER_LIVES);
	HSTRING hRoundTime = g_pLTClient->FormatString(IDS_ROUND_TIME);
	HSTRING hEvacRace = g_pLTClient->FormatString(IDS_EVAC_RACE);
	HSTRING hEvacLives = g_pLTClient->FormatString(IDS_EVAC_LIVES);
	HSTRING hEvacTime = g_pLTClient->FormatString(IDS_EVAC_TIME);

	HSTRING hTeams[MAX_TEAMS];
	hTeams[0] = g_pLTClient->FormatString(IDS_MP_TEAM_ALIENS);
	hTeams[1] = g_pLTClient->FormatString(IDS_MP_TEAM_MARINES);
	hTeams[2] = g_pLTClient->FormatString(IDS_MP_TEAM_PREDATORS);
	hTeams[3] = g_pLTClient->FormatString(IDS_MP_TEAM_CORPORATES);

	HSTRING hOn = g_pLTClient->FormatString(IDS_ON);
	HSTRING hOff = g_pLTClient->FormatString(IDS_OFF);


	switch(pOpt->nGameType)
	{
		case MP_DM:
		{
			sprintf(szTemp, "%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %s",
						g_pLTClient->GetStringData(hMaxPlayers), pOpt->nDM_MaxTotalPlayers,
						g_pLTClient->GetStringData(hMaxMarines), pOpt->nDM_MaxPlayers[Marine],
						g_pLTClient->GetStringData(hMaxAliens), pOpt->nDM_MaxPlayers[Alien],
						g_pLTClient->GetStringData(hMaxPredators), pOpt->nDM_MaxPlayers[Predator],
						g_pLTClient->GetStringData(hMaxCorporates), pOpt->nDM_MaxPlayers[Corporate],
						g_pLTClient->GetStringData(hFragLimit), pOpt->nDM_FragLimit,
						g_pLTClient->GetStringData(hScoreLimit), pOpt->nDM_ScoreLimit,
						g_pLTClient->GetStringData(hTimeLimit), pOpt->nDM_TimeLimit,
						g_pLTClient->GetStringData(hLifecycle), g_pLTClient->GetStringData(pOpt->nDM_Lifecycle ? hOn : hOff));
			break;
		}

		case MP_TEAMDM:
		{
			sprintf(szTemp, "%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %s",
						g_pLTClient->GetStringData(hMaxPlayers), pOpt->nTeam_MaxTotalPlayers,
						g_pLTClient->GetStringData(hMaxMarines), pOpt->nTeam_MaxPlayers[Marine],
						g_pLTClient->GetStringData(hMaxAliens), pOpt->nTeam_MaxPlayers[Alien],
						g_pLTClient->GetStringData(hMaxPredators), pOpt->nTeam_MaxPlayers[Predator],
						g_pLTClient->GetStringData(hMaxCorporates), pOpt->nTeam_MaxPlayers[Corporate],
						g_pLTClient->GetStringData(hFragLimit), pOpt->nTeam_FragLimit,
						g_pLTClient->GetStringData(hScoreLimit), pOpt->nTeam_ScoreLimit,
						g_pLTClient->GetStringData(hTimeLimit), pOpt->nTeam_TimeLimit,
						g_pLTClient->GetStringData(hLifecycle), g_pLTClient->GetStringData(pOpt->nTeam_Lifecycle ? hOn : hOff));
			break;
		}

		case MP_HUNT:
		{
			sprintf(szTemp, "%s: %d\n%s: %s\n%s: %s\n%s: 1:%d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hMaxPlayers), pOpt->nHunt_MaxTotalPlayers,
						g_pLTClient->GetStringData(hHunterRace), g_pLTClient->GetStringData(hTeams[pOpt->nHunt_HunterRace]),
						g_pLTClient->GetStringData(hPreyRace), g_pLTClient->GetStringData(hTeams[pOpt->nHunt_PreyRace]),
						g_pLTClient->GetStringData(hHuntRatio), pOpt->nHunt_Ratio,
						g_pLTClient->GetStringData(hFragLimit), pOpt->nHunt_FragLimit,
						g_pLTClient->GetStringData(hTimeLimit), pOpt->nHunt_TimeLimit);
			break;
		}

		case MP_SURVIVOR:
		{
			sprintf(szTemp, "%s: %d\n%s: %s\n%s: %s\n%s: %d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hMaxPlayers), pOpt->nSurv_MaxTotalPlayers,
						g_pLTClient->GetStringData(hSurvivorRace), g_pLTClient->GetStringData(hTeams[pOpt->nSurv_SurvivorRace]),
						g_pLTClient->GetStringData(hMutateRace), g_pLTClient->GetStringData(hTeams[pOpt->nSurv_MutateRace]),
						g_pLTClient->GetStringData(hScoreLimit), pOpt->nSurv_ScoreLimit,
						g_pLTClient->GetStringData(hRoundLimit), pOpt->nSurv_RoundLimit,
						g_pLTClient->GetStringData(hRoundTime), pOpt->nSurv_TimeLimit);
			break;
		}

		case MP_OVERRUN:
		{
			sprintf(szTemp, "%s: %d\n%s: %s\n%s: %d\n%s: %s\n%s: %d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hMaxPlayers), pOpt->nOver_MaxTotalPlayers,
						g_pLTClient->GetStringData(hDefenderRace), g_pLTClient->GetStringData(hTeams[pOpt->nOver_DefenderRace]),
						g_pLTClient->GetStringData(hDefenderLives), pOpt->nOver_DefenderLives,
						g_pLTClient->GetStringData(hAttackerRace), g_pLTClient->GetStringData(hTeams[pOpt->nOver_AttackerRace]),
						g_pLTClient->GetStringData(hAttackerLives), pOpt->nOver_AttackerLives,
						g_pLTClient->GetStringData(hRoundLimit), pOpt->nOver_RoundLimit,
						g_pLTClient->GetStringData(hRoundTime), pOpt->nOver_TimeLimit);
			break;
		}

		case MP_EVAC:
		{
			sprintf(szTemp, "%s: %d\n%s: %s\n%s: %d\n%s: %s\n%s: %d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hMaxPlayers), pOpt->nEvac_MaxTotalPlayers,
						g_pLTClient->GetStringData(hEvacRace), g_pLTClient->GetStringData(hTeams[pOpt->nEvac_DefenderRace]),
						g_pLTClient->GetStringData(hEvacLives), pOpt->nEvac_DefenderLives,
						g_pLTClient->GetStringData(hAttackerRace), g_pLTClient->GetStringData(hTeams[pOpt->nEvac_AttackerRace]),
						g_pLTClient->GetStringData(hAttackerLives), pOpt->nEvac_AttackerLives,
						g_pLTClient->GetStringData(hRoundLimit), pOpt->nEvac_RoundLimit,
						g_pLTClient->GetStringData(hRoundTime), pOpt->nEvac_TimeLimit);
			break;
		}
	}


	szSetup += szTemp;


	g_pLTClient->FreeString(hMaxPlayers);
	g_pLTClient->FreeString(hMaxMarines);
	g_pLTClient->FreeString(hMaxAliens);
	g_pLTClient->FreeString(hMaxPredators);
	g_pLTClient->FreeString(hMaxCorporates);
	g_pLTClient->FreeString(hFragLimit);
	g_pLTClient->FreeString(hScoreLimit);
	g_pLTClient->FreeString(hTimeLimit);
	g_pLTClient->FreeString(hRoundLimit);
	g_pLTClient->FreeString(hLifecycle);
	g_pLTClient->FreeString(hHunterRace);
	g_pLTClient->FreeString(hPreyRace);
	g_pLTClient->FreeString(hHuntRatio);
	g_pLTClient->FreeString(hSurvivorRace);
	g_pLTClient->FreeString(hMutateRace);
	g_pLTClient->FreeString(hDefenderRace);
	g_pLTClient->FreeString(hDefenderLives);
	g_pLTClient->FreeString(hAttackerRace);
	g_pLTClient->FreeString(hAttackerLives);
	g_pLTClient->FreeString(hRoundTime);
	g_pLTClient->FreeString(hEvacRace);
	g_pLTClient->FreeString(hEvacLives);
	g_pLTClient->FreeString(hEvacTime);

	g_pLTClient->FreeString(hTeams[0]);
	g_pLTClient->FreeString(hTeams[1]);
	g_pLTClient->FreeString(hTeams[2]);
	g_pLTClient->FreeString(hTeams[3]);

	g_pLTClient->FreeString(hOn);
	g_pLTClient->FreeString(hOff);
}


void CFolderHost::BuildAdvancedString(std::string &szAdvanced)
{
	CServerOptions *pOpt = g_pServerOptionMgr->GetCurrentCfg();

	HSTRING hTemp = LTNULL, hTemp2 = LTNULL;
	char szTemp[128];
	char szOn[32];
	char szOff[32];

	hTemp = g_pLTClient->FormatString(IDS_ON);
	SAFE_STRCPY(szOn, g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OFF);
	SAFE_STRCPY(szOff, g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);


	// Build prefs string
	std::string pref = "";
/*	hTemp = g_pLTClient->FormatString(IDS_OPT_RUNSPEED);
	sprintf(szTemp,"%s: %d%%\n",g_pLTClient->GetStringData(hTemp),(int)(pOpt->fSpeed * 100.0f));
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_RESPAWN);
	sprintf(szTemp,"%s: %d%%\n",g_pLTClient->GetStringData(hTemp),(int)(pOpt->fRespawn * 100.0f));
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);
*/
	hTemp = g_pLTClient->FormatString(IDS_OPT_PLAYER_DAMAGE);
	sprintf(szTemp,"%s: %d%%\n",g_pLTClient->GetStringData(hTemp),(int)(pOpt->fDamage * 100.0f));
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_LOC_DAMAGE);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), pOpt->bLocationDamage ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_FRIENDLY_FIRE);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), pOpt->bFriendlyFire ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_FRIENDLY_NAMES);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), pOpt->bFriendlyNames ? szOff : szOn);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_PRED_MASK_LOSS);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), pOpt->bMaskLoss ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_CLASS_WEAPONS);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), pOpt->bClassWeapons ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_EXOSUIT);
	sprintf(szTemp,"%s: %d\n",g_pLTClient->GetStringData(hTemp), pOpt->nExosuit);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_QUEEN);
	sprintf(szTemp,"%s: %d\n",g_pLTClient->GetStringData(hTemp), pOpt->nQueenMolt);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	// Finish prefs section
	if (pref.size())
	{
		szAdvanced = pref;
//		szAdvanced += "\n";
	}


	// Build scoring string
	LTBOOL bCustom = LTFALSE;
	for (uint8 c = 0; c < g_pServerOptionMgr->GetNumScoringClasses(); c++)
	{
		// Check if the scoring value has been customized.
		if( pOpt->GetScoring( g_pServerOptionMgr->GetScoringClass( c )) != CServerOptions::eDefaultScoringValue )
		{
			bCustom = LTTRUE;
			break;
		}
	}

	hTemp = g_pLTClient->FormatString(IDS_HOST_SCORING);
	hTemp2 = bCustom ? g_pLTClient->FormatString(IDS_HOST_CUSTOM) : g_pLTClient->FormatString(IDS_HOST_DEFAULT);
	sprintf(szTemp,"%s: %s\n", g_pLTClient->GetStringData(hTemp), g_pLTClient->GetStringData(hTemp2));
	szAdvanced += szTemp;
	g_pLTClient->FreeString(hTemp);
	g_pLTClient->FreeString(hTemp2);


	// Just fill it in with a 'default' string if the options options aren't listed...
	if(!pref.size() && !bCustom)
	{
		hTemp = g_pLTClient->FormatString(IDS_OPT_DEFAULT);
		sprintf(szTemp,"   <%s>",g_pLTClient->GetStringData(hTemp));
		szAdvanced += szTemp;
		g_pLTClient->FreeString(hTemp);
	}
}


void CFolderHost::SetPassCheck(LTBOOL bOn)
{
	m_bUsePassword = bOn;
	m_pPassCheck->Destroy();

	if(m_bUsePassword)
	{
		m_pPassCheck->Create(g_pLTClient, "interface\\menus\\checkon.pcx", "interface\\menus\\checkon_h.pcx", "interface\\menus\\checkon_d.pcx", this, CMD_PASSCHECKON);
		m_pPassCheck->SetHelpID(IDS_SERVER_UNLOCK);
	}
	else
	{
		m_pPassCheck->Create(g_pLTClient, "interface\\menus\\checkoff.pcx", "interface\\menus\\checkoff_h.pcx", "interface\\menus\\checkoff_d.pcx", this, CMD_PASSCHECKOFF);
		m_pPassCheck->SetHelpID(IDS_SERVER_LOCK);
	}

	ForceMouseUpdate();
}

