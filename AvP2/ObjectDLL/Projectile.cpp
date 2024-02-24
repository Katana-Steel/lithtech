// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.cpp
//
// PURPOSE : Projectile class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Projectile.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "MsgIDs.h"
#include "ServerUtilities.h"
#include "Explosion.h"
#include "Character.h"
#include "ClientWeaponSFX.h"
#include "WeaponFXTypes.h"
#include "VolumeBrush.h"
#include "VolumeBrushTypes.h"
#include "ClientServerShared.h"
#include "SurfaceFunctions.h"
#include "PlayerObj.h"
#include "BodyProp.h"
#include "AI.h"
#include "CVarTrack.h"
#include "modellt.h"
#include "transformlt.h"
#include "physics_lt.h"
#include "ObjectMsgs.h"
#include "CharacterHitBox.h"
#include "CharacterMgr.h"
#include "ModelButeMgr.h"

#include "AIUtils.h"
#include "GameServerShell.h"

BEGIN_CLASS(CProjectile)
END_CLASS_DEFAULT_FLAGS(CProjectile, GameBase, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(CSpriteProjectile)
END_CLASS_DEFAULT_FLAGS(CSpriteProjectile, CProjectile, NULL, NULL, CF_HIDDEN)

BEGIN_CLASS(CFlameSpriteProjectile)
END_CLASS_DEFAULT_FLAGS(CFlameSpriteProjectile, CSpriteProjectile, NULL, NULL, CF_HIDDEN)

extern D_WORD g_wIgnoreFX;
extern uint8  g_nRandomWeaponSeed;

static LTBOOL DoVectorFilterFn(HOBJECT hObj, void *pUserData);

#define MAX_MODEL_NODES			9999
#define MAX_VECTOR_LOOP			20

static CVarTrack g_vtMissileSpeedTrack;
static CVarTrack g_vtInvisibleMaxThickness;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CProjectile::CProjectile(uint8 nType /*= OT_MODEL*/, uint32 add_flags /*= 0*/, uint32 remove_flags /*= 0*/, 
						                             uint32 add_flags2 /*= 0*/, uint32 remove_flags2 /*= 0*/ ) : GameBase(nType)
{
	AddAggregate(&m_damage);

	m_fVelocity				= 0.0f;
	m_fInstDamage			= 0.0f;
	m_fProgDamage			= 0.0f;
	m_fLifeTime				= 5.0f;
	m_fRange				= 10000.0f;
	m_fDamageMultiplier		= 1.0f;

	m_vDims.Init(1.0f, 1.0f, 1.0f);
	m_vFirePos.Init();
	m_vFlashPos.Init();
	m_nFlashSocket = 0;

	m_eInstDamageType		= DT_BULLET;
	m_eProgDamageType		= DT_UNSPECIFIED;
	m_nWeaponId				= 0;
	m_nAmmoId				= 0;
	m_nBarrelId				= 0;

	m_fStartTime			= 0.0f;
	m_bObjectRemoved		= LTFALSE;

	m_bDetonated			= LTFALSE;

	m_dwFlags = FLAG_FORCECLIENTUPDATE | FLAG_POINTCOLLIDE | FLAG_NOSLIDING | 
		FLAG_TOUCH_NOTIFY | /*FLAG_NOLIGHT |*/ FLAG_RAYHIT | FLAG_MODELKEYS;

	m_dwFlags |= add_flags;
	m_dwFlags &= ~remove_flags;


	m_dwFlags2 = 0;

	m_dwFlags2 |= add_flags2;
	m_dwFlags2 &= ~remove_flags2;

	m_pAmmoData				= LTNULL;
	m_pWeaponData			= LTNULL;

	m_pWeapon				= LTNULL;

	m_fDeadTime				= 0.0f;
	m_fDeadDelay			= 0.2f;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_MODELSTRINGKEY:
		{
			OnModelKey((ArgList*) pData);
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = m_dwFlags;
				pStruct->m_Flags2 = m_dwFlags2;

				// This will be set in Setup()...
#ifdef _WIN32
				SAFE_STRCPY(pStruct->m_Filename, "Models\\Default.abc");
#else
				SAFE_STRCPY(pStruct->m_Filename, "Models/Default.abc");
#endif
				pStruct->m_NextUpdate = UPDATE_DELTA;
			}			
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	uint32 dwRet = GameBase::ObjectMessageFn(hSender, messageID, hRead);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (m_damage.IsDead())
			{
				m_fDeadTime = g_pLTServer->GetTime();
			}
		}
		break;

		default : break;
	}

	return dwRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Setup
//
//	PURPOSE:	Set up a projectile using a setup struct
//
// ----------------------------------------------------------------------- //

void CProjectile::Setup(const ProjectileSetup & projectile_setup, const WFireInfo & info)
{

	if (!g_vtMissileSpeedTrack.IsInitted())
	{
		g_vtMissileSpeedTrack.Init(g_pLTServer, "MissileSpeed", LTNULL, 1.0f);
	}

	if (!g_vtInvisibleMaxThickness.IsInitted())
	{
		g_vtInvisibleMaxThickness.Init(g_pLTServer, "InvisibleMaxThickness", LTNULL, 100.0f);
	}

	m_vDir				= info.vPath;
	m_vDir.Norm();

	m_hFiredFrom		= info.hFiredFrom;
	m_nWeaponId			= projectile_setup.nWeaponId;
	m_nBarrelId			= projectile_setup.nBarrelId;
	m_nAmmoId			= projectile_setup.nAmmoId;
	m_fLifeTime			= projectile_setup.fLifeTime;

	m_pWeaponData		= g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (!m_pWeaponData) return;

	m_pAmmoData			= g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!m_pAmmoData) return;

	m_fInstDamage		= LTFLOAT(m_pAmmoData->nInstDamage);
	m_fProgDamage		= m_pAmmoData->fProgDamage;
	m_eInstDamageType	= m_pAmmoData->eInstDamageType;
	m_eProgDamageType	= m_pAmmoData->eProgDamageType;

	m_fDamageMultiplier = info.fDamageMultiplier;

	m_pBarrelData		= g_pWeaponMgr->GetBarrel(m_nBarrelId);
	if (!m_pBarrelData) return;

	// See about impact sounds...
	if(info.bIgnoreImpactSound)
		g_wIgnoreFX |= WFX_IGNORE_IMPACT_SOUND;


	if (info.bAltFire)
	{
		m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nAltVelocity : 0);
	}
	else
	{
		m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nVelocity : 0);
	}
		
	m_fRange			= (LTFLOAT) m_pBarrelData->nRange;

	AmmoType eAmmoType  = m_pAmmoData->eType;
	
	m_vFirePos			= info.vFirePos;
	m_vFlashPos			= info.vFlashPos;
	m_nFlashSocket		= info.nFlashSocket;

	if (m_hFiredFrom && IsCharacter(m_hFiredFrom))
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pChar)
		{
			ASSERT( m_pWeaponData );
			ASSERT( m_pBarrelData );
			ASSERT( m_pAmmoData );

			CharFireInfo char_info;
			char_info.vFiredPos = m_vFirePos;
			char_info.vFiredDir = m_vDir;

			if( m_pWeaponData )	char_info.nWeaponId  = m_pWeaponData->nId;
			if( m_pBarrelData ) char_info.nBarrelId  = m_pBarrelData->nId;
			if( m_pAmmoData )   char_info.nAmmoId	= m_pAmmoData->nId;
			char_info.fTime		= g_pLTServer->GetTime();

			pChar->SetLastFireInfo(char_info);
		}
	}

	// See if we start inside the test object...

	if (!TestInsideObject(info.hTestObj, eAmmoType))
	{
		if (eAmmoType == PROJECTILE)
		{
			DoProjectile();
		}
		else if (eAmmoType == VECTOR || eAmmoType == CANNON)
		{
			DoVector();  // Cast a ray
		}
		else if (eAmmoType == GADGET)
		{
			; // Do nothing for now
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Setup
//
//	PURPOSE:	Set up a projectile with the information needed
//
// ----------------------------------------------------------------------- //

void CProjectile::Setup(CWeapon* pWeapon, const WFireInfo & info)
{
	if(!pWeapon) return;

	const AMMO* pAmmoData = pWeapon->GetAmmoData();
	if (!pAmmoData) return;

	LTFLOAT fLifeTime = 0.0f;
	if( pAmmoData && pAmmoData->pProjectileFX )
	{
		fLifeTime = pAmmoData->pProjectileFX->fLifeTime;
	}

	// [KLS] Only set m_pWeapon if we're a non-projectile projectile.  True
	// projectiles have a lifetime that may be longer than m_pWeapon's...
	if (PROJECTILE == pAmmoData->eType)
	{
		m_pWeapon = LTNULL;
	}
	else
	{
		// Record the weapon pointer...
		m_pWeapon = pWeapon;
	}

	const ProjectileSetup projectile_setup( pWeapon->GetId(), pWeapon->GetBarrelId(), pWeapon->GetAmmoId(), fLifeTime);
	Setup( projectile_setup, info );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ForcedHitSetup
//
//	PURPOSE:	Set up a projectile with the information needed
//				This version used for a forced hit.
//
// ----------------------------------------------------------------------- //

void CProjectile::ForcedHitSetup(const CWeapon &  weapon, const WFireInfo & info)
{
	// Load up a bunch of stuff...  It's good stuff, trust me!
	
	m_vDir			= info.vPath;
	m_vDir.Norm();

	m_hFiredFrom		= info.hFiredFrom;
	m_fDamageMultiplier = info.fDamageMultiplier;
	m_vFirePos			= info.vFirePos;
	m_vFlashPos			= info.vFlashPos;
	m_nFlashSocket		= info.nFlashSocket;

	m_nWeaponId			= weapon.GetId();
	m_nBarrelId			= weapon.GetBarrelId();
	m_nAmmoId			= weapon.GetAmmoId();

	m_pWeaponData		= g_pWeaponMgr->GetWeapon(m_nWeaponId);
	m_pAmmoData			= g_pWeaponMgr->GetAmmo(m_nAmmoId);
	m_pBarrelData		= g_pWeaponMgr->GetBarrel(m_nBarrelId);

	// Can't hurt to do a sanity check here...
	if (!m_pWeaponData || !m_pAmmoData || !m_pBarrelData) return;

	m_fLifeTime			= 0.0f;
	m_fVelocity			= 0.0f;

	m_fInstDamage		= LTFLOAT(m_pAmmoData->nInstDamage);
	m_fProgDamage		= m_pAmmoData->fProgDamage;
	m_eInstDamageType	= m_pAmmoData->eInstDamageType;
	m_eProgDamageType	= m_pAmmoData->eProgDamageType;

	m_fRange			= (LTFLOAT) m_pBarrelData->nRange;

	AmmoType eAmmoType  = m_pAmmoData->eType;

	// Record the laast fire info stuff for the character...
	CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hFiredFrom));
	if (pChar)
	{
		CharFireInfo char_info;

		char_info.vFiredPos = m_vFirePos;
		char_info.vFiredDir = m_vDir;
		char_info.nWeaponId = m_pWeaponData->nId;
		char_info.nBarrelId = m_pBarrelData->nId;
		char_info.nAmmoId	= m_pAmmoData->nId;
		char_info.fTime		= g_pLTServer->GetTime();

		pChar->SetLastFireInfo(char_info);
	}


	// Ok time to do the deed...
	DoForcedHit(info.hTestObj, info.eNodeHit);  
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TestInsideObject
//
//	PURPOSE:	Test to see if the projectile is inside the test object
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType)
{
	if (!hTestObj) return LTFALSE;

	// TO DO???
	// NOTE:  This code may need to be updated to use test the dims
	// of the CharacterHitBox instead of the dims of the object...
	// TO DO???

	// See if we are inside the test object...

	LTVector vTestPos, vTestDims;
	g_pLTServer->GetObjectPos(hTestObj, &vTestPos);
	g_pLTServer->GetObjectDims(hTestObj, &vTestDims);

	if (m_vFirePos.x < vTestPos.x - vTestDims.x ||
		m_vFirePos.x > vTestPos.x + vTestDims.x ||
		m_vFirePos.y < vTestPos.y - vTestDims.y ||
		m_vFirePos.y > vTestPos.y + vTestDims.y ||
		m_vFirePos.z < vTestPos.z - vTestDims.z ||
		m_vFirePos.z > vTestPos.z + vTestDims.z)
	{
		return LTFALSE;
	}

	
	//
	// We're inside the object, so we automatically hit the object...
	//

	// Just set the node to be the default hit node.
	if(IsCharacter(hTestObj))
	{
		CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject(hTestObj);
		if(!pChar) return LTFALSE;

		const ModelNode eModelNode = g_pModelButeMgr->GetSkeletonDefaultHitNode(pChar->GetModelSkeleton());

		pChar->SetModelNodeLastHit(eModelNode);
		AdjustDamage(*pChar, eModelNode);
	}
	else if(IsBodyProp(hTestObj))
	{
		BodyProp *pBody = (BodyProp*)g_pLTServer->HandleToObject(hTestObj);
		if(!pBody) return LTFALSE;

		const ModelNode eModelNode = g_pModelButeMgr->GetSkeletonDefaultHitNode(pBody->GetModelSkeleton());

		pBody->SetModelNodeLastHit(eModelNode);
		AdjustDamage(*pBody, eModelNode);
	}

	// And now we blow up.
	if (eAmmoType == PROJECTILE)
	{
		Detonate(hTestObj, LTFALSE);
	}
	else 
	{
		if (eAmmoType == VECTOR)
		{
			ImpactDamageObject(m_hFiredFrom, hTestObj);
		}

		LTVector vNormal(0, 1, 0);
		AddImpact(hTestObj, m_vFlashPos, vTestPos, vNormal, GetSurfaceType(hTestObj));
		RemoveObject();
	}


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

void CProjectile::InitialUpdate(int nInfo)
{
	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_MOVEABLE | USRFLG_IGNORE_PROJECTILES);

	m_damage.SetMass(0.0f);
	m_damage.SetHitPoints(1.0f);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetCanHeal(LTFALSE);
	m_damage.SetCanRepair(LTFALSE);
	m_damage.SetApplyDamagePhysics(LTFALSE);

	g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void CProjectile::Update()
{
	g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);


	// If we're dead... wait a small time to blow up (chain explosions)

	if (m_damage.IsDead() && (g_pLTServer->GetTime() > m_fDeadTime + m_fDeadDelay))
	{
		Detonate(LTNULL);
	}

	// If we didn't hit anything, blow up...

	if ( (m_fLifeTime != -1.0f) && g_pLTServer->GetTime() >= (m_fStartTime + m_fLifeTime)) 
	{
		Expire();
	} 
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleTouch(HOBJECT hObj)
{
	if (m_bObjectRemoved) return;


	 // Let it get out of our bounding box...

	if (hObj == m_hFiredFrom) return;

	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		if (pHitBox->GetModelObject() == m_hFiredFrom) return;
	}
	else if(IsCharacter(hObj))
	{
		if (hObj == GetFiredFrom()) return;

		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pChar->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else if(IsBody(hObj))
	{
		BodyProp* pBody = (BodyProp*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pBody->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

		if(!(dwFlags & FLAG_SOLID) && !(dwFlags & FLAG_RAYHIT))
			return;
	}

	


	// See if we want to impact on this object...

	uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	LTBOOL bIsWorld = (LT_YES == g_pLTServer->Physics()->IsWorldObject(hObj));


	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);
	if ((!bIsWorld && (!(dwFlags & FLAG_SOLID) && !(dwFlags & FLAG_RAYHIT))) || pHitBox) 
	{
		if (pHitBox) 
		{
			// See if we really impacted on the box...

			if (pHitBox->DidProjectileImpact(this))
			{
				// This is the object that we really hit...

				hObj = pHitBox->GetModelObject();
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	// Don't hit our own projectiles (multi-projectile weapons)...
	CProjectile* pObj = dynamic_cast<CProjectile*>(g_pLTServer->HandleToObject(hObj));
	if (pObj)
	{
		if (pObj->GetFiredFrom() == m_hFiredFrom)
		{
			return;
		}
	}


	// See if we hit the sky...

	if (bIsWorld)
	{
		CollisionInfo info;
		g_pLTServer->GetLastCollision(&info);

		SurfaceType eType = GetSurfaceType(info);

		if (eType == ST_SKY)
		{
			RemoveObject();
			return;
		}
	}


	HandleImpact(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleImpact()
//
//	PURPOSE:	Allow sub-classes to handle impacts...Default is to
//				go boom.
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleImpact(HOBJECT hObj)
{
	Detonate(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectile::Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact)
{
	if (m_bDetonated) return;

	m_bDetonated = LTTRUE;
	SurfaceType eType = ST_UNKNOWN;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Determine the normal of the surface we are impacting on...

	LTVector vNormal;
	vNormal.Init(0.0f, 0.0f, 0.0f);


	if (hObj && bDidProjectileImpact)
	{
		CollisionInfo info;
		g_pLTServer->GetLastCollision(&info);

		eType = GetSurfaceType(info);

		if (LT_YES == g_pLTServer->Physics()->IsWorldObject(hObj))
		{
			vNormal = info.m_Plane.m_Normal;

			LTRotation rRot;
			g_pLTServer->AlignRotation(&rRot, &vNormal, LTNULL);
			g_pLTServer->SetObjectRotation(m_hObject, &rRot);

			// Calculate where we really hit the plane...

			LTVector vVel, vP0, vP1;
			g_pLTServer->GetVelocity(m_hObject, &vVel);

			vP1 = vPos;
			vVel *= g_pLTServer->GetFrameTime();
			vP0 = vP1 - vVel;
			vP1 += vVel;

			LTFLOAT fDot1 = DIST_TO_PLANE(vP0, info.m_Plane);
			LTFLOAT fDot2 = DIST_TO_PLANE(vP1, info.m_Plane);

			if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
			{
				vPos = vP1;
			}
			else
			{
				LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
				VEC_LERP(vPos, vP0, vP1, fPercent);
			}
		}
	}
	else
	{
		// Since hObj was null, this means the projectile's lifetime was up,
		// so we just blew-up in the air.

		eType = ST_AIR; 
	}


	if (eType == ST_UNKNOWN)
	{
		eType = GetSurfaceType(hObj);
	}


	// Handle impact damage...

	if (hObj)
	{
		if(IsCharacter(hObj))
		{
			CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
			if(pChar)
			{
				const ModelNode eModelNode = pChar->GetModelNodeLastHit();

				AdjustDamage(*pChar, eModelNode);
				pChar->GetNodePosition(eModelNode, vPos, pChar->GetModelSkeleton());
			}
		}
		else if(IsBodyProp(hObj))
		{
			BodyProp *pBody = (BodyProp*)g_pLTServer->HandleToObject(hObj);
			if(pBody)
			{
				const ModelNode eModelNode = pBody->GetModelNodeLastHit();

				AdjustDamage(*pBody, eModelNode);
				pBody->GetNodePosition(eModelNode, vPos);
			}
		}

		HOBJECT hDamager;
		if (m_hFiredFrom)
			hDamager = m_hFiredFrom;
		else
			hDamager = m_hObject;

		ImpactDamageObject(hDamager, hObj);
	}

	AddImpact(hObj, m_vFlashPos, vPos, vNormal, eType);

	// Remove projectile from world...

	RemoveObject();
}


PROJECTILECLASSDATA* CProjectile::GetAmmoClassData()
{
	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX )
		return LTNULL;
	
	return m_pAmmoData->pProjectileFX->pClassData;
}


LTVector CProjectile::GetAmmoScale() const
{
	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX )
		return LTVector(1,1,1);
	
	return m_pAmmoData->pProjectileFX->vModelScale;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddImpact()
//
//	PURPOSE:	Add an impact object.
//
// ----------------------------------------------------------------------- //

void CProjectile::AddImpact(HOBJECT hObj, LTVector vFirePos, LTVector vImpactPos,
							LTVector vSurfaceNormal, SurfaceType eType)
{
	// sanity check
	if(!m_pWeaponData || !m_pAmmoData || !m_pBarrelData)
		return;

	// Create the client side weapon fx...

	CLIENTWEAPONFX fxStruct;

	fxStruct.hFiredFrom		= m_hFiredFrom;
	fxStruct.vSurfaceNormal	= vSurfaceNormal;
	fxStruct.vFirePos		= vFirePos;
	fxStruct.vPos			= vImpactPos + vSurfaceNormal;
	fxStruct.hObj			= hObj;
	fxStruct.nWeaponId		= m_pWeaponData->nId;
	fxStruct.nAmmoId		= m_pAmmoData->nId;
	fxStruct.nBarrelId		= m_pBarrelData->nId;
	fxStruct.nSurfaceType	= eType;
	fxStruct.nFlashSocket	= m_nFlashSocket;
	fxStruct.wIgnoreFX		= g_wIgnoreFX;

	// If we tried to play a sound the inform the weapon...
	if(m_pWeapon && !(g_wIgnoreFX & WFX_IGNORE_IMPACT_SOUND) && (eType != ST_SKY))
		m_pWeapon->SetPlayedImpactSound();

	// If we do multiple calls to AddImpact, make sure we only do some
	// effects once :)

	g_wIgnoreFX |= WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;

	
	// Allow exit surface fx on the next call to AddImpact...

	g_wIgnoreFX &= ~WFX_EXITSURFACE;


	if (IsMoveable(hObj))
	{
		// Well, don't do too many exit marks...The server will add one
		// if necessary...

		g_wIgnoreFX |= WFX_EXITMARK;
	}


	// If this is a player object, get the client id...

	CCharacter * pChar = dynamic_cast<CCharacter*>( m_hFiredFrom ? g_pLTServer->HandleToObject(m_hFiredFrom) : LTNULL );
	CPlayerObj * pPlayer = dynamic_cast<CPlayerObj*>( pChar );

	// Don't allow AI's to show blood pools when doing taking friendly fire.
	if( pChar && !pPlayer )
	{
		CCharacter * pCharVictim = dynamic_cast<CCharacter*>( hObj ? g_pLTServer->HandleToObject(hObj) : LTNULL );
		if( pCharVictim && g_pCharacterMgr->AreAllies(*pChar,*pCharVictim) )
		{
			fxStruct.wIgnoreFX |= WFX_IMPACTONFLESH;
		}
	}

	// Set the player's id.
	if (pPlayer)
	{
		fxStruct.nShooterId	= (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
	}

	CreateClientWeaponFX(fxStruct);
	
	if (m_pAmmoData->nAreaDamage > 0.0f && eType != ST_SKY)
	{
		AddExplosion(vImpactPos, vSurfaceNormal);
	}


	// Update Character fire info...

	if (pChar && eType != ST_SKY)
	{
		CharImpactInfo info;
		info.vImpactPos = vImpactPos;

		if( m_pWeaponData )	info.nWeaponId  = m_pWeaponData->nId;
		if( m_pBarrelData ) info.nBarrelId  = m_pBarrelData->nId;
		if( m_pAmmoData )   info.nAmmoId	= m_pAmmoData->nId;
		info.fTime		= g_pLTServer->GetTime();
		info.eSurface	= eType;

		pChar->SetLastImpactInfo(info);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddProjectileMuzzleFX()
//
//	PURPOSE:	Call the muzzle FX for a projectile...
//
// ----------------------------------------------------------------------- //

void CProjectile::AddProjectileMuzzleFX(LTVector vFirePos)
{
	// Create the client side weapon fx...

	LTVector vNULL(0,0,0);

	CLIENTWEAPONFX fxStruct;

	fxStruct.hFiredFrom		= m_hFiredFrom;
	fxStruct.vSurfaceNormal	= vNULL;
	fxStruct.vFirePos		= vFirePos;
	fxStruct.vPos			= vNULL;
	fxStruct.hObj			= LTNULL;
	fxStruct.nWeaponId		= m_pWeaponData->nId;
	fxStruct.nAmmoId		= m_pAmmoData->nId;
	fxStruct.nBarrelId		= m_pBarrelData->nId;
	fxStruct.nSurfaceType	= ST_UNKNOWN;
	fxStruct.nFlashSocket	= m_nFlashSocket;
	fxStruct.wIgnoreFX		= WFX_MUZZLEONLY;

	// If this is a player object, get the client id...

	if (m_hFiredFrom && IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			fxStruct.nShooterId	= (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	CreateClientWeaponFX(fxStruct);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CProjectile::AddExplosion(LTVector vPos, LTVector vNormal)
{
	HCLASS hClass = g_pLTServer->GetClass("Explosion");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_Pos = vPos;

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-Explosion", g_pLTServer->GetObjectName(m_hObject) );
#endif

	LTRotation rRot;
	g_pLTServer->AlignRotation(&rRot, &vNormal, &vNormal);
	ROT_COPY(theStruct.m_Rotation, rRot);

	Explosion* pExplosion = (Explosion*)g_pLTServer->CreateObject(hClass, &theStruct);
	if (pExplosion)
	{
		EXPLOSION_CREATESTRUCT explosion_struct;

		explosion_struct.vPos = vPos;

		explosion_struct.eDamageType = m_pAmmoData->eAreaDamageType;
		explosion_struct.fDamageRadius = LTFLOAT(m_pAmmoData->nAreaDamageRadius);
		explosion_struct.fMaxDamage = LTFLOAT(m_pAmmoData->nAreaDamage)*m_fDamageMultiplier;

		explosion_struct.eProgDamageType = m_pAmmoData->eProgDamageType;
		explosion_struct.fProgDamage = m_pAmmoData->fProgDamage*m_fDamageMultiplier;
		explosion_struct.fProgDamageDuration = m_pAmmoData->fProgDamageDuration;

		explosion_struct.bRemoveWhenDone = LTTRUE;
		explosion_struct.hFiredFrom = m_hFiredFrom;
		
		pExplosion->Setup(explosion_struct);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void CProjectile::AddSpecialFX()
{
	if (!g_pWeaponMgr) return;

	// If this is a player object, get the client id...

	uint8 nShooterId = -1;
	if (m_hFiredFrom && IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	//be sure we are visible
    uint32 dwUserFlags; 
	dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUserFlags |= USRFLG_VISIBLE;
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
	
	// Create a special fx...

	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_PROJECTILE_ID);
	g_pLTServer->WriteToMessageByte(hMessage, m_pWeaponData->nId);
	g_pLTServer->WriteToMessageByte(hMessage, m_pAmmoData->nId);
	g_pLTServer->WriteToMessageByte(hMessage, nShooterId);
	g_pLTServer->EndMessage(hMessage);

	// See if this is a flame thrower
	if(stricmp(m_pWeaponData->szName, "Flame_Thrower") == 0 || stricmp(m_pWeaponData->szName, "Flame_Thrower_MP") == 0 || stricmp(m_pWeaponData->szName, "Exosuit_Flame_Thrower") == 0 )
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hFiredFrom);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_FLAME_FX_SHOT);
		g_pLTServer->EndMessage(hMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::RemoveObject()
//
//	PURPOSE:	Remove the object, and do clean up (isle 4)
//
// ----------------------------------------------------------------------- //

void CProjectile::RemoveObject()
{
	if (m_bObjectRemoved || !m_hObject ) return;

	g_pLTServer->RemoveObject(m_hObject);	

	m_bObjectRemoved = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoVectorFilterFn()
//
//	PURPOSE:	Filter the attacker out of IntersectSegment 
//				calls (so you don't shot yourself).  Also handle
//				AIs of the same alignment not shooting eachother
//
// ----------------------------------------------------------------------- //

LTBOOL DoVectorFilterFn(HOBJECT hObj, void *pUserData)
{
	// We're not attacking our self...

	if (SpecificObjectFilterFn(hObj, pUserData))
	{
		// CharacterHitBox objects are used for vector impacts, don't
		// impact on the character/body prop object itself....

		if (IsCharacter(hObj) || IsBodyProp(hObj))
		{
			return LTFALSE;
		}

		HOBJECT hUs = (HOBJECT)pUserData;

		// This makes sure that any character is not capable of hitting his
		// own hit box.   You might want to change this if you need to handle
		// bouncing vectors projectiles... either that or change the user data
		// after the first shot.
		if (IsCharacter(hUs) && IsCharacterHitBox(hObj))
		{
			CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
			if (pCharHitBox)
			{
				if (pCharHitBox->GetModelObject() == hUs)
					return LTFALSE;
			}
		}

		// This was a check for friendly fire for AI's... it may need to change
/*		if (IsAI(hUs) && IsCharacterHitBox(hObj))
		{
			CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
			if (pCharHitBox)
			{
				HOBJECT hTestObj = pCharHitBox->GetModelObject();
				if (hTestObj && IsAI(hTestObj))
				{
					CAI *pAI = dynamic_cast<CAI*>(g_pLTServer->HandleToObject(hUs));
					if (!pAI) return LTFALSE;

					// We can't hit guys we like...
					if (pAI->CheckAlignment(LIKE, hTestObj))
					{
						return LTFALSE;
					}

				}
			}
		}
*/
		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoProjectile
//
//	PURPOSE:	Do projectile stuff...
//
// ----------------------------------------------------------------------- //

void CProjectile::DoProjectile()
{
	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX) return;

	// Set up the model...

	ObjectCreateStruct createStruct;
	createStruct.Clear();

	SAFE_STRCPY(createStruct.m_Filename, m_pAmmoData->pProjectileFX->szModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], m_pAmmoData->pProjectileFX->szSkin);

	g_pLTServer->Common()->SetObjectFilenames(m_hObject, &createStruct);

	g_pLTServer->ScaleObject(m_hObject, &(m_pAmmoData->pProjectileFX->vModelScale));

	g_pLTServer->Common()->GetModelAnimUserDims(m_hObject, &m_vDims, 0);

	m_vDims.x *= m_pAmmoData->pProjectileFX->vModelScale.x;
	m_vDims.y *= m_pAmmoData->pProjectileFX->vModelScale.y;
	m_vDims.z *= m_pAmmoData->pProjectileFX->vModelScale.z;

	g_pLTServer->SetObjectDims(m_hObject, &m_vDims);

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if(m_pAmmoData->pProjectileFX->bNoLighting)
		dwFlags |= FLAG_NOLIGHT;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		
	// Get and set the color of the model
	LTFLOAT r, g, b, a;
	g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);
	g_pLTServer->SetObjectColor(m_hObject, r, g, b, m_pAmmoData->pProjectileFX->fModelAlpha);
	
	// Start your engines...

	m_fStartTime = g_pLTServer->GetTime();
	

	// Make the flash position the same as the fire position...
	
	m_vFlashPos	= m_vFirePos;
//	m_nFlashSocket = 0;

	// Set our force ignore limit and mass...

	g_pLTServer->SetForceIgnoreLimit(m_hObject, 0.0f);
	g_pLTServer->SetObjectMass(m_hObject, 0.0f);


	// Make sure we are pointing in the direction we are traveling...

	LTRotation rRot;
	g_pLTServer->AlignRotation(&rRot, &m_vDir, LTNULL);
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);


	// Make sure we have the correct flags set...

	dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= m_pAmmoData->pProjectileFX->dwObjectFlags;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);


	// And away we go...
	LTVector vVel;
	vVel = m_vDir * (m_fVelocity * g_vtMissileSpeedTrack.GetFloat(1.0f));

	g_pLTServer->SetVelocity(m_hObject, &vVel);

	// Create the muzzle FX
	AddProjectileMuzzleFX(m_vFirePos);


	// Special case of 0 life time...

	if (m_fLifeTime == 0.0f) 
	{
		Detonate(LTNULL);
	}
	else
	{
		AddSpecialFX();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TestForShooterHitBox
//
//	PURPOSE:	Test if this is the shooter's hit box
//
// ----------------------------------------------------------------------- //

LTBOOL TestForShooterHitBox(HOBJECT hObj, HOBJECT hShooter)
{
	CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
	if (pCharHitBox)
	{
		if (pCharHitBox->GetModelObject() == hShooter)
			return LTTRUE;
	}
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoVector
//
//	PURPOSE:	Do vector stuff
//
// ----------------------------------------------------------------------- //

void CProjectile::DoVector()
{
	LTVector vTo, vFrom;
	vFrom = m_vFirePos;
	vTo	= vFrom + (m_vDir * m_fRange);

	IntersectInfo iInfo, iHitInfo;
	IntersectQuery qInfo;

	// first see if there are any hit boxes near by
	ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vFrom, 50.0f);

	if (pList)
	{
		LTBOOL bHit = LTFALSE;
		uint32 nRange = -1;

		ObjectLink* pLink = pList->m_pFirstLink;
		while (pLink)
		{
			// see if this object is a character hit box and not our own!
			if(IsCharacterHitBox(pLink->m_hObject) && !TestForShooterHitBox(pLink->m_hObject, m_hFiredFrom))
			{
				iInfo.m_hObject = pLink->m_hObject;
				LTVector vTempFrom  = iInfo.m_Point	= m_vFirePos + m_vDir;

				LTVector vNodePos;
				// ok, now see if we hit a hit ball
				if (HandlePotentialHitBoxImpact(&iInfo, &vTempFrom, &vNodePos))
				{
					// Yeah we hit a node!
					bHit = LTTRUE;

					//see if this is close enuf to record
					if((vNodePos-iInfo.m_Point).MagSqr()<nRange)
					{
						//do the record
						nRange = (uint32)(vNodePos-iInfo.m_Point).MagSqr();
						iHitInfo.m_hObject	= iInfo.m_hObject;
						iHitInfo.m_Point	= vNodePos;
					}
				}
			}
			pLink = pLink->m_pNext;
		}

		// ok now give back the list
		g_pLTServer->RelinquishList(pList);

		// test to see if we have a close target
		if(bHit)
		{
			HandleVectorImpact(iHitInfo, &m_vFirePos, &vTo);
			return;
		}
	}

	// now see if we hit anything in the distance...
	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

    LTBOOL	bHitSomething = LTFALSE;
    LTBOOL	bDone = LTFALSE;

	int nLoopCount = 0; // No infinite loops thanks.

	HOBJECT hLastHitbox = LTNULL;

	while (!bDone)
	{
		qInfo.m_FilterFn	 = DoVectorFilterFn;
		qInfo.m_pUserData	 = m_hFiredFrom;

		qInfo.m_From = vFrom;
		qInfo.m_To   = vTo;

		++g_cIntersectSegmentCalls;
		if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
		{
			if ( iInfo.m_hObject == hLastHitbox )
			{
				bDone = LTTRUE;
				continue;
			}

			if (IsCharacterHitBox(iInfo.m_hObject))
			{
				hLastHitbox = iInfo.m_hObject;

				LTVector vNodePos;

				if (HandlePotentialHitBoxImpact(&iInfo, &vFrom, &vNodePos))
				{
					//record the updated point
					iInfo.m_Point	= vNodePos;

					HandleVectorImpact(iInfo, &m_vFirePos, &vTo);
					return;
				}
			}
			else
			{
				if (HandleVectorImpact(iInfo, &m_vFirePos, &vTo))
				{
					return;
				}
				else
				{
					vFrom = m_vFirePos;
				}
			}
		}
		else // Didn't hit anything...
		{
			bDone = LTTRUE;
		}


		// Make sure we don't loop forever...

		if (++nLoopCount > MAX_VECTOR_LOOP)
		{
#ifndef _FINAL
			g_pLTServer->CPrint("ERROR in CProjectile::DoVector() - Infinite loop encountered!!!");
#endif
			bDone = LTTRUE;
		}
	}


	// Didn't hit anything so just impact at the end pos...

	LTVector vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	AddImpact(LTNULL, m_vFlashPos, vTo, vUp, ST_SKY);


	// Okay, we're all done now...bye, bye...

	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoForcedHit
//
//	PURPOSE:	Hit the bitz
//
// ----------------------------------------------------------------------- //

void CProjectile::DoForcedHit(HOBJECT hVictim, ModelNode eNodeHit)
{
	// Tell the hitbox to do its thing...
	CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hVictim));

	// Sanity
	if(!pChar) return;


	CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(pChar->GetHitBox()));

	// More sanity!
	if (!pCharHitBox) return;


	// Take care of the victim's hit box and then do the impact stuff...
	LTVector vNodePos;
	pCharHitBox->HandleForcedImpact(this, eNodeHit, &vNodePos);

	HandleForcedImpact(hVictim, vNodePos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleForcedImpact
//
//	PURPOSE:	Handle a vector hitting something
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleForcedImpact( HOBJECT hVictim, LTVector& vNodePos)
{
	// Do the damage
	ImpactDamageObject(m_hFiredFrom, hVictim);

	// Do the FX
	uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(hVictim);
	SurfaceType eSurfType = UserFlagToSurface(dwUsrFlags);

	AddImpact(hVictim, m_vFlashPos, vNodePos, -m_vDir, eSurfType);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandleVectorImpact( const IntersectInfo & iInfo, LTVector * pvFrom,
									   LTVector * pvTo)
{
	ASSERT( pvFrom && pvTo );

	if( !pvFrom || !pvTo )
		return LTFALSE;

	// Get the surface type...
	const SurfaceType eSurfType = GetSurfaceType(iInfo); 


	// See if we hit an object that should be damaged...

	LTBOOL bHitWorld = (LT_YES == g_pLTServer->Physics()->IsWorldObject(iInfo.m_hObject));

	if (!bHitWorld && eSurfType != ST_LIQUID)
	{
		ImpactDamageObject(m_hFiredFrom, iInfo.m_hObject);
	}


	// If the fire position is the initial fire position, use the flash
	// position when building the impact special fx...

	LTVector vFirePos = (pvFrom->Equals(m_vFirePos) ? m_vFlashPos : *pvFrom);

	if( eSurfType != ST_INVISIBLE )
	{
		AddImpact(iInfo.m_hObject, vFirePos, iInfo.m_Point, iInfo.m_Plane.m_Normal, eSurfType);
	}


	// See if we can shoot through the surface...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	if (!pSurf) return LTTRUE;  // Done.

	if (!bHitWorld && pSurf->bCanShootThrough)
	{		
		return UpdateDoVectorValues(*pSurf, 0, iInfo.m_Point, pvFrom, pvTo);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::UpdateVectorValues
//
//	PURPOSE:	Update our DoVector values
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::UpdateDoVectorValues(const SURFACE & surf, LTFLOAT fThickness, 
										LTVector vImpactPos, LTVector * pvFrom /* = LTNULL */, LTVector * pvTo /* = LTNULL */)
{
	// See if we've traveled the distance...

	LTVector vDistTraveled = vImpactPos - m_vFirePos;
	LTFLOAT fDist = m_fRange - vDistTraveled.Mag();
	if (fDist < 1.0f) return LTTRUE;

	// Dampen damage and distance...
	m_fInstDamage *= surf.fBulletDamageDampen;
	fDist *= surf.fBulletRangeDampen;

	int nPerturb = surf.nMaxShootThroughPerturb;

	if (nPerturb) 
	{
		LTRotation rRot;
		g_pLTServer->AlignRotation(&rRot, &m_vDir, LTNULL);
		
		LTVector vU, vR, vF;
		g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

		LTFLOAT fRPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;
		LTFLOAT fUPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

		m_vDir += (vR * fRPerturb);
		m_vDir += (vU * fUPerturb);

		m_vDir.Norm();
	}

	// Make sure we move the from position...

	if( pvFrom )
	{
		if (pvFrom->Equals(vImpactPos, 1.0f))
		{
			*pvFrom += m_vDir;
		}
		else
		{
			*pvFrom = vImpactPos;
		}

		if( pvTo )
			*pvTo = *pvFrom + (m_vDir * fDist);
	}


	return LTFALSE; 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CalcInvisibleImpact
//
//	PURPOSE:	Update the impact value so it ignores invisible surfaces
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::CalcInvisibleImpact(IntersectInfo * piInfo, SurfaceType * peSurfType)
{
	if( !piInfo ) return LTFALSE;

	// Since we hit an invisible surface try and find a solid surface that
	// is the real surface of impact.  NOTE:  We assume that the solid 
	// surface will have a normal facing basically the opposite direction...

	IntersectInfo iTestInfo;
	IntersectQuery qTestInfo;

	qTestInfo.m_From = piInfo->m_Point + (m_vDir * g_vtInvisibleMaxThickness.GetFloat());
	qTestInfo.m_To   = piInfo->m_Point - m_vDir;

	qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	qTestInfo.m_FilterFn  = DoVectorFilterFn;
	qTestInfo.m_pUserData = m_hFiredFrom;

	++g_cIntersectSegmentCalls;
	if (g_pLTServer->IntersectSegment(&qTestInfo, &iTestInfo))
	{
		SurfaceType eTestSurfType = GetSurfaceType(iTestInfo); 
		
		// If we hit another invisible surface, we're done...

		if (eTestSurfType != ST_INVISIBLE)
		{
			if( peSurfType ) *peSurfType = eTestSurfType;
			*piInfo = iTestInfo;
			return LTTRUE;
		}
	}

	return LTFALSE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandlePotentialHitBoxImpact
//
//	PURPOSE:	Handle a vector hitting a Character's hit box 
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandlePotentialHitBoxImpact(IntersectInfo * piInfo, 
											   LTVector * pvFrom, LTVector* pvNodePos)
{
	if( !piInfo ) return LTFALSE;

	CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(piInfo->m_hObject));
	if (!pCharHitBox) return LTFALSE;

	return pCharHitBox->HandleImpact(this, piInfo, m_vDir, pvFrom, pvNodePos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AdjustDamage
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

void CProjectile::AdjustDamage(const CCharacter & damagee, ModelNode eModelNode)
{
	m_fInstDamage *= damagee.ComputeDamageModifier(eModelNode);
}

void CProjectile::AdjustDamage(const BodyProp & damagee, ModelNode eModelNode)
{
	m_fInstDamage *= damagee.ComputeDamageModifier(eModelNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ImpactDamageObject
//
//	PURPOSE:	Handle impact damage to an object
//
// ----------------------------------------------------------------------- //

void CProjectile::ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj)
{
	DamageStruct damage;

	damage.hDamager = hDamager;
	damage.vDir		= m_vDir;

	// Do Instant damage...

	if (m_fInstDamage > 0.0f)
	{
		damage.eType	= m_eInstDamageType;
		damage.fDamage	= m_fInstDamage*m_fDamageMultiplier;

		if(hDamager)
		{
			damage.DoDamage(g_pLTServer->HandleToObject(hDamager), hObj);
		}
		else
		{
			damage.DoDamage(this, hObj);
		}
	}

	// Do Progressive damage...

	if (m_pAmmoData && m_pAmmoData->fProgDamageDuration > 0.0f)
	{
		damage.eType	 = m_eProgDamageType;
		damage.fDamage	 = m_fProgDamage*m_fDamageMultiplier;
		damage.fDuration = m_pAmmoData->fProgDamageDuration;

		if(hDamager)
		{
			damage.DoDamage(g_pLTServer->HandleToObject(hDamager), hObj);
		}
		else
		{
			damage.DoDamage(this, hObj);
		}
	}

	//do heal if head_bite
	if(m_eInstDamageType == DT_HEADBITE || m_eInstDamageType == DT_ALIEN_CLAW)
	{
		LTFLOAT fHealValue = 0.0f;

		//only heal if character is head bitten NOT clawed
		//heal if body prop is clawed OR head bitten
		CCharacter* pCharVictim	= dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));
		BodyProp* pBodyVictim = dynamic_cast<BodyProp*>(g_pLTServer->HandleToObject(hObj));

		if(pCharVictim && (m_eInstDamageType == DT_HEADBITE))
		{
			const CharacterButes* pButes = pCharVictim->GetButes();

			if(pButes)
				fHealValue = pButes->m_fHeadBiteHealPoints;
		}
		else if(pBodyVictim && pBodyVictim->GetCanFade() && pBodyVictim->GetCanGib())
		{
			if(g_pGameServerShell->LowViolence())
				fHealValue = pBodyVictim->GetClawHealPoints();
			else
				fHealValue = (m_eInstDamageType == DT_HEADBITE) ? pBodyVictim->GetHeadBiteHealPoints() : pBodyVictim->GetClawHealPoints();

			// always increment the counter...
			pBodyVictim->IncrementClawHits();
		}

		//get a pointer to the head biter
		CCharacter*	pAttacker = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hDamager));

		//aply the heal
		if(pAttacker)
		{
			CDestructible* pDest = pAttacker->GetDestructible();

			if(pDest)
				pDest->Heal(fHealValue);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::StringKey()
//
//	PURPOSE:	Handle animation command
//
// ----------------------------------------------------------------------- //

void CProjectile::OnModelKey(ArgList* pArgList)
{
	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	//extract the first key
	char* pKey = pArgList->argv[0];

	//test
	if (!pKey) return;

	//handle FX key
	if (stricmp(pKey, WEAPON_KEY_FX) == 0)
	{
		HandleFXKey(pArgList);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPVFXMgr::HandleFXKey()
//
//	PURPOSE:	Handle fx model key
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleFXKey(ArgList* pArgList)
{
	if (!pArgList) return;
	if (pArgList->argc < 3 || !pArgList->argv[1] || !pArgList->argv[2]) return;

	char* pFXName = pArgList->argv[1];
	char* pFXState = pArgList->argv[2];

	// Turn on/off the necessary fx...

	if((stricmp(pFXName, "SPRITE_KEY") == 0))
	{
		uint32 dwUsrFlg = g_pLTServer->GetObjectUserFlags(m_hObject);
		if(stricmp(pFXState, "ON") == 0)
		{
			dwUsrFlg |= USRFLG_GLOW;
		}
		else
		{
			dwUsrFlg &= ~USRFLG_GLOW;
		}

		g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	*hWrite << m_hFiredFrom;

	g_pLTServer->WriteToMessageVector(hWrite, &m_vFlashPos);
	g_pLTServer->WriteToMessageByte(hWrite, m_nFlashSocket);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vFirePos);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vDir);
	g_pLTServer->WriteToMessageByte(hWrite, m_bObjectRemoved);
	g_pLTServer->WriteToMessageByte(hWrite, m_bDetonated);
	g_pLTServer->WriteToMessageByte(hWrite, m_nWeaponId);
	g_pLTServer->WriteToMessageByte(hWrite, m_nAmmoId);
	g_pLTServer->WriteToMessageByte(hWrite, m_nBarrelId);
	g_pLTServer->WriteToMessageByte(hWrite, m_eInstDamageType);
	g_pLTServer->WriteToMessageByte(hWrite, m_eProgDamageType);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fInstDamage);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fProgDamage);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fStartTime);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLifeTime);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fVelocity);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fRange);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fDamageMultiplier);

	//save object flags
	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	*hWrite << dwFlags;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	*hRead >> m_hFiredFrom;

	g_pLTServer->ReadFromMessageVector(hRead, &m_vFlashPos);
	m_nFlashSocket	= g_pLTServer->ReadFromMessageByte(hRead);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vFirePos);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vDir);
	m_bObjectRemoved	= g_pLTServer->ReadFromMessageByte(hRead);
	m_bDetonated		= g_pLTServer->ReadFromMessageByte(hRead);
	m_nWeaponId			= g_pLTServer->ReadFromMessageByte(hRead);
	m_nAmmoId			= g_pLTServer->ReadFromMessageByte(hRead);
	m_nBarrelId			= g_pLTServer->ReadFromMessageByte(hRead);
	m_eInstDamageType	= (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
	m_eProgDamageType	= (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
	m_fInstDamage		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fProgDamage		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fStartTime		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fLifeTime			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fVelocity			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fRange			= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fDamageMultiplier	= g_pLTServer->ReadFromMessageFloat(hRead);

	//get object flags
	uint32 dwFlags; *hRead >> dwFlags;
	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	m_pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	m_pAmmoData	  = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	m_pBarrelData = g_pWeaponMgr->GetBarrel(m_nBarrelId);

	if(m_pAmmoData->eType == PROJECTILE)
	{
		ObjectCreateStruct createStruct;
		createStruct.Clear();

		SAFE_STRCPY(createStruct.m_Filename, m_pAmmoData->pProjectileFX->szModel);
		SAFE_STRCPY(createStruct.m_SkinNames[0], m_pAmmoData->pProjectileFX->szSkin);

		g_pLTServer->Common()->SetObjectFilenames(m_hObject, &createStruct);

		g_pLTServer->ScaleObject(m_hObject, &(m_pAmmoData->pProjectileFX->vModelScale));

		g_pLTServer->Common()->GetModelAnimUserDims(m_hObject, &m_vDims, 0);

		m_vDims.x *= m_pAmmoData->pProjectileFX->vModelScale.x;
		m_vDims.y *= m_pAmmoData->pProjectileFX->vModelScale.y;
		m_vDims.z *= m_pAmmoData->pProjectileFX->vModelScale.z;

		g_pLTServer->SetObjectDims(m_hObject, &m_vDims);

		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
		g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
	}
}
