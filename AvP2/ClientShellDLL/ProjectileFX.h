// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.h
//
// PURPOSE : Projectile special fx class - Definition
//
// CREATED : 7/6/98
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_FX_H__
#define __PROJECTILE_FX_H__

#include "SpecialFX.h"
#include "WeaponMgr.h"
#include "PolyLineFX.h"


struct PROJECTILECREATESTRUCT : public SFXCREATESTRUCT
{
	PROJECTILECREATESTRUCT::PROJECTILECREATESTRUCT();

	uint8	nWeaponId;
	uint8	nAmmoId;
	uint8	nShooterId;
	LTBOOL	bLocal;
	LTBOOL	bAltFire;
};

inline PROJECTILECREATESTRUCT::PROJECTILECREATESTRUCT()
{
	nShooterId	= -1;
	nWeaponId	= WMGR_INVALID_ID;
	nAmmoId		= WMGR_INVALID_ID;
	bLocal		= LTFALSE;
	bAltFire	= LTFALSE;
}


class CProjectileFX : public CSpecialFX
{
	public :

		CProjectileFX() : CSpecialFX() 
		{
			m_nWeaponId		= WMGR_INVALID_ID;
			m_nAmmoId		= WMGR_INVALID_ID;
			m_nFX			= 0;

			m_pToggleScaleFX= LTNULL;
			m_hFlare		= LTNULL;
			m_hLight		= LTNULL;
			m_hFlyingSound	= LTNULL;

			m_hScaleSocket	= INVALID_MODEL_SOCKET;

			m_nShooterId	= -1;
			m_bLocal		= LTFALSE;

			m_fStartTime	= 0.0f;
			m_bDetonated	= LTFALSE;
			m_bAltFire		= LTFALSE;

			m_pProjectileFX = LTNULL;
			m_pFlareFlame	= LTNULL;

			m_nFlickerPattern		= 1;
			m_fFlickerTime			= 0.0f;

			m_vFlareSpriteScale.Init();
		}

		~CProjectileFX()
		{
			RemoveFX(TRUE);
		}

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(CClientDE* pClientDE);
		virtual LTBOOL Update();

		void HandleTouch(CollisionInfo *pInfo, float forceMag);

	protected :
	
		uint8			m_nWeaponId;		// Id of weapon fired
		uint8			m_nAmmoId;			// Type of ammo fired
		uint8			m_nFX;				// FX associated with projectile
		uint8			m_nShooterId;		// Client Id of shooter
		LTBOOL			m_bLocal;			// Did local client create this fx
		LTBOOL			m_bAltFire;			// Alt-fire?

		int				m_nFlickerPattern;	// Current Light flicker pattern
		LTFLOAT			m_fFlickerTime;		// Current Light flicker time

		void CreateSmokeTrail(LTVector & vPos, LTRotation & rRot);
		void CreateFlare(LTVector & vPos, LTRotation & rRot);
		void CreateLight(LTVector & vPos, LTRotation & rRot);
		void CreateProjectile(LTVector & vPos, LTRotation & rRot);
		void CreateFlyingSound(LTVector & vPos, LTRotation & rRot);
		void CreateFlareFlame(LTVector const & vPos);
		void CreateToggleScaleFX();		
		void UpdateScaleFX();
		void UpdateFlareLight();

		void RemoveFX(LTBOOL bOnDetonate=LTFALSE);

		LTBOOL MoveServerObj();
//		void  Detonate(CollisionInfo* pInfo);

		CSpecialFX*			m_pFlareFlame;		// Flare Flame FX
		CSpecialFX*			m_pToggleScaleFX;	// Special Toggle Scale fx
		HOBJECT				m_hFlare;			// Flare fx
		HOBJECT				m_hLight;			// Light fx
		HSOUNDDE			m_hFlyingSound;		// Sound of the projectile

		HMODELSOCKET		m_hScaleSocket;		// The socket where our FX is located
		
		LTVector		m_vFlareSpriteScale;	// Only used with Flare

		LTFLOAT			m_fStartTime;

		LTVector		m_vFirePos;
		LTVector		m_vPath;

		LTBOOL			m_bDetonated;

		PROJECTILEFX*	m_pProjectileFX;
};

//-------------------------------------------------------------------------------------------------
// SFX_ProjectileFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ProjectileFactory : public CSpecialFXFactory
{
	SFX_ProjectileFactory() : CSpecialFXFactory(SFX_PROJECTILE_ID) {;}
	static const SFX_ProjectileFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __PROJECTILE_FX_H__