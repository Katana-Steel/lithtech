// ----------------------------------------------------------------------- //
//
// MODULE  : PickupObjectFX.h
//
// PURPOSE : PickupObjectFX - Definition
//
// CREATED : 8/4/00
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPOBJECT_FX_H__
#define __PICKUPOBJECT_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"

// ----------------------------------------------------------------------- //

struct PUFXCREATESTRUCT : public SFXCREATESTRUCT
{
	PUFXCREATESTRUCT::PUFXCREATESTRUCT();
	uint8	nFXType;
};	

// ----------------------------------------------------------------------- //

inline PUFXCREATESTRUCT::PUFXCREATESTRUCT()
{
	nFXType = -1;
}

// ----------------------------------------------------------------------- //

class CPickupObjectFX : public CSpecialFX
{
	public :

		CPickupObjectFX() : CSpecialFX()
		{
			m_nFXType = -1;
			m_vOrigPos.Init();

			m_fGlowTime = 0.0f;
			m_fBounceTime = 0.0f;
			m_fParticleTime = 0.0f;

			m_hParticleSystem = LTNULL;
			m_hLight = LTNULL;
		}

		~CPickupObjectFX()
		{
			if(m_hParticleSystem)
				g_pLTClient->DeleteObject(m_hParticleSystem);

			if(m_hLight)
				g_pLTClient->DeleteObject(m_hLight);
		}

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(ILTClient* pLTClient);
		virtual LTBOOL Update();

	private :

		uint8		m_nFXType;			// Type of FX to use out of the attribute file

		LTVector	m_vOrigPos;			// Original server object position

		LTFLOAT		m_fGlowTime;
		LTFLOAT		m_fBounceTime;
		LTFLOAT		m_fParticleTime;

		HOBJECT		m_hParticleSystem;	// Handle to the particle system
		HOBJECT		m_hLight;			// Handle to the light
};

//-------------------------------------------------------------------------------------------------
// SFX_PickupObjectFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_PickupObjectFactory : public CSpecialFXFactory
{
	SFX_PickupObjectFactory() : CSpecialFXFactory(SFX_PICKUPOBJECT_ID) {;}
	static const SFX_PickupObjectFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __PICKUPOBJECT_FX_H__
