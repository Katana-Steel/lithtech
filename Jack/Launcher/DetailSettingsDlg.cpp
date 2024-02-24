/*******************************************************************************
;
;	MODULE:			DETAILSETTINGSDLG (.CPP)
;
;	PURPOSE:		Detail settings dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "DetailSettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////
// Button and image coordinates
/////////////////////////////////////////////

#define DSD_CLOSE_X				435
#define DSD_CLOSE_Y				6
#define DSD_CANCEL_X			178
#define DSD_CANCEL_Y			440

#define DSD_HIGHDETAIL_X		123 // Original = 123
#define DSD_HIGHDETAIL_Y		168
#define DSD_MEDIUMDETAIL_X		DSD_HIGHDETAIL_X
#define DSD_MEDIUMDETAIL_Y		277
#define DSD_LOWDETAIL_X			DSD_HIGHDETAIL_X
#define DSD_LOWDETAIL_Y			386


#define DSD_HEADERTEXT_Y		38
#define DSD_HEADERTEXT_HEIGHT	58
#define DSD_HIGHTEXT_Y			117
#define DSD_HIGHTEXT_HEIGHT		49
#define DSD_MEDIUMTEXT_Y		226
#define DSD_MEDIUMTEXT_HEIGHT	DSD_HIGHTEXT_HEIGHT
#define DSD_LOWTEXT_Y			336
#define DSD_LOWTEXT_HEIGHT		DSD_HIGHTEXT_HEIGHT


/////////////////////////////////////////////////////////////////////////////
// CDetailSettingsDlg dialog


CDetailSettingsDlg::CDetailSettingsDlg(CWnd* pParent /*=NULL*/)
	: CMoveDialog(CDetailSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDetailSettingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDetailSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CMoveDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDetailSettingsDlg)
	DDX_Control(pDX, IDC_MEDIUMTEXT, m_MediumText);
	DDX_Control(pDX, IDC_LOWTEXT, m_LowText);
	DDX_Control(pDX, IDC_HIGHTEXT, m_HighText);
	DDX_Control(pDX, IDC_HEADERTEXT, m_HeaderText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDetailSettingsDlg, CMoveDialog)
	//{{AFX_MSG_MAP(CDetailSettingsDlg)
	ON_BN_CLICKED(IDC_DSD_CLOSE, OnClose)
	ON_BN_CLICKED(IDC_DSD_HIGH, OnHighDetail)
	ON_BN_CLICKED(IDC_DSD_LOW, OnLowDetail)
	ON_BN_CLICKED(IDC_DSD_MEDIUM, OnMediumDetail)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_DSD_CANCEL, OnCancel)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDetailSettingsDlg message handlers

void CDetailSettingsDlg::OnCancel() 
{
	CMoveDialog::OnCancel();
}

void CDetailSettingsDlg::OnClose() 
{
	CMoveDialog::OnCancel();
}

void CDetailSettingsDlg::OnHighDetail() 
{
	theApp.m_dwDetailSetting = DETAIL_HIGH;
	OnOK();
}

void CDetailSettingsDlg::OnLowDetail() 
{
	theApp.m_dwDetailSetting = DETAIL_LOW;
	OnOK();
}

void CDetailSettingsDlg::OnMediumDetail() 
{
	theApp.m_dwDetailSetting = DETAIL_MEDIUM;
	OnOK();
}

BOOL CDetailSettingsDlg::OnInitDialog() 
{
	SetBackGround(IDB_DSD_BACKGROUND);

	CMoveDialog::OnInitDialog();

	// Here's where we define what buttons go in our dialog
	AddButton(IDC_DSD_CLOSE,DSD_CLOSE_X,DSD_CLOSE_Y);
	AddButton(IDC_DSD_HIGH,DSD_HIGHDETAIL_X,DSD_HIGHDETAIL_Y);
	AddButton(IDC_DSD_MEDIUM,DSD_MEDIUMDETAIL_X,DSD_MEDIUMDETAIL_Y);
	AddButton(IDC_DSD_LOW,DSD_LOWDETAIL_X,DSD_LOWDETAIL_Y);
	AddButton(IDC_DSD_CANCEL,DSD_CANCEL_X,DSD_CANCEL_Y);

	CRect rcDlg;
	GetWindowRect(&rcDlg);

	CString csText;
	csText.LoadString(IDS_DETAIL_HEADER);
	m_HeaderText.SetFontName("Arial Narrow");
	m_HeaderText.SetFontSize(13);
	m_HeaderText.SetTextColor(RGB(0,0,0));
	m_HeaderText.SetWindowText(csText);
	m_HeaderText.SetWindowPos(NULL,15,DSD_HEADERTEXT_Y,rcDlg.Width()-30,DSD_HEADERTEXT_HEIGHT,SWP_NOZORDER);
	csText.LoadString(IDS_DETAIL_LOW);
	m_LowText.SetFontName("Arial Narrow");
	m_LowText.SetFontSize(13);
	m_LowText.SetTextColor(RGB(0,0,0));
	m_LowText.SetWindowText(csText);
	m_LowText.SetWindowPos(NULL,15,DSD_LOWTEXT_Y,rcDlg.Width()-30,DSD_LOWTEXT_HEIGHT,SWP_NOZORDER);
	csText.LoadString(IDS_DETAIL_MEDIUM);
	m_MediumText.SetFontName("Arial Narrow");
	m_MediumText.SetFontSize(13);
	m_MediumText.SetTextColor(RGB(0,0,0));
	m_MediumText.SetWindowText(csText);
	m_MediumText.SetWindowPos(NULL,15,DSD_MEDIUMTEXT_Y,rcDlg.Width()-30,DSD_MEDIUMTEXT_HEIGHT,SWP_NOZORDER);
	csText.LoadString(IDS_DETAIL_HIGH);
	m_HighText.SetFontName("Arial Narrow");
	m_HighText.SetFontSize(13);
	m_HighText.SetTextColor(RGB(0,0,0));
	m_HighText.SetWindowText(csText);
	m_HighText.SetWindowPos(NULL,15,DSD_HIGHTEXT_Y,rcDlg.Width()-30,DSD_HIGHTEXT_HEIGHT,SWP_NOZORDER);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDetailSettingsDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	PaintBackGround(&dc);
	// Do not call CDialog::OnPaint() for painting messages
}

HBRUSH CDetailSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	switch(nCtlColor)
	{
		case CTLCOLOR_DLG:
		case CTLCOLOR_BTN:
		{
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		}
		default:
		return CMoveDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}
