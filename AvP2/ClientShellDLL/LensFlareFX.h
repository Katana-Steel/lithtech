// ----------------------------------------------------------------------- //
//
// MODULE  : LensFlareFX.h
//
// PURPOSE : LensFlare special fx class - Definition
//
// CREATED : 5/9/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __LENS_FLARE_FX_H__
#define __LENS_FLARE_FX_H__

#include "SpecialFX.h"

struct LENSFLARECREATESTRUCT : public SFXCREATESTRUCT
{
	LENSFLARECREATESTRUCT();
	
	DBOOL InitFromMessage(LENSFLARECREATESTRUCT & lens, HMESSAGEREAD hMessage);

	DBOOL	bInSkyBox;
	DBOOL	bCreateSprite;
	DBOOL	bSpriteOnly;
	DBOOL	bUseObjectAngle;
	DBOOL	bSpriteAdditive;
	DFLOAT	fSpriteOffset;
	DFLOAT	fMinAngle;
	DFLOAT	fMinSpriteAlpha;
	DFLOAT	fMaxSpriteAlpha;
	DFLOAT	fMinSpriteScale;
	DFLOAT	fMaxSpriteScale;
	HSTRING	hstrSpriteFile;
	DBOOL	bBlindingFlare;
	DFLOAT	fBlindObjectAngle;
	DFLOAT	fBlindCameraAngle;
	DFLOAT	fMinBlindScale;
	DFLOAT	fMaxBlindScale;
};

inline LENSFLARECREATESTRUCT::LENSFLARECREATESTRUCT()
{
	bInSkyBox			= DFALSE;
	bCreateSprite		= DFALSE;
	bSpriteOnly			= DFALSE;
	bUseObjectAngle		= DFALSE;
	bBlindingFlare		= DFALSE;
	bSpriteAdditive		= DTRUE;
	fSpriteOffset		= 0.0f;
	fMinAngle			= 0.0f;
	fMinSpriteAlpha		= 0.0f;
	fMaxSpriteAlpha		= 0.0f;
	fMinSpriteScale		= 0.0f;
	fMaxSpriteScale		= 0.0f;
	fBlindObjectAngle	= 0.0f;
	fBlindCameraAngle	= 0.0f;
	fMinBlindScale		= 0.0f;
	fMaxBlindScale		= 0.0f;
	hstrSpriteFile		= DNULL;
}

class CLensFlareFX : public CSpecialFX
{
	public :

		CLensFlareFX() : CSpecialFX() 
		{
			m_hFlare = DNULL;
			m_bWashout = LTFALSE;
		}

		~CLensFlareFX()
		{
			if (m_hFlare)
			{
				m_pClientDE->DeleteObject(m_hFlare);
			}

			if (m_cs.hstrSpriteFile)
			{
				m_pClientDE->FreeString(m_cs.hstrSpriteFile);
			}
		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update();
		virtual void  SetMode(const std::string& mode);

	protected :
	
		LENSFLARECREATESTRUCT		m_cs;
		HOBJECT						m_hFlare;
		LTBOOL						m_bWashout;
};

//-------------------------------------------------------------------------------------------------
// SFX_LensFlareFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_LensFlareFactory : public CSpecialFXFactory
{
	SFX_LensFlareFactory() : CSpecialFXFactory(SFX_LENSFLARE_ID) {;}
	static const SFX_LensFlareFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __LENS_FLARE_FX_H__