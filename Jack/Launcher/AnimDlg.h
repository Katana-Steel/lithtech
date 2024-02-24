/*******************************************************************************
;
;	MODULE:			ANIMDLG (.H)
;
;	PURPOSE:		Animating dialog box class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_ANIMDLG_H__EC21F9C0_21CC_44A6_9460_16D645726576__INCLUDED_)
#define AFX_ANIMDLG_H__EC21F9C0_21CC_44A6_9460_16D645726576__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimDlg.h : header file
//

#include "DlgEx.h"

#define ANIM_RIGHT_DOWN		0
#define ANIM_RIGHT_UP		1
#define ANIM_LEFT_DOWN		2
#define ANIM_LEFT_UP		3

/////////////////////////////////////////////////////////////////////////////
// CAnimDlg dialog

class CAnimDlg : public CDialogEx
{
// Construction
public:
	CAnimDlg(int IDD, CWnd* pParent = NULL);   // standard constructor
	void SetDialogPos(int nLeft, int nTop) { m_nLeft = nLeft; m_nTop = nTop; }
	void SetAnimTime(DWORD dwTime) { m_dwAnimTime = dwTime; }
	void SetAnimSound(UINT nID) { m_nSoundID = nID; }

// Dialog Data
	//{{AFX_DATA(CAnimDlg)
	enum { IDD = 0 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	DWORD m_dwAnimTime;		// Time for animation to complete in milliseconds
	DWORD m_dwStartTime;	// When we started
	int m_nLeft;			// Current left position
	int m_nTop;				// Current top position
	CRect m_rcDlgSize;		// Size of dialog
	BOOL m_bAnimating;		// Are we animating?
	int m_nAnimDir;			// Animation direction
	DWORD m_dwTimeDelay;	// How often do we get updates?
	UINT m_nSoundID;		// Sound for when we init

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMDLG_H__EC21F9C0_21CC_44A6_9460_16D645726576__INCLUDED_)
