/*******************************************************************************
;
;	MODULE:			LAUNCHERDLG (.H)
;
;	PURPOSE:		Launcher main dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_LAUNCHERDLG_H__1E6712C9_B0F3_11D4_A2BA_00010229388A__INCLUDED_)
#define AFX_LAUNCHERDLG_H__1E6712C9_B0F3_11D4_A2BA_00010229388A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MoveDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CLauncherDlg dialog

class CLauncherDlg : public CMoveDialog
{
// Construction
public:
	CLauncherDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CLauncherDlg)
	enum { IDD = IDD_LAUNCHER_DIALOG };
//	CAnimateCtrl	m_aniButtons;
//	CAnimateCtrl	m_aniMain;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLauncherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	HICON			m_hIcon;
	BOOL			LaunchGame(DWORD dwAction);
	// Generated message map functions
	//{{AFX_MSG(CLauncherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnMinimize();
	afx_msg void OnPlay();
	afx_msg void OnMultiplayer();
	afx_msg void OnOptions();
	afx_msg void OnDisplay();
	afx_msg void OnUninstall();
	afx_msg void OnQuit();
	afx_msg void OnDestroy();
	afx_msg void OnCompanyWeb();
	afx_msg void OnServer();
	afx_msg void OnPublisherweb();
	afx_msg void OnLithtechWeb();
	afx_msg void OnSierraWeb();
	afx_msg void OnCustomize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAUNCHERDLG_H__1E6712C9_B0F3_11D4_A2BA_00010229388A__INCLUDED_)
