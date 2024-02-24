// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 2/22/98
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "WeaponFXTypes.h"
#include "GameClientShell.h"
#include "MarkSFX.h"
#include "ParticleShowerFX.h"
#include "DynamicLightFX.h"
#include "BulletTrailFX.h"
#include "MsgIDs.h"
#include "ShellCasingFX.h"
#include "ParticleExplosionFX.h"
#include "BaseScaleFX.h"
#include "DebrisFX.h"
#include "RandomSparksFX.h"
#include "iltphysics.h"
#include "iltcustomdraw.h"
#include "MuzzleFlashFX.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "PolyDebrisFX.h"

extern CGameClientShell* g_pGameClientShell;

static uint32 s_nNumShells = 0;

VarTrack	g_cvarShowFirePath;
VarTrack	g_cvarLightBeamColorDelta;

VarTrack	g_cvarImpactPitchShift;

VarTrack	g_cvarFlyByRadius;
VarTrack	g_cvarFlyBySoundRadius;

VarTrack	g_cvarSplatMod;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	WCREATESTRUCT w;

	w.hServerObj	= hServObj;
    w.hFiredFrom    = g_pLTClient->ReadFromMessageObject(hMessage);
    w.nWeaponId     = g_pLTClient->ReadFromMessageByte(hMessage);
	w.nBarrelId		= g_pLTClient->ReadFromMessageByte(hMessage);
    w.nAmmoId       = g_pLTClient->ReadFromMessageByte(hMessage);
    w.nSurfaceType  = g_pLTClient->ReadFromMessageByte(hMessage);
	w.nFlashSocket  = g_pLTClient->ReadFromMessageByte(hMessage);
    w.wIgnoreFX     = g_pLTClient->ReadFromMessageWord(hMessage);
    w.nShooterId    = g_pLTClient->ReadFromMessageByte(hMessage);
    g_pLTClient->ReadFromMessageVector(hMessage, &(w.vFirePos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(w.vPos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(w.vSurfaceNormal));
    HOBJECT hObj = g_pLTClient->ReadFromMessageObject(hMessage);

	//don't do impact FX if the local client is getting hit
	m_bLocalImpact = (hObj == g_pClientDE->GetClientObject());

	// see if we need to ignore this impact...
	if(g_pGameClientShell->IsMultiplayerGame())
	{
		// see if this is a vector weapon
		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(w.nAmmoId);

		if (!pAmmo) return LTFALSE;

		if(pAmmo->eType == VECTOR)
		{
			// lastly see if we are the shooter...
			if(w.hFiredFrom == g_pClientDE->GetClientObject())
				return LTFALSE;
		}
	}

	return Init(&w);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	WCREATESTRUCT* pCS = (WCREATESTRUCT*)psfxCreateStruct;

	m_nWeaponId			= pCS->nWeaponId;
	m_nBarrelId			= pCS->nBarrelId;
	m_nAmmoId			= pCS->nAmmoId;
	m_nFlashSocket		= pCS->nFlashSocket;
	m_eSurfaceType		= (SurfaceType)pCS->nSurfaceType;
	m_wIgnoreFX			= pCS->wIgnoreFX;
    m_hFiredFrom		= pCS->hFiredFrom;
	m_vFirePos			= pCS->vFirePos;
	m_vPos				= pCS->vPos;
	m_nShooterId		= pCS->nShooterId;
	m_bLocal			= pCS->bLocal;
	m_vSurfaceNormal	= pCS->vSurfaceNormal;
	m_vSurfaceNormal.Norm();

	m_eCode				= CC_NO_CONTAINER;
	m_eFirePosCode		= CC_NO_CONTAINER;

	m_pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
    if (!m_pAmmo) return LTFALSE;

	m_pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
    if (!m_pWeapon) return LTFALSE;

	m_pBarrel = g_pWeaponMgr->GetBarrel(m_nBarrelId);
	if (!m_pBarrel) return LTFALSE;

    m_fInstDamage   = (LTFLOAT) m_pAmmo->nInstDamage;
    m_fAreaDamage   = (LTFLOAT) m_pAmmo->nAreaDamage;


	if (!g_cvarShowFirePath.IsInitted())
	{
		g_cvarShowFirePath.Init(g_pLTClient, "ShowFirePath", NULL, -1.0f);
    }

	if (!g_cvarLightBeamColorDelta.IsInitted())
	{
		g_cvarLightBeamColorDelta.Init(g_pLTClient, "LightBeamColorDelta", NULL, 50.0f);
	}

	if (!g_cvarImpactPitchShift.IsInitted())
	{
		g_cvarImpactPitchShift.Init(g_pLTClient, "PitchShiftImpact", NULL, -1.0f);
	}

	if (!g_cvarFlyByRadius.IsInitted())
	{
		g_cvarFlyByRadius.Init(g_pLTClient, "FlyByRadius", NULL, 300.0f);
	}

	if (!g_cvarFlyBySoundRadius.IsInitted())
	{
		g_cvarFlyBySoundRadius.Init(g_pLTClient, "FlyBySoundRadius", NULL, 500.0f);
	}

 	if (!g_cvarSplatMod.IsInitted())
	{
		g_cvarSplatMod.Init(g_pLTClient, "SplatSize", NULL, 1.5f);
	}
   return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::CreateObject(ILTClient* pLTClient)
{
    if (!CSpecialFX::CreateObject(pLTClient) || !g_pWeaponMgr) 
		return LTFALSE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return LTFALSE;

	// Set up our data members...

	// Set the local client id...
    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);
    m_nLocalId = (uint8)dwId;

	m_nDetailLevel = pSettings->SpecialFXSetting();

	// set fire FX flags
	m_wFireFX = m_pAmmo->pFireFX ? m_pAmmo->pFireFX->nFlags : 0;

	// Assume alt-fire, silenced, and tracer...these will be cleared by
	// IgnoreFX if not used...
	m_wFireFX |= WFX_TRACER;

	// Clear all the fire fx we want to ignore...
	m_wFireFX &= ~m_wIgnoreFX;

	
	// Fire pos may get tweaked a little...
	if((m_wFireFX & WFX_MUZZLE) || (m_wFireFX & WFX_LIGHT) || (m_wFireFX & WFX_TRACER))
	{
		m_vFirePos = CalcFirePos(m_vFirePos);

		if(m_vFirePos.MagSqr() == 0)
			return LTFALSE;
	}

	m_vDir = m_vPos - m_vFirePos;
	m_fFireDistance = m_vDir.Mag();
	m_vDir.Norm();

    g_pLTClient->AlignRotation(&m_rSurfaceRot, &m_vSurfaceNormal, LTNULL);
    g_pLTClient->AlignRotation(&m_rDirRot, &m_vDir, LTNULL);


	// Determine what container the sfx is in...

	HLOCALOBJ objList[1];
    LTVector vTestPos = m_vPos + m_vSurfaceNormal;  // Test a little closer...
    uint32 dwNum = g_pLTClient->GetPointContainers(&vTestPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
            uint16 dwCode;
            if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				m_eCode = (ContainerCode)dwCode;
			}
		}
	}

	// Determine if the fire point is in liquid

	vTestPos = m_vFirePos + m_vDir;  // Test a little further in...
    dwNum = g_pLTClient->GetPointContainers(&vTestPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
            uint16 dwCode;
            if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				m_eFirePosCode = (ContainerCode)dwCode;
			}
		}
	}


	// figure out the impact FX
	if (IsLiquid(m_eCode))
	{
		m_wImpactFX	= m_pAmmo->pUWImpactFX ? m_pAmmo->pUWImpactFX->nFlags : 0;
	}
	else
	{
		m_wImpactFX	= m_pAmmo->pImpactFX ? m_pAmmo->pImpactFX->nFlags : 0;
	}

	m_wImpactFX &= ~m_wIgnoreFX;


	if(!(m_wIgnoreFX & WFX_MUZZLEONLY))
	{
		SetupExitInfo();

		// If the surface is the sky, don't create any impact related fx...

		if ((m_eSurfaceType != ST_SKY) || (m_wImpactFX & WFX_IMPACTONSKY))
		{
			CreateWeaponSpecificFX();

			if ((m_wImpactFX & WFX_MARK) && ShowsMark(m_eSurfaceType))
			{
				CreateMark(m_vPos, m_vSurfaceNormal, m_rSurfaceRot, m_eSurfaceType);
			}

			if(!(m_wIgnoreFX & WFX_IGNORE_IMPACT_SOUND))
				PlayImpactSound();

			CreateSurfaceSpecificFX();
		}

		PlayBulletFlyBySound();


		if (IsBulletTrailWeapon())
		{
			if (IsLiquid(m_eFirePosCode) && IsLiquid(m_eCode))
			{
				if (m_nDetailLevel != RS_LOW)
				{
					CreateBulletTrail(&m_vFirePos);
				}
			}
		}
	}


	// Always create these FX...Well, as long as the detail level is high
	// enough...

	if (m_nDetailLevel != RS_LOW)
	{
		if((m_nLocalId == m_nShooterId) && !GetConsoleInt("DrawViewModel", 1))
		{
			if(g_pGameClientShell->IsFirstPerson())
				m_wFireFX &= ~WFX_MUZZLE;

			m_wFireFX &= ~WFX_SHELL;
		}


		// No tracers under water...

		if ((m_wFireFX & WFX_TRACER) && !IsLiquid(m_eCode))
		{
			CreateTracer();
		}

		if ((m_wFireFX & WFX_MUZZLE))
		{
			CreateMuzzleFX();
		}

		if ((m_wFireFX & WFX_SHELL))
		{
			CreateShell();
		}

		if ((m_wFireFX & WFX_LIGHT))
		{
			CreateMuzzleLight();
		}
	}

	if ((m_wFireFX & WFX_FIRESOUND) ||
		(m_wFireFX & WFX_ALTFIRESND))
	{
		PlayFireSound();
	}


	// Show the fire path...(debugging...)

	if (g_cvarShowFirePath.GetFloat() > 0)
	{
		PLFXCREATESTRUCT pls;

		pls.vStartPos			= m_vFirePos;
		pls.vEndPos				= m_vPos;
        pls.vInnerColorStart    = LTVector(GetRandom(127.0f, 255.0f), GetRandom(127.0f, 255.0f), GetRandom(127.0f, 255.0f));
		pls.vInnerColorEnd		= pls.vInnerColorStart;
        pls.vOuterColorStart    = LTVector(0, 0, 0);
        pls.vOuterColorEnd      = LTVector(0, 0, 0);
		pls.fAlphaStart			= 1.0f;
		pls.fAlphaEnd			= 1.0f;
		pls.fMinWidth			= 0;
		pls.fMaxWidth			= 10;
		pls.fMinDistMult		= 1.0f;
		pls.fMaxDistMult		= 1.0f;
		pls.fLifeTime			= 10.0f;
		pls.fAlphaLifeTime		= 10.0f;
		pls.fPerturb			= 0.0f;
        pls.bAdditive           = LTFALSE;
		pls.nWidthStyle			= PLWS_CONSTANT;
		pls.nNumSegments		= 2;

		CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_POLYLINE_ID, &pls);
		if (pFX) pFX->Update();
	}

    return LTFALSE;  // Just delete me, I'm done :)
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::SetupExitInfo
//
//	PURPOSE:	Setup our exit info
//
// ----------------------------------------------------------------------- //

void CWeaponFX::SetupExitInfo()
{
	m_eExitSurface	= ST_UNKNOWN;
	m_vExitPos		= m_vFirePos;
	m_vExitNormal	= m_vDir;
	m_eExitCode		= CC_NO_CONTAINER;

	// Determine if the is an "exit" surface...

	IntersectQuery qInfo;
	IntersectInfo iInfo;

	qInfo.m_From = m_vFirePos + m_vDir;
	qInfo.m_To   = m_vFirePos - m_vDir;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

    if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		m_eExitSurface	= GetSurfaceType(iInfo);
		m_vExitNormal	= iInfo.m_Plane.m_Normal;
		m_vExitPos		= iInfo.m_Point + m_vDir;

		// Determine what container the sfx is in...

		HLOCALOBJ objList[1];
        LTVector vTestPos = m_vExitPos + m_vExitNormal;  // Test a little closer...
        uint32 dwNum = g_pLTClient->GetPointContainers(&vTestPos, objList, 1);

		if (dwNum > 0 && objList[0])
		{
            uint32 dwUserFlags;
            g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

			if (dwUserFlags & USRFLG_VISIBLE)
			{
                uint16 dwCode;
                if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
				{
					m_eExitCode = (ContainerCode)dwCode;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateExitDebris
//
//	PURPOSE:	Create any exit debris
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateExitDebris()
{
	int i;

	// Create the surface specific exit fx...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eExitSurface);
	if (!pSurf || !pSurf->bCanShootThrough) return;

	if (IsLiquid(m_eExitCode))
	{
		// Create underwater fx...

		// Create any exit particle shower fx associated with this surface...

		for (i=0; i < pSurf->nNumUWExitPShowerFX; i++)
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(pSurf->aUWExitPShowerFXIds[i]);
			g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vExitPos, m_vExitNormal, m_vSurfaceNormal);
		}
	}
	else
	{
		// Create normal fx...

		// Create any exit scale fx associated with this surface...

		for (i=0; i < pSurf->nNumExitScaleFX; i++)
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(pSurf->aExitScaleFXIds[i]);
			g_pFXButeMgr->CreateScaleFX(pScaleFX, m_vExitPos, m_vExitNormal, &m_vExitNormal, &m_rSurfaceRot);
		}

		// Create any exit particle shower fx associated with this surface...

		for (i=0; i < pSurf->nNumExitPShowerFX; i++)
		{
			CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(pSurf->aExitPShowerFXIds[i]);
			g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vExitPos, m_vExitNormal, m_vSurfaceNormal);
		}

		// Create any exit poly debris fx associated with this surface...

		for (i=0; i < pSurf->nNumExitPolyDebrisFX; i++)
		{
			CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX(pSurf->aExitPolyDebrisFXIds[i]);
			g_pFXButeMgr->CreatePolyDebrisFX(pPolyDebrisFX, m_vExitPos, m_vExitNormal, m_vExitNormal);
		}
	}


	// Determine if we should create a beam of light through the surface...

	CreateLightBeamFX(pSurf);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLightBeamFX
//
//	PURPOSE:	Create a light beam (if appropriate)
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLightBeamFX(SURFACE* pSurf)
{
	if (!pSurf) return;

    LTVector vEnterColor, vExitColor;
    if (g_pLTClient->GetPointShade(&m_vExitPos, &vExitColor) == LT_OK)
	{
		// Get the EnterColor value...

        LTVector vEnterPos = m_vExitPos - (m_vExitNormal * float(pSurf->nMaxShootThroughThickness + 1));

        if (g_pLTClient->GetPointShade(&vEnterPos, &vEnterColor) == LT_OK)
		{
			// Calculate the difference in light value...

            LTFLOAT fMaxEnter = Max(vEnterColor.x, vEnterColor.y);
			fMaxEnter = Max(fMaxEnter, vEnterColor.z);

            LTFLOAT fMaxExit = Max(vExitColor.x, vExitColor.y);
			fMaxExit = Max(fMaxExit, vExitColor.z);

			if (fabs((double)(fMaxExit - fMaxEnter)) >= g_cvarLightBeamColorDelta.GetFloat())
			{
                LTVector vStartPoint, vDir;
				if (fMaxEnter > fMaxExit)
				{
					vStartPoint = m_vExitPos;
					vDir = m_vDir;
				}
				else
				{
					vStartPoint = vEnterPos;
					vDir = -m_vDir;
				}

				PLFXCREATESTRUCT pls;

				pls.vStartPos			= vStartPoint;
				pls.vEndPos				= vStartPoint + (vDir * GetRandom(100.0, 150.0f));
                pls.vInnerColorStart    = LTVector(230, 230, 230);
				pls.vInnerColorEnd		= pls.vInnerColorStart;
                pls.vOuterColorStart    = LTVector(0, 0, 0);
                pls.vOuterColorEnd      = LTVector(0, 0, 0);
				pls.fAlphaStart			= 0.5f;
				pls.fAlphaEnd			= 0.0f;
				pls.fMinWidth			= 0;
				pls.fMaxWidth			= 10;
				pls.fMinDistMult		= 1.0f;
				pls.fMaxDistMult		= 1.0f;
				pls.fLifeTime			= 10.0f;
				pls.fAlphaLifeTime		= 10.0f;
				pls.fPerturb			= 0.0f;
                pls.bAdditive           = LTFALSE;
				pls.nWidthStyle			= PLWS_CONSTANT;
				pls.nNumSegments		= 1;

				CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_POLYLINE_ID, &pls);
				if (pFX) pFX->Update();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateExitMark
//
//	PURPOSE:	Create any exit surface marks
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateExitMark()
{
	if (m_eExitSurface != ST_UNKNOWN && ShowsMark(m_eExitSurface))
	{
        LTRotation rNormRot;
        g_pLTClient->AlignRotation(&rNormRot, &m_vExitNormal, LTNULL);

		CreateMark(m_vExitPos, m_vExitNormal, rNormRot, m_eExitSurface);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMark
//
//	PURPOSE:	Create a mark fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMark(LTVector vPos, LTVector vNorm, LTRotation rRot,
						   SurfaceType eType)
{
	IMPACTFX* pImpactFX = m_pAmmo->pImpactFX;

	if (IsLiquid(m_eCode))
	{
		pImpactFX = m_pAmmo->pUWImpactFX;
	}

	if (!pImpactFX) return;

	// See if the effect is going to be infront of the player
	if(g_pGameClientShell->IsFirstPerson())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

		if (!hCamera) return;

		LTVector vDir;
		g_pLTClient->GetObjectPos(hCamera, &vDir);
		vDir = vPos-vDir;

		LTRotation rRot;
		LTVector vNull, vF;
		g_pInterface->GetObjectRotation(hCamera, &rRot);
		g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);

		// Test of truth!
		if(vF.Dot(vDir) < 0.0f)
			return;
	}

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	MARKCREATESTRUCT mark;

	mark.m_vPos = vPos;
	mark.m_Rotation = rRot;

	// Randomly rotate the bullet hole...

    g_pLTClient->RotateAroundAxis(&mark.m_Rotation, &vNorm, GetRandom(0.0f, MATH_CIRCLE));

	mark.m_fScale		= pImpactFX->fMarkScale;
	mark.nAmmoId		= m_nAmmoId;
	mark.nSurfaceType   = eType;

	psfxMgr->CreateSFX(SFX_MARK_ID, &mark);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBulletTrail
//
//	PURPOSE:	Create a bullet trail fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBulletTrail(LTVector *pvStartPos)
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr || !pvStartPos) return;

	// See if the effect is going to be infront of the player
	// Both start and end points...
	if(g_pGameClientShell->IsFirstPerson())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

		if (!hCamera) return;

		LTVector vToEnd, vToStart, vCamPos;
		g_pLTClient->GetObjectPos(hCamera, &vCamPos);
		vToEnd		= m_vPos-vCamPos;
		vToStart	= *pvStartPos-vCamPos;

		LTRotation rRot;
		LTVector vNull, vF;
		g_pInterface->GetObjectRotation(hCamera, &rRot);
		g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);

		// Test of truth!
		if(vF.Dot(vToEnd) < 0.0f && vF.Dot(vToStart) < 0.0f)
			return;
	}

    LTVector vColor1, vColor2;
	vColor1.Init(200.0f, 200.0f, 200.0f);
	vColor2.Init(255.0f, 255.0f, 255.0f);

	BTCREATESTRUCT bt;

	bt.vStartPos		= *pvStartPos;
	bt.vDir				= m_vDir;
	bt.vColor1			= vColor1;
	bt.vColor2			= vColor2;
	bt.fLifeTime		= 0.5f;
	bt.fFadeTime		= 0.3f;
	bt.fRadius			= 400.0f;
	bt.fGravity			= 0.0f;
	bt.fNumParticles	= (m_nDetailLevel == RS_MED) ? 15.0f : 30.0f;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_BULLETTRAIL_ID, &bt);

	// Let each bullet trail do its initial update...

	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateTracer
//
//	PURPOSE:	Create a tracer fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateTracer()
{
	if (!m_pAmmo || !m_pAmmo->pTracerFX) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

//	if (m_nDetailLevel != RS_HIGH && GetRandom(1, 2) == 1) return;

	// Create tracer...

	if (m_fFireDistance > 100.0f)
	{
		// See if the effect is going to be infront of the player
		// Both start and end points...
		if(m_nLocalId != m_nShooterId)
		{
			HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

			if (!hCamera) return;

			LTVector vToEnd, vToStart, vCamPos;
			g_pLTClient->GetObjectPos(hCamera, &vCamPos);
			vToEnd		= m_vPos-vCamPos;
			vToStart	= m_vFirePos-vCamPos;

			LTRotation rRot;
			LTVector vNull, vF;
			g_pInterface->GetObjectRotation(hCamera, &rRot);
			g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);

			// Test of truth!
			if(vF.Dot(vToEnd) < 0.0f && vF.Dot(vToStart) < 0.0f)
				return;
		}

		TRCREATESTRUCT tracer;

		// Make tracer start in front of gun a little ways...

		tracer.vStartPos	= m_vFirePos + (m_vDir * 25.0f);
		tracer.vEndPos		= m_vPos;
		tracer.pTracerFX	= m_pAmmo->pTracerFX;

		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_TRACER_ID, &tracer);
		if (pFX)
		{
			pFX->Update();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateWeaponSpecificFX()
//
//	PURPOSE:	Create weapon specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateWeaponSpecificFX()
{
	// Do fire fx beam fx...

	if (m_pAmmo->pFireFX && m_pAmmo->pFireFX->nNumBeamFX > 0)
	{
		for (int i=0; i < m_pAmmo->pFireFX->nNumBeamFX; i++)
		{
			g_pFXButeMgr->CreateBeamFX(m_pAmmo->pFireFX->pBeamFX[i],
				m_vFirePos, m_vPos);
		}
	}

	// see if the surface is flesh then check the impact
	LTBOOL bCheckFlesh = IsFleshSurfaceType(m_eSurfaceType);

	if(m_vSurfaceNormal.MagSqr() == 0)
		m_vSurfaceNormal = m_vDir;
	
	IFXCS cs;
	cs.eCode		= m_eCode;
	cs.eSurfType	= m_eSurfaceType;
	cs.rSurfRot		= m_rSurfaceRot;
	cs.vDir			= m_vSurfaceNormal;
	cs.vPos			= m_vPos;
	cs.vSurfNormal	= m_vSurfaceNormal;

	if (IsLiquid(m_eCode))
	{
		// Sanity check
		if(!m_pAmmo->pUWImpactFX) return;

		if(bCheckFlesh)
			if(!(m_pAmmo->pUWImpactFX->nFlags & WFX_IMPACTONFLESH))
				return;

		// Create underwater weapon fx...
		g_pFXButeMgr->CreateImpactFX(m_pAmmo->pUWImpactFX, cs);
	}
	else
	{
		// Sanity check
		if(!m_pAmmo->pImpactFX) return;

		if(bCheckFlesh)
			if(!(m_pAmmo->pImpactFX->nFlags & WFX_IMPACTONFLESH))
				return;

		// Create normal weapon impact...
		g_pFXButeMgr->CreateImpactFX(m_pAmmo->pImpactFX, cs);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSurfaceSpecificFX()
//
//	PURPOSE:	Create surface specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSurfaceSpecificFX()
{
	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

	int i;
    uint8 nVal = pSettings->SpecialFXSetting();


	// Create vector specific flesh effects
	if(IsFleshSurfaceType(m_eSurfaceType))
	{
		// Don't do gore fx...
		if(!pSettings->Gore())
			return;

		if(m_pAmmo->eType == VECTOR && m_pAmmo->eInstDamageType != DT_MARINE_HACKER)
		{
			CreateVectorBloodFX();
		}
	}


	switch (nVal)
	{
		case RS_LOW:
		case RS_MED:
		case RS_HIGH:

		default:
		break;
	}

	// See if the effect is going to be infront of the player
	// If not, don't do the rest of the surface specific impact
	// FX.
	if(g_pGameClientShell->IsFirstPerson())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

		if (!hCamera) return;

		LTVector vDir;
		g_pLTClient->GetObjectPos(hCamera, &vDir);
		vDir = m_vPos-vDir;

		LTRotation rRot;
		LTVector vNull, vF;
		g_pInterface->GetObjectRotation(hCamera, &rRot);
		g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);

		// Test of truth!
		if(vF.Dot(vDir) < 0.0f)
			return;
	}

	if ((m_wFireFX & WFX_EXITMARK) && ShowsMark(m_eExitSurface))
	{
		CreateExitMark();
	}

	if (m_wFireFX & WFX_EXITDEBRIS)
	{
		CreateExitDebris();
	}

	if (!m_pAmmo || !m_pAmmo->pImpactFX) return;
	if (!m_pAmmo->pImpactFX->bDoSurfaceFX) return;


	// Create the surface specific fx...
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eSurfaceType);

	if(pSurf)
	{
		if(IsLiquid(m_eCode))
		{
			// Create underwater fx...

			// Create any impact particle shower fx associated with this surface...
			for (i=0; i < pSurf->nNumUWImpactPShowerFX; i++)
			{
				CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(pSurf->aUWImpactPShowerFXIds[i]);
				g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vPos, m_vDir, m_vSurfaceNormal);
			}
		}
		else
		{
			// Create normal fx...

			// Create any impact scale fx associated with this surface...
			for (i=0; i < pSurf->nNumImpactScaleFX; i++)
			{
				CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(pSurf->aImpactScaleFXIds[i]);
				g_pFXButeMgr->CreateScaleFX(pScaleFX, m_vPos, m_vDir, &m_vSurfaceNormal, &m_rSurfaceRot);
			}

			// Create any impact particle shower fx associated with this surface...
			for (i=0; i < pSurf->nNumImpactPShowerFX; i++)
			{
				CPShowerFX* pPShowerFX = g_pFXButeMgr->GetPShowerFX(pSurf->aImpactPShowerFXIds[i]);
				g_pFXButeMgr->CreatePShowerFX(pPShowerFX, m_vPos, m_vDir, m_vSurfaceNormal);
			}

			// Create any impact poly debris fx associated with this surface...
			for (i=0; i < pSurf->nNumImpactPolyDebrisFX; i++)
			{
				CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX(pSurf->aImpactPolyDebrisFXIds[i]);
				g_pFXButeMgr->CreatePolyDebrisFX(pPolyDebrisFX, m_vPos, m_vDir, m_vSurfaceNormal);
			}
		}
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleLight()
//
//	PURPOSE:	Create a muzzle light associated with this fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleLight()
{
	// Check to see if we have the silencer...

	if (m_nLocalId != m_nShooterId || !g_pGameClientShell->IsFirstPerson())
	{
		MUZZLEFLASHCREATESTRUCT mf;

		mf.pBarrel		= m_pBarrel;
		mf.hParent		= m_hFiredFrom;
		mf.vPos			= m_vFirePos;
		mf.rRot			= m_rDirRot;
		mf.nFlashSocket = m_nFlashSocket;

		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if (!psfxMgr) return;

		psfxMgr->CreateSFX(SFX_MUZZLEFLASH_ID, &mf);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::IsBulletTrailWeapon()
//
//	PURPOSE:	See if this weapon creates bullet trails in liquid
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::IsBulletTrailWeapon()
{
    return m_pAmmo->bBubbleTrail;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayImpactSound()
//
//	PURPOSE:	Play a surface impact sound if appropriate
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayImpactSound()
{
	IMPACTFX* pImpactFX = m_pAmmo->pImpactFX;

	if (IsLiquid(m_eCode))
	{
		pImpactFX = m_pAmmo->pUWImpactFX;
	}

	if (!pImpactFX) return;


	if (m_pAmmo->eType == VECTOR)
	{
		if ((m_nDetailLevel == RS_LOW) && GetRandom(1, 2) != 1) return;
		else if ((m_nDetailLevel == RS_MED) && GetRandom(1, 3) == 1) return;
	}

	char* pSnd = GetImpactSound(m_eSurfaceType, m_nAmmoId);
    LTFLOAT fSndRadius = (LTFLOAT) pImpactFX->nSoundRadius;

	if (pSnd)
	{
		uint32 dwFlags = 0;
		float fPitchShift = 1.0f;
		if (g_cvarImpactPitchShift.GetFloat() > 0.0f)
		{
			dwFlags |= PLAYSOUND_CTRL_PITCH;
		}

        uint8 nVolume = IsLiquid(m_eCode) ? 50 : 100;
		g_pClientSoundMgr->PlaySoundFromPos(m_vPos, pSnd, fSndRadius, 
			SOUNDPRIORITY_MISC_LOW, dwFlags, nVolume, fPitchShift);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleFX()
//
//	PURPOSE:	Create muzzle specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleFX()
{
	// See if the effect is going to be infront of the player
	if(g_pGameClientShell->IsFirstPerson())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

		if (!hCamera) return;

		LTVector vDir;
		g_pLTClient->GetObjectPos(hCamera, &vDir);
		vDir = m_vPos-vDir;

		LTRotation rRot;
		LTVector vNull, vF;
		g_pInterface->GetObjectRotation(hCamera, &rRot);
		g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);

		// Test of truth!
		if(vF.Dot(vDir) < 0.0f)
			return;
	}

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CPShowerFX *pPSFX = LTNULL;
	CPShowerFX PSFX;

	// Check to see if we fired underwater
	if(IsLiquid(m_eFirePosCode))
		pPSFX = g_pFXButeMgr->GetPShowerFX("MuzzleBubbles");
	else
		pPSFX = g_pFXButeMgr->GetPShowerFX("MuzzleSmoke");

	// If the effect wasn't valid... just return
	if(!pPSFX) return;

	// Otherwise, assign this to a temporary structure so the code
	// can modify the effect a little
	PSFX = *pPSFX;

	if(IsLiquid(m_eFirePosCode))
		GetLiquidColorRange(m_eFirePosCode, &PSFX.vColor1, &PSFX.vColor2);


	// Create the effect
	g_pFXButeMgr->CreatePShowerFX(&PSFX, m_vFirePos, m_vSurfaceNormal, m_vSurfaceNormal);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateShell()
//
//	PURPOSE:	Create shell casing
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShell()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	SHELLCREATESTRUCT sc;
	sc.rRot		 = m_rDirRot;
	sc.vStartPos = m_vFirePos;
	sc.nWeaponId = m_nWeaponId;
	sc.nAmmoId	 = m_nAmmoId;

    sc.b3rdPerson = LTFALSE;

	// See if this is our local client who fired, and if so are we in 3rd person...

	if (m_nLocalId == m_nShooterId)
	{
		sc.b3rdPerson = !g_pGameClientShell->IsFirstPerson();
	}
	else
	{
        sc.b3rdPerson = LTTRUE;
	}


	// Adjust the shell position based on the hand-held breach offset...

	if (sc.b3rdPerson)
	{
		sc.vOffset = (m_vDir * m_pBarrel->fHHBreachOffset);
	}
	else  // Get the shell eject pos...
	{
		sc.vOffset = (m_vDir * m_pBarrel->fPVBreachOffset);

		// Add on the player's velocity...

		g_pGameClientShell->GetPlayerMovement()->GetVelocity(sc.vStartVel);
	}


	// Only create every other shell if medium detail set...
	// if (m_nDetailLevel == RS_MED && (++s_nNumShells % 2 == 0)) return;

	psfxMgr->CreateSFX(SFX_SHELLCASING_ID, &sc);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateVectorBloodFX
//
//	PURPOSE:	Create the blood trail, splats, etc.
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateVectorBloodFX()
{
	if(GetConsoleInt("LowViolence",0))
		return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings || !pSettings->Gore()) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX = LTNULL;


	// ----------------------------------------------------------------------- //
	// Check to see if this an FX that we don't want to create blood for.

	if(m_eSurfaceType == ST_FLESH_PRAETORIAN)
	{
		// Don't do blood FX for resistance damage type on Praetorian flesh
		if(DamageTypeResistance(m_pAmmo->eInstDamageType))
			return;

		// Otherwise... treat it as standard Alien flesh
		m_eSurfaceType = ST_FLESH_ALIEN;
	}


	// ----------------------------------------------------------------------- //
	// Create a blood particle shower effect out of the attribute file

	LTBOOL bDoBloodSpray = LTTRUE;

	// See if the effect is going to be infront of the player
	if(g_pGameClientShell->IsFirstPerson())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

		if (!hCamera) return;

		LTVector vDir;
		g_pLTClient->GetObjectPos(hCamera, &vDir);
		vDir = m_vPos-vDir;

		LTRotation rRot;
		LTVector vNull, vF;
		g_pInterface->GetObjectRotation(hCamera, &rRot);
		g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);

		// Test of truth!
		bDoBloodSpray = (vF.Dot(vDir) > 0.0f);
	}

	if(bDoBloodSpray)
	{
		CPShowerFX *pPSFX		= LTNULL;
		CPShowerFX *pDroplet	= LTNULL;

		char *szType = "\0";

		if(m_fInstDamage <= 33.0f)		
			szType = "Small";
		else
			szType = "Large";

		char szSpray[32];
		char szDroplet[32];

		switch(m_eSurfaceType)
		{
			case ST_FLESH_PREDATOR:
			{	
				sprintf(szSpray, "Blood_Shower_Predator_%s", szType);		
				sprintf(szDroplet, "Blood_Droplet_Predator_%s", szType);
				break;
			}

			case ST_FLESH_ALIEN:
			{
				sprintf(szSpray, "Blood_Shower_Alien_%s", szType);			
				sprintf(szDroplet, "Blood_Droplet_Alien_%s", szType);
				break;
			}

			case ST_FLESH_SYNTH:
			{
				sprintf(szSpray, "Blood_Shower_Synth_%s", szType);			
				sprintf(szDroplet, "Blood_Droplet_Synth_%s", szType);		
				break;
			}

			case ST_FLESH_HUMAN:
			default:
			{
				sprintf(szSpray, "Blood_Shower_Human_%s", szType);
				sprintf(szDroplet, "Blood_Droplet_Human_%s", szType);	
				break;
			}
		}

		pPSFX = g_pFXButeMgr->GetPShowerFX(szSpray);
		pDroplet = g_pFXButeMgr->GetPShowerFX(szDroplet);

		// If we didn't create some blood... just return cause we don't want
		// to do the rest of the blood FX
		if(!pPSFX) return;

		// Don't do this for local and firstperson
		if(!m_bLocalImpact || !g_pGameClientShell->IsFirstPerson())
		{
			g_pFXButeMgr->CreatePShowerFX(pPSFX, m_vPos, m_vDir, m_vDir);

			if(pDroplet)
				g_pFXButeMgr->CreatePShowerFX(pDroplet, m_vPos, m_vDir, m_vDir);
		}
	}


	// If we're not of bullet type damage... then we don't want to
	// do the rest of the FX
	if(	(m_pAmmo->eInstDamageType != DT_BULLET) && 
		(m_pAmmo->eInstDamageType != DT_HIGHCALIBER) && 
		(m_pAmmo->eInstDamageType != DT_SHOTGUN) && 
		(m_pAmmo->eInstDamageType != DT_BULLET_NOGIB))
		return;


	// ----------------------------------------------------------------------- //
	// Create a blood splat if we're within range of a wall
	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	LTVector vTemp;
	vTemp = m_vDir * 100.0f;
	vTemp += m_vPos;

	iQuery.m_From = m_vPos;
	iQuery.m_To = vTemp;

	LTBOOL bSplash = g_pLTClient->IntersectSegment(&iQuery, &iInfo);

	if(!bSplash)
	{
		//try on the ground a bit back from victim
		iQuery.m_From = iQuery.m_To;
		iQuery.m_To += m_vDir * 100.0f;
		iQuery.m_To += LTVector(0, -1.0f, 0) * 200.0f;
		bSplash = g_pLTClient->IntersectSegment(&iQuery, &iInfo);
	}

	if(bSplash)
	{
		// See if the effect is going to be infront of the player
		if(g_pGameClientShell->IsFirstPerson())
		{
			HOBJECT hCamera = g_pGameClientShell->GetCameraMgr()->GetCameraObject(g_pGameClientShell->GetPlayerCamera());

			if (!hCamera) return;

			LTVector vDir;
			g_pLTClient->GetObjectPos(hCamera, &vDir);
			vDir = iInfo.m_Point-vDir;

			LTRotation rRot;
			LTVector vNull, vF;
			g_pInterface->GetObjectRotation(hCamera, &rRot);
			g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);

			// Test of truth!
			if(vF.Dot(vDir) < 0.0f)
				return;
		}

		// Create a blood splat...
		BSCREATESTRUCT sc;

		// Randomly rotate the blood splat
		g_pLTClient->AlignRotation(&(sc.rRot), &(iInfo.m_Plane.m_Normal), LTNULL);
		g_pLTClient->RotateAroundAxis(&(sc.rRot), &(iInfo.m_Plane.m_Normal), GetRandom(0.0f, MATH_CIRCLE));

		LTFLOAT fScaleMod = 1.0f - ((iInfo.m_Point - iQuery.m_From).Mag() / 200.0f);

		sc.vPos				= iInfo.m_Point + iInfo.m_Plane.m_Normal * GetRandom(1.0f, 1.9f);  // Off the wall a bit
		sc.vVel				= LTVector(0.0f, 0.0f, 0.0f);
		sc.vInitialScale	= LTVector(g_cvarSplatMod.GetFloat() * fScaleMod * GetRandom(0.2f, 0.3f), g_cvarSplatMod.GetFloat() * fScaleMod * GetRandom(0.2f, 0.3f), 1.0f);
		sc.vFinalScale		= sc.vInitialScale;
		sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
		sc.fLifeTime		= 15.0f;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;
		sc.nType			= OT_SPRITE;
		sc.ePriority		= BS_PRIORITY_LOW;

		char* pBloodFiles[] =
		{
			"Sprites\\MarBld1.spr",
			"Sprites\\MarBld2.spr",
			"Sprites\\MarBld3.spr",
			"Sprites\\PrdBld1.spr",
			"Sprites\\PrdBld2.spr",
			"Sprites\\PrdBld3.spr",
			"Sprites\\AlnBld1.spr",
			"Sprites\\AlnBld2.spr",
			"Sprites\\AlnBld3.spr",
			"Sprites\\SynBld1.spr",
			"Sprites\\SynBld2.spr",
			"Sprites\\SynBld3.spr",
		};

		switch(m_eSurfaceType)
		{
			case ST_FLESH_PREDATOR:		sc.Filename = pBloodFiles[GetRandom(3, 5)];		break;
			case ST_FLESH_ALIEN:		sc.Filename = pBloodFiles[GetRandom(6, 8)];		break;
			case ST_FLESH_PRAETORIAN:	sc.Filename = pBloodFiles[GetRandom(6, 8)];		break;
			case ST_FLESH_SYNTH:		sc.Filename = pBloodFiles[GetRandom(9, 11)];	break;
			case ST_FLESH_HUMAN:
			default:					sc.Filename = pBloodFiles[GetRandom(0, 2)];		break;
		}

		pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
		if(pFX) pFX->Update();

		// Lastly, see if we should sizzle...
		if((m_eSurfaceType == ST_FLESH_ALIEN) || (m_eSurfaceType == ST_FLESH_PRAETORIAN))
		{
			// First see if we have a valid impact FX or not
			IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX("Alien_Blood_Splat");

			if(pImpactFX)
			{
				IFXCS ifxcs;

				// Fill in a create structure
				ifxcs.vPos = sc.vPos;
				ifxcs.rSurfRot = sc.rRot;
				ifxcs.vDir = iInfo.m_Plane.m_Normal;
				ifxcs.vSurfNormal = iInfo.m_Plane.m_Normal;
				ifxcs.bPlaySound = LTTRUE;

				// make another FX
				g_pFXButeMgr->CreateImpactFX(pImpactFX, ifxcs);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayFireSound
//
//	PURPOSE:	Play the fire sound
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayFireSound()
{
	if (m_nLocalId >= 0 && m_nLocalId == m_nShooterId)
	{
		return;  // This client already heard the sound ;)
	}

	WeaponModelKeySoundId eSoundId = WMKS_FIRE;

	if (m_wFireFX & WFX_ALTFIRESND)
	{
		eSoundId = WMKS_ALT_FIRE;
	}

	::PlayWeaponSound(m_pWeapon, m_pBarrel, m_vFirePos, eSoundId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CalcFirePos
//
//	PURPOSE:	Calculate the fire position based on the FireFrom object
//
// ----------------------------------------------------------------------- //

LTVector CWeaponFX::CalcFirePos(LTVector vFirePos)
{
	if (!m_hFiredFrom) return vFirePos;

	// See if this is our local client who fired, and if so
	// only calculate fire position if we are in 3rd person...

	if (m_nLocalId == m_nShooterId)
	{
		if (g_pGameClientShell->IsFirstPerson()) return vFirePos;
	}

    LTVector vPos = vFirePos;
    LTRotation rRotUnused;
	const char * szFlashSocketName = "Flash";
	if (m_pBarrel)
	{
		if (m_nFlashSocket < m_pBarrel->nNumFlashSockets  && m_pBarrel->szFlashSocket[m_nFlashSocket])
		{
			szFlashSocketName = m_pBarrel->szFlashSocket[m_nFlashSocket];
		}
	}

	// Try to find the socket on the parent's attachments.
	if (!GetAttachmentSocketTransform(m_hFiredFrom, const_cast<char*>(szFlashSocketName), vPos, rRotUnused))
	{
		// If that failed, try the parent model.
		HMODELSOCKET hSocket;
		if (g_pModelLT->GetSocket(m_hFiredFrom, const_cast<char*>(szFlashSocketName), hSocket) == LT_OK)
		{
			LTransform transform;
			if (g_pModelLT->GetSocketTransform(m_hFiredFrom, hSocket, transform, DTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRotUnused);
			}
		}
	}

	if((vPos-vFirePos).MagSqr() > 90000.0f)
		return LTVector(0,0,0);

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayBulletFlyBySound()
//
//	PURPOSE:	Play bullet fly by sound (if appropriate)
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayBulletFlyBySound()
{
	if (!m_pWeapon || !m_pAmmo) return;

	if (m_pAmmo->eType != VECTOR) return;

	// Camera pos

    LTVector vPos;
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, vPos);

	// We only play the flyby sound if we won't hear an impact or
	// fire sound, or we are outta range of this weapon...

	LTVector vDist = m_vFirePos - vPos;
	if (vDist.Mag() < m_pBarrel->fFireSoundRadius) return;

	if (vDist.MagSqr() > m_pBarrel->nRangeSqr) return;

	if (m_pAmmo->pImpactFX)
	{
		vDist = m_vPos - vPos;
		if (vDist.Mag() < m_pAmmo->pImpactFX->nSoundRadius) return;
	}


	// See if the camera is close enough to the bullet path to hear the
	// bullet...

	LTFLOAT fRadius = g_cvarFlyByRadius.GetFloat();

	LTVector vDir = m_vDir;

	const LTVector vRelativePos = vPos - m_vFirePos;
    const LTFLOAT fRayDist = vDir.Dot(vRelativePos);
	LTVector vBulletDir = (vDir*fRayDist - vRelativePos);

    const LTFLOAT fDistSqr = vBulletDir.MagSqr();

	if (fDistSqr < fRadius*fRadius)
	{
		vPos += vBulletDir;
		g_pClientSoundMgr->PlaySoundFromPos(vPos, "sounds\\weapons\\flyby.wav", 
			g_cvarFlyBySoundRadius.GetFloat(), SOUNDPRIORITY_MISC_LOW);
	}
}

//-------------------------------------------------------------------------------------------------
// SFX_WeaponFactory
//-------------------------------------------------------------------------------------------------

const SFX_WeaponFactory SFX_WeaponFactory::registerThis;

CSpecialFX* SFX_WeaponFactory::MakeShape() const
{
	return new CWeaponFX();
}


