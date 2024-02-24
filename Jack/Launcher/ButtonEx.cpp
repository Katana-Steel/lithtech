/*******************************************************************************
;
;	MODULE:			BUTTONEX (.CPP)
;
;	PURPOSE:		Extended button class (derived from CBitmapButton)
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "ButtonEx.h"
#include "Resource.h"
#include "DlgEx.h"
#include "PlaySound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CButtonEx

CButtonEx::CButtonEx()
{
	m_HelpTextCtrl	= NULL;
	m_hCursor		= NULL;
}

CButtonEx::~CButtonEx()
{
}


BEGIN_MESSAGE_MAP(CButtonEx, CBitmapButton)
	//{{AFX_MSG_MAP(CButtonEx)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButtonEx::OnMouseMove
//
//	PURPOSE:	Mouse move handler
//
// ----------------------------------------------------------------------- //

void CButtonEx::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDialogEx* pParent = (CDialogEx*)GetParent();
	if (pParent->GetFocusButton() != this)
	{
		// Change the help text
		if(m_HelpTextCtrl != NULL)
		{
			CRect rcCtrl;
			m_HelpTextCtrl->GetWindowRect(&rcCtrl);
			pParent->ScreenToClient(&rcCtrl);
			m_HelpTextCtrl->SetWindowText(m_csHelpText);
			pParent->InvalidateRect(&rcCtrl);
		}

		pParent->SetFocusButton(this);
		if(m_bitmapFocus.m_hObject != NULL)
			PlaySound(IDR_SELECT);

	}
	
	CBitmapButton::OnMouseMove(nFlags, point);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButtonEx::OnLButtonDown
//
//	PURPOSE:	Mouse button handler
//
// ----------------------------------------------------------------------- //

void CButtonEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	PlaySound(IDR_BUTTONDOWN);
	
	CBitmapButton::OnLButtonDown(nFlags, point);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButtonEx::DrawItem
//
//	PURPOSE:	Draws the button
//
// ----------------------------------------------------------------------- //

void CButtonEx::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	ASSERT(lpDIS != NULL);
	// must have at least the first bitmap loaded before calling DrawItem
	ASSERT(m_bitmap.m_hObject != NULL);     // required

	// use the main bitmap for up, the selected bitmap for down
	CBitmap* pBitmap = &m_bitmap;
	UINT state = lpDIS->itemState;
	
	// Check for selected
	if ((state & ODS_SELECTED) && m_bitmapSel.m_hObject != NULL)
	{
		pBitmap = &m_bitmapSel;
	}
	else
	{	
		// Check for focus
		CDialogEx* pParent = (CDialogEx*)GetParent();
		if((pParent->GetFocusButton() == this) && (m_bitmapFocus.m_hObject != NULL))
		{
			pBitmap = &m_bitmapFocus;   // third image for focused
		}
		else
		{
			// Check for disabled
			if ((state & ODS_DISABLED) && m_bitmapDisabled.m_hObject != NULL)
			{
				pBitmap = &m_bitmapDisabled;   // last image for disabled
			}
		}
	}

	// draw the whole button
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap* pOld = memDC.SelectObject(pBitmap);
	if (pOld == NULL)
		return;     // destructors will clean up

	CRect rect;
	rect.CopyRect(&lpDIS->rcItem);
	pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(),
		&memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOld);

}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CButtonEx::OnSetCursor
//
//  PURPOSE:	Sets the cursor to our own when the cursor is over us
//
// ----------------------------------------------------------------------- //
BOOL CButtonEx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if( m_hCursor )
	{
		::SetCursor( m_hCursor );
		return TRUE;
	}

	return CBitmapButton::OnSetCursor( pWnd, nHitTest, message );
}

