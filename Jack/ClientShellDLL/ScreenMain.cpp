// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMain.cpp
//
// PURPOSE : Top level interface screen
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenMain.h"
#include "ScreenMgr.h"
#include "LayoutMgr.h"
#include "ScreenCommands.h"
#include "WinUtil.h"
#include "direct.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
#include "ClientSaveLoadMgr.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"
#include "ResShared.h"
#include "ScreenMulti.h"

namespace
{
	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenMain *pThisScreen = (CScreenMain *)pData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_EXIT,0,0);
	};

}

extern bool g_bLAN;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMain::CScreenMain()
{
    m_pResume = LTNULL;

}

CScreenMain::~CScreenMain()
{
}


// Build the screen
LTBOOL CScreenMain::Build()
{

	char szTmp[1024];
	FormatString(IDS_SINGLEPLAYER,szTmp,sizeof(szTmp));

#if !defined(_MPDEMO)
	CLTGUITextCtrl* pSP = AddTextItem(szTmp, CMD_SINGLE_PLAYER, IDS_HELP_SINGLEPLAYER);
	m_pResume = AddTextItem(IDS_CONTINUE_GAME, CMD_CONTINUE_GAME, IDS_HELP_CONTINUE_GAME);
	m_pResume->Enable(LTFALSE);
#endif // !defined(_MPDEMO)

#if !defined(_SPDEMO)
	CLTGUITextCtrl* pMP = AddTextItem(IDS_MULTIPLAYER, CMD_MULTI_PLAYER, IDS_HELP_MULTIPLAYER);
#endif // !defined(_SPDEMO)

	CLTGUITextCtrl* pOptions = AddTextItem(IDS_OPTIONS, CMD_OPTIONS, IDS_HELP_OPTIONS);
	CLTGUITextCtrl* pCtrl = AddTextItem(IDS_PROFILE, CMD_PROFILE, IDS_HELP_PROFILE);

	AddTextItem(IDS_EXIT, CMD_QUIT, IDS_HELP_EXIT, s_BackPos);

	// Put the version number at versionpos, but don't let it clip off the edge.
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"VersionPos");
	uint8 nFont = g_pLayoutMgr->GetHelpFont();
	
	char szVersion[256];
	g_pVersionMgr->GetDisplayVersion( szVersion, ARRAY_LEN( szVersion ));

// Don't show mod for demo's.
#ifndef _DEMO
	strcat( szVersion,  " - " );
	strcat( szVersion, g_pClientMultiplayerMgr->GetModName());
#endif // _DEMO
	
	pCtrl= AddTextItem( szVersion,LTNULL,LTNULL,pos,LTTRUE,nFont);
	pCtrl->SetFont(LTNULL,g_pLayoutMgr->GetHelpSize());
	uint16 nUnScaledWidth = pCtrl->GetBaseWidth();
	if( pos.x + nUnScaledWidth >= 640 )
	{
		pos.x = 640 - nUnScaledWidth;
	}
	int nUnScaledHeight = pCtrl->GetBaseHeight();
	if( pos.y + nUnScaledHeight > 480 )
	{
		pos.y = 480 - nUnScaledHeight;
	}
	pCtrl->SetBasePos(pos);

	//Build multiplayer screen to force construction of DM mission file
	CScreenMulti *pMulti = (CScreenMulti *)m_pScreenMgr->GetScreenFromID(SCREEN_ID_MULTI);
	pMulti->Build();

	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);

	return LTTRUE;

}

void CScreenMain::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// Make sure we're disconnected from server.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect( true );
			g_pClientMultiplayerMgr->ForceDisconnect();
		}

		//since can get here from various points in the game, let's make sure that we're not fading in or out...
		g_pInterfaceMgr->AbortScreenFade();


		SetSelection(-1);

		// Always assume sp save/load when in the main screen.  This is so the 
		// "continue game" and "quick load" assumes sp.
		g_pGameClientShell->SetGameType( eGameTypeSingle );


		// Initialize to the sp mission bute.
		if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
			return;
  		}

		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );
		g_pClientMultiplayerMgr->SetupServerSinglePlayer( );

		if( m_pResume )
			m_pResume->Enable( g_pClientSaveLoadMgr->CanContinueGame() );

		// We don't go thru the main screen when joining from command line, so disable it if
		// we get here.
		g_pInterfaceMgr->SetCommandLineJoin( false );
	}

	CBaseScreen::OnFocus(bFocus);
}


uint32 CScreenMain::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SINGLE_PLAYER:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_SINGLE);
			break;
		}

	case CMD_MULTI_PLAYER:
		{
			g_bLAN = false;

			char path[256];
			std::string sFN = _getcwd(path,sizeof(path));
			sFN += "\\";
			sFN += MISSION_DM_FILE;

			if (!CWinUtil::FileExist(sFN.c_str()))
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(IDS_NO_DM_MAPS,&mb);
				return 0;
			}

			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
		} break;

	case CMD_MULTI_PLAYER_LAN:
		{
			g_bLAN = true;
			char path[256];
			std::string sFN = _getcwd(path,sizeof(path));
			sFN += "\\";
			sFN += MISSION_DM_FILE;

			if (!CWinUtil::FileExist(sFN.c_str()))
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(IDS_NO_DM_MAPS,&mb);
				return 0;
			}

			g_pGameClientShell->SetGameType( eGameTypeDeathmatch);

			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
		} break;

	case CMD_OPTIONS:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_OPTIONS);
			break;
		}
	case CMD_PROFILE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PROFILE);
			break;
		}
	case CMD_QUIT:
		{

			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_SUREWANTQUIT,&mb);
			break;
		}
	case CMD_EXIT:
		{
#ifdef _DEMO
			g_pInterfaceMgr->ShowDemoScreens(LTTRUE);
#else
            g_pLTClient->Shutdown();
#endif
			break;
		}
	case CMD_RESUME:
		{
			Escape();
			break;
		}
	case CMD_CONTINUE_GAME:
		{
			// Initialize to the sp mission bute.
			if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
			{
				g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
				return 0;
  			}

			// Start the game from the continue save.
			if( !g_pMissionMgr->StartGameFromContinue( ))
				return 0;

			return 1;
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Screen specific rendering
LTBOOL   CScreenMain::Render(HSURFACE hDestSurf)
{
	return CBaseScreen::Render(hDestSurf);

}


void CScreenMain::Escape()
{
	CBaseScreen::Escape();
}