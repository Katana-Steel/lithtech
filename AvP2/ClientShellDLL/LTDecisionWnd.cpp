/****************************************************************************
;
;	 MODULE:		CLTDecisionWnd (.cpp)
;
;	PURPOSE:		Class for a decision window
;
;	HISTORY:		04/16/99 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#include "StdAfx.h"
#include "LTDecisionWnd.h"
#include "LTGuiMgr.h"

#ifdef _AVP2BUILD
#include "GameClientShell.h"
#define g_pPrClientShell g_pGameClientShell
extern CGameClientShell* g_pGameClientShell;
#else
#include "ClientShell.h"
#endif

BOOL CLTDecisionWnd::Init(int nControlID, char* szWndName, CLTWnd* pParentWnd, HSURFACE hSurf, CLTSurfaceArray* pcollFrames,
							CLTGUIFont* pFont, int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized || !pFont)
		return FALSE;

	// Call the base class
	if(!CLTMaskedWnd::Init(nControlID,szWndName,pParentWnd,hSurf,xPos,yPos,dwFlags,dwState))
		return FALSE;

	InitFrame(pcollFrames);

	m_pFont = pFont;

	// Set up the decision windows
	for(int i=0;i<MAX_DECISIONS;i++)
	{
		m_DecisionWnds[i].Init(i,"Decision",this,m_pFont,0,0,LTWF_FIXEDCHILD);
	}

	Reset();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::InitAlpha(int nControlID, char* szWndName, CLTWnd* pParentWnd, CLTSurfaceArray* pcollFrames,
					   CLTGUIFont* pFont, int xPos, int yPos, int nWidth, int nHeight, int nAlpha, 
					   int rTop, int gTop, int bTop, int rBottom, int gBottom, int bBottom, 
					   DWORD dwFlags, DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized || !pFont)
		return FALSE;

	// Call the base class
	if(!CLTMaskedWnd::Init(nControlID,szWndName,pParentWnd,NULL,xPos,yPos,dwFlags,dwState))
		return FALSE;

	// Simple vars
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	if(!InitFrame(pcollFrames))
		return FALSE;

	// Init the alpha window
	if(!m_AlphaWnd.Init(nControlID,"AlphaWnd",this,m_rcFrame.left,m_rcFrame.top,
		nWidth-(m_rcFrame.left+m_rcFrame.right),nHeight-(m_rcFrame.top+m_rcFrame.bottom),nAlpha,
		rTop,gTop,bTop,rBottom,gBottom,bBottom,LTWF_FIXEDCHILD | LTWF_MANUALDRAW | LTWF_DISABLED))
	{
		return FALSE;
	}

	m_pFont = pFont;

	// Set up the decision windows
	for(int i=0;i<MAX_DECISIONS;i++)
	{
		m_DecisionWnds[i].Init(i,"Decision",this,m_pFont,0,0,LTWF_FIXEDCHILD);
	}

	Reset();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //
void CLTDecisionWnd::Term()
{
	// Term the decision windows
	for(int i=0;i<MAX_DECISIONS;i++)
	{
		m_DecisionWnds[i].Term();
	}

	// Term the alpha window
	m_AlphaWnd.Term();

	CLTMaskedWnd::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::DrawToSurface
//
//	PURPOSE:	Draws the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::DrawToSurface(HSURFACE hSurfDest)
{
	ASSERT(m_bInitialized);

	// Draw the alpha window
	if(m_AlphaWnd.IsInitialized())
	{
		m_AlphaWnd.DrawToSurface(hSurfDest);
	}
	else
	{
		if(!CLTMaskedWnd::DrawToSurface(hSurfDest)) 
			return FALSE;
	}

	// Draw the frame
	if(m_collFrames.GetSize() == 8)
	{
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_TOPLEFTFRAME],NULL,m_xPos,m_yPos,m_hColorTransparent);
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_TOPRIGHTFRAME],NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos,m_hColorTransparent);
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_BOTTOMRIGHTFRAME],NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos+m_nHeight-m_rcTotal.bottom,m_hColorTransparent);
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_BOTTOMLEFTFRAME],NULL,m_xPos,m_yPos+m_nHeight-m_rcTotal.bottom,m_hColorTransparent);

		// Draw the four scaleable sides
		DRect rcDest;
		rcDest.left = m_xPos;
		rcDest.top = m_yPos+m_rcTotal.top;
		rcDest.right = m_xPos+m_rcFrame.left;
		rcDest.bottom = m_yPos+m_nHeight-m_rcTotal.bottom;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_LEFTFRAME],&rcDest,NULL,m_hColorTransparent);
		rcDest.left = m_xPos + m_nWidth - m_rcFrame.right;
		rcDest.right = m_xPos + m_nWidth;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_RIGHTFRAME],&rcDest,NULL,m_hColorTransparent);
		rcDest.left = m_xPos+m_rcTotal.left;
		rcDest.top = m_yPos;
		rcDest.right = m_xPos+m_nWidth-m_rcTotal.right;
		rcDest.bottom = m_yPos+m_rcFrame.top;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_TOPFRAME],&rcDest,NULL,m_hColorTransparent);
		rcDest.top = m_yPos + m_nHeight - m_rcFrame.bottom;
		rcDest.bottom = m_yPos + m_nHeight;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DECISIONWND_BOTTOMFRAME],&rcDest,NULL,m_hColorTransparent);
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::ShowWindow
//
//	PURPOSE:	Shows (or hides) the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{
	if(!bAnimate)
		return CLTMaskedWnd::ShowWindow(bShow,bPlaySound,bAnimate);

	if(m_bClosing || m_bOpening)
		return FALSE;

	// Call the base class disabling sound
	BOOL bRet = CLTMaskedWnd::ShowWindow(bShow,FALSE);

	// If we closed, start the closing animation
	if(bRet)
	{
		if(!bShow)
		{
			// We're temporarily visible just so we can close, but we're disabled
			m_bVisible = TRUE;
			m_bEnabled = FALSE;
	
			m_bClosing = TRUE;
			m_tmrClose.Start(0.15f /*g_bm.GetFloat("DialogueWnd","CloseTime",1.0f)*/);
			m_fStartPos = (float)m_nHeight;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::Update
//
//	PURPOSE:	Updates the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::Update(float fTimeDelta)
{
	ASSERT(m_bInitialized);

	if(m_bOpening)
	{
		// Move us towards the center
		int xDest = (g_pPrClientShell->GetScreenWidth()-m_nWidth)/2;
		int xPos = (int)(xDest + ((m_fStartPos - xDest)*m_tmrClose.GetCountdownTime()/m_tmrClose.GetDuration()));
		if(m_tmrClose.Stopped() || (xPos <= xDest))
		{
			m_bEnabled = TRUE;
			m_bOpening = FALSE;
			MoveWindow(xDest,m_yPos);
			return TRUE;
		}
		MoveWindow(xPos,m_yPos);
	}
	else
	if(m_bClosing)
	{
		// Shrink us towards the middle
		int yMid = m_yPos + (m_nHeight / 2);
		int nHeight = (int)(m_fStartPos - (m_fStartPos * (m_tmrClose.GetElapseTime() / m_tmrClose.GetDuration())));
		if(m_tmrClose.Stopped() || (nHeight < (m_rcTotal.top + m_rcTotal.bottom)))
		{
			// We're no longer visible, but we're enabled again
			m_bVisible = FALSE;
			m_bEnabled = TRUE;

			m_bClosing = FALSE;
			m_nHeight = (int)m_fStartPos;
			// Restore us to original size
			MoveWindow(m_xPos,yMid-(m_nHeight / 2));
			m_AlphaWnd.SetHeight(Max(0,m_nHeight - (int)(m_rcFrame.top+m_rcFrame.bottom)));
			return TRUE;
		}
		MoveWindow(m_xPos,yMid - (nHeight / 2),m_nWidth,nHeight);
		m_AlphaWnd.SetHeight(Max(0,nHeight - (int)(m_rcFrame.top+m_rcFrame.bottom)));
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::OnRButtonDown
//
//	PURPOSE:	Right mouse button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::OnRButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	// Right mouse button selects the current selection
	SendMessage(LTWM_COMMAND,LTWC_SELECT_TEXT,m_nCurSelection);
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::OnLButtonDown
//
//	PURPOSE:	Left mouse button down handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::OnLButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	// Call the base class
	return(CLTMaskedWnd::OnLButtonDown(xPos,yPos));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::DisplayText
//
//	PURPOSE:	Sets up the decisions
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::DisplayText(CStringArray *pcollDecisions,int yPos)
{
	m_nNumDecisions = 0;

	// Gotta have at least 2 decisions
	if(!pcollDecisions || pcollDecisions->GetSize() < 2)
		return FALSE;

	// Term the decision windows
	for(int i=0;i<MAX_DECISIONS;i++)
	{
		m_DecisionWnds[i].SetText(NULL);
	}

	// Set the decisions
	DIntPt ptMax;
	DIntPt ptCur;
	ptMax.x = 0;
	ptMax.y = 0;
	m_nCurSelection = 0;
	m_byActivatedSelection = 0;
	for(i=0;((i<pcollDecisions->GetSize()) && (i<MAX_DECISIONS));i++)
	{
		if(pcollDecisions->GetAt(i).IsEmpty())
			return FALSE;

		// Set the window position
		ptCur = m_DecisionWnds[i].SetText(pcollDecisions->GetAt(i),DECISIONWND_MAX_WIDTH);
		m_DecisionWnds[i].MoveWindow(20,20+ptMax.y);
		ptMax.x = Max(ptMax.x,ptCur.x);
		ptMax.y += ptCur.y;

		// If we've got another decision, add space inbetween
		if(i+1 < pcollDecisions->GetSize())
			ptMax.y += 10;
	}

	// Set our number of decisions
	m_nNumDecisions = pcollDecisions->GetSize();

	// Set our size based on the extents of the text
	int nWidth = ptMax.x + 40;
	int nHeight = ptMax.y + 40;

	m_AlphaWnd.SetWidth(Max(0,nWidth - (int)(m_rcFrame.left+m_rcFrame.right)));
	m_AlphaWnd.SetHeight(Max(0,nHeight - (int)(m_rcFrame.top+m_rcFrame.bottom)));

	// Put us at the edge of the screen
	MoveWindow(g_pPrClientShell->GetScreenWidth()-1,yPos,nWidth,nHeight);
	m_fStartPos = (float)m_xPos;

	// Show us
	ShowWindow();
	ShowAllChildren();

	m_bOpening = TRUE;
	m_tmrClose.Start(0.5f /*g_bm.GetFloat("DecisionWnd","OpenTime",1.0f)*/);

	// Move us to the top
	SetFocus();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDecisionWnd::SendMessage
//
//	PURPOSE:	Handles a sent message
//
// ----------------------------------------------------------------------- //
BOOL CLTDecisionWnd::SendMessage(int nMsg, int nParam1,int nParam2)
{
	if(nMsg == LTWM_COMMAND)
	{
		if(nParam1 == LTWC_SELECT_TEXT)
		{
			if(m_byActivatedSelection)
				return TRUE;

			if(nParam2 >= m_nNumDecisions)
				return TRUE;
	
			m_byActivatedSelection = BYTE(nParam2 + 1);
			TRACE("Choice # %d selected!\n",m_byActivatedSelection);
	
			return TRUE;
		}
	}
	return(CLTMaskedWnd::SendMessage(nMsg,nParam1,nParam2));
}

BOOL CLTDecisionWnd::InitFrame(CLTSurfaceArray* pcollSurfs)
{
	if(!pcollSurfs || (pcollSurfs->GetSize() < 8))
		return FALSE;

	for(int i=0;i<pcollSurfs->GetSize();i++)
	{
		m_collFrames.Add(pcollSurfs->GetAt(i));
	}

	// Get the frame thicknesses
	DWORD x,y;
	g_pInterface->GetSurfaceDims(m_collFrames[DECISIONWND_LEFTFRAME],&x,&y);
	m_rcFrame.left = x;
	g_pInterface->GetSurfaceDims(m_collFrames[DECISIONWND_TOPFRAME],&x,&y);
	m_rcFrame.top = y;
	g_pInterface->GetSurfaceDims(m_collFrames[DECISIONWND_RIGHTFRAME],&x,&y);
	m_rcFrame.right = x;
	g_pInterface->GetSurfaceDims(m_collFrames[DECISIONWND_BOTTOMFRAME],&x,&y);
	m_rcFrame.bottom = y;

	g_pInterface->GetSurfaceDims(m_collFrames[DECISIONWND_TOPLEFTFRAME],&x,&y);
	m_rcTotal.left = x;
	m_rcTotal.top = y;
	g_pInterface->GetSurfaceDims(m_collFrames[DECISIONWND_BOTTOMRIGHTFRAME],&x,&y);
	m_rcTotal.right = x;
	m_rcTotal.bottom = y;

	return TRUE;
}