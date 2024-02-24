/*******************************************************************************
;
;	MODULE:			BUTTONEX (.H)
;
;	PURPOSE:		Extended button class (derived from CBitmapButton)
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_BUTTONEX_H__5DC081D4_42E2_41F1_839D_89EE9366D0FF__INCLUDED_)
#define AFX_BUTTONEX_H__5DC081D4_42E2_41F1_839D_89EE9366D0FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ButtonEx.h : header file
//

#include "StaticEx.h"

/////////////////////////////////////////////////////////////////////////////
// CButtonEx window

class CButtonEx : public CBitmapButton
{
// Construction
public:
	CButtonEx();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButtonEx)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButtonEx();

	void SetHelpTextCtrl(CStaticEx *ctrl, const char *szText) { m_HelpTextCtrl = ctrl; m_csHelpText = szText; }

	void SetHCursor( HCURSOR hCursor ) { m_hCursor = hCursor; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CButtonEx)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	
	CStaticEx	*m_HelpTextCtrl;	// Static text control (if any) linked to this button
	CString		m_csHelpText;		// Text for the above control

	HCURSOR		m_hCursor;			// Cursor for this Button	
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTTONEX_H__5DC081D4_42E2_41F1_839D_89EE9366D0FF__INCLUDED_)
