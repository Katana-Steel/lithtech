// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeaponHeat.cpp
//
// PURPOSE : HUDItem to display weapon heat meter
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "InterfaceMgr.h"
#include "ClientWeaponMgr.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"

//******************************************************************************************
//**
//** HUD Air display
//**
//******************************************************************************************

CHUDWeaponHeat::CHUDWeaponHeat()
{
	m_UpdateFlags = kHUDWeaponHeat | kHUDFrame;
}


LTBOOL CHUDWeaponHeat::Init()
{
	UpdateLayout();

	int nCurrentLayout = GetConsoleInt("HUDLayout",0);
	char szTexture[256];
	g_pLayoutMgr->GetWeaponHeatBarTexture( nCurrentLayout, szTexture, ARRAY_LEN( szTexture ));
	if( !szTexture[0] )
		return LTFALSE;

	m_Bar.Init(g_pInterfaceResMgr->GetTexture( szTexture ));

	return LTTRUE;
}

void CHUDWeaponHeat::Term()
{
}

void CHUDWeaponHeat::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	m_Bar.Render();
}

void CHUDWeaponHeat::Update()
{
	IClientWeaponBase const* pClientWeapon = NULL;
	if( g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() && g_pMoveMgr->GetVehicleMgr()->HasVehicleWeapon())
	{
		pClientWeapon = g_pMoveMgr->GetVehicleMgr()->GetVehicleWeapon();
	}
	else
	{
		pClientWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetCurrentClientWeapon();
	}
	if( !pClientWeapon )
		return;

	m_bDraw = ( pClientWeapon->GetWeapon( )->fHeatFactor > 0.0f );
	if (!m_bDraw) 
		return;

	float fPercent = pClientWeapon->GetWeaponHeat( );
	float x = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float y = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetYRatio();

	float w = 100.0f * fPercent * m_fBarScale * g_pInterfaceResMgr->GetXRatio();
	float maxW = 100.0f * m_fBarScale * g_pInterfaceResMgr->GetXRatio();
	float h = (float)m_nBarHeight * g_pInterfaceResMgr->GetYRatio();

	if( fPercent < 0.5f )
	{
		LTVector vColor;
		VEC_LERP( vColor, m_vCool, m_vWarm, fPercent / 0.5f );
		m_Bar.SetForegroundColor( SET_ARGB(255,(uint8)vColor.x,(uint8)vColor.y,(uint8)vColor.z));
	}
	else if( fPercent < 1.0f )
	{
		LTVector vColor;
		VEC_LERP( vColor, m_vWarm, m_vHot, (( fPercent - 0.5f ) / 0.5f ));
		m_Bar.SetForegroundColor( SET_ARGB(255,(uint8)vColor.x,(uint8)vColor.y,(uint8)vColor.z));
	}
	else
		m_Bar.SetForegroundColor( m_argbOverheat );
	
	m_Bar.Update(x,y,w,maxW,h);
}

void CHUDWeaponHeat::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetWeaponHeatBasePos(nCurrentLayout);

	m_nBarHeight		= g_pLayoutMgr->GetWeaponHeatBarHeight(nCurrentLayout);
	m_fBarScale			= g_pLayoutMgr->GetWeaponHeatBarScale(nCurrentLayout);

	LTVector vOverheat;
	g_pLayoutMgr->GetWeaponHeatColors( nCurrentLayout, m_vCool, m_vWarm, m_vHot, vOverheat );
	m_argbOverheat = SET_ARGB(255,(uint8)vOverheat.x,(uint8)vOverheat.y,(uint8)vOverheat.z);
}

