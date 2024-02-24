// Subtitle.cpp: implementation of the CSubtitle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Subtitle.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;
//
//VarTrack	g_vtSubtitles;
//VarTrack	g_vtSubtitleMaxDist;

//	GetConsoleInt("Subtitles", defSubtitles);
//  GetConsoleFloat("SubtitleMaxDist", 1000.0f);

namespace
{
	int defSubtitles = 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSubtitle::CSubtitle()
{
    m_hForeSurf         = LTNULL;
    m_pForeFont         = LTNULL;
	m_bVisible			= LTFALSE;
	m_vSpeakerPos.Init();
	m_fRadius			= 0;
	m_fDuration			= -1.0f;
	m_bOverflow			= LTFALSE;

}



CSubtitle::~CSubtitle()
{
	Clear();
	if (m_hForeSurf)
	{
        g_pLTClient->DeleteSurface(m_hForeSurf);
        m_hForeSurf = LTNULL;
	}
}


void	CSubtitle::Init()
{

	m_pForeFont		= g_pInterfaceResMgr->GetMessageFont();
	m_nLineHeight	= m_pForeFont->GetHeight();


	m_3rdPersonPos		= g_pLayoutMgr->GetSubtitle3rdPersonPos();
	m_n3rdPersonWidth    = (uint32)g_pLayoutMgr->GetSubtitle3rdPersonWidth();
	m_1stPersonPos		= g_pLayoutMgr->GetSubtitle1stPersonPos();
	m_n1stPersonWidth    = (uint32)g_pLayoutMgr->GetSubtitle1stPersonWidth();

	m_nMaxLines		= g_pLayoutMgr->GetSubtitleNumLines();

	m_bVisible		= LTFALSE;
	m_dwWidth		= 0;
	m_dwHeight		= 0;

	if (!g_pInterfaceResMgr->IsEnglish())
		defSubtitles = 1;

	m_vSpeakerPos.Init();
	m_fRadius = 0.0;
	m_fDuration = -1.0f;

	m_hSubtitleColor = g_pLayoutMgr->GetSubtitleTint();
}
	

void CSubtitle::ClearSurfaces()
{
	uint32 dwWidth  = 0;
	uint32 dwHeight  = 0;
	g_pLTClient->GetSurfaceDims(m_hForeSurf,&dwWidth,&dwHeight);
    LTRect rcFore(0,0, dwWidth, dwHeight);
    g_pLTClient->FillRect(m_hForeSurf, &rcFore, LTNULL);
	g_pLTClient->OptimizeSurface(m_hForeSurf, LTNULL);


}


void CSubtitle::Show(int nStringId, LTVector vSpeakerPos, LTFLOAT fRadius, LTFLOAT fDuration)
{
	HSTRING hText = g_pLTClient->FormatString(nStringId);
	if (!hText) return;

	
	m_vSpeakerPos = vSpeakerPos;
	m_fRadius = fRadius > 0 ? fRadius : GetConsoleFloat("SubtitleMaxDist", 1000.0f);

	m_fDuration = fDuration;

	uint32 width = 0;
	if (g_pGameClientShell->IsCinematic() || g_pGameClientShell->Is3rdPerson())
	{
		m_pos.x = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_3rdPersonPos.x);
		m_pos.y = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_3rdPersonPos.y);
		width = (uint32)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_n3rdPersonWidth);
	}
	else
	{
		m_pos.x = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_1stPersonPos.x);
		m_pos.y = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_1stPersonPos.y);
		width = (uint32)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_n1stPersonWidth);
	}
	 
	uint32 height = m_nMaxLines * m_nLineHeight;

	m_dwWidth = 0;
	m_dwHeight = 0;
	
	if (m_hForeSurf)
	{
		g_pLTClient->GetSurfaceDims(m_hForeSurf,&m_dwWidth,&m_dwHeight);
	}
	m_txtSize = m_pForeFont->GetTextExtentsFormat(hText,(int)width);


	if ((uint32)m_txtSize.x > m_dwWidth || (uint32)m_txtSize.y > m_dwHeight)
	{
		if (m_hForeSurf) g_pLTClient->DeleteSurface(m_hForeSurf);
		m_hForeSurf = g_pLTClient->CreateSurface(m_txtSize.x,m_txtSize.y);
		m_dwWidth = m_txtSize.x;
		m_dwHeight = m_txtSize.y;
	}
	ClearSurfaces();

	int numLines = m_txtSize.y / m_nLineHeight;
//	g_pLTClient->CPrint("Lines: %d",numLines);
//	g_pLTClient->CPrint("Duration: %0.2f",fDuration);

	if ((uint32)numLines > m_nMaxLines)
	{
		m_bOverflow = LTTRUE;
		LTFLOAT fTimePerLine = m_fDuration / ((LTFLOAT)numLines + 1.0f);
		LTFLOAT fRemainingLines = (LTFLOAT)((uint32)numLines - m_nMaxLines);
		LTFLOAT fDelay = (LTFLOAT)m_nMaxLines * fTimePerLine;
		m_fScrollStartTime = fDelay + g_pLTClient->GetTime();

		m_fScrollSpeed = (LTFLOAT)m_nLineHeight / fTimePerLine;
		m_fMaxOffset = (LTFLOAT)((uint32)m_txtSize.y - height);

	}
	else
	{
		m_bOverflow = LTFALSE;

		m_pos.x += (width - m_txtSize.x) / 2;
		m_pos.y += (height - m_txtSize.y);
		m_rcSrcRect = LTRect(0,0,width,height);
		m_fScrollSpeed = 0.0f;

	}

	m_rcBaseRect = LTRect(0,0,width-1,height-1);
	m_rcSrcRect = m_rcBaseRect;
	m_fOffset = 0.0f;

	m_pForeFont->DrawFormat(hText,m_hForeSurf,0,0,(uint32)width,kWhite);

	m_bVisible			= LTTRUE;

	if (hText)
	{
        g_pLTClient->FreeString(hText);
	}

}

void CSubtitle::Clear()
{
	ClearSurfaces();
	m_bVisible			= LTFALSE;

}


void CSubtitle::Draw()
{
	if (!m_bVisible) return;
	if ( !GetConsoleInt("Subtitles", defSubtitles) ) return;

	// Show subtitles if conversations in range)...

	LTVector vListenerPos;
	LTBOOL bListenerInClient;
	LTRotation rRot;
	g_pLTClient->GetListener(&bListenerInClient, &vListenerPos, &rRot);

	//HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	//if (!hPlayerObj) return;
	// g_pLTClient->GetObjectPos(hPlayerObj, &vListenerPos);

	LTBOOL bForceDraw = (LTBOOL)(m_vSpeakerPos == LTVector(0, 0, 0));
	bForceDraw = (g_pGameClientShell->IsCinematic() || g_pGameClientShell->Is3rdPerson()) ? LTTRUE : bForceDraw;

	LTVector vPos = m_vSpeakerPos - vListenerPos;
	if (bForceDraw || vPos.Mag() <= m_fRadius)
	{
		LTSurfaceBlend  oldBlend = LTSURFACEBLEND_ALPHA;
//		g_pLTClient->GetOptimized2DBlend(oldBlend);

		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
		g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hForeSurf, &m_rcSrcRect, m_pos.x+1, m_pos.y+1);
		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
		g_pLTClient->SetOptimized2DColor(m_hSubtitleColor);
		g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hForeSurf, &m_rcSrcRect, m_pos.x, m_pos.y);
		g_pLTClient->SetOptimized2DColor(kWhite);
		g_pLTClient->SetOptimized2DBlend(oldBlend);
	}
}


void CSubtitle::Update()
{
	if (!m_bVisible) return;
	if (!m_bOverflow) return;


	if (m_fScrollStartTime < g_pLTClient->GetTime())
	{
		LTFLOAT fElapsedTime = g_pLTClient->GetTime() - m_fScrollStartTime;
		m_fOffset = (fElapsedTime * m_fScrollSpeed);
		if (m_fOffset > m_fMaxOffset) m_fOffset = m_fMaxOffset;
		m_rcSrcRect = m_rcBaseRect;
		m_rcSrcRect.top += (int)m_fOffset;
		m_rcSrcRect.bottom += (int)m_fOffset;
	}
}

void CSubtitle::ScreenDimsChanged()
{
	if (!m_bVisible) return;
	if (!GetConsoleInt("Subtitles", defSubtitles)) return;

	uint32 width = 0;
	if ( g_pGameClientShell->IsCinematic() || g_pGameClientShell->Is3rdPerson())
	{
		m_pos.x = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_3rdPersonPos.x);
		m_pos.y = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_3rdPersonPos.y);
		width = (uint32)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_n3rdPersonWidth);
	}
	else
	{
		m_pos.x = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_1stPersonPos.x);
		m_pos.y = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_1stPersonPos.y);
		width = (uint32)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_n1stPersonWidth);
	}
	
	uint32 height = m_nMaxLines * m_nLineHeight;

	if (m_bOverflow)
	{
	}
	else
	{
		m_pos.x += (width - m_txtSize.x) / 2;
		m_pos.y += (height - m_txtSize.y);
	}
}
