// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 7/6/98
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ProjectileFX.h"
#include "GameClientShell.h"
#include "ParticleTrailFX.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "physics_lt.h"
#include "ClientWeaponUtils.h"
#include "ParticleTrailFX.h"
#include "WeaponFXTypes.h" 
#include "SurfaceFunctions.h"
#include "FireFx.h"
#include "FXButeMgr.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack g_cvarFlareFlickerRate;
VarTrack g_cvarFlareLightRadius;
VarTrack g_cvarFlareLightReducePercent;
VarTrack g_cvarFlareLightSpriteReducePercent;

extern LTBOOL g_nFlicker1[10];
extern LTBOOL g_nFlicker2[10];
extern LTBOOL g_nFlicker3[10];
extern LTBOOL g_nFlicker4[10];
extern LTBOOL g_nFlicker5[10];
extern LTBOOL g_nFlicker6[10];
extern LTBOOL g_nFlicker7[10];
extern LTBOOL g_nFlicker8[10];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Init
//
//	PURPOSE:	Init the projectile system fx
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return DFALSE;
	if (!hMessage) return DFALSE;

	PROJECTILECREATESTRUCT proj;

	proj.hServerObj = hServObj;
	proj.nWeaponId	= g_pClientDE->ReadFromMessageByte(hMessage);
	proj.nAmmoId	= g_pClientDE->ReadFromMessageByte(hMessage);
	proj.nShooterId	= g_pClientDE->ReadFromMessageByte(hMessage);

	if (!g_cvarFlareFlickerRate.IsInitted())
	{
		g_cvarFlareFlickerRate.Init(g_pLTClient, "FlareFlickerRate", NULL, 0.01f);
	}

	if (!g_cvarFlareLightReducePercent.IsInitted())
	{
		g_cvarFlareLightReducePercent.Init(g_pLTClient, "FlareLightReducePercent", NULL, 0.01f);
	}

	if (!g_cvarFlareLightSpriteReducePercent.IsInitted())
	{
		g_cvarFlareLightSpriteReducePercent.Init(g_pLTClient, "FlareLightSpriteReducePercent", NULL, 0.1f);
	}

	return Init(&proj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Init
//
//	PURPOSE:	Init the projectile fx
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	PROJECTILECREATESTRUCT* pCS = (PROJECTILECREATESTRUCT*)psfxCreateStruct;

	m_nWeaponId		= pCS->nWeaponId;
	m_nAmmoId		= pCS->nAmmoId;
	m_nShooterId	= pCS->nShooterId;
	m_bLocal		= pCS->bLocal;
	m_bAltFire		= pCS->bAltFire;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!pAmmo || !pAmmo->pProjectileFX)
	{
		return DFALSE;
	}

	m_pProjectileFX = pAmmo->pProjectileFX;
	m_nFX = m_pProjectileFX->nFlags;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::CreateObject(CClientDE* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return DFALSE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return DFALSE;

	DBYTE nDetailLevel = pSettings->SpecialFXSetting();

	DVector vPos;
	DRotation rRot;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_vFirePos = vPos;

	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	DVector vVel, vU, vR, vF;
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	WEAPON *pWep = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (pWep)
	{
		if (strcmp(pWep->szName, "Flare_Pouch_Weapon")==0)
		{
			CreateFlareFlame(vPos);
		}
	}

	if (nDetailLevel != RS_LOW)
	{
		if (m_nFX & PFX_PARTICLETRAIL)
		{
			CreateSmokeTrail(vPos, rRot);
		}
		
		if (m_nFX & PFX_LIGHT)
		{
			CreateLight(vPos, rRot);
		}
	}

	if (m_nFX & PFX_FLARE)
	{
		CreateFlare(vPos, rRot);
	}
	
	if (m_nFX & PFX_FLYSOUND)
	{
		CreateFlyingSound(vPos, rRot);	
	}

	if (m_nFX & PFX_SCALEFX)
	{
		CreateToggleScaleFX();	
	}

	// Do client-side projectiles in multiplayer games...

/*	if (g_pGameClientShell->IsMultiplayerGame())
	{
		// Set the velocity of the "server" object if it is really just a local
		// object...

		if (m_bLocal)
		{
			m_vFirePos = vPos;
			m_vPath = vF;

			m_fStartTime = m_pClientDE->GetTime();

			// Special case of adjusting the projectile speed...

			DFLOAT fVel = (DFLOAT) m_pProjectileFX->nVelocity;
			if (m_bAltFire)
			{
				fVel = (DFLOAT) m_pProjectileFX->nAltVelocity;
			}

			DFLOAT fMultiplier = 1.0f;
			if (m_pClientDE->GetSConValueFloat("MissileSpeed", fMultiplier) != DE_NOTFOUND)
			{
				fVel *= fMultiplier;
			}

			VEC_MULSCALAR(vVel, vF, fVel);
			m_pClientDE->Physics()->SetVelocity(m_hServerObject, &vVel);
		}
	}
*/
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Update
//
//	PURPOSE:	Update the weapon fx
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::Update()
{
	if (!m_pClientDE) return DFALSE;

//	if (g_pGameClientShell->IsMultiplayerGame() && m_hServerObject)
//	{
//		// If this is a local fx, we control the position of the "server object"...
//
//		if (m_bLocal)
//		{
//			if (!MoveServerObj())
//			{
//				Detonate(DNULL);
//			
//				// Remove the "server" object...
//
//				m_pClientDE->DeleteObject(m_hServerObject);
//				m_hServerObject = DNULL;
//				WantRemove(DTRUE);
//			}
//		}
//	}

	if (IsWaitingForRemove())
	{
		RemoveFX(LTTRUE);
		return DFALSE;
	}

	// Update fx positions...

	DRotation rRot;
	DVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	if (m_hFlare)
	{
		m_pClientDE->SetObjectPos(m_hFlare, &vPos);
		m_pClientDE->SetObjectRotation(m_hFlare, &rRot);
	}

	if (m_hLight)
	{
		m_pClientDE->SetObjectPos(m_hLight, &vPos);
		m_pClientDE->SetObjectRotation(m_hLight, &rRot);

		// Only update the light if this is a flare...

		if (m_pFlareFlame)
		{
			UpdateFlareLight();
		}
	}

	if (m_hFlyingSound)
	{
		m_pClientDE->SetSoundPosition(m_hFlyingSound, &vPos);
	}

	if (m_pFlareFlame)
	{
		m_pFlareFlame->Update();
	}

	if (m_pToggleScaleFX)
	{
		UpdateScaleFX();
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::MoveServerObj
//
//	PURPOSE:	Update mover
//
// ----------------------------------------------------------------------- //

DBOOL CProjectileFX::MoveServerObj()
{	
	if (!m_pClientDE || !m_bLocal || !m_hServerObject || !g_pWeaponMgr) return DFALSE;

	ILTClientPhysics* pPhysicsLT = (ILTClientPhysics*)m_pClientDE->Physics();
	if (!pPhysicsLT) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// If we didn't hit anything we're done...

	if (fTime >= (m_fStartTime + m_pProjectileFX->fLifeTime)) 
	{
		return DFALSE;
	} 
		
	DFLOAT fFrameTime = m_pClientDE->GetFrameTime();

	// Zero out the acceleration to start with.
	
	DVector zeroVec;
	zeroVec.Init();
	pPhysicsLT->SetAcceleration(m_hServerObject, &zeroVec);

	DBOOL bRet = DTRUE;
	MoveInfo info;

	info.m_hObject  = m_hServerObject;
	info.m_dt		= fFrameTime;
	pPhysicsLT->UpdateMovement(&info);

	if (info.m_Offset.MagSqr() > 0.01f)
	{
		DVector vDiff, vNewPos, vCurPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);
		vNewPos = vCurPos + info.m_Offset;
		pPhysicsLT->MoveObject(m_hServerObject, &vNewPos, 0);

		vDiff = vCurPos - vNewPos;
		if (vDiff.MagSqr() < 5.0f)
		{
			bRet = DFALSE;
		}
	}
	else
	{
		bRet = DFALSE;
	}
	
	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateSmokeTrail
//
//	PURPOSE:	Create a smoke trail special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateSmokeTrail(DVector & vPos, DRotation & rRot)
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr || !g_pGameClientShell || !m_hServerObject || !m_pProjectileFX) return;

	PTCREATESTRUCT pt;
	pt.hServerObj	= m_hServerObject;
	pt.nType		= m_pProjectileFX->nParticleTrail;
	pt.nUWType		= m_pProjectileFX->nUWParticleTrail;

	// Create this particle trail segment
	psfxMgr->CreateSFX(SFX_PARTICLETRAIL_ID, &pt);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlare
//
//	PURPOSE:	Create a flare special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlare(DVector & vPos, DRotation & rRot)
{
	if (!m_pClientDE || !m_hServerObject) return;

	ObjectCreateStruct createStruct;
	createStruct.Clear();

	if (!m_pProjectileFX->szFlareSprite[0]) return;

	SAFE_STRCPY(createStruct.m_Filename, m_pProjectileFX->szFlareSprite);
	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE;
	createStruct.m_Pos = vPos;
	createStruct.m_Rotation = rRot;

	m_hFlare = m_pClientDE->CreateObject(&createStruct);
	if (!m_hFlare) return;

	DFLOAT fScale = m_pProjectileFX->fFlareScale;
	DVector vScale(fScale, fScale, 1.0f);
	m_pClientDE->SetObjectScale(m_hFlare, &vScale);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateLight
//
//	PURPOSE:	Create a light special fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateLight(DVector & vPos, DRotation & rRot)
{
	if (!m_pClientDE || !m_hServerObject) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	// Light backfacing polies if we're the flare object...
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;
	createStruct.m_Pos = vPos;

	m_hLight = m_pClientDE->CreateObject(&createStruct);
	if (!m_hLight) return; 

	DVector vColor = m_pProjectileFX->vLightColor;
	DFLOAT fRadius = (DFLOAT) m_pProjectileFX->nLightRadius;

	if (!g_cvarFlareLightRadius.IsInitted())
	{
		g_cvarFlareLightRadius.Init(m_pClientDE, "FlareLightRadius", LTNULL, fRadius);
	}

	m_pClientDE->SetLightColor(m_hLight, vColor.x, vColor.y, vColor.z);
	m_pClientDE->SetLightRadius(m_hLight, fRadius);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlyingSound
//
//	PURPOSE:	Create the flying sound
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlyingSound(DVector & vPos, DRotation & rRot)
{
	if (!m_pClientDE || m_hFlyingSound || !g_pWeaponMgr) return;

	char buf[MAX_CS_FILENAME_LEN];
	buf[0] = '\0';

	DFLOAT fRadius = (DFLOAT) m_pProjectileFX->nSoundRadius;
	if (m_pProjectileFX->szSound[0] && fRadius > 0.0f)
	{
		m_hFlyingSound = g_pClientSoundMgr->PlaySoundFromPos(vPos, m_pProjectileFX->szSound, 
			fRadius, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateToggleScaleFX
//
//	PURPOSE:	Create the toggle scale FX
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateToggleScaleFX()
{
	if (!m_pClientDE || !m_pProjectileFX || m_pToggleScaleFX || !g_pWeaponMgr) return;

	HMODELSOCKET hSocket;

	if (!m_pProjectileFX->szSocket[0]) return;

	//get the socket handle
	if (g_pModelLT->GetSocket(m_hServerObject, m_pProjectileFX->szSocket, hSocket) != LT_OK)
	{
		g_pLTClient->CPrint("ERROR: CProjectileFX::Init() %s is an Invalid socket!", m_pProjectileFX->szSocket);
		return;
	}

	if (!m_pProjectileFX->szToggleScaleFX[0]) return;


	//get the scale fx data
	CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(m_pProjectileFX->szToggleScaleFX);

	if (!pScaleFX || !m_hServerObject || hSocket == INVALID_MODEL_SOCKET) return;

	LTVector vPos;
	LTRotation rRot;
	LTransform transform;
    if (g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) != LT_OK) return;

	g_pTransLT->Get(transform, vPos, rRot);

	m_vFlareSpriteScale = pScaleFX->vFinalScale;

	// now create the FX.  New it up first so it's not added to the SFXMgr.
	if( !m_pToggleScaleFX )
		m_pToggleScaleFX = new CBaseScaleFX;
	m_pToggleScaleFX = g_pFXButeMgr->CreateScaleFX(pScaleFX, vPos,
        LTVector(0, 0, 0), LTNULL, LTNULL, (CBaseScaleFX *)m_pToggleScaleFX);

	m_hScaleSocket = hSocket;

	if(!m_pToggleScaleFX) return;

	HOBJECT hFX = m_pToggleScaleFX->GetObject();

	if (!hFX)
	{
		delete m_pToggleScaleFX;
		m_hScaleSocket = INVALID_MODEL_SOCKET;
		return;
	}

	// Hide object in portals...

    uint32 dwFlags2;
    g_pLTClient->Common()->GetObjectFlags(hFX, OFT_Flags2, dwFlags2);
	dwFlags2 |= FLAG2_PORTALINVISIBLE;
    g_pLTClient->Common()->SetObjectFlags(hFX, OFT_Flags2, dwFlags2);

    uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);

	//turn off flaga
	dwFlags &= ~FLAG_VISIBLE;

    g_pLTClient->SetObjectFlags(hFX, dwFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::CreateFlareFlame
//
//	PURPOSE:	Create the flare flame
//
// ----------------------------------------------------------------------- //

void CProjectileFX::CreateFlareFlame(DVector const & vPos)
{
	// [KLS] 9/6/01 - Updated to be a little safer...
	FIREOBJFX* pFireObjFX = g_pFXButeMgr->GetFireObjFX("Fire_Flare");
	if (pFireObjFX)
	{
		//create the fire effect
		FIRECREATESTRUCT firestruct;

		firestruct.hServerObj	= m_hServerObject;
		firestruct.nFireObjFXId	= pFireObjFX->nId;

		m_pFlareFlame = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_FIRE_ID, &firestruct);
	}
	// [KLS] End 9/6/01 changes
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::RemoveFX
//
//	PURPOSE:	Remove all fx
//
// ----------------------------------------------------------------------- //

void CProjectileFX::RemoveFX(LTBOOL bOnDetonate)
{
	if (!g_pGameClientShell || !m_pClientDE) return;

	if(m_pFlareFlame)
	{
		m_pFlareFlame->WantRemove();
		m_pFlareFlame = LTNULL;
	}

	if(m_pToggleScaleFX)
	{
		delete m_pToggleScaleFX;
		m_pToggleScaleFX = LTNULL;
	}

	if (m_hFlare)
	{
		m_pClientDE->DeleteObject(m_hFlare);
		m_hFlare = DNULL;
	}

	if (m_hLight)
	{
		m_pClientDE->DeleteObject(m_hLight);
		m_hLight = DNULL;
	}

	if (m_hFlyingSound)
	{
		m_pClientDE->KillSound(m_hFlyingSound);
		m_hFlyingSound = DNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectileFX::HandleTouch(CollisionInfo *pInfo, float forceMag)
{
	if (!m_pClientDE || !pInfo || !pInfo->m_hObject || !g_pGameClientShell) return;

	 // Let it get out of our bounding box...

	PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();

	if (pMovement)
	{
		// Don't colide with the move mgr object...

		HLOCALOBJ hMoveObj = pMovement->GetObject();
		if (pInfo->m_hObject == hMoveObj) return;

		// Don't colide with the player object...

		HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();
		if (pInfo->m_hObject == hPlayerObj) return;
	}


	// See if we want to impact on this object...

	DDWORD dwUsrFlags;
	m_pClientDE->GetObjectUserFlags(pInfo->m_hObject, &dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	DBOOL bIsWorld = (LT_YES == m_pClientDE->Physics()->IsWorldObject(pInfo->m_hObject));

	// Don't impact on non-solid objects...

	DDWORD dwFlags = m_pClientDE->GetObjectFlags(pInfo->m_hObject);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID)) return;


	// See if we hit the sky...

	if (bIsWorld)
	{
		SurfaceType eType = GetSurfaceType(*pInfo);

		if (eType == ST_SKY)
		{
			WantRemove(DTRUE);
			return;
		}
	}

//	Detonate(pInfo);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::UpdateScaleFX()
//
//	PURPOSE:	Update the scale FX
//
// ----------------------------------------------------------------------- //

void CProjectileFX::UpdateScaleFX()
{
	// Update the scale fx...

	LTransform transform;
    LTVector vPos;
    LTRotation rRot;
	HOBJECT hFX;

    g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	if (m_hScaleSocket != INVALID_MODEL_SOCKET)
	{
		if (g_pModelLT->GetSocketTransform(m_hServerObject,
            m_hScaleSocket, transform, LTTRUE) == LT_OK)
		{
			g_pTransLT->Get(transform, vPos, rRot);
		}

		hFX = m_pToggleScaleFX->GetObject();
		if (!hFX) return;

        g_pLTClient->SetObjectPos(hFX, &vPos, LTTRUE);
        g_pLTClient->SetObjectRotation(hFX, &rRot);

		m_pToggleScaleFX->Update();

		// Show/Hide object if on/off...

        uint32 dwFlags = g_pLTClient->GetObjectFlags(hFX);

        uint32 dwUsrFlags;
		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);

		if (dwUsrFlags & USRFLG_GLOW)
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else
		{
			dwFlags &= ~FLAG_VISIBLE;
		}

		// [KLS] 9/6/01 - If this projectile is associated with a flare, 
		// set the toggle scale fx flags accordingly...
		if (m_pFlareFlame)
		{
			dwFlags |= FLAG_VISIBLE;
			dwFlags |= FLAG_GLOWSPRITE;
			dwFlags |= FLAG_SPRITEBIAS;
		}
		// [KLS] End 9/6/01 changes

        g_pLTClient->SetObjectFlags(hFX, dwFlags);
	}
}
/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectileFX::Detonate(CollisionInfo* pInfo)
{
	if (!m_pClientDE || m_bDetonated) return;

	m_bDetonated = DTRUE;

	SurfaceType eType = ST_UNKNOWN;

	DVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	// Determine the normal of the surface we are impacting on...

	DVector vNormal(0.0f, 1.0f, 0.0f);

	if (pInfo)
	{
		eType = GetSurfaceType(*pInfo);

		if (pInfo->m_hPoly)
		{
			vNormal = pInfo->m_Plane.m_Normal;

			DRotation rRot;
			m_pClientDE->AlignRotation(&rRot, &vNormal, DNULL);
			m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);

			// Calculate where we really hit the plane...

			DVector vVel, vP0, vP1;
			m_pClientDE->Physics()->GetVelocity(m_hServerObject, &vVel);

			vP1 = vPos;
			vVel *= m_pClientDE->GetFrameTime();
			vP0 = vP1 - vVel;

			DFLOAT fDot1 = DIST_TO_PLANE(vP0, pInfo->m_Plane);
			DFLOAT fDot2 = DIST_TO_PLANE(vP1, pInfo->m_Plane);

			if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
			{
				vPos = vP1;
			}
			else
			{
				DFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
				VEC_LERP(vPos, vP0, vP1, fPercent);
			}
		}
	}
	else
	{
		// Since pInfo was null, this means the projectile's lifetime was up,
		// so we just blow-up in the air.

		eType = ST_AIR; 
	}


	HOBJECT hObj = !!pInfo ? pInfo->m_hObject : DNULL;
	::AddLocalImpactFX(hObj, g_pClientDE->GetClientObject(), m_vFirePos, vPos, vNormal, eType, m_vPath, 
					   m_nWeaponId, m_nAmmoId, m_nBarrelId, 0);

	WantRemove(DTRUE);
}

*/
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectileFX::UpdateFlareLight()
//
//	PURPOSE:	Update the light's color
//
// ----------------------------------------------------------------------- //

void CProjectileFX::UpdateFlareLight()
{
	if (!m_hLight || !m_pFlareFlame || !m_pProjectileFX) return;

	LTVector vColor = m_pProjectileFX->vLightColor;
	LTVector vScale = m_vFlareSpriteScale;

	LTFLOAT fFlickerTime = g_pLTClient->GetTime() - m_fFlickerTime;

	if (fFlickerTime > g_cvarFlareFlickerRate.GetFloat())
	{
		m_nFlickerPattern = GetRandom(1, 8);
		m_fFlickerTime = g_pLTClient->GetTime();
	}

	LTFLOAT fFlickerScale = fFlickerTime / g_cvarFlareFlickerRate.GetFloat();
	int nFlickIndex = (int)(fFlickerScale * 10.0f);

	if (nFlickIndex < 0) nFlickIndex = 0;
	else if (nFlickIndex > 9) nFlickIndex = 9;

	LTBOOL bFlick;

	switch (m_nFlickerPattern)
	{
		case 1:	bFlick = g_nFlicker1[nFlickIndex];	break;
		case 2:	bFlick = g_nFlicker2[nFlickIndex];	break;
		case 3:	bFlick = g_nFlicker3[nFlickIndex];	break;
		case 4:	bFlick = g_nFlicker4[nFlickIndex];	break;
		case 5:	bFlick = g_nFlicker5[nFlickIndex];	break;
		case 6:	bFlick = g_nFlicker6[nFlickIndex];	break;
		case 7:	bFlick = g_nFlicker7[nFlickIndex];	break;
		case 8:	bFlick = g_nFlicker8[nFlickIndex];	break;
	}

	if (!bFlick)
	{
		vColor -= g_cvarFlareLightReducePercent.GetFloat();
		vScale -= (g_cvarFlareLightSpriteReducePercent.GetFloat() / 1.0f);
	}

	g_pLTClient->SetLightColor(m_hLight, vColor.x, vColor.y, vColor.z);

	if (m_pToggleScaleFX)
	{
		HOBJECT hFX = m_pToggleScaleFX->GetObject();
	
		if (hFX)
		{
			g_pLTClient->SetObjectScale(hFX, &vScale);
		}
	}

// Testing, don't set the radius every frame...
m_pClientDE->SetLightRadius(m_hLight, g_cvarFlareLightRadius.GetFloat());
// End testing...
}


//-------------------------------------------------------------------------------------------------
// SFX_ProjectileFactory
//-------------------------------------------------------------------------------------------------

const SFX_ProjectileFactory SFX_ProjectileFactory::registerThis;

CSpecialFX* SFX_ProjectileFactory::MakeShape() const
{
	return new CProjectileFX();
}

