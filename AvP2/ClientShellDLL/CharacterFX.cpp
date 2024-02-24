// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterFX.cpp
//
// PURPOSE : Character special FX - Implementation
//
// CREATED : 8/24/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterFX.h"
#include "GameClientShell.h"
#include "ParticleTrailFX.h"
#include "SmokeFX.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "SoundMgr.h"
#include "Physics_lt.h"
#include "SurfaceFunctions.h"
#include "BaseScaleFX.h"
#include "CharacterButeMgr.h"
#include "CharacterFuncs.h"
#include "VisionModeButeMGR.h"
#include "Flashlight.h"
#include "FireFX.h"
#include "VarTrack.h"
#include "FXButeMgr.h"
#include "DebrisFX.h"
#include "ModelButeMgr.h"
#include "MsgIds.h"
#include "ClientButeMgr.h"

#include <string>

extern CGameClientShell* g_pGameClientShell;

#define FOOTSTEP_SOUND_RADIUS	1000.0f
#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"
#define MAX_BOSS_SMOKE_INTENSITY 8

VarTrack g_vtMinFootstepTime;
VarTrack g_vtPushBack;
VarTrack g_vtElecUpdateTime;
VarTrack g_vtCloakUpdateTime;
VarTrack g_vtCloakMaxTime;
VarTrack g_vtEyeFlashTime;
VarTrack g_vtBleederTime;
VarTrack g_vtBleederPoolTime;
VarTrack g_vtFlameDelayTime;
VarTrack g_vtFlameLifeTime;
VarTrack g_vtPoolSizzleChance;
VarTrack g_vtBossSmokeTime;
VarTrack g_vtBossSparkTime;

// ----------------------------------------------------------------------- //

LTBOOL CFXObjListFilterFn(HOBJECT hTest, void *pUserData)
{
	// If it's a world object, consider it a hit regardless of flags.
	if(g_pPhysicsLT->IsWorldObject(hTest) == LT_YES)
		return LTTRUE;

	// Don't collide with non-solid or non-visible objects
	uint32 nFlags;
	g_pLTClient->Common()->GetObjectFlags(hTest, OFT_Flags, nFlags);

	if(!(nFlags & FLAG_SOLID)) return LTFALSE;
		
	// Check against the list of the objects
	HOBJECT *hList = (HOBJECT*)pUserData;
	while(hList && *hList)
	{
		if(hTest == *hList)
			return LTFALSE;

		++hList;
	}


	return LTTRUE;
}

LTBOOL CFXPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	if(hPoly == INVALID_HPOLY) return LTTRUE;

	uint32 nSurfFlags;
	g_pLTClient->Common()->GetPolySurfaceFlags(hPoly, nSurfFlags);

	if (!(nSurfFlags & SURF_SOLID)) return LTFALSE;
	
	if (nSurfFlags & SURF_INVISIBLE)
	{
		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
		if (pSurf && pSurf->eType != ST_SKY && pSurf->eType != ST_INVISIBLE)
		{
			return LTTRUE;
		}
	
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

static LTBOOL IsFootStepState(CharacterMovementState cms)
{
	switch(cms)
	{
		case CMS_IDLE:
		case CMS_WALKING:
		case CMS_RUNNING:
		case CMS_CRAWLING:
		case CMS_WALLWALKING:
		case CMS_PRE_CRAWLING:  // needed for landing a spring jump with crouch key down
		case CMS_POST_CRAWLING:  
		case CMS_CLIMBING:		
			return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

static int FootStepStateVolumeMod(CharacterMovementState cms)
{
	switch(cms)
	{
		case CMS_RUNNING:
			return 1;

		case CMS_WALLWALKING:
		case CMS_CRAWLING:
			return 6;

		default:
			return 3;
	}

	return 1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
	if (!hMessage) return LTFALSE;

	CHARCREATESTRUCT ch;

	ch.hServerObj = hServObj;
	ch.Read(g_pLTClient, hMessage);

	g_vtMinFootstepTime.Init(g_pLTClient, "MinFootstepTime", NULL, 0.05f);
	g_vtPushBack.Init(g_pLTClient, "AuraOffset", NULL, 10.0f);
	g_vtElecUpdateTime.Init(g_pLTClient, "ElecUpdateTime", NULL, 0.2f);
	g_vtCloakUpdateTime.Init(g_pLTClient, "CloakUpdateTime", NULL, 0.045f);
	g_vtCloakMaxTime.Init(g_pLTClient, "MaxCloakTime", NULL, 1.75f);
	g_vtEyeFlashTime.Init(g_pLTClient, "EyeFlashTime", NULL, 1.25f);
	g_vtBleederTime.Init(g_pLTClient, "BleederTime", NULL, 0.1f);
	g_vtBleederPoolTime.Init(g_pLTClient, "BleederPoolTime", NULL, 0.7f);
	g_vtFlameDelayTime.Init(g_pLTClient, "FlameDelay", NULL, 0.04f);
	g_vtFlameLifeTime.Init(g_pLTClient, "FlameLife", NULL, 0.3f);
	g_vtPoolSizzleChance.Init(g_pLTClient, "PoolSizzleChance", NULL, 1.0f);
	g_vtBossSmokeTime.Init(g_pLTClient, "BossSmokeTime", NULL, 0.08f);
	g_vtBossSparkTime.Init(g_pLTClient, "BossSparkTime", NULL, 0.80f);

	return Init(&ch);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if(!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((CHARCREATESTRUCT*)psfxCreateStruct);

	for ( int i = 1; i < m_cs.nNumTrackers; i++ )
	{
		g_pModelLT->AddTracker(m_cs.hServerObj, &m_aAnimTrackers[i]);
	}

	if ( !m_NodeController.Init(this, LTNULL, (m_cs.nClientID != 0)) )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::~CCharacterFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacterFX::~CCharacterFX()
{
	RemoveLaserFX();

	if(m_hAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hAuraSystem);
		m_hAuraSystem = LTNULL;
	}

	if(m_hHeatAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hHeatAuraSystem);
		m_hHeatAuraSystem = LTNULL;
	}

	if(m_hElecAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hElecAuraSystem);
		m_hElecAuraSystem = LTNULL;
	}

	if(m_hCloakAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hCloakAuraSystem);
		m_hCloakAuraSystem = LTNULL;
	}

	for(int i=0; i<3; i++) 
		if(m_hFireSystem[i])
		{
			//remove the fire system
			g_pLTClient->RemoveObject(m_hFireSystem[i]);
			m_hFireSystem[i] = LTNULL;
		}

	if(m_hFireSound)
	{
		//kill the looping fire sound
		g_pLTClient->KillSound(m_hFireSound);
		m_hFireSound = LTNULL;
	}

	// kill the sound if one is already playing
	if(m_hWeaponLoopSound)
	{
		g_pLTClient->KillSound(m_hWeaponLoopSound);
		m_hWeaponLoopSound = LTNULL;
	}

	if(m_pFlashlight)
	{
		delete m_pFlashlight;
		m_pFlashlight = LTNULL;
	}

	// clean up the targeting sprites
	for(i=0; i<3; i++)
	{
		if(m_hTargetingSprites[i])
		{
			g_pLTClient->RemoveObject(m_hTargetingSprites[i]);
			m_hTargetingSprites[i] = LTNULL;
		}
	}

	if (m_hFlare)
	{
        g_pLTClient->RemoveObject(m_hFlare);
        m_hFlare = LTNULL;
	}

	//clean up string
//	if(m_LFcs.hstrSpriteFile)
//		g_pLTClient->FreeString(m_LFcs.hstrSpriteFile);

	for( BleederList::iterator iter = m_BleederList.begin(); iter != m_BleederList.end(); ++iter )
	{
		delete (*iter);
	}

	// Clean up the flame thrower FX
	for(i=0;i<MAX_FLAME_SEGMENTS;i++)
	{
		if(m_hFlameEmitters[i])
		{
			//remove the system
			g_pLTClient->RemoveObject(m_hFlameEmitters[i]);
			m_hFlameEmitters[i] = LTNULL;
		}
	}

	// Clean up the boss smoke system...
	if(m_hBossSmokeSystem)
	{
		//remove the system
		g_pLTClient->RemoveObject(m_hBossSmokeSystem);
		m_hBossSmokeSystem = LTNULL;
	}

	if( m_hServerObject )
	{
		for ( i = 1; i < m_cs.nNumTrackers; i++ )
		{
			g_pModelLT->RemoveTracker(m_cs.hServerObj, &m_aAnimTrackers[i]);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::CreateObject(CClientDE* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	uint32 dwCFlags = g_pLTClient->GetObjectClientFlags(m_hServerObject);
	g_pLTClient->SetObjectClientFlags(m_hServerObject, dwCFlags | CF_NOTIFYMODELKEYS);

	// Record our initial surface type...
	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);
	m_eSurfaceType = UserFlagToSurface(dwFlags);


	// Set up MoveMgr's point to us, if applicable...

	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
	if (hPlayerObj == m_hServerObject)
	{
		CreateFireFX();
	}
	else
	{
		//go ahead and load up the particle system
		CreateAuraFX();
		CreateHeatAuraFX();
		CreateElecAuraFX();
		CreateCloakAuraFX();
		CreateFireFX();
		CreateFlashlight();
	}

	// If we are in SP, ask for some SP FX verification...
	if(g_pGameClientShell->GetGameType()->IsSinglePlayer())
	{
		HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_PLAYER_VERIFY_CFX);
		g_pInterface->WriteToMessageObject(hWrite, m_hServerObject);
		g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
	}
	// If we are in MP, ask for some MP FX verification...
	else if(hPlayerObj == m_hServerObject)
	{
		HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_MPLAYER_VERIFY_CFX);
		g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
	}
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::Update()
{
	if (!g_pLTClient || !m_hServerObject || IsWaitingForRemove() ) return LTFALSE;

	uint32 dwObjectFlags, dwUserFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);
	dwObjectFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
	SurfaceType eSurfaceType = UserFlagToSurface(dwUserFlags);

	// See if anything important has changed...
	if(m_eSurfaceType != eSurfaceType)
	{
		m_eSurfaceType = eSurfaceType;
		ResetSpecialFX();
		return LTTRUE;
	}

	// Set our dead stuff
	HLOCALOBJ hPlayerObj	= g_pClientDE->GetClientObject();
	LTBOOL 	bIsLocalClient = hPlayerObj == m_hServerObject;

	m_bIsDead = bIsLocalClient?g_pGameClientShell->GetPlayerStats()->GetHealthCount()==0:!(dwObjectFlags & FLAG_VISIBLE);


	// Update our model lighting.
	if( g_pGameClientShell->GetGameType()->IsSinglePlayer() && !bIsLocalClient )
	{
		LTVector vModelLighting(-1.0f,-1.0f,-1.0f);
		g_pModelLT->GetModelLighting(m_hServerObject, vModelLighting);

		HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SFX_MESSAGE);
		g_pLTClient->WriteToMessageByte(hMessage, SFX_CHARACTER_ID );
		g_pLTClient->WriteToMessageObject(hMessage, m_hServerObject );
		g_pLTClient->WriteToMessageByte(hMessage, CFX_MODELLIGHTING );
		g_pLTClient->WriteToMessageVector(hMessage, &vModelLighting);
		g_pLTClient->EndMessage(hMessage);
	}

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	if ((m_cs.nClientID != 0))
	{
		if (dwUserFlags & USRFLG_CHAR_UNDERWATER)
		{
			UpdateUnderwaterFX(vPos);
		}
	}


	if(m_hFireSystem[0])
		UpdateFireFX();

	if(dwUserFlags & USRFLG_CHAR_NAPALM)
	{
		// Update napalm fire...
		UpdateNapalmFX(vPos);
	}
	else
	{
		RemoveNapalmFX();
	}

	if(m_pFlashlight)
	{
		if(m_bIsDead)
			m_pFlashlight->TurnOff();
		else
			m_pFlashlight->Update();
	}

	// kill the sound if one is already playing
	if(m_hWeaponLoopSound)
	{
		// if we are dead, kill the sound.
		if(m_bIsDead)
		{
			g_pLTClient->KillSound(m_hWeaponLoopSound);
			m_hWeaponLoopSound = LTNULL;
		}
		else
		{
			// make sure the sound is positioned properly
			LTVector vPos;
			g_pInterface->GetObjectPos(m_hServerObject, &vPos);
			g_pLTClient->SetSoundPosition(m_hWeaponLoopSound, &vPos);
		}
	}

	if(m_hAuraSystem)
		UpdateAuraFX();

	if(m_hHeatAuraSystem)
		UpdateHeatAuraFX();

	if(m_hElecAuraSystem)
		UpdateElecAuraFX();

	if(m_hCloakAuraSystem)
		UpdateCloakAuraFX();

	if(m_hFlameEmitters[0])
		UpdateFlameThrowerFX();

	if(m_hChestBurstModel)
		UpdateChestBurstFX();

	m_NodeController.Update();

	if(m_bTargetingSprite)
	{
		if(m_bIsDead)
		{
			//remove the sprites
			for(int i=0; i<3; i++)
			{
				if(m_hTargetingSprites[i])
				{
					g_pLTClient->RemoveObject(m_hTargetingSprites[i]);
					m_hTargetingSprites[i] = LTNULL;
				}
			}
			//now remove the flare
			if (m_hFlare)
			{
				g_pLTClient->RemoveObject(m_hFlare);
				m_hFlare = LTNULL;
			}
			m_bTargetingSprite = LTFALSE;
		}
		else
		{
			UpdateTargetingSprite();
			if(m_hFlare)
				UpdateFlare();
		}
	}

	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(m_hServerObject, &r, &g, &b, &a);

	if(m_bAlphaOverride && a < 1.0f)
	{
		a = 1.0f;
		g_pLTClient->SetObjectColor(m_hServerObject, r, g, b, a);
	}

	SetAttachmentsAlpha(m_hServerObject, a, USRFLG_SPECIAL_ATTACHMENT);

	// Set the shadows...
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();

	if(psfxMgr && psfxMgr->GetMode() != "NightVision")
	{
		if(a < 1.0f)
		{
			g_pLTClient->SetObjectFlags(m_hServerObject, dwObjectFlags & ~FLAG_SHADOW);
		}
	}

	
	// Update the color of attachments
	LTFLOAT fScale = 1.0f - (g_pLTClient->GetTime() - m_fEyeFlashTime) / g_vtEyeFlashTime.GetFloat();
	if(fScale < 0.0f) fScale = 0.0f;

	if(!bIsLocalClient)
	{
		LTVector vColor = m_vEyeFlashColor * fScale;
		SetAttachmentsColor(m_hServerObject, vColor, 0, FLAG2_ADDITIVE);
	}


	// Update any blood emmiters we may have
	if(!GetConsoleInt("LowViolence",0))
		UpdateBleeders();

	// Update our moving...
	LTVector vCurPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vCurPos);
	m_bMoving = (vPos != m_vLastPos);
	m_vLastPos = vPos;

	// Update any Boss FX we may have
	if(m_hBossSmokeSystem)
		UpdateBossFX();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateNapalmFX
//
//	PURPOSE:	Update the on fire from flame thrower FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateNapalmFX(LTVector vPos)
{
	HLOCALOBJ hPlayerObj	= g_pClientDE->GetClientObject();
	LTBOOL 	bIsLocalClient = hPlayerObj == m_hServerObject;

	if(bIsLocalClient && !m_bIsDead)
	{
		//play the fire sound
		if(!m_hFireSound)
			m_hFireSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, "CharOnFire");
		else
		{
			// Be sure the sound is located at the source
			g_pLTClient->SetSoundPosition(m_hFireSound, &vPos);
		}


		//set overlay
		g_pGameClientShell->GetVisionModeMgr()->SetOnFireMode(LTTRUE);
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveNaplamFX
//
//	PURPOSE:	Removes the napalm FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveNapalmFX()
{
	HLOCALOBJ hPlayerObj	= g_pClientDE->GetClientObject();
	LTBOOL 	bIsLocalClient = hPlayerObj == m_hServerObject;

	if(bIsLocalClient)
	{
		//kill the sound
		if(m_hFireSound)
		{
			g_pLTClient->KillSound(m_hFireSound);
			m_hFireSound = LTNULL;
		}

		//set overlay
		g_pGameClientShell->GetVisionModeMgr()->SetOnFireMode(LTFALSE);
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateNaplamFX
//
//	PURPOSE:	Creates the napalm FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateNapalmFX(LTVector vPos)
{
	FIRECREATESTRUCT firestruct;

	firestruct.hServerObj	= m_hServerObject;
	firestruct.nFireObjFXId	= 1;
	
	m_pNapalm = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_FIRE_ID, &firestruct);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateLaserFX
//
//	PURPOSE:	Update the laser fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateLaserFX()
{
	if (!m_hServerObject) return;

	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
	LTBOOL bIsLocalClient = hPlayerObj == m_hServerObject;

	if (bIsLocalClient)
	{
		if (g_pGameClientShell->IsFirstPerson())
		{
			if (m_pLaser)
			{
				m_pLaser->TurnOff();	
				return;
			}
		}
	}

	if (!m_pLaser) 
	{
		m_pLaser = new CLaserBeam;
		if (!m_pLaser) return;
	}
	
	m_pLaser->TurnOn();	
	
	

	// Calculate the laser pos/rot...

	LTVector vPos;
	DRotation rRot;
	
	if (GetAttachmentSocketTransform(m_hServerObject, "Flash", vPos, rRot))
	{
		m_pLaser->Update(vPos, &rRot, LTTRUE);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateUnderwaterFX
//
//	PURPOSE:	Update the underwater fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateUnderwaterFX(LTVector & vPos)
{
	if (!g_pLTClient || !m_hServerObject) return;

	// Check the character butes and see if we have an air supply
	CharacterButes pButes = g_pCharacterButeMgr->GetCharacterButes(m_cs.nCharacterSet);
	if(pButes.m_fAirSupplyTime < 0.0f)
		return;

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	LTVector vU, vR, vF, vTemp;
	DRotation rRot;

	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
	LTBOOL bIsLocalClient = hPlayerObj == m_hServerObject;

	if (bIsLocalClient && 
		g_pGameClientShell->IsFirstPerson() && 
		hCamera) 
	{
		g_pClientDE->GetObjectPos(hCamera, &vPos);
		g_pClientDE->GetObjectRotation(hCamera, &rRot);
		g_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		VEC_MULSCALAR(vTemp, vF, 20.0f);
		VEC_ADD(vPos, vPos, vTemp);

		vPos.y -= 20.0f;

		VEC_MULSCALAR(vTemp, vU, -20.0f);
		VEC_ADD(vPos, vPos, vTemp);
	}
	else
	{
		PhysicsLT* pPhysics = g_pLTClient->Physics();
		LTVector vDims;

		pPhysics->GetObjectDims(m_hServerObject, &vDims);
		g_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		g_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vTemp, vF, 20.0f);
		VEC_ADD(vPos, vPos, vTemp);
		vPos.y += vDims.y * 0.4f;
	}


	CreateUnderwaterFX(vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateUnderwaterFX
//
//	PURPOSE:	Create underwater special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateUnderwaterFX(LTVector & vPos)
{
	if (!g_pLTClient || !g_pGameClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	DFLOAT fTime = g_pLTClient->GetTime();

	if (m_fNextBubbleTime > 0.0f && fTime < m_fNextBubbleTime)
	{
		return;
	}

	m_fNextBubbleTime = fTime + GetRandom(0.75f, 1.5f);

	SMCREATESTRUCT sm;

	VEC_COPY(sm.vPos, vPos);
	sm.vPos.y += 25.0f;

	VEC_SET(sm.vColor1, 100.0f, 100.0f, 100.0f);
	VEC_SET(sm.vColor2, 150.0f, 150.0f, 150.0f);
	VEC_SET(sm.vMinDriftVel, -2.5f, 20.0f, -2.5f);
	VEC_SET(sm.vMaxDriftVel, 2.5f, 40.0f, 2.5f);

	GetLiquidColorRange(CC_WATER, &sm.vColor1, &sm.vColor2);
	char* pTexture	  = DEFAULT_BUBBLE_TEXTURE;

	sm.fVolumeRadius		= 10.0f;
	sm.fLifeTime			= 0.2f;
	sm.fRadius				= 1000;
	sm.fParticleCreateDelta	= 0.1f;
	sm.fMinParticleLife		= 1.0f;
	sm.fMaxParticleLife		= 3.0f;
	sm.nNumParticles		= 3;
	sm.bIgnoreWind			= LTTRUE;
	sm.hstrTexture			= g_pLTClient->CreateString(pTexture);
	sm.bUpdatePos			= LTTRUE;
	sm.bRemoveIfNotLiquid	= LTTRUE;

	psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	g_pLTClient->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveLaserFX
//
//	PURPOSE:	Remove the laser fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveLaserFX()
{
	if (m_pLaser)
	{
		delete m_pLaser;
		m_pLaser = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnModelKey
//
//	PURPOSE:	Handle model key
//
// ----------------------------------------------------------------------- //

void CCharacterFX::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!hObj || !pArgs || !pArgs->argv || pArgs->argc == 0) return;

	char* pKey = pArgs->argv[0];
	if (!pKey) return;


	if (stricmp(pKey, KEY_FOOTSTEP_SOUND) == 0)
	{
		DoFootStepKey(hObj);
	}
	// handle sound fx key
	else if(stricmp(pKey, "BUTE_SOUND_KEY") == 0)
	{
		if (pArgs->argc > 1 && pArgs->argv[1])
		{
			LTVector vPos;
			g_pInterface->GetObjectPos(m_hServerObject, &vPos);
			//play the buted sound
			g_pClientSoundMgr->PlaySoundFromPos(vPos, pArgs->argv[1]);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::DoFootStepKey
//
//	PURPOSE:	Handle model foot step key
//
// ----------------------------------------------------------------------- //

void CCharacterFX::DoFootStepKey(HLOCALOBJ hObj, LTBOOL bForceSound)
{
	if(!hObj) return;
	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
	LTBOOL bIsLocalClient = hPlayerObj == hObj;

	SurfaceType eSurface = ST_UNKNOWN;

	PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();
	if(!pMovement) return;

	if (bIsLocalClient)
	{
		if(!IsFootStepState(pMovement->GetMovementState()))
			return;

		if(pMovement->GetMovementState() == CMS_CLIMBING)//pMovement->IsInContainerType(ST_LADDER))
		{
			eSurface = ST_LADDER;
		}
		else if(pMovement->IsInContainerType(CC_WATER, CM_CONTAINER_LOWER))
		{
			// Feet in water...
			eSurface = ST_LIQUID;
		}

		// Use our current standing on surface if we still don't know what
		// we're standing on...
		if(eSurface == ST_UNKNOWN)
		{
			CharacterVars *pVars = pMovement->GetCharacterVars();

			eSurface = GetSurfaceType(pVars->m_ciStandingOn);
		}
	}


	// Alternate feet...
	m_bLeftFoot = !m_bLeftFoot;


	LTVector vPos, vR, vU, vF;
	LTRotation rRot;
	g_pInterface->GetObjectPos(hObj, &vPos);
	g_pInterface->GetObjectRotation(hObj, &rRot);
	g_pInterface->GetMathLT()->GetRotationVectors(rRot, vR, vU, vF);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;
	memset(&iInfo, 0, sizeof(iInfo));


	// If we still don't know what surface we're standing on, do an intersect
	// segment to figure it out...

	if (eSurface == ST_UNKNOWN)
	{
		if (bIsLocalClient)
		{
			// We shouldn't ever get here if this is a local client...
			// g_pClientDE->CPrint("Local client standing on an unknown surface!");
		}

		PhysicsLT* pPhysics = g_pInterface->Physics();

		iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
		iQuery.m_From  = vPos;

		// If the object has Left/RightFoot sockets, use them to determine
		// the location for the InteresectSegment...

		ModelLT* pModelLT   = g_pClientDE->GetModelLT();

		char* pSocketName = m_bLeftFoot ? "LeftFoot" : "RightFoot";
		HMODELSOCKET hSocket;

		if (pModelLT->GetSocket(hObj, pSocketName, hSocket) == LT_OK)
		{
			LTransform transform;
			if (pModelLT->GetSocketTransform(hObj, hSocket, transform, LTTRUE) == LT_OK)
			{
				TransformLT* pTransLT = g_pClientDE->GetTransformLT();
				pTransLT->GetPos(transform, iQuery.m_From);
			}
		}

		iQuery.m_To	= iQuery.m_From + (-vU * 50.0f);

		// Don't hit ourself...
		HOBJECT hFilterList[] = {g_pInterface->GetClientObject(), pMovement->GetObject(), LTNULL};

		if (bIsLocalClient)
		{
			iQuery.m_FilterFn  = ObjListFilterFn;
			iQuery.m_pUserData = hFilterList;
		}

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{		
			eSurface = GetSurfaceType(iInfo);
		}
		else
		{
			// What the...
			return;
		}
	}


	// Play a footstep sound 
	LTFLOAT	fTime		= g_pLTClient->GetTime();

	if(fTime-m_fLastFootFallTime >  g_vtMinFootstepTime.GetFloat())
	{
		if(bForceSound || !bIsLocalClient || !g_pGameClientShell->IsFirstPerson())
			PlayFootstepSound(vPos, eSurface, m_bLeftFoot);

		//re-set time
		m_fLastFootFallTime = fTime;
	}


	// Leave footprints on the appropriate surfaces...
	if (ShowsFootprints(eSurface))
	{
		// Use intersect position for footprint sprite...
		CreateFootprint(eSurface, iInfo);
	}

}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::GetFootStepSound
//
//	PURPOSE:	Get footstep sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::GetFootStepSound(char* szSound, SurfaceType eSurfType, LTBOOL bLeftFoot)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
	_ASSERT(pSurf);
	if (!pSurf) return;

	int nIndex = GetRandom(0, 1);

	sprintf(szSound, bLeftFoot ? pSurf->szLtFootStepSnds[nIndex]:pSurf->szRtFootStepSnds[nIndex],
				g_pCharacterButeMgr->GetFootStepSoundDir(m_cs.nCharacterSet) );
				
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayFootstepSound
//
//	PURPOSE:	Play footstep sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayFootstepSound(LTVector		vPos, 
									 SurfaceType	eSurface, 
									 LTBOOL			bLeftFoot)
{
	char szSound[255];
	szSound[0] = '\0';

	// Don't do movement sounds if in the menu...
	if (g_pInterfaceMgr->GetGameState() != GS_PLAYING) return;

	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
	LTBOOL bIsLocalClient = hPlayerObj == m_hServerObject;

	GetFootStepSound(szSound, eSurface, bLeftFoot);

	if (szSound[0] != '\0') 
	{
		uint32 dwFlags = bIsLocalClient ? PLAYSOUND_CLIENTLOCAL : 0;
		SoundPriority ePriority = (m_cs.nClientID != 0) ? SOUNDPRIORITY_MISC_HIGH : SOUNDPRIORITY_MISC_LOW;

		int nVolume = 100;//(int) (100.0f * m_cs.fStealthPercent);

		if (bIsLocalClient)
		{
			PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();
			if(!pMovement) return;

			nVolume /= FootStepStateVolumeMod(pMovement->GetMovementState());
		}

		g_pClientSoundMgr->PlaySoundFromPos(vPos, szSound, FOOTSTEP_SOUND_RADIUS, ePriority, dwFlags, nVolume);

		// Make AIs "hear" the sound on the server, if this is the player.
		if (bIsLocalClient && g_pGameClientShell->GetGameType()->IsSinglePlayer())
		{
			HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pClientDE->WriteToMessageByte(hMessage, CP_FOOTSTEP_SOUND);
			g_pClientDE->EndMessage(hMessage);
		}
	}

	// now play the random movement sound
	g_pClientSoundMgr->PlayCharSndFromObject(m_cs.nCharacterSet, m_hServerObject, "Move");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateFootprint
//
//	PURPOSE:	Create a footprint sprite at the specified location
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateFootprint(SurfaceType eType, IntersectInfo & iInfo)
{

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eType);
	if (!pSurf || !pSurf->szLtFootPrintSpr[0] || !pSurf->szRtFootPrintSpr[0]) return;

	LTVector vDir = iInfo.m_Plane.m_Normal;

	// Create a sprite...

	BSCREATESTRUCT scale;
	scale.dwFlags = FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_ROTATEABLESPRITE;
	g_pClientDE->AlignRotation(&(scale.rRot), &vDir, LTNULL);
	
	scale.Filename			= m_bLeftFoot ? pSurf->szLtFootPrintSpr : pSurf->szRtFootPrintSpr;
	scale.vPos				= iInfo.m_Point + vDir;
	scale.vInitialScale		= LTVector(0.5, 0.5, 1.0f);
	scale.vFinalScale		= scale.vInitialScale;
	scale.fLifeTime			= 10.0f;
	scale.fInitialAlpha		= 1.0f;
	scale.fFinalAlpha		= 0.0f;
	scale.nType				= OT_SPRITE;

	CSpecialFX* pFX = LTNULL;
	pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_SCALE_ID, &scale);

	if (pFX) 
	{
		pFX->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::OnServerMessage(HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::OnServerMessage(hMessage)) return LTFALSE;

	DBYTE	nMsgId = g_pClientDE->ReadFromMessageByte(hMessage);
	LTBOOL	bIsLocalClient = (g_pClientDE->GetClientObject() == m_hServerObject);

	switch(nMsgId)
	{
		case CFX_NODECONTROL_RECOIL_MSG:
		case CFX_NODECONTROL_LIP_SYNC:
		case CFX_NODECONTROL_HEAD_FOLLOW_OBJ:
		case CFX_NODECONTROL_HEAD_FOLLOW_POS:
		case CFX_NODECONTROL_SCRIPT:
		case CFX_NODECONTROL_AIMING_PITCH:
		case CFX_NODECONTROL_AIMING_YAW:
		case CFX_NODECONTROL_STRAFE_YAW:
		{
			m_NodeController.HandleNodeControlMessage(nMsgId, hMessage);
		}
		break;

		case CFX_SUBTITLES:
		{

			HSTRING hSound = g_pLTClient->ReadFromMessageHString(hMessage);
			LTFLOAT fRadius = g_pLTClient->ReadFromMessageFloat(hMessage);
			LTFLOAT fDuration = g_pLTClient->ReadFromMessageFloat(hMessage);

			if (hSound)
			{
				LTVector speakerPos;
				char szSound[128];

				SAFE_STRCPY(szSound, g_pLTClient->GetStringData(hSound));
				g_pLTClient->FreeString(hSound);

				g_pLTClient->GetObjectPos(m_hServerObject,&speakerPos);
				// Start Captioning
				g_pInterfaceMgr->SetCaption(szSound,speakerPos,fRadius, fDuration );
			}
			else
				g_pInterfaceMgr->StopCaptions();

		}
		break;

		case CFX_RESET_MAINTRACKER:
		{
			g_pClientDE->ResetModelAnimation(m_hServerObject);
		}
		break;

		case CFX_RESET_ATTRIBUTES:
		{
			uint8 nSet = g_pClientDE->ReadFromMessageByte(hMessage);

			if(nSet != m_cs.nCharacterSet)
			{
				m_cs.nCharacterSet = nSet;
				ModelSkeleton eNewSkele = (ModelSkeleton)g_pCharacterButeMgr->GetSkeletonIndex(m_cs.nCharacterSet);

				// Make sure we reset the SFX if we have a new skele...
				if(eNewSkele != m_cs.eModelSkeleton)
				{
					m_cs.eModelSkeleton = eNewSkele;
					ResetSpecialFX();
					m_NodeController.Init(this, LTNULL, bIsLocalClient);
				}

				// Reset the vision mode state
				SetMode(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName());

				// If this is MP and I am local then be sure my 
				// flashlight is off...
				if(!g_pGameClientShell->GetGameType()->IsSinglePlayer())
				{
					if(bIsLocalClient)
					{
						g_pGameClientShell->GetFlashlight()->TurnOff();
					}

					// reset our model here...
					CharacterButes Butes = g_pCharacterButeMgr->GetCharacterButes(nSet);
					ObjectCreateStruct theStruct;
					theStruct.Clear();

					Butes.FillInObjectCreateStruct(theStruct, LTTRUE);
					g_pLTClient->Common()->SetObjectFilenames(m_hServerObject, &theStruct);
				}
			}

			// Always do this...
			SetMode(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName());
		}
		break;

		case CFX_FLASHLIGHT:
		{
			if( m_pFlashlight )
				m_pFlashlight->SetRotation(hMessage->ReadRotation());
		}
		break;

		case CFX_TARGETING_SPRITE:
		{
			// handle turning on and off the targeting sprite effect
			m_bTargetingSprite = g_pLTClient->ReadFromMessageByte(hMessage);

			if(m_bTargetingSprite)
			{
				// load up the targeting sprites and the flare
				CreateTargetingSprites();
			}
			else
			{
				//remove the sprites
				for(int i=0; i<3; i++)
				{
					if(m_hTargetingSprites[i])
					{
						g_pLTClient->RemoveObject(m_hTargetingSprites[i]);
						m_hTargetingSprites[i] = LTNULL;
					}
				}
				//now remove the flare
				if (m_hFlare)
				{
					g_pLTClient->RemoveObject(m_hFlare);
					m_hFlare = LTNULL;
				}
			}
		}
		break;

		case CFX_FLAME_FX:
		{
			// handle turning on and off the targeting sprite effect
			LTBOOL bDoFX = g_pLTClient->ReadFromMessageByte(hMessage);

			if(bDoFX)
			{
				if(!m_hFlameEmitters[0])
					CreateFlameThrowerFX();
			}
			else
			{
				// Clean up the flame thrower FX
				for(int i=0;i<MAX_FLAME_SEGMENTS;i++)
				{
					if(m_hFlameEmitters[i])
					{
						//remove the system
						g_pLTClient->RemoveObject(m_hFlameEmitters[i]);
						m_hFlameEmitters[i] = LTNULL;
					}
				}
				memset(m_fFlameStartTimes, 0, MAX_FLAME_SEGMENTS * sizeof(LTFLOAT));
				memset(m_fFlameEndTimes, 0, MAX_FLAME_SEGMENTS * sizeof(LTFLOAT));
			}
		}
		break;

		case CFX_FLAME_FX_SHOT:
		{
			// don't process these messages if this is our shot and
			// we are playing multiplayer
			if(g_pGameClientShell->IsMultiplayerGame() && bIsLocalClient)
				return LTTRUE;

			ResetFlameThrowerFX();
		}
		break;

		case CFX_EYEFLASH:
		{
			// Get the color of the eye flash
			uint32 nColor = g_pLTClient->ReadFromMessageDWord(hMessage);

			m_vEyeFlashColor.x = (LTFLOAT)GETR(nColor) / 255.0f;
			m_vEyeFlashColor.y = (LTFLOAT)GETG(nColor) / 255.0f;
			m_vEyeFlashColor.z = (LTFLOAT)GETB(nColor) / 255.0f;

			m_fEyeFlashTime = g_pLTClient->GetTime();
		}
		break;

		case CFX_BLEEDER:
		{
			// Get the bleeder node
			ModelNode eBleederNode = (ModelNode)g_pLTClient->ReadFromMessageWord(hMessage);

			// Create the new bleeder
			AddNewBleeder(eBleederNode);
		}
		break;

		case CFX_INFO_STRING:
		{
			// Go ahead and clear the string first... no biggy!
			memset(m_szInfoString, 0, 256);

			// Now read it in one size less to make sure it doesn't overflow (not sure if
			// LT does that for us or not)
			hMessage->ReadStringFL(m_szInfoString, 255);
		}
		break;

		case CFX_POUNCE_SOUND:
		{
			uint8 nType = g_pLTClient->ReadFromMessageByte(hMessage);

			// Only play the sound if we are not local...
			if(!bIsLocalClient)
			{
				switch(nType)
				{
					case 1:		g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, "AlienPounceCry3D");		break;
					case 2:		g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, "FacehuggerPounceCry3D");	break;
					case 3:		g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, "PredalienPounceCry3D");	break;
					default:	g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, "AlienPounceCry3D");		break;
				}
			}
		}
		break;

		case CFX_PLAY_LOOP_FIRE:
		{
			// Only play the sound if we are not local...
			if(!bIsLocalClient)
			{
				HandleMessage_WeaponLoopSound(hMessage);
			}
		}
		break;

		case CFX_KILL_LOOP_FIRE:
		{
			// kill the sound if one is already playing
			if(m_hWeaponLoopSound)
			{
				g_pLTClient->KillSound(m_hWeaponLoopSound);
				m_hWeaponLoopSound = LTNULL;
			}
		}
		break;

		case CFX_JUMP_SOUND:
		{
			// Only play the sound if we are not local...
			if(!bIsLocalClient)
			{
				g_pClientSoundMgr->PlayCharSndFromObject(m_cs.nCharacterSet, m_hServerObject, "Jump");
			}
		}
		break;

		case CFX_LAND_SOUND:
		{
			// Only play the sound if we are not local...
			if(!bIsLocalClient)
			{
				g_pClientSoundMgr->PlayCharSndFromObject(m_cs.nCharacterSet, m_hServerObject, "Land");
			}
		}
		break;

		case CFX_FOOTSTEP_SOUND:
		{
			// Only play the sound if we are not local...
			if(!bIsLocalClient)
			{
				ForceFootstep();
			}
		}
		break;

		case CFX_BOSS_SMOKE_FX:
		{
			// See what we are suposed to do...
			LTBOOL bOn = g_pLTClient->ReadFromMessageByte(hMessage);

			if(bOn)
			{
				// Create the system if it doesn't exist
				if(!m_hBossSmokeSystem)
				{
					CreateBossFX();
				}
				// Increment the intensity...
				if(m_nBossSmokeLevel < MAX_BOSS_SMOKE_INTENSITY)
					++m_nBossSmokeLevel;
			}
			else
			{
				// Clean up the boss smoke system...
				if(m_hBossSmokeSystem)
				{
					//remove the system
					g_pLTClient->RemoveObject(m_hBossSmokeSystem);
					m_hBossSmokeSystem = LTNULL;
				}
			}
		}
		break;

		case CFX_CHESTBURST_FX:
		{
			CreateChestBurstFX();
		}
		break;

		default : break;
	}

	return LTTRUE;
}


void CCharacterFX::OnObjectRemove(HOBJECT hObj)
{

	m_NodeController.OnObjectRemove(hObj);

	CSpecialFX::OnObjectRemove(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateChestBurstFX
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateChestBurstFX()
{
	// If we already created the model... just return
	if(m_hChestBurstModel) return;

	// Check to see if this body prop has a stomach socket
	HMODELSOCKET hSocket;
	g_pLTClient->GetModelLT()->GetSocket(m_hServerObject, "Stomach", hSocket);

	if(hSocket != INVALID_MODEL_SOCKET)
	{
		// Go ahead and create the model
		ObjectCreateStruct ocs;
		ocs.m_Flags = FLAG_VISIBLE;
		ocs.m_ObjectType = OT_MODEL;

		LTransform trans;
		g_pLTClient->GetModelLT()->GetSocketTransform(m_hServerObject, hSocket, trans, LTTRUE);
		g_pLTClient->GetTransformLT()->Get(trans, ocs.m_Pos, ocs.m_Rotation);

		// Get the rotation vectors
		LTVector vUnused, vDir;
		g_pLTClient->GetMathLT()->GetRotationVectors(ocs.m_Rotation, vUnused, vUnused, vDir);

		// Get the user flags of the server object to determine the surface type
		uint32 dwUserFlags;
		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);
		SurfaceType eSurfaceType = UserFlagToSurface(dwUserFlags);

		// Choose the appropriate model and skin
		switch(eSurfaceType)
		{
			case ST_FLESH_PREDATOR:
				strcpy(ocs.m_Filename, "Models\\Props\\chestburst.abc");
				strcpy(ocs.m_SkinNames[0], "Skins\\Props\\chestburst2.dtx");
				break;

			default:
				strcpy(ocs.m_Filename, "Models\\Props\\chestburst.abc");
				strcpy(ocs.m_SkinNames[0], "Skins\\Props\\chestburst.dtx");
				break;
		}

		m_hChestBurstModel = g_pLTClient->CreateObject(&ocs);


		// Now create some GIB and blood FX based off the surface type
		if(!GetConsoleInt("LowViolence",0))
		{
			DEBRIS *pDebris = LTNULL;
			CPShowerFX *pPShower = LTNULL;

			switch(eSurfaceType)
			{
				case ST_FLESH_PREDATOR:
					pDebris = g_pDebrisMgr->GetDebris("Predator_Medium");
					pPShower = g_pFXButeMgr->GetPShowerFX("Blood_Shower_Predator_Huge");
					break;

				default:
					pDebris = g_pDebrisMgr->GetDebris("Human_Medium");
					pPShower = g_pFXButeMgr->GetPShowerFX("Blood_Shower_Human_Huge");
					break;
			}


			// Try to create the debris FX
			CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
			if(!psfxMgr) return;

			DEBRISCREATESTRUCT dcs;
			dcs.nDebrisId = pDebris->nId;
			dcs.vPos = ocs.m_Pos;
			dcs.rRot = ocs.m_Rotation;
			psfxMgr->CreateSFX(SFX_DEBRIS_ID, &dcs);

			if(pPShower)
				g_pFXButeMgr->CreatePShowerFX(pPShower, ocs.m_Pos, vDir, vDir);
		}


		// Play a gruesome sound!! Bwah ha ha ha!!
		g_pClientSoundMgr->PlaySoundFromPos(ocs.m_Pos, "Sounds\\Weapons\\Alien\\FaceHug\\chestburst.wav", 1000.0f, SOUNDPRIORITY_MISC_HIGH);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateChestBurstFX
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateChestBurstFX()
{
	// If we don't have a model... just get out of here
	if(!m_hChestBurstModel) return;


	// Check to see if this body prop has a stomach socket
	HMODELSOCKET hSocket;
	g_pLTClient->GetModelLT()->GetSocket(m_hServerObject, "Stomach", hSocket);

	// Delete the model if the socket doesn't exist
	if(hSocket == INVALID_MODEL_SOCKET)
	{
		g_pLTClient->DeleteObject(m_hChestBurstModel);
		m_hChestBurstModel = LTNULL;
		return;
	}


	// Now we know our model and socket are ok... set update the position and rotation
	// of the chest burst model so it follow the stomach socket
	LTransform trans;
	LTVector vPos;
	LTRotation rRot;

	g_pLTClient->GetModelLT()->GetSocketTransform(m_hServerObject, hSocket, trans, LTTRUE);
	g_pLTClient->GetTransformLT()->Get(trans, vPos, rRot);

	g_pLTClient->SetObjectPos(m_hChestBurstModel, &vPos);
	g_pLTClient->SetObjectRotation(m_hChestBurstModel, &rRot);


	// Now make sure the color is the same as the body prop... alpha too
	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(m_hServerObject, &r, &g, &b, &a);
	g_pLTClient->SetObjectColor(m_hChestBurstModel, r, g, b, a);
}


void CCharacterFX::SetMode(const std::string& mode)
{
	// Check to see if we are in the default mode
	HLOCALOBJ hPlayerObj	= g_pClientDE->GetClientObject();
	LTBOOL 	bIsLocalClient = hPlayerObj == m_hServerObject;

	uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);

	if ("None" == mode || "Normal" == mode || "NightVision" == mode || "Hunting" == mode)
	{
		ObjectCreateStruct newSkins;
		newSkins.Clear();

		CString modelName = g_pCharacterButeMgr->GetDefaultModel(m_cs.nCharacterSet);

		SAFE_STRCPY(newSkins.m_Filename, modelName.GetBuffer());

		CString skinDir = g_pCharacterButeMgr->GetDefaultSkinDir(m_cs.nCharacterSet);

		for (int i = 0; i < MAX_MODEL_TEXTURES; ++i)
		{
			CString skinName = g_pCharacterButeMgr->GetDefaultSkin(m_cs.nCharacterSet, i, skinDir.GetBuffer());
			SAFE_STRCPY(newSkins.m_SkinNames[i], skinName.GetBuffer()); 
		}

		// Set this up to not interupt the current animation.
		newSkins.m_bResetAnimations = LTFALSE;

		g_pLTClient->Common()->SetObjectFilenames(m_hServerObject, &newSkins);

		// Turn on shadows
		if("NightVision" == mode)
			dwFlags &= ~FLAG_SHADOW;
		else
			dwFlags |= FLAG_SHADOW;

		// Reset the alpha
		if(m_bAlphaOverride)
		{
			m_bAlphaOverride = LTFALSE;

			uint32 dwFlags;
			g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);

			if(dwFlags & USRFLG_CHAR_CLOAKED)
			{
				LTFLOAT r, g, b, a;
				g_pLTClient->GetObjectColor(m_hServerObject, &r, &g, &b, &a);

				LTVector vVel;
				m_pClientDE->Physics()->GetVelocity(m_hServerObject, &vVel);
				
				a = vVel.MagSqr()==0.0f?0.0f:0.15f;
				g_pLTClient->SetObjectColor(m_hServerObject, r, g, b, a);
			}
		}
	}
	else
	{
		ObjectCreateStruct newSkins;
		newSkins.Clear();

		CString modelName = g_pCharacterButeMgr->GetDefaultModel(m_cs.nCharacterSet);

		SAFE_STRCPY(newSkins.m_Filename, modelName.GetBuffer());

		CString skinDir = g_pCharacterButeMgr->GetDefaultSkinDir(m_cs.nCharacterSet);
		skinDir += mode.c_str();
		skinDir += "\\";

		for (int i = 0; i < MAX_MODEL_TEXTURES; ++i)
		{
			CString skinName = g_pCharacterButeMgr->GetDefaultSkin(m_cs.nCharacterSet, i, skinDir.GetBuffer());
			SAFE_STRCPY(newSkins.m_SkinNames[i], skinName.GetBuffer()); 
		}

		// check for .spr swap
		LTBOOL bSwap=LTFALSE;

		if("ElectroVision" == mode && IsAlien(m_cs.nCharacterSet)) bSwap=LTTRUE;
		else if("NavVision" == mode && IsPredator(m_cs.nCharacterSet)) bSwap=LTTRUE;
		else if("HeatVision" == mode && (IsHuman(m_cs.nCharacterSet) || IsSynthetic(m_cs.nCharacterSet) || IsExosuit(m_cs.nCharacterSet))) bSwap=LTTRUE;

		if(bSwap)
		{
			//yoink the .dtx and replace with .spr
			for (i = 0; i < MAX_MODEL_TEXTURES; ++i)
			{
				LPTSTR pStr = strstr(newSkins.m_SkinNames[i], ".dtx");
				if(pStr)
					SAFE_STRCPY(pStr, ".spr"); 
			}
		}

		// Set this up to not interupt the current animation.
		newSkins.m_bResetAnimations = LTFALSE;

		// This is OK since if it can't find the model it will bail out.
		g_pLTClient->Common()->SetObjectFilenames(m_hServerObject, &newSkins);

		// Turn off shadows
		dwFlags &= ~FLAG_SHADOW;

		// Reset the alpha
		if(m_bAlphaOverride)
		{
			uint32 dwFlags;
			g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);

			if(dwFlags & USRFLG_CHAR_CLOAKED)
			{
				LTFLOAT r, g, b, a;
				g_pLTClient->GetObjectColor(m_hServerObject, &r, &g, &b, &a);

				LTVector vVel;
				m_pClientDE->Physics()->GetVelocity(m_hServerObject, &vVel);
				
				a = vVel.MagSqr()==0.0f?0.0f:0.15f;
				g_pLTClient->SetObjectColor(m_hServerObject, r, g, b, a);
			}
		}

		// Turn on or off the alpha override
		m_bAlphaOverride = IsPredator(m_cs.nCharacterSet) && ("NavVision" == mode) && !bIsLocalClient;
	}

	//now set our object flags
	g_pLTClient->SetObjectFlags(m_hServerObject, dwFlags);
}


void CCharacterFX::WantRemove(LTBOOL bRemove)
{
	// Be sure to remove the trackers.
	if( bRemove && m_hServerObject )
	{
		// Remove the trackers.
		for (int i = 1; i < m_cs.nNumTrackers; i++ )
		{
			g_pModelLT->RemoveTracker(m_cs.hServerObj, &m_aAnimTrackers[i]);
		}

		// Remove the node controller.
		m_NodeController.Term();
	}


	CSpecialFX::WantRemove(bRemove);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateFireFX
//
//	PURPOSE:	Create the Fire FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateFireFX()
{
#ifdef _DEMO
	return;
#endif

	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = 0;
	ocs.m_ObjectType = OT_PARTICLESYSTEM;
	ocs.m_Flags2 = FLAG2_ADDITIVE;

	for(int i=0; i<3; ++i)
	{
		m_hFireSystem[i] = g_pLTClient->CreateObject(&ocs);

		if(m_hFireSystem[i])
		{
			g_pLTClient->SetObjectColor(m_hFireSystem[i], 1.0f,1.0f,1.0f,0.1f);

			char buf[36];
			sprintf(buf, "sprites\\sfx\\particles\\onfire%d.spr", i);

			g_pLTClient->SetupParticleSystem(m_hFireSystem[i], buf, 0, 0, 500.0f);
		}
	}

	ModelSkeleton eModelSkeleton = GetModelSkeleton();

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	const char* szNodeName = DNULL;

	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		uint32 nIndex = iNode%3;
		ModelNode eCurrentNode = (ModelNode)iNode;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
		
		if (szNodeName)
		{
			LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eCurrentNode);

			LTVector vDummy(0.0f, 0.0f, 0.0f);
			LTParticle *p = g_pLTClient->AddParticle(m_hFireSystem[nIndex], &ocs.m_Pos, &vDummy, &vDummy, 10000.0f);

			p->m_Size = fRadius;

			//TEMP
			p->m_Color = LTVector(125.0, 125.0, 125.0);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateHeatAuraFX
//
//	PURPOSE:	Create the Heat aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateHeatAuraFX()
{
	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);
	SurfaceType eSurfaceType = UserFlagToSurface(dwFlags);
	if(eSurfaceType != ST_FLESH_HUMAN) return;

	//ok we are not a synth so go ahead and make the aura system
	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = 0;
	ocs.m_ObjectType = OT_PARTICLESYSTEM;
	ocs.m_Flags2 = FLAG2_ADDITIVE;
	m_hHeatAuraSystem = g_pLTClient->CreateObject(&ocs);

	if(m_hHeatAuraSystem)
	{
		g_pLTClient->SetObjectColor(m_hHeatAuraSystem, 1.0f,1.0f,1.0f,0.1f);

		g_pLTClient->SetupParticleSystem(m_hHeatAuraSystem, "sprites\\sfx\\particles\\heat.spr", 0, 0, 500.0f);

		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
			
			if (szNodeName)
			{
				LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eCurrentNode);

				LTVector vDummy(0.0f, 0.0f, 0.0f);
				LTParticle *p = g_pLTClient->AddParticle(m_hHeatAuraSystem, &ocs.m_Pos, &vDummy, &vDummy, 10000.0f);

				p->m_Size = fRadius;
				p->m_Color = LTVector(14,202,212);
			} 
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateElecAuraFX
//
//	PURPOSE:	Create the Electro aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateElecAuraFX()
{
	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);
	SurfaceType eSurfaceType = UserFlagToSurface(dwFlags);

	if((eSurfaceType != ST_FLESH_ALIEN) && (eSurfaceType != ST_FLESH_PRAETORIAN))
		return;

	//ok we are not a synth so go ahead and make the aura system
	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = 0;
	ocs.m_ObjectType = OT_PARTICLESYSTEM;
	ocs.m_Flags2 = FLAG2_ADDITIVE;
	m_hElecAuraSystem = g_pLTClient->CreateObject(&ocs);

	if(m_hElecAuraSystem)
	{
		g_pLTClient->SetObjectColor(m_hElecAuraSystem, 1.0f,1.0f,1.0f,0.1f);

		g_pLTClient->SetupParticleSystem(m_hElecAuraSystem, "sprites\\sfx\\particles\\e_aura.spr", 0, 0, 500.0f);

		for (int i = 0; i < MAX_ELEC_EFFECTS; i++)
		{
			LTVector vDummy(0.0f, 0.0f, 0.0f);
			LTParticle *p = g_pLTClient->AddParticle(m_hElecAuraSystem, &ocs.m_Pos, &vDummy, &vDummy, 10000.0f);
			p->m_Color = LTVector(37,187,201);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateCloakAuraFX
//
//	PURPOSE:	Create the Cloak aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateCloakAuraFX()
{
	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);
	SurfaceType eSurfaceType = UserFlagToSurface(dwFlags);
	if(eSurfaceType != ST_FLESH_PREDATOR) return;

	//ok we are not a synth so go ahead and make the aura system
	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = 0;
	ocs.m_ObjectType = OT_PARTICLESYSTEM;
	ocs.m_Flags2 = FLAG2_ADDITIVE;
	m_hCloakAuraSystem = g_pLTClient->CreateObject(&ocs);

	if(m_hCloakAuraSystem)
	{
		g_pLTClient->SetObjectColor(m_hCloakAuraSystem, 1.0f,1.0f,1.0f,0.1f);

		g_pLTClient->SetupParticleSystem(m_hCloakAuraSystem, "sprites\\cloak.spr", 0, 0, 500.0f);

		for (int i = 0; i < MAX_CLOAK_EFFECTS; i++)
		{
			LTVector vDummy(0.0f, 0.0f, 0.0f);
			LTParticle *p = g_pLTClient->AddParticle(m_hCloakAuraSystem, &ocs.m_Pos, &vDummy, &vDummy, 10000.0f);
			p->m_Color = LTVector(37,187,201);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateAuraFX
//
//	PURPOSE:	Create the aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateAuraFX()
{
	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);
	SurfaceType eSurfaceType = UserFlagToSurface(dwFlags);
	if(eSurfaceType == ST_FLESH_SYNTH) return;

	//ok we are not a synth so go ahead and make the aura system
	ObjectCreateStruct ocs;
	g_pLTClient->GetObjectPos(m_hServerObject, &ocs.m_Pos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &ocs.m_Rotation);
	ocs.m_Flags = 0;
	ocs.m_ObjectType = OT_PARTICLESYSTEM;
	ocs.m_Flags2 = FLAG2_ADDITIVE;
	m_hAuraSystem = g_pLTClient->CreateObject(&ocs);

	if(m_hAuraSystem)
	{
		g_pLTClient->SetObjectColor(m_hAuraSystem, 1.0f,1.0f,1.0f,0.1f);

		g_pLTClient->SetupParticleSystem(m_hAuraSystem, "sprites\\sfx\\particles\\aura.spr", 0, 0, 500.0f);

		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
			
			if (szNodeName)
			{
				//use a radius 10% larger then the node radius
				LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eCurrentNode)*1.1f ;

				LTVector vDummy(0.0f, 0.0f, 0.0f);
				LTParticle *p = g_pLTClient->AddParticle(m_hAuraSystem, &ocs.m_Pos, &vDummy, &vDummy, 10000.0f);

				p->m_Size = fRadius;
				p->m_Color = GetAuraColor();
			} 
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateFlashlight
//
//	PURPOSE:	Create the flashlight
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateFlashlight()
{
	if(m_pFlashlight)
		delete m_pFlashlight;

	m_pFlashlight = new CFlashLightAI();

	m_pFlashlight->Init(m_hServerObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::GetAuraColor
//
//	PURPOSE:	Get the aura color
//
// ----------------------------------------------------------------------- //

LTVector CCharacterFX::GetAuraColor(LTBOOL bAutoTargeted)
{
	LTVector rVal = LTVector(0,0,0);

	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwFlags);

	SurfaceType eSurfaceType = UserFlagToSurface(dwFlags);

	switch(eSurfaceType)
	{
		case ST_FLESH_PREDATOR:
			rVal = g_pClientButeMgr->GetPredAuraColor(); //!bAutoTargeted?LTVector(56,249,18):LTVector(56,249,18);
			break;

		case ST_FLESH_ALIEN:
		case ST_FLESH_PRAETORIAN:
			rVal = g_pClientButeMgr->GetAlienAuraColor(); //!bAutoTargeted?LTVector(221,239,12):LTVector(221,239,12);
			break;

		case ST_FLESH_HUMAN:
		default:
			rVal = !bAutoTargeted ?  g_pClientButeMgr->GetHumanAuraColor() : g_pClientButeMgr->GetSelHumanAuraColor();  // !bAutoTargeted ? LTVector(17,132,178):LTVector(17,132,178);
			break;
	}

	return rVal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::GetNodePos
//
//	PURPOSE:	Get the position of the node
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::GetNodePos(const char* szNodeName, LTVector &vPos, LTBOOL bWorldPos)
{
	//see about head bite
	ModelLT* pModelLT   = g_pClientDE->GetModelLT();

	char pTemp[255];
	sprintf(pTemp,szNodeName);

	HMODELNODE hNode;
	HMODELSOCKET hSocket;

	if (pModelLT->GetNode(m_hServerObject, pTemp, hNode) == LT_OK)
	{
		LTransform	tTransform;

		if (pModelLT->GetNodeTransform(m_hServerObject, hNode, tTransform, bWorldPos) == LT_OK)
		{
			TransformLT* pTransLT = g_pLTClient->GetTransformLT();
			pTransLT->GetPos(tTransform, vPos);
			return LTTRUE;
		}
	}
	else if (pModelLT->GetSocket(m_hServerObject, pTemp, hSocket) == LT_OK)
	{
		LTransform	tTransform;

		if (pModelLT->GetSocketTransform(m_hServerObject, hSocket, tTransform, bWorldPos) == LT_OK)
		{
			TransformLT* pTransLT = g_pLTClient->GetTransformLT();
			pTransLT->GetPos(tTransform, vPos);
			return LTTRUE;
		}
	}
	else
	{
		// Humm... Problems...
		g_pLTClient->CPrint("CharacterFX (%s) ERROR: Couldn't find node or socket named %s.", g_pCharacterButeMgr->GetName(m_cs.nCharacterSet), szNodeName);
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateFireFX
//
//	PURPOSE:	Create the aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateFireFX()
{
	if(m_hFireSystem[0])
	{
		uint32 dwUsrFlags;
		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
		HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
		LTBOOL 	bIsLocalClient = hPlayerObj == m_hServerObject;
		LTBOOL bHide = bIsLocalClient && g_pGameClientShell->IsFirstPerson();

		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		//if this character is not visible or not on fire
		if(!bHide && (dwUsrFlags & USRFLG_CHAR_NAPALM) && (dwFlags & FLAG_VISIBLE))
		{
			for(int i=0; i<3; ++i)
			{
				dwFlags = g_pLTClient->GetObjectFlags(m_hFireSystem[i]);
				g_pLTClient->SetObjectFlags(m_hFireSystem[i], dwFlags | FLAG_VISIBLE);
			}

			//play the fire sound (don't mess with sound if I am the local client)
			if(!bIsLocalClient)
			{
				if(!m_hFireSound)
					m_hFireSound = g_pClientSoundMgr->PlaySoundFromObject(m_hServerObject, "CharOnFire");
				else
				{
					// Be sure the sound is located at the source
					g_pLTClient->SetSoundPosition(m_hFireSound, &vPos);
				}
			}
		}
		else
		{
			for(int i=0; i<3; ++i)
			{
				dwFlags = g_pLTClient->GetObjectFlags(m_hFireSystem[i]);
				g_pLTClient->SetObjectFlags(m_hFireSystem[i], dwFlags & ~FLAG_VISIBLE);
			}

			//kill the sound (don't mess with sound if I am the local client)
			if(!bIsLocalClient)
			{
				if(m_hFireSound)
				{
					g_pLTClient->KillSound(m_hFireSound);
					m_hFireSound = LTNULL;
				}
			}

			return;
		}
		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

		for(int i=0; i<3; ++i)
		{
			g_pLTClient->SetObjectPos(m_hFireSystem[i], &vPos);
			g_pLTClient->SetObjectRotation(m_hFireSystem[i], &rRot);
		}

		//now update the particles
		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		LTParticle *pHead[3];
		LTParticle *pTail[3];

		for(i=0; i<3; ++i)
			g_pLTClient->GetParticles(m_hFireSystem[i], &pHead[i], &pTail[i]);

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			uint32 nIndex = iNode%3;
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);

			const char* szPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkeleton, eCurrentNode);
			
			if(szPiece[0] != '\0')
			{
				HMODELPIECE hPiece = INVALID_MODEL_PIECE;

				// Make the piece invisible
				if(g_pModelLT->GetPiece(m_hServerObject, const_cast<char*>(szPiece), hPiece) == LT_OK)
				{
					LTBOOL bHidden;

					g_pModelLT->GetPieceHideStatus(m_hServerObject, hPiece, bHidden);

					if(bHidden)
						pHead[nIndex]->m_Size = 0.0f;
					else if (szNodeName)
					{
						LTVector vPos;
						GetNodePos(szNodeName, vPos);
						
						g_pLTClient->SetParticlePos(m_hFireSystem[nIndex], pHead[nIndex], &vPos);
					}
				}
			}
			else
			{
				// No piece for this node so hide it...
				pHead[nIndex]->m_Size = 0.0f;
			}

			// Move on to the next node...
			pHead[nIndex] = pHead[nIndex]->m_pNext;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateHeatAuraFX
//
//	PURPOSE:	Update the Heat aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateHeatAuraFX()
{
	if(m_hHeatAuraSystem)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hHeatAuraSystem);
		uint32 dwSOFlags = g_pLTClient->GetObjectFlags(m_hServerObject);

		// Check to see if the character is visible or not
		if(	VisionMode::g_pButeMgr->IsPredatorHeatVision(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName()) &&
			(dwSOFlags & FLAG_VISIBLE))
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else
		{
			dwFlags &= ~FLAG_VISIBLE;
		}

		g_pLTClient->SetObjectFlags(m_hHeatAuraSystem, dwFlags);

		if(!(dwFlags & FLAG_VISIBLE))
			return;


		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pLTClient->SetObjectPos(m_hHeatAuraSystem, &vPos);

		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		g_pLTClient->SetObjectRotation(m_hHeatAuraSystem, &rRot);
		//now update the particles
		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		LTParticle *pHead;
		LTParticle *pTail;
		g_pLTClient->GetParticles(m_hHeatAuraSystem, &pHead, &pTail);

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
			
			if (szNodeName)
			{
				LTVector vPos;
				GetNodePos(szNodeName, vPos);
				
				g_pLTClient->SetParticlePos(m_hHeatAuraSystem, pHead, &vPos);

				pHead = pHead->m_pNext;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateElecAuraFX
//
//	PURPOSE:	Update the Electro aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateElecAuraFX()
{
	if(m_hElecAuraSystem)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hElecAuraSystem);
		uint32 dwSOFlags = g_pLTClient->GetObjectFlags(m_hServerObject);

		if(	VisionMode::g_pButeMgr->IsPredatorElecVision(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName()) &&
			(dwSOFlags & FLAG_VISIBLE))
		{
			dwFlags |= FLAG_VISIBLE;
		}
		else
		{
			dwFlags &= ~FLAG_VISIBLE;
		}

		g_pLTClient->SetObjectFlags(m_hElecAuraSystem, dwFlags);

		if(!(dwFlags & FLAG_VISIBLE))
			return;

		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pLTClient->SetObjectPos(m_hElecAuraSystem, &vPos);

		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		g_pLTClient->SetObjectRotation(m_hElecAuraSystem, &rRot);
		//now update the particles
		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		LTParticle *pHead;
		LTParticle *pTail;
		g_pLTClient->GetParticles(m_hElecAuraSystem, &pHead, &pTail);

		//see if it is time to update (this is done to seperate the look of the effect from frame rate)
		if(g_pLTClient->GetTime() - m_ElecEffectData.fStartTime > g_vtElecUpdateTime.GetFloat())
		{
			//yep time to update

			//set new nodes
			for(int i = 0 ; i < MAX_ELEC_EFFECTS ; i++)
			{
				m_ElecEffectData.eModelNode[i] = (ModelNode)GetRandom(0, cNodes-1);
			}
			//time stamp
			m_ElecEffectData.fStartTime = g_pLTClient->GetTime();
		}

		for ( int i = 0 ; i < MAX_ELEC_EFFECTS ; i++ )
		{
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, m_ElecEffectData.eModelNode[i]);
		
			if (szNodeName)
			{
					GetNodePos(szNodeName, vPos);
					g_pLTClient->SetParticlePos(m_hElecAuraSystem, pHead, &vPos);

					LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, m_ElecEffectData.eModelNode[i]);
					pHead->m_Size = fRadius * GetRandom(0.5f, 0.8f);
			}

			pHead = pHead->m_pNext;
		}
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::BuildCloakNodeList
//
//	PURPOSE:	Build and sort the cloak node list
//
// ----------------------------------------------------------------------- //

void CCharacterFX::BuildCloakNodeList()
{
	ModelSkeleton eModelSkeleton = GetModelSkeleton();

	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	const char* szNodeName = DNULL;

	// build the array and record the height and the node index
	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
		
		if (szNodeName)
		{
			LTVector vPos;
			GetNodePos(szNodeName, vPos);

			if(iNode < 40)
			{
				m_CloakNodeData.NodeArray[iNode].eModelNode = eCurrentNode;
				m_CloakNodeData.NodeArray[iNode].fHeight = vPos.y;
			}
		}
	}

	//now sort the array by height
	std::sort(	m_CloakNodeData.NodeArray, 
				m_CloakNodeData.NodeArray+cNodes, 
				tagCloakNodeList::CloakCompareFunction() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateCloakAuraFX
//
//	PURPOSE:	Update the aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateCloakAuraFX()
{
	if(m_hCloakAuraSystem)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
		uint32 dwUsrFlags;
		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);
		LTFLOAT fTime	= g_pLTClient->GetTime();

		if(!(dwFlags & FLAG_VISIBLE) || !(dwUsrFlags & USRFLG_CHAR_CLOAKED) )
		{
			//hide the effect
			dwFlags = g_pLTClient->GetObjectFlags(m_hCloakAuraSystem);
			g_pLTClient->SetObjectFlags(m_hCloakAuraSystem, dwFlags & ~FLAG_VISIBLE);
			m_CloakNodeData.fEffectStartTime = 0.0f;
			return;
		}
		else
		{
			//see about re-setting
			if(m_CloakNodeData.fEffectStartTime == 0.0f)
			{
				m_CloakNodeData.fEffectStartTime = fTime;
				m_CloakNodeData.fStartTime = 0.0f;
				m_CloakNodeData.nFirstNode = 0;
				dwFlags = g_pLTClient->GetObjectFlags(m_hCloakAuraSystem);
				g_pLTClient->SetObjectFlags(m_hCloakAuraSystem, dwFlags | FLAG_VISIBLE);

				//build and sort our list of nodes
				BuildCloakNodeList();
			}
			else
			{
				if(fTime - m_CloakNodeData.fEffectStartTime > g_vtCloakMaxTime.GetFloat() )
				{
					//time to go away
					dwFlags = g_pLTClient->GetObjectFlags(m_hCloakAuraSystem);
					g_pLTClient->SetObjectFlags(m_hCloakAuraSystem, dwFlags & ~FLAG_VISIBLE);
					return;
				}
			}
		}

		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pLTClient->SetObjectPos(m_hCloakAuraSystem, &vPos);

		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		g_pLTClient->SetObjectRotation(m_hCloakAuraSystem, &rRot);
		//now update the particles
		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		LTParticle *pHead;
		LTParticle *pTail;
		g_pLTClient->GetParticles(m_hCloakAuraSystem, &pHead, &pTail);

		//see if it is time to update (this is done to seperate the look of the effect from frame rate)
		if(fTime - m_CloakNodeData.fStartTime > g_vtCloakUpdateTime.GetFloat())
		{
			//yep time to update
			if(m_CloakNodeData.fStartTime == 0.0f)
				m_CloakNodeData.nFirstNode = 0;
			else
				m_CloakNodeData.nFirstNode++;

			//time stamp
			m_CloakNodeData.fStartTime = fTime;
		}

		for (	uint32 i = m_CloakNodeData.nFirstNode ; 
				i < m_CloakNodeData.nFirstNode+MAX_CLOAK_EFFECTS ; 
				i++ )
		{
			if(i < (uint32)cNodes)
			{
				szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, m_CloakNodeData.NodeArray[i].eModelNode);
			
				if (szNodeName)
				{
						GetNodePos(szNodeName, vPos);
						g_pLTClient->SetParticlePos(m_hCloakAuraSystem, pHead, &vPos);

						LTFLOAT fRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, m_CloakNodeData.NodeArray[i].eModelNode);
						pHead->m_Size = fRadius * GetRandom(0.5f, 0.8f);
				}
			}
			else
				pHead->m_Size = 0;

			pHead = pHead->m_pNext;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateAuraFX
//
//	PURPOSE:	Update the aura FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateAuraFX()
{
	if(m_hAuraSystem)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);

		//if this character is not visible or we are not an alien, hide the system
		if(	(	dwFlags & FLAG_VISIBLE) && 
				IsAlien(g_pGameClientShell->GetPlayerStats()->GetButeSet()) && 
				!(VisionMode::g_pButeMgr->IsAlienHuntingVision(g_pGameClientShell->GetVisionModeMgr()->GetCurrentModeName()))
				&& g_pGameClientShell->IsFirstPerson())
		{
			dwFlags = g_pLTClient->GetObjectFlags(m_hAuraSystem);
			g_pLTClient->SetObjectFlags(m_hAuraSystem, dwFlags | FLAG_VISIBLE);
		}
		else
		{
			dwFlags = g_pLTClient->GetObjectFlags(m_hAuraSystem);
			g_pLTClient->SetObjectFlags(m_hAuraSystem, dwFlags & ~FLAG_VISIBLE);
			return;
		}


		LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		g_pLTClient->SetObjectRotation(m_hAuraSystem, &rRot);

		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		// get the local camera info
		LTVector vCamPos;
		HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
		g_pLTClient->GetObjectPos(hCamera, &vCamPos);

		LTVector vTemp = vPos-vCamPos;
		vTemp.Norm();
		vPos += vTemp * g_vtPushBack.GetFloat();

		g_pLTClient->SetObjectPos(m_hAuraSystem, &vPos);

		//now update the particles
		ModelSkeleton eModelSkeleton = GetModelSkeleton();

		int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
		const char* szNodeName = DNULL;

		LTParticle *pHead;
		LTParticle *pTail;
		g_pLTClient->GetParticles(m_hAuraSystem, &pHead, &pTail);

		LTBOOL bHidden=LTTRUE;

		for (int iNode = 0; iNode < cNodes; iNode++)
		{
			ModelNode eCurrentNode = (ModelNode)iNode;
			szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
			
			const char* szPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkeleton, eCurrentNode);

			if(szPiece[0] != '\0')
			{
				HMODELPIECE hPiece = INVALID_MODEL_PIECE;

				// Make the piece invisible
				if(g_pModelLT->GetPiece(m_hServerObject, const_cast<char*>(szPiece), hPiece) == LT_OK)
				{
					g_pModelLT->GetPieceHideStatus(m_hServerObject, hPiece, bHidden);
				}
			}

			if (szNodeName)
			{
				LTVector vPos;
				GetNodePos(szNodeName, vPos);

				g_pLTClient->SetParticlePos(m_hAuraSystem, pHead, &vPos);

				pHead->m_Color = GetAuraColor(g_pInterfaceMgr->GetAutoTargetMgr()->GetAlienTarget() == m_hServerObject);

			}

			if(bHidden)
				pHead->m_Size = 0.0f;

			bHidden = LTTRUE;

			pHead = pHead->m_pNext;
		}
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateTargetingSprites
//
//	PURPOSE:	Create the Predator targeting sprites
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateTargetingSprites()
{
	for(int i=0; i<3; i++)
	{
		if(!m_hTargetingSprites[i])
		{
			// Our object du'jour
			ObjectCreateStruct	objectStruct;

			// Clear out the object....
			objectStruct.Clear();
			objectStruct.m_ObjectType = OT_SPRITE;
			objectStruct.m_Flags =  FLAG_NOLIGHT | FLAG_FOGDISABLE | FLAG_ROTATEABLESPRITE | FLAG_VISIBLE;

			SAFE_STRCPY(objectStruct.m_Filename, "sprites\\lasercursor.spr");

			objectStruct.m_Pos.Init();
			objectStruct.m_Scale = LTVector(0.02f, 0.02f, 0.02f);

			m_hTargetingSprites[i]	= g_pLTClient->CreateObject(&objectStruct);
		}
	}

	// call the update to get the initial position set up.
	UpdateTargetingSprite();

	// get player object and test for local player
	HLOCALOBJ	hPlayerObj = g_pClientDE->GetClientObject();
	LTBOOL		bIsLocalPlayer = hPlayerObj == m_hServerObject;

	//now create the lense flare (don't do it for local player)
	if(!m_hFlare && !bIsLocalPlayer)
	{
//		m_LFcs.hstrSpriteFile		= g_pLTClient->CreateString("Sprites\\sfx\\flares\\predflare01.spr");
		m_LFcs.bInSkyBox			= LTFALSE;
		m_LFcs.bCreateSprite		= LTTRUE;
		m_LFcs.bSpriteOnly		= LTTRUE;
		m_LFcs.bUseObjectAngle	= LTFALSE;
		m_LFcs.bSpriteAdditive	= LTTRUE;
		m_LFcs.fSpriteOffset		= 0.0f;
		m_LFcs.fMinAngle			= 30.0f;
		m_LFcs.fMinSpriteAlpha	= 0.0f;
		m_LFcs.fMaxSpriteAlpha	= 1.0f;
		m_LFcs.fMinSpriteScale	= 0.0f;
		m_LFcs.fMaxSpriteScale	= 0.175f;
		m_LFcs.hServerObj = m_hServerObject;

		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		// Create the lens flare sprite...

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, createStruct.m_Pos);

		createStruct.m_ObjectType = OT_SPRITE;

		char* pFilename = "Sprites\\sfx\\flares\\predflare01.spr";

		SAFE_STRCPY(createStruct.m_Filename, pFilename);
		createStruct.m_Flags = FLAG_NOLIGHT | FLAG_SPRITEBIAS;

		if (m_LFcs.bSpriteAdditive)
		{
			createStruct.m_Flags2 = FLAG2_ADDITIVE;  
		}

		m_hFlare = g_pLTClient->CreateObject(&createStruct);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateTargetingSprite
//
//	PURPOSE:	Update the targeting sprites
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateTargetingSprite()
{
	LTVector vPos, vCameraPos;	// camera position
	LTVector vF, vR, vU;		// direction vectors
	LTVector vCamF;				// direction vectors
	LTRotation rRot;			// camera/socket rotation

	// get player object and test for local player
	HLOCALOBJ	hPlayerObj = g_pClientDE->GetClientObject();
	LTBOOL		bIsLocalPlayer = hPlayerObj == m_hServerObject;

	// get the local camera info
	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// save off camera pos and forward vector
	vCameraPos = vPos;
	vCamF = vF;

	// if we are a local player see about auto-targeting
	if (bIsLocalPlayer)
	{
		if(g_pInterfaceMgr->GetPlayerStats()->GetAutoTarget())
		{
			//hide the image if we are auto-targeting
			HideTargetingSprite();
			return;
		}
	}
	else
	{
		// get the player or AI's info
		HMODELSOCKET hSocket;
		if (g_pModelLT->GetSocket(m_hServerObject, "HelmLight", hSocket) == LT_OK)
		{
			LTransform transform;
			if (g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, DTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRot);
				g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
			}
		}	
	}

	LTVector vRay[3];
	SpriteData myData[3];

	// set the start points for the three ray intersects
	myData[0].vStart = vPos + vU*1.5f;			// top dot
	myData[1].vStart = vPos - vU*1.5f + vR*1.5f;// bottom right dot
	myData[2].vStart = vPos - vU*1.5f - vR*1.5f;// bottom left dot

	// now figure out where we need to be and at what angle.
	IntersectQuery	IQuery;
	IntersectInfo	IInfo[3];


	// Don't hit yourself!
	PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();
	HOBJECT hFilterList[] = {g_pInterface->GetClientObject(), pMovement->GetObject(), LTNULL};

	IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
	IQuery.m_FilterFn = CFXObjListFilterFn;
	IQuery.m_PolyFilterFn = CFXPolyFilterFn;

	if (bIsLocalPlayer)
	{
		IQuery.m_pUserData = hFilterList;
	}
	else
	{
		IQuery.m_pUserData = LTNULL;;
	}

	// set up the query
	for(int i=0; i<3; i++)
	{
		IQuery.m_From	= myData[i].vStart;
		IQuery.m_To		= myData[i].vStart + (vF * 5000);

		if(g_pLTClient->IntersectSegment(&IQuery, &IInfo[i]))
		{
			//if the shooter is not the local player but the target is
			//hide the sprites
			if(!bIsLocalPlayer)
			{

				if (pMovement)
				{
					if(IInfo[i].m_hObject == pMovement->GetObject() || IInfo[i].m_hObject == hPlayerObj)
					{
						myData[i].bResolved	= LTTRUE;
						myData[i].bHide		= LTTRUE;
					}
				}
			}

			//if we are not resolved then see if se have a simple
			//solution
			if(!myData[i].bResolved)
			{
				uint32 nType = g_pLTClient->GetObjectType(IInfo[i].m_hObject);

				if(nType != OT_MODEL)
				{
					//good enuf to show
					myData[i].bResolved = LTTRUE;
					myData[i].vNorm		= IInfo[i].m_Plane.m_Normal;
					myData[i].vPos		= IInfo[i].m_Point;
				}
			}
		}
		else
		{
			//no hit so just resolve and hide the sprite
			myData[i].bResolved	= LTTRUE;
			myData[i].bHide		= LTTRUE;
		}
	}


	// no then, see if we can group resolve
	if(!myData[0].bResolved && !myData[1].bResolved && !myData[2].bResolved &&
		IInfo[0].m_hObject == IInfo[1].m_hObject && IInfo[1].m_hObject == IInfo[2].m_hObject)
	{
		// all is good we can do an optimized intersection
		OptimizedResolve(myData, IInfo, vF, vCameraPos);
	}

	//now loop thru and see about individual resolves
	for(i=0; i<3; i++)
	{
		if(!myData[i].bResolved)
		{
			 SingleResolve(&myData[i], vF, vCameraPos);
		}
	}

	//ok now that we are done we can set the position or hide the sprite
	SetSprites(myData, vCamF);
}

void CCharacterFX::OptimizedResolve(SpriteData* pData, IntersectInfo* pInfo, LTVector& vF, LTVector& vCamPos)
{
	ILTModel::LTRayResult Results[3];

	for(int i=0; i<3 ; i++)
	{
		Results[i].m_vOrigin = pData[i].vStart;
		Results[i].m_vDir = vF;
		Results[i].m_fMinDist = 0;
		Results[i].m_fMaxDist = 5000;
	}

	LTVector	vFinalNormal[3];
	LTFLOAT		fFinalDist[3];
	LTBOOL		bDone[3] = {LTFALSE,LTFALSE,LTFALSE};

	HOBJECT hAttachList[30];
	DDWORD dwListSize, dwNumAttachments;


	// loop thru all the attachments
	if (g_pLTClient->Common()->GetAttachments(pInfo[0].m_hObject, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		for (uint32 j=0; j < dwListSize; j++)
		{
			if (hAttachList[j])
			{
				g_pModelLT->IntersectRays(	hAttachList[j], 
											vCamPos, 0, 
											LTNULL, 0, Results, 3);

				for(i=0; i<3; i++)
				{
					if(Results[i].m_bIntersect)
					{
						//update the max distance and record the results
						Results[i].m_fMaxDist = fFinalDist[i] = Results[i].m_fDistance;
						vFinalNormal[i] = Results[i].m_vNormal;
						bDone[i] = LTTRUE;
					}
				}
			}
		}
	}

	// now check the base model
	g_pModelLT->IntersectRays(	pInfo[0].m_hObject, 
								vCamPos, 0, 
								LTNULL,	0, Results, 3);
	for(i=0; i<3; i++)
	{
		if(Results[i].m_bIntersect)
		{
			//update the max distance and record the results
			Results[i].m_fMaxDist = fFinalDist[i] = Results[i].m_fDistance;
			vFinalNormal[i] = Results[i].m_vNormal;
			bDone[i] = LTTRUE;
		}
	}

	for(i=0; i<3; i++)
	{
		if(bDone[i])
		{
			pData[i].bResolved	= LTTRUE;
			pData[i].vNorm		= vFinalNormal[i];
			pData[i].vPos		= pData[i].vStart + vF *fFinalDist[i];
		}
		else
		{
			//reset the start point and try again
			pData[i].vStart = pInfo[i].m_Point;
		}
	}
}

void CCharacterFX::SingleResolve(SpriteData* pData, LTVector& vF, LTVector& vCamPos)
{
	// get player object and test for local player
	HLOCALOBJ	hPlayerObj = g_pClientDE->GetClientObject();
	LTBOOL		bIsLocalPlayer = hPlayerObj == m_hServerObject;

	LTBOOL	bDone = LTFALSE;
	uint8	nCounter = 0;

	// Don't hit yourself!
	PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();
	HOBJECT hFilterList[] = {g_pInterface->GetClientObject(), pMovement->GetObject(), LTNULL};

	IntersectQuery	IQuery;
	IntersectInfo	IInfo;
	IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
	IQuery.m_FilterFn = CFXObjListFilterFn;
	IQuery.m_PolyFilterFn = CFXPolyFilterFn;

	if (bIsLocalPlayer)
	{
		IQuery.m_pUserData = hFilterList;
	}
	else
	{
		IQuery.m_pUserData = LTNULL;;
	}

	do
	{
		IQuery.m_From	= pData->vStart;
		IQuery.m_To		= pData->vStart + (vF * 5000);


		//test for intersect-segment
		if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
		{
			//if the shooter is not the local player but the target is
			//hide the sprites
			if(!bIsLocalPlayer)
			{
				PlayerMovement *pMovement = g_pGameClientShell->GetPlayerMovement();

				if (pMovement)
				{
					if(IInfo.m_hObject == pMovement->GetObject() || IInfo.m_hObject == hPlayerObj)
					{
						pData->bResolved	= LTTRUE;
						pData->bHide		= LTTRUE;
						return;
					}
				}
			}

			uint32 nType = g_pLTClient->GetObjectType(IInfo.m_hObject);

			if(nType != OT_MODEL)
			{
				//good enuf to show
				pData->bResolved	= LTTRUE;
				pData->vNorm		= IInfo.m_Plane.m_Normal;
				pData->vPos			= IInfo.m_Point;
				return;
			}
			else
			{
				// get our normal and point info from the model here!
				ILTModel::LTRayResult Results;

				Results.m_vOrigin = pData->vStart;
				Results.m_vDir = vF;
				Results.m_fMinDist = 0;
				Results.m_fMaxDist = 5000;

				LTVector	vFinalNormal;
				LTFLOAT		fFinalDist;

				HOBJECT hAttachList[30];
				DDWORD dwListSize, dwNumAttachments;

				if (g_pLTClient->Common()->GetAttachments(IInfo.m_hObject, hAttachList, 
					ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
				{
					for (uint32 j=0; j < dwListSize; j++)
					{
						if (hAttachList[j])
						{
							g_pModelLT->IntersectRays(	hAttachList[j], 
														vCamPos, 0, 
														LTNULL, 0, &Results, 1);

							if(Results.m_bIntersect)
							{
								//update the max distance and record the results
								Results.m_fMaxDist = fFinalDist = Results.m_fDistance;
								vFinalNormal = Results.m_vNormal;
								bDone = LTTRUE;
							}
						}
					}
				}

				// if we didn't get a hit on the attachments then try the model
				g_pModelLT->IntersectRays(	IInfo.m_hObject, 
											vCamPos, 0, 
											LTNULL,	0, &Results, 1);
				if(Results.m_bIntersect)
				{
					//update the max distance and record the results
					Results.m_fMaxDist = fFinalDist = Results.m_fDistance;
					vFinalNormal = Results.m_vNormal;
					bDone = LTTRUE;
				}

				if(bDone)
				{
					pData->bResolved	= LTTRUE;
					pData->vNorm		= vFinalNormal;
					pData->vPos			= pData->vStart + vF *fFinalDist;
				}
				else
				{
					//reset the start point and try again
					pData->vStart = IInfo.m_Point;
					if(++nCounter >= 3)
					{
						//this limits the number of models we look at to 3
						pData->bResolved	= LTTRUE;
						pData->bHide		= LTTRUE;
						return;
					}
				}
			}
		}
		else
		{
			//no hit so just resolve and hide the sprite
			pData->bResolved	= LTTRUE;
			pData->bHide		= LTTRUE;
			return;
		}
	}while (!bDone);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::SetSprites
//
//	PURPOSE:	Set the targeting sprites
//
// ----------------------------------------------------------------------- //

void CCharacterFX::SetSprites(SpriteData* pData, LTVector& vCamF)
{
	//loop thru the data and set the visibility
	for(int i=0; i<3; i++)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hTargetingSprites[i]);
		pData[i].bHide?dwFlags &= ~FLAG_VISIBLE:dwFlags |= FLAG_VISIBLE;
		g_pLTClient->SetObjectFlags(m_hTargetingSprites[i], dwFlags);
	}

	//now set the pos and rot
	LTVector vTemp;
	VEC_MULSCALAR(vTemp, vCamF, -2.0f);

	for(i=0; i<3; i++)
	{
		if(!pData[i].bHide)
		{
			LTRotation rNewRot;
			g_pInterface->AlignRotation(&rNewRot, &pData[i].vNorm, LTNULL);
			g_pLTClient->SetObjectRotation(m_hTargetingSprites[i], &rNewRot);

			VEC_ADD(pData[i].vPos, pData[i].vPos, vTemp);  // Off the wall/floor/object a bit
			g_pLTClient->SetObjectPos(m_hTargetingSprites[i], &pData[i].vPos);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HideTargetingSprite
//
//	PURPOSE:	Hide the targeting sprites
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HideTargetingSprite()
{
	for(int i=0; i<3; i++)
	{
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hTargetingSprites[i]);
		dwFlags &= ~FLAG_VISIBLE;
		g_pLTClient->SetObjectFlags(m_hTargetingSprites[i], dwFlags);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateFlare()
//
//	PURPOSE:	Update the lens flare FX...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateFlare()
{
	DDWORD dwFlags = 0;
	
	// Only show the flare if the camera is looking at it...

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	if (!hCamera) return;

	LTVector vCamPos;
	g_pLTClient->GetObjectPos(hCamera, &vCamPos);

	//get the socket position here and set the new flare pos
	LTRotation rFlareRot;
	LTVector vFlarePos;

	HMODELSOCKET hSocket;
	ModelLT* pModelLT = g_pLTClient->GetModelLT();
	if (pModelLT->GetSocket(m_hServerObject, "HelmLight", hSocket) == LT_OK)
	{
		LTransform transform;
		if (pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, LTTRUE) == LT_OK)
		{
			TransformLT* pTransLT = g_pClientDE->GetTransformLT();
			pTransLT->GetPos(transform, vFlarePos);
			pTransLT->GetRot(transform, rFlareRot);
		}
	}
	//set the position of the flare
	g_pLTClient->SetObjectPos(m_hFlare, &vFlarePos);

	LTVector vFlareU, vFlareR, vFlareF;
	g_pLTClient->GetRotationVectors(&rFlareRot, &vFlareU, &vFlareR, &vFlareF);

	LTRotation rCamRot;
	LTVector vCamU, vCamR, vCamF;
	g_pLTClient->GetObjectRotation(hCamera, &rCamRot);
	g_pLTClient->GetRotationVectors(&rCamRot, &vCamU, &vCamR, &vCamF);

	LTVector vDir = vFlarePos - vCamPos;
	vDir.Norm();

	DFLOAT fCameraAngle = VEC_DOT(vDir, vCamF);
	fCameraAngle = fCameraAngle < 0.0f ? 0.0f : fCameraAngle;
	fCameraAngle = fCameraAngle > 1.0f ? 1.0f : fCameraAngle;
	fCameraAngle = MATH_RADIANS_TO_DEGREES((LTFLOAT)acos(fCameraAngle));

	DFLOAT fObjectAngle = VEC_DOT(-vDir, vFlareF);
	fObjectAngle = fObjectAngle < 0.0f ? 0.0f : fObjectAngle;
	fObjectAngle = fObjectAngle > 1.0f ? 1.0f : fObjectAngle;
	fObjectAngle = MATH_RADIANS_TO_DEGREES((LTFLOAT)acos(fObjectAngle));

	DFLOAT fMinAngle = m_LFcs.fMinAngle;

	if (fObjectAngle + fCameraAngle > fMinAngle*2)
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
		g_pLTClient->SetObjectFlags(m_hFlare, dwFlags & ~FLAG_VISIBLE);
	}
	else
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
		g_pLTClient->SetObjectFlags(m_hFlare, dwFlags | FLAG_VISIBLE);

		DFLOAT fVal = 1-(fObjectAngle + fCameraAngle)/(fMinAngle*2); 
	
		// Calculate new alpha...

		DFLOAT fMinAlpha = m_LFcs.fMinSpriteAlpha;
		DFLOAT fMaxAlpha = m_LFcs.fMaxSpriteAlpha;
		DFLOAT fAlphaRange = fMaxAlpha - fMinAlpha;

		DFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(m_hFlare, &r, &g, &b, &a);

		a = fMinAlpha + (fVal * fAlphaRange);
		g_pLTClient->SetObjectColor(m_hFlare, r, g, b, a);

		// Calculate new scale...

		DFLOAT fMinScale = m_LFcs.fMinSpriteScale;
		DFLOAT fMaxScale = m_LFcs.fMaxSpriteScale;
		DFLOAT fScaleRange = fMaxScale - fMinScale;

		DFLOAT fScale = fMinScale + (fVal * fScaleRange);
		DVector vScale(fScale, fScale, fScale);
		g_pLTClient->SetObjectScale(m_hFlare, &vScale);

		// Don't do any more processing if the alpha is 0...

		if (a < 0.001f) return;

		
		// See if we should make a "bliding" flare, and if so see if the
		// camera is looking directly at the flare...
		DDWORD dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
		dwFlags &= ~FLAG_SPRITE_NOZ;
		dwFlags |= FLAG_SPRITEBIAS;
		g_pLTClient->SetObjectFlags(m_hFlare, dwFlags);

		if (m_LFcs.bBlindingFlare)
		{
			DFLOAT fBlindObjAngle = m_LFcs.fBlindObjectAngle;
			DFLOAT fBlindCamAngle = m_LFcs.fBlindCameraAngle;
		
			if ((fObjectAngle > (90.0f - fBlindObjAngle)) &&
				(fCameraAngle > (90.0f - fBlindCamAngle)))
			{
				// Update the no-z flare if possible...
				DDWORD dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);

				// Make sure there is a clear path from the flare to the camera...
					
				HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
				HOBJECT hFilterList[] = {hPlayerObj, g_pGameClientShell->GetPlayerMovement()->GetObject(), DNULL};

				IntersectInfo iInfo;
				IntersectQuery qInfo;
				qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
				qInfo.m_FilterFn	= ObjListFilterFn;
				qInfo.m_pUserData	= g_pGameClientShell->IsFirstPerson() ? hFilterList : DNULL;
				qInfo.m_From		= vFlarePos;
				qInfo.m_To			= vCamPos;

				if (g_pInterface->IntersectSegment(&qInfo, &iInfo))
				{
					// Doesn't look quite as good, but will clip on world/objects...

					dwFlags &= ~FLAG_SPRITE_NOZ;
					dwFlags |= FLAG_SPRITEBIAS;
				}
				else
				{
					// Calculate new flare scale...
		
					fVal = (fObjectAngle + fBlindObjAngle - 90.0f)/fBlindObjAngle; 

					DFLOAT fMaxBlindScale = m_LFcs.fMaxBlindScale;
					DFLOAT fMinBlindScale = m_LFcs.fMinBlindScale;

					fScaleRange =  fMaxBlindScale - fMinBlindScale;
					fScale = fMinBlindScale + (fVal * fScaleRange);
					vScale.Init(fScale, fScale, fScale);
					g_pLTClient->SetObjectScale(m_hFlare, &vScale);


					// This looks better, but will show through world/objects...

					dwFlags &= ~FLAG_SPRITEBIAS;
					dwFlags |= FLAG_SPRITE_NOZ;
				}
				
				g_pLTClient->SetObjectFlags(m_hFlare, dwFlags);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CheckForBleederRemoval()
//
//	PURPOSE:	See if we need to remove any of the bleeder FX...
//
// ----------------------------------------------------------------------- //

bool RemoveBleeder(BleederData* bleeder)
{

	ModelNode	eParent	= g_pModelButeMgr->GetSkeletonNodeParent(bleeder->pFX->m_cs.eModelSkeleton, bleeder->eModelNode);
	const char* szPiece	= g_pModelButeMgr->GetSkeletonNodePiece(bleeder->pFX->m_cs.eModelSkeleton, eParent);
	HMODELPIECE hPiece	= INVALID_MODEL_PIECE;
	LTBOOL		bHidden	= LTFALSE;

	if(szPiece && g_pModelLT->GetPiece(bleeder->pFX->GetServerObj(),const_cast<char*>(szPiece), hPiece) == LT_OK) 
		g_pModelLT->GetPieceHideStatus(bleeder->pFX->GetServerObj(), hPiece, bHidden);

	// If the parent piece is hidden then we need to go away...
	if(bHidden)
	{
		g_pLTClient->RemoveObject(bleeder->hBleeder);
		delete bleeder;
		bleeder = LTNULL;
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CheckForBleederRemoval()
//
//	PURPOSE:	See if we need to remove any of the bleeder FX...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CheckForBleederRemoval()
{
	BleederList::iterator start_remove = std::remove_if(m_BleederList.begin(),m_BleederList.end(),RemoveBleeder);
	m_BleederList.erase(start_remove, m_BleederList.end());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::AddNewBleeder()
//
//	PURPOSE:	Add a new bleeder..
//
// ----------------------------------------------------------------------- //

void CCharacterFX::AddNewBleeder(ModelNode eNode)
{
	if(GetConsoleInt("LowViolence",0))
		return;

	// Allocate a new object
	BleederData* pData = new BleederData;

	if(!pData) return;

	// Load up some data
	pData->eModelNode = eNode;
	pData->pFX = this;

	// Get our position and initialize the rotation
	LTVector vPos;
	LTRotation rRot;
	const char* szNodeName;
	szNodeName = g_pModelButeMgr->GetSkeletonNodeName(m_cs.eModelSkeleton, eNode);
	GetNodePos( szNodeName, vPos, LTTRUE);
	rRot.Init();

	// Now create the system
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_PARTICLESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN | FLAG_FOGDISABLE ;
	createStruct.m_Pos = pData->vLastPos = vPos;
	createStruct.m_Rotation = rRot;

	pData->hBleeder = m_pClientDE->CreateObject(&createStruct);

	if(!pData->hBleeder)
	{
		// Somthing went terribly wrong...
		delete pData;
		return;
	}

	// Set up the new system
	PARTICLETRAILFX *pTrail = g_pFXButeMgr->GetParticleTrailFX("Alien_Blood_Stump_Spray");

    uint32 dwWidth, dwHeight;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	g_pLTClient->GetSurfaceDims (hScreen, &dwWidth, &dwHeight);

	LTFLOAT fRad = pTrail->fRadius;
    fRad /= ((LTFLOAT)dwWidth);

	g_pLTClient->SetupParticleSystem(pData->hBleeder, pTrail->szTexture,
									 pTrail->fGravity, 0, fRad);

	// Set blend modes if applicable...
    uint32 dwFlags2;
    g_pLTClient->Common()->GetObjectFlags(pData->hBleeder, OFT_Flags2, dwFlags2);

	if (pTrail->bAdditive)
	{
		dwFlags2 |= FLAG2_ADDITIVE;
	}
	else if (pTrail->bMultiply)
	{
		dwFlags2 |= FLAG2_MULTIPLY;
	}
    g_pLTClient->Common()->SetObjectFlags(pData->hBleeder, OFT_Flags2, dwFlags2);

	// Now add some particls...
	AddBleederParticles(pData);

	// Add the emmitter to the list
	m_BleederList.push_back(pData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::AddBleederParticles()
//
//	PURPOSE:	Add new bleeder particles
//
// ----------------------------------------------------------------------- //

void CCharacterFX::AddBleederParticles(BleederData* pData)
{
	if(GetConsoleInt("LowViolence",0))
		return;

	// Now create the particle FX
	PARTICLETRAILFX *pTrail = g_pFXButeMgr->GetParticleTrailFX("Alien_Blood_Stump_Spray");

	// Mark the time
	pData->fLastDripTime = g_pLTClient->GetTime();

	// Null vector
	LTVector vNull(0,0,0);

	g_pLTClient->AddParticles(pData->hBleeder, pTrail->nEmitAmount,
							  &pTrail->vMinDrift, &pTrail->vMaxDrift,	// Position offset
							  &vNull, &vNull,	// Velocity
							  &pTrail->vMinColor, &pTrail->vMaxColor,	// Color
							  pTrail->fLifetime + pTrail->fFadetime, pTrail->fLifetime + pTrail->fFadetime);

	// Go through and set sizes
	LTParticle *pHead = LTNULL, *pTail = LTNULL;
	
	if(g_pLTClient->GetParticles(pData->hBleeder, &pHead, &pTail))
	{
		while(pHead && pHead != pTail)
		{
			pHead->m_Size = pTrail->fStartScale;
			pHead->m_Alpha = 1.0f;
			pHead = pHead->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::AddBleederPool()
//
//	PURPOSE:	Add a pool of blood!
//
// ----------------------------------------------------------------------- //

void CCharacterFX::AddBleederPool(BleederData* pData)
{
	if(GetConsoleInt("LowViolence",0))
		return;

	// Get the position and make an intersect
	LTVector vPos;
	g_pLTClient->GetObjectPos(pData->hBleeder, &vPos);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_From = vPos;
	iQuery.m_To = vPos + LTVector(0,-500,0);

	if(g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		// Create a blood splat...
		BSCREATESTRUCT sc;
		
		// Randomly rotate the blood splat
		g_pLTClient->AlignRotation(&(sc.rRot), &(iInfo.m_Plane.m_Normal), LTNULL);
		g_pLTClient->RotateAroundAxis(&(sc.rRot), &(iInfo.m_Plane.m_Normal), GetRandom(0.0f, MATH_CIRCLE));
		
		sc.vPos				= iInfo.m_Point + iInfo.m_Plane.m_Normal * GetRandom(1.0f, 1.9f);  // Off the wall a bit
		sc.vVel				= LTVector(0.0f, 0.0f, 0.0f);
		sc.vInitialScale	= LTVector(0.15f, 0.15f, 1.0f);
		sc.vFinalScale		= sc.vInitialScale;
		sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
		sc.fLifeTime		= 15.0f;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;
		sc.nType			= OT_SPRITE;
		
		char* pBloodFiles[] =
		{
			"Sprites\\AlnBld1.spr",
			"Sprites\\AlnBld2.spr",
			"Sprites\\AlnBld3.spr",
		};
		
		sc.Filename = pBloodFiles[GetRandom(0,2)];
		
		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if(psfxMgr)
		{
			CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
			if(pFX) 
				pFX->Update();

			if(GetRandom(0.0f, 1.0f) < g_vtPoolSizzleChance.GetFloat())
			{
				// First see if we have a valid impact FX or not
				IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX("Alien_Blood_Pool");

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

	// Now reset the time
	pData->fLastPoolTime = g_pLTClient->GetTime() + GetRandom(0.0f, 0.5f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateBleeders()
//
//	PURPOSE:	Update the bleeders...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateBleeders()
{
	// See if we need to remove a bleeder
	CheckForBleederRemoval();

	for( BleederList::iterator iter = m_BleederList.begin(); iter != m_BleederList.end(); ++iter )
	{
		// First set our position
		LTVector vPos;
		const char* szNodeName;
		szNodeName = g_pModelButeMgr->GetSkeletonNodeName(m_cs.eModelSkeleton, (*iter)->eModelNode);
		GetNodePos( szNodeName, vPos, LTTRUE);

		g_pLTClient->SetObjectPos((*iter)->hBleeder, &vPos);

		// Go through and offset their positions
		LTParticle *pHead = LTNULL, *pTail = LTNULL;
		LTVector vOffset= vPos - (*iter)->vLastPos;

		if(g_pLTClient->GetParticles((*iter)->hBleeder, &pHead, &pTail))
		{
			while(pHead && pHead != pTail)
			{
				//get particle position
				LTVector vPartPos(0,0,0);
				g_pLTClient->GetParticlePos((*iter)->hBleeder, pHead, &vPartPos);

				//adjust for new pos
				vPartPos -= vOffset;

				//re-set pos
				g_pLTClient->SetParticlePos((*iter)->hBleeder, pHead, &vPartPos);
				
				pHead = pHead->m_pNext;
			}
		}

		// Now record the new pos
		(*iter)->vLastPos = vPos;
	
		// Now see if it is time to make more particles..
		if(g_pLTClient->GetTime() - (*iter)->fLastDripTime > g_vtBleederTime.GetFloat())
		{
			AddBleederParticles((*iter));
		}

		// Now see if it is time to make a pool
		if(g_pLTClient->GetTime() - (*iter)->fLastPoolTime > g_vtBleederPoolTime.GetFloat())
		{
			AddBleederPool((*iter));
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateFlameThrowerFX()
//
//	PURPOSE:	Create the flame thrower FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateFlameThrowerFX()
{
	// Load up the create structure
	ObjectCreateStruct ocs;
	ocs.m_Flags			= FLAG_UPDATEUNSEEN;
	ocs.m_ObjectType	= OT_PARTICLESYSTEM;
	ocs.m_Flags2		= FLAG2_ADDITIVE;

	// Create the systems
	for(int i=0; i<MAX_FLAME_SEGMENTS ; i++)
	{
		// Create our object
		m_hFlameEmitters[i] = g_pLTClient->CreateObject(&ocs);

		// Pick a sprite
		if(i<4)
			g_pLTClient->SetupParticleSystem(m_hFlameEmitters[i], "sprites\\sfx\\muzzleFX\\flamenew1.spr", 0, 0, 10000.0f);
		if(i<8)
			g_pLTClient->SetupParticleSystem(m_hFlameEmitters[i], "sprites\\sfx\\muzzleFX\\flamenew2.spr", 0, 0, 10000.0f);
		else
			g_pLTClient->SetupParticleSystem(m_hFlameEmitters[i], "sprites\\sfx\\muzzleFX\\flamenew3.spr", 0, 0, 10000.0f);

		LTVector vDummy(0,0,0);
		LTVector vColor(125.0f, 125.0f, 125.0f);

		uint8 nNumSprites	= 3+MAX_FLAME_SEGMENTS-i;	// With 11 segments this will make 14 sprites up close and 3 at the end.
		LTVector vMaxOffset(3.0f*(i+1),3.0f*(i+1),0);
		LTVector vMinOffset(-3.0f*(i+1), -3.0f*(i+1), 50.0f);

		if(i==0) // Back off the first one please
			vMaxOffset.z = 20;

		g_pLTClient->AddParticles(m_hFlameEmitters[i], nNumSprites,
								  &vMaxOffset, &vMinOffset,	// Position offset
								  &vDummy, &vDummy,	// Velocity
								  &vColor, &vColor,	// Color
								  100000, 100000);	// Life (if the player holds this weapon for 27 hours+ we are in trouble!)

		// Loop through the particles and set the sizes...
		LTParticle *pHead;
		LTParticle *pTail;
		g_pLTClient->GetParticles(m_hFlameEmitters[i], &pHead, &pTail);
		do
		{
			if (pHead)
			{
				pHead->m_Size = 3.0f * (i+1);
				pHead = pHead->m_pNext;
			}
		}while (pHead && (pHead != pTail) );
	}

	// Initialize the IK chain utility
	m_FlameIKChain.Init(MAX_FLAME_SEGMENTS, 50.0f, 0.99f, 0.12f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateFlameThrowerFX()
//
//	PURPOSE:	Update the flame thrower FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateFlameThrowerFX()
{
	// Quick is dead check...
	if(m_bIsDead)
	{
		// Clean up the flame thrower FX
		for(int i=0;i<MAX_FLAME_SEGMENTS;i++)
		{
			if(m_hFlameEmitters[i])
			{
				//remove the system
				g_pLTClient->RemoveObject(m_hFlameEmitters[i]);
				m_hFlameEmitters[i] = LTNULL;
			}
		}
		memset(m_fFlameStartTimes, 0, MAX_FLAME_SEGMENTS * sizeof(LTFLOAT));
		memset(m_fFlameEndTimes, 0, MAX_FLAME_SEGMENTS * sizeof(LTFLOAT));
		return;
	}

	LTVector	vPos(0,0,0);
	LTRotation	rRot;
	LTVector vR, vU, vF;

	// See where our camera or flash socket is and how it is oriented
	if(g_pClientDE->GetClientObject() == m_hServerObject && g_pGameClientShell->IsFirstPerson())
	{
		HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
		g_pLTClient->GetObjectPos(hCamera, &vPos);
		g_pLTClient->GetObjectRotation(hCamera, &rRot);
		g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
		LTVector vMuzzleOffset = g_pGameClientShell->GetWeaponModel()->GetMuzzleOffset();
		vPos += (vR * vMuzzleOffset.x) + (vU * vMuzzleOffset.y) + (vF * vMuzzleOffset.z);
	}
	else
	{
		if( !GetAttachmentSocketTransform(m_hServerObject, "Flash", vPos, rRot) ) 
		{
			// If that failed, try the parent model.
			HMODELSOCKET hSocket;
			if( g_pModelLT->GetSocket(m_hServerObject, "Flash", hSocket) == LT_OK )
			{
				LTransform transform;
				if (g_pModelLT->GetSocketTransform(m_hServerObject, hSocket, transform, DTRUE) == LT_OK)
				{
					g_pTransLT->Get(transform, vPos, rRot);
				}
			}
			else
			{
				return;
			}
		}

		g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
	}

	// Update the IK chain
	m_FlameIKChain.Update(vPos, vF, g_pLTClient->GetFrameTime());

	// Update Emitter Positions
	UpdateEmitterPositions();

	// Now see about displaying the FX
	LTFLOAT fTime = g_pLTClient->GetTime();

	// Update emitter particles and sequential display
	UpdateEmitterFX(fTime);

	// Update the emitter FX based on segment intersects.
	UpdateEmitterImpact(fTime);

	// Update flare up FX
	UpdateEmitterFlareUp(fTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ResetFlameThrowerFX()
//
//	PURPOSE:	Update the flame thrower FX
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ResetFlameThrowerFX()
{
	// saftey check to be sure we have the FX set up...
	if(!m_hFlameEmitters[0])
		CreateFlameThrowerFX();

	// Only reset the start times if the effect is not playing
	LTFLOAT fTime = g_pLTClient->GetTime();

	if(m_bCanResetFlame)
	{

		for(int i=0; i<MAX_FLAME_SEGMENTS ; i++)
		{
			m_fFlameStartTimes[i] = fTime + g_vtFlameDelayTime.GetFloat() * i;
		}
		m_bCanResetFlame = LTFALSE;
	}

	// Always reset the end times
	for(int i=0; i<MAX_FLAME_SEGMENTS ; i++)
	{
		m_fFlameEndTimes[i] = fTime + g_vtFlameDelayTime.GetFloat() * i + g_vtFlameLifeTime.GetFloat();
	}
}

// ----------------------------------------------------------------------- //

LTBOOL SFXObjectFilterFn(HOBJECT hObj, void *pUserData);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::TestForImpact()
//
//	PURPOSE:	See if we hit anything...
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacterFX::TestForImpact(IntersectInfo& IInfo)
{
	//  Loop baby loop!
	uint32 dwFlags;
	IntersectQuery	IQuery;
	IQuery.m_Flags	= INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;
	IQuery.m_FilterFn = SFXObjectFilterFn;
	IQuery.m_PolyFilterFn = CFXPolyFilterFn;
	
	SfxFilterStruct MyData;
	uint32 SfxTypes[2]={SFX_CHARACTER_ID,SFX_BODYPROP_ID};
	MyData.nNumTypes	= 2;
	MyData.pTypes		= SfxTypes;

	IQuery.m_pUserData = &MyData;

	//test for intersect-segment
	
	for(int i=0; i<MAX_FLAME_SEGMENTS ; i++)
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hFlameEmitters[i]);

		if(dwFlags & FLAG_VISIBLE)
		{
			IQuery.m_From	= m_FlameIKChain.GetPoint(i);
			IQuery.m_To		= m_FlameIKChain.GetPoint(i+1);

			if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
			{
				uint32 nFlags;
				g_pLTClient->Common()->GetObjectFlags(IInfo.m_hObject, OFT_Flags, nFlags);

				if((nFlags & FLAG_SOLID) && (nFlags & FLAG_VISIBLE))
				{
					// Hide all the emitters
					for(int j=i; j<MAX_FLAME_SEGMENTS ; j++)
					{
						dwFlags = g_pLTClient->GetObjectFlags(m_hFlameEmitters[j]);
						g_pLTClient->SetObjectFlags(m_hFlameEmitters[j], dwFlags & ~FLAG_VISIBLE);
					}
					return LTTRUE;
				}
			}
		}
	}
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateEmitterPositions()
//
//	PURPOSE:	Update the flame thrower FX: Emitter Positions
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateEmitterPositions()
{
	LTRotation	rRot;
	LTVector	vF;

	for(int i=0; i<MAX_FLAME_SEGMENTS ; i++)
	{
		g_pLTClient->SetObjectPos(m_hFlameEmitters[i], &m_FlameIKChain.GetPoint(i, &vF));
		g_pLTClient->AlignRotation(&rRot, &vF, LTNULL);
		g_pLTClient->SetObjectRotation(m_hFlameEmitters[i], &rRot);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateEmitterFX()
//
//	PURPOSE:	Update the flame thrower FX: Emitter Effects
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateEmitterFX(LTFLOAT fTime)
{
	uint32	dwFlags;

	// See if it is time to reposition the sprites...
	LTBOOL bMoveSprites = (fTime-m_fLastFlameMovementTime > 0.1);
	if(bMoveSprites) m_fLastFlameMovementTime = fTime + GetRandom(-0.03f,0.03f);;

	for(int i=0; i<MAX_FLAME_SEGMENTS ; i++)
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hFlameEmitters[i]);

		if(fTime >= m_fFlameStartTimes[i]  && fTime < m_fFlameEndTimes[i])
		{
			// Make visible
			dwFlags |= FLAG_VISIBLE;

			if(bMoveSprites)
			{
				// Loop through the particles and set the positions...
				LTParticle *pHead;
				LTParticle *pTail;
				g_pLTClient->GetParticles(m_hFlameEmitters[i], &pHead, &pTail);
				LTFLOAT fMinZ = i==0?20.0f:0.0f;
				LTFLOAT fMinXY = -3.0f*(i+1);
				LTFLOAT fMaxXY = 3.0f*(i+1);
				do
				{
					if (pHead)
					{
						g_pLTClient->SetParticlePos(	m_hFlameEmitters[i], pHead, 
														&LTVector(	GetRandom(fMinXY, fMaxXY), 
																	GetRandom(fMinXY, fMaxXY), 
																	GetRandom(fMinZ, 60.0f) )
													);
						pHead = pHead->m_pNext;
					}
				}while (pHead && (pHead != pTail) );
			}
		}
		else
		{
			// Hide it
			dwFlags &= ~FLAG_VISIBLE;

			if(i==0)
				m_bCanResetFlame = LTTRUE;
		}

		g_pLTClient->SetObjectFlags(m_hFlameEmitters[i], dwFlags);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateEmitterImpact()
//
//	PURPOSE:	Update the flame thrower FX: Emitter intersects
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateEmitterImpact(LTFLOAT fTime)
{
	// See about an impact FX
	uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hFlameEmitters[0]);
	if(dwFlags & FLAG_VISIBLE)
	{
		// See about our impact FX
		IntersectInfo	IInfo;
		if(TestForImpact(IInfo))
		{
			// Just go away if we hit the sky...
			if (GetSurfaceType(IInfo) == ST_SKY) return;

			if(fTime-m_fLastFlameImpactTime > 0.2)
			{
				IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX("Flame_Thrower_ImpactFX");

				if(pFX)
				{
					IFXCS cs;

					cs.vDir = -IInfo.m_Plane.m_Normal;
					cs.vSurfNormal = IInfo.m_Plane.m_Normal;
					cs.vPos = IInfo.m_Point;
					cs.bPlaySound = LTFALSE;

					g_pFXButeMgr->CreateImpactFX(pFX, cs);
				}
				m_fLastFlameImpactTime = fTime+GetRandom(-0.1f, 0.1f);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateEmitterFlareUp()
//
//	PURPOSE:	Update the flame thrower FX: Emitter flare ups
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateEmitterFlareUp(LTFLOAT fTime)
{
	// See if it is time to make another FX
	if(fTime-m_fLastFlameFlareUpTime > 0.5)
	{
		// Pick a node
		uint8 nId = GetRandom(2, MAX_FLAME_SEGMENTS-1);

		// See if this one is visible
		uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hFlameEmitters[nId]);
		if(dwFlags & FLAG_VISIBLE)
		{
			// Get some info
			LTVector vF;
			LTVector vPos = m_FlameIKChain.GetPoint(nId, &vF);

			// Pick an FX based on node ID
			IMPACTFX *pFX = LTNULL;
			if(nId < 4)
				pFX = g_pFXButeMgr->GetImpactFX("Flame_Thrower_FlareUpFX0");
			else if(nId < 8)
				pFX = g_pFXButeMgr->GetImpactFX("Flame_Thrower_FlareUpFX1");
			else
				pFX = g_pFXButeMgr->GetImpactFX("Flame_Thrower_FlareUpFX2");

			// Display the FX
			if(pFX)
			{
				IFXCS cs;

				cs.vDir = vF;
				cs.vSurfNormal = vF.Cross(LTVector(1.0f,0,0));
				cs.vPos = vPos;

				g_pFXButeMgr->CreateImpactFX(pFX, cs);
			}

			// Reset the time
			m_fLastFlameFlareUpTime = fTime+GetRandom(-0.1f, 0.1f);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateBossFX()
//
//	PURPOSE:	Create a new particle system
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateBossFX()
{
	// Get our position and initialize the rotation
	LTVector vPos;
	LTRotation rRot;
	rRot.Init();

	if(!GetNodePos( "Back", vPos, LTTRUE))
		return;

	// Now create the system
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_PARTICLESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN | FLAG_FOGDISABLE ;
	createStruct.m_Pos = m_vLastBossSmokePos = vPos;
	createStruct.m_Rotation = rRot;

	m_hBossSmokeSystem = m_pClientDE->CreateObject(&createStruct);

	if(!m_hBossSmokeSystem)
	{
		// Somthing went terribly wrong...
		return;
	}

	// Set up the new system
	PARTICLETRAILFX *pTrail = g_pFXButeMgr->GetParticleTrailFX("Dunya_SmokeTrail");

    uint32 dwWidth, dwHeight;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	g_pLTClient->GetSurfaceDims (hScreen, &dwWidth, &dwHeight);

	LTFLOAT fRad = pTrail->fRadius;
    fRad /= ((LTFLOAT)dwWidth);

	g_pLTClient->SetupParticleSystem(m_hBossSmokeSystem, pTrail->szTexture,
									 pTrail->fGravity, 0, fRad);

	// Set blend modes if applicable...
    uint32 dwFlags2;
    g_pLTClient->Common()->GetObjectFlags(m_hBossSmokeSystem, OFT_Flags2, dwFlags2);

	if (pTrail->bAdditive)
	{
		dwFlags2 |= FLAG2_ADDITIVE;
	}
	else if (pTrail->bMultiply)
	{
		dwFlags2 |= FLAG2_MULTIPLY;
	}
    g_pLTClient->Common()->SetObjectFlags(m_hBossSmokeSystem, OFT_Flags2, dwFlags2);

	// Now add some particls...
	AddBossSmokeParticles();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::AddBossSmokeParticles()
//
//	PURPOSE:	Add new smoke particles
//
// ----------------------------------------------------------------------- //

void CCharacterFX::AddBossSmokeParticles()
{
	// Now create the particle FX
	PARTICLETRAILFX *pTrail = g_pFXButeMgr->GetParticleTrailFX("Dunya_SmokeTrail");

	// Mark the time
	m_fLastBossSmokeTime = g_pLTClient->GetTime() + GetRandom(-0.02f, 0.02f);

	// Null vector
	LTVector vNull(0,0,0);

	g_pLTClient->AddParticles(m_hBossSmokeSystem, pTrail->nEmitAmount,
							  &pTrail->vMinDrift, &pTrail->vMaxDrift,	// Position offset
							  &vNull, &vNull,	// Velocity
							  &pTrail->vMinColor, &pTrail->vMaxColor,	// Color
							  pTrail->fLifetime + pTrail->fFadetime, pTrail->fLifetime + pTrail->fFadetime);

	// Go through and set sizes
	LTParticle *pHead = LTNULL, *pTail = LTNULL;
	
	if(g_pLTClient->GetParticles(m_hBossSmokeSystem, &pHead, &pTail))
	{
		while(pHead && pHead != pTail)
		{
			pHead->m_Size = pTrail->fStartScale + m_nBossSmokeLevel;
			pHead = pHead->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateBossFX()
//
//	PURPOSE:	Update the boss smoke FX...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateBossFX()
{
	// First set our position
	LTVector vPos;
	GetNodePos( "Back", vPos, LTTRUE);

	g_pLTClient->SetObjectPos(m_hBossSmokeSystem, &vPos);

	// Go through and offset their positions
	LTParticle *pHead = LTNULL, *pTail = LTNULL;
	LTVector vOffset= vPos - m_vLastBossSmokePos;

	if(g_pLTClient->GetParticles(m_hBossSmokeSystem, &pHead, &pTail))
	{
		LTFLOAT fTotalLife;
		
		if(pHead)
			g_pLTClient->GetParticleTotalLifetime(m_hBossSmokeSystem, pHead, fTotalLife);

		while(pHead && pHead != pTail)
		{
			//get particle position
			LTVector vPartPos(0,0,0);
			g_pLTClient->GetParticlePos(m_hBossSmokeSystem, pHead, &vPartPos);

			//adjust for new pos
			vPartPos -= vOffset;

			//re-set pos
			g_pLTClient->SetParticlePos(m_hBossSmokeSystem, pHead, &vPartPos);

			// Now figure out the alpha
			LTFLOAT fLife;
			g_pLTClient->GetParticleLifetime(m_hBossSmokeSystem, pHead, fLife);
			pHead->m_Alpha = fLife / fTotalLife;
			
			pHead = pHead->m_pNext;
		}
	}

	// Now record the new pos
	m_vLastBossSmokePos = vPos;

	// Now see if it is time to make more particles..
	if(g_pLTClient->GetTime() - m_fLastBossSmokeTime > g_vtBossSmokeTime.GetFloat())
	{
		AddBossSmokeParticles();
	}

	// Now see if it is time to make more particles..
	if(g_pLTClient->GetTime() - m_fLastSparkTime > g_vtBossSparkTime.GetFloat())
	{
		LTRotation rRot;
		LTVector vU, vR, vF;
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);
		g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

		CPolyDebrisFX* pPolyDebrisFX = g_pFXButeMgr->GetPolyDebrisFX("Dunya_Sparks");

		if(pPolyDebrisFX)
			g_pFXButeMgr->CreatePolyDebrisFX(pPolyDebrisFX, vPos, -vF, -vF);

		// Mark the time
		m_fLastSparkTime = g_pLTClient->GetTime() + GetRandom(-0.3f, 0.3f);

		// Play a sound
		g_pClientSoundMgr->PlaySoundFromPos(vPos, "Dunya_Sparks");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ResetSpecialFX()
//
//	PURPOSE:	Reset the special FX stuff...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ResetSpecialFX()
{
	//Nuke all the onld stuff.
	RemoveLaserFX();

	if(m_hAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hAuraSystem);
		m_hAuraSystem = LTNULL;
	}

	if(m_hHeatAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hHeatAuraSystem);
		m_hHeatAuraSystem = LTNULL;
	}

	if(m_hElecAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hElecAuraSystem);
		m_hElecAuraSystem = LTNULL;
	}

	if(m_hCloakAuraSystem)
	{
		//remove the aura system
		g_pLTClient->RemoveObject(m_hCloakAuraSystem);
		m_hCloakAuraSystem = LTNULL;
	}

	for(int i=0; i<3; i++)
	{
		if(m_hFireSystem[i])
		{
			//remove the fire system
			g_pLTClient->RemoveObject(m_hFireSystem[i]);
			m_hFireSystem[i] = LTNULL;
		}
	}

	if(m_hFireSound)
	{
		//kill the looping fire sound
		g_pLTClient->KillSound(m_hFireSound);
		m_hFireSound = LTNULL;
	}

	// kill the sound if one is already playing
	if(m_hWeaponLoopSound)
	{
		g_pLTClient->KillSound(m_hWeaponLoopSound);
		m_hWeaponLoopSound = LTNULL;
	}

	if(m_pFlashlight)
	{
		delete m_pFlashlight;
		m_pFlashlight = LTNULL;
	}

	// clean up the targeting sprites
	for(i=0; i<3; i++)
	{
		if(m_hTargetingSprites[i])
		{
			g_pLTClient->RemoveObject(m_hTargetingSprites[i]);
			m_hTargetingSprites[i] = LTNULL;
		}
	}

	if (m_hFlare)
	{
        g_pLTClient->RemoveObject(m_hFlare);
        m_hFlare = LTNULL;
	}

	//clean up string
//	if(m_LFcs.hstrSpriteFile)
//		g_pLTClient->FreeString(m_LFcs.hstrSpriteFile);

	for( BleederList::iterator iter = m_BleederList.begin(); iter != m_BleederList.end(); ++iter )
	{
		delete (*iter);
	}

	// Clean up the flame thrower FX
	for(i=0;i<MAX_FLAME_SEGMENTS;i++)
	{
		if(m_hFlameEmitters[i])
		{
			//remove the system
			g_pLTClient->RemoveObject(m_hFlameEmitters[i]);
			m_hFlameEmitters[i] = LTNULL;
		}
	}

	// Clean up the boss smoke system...
	if(m_hBossSmokeSystem)
	{
		//remove the system
		g_pLTClient->RemoveObject(m_hBossSmokeSystem);
		m_hBossSmokeSystem = LTNULL;
	}

	// Okay now load up the new FX...
	HLOCALOBJ hPlayerObj = g_pClientDE->GetClientObject();
	if (hPlayerObj == m_hServerObject)//(m_cs.nClientID != 0) && hPlayerObj == m_hServerObject)
	{
		CreateFireFX();
	}
	else
	{
		//go ahead and load up the particle system
		CreateAuraFX();
		CreateHeatAuraFX();
		CreateElecAuraFX();
		CreateCloakAuraFX();
		CreateFireFX();
		CreateFlashlight();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleMessage_WeaponLoopSound()
//
//	PURPOSE:	Handle playing the looping fire sound...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleMessage_WeaponLoopSound(HMESSAGEREAD hRead)
{
	if(!hRead) return;

	// read in the message
	DBYTE nType		= g_pLTClient->ReadFromMessageByte(hRead);
	DBYTE nBarrelId	= g_pLTClient->ReadFromMessageByte(hRead);
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel(nBarrelId);

	// sanity check
	if(!pBarrel) 
		return;

	// kill the sound if one is already playing
	if(m_hWeaponLoopSound)
	{
		g_pLTClient->KillSound(m_hWeaponLoopSound);
		m_hWeaponLoopSound = LTNULL;
	}

	// play the looping sound
	{
		char* pBuf = LTNULL;

		switch(nType)
		{
			case WMKS_FIRE:				pBuf = pBarrel->szFireSound;			break;
			case WMKS_ALT_FIRE:			pBuf = pBarrel->szAltFireSound;			break;
			case WMKS_DRY_FIRE:			pBuf = pBarrel->szDryFireSound;			break;
			case WMKS_MISC0:
			case WMKS_MISC1:
			case WMKS_MISC2:			pBuf = pBarrel->szMiscSounds[nType];	break;

			case WMKS_INVALID:
			default:					return;
		}

		LTVector vPos;
		g_pInterface->GetObjectPos(m_hServerObject, &vPos);

		m_hWeaponLoopSound = g_pClientSoundMgr->PlaySoundFromPos(vPos, pBuf, pBarrel->fFireSoundRadius, SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
	}
}
















//-------------------------------------------------------------------------------------------------
// SFX_CharacterFactory
//-------------------------------------------------------------------------------------------------

const SFX_CharacterFactory SFX_CharacterFactory::registerThis;

CSpecialFX* SFX_CharacterFactory::MakeShape() const
{
	return new CCharacterFX();
}

