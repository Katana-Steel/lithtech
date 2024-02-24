#if !defined(AFX_TEXTCHECKBOX_H__818F0ED2_BF14_4021_A26A_52A778238139__INCLUDED_)
#define AFX_TEXTCHECKBOX_H__818F0ED2_BF14_4021_A26A_52A778238139__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextCheckBox.h : header file
//

#include "StaticEx.h"

/////////////////////////////////////////////////////////////////////////////
// CTextCheckBox window

class CTextCheckBox : public CButton
{
// Construction
public:
	CTextCheckBox();

// Attributes
public:
	

protected:
	COLORREF	m_crText;
	HBRUSH		m_hBrush;
	BOOL		m_bChecked;

	CBitmap		m_bmpNormal;			// basic state image
	CBitmap		m_bmpFocus;				// mouse over image
	CBitmap		m_bmpChecked;			// checked state image

	LOGFONT		m_lfNormal;				// basic state font

	CString		m_strWinText;			// window text

	CStaticEx	*m_HelpTextCtrl;
	CString		m_csHelpText;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextCheckBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTextCheckBox();

	void	SetHelpTextCtrl(CStaticEx *ctrl, const char *szText) { m_HelpTextCtrl = ctrl; m_csHelpText = szText; }
	void	SetWindowText( const char *szText) { m_strWinText = szText; }
	void	GetWindowText( CString &rString ) { rString = m_strWinText; }
	BOOL	AutoLoad(UINT nID, CWnd* pParent);

	void	SizeToContent( void );
	
	inline	void SetCheck(BOOL bCheck) { m_bChecked = bCheck; }
	inline	BOOL GetCheck() { return m_bChecked; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextCheckBox)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTCHECKBOX_H__818F0ED2_BF14_4021_A26A_52A778238139__INCLUDED_)
