// FolderOptions.cpp: implementation of the CFolderOptions class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderOptions.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
#include "GameSettings.h"


namespace
{
	int kGap = 0;
	int kWidth = 0;
	uint32 CMD_CREDITS = FOLDER_CMD_CUSTOM + 1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderOptions::CFolderOptions()
{
}

CFolderOptions::~CFolderOptions()
{

}

// Build the folder
LTBOOL CFolderOptions::Build()
{
	g_pLayoutMgr->GetFolderCustomString((eFolderID)m_nFolderID,"SelectSound",m_szSelectSound,sizeof(m_szSelectSound));

	CreateTitle(IDS_TITLE_OPTIONS);

	AddLink(IDS_CONTROLS, FOLDER_CMD_CONTROLS, IDS_HELP_CONTROLS);
	AddLink(IDS_GAME, FOLDER_CMD_GAME, IDS_HELP_GAME);
	AddLink(IDS_DISPLAY, FOLDER_CMD_DISPLAY, IDS_HELP_DISPLAY);
	AddLink(IDS_SOUND, FOLDER_CMD_AUDIO, IDS_HELP_SOUND);
	AddLink(IDS_CREDITS, CMD_CREDITS, IDS_HELP_CREDITS);


	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	return LTTRUE;
}

uint32 CFolderOptions::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if	(dwCommand == CMD_CREDITS)
	{
		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_CREDITS);
		return 1;
	}

	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};

char* CFolderOptions::GetSelectSound()	
{
	return m_szSelectSound;
}
