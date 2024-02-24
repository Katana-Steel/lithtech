// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerStats.cpp
//
// PURPOSE : Implementation of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
 
#include "stdafx.h"
#include "WeaponMgr.h"
#include <stdio.h>
#include "GameClientShell.h"
#include "client_de.h"
#include "ClientRes.h"
#include "PlayerStats.h"
#include "MsgIDs.h"
#include "LayoutMgr.h"
#include "CharacterFuncs.h"
#include "VarTrack.h"
#include "ProfileMgr.h"
#include "MultiplayerClientMgr.h"

extern CGameClientShell* g_pGameClientShell;
extern CInterfaceMgr* g_pInterfaceMgr;
extern CObjectiveButeMgr* g_pObjectiveButeMgr;


namespace
{
	VarTrack g_vtHUDLayout;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CPlayerStats()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerStats::CPlayerStats()
{
	m_pnAmmoPool		= LTNULL;
	m_pnBarrelClip		= LTNULL;
	m_pbHaveWeapon		= LTNULL;
	m_nBarrelID			= LTNULL;

	m_nHealth			= 0;
	m_nArmor			= 0;
	m_nMaxHealth		= 0;
	m_nMaxArmor			= 0;
	m_nWepSet			= WMGR_INVALID_ID;
	m_nWepCount			= 0;
	m_nCurrentLayout	= 0;
	m_nButeSet			= 0;
	m_nCannonChargeLev	= 0;
	m_nSkullCount		= 0;

	m_bAutoTargetOn		= LTFALSE;

	m_fAirPercent		= 1.0f;
	m_fBatteryChargeLev	= 0.0f;
	
	m_hAutoTarget		= LTNULL;
	m_nZoomLevel		= 0;
	m_bCanZoom			= LTFALSE;
	m_bZooming			= LTFALSE;

	m_bHasMask			= LTFALSE;
	m_bHasNightVision	= LTFALSE;
	m_nObjectiveOverview= -1;
	m_bResetHealth		= LTTRUE;					
	m_bResetArmor		= LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::~CPlayerStats()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerStats::~CPlayerStats()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerStats::Init()
{
	if (!g_pLTClient) return DFALSE;

	if (m_pnAmmoPool)
	{
		delete [] m_pnAmmoPool;
		m_pnAmmoPool = LTNULL;
	}

	int nNumAmmoPoolTypes = g_pWeaponMgr->GetNumAmmoPools();
	if (nNumAmmoPoolTypes > 0)
	{
		m_pnAmmoPool = new DDWORD [nNumAmmoPoolTypes];
		memset(m_pnAmmoPool, 0, sizeof(DDWORD) * nNumAmmoPoolTypes);
	}

	int nNumBarrels = g_pWeaponMgr->GetNumBarrels();
	if (nNumBarrels > 0)
	{
		m_pnBarrelClip = new DDWORD [nNumBarrels];
		memset(m_pnBarrelClip, 0, sizeof(DDWORD) * nNumBarrels);
	}

	if (m_pbHaveWeapon)
	{
		delete [] m_pbHaveWeapon;
		m_pbHaveWeapon = LTNULL;
	}

	if (m_nBarrelID)
	{
		delete [] m_nBarrelID;
		m_nBarrelID = LTNULL;
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
		m_pbHaveWeapon = new LTBOOL [nNumWeapons];
		memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
		m_nBarrelID = new uint32 [nNumWeapons];
		memset(m_nBarrelID, -1, sizeof(uint32) * nNumWeapons);
	}

	g_vtHUDLayout.Init(g_pLTClient, "HUDLayout", NULL, 0.0f);

	// Set the initial attributes
	SetCharacterButes("Harris");

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Term()
//
//	PURPOSE:	Terminate the player stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Term()
{
	if (!g_pLTClient) return;

	m_nHealth			= 0;
	m_nArmor			= 0;
	m_nMaxHealth		= 0;
	m_nMaxArmor			= 0;
	m_nCannonChargeLev	= 0;
	m_nZoomLevel		= 0;
	m_bCanZoom			= LTFALSE;
	m_bZooming			= LTFALSE;
	m_nObjectiveOverview= -1;

	
	if (m_pnAmmoPool)
	{
		delete [] m_pnAmmoPool;
		m_pnAmmoPool = LTNULL;
	}

	if (m_pnBarrelClip)
	{
		delete [] m_pnBarrelClip;
		m_pnBarrelClip = LTNULL;
	}

	if (m_pbHaveWeapon)
	{
		delete [] m_pbHaveWeapon;
		m_pbHaveWeapon = LTNULL;
	}
	if (m_nBarrelID)
	{
		delete [] m_nBarrelID;
		m_nBarrelID = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::OnEnterWorld()
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CPlayerStats::OnEnterWorld(LTBOOL bRestoringGame)
{
	if (!g_pLTClient || !g_pGameClientShell) return;

	// find out what mode we are in and make sure that mode is set

	ResetStats();

	if (!bRestoringGame)
	{
		// clear the values
		Clear();

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CPlayerStats::OnExitWorld()
{
	if (!g_pLTClient) return;

	ClearObjectiveData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Draw()
//
//	PURPOSE:	Handle drawing the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Draw(LTBOOL bShowStats)
{
	if (!g_pLTClient)// || !g_pGameClientShell || !g_pGameClientShell->GetPlayerCameraObject()) 
		return;

	float m_nCurrentTime = g_pLTClient->GetTime();

	// get the screen size

	DDWORD nWidth = 0;
	DDWORD nHeight = 0;
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);


	DrawPlayerStats(hScreen, 0, 0, nWidth, nHeight, bShowStats);
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Clear()
//
//	PURPOSE:	Handle clearing the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Clear()
{
	UpdateHealth(0);
	UpdateArmor(0);

	int nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();
	for (DBYTE i = 0; i < nNumAmmoPools; i++)
	{
		UpdateAmmo(i, 0);
	}

	int nNumBarrels = g_pWeaponMgr->GetNumBarrels();
	for (i = 0; i < nNumBarrels; i++)
	{
		UpdateClip(i, 0);
	}

	// Reset our skull counter..
	m_nSkullCount = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ResetStats()
//
//	PURPOSE:	Reset the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ResetStats()
{
	//set the weapon set index
	if (g_pGameClientShell->GetGameType()->IsSinglePlayer())
		m_nWepSet = g_pCharacterButeMgr->GetWeaponSet(m_nButeSet);
	else if(g_pGameClientShell->GetMultiplayerMgr()->IsClassWeapons())
		m_nWepSet = g_pCharacterButeMgr->GetMPClassWeaponSet(m_nButeSet);
	else
		m_nWepSet = g_pCharacterButeMgr->GetMPWeaponSet(m_nButeSet);

	//reset zoom info
	m_nZoomLevel		= 0;
	m_bCanZoom			= LTFALSE;
	m_bZooming			= LTFALSE;
	m_nZoomLevel		= 0;

	//zoom in the camera
	LTFLOAT fZoom = g_pCharacterButeMgr->GetZoomLevel(m_nButeSet, m_nZoomLevel);
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	CameraMgrFX_Clear(hCamera, CAMERAMGRFX_ID_ZOOM);

	if(IsPredator(m_nButeSet) && m_bHasMask)
		m_bCanZoom = LTTRUE;

	m_bResetArmor = m_bResetHealth = LTTRUE;	
}	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateHealth()
//
//	PURPOSE:	Update the health stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateHealth(DDWORD nHealth)
{ 
	if (m_nHealth == nHealth) return;

	m_nHealth = nHealth;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateArmor()
//
//	PURPOSE:	Update the armor stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateArmor(DDWORD nArmor)
{ 
	if (m_nArmor == nArmor) return;

	if (nArmor > m_nMaxArmor)
		nArmor = m_nMaxArmor;

	// update the member variable
	m_nArmor = nArmor; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateCannonChargeLev()
//
//	PURPOSE:	Update the Cannon Charge Level stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateCannonChargeLev(uint32 nLevel)
{ 
	if (m_nCannonChargeLev == nLevel) return;

	// update the member variable
	m_nCannonChargeLev = nLevel; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMaxHealth()
//
//	PURPOSE:	Update the health stat maximum
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMaxHealth(DDWORD nHealth)
{ 
	if (m_nMaxHealth == nHealth) return;

	// update the member variable
	m_nMaxHealth		= nHealth;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMaxArmor()
//
//	PURPOSE:	Update the armor stat maximum
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMaxArmor(DDWORD nArmor)
{ 
	if (m_nMaxArmor == nArmor) return;

	// update the member variable
	m_nMaxArmor		= nArmor; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateBatteryLevel()
//
//	PURPOSE:	Update the battery level stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateBatteryLevel(LTFLOAT fLevel)
{ 
	if (m_fBatteryChargeLev == fLevel) return;

	// update the member variable
	m_fBatteryChargeLev	= fLevel; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAmmo()
//
//	PURPOSE:	Update the ammo stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateAmmo(DBYTE nAmmoPoolId, DDWORD nAmt, LTBOOL bPickedup, LTBOOL bDoHudNotify)
{ 
	if (!g_pGameClientShell || !g_pWeaponMgr) return;

	//get pointer to the ammo pool data
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool(nAmmoPoolId);

	//test
	if (pPool)
	{
		//test
		if (m_pnAmmoPool)
		{
			//get max amount data point
			DDWORD maxAmmo = pPool->GetMaxAmount(LTNULL);

			//bounds check for the new amount
			if (nAmt > maxAmmo)
				nAmt = maxAmmo;

			//re-set amount in pool to new amount
			if(bPickedup)
				m_pnAmmoPool[nAmmoPoolId] += nAmt;
			else
				m_pnAmmoPool[nAmmoPoolId] = nAmt;

			//verify that the current amount of ammo in the pool
			//is not greater than the max amount
			if (m_pnAmmoPool[nAmmoPoolId] > maxAmmo)
				m_pnAmmoPool[nAmmoPoolId] = maxAmmo;

			//if we have take some ammo, tell tell the player about it
			//with text or tint, etc.
			if (nAmt != 0)
			{
				if (bPickedup && bDoHudNotify)
					g_pGameClientShell->HandleAmmoPickup(nAmmoPoolId,nAmt);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ValidateClips()
//
//	PURPOSE:	Update the clip stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ValidateClips()
{
	uint32 nBarrels = g_pWeaponMgr->GetNumBarrels();

	for( uint32 i=0 ; i<nBarrels ; i++)
	{
		BARREL*		pBarrel		= g_pWeaponMgr->GetBarrel(i);
		AMMO*		pAmmo		= LTNULL;
		AMMO_POOL*	pAmmoPool	= LTNULL;

		if(pBarrel)
			pAmmo = g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);

		if(pAmmo)
			pAmmoPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

		if(pAmmoPool)
			if(m_pnBarrelClip[i] > m_pnAmmoPool[pAmmoPool->nId])
				m_pnBarrelClip[i] = m_pnAmmoPool[pAmmoPool->nId];
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateClip()
//
//	PURPOSE:	Update the clip stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateClip(DBYTE nBarrelId, DDWORD nAmt)
{ 
	if (!g_pGameClientShell || !g_pWeaponMgr) return;

	//get pointer to the ammo pool data
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel(nBarrelId);

	//test
	if (pBarrel)
	{
		//test
		if (m_pnBarrelClip)
		{
			//get max amount data point
			DDWORD maxAmt = pBarrel->nShotsPerClip;

			//verify that the current amount of ammo in the pool
			//is not greater than the max amount
			if (m_pnBarrelClip[nBarrelId] > maxAmt)
				m_pnBarrelClip[nBarrelId] = maxAmt;

			//bounds check for the new amount
			if (nAmt > maxAmt)
				nAmt = maxAmt;

			//re-set amount in pool to new amount
			m_pnBarrelClip[nBarrelId] = nAmt;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateWeapon()
//
//	PURPOSE:	Update the weapon stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateWeapon(DBYTE nWeaponId, LTBOOL bPickedup, LTBOOL bDoHudNotify)
{ 
	if (!g_pLTClient || !g_pWeaponMgr) return;

	if (bPickedup && g_pWeaponMgr->IsValidWeapon(nWeaponId))
	{
		if (m_pbHaveWeapon)
		{
			if (!m_pbHaveWeapon[nWeaponId] && bDoHudNotify)
				g_pGameClientShell->HandleWeaponPickup(nWeaponId);

			m_pbHaveWeapon[nWeaponId] = DTRUE;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAir()
//
//	PURPOSE:	Update the air stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateAir(DFLOAT fPercent)
{
	m_fAirPercent = fPercent;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawPlayerStats()
//
//	PURPOSE:	Draw the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawPlayerStats(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom, LTBOOL bShowStats)
{
	if (!g_pLTClient) return;

	// set up the transparent color

	HDECOLOR hTransColor = LTNULL;

	eOverlayMask eCurrMask = g_pInterfaceMgr->GetCurrentOverlay();


	switch (eCurrMask)
	{
		case OVM_SCOPE:
		{
			DrawScope(hScreen,nLeft,nTop,nRight,nBottom);
			break;
		}
	}

	
	// draw the HUD
	if (bShowStats)
	{
		g_pInterfaceMgr->DrawHud();

		// Make sure we are done forcing the statics..
		m_bResetArmor = m_bResetHealth = LTFALSE;	
	}

	//check the zoom FX status
	if(m_bZooming)
	{
		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_ZOOM, LTNULL);
		if(pEffect)
		{
			CAMERAMGRFX_ZOOM *pData = (CAMERAMGRFX_ZOOM*)pEffect->ceCS.pUserData;

			//turn effect on or off
			if(pData->fMagnitude == pData->fDestMagnitude)
			{
				//done zooming
				m_bZooming = LTFALSE;
				g_pGameClientShell->GetVisionModeMgr()->SetPredZoomMode(LTFALSE);
			}
			else
				g_pGameClientShell->GetVisionModeMgr()->SetPredZoomMode(LTTRUE);
		}
		else
		{
			//if there is no effect then be sure to turn off the overlay
			m_bZooming = LTFALSE;
			g_pGameClientShell->GetVisionModeMgr()->SetPredZoomMode(LTFALSE);
		}
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetAmmoCount
//
//	PURPOSE:	Get the ammo count for the passed in ammo id
//
// --------------------------------------------------------------------------- //

DDWORD CPlayerStats::GetAmmoCount(DBYTE nAmmoId) const
{
	if (!m_pnAmmoPool || !g_pWeaponMgr->IsValidAmmoType(nAmmoId))  return 0; 
	 
	int nPoolId = g_pWeaponMgr->GetPoolId(nAmmoId);

	return m_pnAmmoPool[nPoolId]; 
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveWeapon
//
//	PURPOSE:	Do we have the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerStats::HaveWeapon(DBYTE nWeaponId) const
{
	 if (!m_pbHaveWeapon || !g_pWeaponMgr->IsValidWeapon(nWeaponId)) return DFALSE; 
	 
	 return m_pbHaveWeapon[nWeaponId]; 
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::SetHasMask
//
//	PURPOSE:	Toggle back to the normal mode if we lose our mask
//
// --------------------------------------------------------------------------- //

void CPlayerStats::SetHasMask(LTBOOL bHas)
{
	if(m_bHasMask && !bHas)
	{
		g_pGameClientShell->GetVisionModeMgr()->SetNormalMode();

		// be sure we are zoomed out
		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		CameraMgrFX_ZoomToggle(hCamera, 1.0f);
	}

	m_bHasMask	= bHas;
	m_bCanZoom	= bHas;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::SetHasNightVision
//
//	PURPOSE:	Set do we have night vision
//
// --------------------------------------------------------------------------- //

void CPlayerStats::SetHasNightVision(LTBOOL bHas)
{
	if(m_bHasNightVision && !bHas)
	{
		g_pGameClientShell->GetVisionModeMgr()->SetNormalMode();
	}

	m_bHasNightVision = bHas;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ResetInventory
//
//	PURPOSE:	Reset all inventory items
//
// --------------------------------------------------------------------------- //

void CPlayerStats::ResetInventory()
{
	// Clear our data...

	int nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();
	if (nNumAmmoPools > 0)
	{
		memset(m_pnAmmoPool, 0, sizeof(DDWORD) * nNumAmmoPools);
	}

	int nNumBarrels = g_pWeaponMgr->GetNumBarrels();
	if (nNumBarrels > 0)
	{
		memset(m_pnBarrelClip, 0, sizeof(DDWORD) * nNumBarrels);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
		memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
		memset(m_nBarrelID, -1, sizeof(uint32) * nNumWeapons);
	}

	// Now clear out the misc stuff...
	SetHasMask(LTFALSE);
	SetHasNightVision(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DrawScope()
//
//	PURPOSE:	Draw the mask and crosshairs for the zoomed view
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DrawScope(HSURFACE hScreen, int nLeft, int nTop, int nRight, int nBottom)
{
	LTRect rect;
	int nType = 1;
	int nScreenWidth		= nRight - nLeft;
	int nScreenHeight		= nBottom - nTop;

	HDECOLOR hBlack = SETRGB(0,0,0);
	HDECOLOR hGray  = SETRGB(128,128,128);
	HDECOLOR hGold  = SETRGB(140,128,20);


	//left mask
	rect.top = nTop;
	rect.left = nLeft;
	rect.bottom = nBottom;
	rect.right = nLeft + (nScreenWidth / 8);
	g_pLTClient->FillRect(hScreen,&rect,hBlack);
	
	//right mask
	rect.top = nTop;
	rect.left = nRight - (nScreenWidth / 8);
	rect.bottom = nBottom;
	rect.right = nRight;
	g_pLTClient->FillRect(hScreen,&rect,hBlack);

	int cx = nLeft + (nScreenWidth) / 2;
	int cy = nTop + (nScreenHeight) / 2;

	int nRadius = (int)( 0.475f * (float)nScreenHeight );

	LTRect srect;
	srect.top = 0;
	srect.left = 0;
	srect.bottom = 2;
	srect.right = 2;

	switch (nType)
	{
	case 1:
		{
			int nGap = nScreenHeight / 24;

			//left outline
			rect.top = cy - 2;
			rect.left = cx - nRadius;
			rect.bottom = cy + 2;
			rect.right = cx - nGap;
			g_pLTClient->FillRect(hScreen,&rect,hBlack);

			//left post
			rect.top = cy - 1;
			rect.left = cx - nRadius;
			rect.bottom = cy + 1;
			rect.right = (cx - nGap) - 1;
			g_pLTClient->FillRect(hScreen,&rect,hGold);

			//right outline
			rect.top = cy - 2;
			rect.left = cx + nGap;
			rect.bottom = cy + 2;
			rect.right = cx + nRadius;
			g_pLTClient->FillRect(hScreen,&rect,hBlack);

			//right post
			rect.top = cy - 1;
			rect.left = cx + nGap + 1;
			rect.bottom = cy + 1;
			rect.right = cx + nRadius;
			g_pLTClient->FillRect(hScreen,&rect,hGold);

			//top outline
			rect.top = cy - nRadius;
			rect.left = cx - 2;
			rect.bottom = cy - nGap;
			rect.right = cx + 2;
			g_pLTClient->FillRect(hScreen,&rect,hBlack);

			//top post
			rect.top = cy - nRadius;
			rect.left = cx - 1;
			rect.bottom = (cy - nGap) - 1;
			rect.right = cx + 1;
			g_pLTClient->FillRect(hScreen,&rect,hGold);

			//bottom outline
			rect.top = cy + nGap;
			rect.left = cx - 2;
			rect.bottom = cy + nRadius;
			rect.right = cx + 2;
			g_pLTClient->FillRect(hScreen,&rect,hBlack);

			//bottom post
			rect.top = cy + nGap + 1;
			rect.left = cx - 1;
			rect.bottom = cy+ nRadius;
			rect.right = cx + 1;
			g_pLTClient->FillRect(hScreen,&rect,hGold);

			//horizontal hair
			rect.top = cy;
			rect.left = cx - nGap;
			rect.bottom = rect.top + 1;
			rect.right = cx + nGap;
			g_pLTClient->FillRect(hScreen,&rect,hBlack);

			//vertical hair
			rect.top = cy - nGap;
			rect.left = cx;
			rect.bottom = cy + nGap;
			rect.right = rect.left + 1;
			g_pLTClient->FillRect(hScreen,&rect,hBlack);

			//horizontal hair
			rect.top = cy-1;
			rect.left = cx - nGap;
			rect.bottom = rect.top + 1;
			rect.right = cx + nGap;
			g_pLTClient->FillRect(hScreen,&rect,hGold);

			//vertical hair
			rect.top = cy - nGap;
			rect.left = cx-1;
			rect.bottom = cy + nGap;
			rect.right = rect.left + 1;
			g_pLTClient->FillRect(hScreen,&rect,hGold);

		}	break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveAirSupply
//
//	PURPOSE:	Returns true if current gear provides air supply
// 
// ----------------------------------------------------------------------- //

LTBOOL CPlayerStats::HaveAirSupply()
{
	return LTTRUE;
}

void CPlayerStats::NextLayout()
{
	if(g_pLayoutMgr->GetNumHUDLayouts(GetHUDType()) > 1)
	{
		m_nCurrentLayout++;
		if (m_nCurrentLayout >= g_pLayoutMgr->GetNumHUDLayouts(GetHUDType()))
			m_nCurrentLayout = 0;
		g_vtHUDLayout.SetFloat((DFLOAT)m_nCurrentLayout);

		g_pInterfaceMgr->ReloadHUD(m_nCurrentLayout);
	}
}

void CPlayerStats::PrevLayout()
{
	if(g_pLayoutMgr->GetNumHUDLayouts(GetHUDType()) > 1)
	{
		m_nCurrentLayout--;
		if (m_nCurrentLayout < 0)
			m_nCurrentLayout =  g_pLayoutMgr->GetNumHUDLayouts(GetHUDType()) - 1;
		g_vtHUDLayout.SetFloat((DFLOAT)m_nCurrentLayout);

		g_pInterfaceMgr->ReloadHUD(m_nCurrentLayout);
	}
}


void CPlayerStats::ClearSurface(HSURFACE hSurf, HDECOLOR hColor)
{
	DDWORD dwWidth, dwHeight;
	g_pLTClient->GetSurfaceDims(hSurf, &dwWidth, &dwHeight);
	LTRect rcSrc;
	rcSrc.left = rcSrc.top = 0;
	rcSrc.right = dwWidth;
	rcSrc.bottom = dwHeight;
	g_pLTClient->FillRect(hSurf, &rcSrc, hColor);
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::SetCharacterButes()
//
// PURPOSE:		Find the correct character name, and then set the attributes
//
// ----------------------------------------------------------------------- //

void CPlayerStats::SetCharacterButes(char *szName)
{
	if(!szName || !g_pCharacterButeMgr || !g_pInterface) return;

	// Find out which character we're looking for
	for(int i = 0; i < g_pCharacterButeMgr->GetNumSets(); i++)
	{
		if(!stricmp(szName, g_pCharacterButeMgr->GetModelType(i)))
		{
			// If we found the character, set the butes and exit
			SetCharacterButes(i);
			return;
		}
	}

	// Otherwise, leave it the same and print out an error
	g_pInterface->CPrint("ERROR!! CPlayerStats::SetCharacterButes() --- Could not locate the character set: %s", szName);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::SetCharacterButes()
//
// PURPOSE:		Set the attributes from an index
//
// ----------------------------------------------------------------------- //

void CPlayerStats::SetCharacterButes(int nType)
{
	if(!g_pCharacterButeMgr || !g_pInterface) return;

	// Make sure the type is within range
	if((nType < 0) || (nType >= g_pCharacterButeMgr->GetNumSets()))
	{
		g_pInterface->CPrint("ERROR!! CharacterMovement::SetCharacterButes() --- Type %d is not within range from zero to %d", nType, g_pCharacterButeMgr->GetNumSets() - 1);
		return;
	}

	// Fill in all the attributes here
	m_nButeSet = nType;

	ResetStats();

	if(g_pInterfaceMgr)
	{
		//set num weapons
		WEAPON_SET *pWepSet = g_pWeaponMgr->GetWeaponSet(m_nWepSet);

		if(pWepSet)
		{
			m_nWepCount = pWepSet->nNumWeapons;
		}

		g_pInterfaceMgr->InitWepChooser();

	}

	if (g_pProfileMgr)
		g_pProfileMgr->SetSpecies(g_pGameClientShell->GetPlayerSpecies());
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetHUDType()
//
// PURPOSE:		Gets the current HUD type
//
// ----------------------------------------------------------------------- //

DDWORD CPlayerStats::GetHUDType()
{
	return g_pCharacterButeMgr->GetHUDType(m_nButeSet);
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetWeaponFromSet()
//
// PURPOSE:		Gets the index value of the weapon from the set
//
// ----------------------------------------------------------------------- //

const char * CPlayerStats::GetWeaponFromSet(int nIndex)
{
	WEAPON_SET *pWepSet = g_pWeaponMgr->GetWeaponSet(m_nWepSet);

	if(pWepSet && nIndex < pWepSet->nNumWeapons)
	{
		return pWepSet->szWeaponName[nIndex];
	}
	else 
		return LTNULL;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetWeaponIndex()
//
// PURPOSE:		Returns the index of a weapon into the weapon set
//				ie 0 for first weapon, 1 for second in set, etc.
//
// ----------------------------------------------------------------------- //

int CPlayerStats::GetWeaponIndex(int nWepId)
{
	//get pointer to my weapon set
	WEAPON_SET* pWepSet = g_pWeaponMgr->GetWeaponSet(m_nWepSet);

	//test
	if(!pWepSet)
		return WMGR_INVALID_ID;

	//run the weapons in the set
	for(int i=0 ; i < pWepSet->nNumWeapons ; i++)
	{
		//get pointer to the weapon
		WEAPON* pWep = g_pWeaponMgr->GetWeapon(pWepSet->szWeaponName[i]);

		//test against argument
		if(pWep)
			if(pWep->nId == nWepId)
				return i;
	}

	//couldn't find weapon in the set
	return WMGR_INVALID_ID;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetAmmoInClip()
//
// PURPOSE:		Returns the index of a weapon into the weapon set
//				ie 0 for first weapon, 1 for second in set, etc.
//
// ----------------------------------------------------------------------- //

int	CPlayerStats::GetAmmoInClip(int nBarrelId)
{
	if(!m_pnBarrelClip && !g_pWeaponMgr)
		return 0;
	
	int nNumBarrels = g_pWeaponMgr->GetNumBarrels();

	if(nBarrelId < nNumBarrels && nBarrelId >= 0)
		return m_pnBarrelClip[nBarrelId];
	else
		return 0;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::DecrementClip()
//
// PURPOSE:		Decrements a clip
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DecrementClip(int nBarrelId)
{
	if(!m_pnBarrelClip && !g_pWeaponMgr)
		return;
	
	int nNumBarrels = g_pWeaponMgr->GetNumBarrels();

	if(nBarrelId < nNumBarrels && nBarrelId >= 0)
		m_pnBarrelClip[nBarrelId]--;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetMaxMarineFlares()
//
// PURPOSE:		Get the maximum amount of Marine flares
//
// ----------------------------------------------------------------------- //

int	CPlayerStats::GetMaxMarineFlares()
{
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("flares");

	if(pPool)
	{
		return pPool->GetMaxAmount(g_pLTClient->GetClientObject());
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetMarineFlares()
//
// PURPOSE:		Get the current amount of Marine flares
//
// ----------------------------------------------------------------------- //

int	CPlayerStats::GetMarineFlares()
{
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("flares");

	if(pPool && m_pnAmmoPool)
	{
		return m_pnAmmoPool[pPool->nId];
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetMaxPredatorEnergy()
//
// PURPOSE:		Get the maximum amount of predator energy
//
// ----------------------------------------------------------------------- //

int	CPlayerStats::GetMaxPredatorEnergy()
{
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Predator_Energy");

	if(pPool)
	{
		return pPool->GetMaxAmount(g_pLTClient->GetClientObject());
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::GetPredatorEnergy()
//
// PURPOSE:		Get the current amount of predator energy
//
// ----------------------------------------------------------------------- //

int	CPlayerStats::GetPredatorEnergy()
{
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Predator_Energy");

	if(pPool && m_pnAmmoPool)
	{
		return m_pnAmmoPool[pPool->nId];
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::ToggleAutoTargeting()
//
// PURPOSE:		Get the current amount of predator energy
//
// ----------------------------------------------------------------------- //
void CPlayerStats::ToggleAutoTargeting()
{ 
	if(m_bAutoTargetOn)
	{
		//we are on so reset crosshair and turn off
		SetAutoTargetOn(LTFALSE);
		SetAutoTarget(LTNULL);

		// white crosshair
		g_pInterfaceMgr->GetCrosshairMgr()->SetCrosshairColors( 1.0, 1.0, 1.0, 1.0);
		g_pInterfaceMgr->GetCrosshairMgr()->SetCustomCHPosOn(LTFALSE);

		g_pInterfaceMgr->GetAutoTargetMgr()->ResetSounds();
	}
	else
	{
		SetAutoTargetOn(LTTRUE);
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CPlayerStats::DecrementAmmo()
//
// PURPOSE:		Decrements a ammo type
//
// ----------------------------------------------------------------------- //

void CPlayerStats::DecrementAmmo(int nAmmoId)
{
	//get ammo data
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);

	//test
	if(!pAmmo)
		return;

	//get ammo pool data
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);

	//test
	if(!pPool)
		return;
		
	//decrement the ammo pool
	m_pnAmmoPool[pPool->nId] -= pAmmo->nAmmoPerShot;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ZoomOut()
//
//	PURPOSE:	Zoom in the view
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ZoomOut()
{
	if(!m_bCanZoom || m_nZoomLevel==0) return;

	//decriment zoom level
	--m_nZoomLevel;

	//get the next zoom level from the bute mgr
	LTFLOAT fZoom = g_pCharacterButeMgr->GetZoomLevel(m_nButeSet, m_nZoomLevel);

	//zoom in the camera
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	CameraMgrFX_ZoomToggle(hCamera, fZoom);
	g_pClientSoundMgr->PlaySoundLocal("Sounds\\interface\\predator_zoom_out.wav", SOUNDPRIORITY_PLAYER_HIGH);

	//set the overlay FX
	m_bZooming = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ZoomIn()
//
//	PURPOSE:	Zoom in the view
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ZoomIn()
{
	if(!m_bCanZoom || m_nZoomLevel+1>=MAX_ZOOM_LEVELS) return;

	//decriment zoom level
	++m_nZoomLevel;

	//get the next zoom level from the bute mgr
	LTFLOAT fZoom = g_pCharacterButeMgr->GetZoomLevel(m_nButeSet, m_nZoomLevel);

	//zoom in the camera
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	CameraMgrFX_ZoomToggle(hCamera, fZoom);
	g_pClientSoundMgr->PlaySoundLocal("Sounds\\interface\\predator_zoom_in.wav", SOUNDPRIORITY_PLAYER_HIGH);

	//set the overlay FX
	m_bZooming = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetExoEnergyCount()
//
//	PURPOSE:	Get the amount of exo energy
//
// ----------------------------------------------------------------------- //

uint32 CPlayerStats::GetExoEnergyCount()
{
	if(!g_pWeaponMgr) return 0;

	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Energy");

	if(!pPool) return 0;

	return m_pnAmmoPool[pPool->nId];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetMaxExoEnergyCount()
//
//	PURPOSE:	Get the maximum amount of exo energy
//
// ----------------------------------------------------------------------- //

uint32 CPlayerStats::GetMaxExoEnergyCount()
{
	if(!g_pWeaponMgr) return 0;

	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Energy");

	if(pPool)
		return pPool->GetMaxAmount(g_pLTClient->GetClientObject()); 
	
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetHealthCount()
//
//	PURPOSE:	Get the player's health
//
// ----------------------------------------------------------------------- //

uint32 CPlayerStats::GetHealthCount()	
{ 
	return g_pGameClientShell->IsPlayerDead()?0:m_nHealth; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::AddObjective
//
//	PURPOSE:	Add an objective
//
// ----------------------------------------------------------------------- //

void CPlayerStats::AddObjective(uint32 nId)
{
	_ASSERT(nId != 0);

	if(nId > (uint32)g_pObjectiveButeMgr->GetNumObjectiveButes()) return;

	//find an empty place to add our objective
	for (int i=0; i<MAX_OBJECTIVES; i++)
	{
		if(m_Objective[i].nId == 0)
		{
			m_Objective[i].nId = nId;
			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateObjective
//
//	PURPOSE:	Complete an objective
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateObjective(uint32 nId, ObjectiveState eState)
{
	_ASSERT(nId != 0);

	//find this objective and close it
	for (int i=0; i<MAX_OBJECTIVES; i++)
	{
		if(m_Objective[i].nId == nId)
		{
			m_Objective[i].eState = eState;
			return;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::SetBarrelID
//
//	PURPOSE:	Record the last barrel used.
//
// ----------------------------------------------------------------------- //

void CPlayerStats::SetBarrelID(uint32 nWeaponID, uint32 nBarrelID)
{
	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

	if(nWeaponID < (uint32)nNumWeapons)
	{
		m_nBarrelID[nWeaponID] = nBarrelID;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetBarrelID
//
//	PURPOSE:	Get the last barrel used.
//
// ----------------------------------------------------------------------- //

uint32 CPlayerStats::GetBarrelID(uint32 nWeaponID)
{
	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

	if(nWeaponID < (uint32)nNumWeapons)
	{
		return m_nBarrelID[nWeaponID];
	}

	return -1;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Save
//
//	PURPOSE:	Save the player stats info
//
// --------------------------------------------------------------------------- //

void CPlayerStats::Save(HMESSAGEWRITE hWrite)
{
	if (!g_pLTClient || !g_pWeaponMgr) return;

	g_pLTClient->WriteToMessageDWord(hWrite, m_nButeSet);
	g_pLTClient->WriteToMessageDWord(hWrite, m_nWepSet);
	g_pLTClient->WriteToMessageDWord(hWrite, m_nHealth);
	g_pLTClient->WriteToMessageDWord(hWrite, m_nArmor);
	g_pLTClient->WriteToMessageDWord(hWrite, m_nMaxHealth);
	g_pLTClient->WriteToMessageDWord(hWrite, m_nMaxArmor);
	g_pLTClient->WriteToMessageDWord(hWrite, m_nCannonChargeLev);
	g_pLTClient->WriteToMessageDWord(hWrite, m_nSkullCount);

	int nNumAmmoPoolTypes = g_pWeaponMgr->GetNumAmmoPools();
	AMMO_POOL* pPool = LTNULL;
	for (DBYTE i = 0; i < nNumAmmoPoolTypes; i++)
	{
		pPool = g_pWeaponMgr->GetAmmoPool(i);

		if(pPool && pPool->bSave)
			g_pLTClient->WriteToMessageWord(hWrite, (uint16)m_pnAmmoPool[i]);
	}

	int nNumBarrels = g_pWeaponMgr->GetNumBarrels();
	BARREL* pBarrel = LTNULL;
	for (i = 0; i < nNumBarrels; i++)
	{
		pBarrel = g_pWeaponMgr->GetBarrel(i);

		if(pBarrel && pBarrel->bSave)
			g_pLTClient->WriteToMessageWord(hWrite, (uint16)m_pnBarrelClip[i]);
	}


	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	WEAPON* pWeapon = LTNULL;
	for ( i = 0; i < nNumWeapons; i++)
	{
		pWeapon = g_pWeaponMgr->GetWeapon(i);

		if(pWeapon && pWeapon->bSave)
		{
			g_pLTClient->WriteToMessageByte(hWrite, m_pbHaveWeapon[i]);
			g_pLTClient->WriteToMessageDWord(hWrite, m_nBarrelID[i]);
		}
	}

	g_pLTClient->WriteToMessageFloat(hWrite, m_fAirPercent);
	g_pLTClient->WriteToMessageByte(hWrite, m_bHasMask);
	g_pLTClient->WriteToMessageByte(hWrite, m_bHasNightVision);

	for(  i = 0; i < MAX_OBJECTIVES; ++i )
	{
		m_Objective[i].Save(hWrite);
	}

	g_pLTClient->WriteToMessageDWord(hWrite, (uint32)m_nObjectiveOverview);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Load
//
//	PURPOSE:	Load the player stats info
//
// --------------------------------------------------------------------------- //

void CPlayerStats::Load(HMESSAGEREAD hRead)
{
	if (!g_pLTClient || !g_pWeaponMgr) return;
	
	m_nButeSet			= g_pLTClient->ReadFromMessageDWord(hRead);
	m_nWepSet			= g_pLTClient->ReadFromMessageDWord(hRead);
	m_nHealth			= g_pLTClient->ReadFromMessageDWord(hRead);
	m_nArmor			= g_pLTClient->ReadFromMessageDWord(hRead);
	m_nMaxHealth		= g_pLTClient->ReadFromMessageDWord(hRead);
	m_nMaxArmor			= g_pLTClient->ReadFromMessageDWord(hRead);
	m_nCannonChargeLev	= g_pLTClient->ReadFromMessageDWord(hRead);
	m_nSkullCount		= g_pLTClient->ReadFromMessageDWord(hRead);

	int nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();
	AMMO_POOL* pPool = LTNULL;
	for (DBYTE i = 0; i < nNumAmmoPools; i++)
	{
		pPool = g_pWeaponMgr->GetAmmoPool(i);

		if(pPool && pPool->bSave)
			m_pnAmmoPool[i]		= (uint32)g_pLTClient->ReadFromMessageWord(hRead);
	}

	int nNumBarrels = g_pWeaponMgr->GetNumBarrels();
	BARREL* pBarrel = LTNULL;
	for (i = 0; i < nNumBarrels; i++)
	{
		pBarrel = g_pWeaponMgr->GetBarrel(i);

		if(pBarrel && pBarrel->bSave)
			m_pnBarrelClip[i] = (uint32)g_pLTClient->ReadFromMessageWord(hRead);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	WEAPON* pWeapon = LTNULL;
	for ( i = 0; i < nNumWeapons; i++)
	{
		pWeapon = g_pWeaponMgr->GetWeapon(i);

		if(pWeapon && pWeapon->bSave)
		{
			m_pbHaveWeapon[i] = (LTBOOL)g_pLTClient->ReadFromMessageByte(hRead);
			m_nBarrelID[i] = g_pLTClient->ReadFromMessageDWord(hRead);
		}
	}

	m_fAirPercent		= g_pLTClient->ReadFromMessageFloat(hRead);
	m_bHasMask			= (LTBOOL)g_pLTClient->ReadFromMessageByte(hRead);
	m_bHasNightVision	= g_pLTClient->ReadFromMessageByte(hRead);

	for( i = 0; i < MAX_OBJECTIVES; ++i )
	{
		m_Objective[i].Load(hRead);
	}

	m_nObjectiveOverview = (int)g_pLTClient->ReadFromMessageDWord(hRead);

	// Make sure the hud updates...
	m_bResetArmor = m_bResetHealth = LTTRUE;	
}

