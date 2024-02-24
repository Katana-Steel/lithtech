// FolderMulti.h: interface for the CFolderMulti class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_MULTI_H_
#define _FOLDER_MULTI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"
#include "GameSpyClientMgr.h"


#define CDKEY_STRING_SIZE			64
#define MOTD_STRING_SIZE			512


class CFolderMulti : public CBaseFolder
{
	public:

		CFolderMulti();
		virtual ~CFolderMulti();

		LTBOOL	Build();
		void	OnFocus(LTBOOL bFocus);

		void	Escape();
		LTBOOL	OnEnter();
		LTBOOL	OnUp();
		LTBOOL	OnDown();

		LTBOOL	HandleKeyDown(int key, int rep);

		LTBOOL  Render(HSURFACE hDestSurf);

		void	SetLAN(LTBOOL bLAN)				{ m_bLAN = bLAN; }

	public:

		CGameSpyClientMgr* GetGameSpyMgr()		{ return &m_GameSpyClientMgr; }

		void	JoinGameSpyArcadeServer();

	protected:

		uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

		LTBOOL	BuildDialog_CDK();
		LTBOOL	SetDialogText_CDK(uint32 nTitle, uint32 nDesc);
		LTBOOL	ShowDialog_CDK(LTBOOL bShow);

		LTBOOL	BuildDialog_MOTD();
		LTBOOL	SetDialogText_MOTD(uint32 nTitle, char *szDesc);
		LTBOOL	ShowDialog_MOTD(LTBOOL bShow);

		LTBOOL	BuildDialog_PRG();
		LTBOOL	SetDialogText_PRG(uint32 nTitle, uint32 nDesc);
		LTBOOL	ShowDialog_PRG(LTBOOL bShow);

		LTBOOL	Update_PRG();

		LTBOOL	BuildDialog_DCS();
		LTBOOL	SetDialogText_DCS(uint32 nTitle, uint32 nDesc, char *szDesc = LTNULL);
		LTBOOL	ShowDialog_DCS(LTBOOL bShow, uint8 nFlags = 0);

		LTBOOL	LaunchSierraUp();

	protected:

		// Menu controls
		CLTGUITextItemCtrl	*m_pHost;
		CLTGUITextItemCtrl	*m_pHostLAN;
		CLTGUITextItemCtrl	*m_pJoinNet;
		CLTGUITextItemCtrl	*m_pJoinLAN;
		CLTGUITextItemCtrl	*m_pUpdateCDK;
		CLTGUITextItemCtrl	*m_pMain;

		CStaticTextCtrl		*m_pCurCDKey;

		CCycleCtrl			*m_pJoinSpeed;
		int					m_nConnection;


		// Dialog box controls (CD Key Input)
		CGroupCtrl			*m_pCDKDialog;
		CStaticTextCtrl		*m_pCDKTitle;
		CStaticTextCtrl		*m_pCDKDesc;
		CLTGUIEditCtrl		*m_pCDKEdit;


		// Dialog box controls (Message of the day)
		CGroupCtrl			*m_pMOTDDialog;
		CStaticTextCtrl		*m_pMOTDTitle;
		CBitmapCtrl			*m_pMOTDUp;
		CBitmapCtrl			*m_pMOTDDown;
		CLTGUITextItemCtrl	*m_pMOTDOk;
		HSURFACE			m_hMOTDDesc;

		// Message of the day description display information
		int					m_nMOTDLines;
		int					m_nMOTDDispLines;
		int					m_nMOTDCurLine;

		// Message of the day buffers
		char				m_szMOTDSys[MOTD_STRING_SIZE];
		char				m_szMOTDGame[MOTD_STRING_SIZE];


		// Dialog box controls (Progress)
		CGroupCtrl			*m_pPRGDialog;
		CStaticTextCtrl		*m_pPRGTitle;
		CStaticTextCtrl		*m_pPRGDesc;
		CStaticTextCtrl		*m_pPRGUpdate;

		// Misc variables needed for the progress
		LTFLOAT				m_fProgessStart;
		int					m_nCurProgress;


		// Dialog box controls (Decision)
		CGroupCtrl			*m_pDCSDialog;
		CStaticTextCtrl		*m_pDCSTitle;
		CStaticTextCtrl		*m_pDCSDesc;
		CLTGUITextItemCtrl	*m_pDCSOk;
		CLTGUITextItemCtrl	*m_pDCSCancel;

		// Misc variables needed for the progress
		uint8				m_nDCSType;
		LTBOOL				m_bDecisionOk;


		// The main access to the WON API... yes, yes I know it's called 'GameSpy'!
		CGameSpyClientMgr	m_GameSpyClientMgr;


		// Some state variables for the menu so we can pace everything
		int					m_nDialogState;
		int					m_nVerifyState;


		// Determine if we're using LAN connections
		LTBOOL				m_bLAN;

		// The current CD key value
		char				m_szCDK[CDKEY_STRING_SIZE];
};

#endif // _FOLDER_MULTI_H_

