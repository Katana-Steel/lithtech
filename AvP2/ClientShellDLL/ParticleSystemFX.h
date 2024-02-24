// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystemFX.h
//
// PURPOSE : ParticleSystem special fx class - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_SYSTEM_FX_H__
#define __PARTICLE_SYSTEM_FX_H__

#include "BaseParticleSystemFX.h"


struct PSCREATESTRUCT : public BPSCREATESTRUCT
{
	PSCREATESTRUCT::PSCREATESTRUCT();

	DVector		vColor1;
	DVector		vColor2;
	DVector		vDims;
	DVector		vMinVel;
	DVector		vMaxVel;
	DVector		vPos;
	DDWORD		dwFlags;
	DFLOAT		fBurstWait;
	DFLOAT		fBurstWaitMin;
	DFLOAT		fBurstWaitMax;
	DFLOAT		fParticlesPerSecond;
	DFLOAT		fParticleLifetime;
	DFLOAT		fParticleRadius;
	DFLOAT		fGravity;
	DFLOAT		fRotationVelocity;
	DFLOAT		fViewDist;
	HSTRING		hstrTextureName;
};

inline PSCREATESTRUCT::PSCREATESTRUCT()
{
	vColor1.Init();
	vColor2.Init();
	vDims.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vPos.Init();
	dwFlags				= 0;
	fBurstWait			= 0.0f;
	fParticlesPerSecond	= 0.0f;
	fParticleLifetime	= 0.0f;
	fParticleRadius		= 0.0f;
	fGravity			= 0.0f;
	fRotationVelocity	= 0.0f;
	fViewDist			= 0.0f;
	fBurstWaitMin		= 0.01f;
	fBurstWaitMax		= 1.0f;
	hstrTextureName		= DNULL;
}

class CParticleSystemFX : public CBaseParticleSystemFX
{
	public :

		CParticleSystemFX();
		~CParticleSystemFX()
		{
			if (m_cs.hstrTextureName && m_pClientDE)
			{
				m_pClientDE->FreeString(m_cs.hstrTextureName);
			}
		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	protected :

		// Creation data...

		PSCREATESTRUCT	m_cs;		// Holds all initialization data

		DBOOL	m_bFirstUpdate;		// Is this the first update
		DFLOAT	m_fNextUpdate;		// Time between updates
		DFLOAT	m_fLastTime;		// When was the last time
		DFLOAT	m_fMaxViewDistSqr;	// Max dist to add particles (squared)
		LTFLOAT	m_fParticlesToAdd;	// How many lines to add this time...

		DVector	m_vMinOffset;
		DVector	m_vMaxOffset;
};

//-------------------------------------------------------------------------------------------------
// SFX_ParticleSystemFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ParticleSystemFactory : public CSpecialFXFactory
{
	SFX_ParticleSystemFactory() : CSpecialFXFactory(SFX_PARTICLESYSTEM_ID) {;}
	static const SFX_ParticleSystemFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __PARTICLE_SYSTEM_FX_H__