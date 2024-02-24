// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.h
//
// PURPOSE : ProjectileTypes class - definition
//
// CREATED : 10/3/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_TYPES_H__
#define __PROJECTILE_TYPES_H__

#include "Projectile.h"
#include "Timer.h"
#include "CharacterMovement.h"
#include "ContainerCodes.h"
#include "SurfaceDefs.h"

struct PROXCLASSDATA;
struct SPIDERCLASSDATA;
struct EMPCLASSDATA;
struct TRACKINGSADARCLASSDATA;

class BodyProp;

class CGrenade : public CProjectile
{
	public :
		
		CGrenade();
		~CGrenade();

	protected :

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void HandleTouch(HOBJECT hObj);
		virtual void UpdateGrenade();
		virtual void ResetRotationVel(LTVector* pvNewVel=LTNULL, SURFACE* pSurf=LTNULL);
		virtual void RotateToRest();

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		virtual void PostPropRead(ObjectCreateStruct *pStruct) {}

		LTBOOL	m_bSpinGrenade;		// Should the grenade spin
		LTBOOL	m_bUpdating;		// Are we updating the grenade

		ContainerCode m_eContainerCode;
		SurfaceType	  m_eLastHitSurface;

		LTFLOAT	m_fPitch;
		LTFLOAT	m_fYaw;
		LTFLOAT	m_fRoll;
		LTFLOAT	m_fPitchVel;
		LTFLOAT	m_fYawVel;
		LTFLOAT	m_fRollVel;

		// Don't need to save this...

		HSOUNDDE	m_hBounceSnd;	// Handle to current bounce sound
};


class CProxGrenade : public CGrenade
{
	public :

		CProxGrenade() : CGrenade() 
		{
			m_vSurfaceNormal.Init();
			m_bArmed	 = DFALSE;
			m_bActivated = DFALSE;
			m_pClassData = LTNULL;
			m_nBounceCount	= 0;
		}
	
		virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);

	protected :
		void Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact = LTTRUE);

		virtual void UpdateGrenade();
		virtual void HandleTouch(HOBJECT hObj);
		virtual void RotateToRest();

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTVector m_vSurfaceNormal;
		LTBOOL	m_bArmed;
		LTBOOL	m_bActivated;
		CTimer	m_DetonateTime;
		CTimer  m_ArmTime;
		uint8	m_nBounceCount;

		PROXCLASSDATA* m_pClassData;
};

class CSpiderGrenade : public CGrenade
{
	public :

		CSpiderGrenade() : CGrenade() 
		{
			m_vSurfaceNormal.Init();
			m_bArmed		= DFALSE;
			m_bActivated	= DFALSE;
			m_pClassData	= LTNULL;
			m_hTarget		= LTNULL;
			m_fLostTargetTime = LTFALSE;
			m_bLostTarget	= LTFALSE;
			m_bDEditPlaced	= LTFALSE;
			m_fTimeActivated= 0.0f;
			m_nBounceCount	= 0;
		}
	
		virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);

		virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	protected :

		virtual void UpdateGrenade();
		virtual void HandleTouch(HOBJECT hObj);
		virtual void RotateToRest();

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		LTBOOL	TestLOS(HOBJECT hTarget);
		void	Setup();

		void	Pursue();
		LTBOOL	CheckForDetonation();

		LTVector m_vSurfaceNormal;
		LTBOOL	m_bArmed;
		LTBOOL	m_bActivated;
		CTimer  m_ArmTime;
		LTSmartLink m_hTarget;
		LTFLOAT m_fLostTargetTime;
		LTBOOL	m_bLostTarget;
		LTBOOL	m_bDEditPlaced;
		LTFLOAT m_fTimeActivated;
		uint8	m_nBounceCount;

		CharacterMovement cm_CharacterMovement;

		SPIDERCLASSDATA* m_pClassData;
};

class CEMPGrenade : public CGrenade
{
	public :

		CEMPGrenade() : CGrenade() 
		{
			m_pClassData	= LTNULL;
		}
	
		virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);

	protected :

		virtual void HandleTouch(HOBJECT hObj);

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		EMPCLASSDATA* m_pClassData;
};

class CTrackingSADAR : public CProjectile
{
	public :

		CTrackingSADAR() : CProjectile() 
		{
			m_pClassData	= LTNULL;
			m_hTarget		= LTNULL;
		}
	
		virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);
		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	protected :

		virtual void Update();

		LTBOOL	TestLOS(HOBJECT hTarget);

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTSmartLink m_hTarget;

		TRACKINGSADARCLASSDATA* m_pClassData;
};


class CShoulderCannon : public CProjectile
{
	public :

		CShoulderCannon() : CProjectile() 
		{
			m_hTarget		= LTNULL;
		}
	
		virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);
		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

	protected :

		virtual void Update();

		LTBOOL	TestLOS(HOBJECT hTarget);

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTSmartLink m_hTarget;
};


class CSpear : public CProjectile
{
	public :

		CSpear();
		~CSpear();

	protected :

		virtual void HandleImpact(HOBJECT hObj);
		virtual void Update();

	private:
		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		void	HandlePostHeadShotImpact(HOBJECT hObj);
		LTBOOL	CreateHeadObject(CCharacter* pCharacter);

		LTSmartLink m_hHead;
};

class CFlare : public CGrenade
{
	public :

		CFlare();
		~CFlare();

	virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);

	protected:
		//overridden virtual functions
		void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void HandleTouch(HOBJECT hObj);
		void UpdateGrenade();

	private:
		//member functions
		void Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact = LTTRUE);

		//member variables
		LTFLOAT		m_fRotation;			//speed of our rotation in radian/sec
		uint8		m_nBounceCount;		// how many times we have bounced.
		LTBOOL		m_bSetPos;			// Do we need to reset our position?
		LTVector	m_vNewPos;			// Our new position.
};


class CStickyHotbomb : public CGrenade
{
	public :

		CStickyHotbomb() : CGrenade() 
		{
			m_vSurfaceNormal.Init();
			m_nBounceCount = 0;
		}
	
		void	TriggerDetonate(HOBJECT hObj);
		uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);

	protected :
		//member functions
		void Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact = LTTRUE);

		virtual void HandleTouch(HOBJECT hObj);
		virtual void RotateToRest();

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		LTVector	m_vSurfaceNormal;
		uint8		m_nBounceCount;

		PROXCLASSDATA* m_pClassData;
};

#endif //  __PROJECTILE_TYPES_H__
