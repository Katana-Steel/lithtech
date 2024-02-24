/****************************************************************************
;
;	MODULE:			STATICEX (.h)
;
;	PURPOSE:		Extended static text control class (Derived from CStatic)
;
;	HISTORY:		11/21/2000 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Productions, Inc.
;
****************************************************************************/

#if !defined(STATIC_H_INCLUDED_)
#define STATIC_H_INCLUDED_

/////////////////////////////////////////////////////////////////////////////
// CStaticEx window
enum FlashType {None, Text, Background };

class CStaticEx : public CStatic
{
// Construction
public:
	CStaticEx();
	CStaticEx& SetBkColor(COLORREF crBkgnd);
	CStaticEx& SetTextColor(COLORREF crText);
	CStaticEx& SetText(const CString& strText);
	CStaticEx& SetFontBold(BOOL bBold);
	CStaticEx& SetFontName(const CString& strFont);
	CStaticEx& SetFontUnderline(BOOL bSet);
	CStaticEx& SetFontItalic(BOOL bSet);
	CStaticEx& SetFontSize(int nSize);
	CStaticEx& SetSunken(BOOL bSet);
	CStaticEx& SetBorder(BOOL bSet);
	CStaticEx& FlashText(BOOL bActivate);
	CStaticEx& FlashBackground(BOOL bActivate);
	CStaticEx& SetLink(BOOL bLink);
	CStaticEx& SetLinkCursor(HCURSOR hCursor);
	void		SetShadow(BOOL bSet) { m_bShadow = bSet; }

// Attributes
public:
protected:
	void ReconstructFont();
	COLORREF	m_crText;
	HBRUSH		m_hBrush;
	HBRUSH		m_hwndBrush;
	LOGFONT		m_lf;
	CFont		m_font;
	CString		m_strText;
	BOOL		m_bState;
	BOOL		m_bTimer;
	BOOL		m_bLink;
	FlashType	m_Type;
	HCURSOR		m_hCursor;
	BOOL		m_bShadow;
			
// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticEx)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif //!defined(STATIC_H_INCLUDED_)
