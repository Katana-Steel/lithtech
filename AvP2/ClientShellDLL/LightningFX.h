// ----------------------------------------------------------------------- //
//
// MODULE  : LightningFX.h
//
// PURPOSE : Lightning special fx class - Definition
//
// CREATED : 4/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LIGHTNING_FX_H__
#define __LIGHTNING_FX_H__

// ----------------------------------------------------------------------- //

#include "SpecialFX.h"
#include "TemplateList.h"
#include "PolyLineFX.h"

// ----------------------------------------------------------------------- //

struct LFXCREATESTRUCT : public SFXCREATESTRUCT
{
    LFXCREATESTRUCT();

	PLFXCREATESTRUCT	lfx;

    LTVector	vLightColor;
    LTBOOL		bOneTimeOnly;
    LTBOOL		bDynamicLight;
	LTFLOAT		fLightRadius;
    LTFLOAT		fMinDelayTime;
    LTFLOAT		fMaxDelayTime;

	uint16		nNearSound;
	uint16		nFarSound;
	LTFLOAT		fNearRadius;
	LTFLOAT		fDelayRadius;
};

// ----------------------------------------------------------------------- //

inline LFXCREATESTRUCT::LFXCREATESTRUCT()
{
	vLightColor.Init();

	fLightRadius	= 0.0f;
    bDynamicLight	= LTFALSE;
	bOneTimeOnly	= LTFALSE;
    fMinDelayTime	= 0.0f;
    fMaxDelayTime	= 0.0f;

	nNearSound		= -1;
	nFarSound		= -1;
	fNearRadius		= 1000.0f;
	fDelayRadius	= 1500.0f;
}

// ----------------------------------------------------------------------- //

struct PolyVert
{
	PolyVert::PolyVert()
	{
		vPos.Init();
		fOffset = 0.0f;
		fPosOffset = 0.0f;
	}

    LTVector vPos;
    LTFLOAT  fOffset;
    LTFLOAT  fPosOffset;
};

// ----------------------------------------------------------------------- //

typedef CTList<PolyVert*> PolyVertList;

// ----------------------------------------------------------------------- //

class CLightningFX : public CSpecialFX
{
	public :

		CLightningFX() : CSpecialFX()
		{
            m_hLight            = LTNULL;
			m_fStartTime		= 0.0f;
			m_fEndTime			= 0.0f;
            m_bFirstTime        = LTTRUE;
            m_bPlayedSound      = LTTRUE;
			m_fPlaySoundTime	= 0.0f;

			m_vMidPos.Init();

			m_hstrTexture		= LTNULL;
		}

		~CLightningFX()
		{
            if (g_pLTClient)
			{
				if (m_hLight)
				{
                    g_pLTClient->DeleteObject(m_hLight);
				}

				if (m_hstrTexture)
				{
					g_pLTClient->FreeString(m_hstrTexture);
				}
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
		virtual	LTBOOL OnServerMessage(HMESSAGEREAD hMessage);

	protected :

 		LFXCREATESTRUCT	m_cs;

        LTFLOAT      m_fStartTime;
        LTFLOAT      m_fEndTime;
		LTFLOAT      m_fDelayTime;
        LTVector     m_vMidPos;

        LTFLOAT     m_fPlaySoundTime;
        LTBOOL      m_bPlayedSound;
        LTBOOL      m_bFirstTime;
 		uint16		m_nSound;

		HOBJECT		m_hLight;
		HSTRING		m_hstrTexture;
 
		CPolyLineFX	m_Line;		// Lightning

        LTBOOL      Setup();
		void		HandleFirstTime();
 		void		UpdateSound();
};

//-------------------------------------------------------------------------------------------------
// SFX_LightningFactory
//-------------------------------------------------------------------------------------------------

#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_LightningFactory : public CSpecialFXFactory
{
	SFX_LightningFactory() : CSpecialFXFactory(SFX_LIGHTNING_ID) {;}
	static const SFX_LightningFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

// ----------------------------------------------------------------------- //

#endif // __LIGHTNING_FX_H__
