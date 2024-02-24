// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCarrying.cpp
//
// PURPOSE : HUDItem to display an icon while carrying a body
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "TO2PlayerMgr.h"
#include "ChassisButeMgr.h"
#include "ChassisPieceFX.h"
#include <algorithm>
#include "KeyMgr.h"

//******************************************************************************************
//**
//** HUD Carry Icon display
//**
//******************************************************************************************

CHUDCarrying::CHUDCarrying()
{
	m_UpdateFlags = kHUDCarry | kHUDFrame;
	for (int i = 0; i < kNumCarryBodyIcons; i++)
	{
		m_hIcon[i] = LTNULL;
	};
}


LTBOOL CHUDCarrying::Init()
{
	m_hIcon[eBodyCanDrop] = g_pInterfaceResMgr->GetTexture("interface\\hud\\carrying.dtx");
	m_hIcon[eBodyNoDrop] = g_pInterfaceResMgr->GetTexture("interface\\hud\\nodrop.dtx");
	m_hIcon[eBodyCanCarry] = g_pInterfaceResMgr->GetTexture("interface\\hud\\carry.dtx");

	// Add all the chassis piece icons.
	ChassisPieceIcon chassisPieceIcon;
	ChassisButeMgr::ChassisPieceButeList::const_iterator iter = ChassisButeMgr::Instance( ).GetChassisPieceButeList( ).begin( );
	for( ; iter != ChassisButeMgr::Instance( ).GetChassisPieceButeList( ).end( ); iter++ )
	{
		ChassisPieceBute const* pChassisPieceBute = *iter;
		chassisPieceIcon.m_hCarryingIcon = g_pInterfaceResMgr->GetTexture( pChassisPieceBute->m_sCarryingIcon.c_str( ));
		chassisPieceIcon.m_hPickupIcon = g_pInterfaceResMgr->GetTexture( pChassisPieceBute->m_sPickupIcon.c_str( ));
		m_lstChassisPieceIcon.push_back( chassisPieceIcon );
	}

	g_pDrawPrim->SetRGBA(&m_Poly,argbWhite);
	SetupQuadUVs(m_Poly, m_hIcon[0], 0.0f,0.0f,1.0f,1.0f);

	UpdateLayout();

	return LTTRUE;
}

void CHUDCarrying::Term()
{

}

void CHUDCarrying::Render()
{
	CSpecialFX* pSFX;
	uint8 nCarry = g_pPlayerMgr->GetCarryingObject();
	uint8 nCanCarry = g_pPlayerMgr->CanCarryObject( &pSFX );

	if( nCarry || nCanCarry)
	{
		float fx = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
		float fy = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetXRatio();
		float fw = (float)(m_nSize) * g_pInterfaceResMgr->GetXRatio();

		SetRenderState();

		g_pDrawPrim->SetTexture(NULL);
		g_pDrawPrim->SetXYWH(&m_Poly,fx,fy,fw,fw);

		if(nCanCarry)
		{
			switch (nCanCarry)
			{
			case CFX_CARRY_BODY:
				g_pDrawPrim->SetTexture(m_hIcon[eBodyCanCarry]);
				break;
			case CFX_CARRY_CHASSIS_PIECE:
				ChassisPieceFX* pChassisPieceFX = dynamic_cast< ChassisPieceFX* >( pSFX );
				ChassisPieceBute const* pChassisPieceBute = pChassisPieceFX->GetChassisPieceBute( );
				if( pChassisPieceBute )
					g_pDrawPrim->SetTexture( m_lstChassisPieceIcon[pChassisPieceBute->m_nIndex].m_hPickupIcon );
				break;
			};
		}
		else if (g_pPlayerMgr->CanDropCarriedObject())
		{
			switch (nCarry)
			{
			case CFX_CARRY_BODY:
				g_pDrawPrim->SetTexture(m_hIcon[eBodyCanDrop]);
				break;
			case CFX_CARRY_CHASSIS_PIECE:
				ChassisPieceBute const* pChassisPieceBute = g_pPlayerMgr->GetCarryingChassisPiece( );
				if( pChassisPieceBute )
					g_pDrawPrim->SetTexture( m_lstChassisPieceIcon[pChassisPieceBute->m_nIndex].m_hCarryingIcon );
				break;
			};
		}
		else
		{
			g_pDrawPrim->SetTexture(m_hIcon[eBodyNoDrop]);
		}

		g_pDrawPrim->DrawPrim(&m_Poly);

	}

	// Check if we have any keyicons to render.
	if( m_lstKeyIcons.size( ))
	{
		// Always put the keyitems to the right of the carrying slot, even if it's empty.
		uint32 nXPos = m_BasePos.x + m_nSize + 2;

		float fy = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetXRatio();
		g_pDrawPrim->SetTexture(NULL);

		KeyIconList::iterator iter = m_lstKeyIcons.begin( );
		for( ; iter != m_lstKeyIcons.end( ); iter++ )
		{
			KeyIcon& keyIcon = *iter;

			float fx = (float)( nXPos ) * g_pInterfaceResMgr->GetXRatio();
			float fw = (float)( keyIcon.m_Size.x ) * g_pInterfaceResMgr->GetXRatio();
			float fh = (float)( keyIcon.m_Size.y ) * g_pInterfaceResMgr->GetXRatio();
			g_pDrawPrim->SetXYWH(&m_Poly,fx,fy,fw,fh);
			g_pDrawPrim->SetTexture( keyIcon.m_hIcon );
			g_pDrawPrim->DrawPrim( &m_Poly );

			// Move over for the next icon.
			nXPos += keyIcon.m_Size.x + 2;
		}
	}
}

void CHUDCarrying::Update()
{
}

void CHUDCarrying::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetCarryIconPos(nCurrentLayout);
	m_nSize			= g_pLayoutMgr->GetCarryIconSize(nCurrentLayout);

	if (!m_nSize)
	{
		m_BasePos		= LTIntPt(40,360);
		m_nSize			= 64;
	}
}


bool CHUDCarrying::AddKey( uint32 nKeyId )
{
	KeyIcon keyIcon;
	keyIcon.m_nKeyIndex = nKeyId;

	// Check if we already have it.
	if( std::find( m_lstKeyIcons.begin( ), m_lstKeyIcons.end( ), keyIcon ) != m_lstKeyIcons.end( ))
		return false;

	// Get the key info.
	KEY* pKey = g_pKeyMgr->GetKey( nKeyId );
	if( !pKey )
		return false;

	keyIcon.m_hIcon = g_pInterfaceResMgr->GetTexture( pKey->szImage );
	uint32 nWidth, nHeight;
	g_pTexInterface->GetTextureDims( keyIcon.m_hIcon, nWidth, nHeight );

	// Scale the image into the slot size.
	float fScale = 1.0;
	if( nWidth > nHeight )
	{
		fScale = ( float )m_nSize / nWidth;
	}
	else
	{
		fScale = ( float )m_nSize / nHeight;
	}
	nWidth = ( uint32 )(( nWidth * fScale ) + 0.5f );
	nHeight = ( uint32 )(( nHeight * fScale ) + 0.5f );

	keyIcon.m_Size.x = nWidth;
	keyIcon.m_Size.y = nHeight;
	m_lstKeyIcons.push_back( keyIcon );
	g_pHUDMgr->QueueUpdate(kHUDCarry);

	return true;
}

bool CHUDCarrying::RemoveKey( uint32 nKeyId )
{
	KeyIcon keyIcon;
	keyIcon.m_nKeyIndex = nKeyId;

	// Check if we don't have it.
	KeyIconList::iterator iter = std::find( m_lstKeyIcons.begin( ), m_lstKeyIcons.end( ), keyIcon );
	if( iter == m_lstKeyIcons.end( ))
		return false;

	m_lstKeyIcons.erase( iter );
	g_pHUDMgr->QueueUpdate(kHUDCarry);

	return true;
}

void CHUDCarrying::ClearAllKeys( )
{
	m_lstKeyIcons.clear( );
	g_pHUDMgr->QueueUpdate(kHUDCarry);
}