// ----------------------------------------------------------------------- //
//
// MODULE  : FogVolumeFX.h
//
// PURPOSE : Volumetric fog controller
//
// CREATED : 8/27/00
//
// ----------------------------------------------------------------------- //

#ifndef __FOG_VOLUME_FX_H__
#define __FOG_VOLUME_FX_H__

//-------------------------------------------------------------------------------------------------

#include "SpecialFX.h"

//-------------------------------------------------------------------------------------------------

struct FOGVOLCREATESTRUCT : public SFXCREATESTRUCT
{
	FOGVOLCREATESTRUCT();

	LTVector	vDims;
	LTVector	vColor;

	LTFLOAT		fDensity;
	LTFLOAT		fMinOpacity;
	LTFLOAT		fMaxOpacity;

	uint8		nRes;
	uint8		nShape;
	uint8		nGradient;
	uint8		nDensityFn;

	LTBOOL		bFogWorld;
	LTBOOL		bFogModels;
};

inline FOGVOLCREATESTRUCT::FOGVOLCREATESTRUCT()
{
	vDims.Init(128.0f, 128.0f, 128.0f);
	vColor.Init(1.0f, 1.0f, 1.0f);

	fDensity		= 1.0f;
	fMinOpacity		= 0.0f;
	fMaxOpacity		= 1.0f;

	nRes			= 32;
	nShape			= 0;
	nGradient		= 0;
	nDensityFn		= 0;

	bFogWorld		= LTTRUE;
	bFogModels		= LTTRUE;
}

//-------------------------------------------------------------------------------------------------

class CFogVolumeFX : public CSpecialFX
{
	public :

		CFogVolumeFX() : CSpecialFX() 
		{
			m_vDims.Init(0.0f, 0.0f, 0.0f);
			m_vColor.Init(0.0f, 0.0f, 0.0f);
			m_fDensity = 0.0f;

			m_vLastPos.Init(0.0f, 0.0f, 0.0f);
			m_dwLastUserFlags = 0;

			m_bNeedsUpdates = LTFALSE;
		}

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT *psfx);
		virtual LTBOOL Update();

		virtual LTBOOL OnServerMessage(HMESSAGEREAD hRead);

	protected :

		FOGVOLCREATESTRUCT	m_Fog;

		LTVector			m_vDims;
		LTVector			m_vColor;
		LTFLOAT				m_fDensity;

		LTVector			m_vLastPos;
		uint32				m_dwLastUserFlags;

		LTBOOL				m_bNeedsUpdates;
};


//-------------------------------------------------------------------------------------------------
// SFX_FogVolumeFactory
//-------------------------------------------------------------------------------------------------

#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_FogVolumeFactory : public CSpecialFXFactory
{
	SFX_FogVolumeFactory() : CSpecialFXFactory(SFX_FOG_VOLUME_ID) {;}
	static const SFX_FogVolumeFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __FOG_VOLUME_FX_H__

