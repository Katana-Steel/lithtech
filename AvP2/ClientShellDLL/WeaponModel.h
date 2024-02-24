// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponModel.h
//
// PURPOSE : Generic client-side WeaponModel wrapper class - Definition
//
// CREATED : 9/27/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_MODEL_H__
#define __WEAPON_MODEL_H__

#include "cpp_clientshell_de.h"
#include "SurfaceMgr.h"
#include "WeaponMgr.h"
#include "LaserBeam.h"
#include "PVFXMgr.h"
#include "ModelDefs.h"

class CCharacterFX;
class CMuzzleFlashFX;

#define WM_MAX_FIRE_ANIS		3
#define WM_MAX_ALTFIRE_ANIS		3
#define WM_MAX_IDLE_ANIS		3
#define WM_MAX_ALTIDLE_ANIS		3

enum FireType
{
	FT_INVALID=-1,
	FT_NORMAL_FIRE=0,
	FT_ALT_FIRE,
};

enum AmmoStatus
{
	AS_INVALID=-1,
	AS_HAS_AMMO=0,
	AS_INFINITE,
	AS_CLIP_EMPTY,
	AS_OUT_OF_AMMO,
	AS_NEXT_BARREL,
};

class CWeaponModel
{
	public :

		CWeaponModel();
		virtual ~CWeaponModel();

		LTBOOL Init();
		LTBOOL Create(ILTClient* pClient, uint8 nWeaponId, LTBOOL bPlaySelSound);
		void ChangeWeapon(uint8 nCommandId, LTBOOL bNoDeselect= LTFALSE);
		void ForceChangeWeapon(uint8 nWeaponId, LTBOOL bDeselect=LTTRUE);
		void ChangeToPrevWeapon();
		void ChangeToNextBestWeapon(LTBOOL bNoDeselect=LTFALSE);	

		void ReloadClip(LTBOOL bPlayReload=LTTRUE, int nNewAmmo=-1, LTBOOL bForce=LTFALSE);
		LTVector GetShellEjectPos(LTVector & vOriginalPos);

		WeaponState UpdateWeaponModel(LTRotation rRot, LTVector vPos, LTBOOL bFire, FireType eFireType=FT_NORMAL_FIRE, LTBOOL bForceIdle = LTFALSE);

		void HandleStateChange(HMESSAGEREAD hMessage);

		void Reset();
		HLOCALOBJ GetHandle() const { return m_hObject; }


        void UpdateBob(LTFLOAT fWidth, LTFLOAT fHeight);

		void	UpdateMovementPerturb();
		LTFLOAT	GetMovementPerturb() {return m_fMovementPerturb;}

		void SetVisible(LTBOOL bVis=LTTRUE);
		LTBOOL IsVisible();

		void KillLoopSound();

		LTVector GetFlashPos() const { return m_vFlashPos; }
		LTVector GetModelPos() const;

		void SetWeaponOffset(LTVector v);
		void SetMuzzleOffset(LTVector v);

		LTVector GetWeaponOffset();
		LTVector GetMuzzleOffset();

		int GetWeaponId()	const { return m_nWeaponId; }
		int GetAmmoId()		const { return m_nAmmoId; }
		int GetAmmoInClip(BARREL* pBarrel);
		int GetBarrelId()	const { return m_nBarrelId; }
		int GetAmmoPerClip() const { if (m_pBarrel) return m_pBarrel->nShotsPerClip; else return 0;}

		WEAPON* GetWeapon() const { return m_pWeapon; }
		AMMO*	GetAmmo()	const { return m_pAmmo; }
		BARREL* GetBarrel() const { return m_pBarrel; }
		BARREL* GetPrimBarrel() const { return m_pPrimBarrel; }
		BARREL* GetAltBarrel()	const { return m_pAltBarrel; }
		WEAPON* GetRequestedWeapon() const { return g_pWeaponMgr->GetWeapon(m_nRequestedWeaponId); }

		uint8	PrevWeapon(LTBOOL bNoGarbage=LTFALSE);
		uint8	NextWeapon(LTBOOL bNoGarbage=LTFALSE);
		void	LastWeapon(LTBOOL bNoDeselect=LTFALSE);

		void	OnModelKey(HLOCALOBJ hObj, ArgList* pArgs);

		void	OnFireButtonUp();
		void	OnAltFireButtonUp();
		void	OnAltFireButtonDown();
		int		WeaponCounterType(){ return m_pWeapon?m_pWeapon->nCounterType:0; }
		AmmoStatus	StatusAmmo(BARREL* pBarrel);
	
		void Cheat() { UpdateGLHideStatus(LTTRUE); UpdateExoHideStatus(LTTRUE); }
		void ResetModelSkins();

		LTBOOL IsOutOfAmmoOrReloading();

		WeaponState GetState() {return m_eState; }			// What are we currently doing
		

	private : 

		void CreateModel();
		void CreateFlash();
		void RemoveModel();
		void UpdateFlash(WeaponState eState);
		
		void CreateAttachments();
		void RemoveAttachments();
		void UpdateAttachments();

		void UpdateZooming();

		WeaponState Fire(LTBOOL bUpdateAmmo=LTTRUE);
		WeaponState UpdateModelState(LTBOOL bFire);

		void	SendFireMsg();
		void	UpdateFiring();
		void	UpdateNonFiring();
		void	ResetWeaponData();

		void	InitAnimations();

		LTBOOL	PlaySelectAnimation();
		LTBOOL	PlayDeselectAnimation();
		LTBOOL	PlayFireAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayChargeFireAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayReloadAnimation();
		LTBOOL	PlayIdleAnimation();
		LTBOOL	PlayChargeAnimation(LTBOOL bResetAni=LTTRUE, LTBOOL bLooping=LTFALSE);
		LTBOOL	PlayPreChargeAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayStartFireAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayEndFireAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlaySpinupAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayStartSpinupAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayEndSpinupAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayAmmoChangeAnimation();
		LTBOOL	PlayStartZoomAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayEndZoomAnimation(LTBOOL bResetAni=LTTRUE);
		LTBOOL	PlayDetonateAnimation(LTBOOL bResetAni=LTTRUE);
		
		void	Deselect(LTBOOL bNoDeselect=LTFALSE);
		void	Select();

		HLOCALOBJ CreateServerObj();

		void	AddImpact(HLOCALOBJ hObj, LTVector & vInpactPoint, 
						  LTVector & vNormal, SurfaceType eType);
		void	HandleVectorImpact(IntersectQuery & qInfo, IntersectInfo & iInfo);
		void	HandleGadgetImpact(HOBJECT hObj);

//		LTBOOL	GetFirstAvailableAmmoType(int & nAmmoType);
		void	DecrementAmmo();

		void	HandleInternalWeaponChange(uint8 nWeaponId);
		void	DoWeaponChange(uint8 nWeaponId);

//		void	BuildTargetingSpriteUpdate(HMESSAGEWRITE hMessage);

		LTBOOL	CanChangeToWeapon(uint8 nCommandId, LTBOOL bForceNextWep);

		HOBJECT CreateModelObject(HOBJECT hOldObj, ObjectCreateStruct & createStruct);

		LTFLOAT	GetNextIdleTime();

		LTBOOL	IsFireAni(uint32 dwAni);
		uint32	GetFireAni(FireType eFireType=FT_NORMAL_FIRE);

		LTBOOL	IsChargeFireAni(uint32 dwAni);

		LTBOOL	IsIdleAni(uint32 dwAni);
		uint32	GetIdleAni();
		uint32	GetSubtleIdleAni();

		LTBOOL	IsReloadAni(uint32 dwAni);
		uint32	GetReloadAni();

		LTBOOL	IsSelectAni(uint32 dwAni);
		uint32	GetSelectAni();

		LTBOOL	IsDeselectAni(uint32 dwAni);
		uint32	GetDeselectAni();

		LTBOOL	CanUseAltFireAnis();

		LTBOOL	IsChargeAni(uint32 dwAni);
		LTBOOL	IsPreChargeAni(uint32 dwAni);

		LTBOOL	IsStartFireAni(uint32 dwAni);
		LTBOOL	IsEndFireAni(uint32 dwAni);
		
		LTBOOL	IsSpinupAni(uint32 dwAni);

		LTBOOL	IsStartSpinupAni(uint32 dwAni);
		LTBOOL	IsEndSpinupAni(uint32 dwAni);

		LTBOOL	IsStartZoomAni(uint32 dwAni);
		LTBOOL	IsEndZoomAni(uint32 dwAni);

		LTBOOL	IsAltStartFireAni(uint32 dwAni);
		LTBOOL	IsAltEndFireAni(uint32 dwAni);

		LTBOOL	IsAmmoChangeAni(uint32 dwAni);
		uint32	GetAmmoChangeAni() { return m_nAmmoChangeAni; }

		LTBOOL	IsDetonateAni(uint32 dwAni);

		uint8	GetLastSndFireType();

		LTBOOL	GetFireInfo(LTVector & vU, LTVector & vR, LTVector & vF, 
			LTVector & vFirePos);

		void	UpdateExoHideStatus(LTBOOL bShowAll=LTFALSE);
		void	UpdateGLHideStatus(LTBOOL bShowAll=LTFALSE);

		void	UpdateWeaponSpeedMod();
		
		LTBOOL	ClientHitDetection(WeaponPath& wp, LTVector& vFirePos);
		LTBOOL	HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir, CCharacterFX* pCharFX, ModelNode* peModelNode);
		
		HOBJECT			m_hObject;			// Handle of WeaponModel model
		
		HMODELSOCKET	m_hBreachSocket;	// Handle of breach socket

		HOBJECT			m_hGlassModel;		// Handle of glass model

		HMODELSOCKET	m_hGlassSocket;		// Handle of glass socket

		CLaserBeam		m_LaserBeam;		// Laser beam

		CPVFXMgr		m_PVFXMgr;			// Player-view fx mgr

		int			m_nLastWeaponId;

		int			m_nWeaponId;
		int			m_nAmmoId;
		int			m_nBarrelId;

		WEAPON*		m_pWeapon;			//Current Weapon
		BARREL*		m_pPrimBarrel;		//Primary Barrel
		BARREL*		m_pAltBarrel;		//Alternate Barrel
		BARREL*		m_pBarrel;			//Active Barrel
		BARREL*		m_pOldBarrel;		//For switching weapons
		AMMO*		m_pAmmo;			//Current Ammo

		LTVector	m_vFlashPos;
		LTVector	m_vFlashOffset;
		LTFLOAT		m_fFlashStartTime;	// When did flash start

		CMuzzleFlashFX * m_pMuzzleFlash;

		LTFLOAT		m_fBobHeight;
		LTFLOAT		m_fBobWidth;

		LTFLOAT		m_fMovementPerturb;

		FireType	m_eLastFireType;	// How did we last fire
		LTBOOL		m_bCanSetLastFire;	// Can we set m_eLastFireType

		LTFLOAT		m_fNextIdleTime;
		LTBOOL		m_bFire;

		WeaponState m_eState;			// What are we currently doing
		WeaponState	m_eLastWeaponState;

		HMODELANIM	m_nSelectAni;		// Select weapon
		HMODELANIM	m_nDeselectAni;		// Deselect weapon
		HMODELANIM	m_nReloadAni;		// Reload weapon
		HMODELANIM	m_nStillAni;			// Still animation
		HMODELANIM	m_nAmmoChangeAni;			// Still animation

		HMODELANIM	m_nIdleAnis[WM_MAX_IDLE_ANIS];	// Idle animations
		HMODELANIM	m_nFireAnis[WM_MAX_FIRE_ANIS];	// Fire animations

		HMODELANIM	m_nChargeFireAni;	// Charge Fire anim

		HMODELANIM	m_nAltSelectAni;		// Alt-Fire Select weapon
		HMODELANIM	m_nAltDeselectAni;		// Alt-Fire Deselect weapon (back to normal weapon)
		HMODELANIM	m_nAltDeselect2Ani;		// Alt-Fire Deselect weapon (to new weapon)
		HMODELANIM	m_nAltReloadAni;		// Alt-Fire Reload weapon

		HMODELANIM	m_nAltIdleAnis[WM_MAX_ALTIDLE_ANIS];	// Alt-Fire Idle animations
		HMODELANIM	m_nAltFireAnis[WM_MAX_ALTFIRE_ANIS];	// Alt-Fire animations

		HMODELANIM	m_nChargingAnim;	//charging animation
		HMODELANIM	m_nPreChargingAnim;	//pre-charging animation
		HMODELANIM	m_nStartFireAnim;	//start fire animation
		HMODELANIM	m_nEndFireAnim;		//end fire animation

		HMODELANIM	m_nSpinupAnim;		//looping spin-up animation
		HMODELANIM	m_nSpinupStartAnim;	//start of spin-up animation
		HMODELANIM	m_nSpinupEndAnim;	//end of spin-up animation

		HMODELANIM	m_nAltStartFireAnim;	//charging animation
		HMODELANIM	m_nAltEndFireAnim;	//charging animation

		HMODELANIM	m_nStartZoomAnim;	//starting to zoom animation
		HMODELANIM	m_nEndZoomAnim;		//ending zoom animation

		HMODELANIM	m_nDetonateAnim;	//detonate animation

		LTBOOL		m_bUsingAltFireAnis;		// Use the m_nAltXXX anis?
		LTBOOL		m_bFireKeyDownLastUpdate;	// Was the fire key down last update?

		LTVector		m_vPath;		// Path of current vector/projectile
		LTVector		m_vFirePos;		// Fire position of current vector/projectile
		LTVector		m_vEndPos;		// Impact location of current vector
		D_WORD			m_wIgnoreFX;	// FX to ignore for current vector/projectile
		AmmoType		m_eAmmoType;	// Type of ammo

		uint8			m_nRequestedWeaponId;	// Id of weapon to select
		LTBOOL			m_bWeaponDeselected;	// Did we just deselect the weapon
		LTBOOL			NextPrimBarrel(BARREL* pCurBarrel);
		AmmoStatus		AmmoCheck(WEAPON *pWeapon=LTNULL);
		LTFLOAT			m_fBarrelSwitchStartTime;
		LTFLOAT			m_fCurZoomLevel;
		LTFLOAT			m_bZooming, m_bZoomed, m_bPreZooming;

		LTFLOAT			m_fLastFOVX, m_fLastFOVY;

		LTBOOL			m_bNewShot;
		HOBJECT			m_hHotbombTarget;
		LTBOOL			m_bAutoSpinOn;

		LTBOOL			m_bDelayedWeaponChange;
		uint8			m_nDealyedCommandId;

		LTBOOL			m_bAlienTail;
		LTBOOL			m_bPlaySelectSound;

		DBYTE			m_nLoopKeyId;
		HLTSOUND		m_hLoopFireSound;
};

#endif // __WEAPON_MODEL_H__