// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponUtils.cpp
//
// PURPOSE : Client-side firing helper functions
//
// CREATED : 11/2/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeaponUtils.h"
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "ClientServerShared.h"
#include "WeaponFX.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"

extern CGameClientShell* g_pGameClientShell;
extern ILTClient* g_pLTClient;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AttackerLiquidFilterFn()
//
//	PURPOSE:	Filter the attacker out of CastRay and/or
//				IntersectSegment calls (so you don't shot yourself).
//				However, we want to ignore liquid as well...
//
// ----------------------------------------------------------------------- //

LTBOOL AttackerLiquidFilterFn(HLOCALOBJ hObj, void *pUserData)
{
	// We're not attacking our self...

	if (SpecificObjectFilterFn(hObj, pUserData))
	{
        // Return LTTRUE to keep this object (not liquid), or LTFALSE to ignore
		// this object (is liquid)...

        uint16 code;
        if (g_pLTClient && g_pLTClient->GetContainerCode(hObj, &code))
		{
			ContainerCode eCode = (ContainerCode)code;

			if (IsLiquid(eCode))
			{
                return LTFALSE;
			}
		}

        return LTTRUE;
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddLocalImpactFX
//
//	PURPOSE:	Add a weapon impact special fx
//
// ----------------------------------------------------------------------- //

void AddLocalImpactFX(HLOCALOBJ hTargetObj,
					  HLOCALOBJ hFiredFrom,
					  LTVector & vFirePos, 
					  LTVector & vImpactPoint,
                      LTVector & vNormal, 
					  SurfaceType eType, 
					  LTVector & vPath,
                      uint8 nWeaponId, 
					  uint8 nAmmoId, 
					  uint8 nBarrelId, 
					  uint16 wIgnoreFX)
{
    if (!g_pLTClient || !g_pGameClientShell) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// see if we need to block the impact mark
    uint16 wIgnFX = wIgnoreFX;
	if (!CanMarkObject(hTargetObj))
		wIgnFX |= WFX_MARK;
	
	// back off the impact point by one unit
    LTVector vDir = vImpactPoint - vFirePos;
	vDir.Norm();
	vImpactPoint -= vDir * GetRandom(0.9f, 1.1f);

	// get the local client ID
    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);

	// fill in the create struct
	WCREATESTRUCT w;

	w.nWeaponId		= nWeaponId;
	w.nBarrelId		= nBarrelId;
	w.nAmmoId		= nAmmoId;
	w.nFlashSocket	= 0;
	w.nSurfaceType	= eType;
	w.wIgnoreFX		= wIgnFX;
    w.hFiredFrom	= hFiredFrom;
	w.vFirePos		= vFirePos;
	w.vPos			= vImpactPoint;//vPos;
	w.nShooterId	= (uint8)dwId;
	w.bLocal		= LTTRUE;
	w.vSurfaceNormal= vNormal;

	psfxMgr->CreateSFX(SFX_WEAPON_ID, &w);
}
