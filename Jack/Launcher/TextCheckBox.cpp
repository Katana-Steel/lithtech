// TextCheckBox.cpp : implementation file
//

#include "stdafx.h"
#include "launcher.h"
#include "TextCheckBox.h"
#include "dlgex.h"
#include "playsound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextCheckBox

CTextCheckBox::CTextCheckBox()
:	m_bChecked (FALSE)
{
	m_crText = crNormalText;
	m_hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);

	::GetObject( (HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(m_lfNormal), &m_lfNormal );
}

CTextCheckBox::~CTextCheckBox()
{
	::DeleteObject(m_hBrush);
}


BEGIN_MESSAGE_MAP(CTextCheckBox, CButton)
	//{{AFX_MSG_MAP(CTextCheckBox)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextCheckBox message handlers


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTextCheckBox::OnMouseMove
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CTextCheckBox::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Let our parent know we have focus
	CDialogEx *pParent = (CDialogEx*)GetParent();
	if( pParent->GetFocusButton() != this ) 
	{
		// Change the help text
		if (m_HelpTextCtrl != NULL)
		{
			CRect rcCtrl;
			m_HelpTextCtrl->GetWindowRect(&rcCtrl);
			pParent->ScreenToClient(&rcCtrl);
			m_HelpTextCtrl->SetWindowText(m_csHelpText);
			pParent->InvalidateRect(&rcCtrl);
		}

		pParent->SetFocusButton( this );
		if(!m_bChecked && (m_bmpFocus.m_hObject != NULL))
			PlaySound(IDR_SELECT);
	}
			
	CButton::OnMouseMove(nFlags, point);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBitmapCheckButton::AutoLoad
//
//	PURPOSE:	Automatically loads the buton
//
// ----------------------------------------------------------------------- //

BOOL CTextCheckBox::AutoLoad(UINT nID, CWnd* pParent)
{
	// first attach the CBitmapCheckButton to the dialog control
	if (!SubclassDlgItem(nID, pParent))
		return FALSE;

	//
	// Load the bitmaps for the different states
	//
	m_bmpNormal.DeleteObject();
	m_bmpNormal.LoadBitmap( "CHECKBOXN" );

	m_bmpFocus.DeleteObject();
	m_bmpFocus.LoadBitmap( "CHECKBOXF" );

	m_bmpChecked.DeleteObject();
	m_bmpChecked.LoadBitmap( "CHECKBOXC" );

	//
	// Load the fonts and color
	//
	m_lfNormal.lfWeight = FW_DEMIBOLD;
	//m_lfNormal.lfQuality = ANTIALIASED_QUALITY;
	//strcpy( m_lfNormal.lfFaceName, "Arial" );

	SizeToContent();

	return TRUE;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTextCheckBox::DrawItem
//
//  PURPOSE:	Manualy draw the button.
//
// ----------------------------------------------------------------------- //

void CTextCheckBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC			memDC;
	CDC			*pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CBitmap		*pNewBitmap;
	CFont		cFont;
	
	// Grab the bitmap and font based on our check state
	if( m_bChecked )
	{
		m_crText = RGB(128, 128, 128);
		pNewBitmap = &m_bmpChecked;
	}
	else
	{
		// See if we have focus
		CDialogEx	*pParent = (CDialogEx*)GetParent();
		if( (pParent->GetFocusButton() == this) && (m_bmpNormal.m_hObject != NULL) )
		{
			m_crText = crHighlightText;
			pNewBitmap = &m_bmpFocus;
		}
		else
		{
			m_crText = crNormalText;
			pNewBitmap = &m_bmpNormal;
		}
	}

	//
	// Display the appropriate CheckBox bitmap
	//
	memDC.CreateCompatibleDC( pDC );
	CBitmap	*pOldBitmap = memDC.SelectObject( pNewBitmap );
	if( !pOldBitmap )
		return;

	BITMAP	bmpInfo;
	m_bmpNormal.GetBitmap( &bmpInfo );
	pDC->BitBlt( 0, 1, bmpInfo.bmWidth, bmpInfo.bmHeight, &memDC, 0, 0, SRCCOPY );
	memDC.SelectObject( pOldBitmap );

	//
	// Display the text with the appropriate font and color
	//
	cFont.CreateFontIndirect( &m_lfNormal );
	CFont	*pOldFont = pDC->SelectObject( &cFont );

	pDC->SetTextColor(m_crText);
	pDC->SetBkMode(TRANSPARENT);
	pDC->TextOut(bmpInfo.bmWidth + 2, 0, m_strWinText );

	pDC->SelectObject( pOldFont );

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTextCheckBox::OnLButtonDown
//
//  PURPOSE:	Check or uncheck the box
//
// ----------------------------------------------------------------------- //

void CTextCheckBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bChecked = !m_bChecked;
	InvalidateRect(NULL);
	PlaySound(IDR_BUTTONDOWN);
	
	CButton::OnLButtonDown(nFlags, point);
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTextCheckBox::SizeToContent
//
//  PURPOSE:	Resize the button to fit the bitmap and text
//
// ----------------------------------------------------------------------- //

void CTextCheckBox::SizeToContent( void )
{
	if(m_bmpNormal.m_hObject == NULL)
		return;

	CDC	*pDC = GetDC();

	BITMAP bmpInfo;
	m_bmpNormal.GetBitmap(&bmpInfo);
	
	CSize TextSize = pDC->GetOutputTextExtent( m_strWinText );

	SetWindowPos(NULL, 0, 0, bmpInfo.bmWidth + TextSize.cx, bmpInfo.bmHeight + 5, SWP_NOZORDER|SWP_NOMOVE);
}
