/*******************************************************************************
;
;	MODULE:			MULTIPLAYERDLG (.H)
;
;	PURPOSE:		Multiplayer dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_MULTIPLAYERDLG_H__126D5682_B3F2_11D4_A2BA_00010229388A__INCLUDED_)
#define AFX_MULTIPLAYERDLG_H__126D5682_B3F2_11D4_A2BA_00010229388A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiplayerDlg.h : header file
//

#include "AnimDlg.h"

/////////////////////////////////////////////////////////////////////////////
// MultiplayerDlg dialog

class CMultiplayerDlg : public CAnimDlg
{
// Construction
public:
	CMultiplayerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMultiplayerDlg)
	enum { IDD = IDD_MULTIPLAYER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiplayerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMultiplayerDlg)
	afx_msg void OnFind();
	afx_msg void OnHost();
	afx_msg void OnJoin();
	afx_msg void OnServer();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnMpBack();
	afx_msg void OnMpClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIPLAYERDLG_H__126D5682_B3F2_11D4_A2BA_00010229388A__INCLUDED_)
