// ----------------------------------------------------------------------- //
//
// MODULE  : HUDBombTarget.cpp
//
// PURPOSE : HUDItem to display status of doomsday pieces
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "HUDBombTarget.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "TO2PlayerMgr.h"
#include "GameClientShell.h"

//******************************************************************************************
//**
//** HUD Carry Icon display
//**
//******************************************************************************************
CHUDBombTarget::CHUDBombTarget()
{
	m_eLevel = kHUDRenderDead;
	m_UpdateFlags = kHUDBombTarget;
	m_lstIconData.resize( 3 );
	m_fScale = 1.0f;
}


LTBOOL CHUDBombTarget::Init()
{
	UpdateLayout();

	m_lstIconData[0].m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\radar_bomb1.dtx");
	m_lstIconData[1].m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\radar_bomb2.dtx");
	m_lstIconData[2].m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\radar_bomb3.dtx");

	float fx = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float fy = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetXRatio();
	float fw = (float)(m_nSize) * g_pInterfaceResMgr->GetXRatio();
	float fgap = fw * 1.25f;

	uint8 nFont = g_pLayoutMgr->GetInt("Miscellaneous","BombTimerFont");
    uint8 nBaseSize = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","BombTimerSize");
	LTVector const& vColor = g_pLayoutMgr->GetVector("Miscellaneous","BombTimerColor");

	LTIntPt pos( m_BasePos.x + m_nSize + 2, m_BasePos.y - 2 );
	for (uint32 i = 0; i < m_lstIconData.size( ); i++)
	{
		g_pDrawPrim->SetRGBA(&m_lstIconData[i].m_Poly,argbWhite);
		SetupQuadUVs( m_lstIconData[i].m_Poly, m_lstIconData[i].m_hIcon, 0.0f,0.0f,1.0f,1.0f);

		m_lstIconData[i].m_Timer.Init( nFont, nBaseSize, pos, vColor, CUI_HALIGN_LEFT );
		// The pos for the timer is in unscaled space, so we need to convert
		// the scaled fgab to unscaled. 
		pos.y += ( int )(( fgap / g_pInterfaceResMgr->GetXRatio()) + 0.5f );

		g_pDrawPrim->SetXYWH(&m_lstIconData[i].m_Poly,fx,fy,fw,fw);
		fy += fgap;
	}

	return LTTRUE;
}

void CHUDBombTarget::Term()
{

}
void CHUDBombTarget::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		m_fScale = g_pInterfaceResMgr->GetXRatio();

		for ( uint32 i = 0; i < m_lstIconData.size( ); i++)
		{
			IconData& iconData = m_lstIconData[i];
			iconData.m_Timer.ScreenDimsChanged( );
		}

		float fx = (float)(m_BasePos.x) * m_fScale;
		float fy = (float)(m_BasePos.y) * m_fScale;
		float fw = (float)(m_nSize) * m_fScale;
		float fgap = fw * 1.25f;

		LTIntPt pos( m_BasePos.x + m_nSize + 2, m_BasePos.y - 2 );
		for (uint32 i = 0; i < m_lstIconData.size( ); i++)
		{
			g_pDrawPrim->SetXYWH(&m_lstIconData[i].m_Poly,fx,fy,fw,fw);
			fy += fgap;
		}
	}
}

void CHUDBombTarget::Render()
{
	if (!g_pRadar->GetDraw()) 
		return;

	// Check if none of the timers are running.
	bool bRunning = false;
	for ( uint32 i = 0; i < m_lstIconData.size( ); i++)
	{
		if( m_lstIconData[i].m_Timer.GetTime( ) > 0 )
			bRunning = true;
	}
	if( !bRunning )
		return;

	SetRenderState();

	for ( uint32 i = 0; i < m_lstIconData.size( ); i++)
	{
		IconData& iconData = m_lstIconData[i];

		// Only draw if the timer is going.
		if( iconData.m_Timer.GetTimeLeft( ) > 0 )
		{
			//use the appropriate icon
			g_pDrawPrim->SetTexture( iconData.m_hIcon );
			// draw the icon
			g_pDrawPrim->DrawPrim(&m_lstIconData[i].m_Poly);

			// Draw the timer.
			iconData.m_Timer.Draw( );
		}
	}
}

void CHUDBombTarget::UpdateLayout()
{
	static char *pTag = "Miscellaneous";
	m_BasePos	= g_pLayoutMgr->GetPoint(pTag,"BombTimerPos");
	m_nSize		= (uint8)g_pLayoutMgr->GetInt(pTag,"BombTimerSize");

	if (!m_nSize)
	{
		m_BasePos		= LTIntPt(500,10);
		m_nSize			= 32;
	}
}

bool CHUDBombTarget::SetTimer( uint8 nTimer, float fTime, bool bPause )
{
	if( nTimer > m_lstIconData.size( ))
	{
		ASSERT( !"CHUDBombTarget::SetTimer: Invalid timer specified." );
		return false;
	}

	m_lstIconData[nTimer].m_Timer.SetTime( fTime, bPause );

	return true;
}

float CHUDBombTarget::GetTimer( uint8 nTimer ) const
{
	if( nTimer > m_lstIconData.size( ))
	{
		ASSERT( !"CHUDBombTarget::GetTimer: Invalid timer specified." );
		return 0.0f;
	}

	return m_lstIconData[nTimer].m_Timer.GetTime( );
}