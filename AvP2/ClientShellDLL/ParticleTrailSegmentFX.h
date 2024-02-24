// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailSegmentFX.h
//
// PURPOSE : ParticleTrail segment special fx class - Definition
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_TRAIL_SEGMENT_FX_H__
#define __PARTICLE_TRAIL_SEGMENT_FX_H__

#include "BaseParticleSystemFX.h"

struct PARTICLETRAILFX;

// ----------------------------------------------------------------------- //

struct PTSCREATESTRUCT : public BPSCREATESTRUCT
{
	PTSCREATESTRUCT::PTSCREATESTRUCT();

	uint8		nType;

	LTVector	vLastPos;
	LTRotation	rLastRot;

	LTVector	vCurPos;
	LTRotation	rCurRot;
};

inline PTSCREATESTRUCT::PTSCREATESTRUCT()
{
	nType = -1;

	vLastPos.Init();
	rLastRot.Init();

	vCurPos.Init();
	rCurRot.Init();
}

// ----------------------------------------------------------------------- //

class CParticleTrailSegmentFX : public CBaseParticleSystemFX
{
	public :

		CParticleTrailSegmentFX() : CBaseParticleSystemFX() 
		{
			m_pPTFX = LTNULL;
			m_vLastPos.Init();
			m_rLastRot.Init();

			m_vCurPos.Init();
			m_rCurRot.Init();

			m_fStartTime = 0.0f;
			m_bFirstUpdate = LTTRUE;
		}

		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pLTClient);
		virtual LTBOOL Update();

	private :

		PARTICLETRAILFX	*m_pPTFX;	// Pointer to the particle trail FX struct
		LTVector		m_vLastPos;	// Last position of the server object
		LTRotation		m_rLastRot;	// Last rotation of the server object

		LTVector		m_vCurPos;	// Current position of the server object
		LTRotation		m_rCurRot;	// Current rotation of the server object

		LTFLOAT			m_fStartTime;
		LTBOOL			m_bFirstUpdate;
};

//-------------------------------------------------------------------------------------------------
// SFX_ParticleTrailSegFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ParticleTrailSegFactory : public CSpecialFXFactory
{
	SFX_ParticleTrailSegFactory() : CSpecialFXFactory(SFX_PARTICLETRAILSEG_ID) {;}
	static const SFX_ParticleTrailSegFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __PARTICLE_TRAIL_SEGMENT_FX_H__
