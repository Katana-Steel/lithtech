// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.cpp
//
// PURPOSE : Projectile classs - implementation
//
// CREATED : 10/3/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileTypes.h"
#include "ServerUtilities.h"
#include "DamageTypes.h"
#include "SurfaceFunctions.h"
#include "ServerSoundMgr.h"
#include "physics_lt.h"
#include "CVarTrack.h"
#include "CharacterHitBox.h"
#include "Character.h"
#include "DebugLineSystem.h"
#include "Spawner.h"
#include "ModelButeMgr.h"
#include "CharacterButeMgr.h"
#include "ObjectMsgs.h"
#include "CharacterFuncs.h"
#include "FXButeMgr.h"
#include "MsgIDs.h"
#include "SharedFXStructs.h"
#include "BodyProp.h"
#include "GameServerShell.h"
#include "PickupObject.h"
#include "MultiplayerMgr.h"


#define DEFAULT_GRENADE_DAMPEN_PERCENT	0.75f
#define DEFAULT_GRENADE_MIN_VELOCITY	10.0f

static CVarTrack g_vtGrenadeDampenPercent;
static CVarTrack g_vtGrenadeMinVelMag;

static CVarTrack g_vtSpearStickPercentage;
static CVarTrack g_cvarMaxSCTurnDot;

static CVarTrack g_vtSCTurnRate;

BEGIN_CLASS(CGrenade)
END_CLASS_DEFAULT_FLAGS(CGrenade, CProjectile, LTNULL, LTNULL, CF_HIDDEN)

BEGIN_CLASS(CProxGrenade)
END_CLASS_DEFAULT_FLAGS(CProxGrenade, CGrenade, LTNULL, LTNULL, CF_HIDDEN)

BEGIN_CLASS(CSpiderGrenade)
END_CLASS_DEFAULT_FLAGS(CSpiderGrenade, CGrenade, LTNULL, LTNULL, 0)

BEGIN_CLASS(CTrackingSADAR)
END_CLASS_DEFAULT_FLAGS(CTrackingSADAR, CProjectile, LTNULL, LTNULL, CF_HIDDEN)

BEGIN_CLASS(CShoulderCannon)
END_CLASS_DEFAULT_FLAGS(CShoulderCannon, CProjectile, LTNULL, LTNULL, CF_HIDDEN)

BEGIN_CLASS(CSpear)
END_CLASS_DEFAULT_FLAGS(CSpear, CProjectile, LTNULL, LTNULL, CF_HIDDEN)

BEGIN_CLASS(CFlare)
END_CLASS_DEFAULT_FLAGS(CFlare, CGrenade, LTNULL, LTNULL, CF_HIDDEN)

BEGIN_CLASS(CStickyHotbomb)
END_CLASS_DEFAULT_FLAGS(CStickyHotbomb, CGrenade, LTNULL, LTNULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::CGrenade
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGrenade::CGrenade() : CProjectile(OT_MODEL, 0, FLAG_CLIENTNONSOLID)
{
	SetDims(LTVector(5.0f, 5.0f, 5.0f));

	m_bSpinGrenade			= LTTRUE;
	m_bUpdating				= LTTRUE;

	m_fPitch	= 0.0f;
	m_fYaw		= 0.0f;
	m_fRoll		= 0.0f;
	m_fPitchVel	= 0.0f;
	m_fYawVel	= 0.0f;
	m_fRollVel	= 0.0f;

	m_hBounceSnd = LTNULL;
	m_eContainerCode = CC_NO_CONTAINER;
	m_eLastHitSurface = ST_UNKNOWN;

	ResetRotationVel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::~CGrenade
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGrenade::~CGrenade()
{
	if (m_hBounceSnd)
	{
		g_pLTServer->KillSound(m_hBounceSnd);
		m_hBounceSnd = LTNULL;
	}
}


void CGrenade::HandleTouch(HOBJECT hObj)
{
	if (!g_vtGrenadeDampenPercent.IsInitted())
	{
		g_vtGrenadeDampenPercent.Init(g_pLTServer, "GrenadeDampenPercent", LTNULL, DEFAULT_GRENADE_DAMPEN_PERCENT);
	}

	if (!g_vtGrenadeMinVelMag.IsInitted())
	{
		g_vtGrenadeMinVelMag.Init(g_pLTServer, "GrenadeMinVelMag", LTNULL, DEFAULT_GRENADE_MIN_VELOCITY);
	}


	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);
	m_eLastHitSurface = GetSurfaceType(info); 

	// Just go away if we hit the sky...
	if (m_eLastHitSurface == ST_SKY)
	{
		RemoveObject();
		return;
	}

	//test to see if we are hitting the shooter's hit box or the shooter

	LTVector vVel, vPos;
	g_pLTServer->GetVelocity(m_hObject, &vVel);
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(info.m_hObject))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(info.m_hObject);
		if (!pHitBox) return;

		if (pHitBox->GetModelObject() == GetFiredFrom()) return;
	}
	else if (IsCharacter(info.m_hObject))
	{
		if (info.m_hObject == GetFiredFrom()) return;

		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pChar->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(info.m_hObject);

		if(!(dwFlags & FLAG_SOLID))
		{
			// Check for water here
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// See if we got bogus collision info...
			if(info.m_Plane.m_Normal.MagSqr() == 0)
				info.m_hPoly = INVALID_HPOLY;

			// See if we should stick to the object we just hit...
			m_eLastHitSurface = GetSurfaceType(info); 

			if(m_eLastHitSurface == ST_LIQUID)
				Detonate(LTNULL);

			return;
		}
	}

	if(pHitBox)
		if (pHitBox->DidProjectileImpact(this))
		{
			// This is the object that we really hit...
			hObj = pHitBox->GetModelObject();
		}
		else
		{
			return;
		}

	uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	LTBOOL bIsValidPoly = info.m_hPoly != INVALID_HPOLY;
	LTBOOL bIsWorld = (LT_YES == g_pServerDE->Physics()->IsWorldObject(hObj));

	// See if we got bogus collision info...
	if(bIsValidPoly && info.m_Plane.m_Normal.MagSqr() == 0)
		bIsValidPoly = LTFALSE;


	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	if (!bIsValidPoly)
	{
		//we hit a solig thing and its not the world so detonate!
		Detonate(hObj);
		return;
	}


	// See if we are impacting on liquid...

	LTBOOL bEnteringLiquid = LTFALSE;
	uint16 code;
	if (g_pInterface->GetContainerCode(hObj, &code))
	{
		if (IsLiquid((ContainerCode)code))
		{
			bEnteringLiquid = LTTRUE;
		}
	}


	// Do the bounce, if the object we hit isn't liquid...


	if (!bEnteringLiquid)
	{
		vVel += (info.m_vStopVel * 2.0f);
	}


	// Dampen the grenade's new velocity based on the surface type...

	LTFLOAT fDampenPercent = g_vtGrenadeDampenPercent.GetFloat();

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);

	if (pSurf)
		fDampenPercent = (1.0f - pSurf->fHardness);

	// Play the bounce sound
	if(!m_hBounceSnd)
		m_hBounceSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "GrenBounce");

	fDampenPercent = fDampenPercent > 1.0f ? 1.0f : (fDampenPercent < 0.0f ? 0.0f : fDampenPercent);

	vVel *= (1.0f - fDampenPercent);


	// See if we should come to a rest...

	LTVector vTest = vVel;
	vTest.y = 0.0f;

	if (vTest.Mag() < g_vtGrenadeMinVelMag.GetFloat())
	{
		// If we're on the ground (or an object), stop movement...

		CollisionInfo standingInfo;
		g_pLTServer->GetStandingOn(m_hObject, &standingInfo);
				
		CollisionInfo* pInfo = standingInfo.m_hObject ? &standingInfo : &info;
		
		if (pInfo->m_hObject) 
		{
			// Don't stop on walls...

			if (pInfo->m_Plane.m_Normal.y > 0.75f)
			{
				vVel.Init();

				// Rotate to rest...

				RotateToRest();
			}
		}
	}


	// Reset rotation velocities due to the bounce...

	ResetRotationVel(&vVel, pSurf);


	// We need to subtact this out because the engine will add it back in,
	// kind of a kludge but necessary...
	
	vVel -= info.m_vStopVel;

	g_pLTServer->SetVelocity(m_hObject, &vVel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CGrenade::UpdateGrenade()
{
	// Update my container code...

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	m_eContainerCode = GetContainerCode(vPos);


	// Update grenade spin...

	if (m_bSpinGrenade && m_bUpdating)
	{
		uint32 nNetFlags;
		g_pLTServer->GetNetFlags(m_hObject, nNetFlags);
		g_pLTServer->SetNetFlags(m_hObject, nNetFlags | NETFLAG_ROTUNGUARANTEED);

		if (m_fPitchVel != 0 || m_fYawVel != 0 || m_fRollVel != 0)
		{
			m_fPitch += m_fPitchVel * g_pLTServer->GetFrameTime();
			m_fYaw   += m_fYawVel * g_pLTServer->GetFrameTime();
			m_fRoll  += m_fRollVel * g_pLTServer->GetFrameTime();

			LTRotation rRot;
			g_pLTServer->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
			g_pLTServer->SetObjectRotation(m_hObject, &rRot);
		}
	}


	// See if the bounce sound is done playing...

	if (m_hBounceSnd)
	{
		LTBOOL bIsDone;
		if (g_pLTServer->IsSoundDone(m_hBounceSnd, &bIsDone) != LT_OK || bIsDone)
		{
			g_pLTServer->KillSound(m_hBounceSnd);
			m_hBounceSnd = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CGrenade::RotateToRest()
{
	if (!m_bSpinGrenade) return;

	uint32 nNetFlags;
	g_pLTServer->GetNetFlags(m_hObject, nNetFlags);
	g_pLTServer->SetNetFlags(m_hObject, nNetFlags & ~NETFLAG_ROTUNGUARANTEED);

	LTRotation rRot;
	g_pLTServer->SetupEuler(&rRot, 0.0f, m_fYaw, 0.0f);
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	m_bUpdating = LTFALSE;
}
				

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::ResetRotationVel()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CGrenade::ResetRotationVel(LTVector* pvNewVel, SURFACE* pSurf)
{
	if (!m_bSpinGrenade) return;

	// (TO DO: base this on new velocity and surface hardness?)
	
	LTFLOAT fVal  = MATH_CIRCLE/2.0f;
	LTFLOAT fVal2 = MATH_CIRCLE;
	m_fPitchVel	 = GetRandom(-fVal, fVal);
	m_fYawVel	 = GetRandom(-fVal2, fVal2);
	m_fRollVel   = GetRandom(-fVal2, fVal2);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CGrenade::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			UpdateGrenade();
		}
		break;

		case MID_PRECREATE:
		{
			PostPropRead((ObjectCreateStruct*)pData);
		}
		break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CProjectile::Save(hWrite, dwSaveFlags);

	g_pLTServer->WriteToMessageByte(hWrite, m_bSpinGrenade);
	g_pLTServer->WriteToMessageByte(hWrite, m_bUpdating);
	g_pLTServer->WriteToMessageByte(hWrite, m_eContainerCode);
	g_pLTServer->WriteToMessageByte(hWrite, m_eLastHitSurface);

	g_pLTServer->WriteToMessageFloat(hWrite, m_fPitch);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fRoll);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fPitchVel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fYawVel);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fRollVel);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CProjectile::Load(hRead, dwLoadFlags);

	m_bSpinGrenade		= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_bUpdating			= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_eContainerCode	= (ContainerCode) g_pLTServer->ReadFromMessageByte(hRead);
	m_eLastHitSurface	= (SurfaceType) g_pLTServer->ReadFromMessageByte(hRead);

	m_fPitch	= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fYaw		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fRoll		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fPitchVel = g_pLTServer->ReadFromMessageFloat(hRead);
	m_fYawVel	= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fRollVel	= g_pLTServer->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::Setup()
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

void CProxGrenade::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	CGrenade::Setup(setup, info);

	if ( !GetAmmoClassData() ) return;

	m_pClassData = dynamic_cast<PROXCLASSDATA*>(GetAmmoClassData());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CProxGrenade::UpdateGrenade()
{
	CGrenade::UpdateGrenade();

	// If we're doing normal (in-air) updates, don't do any more
	// processing...

	if (m_bUpdating || !m_pClassData) return;

	// Waiting to go boom...

	if (m_bActivated && m_DetonateTime.Stopped())
	{
		Detonate(LTNULL);
		return;
	}

	
	// See if it is time to arm yet...

	if (!m_bArmed && m_ArmTime.Stopped())
	{
		m_bArmed = LTTRUE;
	
		// Play armed sound...

		if (m_pClassData->szArmSound[0])
		{
			int nVolume = IsLiquid(m_eContainerCode) ? 50 : 100;
				
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			g_pServerSoundMgr->PlaySoundFromPos(vPos, m_pClassData->szArmSound, 
				(LTFLOAT)m_pClassData->nArmSndRadius, SOUNDPRIORITY_MISC_MEDIUM, 
				0, nVolume);
		}
	}


	// Is there anything close enough to cause us to go boom?

	if (!m_bActivated && m_bArmed)
	{		
		if ( !GetAmmoClassData() ) return;

		LTFLOAT fRadius = (LTFLOAT) m_pClassData->nActivateRadius;

		// NEED TO FIGURE OUT A BETTER WAY TO DO THIS!!!

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, fRadius);
		if (!pList) return;

		ObjectLink* pLink = pList->m_pFirstLink;
		while (pLink)
		{
			if (IsCharacter(pLink->m_hObject))
			{
				// Check to see that the character is active...
				uint32 nFlags = g_pLTServer->GetObjectFlags(pLink->m_hObject);
				if(nFlags & FLAG_VISIBLE)
				{
					m_bActivated = LTTRUE;

					m_DetonateTime.Start(m_pClassData->fActivateDelay);

					// Play activation sound...

					if (m_pClassData->szActivateSound[0])
					{
						int nVolume = IsLiquid(m_eContainerCode) ? 50 : 100;
							
						LTVector vPos;
						g_pLTServer->GetObjectPos(m_hObject, &vPos);

						g_pServerSoundMgr->PlaySoundFromPos(vPos, m_pClassData->szActivateSound, 
							(LTFLOAT) m_pClassData->nActivateSndRadius, 
							SOUNDPRIORITY_MISC_MEDIUM, 0, nVolume);
					}

					break;
				}
			}

			pLink = pLink->m_pNext;
		}
		g_pLTServer->RelinquishList(pList);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::HandleTouch
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CProxGrenade::HandleTouch(HOBJECT hObj)
{
	if (hObj == GetFiredFrom()) return;


	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		if (pHitBox->GetModelObject() == GetFiredFrom()) return;
	}
	else if (IsCharacter(hObj))
	{
		if (hObj == GetFiredFrom()) return;

		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pChar->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

		if(!(dwFlags & FLAG_SOLID))
		{
			// Check for water here
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// See if we got bogus collision info...
			if(info.m_Plane.m_Normal.MagSqr() == 0)
				info.m_hPoly = INVALID_HPOLY;

			// See if we should stick to the object we just hit...
			m_eLastHitSurface = GetSurfaceType(info); 

			if(m_eLastHitSurface == ST_LIQUID)
				Detonate(LTNULL);

			return;
		}
	}

	if(pHitBox)
		if (pHitBox->DidProjectileImpact(this))
		{
			// This is the object that we really hit...
			hObj = pHitBox->GetModelObject();
		}
		else
		{
			return;
		}

	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);
	// See if we should stick to the object we just hit...
	m_eLastHitSurface = GetSurfaceType(info); 

	// Just go away if we hit the sky...
	if (m_eLastHitSurface == ST_SKY)
	{
		RemoveObject();
		return;
	}

	if(info.m_Plane.m_Normal.MagSqr() == 0)
		info.m_hPoly = INVALID_HPOLY;

	// See if we should stick to the object we just hit...
	uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

	LTBOOL bIsValidPoly = info.m_hPoly != INVALID_HPOLY;
	LTBOOL bIsWorld = (LT_YES == g_pServerDE->Physics()->IsWorldObject(hObj) && (m_eLastHitSurface != ST_INVISIBLE));



	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	if (!bIsValidPoly)
	{
		//we hit a solig thing and its not the world so detonate!
		Detonate(hObj);
		return;
	}

	if (m_eLastHitSurface != ST_LIQUID)
	{
		// Need to set velocity to 0.0f but account for stoping vel
		// being added back in...
		LTVector vMyPos, vVel;

		//get positions
		g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
		g_pLTServer->GetVelocity(m_hObject, &vVel);;

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From	= vMyPos;
		IQuery.m_To		= vMyPos+vVel;

		IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;

		if(g_pServerDE->IntersectSegment(&IQuery, &IInfo))
		{
			g_pLTServer->SetObjectPos(m_hObject, &IInfo.m_Point);
		}
	


		CollisionInfo info;
		g_pLTServer->GetLastCollision(&info);

		if(!bIsWorld)
		{
			if(++m_nBounceCount > 2)
			{
				Detonate(info.m_hObject);
				return;
			}

			vVel += (info.m_vStopVel * 2.0f);

			LTFLOAT fDampenPercent = g_vtGrenadeDampenPercent.GetFloat();

			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
			if (pSurf)
			{
				fDampenPercent = (1.0f - pSurf->fHardness);
			}

			// Play the bounce sound
			if(!m_hBounceSnd)
				m_hBounceSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "GrenBounce");

			fDampenPercent = fDampenPercent > 1.0f ? 1.0f : (fDampenPercent < 0.0f ? 0.0f : fDampenPercent);

			vVel *= (1.0f - fDampenPercent);
		}
		else
		{
			// Turn off gravity, solid, and touch notify....
			vVel = LTVector(0,0,0);
			uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			dwFlags &= ~(FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);
			g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
			// Rotate to rest...

			m_vSurfaceNormal.Init(0, 1, 0);
			m_vSurfaceNormal = info.m_Plane.m_Normal;

			RotateToRest();
		}

		vVel -= info.m_vStopVel;
		g_pLTServer->SetVelocity(m_hObject, &vVel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CProxGrenade::RotateToRest()
{
	CGrenade::RotateToRest();

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	// Okay, rotated based on the surface normal we're on...
	
	g_pLTServer->AlignRotation(&rRot, &m_vSurfaceNormal, LTNULL);
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	// Arm the grenade after a few...

	m_ArmTime.Start(m_pClassData->fArmDelay);

	m_bUpdating = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProxGrenade::Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact)
{
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

		// Maybe we stored a normal...
		vNormal = m_vSurfaceNormal;

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
		if (GetFiredFrom())
			hDamager = GetFiredFrom();
		else
			hDamager = m_hObject;

		ImpactDamageObject(hDamager, hObj);
	}

	AddImpact(hObj, LTVector(0,0,0), vPos, vNormal, eType);

	// Remove projectile from world...

	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CProxGrenade::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CGrenade::Save(hWrite, dwSaveFlags);

	m_DetonateTime.Save(hWrite);
	m_ArmTime.Save(hWrite);
	
	g_pLTServer->WriteToMessageVector(hWrite, &m_vSurfaceNormal);
	g_pLTServer->WriteToMessageByte(hWrite, m_bArmed);
	g_pLTServer->WriteToMessageByte(hWrite, m_bActivated);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CProxGrenade::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CGrenade::Load(hRead, dwLoadFlags);

	m_DetonateTime.Load(hRead);
	m_ArmTime.Load(hRead);
	
	g_pLTServer->ReadFromMessageVector(hRead, &m_vSurfaceNormal);

	m_bArmed		= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_bActivated	= (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

	m_pClassData = dynamic_cast<PROXCLASSDATA*>(GetAmmoClassData());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CSpiderGrenade::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct *)pData;
			m_bDEditPlaced = !pStruct->m_UserData;
			break;
		}

		case MID_INITIALUPDATE:
		{
			Setup();
		}
		break;

		default : break;
	}

	return CGrenade::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::Setup()
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::Setup()
{
	if(!m_bDEditPlaced) return;

	ProjectileSetup Setup;
	WFireInfo		Info;

	Info.bAltFire = LTFALSE;
	Info.fPerturbR = 0.0f;
	Info.fPerturbU = 0.0f;
	Info.hFiredFrom = m_hObject;
	Info.hTestObj = LTNULL;
	Info.nSeed	= 0;
	g_pLTServer->GetObjectPos(m_hObject, &Info.vFirePos);
	Info.vFlashPos = Info.vFirePos;
	Info.vPath = LTVector(0.0f, -1.0f, 0.0f);

	WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Grenade_Launcher");
	if(!pWeap) return;
	Setup.nWeaponId = pWeap->nId;
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo("Spider_Grenades_Ammo");
	if(!pAmmo) return;
	Setup.nAmmoId = pAmmo->nId;
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel("Spider_Grenades_Barrel");
	if(!pBarrel) return;
	Setup.nBarrelId = pBarrel->nId;

	if(!pAmmo->pProjectileFX) return;
	Setup.fLifeTime = pAmmo->pProjectileFX->fLifeTime;
	m_pClassData = dynamic_cast<SPIDERCLASSDATA*>(pAmmo->pProjectileFX->pClassData);
	if(!m_pClassData) return;

	CGrenade::Setup(Setup, Info);

	//since this is placed do not turn on gravity
	cm_CharacterMovement.Init(g_pLTServer, m_hObject);
	cm_CharacterMovement.SetCharacterButes("SpiderBomb", LTFALSE);
	cm_CharacterMovement.SetObjectFlags(	OFT_Flags,
											FLAG_TOUCH_NOTIFY |
											FLAG_RAYHIT | 
											FLAG_VISIBLE | 
											FLAG_MODELKEYS | FLAG_POINTCOLLIDE | FLAG_NOSLIDING,
											LTTRUE );

	LTRotation rRot;
	LTVector vUp, vR, vF;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	g_pLTServer->GetRotationVectors(&rRot, &vUp, &vR, &vF);
	
//TVector vUp = cm_CharacterMovement.GetUp();
	cm_CharacterMovement.SetVelocity(LTVector (0.0f, 0.0f, 0.0f));//-vUp * 2000);
	m_bArmed		= LTTRUE;
	m_bSpinGrenade	= LTFALSE;
	m_bUpdating		= LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::Setup()
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	CGrenade::Setup(setup, info);

	if ( !GetAmmoClassData() ) return;

	m_pClassData = dynamic_cast<SPIDERCLASSDATA*>(GetAmmoClassData());

	cm_CharacterMovement.Init(g_pLTServer, m_hObject);
	cm_CharacterMovement.SetCharacterButes("SpiderBomb", LTFALSE);
	cm_CharacterMovement.SetObjectFlags(	OFT_Flags,
											FLAG_TOUCH_NOTIFY |
											FLAG_GRAVITY | 
											FLAG_RAYHIT | 
											FLAG_VISIBLE | 
											FLAG_MODELKEYS | FLAG_POINTCOLLIDE | FLAG_NOSLIDING,
											LTTRUE );

	//set flight rotation
	LTVector vR = cm_CharacterMovement.GetRight();

	LTRotation rRot = cm_CharacterMovement.GetRot();

	g_pLTServer->RotateAroundAxis(&rRot, &vR, MATH_HALFPI);

	cm_CharacterMovement.SetRot(rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::UpdateGrenade()
{
	CGrenade::UpdateGrenade();

	// If we're doing normal (in-air) updates, don't do any more
	// processing...

	if (m_bUpdating || !m_pClassData) return;

	// Waiting to go boom...

	if (m_bActivated) 
	{
		Pursue();
		return;
	}

	
	// See if it is time to arm yet...

	if (!m_bArmed && m_ArmTime.Stopped())
	{
		m_bArmed = LTTRUE;
	
		// Play armed sound...

		if (m_pClassData->szArmSound[0])
		{
			int nVolume = IsLiquid(m_eContainerCode) ? 50 : 100;
				
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			g_pServerSoundMgr->PlaySoundFromPos(vPos, m_pClassData->szArmSound, 
				(LTFLOAT)m_pClassData->nArmSndRadius, SOUNDPRIORITY_MISC_MEDIUM, 
				0, nVolume);
		}
	}


	// Is there anything close enough to cause us to go boom?

	if (!m_bActivated && m_bArmed)
	{		
		if ( !GetAmmoClassData() ) return;

		LTFLOAT fRadius = (LTFLOAT) m_pClassData->nActivateRadius;

		// NEED TO FIGURE OUT A BETTER WAY TO DO THIS!!!

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, fRadius);
		if (!pList) return;

		ObjectLink* pLink = pList->m_pFirstLink;
		while (pLink)
		{
			if (IsCharacter(pLink->m_hObject) && TestLOS(pLink->m_hObject))
			{
				// Check to see that the character is active...
				uint32 nFlags = g_pLTServer->GetObjectFlags(pLink->m_hObject);
				if(nFlags & FLAG_VISIBLE)
				{
					m_bActivated = LTTRUE;

					// Time stamp
					m_fTimeActivated = g_pLTServer->GetTime();

					// Play activation sound...

					LTVector vPos, vTargetPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);
					g_pLTServer->GetObjectPos(pLink->m_hObject, &vTargetPos);

					if (m_pClassData->szActivateSound[0])
					{
						int nVolume = IsLiquid(m_eContainerCode) ? 50 : 100;
							

						g_pServerSoundMgr->PlaySoundFromPos(vPos, m_pClassData->szActivateSound, 
							(LTFLOAT) m_pClassData->nActivateSndRadius, 
							SOUNDPRIORITY_MISC_MEDIUM, 0, nVolume);
					}
					
					//lock on target big boy!
					m_hTarget = pLink->m_hObject;

					LTRotation rRot;
					LTVector vF = vTargetPos - vPos;
					vF.Norm();

					// This gets the right vector
					vF = vF.Cross(m_vSurfaceNormal);

					// This gets a new forward that is perpendicular to the normal.
	//				vF = vF.Cross(m_vSurfaceNormal);
				
					g_pLTServer->AlignRotation(&rRot, &vF, &m_vSurfaceNormal);

					cm_CharacterMovement.SetRot(rRot);


					// Turn on gravity, solid, and touch notify....

					uint32 dwFlags = cm_CharacterMovement.GetObjectFlags(OFT_Flags);
					dwFlags |= (FLAG_GRAVITY | FLAG_SOLID);
					cm_CharacterMovement.SetObjectFlags(OFT_Flags, dwFlags, LTTRUE);

					//play open animation
					HMODELANIM nOpenAni	= g_pServerDE->GetAnimIndex(m_hObject, "Open");
					g_pServerDE->SetModelAnimation(m_hObject, nOpenAni);
					g_pServerDE->SetModelLooping(m_hObject, LTFALSE);
					g_pServerDE->ResetModelAnimation(m_hObject);  // Start from beginning

					break;
				}
			}

			pLink = pLink->m_pNext;
		}
		g_pLTServer->RelinquishList(pList);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::HandleTouch
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::HandleTouch(HOBJECT hObj)
{
	if(m_bActivated) return;

	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);

	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(info.m_hObject))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(info.m_hObject);
		if (!pHitBox) return;

		if (pHitBox->GetModelObject() == GetFiredFrom()) return;
	}
	else if (IsCharacter(hObj))
	{
		if (hObj == GetFiredFrom()) return;

		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pChar->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(info.m_hObject);

		if(!(dwFlags & FLAG_SOLID))
		{
			// Check for water here
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// See if we got bogus collision info...
			if(info.m_Plane.m_Normal.MagSqr() == 0)
				info.m_hPoly = INVALID_HPOLY;

			// See if we should stick to the object we just hit...
			m_eLastHitSurface = GetSurfaceType(info); 

			if(m_eLastHitSurface == ST_LIQUID)
				Detonate(LTNULL);

			return;
		}
	}

	if(pHitBox)
		if (pHitBox->DidProjectileImpact(this))
		{
			// This is the object that we really hit...
			hObj = pHitBox->GetModelObject();
		}
		else
		{
			return;
		}

	// See if we should stick to the object we just hit...
	m_eLastHitSurface = GetSurfaceType(info); 

	// Just go away if we hit the sky...
	if (m_eLastHitSurface == ST_SKY)
	{
		RemoveObject();
		return;
	}

	if(info.m_Plane.m_Normal.MagSqr() == 0)
		info.m_hPoly = INVALID_HPOLY;

	// See if we should stick to the object we just hit...
	uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

	LTBOOL bIsValidPoly = info.m_hPoly != INVALID_HPOLY;
	LTBOOL bIsWorld = (LT_YES == g_pServerDE->Physics()->IsWorldObject(hObj) && (m_eLastHitSurface != ST_INVISIBLE));


	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	if (!bIsValidPoly)
	{
		//we hit a solig thing and its not the world so detonate!
		Detonate(hObj);
		return;
	}

	if (m_eLastHitSurface != ST_LIQUID)
	{
		// Need to set velocity to 0.0f but account for stoping vel
		// being added back in...
		LTVector vMyPos, vVel;

		//get positions
		g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
		g_pLTServer->GetVelocity(m_hObject, &vVel);;

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From	= vMyPos;
		IQuery.m_To		= vMyPos+vVel;

		IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;

		if(g_pServerDE->IntersectSegment(&IQuery, &IInfo))
		{
			g_pLTServer->SetObjectPos(m_hObject, &IInfo.m_Point);
		}
	

		CollisionInfo info;
		g_pLTServer->GetLastCollision(&info);

		if(!bIsWorld)
		{
			if(++m_nBounceCount > 2)
			{
				Detonate(info.m_hObject);
				return;
			}

			vVel += (info.m_vStopVel * 2.0f);

			LTFLOAT fDampenPercent = g_vtGrenadeDampenPercent.GetFloat();

			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
			if (pSurf)
			{
				fDampenPercent = (1.0f - pSurf->fHardness);
			}

			// Play the bounce sound
			if(!m_hBounceSnd)
				m_hBounceSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "GrenBounce");

			fDampenPercent = fDampenPercent > 1.0f ? 1.0f : (fDampenPercent < 0.0f ? 0.0f : fDampenPercent);

			vVel *= (1.0f - fDampenPercent);
		}
		else
		{
			// Turn off gravity, solid, and touch notify....
			vVel = LTVector(0,0,0);

			m_vSurfaceNormal.Init(0, 1, 0);
			m_vSurfaceNormal = info.m_Plane.m_Normal;

			// Turn off gravity, solid, and touch notify....

			uint32 dwFlags = cm_CharacterMovement.GetObjectFlags(OFT_Flags);
			dwFlags &= ~(FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);
			cm_CharacterMovement.SetObjectFlags(OFT_Flags, dwFlags, LTTRUE);

			// Setup our flags to do wall walking
			cm_CharacterMovement.SetObjectFlags(OFT_Flags2, FLAG2_SPHEREPHYSICS | FLAG2_ORIENTMOVEMENT, LTTRUE);
			cm_CharacterMovement.SetControlFlags(CM_FLAG_WALLWALK);


			// Rotate to rest...

			RotateToRest();
		}

		vVel -= info.m_vStopVel;
		g_pLTServer->SetVelocity(m_hObject, &vVel);

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::RotateToRest()
{
	CGrenade::RotateToRest();

	LTRotation rRot;
	LTVector vF = cm_CharacterMovement.GetForward();

	vF = m_vSurfaceNormal.Cross(vF);

//	cm_CharacterMovement.GetRot(rRot);

	// Okay, rotated based on the surface normal we're on...
	
	g_pLTServer->AlignRotation(&rRot, &vF, &m_vSurfaceNormal);

//	g_pLTServer->GetRotationVectors(&rRot, &vNull, &vR, &vNull);

//	g_pLTServer->RotateAroundAxis(&rRot, &vR, MATH_HALFPI);

	cm_CharacterMovement.SetRot(rRot);

	m_ArmTime.Start(m_pClassData->fArmDelay);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::Pursue()
//
//	PURPOSE:	Update the grenade in pursue state...
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::Pursue()
{
	if(CheckForDetonation())
		//we went BOOM!
		return;

	//see if we need to change to walk anim
	uint32 dwState	= g_pServerDE->GetModelPlaybackState(m_hObject);
	if(dwState & MS_PLAYDONE)
	{
		HMODELANIM nWalkAni	= g_pServerDE->GetAnimIndex(m_hObject, "Walk");
		g_pServerDE->SetModelAnimation(m_hObject, nWalkAni);
		g_pServerDE->SetModelLooping(m_hObject, LTTRUE);
		g_pServerDE->ResetModelAnimation(m_hObject);  // Start from beginning
	}


	//control flags
	uint32 dwFlags= (CM_FLAG_FORWARD | CM_FLAG_WALLWALK );

	if(!TestLOS(m_hTarget))
	{
		if(m_bLostTarget)
		{
			if(g_pLTServer->GetTime()-m_fLostTargetTime > 3)
			{
				//time to re-set back to inactive mode
				m_bActivated = LTFALSE;
				m_hTarget = LTNULL;

				uint32 dwFlags = cm_CharacterMovement.GetObjectFlags(OFT_Flags);
				dwFlags &= ~(FLAG_GRAVITY | FLAG_SOLID);
				cm_CharacterMovement.SetObjectFlags(OFT_Flags, dwFlags, LTTRUE);

				cm_CharacterMovement.SetVelocity(LTVector(0,0,0));
				cm_CharacterMovement.SetControlFlags(CM_FLAG_NONE);

				HMODELANIM nCloseAni	= g_pServerDE->GetAnimIndex(m_hObject, "Close");
				g_pServerDE->SetModelAnimation(m_hObject, nCloseAni);
				g_pServerDE->SetModelLooping(m_hObject, LTFALSE);
				g_pServerDE->ResetModelAnimation(m_hObject);  // Start from beginning

				return;
			}
		}
		else
		{
			//first time lost target
			m_bLostTarget = LTTRUE;
			m_fLostTargetTime = g_pLTServer->GetTime();
		}
	}
	else
	{
		//be sure to re-set this flag
		m_bLostTarget = LTFALSE;

		//time to update our status
		LTVector vMyPos, vTargetPos, vU, vDims;

		//get positions
		g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
		g_pLTServer->GetObjectPos(m_hTarget, &vTargetPos);

		//get targets rotation
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hTarget, &rRot);

		LTVector vTargR, vTargU, vTargF;
		g_pMathLT->GetRotationVectors(rRot, vTargR, vTargU, vTargF);

		g_pInterface->GetObjectDims(m_hTarget,&vDims);

		// Adjust to the feet
		vDims *= vTargU;
		vTargetPos.y -= vDims.y;

//		g_pLTServer->CPrint("Up = %.2f", vDims.y);

		//get vector to target
		LTVector vToTarget = vTargetPos-vMyPos;

		vU = cm_CharacterMovement.GetUp();
		
		vToTarget -= vU * (vU.Dot(vToTarget));

		LTVector vR;
		vR = cm_CharacterMovement.GetRight();

		dwFlags |= vR.Dot(vToTarget)>0 ? CM_FLAG_RIGHT : CM_FLAG_LEFT;
	}
	cm_CharacterMovement.SetControlFlags(dwFlags);
	
	cm_CharacterMovement.Update(g_pLTServer->GetFrameTime());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::CheckForDetonation()
//
//	PURPOSE:	See if it's time to go BOOM!...
//
// ----------------------------------------------------------------------- //

LTBOOL CSpiderGrenade::CheckForDetonation()
{
	if ( !GetAmmoClassData() ) return LTFALSE;

	LTFLOAT fRadius = (LTFLOAT) m_pClassData->nDetonateRadius;

	// see if it is time, don't chase forever
	if(g_pLTServer->GetTime()-m_fTimeActivated > 5.0f)
	{
		Detonate(LTNULL);
		return LTTRUE;
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	ObjectList* pList = g_pLTServer->FindObjectsTouchingSphere(&vPos, fRadius);
	if (!pList) return LTFALSE;

	ObjectLink* pLink = pList->m_pFirstLink;
	while (pLink)
	{
		if (IsCharacter(pLink->m_hObject))
		{
			Detonate(LTNULL);
			g_pLTServer->RelinquishList(pList);
			return LTTRUE;
		}

		pLink = pLink->m_pNext;
	}
	g_pLTServer->RelinquishList(pList);

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::TestLOS
//
//	PURPOSE:	Test line of sight to object
//
// ----------------------------------------------------------------------- //

LTBOOL CSpiderGrenade::TestLOS(HOBJECT hTarget)
{
	if(!hTarget)
		return LTFALSE;

	//time to update our status
	LTVector vMyPos, vTargetPos;

	CCharacter * pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hTarget));

	if(!pChar)
		return LTFALSE;

	//get positions
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
	g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vMyPos;
	IQuery.m_To		= vTargetPos;

	IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;

	g_pServerDE->IntersectSegment(&IQuery, &IInfo);

	return IInfo.m_hObject == pChar->GetHitBox();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CGrenade::Save(hWrite, dwSaveFlags);

	*hWrite << m_vSurfaceNormal;
	*hWrite << m_bArmed;
	*hWrite << m_bActivated;
	*hWrite << m_hTarget;
	hWrite->WriteFloat(m_fLostTargetTime - g_pLTServer->GetTime());
	*hWrite << m_bLostTarget;
	*hWrite << m_bDEditPlaced;
	hWrite->WriteFloat(m_fTimeActivated - g_pLTServer->GetTime());

	m_ArmTime.Save(hWrite);

	cm_CharacterMovement.Save(hWrite);
	//save object flags
	uint32 dwFlags = cm_CharacterMovement.GetObjectFlags(OFT_Flags);
	*hWrite << dwFlags;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpiderGrenade::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CSpiderGrenade::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CGrenade::Load(hRead, dwLoadFlags);

	*hRead >> m_vSurfaceNormal;
	*hRead >> m_bArmed;
	*hRead >> m_bActivated;
	*hRead >> m_hTarget;
	m_fLostTargetTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_bLostTarget;
	*hRead >> m_bDEditPlaced;
	m_fTimeActivated = hRead->ReadFloat() + g_pLTServer->GetTime();

	m_ArmTime.Load(hRead);
	
	cm_CharacterMovement.Load(hRead);
	//get object flags
	uint32 dwFlags; *hRead >> dwFlags;

	cm_CharacterMovement.Init(g_pLTServer, m_hObject);
	cm_CharacterMovement.SetCharacterButes("SpiderBomb", LTFALSE);
	cm_CharacterMovement.SetObjectFlags( OFT_Flags, dwFlags, LTTRUE );

	m_pClassData = dynamic_cast<SPIDERCLASSDATA*>(GetAmmoClassData());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackingSADAR::Setup()
//
//	PURPOSE:	Setup the SADAR...
//
// ----------------------------------------------------------------------- //

void CTrackingSADAR::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	CProjectile::Setup(setup, info);

	if ( !GetAmmoClassData() ) return;

	m_hTarget = info.hTestObj;

	m_pClassData = dynamic_cast<TRACKINGSADARCLASSDATA*>(GetAmmoClassData());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackingSADAR::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

void CTrackingSADAR::Update()
{
	// If we're doing normal (in-air) updates, don't do any more
	// processing...

	if (!m_pClassData) return;

	// Waiting to go boom...

	if(TestLOS(m_hTarget))
	{
		//we can still see the target, track it!

		//time to update our status
		LTVector vMyPos, vTargetPos;

		//get positions
		g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

		ModelLT* pModelLT   = g_pLTServer->GetModelLT();
		HMODELNODE hNode;
		if (pModelLT->GetNode(m_hTarget, "Torso_u_node", hNode) == LT_OK)
		{
			LTransform	tTransform;

			if (pModelLT->GetNodeTransform(m_hTarget, hNode, tTransform, LTTRUE) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				pTransLT->GetPos(tTransform, vTargetPos);
			}
		}
		else
			g_pLTServer->GetObjectPos(m_hTarget, &vTargetPos);

		//get my velocity
		LTVector vVel;
		g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

		//get my rotation
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		LTVector vR, vU, vF;
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

		//get vector to target
		LTVector vToTarget = vTargetPos-vMyPos;

		//store my velocity
		LTFLOAT fVelocity = vVel.Mag();

		//normalize the vectors
		vVel.Norm();
		vToTarget.Norm();
		
		//get the angle to the target
		LTFLOAT fDot = vVel.Dot(vToTarget);

		if(fDot < 0)
		{
			m_hTarget = LTNULL;
			return;
		}

		// Bounds check
		fDot = fDot < -1.0f ? -1.0f : fDot;
		fDot = fDot > 1.0f ? 1.0f : fDot;

		fDot = (LTFLOAT) acos(fDot);

		LTFLOAT fMaxTurn = MATH_DEGREES_TO_RADIANS(m_pClassData->fTurnRadius * g_pLTServer->GetFrameTime());

		//get my desired rotation
		LTRotation rDesired, rDest;
		g_pMathLT->AlignRotation(rDesired, vToTarget, vU);

		LTFLOAT fRatio = (LTFLOAT)fabs(fMaxTurn/fDot);

		g_pMathLT->InterpolateRotation(rDest, rRot, rDesired, fRatio > 1.0f ? 1.0f : fRatio);

		g_pLTServer->SetObjectRotation(m_hObject, &rDest);

		g_pMathLT->GetRotationVectors(rDest, vR, vU, vF);

		vF *= fVelocity;
		g_pPhysicsLT->SetVelocity(m_hObject, &vF);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackingSADAR::TestLOS
//
//	PURPOSE:	Test line of sight to object
//
// ----------------------------------------------------------------------- //

LTBOOL CTrackingSADAR::TestLOS(HOBJECT hTarget)
{
	if(!hTarget)
		return LTFALSE;

	//time to update our status
	LTVector vMyPos, vTargetPos;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hTarget));

	if(!pChar)
		return LTFALSE;

	//get positions
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
	g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vMyPos;
	IQuery.m_To		= vTargetPos;

	IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;

	g_pServerDE->IntersectSegment(&IQuery, &IInfo);

	return IInfo.m_hObject == pChar->GetHitBox();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackingSADAR::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CTrackingSADAR::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
		}
		break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackingSADAR::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CTrackingSADAR::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CProjectile::Save(hWrite, dwSaveFlags);

	*hWrite << m_hTarget;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackingSADAR::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CTrackingSADAR::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CProjectile::Load(hRead, dwLoadFlags);

	*hRead >> m_hTarget;

	m_pClassData = dynamic_cast<TRACKINGSADARCLASSDATA*>(GetAmmoClassData());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShoulderCannon::Setup()
//
//	PURPOSE:	Setup the Shoulder Cannon projectile...
//
// ----------------------------------------------------------------------- //

void CShoulderCannon::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	if (!g_cvarMaxSCTurnDot.IsInitted())
	{
        g_cvarMaxSCTurnDot.Init(g_pLTServer, "MaxSCTurn", LTNULL, 0.866f);
	}

	if (!g_vtSCTurnRate.IsInitted())
	{
        g_vtSCTurnRate.Init(g_pLTServer, "SCTurnRate", LTNULL, 180.0f);
	}

	CProjectile::Setup(setup, info);

	m_hTarget = info.hTestObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShoulderCannon::Update()
//
//	PURPOSE:	Update the projectile...
//
// ----------------------------------------------------------------------- //

void CShoulderCannon::Update()
{
	// Waiting to go boom...

	if(TestLOS(m_hTarget))
	{
		//we can still see the target, track it!

		//time to update our status
		LTVector vMyPos, vTargetPos;

		//get positions
		g_pLTServer->GetObjectPos(m_hObject, &vMyPos);

		ModelLT* pModelLT   = g_pLTServer->GetModelLT();
		HMODELNODE hNode;
		if (pModelLT->GetNode(m_hTarget, "Torso_u_node", hNode) == LT_OK)
		{
			LTransform	tTransform;

			if (pModelLT->GetNodeTransform(m_hTarget, hNode, tTransform, LTTRUE) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				pTransLT->GetPos(tTransform, vTargetPos);
			}
		}
		else
			g_pLTServer->GetObjectPos(m_hTarget, &vTargetPos);

		//get my velocity
		LTVector vVel;
		g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

		//get my rotation
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		LTVector vR, vU, vF;
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

		//get vector to target
		LTVector vToTarget = vTargetPos-vMyPos;

		//store my velocity
		LTFLOAT fVelocity = vVel.Mag();

		//normalize the vectors
		vVel.Norm();
		vToTarget.Norm();
		
		//get the angle to the target
		LTFLOAT fDot = vVel.Dot(vToTarget);

		if(fDot < 0)
		{
			m_hTarget = LTNULL;
			return;
		}

		// Bounds check
		fDot = fDot < -1.0f ? -1.0f : fDot;
		fDot = fDot > 1.0f ? 1.0f : fDot;

		fDot = (LTFLOAT) acos(fDot);

		// Get frame time
		LTFLOAT fFrameTime = g_pLTServer->GetFrameTime();

		// Cap it...
		if(fFrameTime > 0.1f)
			fFrameTime = 0.1f;

		LTFLOAT fMaxTurn = MATH_DEGREES_TO_RADIANS(g_vtSCTurnRate.GetFloat() * fFrameTime);

		//get my desired rotation
		LTRotation rDesired, rDest;
		g_pMathLT->AlignRotation(rDesired, vToTarget, vU);

		LTFLOAT fRatio = (LTFLOAT)fabs(fMaxTurn/fDot);

		g_pMathLT->InterpolateRotation(rDest, rRot, rDesired, fRatio > 1.0f ? 1.0f : fRatio);

		g_pMathLT->GetRotationVectors(rDest, vR, vU, vF);

		LTFLOAT fMaxTurnDot = vF.Dot(GetDir());

		// final check for our max turn
		if(fMaxTurnDot > g_cvarMaxSCTurnDot.GetFloat())
		{
			g_pLTServer->SetObjectRotation(m_hObject, &rDest);

			vF *= fVelocity;
			g_pPhysicsLT->SetVelocity(m_hObject, &vF);
		}
	}
	else
	{
		// No line of sight so lose the target....
		m_hTarget = LTNULL;
	}
}

LTBOOL LOSFilterFn(HOBJECT hObject, void *pUserData)
{
	if (!hObject || !g_pInterface) return LTFALSE;

	HOBJECT hFiredFrom = *((HOBJECT*)pUserData);

	return hFiredFrom == hObject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShoulderCannon::TestLOS
//
//	PURPOSE:	Test line of sight to object
//
// ----------------------------------------------------------------------- //

LTBOOL CShoulderCannon::TestLOS(HOBJECT hTarget)
{
	if(!hTarget)
		return LTFALSE;

	//time to update our status
	LTVector vMyPos, vTargetPos;

	CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hTarget));

	if(!pChar)
		return LTFALSE;

	//get positions
	g_pLTServer->GetObjectPos(m_hObject, &vMyPos);
	g_pLTServer->GetObjectPos(hTarget, &vTargetPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vMyPos;
	IQuery.m_To		= vTargetPos;

	IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;

	g_pServerDE->IntersectSegment(&IQuery, &IInfo);

	char* pName = g_pLTServer->GetObjectName(IInfo.m_hObject);

	return (IInfo.m_hObject == pChar->GetHitBox() || IInfo.m_hObject == hTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShoulderCannon::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CShoulderCannon::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
			g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
		}
		break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShoulderCannon::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CShoulderCannon::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CProjectile::Save(hWrite,dwSaveFlags);

	*hWrite << m_hTarget;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CShoulderCannon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CShoulderCannon::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CProjectile::Load(hRead,dwLoadFlags);

	*hRead >> m_hTarget;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::CSpear
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSpear::CSpear() : CProjectile()
{
	m_hHead		= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::~CSpear
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSpear::~CSpear()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::HandleImpact
//
//	PURPOSE:	Handle impact
//
// ----------------------------------------------------------------------- //

void CSpear::HandleImpact(HOBJECT hObj)
{
	// Ignore pickups...
	PickupObject* pPU = dynamic_cast<PickupObject*>(g_pLTServer->HandleToObject(hObj));

	if(pPU)
		return;

	//see if we are post-headshot
	if(m_hHead)
	{
		//handle post-headshot impact
		HandlePostHeadShotImpact(hObj);
		return;
	}

	//do the normal impact to set last node hit
	if(IsBody(hObj))
	{
		Detonate(hObj);
		return;
	}

	if (!g_vtSpearStickPercentage.IsInitted())
	{
        g_vtSpearStickPercentage.Init(g_pLTServer, "SpearStickPercent", LTNULL, 0.9f);
	}

	CollisionInfo info;
    g_pLTServer->GetLastCollision(&info);

    LTVector vPos, vVel, vCurVel, vP0, vP1;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);


	// Should we break the spear?

	enum SpearAction
	{
		eSpearActionBreak,
		eSpearActionStickWorld,
		eSpearActionStickAI,
		eSpearActionStickPlayer,
	};

	SpearAction eSpearAction = eSpearActionStickWorld;

	// Only break every time if shot from an AI...
	if (IsAI(GetFiredFrom()))
	{
		eSpearAction = eSpearActionBreak;
	}
	else 
	{
		if (g_pPhysicsLT->IsWorldObject(hObj) == LT_YES)
		{
			if(GetSurfaceType(info) == ST_INVISIBLE)
				eSpearAction = eSpearActionBreak;
			else
			{
 				// Calculate where we really hit the world...

				g_pLTServer->GetVelocity(m_hObject, &vVel);

				vP1 = vPos;
				vCurVel = vVel * g_pLTServer->GetFrameTime();
				vP0 = vP1 - vCurVel;
				vP1 += vCurVel;

				LTFLOAT fDot1 = VEC_DOT(info.m_Plane.m_Normal, vP0) - info.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(info.m_Plane.m_Normal, vP1) - info.m_Plane.m_Dist;

				if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
					VEC_LERP(vPos, vP0, vP1, fPercent);
				}

				// Set our new "real" pos...

				g_pLTServer->SetObjectPos(m_hObject, &vPos);

				if(g_pGameServerShell->GetGameType().IsMultiplayer())
					eSpearAction = eSpearActionBreak;
				else
					eSpearAction = eSpearActionStickWorld;
			}
		}
		else
		{
			if (IsMoveable(hObj))
			{
				if (IsAI(hObj))
				{
					// Attach to a AI
					eSpearAction = eSpearActionStickAI;
				}
				else if (IsPlayer(hObj))
				{
					// Attach to a Player
					eSpearAction = eSpearActionStickPlayer;
				}
				else
				{
					// Could probably come up with a way to attach to moveable
					// non-character objects (like doors), but it is much easier
					// to just break it ;)...

					eSpearAction = eSpearActionBreak;
				}
			}
		}
	}
	// Create the Spear powerup...

	LTVector vScale = GetAmmoScale();

	// Make sure the spear sticks out a little ways...

	vVel.Norm();
	vPos -= (vVel * vScale.z/2.0f);

	if (eSpearActionStickWorld == eSpearAction)
	{
		char pStr[255];
		sprintf(pStr,"PickupObject Pickup Ammo_Spears_Stuck_Wall");

		BaseClass* pClass = SpawnObject( pStr, vPos, rRot);

		if(pClass)
		{
			g_pLTServer->ScaleObject(pClass->m_hObject, &vScale);

			LTVector vDims;
			g_pLTServer->GetObjectDims(pClass->m_hObject, &vDims);
			vDims.x *= vScale.x;
			vDims.y *= vScale.y;
			vDims.z *= vScale.z;

			g_pLTServer->SetObjectDims(pClass->m_hObject, &vDims);
			g_pLTServer->AlignRotation(&rRot, &vVel, LTNULL);
			g_pLTServer->SetObjectRotation(pClass->m_hObject, &rRot);
			
			g_pLTServer->SetObjectUserFlags(pClass->m_hObject, g_pLTServer->GetObjectUserFlags(pClass->m_hObject) & ~USRFLG_GLOW);
			g_pLTServer->SetObjectFlags(pClass->m_hObject, g_pLTServer->GetObjectFlags(pClass->m_hObject) & ~FLAG_GRAVITY);

			PickupObject* pPU = dynamic_cast<PickupObject*>(pClass);

			if(pPU)
				pPU->SetAllowMovement(LTFALSE);
		}
	}


	if ( eSpearActionStickAI == eSpearAction || eSpearActionStickPlayer == eSpearAction )
	{
		// Attach it to the character
		CCharacter* pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));

		if(pCharacter && !pCharacter->IsDead())
		{
			if(g_pModelButeMgr->GetSkeletonNodeHitLocation(pCharacter->GetModelSkeleton(), pCharacter->GetModelNodeLastHit()) == HL_HEAD)
			{
				//Head shot!
				if(CreateHeadObject(pCharacter))
				{
					AdjustDamage(*pCharacter, pCharacter->GetModelNodeLastHit());
					ImpactDamageObject(GetFiredFrom(), pCharacter->m_hObject);
					//don't stop for anything but walls
					g_pLTServer->SetObjectFlags(m_hObject, g_pLTServer->GetObjectFlags(m_hObject) & ~FLAG_SOLID);

					// Play the evil sound!
					if( g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY && IsPlayer(GetFiredFrom()))
					{
						g_pServerSoundMgr->PlaySoundFromObject(GetFiredFrom(), "pred_spearhead");
					}
				}
				else
				{
					//wrong character type
					DEBRIS* pDebris = g_pDebrisMgr->GetDebris((char*)GetAmmoName());
					if (pDebris)
					{
						vVel.Norm();
						LTVector vNegVel = -vVel;
						CreatePropDebris(vPos, vNegVel, pDebris->nId);
					}
					CProjectile::Detonate(hObj);
				}
				return;
			}
			else
			{
				if(!IsPlayer(hObj))
				{
					char pStr[255];
					sprintf(pStr,"PickupObject Pickup Ammo_Spears_Stuck_Character");

					g_pLTServer->GetObjectPos(hObj, &vPos);
					BaseClass* pClass = SpawnObject( pStr, vPos, rRot);

					if(pClass)
					{
						g_pLTServer->ScaleObject(pClass->m_hObject, &vScale);

						LTVector vDims;
						g_pLTServer->GetObjectDims(pClass->m_hObject, &vDims);
						vDims.x *= vScale.x;
						vDims.y *= vScale.y;
						vDims.z *= vScale.z;

						g_pLTServer->SetObjectDims(pClass->m_hObject, &vDims);

						uint32 dwFlags =  g_pLTServer->GetObjectFlags(pClass->m_hObject);

						//required for the attachment, non-solid things don't like to be attached.
						dwFlags |= FLAG_SOLID;
						dwFlags &= ~FLAG_REMOVEIFOUTSIDE;

						if ( eSpearActionStickPlayer == eSpearAction )
						{
							dwFlags &= ~FLAG_VISIBLE;
						}
						//set the new flags
						g_pLTServer->SetObjectFlags(pClass->m_hObject, dwFlags);
						
						// Be sure we are not moving
						g_pLTServer->SetVelocity(pClass->m_hObject, &LTVector(0,0,0));

						// Attach the spear
						pCharacter->AddSpear(hObj, pClass->m_hObject, pCharacter->GetModelNodeLastHit(), rRot);
					}
				}
				else
					eSpearAction = eSpearActionBreak;
			}
		}
		else
			eSpearAction = eSpearActionBreak;

	}

	// If the surface is too hard, the spear will just break when
	// it hits it...

	SurfaceType eSurf = GetSurfaceType(info);
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurf);
	if ((eSpearActionBreak == eSpearAction) || (pSurf && pSurf->fHardness > 0.5))
	{
		// Create spear debris...

		DEBRIS* pDebris = g_pDebrisMgr->GetDebris((char*) GetAmmoName() );
		if (pDebris)
		{
			vVel.Norm();
			LTVector vNegVel = -vVel;
            CreatePropDebris(vPos, vNegVel, pDebris->nId);
		}

		//remove the spear
		CProjectile::Detonate(hObj);
		return;
	}


	//terminate the object here
	CProjectile::Detonate(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::Update
//
//	PURPOSE:	Handle impact
//
// ----------------------------------------------------------------------- //

void CSpear::Update()
{
	CProjectile::Update();

	if(m_hHead)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->SetObjectPos(m_hHead, &vPos);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::HandlePostHeadShotImpact
//
//	PURPOSE:	Handle hitting things after we have collecte a head =)
//
// ----------------------------------------------------------------------- //

void CSpear::HandlePostHeadShotImpact(HOBJECT hObj)
{
	CollisionInfo info;
    g_pLTServer->GetLastCollision(&info);

    LTVector vPos, vVel, vCurVel, vP0, vP1;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	if (g_pPhysicsLT->IsWorldObject(hObj) == LT_YES)
	{
 		// Calculate where we really hit the world...

		g_pLTServer->GetVelocity(m_hObject, &vVel);

		vP1 = vPos;
		vCurVel = vVel * g_pLTServer->GetFrameTime();
		vP0 = vP1 - vCurVel;
		vP1 += vCurVel;

		LTFLOAT fDot1 = VEC_DOT(info.m_Plane.m_Normal, vP0) - info.m_Plane.m_Dist;
		LTFLOAT fDot2 = VEC_DOT(info.m_Plane.m_Normal, vP1) - info.m_Plane.m_Dist;

		if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
		{
			vPos = vP1;
		}
		else
		{
			LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
			VEC_LERP(vPos, vP0, vP1, fPercent);
		}

		// Set our new "real" pos...

		g_pLTServer->SetObjectPos(m_hObject, &vPos);


		//now convert ourself to a power-up
		char pStr[255];
		if(g_pGameServerShell->GetGameType().IsMultiplayer())
			sprintf(pStr,"PickupObject Pickup Ammo_Spears_Stuck_Wall; ForceNoRespawn 1");
		else
			sprintf(pStr,"PickupObject Pickup Ammo_Spears_Stuck_Wall");

		BaseClass* pClass = SpawnObject(pStr, vPos, rRot);

		if(pClass)
		{
			LTVector vScale = GetAmmoScale();

			g_pLTServer->ScaleObject(pClass->m_hObject, &vScale);

			LTVector vDims;
			g_pLTServer->GetObjectDims(pClass->m_hObject, &vDims);
			vDims.x *= vScale.x;
			vDims.y *= vScale.y;
			vDims.z *= vScale.z;

			g_pLTServer->SetObjectDims(pClass->m_hObject, &vDims);
			g_pLTServer->AlignRotation(&rRot, &vVel, LTNULL);
			g_pLTServer->SetObjectUserFlags(pClass->m_hObject, g_pLTServer->GetObjectUserFlags(pClass->m_hObject) & ~USRFLG_GLOW);
			g_pLTServer->SetObjectFlags(pClass->m_hObject, g_pLTServer->GetObjectFlags(pClass->m_hObject) & ~FLAG_GRAVITY);

			PickupObject* pPU = dynamic_cast<PickupObject*>(pClass);

			if(pPU)
				pPU->SetAllowMovement(LTFALSE);
		}
		
		//now stop the head too
		LTVector vHeadDims;
		g_pLTServer->GetObjectDims(m_hHead, &vHeadDims);

		//move off the wall a bit
		vPos += (info.m_Plane.m_Normal * (vHeadDims.z/2.0f));

		g_pLTServer->SetObjectPos(m_hHead, &vPos);

		LTVector vSetVel(0, 0, 0);

		g_pLTServer->SetVelocity(m_hHead, &vSetVel);

		//create inter-object link
		if(pClass && m_hHead)
		{
			g_pLTServer->CreateInterObjectLink(m_hHead, pClass->m_hObject);
			BodyProp* pHead = dynamic_cast<BodyProp*>(g_pLTServer->HandleToObject(m_hHead));

			if(pHead)
				pHead->RemoveOnLinkBroken();
		}

		//terminate the object here
		CProjectile::Detonate(hObj);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::CreateHeadObject
//
//	PURPOSE:	Handle creating the head object
//
// ----------------------------------------------------------------------- //

LTBOOL CSpear::CreateHeadObject(CCharacter* pCharacter)
{
	if(g_pGameServerShell->LowViolence())
		return LTFALSE;

	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		// see if we are using location based damage
		if(g_pGameServerShell->GetGameInfo() && !g_pGameServerShell->GetGameInfo()->m_bLocationDamage)
			return LTFALSE;
	}


	//see if this is a predator or human
	const CharacterButes* pButes = pCharacter->GetButes();

	//only create a head if we are a human or synthetic
	if(IsHuman(pButes) || IsSynthetic(pButes))
	{
		HCLASS hClass = g_pServerDE->GetClass("BodyProp");
		if (!hClass) return LTFALSE;

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
		sprintf(theStruct.m_Name, "%s-Head", g_pLTServer->GetObjectName(m_hObject) );
#endif
		g_pCharacterButeMgr->GetDefaultFilenames(pCharacter->GetCharacter(),theStruct);

		LTVector vDims;
		g_pServerDE->GetObjectDims(pCharacter->m_hObject, &vDims);

		// Allocate an object...
		BodyProp* pHead = LTNULL;
		pHead = (BodyProp*)g_pServerDE->CreateObject(hClass, &theStruct);
		if (!pHead) 
			return LTFALSE;

		pHead->SetFadeAway(LTFALSE);

		// Don't update the gravity.
		pHead->SetUpdateGravity( LTFALSE );

		// Record our head...
		m_hHead = pHead->m_hObject;

		// Get the limb_head animation since it is centered on the head and 
		// has the proper dims we need.
		HMODELANIM	hAnim = g_pServerDE->GetAnimIndex(m_hHead, "limb_head");
		LTAnimTracker* pTracker;
		g_pLTServer->GetModelLT()->GetMainTracker(m_hHead, pTracker);
		g_pLTServer->GetModelLT()->SetCurAnim(pTracker, hAnim);
		g_pLTServer->Common()->GetModelAnimUserDims(m_hHead, &vDims, hAnim);
		g_pLTServer->SetObjectDims(m_hHead, &vDims);

		uint32 dwFlags = 0; 
		dwFlags |= FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_NOSLIDING ;	// Make sure we are visible
		dwFlags &= ~(FLAG_SOLID | FLAG_TOUCH_NOTIFY);					

		uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hHead);


		// Set our surface type...
		uint32 dwHeadUsrFlags = g_pLTServer->GetObjectUserFlags(m_hHead);
		SurfaceType nHeadSurfType = UserFlagToSurface(dwHeadUsrFlags);

		dwUsrFlags |= SurfaceToUserFlag(nHeadSurfType);


		// Get the location of the head node on this model... and set the head piece to that info
		HMODELNODE hNode;
		LTVector vPos;
		LTRotation rRot;
		g_pServerDE->GetModelLT()->GetNode(pCharacter->m_hObject, "Head_Node", hNode);

		if(hNode != INVALID_MODEL_NODE)
		{
			LTransform ltrans;
			g_pServerDE->GetModelLT()->GetNodeTransform(pCharacter->m_hObject, hNode, ltrans, LTTRUE);
			g_pServerDE->SetObjectPos(m_hHead, &ltrans.m_Pos);
		}
		else
		{
			g_pServerDE->GetObjectPos(pCharacter->m_hObject, &vPos);
			g_pServerDE->SetObjectPos(m_hHead, &vPos);
		}

		g_pServerDE->GetObjectRotation(pCharacter->m_hObject, &rRot);
		g_pServerDE->SetObjectRotation(m_hHead, &rRot);


		g_pServerDE->SetObjectUserFlags(m_hHead, dwUsrFlags);
		g_pServerDE->SetObjectFlags(m_hHead, dwFlags);

		//match our velocity to the spear
		LTVector vVel;
		g_pServerDE->GetVelocity(m_hObject, &vVel);
		g_pServerDE->SetVelocity(m_hHead, &vVel);

		//hide all pieces but the head
		HMODELPIECE hPiece;
		g_pLTServer->GetModelLT()->GetPiece(m_hHead, "Head", hPiece);

		//hide 'em all
		for(HMODELPIECE hTemp=0 ; hTemp<32 ; hTemp++)
			g_pLTServer->GetModelLT()->SetPieceHideStatus(m_hHead, hTemp, LTTRUE);

		//unhide head...
		g_pLTServer->GetModelLT()->SetPieceHideStatus(m_hHead, hPiece, LTFALSE);

		// Now try the sub pieces
		char pTempPiece[255];

		for(int i=1; hPiece != INVALID_MODEL_NODE; i++)
		{
			sprintf(pTempPiece, "Head_OBJ%d", i);

			if(g_pModelLT->GetPiece(m_hHead, pTempPiece, hPiece) == LT_OK)
				g_pModelLT->SetPieceHideStatus(m_hHead, hPiece, LTFALSE);	
		}

		//finally send a body prop message so we get the goody effects!
		BODYPROPCREATESTRUCT cs;
		cs.nCharacterSet = pCharacter->GetCharacter();
		cs.bBloodTrail = LTTRUE;
		cs.bLimbProp = LTTRUE;

		HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(pHead);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
		cs.Write(g_pLTServer, hMessage);
		g_pLTServer->EndMessage(hMessage);

		return LTTRUE;
	}
	else
	{
		return LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CSpear::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CProjectile::Save(hWrite, dwSaveFlags);

	//save the head's object
	*hWrite << m_hHead;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTrackingSADAR::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CSpear::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CProjectile::Load(hRead, dwLoadFlags);

	//load the head pointer
	*hRead >> m_hHead;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::CFlare
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlare::CFlare() : CGrenade()
{
	m_fRotation		= GetRandom(MATH_PI*5, MATH_PI*8);
	m_nBounceCount	= 0;
	m_bSetPos		= LTFALSE;	
	m_vNewPos.Init();		
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::~CFlare
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFlare::~CFlare()
{
	CCharacter *pChar = dynamic_cast<CCharacter *>(g_pLTServer->HandleToObject(GetFiredFrom()));
	if (pChar)
		pChar->RemoveFlare(this);	// maintain the flare list
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CFlare::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CGrenade::Save(hWrite, dwSaveFlags);

	*hWrite << m_fRotation;
	*hWrite << m_nBounceCount;
	*hWrite << m_bSetPos;
	*hWrite << m_vNewPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CFlare::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CGrenade::Load(hRead, dwLoadFlags);

	*hRead >> m_fRotation;
	*hRead >> m_nBounceCount;
	*hRead >> m_bSetPos;
	*hRead >> m_vNewPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::Detonate
//
//	PURPOSE:	Time to finish up...
//
// ----------------------------------------------------------------------- //

void CFlare::Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact)
{
	//now say goodbye
	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::Setup()
//
//	PURPOSE:	Setup the Hotbomb...
//
// ----------------------------------------------------------------------- //

void CFlare::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	CGrenade::Setup(setup, info);

	CCharacter *pChar = dynamic_cast<CCharacter *>(g_pLTServer->HandleToObject(GetFiredFrom()));
	if (pChar)
		pChar->AddFlare(this);	// maintain the flare list

	uint32 dwFlags;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags);
	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags | FLAG2_CYLINDERPHYSICS);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::HandleTouch
//
//	PURPOSE:	Handle hitting stuff
//
// ----------------------------------------------------------------------- //

void CFlare::HandleTouch(HOBJECT hObj)
{
	if (hObj == GetFiredFrom()) return;


	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		if (pHitBox->GetModelObject() == GetFiredFrom()) return;
	}
	else if (IsCharacter(hObj))
	{
		if (hObj == GetFiredFrom()) return;

	
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pChar->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

		if(!(dwFlags & FLAG_SOLID))
		{
			// Check for water here
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// See if we got bogus collision info...
			if(info.m_Plane.m_Normal.MagSqr() == 0)
				info.m_hPoly = INVALID_HPOLY;

			// See if we should stick to the object we just hit...
			m_eLastHitSurface = GetSurfaceType(info); 

			if(m_eLastHitSurface == ST_LIQUID)
				RemoveObject();

			return;
		}
	}

	if(pHitBox)
	{
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

	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);
	m_eLastHitSurface = GetSurfaceType(info); 

	// Just go away if we hit the sky...
	if (m_eLastHitSurface == ST_SKY)
	{
		RemoveObject();
		return;
	}


	// See if we got bogus collision info...
	if(info.m_Plane.m_Normal.MagSqr() == 0)
		info.m_hPoly = INVALID_HPOLY;

	// See if we should stick to the object we just hit...
	uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

	LTBOOL bIsValidPoly = info.m_hPoly != INVALID_HPOLY;
	LTBOOL bIsWorld = (LT_YES == g_pServerDE->Physics()->IsWorldObject(hObj) && (m_eLastHitSurface != ST_INVISIBLE));


	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	if (!bIsValidPoly)
	{
		//we hit a solid thing and its not the world so just keep going!
		return;
	}

	LTVector vVel;
	g_pLTServer->GetVelocity(m_hObject, &vVel);

	// Bounce or stay put
	if(!bIsWorld && m_nBounceCount < 4)
	{
		// Bounce
		vVel += (info.m_vStopVel * 2.0f);

		LTFLOAT fDampenPercent = g_vtGrenadeDampenPercent.GetFloat();

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
		if (pSurf)
		{
			fDampenPercent = (1.0f - pSurf->fHardness);
		}

		// Play the bounce sound
		if(!m_hBounceSnd)
			m_hBounceSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "GrenBounce");

		fDampenPercent = fDampenPercent > 1.0f ? 1.0f : (fDampenPercent < 0.0f ? 0.0f : fDampenPercent);

		vVel *= (1.0f - fDampenPercent);
		vVel -= info.m_vStopVel;
		g_pLTServer->SetVelocity(m_hObject, &vVel);

		++m_nBounceCount;

		return;
	}
	else
	{
		// Stay put
		vVel = LTVector(0,0,0);
		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
		dwFlags &= ~(FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);
		g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

		// Make sure we move off the wall a bit.
		m_bSetPos = LTTRUE;
		m_vNewPos += info.m_Plane.m_Normal;
	}

	// Finish up
	vVel -= info.m_vStopVel;
	g_pLTServer->SetVelocity(m_hObject, &vVel);

	// Rotate to rest...
	m_bSpinGrenade = LTFALSE;

	// play James' silly impact sound
	g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Flare_Impact");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlare::UpdateGrenade()
//
//	PURPOSE:	Update the flare...
//
// ----------------------------------------------------------------------- //

void CFlare::UpdateGrenade()
{
	// Update my container code...

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	m_eContainerCode = GetContainerCode(vPos);


	// Update grenade spin...

	if (m_bSpinGrenade)
	{
		LTFLOAT fTemp = m_fRotation * g_pLTServer->GetFrameTime();
		DVector vU, vR, vF;
		DRotation rRot;
		g_pServerDE->GetObjectRotation(m_hObject, &rRot);
		g_pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		g_pServerDE->RotateAroundAxis(&rRot, &vR, fTemp);
		g_pServerDE->SetObjectRotation(m_hObject, &rRot);
	}

	// See if we need to reset our position...
	if(m_bSetPos)
	{
		g_pLTServer->SetObjectPos(m_hObject, &(vPos + m_vNewPos));
		m_bSetPos = LTFALSE;
	}


	// See if the bounce sound is done playing...

	if (m_hBounceSnd)
	{
		LTBOOL bIsDone;
		if (g_pLTServer->IsSoundDone(m_hBounceSnd, &bIsDone) != LT_OK || bIsDone)
		{
			g_pLTServer->KillSound(m_hBounceSnd);
			m_hBounceSnd = LTNULL;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyHotbomb::Setup()
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

void CStickyHotbomb::Setup(const ProjectileSetup & setup, const WFireInfo & info)
{
	CGrenade::Setup(setup, info);

	m_bSpinGrenade	= LTFALSE;

	uint32 dwFlags;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags);
	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags | FLAG2_CYLINDERPHYSICS);

	//play powerup animation
	HMODELANIM nAni	= g_pServerDE->GetAnimIndex(m_hObject, "PowerUp");
	g_pServerDE->SetModelAnimation(m_hObject, nAni);
	g_pServerDE->SetModelLooping(m_hObject, LTFALSE);
	g_pServerDE->ResetModelAnimation(m_hObject);  // Start from beginning
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyHotbomb::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CStickyHotbomb::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				SAFE_STRCPY(pStruct->m_Name, "HotbombProjectile");
			}			
		}
		break;

		default : break;
	}

	return CGrenade::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerDetonate::EngineMessageFn
//
//	PURPOSE:	Handle going BOOM!
//
// ----------------------------------------------------------------------- //

void CStickyHotbomb::TriggerDetonate(HOBJECT hObj)
{
	// Only blow up our our owners bombs...
	if(hObj == GetFiredFrom())
		Detonate(LTNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::HandleTouch
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CStickyHotbomb::HandleTouch(HOBJECT hObj)
{
	if (hObj == GetFiredFrom()) return;


	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		if (pHitBox->GetModelObject() == GetFiredFrom()) return;
	}
	else if (IsCharacter(hObj))
	{
		if (hObj == GetFiredFrom()) return;

		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		HOBJECT hHitBox = pChar->GetHitBox();
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hHitBox);
		if (!pHitBox) return;
	}
	else if(IsBodyProp(hObj))
	{
		return;
	}
	else
	{
		uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

		if(!(dwFlags & FLAG_SOLID) && !(dwFlags & FLAG_RAYHIT))
		{
			// Check for water here
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// See if we got bogus collision info...
			if(info.m_Plane.m_Normal.MagSqr() == 0)
				info.m_hPoly = INVALID_HPOLY;

			// See if we should stick to the object we just hit...
			m_eLastHitSurface = GetSurfaceType(info); 

			if(m_eLastHitSurface == ST_LIQUID)
				Detonate(LTNULL);

			return;
		}
	}

	if(pHitBox)
	{
		if (pHitBox->DidProjectileImpact(this))
			// This is the object that we really hit...
			hObj = pHitBox->GetModelObject();
		else
			return;
	}

	CollisionInfo info;
	g_pLTServer->GetLastCollision(&info);
	m_eLastHitSurface = GetSurfaceType(info); 

	// Just go away if we hit the sky...
	if (m_eLastHitSurface == ST_SKY)
	{
		RemoveObject();
		return;
	}


	// See if we got bogus collision info...
	if(info.m_Plane.m_Normal.MagSqr() == 0)
		info.m_hPoly = INVALID_HPOLY;

	// See if we should stick to the object we just hit...
	uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(hObj);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

	LTBOOL bIsValidPoly = info.m_hPoly != INVALID_HPOLY;
	LTBOOL bIsWorld = (LT_YES == g_pServerDE->Physics()->IsWorldObject(hObj) && (m_eLastHitSurface != ST_INVISIBLE));



	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...

	if (!bIsValidPoly)
	{
		//we hit a solig thing and its not the world so detonate!
		Detonate(hObj);
		return;
	}

	if (m_eLastHitSurface != ST_LIQUID)
	{
		LTVector vVel;
		g_pLTServer->GetVelocity(m_hObject, &vVel);

		// Bounce or stay put
		if(!bIsWorld && m_nBounceCount < 4)
		{
			// Bounce
			vVel += (info.m_vStopVel * 2.0f);

			LTFLOAT fDampenPercent = g_vtGrenadeDampenPercent.GetFloat();

			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eLastHitSurface);
			if (pSurf)
			{
				fDampenPercent = (1.0f - pSurf->fHardness);
			}

			// Play the bounce sound
			if(!m_hBounceSnd)
				m_hBounceSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "GrenBounce");

			fDampenPercent = fDampenPercent > 1.0f ? 1.0f : (fDampenPercent < 0.0f ? 0.0f : fDampenPercent);

			vVel *= (1.0f - fDampenPercent);
			vVel -= info.m_vStopVel;
			g_pLTServer->SetVelocity(m_hObject, &vVel);

			++m_nBounceCount;

			return;
		}
		else
		{
			// Stay put
			vVel = LTVector(0,0,0);
			uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			dwFlags &= ~(FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);
			g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
		}

		// Finish up
		vVel -= info.m_vStopVel;
		g_pLTServer->SetVelocity(m_hObject, &vVel);

		// Rotate to rest...
		m_bSpinGrenade = LTFALSE;

		// play James' silly impact sound
		g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "DiscstickinWall");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyHotbomb::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CStickyHotbomb::RotateToRest()
{
	CGrenade::RotateToRest();

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	// Okay, rotated based on the surface normal we're on...
	
	g_pLTServer->AlignRotation(&rRot, &m_vSurfaceNormal, LTNULL);
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);

	m_bUpdating = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyHotbomb::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CStickyHotbomb::Detonate(HOBJECT hObj, LTBOOL bDidProjectileImpact)
{
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

		// Maybe we stored a normal...
		vNormal = m_vSurfaceNormal;

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
		if (GetFiredFrom())
			hDamager = GetFiredFrom();
		else
			hDamager = m_hObject;

		ImpactDamageObject(hDamager, hObj);
	}

	AddImpact(hObj, LTVector(0,0,0), vPos, vNormal, eType);

	// Remove projectile from world...

	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CStickyHotbomb::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CStickyHotbomb::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	CGrenade::Save(hWrite, dwSaveFlags);

	*hWrite << m_vSurfaceNormal;
	*hWrite << m_nBounceCount;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProxGrenade::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CStickyHotbomb::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	CGrenade::Load(hRead, dwLoadFlags);
	
	*hRead >> m_vSurfaceNormal;
	*hRead >> m_nBounceCount;
}
