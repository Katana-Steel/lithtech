// ----------------------------------------------------------------------- //
//
// MODULE  : CrosshairMgr.h
//
// PURPOSE : Implementation of CrosshairMgr class
//
// CREATED : 8/28/00
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
 
#include "stdafx.h"
#include "CrosshairMgr.h"
#include "GameClientShell.h"
#include "CharacterFuncs.h"
#include "VarTrack.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::CCrosshairMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCrosshairMgr::CCrosshairMgr()
{
	// common members
	m_bCrosshairEnabled		= DTRUE;
	m_bCrosshairOverride	= LTTRUE;
	m_bUseActivate			= LTFALSE;
	
	// activate crosshair members
	m_hActivateCrosshair	= LTNULL;
	m_nHalfActXhairWidth	= 0;
	m_nHalfActXhairHeight	= 0;
	m_eActMode				= XHM_TARGETING;
	m_fActScale				= 1.0f;

	// targeting crosshair members
	m_hCrosshair			= LTNULL;
	m_ptCHPos				= LTIntPt(0,0);
	m_bCustomCrosshairPos	= LTFALSE;
	m_nHalfXhairWidth		= 0;	
	m_nHalfXhairHeight		= 0;
	m_fScale				= 1.0f;
	m_hCursorColor			= LTNULL;
	m_bDrawSolid			= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::~CCrosshairMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCrosshairMgr::~CCrosshairMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CCrosshairMgr::Init()
{
	//load up the activate and targeting crosshairs
	SetActivateCrosshair(XHM_DEFAULT_ACTIVATE);
	LoadCrosshair("Interface\\StatusBar\\Marine\\Xhair_normal.pcx", 4.0f);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::Term()
//
//	PURPOSE:	Terminate the crosshair stuff
//
// ----------------------------------------------------------------------- //

void CCrosshairMgr::Term()
{
	if (!g_pLTClient) return;

//	if (m_hCrosshair)
//	{
//		g_pLTClient->DeleteSurface (m_hCrosshair);
//		m_hCrosshair = LTNULL;
//	}

//	if (m_hActivateCrosshair)
//	{
//		g_pLTClient->DeleteSurface (m_hActivateCrosshair);
//		m_hActivateCrosshair = LTNULL;
//	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::Save
//
//	PURPOSE:	Save the player stats info
//
// --------------------------------------------------------------------------- //

void CCrosshairMgr::Save(HMESSAGEWRITE hWrite)
{
	if (!g_pLTClient || !g_pWeaponMgr) return;

	g_pLTClient->WriteToMessageByte(hWrite, m_bCrosshairEnabled);
	g_pLTClient->WriteToMessageByte(hWrite, m_bCrosshairOverride);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::Load
//
//	PURPOSE:	Load the player stats info
//
// --------------------------------------------------------------------------- //

void CCrosshairMgr::Load(HMESSAGEREAD hRead)
{
	if (!g_pLTClient || !g_pWeaponMgr) return;

	m_bCrosshairEnabled		= (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
	m_bCrosshairOverride	= (LTBOOL) g_pLTClient->ReadFromMessageByte(hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::DrawCrosshair
//
//	PURPOSE:	Draw the crosshair
//
// --------------------------------------------------------------------------- //

void CCrosshairMgr::DrawCrosshair()
{
	if (!g_pLTClient || !m_bCrosshairEnabled || !m_bCrosshairOverride) return;


	if(m_bUseActivate)
	{
		// For the torch and hacking device, see that we are not firing...
		if(m_eActMode == XHM_MARINE_WELD || m_eActMode == XHM_MARINE_CUT)
		{
			// Check the weapon...
			WEAPON* pWep = g_pGameClientShell->GetWeaponModel()->GetWeapon();

			if(pWep && (stricmp("Blowtorch", pWep->szName)==0))
			{
				uint32 dwFlags = g_pGameClientShell->GetPlayerMovement()->GetControlFlags();

				// Hide the cursor if we are firing...
				if(dwFlags & CM_FLAG_PRIMEFIRING)
					return;
			}
		}
		else if(m_eActMode == XHM_MARINE_HACK)
		{
			// Check the weapon...
			WEAPON* pWep = g_pGameClientShell->GetWeaponModel()->GetWeapon();

			if(pWep && ((stricmp("MarineHackingDevice", pWep->szName)==0) || (stricmp("PredatorHackingDevice", pWep->szName)==0)) )
			{
				uint32 dwFlags = g_pGameClientShell->GetPlayerMovement()->GetControlFlags();

				// Hide the cursor if we are firing...
				if(dwFlags & CM_FLAG_PRIMEFIRING)
					return;
			}
		}

		DrawActivateCrosshair();
	}
	else
	{
		DrawTargetingCrosshair();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::DrawActivateCrosshair
//
//	PURPOSE:	Draw the activate crosshair
//
// --------------------------------------------------------------------------- //

void CCrosshairMgr::DrawActivateCrosshair()
{
	//get screen dims
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nHalfScreenWidth, nHalfScreenHeight;
	g_pLTClient->GetSurfaceDims(hScreen, &nHalfScreenWidth, &nHalfScreenHeight);
	nHalfScreenWidth>>=1;
	nHalfScreenHeight>>=1;

	//draw the crosshair to the screen
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	LTRect rcDest;
	rcDest.top = nHalfScreenHeight-(uint32)(m_nHalfActXhairHeight*m_fActScale);
	rcDest.left = nHalfScreenWidth-(uint32)(m_nHalfActXhairWidth*m_fActScale);
	rcDest.bottom = rcDest.top + (uint32)(m_nHalfActXhairHeight*m_fActScale*2);
	rcDest.right = rcDest.left + (uint32)(m_nHalfActXhairWidth*m_fActScale*2);

	g_pLTClient->ScaleSurfaceToSurfaceTransparent(	hScreen, 
													m_hActivateCrosshair,
													&rcDest, 
													LTNULL, 
													hTransColor);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCrosshairMgr::DrawTargatingCrosshair
//
//	PURPOSE:	Draw the targeting crosshair
//
// --------------------------------------------------------------------------- //

void CCrosshairMgr::DrawTargetingCrosshair()
{
	//get screen dims
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nHalfScreenWidth, nHalfScreenHeight;
	g_pLTClient->GetSurfaceDims(hScreen, &nHalfScreenWidth, &nHalfScreenHeight);
	nHalfScreenWidth>>=1;
	nHalfScreenHeight>>=1;

	//find the center of the crosshair dest
	int cx = m_bCustomCrosshairPos?m_ptCHPos.x:nHalfScreenWidth;
	int cy = m_bCustomCrosshairPos?m_ptCHPos.y:nHalfScreenHeight;


	//draw the crosshair to the screen
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	LTRect rcDest;
	rcDest.top = cy-(uint32)(m_nHalfXhairHeight*m_fScale);
	rcDest.left = cx-(uint32)(m_nHalfXhairWidth*m_fScale);
	rcDest.bottom = rcDest.top + (uint32)(m_nHalfXhairHeight*m_fScale*2);
	rcDest.right = rcDest.left + (uint32)(m_nHalfXhairWidth*m_fScale*2);

	if(!m_bDrawSolid)
	{
		if(m_fScale != 1.0f)
			g_pLTClient->ScaleSurfaceToSurfaceTransparent(	hScreen, 
															m_hCrosshair,
															&rcDest, 
															LTNULL, 
															hTransColor);
		else
			g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
															m_hCrosshair,
															LTNULL,
															rcDest.left,
															rcDest.top, 
															hTransColor);
	}
	else
	{
		if(m_fScale != 1.0f)
			g_pLTClient->ScaleSurfaceToSurfaceSolidColor(	hScreen, 
															m_hCrosshair,
															&rcDest, 
															LTNULL, 
															hTransColor, 
															m_hCursorColor);
		else
			g_pLTClient->DrawSurfaceSolidColor(	hScreen, 
												m_hCrosshair,
												LTNULL, 
												rcDest.left,
												rcDest.top, 
												hTransColor, 
												m_hCursorColor);
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CCrosshairMgr::SetCrosshairColors()
//
// PURPOSE:		Sets the colors and alpha of the crosshair
//
// ----------------------------------------------------------------------- //

void CCrosshairMgr::SetCrosshairColors(LTFLOAT fRed, LTFLOAT fGreen, LTFLOAT fBlue, LTFLOAT fAlpha)
{
	m_bDrawSolid = LTTRUE;
	m_hCursorColor = g_pLTClient->SetupColor1(fRed,fGreen,fBlue,LTTRUE);
	g_pLTClient->SetSurfaceAlpha(m_hCrosshair,fAlpha);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CCrosshairMgr::SetCrosshairColors()
//
// PURPOSE:		Sets the colors and alpha of the crosshair
//
// ----------------------------------------------------------------------- //

void CCrosshairMgr::UpdateCrosshairColors(LTFLOAT fRed, LTFLOAT fGreen, LTFLOAT fBlue, LTFLOAT fAlpha)
{
	SetCrosshairColors(fRed, fGreen, fBlue, fAlpha);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CCrosshairMgr::EnableCrosshair()
//
// PURPOSE:		Enable croshairs
//
// ----------------------------------------------------------------------- //

void CCrosshairMgr::EnableCrosshair(LTBOOL b)
{ 
	m_bCrosshairEnabled = b; 
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CCrosshairMgr::LoadCrosshair()
//
// PURPOSE:		Load a new croshair
//
// ----------------------------------------------------------------------- //

void CCrosshairMgr::LoadCrosshair(char* szImage, LTFLOAT fScale, LTFLOAT fAlpha)
{
	if(szImage && strlen(szImage) > 0)
	{
//		if(m_hCrosshair)
//		{
//			g_pLTClient->DeleteSurface (m_hCrosshair);
//			m_hCrosshair		= LTNULL;
//		}

		m_bDrawSolid = LTFALSE;

		m_hCrosshair = g_pInterfaceResMgr->GetSharedSurface(szImage);
		g_pLTClient->SetSurfaceAlpha (m_hCrosshair, fAlpha);
		g_pLTClient->GetSurfaceDims(m_hCrosshair, &m_nHalfXhairWidth, &m_nHalfXhairHeight);
		m_nHalfXhairWidth>>=1;
		m_nHalfXhairHeight>>=1;
		m_fScale = fScale;
		m_bCustomCrosshairPos	= LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CCrosshairMgr::SetActivateCrosshair()
//
// PURPOSE:		Load a new croshair
//
// ----------------------------------------------------------------------- //

void CCrosshairMgr::SetActivateCrosshair(XHairMode eMode)
{
	//clean up the old crosshair
//	if(m_hActivateCrosshair)
//	{
//		g_pLTClient->DeleteSurface (m_hActivateCrosshair);
//		m_hActivateCrosshair = LTNULL;
//	}

	//load the name and scale
	switch (m_eActMode)
	{
	case (XHM_DEFAULT_ACTIVATE):
		m_hActivateCrosshair = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Marine\\Xhair_activate.pcx");
		m_fActScale = 1.0f;
		break;
	case (XHM_MARINE_WELD):
		m_hActivateCrosshair = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Marine\\Xhair_weld.pcx");
		m_fActScale = 1.0f;
		break;
	case (XHM_MARINE_CUT):
		m_hActivateCrosshair = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Marine\\Xhair_cut.pcx");
		m_fActScale = 1.0f;
		break;
	case (XHM_MARINE_HACK):
		if(IsPredator(g_pGameClientShell->GetPlayerMovement()->GetCharacterButes()))
			m_hActivateCrosshair = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Predator\\xhair_predhack.pcx");
		else
			m_hActivateCrosshair = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\Marine\\Xhair_hack.pcx");
		m_fActScale = 1.0f;
		break;
	case (XHM_PRED_DETONATE):
		m_hActivateCrosshair = g_pInterfaceResMgr->GetSharedSurface("Interface\\StatusBar\\predator\\Xhair_predbomb.pcx");
		m_fActScale = 1.0f;
		break;
	default: return;
	}

	//finish the setup here
	g_pLTClient->SetSurfaceAlpha (m_hActivateCrosshair, 1.0f);
	g_pLTClient->GetSurfaceDims(m_hActivateCrosshair, &m_nHalfActXhairWidth, &m_nHalfActXhairHeight);
	m_nHalfActXhairWidth>>=1;
	m_nHalfActXhairHeight>>=1;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CCrosshairMgr::SetActivateMode()
//
// PURPOSE:		Load a new croshair
//
// ----------------------------------------------------------------------- //

void CCrosshairMgr::SetActivateMode(XHairMode eMode)
{ 
	if(m_eActMode == eMode) return;
	
	//record our new mode
	m_eActMode = eMode;

	m_bUseActivate = eMode==XHM_TARGETING?LTFALSE:LTTRUE;

	if(m_bUseActivate)
		SetActivateCrosshair(eMode); 
}