// ----------------------------------------------------------------------- //
//
// MODULE  : FireFX.cpp
//
// PURPOSE : FireFX special FX - Implementation
//
// CREATED : 5/06/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FireFX.h"
#include "RandomSparksFX.h"
#include "GameClientShell.h"
#include "iltcustomdraw.h"
#include "FXButeMgr.h"

#define FFX_DEFAULT_RADIUS					100.0f
#define	FFX_MIN_RADIUS						20.0f
#define	FFX_MAX_RADIUS						500.0f
#define	FFX_INNER_CAM_RADIUS				300.0f
#define	FFX_CAM_FALLOFF_RANGE				300.0f
#define FFX_DEFAULT_SMOKE_PARTICLE_RADIUS	7000.0f
#define FFX_DEFAULT_FIRE_PARTICLE_RADIUS	4000.0f
#define FFX_MAX_SMOKE_PARTICLE_RADIUS		(FFX_DEFAULT_SMOKE_PARTICLE_RADIUS * 1.3f)
#define FFX_MAX_FIRE_PARTICLE_RADIUS		(FFX_DEFAULT_FIRE_PARTICLE_RADIUS)
#define FFX_MIN_FIRE_PARTICLE_LIFETIME		0.25f
#define FFX_MAX_FIRE_PARTICLE_LIFETIME		2.0f
#define FFX_MIN_SMOKE_PARTICLE_LIFETIME		0.5f
#define FFX_MAX_SMOKE_PARTICLE_LIFETIME		6.0f
#define FFX_MAX_LIGHT_RADIUS				300.0f


extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Init
//
//	PURPOSE:	Init the lightning fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Read in the init info from the message...

	FIRECREATESTRUCT fire;
	fire.hServerObj		= hServObj;
	fire.nFireObjFXId	= g_pLTClient->ReadFromMessageByte(hMessage);

	return Init(&fire);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Init
//
//	PURPOSE:	Init the Fire fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((FIRECREATESTRUCT*)psfxCreateStruct);

	FIREOBJFX *pFire = g_pFXButeMgr->GetFireObjFX(m_cs.nFireObjFXId);

	if(pFire)
	{
		m_cs.fRadius = pFire->fSizeRadiusScale < FFX_MIN_RADIUS ? FFX_MIN_RADIUS :
				(pFire->fSizeRadiusScale > FFX_MAX_RADIUS ? FFX_MAX_RADIUS : pFire->fSizeRadiusScale);

		m_fSizeAdjust = m_cs.fRadius / FFX_DEFAULT_RADIUS;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return LTFALSE;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTFALSE;

	// Get a handle to the Fire obj FX
	FIREOBJFX *pFire = g_pFXButeMgr->GetFireObjFX(m_cs.nFireObjFXId);
	if(!pFire) return LTFALSE;

    LTVector vZero(0, 0, 0), vOffset(0, 0, 0);
    uint32 dwFlags;
	CString str;

	// Get our initial pos...
	if (m_cs.vPos.Equals(vZero) && m_hServerObject)
	{
        g_pLTClient->GetObjectPos(m_hServerObject, &(m_cs.vPos));
	}


	// Setup the smoke create structure with common parameters across
	// all the combined FX
	SMCREATESTRUCT sm;
	sm.hServerObj		= m_hServerObject;
    sm.bRelToCameraPos  = LTTRUE;
	sm.fInnerCamRadius	= FFX_INNER_CAM_RADIUS;
	sm.fOuterCamRadius	= FFX_INNER_CAM_RADIUS + (FFX_CAM_FALLOFF_RANGE * m_fSizeAdjust);

	// Create the smoke particles...
	SMOKEFX *pSmokeFX = g_pFXButeMgr->GetSmokeFX(pFire->nSmokeFX);

	if(pSmokeFX)
	{
		vOffset.Init(0, 20.0f * m_fSizeAdjust, 0);
		vOffset.y = vOffset.y > 50.0f ? 50.0f : vOffset.y;

		sm.vPos					= m_cs.vPos + vOffset;

		sm.hstrTexture			= g_pLTClient->CreateString(pSmokeFX->szFile);
		sm.vColor1				= pSmokeFX->vColor1;
		sm.vColor2				= pSmokeFX->vColor2;
		sm.vMinDriftVel			= pSmokeFX->vMinDrift;
		sm.vMaxDriftVel			= pSmokeFX->vMaxDrift;
		sm.fLifeTime			= pSmokeFX->fLifetime;
		sm.fVolumeRadius		= pSmokeFX->fEmitRadius * (1.5f * m_fSizeAdjust);
		sm.fRadius				= pSmokeFX->fRadius * m_fSizeAdjust;
		sm.fParticleCreateDelta	= pSmokeFX->fCreateDelta;
		sm.fMinParticleLife		= pSmokeFX->fMinParticleLife * m_fSizeAdjust;
		sm.fMaxParticleLife		= pSmokeFX->fMaxParticleLife * m_fSizeAdjust;
		sm.nNumParticles		= pSmokeFX->nNumParticles;
		sm.bIgnoreWind			= pSmokeFX->bIgnorWind;

		sm.bAdditive			= pSmokeFX->bAdditive;
		sm.bMultiply			= pSmokeFX->bMultiply;

		sm.bAdjustParticleAlpha	= pSmokeFX->bAdjustAlpha;
		sm.fStartParticleAlpha	= pSmokeFX->fStartAlpha;
		sm.fEndParticleAlpha	= pSmokeFX->fEndAlpha;

		sm.bAdjustParticleScale	= pSmokeFX->bAdjustScale;
		sm.fStartParticleScale	= pSmokeFX->fStartScale;
		sm.fEndParticleScale	= pSmokeFX->fEndScale;
		sm.bUpdatePos			= pSmokeFX->bUpdatePos;

		sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_SMOKE_PARTICLE_LIFETIME ? FFX_MIN_SMOKE_PARTICLE_LIFETIME :
			(sm.fMinParticleLife > FFX_MAX_SMOKE_PARTICLE_LIFETIME ? FFX_MAX_SMOKE_PARTICLE_LIFETIME : sm.fMinParticleLife);
		sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_SMOKE_PARTICLE_LIFETIME ? FFX_MAX_SMOKE_PARTICLE_LIFETIME :
			(sm.fMaxParticleLife < FFX_MIN_SMOKE_PARTICLE_LIFETIME ? FFX_MIN_SMOKE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
		sm.fRadius = sm.fRadius > FFX_MAX_SMOKE_PARTICLE_RADIUS ? FFX_MAX_SMOKE_PARTICLE_RADIUS : sm.fRadius;

		if(!m_Smoke1.Init(&sm) || !m_Smoke1.CreateObject(g_pLTClient))
            return LTFALSE;

        g_pLTClient->FreeString(sm.hstrTexture);

        dwFlags = g_pLTClient->GetObjectFlags(m_Smoke1.GetObject());
        g_pLTClient->SetObjectFlags(m_Smoke1.GetObject(), dwFlags | FLAG_NOGLOBALLIGHTSCALE);

		m_Smoke1.Update();
	}

	// Create the inner and outer fire particles...
	SMOKEFX *pInnerFireFX = g_pFXButeMgr->GetSmokeFX(pFire->nInnerFireFX);
	SMOKEFX *pOuterFireFX = g_pFXButeMgr->GetSmokeFX(pFire->nOuterFireFX);

	if(pInnerFireFX || pOuterFireFX)
	{
        LTFLOAT fVolumeAdjust = m_fSizeAdjust < 1.0 ? m_fSizeAdjust / 1.5f : m_fSizeAdjust * 1.5f;

		sm.vPos					= m_cs.vPos;
		sm.hServerObj			= m_hServerObject;

		if(pInnerFireFX)
		{
			sm.hstrTexture			= g_pLTClient->CreateString(pInnerFireFX->szFile);
			sm.vColor1				= pInnerFireFX->vColor1;
			sm.vColor2				= pInnerFireFX->vColor2;
			sm.vMinDriftVel			= pInnerFireFX->vMinDrift;
			sm.vMaxDriftVel			= pInnerFireFX->vMaxDrift;
			sm.fLifeTime			= pInnerFireFX->fLifetime;
			sm.fVolumeRadius		= pInnerFireFX->fEmitRadius * fVolumeAdjust;
			sm.fRadius				= pInnerFireFX->fRadius * m_fSizeAdjust;
			sm.fParticleCreateDelta	= pInnerFireFX->fCreateDelta;
			sm.fMinParticleLife		= pInnerFireFX->fMinParticleLife * m_fSizeAdjust;
			sm.fMaxParticleLife		= pInnerFireFX->fMaxParticleLife * m_fSizeAdjust;
			sm.nNumParticles		= pInnerFireFX->nNumParticles;
			sm.bIgnoreWind			= pInnerFireFX->bIgnorWind;

			sm.bAdditive			= pInnerFireFX->bAdditive;
			sm.bMultiply			= pInnerFireFX->bMultiply;

			sm.bAdjustParticleAlpha	= pInnerFireFX->bAdjustAlpha;
			sm.fStartParticleAlpha	= pInnerFireFX->fStartAlpha;
			sm.fEndParticleAlpha	= pInnerFireFX->fEndAlpha;

			sm.bAdjustParticleScale	= pInnerFireFX->bAdjustScale;
			sm.fStartParticleScale	= pInnerFireFX->fStartScale;
			sm.fEndParticleScale	= pInnerFireFX->fEndScale;
			sm.bUpdatePos			= pInnerFireFX->bUpdatePos;

			sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME :
				(sm.fMinParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME : sm.fMinParticleLife);
			sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME :
				(sm.fMaxParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
			sm.fRadius = sm.fRadius > FFX_MAX_FIRE_PARTICLE_RADIUS ? FFX_MAX_FIRE_PARTICLE_RADIUS : sm.fRadius;


			if (!m_Fire1.Init(&sm) || !m_Fire1.CreateObject(m_pClientDE))
				return LTFALSE;

			g_pLTClient->FreeString(sm.hstrTexture);

			dwFlags = g_pLTClient->GetObjectFlags(m_Fire1.GetObject());
			g_pLTClient->SetObjectFlags(m_Fire1.GetObject(), dwFlags | FLAG_NOGLOBALLIGHTSCALE);

			m_Fire1.Update();
		}

		if(pOuterFireFX)
		{
			sm.hstrTexture			= g_pLTClient->CreateString(pOuterFireFX->szFile);
			sm.vColor1				= pOuterFireFX->vColor1;
			sm.vColor2				= pOuterFireFX->vColor2;
			sm.vMinDriftVel			= pOuterFireFX->vMinDrift;
			sm.vMaxDriftVel			= pOuterFireFX->vMaxDrift;
			sm.fLifeTime			= pOuterFireFX->fLifetime;
			sm.fVolumeRadius		= pOuterFireFX->fEmitRadius * fVolumeAdjust;
			sm.fRadius				= pOuterFireFX->fRadius * m_fSizeAdjust;
			sm.fParticleCreateDelta	= pOuterFireFX->fCreateDelta;
			sm.fMinParticleLife		= pOuterFireFX->fMinParticleLife * m_fSizeAdjust;
			sm.fMaxParticleLife		= pOuterFireFX->fMaxParticleLife * m_fSizeAdjust;
			sm.nNumParticles		= pOuterFireFX->nNumParticles;
			sm.bIgnoreWind			= pOuterFireFX->bIgnorWind;

			sm.bAdditive			= pOuterFireFX->bAdditive;
			sm.bMultiply			= pOuterFireFX->bMultiply;

			sm.bAdjustParticleAlpha	= pOuterFireFX->bAdjustAlpha;
			sm.fStartParticleAlpha	= pOuterFireFX->fStartAlpha;
			sm.fEndParticleAlpha	= pOuterFireFX->fEndAlpha;

			sm.bAdjustParticleScale	= pOuterFireFX->bAdjustScale;
			sm.fStartParticleScale	= pOuterFireFX->fStartScale;
			sm.fEndParticleScale	= pOuterFireFX->fEndScale;
			sm.bUpdatePos			= pOuterFireFX->bUpdatePos;

			sm.fMinParticleLife	= sm.fMinParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME :
				(sm.fMinParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME : sm.fMinParticleLife);
			sm.fMaxParticleLife	= sm.fMaxParticleLife > FFX_MAX_FIRE_PARTICLE_LIFETIME ? FFX_MAX_FIRE_PARTICLE_LIFETIME :
				(sm.fMaxParticleLife < FFX_MIN_FIRE_PARTICLE_LIFETIME ? FFX_MIN_FIRE_PARTICLE_LIFETIME : sm.fMaxParticleLife);
			sm.fRadius = sm.fRadius > FFX_MAX_FIRE_PARTICLE_RADIUS ? FFX_MAX_FIRE_PARTICLE_RADIUS : sm.fRadius;

			sm.bUpdatePos = LTFALSE;

			if (!m_Fire2.Init(&sm) || !m_Fire2.CreateObject(m_pClientDE))
				return LTFALSE;

			dwFlags = g_pLTClient->GetObjectFlags(m_Fire2.GetObject());
			g_pLTClient->SetObjectFlags(m_Fire2.GetObject(), dwFlags | FLAG_NOGLOBALLIGHTSCALE);

			m_Fire2.Update();
		}
	}

	// Create the sound effect
	SOUNDFX *pSoundFX = g_pFXButeMgr->GetSoundFX(pFire->nSoundFX);

	if(pSoundFX)
	{
		m_hSound = g_pClientSoundMgr->PlaySoundFromPos(m_cs.vPos, pSoundFX->szFile,
			pSoundFX->fRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
	}

	// Create the dynamic light...
	if(pFire->bCreateLight)
	{
		LIGHTCREATESTRUCT light;

        LTFLOAT fRadiusMin = pFire->fLightRadius;
		fRadiusMin = fRadiusMin < 20.0f ? 20.0f : (fRadiusMin > FFX_MAX_LIGHT_RADIUS ? FFX_MAX_LIGHT_RADIUS : fRadiusMin);

		light.hServerObj			= m_hServerObject;

		light.vColor				= pFire->vLightColor;
		light.vOffset				= pFire->vLightOffset;
		light.dwLightFlags			= FLAG_DONTLIGHTBACKFACING;
		light.fIntensityMin			= 1.0f;
		light.fIntensityMax			= 1.0f;
		light.nIntensityWaveform	= WAVE_NONE;
		light.fIntensityFreq		= 1.0f;
		light.fIntensityPhase		= 0.0f;
		light.fRadiusMin			= fRadiusMin;
		light.fRadiusMax			= fRadiusMin * 1.1f;
		light.nRadiusWaveform		= WAVE_FLICKER2;
		light.fRadiusFreq			= pFire->fLightFreq;
		light.fRadiusPhase			= pFire->fLightPhase;
		light.m_hLightAnim			= INVALID_LIGHT_ANIM;

		if(!m_Light.Init(&light) || !m_Light.CreateObject(m_pClientDE))
		{
            return LTFALSE;
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFireFX::Update
//
//	PURPOSE:	Update the Fire Fx
//
// ----------------------------------------------------------------------- //

LTBOOL CFireFX::Update()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr || !m_pClientDE || !m_hServerObject) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (m_bWantRemove)
	{
        return LTFALSE;
	}


	// Create the random spark particles...
	FIREOBJFX *pFire = g_pFXButeMgr->GetFireObjFX(m_cs.nFireObjFXId);


	// Update FX...

	if(pFire->bCreateLight)
		m_Light.Update();

	m_Fire1.Update();
	m_Fire2.Update();

	m_Smoke1.Update();

	// Hide/show the fire if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			if (m_hSound)
			{
                g_pLTClient->KillSoundLoop(m_hSound);
                m_hSound = LTNULL;
			}

            return LTTRUE;
		}
		else
		{
			if (!m_hSound)
			{
				SOUNDFX *pSoundFX = g_pFXButeMgr->GetSoundFX(pFire->nSoundFX);

				if(pSoundFX)
				{
					m_hSound = g_pClientSoundMgr->PlaySoundFromPos(m_cs.vPos, pSoundFX->szFile,
						pSoundFX->fRadius, SOUNDPRIORITY_MISC_MEDIUM, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
				}
			}
			else
			{
				LTVector vPos;
				g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
				g_pLTClient->SetSoundPosition(m_hSound, &vPos);
			}
		}
	}

	if (pFire->bCreateSparks && GetRandom(1, 10) == 1)
	{
		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
        if (!psfxMgr) return LTFALSE;

		RANDOMSPARKSCREATESTRUCT sparks;
		sparks.hServerObj = m_hServerObject;

        LTFLOAT fVel = m_fSizeAdjust * GetRandom(50.0f, 70.0f);
		fVel = (fVel < 30.0f ? 30.0f : (fVel > 100.0f ? 100.0f : fVel));

        LTVector vDir(0.0, 1.0, 0.0);
		sparks.vMinVelAdjust.Init(1, 3, 1);
		sparks.vMaxVelAdjust.Init(1, 6, 1);
		sparks.vDir	= vDir * fVel;
		sparks.nSparks = GetRandom(1, 5);
		sparks.fDuration = m_fSizeAdjust * GetRandom(1.0f, 2.0f);
        sparks.bRelToCameraPos = LTTRUE;
		sparks.fInnerCamRadius = FFX_INNER_CAM_RADIUS;
		sparks.fOuterCamRadius = FFX_INNER_CAM_RADIUS + (FFX_CAM_FALLOFF_RANGE * m_fSizeAdjust);
		sparks.fRadius = 300.0f * m_fSizeAdjust;
		sparks.fRadius = sparks.fRadius < 100.0f ? 100.0f : (sparks.fRadius > 500.0f ? 500.0f : sparks.fRadius);

		psfxMgr->CreateSFX(SFX_RANDOMSPARKS_ID, &sparks);
	}

    return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_FireFactory
//-------------------------------------------------------------------------------------------------

const SFX_FireFactory SFX_FireFactory::registerThis;

CSpecialFX* SFX_FireFactory::MakeShape() const
{
	return new CFireFX();
}


