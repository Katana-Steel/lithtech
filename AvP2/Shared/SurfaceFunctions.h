// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceFunctions.h
//
// PURPOSE : Definition of shared surface functions
//
// CREATED : 4/16/98
//
// ----------------------------------------------------------------------- //

#ifndef __SURFACE_FUNCTIONS_H__
#define __SURFACE_FUNCTIONS_H__

#include "basedefs_de.h"
#include "SurfaceMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsMark
//
//	PURPOSE:	Does this type of surface show marks
//
// ----------------------------------------------------------------------- //

LTBOOL ShowsMark(SurfaceType eSurfType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShowsFootprints
//
//	PURPOSE:	Does this type of surface show footprints
//
// ----------------------------------------------------------------------- //

LTBOOL ShowsFootprints(SurfaceType eSurfType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSprite()
//
//	PURPOSE:	Get impact sprite with this surface
//
// ----------------------------------------------------------------------- //

LTBOOL GetImpactSprite(SurfaceType eSurfType, LTFLOAT & fScale, int nAmmoId,
					  char* pBuf, int nBufLen);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagToSurface()
//
//	PURPOSE:	Convert a user flag to a surface type
//
// ----------------------------------------------------------------------- //

SurfaceType UserFlagToSurface(uint32 dwUserFlag);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SurfaceToUserFlag()
//
//	PURPOSE:	Convert surface type to a user flag
//
// ----------------------------------------------------------------------- //

uint32 SurfaceToUserFlag(SurfaceType eSurfType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UserFlagSurfaceMask()
//
//	PURPOSE:	Masks out any surface type from the user flags
//
// ----------------------------------------------------------------------- //

uint32 UserFlagSurfaceMask(uint32 dwUserFlag);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsFleshSurfaceType()
//
//	PURPOSE:	Is this a type of flesh surface type
//
// ----------------------------------------------------------------------- //

LTBOOL IsFleshSurfaceType(SurfaceType eSurfType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetFootStepSound()
//
//	PURPOSE:	Get foot step sounds associated with this surface
//
// ----------------------------------------------------------------------- //

char* GetFootStepSound(SurfaceType eSurfType, LTBOOL bLeftFoot);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetImpactSound()
//
//	PURPOSE:	Get impact sounds associated with this surface
//
// ----------------------------------------------------------------------- //

char* GetImpactSound(SurfaceType eSurfType, int nAmmoId);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsMoveable()
//
//	PURPOSE:	Determine if the passed in object is moveable
//
// ----------------------------------------------------------------------- //

LTBOOL IsMoveable(HOBJECT hObj);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CanMarkObject()
//
//	PURPOSE:	Determine if the passed in object can be marked
//
// ----------------------------------------------------------------------- //

LTBOOL CanMarkObject(HOBJECT hObj);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from an intersection...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(const IntersectInfo & iInfo);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with the info
//				returned from a collision...
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(const CollisionInfo & iInfo);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with a poly
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HPOLY hPoly);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetSurfaceType()
//
//	PURPOSE:	Determine the type of surface associated with an object
//
// ----------------------------------------------------------------------- //

SurfaceType GetSurfaceType(HOBJECT hObj);

#endif // __SURFACE_FUNCTIONS_H__
