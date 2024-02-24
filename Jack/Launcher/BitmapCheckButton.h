/*******************************************************************************
;
;	MODULE:			BITMAPCHECKBUTTON (.H)
;
;	PURPOSE:		Bitmap check button class (derived from CButton)
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_BITMAPCHECKBUTTON_H__DA726A41_B58B_11D4_A2BA_00010229388A__INCLUDED_)
#define AFX_BITMAPCHECKBUTTON_H__DA726A41_B58B_11D4_A2BA_00010229388A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BitmapCheckButton.h : header file
//

#include "StaticEx.h"

/////////////////////////////////////////////////////////////////////////////
// CBitmapCheckButton window

class CBitmapCheckButton : public CButton
{
// Construction
public:
	CBitmapCheckButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBitmapCheckButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBitmapCheckButton();
	void SetHelpTextCtrl(CStaticEx *ctrl, const char *szText) { m_HelpTextCtrl = ctrl; m_csHelpText = szText; }

	CBitmap m_bmpNormal;			// Normal image
	CBitmap m_bmpChecked;			// Checked image
	CBitmap m_bmpDisabled;			// Disabled image
	CBitmap m_bmpCheckedDisabled;	// Checked/disabled image
	CBitmap m_bmpFocus;				// Focus image (for rollover)

	BOOL	AutoLoad(UINT nID, CWnd* pParent);
	BOOL	LoadBitmaps(LPCTSTR lpszNormal, LPCTSTR lpszChecked,
				LPCTSTR lpszDisabled = NULL, LPCTSTR lpszCheckedDisabled = NULL,
				LPCTSTR lpszFocus = NULL);
	
	BOOL GetCheck() { return m_bChecked; }
	void SetCheck(BOOL bCheck) { m_bChecked = bCheck; }
	void SizeToContent();

	// Generated message map functions
protected:
	//{{AFX_MSG(CBitmapCheckButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	BOOL	m_bChecked;
	CStaticEx *m_HelpTextCtrl;
	CString m_csHelpText;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPCHECKBUTTON_H__DA726A41_B58B_11D4_A2BA_00010229388A__INCLUDED_)
