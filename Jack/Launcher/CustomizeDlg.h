#if !defined(AFX_CUSTOMIZEDLG_H__852445D9_2231_4694_8E1C_6B9BDBD6C089__INCLUDED_)
#define AFX_CUSTOMIZEDLG_H__852445D9_2231_4694_8E1C_6B9BDBD6C089__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomizeDlg.h : header file
//

#include "MoveDlg.h"
#include "EditEx.h"
#include "StaticEx.h"
#include "Utils.h"
#include <set>
#include <string>

#define CD_MAX_MODS_PER_PAGE	12

class CTextCheckBox;
/////////////////////////////////////////////////////////////////////////////
// CustomizeDlg dialog

class CustomizeDlg : public CMoveDialog
{
// Construction
public:
	CustomizeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CustomizeDlg)
	enum { IDD = IDD_CUSTOMIZE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CustomizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	bool	BuildCustomList();

	CTextCheckBox	*m_apTextCheckBox[CD_MAX_MODS_PER_PAGE];

	typedef std::set<std::string,CaselessLesser> StringSet;
	StringSet	m_setMods;

	std::string		m_sSelectedMod;

	// Generated message map functions
	//{{AFX_MSG(CustomizeDlg)
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg void OnOK();
	afx_msg void OnNext();
	afx_msg void OnPrevious();
	afx_msg void OnPaint();
	afx_msg void OnModButtonClicked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMIZEDLG_H__852445D9_2231_4694_8E1C_6B9BDBD6C089__INCLUDED_)
