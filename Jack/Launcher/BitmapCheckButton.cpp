/*******************************************************************************
;
;	MODULE:			BITMAPCHECKBUTTON (.CPP)
;
;	PURPOSE:		Bitmap check button class (derived from CButton)
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/


#include "stdafx.h"
#include "Launcher.h"
#include "BitmapCheckButton.h"
#include "DlgEx.h"
#include "Resource.h"
#include "PlaySound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapCheckButton

CBitmapCheckButton::CBitmapCheckButton()
{
	m_bChecked = FALSE;
	m_HelpTextCtrl = NULL;
}

CBitmapCheckButton::~CBitmapCheckButton()
{
}


BEGIN_MESSAGE_MAP(CBitmapCheckButton, CButton)
	//{{AFX_MSG_MAP(CBitmapCheckButton)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBitmapCheckButton::OnMouseMove
//
//	PURPOSE:	Mouse move handler
//
// ----------------------------------------------------------------------- //

void CBitmapCheckButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	CDialogEx* pParent = (CDialogEx*)GetParent();
	if (pParent->GetFocusButton() != this)
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

		pParent->SetFocusButton(this);
		if(!m_bChecked && (m_bmpFocus.m_hObject != NULL))
			PlaySound(IDR_SELECT);
	}
	
	CButton::OnMouseMove(nFlags, point);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBitmapCheckButton::DrawItem
//
//	PURPOSE:	Draws the button
//
// ----------------------------------------------------------------------- //

void CBitmapCheckButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CBitmap* pBitmapToDraw;

	// Figure out which image to draw
	if(m_bChecked)
	{
		if((lpDrawItemStruct->itemState & ODS_DISABLED) && (m_bmpCheckedDisabled.m_hObject != NULL))
			pBitmapToDraw = &m_bmpCheckedDisabled;
		else
			pBitmapToDraw = &m_bmpChecked;
	}
	else
	{
		if((lpDrawItemStruct->itemState & ODS_DISABLED) && (m_bmpDisabled.m_hObject != NULL))
		{
			pBitmapToDraw = &m_bmpDisabled;
		}
		else
		{
			// If we've got the focus, draw our focus
			CDialogEx* pParent = (CDialogEx*)GetParent();
			if((pParent->GetFocusButton() == this) && (m_bmpFocus.m_hObject != NULL))
			{
				pBitmapToDraw = &m_bmpFocus;
			}
			else
			{
				pBitmapToDraw = &m_bmpNormal;
			}
		}
	}

	// Draw the button
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap* pOld = memDC.SelectObject(pBitmapToDraw);
	if (pOld == NULL)
		return;     // destructors will clean up

	BITMAP bmpInfo;
	pBitmapToDraw->GetBitmap(&bmpInfo);
	pDC->BitBlt(0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, &memDC, 0,0, SRCCOPY);

	memDC.SelectObject(pOld);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBitmapCheckButton::LoadBitmaps
//
//	PURPOSE:	Loads all the bitmap images
//
// ----------------------------------------------------------------------- //

BOOL CBitmapCheckButton::LoadBitmaps(LPCTSTR lpszNormal, LPCTSTR lpszChecked, LPCTSTR lpszDisabled,
					 LPCTSTR lpszCheckedDisabled, LPCTSTR lpszFocus)
{
	// Lil' house cleanin
	m_bmpNormal.DeleteObject();
	m_bmpChecked.DeleteObject();
	m_bmpDisabled.DeleteObject();
	m_bmpCheckedDisabled.DeleteObject();
	m_bmpFocus.DeleteObject();

	// Load 'em up.
	if (!m_bmpNormal.LoadBitmap(lpszNormal))
	{
		TRACE("Failed to load bitmap for normal image.\n");
		return FALSE;
	}
	if (!m_bmpChecked.LoadBitmap(lpszChecked))
	{
		TRACE("Failed to load bitmap for checked image.\n");
		return FALSE;
	}

	BOOL bRet = TRUE;

	if (lpszDisabled != NULL)
	{
		if (!m_bmpDisabled.LoadBitmap(lpszDisabled))
		{
			TRACE("Failed to load bitmap for disabled image.\n");
			bRet = FALSE;
		}
	}

	if (lpszCheckedDisabled != NULL)
	{
		if (!m_bmpCheckedDisabled.LoadBitmap(lpszCheckedDisabled))
		{
			TRACE("Failed to load bitmap for checked disabled image.\n");
			bRet = FALSE;
		}
	}

	if (lpszFocus != NULL)
	{
		if (!m_bmpFocus.LoadBitmap(lpszFocus))
		{
			TRACE("Failed to load bitmap for focus image.\n");
			bRet = FALSE;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBitmapCheckButton::AutoLoad
//
//	PURPOSE:	Automatically loads the bitmaps from the resource file
//
// ----------------------------------------------------------------------- //

BOOL CBitmapCheckButton::AutoLoad(UINT nID, CWnd* pParent)
{
	// first attach the CBitmapCheckButton to the dialog control
	if (!SubclassDlgItem(nID, pParent))
		return FALSE;

	CString csButtonName;
	GetWindowText(csButtonName);
	ASSERT(!csButtonName.IsEmpty());      // must provide a title

	LoadBitmaps(csButtonName + _T("N"), csButtonName + _T("C"),
	  csButtonName + _T("X"), csButtonName + _T("CX"), csButtonName + _T("F"));

	// we need at least the primary images
	if((m_bmpNormal.m_hObject == NULL) || (m_bmpChecked.m_hObject == NULL))
		return FALSE;

	// size to content
	SizeToContent();
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBitmapCheckButton::SizeToContent
//
//	PURPOSE:	Sizes the button to the bitmap
//
// ----------------------------------------------------------------------- //

void CBitmapCheckButton::SizeToContent()
{
	if(m_bmpChecked.m_hObject == NULL)
		return;

	BITMAP bmpInfo;
	m_bmpChecked.GetBitmap(&bmpInfo);
	SetWindowPos(NULL, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SWP_NOZORDER|SWP_NOMOVE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBitmapCheckButton::OnLButtonDown
//
//	PURPOSE:	Mouse button handler
//
// ----------------------------------------------------------------------- //

void CBitmapCheckButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bChecked = !m_bChecked;
	InvalidateRect(NULL);
	PlaySound(IDR_BUTTONDOWN);
	CButton::OnLButtonDown(nFlags, point);
}