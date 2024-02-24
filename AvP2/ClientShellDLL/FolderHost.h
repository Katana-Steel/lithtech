// FolderHost.h: interface for the CFolderHost class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_HOST_H_
#define _FOLDER_HOST_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "ServerOptionMgr.h"

class CFolderHost : public CBaseFolder
{
	public:

		CFolderHost();
		virtual ~CFolderHost();

		// Build the folder
		LTBOOL	Build();

		void	Escape();
		LTBOOL	OnEnter();
		void	OnFocus(LTBOOL bFocus);
		void	FocusSetup(LTBOOL bFocus);

		LTBOOL	OnRButtonUp(int x, int y);
		LTBOOL	OnLButtonUp(int x, int y);

		void	SetPassCheck(LTBOOL bOn);

	protected:

		uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

		void	BuildSetupString(std::string &rules);
		void	BuildAdvancedString(std::string &rules);

		CGroupCtrl			*m_pNameGroup;
		CLTGUIEditCtrl		*m_pEdit;
		CLTGUITextItemCtrl	*m_pLabel;

		CGroupCtrl			*m_pPassGroup;
		CLTGUIEditCtrl		*m_pPassEdit;
		CLTGUITextItemCtrl	*m_pPassLabel;

		CGroupCtrl			*m_pSCPassGroup;
		CLTGUIEditCtrl		*m_pSCPassEdit;
		CLTGUITextItemCtrl	*m_pSCPassLabel;

		CStaticTextCtrl		*m_pSetup;
		CStaticTextCtrl		*m_pAdvanced;

		CBitmapCtrl			*m_pPassCheck;

		CServerOptions		m_Options;
		char				m_szOldName[32];
		char				m_szOldPass[16];
		char				m_szOldSCPass[16];

		LTBOOL				m_bUsePassword;

		int					m_nType;
		int					m_nMaxPlayers;
};

#endif // _FOLDER_HOST_H_

