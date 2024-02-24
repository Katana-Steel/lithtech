// FolderSingle.cpp: implementation of the CFolderSingle class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderSingle.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "ProfileMgr.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSingle::CFolderSingle()
{
    m_pLoad = LTNULL;

	m_LastCharacter = -1;
	memset(m_pModels,0,sizeof(m_pModels));

}

CFolderSingle::~CFolderSingle()
{

}

// Build the folder
LTBOOL CFolderSingle::Build()
{

	CreateTitle(IDS_TITLE_SINGLE);

	m_pLoad = AddLink(IDS_LOADGAME,	FOLDER_CMD_LOAD_GAME, IDS_HELP_LOAD);

	// Add AVP Stuff
	CLTGUITextItemCtrl *pCtrl = AddLink(IDS_MARINE, FOLDER_CMD_MARINE_MODE, IDS_MARINE_HELP);
	pCtrl->SetParam1(1);

	pCtrl = AddLink(IDS_PREDATOR, FOLDER_CMD_PREDATOR_MODE, IDS_PREDATOR_HELP);
	pCtrl->SetParam1(2);

//#ifdef _DEMO
//	pCtrl->Enable(LTFALSE);
//#endif

	pCtrl = AddLink(IDS_ALIEN, FOLDER_CMD_ALIEN_MODE, IDS_ALIEN_HELP);
	pCtrl->SetParam1(3);

//#ifdef _DEMO
//	pCtrl->Enable(LTFALSE);
//#endif

	memset(m_FXNames,0,sizeof(m_FXNames));
	g_pLayoutMgr->GetFolderCustomString(FOLDER_ID_SINGLE,"MarineFX",m_FXNames[0],128);
	g_pLayoutMgr->GetFolderCustomString(FOLDER_ID_SINGLE,"PredatorFX0",m_FXNames[1],128);
	g_pLayoutMgr->GetFolderCustomString(FOLDER_ID_SINGLE,"PredatorFX1",m_FXNames[3],128);
	g_pLayoutMgr->GetFolderCustomString(FOLDER_ID_SINGLE,"AlienFX",m_FXNames[2],128);

#ifndef _DEMO
	AddLink(IDS_WARP, FOLDER_CMD_CUSTOM_LEVEL, IDS_HELP_CUSTOM);
#endif

 	// Make sure to call the base class
	return CBaseFolder::Build();

}

uint32 CFolderSingle::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_LOAD_GAME:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_LOAD);
			break;
		}
	case FOLDER_CMD_CUSTOM_LEVEL:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_CUSTOM_LEVEL);
			break;
		}
	case FOLDER_CMD_MARINE_MODE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MARINE_MISSIONS);
			break;
		}
	case FOLDER_CMD_PREDATOR_MODE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PREDATOR_MISSIONS);
			break;
		}
	case FOLDER_CMD_ALIEN_MODE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_ALIEN_MISSIONS);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CFolderSingle::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32];

		sprintf (strKey, "Reload");
		char strIniName[128];
		sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, strIniName);

#ifdef _DEMO
		m_pLoad->Enable(LTFALSE);
#else
		m_pLoad->Enable( (strlen (strSaveGameSetting) > 0) );
#endif
	}
	else
	{
		for (int i = 0; i < kMaxModels; i++)
		{
			if (m_pModels[i])
			{
				m_pModels[i]->Term();
				g_pInterfaceMgr->RemoveInterfaceSFX(m_pModels[i]);
				m_pModels[i] = LTNULL;
			}
		}
		m_LastCharacter = -1;
	}
	CBaseFolder::OnFocus(bFocus);
}



/******************************************************************/
LTBOOL CFolderSingle::Render(HSURFACE hDestSurf)
{
	// Which button is selected?
	CLTGUICtrl *pCtrl = GetSelectedControl();
	// Make sure there is a selected button
	if (pCtrl)
	{
		// decrement one for the load button
		int nModel = pCtrl->GetParam1() - 1;
		if (nModel < 0 || nModel > 2)
		{
			// Turn off model and set m_LastCharacter to -1
			for (int i = 0; i < kMaxModels; i++)
			{
				if (m_pModels[i])
				{
					m_pModels[i]->Term();
					g_pInterfaceMgr->RemoveInterfaceSFX(m_pModels[i]);
					m_pModels[i] = LTNULL;
				}
			}
			m_LastCharacter = -1;
		}
		else
		{
			// See if we need an update
			if (nModel != m_LastCharacter)
			{
				// Turn off model
				for (int i = 0; i < kMaxModels; i++)
				{
					if (m_pModels[i])
					{
						m_pModels[i]->Term();
						g_pInterfaceMgr->RemoveInterfaceSFX(m_pModels[i]);
						m_pModels[i] = LTNULL;
					}
				}

				// Make a new one
				switch (nModel)
				{
					case 0:
						m_pModels[0] = CreateScaleFX(m_FXNames[0]);
						break;

					case 1:
						m_pModels[0] = CreateScaleFX(m_FXNames[1]);
						m_pModels[1] = CreateScaleFX(m_FXNames[3]);
						break;

					case 2:
						m_pModels[0] = CreateScaleFX(m_FXNames[2]);
						break;
				}

				// Give them some chrome
				if(m_pModels[0])
				{
					HOBJECT hObj = m_pModels[0]->GetObject();
					uint32 nFlags;

					g_pLTClient->Common()->GetObjectFlags(hObj, OFT_Flags, nFlags);
					g_pLTClient->Common()->SetObjectFlags(hObj, OFT_Flags, nFlags | FLAG_ENVIRONMENTMAP);
				}

				// Update our tracker.
				m_LastCharacter = nModel;
			}
		}
	}
	// Now do the real stuff
	return CBaseFolder::Render(hDestSurf);
}

