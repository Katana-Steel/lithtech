/*******************************************************************************
;
;	MODULE:			MESSAGEBOXDLG (.CPP)
;
;	PURPOSE:		Message box dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "MessageBoxDlg.h"
#include "ButtonEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////
// Button and image coordinates
/////////////////////////////////////////////

#define MB_CLOSE_X		577
#define MB_CLOSE_Y		6

#define MB_OK_X			189
#define MB_OK_XCENTER	250
#define MB_OK_Y			206
#define MB_CANCEL_X		312
#define MB_CANCEL_Y		206

#define MB_CAPTION_Y	18
#define MB_TEXT_Y		65
#define MB_ICON_X		6
#define MB_ICON_Y		15


/////////////////////////////////////////////////////////////////////////////
// CMessageBoxDlg dialog


CMessageBoxDlg::CMessageBoxDlg(CWnd* pParent /*=NULL*/)
	: CMoveDialog(CMessageBoxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessageBoxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_dwStyle = MB_OK;
}


void CMessageBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CMoveDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageBoxDlg)
	DDX_Control(pDX, IDC_TEXT, m_Text);
	DDX_Control(pDX, IDC_CAPTION, m_Caption);
	DDX_Control(pDX, IDC_MB_ICON, m_Icon);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageBoxDlg, CMoveDialog)
	//{{AFX_MSG_MAP(CMessageBoxDlg)
	ON_BN_CLICKED(IDC_MB_OK, OnOk)
	ON_BN_CLICKED(IDC_MB_CLOSE, OnClose)
	ON_BN_CLICKED(IDC_MB_CANCEL, OnCancel)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageBoxDlg::OnCancel
//
//	PURPOSE:	Cancel button handler
//
// ----------------------------------------------------------------------- //

void CMessageBoxDlg::OnCancel() 
{
	CDialogEx::OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageBoxDlg::OnOk
//
//	PURPOSE:	OK button handler
//
// ----------------------------------------------------------------------- //

void CMessageBoxDlg::OnOk() 
{
	CDialogEx::OnOK();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageBoxDlg::OnClose
//
//	PURPOSE:	Close button handler
//
// ----------------------------------------------------------------------- //

void CMessageBoxDlg::OnClose() 
{
	CDialogEx::OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageBoxDlg::OnInitDialog
//
//	PURPOSE:	Cancel button handler
//
// ----------------------------------------------------------------------- //

BOOL CMessageBoxDlg::OnInitDialog() 
{
	SetBackGround(IDB_MB_BACKGROUND);

	CMoveDialog::OnInitDialog();

	// Set up the text and caption
	CRect rcDlg;
	GetWindowRect(&rcDlg);
	m_Caption.SetWindowText(m_csCaption);
	m_Caption.SetWindowPos(NULL,0,MB_CAPTION_Y,rcDlg.Width(),rcDlg.Height()-MB_CAPTION_Y,SWP_NOZORDER);
	m_Text.SetWindowText(m_csText);
	m_Text.SetWindowPos(NULL,15,MB_TEXT_Y,rcDlg.Width()-30,rcDlg.Height()-MB_TEXT_Y,SWP_NOZORDER);

	m_Caption.SetTextColor(RGB(0,0,0));
	m_Caption.SetFontName("Arial Narrow");
	m_Caption.SetFontSize(20);
	m_Caption.SetFontBold(TRUE);
	m_Text.SetTextColor(RGB(0,0,0));

	// Here's where we define what buttons go in our dialog
	AddButton(IDC_MB_CLOSE,MB_CLOSE_X,MB_CLOSE_Y);
	AddButton(IDC_MB_OK,MB_OK_X,MB_OK_Y);
	AddButton(IDC_MB_CANCEL,MB_CANCEL_X,MB_CANCEL_Y);

	// Set the Icons pos
	m_Icon.SetWindowPos( NULL, MB_ICON_X, MB_ICON_Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	HBITMAP	hIcon = NULL;

	// Figure out what ICON to draw
	switch( m_dwStyle & MB_ICONMASK )
	{
	case MB_ICONEXCLAMATION:
		{
			hIcon = ::LoadBitmap( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_WARNING) );
		}
		break;

	case MB_ICONSTOP:
		{
			hIcon = ::LoadBitmap( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_ERROR) );
		}
		break;

	case MB_ICONINFORMATION:
		{
			hIcon = ::LoadBitmap( AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_INFORMATION) );
		}
		break;

	default:
		hIcon = NULL;
		break;
		
	}

	// Draw the appropriate Icon, or nothing if NULL
	m_Icon.SetBitmap( hIcon );
	

	if((m_dwStyle & MB_TYPEMASK) == MB_OK)
	{
		// Center the OK button and hide the cancel button
		CWnd* pWnd = GetDlgItem(IDC_MB_OK);
		if(pWnd)
			pWnd->SetWindowPos(NULL,MB_OK_XCENTER,MB_OK_Y,0,0,SWP_NOZORDER | SWP_NOSIZE);
		pWnd = GetDlgItem(IDC_MB_CANCEL);
		if(pWnd)
			pWnd->ShowWindow(SW_HIDE);
	}

	CenterWindow();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageBoxDlg::OnPaint
//
//	PURPOSE:	Paint handler
//
// ----------------------------------------------------------------------- //

void CMessageBoxDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	PaintBackGround(&dc);

	// Do not call CMoveDialog::OnPaint() for painting messages
}
