// FolderCustomLevel.h: interface for the CFolderCustomLevel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_CUSTOM_LEVEL_H_
#define _FOLDER_CUSTOM_LEVEL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

#include <vector>
#include <string>

class CFolderCustomLevel : public CBaseFolder
{
public:
	CFolderCustomLevel();
	virtual ~CFolderCustomLevel();

	// Build the folder
    LTBOOL   Build();

    void    OnFocus(LTBOOL bFocus);

	int		CountDatFiles(FileEntry* pFiles);
	void	AddFilesToFilenames(FileEntry* pFiles, char* pPath);
	int		GetNumFiles()			{ return m_FileNames.size(); }

    LTBOOL   HandleKeyDown(int key, int rep);

private:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	typedef std::vector<std::string>	FileList;
	FileList							m_FileNames;

	// Bye bye
//	int					m_nFiles;
//	char**				m_pFilenames;

};

#endif // _FOLDER_CUSTOM_LEVEL_H_