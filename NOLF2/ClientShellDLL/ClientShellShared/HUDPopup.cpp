// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPopup.cpp
//
// PURPOSE : Implementation of CHUDPopup to display popups
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "StdAfx.h"
	#include "HUDMgr.h"
	#include "InterfaceResMgr.h"
	#include "PopupMgr.h"
	#include "HUDPopup.h"



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::CHUDPopup
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHUDPopup::CHUDPopup()
:	CHUDItem		( )
{
	m_UpdateFlags	= kHUDFrame;
	m_bVisible		= LTFALSE;	
	m_bColorOverride = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Init
//
//  PURPOSE:	Initialize the popup...
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDPopup::Init()
{
	m_Text.Create(" ",0,0,g_pInterfaceResMgr->GetFont(0),8,LTNULL);
	m_Frame.Create(g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\frame.dtx"),200,320,LTTRUE);

	return LTTRUE;

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Term
//
//  PURPOSE:	Destroy thyself...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Term()
{
	m_Frame.Destroy();
	m_Text.Destroy();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Hide
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Hide( )
{
	m_bVisible = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Render
//
//  PURPOSE:	Draw the popup...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Render()
{
	if (!m_bVisible) return;
	m_Frame.Render();
	m_Text.Render();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Update
//
//  PURPOSE:	Update the popup...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Update()
{
	// Sanity checks...
	if( !IsVisible() ) return;

	if( m_fScale != g_pInterfaceResMgr->GetXRatio() )
		SetScale( g_pInterfaceResMgr->GetXRatio() );

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::SetScale
//
//  PURPOSE:	Set the items scale...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::SetScale(float fScale)
{
	m_Frame.SetScale( fScale );
	m_Text.SetScale( fScale );

	m_fScale = fScale;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDPopup::Show
//
//  PURPOSE:	Display the popup with the passed in text...
//
// ----------------------------------------------------------------------- //

void CHUDPopup::Show( uint8 nPopupID, const char *pText )
{
	POPUP* pPopup = g_pPopupMgr->GetPopup(nPopupID);
	if (!pPopup) return;

	CUIFont *pFont = g_pInterfaceResMgr->GetFont(pPopup->nFont);
	LTIntPt pos;

	// We need to expand the width of our 640x480 to whatever aspect ratio we're using.
	int offset = g_pInterfaceResMgr->Get640x480Offset();
	int width = pPopup->sSize.x;
	int height = pPopup->sSize.y;

	// Reference: 640 / 2 = 320
	pos.x = (320 + offset) - (width / 2);
	pos.y = (480 - height) / 2;

	m_Frame.SetFrame(g_pInterfaceResMgr->GetTexture(pPopup->szFrame));
	m_Frame.SetSize(width, height);
	m_Frame.SetBasePos(pos);
	m_Frame.SetScale(g_pInterfaceResMgr->GetYRatio());

	pos.x += pPopup->sTextOffset.x;
	pos.y += pPopup->sTextOffset.y;

	m_Text.SetScale(1.0f);

	m_Text.SetString( (pText ? pText : "") );

	m_Text.SetFont(pFont,pPopup->nFontSize);

	if( !m_bColorOverride )
		m_Text.SetColors(pPopup->argbTextColor,pPopup->argbTextColor,pPopup->argbTextColor);

	m_bColorOverride = LTFALSE;
	
	m_Text.SetFixedWidth(pPopup->nTextWidth);
	m_Text.SetBasePos(pos);
	m_Text.SetScale(g_pInterfaceResMgr->GetYRatio());

	m_bVisible = LTTRUE;
}