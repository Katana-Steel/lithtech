// FolderSave.cpp: implementation of the CFolderSave class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderSave.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "SaveGameData.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ProfileMgr.h"
extern CGameClientShell* g_pGameClientShell;

#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	enum eLocalCommands
	{
		CMD_OVERWRITE = FOLDER_CMD_CUSTOM+100,
		CMD_QUICKSAVE,
		CMD_SAVE,
		CMD_DELETE,
	};
	uint32	 g_nSaveSlot = kMaxSave+1;
	void OverwriteCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderSave *pThisFolder = (CFolderSave *)pData;
		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(CMD_OVERWRITE,0,0);
	}

	int kMissionWidth	 = 200;
	int kDateWidth		 = 100;
	int kTimeWidth		 = 100;
	int kDifficultyWidth = 100;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSave::CFolderSave()
{
}

CFolderSave::~CFolderSave()
{

}

LTBOOL CFolderSave::Build()
{
	LITHFONTCREATESTRUCT lfcs;
	lfcs.szFontName = "Arial";
	lfcs.nWidth = g_nLoadSaveTextWidth;
	lfcs.nHeight = GetTinyFont()->GetHeight(); 

	m_TextFont.Init(g_pLTClient, &lfcs);

	CreateTitle(IDS_TITLE_SAVEGAME);

	if (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID, "DateWidth"))
	{
		kDateWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "DateWidth");		
	}
	if (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID, "TimeWidth"))
	{
		kTimeWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "TimeWidth");		
	}
	if (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID, "MissionWidth"))
	{
		kMissionWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "MissionWidth");		
	}
	if (g_pLayoutMgr->HasCustomValue((eFolderID)m_nFolderID, "DifficultyWidth"))
	{
		kDifficultyWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID, "DifficultyWidth");		
	}

	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"QuickPos");
	CLTGUITextItemCtrl *pCtrl = CreateTextItem(IDS_QUICKSAVE,CMD_QUICKSAVE, IDS_HELP_QUICKSAVE, LTFALSE, GetSmallFont());
	AddFixedControl(pCtrl,kFreeGroup,pos);

	LTRect listrect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"SaveRect");
	int nWidth = listrect.right - listrect.left;
	int nHeight = listrect.bottom - listrect.top;
	pos.x = listrect.left;
	pos.y = listrect.top;

	AddFrame(listrect.left, listrect.top - 2, nWidth + 27, nHeight + 2, GetSmallFixedFont()->GetHeight());


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

	AddFixedControl(pCtrl,kNoGroup,pos,LTFALSE);
	AddFixedControl(m_pList,kFreeGroup,LTIntPt(listrect.left,listrect.top));

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"SavePos");
	m_pSave = CreateTextItem(IDS_SAVE, CMD_SAVE, IDS_HELP_SAVED_SAVE, LTFALSE, GetSmallFont());
	AddFixedControl(m_pSave,kCustomGroup0,pos, LTTRUE);

	pos = g_pLayoutMgr->GetFolderCustomPoint((eFolderID)m_nFolderID,"DeletePos");
	m_pDelete = CreateTextItem(IDS_DELETE, CMD_DELETE, IDS_HELP_SAVED_DELETE, LTFALSE, GetSmallFont());
	AddFixedControl(m_pDelete,kCustomGroup0,pos, LTTRUE);

	if ( !CBaseFolder::Build() ) return LTFALSE;

	UseBack(LTTRUE);
	
	return LTTRUE;
}


void CFolderSave::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		g_nSaveSlot = (kMaxSave+1);
		BuildSavedLevelList();
		CheckSelection();
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelList();
	}
	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderSave::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_QUICKSAVE:
		if (!SaveGame(kMaxSave+1))
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_SAVEGAMEFAILED);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
			g_pLTClient->FreeString(hString);
		}
		break;
	case CMD_SAVE:
		if (m_pList->GetSelectedItem() >= 0)
		{
			CLTGUICtrl *pCtrl = m_pList->GetSelectedControl();
			uint32 slot = pCtrl->GetParam1();
			char strSaveGameSetting[256];
			char strKey[32];
			sprintf (strKey, "SaveGame%02d", slot);
			char strIniName[128];
			sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, strIniName);

			char strFilename[128];
			sprintf (strFilename, "%s\\%s\\Slot%02d.sav", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), slot);

			g_nSaveSlot = slot;

			if (strlen (strSaveGameSetting) > 0 && CWinUtil::FileExist(strFilename))
			{
				HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRMSAVE);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,OverwriteCallBack,this);
				g_pLTClient->FreeString(hString);
				return 1;
			}
			if (!SaveGame(g_nSaveSlot))
			{
				HSTRING hString = g_pLTClient->FormatString(IDS_SAVEGAMEFAILED);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
				g_pLTClient->FreeString(hString);
			}
		}
		break;
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
				m_pDelete->Enable(LTFALSE);
			}
		}
		break;
	case CMD_OVERWRITE:
		if (g_nSaveSlot >= 0)
		{
			if (!SaveGame(g_nSaveSlot))
			{
				HSTRING hString = g_pLTClient->FormatString(IDS_SAVEGAMEFAILED);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
				g_pLTClient->FreeString(hString);
			}
		}
		break;
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};

void CFolderSave::BuildSavedLevelList()
{
	char strIniName[128];
	sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
	int nFirstEmpty = -1;

	//
	// Set up our empty slot.
	//
	CLTGUITextItemCtrl *pEmptyCtrl = CreateTextItem(IDS_EMPTY,LTNULL, LTNULL, LTFALSE, &m_TextFont);
	pEmptyCtrl->SetColor(SETRGB_T(255,255,255), SETRGB_T(192,192,192));
	// param1 will be set to the first empty slot.
	pEmptyCtrl->SetParam2(UINT_MAX);

	// Add the empty slot to the top of the list.
	m_pList->AddControl(pEmptyCtrl);
	

	//
	// Set up our previous saved games.
	//
	char strSaveGameSetting[256];
	memset (strSaveGameSetting, 0, 256);

	for (int slot = 0; slot < kMaxSave; slot++)
	{
		CSaveGameData sSave;
		LTBOOL bEmpty = LTTRUE;

		char strFilename[128];
		sprintf (strFilename, "%s\\%s\\Slot%02d.sav", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), slot);
		if (CWinUtil::FileExist(strFilename))
		{

			char szKey[32] = "";
			sprintf (szKey, "SaveGame%02d", slot);
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, szKey, "", strSaveGameSetting, 256, strIniName);

			if (strlen (strSaveGameSetting) > 0)
			{
				sSave.ParseSaveString(strSaveGameSetting);

			}

			//build the entry
			if (strlen(sSave.szWorldName) > 0)
			{
				bEmpty = LTFALSE;

				CLTGUIColumnTextCtrl* pColCtrl = CreateColumnText(LTNULL, IDS_HELP_SAVEGAME, LTFALSE, &m_TextFont);
				pColCtrl->SetParam1( slot );

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


		// Did we find our first empty slot?
		if (bEmpty && nFirstEmpty < 0) 
			nFirstEmpty = slot;

	}

	if (nFirstEmpty >= 0)
	{
		pEmptyCtrl->SetParam1(nFirstEmpty);
	}
	else
	{
		// No more empty slots, so don't offer.
		m_pList->RemoveControl(pEmptyCtrl);
	}

}

void CFolderSave::ClearSavedLevelList()
{
	m_pList->RemoveAllControls();
}

LTBOOL CFolderSave::SaveGame(uint32 slot)
{
	// First make sure we have somewhere to put our save...
	char saveDir[128];
	sprintf(saveDir,"%s\\%s",SAVE_DIR,g_pProfileMgr->GetCurrentProfileName());

	if (!CWinUtil::DirExist(saveDir))
	{
		CWinUtil::CreateDir(saveDir);
	}
	
 	char strFilename[128];
	if (slot < kMaxSave)
		sprintf (strFilename, "%s\\%s\\Slot%02d.sav", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), slot);
	else
	{
		// they are trying to save over the quick save so let's archive...
		sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_FILENAME);

		// if the quick save exists..  archive it..
		if(CWinUtil::FileExist(strFilename))
		{
			char strNewFilename[128];
			sprintf (strNewFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_BACKUP_FILENAME);
			rename(strFilename,strNewFilename);

			char strIniName[128];
			CWinUtil::WinWritePrivateProfileString (GAME_NAME, "QuickBackup", strNewFilename, strIniName);
		}
	}


	if (!g_pGameClientShell->SaveGame (strFilename))
	{
		return LTFALSE;
	}
	
	char strKey[32];
	if (slot < kMaxSave)
		sprintf (strKey, "SaveGame%02d", slot);
	else
		SAFE_STRCPY(strKey, "Quick");

	char strSaveGame[256];
	char strIniName[128];
	sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);

	CSaveGameData savedGame;
	FillSavedGameData(&savedGame);
	savedGame.BuildSaveString(strSaveGame);
	CWinUtil::WinWritePrivateProfileString (GAME_NAME, strKey, strSaveGame, strIniName);

//	g_pInterfaceMgr->GetMessageMgr()->AddLine(IDS_GAMESAVED);
	g_pInterfaceMgr->UseCursor(LTFALSE);
	g_pInterfaceMgr->ChangeState(GS_PLAYING);
	return LTTRUE;
}


LTBOOL   CFolderSave::OnLButtonUp(int x, int y)
{
	LTBOOL bHandled = CBaseFolder::OnLButtonUp(x,y);
	CheckSelection();
	return bHandled;
}

LTBOOL   CFolderSave::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CBaseFolder::OnMouseMove(x,y);
	if (bHandled)
		CheckSelection();
	return bHandled;
}

LTBOOL   CFolderSave::OnUp()
{
	LTBOOL bHandled = CBaseFolder::OnUp();
	CheckSelection();
	return bHandled;
}

LTBOOL   CFolderSave::OnDown()
{
	LTBOOL bHandled = CBaseFolder::OnDown();
	CheckSelection();
	return bHandled;
}

LTBOOL   CFolderSave::OnEnter()
{
	if (GetSelectedControl() == m_pList && m_pList->GetSelectedItem() >= 0)
	{
		return OnCommand(CMD_SAVE,0,0);
	}
	return CBaseFolder::OnEnter();
}

void CFolderSave::CheckSelection()
{
	CLTGUICtrl *pCtrl = LTNULL;
	if (m_pList->GetSelectedItem() >= 0)
		pCtrl = m_pList->GetSelectedControl();
	m_pDelete->Enable( (pCtrl && pCtrl->GetParam2() != UINT_MAX) );
	m_pSave->Enable( pCtrl != LTNULL );
}
