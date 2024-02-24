// FolderCustomLevel.cpp: implementation of the CFolderCustomLevel class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderCustomLevel.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"

#include <algorithm>

extern CGameClientShell* g_pGameClientShell;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderCustomLevel::CFolderCustomLevel()
{
}

CFolderCustomLevel::~CFolderCustomLevel()
{
}

// Build the folder
LTBOOL CFolderCustomLevel::Build()
{
	// Set the title's text
	CreateTitle(IDS_TITLE_CUSTOMLEVELS);

    LTBOOL bUseRetailLevels = LTFALSE;
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("EnableRetailLevels");

    if (hVar && g_pLTClient->GetVarValueFloat(hVar) == 1.0f)
	{
        bUseRetailLevels = LTTRUE;
	}

#ifndef _FINAL
	bUseRetailLevels = LTTRUE;
#endif


	// Get a list of level names and sort them alphabetically
    FileEntry* pFiles = g_pLTClient->GetFileList("\\");

	FileEntry* pWorldFiles = DNULL;
	FileEntry* pMultiFiles = DNULL;
	FileEntry* pTestFiles = DNULL;

	if (bUseRetailLevels)
	{
		pWorldFiles = g_pClientDE->GetFileList("Worlds");
	}
	else
	{
		pWorldFiles = g_pLTClient->GetFileList("Worlds\\Custom");
	}

	g_pClientDE->FreeFileList(pFiles);

	if (bUseRetailLevels)
	{
		AddFilesToFilenames(pWorldFiles, "Worlds\\");
		g_pClientDE->FreeFileList(pWorldFiles);
	}
	else
	{
		AddFilesToFilenames(pWorldFiles, "Worlds\\Custom\\");
		g_pClientDE->FreeFileList(pWorldFiles);
	}


	// Put the file names in order
	std::sort(m_FileNames.begin(), m_FileNames.end(), CaselessLesser() );

	CLTGUITextItemCtrl* pItem;
	for (uint32 i = 0; i < m_FileNames.size(); ++i)
	{
		pItem = AddTextItem(m_FileNames[i].c_str(), FOLDER_CMD_CUSTOM+i, IDS_HELP_CUSTOMLEVEL, LTFALSE);
	}


	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE);

	return LTTRUE;


}


void CFolderCustomLevel::OnFocus(LTBOOL bFocus)
{
	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderCustomLevel::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if( dwCommand >= FOLDER_CMD_CUSTOM )
	{
		const DWORD selectNum = dwCommand-FOLDER_CMD_CUSTOM;

		if( g_pInterfaceMgr->LoadCustomLevel(m_FileNames[selectNum].c_str()) )
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
	}
	else
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

	return 1;
};

int CFolderCustomLevel::CountDatFiles(FileEntry* pFiles)
{
	if (!pFiles) return 0;

	int nNum = 0;
	FileEntry* ptr = pFiles;
	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			if (strnicmp (&ptr->m_pBaseFilename[strlen(ptr->m_pBaseFilename) - 4], ".dat", 4) == 0) nNum++;
		}
		ptr = ptr->m_pNext;
	}

	return nNum;
}

void CFolderCustomLevel::AddFilesToFilenames(FileEntry* pFiles, char* pPath)
{
	if (!pFiles || !pPath) return;

	FileEntry* ptr = pFiles;
	std::string path = std::string(pPath);

	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			std::string fileName = std::string(ptr->m_pBaseFilename);

			std::string::size_type dot = fileName.find('.');
			if (std::string::npos != dot)
			{
				std::string baseName = fileName.substr(0,dot);
				std::string baseExt = fileName.substr(dot+1);

				if (stricmp(baseExt.c_str(), "dat") == 0)
				{
					std::string fullName = path + baseName;

					// add this to the array
					m_FileNames.push_back(fullName);
				}
			}
		}
		else if (ptr->m_Type == TYPE_DIRECTORY)
		{
			std::string path = pPath;
			path += ptr->m_pBaseFilename;
			path += "\\";

			FileEntry * pFiles = g_pClientDE->GetFileList( const_cast<char*>(path.c_str()) );
			AddFilesToFilenames(pFiles,const_cast<char*>(path.c_str()));
			g_pClientDE->FreeFileList(pFiles);
		}

		ptr = ptr->m_pNext;
	}
}


// Handles a key press.  Returns FALSE if the key was not processed through this method.
// Left, Up, Down, Right, and Enter are automatically passed through OnUp(), OnDown(), etc.
LTBOOL CFolderCustomLevel::HandleKeyDown(int key, int rep)
{
    LTBOOL handled = LTFALSE;

	switch (key)
	{
	case VK_LEFT:
		{
			handled = OnLeft();
			break;
		}
	case VK_RIGHT:
		{
			handled = OnRight();
			break;
		}
	case VK_UP:
		{
			handled = OnUp();
			break;
		}
	case VK_DOWN:
		{
			handled = OnDown();
			break;
		}
	case VK_RETURN:
		{
			handled = OnEnter();
			break;
		}
	case VK_TAB:
		{
			handled = OnTab();
			break;
		}
	case VK_PRIOR:
		{
			handled = OnPageUp();
			break;
		}
	case VK_NEXT:
		{
			handled = OnPageDown();
			break;
		}
	default:
		{
            handled = LTFALSE;
			break;
		}
	}

	// Handled the key
	return handled;
}

