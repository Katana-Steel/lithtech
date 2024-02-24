// FrameCtrl.cpp: implementation of the CFrameCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FrameCtrl.h"
#include "InterfaceMgr.h"

namespace
{
	const int kHighlightWidth = 2;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFrameCtrl::CFrameCtrl()
{
    m_hSurf = LTNULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bSolid = LTTRUE;

}

CFrameCtrl::~CFrameCtrl()
{
	Destroy();
}


LTBOOL CFrameCtrl::Create(ILTClient *pClientDE, HLTCOLOR hColor, LTFLOAT fAlpha, int nWidth, int nHeight, LTBOOL bSolid)
{
	m_pClientDE=pClientDE;

	if (fAlpha < 0.1 || fAlpha > 1.0)
		return LTFALSE;
	if (nHeight < 2*kHighlightWidth || nWidth < 2*kHighlightWidth)
		return LTFALSE;

	HLTCOLOR trans = kWhite;
	if (hColor == kWhite)
		trans = kBlack;

	m_hSurf = g_pLTClient->CreateSurface(2,2);
    LTRect rect(0,0,2,2);
	g_pLTClient->FillRect(m_hSurf,&rect,hColor);
	g_pLTClient->SetSurfaceAlpha(m_hSurf,fAlpha);
	g_pLTClient->OptimizeSurface(m_hSurf,trans);

	m_nWidth=nWidth; 
	m_nHeight=nHeight;
	m_bEnabled=LTFALSE;
	m_bSolid = bSolid;

	CLTGUICtrl::Create(0,0,0);
	return LTTRUE;
}



// Destruction
void CFrameCtrl::Destroy()
{
	if (m_hSurf)
	{
		g_pLTClient->DeleteSurface(m_hSurf);
		m_hSurf = LTNULL;
	}
}

// Render the control
void CFrameCtrl::Render ( HSURFACE hDestSurf )
{
	if (m_bSolid)
	{
        LTRect rect(m_pos.x,m_pos.y,m_pos.x+m_nWidth,m_pos.y+m_nHeight);
		g_pLTClient->ScaleSurfaceToSurface(	hDestSurf, m_hSurf, &rect, LTNULL);
	}
	else
	{
		//left
        LTRect rect(m_pos.x, m_pos.y, m_pos.x+kHighlightWidth ,m_pos.y+m_nHeight);
		g_pLTClient->ScaleSurfaceToSurface(	hDestSurf, m_hSurf, &rect, LTNULL);

		//right
		rect.Init(m_pos.x+m_nWidth-kHighlightWidth,m_pos.y,m_pos.x+m_nWidth,m_pos.y+m_nHeight);
		g_pLTClient->ScaleSurfaceToSurface(	hDestSurf, m_hSurf, &rect, LTNULL);

		//top
        rect.Init(m_pos.x,m_pos.y,m_pos.x+m_nWidth,m_pos.y+kHighlightWidth);
		g_pLTClient->ScaleSurfaceToSurface(	hDestSurf, m_hSurf, &rect, LTNULL);

		//bottom
        rect.Init(m_pos.x,m_pos.y+m_nHeight-kHighlightWidth,m_pos.x+m_nWidth,m_pos.y+m_nHeight);
		g_pLTClient->ScaleSurfaceToSurface(	hDestSurf, m_hSurf, &rect, LTNULL);

	}


}

