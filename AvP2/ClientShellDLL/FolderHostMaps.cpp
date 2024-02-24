// FolderHostMaps.cpp: implementation of the CFolderHostMaps class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderHostMaps.h"
#include "FolderMgr.h"
#include "LayoutMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "ServerOptionMgr.h"
#include "ClientUtilities.h"

namespace
{
	const std::string MULTI_DIR("WORLDS\\MULTI\\");
	const std::string DM_DIR("DM");
	const std::string TEAMDM_DIR("TEAMDM");
	const std::string HUNT_DIR("HUNT");
	const std::string SURVIVOR_DIR("SURVIVOR");
	const std::string OVERRUN_DIR("OVERRUN");
	const std::string EVAC_DIR("EVAC");

	enum eLocalCommands
	{
		CMD_ADD_ALL = FOLDER_CMD_CUSTOM,
		CMD_REMOVE_ALL,
		CMD_ADD,
		CMD_REMOVE
	};
}


char* GetBaseMapName(char *szMap)
{
	if(!szMap) return LTNULL;

	int size = strlen(szMap);

	for(char *szTemp = szMap + size; szTemp > szMap; szTemp--)
	{
		if(*szTemp == '\\')
		{
			szTemp = szTemp + 1;
			break;
		}
	}

	return szTemp;
}


class MapSortLesser
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		char *pX, szBufX[128];
		char *pY, szBufY[128];

		strcpy(szBufX, x.c_str());
		strcpy(szBufY, y.c_str());

		pX = GetBaseMapName(szBufX);
		pY = GetBaseMapName(szBufY);

		return (stricmp(pX, pY) < 0 );
	}
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderHostMaps::CFolderHostMaps()
{
	m_pAvailList = LTNULL;
	m_pSelectList = LTNULL;

	m_pAdd = LTNULL;
	m_pRemove = LTNULL;
	m_pAddAll = LTNULL;
	m_pRemoveAll = LTNULL;
}

CFolderHostMaps::~CFolderHostMaps()
{
}


// Build the folder
LTBOOL CFolderHostMaps::Build()
{
	// Some temp variables for this function...
	CLTGUITextItemCtrl* pCtrl = LTNULL;
	CFrameCtrl *pFrame = LTNULL;
	LTIntPt pos;


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	// Create the title of this menu...
	CreateTitle(IDS_TITLE_HOST_MAPS);
	UseLogo(LTFALSE);


	// Add the links to other menus...
	AddLink(IDS_GAME, FOLDER_CMD_MP_HOST, IDS_HELP_HOST);
	AddLink(IDS_SETUP, FOLDER_CMD_MP_SETUP, IDS_HELP_SETUP);
	AddLink(IDS_MAPS, FOLDER_CMD_MP_MAPS, IDS_HELP_MAPS)->Enable(LTFALSE);
	AddLink(IDS_ADVANCED, FOLDER_CMD_MP_ADVANCED, IDS_HELP_ADVANCED);
	AddLink(IDS_PLAYER, FOLDER_CMD_MP_PLAYER, IDS_HELP_PLAYER);


	// Grab some custom values out of the attributes
	LTRect rAvailable = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID, "AvailableRect");
	LTRect rSelected = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID, "SelectedRect");



	int nCtrlHeight;

	// Make the title for the available list
	pCtrl = CreateTextItem(IDS_MAPS_AVAILABLE, LTNULL, LTNULL, LTTRUE, GetHelpFont());
	pCtrl->Enable(LTFALSE);
	nCtrlHeight = (GetHelpFont()->GetHeight() + 6);

	// Make the available list control
	m_pAvailList = new CListCtrl();
	m_pAvailList->Create(rAvailable.bottom - rAvailable.top - nCtrlHeight, LTTRUE, rAvailable.right - rAvailable.left);
	m_pAvailList->EnableMouseClickSelect(LTTRUE);
	m_pAvailList->EnableMouseMoveSelect(LTFALSE);
	m_pAvailList->UseHighlight(LTTRUE);
	m_pAvailList->SetHighlightColor(m_hHighlightColor);
	m_pAvailList->SetIndent(LTIntPt(12, 4));


	// Create the backdrops for these areas
	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, m_pAvailList->GetWidth(), rAvailable.bottom - rAvailable.top);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rAvailable.left, rAvailable.top), LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, m_pAvailList->GetWidth(), nCtrlHeight);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rAvailable.left, rAvailable.top), LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, m_pAvailList->GetWidth(), m_pAvailList->GetHeight(), LTFALSE);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rAvailable.left, rAvailable.top + nCtrlHeight), LTFALSE);


	// Add the other controls now so they'll draw over the top of the frames
	AddFixedControl(pCtrl, kNoGroup, LTIntPt(rAvailable.left + 3, rAvailable.top + 3), LTFALSE);
	AddFixedControl(m_pAvailList, kCustomGroup0, LTIntPt(rAvailable.left, rAvailable.top + nCtrlHeight));



	// Make the title for the selected list
	pCtrl = CreateTextItem(IDS_MAPS_SELECTED, LTNULL, LTNULL, LTTRUE, GetHelpFont());
	pCtrl->Enable(LTFALSE);
	nCtrlHeight = (GetHelpFont()->GetHeight() + 6);

	// Make a list control
	m_pSelectList = new CListCtrl();
	m_pSelectList->Create(rSelected.bottom - rSelected.top - nCtrlHeight, LTTRUE, rSelected.right - rSelected.left);
	m_pSelectList->EnableMouseClickSelect(LTTRUE);
	m_pSelectList->EnableMouseMoveSelect(LTFALSE);
	m_pSelectList->UseHighlight(LTTRUE);
	m_pSelectList->SetHighlightColor(m_hHighlightColor);
	m_pSelectList->SetIndent(LTIntPt(12, 4));

	// Create the backdrops for these areas
	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(10,10,10), 0.75f, m_pSelectList->GetWidth(), rSelected.bottom - rSelected.top);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rSelected.left, rSelected.top), LTFALSE);

	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, m_pSelectList->GetWidth(), nCtrlHeight);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rSelected.left, rSelected.top), LTFALSE);


	pFrame = new CFrameCtrl;
	pFrame->Create(g_pLTClient, SETRGB(0,32,128), 0.75f, m_pSelectList->GetWidth(), m_pSelectList->GetHeight(), LTFALSE);
	AddFixedControl(pFrame, kNoGroup, LTIntPt(rSelected.left, rSelected.top + nCtrlHeight), LTFALSE);


	// Add the other controls now so they'll draw over the top of the frames
	AddFixedControl(pCtrl, kNoGroup, LTIntPt(rSelected.left + 3, rSelected.top + 3), LTFALSE);
	AddFixedControl(m_pSelectList, kCustomGroup2, LTIntPt(rSelected.left, rSelected.top + nCtrlHeight));



	// Added in the list modification controls
	pos = LTIntPt(GetPageLeft(), GetPageTop());
	m_pAdd = CreateTextItem(IDS_MAPS_ADD, CMD_ADD, IDS_HELP_MAPS_ADD);
	AddFixedControl(m_pAdd, kCustomGroup1, pos);

	pos.y += m_pAdd->GetHeight();
	m_pRemove = CreateTextItem(IDS_MAPS_REMOVE, CMD_REMOVE, IDS_HELP_MAPS_REMOVE);
	AddFixedControl(m_pRemove, kCustomGroup1, pos);

	pos.y += m_pRemove->GetHeight();
	m_pAddAll = CreateTextItem(IDS_MAPS_ADD_ALL, CMD_ADD_ALL, IDS_HELP_MAPS_ADD_ALL);
	AddFixedControl(m_pAddAll, kCustomGroup1, pos);

	pos.y += m_pAddAll->GetHeight();
	m_pRemoveAll = CreateTextItem(IDS_MAPS_REMOVE_ALL, CMD_REMOVE_ALL, IDS_HELP_MAPS_REMOVE_ALL);
	AddFixedControl(m_pRemoveAll, kCustomGroup1, pos);



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


uint32 CFolderHostMaps::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_ADD:
		{
			if (m_pSelectList->GetNum() < MAX_MP_LEVELS)
			{
				CLTGUICtrl *pCtrl = m_pAvailList->GetSelectedControl();
				if (pCtrl)
				{
					HSTRING hStr =  ((CLTGUITextItemCtrl *)pCtrl)->GetString(0);
					CLTGUITextItemCtrl *pNewCtrl = CreateTextItem(hStr,CMD_REMOVE,LTNULL,LTFALSE,GetTinyFont());
					m_pSelectList->AddControl(pNewCtrl);
				}
			}
			break;
		}

		case CMD_ADD_ALL:
		{
			for (int i = 0; i < m_pAvailList->GetNum() && m_pSelectList->GetNum() < MAX_MP_LEVELS; i++)
			{
				CLTGUICtrl *pCtrl = m_pAvailList->GetControl(i);
				if (pCtrl)
				{
					HSTRING hStr =  ((CLTGUITextItemCtrl *)pCtrl)->GetString(0);
					CLTGUITextItemCtrl *pNewCtrl = CreateTextItem(hStr,CMD_REMOVE,LTNULL,LTFALSE,GetTinyFont());
					m_pSelectList->AddControl(pNewCtrl);
				}
			}
			break;
		}

		case CMD_REMOVE:
		{
			CLTGUICtrl *pCtrl = m_pSelectList->GetSelectedControl();
			if (pCtrl)
			{
				m_pSelectList->RemoveControl(pCtrl);
			}
			CheckSelection();
			if (!m_pRemove->IsEnabled() && GetSelectedControl() == m_pRemove)
			{
				SetSelection(kNoSelection);
			}

			break;
		}

		case CMD_REMOVE_ALL:
		{
			m_pSelectList->RemoveAllControls();
			CheckSelection();
			if (!m_pRemoveAll->IsEnabled() && GetSelectedControl() == m_pRemoveAll)
			{
				SetSelection(kNoSelection);
			}

			break;
		}

		default:
			return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;

	
}

void CFolderHostMaps::Escape()
{
	g_pInterfaceMgr->PlayInterfaceSound(IS_ESCAPE);
	g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MULTI);
}

void CFolderHostMaps::OnFocus(LTBOOL bFocus)
{
	FocusSetup(bFocus);

	CBaseFolder::OnFocus(bFocus);
}


void CFolderHostMaps::FocusSetup(LTBOOL bFocus)
{
	if (bFocus)
	{
		char szDir[128];
		m_slMaps.clear();

		// Make sure the control lists are cleared
		m_pAvailList->RemoveAllControls();
		m_pSelectList->RemoveAllControls();

		CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

		// Fill in all the available maps based off the game type
		switch(pOptions->nGameType)
		{
			case MP_DM:
			{
				std::string sDir = MULTI_DIR + DM_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);
				break;
			}

			case MP_TEAMDM:
			{
				std::string sDir = MULTI_DIR + TEAMDM_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);

				sDir = MULTI_DIR + DM_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);
				break;
			}

			case MP_HUNT:
			{
				std::string sDir = MULTI_DIR + HUNT_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);

				sDir = MULTI_DIR + DM_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);
				break;
			}

			case MP_SURVIVOR:
			{
				std::string sDir = MULTI_DIR + SURVIVOR_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);

				sDir = MULTI_DIR + DM_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);
				break;
			}

			case MP_OVERRUN:
			{
				std::string sDir = MULTI_DIR + OVERRUN_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);
				break;
			}

			case MP_EVAC:
			{
				std::string sDir = MULTI_DIR + EVAC_DIR;
				SAFE_STRCPY(szDir, sDir.c_str());
				AppendToMapList(szDir);
				break;
			}
		}

		FillLevelList(m_pAvailList);
		LoadLevelList(m_pSelectList);

		SetSelection(GetIndex(m_pAvailList));
		CheckSelection();


		UpdateData(LTFALSE);
	}
	else
	{
		UpdateData(LTTRUE);

		CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

		StringList *sMaps;

		switch(pOptions->nGameType)
		{
			case MP_DM:			sMaps = &(pOptions->sDM_Maps);		break;
			case MP_TEAMDM:		sMaps = &(pOptions->sTeam_Maps);	break;
			case MP_HUNT:		sMaps = &(pOptions->sHunt_Maps);	break;
			case MP_SURVIVOR:	sMaps = &(pOptions->sSurv_Maps);	break;
			case MP_OVERRUN:	sMaps = &(pOptions->sOver_Maps);	break;
			case MP_EVAC:		sMaps = &(pOptions->sEvac_Maps);	break;
		}

		sMaps->clear();

		HSTRING hStr = LTNULL;
		char *pStr = LTNULL;
		char szBuffer[128];
		char *szBase;

		for (int i = 0; i < MAX_MP_LEVELS; i++)
		{
			CLTGUICtrl *pCtrl = (i < m_pSelectList->GetNum()) ? m_pSelectList->GetControl(i) : LTNULL;

			if(pCtrl)
			{
				hStr = ((CLTGUITextItemCtrl *)pCtrl)->GetString(0);
				pStr = g_pLTClient->GetStringData(hStr);

				StringList::iterator iter = m_slMaps.begin();
				while(iter != m_slMaps.end())
				{
					std::string str = *iter;
					strcpy(szBuffer, str.c_str());

					// Get the base name of the map
					szBase = GetBaseMapName(szBuffer);

					if(!_stricmp(pStr, szBase))
					{
						sMaps->push_back(szBuffer);
						break;
					}

					iter++;
				}
			}
			else
			{
				sMaps->push_back("");
			}
		}


		g_pServerOptionMgr->SaveCurrentCfg();
	}
}


LTBOOL CFolderHostMaps::OnLButtonUp(int x, int y)
{
	LTBOOL bHandled = CBaseFolder::OnLButtonUp(x,y);
	CheckSelection();
	return bHandled;
}

LTBOOL CFolderHostMaps::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CBaseFolder::OnMouseMove(x,y);
	if (bHandled)
		CheckSelection();
	return bHandled;
}

LTBOOL CFolderHostMaps::OnUp()
{
	LTBOOL bHandled = CBaseFolder::OnUp();
	CheckSelection();
	return bHandled;
}

LTBOOL CFolderHostMaps::OnDown()
{
	LTBOOL bHandled = CBaseFolder::OnDown();
	CheckSelection();
	return bHandled;
}

LTBOOL CFolderHostMaps::OnEnter()
{
/*
	if (GetSelectedControl() == m_pAvailList && m_pAvailList->GetSelectedItem() != CListCtrl::kNoSelection)
	{
		return OnCommand(CMD_ADD,0,0);
	}
	if (GetSelectedControl() == m_pSelectList && m_pSelectList->GetSelectedItem() != CListCtrl::kNoSelection)
	{
		return OnCommand(CMD_REMOVE,0,0);
	}
*/
	return CBaseFolder::OnEnter();
}

void CFolderHostMaps::CheckSelection()
{
	m_pAdd->Enable(m_pSelectList->GetNum() < MAX_MP_LEVELS && m_pAvailList->GetNum() > 0 && m_pAvailList->GetSelectedItem() != CListCtrl::kNoSelection);
	m_pAddAll->Enable(m_pSelectList->GetNum() < MAX_MP_LEVELS && m_pAvailList->GetNum() > 0);
	m_pRemove->Enable(m_pSelectList->GetNum() > 0 && m_pSelectList->GetSelectedItem() != CListCtrl::kNoSelection);
	m_pRemoveAll->Enable(m_pSelectList->GetNum() > 0);
}


void CFolderHostMaps::AppendToMapList(char *szDir)
{
	// Make sure everthing is valid
	if(!szDir) return;


	// Grab the file list for this directory
    FileEntry* pFiles = g_pLTClient->GetFileList(szDir);
	if (!pFiles) return;

	FileEntry* ptr = pFiles;

	// Go through all the available files and append them to the lists
	while(ptr)
	{
		if(ptr->m_Type == TYPE_FILE)
		{
			if(strnicmp(&ptr->m_pBaseFilename[strlen(ptr->m_pBaseFilename) - 4], ".dat", 4) == 0)
			{
				char sLevel[128];
				strcpy(sLevel, ptr->m_pFullFilename);
				int len = strlen(sLevel);
				if (len > 4) sLevel[len - 4] = '\0';

				strlwr(sLevel);
				m_slMaps.push_back(sLevel);
			}
		}

		ptr = ptr->m_pNext;
	}

	// Free up the memory from the file list
    g_pLTClient->FreeFileList(pFiles);
}


void CFolderHostMaps::FillLevelList(CListCtrl *pList)
{
	// Make sure everthing is valid
	if(!pList) return;

	m_slMaps.sort(MapSortLesser());


	char szBuffer[128];
	char *szBase;
	int i = 0;

	// Go through each map name and add a control for it
	StringList::iterator iter = m_slMaps.begin();
//	- jrg 9/7/01 - this limit not needed on the available maps list
//	while(iter != m_slMaps.end() && i < MAX_MP_LEVELS) 
	while(iter != m_slMaps.end())
	{
		std::string str = *iter;
		strcpy(szBuffer, str.c_str());

		// Get the base name of the map
		szBase = GetBaseMapName(szBuffer);

		CLTGUITextItemCtrl *pCtrl = CreateTextItem(szBase,CMD_ADD,LTNULL,LTFALSE,GetTinyFont());
		pList->AddControl(pCtrl);

		iter++;
		i++;
	}
}

void CFolderHostMaps::LoadLevelList(CListCtrl *pList)
{
	// Sanity checks...
	if (!pList) return;
	CServerOptions *pOptions = g_pServerOptionMgr->GetCurrentCfg();

	char szBuffer[128];
	char *szBase;


	StringList *sMaps;

	switch(pOptions->nGameType)
	{
		case MP_DM:			sMaps = &(pOptions->sDM_Maps);		break;
		case MP_TEAMDM:		sMaps = &(pOptions->sTeam_Maps);	break;
		case MP_HUNT:		sMaps = &(pOptions->sHunt_Maps);	break;
		case MP_SURVIVOR:	sMaps = &(pOptions->sSurv_Maps);	break;
		case MP_OVERRUN:	sMaps = &(pOptions->sOver_Maps);	break;
		case MP_EVAC:		sMaps = &(pOptions->sEvac_Maps);	break;
	}


	StringList::iterator iter = sMaps->begin();
	int i = 0;

	while (iter != sMaps->end() && i < MAX_MP_LEVELS)
	{
		std::string str = *iter;
		strcpy(szBuffer, str.c_str());

		if(szBuffer[0])
		{
			// Get the base name of the map
			szBase = GetBaseMapName(szBuffer);

			CLTGUITextItemCtrl *pCtrl = CreateTextItem(szBase,CMD_REMOVE,LTNULL,LTFALSE,GetTinyFont());
			pList->AddControl(pCtrl);
		}

		iter++;
		i++;
	}
}

