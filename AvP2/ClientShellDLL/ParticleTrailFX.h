// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailFX.h
//
// PURPOSE : ParticleTrail special fx class - Definition
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_TRAIL_FX_H__
#define __PARTICLE_TRAIL_FX_H__

#include "SpecialFX.h"

// ----------------------------------------------------------------------- //

struct PTCREATESTRUCT : public SFXCREATESTRUCT
{
	PTCREATESTRUCT::PTCREATESTRUCT();
	uint8 nType;
	uint8 nUWType;
};

inline PTCREATESTRUCT::PTCREATESTRUCT()
{
	nType = -1;
	nUWType = -1;
}

// ----------------------------------------------------------------------- //

class CParticleTrailFX : public CSpecialFX
{
	public :

		CParticleTrailFX() : CSpecialFX() 
		{
			m_nType = -1;
			m_nUWType = -1;
		}

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL Update();

	private :

		LTVector	m_vLastPos;		// Last particle trail position
		LTRotation	m_rLastRot;		// Last particle trail rotation

		int			m_nType;		// Type of particle trail from the FX butes
		int			m_nUWType;		// Type of particle trail from the FX butes for Underwater
};

//-------------------------------------------------------------------------------------------------
// SFX_ParticleTrailFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ParticleTrailFactory : public CSpecialFXFactory
{
	SFX_ParticleTrailFactory() : CSpecialFXFactory(SFX_PARTICLETRAIL_ID) {;}
	static const SFX_ParticleTrailFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __PARTICLE_TRAIL_FX_H__
