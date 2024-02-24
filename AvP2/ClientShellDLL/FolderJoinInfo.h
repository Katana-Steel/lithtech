// FolderJoinInfo.h: interface for the CFolderJoinInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_JOIN_INFO_H_
#define _FOLDER_JOIN_INFO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "GameSpyClientMgr.h"

class CFolderJoinInfo : public CBaseFolder
{
	public:

		CFolderJoinInfo();
		virtual ~CFolderJoinInfo();

		// Build the folder
		LTBOOL  Build();

		void	Escape();
		void	OnFocus(LTBOOL bFocus);

		void	SetGameSpyInfo(CGameSpyClientMgr *pGSCM, CGameSpyServer *pServer)
					{ m_pGameSpyClientMgr = pGSCM; m_pServer = pServer; }

	protected:

		void	BuildGeneralString(std::string &szGeneral);
		void	BuildSetupString(std::string &szSetup);
		void	BuildAdvancedString(std::string &szAdvanced);

		CGameSpyClientMgr*	m_pGameSpyClientMgr;
		CGameSpyServer*		m_pServer;

		CStaticTextCtrl		*m_pGeneral;
		CStaticTextCtrl		*m_pSetup;
		CStaticTextCtrl		*m_pAdvanced;
};

#endif // _FOLDER_JOIN_INFO_H_

