// ----------------------------------------------------------------------- //
//
// MODULE  : Weapons.h
//
// PURPOSE : Weapons container object - Definition
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPONS_H__
#define __WEAPONS_H__

#include "Weapon.h"
#include "Projectile.h"

class BaseClass;
class CServerDE;

// Use weapon's default ammo id.  NOTE: This shares the same namespace as 
// the WeaponMgr::m_AmmoList indexes: 0 - WeaponMgr::m_AmmoList->GetLength()).
// Also, 255 is reserved by the WeaponMgr...

#define AMMO_DEFAULT_ID			254
#define BARREL_DEFAULT_ID		254

void UpdateCharacterWeaponFX(CCharacter * pChar, const WEAPON * pWeaponData );

// Class Definition

// @class CWeapons Weapon container/management object
class CWeapons : public IAggregate
{
	public :

		CWeapons();
		~CWeapons();

		DBOOL Init(HOBJECT hCharacter, HOBJECT hWeaponModel = DNULL, int nNumWeapons = -1, int nNumAmmoPools = -1);

		void ObtainWeapon(DBYTE nWeapon, uint8 nBarrelId = BARREL_DEFAULT_ID, uint8 nAmmoPoolId = AMMO_DEFAULT_ID, 
						  int nDefaultAmmo = -1, DBOOL bNotifyClient=DFALSE, LTBOOL bClientHudNotify=LTTRUE);	

		DBOOL ChangeWeapon(DBYTE nNewWeapon);

		void DeselectCurWeapon();

		int		AddAmmo(int nAmmoId, int nAmmo); //returns amount of ammo actually added
		int		AddAmmoToPool(int nAmmoPoolId, int nAmount);
		DBOOL SetAmmo(int nAmmoPoolId, int nAmmo=-1);
		DBOOL SetAmmoPool(const char * szPoolName, int nAmmo=-1);

		void DecrementAmmo(int nAmmoId, int nAmount=1);
		void DecrementAmmoPool(int nAmmoPoolId, int nAmount=1);
		int GetAmmoCount(int nAmmoId);
		int GetAmmoPoolCount(int nAmmoPoolId);
		int GetWeaponAmmoCount(int nWeaponId);

		void SetInfinitePool(int nAmmoPoolId, LTBOOL bIsInfinite = LTTRUE);
		LTBOOL IsPoolInfinite(int nAmmoPoolId);


		int GetCurWeaponId() const { return m_nCurWeapon; }

		CWeapon* GetCurWeapon();
		CWeapon* GetWeapon(DBYTE nWeaponId);

		DBOOL IsValidIndex(DBYTE nWeaponId);
		DBOOL IsValidWeapon(DBYTE nWeaponId);

		DBOOL IsValidAmmoId(int nAmmoId);
		DBOOL IsValidAmmoPoolId(int nAmmoPoolId);

		void Reset();

		CProjectile*  GetVecProjectile() { return &m_VecProjectile; }

		DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		void Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void Load(HMESSAGEREAD hRead, DBYTE nType);
		void FillAllClips();
		void FillClip(DBYTE nWeaponId);
		HOBJECT	GetCharacter() { return m_hCharacter; }
		LTBOOL HasWeapon(uint8 nWeapId);

	protected :

		DBOOL AddWeapon(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);
		DBOOL HandleAmmoPickup(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead);

	private :

		void DeleteAllocations();

		HOBJECT			m_hCharacter;		// The character
		HOBJECT			m_hWeaponModel;	// The hand held weapon model
		int				m_nCurWeapon;   // Current weapon index

		CWeapon**		m_pWeapons;
		int*			m_pAmmoCount;
		LTBOOL*			m_pInfinitePool;

		CProjectile		m_VecProjectile; // Projectile class used with vector weapons

};

inline DBOOL CWeapons::IsValidAmmoId(int nAmmoId)
{
	if (!m_pAmmoCount || !g_pWeaponMgr->IsValidAmmoType(nAmmoId)) return DFALSE;

	return DTRUE;
}

inline DBOOL CWeapons::IsValidAmmoPoolId(int nAmmoPoolId)
{
	if (!m_pAmmoCount || !g_pWeaponMgr->IsValidAmmoPoolType(nAmmoPoolId)) return DFALSE;

	return DTRUE;
}

inline CWeapon* CWeapons::GetCurWeapon()
{ 
	CWeapon* pRet = DNULL;
	if (IsValidWeapon(m_nCurWeapon))
	{
		pRet = m_pWeapons[m_nCurWeapon];
	}
	return pRet;
}

inline DBOOL CWeapons::IsValidIndex(DBYTE nWeaponId)
{
	return g_pWeaponMgr->IsValidWeapon(nWeaponId);
}

inline DBOOL CWeapons::IsValidWeapon(DBYTE nWeaponId)
{
	if(m_pWeapons && IsValidIndex(nWeaponId) && m_pWeapons[nWeaponId] ) 
		return DTRUE;

	return DFALSE;
}

inline CWeapon* CWeapons::GetWeapon(DBYTE nWeaponId)
{
	CWeapon* pRet = DNULL;

	if (IsValidWeapon(nWeaponId))
	{
		pRet = m_pWeapons[nWeaponId];
	}

	return pRet;
}



inline void CWeapons::SetInfinitePool(int nAmmoPoolId, LTBOOL bIsInfinite /*= LTTRUE*/ )
{
	if (IsValidAmmoPoolId(nAmmoPoolId)) m_pInfinitePool[nAmmoPoolId] = bIsInfinite;
}

inline LTBOOL CWeapons::IsPoolInfinite(int nAmmoPoolId)
{
	if (IsValidAmmoPoolId(nAmmoPoolId)) return m_pInfinitePool[nAmmoPoolId];

	return LTFALSE;
}

inline LTBOOL CWeapons::HasWeapon(uint8 nWeapId)
{ 
	return nWeapId < g_pWeaponMgr->GetNumWeapons() ? (m_pWeapons[nWeapId] != LTNULL) : LTFALSE; 
}


#endif //__WEAPONS_H__
