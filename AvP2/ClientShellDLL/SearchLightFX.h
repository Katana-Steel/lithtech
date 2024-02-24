// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLightFX.h
//
// PURPOSE : SearchLight special fx class - Definition
//
// CREATED : 6/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SEARCH_LIGHT_FX_H__
#define __SEARCH_LIGHT_FX_H__

#include "SpecialFX.h"
#include "LensFlareFX.h"

struct SEARCHLIGHTCREATESTRUCT : public SFXCREATESTRUCT
{
	SEARCHLIGHTCREATESTRUCT();

	DFLOAT	fBeamLength;
	DFLOAT	fBeamRadius;
	DFLOAT	fBeamAlpha;
	DFLOAT	fBeamRotTime;
	DFLOAT	fLightRadius;
	DVector	vLightColor;
	DBOOL	bBeamAdditive;

	LENSFLARECREATESTRUCT lens;
};

inline SEARCHLIGHTCREATESTRUCT::SEARCHLIGHTCREATESTRUCT()
{
	fBeamLength		= 0.0f;
	fBeamRadius		= 0.0f;
	fBeamAlpha		= 0.0f;
	fBeamRotTime	= 0.0f;
	fLightRadius	= 0.0f;
	bBeamAdditive	= DTRUE;
	vLightColor.Init();
}


class CSearchLightFX : public CSpecialFX
{
	public :

		CSearchLightFX() : CSpecialFX() 
		{
			m_hBeam		= DNULL;
			m_hLight	= DNULL;

			m_fBeamRotation = 0.0f;
		}

		~CSearchLightFX()
		{
			if (m_hBeam)
			{
				m_pClientDE->DeleteObject(m_hBeam);
			}

			if (m_hLight)
			{
				m_pClientDE->DeleteObject(m_hLight);
			}
		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();

	protected :
	
		SEARCHLIGHTCREATESTRUCT		m_cs;
		HOBJECT						m_hBeam;
		HOBJECT						m_hLight;

		DFLOAT						m_fBeamRotation;
		CLensFlareFX				m_LensFlare;
};

//-------------------------------------------------------------------------------------------------
// SFX_SearchLightFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_SearchLightFactory : public CSpecialFXFactory
{
	SFX_SearchLightFactory() : CSpecialFXFactory(SFX_SEARCHLIGHT_ID) {;}
	static const SFX_SearchLightFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __SEARCH_LIGHT_FX_H__