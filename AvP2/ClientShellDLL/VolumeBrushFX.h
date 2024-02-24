// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushFX.h
//
// PURPOSE : VolumeBrush special fx class - Definition
//
// CREATED : 4/1/98
//
// ----------------------------------------------------------------------- //

#ifndef __VOLUME_BRUSH_FX_H__
#define __VOLUME_BRUSH_FX_H__

#include "SpecialFX.h"

// ----------------------------------------------------------------------- //
// SFX create structure for the volume brush class

struct VBCREATESTRUCT : public SFXCREATESTRUCT
{
	VBCREATESTRUCT::VBCREATESTRUCT();

	LTBOOL		bFogEnable;
	LTFLOAT		fMinFogFarZ;
	LTFLOAT		fMinFogNearZ;
	LTFLOAT		fFogFarZ;
	LTFLOAT		fFogNearZ;
	LTVector	vFogColor;
	LTVector	vTintColor;
	LTVector	vLightAdd;

	LTFLOAT		fViscosity;
	LTFLOAT		fFriction;
	LTVector	vCurrent;
};

inline VBCREATESTRUCT::VBCREATESTRUCT()
{
	bFogEnable		= LTFALSE;
	fMinFogFarZ		= 0.0f;
	fMinFogNearZ	= 0.0f;
	fFogFarZ		= 0.0f;
	fFogNearZ		= 0.0f;
	vFogColor.Init();
	vTintColor.Init();
	vLightAdd.Init();

	fViscosity		= 0.0f;
	fFriction		= 0.0f;
	vCurrent.Init();
}

// ----------------------------------------------------------------------- //

class CVolumeBrushFX : public CSpecialFX
{
	public :

		// Constructors
		CVolumeBrushFX();

		// Initialization functions for this SFX
		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

		// Update this SFX
		virtual LTBOOL Update()			{ return !IsWaitingForRemove(); }

		// Object creation function
		LTBOOL CreateObject(CClientDE* pClientDE);

		// Access functions for the fog values
		LTBOOL		IsFogEnable()		const { return m_bFogEnable; }
		LTVector	GetFogColor()		const { return m_vFogColor; }
		LTFLOAT		GetMinFogFarZ()		const { return m_fMinFogFarZ; }
		LTFLOAT		GetMinFogNearZ()	const { return m_fMinFogNearZ; }
		LTFLOAT		GetFogFarZ()		const { return m_fFogFarZ; }
		LTFLOAT		GetFogNearZ()		const { return m_fFogNearZ; }

		// Access function for the tint color
		LTVector	GetTintColor()		const { return m_vTintColor; }

		// Access function for the camera light add value
		LTVector	GetLightAdd()		const { return m_vLightAdd; }

		// Access function for physics modifiers
		LTFLOAT		GetViscosity()		const { return m_fViscosity; }
		LTFLOAT		GetFriction()		const { return m_fFriction; }
		LTVector	GetCurrent()		const { return m_vCurrent; }

	protected :

		// Values required for client-side SFX (ie. Fog, LightAdd, Tinting, etc.)
		LTBOOL		m_bFogEnable;
		LTFLOAT		m_fMinFogFarZ;
		LTFLOAT		m_fMinFogNearZ;
		LTFLOAT		m_fFogFarZ;
		LTFLOAT		m_fFogNearZ;
		LTVector	m_vFogColor;
		LTVector	m_vTintColor;
		LTVector	m_vLightAdd;

		// Values required for client-side physics
		LTFLOAT		m_fViscosity;
		LTFLOAT		m_fFriction;
		LTVector	m_vCurrent;
};

//-------------------------------------------------------------------------------------------------
// SFX_VolumeBrushFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_VolumeBrushFactory : public CSpecialFXFactory
{
	SFX_VolumeBrushFactory() : CSpecialFXFactory(SFX_VOLUMEBRUSH_ID) {;}
	static const SFX_VolumeBrushFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __VOLUME_BRUSH_FX_H__