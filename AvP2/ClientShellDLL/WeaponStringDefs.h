#ifndef __WEAPON_STRING_DEFS_H__
#define __WEAPON_STRING_DEFS_H__

#include "ClientRes.h"
#include "WeaponMgr.h"

#define	  WS_ERROR_STRING	"Error"

extern ClientDE *g_pClientDE;

inline char* GetWeaponString(DBYTE nWeaponId)
{
	if (!g_pClientDE || !g_pWeaponMgr) return WS_ERROR_STRING;

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return WS_ERROR_STRING;

	static char strWeapon[128];
	SAFE_STRCPY(strWeapon, g_pClientDE->GetStringData(pWeapon->hName));

	return strWeapon;
}

inline char* GetAmmoString(int nAmmoId)
{
	if (!g_pClientDE || !g_pWeaponMgr) return WS_ERROR_STRING;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return WS_ERROR_STRING;

	DDWORD nStringID = pAmmo->nNameId;

	HSTRING hStr = g_pClientDE->FormatString(nStringID);
	if (!hStr) return WS_ERROR_STRING;

	static char strAmmo[128];
	SAFE_STRCPY(strAmmo, g_pClientDE->GetStringData(hStr));
	g_pClientDE->FreeString(hStr);

	return strAmmo;
}

#endif // __WEAPON_STRING_DEFS_H__
