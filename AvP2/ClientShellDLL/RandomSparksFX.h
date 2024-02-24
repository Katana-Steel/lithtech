// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSparksFX.h
//
// PURPOSE : Sparks special fx class - Definition
//
// CREATED : 1/21/99
//
// ----------------------------------------------------------------------- //

#ifndef __RANDOM_SPARKS_FX_H__
#define __RANDOM_SPARKS_FX_H__

#include "BaseParticleSystemFX.h"
#include "LTString.h"

struct RANDOMSPARKSCREATESTRUCT : public BPSCREATESTRUCT
{
	RANDOMSPARKSCREATESTRUCT::RANDOMSPARKSCREATESTRUCT();

	DVector vPos;
	DVector vDir;
	DBYTE	nSparks;
	DFLOAT	fDuration;
	DVector	vMinVelAdjust;
	DVector	vMaxVelAdjust;
	DFLOAT	fRadius;
};

inline RANDOMSPARKSCREATESTRUCT::RANDOMSPARKSCREATESTRUCT()
{
	vPos.Init();
	vDir.Init();
	vMinVelAdjust.Init(1, 0.025f, 1);
	vMaxVelAdjust.Init(1, 1, 1);
	nSparks		= 0;
	fDuration	= 0.0f;
	fRadius		= 300.0f;
}


class CRandomSparksFX : public CBaseParticleSystemFX
{
	public :

		CRandomSparksFX() : CBaseParticleSystemFX() 
		{
			VEC_INIT(m_vDir);
			m_nSparks	 = 5;
			m_fDuration	 = 1.0f;
			m_fStartTime = 0.0f;
			m_vMinVelAdjust.Init(1, 0.025f, 1);
			m_vMaxVelAdjust.Init(1, 1, 1);
		}

		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	private :

		DBOOL AddSparks();

		DVector m_vDir;				// Direction sparks shoot
		DBYTE	m_nSparks;			// Number of sparks
		DFLOAT	m_fDuration;		// Life time of sparks
		DFLOAT	m_fStartTime;		// When did we start
		DVector	m_vMinVelAdjust;	// How much to adjust the min velocity
		DVector	m_vMaxVelAdjust;	// How much to adjust the max velocity.
};

//-------------------------------------------------------------------------------------------------
// SFX_RandomSparksFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_RandomSparksFactory : public CSpecialFXFactory
{
	SFX_RandomSparksFactory() : CSpecialFXFactory(SFX_RANDOMSPARKS_ID) {;}
	static const SFX_RandomSparksFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __RANDOM_SPARKS_FX_H__