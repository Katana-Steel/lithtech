/*******************************************************************************
;
;	MODULE:			OPTIONSDLG (.H)
;
;	PURPOSE:		Options dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "OptionsDlg.h"
#include "BitmapCheckButton.h"
#include "textcheckbox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////
// Button and image coordinates
/////////////////////////////////////////////

#define OD_CLOSE_X 435
#define OD_CLOSE_Y 6

#define OD_COLUMN_1_X					25
#define OD_COLUMN_2_X					227
#define OD_ROW_Y_START					67
#define OD_ROW_Y_OFFSET					25 // 20

#define OD_DISABLESOUND_X				OD_COLUMN_1_X
#define OD_DISABLESOUND_Y				OD_ROW_Y_START
#define OD_DISABLEMUSIC_X				OD_COLUMN_1_X
#define OD_DISABLEMUSIC_Y				(OD_ROW_Y_START + OD_ROW_Y_OFFSET)
#define OD_DISABLEMOVIES_X				OD_COLUMN_1_X
#define OD_DISABLEMOVIES_Y				(OD_ROW_Y_START + 2*OD_ROW_Y_OFFSET)
#define	OD_DISABLEHARDWARESOUND_X		OD_COLUMN_1_X
#define	OD_DISABLEHARDWARESOUND_Y		(OD_ROW_Y_START + 3*OD_ROW_Y_OFFSET)
#define OD_DISABLETRIPLEBUFFERING_X	OD_COLUMN_1_X
#define OD_DISABLETRIPLEBUFFERING_Y	(OD_ROW_Y_START + 4*OD_ROW_Y_OFFSET)

#define OD_DISABLEJOYSTICKS_X		OD_COLUMN_2_X
#define OD_DISABLEJOYSTICKS_Y		OD_ROW_Y_START
#define OD_DISABLEHARDWARECURSOR_X			OD_COLUMN_2_X
#define OD_DISABLEHARDWARECURSOR_Y			(OD_ROW_Y_START + OD_ROW_Y_OFFSET)
#define OD_DISABLESOUNDFILTERS_X		OD_COLUMN_2_X
#define OD_DISABLESOUNDFILTERS_Y		(OD_ROW_Y_START + 2*OD_ROW_Y_OFFSET)
#define OD_ENABLESCREENSHOTS_X		OD_COLUMN_2_X
#define OD_ENABLESCREENSHOTS_Y 		(OD_ROW_Y_START + 3*OD_ROW_Y_OFFSET)

#define OD_RESTOREDEFAULTSETTINGS_X		OD_COLUMN_1_X
#define OD_RESTOREDEFAULTSETTINGS_Y		219
#define OD_ALWAYSSPECIFY_X				OD_COLUMN_1_X
#define OD_ALWAYSSPECIFY_Y				295
#define OD_COMMANDLINE_X				OD_COLUMN_1_X
#define OD_COMMANDLINE_Y				265
#define OD_COMMANDLINE_WIDTH			405
#define OD_COMMANDLINE_HEIGHT			23

#define OD_OK_X							123
#define OD_OK_Y							435
#define OD_CANCEL_X						235
#define OD_CANCEL_Y						435

#define OD_HELPTEXT_Y					341 // 335
#define OD_HELPTEXT_HEIGHT				80

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog


COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CMoveDialog(COptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionsDlg)
	//}}AFX_DATA_INIT
}


void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CMoveDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDlg)
	DDX_Control(pDX, IDC_HELPTEXT, m_HelpText);
	DDX_Control(pDX, IDC_COMMANDLINE, m_CommandLine);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDlg, CMoveDialog)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_BN_CLICKED(IDC_OD_CLOSE, OnClose)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_OD_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_OD_OK, OnOK)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COptionsDlg::OnClose
//
//	PURPOSE:	Close button handler
//
// ----------------------------------------------------------------------- //

void COptionsDlg::OnClose() 
{
	OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COptionsDlg::OnInitDialog
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL COptionsDlg::OnInitDialog() 
{
	SetBackGround(IDB_OD_BACKGROUND);

	CMoveDialog::OnInitDialog();

	CTextCheckBox	*pCheckBox;

	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLESOUND,OD_DISABLESOUND_X,OD_DISABLESOUND_Y, IDS_OD_DISABLESOUND, &m_HelpText,IDS_HELP_DISABLESOUND);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableSound);

	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLEMUSIC,OD_DISABLEMUSIC_X,OD_DISABLEMUSIC_Y, IDS_OD_DISABLEMUSIC, &m_HelpText,IDS_HELP_DISABLEMUSIC);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableMusic);
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLEMOVIES,OD_DISABLEMOVIES_X,OD_DISABLEMOVIES_Y, IDS_OD_DISABLEMOVIES, &m_HelpText,IDS_HELP_DISABLEMOVIES);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableMovies);
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLETRIPLEBUFFERING,OD_DISABLETRIPLEBUFFERING_X,OD_DISABLETRIPLEBUFFERING_Y, IDS_OD_DISABLETRIPLEBUFFERING, &m_HelpText,IDS_HELP_DISABLETRIPLEBUFFERING);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableTripleBuffering);
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLEJOYSTICKS,OD_DISABLEJOYSTICKS_X,OD_DISABLEJOYSTICKS_Y, IDS_OD_DISABLEJOYSTICKS, &m_HelpText,IDS_HELP_DISABLEJOYSTICKS);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableJoysticks);
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLEHARDWARECURSOR,OD_DISABLEHARDWARECURSOR_X,OD_DISABLEHARDWARECURSOR_Y, IDS_OD_DISABLEHARDWARECURSOR, &m_HelpText,IDS_HELP_DISABLEHARDWARECURSOR);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableHardwareCursor);
	
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLEHARDWARESOUND,OD_DISABLEHARDWARESOUND_X,OD_DISABLEHARDWARESOUND_Y, IDS_OD_DISABLEHARDWARESOUND, &m_HelpText,IDS_HELP_DISABLEHARDWARESOUND);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableHardwareSound);
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_DISABLESOUNDFILTERS,OD_DISABLESOUNDFILTERS_X,OD_DISABLESOUNDFILTERS_Y, IDS_OD_DISABLESOUNDFILTERS, &m_HelpText,IDS_HELP_DISABLESOUNDFILTERS);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bDisableSoundFilters);

	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_ENABLESCREENSHOTS,OD_ENABLESCREENSHOTS_X,OD_ENABLESCREENSHOTS_Y, IDS_OD_ENABLESCREENSHOTS, &m_HelpText,IDS_HELP_ENABLESCREENSHOTS);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bEnableScreenshots);
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_RESTOREDEFAULTSETTINGS,OD_RESTOREDEFAULTSETTINGS_X,OD_RESTOREDEFAULTSETTINGS_Y, IDS_OD_RESTOREDEFAULTS, &m_HelpText,IDS_HELP_RESTOREDEFAULTS);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bRestoreDefaults);
	
	pCheckBox = (CTextCheckBox*)AddTextCheckBox(IDC_OD_ALWAYSSPECIFY,OD_ALWAYSSPECIFY_X,OD_ALWAYSSPECIFY_Y, IDS_OD_ALWAYSSPECIFY, &m_HelpText,IDS_HELP_ALWAYSSPECIFY);
	if(pCheckBox)
		pCheckBox->SetCheck(theApp.m_bSaveCommands);

	AddButton(IDC_OD_CLOSE,OD_CLOSE_X,OD_CLOSE_Y);
	AddButton(IDC_OD_OK,OD_OK_X,OD_OK_Y);
	AddButton(IDC_OD_CANCEL,OD_CANCEL_X,OD_CANCEL_Y);

	// Let's place and size the edit control appropriately
	m_CommandLine.SetFont(&theApp.m_Font);
	m_CommandLine.SetWindowPos(NULL,OD_COMMANDLINE_X,OD_COMMANDLINE_Y,OD_COMMANDLINE_WIDTH,OD_COMMANDLINE_HEIGHT,SWP_NOZORDER);
	m_CommandLine.SetWindowText(theApp.m_csCommands);

	// Place and size the help text control
	CRect rcDlg;
	GetWindowRect(&rcDlg);
	CString	strDefault;

	// Set the no option selected text
	strDefault.LoadString( IDS_HELP_DEFAULT );
	m_HelpText.SetWindowText( strDefault );

	m_HelpText.SetWindowPos(NULL,15,OD_HELPTEXT_Y,rcDlg.Width()-30,OD_HELPTEXT_HEIGHT,SWP_NOZORDER);
	m_HelpText.SetTextColor(RGB(0,0,0));
	//m_HelpText.SetFontName("Arial Narrow");
	//m_HelpText.SetFontSize(15);
	m_HelpText.SetFontBold(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COptionsDlg::OnPaint
//
//	PURPOSE:	WM_PAINT handler
//
// ----------------------------------------------------------------------- //

void COptionsDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	PaintBackGround(&dc);
	
	// Do not call CMoveDialog::OnPaint() for painting messages
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COptionsDlg::OnCancel
//
//	PURPOSE:	Cancel button handler
//
// ----------------------------------------------------------------------- //

void COptionsDlg::OnCancel() 
{
	CMoveDialog::OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COptionsDlg::OnOK
//
//	PURPOSE:	OK button handler
//
// ----------------------------------------------------------------------- //

void COptionsDlg::OnOK() 
{
	// Save this sheeyot into the variablez
	CTextCheckBox* pCheckBox;
	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLESOUND);
	theApp.m_bDisableSound = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLEMUSIC);
	theApp.m_bDisableMusic = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLEMOVIES);
	theApp.m_bDisableMovies = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLETRIPLEBUFFERING);
	theApp.m_bDisableTripleBuffering = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLEJOYSTICKS);
	theApp.m_bDisableJoysticks = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLEHARDWARECURSOR);
	theApp.m_bDisableHardwareCursor = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLEHARDWARESOUND);
	theApp.m_bDisableHardwareSound = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_DISABLESOUNDFILTERS);
	theApp.m_bDisableSoundFilters = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_ENABLESCREENSHOTS);
	theApp.m_bEnableScreenshots = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_RESTOREDEFAULTSETTINGS);
	theApp.m_bRestoreDefaults = pCheckBox->GetCheck();

	pCheckBox = (CTextCheckBox*)GetDlgItem(IDC_OD_ALWAYSSPECIFY);
	theApp.m_bSaveCommands = pCheckBox->GetCheck();

	m_CommandLine.GetWindowText(theApp.m_csCommands);
	theApp.m_RegMgr.SetField("Save Commands",theApp.m_bSaveCommands);
	if(theApp.m_bSaveCommands)
	{
		theApp.m_RegMgr.SetField("Commands",(char *)(LPCTSTR)theApp.m_csCommands);
	}

	CMoveDialog::OnOK();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COptionsDlg::OnCtlColor
//
//	PURPOSE:	WM_CTLCOLOR handler
//
// ----------------------------------------------------------------------- //

HBRUSH COptionsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	switch(nCtlColor)
	{
		case CTLCOLOR_EDIT:
		case CTLCOLOR_MSGBOX:
		{
			// Set color to green on black and return the background brush.
			pDC->SetTextColor(crNormalText);
			pDC->SetBkColor(RGB(0, 0, 0));
			return (HBRUSH)(theApp.m_BkBrush.GetSafeHandle());
		}
		case CTLCOLOR_DLG:
		case CTLCOLOR_BTN:
		{
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		}
		default:
		return CMoveDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	COptionsDlg::OnMouseMove
//
//	PURPOSE:	Mouse move handler
//
// ----------------------------------------------------------------------- //

void COptionsDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(m_pFocusButton != NULL)
	{
		CRect rcCtrl;
		CString	strDefault;

		// Set the no option selected text
		strDefault.LoadString( IDS_HELP_DEFAULT );
		m_HelpText.SetWindowText( strDefault );
		
		m_HelpText.GetWindowRect(&rcCtrl);
		ScreenToClient(&rcCtrl);
		InvalidateRect(&rcCtrl);
	}
	CMoveDialog::OnMouseMove(nFlags, point);
}

