// ----------------------------------------------------------------------- //
//
// MODULE  : FireFX.cpp
//
// PURPOSE : FireFX special FX - Definitions
//
// CREATED : 5/06/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FIRE_FX_H__
#define __FIRE_FX_H__

#include "SpecialFX.h"
#include "SmokeFX.h"
#include "LightFX.h"

struct FIRECREATESTRUCT : public SFXCREATESTRUCT
{
    FIRECREATESTRUCT();

    LTVector	vPos;
	LTFLOAT		fRadius;
	uint8		nFireObjFXId;
};

inline FIRECREATESTRUCT::FIRECREATESTRUCT()
{
	vPos.Init();
	nFireObjFXId	= -1;
}

class CFireFX : public CSpecialFX
{
	public :

		CFireFX() : CSpecialFX()
		{
            m_hSound = LTNULL;
			m_fSizeAdjust = 1.0f;
		}

		~CFireFX()
		{
			if (m_hSound)
			{
                g_pLTClient->KillSoundLoop(m_hSound);
			}
		}

        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();

	private :

		FIRECREATESTRUCT	m_cs;
        HLTSOUND            m_hSound;
        LTFLOAT             m_fSizeAdjust;

		CSmokeFX			m_Smoke1;
		CSmokeFX			m_Fire1;
		CSmokeFX			m_Fire2;
		CLightFX			m_Light;
};

//-------------------------------------------------------------------------------------------------
// SFX_FireFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_FireFactory : public CSpecialFXFactory
{
	SFX_FireFactory() : CSpecialFXFactory(SFX_FIRE_ID) {;}
	static const SFX_FireFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __FIRE_FX_H__