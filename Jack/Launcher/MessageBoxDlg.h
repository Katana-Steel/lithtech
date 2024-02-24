/*******************************************************************************
;
;	MODULE:			MESSAGEBOXDLG (.H)
;
;	PURPOSE:		Message box dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_MESSAGEBOXDLG_H__52E7EEB8_B979_11D4_A2BA_00010229388A__INCLUDED_)
#define AFX_MESSAGEBOXDLG_H__52E7EEB8_B979_11D4_A2BA_00010229388A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MessageBoxDlg.h : header file
//

#include "MoveDlg.h"
#include "StaticEx.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageBoxDlg dialog

class CMessageBoxDlg : public CMoveDialog
{
// Construction
public:
	CMessageBoxDlg(CWnd* pParent = NULL);   // standard constructor
	void Init(const char *szText, const char *szCaption, DWORD dwStyle = MB_OK)
				{ m_csCaption = szCaption; m_csText = szText; m_dwStyle = dwStyle; }

// Dialog Data
	//{{AFX_DATA(CMessageBoxDlg)
	enum { IDD = IDD_MESSAGEBOX };
	CStaticEx	m_Text;
	CStaticEx	m_Caption;
	CStaticEx	m_Icon;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessageBoxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMessageBoxDlg)
	afx_msg void OnCancel();
	afx_msg void OnOk();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	DWORD m_dwStyle;
	CString m_csCaption;
	CString m_csText;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESSAGEBOXDLG_H__52E7EEB8_B979_11D4_A2BA_00010229388A__INCLUDED_)
