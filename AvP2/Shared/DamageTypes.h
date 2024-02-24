// ----------------------------------------------------------------------- //
//
// MODULE  : DamageTypes.h
//
// PURPOSE : Definition of damage types
//
// CREATED : 11/26/97
//
// ----------------------------------------------------------------------- //

#ifndef __DAMAGE_TYPES_H__
#define __DAMAGE_TYPES_H__

#include "basedefs_de.h"

// ----------------------------------------------------------------------- //
// IMPORTANT NOTE:  If you add a new DamageType, make sure you update
// the DamageTypeToFlag array in DamageTypes.cpp!!!

// *** = can be used as progressive damage

enum DamageType
{ 
	DT_INVALID = -1,	// Not a valid damage type
	DT_UNSPECIFIED = 0,	// Unknown type (self-damage)
	DT_BULLET,			// (bullets)
	DT_BURN,			// *** (fire, intense heat)
	DT_ACID,			// (acid, corrosives)
	DT_CHOKE,			// (water, hostile atmosphere)
	DT_CRUSH,			// (falling, crushing, collision)
	DT_ELECTRIC,		// *** (electricity)
	DT_EXPLODE,			// (explosions) 
	DT_FREEZE,			// (freezing air or fluid)
	DT_POISON,			// *** (poison gas/dart)
	DT_BLEEDING,		// *** (bleeding)
	DT_STUN,			// *** (stun)
	DT_BITE,			// (little bite attacks)
	DT_HEADBITE,		// (a special head bite attack damage)
	DT_ENDLESS_FALL,	// Falling and can never get up
	DT_ALIEN_CLAW,		// Alien claw damage, also heals when used on bodyprop
	DT_EMP,				// *** EMP effect damage
	DT_NAPALM,			// *** Napalm damage like from marine flame thrower
	DT_SPEAR,			// Spear projectile from predator
	DT_SLICE,			// Slicing damage required for limb loss
	DT_FACEHUG,			// Facehug damage
	DT_TORCH_CUT,		// Blowtorch damage
	DT_TORCH_WELD,		// Blowtorch damage
	DT_MARINE_HACKER,	// Marine hacking device damage
	DT_HIGHCALIBER,		// high caliber bullet
	DT_TEAR,			// Alien tear weapon
	DT_PRED_HOTBOMB,	// Predator Hotbomb weapon
	DT_SHOTGUN,			// Predator Hotbomb weapon
	DT_BULLET_NOGIB,	// Bullets that don't create GIBs

	DT_MAX,				// Leave this here for looping purposes
};

inline ILTMessage & operator<<(ILTMessage & out, DamageType & x)
{
	ASSERT( x >= DT_INVALID );
	ASSERT( x <= DT_MAX );

	out.WriteDWord(x);

	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, DamageType & x)
{
	x = DamageType(in.ReadDWord());

	return in;
}

// ----------------------------------------------------------------------- //

#define DAMAGETYPE_TINT_LIGHTADD		0
#define DAMAGETYPE_TINT_LIGHTSCALE		1

// ----------------------------------------------------------------------- //

const uint32 EMP_FLAG_STUN				= 1;
const uint32 EMP_FLAG_VISION_MODES		= 2;
const uint32 EMP_FLAG_MOTION_DETECTOR	= 4;
const uint32 EMP_FLAG_UNUSED1			= 16;
const uint32 EMP_FLAG_UNUSED2			= 32;
const uint32 EMP_FLAG_UNUSED3			= 64;
const uint32 EMP_FLAG_UNUSED4			= 128;
const uint32 EMP_FLAG_UNUSED5			= 256;
const uint32 EMP_FLAG_UNUSED6			= 512;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecificObjectFilterFn()
//
//	PURPOSE:	Filter a specific object out of CastRay and/or 
//				IntersectSegment calls.
//
// ----------------------------------------------------------------------- //

inline LTBOOL SpecificObjectFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj) return DFALSE;

	return (hObj != (HOBJECT)pUserData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsJarCameraType()
//
//	PURPOSE:	Does this damage type cause the camera to be "jarred"
//
// ----------------------------------------------------------------------- //

inline LTBOOL IsJarCameraType(DamageType eType)
{
	switch(eType)
	{
		case DT_BULLET:
		case DT_BULLET_NOGIB:
		case DT_HIGHCALIBER:
		case DT_EXPLODE:
		case DT_SHOTGUN:
			return LTTRUE;

		default:
			break;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetCameraTintColor()
//
//	PURPOSE:	Retrieve the tint color and tine type for this damage
//
// ----------------------------------------------------------------------- //

uint8 GetCameraTintColor(DamageType eType, LTVector &vTint);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToFlag()
//
//	PURPOSE:	Convert the damage type to its flag equivillent
//
// ----------------------------------------------------------------------- //

uint32 DamageTypeToFlag(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeHPPercent()
//
//	PURPOSE:	Get the damage percent to apply to health
//
// ----------------------------------------------------------------------- //

LTFLOAT DamageTypeHPPercent(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeAPPercent()
//
//	PURPOSE:	Get the damage percent to apply to armor
//
// ----------------------------------------------------------------------- //

LTFLOAT DamageTypeAPPercent(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeResistance()
//
//	PURPOSE:	Get the damage percent to apply to armor
//
// ----------------------------------------------------------------------- //

LTBOOL DamageTypeResistance(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeToName()
//
//	PURPOSE:	Convert the damage type to its string name
//
// ----------------------------------------------------------------------- //

const char* DamageTypeToName(DamageType eType);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageTypeFromName()
//
//	PURPOSE:	Convert the damage type name to its enum equivillent
//
// ----------------------------------------------------------------------- //

DamageType DamageTypeFromName(const char *szName);


#endif // __DAMAGE_TYPES_H__
