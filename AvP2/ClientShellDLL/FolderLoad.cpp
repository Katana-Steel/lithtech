// FolderLoad.cpp: implementation of the CFolderLoad class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderLoad.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ProfileMgr.h"
#include "MissionMgr.h"
#include "SaveGameData.h"

extern CGameClientShell* g_pGameClientShell;

#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	int kTimeWidth		= 100;
	int kDateWidth		= 100;
	int kMissionWidth	= 200;
	int kDifficultyWidth = 100;

	enum eLocalCommands
	{
		CMD_QUICKLOAD = FOLDER_CMD_CUSTOM+100,
		CMD_QUICKLOAD_BAK,
		CMD_RELOAD,
		CMD_LOAD,
		CMD_DELETE
	};

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderLoad::CFolderLoad()
{
	m_pQuick	= LTNULL;
	m_pBakQuick	= LTNULL;
	m_pReload	= LTNULL;
	m_pQuickName	= LTNULL;
	m_pBakQuickName	= LTNULL;
	m_pReloadName	= LTNULL;
	m_pList		= LTNULL;
	m_pLoad		= LTNULL;
	m_pDelete	= LTNULL;
}

CFolderLoad::~CFolderLoad()
{
	m_pList	  = LTNULL;
}

LTBOOL CFolderLoad::Build()
{
	LITHFONTCREATESTRUCT lfcs;
	lfcs.szFontName = "Arial";
	lfcs.nWidth = g_nLoadSaveTextWidth;
	lfcs.nHeight = GetTinyFont()->GetHeight(); 
	m_TextFont.Init(g_pLTClient, &lfcs);

	CreateTitle(IDS_TITLE_LOADGAME);
	kDateWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "DateWidth");		
	kTimeWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "TimeWidth");		
	kMissionWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "MissionWidth");		
	kDifficultyWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "DifficultyWidth");

	int	nameOffset = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "NameOffset");

	//-------------------------------------------------------------//
	//-------------------------------------------------------------//

	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"QuickPos");
	m_pQuick = CreateTextItem(IDS_QUICKLOAD,CMD_QUICKLOAD, IDS_HELP_QUICKLOAD, LTFALSE, GetSmallFont());
	m_pQuickName = CreateTextItem("()",LTNULL,LTNULL, LTTRUE, &m_TextFont);

	int htDiff = (m_pQuick->GetHeight() - m_pQuickName->GetHeight()) / 2;
	LTIntPt tmpPos;
	tmpPos.x = pos.x + nameOffset;
	tmpPos.y = pos.y + htDiff;

	AddFixedControl(m_pQuick,kFreeGroup,pos);
	AddFixedControl(m_pQuickName,kNoGroup,tmpPos);

	//-------------------------------------------------------------//
	//-------------------------------------------------------------//

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"BackupQuickPos");
	m_pBakQuick = CreateTextItem(IDS_QUICKLOAD_BAK,CMD_QUICKLOAD_BAK, IDS_HELP_QUICKLOAD_BAK, LTFALSE, GetSmallFont());
	m_pBakQuickName = CreateTextItem("()",LTNULL,LTNULL, LTTRUE, &m_TextFont);

	htDiff = (m_pBakQuick->GetHeight() - m_pBakQuickName->GetHeight()) / 2;
	tmpPos.x = pos.x + nameOffset;
	tmpPos.y = pos.y + htDiff;

	AddFixedControl(m_pBakQuick,kFreeGroup,pos);
	AddFixedControl(m_pBakQuickName,kNoGroup,tmpPos);

	//-------------------------------------------------------------//
	//-------------------------------------------------------------//

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"ReloadPos");
	m_pReload = CreateTextItem(IDS_LOADCURRENT,CMD_RELOAD, IDS_HELP_RELOAD, LTFALSE, GetSmallFont());
	m_pReloadName = CreateTextItem("()",LTNULL,LTNULL, LTTRUE, &m_TextFont);

	htDiff = (m_pReload->GetHeight() - m_pReloadName->GetHeight()) / 2;
	tmpPos.x = pos.x + nameOffset;
	tmpPos.y = pos.y + htDiff;

	AddFixedControl(m_pReload,kFreeGroup,pos);
	AddFixedControl(m_pReloadName,kNoGroup,tmpPos);

	LTRect listrect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"SaveRect");
	int nWidth = listrect.right - listrect.left;
	int nHeight = listrect.bottom - listrect.top;
	pos.x = listrect.left;
	pos.y = listrect.top;

	AddFrame(listrect.left,listrect.top-2,(listrect.right-listrect.left)+27,(listrect.bottom-listrect.top)+2,GetSmallFixedFont()->GetHeight());

	CStaticTextCtrl *pStatic = CreateStaticTextItem(IDS_LOAD_USERGAME, LTNULL, LTNULL, nWidth, nHeight, LTFALSE, GetHelpFont());
	AddFixedControl(pStatic, kNoGroup, LTIntPt(listrect.left + 3, listrect.top + 3), LTFALSE);

	listrect.top += GetHelpFont()->GetHeight() + 6;

	// Make a list control
	m_pList = new CListCtrl();
	m_pList->Create(listrect.bottom-listrect.top, LTTRUE, listrect.right-listrect.left);
	m_pList->EnableMouseClickSelect(LTTRUE);
	m_pList->EnableMouseMoveSelect(LTFALSE);
	m_pList->UseHighlight(LTTRUE);
	m_pList->SetHighlightColor(m_hHighlightColor);
	m_pList->SetIndent(LTIntPt(12,4));



	AddFixedControl(m_pList,kCustomGroup0,LTIntPt(listrect.left,listrect.top));


	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"LoadPos");
	m_pLoad = CreateTextItem(IDS_LOAD, CMD_LOAD, IDS_HELP_SAVED_LOAD, LTFALSE, GetSmallFont());
	AddFixedControl(m_pLoad,kCustomGroup1,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"DeletePos");
	m_pDelete = CreateTextItem(IDS_DELETE, CMD_DELETE, IDS_HELP_SAVED_DELETE, LTFALSE, GetSmallFont());
	AddFixedControl(m_pDelete,kCustomGroup1,pos, LTTRUE);

	if ( !CBaseFolder::Build() ) return LTFALSE;

	UseBack(LTTRUE);
	
	return LTTRUE;

}


void CFolderLoad::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildSavedLevelList();
		if (m_pList->GetNum() > 0)
			SetSelection(GetIndex(m_pList));
		else if (m_pQuick->IsEnabled())
			SetSelection(GetIndex(m_pQuick));
		else if (m_pReload->IsEnabled())
			SetSelection(GetIndex(m_pReload));
		else
			SetSelection(GetIndex(m_pBack));

		if (m_pList->GetNum() > 0)
		{
			m_pList->SelectItem(0);
			m_pLoad->Enable(LTTRUE);
			m_pDelete->Enable(LTTRUE);
		}
		else
		{
			m_pDelete->Select(LTFALSE);
			m_pLoad->Enable(LTFALSE);
			m_pDelete->Enable(LTFALSE);
		}

	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelList();
	}
	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderLoad::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_DELETE:
		if (m_pList->GetSelectedItem() >= 0)
		{
			CLTGUICtrl *pCtrl = m_pList->GetSelectedControl();
			uint32 slot = pCtrl->GetParam1();
			char strFilename[128];
			sprintf (strFilename, "%s\\%s\\Slot%02d.sav", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), slot);
			remove(strFilename);
			m_pList->ClearSelection();
			m_pList->RemoveControl(pCtrl);
			if (m_pList->GetNum() > 0)
				m_pList->SelectItem(0);
			else
			{
				m_pDelete->Select(LTFALSE);
				m_pLoad->Enable(LTFALSE);
				m_pDelete->Enable(LTFALSE);
			}
		}
		break;

	case CMD_QUICKLOAD:
	case CMD_QUICKLOAD_BAK:
	case CMD_RELOAD:
	case CMD_LOAD:
		{
			char strSaveGameSetting[256];
			memset (strSaveGameSetting, 0, 256);
			char strKey[32];
			char strFilename[128];

			if (dwCommand == CMD_QUICKLOAD)
			{
				sprintf (strKey, "Quick");
				sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_FILENAME);
			}
			else if (dwCommand == CMD_QUICKLOAD_BAK)
			{
				sprintf (strKey, "QuickBackup");
				sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_BACKUP_FILENAME);
			}
			else if (dwCommand == CMD_RELOAD)
			{
				sprintf (strKey, "Reload");
				sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), RELOADLEVEL_FILENAME);
			}
			else if (dwCommand == CMD_LOAD)
			{
				if (m_pList->GetNum() > 0 && m_pList->GetSelectedItem() >= 0)
				{
					CLTGUICtrl *pCtrl = m_pList->GetSelectedControl();

					if(!pCtrl) return 0;

					uint32 slot = pCtrl->GetParam1();
					sprintf (strKey, "SaveGame%02d", slot);
					sprintf (strFilename, "%s\\%s\\Slot%02d.sav", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), slot);
				}
				else
					return 0;
			}
			else
				return 0;

			char strIniName[128];
			sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, strIniName);
			if (!strlen(strSaveGameSetting)) return 0;

			CSaveGameData save;

			save.ParseSaveString(strSaveGameSetting);

			g_pProfileMgr->SetSpecies(save.eSpecies);

			g_pGameClientShell->GetGameType()->SetDifficulty(save.nDifficulty);

			if (g_pGameClientShell->LoadGame(save.szWorldName, strFilename))
			{
				return 1;
			}
			else
			{
				g_pInterfaceMgr->ChangeState(GS_FOLDER);

				HSTRING hString = g_pLTClient->FormatString(IDS_LOADGAMEFAILED);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
				g_pLTClient->FreeString(hString);
				return 0;
			}

		} break;
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
		
	}

	return 1;

};

void CFolderLoad::BuildSavedLevelList()
{
	char strFilename[128];
	char strSaveGameSetting[256];
	memset (strSaveGameSetting, 0, 256);
	CSaveGameData sSave;

	char strIniName[128];
	sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);

	sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_FILENAME);
	m_pQuick->Enable(CWinUtil::FileExist(strFilename));
	m_pQuickName->RemoveAll();
	if (CWinUtil::FileExist(strFilename))
	{
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, "Quick", "", strSaveGameSetting, 256, strIniName);
		sSave.ParseSaveString(strSaveGameSetting);
		SetSaveGameText(m_pQuickName,sSave);

	}
		
	sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_BACKUP_FILENAME);
	m_pBakQuick->Enable(CWinUtil::FileExist(strFilename));
	m_pBakQuickName->RemoveAll();
	if (CWinUtil::FileExist(strFilename))
	{
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, "QuickBackup", "", strSaveGameSetting, 256, strIniName);
		sSave.ParseSaveString(strSaveGameSetting);
		SetSaveGameText(m_pBakQuickName,sSave);

	}

	sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), RELOADLEVEL_FILENAME);
	m_pReload->Enable(CWinUtil::FileExist(strFilename));
	m_pReloadName->RemoveAll();
	if (CWinUtil::FileExist(strFilename))
	{
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, "Reload", "", strSaveGameSetting, 256, strIniName);
		sSave.ParseSaveString(strSaveGameSetting);
		SetSaveGameText(m_pReloadName,sSave);

	}
	

	for (int i = 0; i < kMaxSave; i++)
	{
		sprintf (strFilename, "%s\\%s\\Slot%02d.sav", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), i);
		if (CWinUtil::FileExist(strFilename))
		{

			char szKey[32] = "";
			sprintf (szKey, "SaveGame%02d", i);
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, szKey, "", strSaveGameSetting, 256, strIniName);
			sSave.ParseSaveString(strSaveGameSetting);
			if (strlen(sSave.szWorldName) > 0)
			{
				CLTGUIColumnTextCtrl* pColCtrl = CreateColumnText(LTNULL, IDS_HELP_LOADGAME, LTFALSE, &m_TextFont);
				pColCtrl->SetParam1(i);

				const uint32 nSortIndex = BuildSaveGameEntry(pColCtrl, sSave, kMissionWidth, kDateWidth, kTimeWidth, kDifficultyWidth);

				pColCtrl->SetParam2(nSortIndex);
				LTBOOL bAddedCtrl = LTFALSE;
				for( int curControl = 0; curControl < m_pList->GetNum() && !bAddedCtrl; ++curControl )
				{
					CLTGUICtrl * pCurCtrl = m_pList->GetControl(curControl);
					if( pCurCtrl->GetParam2() < nSortIndex )
					{
						m_pList->InsertControl(curControl, pColCtrl);
						bAddedCtrl = LTTRUE;
					}
				}

				if( !bAddedCtrl )
				{
					m_pList->AddControl(pColCtrl);
				}
			}

		}

	}


}

void CFolderLoad::ClearSavedLevelList()
{
	if (m_pList)
	{
		m_pList->RemoveAllControls();
	}

	m_pQuick->Enable(LTFALSE);
	m_pBakQuick->Enable(LTFALSE);
	m_pReload->Enable(LTFALSE);
}




LTBOOL CFolderLoad::HandleKeyDown(int key, int rep)
{

//	if (key == VK_F9)
//	{
//		SendCommand(FOLDER_CMD_CUSTOM,0,0);
//       return LTTRUE;
//	}
    return CBaseFolder::HandleKeyDown(key,rep);

}


LTBOOL   CFolderLoad::OnLButtonUp(int x, int y)
{
	LTBOOL bHandled = CBaseFolder::OnLButtonUp(x,y);
	CheckSelection();
	return bHandled;
}

LTBOOL   CFolderLoad::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CBaseFolder::OnMouseMove(x,y);
	if (bHandled)
		CheckSelection();
	return bHandled;
}

LTBOOL   CFolderLoad::OnUp()
{
	LTBOOL bHandled = CBaseFolder::OnUp();
	CheckSelection();
	return bHandled;
}

LTBOOL   CFolderLoad::OnDown()
{
	LTBOOL bHandled = CBaseFolder::OnDown();
	CheckSelection();
	return bHandled;
}

LTBOOL   CFolderLoad::OnEnter()
{
	if (GetSelectedControl() == m_pList && m_pList->GetSelectedItem() >= 0)
	{
		return OnCommand(CMD_LOAD,0,0);
	}
	return CBaseFolder::OnEnter();
}

void CFolderLoad::CheckSelection()
{
	CLTGUICtrl *pCtrl = LTNULL;
	if (m_pList->GetSelectedItem() >= 0)
		pCtrl = m_pList->GetSelectedControl();
	m_pDelete->Enable( pCtrl && pCtrl->GetParam2() != UINT_MAX );
	m_pLoad->Enable( pCtrl != LTNULL );
}
