// FolderJoinInfo.cpp: implementation of the CFolderJoinInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderJoinInfo.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "ServerOptionMgr.h"


namespace
{
	int kNameWidth = 128;
	int kFragWidth = 64;
	int kScoreWidth = 64;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CFolderJoinInfo::CFolderJoinInfo()
{
	m_pGameSpyClientMgr = LTNULL;
	m_pServer = LTNULL;

	m_pGeneral = LTNULL;
	m_pSetup = LTNULL;
	m_pAdvanced = LTNULL;
}


CFolderJoinInfo::~CFolderJoinInfo()
{

}


// Build the folder
LTBOOL CFolderJoinInfo::Build()
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

	// Setup the title of this folder
	CreateTitle(IDS_TITLE_JOIN_INFO);
	UseLogo(LTTRUE);


	// Add the links to other menus...
	AddLink(IDS_SERVER_LIST, FOLDER_CMD_MP_JOIN, IDS_HELP_SERVER_LIST);
	AddLink(IDS_SERVER_INFO, FOLDER_CMD_MP_INFO, IDS_HELP_SERVER_INFO)->Enable(LTFALSE);
	AddLink(IDS_PLAYER, FOLDER_CMD_MP_PLAYER_JOIN, IDS_HELP_PLAYER);



	// -----------------------------------------------------------------
	// Create the frames for this folder

	// Now create the setup description field
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_JOIN_INFO, "GeneralTextRect");
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

	hTemp = g_pLTClient->FormatString(IDS_GENERAL_SERVER_INFO);
	sprintf(szTemp, "--- %s ---\n", g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);

	pStatic = CreateStaticTextItem(szTemp, LTNULL, LTNULL, nWidth, nHeight, LTTRUE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, LTIntPt(rect.left + 3, rect.top + 3), LTFALSE);

	m_pGeneral = CreateStaticTextItem("<>", LTNULL, LTNULL, nWidth - 20, nHeight - GetHelpFont()->GetHeight() - 15, LTTRUE, GetHelpFont());
	AddFixedControl(m_pGeneral, kNoGroup, LTIntPt(rect.left + 10, rect.top + GetHelpFont()->GetHeight() + 10), LTFALSE);


	// Now create the setup description field
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_JOIN_INFO, "SetupTextRect");
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
	rect = g_pLayoutMgr->GetFolderCustomRect(FOLDER_ID_JOIN_INFO, "AdvancedTextRect");
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



	// Create the Join button
	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"JoinPos");
	pCtrl = CreateTextItem(IDS_JOIN, FOLDER_CMD_MP_JOIN_GAME, IDS_HELP_JOIN, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl,kLinkGroup,pos);


	return LTTRUE;
}


void CFolderJoinInfo::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		if (m_pServer)
		{
			std::string szGeneral;
			BuildGeneralString(szGeneral);
			m_pGeneral->SetString((char *)szGeneral.c_str());

			std::string szSetup;
			BuildSetupString(szSetup);
			m_pSetup->SetString((char *)szSetup.c_str());

			std::string szAdvanced;
			BuildAdvancedString(szAdvanced);
			m_pAdvanced->SetString((char *)szAdvanced.c_str());
		}

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		m_pGameSpyClientMgr = LTNULL;
		m_pServer = LTNULL;
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);
}


void CFolderJoinInfo::Escape()
{
	if(GetCapture() == m_pPWPassword)
	{
		HidePasswordDlg();
		return;
	}

	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
}


void CFolderJoinInfo::BuildGeneralString(std::string &szGeneral)
{
	HSTRING hTemp = LTNULL, hTemp2 = LTNULL;
	char szTemp[128];

	szGeneral = "";

	// Name of the server
	hTemp = g_pLTClient->FormatString(IDS_SERVER);
	sprintf(szTemp, "%s: %s\n", g_pLTClient->GetStringData(hTemp), m_pServer->GetStringValue("hostname"));
	szGeneral += szTemp;
	g_pLTClient->FreeString(hTemp);

	// Server IP
	hTemp = g_pLTClient->FormatString(IDS_IP);
	sprintf(szTemp, "%s: %s:%d\n", g_pLTClient->GetStringData(hTemp), m_pServer->GetAddress(), m_pServer->GetIntValue("hostport", 0));
	szGeneral += szTemp;
	g_pLTClient->FreeString(hTemp);

	// Game version
	hTemp = g_pLTClient->FormatString(IDS_INFO_VERSION);
	sprintf(szTemp, "%s: %s\n\n", g_pLTClient->GetStringData(hTemp), m_pServer->GetStringValue("gamever"));
	szGeneral += szTemp;
	g_pLTClient->FreeString(hTemp);



	// Game type
	hTemp = g_pLTClient->FormatString(IDS_GAME_TYPE);
	sprintf(szTemp, "%s: %s\n", g_pLTClient->GetStringData(hTemp), m_pServer->GetStringValue("gametype"));
	szGeneral += szTemp;
	g_pLTClient->FreeString(hTemp);

	// Current level
	hTemp = g_pLTClient->FormatString(IDS_LEVEL);
	sprintf(szTemp, "%s: %s\n", g_pLTClient->GetStringData(hTemp), m_pServer->GetStringValue("mapname"));
	szGeneral += szTemp;
	g_pLTClient->FreeString(hTemp);

	// Game Bandwidth
	int nBandwidth = m_pServer->GetIntValue("bandwidth", 0);
	hTemp = g_pLTClient->FormatString(IDS_SERVER_BANDWIDTH);
	// Calculate the bandwidth
	if(nBandwidth <= 15999)
		hTemp2 = g_pLTClient->FormatString(IDS_CONNECT_VSLOW);
	else if (nBandwidth <= 31999)
		hTemp2 = g_pLTClient->FormatString(IDS_CONNECT_SLOW);
	else if (nBandwidth <= 999999)
		hTemp2 = g_pLTClient->FormatString(IDS_CONNECT_MEDIUM);
	else if (nBandwidth <= 9999999)
		hTemp2 = g_pLTClient->FormatString(IDS_CONNECT_FAST);
	else
		hTemp2 = g_pLTClient->FormatString(IDS_CONNECT_VFAST);
	sprintf(szTemp, "%s: %s\n\n", g_pLTClient->GetStringData(hTemp), g_pLTClient->GetStringData(hTemp2));
	szGeneral += szTemp;
	g_pLTClient->FreeString(hTemp);
	g_pLTClient->FreeString(hTemp2);

	// Players
	hTemp = g_pLTClient->FormatString(IDS_PLAYERS);
	sprintf(szTemp, "%s: (%d/%d)\n", g_pLTClient->GetStringData(hTemp), m_pServer->GetIntValue("numplayers", 0), m_pServer->GetIntValue("maxplayers", 0));
	szGeneral += szTemp;
	g_pLTClient->FreeString(hTemp);


	// Setup the names of the races
	HSTRING hTeams[MAX_TEAMS];
	hTeams[0] = g_pLTClient->FormatString(IDS_ALIEN);
	hTeams[1] = g_pLTClient->FormatString(IDS_MARINE);
	hTeams[2] = g_pLTClient->FormatString(IDS_PREDATOR);
	hTeams[3] = g_pLTClient->FormatString(IDS_CORPORATE);


	// Add each player...
	m_pServer->UpdatePlayers();
	m_pServer->SortPlayersByName();

	CGameSpyPlayer *pPlayer = m_pServer->GetFirstPlayer();

	while(pPlayer)
	{
		sprintf(szTemp, "  %s (%s)\n", pPlayer->GetName(), g_pLTClient->GetStringData(hTeams[pPlayer->GetRace()]));
		szGeneral += szTemp;

//		pPlayer->GetFrags()
//		pPlayer->GetScore()

		// Get the next player
		pPlayer = m_pServer->GetNextPlayer();
	}


	g_pLTClient->FreeString(hTeams[0]);
	g_pLTClient->FreeString(hTeams[1]);
	g_pLTClient->FreeString(hTeams[2]);
	g_pLTClient->FreeString(hTeams[3]);
}


void CFolderJoinInfo::BuildSetupString(std::string &szSetup)
{
	GameType gt;
	gt.SetTypeFromDesc(m_pServer->GetStringValue("gametype"));

	char szTemp[256];
	szTemp[0] = 0;

	szSetup = "";

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


	switch(gt.Get())
	{
		case MP_DM:
		case MP_TEAMDM:
		{
			sprintf(szTemp, "%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %d\n%s: %s",
						g_pLTClient->GetStringData(hMaxMarines), m_pServer->GetIntValue("maxm", 0),
						g_pLTClient->GetStringData(hMaxAliens), m_pServer->GetIntValue("maxa", 0),
						g_pLTClient->GetStringData(hMaxPredators), m_pServer->GetIntValue("maxp", 0),
						g_pLTClient->GetStringData(hMaxCorporates), m_pServer->GetIntValue("maxc", 0),
						g_pLTClient->GetStringData(hFragLimit), m_pServer->GetIntValue("frags", 0),
						g_pLTClient->GetStringData(hScoreLimit), m_pServer->GetIntValue("score", 0),
						g_pLTClient->GetStringData(hTimeLimit), m_pServer->GetIntValue("time", 0) / 60,
						g_pLTClient->GetStringData(hLifecycle), g_pLTClient->GetStringData(m_pServer->GetIntValue("lc", 0) ? hOn : hOff));
			break;
		}

		case MP_HUNT:
		{
			sprintf(szTemp, "%s: %s\n%s: %s\n%s: 1:%d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hHunterRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("hrace", 0)]),
						g_pLTClient->GetStringData(hPreyRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("prace", 0)]),
						g_pLTClient->GetStringData(hHuntRatio), m_pServer->GetIntValue("ratio", 0),
						g_pLTClient->GetStringData(hFragLimit), m_pServer->GetIntValue("frags", 0),
						g_pLTClient->GetStringData(hTimeLimit), m_pServer->GetIntValue("time", 0) / 60);
			break;
		}

		case MP_SURVIVOR:
		{
			sprintf(szTemp, "%s: %s\n%s: %s\n%s: %d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hSurvivorRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("srace", 0)]),
						g_pLTClient->GetStringData(hMutateRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("mrace", 0)]),
						g_pLTClient->GetStringData(hScoreLimit), m_pServer->GetIntValue("score", 0),
						g_pLTClient->GetStringData(hRoundLimit), m_pServer->GetIntValue("rounds", 0),
						g_pLTClient->GetStringData(hRoundTime), m_pServer->GetIntValue("time", 0) / 60);
			break;
		}

		case MP_OVERRUN:
		{
			sprintf(szTemp, "%s: %s\n%s: %d\n%s: %s\n%s: %d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hDefenderRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("drace", 0)]),
						g_pLTClient->GetStringData(hDefenderLives), m_pServer->GetIntValue("dlive", 0),
						g_pLTClient->GetStringData(hAttackerRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("arace", 0)]),
						g_pLTClient->GetStringData(hAttackerLives), m_pServer->GetIntValue("alive", 0),
						g_pLTClient->GetStringData(hRoundLimit), m_pServer->GetIntValue("rounds", 0),
						g_pLTClient->GetStringData(hRoundTime), m_pServer->GetIntValue("time", 0) / 60);
			break;
		}

		case MP_EVAC:
		{
			sprintf(szTemp, "%s: %s\n%s: %d\n%s: %s\n%s: %d\n%s: %d\n%s: %d",
						g_pLTClient->GetStringData(hEvacRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("drace", 0)]),
						g_pLTClient->GetStringData(hEvacLives), m_pServer->GetIntValue("dlive", 0),
						g_pLTClient->GetStringData(hAttackerRace), g_pLTClient->GetStringData(hTeams[m_pServer->GetIntValue("arace", 0)]),
						g_pLTClient->GetStringData(hAttackerLives), m_pServer->GetIntValue("alive", 0),
						g_pLTClient->GetStringData(hRoundLimit), m_pServer->GetIntValue("rounds", 0),
						g_pLTClient->GetStringData(hRoundTime), m_pServer->GetIntValue("time", 0) / 60);
			break;
		}
	}


	szSetup += szTemp;


	g_pLTClient->FreeString(hMaxMarines);
	g_pLTClient->FreeString(hMaxAliens);
	g_pLTClient->FreeString(hMaxPredators);
	g_pLTClient->FreeString(hMaxCorporates);
	g_pLTClient->FreeString(hFragLimit);
	g_pLTClient->FreeString(hScoreLimit);
	g_pLTClient->FreeString(hTimeLimit);
	g_pLTClient->FreeString(hRoundLimit);
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


void CFolderJoinInfo::BuildAdvancedString(std::string &szAdvanced)
{
	HSTRING hTemp = LTNULL, hTemp2 = LTNULL;
	char szTemp[128];
	char szOn[32];
	char szOff[32];

	hTemp = g_pLTClient->FormatString(IDS_ON);
	SAFE_STRCPY(szOn,g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OFF);
	SAFE_STRCPY(szOff,g_pLTClient->GetStringData(hTemp));
	g_pLTClient->FreeString(hTemp);


	// Build prefs string
	std::string pref = "";

	hTemp = g_pLTClient->FormatString(IDS_OPT_PLAYER_DAMAGE);
	sprintf(szTemp,"%s: %d%%\n",g_pLTClient->GetStringData(hTemp),m_pServer->GetIntValue("damage"));
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_LOC_DAMAGE);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), (LTBOOL)m_pServer->GetIntValue("hitloc") ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_FRIENDLY_FIRE);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), (LTBOOL)m_pServer->GetIntValue("ff") ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_FRIENDLY_NAMES);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), (LTBOOL)m_pServer->GetIntValue("fn") ? szOff : szOn);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_PRED_MASK_LOSS);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), (LTBOOL)m_pServer->GetIntValue("mask") ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_CLASS_WEAPONS);
	sprintf(szTemp,"%s: %s\n",g_pLTClient->GetStringData(hTemp), (LTBOOL)m_pServer->GetIntValue("class") ? szOn : szOff);
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_EXOSUIT);
	sprintf(szTemp,"%s: %d\n",g_pLTClient->GetStringData(hTemp), m_pServer->GetIntValue("exosuit"));
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	hTemp = g_pLTClient->FormatString(IDS_OPT_QUEEN);
	sprintf(szTemp,"%s: %d\n",g_pLTClient->GetStringData(hTemp),m_pServer->GetIntValue("queen"));
	pref += szTemp;
	g_pLTClient->FreeString(hTemp);

	// Finish prefs section
	if (pref.size())
	{
		szAdvanced = pref;
//		szAdvanced += "\n";
	}


	// Build scoring string
	LTBOOL bCustom = m_pServer->GetIntValue("cscore");

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

