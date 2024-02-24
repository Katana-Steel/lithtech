// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeFX.h
//
// PURPOSE : Smoke special fx class - Definition
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __SMOKE_FX_H__
#define __SMOKE_FX_H__

#include "BaseParticleSystemFX.h"
#include "ContainerCodes.h"


struct SMCREATESTRUCT : public BPSCREATESTRUCT
{
    SMCREATESTRUCT();

    LTVector vPos;
    LTVector vColor1;
    LTVector vColor2;
    LTVector vMinDriftVel;
    LTVector vMaxDriftVel;
    LTFLOAT fLifeTime;
    LTFLOAT fVolumeRadius;
    LTFLOAT fRadius;
    LTFLOAT fParticleCreateDelta;
    LTFLOAT fMinParticleLife;
    LTFLOAT fMaxParticleLife;
    uint8   nNumParticles;
    uint32  dwSystemFlags;
    LTBOOL  bIgnoreWind;
	HSTRING hstrTexture;
    LTBOOL  bUpdatePos;
	LTBOOL	bRemoveIfNotLiquid;
};

inline SMCREATESTRUCT::SMCREATESTRUCT()
{
	vPos.Init();
	vColor1.Init();
	vColor2.Init();
	vMinDriftVel.Init();
	vMaxDriftVel.Init();
	fLifeTime				= 0.0f;
	fVolumeRadius			= 0.0f;
	fRadius					= 0.0f;
	fParticleCreateDelta	= 0.0f;
	fMinParticleLife		= 0.0f;
	fMaxParticleLife		= 0.0f;
	nNumParticles			= 0;
	dwSystemFlags			= 0;
    bIgnoreWind             = LTFALSE;
    hstrTexture             = LTNULL;
	bUpdatePos				= LTTRUE;
	bRemoveIfNotLiquid		= LTFALSE;
}

class CSmokeFX : public CBaseParticleSystemFX
{
	public :

		CSmokeFX() : CBaseParticleSystemFX()
		{
			m_vMinDriftVel.Init();
			m_vMaxDriftVel.Init();
			m_fStartTime			= -1.0f;
			m_fLastTime				= -1.0f;
			m_fVolumeRadius			= 0.0f;
			m_fLifeTime				= 0.0f;
			m_fParticleCreateDelta	= 0.0f;
			m_nNumParticles			= 5;
			m_fMinParticleLife		= 5.0f;
			m_fMaxParticleLife		= 10.0f;
            m_bIgnoreWind           = LTFALSE;
            m_hstrTexture           = LTNULL;
			m_bUpdatePos			= LTTRUE;
			m_vLastPos.Init();
			m_bRemoveIfNotLiquid		= LTFALSE;
		}

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

        inline void SetDriftVel(LTVector vMinVel, LTVector vMaxVel)
		{
			m_vMinDriftVel = vMinVel;
			m_vMaxDriftVel = vMaxVel;
		}

	private :

        LTFLOAT  m_fVolumeRadius;        // Radius of smoke volume
        LTFLOAT  m_fLifeTime;            // How long each particle stays around
        LTFLOAT  m_fStartTime;           // When did we start this crazy thing
        LTFLOAT  m_fLastTime;            // When did we last create particles

        LTVector m_vMinDriftVel;         // Min Drift velocity
        LTVector m_vMaxDriftVel;         // Max Drift velocity

        LTFLOAT  m_fParticleCreateDelta; // How often we create smoke particles
        uint8   m_nNumParticles;        // Number we create every delta
        LTFLOAT  m_fMaxParticleLife;     // Maximum lifetime of a particle
        LTFLOAT  m_fMinParticleLife;     // Minimum lifetime of a particle
		HSTRING m_hstrTexture;			// Texture to sprite to use

        LTBOOL   m_bIgnoreWind;          // Ignore world wind

		LTBOOL	m_bUpdatePos;			// Should I update my position and rotation with 
										// the movement of the server object.

		LTBOOL	m_bRemoveIfNotLiquid;
		LTVector m_vLastPos;

		ContainerCode GetPointContainer(LTVector &vPos, HOBJECT &hObj);
};

//-------------------------------------------------------------------------------------------------
// SFX_SmokeFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_SmokeFactory : public CSpecialFXFactory
{
	SFX_SmokeFactory() : CSpecialFXFactory(SFX_SMOKE_ID) {;}
	static const SFX_SmokeFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __SMOKE_FX_H__
