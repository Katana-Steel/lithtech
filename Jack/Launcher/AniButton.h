#if !defined(AFX_ANIBUTTON_H__C8477D0C_24E5_4A69_96D5_D9803EFFA30E__INCLUDED_)
#define AFX_ANIBUTTON_H__C8477D0C_24E5_4A69_96D5_D9803EFFA30E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AniButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAniButton window

class CAniButton : public CButton
{
// Construction
public:
	CAniButton();

// Attributes
public:
	UINT			m_nAniID;		// ResourceID of the .avi

protected:
	CAnimateCtrl	m_AnimateCtrl;	// Animation control
	BOOL			m_bPlaying;		// Are we playing?

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAniButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAniButton();

	void	LoadAVI(UINT nAniID);
	BOOL	AutoLoad(UINT nID, CWnd* pParent);

	// Generated message map functions
protected:
	//{{AFX_MSG(CAniButton)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIBUTTON_H__C8477D0C_24E5_4A69_96D5_D9803EFFA30E__INCLUDED_)
