// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.h
//
// PURPOSE : Weapon class - definition
//
// CREATED : 9/25/97
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "basedefs_de.h"
#include "WeaponMgr.h"
#include "serverobj_de.h"

#include "ltsmartlink.h"
#include "ModelButeMgr.h"


class CWeapons;
class CharacterAnimation;
class CCharacter;

#define W_SOUND_RADIUS	2000.0f
#define W_MAX_MODS		4

struct WFireInfo
{
	WFireInfo()
	{
		hFiredFrom = DNULL;
		hTestObj = DNULL;
		vPath.Init();
		vFirePos.Init();
		vFlashPos.Init();
		nFlashSocket = 0;
		nSeed = 0;
		bAltFire = DFALSE;
		fPerturbU = 1.0f;
		fPerturbR = 1.0f;
		fDamageMultiplier = 1.0f;
		eNodeHit = eModelNodeInvalid;
		bIgnoreImpactSound = LTFALSE;
	}

	HOBJECT		hFiredFrom;  // object firing weapon
	HOBJECT		hTestObj;    // object we're trying to hit
	DVector		vPath;       // vector from fire pos to target
	DVector		vFirePos;    // fire pos, projectile starts here
	DVector		vFlashPos;   // pos for muzzle flash
	DBYTE		nFlashSocket; // the socket offset into Barrel's FlashSockets array.
	DBYTE		nSeed;       // random seed to sync client and server side perturbations (this is crap!)
	DBOOL		bAltFire;    // use alternate fire?
	DFLOAT		fPerturbU;   // upward perturbation factor    (0.0 - 1.0)
	DFLOAT		fPerturbR;   // rightward perturbation factor (0.0 - 1.0)
	DFLOAT		fDamageMultiplier; // Over-all multiplier for damage (used by AI's).
	ModelNode	eNodeHit;	// The node claimed hit, multiplayer only
	DBOOL		bIgnoreImpactSound;    // Should we not make an impact sound?
};

class CWeapon
{
	public :


		CWeapon(CWeapons* pParent, HOBJECT hObj, int nWeaponId, int nBarrelId, int nAmmoId);
		~CWeapon() {}

		CWeapons* GetParent() const { return m_pParent; }
		void SetParent(CWeapons* pParent) { m_pParent = pParent; }

		void ReloadClip(int nNewAmmo=-1);

		int		GetId()					const { return m_nWeaponId; }
		int		GetBarrelId()			const { return m_nBarrelId; }
		int		GetAmmoId()				const { return m_nAmmoId; }
		int		GetAmmoInClip()			const { return m_nAmmoInClip; }

		const WEAPON*	GetWeaponData()			const { return m_pWeaponData; }
		const AMMO*		GetAmmoData()			const { return m_pAmmoData; }
		const BARREL*	GetBarrelData()			const { return m_pBarrelData; }

		void	SetAmmoId(int nId)				{ m_nAmmoId = nId; m_pAmmoData = g_pWeaponMgr->GetAmmo(nId); }
		void	SetBarrelId(int nId)			{ m_nBarrelId = nId; m_pBarrelData = g_pWeaponMgr->GetBarrel(nId); }

		const LTSmartLink &  GetModelObject() const { return m_hModelObject; }
		void	SetModelObject(const LTSmartLink & hObj);

		const char * GetFlashSocketName() const;  // Gives the "current" flash socket.  Is alternated by shots.
		const char * GetFlashSocketName(uint32 n) const;  // Gives the n-th flash socket.
		uint8 GetNumFlashSockets() const;
		uint8 GetCurrentFlashSocket() const { return m_nCurrentFlashSocket; }

		void WriteClientInfo(HMESSAGEWRITE hMessage);

		DBOOL	Fire(const WFireInfo & info);
		void	HitFire(const WFireInfo & info, LTBOOL& bHidePVFX);

		void Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void Load(HMESSAGEREAD hRead, DBYTE nType);

		HOBJECT GetObject()		const { return m_hOwner; }

		void CacheFiles();

		void SetPlayedImpactSound() { m_bImpactSoundPlayed = LTTRUE; }

	protected : 

		DBOOL CreateProjectile(const DRotation & rRot,const WFireInfo & info);

	private :

		HOBJECT		m_hOwner;			// Who owns us
		LTSmartLink	m_hModelObject;		// Weapon Model

		DBOOL	m_bHave;				// Do we have this weapon
		LTBOOL	m_bImpactSoundPlayed;	// Did our impact sound play?
		int		m_nAmmoInClip;			// How much ammo is in our clip
		uint8   m_nCurrentFlashSocket;  // Our current flash socket.

		DFLOAT	m_fLifeTime;			// New LifeTime of projectile.

		uint32	m_nWeaponId;			// What kind of weapon are we, only used by PlayerObj
		uint32	m_nBarrelId;			// What kind of barrel do we use, only used by PlayerObj
		uint32	m_nAmmoId;				// What kind of ammo do we use, only used by PlayerObj


		CWeapons* m_pParent;			// Parent container...

		const WEAPON *	m_pWeaponData;			// Static weapon data
		const BARREL *	m_pBarrelData;		// Static primary barrel data
		const AMMO *	m_pAmmoData;			// Static ammo data

};


#endif // __WEAPON_H__
