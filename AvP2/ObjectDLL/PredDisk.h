// ----------------------------------------------------------------------- //
//
// MODULE  : PredDisk.h
//
// PURPOSE : Predator Disk Weapon class - definition
//
// CREATED : 8/8/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PREDDISK_H__
#define __PREDDISK_H__

#include "Projectile.h"
#include "ProjectileTypes.h"
#include "CharacterMovement.h"
#include "ltsmartlink.h"

struct PREDDISKCLASSDATA;

class CPredDisk : public CProjectile
{
	public :
		
		CPredDisk() : CProjectile()
		{
			m_bMoving		= LTFALSE;
			m_pClassData	= LTNULL;
			m_hTarget		= LTNULL;
			m_vStickHackPos = LTVector(0,0,0);
			m_bStickHack	= LTFALSE;
			m_nBounceCount	= 0;
			m_hLastVictim	= LTFALSE;
			m_hTargetNode	= eModelNodeInvalid;
		}
	
		virtual void Setup(const ProjectileSetup & setup, const WFireInfo & info);

		virtual void	Update();
		virtual void	HandleTouch(HOBJECT hObj);

		virtual void	Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		virtual uint32	EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		void			HandleDiskRetrieve(LTBOOL bForce=LTFALSE);

	private:

		void	Pursue();
		LTBOOL	IsShooter(HOBJECT &hObj);
		void	HandlePickup();
		void	DamageTarget(HOBJECT &hObj);
		void	HandleWallImpact(HOBJECT &hObj);
		LTBOOL	IsPredatorDisk(HOBJECT &hObj);
		LTBOOL	ShouldDiskStick(CollisionInfo &info);
		char*	GetRandomNode();

		LTBOOL		m_bMoving;
		CTimer		m_ActivateTime;
		LTSmartLink	m_hTarget;
		LTSmartLink	m_hFiredFrom;
		LTSmartLink	m_hLastVictim;
		LTVector	m_vStickHackPos;
		LTBOOL		m_bStickHack;
		uint8		m_nBounceCount;
		HMODELNODE	m_hTargetNode;

		PREDDISKCLASSDATA* m_pClassData;
};

#endif //  __PREDDISK_H__
