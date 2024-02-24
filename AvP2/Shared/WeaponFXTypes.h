// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFXTypes.h
//
// PURPOSE : Weapon FX types contains the different types and enums associated
//			 with the weapon fx class.
//
// CREATED : 2/22/98
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_FX_TYPES_H__
#define __WEAPON_FX_TYPES_H__

// Weapon fx (impact and fire)...

#define	WFX_MARK		(1<<0)

#define WFX_TINTSCREEN	(1<<2)
#define WFX_BLASTMARK	(1<<3)

#define	WFX_TRACER				(1<<4)
#define WFX_MUZZLE				(1<<5)
#define WFX_SHELL				(1<<6)
#define WFX_LIGHT				(1<<7)
#define WFX_FIRESOUND			(1<<8)
#define WFX_EXITMARK			(1<<9)
#define WFX_EXITDEBRIS			(1<<10)
#define WFX_ALTFIRESND			(1<<11)
#define WFX_IMPACTONSKY			(1<<12)
#define WFX_IMPACTONFLESH		(1<<13)
#define WFX_MUZZLEONLY			(1<<13)
#define WFX_IGNORE_IMPACT_SOUND	(1<<14)

#define WFX_EXITSURFACE (WFX_EXITMARK | WFX_EXITDEBRIS)

#define WFX_3RDPERSONONLY	(1<<15)		// Used in ExplosionFX, plays in 3rd person only, not 1st

// Projectile FX...

#define	PFX_PARTICLETRAIL	(1<<0)
#define	PFX_FLARE			(1<<1)
#define PFX_LIGHT			(1<<2)
#define PFX_FLYSOUND		(1<<3)
#define PFX_SCALEFX			(1<<4)

// Particle trail types...
// Bits 0-6 types

#define PT_SMOKE		(1<<0)
#define PT_BLOOD		(1<<1)
#define PT_GIBSMOKE		(1<<2)
#define PT_SPARKS		(1<<3)

#endif // __WEAPON_FX_TYPES_H__
