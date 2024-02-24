/*******************************************************************************
;
;	MODULE:			DETAILSETTINGSDLG (.H)
;
;	PURPOSE:		Detail settings dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_DETAILSETTINGSDLG_H__ABB668E1_BC9E_11D4_A2BA_00010229388A__INCLUDED_)
#define AFX_DETAILSETTINGSDLG_H__ABB668E1_BC9E_11D4_A2BA_00010229388A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DetailSettingsDlg.h : header file
//

#include "MoveDlg.h"
#include "StaticEx.h"

/////////////////////////////////////////////////////////////////////////////
// CDetailSettingsDlg dialog

class CDetailSettingsDlg : public CMoveDialog
{
// Construction
public:
	CDetailSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDetailSettingsDlg)
	enum { IDD = IDD_DETAILSETTINGS_DIALOG };
	CStaticEx	m_MediumText;
	CStaticEx	m_LowText;
	CStaticEx	m_HighText;
	CStaticEx	m_HeaderText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDetailSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDetailSettingsDlg)
	afx_msg void OnCancel();
	afx_msg void OnClose();
	afx_msg void OnHighDetail();
	afx_msg void OnLowDetail();
	afx_msg void OnMediumDetail();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DETAILSETTINGSDLG_H__ABB668E1_BC9E_11D4_A2BA_00010229388A__INCLUDED_)
