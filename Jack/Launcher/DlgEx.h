/*******************************************************************************
;
;	MODULE:			DLGEX (.H)
;
;	PURPOSE:		Extended dialog class (derived from CDialog) that has
;					a bitmap as a background
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_DLGEX_H__9EBE7C50_295C_4466_94B6_B2AC375EC962__INCLUDED_)
#define AFX_DLGEX_H__9EBE7C50_295C_4466_94B6_B2AC375EC962__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgEx.h : header file
//

#include "AfxTempl.h"

typedef CArray<CButton*, CButton*> CButtonArray;

class CStaticEx;

/////////////////////////////////////////////////////////////////////////////
// CDlgEx dialog

class CDialogEx : public CDialog
{
// Construction
public:
	CDialogEx(int IDD, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogEx() { FreeAllButtons(); }
	BOOL SetBackGround(int nID);
	void PaintBackGround(CPaintDC * dc);
	void FreeAllButtons();

	CButton*	AddButton(UINT nID, int x, int y, UINT nCursorID = 0, CStaticEx* pHelpText = NULL, DWORD dwHelpTextID = 0);
	CButton*	AddCheckBox(UINT nID, int x, int y, CStaticEx* pHelpText = NULL, DWORD dwHelpTextID = 0);

	CButton*	AddTextCheckBox(UINT nID, int x, int y, UINT nTextID = 0, CStaticEx* pHelpText = NULL, DWORD dwHelpTextID = 0);
	CButton*	AddTextCheckBox(UINT nID, int x, int y, const char *szText = NULL, CStaticEx* pHelpText = NULL, DWORD dwHelpTextID = 0);

//	CButton*	AddAniButton( UINT nID, int x, int y, UINT nAniID );
	
	void		SetFocusButton(CButton* pButton);
	CButton*	GetFocusButton() { return m_pFocusButton; }

// Dialog Data
	//{{AFX_DATA(CDialogEx)
	enum { IDD = 0 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDialogEx)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_VIRTUAL

// Implementation
protected:

	CButtonArray	m_collButtons;		// The list of buttons
	CBitmap			m_Background;		// The background image
	CButton*		m_pFocusButton;		// Which button has the focus?

	// Generated message map functions
	//{{AFX_MSG(CDialogEx)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGEX_H__9EBE7C50_295C_4466_94B6_B2AC375EC962__INCLUDED_)
