// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.h
//
// PURPOSE : Projectile class - definition
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "GameBase.h"
#include "DamageTypes.h"
#include "SurfaceMgr.h"
#include "WeaponMgr.h"
#include "Destructible.h"
#include "ModelDefs.h"
#include "Weapon.h"

struct PROJECTILECLASSDATA;
class BodyProp;
class CCharacter;

const LTFLOAT UPDATE_DELTA	= 0.05f;
const LTFLOAT MIN_VEL_SQR	= 100.0f;

struct ProjectileSetup
{
	explicit ProjectileSetup::ProjectileSetup( uint32 weapon_id = 0, uint32 barrel_id = 0, int32 ammo_id = 0, LTFLOAT life_time = 0.0f)
		: nWeaponId(weapon_id),
		  nBarrelId(barrel_id),
		  nAmmoId(ammo_id),
		  fLifeTime(life_time)	{ }

	uint32	nWeaponId;
	uint32	nAmmoId;
	uint32	nBarrelId;
	LTFLOAT	fLifeTime;
};

class CProjectile : public GameBase
{
	public :

		CProjectile(uint8 nType = OT_MODEL, uint32 add_flags = 0, uint32 remove_flags = 0, uint32 add_flags2 = 0, uint32 remove_flags2 = 0 );
		
		virtual void Setup(const ProjectileSetup & projectile_setup, const WFireInfo & info);

		void Setup(CWeapon* pWeapon, const WFireInfo & info);  // This is just a wrapper for setup with ProjectileSetup.
		void ForcedHitSetup(const CWeapon &  weapon, const WFireInfo & info);  // This is just a wrapper for setup with ProjectileSetup.

		// These are used by CCharacterHitBox.
		void  AdjustDamage(const CCharacter & damagee, ModelNode eModelNode);
		void  AdjustDamage(const BodyProp & damagee, ModelNode eModelNode);

		LTFLOAT			GetInstantDamage()		const { return m_fInstDamage*m_fDamageMultiplier; }

		// Used by Character for pred-disks.
		const LTSmartLink &	GetFiredFrom()			const { return m_hFiredFrom; }


	protected :

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		void  OnModelKey(ArgList* pArgs);

		virtual void HandleTouch(HOBJECT hObj);

		virtual void AddImpact(	HOBJECT hObj, 
								LTVector vFirePos, 
								LTVector vImpactPos, 
								LTVector vNormal, 
								SurfaceType eSurfaceType=ST_UNKNOWN );

		virtual void AddExplosion(LTVector vPos, LTVector vNormal);

		virtual void AddProjectileMuzzleFX(LTVector vFirePos);

		virtual void AddSpecialFX();
		virtual void RemoveObject();

		virtual void HandleImpact(HOBJECT hObj);
		virtual void ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj);

		virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		
		virtual void Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact = LTTRUE);
		virtual void Expire() { Detonate(LTNULL); }

		virtual void Update();

		void		SetDims(const LTVector & new_dims)	{ m_vDims = new_dims; }


		PROJECTILECLASSDATA* GetAmmoClassData();
		const char * GetAmmoName() const   { if( m_pAmmoData ) return m_pAmmoData->szName; return ""; }
		LTVector     GetAmmoScale() const;

		LTFLOAT			GetStartTime()			const { return m_fStartTime; }
		LTFLOAT			GetLifeTime()			const { return m_fLifeTime; }
		DamageType		GetInstantDamageType()	const { return m_eInstDamageType; } 
		LTVector		GetDir()				const { return m_vDir; }
		LTFLOAT			GetVelocity()			const { return m_fVelocity; }
		void			SetCanDamage(LTBOOL b)	{ m_damage.SetCanDamage(b); }

	private :

		void HandleFXKey(ArgList* pArgList);

		void InitialUpdate(int nInfo);

		void DoProjectile();
		void DoVector();
		void DoForcedHit(HOBJECT hVictim, ModelNode eNodeHit);

		LTBOOL TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType);

		LTBOOL	HandleVectorImpact(const IntersectInfo & iInfo, LTVector * pvFrom, LTVector * pvTo);
		LTBOOL	HandlePotentialHitBoxImpact(IntersectInfo * piInfo, LTVector * pvFrom, LTVector* pvNodePos);
		void	HandleForcedImpact( HOBJECT hVictim, LTVector& vNodePos);

		LTBOOL UpdateDoVectorValues(const SURFACE & surf, LTFLOAT fThickness, LTVector vImpactPos, 
								   LTVector * pvFrom = LTNULL, LTVector * pvTo = LTNULL);

		LTBOOL CalcInvisibleImpact(IntersectInfo * piInfo, SurfaceType * peSurfType = LTNULL);


	private :
		
		LTVector		m_vFlashPos;			// Where the fired from special fx should be created 
		int				m_nFlashSocket;			// The socket number to use from the barrel's FlashSockets array.
		LTVector		m_vFirePos;				// Where were we fired from
		LTVector		m_vDir;					// What direction our we moving

		LTBOOL			m_bObjectRemoved;		// Have we been removed?
		LTBOOL			m_bDetonated;			// Have we detonated yet?

		int				m_nWeaponId;			// Type of weapon
		int				m_nAmmoId;				// Type of ammo
		int				m_nBarrelId;			// Type of barrel
		DamageType		m_eInstDamageType;		// Instant damage type - DT_XXX
		DamageType		m_eProgDamageType;		// Progressive damage type - DT_XXX

		LTFLOAT			m_fStartTime;			// When did we start
		LTFLOAT			m_fInstDamage;			// How much instant damage do we do
		LTFLOAT			m_fProgDamage;			// How much progressive damage do we do
		LTFLOAT			m_fLifeTime;			// How long do we stay around
		LTFLOAT			m_fVelocity;			// What is our velocity
		LTFLOAT			m_fRange;				// Vector weapon range
		LTFLOAT			m_fDamageMultiplier;	// A multiplier for any weapon done by this projectile.

		LTSmartLink		m_hFiredFrom;			// Who fired us

		LTFLOAT			m_fDeadTime;			// Time that we took enough damage to die
		LTFLOAT			m_fDeadDelay;			// The amount of time after dying that we should detonate



	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		// Member Variables
		CWeapon*		m_pWeapon;				// Needed to give feedback to the weapon.

		CDestructible	m_damage;				// Handle damage

		LTVector			m_vDims;				// Model dimensions

		uint32			m_dwFlags;				// Model flags
		uint32			m_dwFlags2;				// Model flags2

		const WEAPON*		m_pWeaponData;
		const BARREL*		m_pBarrelData;
		const AMMO*			m_pAmmoData;

};


class CSpriteProjectile : public CProjectile
{
	public :

		CSpriteProjectile(uint32 add_flags = 0, uint32 remove_flags = 0, uint32 add_flags2 = 0, uint32 remove_flags2 = 0 )
			: CProjectile( OT_SPRITE, add_flags | uint32(FLAG_NOLIGHT), remove_flags, add_flags2 | uint32(FLAG2_ADDITIVE), remove_flags2 ) {}
};


class CFlameSpriteProjectile : public CSpriteProjectile
{
	protected :

		virtual void Expire() { RemoveObject(); }
};


#endif //  __PROJECTILE_H__
