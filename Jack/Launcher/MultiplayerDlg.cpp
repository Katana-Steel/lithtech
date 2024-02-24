/*******************************************************************************
;
;	MODULE:			MULTIPLAYERDLG (.CPP)
;
;	PURPOSE:		Multiplayer dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "MultiplayerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////
// Button and image coordinates
/////////////////////////////////////////////

#define MD_CLOSE_X	102
#define MD_CLOSE_Y	1

#define MD_HOST_X	14
#define MD_HOST_Y	21
#define MD_JOIN_X	14
#define MD_JOIN_Y	57
#define MD_FIND_X	14
#define MD_FIND_Y	93
#define MD_SERVER_X	14
#define MD_SERVER_Y	129
#define MD_BACK_X	14
#define MD_BACK_Y	165

////////////////////////////////////////////////////////////////////////////
// MultiplayerDlg dialog


CMultiplayerDlg::CMultiplayerDlg(CWnd* pParent /*=NULL*/)
	: CAnimDlg(CMultiplayerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiplayerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMultiplayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CAnimDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiplayerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiplayerDlg, CAnimDlg)
	//{{AFX_MSG_MAP(CMultiplayerDlg)
	ON_BN_CLICKED(IDC_MP_FIND, OnFind)
	ON_BN_CLICKED(IDC_MP_HOST, OnHost)
	ON_BN_CLICKED(IDC_MP_JOIN, OnJoin)
	ON_BN_CLICKED(IDC_MP_SERVER, OnServer)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_MP_BACK, OnMpBack)
	ON_BN_CLICKED(IDC_MP_CLOSE, OnMpClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnFind
//
//	PURPOSE:	Find button handler
//
// ----------------------------------------------------------------------- //

void CMultiplayerDlg::OnFind() 
{
	EndDialog(ACTION_FINDINTERNETSERVERS);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnHost
//
//	PURPOSE:	Host button handler
//
// ----------------------------------------------------------------------- //

void CMultiplayerDlg::OnHost() 
{
	EndDialog(ACTION_HOSTGAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnJoin
//
//	PURPOSE:	Join button handler
//
// ----------------------------------------------------------------------- //

void CMultiplayerDlg::OnJoin() 
{
	EndDialog(ACTION_JOINGAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnServer
//
//	PURPOSE:	Server button handler
//
// ----------------------------------------------------------------------- //

void CMultiplayerDlg::OnServer() 
{
	EndDialog(ACTION_STANDALONESERVER);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnInitDialog
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CMultiplayerDlg::OnInitDialog() 
{
	SetBackGround(IDB_MD_BACKGROUND);

	CAnimDlg::OnInitDialog();

	// Here's where we define what buttons go in our dialog
	AddButton(IDC_MP_CLOSE,MD_CLOSE_X,MD_CLOSE_Y);
	AddButton(IDC_MP_HOST,MD_HOST_X,MD_HOST_Y);
	AddButton(IDC_MP_JOIN,MD_JOIN_X,MD_JOIN_Y);
	AddButton(IDC_MP_FIND,MD_FIND_X,MD_FIND_Y);
	AddButton(IDC_MP_SERVER,MD_SERVER_X,MD_SERVER_Y);
	AddButton(IDC_MP_BACK,MD_BACK_X,MD_BACK_Y);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnPaint
//
//	PURPOSE:	WM_PAINT handler
//
// ----------------------------------------------------------------------- //

void CMultiplayerDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	PaintBackGround(&dc);
	
	// Do not call CAnimDlg::OnPaint() for painting messages
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnMpBack
//
//	PURPOSE:	Back button handler
//
// ----------------------------------------------------------------------- //

void CMultiplayerDlg::OnMpBack() 
{
	OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiplayerDlg::OnMpClose
//
//	PURPOSE:	Close button handler
//
// ----------------------------------------------------------------------- //

void CMultiplayerDlg::OnMpClose() 
{
	OnCancel();
}
