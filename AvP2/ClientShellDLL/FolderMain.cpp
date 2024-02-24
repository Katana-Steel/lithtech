// FolderMain.cpp: implementation of the CFolderMain class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMain.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "FolderMulti.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
#include "ProfileMgr.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
	enum LocalCommands
	{
		CMD_GS_ARCADE = FOLDER_CMD_CUSTOM,
	};

	void GSArcadeCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderMain *pThisFolder = (CFolderMain *)pData;

		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(CMD_GS_ARCADE,0,0);
	}

	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderMain *pThisFolder = (CFolderMain *)pData;

		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(FOLDER_CMD_EXIT,0,0);
	}

	char szSelect[128];
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMain::CFolderMain()
{
    m_BuildVersion = LTNULL;
    m_pResume = LTNULL;
    m_pQuit = LTNULL;
}

CFolderMain::~CFolderMain()
{
    if (g_pLTClient && m_BuildVersion)
	{
        g_pLTClient->FreeString(m_BuildVersion);
	}

}


// Build the folder
LTBOOL CFolderMain::Build()
{
	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	g_pLayoutMgr->GetEscapeSound(szSelect,sizeof(szSelect));

	CreateTitle(IDS_TITLE_MAINMENU);

#ifdef _DEMO
//	AddLink(IDS_SINGLEPLAYER, FOLDER_CMD_SINGLE_PLAYER, IDS_HELP_SINGLEPLAYER);
//	m_pResume = AddLink(IDS_CONTINUE_GAME, FOLDER_CMD_CONTINUE_GAME, IDS_HELP_CONTINUE_GAME);
//	AddLink(IDS_MULTIPLAYER, FOLDER_CMD_MULTIPLAYER, IDS_HELP_MULTIPLAYER)->Enable(LTFALSE);
//	AddLink(IDS_MULTIPLAYER_LAN, FOLDER_CMD_MULTIPLAYER_LAN, IDS_HELP_MULTIPLAYER_LAN)->Enable(LTFALSE);
//	AddLink(IDS_OPTIONS,FOLDER_CMD_OPTIONS,	IDS_HELP_OPTIONS);
//	AddLink(IDS_PROFILE, FOLDER_CMD_PROFILE, IDS_HELP_PROFILE)->Enable(LTFALSE);

	AddLink(IDS_SINGLEPLAYER, FOLDER_CMD_SINGLE_PLAYER, IDS_HELP_SINGLEPLAYER)->Enable(LTFALSE);
	m_pResume = AddLink(IDS_CONTINUE_GAME, FOLDER_CMD_CONTINUE_GAME, IDS_HELP_CONTINUE_GAME);
	AddLink(IDS_MULTIPLAYER, FOLDER_CMD_MULTIPLAYER, IDS_HELP_MULTIPLAYER)->Enable(LTFALSE);
	AddLink(IDS_MULTIPLAYER_LAN, FOLDER_CMD_MULTIPLAYER_LAN, IDS_HELP_MULTIPLAYER_LAN);
	AddLink(IDS_OPTIONS,FOLDER_CMD_OPTIONS,	IDS_HELP_OPTIONS);
	AddLink(IDS_PROFILE, FOLDER_CMD_PROFILE, IDS_HELP_PROFILE)->Enable(LTFALSE);
#else
	AddLink(IDS_SINGLEPLAYER, FOLDER_CMD_SINGLE_PLAYER, IDS_HELP_SINGLEPLAYER);
	m_pResume = AddLink(IDS_CONTINUE_GAME, FOLDER_CMD_CONTINUE_GAME, IDS_HELP_CONTINUE_GAME);
	AddLink(IDS_MULTIPLAYER, FOLDER_CMD_MULTIPLAYER, IDS_HELP_MULTIPLAYER);
	AddLink(IDS_MULTIPLAYER_LAN, FOLDER_CMD_MULTIPLAYER_LAN, IDS_HELP_MULTIPLAYER_LAN);
	AddLink(IDS_OPTIONS,FOLDER_CMD_OPTIONS,	IDS_HELP_OPTIONS);
	AddLink(IDS_PROFILE, FOLDER_CMD_PROFILE, IDS_HELP_PROFILE);
#endif

	m_pQuit = CreateTextItem(IDS_EXIT,FOLDER_CMD_QUIT, IDS_HELP_EXIT, LTFALSE, GetLargeFont());
	AddFixedControl(m_pQuit,kBackGroup,m_BackPos);
	UseBack(LTFALSE);


	HSTRING hTemp = g_pLTClient->FormatString(IDS_VERSION);
#ifdef _DEMO
	m_BuildVersion = g_pLTClient->FormatString(IDS_DEMO_BUILDINFO, g_pLTClient->GetStringData(hTemp));
#else
	m_BuildVersion = g_pLTClient->FormatString(IDS_BUILDINFO, g_pLTClient->GetStringData(hTemp));
#endif
	g_pLTClient->FreeString(hTemp);

	m_BuildPos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"VersionPos");

    UseBack(LTFALSE);

	return LTTRUE;

}

void CFolderMain::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pResume->RemoveAll();
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();

		m_pResume->AddString(IDS_CONTINUE_GAME);
		m_pResume->SetHelpID(IDS_HELP_CONTINUE_GAME);
		m_pResume->CLTGUICtrl::Create(FOLDER_CMD_CONTINUE_GAME,0,0);
		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32] = "Continue";
		char strIniName[128];
		sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, strIniName);

		// get the file name from the ini string...
		char strFileName[128];
		char* pWorldName = strtok(strSaveGameSetting,"|");
        char* pFileName = strtok(LTNULL,"|");

		_snprintf (strFileName, 127, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), pFileName);

#ifdef _DEMO
		m_pResume->Enable(LTFALSE);
#else
		m_pResume->Enable(CWinUtil::FileExist(strFileName));
#endif


		// Determine if we need to use the GameSpy Arcade setup...
		HCONSOLEVAR	m_hVar_GSA = g_pLTClient->GetConsoleVar("GSA");

		if(m_hVar_GSA)
		{
			if(g_pLTClient->GetVarValueFloat(m_hVar_GSA))
			{
				// Reset the variable so we don't get in here again except
				// from the command line.
				g_pLTClient->RunConsoleString("GSA 0");


				HCONSOLEVAR	m_hVar_GSA_IP = g_pLTClient->GetConsoleVar("GSA_IP");
				HCONSOLEVAR	m_hVar_GSA_Port = g_pLTClient->GetConsoleVar("GSA_Port");
				HCONSOLEVAR	m_hVar_GSA_PW = g_pLTClient->GetConsoleVar("GSA_PW");
				HCONSOLEVAR	m_hVar_GSA_Name = g_pLTClient->GetConsoleVar("GSA_Name");

				if(m_hVar_GSA_IP)
					m_pFolderMgr->SetGSA_IP(g_pLTClient->GetVarValueString(m_hVar_GSA_IP));

				if(m_hVar_GSA_Port)
					m_pFolderMgr->SetGSA_Port(g_pLTClient->GetVarValueString(m_hVar_GSA_Port));

				if(m_hVar_GSA_PW)
					m_pFolderMgr->SetGSA_PW(g_pLTClient->GetVarValueString(m_hVar_GSA_PW));

				if(m_hVar_GSA_Name)
					m_pFolderMgr->SetGSA_Name(g_pLTClient->GetVarValueString(m_hVar_GSA_Name));


				// Make sure we want to use the GameSpy Arcade junk...
				HSTRING hString = g_pLTClient->FormatString(IDS_GAMESPYARCADE);
				g_pInterfaceMgr->ShowMessageBox(hString, LTMB_YESNO, GSArcadeCallBack, this);
				g_pLTClient->FreeString(hString);
			}
		}
	}

	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderMain::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_SINGLE_PLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_SINGLE);
			break;
		}
	case FOLDER_CMD_MULTIPLAYER:
		{ 
			CFolderMulti *pMulti = (CFolderMulti*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_MULTI);
			pMulti->SetLAN(LTFALSE);

			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTI);
			break;
		}
	case FOLDER_CMD_MULTIPLAYER_LAN:
		{ 
			CFolderMulti *pMulti = (CFolderMulti*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_MULTI);
			pMulti->SetLAN(LTTRUE);

			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTI);
			break;
		}
	case FOLDER_CMD_OPTIONS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_OPTIONS);
			break;
		}
	case FOLDER_CMD_PROFILE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PROFILE);
			break;
		}
	case FOLDER_CMD_QUIT:
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_SUREWANTQUIT);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,QuitCallBack,this);
			g_pLTClient->FreeString(hString);
			break;
		}
	case FOLDER_CMD_EXIT:
		{
//#ifdef _DEMO
//			g_pInterfaceMgr->ShowDemoScreens(LTTRUE);
//#else
            g_pLTClient->Shutdown();
//#endif
			break;
		}
	case FOLDER_CMD_CONTINUE_GAME:
		{
			// Read the continue game string.
			// This string is written in CGameClientShell::SaveGame.
			char strSaveGameSetting[256];
			memset (strSaveGameSetting, 0, 256);
			char strKey[32] = "Continue";
			char strIniName[128];
			char strFileName[128];

			_snprintf (strIniName, 127, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, strIniName);

			char* pWorldName = strtok(strSaveGameSetting,"|");
            char* pFileName = strtok(LTNULL,"|");
			char* pSpecies = strtok(NULL,"|");
			char* pDifficulty = strtok(NULL,"|");

			Species eSpecies = Marine;
			if( pSpecies && strlen(pSpecies) )
				eSpecies = Species(atoi(pSpecies));
			g_pProfileMgr->SetSpecies(eSpecies);

			int nDifficulty = DIFFICULTY_PC_MEDIUM;
			if( pDifficulty && strlen(pDifficulty) )
				nDifficulty = atoi(pDifficulty);
			g_pGameClientShell->GetGameType()->SetDifficulty(nDifficulty);


			_snprintf (strFileName, 127, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), pFileName);

			if (g_pGameClientShell->LoadGame(pWorldName, strFileName))
			{
				g_pInterfaceMgr->ChangeState(GS_PLAYING);
				return 1;
			}

			break;
		}
	case CMD_GS_ARCADE:
		{
			CFolderMulti *pMulti = (CFolderMulti*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_MULTI);
			pMulti->SetLAN(LTFALSE);

			m_pFolderMgr->SetGSA(LTTRUE);
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MULTI);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Folder specific rendering
LTBOOL   CFolderMain::Render(HSURFACE hDestSurf)
{
	if (CBaseFolder::Render(hDestSurf))
	{
		if( CLTGUIFont* pFont = GetHelpFont() )
		{
			pFont->Draw(m_BuildVersion, hDestSurf, m_BuildPos.x + g_pInterfaceResMgr->GetXOffset(), 
				m_BuildPos.y + g_pInterfaceResMgr->GetYOffset(), LTF_JUSTIFY_RIGHT, kGray);
		}

        return LTTRUE;
	}

    return LTFALSE;

}

void CFolderMain::Escape()
{
    HSTRING hString = g_pLTClient->FormatString(IDS_SUREWANTQUIT);
	g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,QuitCallBack,this);
	g_pLTClient->FreeString(hString);
}

char* CFolderMain::GetSelectSound()
{
	if (GetSelectedControl() == m_pQuit)
		return szSelect;
	return m_sDefaultSelectSound;
}
