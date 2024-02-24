/****************************************************************************
;
;	 MODULE:		CLTDialogueWnd (.h)
;
;	PURPOSE:		Class for a window with an irregular shape (like a bitmap)
;
;	HISTORY:		12/10/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#ifndef _LTDIALOGUEWND_H_
#define _LTDIALOGUEWND_H_

// Includes
#include "LTMaskedWnd.h"
#include "Timer.h"
#include "LTAlphaWnd.h"
#include "LTDecisionWnd.h"
#include "CSDefs.h"

#define DIALOGUEWND_COMMAND_CHAR	'^'

#define DIALOGUEWND_TOPLEFTFRAME		0
#define DIALOGUEWND_TOPFRAME			1
#define DIALOGUEWND_TOPRIGHTFRAME		2
#define DIALOGUEWND_RIGHTFRAME			3
#define DIALOGUEWND_BOTTOMRIGHTFRAME	4
#define DIALOGUEWND_BOTTOMFRAME			5
#define DIALOGUEWND_BOTTOMLEFTFRAME		6
#define DIALOGUEWND_LEFTFRAME			7

class CLTGUIFont;

// Classes
class CLTDialogueWnd : public CLTMaskedWnd
{
public:

	CLTDialogueWnd();
	virtual ~CLTDialogueWnd() { Term(); }
	virtual void Term();

	// Alpha window version of Init()
	virtual BOOL InitAlpha(int nControlID, char* szWndName, CLTWnd* pParentWnd, 
								const char *szFrame, CLTGUIFont* pFont, int xPos, int yPos, 
								int nWidth, int nHeight, int nAlpha = 255, int rTop = 0,
								int gTop = 0, int bTop = 0, int rBottom = 0, int gBottom = 0, 
								int bBottom = 0, DWORD dwFlags = LTWF_NORMAL, 
								DWORD dwState = LTWS_NORMAL);

	// Non alpha version of Init with a bitmap/surface and an optional frame
	virtual BOOL InitFromBitmap(int nControlID, char* szWndName, CLTWnd* pParentWnd, 
								const char *szBitmap, const char *szDecisionBitmap, 
								const char *szFrame, CLTGUIFont* pFont, int xPos, int yPos, 
								DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
	virtual BOOL Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, HSURFACE hSurf, 
								HSURFACE hDecisionSurf, const char *szFrame, CLTGUIFont* pFont,
								int xPos, int yPos, DWORD dwFlags = LTWF_NORMAL, 
								DWORD dwState = LTWS_NORMAL);

	virtual BOOL DrawToSurface(HSURFACE hSurfDest);

	virtual BOOL Update(float fTimeDelta);
	virtual BOOL ShowWindow(BOOL bShow = TRUE, BOOL bPlaySound = TRUE, BOOL bAnimate = TRUE);

	BOOL	ShowPic(BOOL bShow = TRUE) { return(m_Pic.ShowWindow(bShow)); }
	BOOL	SetPic(char *szBitmap);
	void	SetText(const char *szString) { m_csText = szString; }
	BOOL	DisplayText(char *szText, char *szAvatar, BOOL bStayOpen, char *szDecisions = NULL);
	BOOL	DisplayText(DWORD dwID, char *szAvatar, BOOL bStayOpen, char *szDecisions = NULL);
	BOOL	ParseText();
	BOOL	Skip();
	void	DoneShowing(BYTE bySelection = 0, DWORD dwSelection = 0);
	void	Reset();
	BOOL	InitFrame(const char *szFrame);
	BOOL	ShowDecisions();
	void	Open();
	void	Close();

	virtual BOOL OnLButtonDown(int xPos, int yPos);
	virtual BOOL OnRButtonDown(int xPos, int yPos);

	CLTDecisionWnd*	GetDecisionWnd() { return &m_DecisionWnd; }
	BOOL	IsDisplaying()	{ return m_bDisplaying; }
	void	SetImmediateDecisions(BOOL bSet = TRUE) { m_bImmediateDecisions = bSet; }

protected:

	CLTGUIFont *m_pFont;
	CString		m_csText;
	CLTMaskedWnd	m_Pic;
	BOOL		m_bOpening;
	BOOL		m_bClosing;
	CTimer		m_tmrClose;
	float		m_fCloseHeight;
	BOOL		m_bDisplaying;
	BOOL		m_bDecisions;
	BOOL		m_bSkipping;
	BOOL		m_bCanClose;
	BOOL		m_bCanSkip;
	BOOL		m_bMore;
	CString		m_csDecisions;
	CLTDecisionWnd	m_DecisionWnd;
	CLTAlphaWnd	m_AlphaWnd;
	CRect		m_rcFrame;
	CRect		m_rcTotal;
	BOOL		m_bImmediateDecisions;

	CLTSurfaceArray m_collFrames;
	CDWordArray m_collDialogueIDs;
};

// Inlines
inline CLTDialogueWnd::CLTDialogueWnd()
{
	m_rcFrame.SetRect(0,0,0,0);
	m_rcTotal.SetRect(0,0,0,0);
	m_pFont = NULL;
	m_bClosing = FALSE;
	m_bSkipping = FALSE;
	m_bDisplaying = FALSE;
	m_bDecisions = FALSE;
	m_bCanClose = FALSE;
	m_bCanSkip = TRUE;
	m_bMore = FALSE;
	m_bImmediateDecisions = FALSE;
}

inline void CLTDialogueWnd::DoneShowing(BYTE bySelection, DWORD dwSelection)
{
	m_bDecisions = FALSE;
	m_DecisionWnd.ShowWindow(FALSE,TRUE,FALSE);
	m_bEnabled = TRUE;

	// Notify the server that we're done showing
	HMESSAGEWRITE hMessage;
	if(bySelection)
	{
		hMessage = g_pInterface->StartMessage(CSM_DIALOGUE_DONE_SELECTION);
		g_pInterface->WriteToMessageByte(hMessage,bySelection);
		g_pInterface->WriteToMessageDWord(hMessage,dwSelection);
	}
	else
	{
		hMessage = g_pInterface->StartMessage(CSM_DIALOGUE_DONE);
	}
	g_pInterface->EndMessage(hMessage);

	m_bDisplaying = FALSE;
	m_bSkipping = FALSE;
	if(!m_bMore)
	{
		ShowWindow(FALSE,FALSE);
	}
	else
	{
		// We need to disable us until our next text comes in
		m_bEnabled = FALSE;
	}
}

inline void CLTDialogueWnd::Reset()
{
	m_bClosing = FALSE;
	m_bOpening = FALSE;
	m_bSkipping = FALSE;
	m_bDisplaying = FALSE;
	m_bDecisions = FALSE;
	m_bCanClose = FALSE;
	m_bCanSkip = TRUE;
	m_bMore = FALSE;
	m_csDecisions.Empty();
}

inline void CLTDialogueWnd::Open()
{
	// We're no longer visible, but we're enabled again
	m_bEnabled = TRUE;
	m_bOpening = FALSE;
	m_bClosing = FALSE;

	int yMid = m_yPos + (m_nHeight / 2);
	m_nHeight = (int)m_fCloseHeight;
	// Restore us to original size
	MoveWindow(m_xPos,yMid-(m_nHeight / 2));
	m_AlphaWnd.SetHeight(Max(0,m_nHeight - (int)(m_rcFrame.top+m_rcFrame.bottom)));
}

inline void CLTDialogueWnd::Close()
{
	// We're no longer visible, but we're enabled again
	m_bVisible = FALSE;
	m_bEnabled = TRUE;
	m_bClosing = FALSE;
	m_bOpening = FALSE;

	int yMid = m_yPos + (m_nHeight / 2);
	m_nHeight = (int)m_fCloseHeight;
	// Restore us to original size
	MoveWindow(m_xPos,yMid-(m_nHeight / 2));
	m_AlphaWnd.SetHeight(Max(0,m_nHeight - (int)(m_rcFrame.top+m_rcFrame.bottom)));
}

#endif