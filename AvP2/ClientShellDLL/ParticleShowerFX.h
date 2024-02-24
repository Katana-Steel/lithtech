// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleShowerFX.h
//
// PURPOSE : ParticleShower special fx class - Definition 
//			(used to be CSparksFX)
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_SHOWER_FX_H__
#define __PARTICLE_SHOWER_FX_H__

#include "BaseParticleSystemFX.h"
#include "ContainerCodes.h"

struct PARTICLESHOWERCREATESTRUCT : public BPSCREATESTRUCT
{
	PARTICLESHOWERCREATESTRUCT::PARTICLESHOWERCREATESTRUCT();

	LTVector vPos;
	LTVector vDir;
	LTVector vColor1;
	LTVector vColor2;
	uint8	nParticles;
	LTFLOAT fMinSize;
	LTFLOAT fMaxSize;
	LTFLOAT	fDuration;
    LTFLOAT fFadeTime;
	LTFLOAT	fEmissionRadius;
	LTFLOAT	fRadius;
	LTFLOAT	fGravity;
	LTBOOL	bRemoveIfNotLiquid;
	char*	pTexture;
};

inline PARTICLESHOWERCREATESTRUCT::PARTICLESHOWERCREATESTRUCT()
{
	vPos.Init();
	vDir.Init();
	vColor1.Init();
	vColor2.Init();
	nParticles		= 0;
	fMinSize		= 0.0f;
	fMaxSize		= 0.0f;
	fDuration		= 0.0f;
    fFadeTime		= 0.0f;
	fEmissionRadius	= 0.0f;
	fRadius			= 0.0f;
	fGravity		= 0.0f;
	pTexture		= DNULL;
	bRemoveIfNotLiquid	= LTFALSE;
}


class CParticleShowerFX : public CBaseParticleSystemFX
{
	public :

		CParticleShowerFX() : CBaseParticleSystemFX() 
		{
			VEC_SET(m_vColor1, 255.0f, 255.0f, 255.0f);
			VEC_SET(m_vColor2, 255.0f, 255.0f, 0.0f);

			m_fStartTime = 0.0f;
		}

		virtual LTBOOL CreateObject(CClientDE* pClientDE);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CParticleShowerFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage);
		virtual LTBOOL Update();

	private :

		LTBOOL AddParticles();

		PARTICLESHOWERCREATESTRUCT m_cs;

		LTFLOAT	m_fStartTime;		// When did we start
		ContainerCode	GetPointContainer(LTVector &vPos, HOBJECT &hObj);
};

//-------------------------------------------------------------------------------------------------
// SFX_ParticleShowerFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ParticleShowerFactory : public CSpecialFXFactory
{
	SFX_ParticleShowerFactory() : CSpecialFXFactory(SFX_PARTICLESHOWER_ID) {;}
	static const SFX_ParticleShowerFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __PARTICLE_SHOWER_FX_H__