/****************************************************************************
;
;	 MODULE:		CLTTextWnd (.h)
;
;	PURPOSE:		Class for a window with an irregular shape (like a bitmap)
;
;	HISTORY:		12/10/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#ifndef _LTTEXTWND_H_
#define _LTTEXTWND_H_

// Includes
#include "LTWnd.h"

class CLTGUIFont;

// Classes
class CLTTextWnd : public CLTWnd
{
public:

	CLTTextWnd();
	virtual ~CLTTextWnd() { Term(); }
	virtual void Term();
	virtual BOOL Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, CLTGUIFont* pFont, int xPos = 0, int yPos = 0, DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
	DIntPt	SetText(const char *szText,int nWidth = 0);
	virtual BOOL DrawToSurface(HSURFACE hSurfDest);
	virtual BOOL ShowWindow(BOOL bShow = TRUE, BOOL bPlaySound = TRUE, BOOL bAnimate = TRUE);
	virtual BOOL OnLButtonDown(int xPos, int yPos);
	virtual BOOL OnRButtonDown(int xPos, int yPos);

protected:
	CString m_csText;
	CLTGUIFont* m_pFont;
};

typedef CArray<CLTTextWnd*,CLTTextWnd*> CLTTextWndArray;

// Inlines
inline CLTTextWnd::CLTTextWnd()
{
}

inline void CLTTextWnd::Term()
{
	// NOTE - the font was passed to us in the Init... we do not need to Term it.
	CLTWnd::Term();
}

#endif