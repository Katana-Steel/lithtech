// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.h
//
// PURPOSE : Weapon special fx class - Definition
//
// CREATED : 2/22/98
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_FX_H__
#define __WEAPON_FX_H__

#include "SpecialFX.h"
#include "SurfaceMgr.h"
#include "WeaponMgr.h"
#include "TracerFX.h"
#include "SmokeFX.h"
#include "ContainerCodes.h"
#include "GameSettings.h"

struct WCREATESTRUCT : public SFXCREATESTRUCT
{
    WCREATESTRUCT();
	
	HOBJECT		hFiredFrom;
    uint8		nWeaponId;
	uint8		nBarrelId;
    uint8		nAmmoId;
    uint8		nSurfaceType;
	uint8		nFlashSocket;
    uint16		wIgnoreFX;
    LTVector	vFirePos;
    LTVector	vPos;
    LTVector	vSurfaceNormal;
    uint8		nShooterId;
    LTBOOL		bLocal;
};

inline WCREATESTRUCT::WCREATESTRUCT()
{
	vFirePos.Init();
	vPos.Init();
	vSurfaceNormal.Init();
    hFiredFrom      = LTNULL;
	nWeaponId		= 0;
	nAmmoId			= 0;
	nSurfaceType	= 0;
	nFlashSocket	= 0;
	wIgnoreFX		= 0;
    bLocal          = LTFALSE;
	nShooterId		= -1;
}


class CWeaponFX : public CSpecialFX
{
public :
	
	CWeaponFX() : CSpecialFX()
	{
		m_hFiredFrom    = LTNULL;

		m_nWeaponId		= WMGR_INVALID_ID;
		m_nAmmoId		= WMGR_INVALID_ID;
		m_nBarrelId		= WMGR_INVALID_ID;

		m_nFlashSocket  = 0;

		m_nShooterId	= -1;
		m_nLocalId		= -1;
		m_nDetailLevel	= RS_HIGH;

		m_eSurfaceType	= ST_UNKNOWN;
		m_eExitSurface	= ST_UNKNOWN;
		m_eExitCode		= CC_NO_CONTAINER;
		m_eCode			= CC_NO_CONTAINER;
		m_eFirePosCode	= CC_NO_CONTAINER;

		m_wImpactFX		= 0;
		m_wFireFX		= 0;
		m_wIgnoreFX		= 0;

		m_fInstDamage	= 0;
		m_fAreaDamage	= 0;
		m_fFireDistance	= 100.0f;

		m_bLocal        = LTFALSE;
		m_bLocalImpact	= LTFALSE;
		
		m_vFirePos.Init();
		m_vPos.Init();
		m_vDir.Init();
		m_vSurfaceNormal.Init();
		m_vExitPos.Init();
		m_vExitNormal.Init();
		m_vLightColor.Init();
		
		m_rSurfaceRot.Init();
		m_rDirRot.Init();
		
		m_pWeapon       = LTNULL;
		m_pAmmo         = LTNULL;
		m_pBarrel		= LTNULL;
	}
	
	virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
	virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
	virtual LTBOOL CreateObject(ILTClient* pClientDE);
	virtual LTBOOL Update() { return LTFALSE; }
	
protected :
	
	HOBJECT			m_hFiredFrom;		// Who fired the weapon

	int				m_nWeaponId;		// Id of weapon fired
	int				m_nBarrelId;		// Id of barrel fired
	int				m_nAmmoId;			// Type of ammo used

	uint8			m_nFlashSocket;     // flash socket from which we fired.

	uint8			m_nDetailLevel;     // Current detail level setting
	uint8			m_nShooterId;       // Client id of the shooter
	uint8			m_nLocalId;         // Local client id

	uint16			m_wFireFX;          // Fire FX to create
	uint16			m_wImpactFX;        // Impact FX to create
	uint16			m_wIgnoreFX;        // Fire FX to ignore

	LTFLOAT			m_fInstDamage;      // Instantaneous damage (vector)
	LTFLOAT			m_fAreaDamage;      // Area damage (explosion)
	LTFLOAT			m_fFireDistance;    // Distance from fire pos to pos

	LTVector		m_vFirePos;         // Position bullet was fired from
	LTVector		m_vPos;             // Impact pos
	LTVector		m_vExitPos;         // Bullet exit pos
	LTVector		m_vExitNormal;      // Exit surface normal
	LTVector		m_vDir;             // Direction from fire pos to pos
	LTVector		m_vSurfaceNormal;   // Normal of surface of impact
	LTVector		m_vLightColor;      // Impact light color
	
	ContainerCode	m_eCode;			// Container effect is in
	ContainerCode	m_eFirePosCode;		// Container fire pos is in
	ContainerCode	m_eExitCode;		// Container exit pos is in

	SurfaceType		m_eSurfaceType;		// Surface hit by bullet
	SurfaceType		m_eExitSurface;		// Surface bullet is exiting
	
	LTRotation		m_rSurfaceRot;      // Normal of surface (as rotation)
	LTRotation		m_rDirRot;          // Rotation based on m_vDir
	
	LTBOOL			m_bLocal;           // Is this a local fx (only done on this client?)
	LTBOOL			m_bLocalImpact;     // Is this an impact on the local player?
	
	WEAPON*			m_pWeapon;			// Weapon data
	BARREL*			m_pBarrel;			// Barrel data
	AMMO*			m_pAmmo;			// Ammo data
	
	void	SetupExitInfo();

	void	CreateExitMark();
	void	CreateExitDebris();
	void	CreateMark(LTVector vPos, LTVector vNorm, LTRotation rRot, SurfaceType eType);
	void	CreateTracer();
	void	CreateBulletTrail(LTVector* pvStartPos);
	void	CreateWeaponSpecificFX();
	void	CreateSurfaceSpecificFX();
	void	CreateLightBeamFX(SURFACE* pSurf);
	void	CreateMuzzleFX();
	void	CreateShell();
	void	CreateMuzzleLight();
	void	CreateLightFX();
	void	CreateVectorBloodFX();

	void	PlayImpactSound();
	void	PlayFireSound();
	void	PlayBulletFlyBySound();
	
	LTBOOL		IsBulletTrailWeapon();
	LTVector	CalcFirePos(LTVector vFirePos);
};

//-------------------------------------------------------------------------------------------------
// SFX_WeaponFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_WeaponFactory : public CSpecialFXFactory
{
	SFX_WeaponFactory() : CSpecialFXFactory(SFX_WEAPON_ID) {;}
	static const SFX_WeaponFactory registerThis;
	
	virtual CSpecialFX* MakeShape() const;
};

#endif // __WEAPON_FX_H__

