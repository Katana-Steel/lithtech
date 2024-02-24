// FolderCampaignLevel.h: interface for the CFolderCustomLevel class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FOLDER_PROFILE_H
#define FOLDER_PROFILE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

#include "profileMgr.h"

#pragma warning (disable : 4503)
#include <vector>
#include <string>

const int MAX_PROFILE_LEN = 16;

class CFolderProfile : public CBaseFolder
{
public:
	
	CFolderProfile();
	virtual ~CFolderProfile();

	// Build the folder
    LTBOOL   Build();

    virtual uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void			Escape();
	void			OnFocus(LTBOOL bFocus);
	LTBOOL			OnRButtonUp(int x, int y);
	LTBOOL			OnLButtonUp(int x, int y);

private:
	void	CreateProfileList();
	void	UpdateProfileName();

	void	BuildDlg();
	void	ShowDlg();
	void	HideDlg(LTBOOL bOK);

	char	m_szProfile[MAX_PROFILE_LEN];
	char	m_szOldProfile[MAX_PROFILE_LEN];

	CLTGUIEditCtrl		*m_pEdit;
	CLTGUITextItemCtrl	*m_pCurrent;
	CLTGUITextItemCtrl	*m_pLoad;
	CLTGUITextItemCtrl	*m_pDelete;
	CLTGUITextItemCtrl	*m_pRename;
	CFrameCtrl*			m_pDlgBack;
	CGroupCtrl*			m_pDlgGroup;


	StringSet			m_ProfileList;
	CListCtrl*			m_pListCtrl;

	CUserProfile*		m_pProfile;

};

#endif // FOLDER_PROFILE_H