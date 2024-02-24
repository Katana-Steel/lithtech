/*******************************************************************************
;
;	MODULE:			ANIMDLG (.CPP)
;
;	PURPOSE:		Animating dilog box class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "AnimDlg.h"
#include "PlaySound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ANIM_TIMER	100

/////////////////////////////////////////////////////////////////////////////
// CAnimDlg dialog


CAnimDlg::CAnimDlg(int IDD, CWnd* pParent)
	: CDialogEx(IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nLeft = 0;
	m_nTop = 0;
	m_bAnimating = TRUE;
	m_nAnimDir = ANIM_RIGHT_DOWN;
	m_dwAnimTime = 1000;
	m_nSoundID = 0;
}


void CAnimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimDlg, CDialogEx)
	//{{AFX_MSG_MAP(CAnimDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimDlg message handlers

BOOL CAnimDlg::OnInitDialog() 
{
	// We're not calling CDialogEx::OnInitDialog() because we're going to resize the dialog
	// from here anyway. 

	// IMPORTANT: Just like for CDialogEx::OnInitDialog() SetBackGround() should have been
	// called before we get here, because we'll need the dimensions of the background bitmap
	// to calculate the final size of the dialog. In case you want to leave the dialog sizing
	// to MFC, and stretching the background, just put the MFC dialog size in m_rcDlgSize instead of
	// the background bitmap dimensions.

	// SetDialogPos() should have been called before we get here

	// Get background bitmap size
	BITMAP bmpInfo;
	m_Background.GetBitmap(&bmpInfo);

	SetWindowPos(NULL, m_nLeft, m_nTop, 0, 0, SWP_NOZORDER);

	// Store the final size of the dialog
	m_rcDlgSize.SetRect(m_nLeft, m_nTop, m_nLeft+bmpInfo.bmWidth, m_nTop+bmpInfo.bmHeight);

	// Now we need to know in which direction we're going to animate the dialog to appear,
	// so that all of it stays on screen, much like with popup menus.

	// Get screen dims
	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	BOOL bLeft = FALSE;
	BOOL bUp = FALSE;

	// If we're off the screen even by a pixel, we're going in the opposite direction (right-down is default)
	if(m_rcDlgSize.right > nScreenWidth)
		bLeft = TRUE;
	if(m_rcDlgSize.bottom > nScreenHeight)
		bUp = TRUE;

	if(bLeft && bUp)
	{
		m_nAnimDir = ANIM_LEFT_UP;
	}
	else if(bLeft && !bUp)
	{
		m_nAnimDir = ANIM_LEFT_DOWN;
	}
	else if(!bLeft && bUp)
	{
		m_nAnimDir = ANIM_RIGHT_UP;
	}
	else// if(!bLeft && !bUp)
	{
		m_nAnimDir = ANIM_RIGHT_DOWN;
	}

	// Set our start time
	m_dwStartTime = timeGetTime();

	// Set our animation timer
	SetTimer(ANIM_TIMER, 0, NULL);

	// Play a sound
	if(m_nSoundID != 0)
		PlaySound(m_nSoundID);

	return TRUE;
}

void CAnimDlg::OnTimer(UINT nIDEvent) 
{
	if((nIDEvent == ANIM_TIMER) && m_bAnimating)
	{
		// Get current size of dialog
		CRect rcDlg, rcNewDlg;
		GetWindowRect(&rcDlg);

		// Figure out our deltas
		DWORD dwDeltaTime = timeGetTime() - m_dwStartTime;

		// Check to see if we're done
		if(dwDeltaTime >= m_dwAnimTime)
		{
			// We're done
			rcNewDlg.left = rcDlg.left;
			rcNewDlg.top = rcDlg.top;
			rcNewDlg.right = rcNewDlg.left + m_rcDlgSize.Width() - 1;
			rcNewDlg.bottom = rcNewDlg.top + m_rcDlgSize.Height() - 1;
			m_bAnimating = FALSE;
		}
		else
		{
			// Calculate our deltas and move us
			float fPercent = (float)dwDeltaTime / (float)m_dwAnimTime;
			int nNewWidth = (int)((float)m_rcDlgSize.Width() * fPercent);
			int nNewHeight = (int)((float)m_rcDlgSize.Height() * fPercent);

			// Make sure we actually need to resize
			if((nNewWidth != rcDlg.Width()) || (nNewHeight != rcDlg.Height()))
			{
				switch(m_nAnimDir)
				{
					case ANIM_RIGHT_UP:
					{
						rcNewDlg.SetRect(0,0,0,0);
						break;
					}
					case ANIM_LEFT_UP:
					{
						rcNewDlg.SetRect(0,0,0,0);
						break;
					}
					case ANIM_LEFT_DOWN:
					{
						rcNewDlg.SetRect(0,0,0,0);
						break;
					}
					case ANIM_RIGHT_DOWN:
					default:
					{
						rcNewDlg.SetRect(rcDlg.left,rcDlg.top,rcDlg.left + nNewWidth,rcDlg.top + nNewHeight);
						break;
					}
				}
			}
		}

		SetWindowPos(NULL, rcNewDlg.left, rcNewDlg.top, rcNewDlg.Width(), rcNewDlg.Height(), SWP_NOZORDER);
	}
	
	CDialogEx::OnTimer(nIDEvent);
}
