// ----------------------------------------------------------------------- //
//
// MODULE  : MuzzleFlashParticleFX.cpp
//
// PURPOSE : MuzzleFlash special FX - Implementation
//
// CREATED : 1/17/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MuzzleFlashParticleFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "iltmodel.h"
#include "ilttransform.h"

#define MFPFX_INFINITE_DURATION 1000000.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Init
//
//	PURPOSE:	Init the MuzzleFlash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CBaseParticleSystemFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Don't support server-side versions of this fx...

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Init
//
//	PURPOSE:	Init the MuzzleFlash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	MFPCREATESTRUCT* pMF = (MFPCREATESTRUCT*)psfxCreateStruct;

	m_cs = *((MFPCREATESTRUCT*)pMF);

    if (!m_cs.pPMuzzleFX) return LTFALSE;

	// Make sure parent fx has correct values...

	m_basecs.bAdditive = m_cs.bAdditive	= m_cs.pPMuzzleFX->bAdditive;
	m_basecs.bMultiply = m_cs.bMultiply	= m_cs.pPMuzzleFX->bMultiply;

	m_vPos		= m_cs.vPos;
	m_rRot		= m_cs.rRot;
	m_fRadius	= m_cs.pPMuzzleFX->fRadius;
	m_fGravity	= 0.0f;
	m_vColor1	= m_cs.pPMuzzleFX->vStartColor;
	m_vColor2	= m_cs.pPMuzzleFX->vEndColor;
	
	VEC_SET(m_vColorRange,	m_vColor2.x - m_vColor1.x,
							m_vColor2.y - m_vColor1.y,
							m_vColor2.z - m_vColor1.z);

	// Set our server object to our fired from so we get notified when
	// the fired from goes away...

	if (!m_hServerObject && m_cs.hFiredFrom)
	{
		m_hServerObject = m_cs.hFiredFrom;

		const DDWORD dwCFlags = g_pLTClient->GetObjectClientFlags(m_hServerObject);
		g_pLTClient->SetObjectClientFlags(m_hServerObject, dwCFlags | CF_NOTIFYREMOVE);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE ) return LTFALSE;

	if (m_cs.pPMuzzleFX->szFile[0])
	{
		m_pTextureName = m_cs.pPMuzzleFX->szFile;
	}

    LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet)
	{
		bRet = AddMuzzleFlash();
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Reset
//
//	PURPOSE:	Reset the muzzle flash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Reset(MFPCREATESTRUCT & mfcs)
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

	m_cs = mfcs;

    if (!m_cs.pPMuzzleFX) return LTFALSE;

	m_vPos			= m_cs.vPos;
	m_rRot			= m_cs.rRot;
	m_fRadius		= m_cs.pPMuzzleFX->fRadius;
	m_fGravity		= 0.0f;
	m_vColor1	= m_cs.pPMuzzleFX->vStartColor;
	m_vColor2	= m_cs.pPMuzzleFX->vEndColor;
	
	VEC_SET(m_vColorRange,	m_vColor2.x - m_vColor1.x,
							m_vColor2.y - m_vColor1.y,
							m_vColor2.z - m_vColor1.z);

	if (m_cs.pPMuzzleFX->szFile[0])
	{
		m_pTextureName = m_cs.pPMuzzleFX->szFile;
	}

	RemoveAllParticles();

	CBaseParticleSystemFX::SetupSystem();

	return AddMuzzleFlash();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Reset
//
//	PURPOSE:	Reset the muzzle flash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Reset()
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

    if (!m_cs.pPMuzzleFX) return LTFALSE;

	m_vPos		= m_cs.vPos;
	m_rRot		= m_cs.rRot;
	m_fRadius	= m_cs.pPMuzzleFX->fRadius;
	m_fGravity	= 0.0f;
	m_vColor1	= m_cs.pPMuzzleFX->vStartColor;
	m_vColor2	= m_cs.pPMuzzleFX->vEndColor;
	
	VEC_SET(m_vColorRange,	m_vColor2.x - m_vColor1.x,
							m_vColor2.y - m_vColor1.y,
							m_vColor2.z - m_vColor1.z);

	if (m_cs.pPMuzzleFX->szFile[0])
	{
		m_pTextureName = m_cs.pPMuzzleFX->szFile;
	}

	RemoveAllParticles();

	CBaseParticleSystemFX::SetupSystem();

	return AddMuzzleFlash();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::GetColor
//
//	PURPOSE:	Get the color
//
// ----------------------------------------------------------------------- //

LTVector CMuzzleFlashParticleFX::GetColor(LTFLOAT fRatio)
{
	return (m_vColor2 - (m_vColorRange*fRatio));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::Update()
{
    if (!m_hObject || !m_pClientDE || !m_cs.pPMuzzleFX) return LTFALSE;

	// If we have a fired from, it should be the same as our
	// server object.  If our server object has been removed, in
	// this case, we should go away...

    if (m_cs.hFiredFrom && !m_hServerObject) return LTFALSE;


    LTFLOAT fTime = m_pClientDE->GetTime();

    LTFLOAT fDuration = m_cs.bPlayerView ? MFPFX_INFINITE_DURATION : m_cs.pPMuzzleFX->fDuration;

	// Check to see if we should go away...

	if (fTime > m_fStartTime + fDuration)
	{
		return LTFALSE;
	}

	LTParticle *pHead;
	LTParticle *pTail;
	g_pLTClient->GetParticles(m_hObject, &pHead, &pTail);

	LTFLOAT fRatio = 1 - (fTime-m_fStartTime) / m_cs.pPMuzzleFX->fDuration;

	if(fRatio < 0.0f)
		fRatio = 0.0f;

	LTVector vColor = GetColor(fRatio);
	LTFLOAT	fSize = m_cs.pPMuzzleFX->fRadius * (m_cs.pPMuzzleFX->fStartScale + (1-fRatio)*(m_cs.pPMuzzleFX->fEndScale - m_cs.pPMuzzleFX->fStartScale)); 

	do
	{
		if (pHead)
		{
			pHead->m_Size	= fSize;
			pHead->m_Alpha	= fRatio;
			pHead->m_Color	= vColor;

			pHead = pHead->m_pNext;
		}
	}while (pHead && (pHead != pTail) );

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFlashParticleFX::AddMuzzleFlash
//
//	PURPOSE:	Make the MuzzleFlash
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFlashParticleFX::AddMuzzleFlash()
{
	if (!m_hObject || !m_pClientDE || !m_cs.pPMuzzleFX) return LTFALSE;

	LTParticle* pParticle = LTNULL;

	LTVector vF, vU, vR;
	m_pClientDE->GetRotationVectors(&m_cs.rRot, &vU, &vR, &vF);

	LTFLOAT fLength = m_cs.pPMuzzleFX->fLength;

	LTVector vPos(0, 0, 0), vVel(0, 0, 0), vColor;
	int nNumParticles = GetNumParticles(m_cs.pPMuzzleFX->nNumParticles);

	LTFLOAT fFDelta = fLength / (LTFLOAT)nNumParticles;
	LTFLOAT fDuration = m_cs.bPlayerView ? MFPFX_INFINITE_DURATION : m_cs.pPMuzzleFX->fDuration;
	vPos.z = fLength;

	// Add all the particles...stepping along the forward vector...
	for (LTFLOAT fFOffset = 0.0f; fFOffset < fLength; fFOffset += fFDelta)
	{
		// Add particle at current position along the forward vector...
		vVel.y = GetRandom(m_cs.pPMuzzleFX->fMinVertVel, m_cs.pPMuzzleFX->fMaxVertVel);
		vVel.x = GetRandom(m_cs.pPMuzzleFX->fMinHorizVel, m_cs.pPMuzzleFX->fMaxHorizVel);

		pParticle = m_pClientDE->AddParticle(m_hObject, &vPos, &vVel, & m_cs.pPMuzzleFX->vStartColor, fDuration);

		// Adjust particle size...
		if (pParticle)
		{
//			LTFLOAT fDistPercent = 1.0f - (fFOffset / fLength);
//			LTFLOAT fScale = (fDistPercent * m_cs.pPMuzzleFX->fStartScale);
//			fScale *= GetRandom(0.75f, 1.25f);

			pParticle->m_Size	= m_fRadius * m_cs.pPMuzzleFX->fStartScale;
			pParticle->m_Alpha	= m_cs.pPMuzzleFX->fStartAlpha;
		}

		vPos.z -= fFDelta;

		if(vPos.z < 0.0f)
			vPos.z = 0.0f;
	}

	m_fStartTime = m_pClientDE->GetTime();

    return LTTRUE;
}