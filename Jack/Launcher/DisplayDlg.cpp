/*******************************************************************************
;
;	MODULE:			DISPLAYDLG (.CPP)
;
;	PURPOSE:		Display options dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "DisplayDlg.h"
#include "DisplayMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////
// Button and image coordinates
/////////////////////////////////////////////

#define DD_CLOSE_X					577
#define DD_CLOSE_Y					6
#define DD_OK_X						194
#define DD_OK_Y						349
#define DD_CANCEL_X					306
#define DD_CANCEL_Y					349

#define DD_RESOLUTION_LIST_X		16
#define DD_RESOLUTION_LIST_Y		73
#define DD_RESOLUTION_LIST_WIDTH	133
#define DD_RESOLUTION_LIST_HEIGHT	248
#define DD_RENDERER_LIST_X			181
#define DD_RENDERER_LIST_Y			73
#define DD_RENDERER_LIST_WIDTH		405
#define DD_RENDERER_LIST_HEIGHT		106
#define DD_DISPLAY_LIST_X			181
#define DD_DISPLAY_LIST_Y			205
#define DD_DISPLAY_LIST_WIDTH		405
#define DD_DISPLAY_LIST_HEIGHT		106

CDisplayMgr* g_pDisplayMgr = NULL;

/////////////////////////////////////////////////////////////////////////////
// CDisplayDlg dialog


CDisplayDlg::CDisplayDlg(CWnd* pParent /*=NULL*/)
	: CMoveDialog(CDisplayDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisplayDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CMoveDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisplayDlg, CMoveDialog)
	//{{AFX_MSG_MAP(CDisplayDlg)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_LBN_SELCHANGE(IDC_DISPLAY_LIST, OnSelchangeDisplayList)
	ON_LBN_SELCHANGE(IDC_RENDERER_LIST, OnSelchangeRendererList)
	ON_LBN_SELCHANGE(IDC_RESOLUTION_LIST, OnSelchangeResolutionList)
	ON_BN_CLICKED(IDC_DD_OK, OnOK)
	ON_BN_CLICKED(IDC_DD_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_DD_CLOSE, OnClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisplayDlg message handlers


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDisplayDlg::OnClose
//
//  PURPOSE:	Closes Display Dialog Box when the X in the corner is clicked
//
// ----------------------------------------------------------------------- //
void CDisplayDlg::OnClose()
{
	OnCancel();
}

BOOL CDisplayDlg::OnInitDialog() 
{
	SetBackGround(IDB_DD_BACKGROUND);

	if (!g_pDisplayMgr)
	{
		g_pDisplayMgr = new CDisplayMgr();
		if (!g_pDisplayMgr->Init("", "display.cfg"))
		{
			OnDestroy();
			return TRUE;
		}
	}

	
	CMoveDialog::OnInitDialog();

	AddButton(IDC_DD_CLOSE,DD_CLOSE_X,DD_CLOSE_Y);
	AddButton(IDC_DD_OK,DD_OK_X,DD_OK_Y);
	AddButton(IDC_DD_CANCEL,DD_CANCEL_X,DD_CANCEL_Y);

	CWnd* pWnd = GetDlgItem(IDC_RENDERER_LIST);
	if(pWnd)
	{
		//pWnd->SetFont(&theApp.m_Font);
		pWnd->SetWindowPos(NULL,DD_RENDERER_LIST_X,DD_RENDERER_LIST_Y,DD_RENDERER_LIST_WIDTH,DD_RENDERER_LIST_HEIGHT,SWP_NOZORDER);
	}
	pWnd = GetDlgItem(IDC_DISPLAY_LIST);
	if(pWnd)
	{
		//pWnd->SetFont(&theApp.m_Font);
		pWnd->SetWindowPos(NULL,DD_DISPLAY_LIST_X,DD_DISPLAY_LIST_Y,DD_DISPLAY_LIST_WIDTH,DD_DISPLAY_LIST_HEIGHT,SWP_NOZORDER);
	}
	pWnd = GetDlgItem(IDC_RESOLUTION_LIST);
	if(pWnd)
	{
		//pWnd->SetFont(&theApp.m_Font);
		pWnd->SetWindowPos(NULL,DD_RESOLUTION_LIST_X,DD_RESOLUTION_LIST_Y,DD_RESOLUTION_LIST_WIDTH,DD_RESOLUTION_LIST_HEIGHT,SWP_NOZORDER);
	}

	if (g_pDisplayMgr->GetNumRenderers() <= 0)
	{
		theApp.MessageBox( IDS_NORENS, IDS_GAME_NAME, MB_OK | MB_ICONSTOP );
		return(0);
	}

	g_pDisplayMgr->FillListBoxes(GetDlgItem(IDC_RENDERER_LIST)->GetSafeHwnd(), GetDlgItem(IDC_DISPLAY_LIST)->GetSafeHwnd(), GetDlgItem(IDC_RESOLUTION_LIST)->GetSafeHwnd());
	g_pDisplayMgr->UpdateListSelections(GetDlgItem(IDC_RENDERER_LIST)->GetSafeHwnd(), GetDlgItem(IDC_DISPLAY_LIST)->GetSafeHwnd(), GetDlgItem(IDC_RESOLUTION_LIST)->GetSafeHwnd());

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CDisplayDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	switch(nCtlColor)
	{
		case CTLCOLOR_LISTBOX:
		case CTLCOLOR_MSGBOX:
		{
			// Set color to green on black and return the background brush.
			pDC->SetTextColor(crNormalText);
			pDC->SetBkColor(RGB(0, 0, 0));
			return (HBRUSH)(theApp.m_BkBrush.GetSafeHandle());
		}
		default:
		return CMoveDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}

void CDisplayDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	PaintBackGround(&dc);
	
	// Do not call CMoveDialog::OnPaint() for painting messages
}

void CDisplayDlg::OnOK() 
{
	if (g_pDisplayMgr)
	{
		g_pDisplayMgr->UpdateListSelections(GetDlgItem(IDC_RENDERER_LIST)->GetSafeHwnd(), GetDlgItem(IDC_DISPLAY_LIST)->GetSafeHwnd(), GetDlgItem(IDC_RESOLUTION_LIST)->GetSafeHwnd());
	}
	CMoveDialog::OnOK();
}

void CDisplayDlg::OnCancel() 
{
	if (g_pDisplayMgr)
		g_pDisplayMgr->SetCurrentRenderer((CRenderer*)NULL);

	CMoveDialog::OnCancel();	
}

void CDisplayDlg::OnSelchangeDisplayList() 
{
	g_pDisplayMgr->UpdateListSelections(GetDlgItem(IDC_RENDERER_LIST)->GetSafeHwnd(), GetDlgItem(IDC_DISPLAY_LIST)->GetSafeHwnd(), GetDlgItem(IDC_RESOLUTION_LIST)->GetSafeHwnd());
}

void CDisplayDlg::OnSelchangeRendererList() 
{
	g_pDisplayMgr->UpdateListSelections(GetDlgItem(IDC_RENDERER_LIST)->GetSafeHwnd(), GetDlgItem(IDC_DISPLAY_LIST)->GetSafeHwnd(), GetDlgItem(IDC_RESOLUTION_LIST)->GetSafeHwnd());
}

void CDisplayDlg::OnSelchangeResolutionList() 
{
	g_pDisplayMgr->UpdateListSelections(GetDlgItem(IDC_RENDERER_LIST)->GetSafeHwnd(), GetDlgItem(IDC_DISPLAY_LIST)->GetSafeHwnd(), GetDlgItem(IDC_RESOLUTION_LIST)->GetSafeHwnd());
}
