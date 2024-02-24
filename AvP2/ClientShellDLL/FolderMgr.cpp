// FolderMgr.cpp: implementation of the CFolderMgr class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ClientRes.h"
#include "FolderMgr.h"
#include "ClientButeMgr.h"
#include "SoundMgr.h"

//folders
#include "BaseFolder.h"
#include "FolderMain.h"
#include "FolderSingle.h"
#include "FolderEscape.h"
#include "FolderOptions.h"
#include "FolderDisplay.h"
#include "FolderAudio.h"
#include "FolderGame.h"
#include "FolderPerformance.h"
#include "FolderCustomControls.h"
#include "FolderMouse.h"
#include "FolderKeyboard.h"
#include "FolderJoystick.h"
#include "FolderCustomLevel.h"
#include "FolderCampaignLevel.h"
#include "FolderProfile.h"
#include "FolderMulti.h"
#include "FolderSave.h"
#include "FolderLoad.h"
#include "FolderPlayer.h"
#include "FolderPlayerJoin.h"
#include "FolderJoin.h"
#include "FolderJoinInfo.h"
#include "FolderHost.h"
#include "FolderSetupDM.h"
#include "FolderSetupTeamDM.h"
#include "FolderSetupHunt.h"
#include "FolderSetupSurvivor.h"
#include "FolderSetupOverrun.h"
#include "FolderSetupEvac.h"
#include "FolderHostOptions.h"
#include "FolderHostMaps.h"
#include "FolderHostConfig.h"
#include "FolderCredits.h"


#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMgr::CFolderMgr()
{
    m_pCurrentFolder = LTNULL;
	m_eCurrentFolderID = FOLDER_ID_NONE;
	m_eLastFolderID = FOLDER_ID_NONE;
	m_nHistoryLen = 0;

	m_bGSA = LTFALSE;
	m_szGSA_IP[0] = LTNULL;
	m_szGSA_Port[0] = LTNULL;
	m_szGSA_PW[0] = LTNULL;
	m_szGSA_Name[0] = LTNULL;
}

CFolderMgr::~CFolderMgr()
{

}


//////////////////////////////////////////////////////////////////////
// Function name	: CFolderMgr::Init
// Description	    :
// Return type      : LTBOOL
// Argument         : CClientDE* pClientDE
// Argument         : CGameClientShell* pClientShell
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMgr::Init()
{
	//build folder array
	m_folderArray.SetSize(0);

	for (int nID = FOLDER_ID_MAIN; nID < FOLDER_ID_UNASSIGNED; nID++)
	{
		AddFolder((eFolderID)nID);
	}

    return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CFolderMgr::Term
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////

void CFolderMgr::Term()
{
	// Term the folders
	unsigned int i;
	for (i=0; i < m_folderArray.GetSize(); i++)
	{
		m_folderArray[i]->Term();
		delete m_folderArray[i];
	}

}



// Renders the folder to a surface
LTBOOL CFolderMgr::Render(HSURFACE hDestSurf)
{
	if (m_pCurrentFolder)
	{

		return m_pCurrentFolder->Render(hDestSurf);
	}
    return LTFALSE;
}

void CFolderMgr::UpdateInterfaceSFX()
{
	if (m_pCurrentFolder)
	{

		m_pCurrentFolder->UpdateInterfaceSFX();
	}
}


void CFolderMgr::HandleKeyDown (int vkey, int rep)
{
	if (m_pCurrentFolder)
	{
		if (vkey == g_pClientButeMgr->GetEscapeKey() )
		{
			m_pCurrentFolder->Escape();
		}
		else
			m_pCurrentFolder->HandleKeyDown(vkey,rep);
	}
}
void CFolderMgr::HandleKeyUp (int vkey)
{
	if (m_pCurrentFolder)
	{
		m_pCurrentFolder->HandleKeyUp(vkey);
	}
}

void CFolderMgr::HandleChar (char c)
{
	if (m_pCurrentFolder)
	{
		m_pCurrentFolder->HandleChar(c);
	}
}


LTBOOL CFolderMgr::PreviousFolder()
{
	if (m_nHistoryLen < 1) return LTFALSE;
	
	CBaseFolder *pNewFolder=GetFolderFromID(m_eFolderHistory[m_nHistoryLen-1]);
	if (pNewFolder)
	{
		SwitchToFolder(pNewFolder,LTTRUE);

		// The music may change per folder...
        g_pInterfaceMgr->SetMenuMusic(LTTRUE);

        return LTTRUE;
	}
    return LTFALSE;
}

LTBOOL CFolderMgr::SetCurrentFolder(eFolderID folderID)
{
	CBaseFolder *pNewFolder=GetFolderFromID(folderID);
	if (pNewFolder)
	{
		SwitchToFolder(pNewFolder);

		// The music may change per folder...
        g_pInterfaceMgr->SetMenuMusic(LTTRUE);

        return LTTRUE;
	}
    return LTFALSE;
}

void CFolderMgr::EscapeCurrentFolder()
{
	if (m_pCurrentFolder)
	{
		m_pCurrentFolder->Escape();
	}
}

void CFolderMgr::ExitFolders()
{
	// Tell the old folder that it is losing focus
	if (m_pCurrentFolder)
	{
        m_pCurrentFolder->OnFocus(LTFALSE);
	}

	//clear our folder history (no longer relevant)
	m_nHistoryLen = 0;
	m_eFolderHistory[0] = FOLDER_ID_NONE;
	m_pCurrentFolder = LTNULL;
}


void CFolderMgr::SwitchToFolder(CBaseFolder *pNewFolder, LTBOOL bBack)
{
	// Tell the old folder that it is losing focus
	if (m_pCurrentFolder)
	{
        m_pCurrentFolder->OnFocus(LTFALSE);
		if (bBack)
		{
			m_nHistoryLen--;
		}
		else if (m_nHistoryLen < MAX_FOLDER_HISTORY)
		{
			int insert = 0;
			
			eFolderID currentFolderID = (eFolderID)m_pCurrentFolder->GetFolderID();
			eFolderID nextFolderID = (eFolderID)pNewFolder->GetFolderID();
			while (insert < m_nHistoryLen && m_eFolderHistory[insert] != nextFolderID)
				insert++;
			if (insert == m_nHistoryLen)
			{
				++m_nHistoryLen;
				m_eFolderHistory[insert] = currentFolderID;
			}
			else
			{
				m_nHistoryLen = insert;
			}
		}
	}

	m_pCurrentFolder=pNewFolder;
	m_eLastFolderID = m_eCurrentFolderID;
	m_eCurrentFolderID = (eFolderID)m_pCurrentFolder->GetFolderID();
	if (FOLDER_ID_MAIN == m_eCurrentFolderID)
	{
		m_nHistoryLen = 0;
	}

	// If the new folder hasn't been built yet... better build it!
	if (!m_pCurrentFolder->IsBuilt())
	{

		m_pCurrentFolder->Build();
	}

	// Do any special case work for each folder
//	switch(folderID)
//	{
//	case FOLDER_ID_NEW:
//		break;
//	default:
//		break;
//	}

	// Tell the new folder that it is gaining focus
	if (pNewFolder)
	{
        pNewFolder->OnFocus(LTTRUE);
	}



}


// Returns a folder based on an ID
CBaseFolder *CFolderMgr::GetFolderFromID(eFolderID folderID)
{
	CBaseFolder *pFolder=NULL;

	int f = 0;
	while (f < (int)m_folderArray.GetSize() && m_folderArray[f]->GetFolderID() != (int)folderID)
		f++;

	if (f < (int)m_folderArray.GetSize())
		pFolder = m_folderArray[f];

	return pFolder;

}

// Mouse messages
void	CFolderMgr::OnLButtonDown(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnLButtonDown(x,y);
}

void	CFolderMgr::OnLButtonUp(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnLButtonUp(x,y);
}

void	CFolderMgr::OnLButtonDblClick(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnLButtonDblClick(x,y);
}

void	CFolderMgr::OnRButtonDown(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnRButtonDown(x,y);
}

void	CFolderMgr::OnRButtonUp(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnRButtonUp(x,y);
}

void	CFolderMgr::OnRButtonDblClick(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnRButtonDblClick(x,y);
}

void	CFolderMgr::OnMouseMove(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnMouseMove(x,y);
}

void	CFolderMgr::OnMouseWheel(int x, int y, int delta)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnMouseWheel(x,y,delta);
}


void CFolderMgr::AddFolder(eFolderID folderID)
{
    CBaseFolder* pFolder = LTNULL;
	switch (folderID)
	{

	case FOLDER_ID_MAIN:
		pFolder = new CFolderMain;
		break;

	case FOLDER_ID_SINGLE:
		pFolder = new CFolderSingle;
		break;

	case FOLDER_ID_CUSTOM_LEVEL:
		pFolder = new CFolderCustomLevel;
		break;

	case FOLDER_ID_OPTIONS:
		pFolder = new CFolderOptions;
		break;

	case FOLDER_ID_DISPLAY:
		pFolder = new CFolderDisplay;
		break;

	case FOLDER_ID_AUDIO:
		pFolder = new CFolderAudio;
		break;

	case FOLDER_ID_CUST_CONTROLS:
		pFolder = new CFolderCustomControls;
		break;

	case FOLDER_ID_MOUSE:
		pFolder = new CFolderMouse;
		break;

	case FOLDER_ID_KEYBOARD:
		pFolder = new CFolderKeyboard;
		break;

	case FOLDER_ID_JOYSTICK:
		pFolder = new CFolderJoystick;
		break;

	// AVP Species menues
	case FOLDER_ID_MARINE_MISSIONS:
		pFolder = new CFolderCampaignLevel(Marine);
		break;

	case FOLDER_ID_PREDATOR_MISSIONS:
		pFolder = new CFolderCampaignLevel(Predator);
		break;

	case FOLDER_ID_ALIEN_MISSIONS:
		pFolder = new CFolderCampaignLevel(Alien);
		break;

	case FOLDER_ID_PROFILE:
		pFolder = new CFolderProfile();
		break;

	case FOLDER_ID_ESCAPE:
		pFolder = new CFolderEscape;
		break;

	case FOLDER_ID_SAVE:
		pFolder = new CFolderSave;
		break;

	case FOLDER_ID_LOAD:
		pFolder = new CFolderLoad;
		break;

	case FOLDER_ID_GAME:
		pFolder = new CFolderGame;
		break;

	case FOLDER_ID_CREDITS:
		pFolder = new CFolderCredits;
		break;

	case FOLDER_ID_PERFORMANCE:
		pFolder = new CFolderPerformance;
		break;

	case FOLDER_ID_MULTI:
		pFolder = new CFolderMulti;
		break;

	case FOLDER_ID_HOST:
		pFolder = new CFolderHost;
		break;

	case FOLDER_ID_HOST_OPTS:
		pFolder = new CFolderHostOptions;
		break;

	case FOLDER_ID_HOST_MAPS:
		pFolder = new CFolderHostMaps;
		break;

	case FOLDER_ID_HOST_CONFIG:
		pFolder = new CFolderHostConfig;
		break;

	case FOLDER_ID_JOIN:
		pFolder = new CFolderJoin;
		break;

	case FOLDER_ID_JOIN_INFO:
		pFolder = new CFolderJoinInfo;
		break;

	case FOLDER_ID_PLAYER:
		pFolder = new CFolderPlayer;
		break;

	case FOLDER_ID_PLAYER_JOIN:
		pFolder = new CFolderPlayerJoin;
		break;

	case FOLDER_ID_HOST_SETUP_DM:
		pFolder = new CFolderSetupDM;
		break;

	case FOLDER_ID_HOST_SETUP_TEAMDM:
		pFolder = new CFolderSetupTeamDM;
		break;

	case FOLDER_ID_HOST_SETUP_HUNT:
		pFolder = new CFolderSetupHunt;
		break;

	case FOLDER_ID_HOST_SETUP_SURVIVOR:
		pFolder = new CFolderSetupSurvivor;
		break;

	case FOLDER_ID_HOST_SETUP_OVERRUN:
		pFolder = new CFolderSetupOverrun;
		break;

	case FOLDER_ID_HOST_SETUP_EVAC:
		pFolder = new CFolderSetupEvac;
		break;
	}

	if (pFolder)
	{
		pFolder->Init(folderID);
		m_folderArray.Add(pFolder);
	}

}

LTBOOL CFolderMgr::ForceFolderUpdate(eFolderID folderID)
{
    if (!m_pCurrentFolder || m_eCurrentFolderID != folderID) return LTFALSE;

	return m_pCurrentFolder->HandleForceUpdate();
}


void CFolderMgr::ClearFolderHistory()
{
	m_nHistoryLen = 0;
	m_eFolderHistory[0] = FOLDER_ID_NONE;
}


void CFolderMgr::AddToFolderHistory(eFolderID nFolder)
{
	m_eFolderHistory[m_nHistoryLen++] = nFolder;
}


