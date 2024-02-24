// FolderJoin.h: interface for the CFolderJoin class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_JOIN_H_
#define _FOLDER_JOIN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "GameSpyClientMgr.h"
#include "MultiplayerMgrDefs.h"


#define FOLDERJOIN_NOCHANGE			0
#define FOLDERJOIN_LANONLY			1
#define FOLDERJOIN_INTERNET			2

#define FOLDERJOIN_NOREFRESH		0
#define FOLDERJOIN_REFRESH			1


class CFolderJoin : public CBaseFolder
{
	public:

		CFolderJoin();
		virtual ~CFolderJoin();


		// Build the folder
		LTBOOL  Build();
		void	OnFocus(LTBOOL bFocus);
		void	Escape();

		LTBOOL  Render(HSURFACE hDestSurf);

		LTBOOL	OnLeft();
		LTBOOL	OnRight();
		LTBOOL	OnPageUp();
		LTBOOL	OnPageDown();
		LTBOOL	OnMouseWheel(int x, int y, int delta);

		LTBOOL	HandleKeyDown(int key, int rep);

		void	SetLANOnly(LTBOOL bLANOnly)				{ m_bLANOnly = bLANOnly; }
		void	SetRefreshOnFocus(LTBOOL bRefresh)		{ m_bRefreshOnFocus = bRefresh; }

		CListCtrl*	GetServerList()						{ return m_pServerList; }

		CGameSpyServer* GetSelectedServer();
		void	JoinServer(CGameSpyServer *pGame, const char *szPassword = LTNULL);

		CGameSpyClientMgr* GetGameSpyMgr();

	protected:
 
		uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
		void	ForceNextUpdate() {m_bForceUpdate = LTTRUE;}
		void	UpdateServers(LTBOOL bForce);
		void	JoinSpecificIP();
		void	AddServerCtrl(CGameSpyServer* pGame);
		void	SortServers();
		void	SetStatusText(HSTRING hStr);
		void	SetDummyStatusState(HSTRING hStr, DWORD dwWaitTime, int nNextState);
		void	SetState(int nNewState);

		void	Update(HSURFACE hDestSurf);
		void	UpdateGetServices(HSURFACE hDestSurf);
		void	UpdateGetPings(HSURFACE hDestSurf);
		void	UpdateIdle(HSURFACE hDestSurf);
		void	UpdateDummyStatus(HSURFACE hDestSurf);

		void	BuildSpecificIPDlg();
		void	ShowSpecificIPDlg();
		void	HideSpecificIPDlg();

		void	UpdateIPText();

		void	SetUpdateControls(LTBOOL bEnabled);

		void	RefreshServers();


		CListCtrl*			m_pServerList;

		CLTGUITextItemCtrl*	m_pServerListTitle;
		CLTGUITextItemCtrl*	m_pFilterTitle;

		CLTGUITextItemCtrl*	m_pJoinIP;

		CLTGUITextItemCtrl*	m_pPopFilter;
		CLTGUITextItemCtrl*	m_pVersionFilter;
		CLTGUITextItemCtrl*	m_pTypeFilter;
		CLTGUITextItemCtrl*	m_pPingFilter;
		CLTGUITextItemCtrl*	m_pDedicatedFilter;

		CLTGUITextItemCtrl*	m_pSortName;
		CLTGUITextItemCtrl*	m_pSortPing;
		CLTGUITextItemCtrl*	m_pSortNum;
		CLTGUITextItemCtrl*	m_pSortType;
		CLTGUITextItemCtrl*	m_pSortMap;
		CFrameCtrl*			m_pSortFrame;

		char				m_szIP[24];
		char				m_szPort[7];

		CStaticTextCtrl*	m_pJSIPTitle;
		CLTGUIEditCtrl*		m_pJSIPIPEdit;
		CLTGUIEditCtrl*		m_pJSIPPortEdit;
		CLTGUIEditCtrl*		m_pJSIPPassword;
		CGroupCtrl*			m_pJSIPDlgGroup;

		CLTGUITextItemCtrl*	m_pIPCtrl;

		LTBOOL		m_bEscape;

		int			m_nPopFilter;
		int			m_nVersionFilter;
		int			m_nTypeFilter;
		int			m_nPingFilter;
		int			m_nDedicatedFilter;

		LTBOOL		m_bLANOnly;
		LTBOOL		m_bRefreshOnFocus;

		LTBOOL		m_bForceUpdate;
		LTBOOL		m_bNeedServerSorting;
		uint32		m_timeDummyEnd;
		HSTRING		m_hServersShown;
		HSTRING		m_hDummyStatus;
		HSTRING		m_hStatus;
		int			m_nNextDummyState;
		int			m_nNumServers;
		int			m_nNumServersListed;
		int			m_nState;
		int			m_nServerSort;

		HSURFACE	m_hLock;
};

#endif // _FOLDER_JOIN_H_

