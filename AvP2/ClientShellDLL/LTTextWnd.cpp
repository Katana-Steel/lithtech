#include "StdAfx.h"
#include "LTTextWnd.h"
#include "LTGUIMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, CLTGUIFont* pFont,
					  int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	// Base class it
	if(!CLTWnd::Init(nControlID,szWndName,pParentWnd,NULL,xPos,yPos,dwFlags,dwState))
		return FALSE;

	m_pFont = pFont;

	SetText(NULL);
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::SetText
//
//	PURPOSE:	Sets the text to display
//
// ----------------------------------------------------------------------- //
DIntPt CLTTextWnd::SetText(const char *szText, int nWidth)
{
	DIntPt pt;
	pt.x = 0;
	pt.y = 0;

	// Make sure we have text
	if(!szText)
	{
		m_csText.Empty();
		ShowWindow(FALSE);
		return pt;
	}

	// Set the size of the window based on the extents of the text
	m_csText = szText;
	HSTRING hStr = g_pInterface->CreateString((char *)(LPCSTR)m_csText);
	if( m_pFont )
	{
		if(nWidth > 0)
		{
			m_nWidth = nWidth;
			pt = m_pFont->GetTextExtentsFormat(hStr,m_nWidth);
		}
		else
		{
			pt = m_pFont->GetTextExtents(hStr);
		}
	}

	m_nWidth = pt.x;
	m_nHeight = pt.y;
	g_pInterface->FreeString(hStr);
	return pt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::DrawToSurface
//
//	PURPOSE:	Draw us
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::DrawToSurface(HSURFACE hSurfDest)
{
	if(m_csText.IsEmpty())
	{
		ASSERT(FALSE);
		return TRUE;
	}

	if( m_pFont )
		m_pFont->DrawFormat((char *)(LPCSTR)m_csText,hSurfDest,GetWindowLeft(),GetWindowTop(),m_nWidth);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::ShowWindow
//
//	PURPOSE:	Shows (or hides) the window
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{
	// Call the base class disabling sound
	BOOL bRet = CLTWnd::ShowWindow(bShow,bPlaySound,bAnimate);

	// We can't go visible without text
	if(m_bVisible && m_csText.IsEmpty())
	{
		CLTWnd::ShowWindow(FALSE);
	}
	
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnLButtonDown
//
//	PURPOSE:	Left button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::OnLButtonDown(int xPos, int yPos)
{
	// Call the base class
	if(!CLTWnd::OnLButtonDown(xPos,yPos))
		return FALSE;

	// We have a click... Tell our parent about it
	CLTWnd* pWnd = this;
	CLTWnd* pParent = m_pParentWnd;
	while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
	{
		pWnd = pParent;
		pParent = pWnd->GetParent();
	}

	if(pParent)
	{
		pWnd->SendMessage(LTWM_COMMAND,LTWC_SELECT_TEXT,m_nControlID);
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTTextWnd::OnRButtonDown
//
//	PURPOSE:	Right button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTTextWnd::OnRButtonDown(int xPos, int yPos)
{
	// We have a click... Tell our parent about it
	CLTWnd* pWnd = this;
	CLTWnd* pParent = m_pParentWnd;
	while(pWnd->IsFlagSet(LTWF_FIXEDCHILD) && pParent)
	{
		pWnd = pParent;
		pParent = pWnd->GetParent();
	}

	if(pParent)
	{
		pWnd->SendMessage(LTWM_COMMAND,LTWC_SELECT_TEXT,m_nControlID);
	}
	return TRUE;
}