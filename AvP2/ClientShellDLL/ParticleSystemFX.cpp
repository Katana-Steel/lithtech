// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.cpp
//
// PURPOSE : ParticleSystem special FX - Implementation
//
// CREATED : 10/24/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleSystemFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "VarTrack.h"

#define MAX_PARTICLES_PER_SECOND 5000
#define MAX_PS_VIEW_DIST_SQR	(10000*10000)	// Max global distance to add particles

extern CGameClientShell*	g_pGameClientShell;

extern DVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CParticleSystemFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CParticleSystemFX::CParticleSystemFX() : CBaseParticleSystemFX()
{
	m_bFirstUpdate			= DTRUE;
	m_fLastTime				= 0.0f;
	m_fNextUpdate			= 0.01f;
	m_fParticlesToAdd			= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Init
//
//	PURPOSE:	Init the particle system fx
//
// ----------------------------------------------------------------------- //

DBOOL CParticleSystemFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CBaseParticleSystemFX::Init(hServObj, hMessage)) return DFALSE;
	if (!hMessage) return DFALSE;

	PSCREATESTRUCT ps;

	ps.hServerObj = hServObj;
	g_pClientDE->ReadFromMessageVector(hMessage, &(ps.vColor1));
	g_pClientDE->ReadFromMessageVector(hMessage, &(ps.vColor2));
	g_pClientDE->ReadFromMessageVector(hMessage, &(ps.vDims));
	g_pClientDE->ReadFromMessageVector(hMessage, &(ps.vMinVel));
	g_pClientDE->ReadFromMessageVector(hMessage, &(ps.vMaxVel));
	ps.dwFlags			   = (DDWORD) g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fBurstWait		   = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fBurstWaitMin	   = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fBurstWaitMax	   = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fParticlesPerSecond = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fParticleLifetime   = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fParticleRadius	   = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fGravity			   = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fRotationVelocity   = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.fViewDist           = g_pClientDE->ReadFromMessageFloat(hMessage);
	ps.hstrTextureName	   = g_pClientDE->ReadFromMessageHString(hMessage);

	return Init(&ps);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CParticleSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	// Set up our creation struct...

	PSCREATESTRUCT* pPS = (PSCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pPS;

	// Set our (parent's) flags...

	m_dwFlags  = m_cs.dwFlags;
	m_fRadius  = m_cs.fParticleRadius;
	m_fGravity = m_cs.fGravity;
	m_vPos	   = m_cs.vPos;

	// Set our max viewable distance...

	m_fMaxViewDistSqr = m_cs.fViewDist*m_cs.fViewDist;
	m_fMaxViewDistSqr = m_fMaxViewDistSqr > MAX_PS_VIEW_DIST_SQR ? MAX_PS_VIEW_DIST_SQR : m_fMaxViewDistSqr;

	
	m_vMinOffset = -m_cs.vDims;
	m_vMaxOffset = m_cs.vDims;

	// Adjust velocities based on global wind values...

	m_cs.vMinVel += g_vWorldWindVel;
	m_cs.vMaxVel += g_vWorldWindVel;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CParticleSystemFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	if (m_cs.hstrTextureName)
	{
		m_pTextureName = pClientDE->GetStringData(m_cs.hstrTextureName);
	}

	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet && m_hObject && m_hServerObject)
	{
		DRotation rRot;
		pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		pClientDE->SetObjectRotation(m_hObject, &rRot);

		DDWORD dwUserFlags;
		pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);
		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			DDWORD dwFlags = pClientDE->GetObjectFlags(m_hObject);
			pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleSystemFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CParticleSystemFX::Update()
{
	if (!m_hObject || !m_pClientDE || IsWaitingForRemove()) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Hide/show the particle system if necessary...

	if (m_hServerObject)
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			// Once last puff as disappeared, hide the system (no new puffs
			// will be added...)

			if (dwFlags & FLAG_VISIBLE)
			{
				if (fTime > m_fLastTime + m_cs.fParticleLifetime)
				{
					m_pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
				}
			}
			else
			{
				m_fLastTime = fTime;
			}

			return DTRUE;
		}
		else
		{
			m_pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		}
	}


	if (m_bFirstUpdate)
	{
		m_fLastTime = fTime;
		m_bFirstUpdate = DFALSE;
	}


	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
		return DTRUE;
	}


	// Ok, how many to add this frame....(make sure time delta is no more than
	// 15 frames/sec...

	float fTimeDelta = fTime - m_fLastTime;
	fTimeDelta = fTimeDelta > 0.0666f ? 0.0666f : fTimeDelta;

	m_fParticlesToAdd += m_cs.fParticlesPerSecond * fTimeDelta;

	// Cap it
	if(m_fParticlesToAdd > MAX_PARTICLES_PER_SECOND)
		m_fParticlesToAdd = MAX_PARTICLES_PER_SECOND;
	
	// Add new lines...
	if(m_fParticlesToAdd > 1.0f)
	{
		int nNumParticles = GetNumParticles((int)m_fParticlesToAdd);

		m_pClientDE->AddParticles(m_hObject, nNumParticles,
			&m_vMinOffset, &m_vMaxOffset,			// Position offset
			&(m_cs.vMinVel), &(m_cs.vMaxVel),		// Velocity
			&(m_cs.vColor1), &(m_cs.vColor2),		// Color
			m_cs.fParticleLifetime, m_cs.fParticleLifetime);

		m_fParticlesToAdd -= (int)m_fParticlesToAdd;
	}

	// Determine when next update should occur...

	if (m_cs.fBurstWait > 0.001f) 
	{
		m_fNextUpdate = m_cs.fBurstWait * GetRandom(m_cs.fBurstWaitMin, m_cs.fBurstWaitMax);
	}
	else 
	{
		m_fNextUpdate = 0.001f;
	}

	
	// Rotate the particle system...

	if (m_cs.fRotationVelocity != 0.0f)
	{
		DRotation rRot;
		m_pClientDE->GetObjectRotation(m_hObject, &rRot);
		m_pClientDE->EulerRotateY(&rRot, m_pClientDE->GetFrameTime() * m_cs.fRotationVelocity);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	m_fLastTime = fTime;

	return DTRUE;
}


//-------------------------------------------------------------------------------------------------
// SFX_ParticleSystemFactory
//-------------------------------------------------------------------------------------------------

const SFX_ParticleSystemFactory SFX_ParticleSystemFactory::registerThis;

CSpecialFX* SFX_ParticleSystemFactory::MakeShape() const
{
	return new CParticleSystemFX();
}

