/****************************************************************************
;
;	 MODULE:		CLTDialogueWnd (.cpp)
;
;	PURPOSE:		Class for a dialogue window
;
;	HISTORY:		12/10/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#include "StdAfx.h"
#include "LTDialogueWnd.h"
#include "LTGuiMgr.h"
#include "LTDecisionWnd.h"

#ifdef _AVP2BUILD
#include "GameClientShell.h"
#define g_pPrClientShell g_pGameClientShell
extern CGameClientShell* g_pGameClientShell;
#else
#include "ClientShell.h"
#include "Play.h"
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::InitFromBitmap(int nControlID, char* szWndName, CLTWnd* pParentWnd, 
						  const char *szBitmap, const char *szDecisionBitmap, 
						  const char *szFrame, CLTGUIFont* pFont, int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized || !pFont)
		return FALSE;

	// We will be deleting the surface
	m_bDeleteSurfOnTerm = TRUE;

	// Create the surface
	HSURFACE hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)szBitmap);
	if(!hSurf)
	{
		TRACE("CLTDisplayWnd::InitFromBitmap - ERROR - Could not create the surface: ""%s""\n",szBitmap);
		return FALSE;
	}

	HSURFACE hDecisionSurf = g_pInterface->CreateSurfaceFromBitmap((char *)szDecisionBitmap);
	if(!hDecisionSurf)
	{
		TERMSURF(hSurf);
		TRACE("CLTDisplayWnd::InitFromBitmap - ERROR - Could not create the surface: ""%s""\n",szDecisionBitmap);
		return FALSE;
	}

	if(!Init(nControlID,szWndName,pParentWnd,hSurf,hDecisionSurf,szFrame,pFont,xPos,yPos,dwFlags,dwState))
	{
		TERMSURF(hSurf);
		TERMSURF(hDecisionSurf);
		return FALSE;
	}

	return TRUE;
}

BOOL CLTDialogueWnd::Init(int nControlID, char *szWndName, CLTWnd* pParentWnd, HSURFACE hSurf,
						  HSURFACE hDecisionSurf, const char *szFrame, CLTGUIFont* pFont,
						  int xPos, int yPos, DWORD dwFlags, DWORD dwState)
{
	// Sanity Check
	if(m_bInitialized || !pFont)
		return FALSE;

	// Call the base class
	if(!CLTMaskedWnd::Init(nControlID,szWndName,pParentWnd,hSurf,xPos,yPos,dwFlags,dwState))
		return FALSE;

	InitFrame(szFrame);

	m_pFont = pFont;

	// Init the decision window
	if(!m_DecisionWnd.Init(0,"Decision",pParentWnd,hDecisionSurf,&m_collFrames,pFont,0,0,LTWF_SIZEABLE,LTWS_CLOSED))
	{
		return FALSE;
	}

	Reset();

	// Center us
	MoveWindow((g_pPrClientShell->GetScreenWidth()-m_nWidth)/2,40);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::InitAlpha(int nControlID, char* szWndName, CLTWnd* pParentWnd, const char *szFrame,
							   CLTGUIFont* pFont, int xPos, int yPos, int nWidth, int nHeight, 
							   int nAlpha, int rTop, int gTop, int bTop, int rBottom, int gBottom, int bBottom,
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

	if(!InitFrame(szFrame))
		return FALSE;

	// Init the alpha window
	if(!m_AlphaWnd.Init(nControlID,"AlphaWnd",this,m_rcFrame.left,m_rcFrame.top,
		nWidth-(m_rcFrame.left+m_rcFrame.right),nHeight-(m_rcFrame.top+m_rcFrame.bottom),nAlpha,
		rTop,gTop,bTop,rBottom,gBottom,bBottom,LTWF_FIXEDCHILD | LTWF_MANUALDRAW | LTWF_DISABLED))
	{
		return FALSE;
	}

	// Set up the font
	m_pFont = pFont;

	// Init the decision window
	if(!m_DecisionWnd.InitAlpha(0,"Decision",pParentWnd,&m_collFrames,pFont,0,0,100,100,nAlpha,
		rTop,gTop,bTop,rBottom,gBottom,bBottom,LTWF_SIZEABLE,LTWS_CLOSED))
	{
		return FALSE;
	}

	Reset();

	// Center us
	MoveWindow((g_pPrClientShell->GetScreenWidth()-m_nWidth)/2,40);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //
void CLTDialogueWnd::Term()
{
	// Term the frames
	for(int i=0;i<m_collFrames.GetSize();i++)
	{
		TERMSURF(m_collFrames[i]);
	}
	m_collFrames.RemoveAll();

	// Term the decision window
	m_DecisionWnd.Term();

	// Term the picture
	m_Pic.Term();

	// Term the alpha window
	m_AlphaWnd.Term();

	// Base class
	CLTMaskedWnd::Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::DrawToSurface
//
//	PURPOSE:	Does the dirty work for drawing a single window to a surface
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::DrawToSurface(HSURFACE hSurfDest)
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
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_TOPLEFTFRAME],NULL,m_xPos,m_yPos,m_hColorTransparent);
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_TOPRIGHTFRAME],NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos,m_hColorTransparent);
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_BOTTOMRIGHTFRAME],NULL,m_xPos+m_nWidth-m_rcTotal.right,m_yPos+m_nHeight-m_rcTotal.bottom,m_hColorTransparent);
		g_pInterface->DrawSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_BOTTOMLEFTFRAME],NULL,m_xPos,m_yPos+m_nHeight-m_rcTotal.bottom,m_hColorTransparent);

		// Draw the four scaleable sides
		DRect rcDest;
		rcDest.left = m_xPos;
		rcDest.top = m_yPos+m_rcTotal.top;
		rcDest.right = m_xPos+m_rcFrame.left;
		rcDest.bottom = m_yPos+m_nHeight-m_rcTotal.bottom;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_LEFTFRAME],&rcDest,NULL,m_hColorTransparent);
		rcDest.left = m_xPos + m_nWidth - m_rcFrame.right;
		rcDest.right = m_xPos + m_nWidth;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_RIGHTFRAME],&rcDest,NULL,m_hColorTransparent);
		rcDest.left = m_xPos+m_rcTotal.left;
		rcDest.top = m_yPos;
		rcDest.right = m_xPos+m_nWidth-m_rcTotal.right;
		rcDest.bottom = m_yPos+m_rcFrame.top;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_TOPFRAME],&rcDest,NULL,m_hColorTransparent);
		rcDest.top = m_yPos + m_nHeight - m_rcFrame.bottom;
		rcDest.bottom = m_yPos + m_nHeight;
		g_pInterface->ScaleSurfaceToSurfaceTransparent(hSurfDest,m_collFrames[DIALOGUEWND_BOTTOMFRAME],&rcDest,NULL,m_hColorTransparent);
	}

	// Draw the font
	if(!m_bClosing && !m_bOpening && m_pFont)
	{
		int space = 10+20;
		if(m_Pic.IsInitialized())
			space += m_Pic.GetWidth();
		int width = m_nWidth - space - 20;
		m_pFont->DrawFormat((char *)(LPCSTR)m_csText,hSurfDest,GetWindowLeft()+space,GetWindowTop()+20,width);
		return TRUE;
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::SetPic
//
//	PURPOSE:	Sets the picture
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::SetPic(char *szBitmap)
{
	// Make sure it's not already initialized - this will take care of freeing any surface
	// that was previously created
	if(m_Pic.IsInitialized())
		m_Pic.Term();

	if(!m_Pic.InitFromBitmap(0,"DialogueWnd Pic",this,szBitmap,20,20,LTWF_TRANSPARENT | LTWF_DISABLED))
		return FALSE;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::ShowWindow
//
//	PURPOSE:	Shows (or hides) the window
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::ShowWindow(BOOL bShow, BOOL bPlaySound, BOOL bAnimate)
{
	if(!bAnimate)
		return CLTMaskedWnd::ShowWindow(bShow,bPlaySound,bAnimate);

	//if(m_bClosing || m_bOpening) // We might be closing and something wants us to open
	if(m_bOpening)
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
			m_fCloseHeight = (float)m_nHeight;
			g_pPrClientShell->PlaySoundLocal("/interface/sounds/DialogueClose.wav");
			// Hide the picture
			if(m_Pic.IsInitialized())
				m_Pic.ShowWindow(FALSE,FALSE);
#ifndef _AVP2BUILD
			// Unlock the player
			CPlay* pPlay = (CPlay*)g_pPrClientShell->GetState();
			if(pPlay && g_pPrClientShell->IsPlayState())
				pPlay->UnlockPlayer();
#endif
		}
		else
		{
			// We were visible, and we're supposed to show (we might have been closing tho)
			if(m_bClosing)
			{
				Open();
#ifndef _AVP2BUILD
				// Lock the player
				CPlay* pPlay = (CPlay*)g_pPrClientShell->GetState();
				if(pPlay && g_pPrClientShell->IsPlayState())
					pPlay->LockPlayer();
#endif
			}
		}
	}
	else
	{
		if(bShow)
		{
			m_bEnabled = FALSE;
			m_bOpening = TRUE;
			m_tmrClose.Start(0.15f /*g_bm.GetFloat("DialogueWnd","CloseTime",1.0f)*/);
			m_fCloseHeight = (float)m_nHeight;
			g_pPrClientShell->PlaySoundLocal("/interface/sounds/DialogueOpen.wav");
#ifndef _AVP2BUILD
			// Lock the player
			CPlay* pPlay = (CPlay*)g_pPrClientShell->GetState();
			if(pPlay && g_pPrClientShell->IsPlayState())
				pPlay->LockPlayer();
#endif
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Update
//
//	PURPOSE:	Main update function
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::Update(float fTimeDelta)
{
	ASSERT(m_bInitialized);

	if(m_bOpening)
	{
		// Open us from the middle
		int nHeight = (int)(m_fCloseHeight - (m_fCloseHeight * (m_tmrClose.GetCountdownTime() / m_tmrClose.GetDuration())));
		if(m_tmrClose.Stopped() || (nHeight >= m_fCloseHeight))
		{
			Open();
			return TRUE;
		}
		int yMid = m_yPos + (m_nHeight / 2);
		MoveWindow(m_xPos,yMid - (nHeight / 2),m_nWidth,nHeight);
		m_AlphaWnd.SetHeight(Max(0,nHeight - (int)(m_rcFrame.top+m_rcFrame.bottom)));
	}
	else if(m_bClosing)
	{
		// Shrink us towards the middle
		
		int nHeight = (int)(m_fCloseHeight - (m_fCloseHeight * (m_tmrClose.GetElapseTime() / m_tmrClose.GetDuration())));
		if(m_tmrClose.Stopped() || (nHeight < (m_rcTotal.top + m_rcTotal.bottom)))
		{
			Close();
			return TRUE;
		}
		int yMid = m_yPos + (m_nHeight / 2);
		MoveWindow(m_xPos,yMid - (nHeight / 2),m_nWidth,nHeight);
		m_AlphaWnd.SetHeight(Max(0,nHeight - (int)(m_rcFrame.top+m_rcFrame.bottom)));
	}
	else if(m_bDecisions)
	{
		BYTE bySelection = m_DecisionWnd.GetActivatedSelection();
		if(bySelection)
		{
			int nDecision = bySelection;
			if (bySelection > m_collDialogueIDs.GetSize())
				nDecision = m_collDialogueIDs.GetSize();

			DoneShowing(nDecision,m_collDialogueIDs[nDecision-1]);
		}
	}
	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::OnLButtonDown
//
//	PURPOSE:	Left mouse button handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::OnLButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	// Call the base class
	CLTMaskedWnd::OnLButtonDown(xPos,yPos);

	// Skip if necessary
	if(m_bCanSkip)
	{
		Skip();
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::DisplayText
//
//	PURPOSE:	Sets up the dialogue text
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::DisplayText(DWORD dwID, char *szAvatar, BOOL bStayOpen, char *szDecisions)
{
	//if(m_bOpening || m_bClosing)
		//return FALSE;

	HSTRING hString=g_pInterface->FormatString(dwID);
	CString csText;
	char *szText;
	if(hString)
		szText = g_pInterface->GetStringData(hString);
	else
	{
		csText.Format("ERROR - NO TEXT FOUND FOR ID: %d",dwID);
		szText = (char *)(LPCSTR)csText;
	}

	BOOL bRet = DisplayText(szText,szAvatar,bStayOpen,szDecisions);
	g_pInterface->FreeString(hString);

	if(bRet && m_bImmediateDecisions)
		ShowDecisions();

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::DisplayText
//
//	PURPOSE:	Sets up the dialogue text
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::DisplayText(char *szText, char *szAvatar, BOOL bStayOpen, char *szDecisions)
{
	if(m_bDisplaying || m_bDecisions)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if(!szText)
	{
		m_csText = "ERROR - NO TEXT FOUND!";

	}
	else
	{
		// Set the text
		m_csText = szText;
	}

	// If we weren't visible, reset our command parameters to the default
	m_bCanClose = FALSE;
	m_bCanSkip = TRUE;
	m_bMore = bStayOpen;

	// Parse the text for parameters
	if(!ParseText())
		return FALSE;

	m_csDecisions.Empty();
	if(szDecisions)
	{
		m_csDecisions = szDecisions;
	}

	// Set the pic
	if(szAvatar && szAvatar[0])
	{
		CString csPic;
		csPic.Format("/interface/avatar/%s.pcx",szAvatar);
		SetPic((char *)(LPCSTR)csPic);
	}

	// Show us
	ShowWindow();
	ShowAllChildren();

	// Move us to the top and enable us
	m_bEnabled = TRUE;
	SetFocus();

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::ParseText
//
//	PURPOSE:	Parse the text for commands
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::ParseText()
{
	CString csText = m_csText;
	m_csText.Empty();
	CString csCmd;

	int nStart = csText.Find(DIALOGUEWND_COMMAND_CHAR);
	int nEnd;
	while(nStart != -1)
	{
		// Add on the non-command text
		m_csText += csText.Left(nStart);

		// Find the command
		csText = csText.Right(csText.GetLength()-nStart-1);

		nEnd = csText.Find(DIALOGUEWND_COMMAND_CHAR);
		if(nEnd == -1)
		{
			// Found start command with no end command
			TRACE("CLTDialogueWnd::ParseText - ERROR - found start command without end command\n");
			return FALSE;
		}

		csCmd = csText.Left(nEnd);

		if(csCmd.IsEmpty())
		{
			// They just wanted the special character, add it on
			m_csText += DIALOGUEWND_COMMAND_CHAR;
		}
		else
		{
			// TODO: Handle the command
		}

		// Trim off the command
		csText = csText.Right(csText.GetLength()-nEnd-1);
		nStart = csText.Find(DIALOGUEWND_COMMAND_CHAR);
	}
	
	// Add on any remaining non-command text
	m_csText += csText;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::OnRButtonDown
//
//	PURPOSE:	Right mouse button handler
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::OnRButtonDown(int xPos, int yPos)
{
	// If the window isn't enabled, just return
	if(!m_bEnabled || !m_bVisible)
		return FALSE;

	// Skip if necessary
	if(m_bCanSkip)
	{
		Skip();
		return TRUE;
	}

	// Call the base class
	return(CLTMaskedWnd::OnRButtonDown(xPos,yPos));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::Skip
//
//	PURPOSE:	Skips dialogue playing or skips to the next dialogue
//				if we're done reading.
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::Skip()
{
	ASSERT(!m_bDecisions);

	// If we're displaying text (scrolling), then we need to skip before we can move on
	if(m_bDisplaying)
	{
		m_bSkipping = TRUE;
	}
	else
	{
		ShowDecisions();

		if(!m_bDecisions)
		{
			DoneShowing();
		}
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::InitFrame
//
//	PURPOSE:	Initializes the frame
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::InitFrame(const char *szFrame)
{
	// Init the frame
	if(szFrame)
	{
		HSURFACE hSurf;
		// Init the frames
		CString csFrame;
		csFrame.Format("%stopleft.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%stop.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%stopright.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sright.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottomright.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottom.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sbottomleft.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		csFrame.Format("%sleft.pcx",szFrame);
		hSurf = g_pInterface->CreateSurfaceFromBitmap((char *)(LPCSTR)csFrame);
		if(!hSurf)
		{
			TRACE("CLTDialogueWnd::Init - ERROR - Could not create the surface: ""%s""\n",csFrame);
			Term();
			return FALSE;
		}
		m_collFrames.Add(hSurf);
		
		// Get the frame thicknesses
		DWORD x,y;
		g_pInterface->GetSurfaceDims(m_collFrames[DIALOGUEWND_LEFTFRAME],&x,&y);
		m_rcFrame.left = x;
		g_pInterface->GetSurfaceDims(m_collFrames[DIALOGUEWND_TOPFRAME],&x,&y);
		m_rcFrame.top = y;
		g_pInterface->GetSurfaceDims(m_collFrames[DIALOGUEWND_RIGHTFRAME],&x,&y);
		m_rcFrame.right = x;
		g_pInterface->GetSurfaceDims(m_collFrames[DIALOGUEWND_BOTTOMFRAME],&x,&y);
		m_rcFrame.bottom = y;

		g_pInterface->GetSurfaceDims(m_collFrames[DIALOGUEWND_TOPLEFTFRAME],&x,&y);
		m_rcTotal.left = x;
		m_rcTotal.top = y;
		g_pInterface->GetSurfaceDims(m_collFrames[DIALOGUEWND_BOTTOMRIGHTFRAME],&x,&y);
		m_rcTotal.right = x;
		m_rcTotal.bottom = y;
		return TRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTDialogueWnd::ShowDecisions
//
//	PURPOSE:	Shows the decisions
//
// ----------------------------------------------------------------------- //
BOOL CLTDialogueWnd::ShowDecisions()
{
	// If we have decisions to make, do them now
	m_bDecisions = FALSE;
	if(!m_csDecisions.IsEmpty())
	{
		CommonLT* pCommon = g_pInterface->Common();
		if (!pCommon)
		{
			return FALSE;
		}

		ConParse parse;
		parse.Init((char *)(LPCSTR)m_csDecisions);

		m_collDialogueIDs.RemoveAll();
		CStringArray collDecisions;
		DWORD dwID;
		while (pCommon->Parse(&parse) == LT_OK)
		{
			if (parse.m_nArgs > 0 && parse.m_Args[0])
			{
				dwID = atoi(parse.m_Args[0]);
				HSTRING hString=g_pInterface->FormatString(dwID);
				if(!hString)
				{
					return FALSE;
				}
				m_collDialogueIDs.Add(dwID);
				collDecisions.Add(g_pInterface->GetStringData(hString));
				g_pInterface->FreeString(hString);
			}
		}

		if(collDecisions.GetSize() <= 0)
		{
			return FALSE;
		}

		if(m_DecisionWnd.DisplayText(&collDecisions,m_yPos + m_nHeight + 20))
		{
			// If we immediatelly displayed the decisions, let the server
			// finish talking...

			if (!m_bImmediateDecisions)
			{
				// Tell the server to stop speaking
				HMESSAGEWRITE hMessage;
				hMessage = g_pInterface->StartMessage(CSM_DIALOGUE_STOP);
				g_pInterface->EndMessage(hMessage);
			}

			m_bEnabled = FALSE;
			m_bDecisions = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}