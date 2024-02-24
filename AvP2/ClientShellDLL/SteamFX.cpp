// ----------------------------------------------------------------------- //
//
// MODULE  : SteamFX.cpp
//
// PURPOSE : Steam special FX - Implementation
//
// CREATED : 10/19/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SteamFX.h"
#include "VarTrack.h"
#include "GameClientShell.h"
#include "iltcustomdraw.h"
#include "SoundMgr.h"
#include "FXButeMgr.h"

static VarTrack	s_cvarTweak;

extern CGameClientShell*	g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::CSteamFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CSteamFX::CSteamFX() : CSpecialFX()
{
	m_nSteamFXId		= -1;
	m_nSoundFXId		= -1;
    m_hSound            = LTNULL;
	m_dwLastUserFlags	= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::Init
//
//	PURPOSE:	Init the particle system fx
//
// ----------------------------------------------------------------------- //

LTBOOL CSteamFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	m_nSteamFXId = g_pLTClient->ReadFromMessageByte(hMessage);
	m_nSoundFXId = g_pLTClient->ReadFromMessageByte(hMessage);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CSteamFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	// Set up our creation struct...

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CSteamFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	// Make sure the particles are moving...

    LTRotation rRot;
	pClientDE->GetObjectRotation(m_hServerObject, &rRot);

    LTVector vU, vR, vF;
	pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	SMOKEFX *pSmokeFX = g_pFXButeMgr->GetSmokeFX(m_nSteamFXId);

	if(!pSmokeFX)
		return LTFALSE;

	// Create the "smoke" that we'll ust as steam...

	SMCREATESTRUCT sm;

	// Don't set the position, make it use the server object
	// so that we can be keyframed on the server...
	// pClientDE->GetObjectPos(m_hServerObject, &(sm.vPos));

	sm.hServerObj			= m_hServerObject;

	sm.hstrTexture			= g_pLTClient->CreateString(pSmokeFX->szFile);
	sm.vColor1				= pSmokeFX->vColor1;
	sm.vColor2				= pSmokeFX->vColor2;
	m_vMinDrift				= pSmokeFX->vMinDrift;
	m_vMaxDrift				= pSmokeFX->vMaxDrift;
	sm.fLifeTime			= pSmokeFX->fLifetime;
	sm.fVolumeRadius		= pSmokeFX->fEmitRadius;
	sm.fRadius				= pSmokeFX->fRadius;
	sm.fParticleCreateDelta	= pSmokeFX->fCreateDelta;
	sm.fMinParticleLife		= pSmokeFX->fMinParticleLife;
	sm.fMaxParticleLife		= pSmokeFX->fMaxParticleLife;
	sm.nNumParticles		= pSmokeFX->nNumParticles;
	sm.bIgnoreWind			= pSmokeFX->bIgnorWind;
	sm.bUpdatePos			= pSmokeFX->bUpdatePos;

	sm.bAdditive			= pSmokeFX->bAdditive;
	sm.bMultiply			= pSmokeFX->bMultiply;

	sm.bAdjustParticleAlpha	= pSmokeFX->bAdjustAlpha;
	sm.fStartParticleAlpha	= pSmokeFX->fStartAlpha;
	sm.fEndParticleAlpha	= pSmokeFX->fEndAlpha;

	sm.bAdjustParticleScale	= pSmokeFX->bAdjustScale;
	sm.fStartParticleScale	= pSmokeFX->fStartScale;
	sm.fEndParticleScale	= pSmokeFX->fEndScale;

	sm.vMinDriftVel			= (vR * m_vMinDrift.x) + (vU * m_vMinDrift.y) + (vF * m_vMinDrift.z);
	sm.vMaxDriftVel			= (vR * m_vMaxDrift.x) + (vU * m_vMaxDrift.y) + (vF * m_vMaxDrift.z);

	sm.bIgnoreParentRemoval	= LTTRUE;

	m_Steam.Init(&sm);

    LTBOOL bRet = m_Steam.CreateObject(pClientDE);

	g_pLTClient->FreeString(sm.hstrTexture);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CSteamFX::Update()
{
    if (m_bWantRemove) return LTFALSE;

	// Debugging aid...

	if (s_cvarTweak.GetFloat() > 0)
	{
		TweakSystem();
	}

	// Start/stop steam sound if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			if ((m_dwLastUserFlags & USRFLG_VISIBLE))
			{
				StopSound();
			}
		}
		else  // visible
		{
			if (!(m_dwLastUserFlags & USRFLG_VISIBLE))
			{
				StartSound();
			}
		}

		m_dwLastUserFlags = dwUserFlags;

		// Make sure the sound is in the correct place (in case we are getting
		// keyframed)...

		if (m_hSound)
		{
            LTVector vPos;
            g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
            g_pLTClient->SetSoundPosition(m_hSound, &vPos);
		}


		// Update the steam velocity based on our current rotation (again
		// for keyframing)...

        LTRotation rRot;
        g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

        LTVector vU, vR, vF;
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

		LTVector vMin = (vR * m_vMinDrift.x) + (vU * m_vMinDrift.y) + (vF * m_vMinDrift.z);
		LTVector vMax = (vR * m_vMaxDrift.x) + (vU * m_vMaxDrift.y) + (vF * m_vMaxDrift.z);

		m_Steam.SetDriftVel(vMin, vMax);
	}

	return m_Steam.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::StartSound
//
//	PURPOSE:	Start the sound...
//
// ----------------------------------------------------------------------- //

void CSteamFX::StartSound()
{
	SOUNDFX *pSoundFX = g_pFXButeMgr->GetSoundFX(m_nSoundFXId);

	if(pSoundFX && !m_hSound)
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP;
		m_hSound = g_pClientSoundMgr->PlaySoundFromPos(vPos, pSoundFX->szFile, pSoundFX->fRadius, SOUNDPRIORITY_MISC_MEDIUM, dwFlags);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::StopSound
//
//	PURPOSE:	Stop the sound...
//
// ----------------------------------------------------------------------- //

void CSteamFX::StopSound()
{
	if (m_hSound)
	{
        g_pLTClient->KillSound(m_hSound);
        m_hSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamFX::TweakSystem
//
//	PURPOSE:	Tweak the particle system
//
// ----------------------------------------------------------------------- //

void CSteamFX::TweakSystem()
{
 /*   LTFLOAT fIncValue = 0.01f;
    LTBOOL bChanged = LTFALSE;

    LTVector vScale;
	vScale.Init();

    uint32 dwPlayerFlags = g_pGameClientShell->GetPlayerFlags();

	// Move faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .5f;
	}

	// Move Red up/down...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{

		//m_cs.vMinVel
		//m_cs.vMaxVel
		//m_cs.vColor1
		//m_cs.vColor2

        bChanged = LTTRUE;
	}


	// Add/Subtract number of particles per second

	if ((dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) || (dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
        //m_cs.fParticlesPerSecond += (LTFLOAT)(fIncValue * 101.0);

		//m_cs.fParticlesPerSecond = m_cs.fParticlesPerSecond < 0.0f ? 0.0f :
		//	(m_cs.fParticlesPerSecond > MAX_PARTICLES_PER_SECOND ? MAX_PARTICLES_PER_SECOND : m_cs.fParticlesPerSecond);

        bChanged = LTTRUE;
	}


	// Lower/Raise burst wait...

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;

        bChanged = LTTRUE;
	}


	if (bChanged)
	{
		//g_pGameClientShell->CSPrint("Particles per second: %.2f", m_cs.fParticlesPerSecond);
	}
*/
}

//-------------------------------------------------------------------------------------------------
// SFX_SteamFactory
//-------------------------------------------------------------------------------------------------

const SFX_SteamFactory SFX_SteamFactory::registerThis;

CSpecialFX* SFX_SteamFactory::MakeShape() const
{
	return new CSteamFX();
}


