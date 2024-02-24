// FolderHostConfig.h: interface for the CFolderHostConfig class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FOLDER_CONFIG_H
#define FOLDER_CONFIG_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "ServerOptionMgr.h"

#pragma warning (disable : 4503)
#include <vector>
#include <string>

const int MAX_CONFIG_LEN = 16;

class CFolderHostConfig : public CBaseFolder
{
public:
	
	CFolderHostConfig();
	virtual ~CFolderHostConfig();

	// Build the folder
    LTBOOL   Build();

    virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void			Escape();
	void			OnFocus(LTBOOL bFocus);
	LTBOOL			OnRButtonUp(int x, int y);
	LTBOOL			OnLButtonUp(int x, int y);

private:
	void	CreateConfigList();
	void	UpdateConfigName();

	void	BuildDlg();
	void	ShowDlg();
	void	HideDlg(LTBOOL bOK);

	char	m_szConfig[MAX_CONFIG_LEN];
	char	m_szOldConfig[MAX_CONFIG_LEN];

	CLTGUIEditCtrl		*m_pEdit;
	CLTGUITextItemCtrl	*m_pCurrent;
	CLTGUITextItemCtrl	*m_pLoad;
	CLTGUITextItemCtrl	*m_pDelete;
	CLTGUITextItemCtrl	*m_pRename;
	CFrameCtrl*			m_pDlgBack;
	CGroupCtrl*			m_pDlgGroup;


	StringSet			m_ConfigNames;
	CListCtrl*			m_pListCtrl;

	CServerOptions*		m_pConfig;
};

#endif // FOLDER_CONFIG_H