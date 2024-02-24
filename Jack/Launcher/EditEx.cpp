/*******************************************************************************
;
;	MODULE:			EDITEX (.CPP)
;
;	PURPOSE:		Extended Edit control class (derived from CEdit)
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "EditEx.h"
#include "PlaySound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditEx

CEditEx::CEditEx()
{
}

CEditEx::~CEditEx()
{
}


BEGIN_MESSAGE_MAP(CEditEx, CEdit)
	//{{AFX_MSG_MAP(CEditEx)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditEx::OnChar
//
//	PURPOSE:	Character handler (to play a sound)
//
// ----------------------------------------------------------------------- //

void CEditEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_BACK)
	{
		PlaySound(IDR_TYPEBACK);
	}
	else
	{
		float randNum = (float)rand() / RAND_MAX;
		float num = 0 + (3 - 0) * randNum;
		if(num > 2.9f)
			num = 2.0f;
		PlaySound(IDR_TYPE+(int)num);
	}
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
