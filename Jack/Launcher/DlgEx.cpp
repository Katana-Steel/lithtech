/*******************************************************************************
;
;	MODULE:			DLGEX (.CPP)
;
;	PURPOSE:		Extended dialog class (derived from CDialog) that has
;					a bitmap as a background
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/
#include "stdafx.h"
#include "DlgEx.h"
#include "BitmapCheckButton.h"
#include "TextCheckBox.h"
#include "ButtonEx.h"
//#include "anibutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialogEx dialog


CDialogEx::CDialogEx(int IDD, CWnd* pParent)
	: CDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgEx)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pFocusButton = NULL;
}


void CDialogEx::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgEx)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogEx, CDialog)
	//{{AFX_MSG_MAP(CDialogEx)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::OnInitDialog
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CDialogEx::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// IMPORTANT: SetBackGround() should have been called before we get here!!
	// Call SetBackGround() to set the background bitmap before calling CDialogEx::OnInitDialog()
	// from your implmentation of OnInitDialog. Check some dialog code in launchpad source to
	// see how this is done.
	
	// Get background bitmap size
	BITMAP bmpInfo;
	m_Background.GetBitmap(&bmpInfo);

	// Resize the dialog to fit the bitmap and then center it
	// NOTE: MFC Automatically resizes the dialog according to system font settings etc.
	// on every system. If you use this code, you'll need to position each control in your dialog
	// to make sure they will be visible after the following line of code resizes the dialog 
	// (see how this was done in the launchpaddlg OnInitDialog() code).
	// In case you want to leave the dialog sizing to MFC, remove the following 2 lines of code,
	// and then change the PaintBackGround() function and use a StretchBlt instead of the BitBlt,
	// so that the background BMP gets stretched to fit the dialogsize. Get the window size
	// using the GetClientRect() function. (The OnPaint() function in Blood2Dlg.cpp resizes the background
	// automatically to fit the dialog, use this as an example).

	SetWindowPos(NULL, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SWP_NOZORDER|SWP_NOMOVE);
	CenterWindow();
	
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::SetBackGround
//
//	PURPOSE:	Sets the background image from a resource ID
//
// ----------------------------------------------------------------------- //

BOOL CDialogEx::SetBackGround(int nID)
{
	// We could also use CBitmap::LoadBitmap but that has code that screws with the palette

	LPCTSTR lpszResourceName = (LPCTSTR)nID;

	HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), lpszResourceName, IMAGE_BITMAP, 0,0, LR_CREATEDIBSECTION);

	if (hBmp == NULL) return FALSE;

	m_Background.Attach(hBmp);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::PaintBackground
//
//	PURPOSE:	Draws the background image
//
//  IMPORTANT:	This function should be called from your OnPaint()
//				implementation BEFORE anything is painted on the dialog, 
//				to make sure the background gets drawn first.
//
// ----------------------------------------------------------------------- //

void CDialogEx::PaintBackGround(CPaintDC * dc)
{
	BITMAP bmpInfo;
	m_Background.GetBitmap(&bmpInfo);

	CDC dcTmp;
	dcTmp.CreateCompatibleDC(dc);
	dcTmp.SelectObject(&m_Background);

	// Draw bitmap regardless of dialog size. The OnInitDialog function will automatically
	// resize the dialog to fit the Bitmap when loading, so everything should look good. 
	dc->BitBlt(0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, &dcTmp, 0,0, SRCCOPY);

	dcTmp.DeleteDC();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::FreeAllButtons
//
//	PURPOSE:	Frees the memory for the buttons
//
// ----------------------------------------------------------------------- //

void CDialogEx::FreeAllButtons()
{
	CButton* pButton;
	for(int i=0;i<m_collButtons.GetSize();i++)
	{
		pButton = m_collButtons[i];
		if(pButton)
			delete pButton;
	}

	m_collButtons.RemoveAll();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::AddButton
//
//	PURPOSE:	Adds a button from a resource ID
//
// ----------------------------------------------------------------------- //

CButton* CDialogEx::AddButton(UINT nID, int x, int y, UINT nCursorID, CStaticEx* pHelpText, DWORD dwHelpTextID)
{
	// Create the button
	CButtonEx* pButton = new CButtonEx;

	// Load the bitmaps
	if(!pButton->AutoLoad(nID,this))
	{
		delete pButton;
		return NULL;
	}

	// Set the help text if applicable
	if(pHelpText)
	{
		CString csHelpText;
		if(dwHelpTextID)
			csHelpText.LoadString(dwHelpTextID);
		pButton->SetHelpTextCtrl(pHelpText,csHelpText);
	}

	// Set a cursor for this button... if we want one
	if( nCursorID )
	{
		// We can use the standard Windows cursors or our own 
		if( (nCursorID >= (UINT)IDC_ARROW) && (nCursorID <= (UINT)IDC_HELP) )
		{
			pButton->SetHCursor( AfxGetApp()->LoadStandardCursor( (char*)nCursorID ) );
		}
		else
			pButton->SetHCursor( AfxGetApp()->LoadCursor( nCursorID ) );
	}

	// Set the position
	pButton->SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Add us to our array
	m_collButtons.Add(pButton);

	// All done!
	return pButton;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::AddCheckBox
//
//	PURPOSE:	Adds a checkbox from a resource ID
//
// ----------------------------------------------------------------------- //

CButton* CDialogEx::AddCheckBox(UINT nID, int x, int y, CStaticEx* pHelpText, DWORD dwHelpTextID)
{
	// Create the button
	CBitmapCheckButton* pButton = new CBitmapCheckButton;

	// Load the bitmaps
	if(!pButton->AutoLoad(nID,this))
	{
		delete pButton;
		return NULL;
	}

	// Set the help text if applicable
	if(pHelpText)
	{
		CString csHelpText;
		if(dwHelpTextID)
			csHelpText.LoadString(dwHelpTextID);
		pButton->SetHelpTextCtrl(pHelpText,csHelpText);
	}

	// Set the position
	pButton->SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Add us to our array
	m_collButtons.Add(pButton);

	// All done!
	return pButton;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDialogEx::AddTextCheckBox
//
//  PURPOSE:	Adds a checkbox from a resource ID
//
// ----------------------------------------------------------------------- //

CButton* CDialogEx::AddTextCheckBox(UINT nID, int x, int y, UINT nTextID, CStaticEx* pHelpText, DWORD dwHelpTextID )
{
	CString	strText;
	if( nTextID )
	{
		strText.LoadString( nTextID );
	}
	
	return AddTextCheckBox( nID, x, y, strText, pHelpText, dwHelpTextID );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDialogEx::AddTextCheckBox
//
//  PURPOSE:	Adds a checkbox from a resource ID
//
// ----------------------------------------------------------------------- //

CButton* CDialogEx::AddTextCheckBox( UINT nID, int x, int y, const char *szText /* = NULL */, CStaticEx* pHelpText /* = NULL */, DWORD dwHelpTextID /* = 0 */ )
{
	// Create the button
	CTextCheckBox* pButton = new CTextCheckBox;

	if( szText )
	{
		pButton->SetWindowText( szText );
	}
	
	// Load the buton
	if(!pButton->AutoLoad(nID,this))
	{
		delete pButton;
		return NULL;
	}

	// Set the help text if applicable
	CString csHelpText;
	if(dwHelpTextID)
		csHelpText.LoadString(dwHelpTextID);
	pButton->SetHelpTextCtrl(pHelpText,csHelpText);


	// Force the button to draw itself
	pButton->ModifyStyle( 0, BS_OWNERDRAW | BS_CHECKBOX, 0 );

	// Set the position
	pButton->SetWindowPos(NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

	// Add us to our array
	m_collButtons.Add(pButton);

	// All done!
	return pButton;
}

/*
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDialogEx::AddAniButton
//
//  PURPOSE:	Adds an Animated Button from a resource ID
//
// ----------------------------------------------------------------------- //

CButton* CDialogEx::AddAniButton( UINT nID, int x, int y, UINT nAniID )
{
	// Create the button
	CAniButton	*pButton = new CAniButton;

	// Set the .avi resource ID
	pButton->LoadAVI( nAniID );

	// Load the button
	if( !pButton->AutoLoad( nID, this ) )
	{
		delete pButton;
		return NULL;
	}

	// Add the button to our array
	m_collButtons.Add( pButton );

	// Set position
	pButton->SetWindowPos( NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	return pButton;
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::OnMouseMove
//
//	PURPOSE:	Mouse move handler
//
// ----------------------------------------------------------------------- //

void CDialogEx::OnMouseMove(UINT nFlags, CPoint point) 
{
	SetFocusButton(NULL);
	CDialog::OnMouseMove(nFlags, point);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDialogEx::SetFocusButton
//
//	PURPOSE:	Sets the button that has the "Focus"
//
// ----------------------------------------------------------------------- //

void CDialogEx::SetFocusButton(CButton* pButton)
{
	// Force a draw on the button that's losing focus
	if(m_pFocusButton)
	{
		m_pFocusButton->InvalidateRect(NULL);
	}

	m_pFocusButton = pButton;

	// Force a draw on the button that's gaining focus
	if(m_pFocusButton)
	{
		m_pFocusButton->InvalidateRect(NULL);
	}
}