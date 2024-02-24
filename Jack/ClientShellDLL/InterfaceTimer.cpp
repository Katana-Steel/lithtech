// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.cpp
//
// PURPOSE : Implementation of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "InterfaceTimer.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "InterfaceResMgr.h"

static void BuildTimeString(char* aBuffer, int nTime);

CInterfaceTimer::CInterfaceTimer() 
{
	m_fTime		= 0.0f;
	m_fTimeLeft = 0.0f;
	m_bPause	= LTFALSE;
	m_pTimeStr	= LTNULL;
	m_nBaseSize = 0;
	m_pFont = NULL;
	m_nColor	= argbWhite;
	m_eAlignment = CUI_HALIGN_CENTER;
}

CInterfaceTimer::~CInterfaceTimer() 
{ 
	if (m_pTimeStr) 
		g_pFontManager->DestroyPolyString(m_pTimeStr); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceTimer::Init()
//
//	PURPOSE:	Initializes the interface timer.
//
// ----------------------------------------------------------------------- //
bool CInterfaceTimer::Init( uint8 nFont, uint8 nBaseSize, LTIntPt const& basePos, LTVector const& vColor, CUI_ALIGNMENTTYPE eAlignment )
{
	m_pFont = g_pInterfaceResMgr->GetFont( nFont );
	if( !m_pFont )
		return false;

	m_nBaseSize = nBaseSize;
	m_BasePos = basePos;
	if(m_BasePos.x == 0)
		m_BasePos = LTIntPt(320,40);
	m_nColor = SET_ARGB( 0xFF, ( uint8 )vColor.x, ( uint8 )vColor.y, ( uint8 )vColor.z );

	m_eAlignment = eAlignment;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceTimer::Draw()
//
//	PURPOSE:	Handle drawing the stats
//
// ----------------------------------------------------------------------- //

void CInterfaceTimer::Draw()
{

	// If we're not paused, recalculate the time left.
	if( !m_bPause )
		m_fTimeLeft = m_fTime - g_pLTClient->GetGameTime( );

	// Update/Draw the timer if there is anytime left...
	if( m_fTimeLeft <= 0.0f )
		return;

	// Draw the string to the surface...

	int nMinutes = int(m_fTimeLeft) / 60;
	int nSeconds = m_fTimeLeft > 60.0 ? int(m_fTimeLeft) % 60 : int(m_fTimeLeft);

	char aBuffer[8];
	uint8 nFont = 0;
	LTVector vColor;

	sprintf(aBuffer, "%02d:%02d", nMinutes, nSeconds);
	
 	if (!m_pTimeStr)
 	{
		m_pTimeStr = g_pFontManager->CreateFormattedPolyString(m_pFont, aBuffer);
		m_pTimeStr->SetAlignmentH(m_eAlignment);
		uint8 nSize = (uint8)((LTFLOAT)m_nBaseSize * g_pInterfaceResMgr->GetXRatio());
		m_pTimeStr->SetCharScreenHeight(nSize);
 	}
 	else
 	{
		m_pTimeStr->SetText(aBuffer);
 	}

	// Position the text
	float x = (float)m_BasePos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio();

	m_pTimeStr->SetPosition( x+2.0f, y+2.0f);
	m_pTimeStr->SetColor(argbBlack);
	m_pTimeStr->Render();
	
	m_pTimeStr->SetPosition( x, y );
	m_pTimeStr->SetColor(m_nColor);
	m_pTimeStr->Render();
}

static void BuildTimeString(char* aBuffer, int nTime)
{
	if (nTime > 9)
	{
		sprintf(aBuffer, "%d", nTime);
	}
	else
	{
		sprintf(aBuffer, "0%d", nTime);
	}
}

void CInterfaceTimer::ScreenDimsChanged()
{
	if (m_pTimeStr)
	{
		uint8 nSize = (uint8)((LTFLOAT)m_nBaseSize * g_pInterfaceResMgr->GetXRatio());
		m_pTimeStr->SetCharScreenHeight(nSize);
	}
}
