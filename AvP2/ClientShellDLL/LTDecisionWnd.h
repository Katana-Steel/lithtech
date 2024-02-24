/****************************************************************************
;
;	 MODULE:		CLTDecisionWnd (.h)
;
;	PURPOSE:		Class for a decision window
;
;	HISTORY:		04/16/99 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#ifndef _LTDECISIONWND_H_
#define _LTDECISIONWND_H_

// Includes
#include "LTMaskedWnd.h"
#include "Timer.h"
#include "LTAlphaWnd.h"
#include "LTTextWnd.h"

#define MAX_DECISIONS			5
#define DECISIONWND_MAX_WIDTH	540

#define DECISIONWND_TOPLEFTFRAME		0
#define DECISIONWND_TOPFRAME			1
#define DECISIONWND_TOPRIGHTFRAME		2
#define DECISIONWND_RIGHTFRAME			3
#define DECISIONWND_BOTTOMRIGHTFRAME	4
#define DECISIONWND_BOTTOMFRAME			5
#define DECISIONWND_BOTTOMLEFTFRAME		6
#define DECISIONWND_LEFTFRAME			7


class CLTGUIFont;

// Classes
class CLTDecisionWnd : public CLTMaskedWnd
{
public:

	CLTDecisionWnd();
	virtual ~CLTDecisionWnd() { Term(); }
	virtual void Term();
	virtual BOOL InitAlpha(int nControlID, char* szWndName, CLTWnd* pParentWnd, CLTSurfaceArray* pcollFrames,
							CLTGUIFont* pFont, int xPos, int yPos, int nWidth, int nHeight, 
							int nAlpha = 255, int rTop = 0, int gTop = 0, int bTop = 0, 
							int rBottom = 0, int gBottom = 0, int bBottom = 0, 
							DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);
							
	virtual BOOL Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, HSURFACE hSurf, CLTSurfaceArray* pcollFrames,
							CLTGUIFont* pFont, int xPos, int yPos, DWORD dwFlags = LTWF_NORMAL, DWORD dwState = LTWS_NORMAL);


	virtual BOOL DrawToSurface(HSURFACE hSurfDest);

	virtual BOOL Update(float fTimeDelta);
	virtual BOOL ShowWindow(BOOL bShow = TRUE, BOOL bPlaySound = TRUE, BOOL bAnimate = TRUE);
	virtual BOOL OnLButtonDown(int xPos, int yPos);
	virtual BOOL OnRButtonDown(int xPos, int yPos);
	virtual BOOL SendMessage(int nMsg, int nParam1 = 0,int nParam2 = 0);
	BOOL	DisplayText(CStringArray *pcollDecisions,int yPos);
	BYTE	GetActivatedSelection() { return m_byActivatedSelection; }
	void	Reset() { m_bClosing = FALSE; m_bOpening = FALSE; }
	BOOL	InitFrame(CLTSurfaceArray* pcollSurfs);

protected:

	CLTGUIFont		*m_pFont;
	BOOL			m_bOpening;
	BOOL			m_bClosing;
	CTimer			m_tmrClose;
	float			m_fStartPos;
	CLTAlphaWnd		m_AlphaWnd;
	CRect			m_rcFrame;
	CRect			m_rcTotal;
	HSURFACE		m_hLeftFrame;
	int				m_nCurSelection;
	CLTTextWnd		m_DecisionWnds[MAX_DECISIONS];
	BYTE			m_byActivatedSelection;
	CLTSurfaceArray m_collFrames;
	int				m_nNumDecisions;
};

// Inlines
inline CLTDecisionWnd::CLTDecisionWnd()
{
	m_rcFrame.SetRect(0,0,0,0);
	m_rcTotal.SetRect(0,0,0,0);
	m_pFont = NULL;
	m_nCurSelection = 0;
	m_nNumDecisions = 0;
}

#endif