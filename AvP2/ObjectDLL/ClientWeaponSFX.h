// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.h
//
// PURPOSE : CClientWeaponSFX - Definition
//
// CREATED : 2/22/98
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_WEAPON_SFX_H__
#define __CLIENT_WEAPON_SFX_H__

#include "cpp_server_de.h"
#include <memory.h>  // for memset

struct CLIENTWEAPONFX
{
	CLIENTWEAPONFX::CLIENTWEAPONFX();

	HOBJECT		hFiredFrom;
	HOBJECT		hObj;
	DBYTE		nWeaponId;
	DBYTE		nAmmoId;
	DBYTE		nBarrelId;
	DBYTE		nSurfaceType;
	DBYTE		nFlashSocket;
	D_WORD		wIgnoreFX;
	DBYTE		nShooterId;
	DVector		vFirePos;
	DVector		vPos;
	DVector		vSurfaceNormal;
};

inline CLIENTWEAPONFX::CLIENTWEAPONFX()
{
	memset(this, 0, sizeof(CLIENTWEAPONFX));
	nShooterId = -1;
}

		
void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct);


#endif // __CLIENT_WEAPON_SFX_H__
