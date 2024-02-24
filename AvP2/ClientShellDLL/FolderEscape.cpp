// FolderEscape.cpp: implementation of the CFolderEscape class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderEscape.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderEscape *pThisFolder = (CFolderEscape *)pData;

		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(FOLDER_CMD_EXIT,0,0);
	}

	void AbortCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderEscape *pThisFolder = (CFolderEscape *)pData;

		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(FOLDER_CMD_MAIN,0,0);
	}

	enum LocalCommands
	{
		CMD_ABORT = FOLDER_CMD_CUSTOM,
	};
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderEscape::CFolderEscape()
{
	m_pResume = LTNULL;
	m_pLoad = LTNULL;
	m_pSave = LTNULL;
	m_pOptions = LTNULL;
	m_pAbort = LTNULL;

	m_bAborting = LTFALSE;
}

CFolderEscape::~CFolderEscape()
{

}


// Build the folder
LTBOOL CFolderEscape::Build()
{
	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	CreateTitle(IDS_TITLE_PAUSE);

	m_pLoad = AddLink(IDS_LOADGAME, FOLDER_CMD_LOAD_GAME, IDS_HELP_LOAD);

#ifdef _DEMO
	m_pLoad->Enable(LTFALSE);
#endif

	m_pSave = AddLink(IDS_SAVEGAME, FOLDER_CMD_SAVE_GAME, IDS_HELP_SAVE);

	m_pOptions = AddLink(IDS_OPTIONS, FOLDER_CMD_OPTIONS, IDS_HELP_OPTIONS);
	m_pAbort = AddLink(IDS_ABORT_GAME, CMD_ABORT, IDS_HELP_ABORT_GAME);

	m_pResume = CreateTextItem(IDS_RESUME, FOLDER_CMD_RESUME, IDS_HELP_RESUME, LTFALSE, GetLargeFont());
	AddFixedControl(m_pResume,kBackGroup,m_BackPos);

	CLTGUITextItemCtrl* pCtrl = CreateTextItem(IDS_EXIT,FOLDER_CMD_QUIT, IDS_HELP_EXIT, LTFALSE, GetLargeFont());
	AddFixedControl(pCtrl,kNextGroup,m_ContinuePos);

    UseBack(LTFALSE);

	return LTTRUE;

}

void CFolderEscape::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// Remove all the main controls and then readd them based off game type...
		RemoveFixedControl(m_pLoad);
		RemoveFixedControl(m_pSave);
		RemoveFixedControl(m_pOptions);
		RemoveFixedControl(m_pAbort);

		// If it's a multiplayer game we want to get rid of the Load and Save options
		if(g_pGameClientShell->GetGameType()->IsMultiplayer())
		{
			AddFixedControl(m_pOptions, kLinkGroup, m_LinkPos[0]);
			AddFixedControl(m_pAbort, kLinkGroup, m_LinkPos[1]);
		}
		else
		{
			AddFixedControl(m_pLoad, kLinkGroup, m_LinkPos[0]);
			AddFixedControl(m_pSave, kLinkGroup, m_LinkPos[1]);
			AddFixedControl(m_pOptions, kLinkGroup, m_LinkPos[2]);
			AddFixedControl(m_pAbort, kLinkGroup, m_LinkPos[3]);
		}


		LTBOOL bCanSave = LTTRUE;
		LTBOOL bCanResume = LTTRUE;

#ifdef _DEMO
		bCanSave = LTFALSE;
#else
		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();

		if(g_pGameClientShell->GetGameType()->IsSinglePlayer())
		{
			if(	!hPlayerObj ||
				!g_pGameClientShell->IsInWorld() ||
				g_pGameClientShell->IsPlayerDead()
				)
			{
				bCanSave = LTFALSE;
				bCanResume = LTFALSE;
			}
		}
		else
		{
			bCanSave = LTFALSE;
			bCanResume = LTTRUE;
		}


		if 	(g_pGameClientShell->GetGameType()->GetDifficulty() == DIFFICULTY_PC_INSANE)
			bCanSave = LTFALSE;

#endif

		m_pSave->Enable(bCanSave);
		m_pResume->Enable(bCanResume);
	}

	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderEscape::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_ABORT:
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_SUREWANTABORT);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,AbortCallBack,this);
			g_pLTClient->FreeString(hString);

			break;
		}
	case FOLDER_CMD_MAIN:
		{
			m_bAborting = LTTRUE;
			if(g_pLTClient->IsConnected())
				g_pLTClient->Disconnect();
			m_bAborting = LTFALSE;

			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MAIN);

			break;
		}
	case FOLDER_CMD_SAVE_GAME:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_SAVE);
			break;
		}
	case FOLDER_CMD_LOAD_GAME:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_LOAD);
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
	case FOLDER_CMD_RESUME:
		{
			Escape();
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CFolderEscape::Escape()
{
	//quit or return to game?
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (g_pGameClientShell->IsInWorld() && hPlayerObj && 
		(!g_pGameClientShell->IsPlayerDead() || g_pGameClientShell->GetGameType()->IsMultiplayer()))
	{
		g_pInterfaceMgr->ChangeState(GS_PLAYING);
	}
}

