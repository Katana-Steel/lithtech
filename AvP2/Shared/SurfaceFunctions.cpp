// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceFunctions.cpp
//
// PURPOSE : Implementation of shared surface functions
//
// CREATED : 7/13/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ContainerCodes.h"
#include "SurfaceFunctions.h"
#include "ClientServerShared.h"
#include "CommonUtilities.h"
#include "WeaponMgr.h"
#include "physics_lt.h"
#include "FXButeMgr.h"

extern char s_FileBuffer[MAX_CS_FILENAME_LEN];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsMark
//
//	PURPOSE:	Does this type of surface show marks
//
// ----------------------------------------------------------------------- //

LTBOOL ShowsMark(SurfaceType eSurfType)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	if (pSurf)
	{
		return pSurf->bShowsMark;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsFootprints
//
//	PURPOSE:	Does this type of surface show footprints
//
// ----------------------------------------------------------------------- //

LTBOOL ShowsFootprints(SurfaceType eSurfType)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	if (pSurf)
	{
		return (pSurf->szLtFootPrintSpr[0] && pSurf->szRtFootPrintSpr[0]);
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSprite()
//
//	PURPOSE:	Get impact sprite with this surface
//
// ----------------------------------------------------------------------- //

LTBOOL GetImpactSprite(SurfaceType eSurfType, LTFLOAT & fScale, int nAmmoId,
					  char* pBuf, int nBufLen)
{
	if (!g_pWeaponMgr || !ShowsMark(eSurfType) || !pBuf)
	{
		return LTFALSE;
	}

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo || !pAmmo->pImpactFX) return LTFALSE;

	// Get the impact mark filename...

	strncpy(pBuf, pAmmo->pImpactFX->szMark, nBufLen);
	if (!pBuf[0]) return LTFALSE;
	
	if (stricmp(pBuf, "SURFACE") == 0)
	{
		// Use the surface to dtermine the bullet hole...

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
		if (pSurf && pSurf->szBulletHoleSpr[0])
		{
			strcpy(pBuf, pSurf->szBulletHoleSpr);
			fScale = GetRandom(pSurf->fBulletHoleMinScale, 
							   pSurf->fBulletHoleMaxScale);
		}
		else
		{
			return LTFALSE;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagToSurface()
//
//	PURPOSE:	Convert a user flag to a surface type
//
// ----------------------------------------------------------------------- //

SurfaceType UserFlagToSurface(uint32 dwUserFlag)
{
	// Top byte contains surface flag

	SurfaceType eSurfType = (SurfaceType) (dwUserFlag >> USRFLG_SURFACE_BITOFFSET);

	return eSurfType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SurfaceToUserFlag()
//
//	PURPOSE:	Convert surface type to a user flag
//
// ----------------------------------------------------------------------- //

uint32 SurfaceToUserFlag(SurfaceType eSurfType)
{
	// Top byte should contain surface flag
	if(eSurfType > 15) return 0;

	uint32 dwUserFlag = (uint32)eSurfType;
	dwUserFlag = (dwUserFlag << USRFLG_SURFACE_BITOFFSET);

	return dwUserFlag;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagSurfaceMask()
//
//	PURPOSE:	Masks out any surface type from the user flags
//
// ----------------------------------------------------------------------- //

uint32 UserFlagSurfaceMask(uint32 dwUserFlag)
{
	return dwUserFlag & ~(uint32(-1) << USRFLG_SURFACE_BITOFFSET);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsFleshSurfaceType()
//
//	PURPOSE:	Is this a type of flesh surface type
//
// ----------------------------------------------------------------------- //

LTBOOL IsFleshSurfaceType(SurfaceType eSurfType)
{
	switch(eSurfType)
	{
		case ST_FLESH_HUMAN:		return LTTRUE;
		case ST_FLESH_PREDATOR:		return LTTRUE;
		case ST_FLESH_ALIEN:		return LTTRUE;
		case ST_FLESH_SYNTH:		return LTTRUE;
		case ST_FLESH_PRAETORIAN:	return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetFootStepSound()
//
//	PURPOSE:	Get foot step sounds associated with this surface
//
// ----------------------------------------------------------------------- //

char* GetFootStepSound(SurfaceType eSurfType, LTBOOL bLeftFoot)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	_ASSERT(pSurf);
	if (!pSurf) return DNULL;

	int nIndex = GetRandom(0, SRF_MAX_FOOTSTEP_SNDS-1);
	return (bLeftFoot ? pSurf->szLtFootStepSnds[nIndex] : 
		pSurf->szRtFootStepSnds[nIndex]);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSound()
//
//	PURPOSE:	Get impact sounds associated with this surface
//
// ----------------------------------------------------------------------- //

char* GetImpactSound(SurfaceType eSurfType, int nAmmoId)
{
	static char szRval[255];

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	_ASSERT(pAmmo);
	if (!pAmmo) return LTNULL;

	if (!(pAmmo->pImpactFX->szSound[0])) return DNULL;
	SAFE_STRCPY(s_FileBuffer, pAmmo->pImpactFX->szSound);

	if (_stricmp(s_FileBuffer, "SURFACE") == 0)
	{
		if (pAmmo->eType == VECTOR)
		{
			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
			_ASSERT(pSurf);
			if (!pSurf) return LTNULL;

			sprintf(szRval, pSurf->szBulletImpactSnds[GetRandom(0, SRF_MAX_IMPACT_SNDS-1)], pAmmo->pImpactFX->szSoundDir);

			return szRval;
		}
		else
		{
			return LTNULL;
		}
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

LTBOOL IsMoveable(HOBJECT hObj)
{
	if (!hObj) return LTFALSE;

	uint32 dwUserFlags;

#ifdef _CLIENTBUILD
	g_pInterface->GetObjectUserFlags(hObj, &dwUserFlags);
#else
	dwUserFlags = g_pInterface->GetObjectUserFlags(hObj);
#endif

	return !!(dwUserFlags & USRFLG_MOVEABLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CanMarkObject()
//
//	PURPOSE:	Determine if the passed in object can be marked
//
// ----------------------------------------------------------------------- //

LTBOOL CanMarkObject(HOBJECT hObj)
{
	if (!hObj) return LTFALSE;

	// Objects that don't move can be marked...

	if (!IsMoveable(hObj)) return LTTRUE;

#ifndef _CLIENTBUILD

	// Can also mark world models on the server...

	HCLASS hObjClass	= g_pInterface->GetObjectClass(hObj);
	HCLASS hDoor		= g_pInterface->GetClass("Door");
	HCLASS hRotWM		= g_pInterface->GetClass("RotatingWorldModel");
	HCLASS hTransWM		= g_pInterface->GetClass("TranslucentWorldModel");

	if (g_pInterface->IsKindOf(hObjClass, hDoor) ||
		g_pInterface->IsKindOf(hObjClass, hRotWM) ||
		g_pInterface->IsKindOf(hObjClass, hTransWM))
	{
		return LTTRUE;
	}

#endif

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from an intersection...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(const IntersectInfo & iInfo)
{
	SurfaceType eType = ST_UNKNOWN;

	// Next get the flags from the poly...

	if (iInfo.m_hPoly != INVALID_HPOLY)
	{
		eType = GetSurfaceType(iInfo.m_hPoly);
	}

	// Try the object...

	if (eType == ST_UNKNOWN && iInfo.m_hObject)
	{
		HOBJECT hObj = iInfo.m_hObject;

		if ((LT_YES == g_pInterface->Physics()->IsWorldObject(hObj)) || 
			g_pInterface->GetObjectType(hObj) == OT_WORLDMODEL)
		{
			eType = (SurfaceType)iInfo.m_SurfaceFlags;
		}
		
		if (eType == ST_UNKNOWN)
		{
			eType = GetSurfaceType(iInfo.m_hObject);
		}
	}

	return eType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from a collision...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(const CollisionInfo & iInfo)
{
	SurfaceType eType = ST_UNKNOWN;

	// Next get the flags from the poly...

	if (iInfo.m_hPoly != INVALID_HPOLY)
	{
		eType = GetSurfaceType(iInfo.m_hPoly);
	}

	// Try the object...

	if (eType == ST_UNKNOWN && iInfo.m_hObject)
	{
		eType = GetSurfaceType(iInfo.m_hObject);
	}

	return eType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a poly
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPOLY hPoly)
{
	SurfaceType eType = ST_UNKNOWN;

	if (hPoly != INVALID_HPOLY)
	{
		// Get the flags associated with the poly...

		uint32 dwTextureFlags;
		g_pInterface->GetPolyTextureFlags(hPoly, &dwTextureFlags);

		eType = (SurfaceType)dwTextureFlags;
	}
	
	return eType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with an object
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HOBJECT hObj)
{
	if(!hObj) return ST_UNKNOWN;

	SurfaceType eType = ST_UNKNOWN;

	uint32 dwUserFlags;
#ifdef _CLIENTBUILD
	g_pInterface->GetObjectUserFlags(hObj, &dwUserFlags);
#else
	dwUserFlags = g_pInterface->GetObjectUserFlags(hObj);
#endif

	D_WORD code;

	if(LT_NO == g_pInterface->Physics()->IsWorldObject(hObj))
	{
		if(dwUserFlags & USRFLG_CHARACTER)
		{
			eType = UserFlagToSurface(dwUserFlags);
		}
		else if (g_pInterface->GetContainerCode(hObj, &code))
		{
			if (IsLiquid((ContainerCode)code))
			{
				eType = ST_LIQUID;
			}
		}
		else 
		{
			eType = UserFlagToSurface(dwUserFlags);
		}
	}

	return eType;
}


