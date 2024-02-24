// ----------------------------------------------------------------------- //
//
// MODULE  : VolumeBrushFX.cpp
//
// PURPOSE : VolumeBrush special fx class - Definition
//
// CREATED : 4/1/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#ifndef __VOLUME_BRUSH_FX_H__
#include "VolumeBrushFX.h"
#endif

#include "ContainerCodes.h"

//-------------------------------------------------------------------------------------------------
//	Constructor
//-------------------------------------------------------------------------------------------------

CVolumeBrushFX::CVolumeBrushFX() : CSpecialFX()
{
	m_vFogColor.Init();
	m_vTintColor.Init(255.0f, 255.0f, 255.0f);
	m_vLightAdd.Init();
	m_bFogEnable	= LTFALSE;
	m_fMinFogFarZ	= 0.0f;
	m_fMinFogNearZ	= 0.0f;
	m_fFogFarZ		= 0.0f;
	m_fFogNearZ		= 0.0f;

	m_fViscosity	= 0.0f;
	m_fFriction		= 5.0f;
	m_vCurrent.Init();
}

//-------------------------------------------------------------------------------------------------
//	Init functions
//-------------------------------------------------------------------------------------------------

LTBOOL CVolumeBrushFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	VBCREATESTRUCT vb;

	vb.hServerObj	= hServObj;
	vb.bFogEnable	= (LTBOOL)hRead->ReadByte();
	vb.fMinFogFarZ	= hRead->ReadFloat();
	vb.fMinFogNearZ	= hRead->ReadFloat();
	vb.fFogFarZ		= hRead->ReadFloat();
	vb.fFogNearZ	= hRead->ReadFloat();
	vb.vFogColor	= hRead->ReadVector();
	vb.vTintColor	= hRead->ReadVector();
	vb.vLightAdd	= hRead->ReadVector();

	vb.fViscosity	= hRead->ReadFloat();
	vb.fFriction	= hRead->ReadFloat();
	vb.vCurrent		= hRead->ReadVector();

	return Init(&vb);
}

//-------------------------------------------------------------------------------------------------

LTBOOL CVolumeBrushFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	VBCREATESTRUCT* pVB = (VBCREATESTRUCT*)psfxCreateStruct;

	m_bFogEnable	= pVB->bFogEnable;
	m_fMinFogFarZ	= pVB->fMinFogFarZ;
	m_fMinFogNearZ	= pVB->fMinFogNearZ;
	m_fFogFarZ		= pVB->fFogFarZ;
	m_fFogNearZ		= pVB->fFogNearZ;
	m_vFogColor		= pVB->vFogColor;
	m_vTintColor	= pVB->vTintColor;
	m_vLightAdd		= pVB->vLightAdd;
	m_fViscosity	= pVB->fViscosity;
	m_fFriction		= pVB->fFriction;
	m_vCurrent		= pVB->vCurrent;

	m_vLightAdd.x /= 255.0f;
	m_vLightAdd.y /= 255.0f;
	m_vLightAdd.z /= 255.0f;

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// Create
//-------------------------------------------------------------------------------------------------

LTBOOL CVolumeBrushFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	// Special case for liquid volume brushes, set the FLAG_RAYHIT flag...
	uint16 code;

	if (m_hServerObject && pClientDE->GetContainerCode(m_hServerObject, &code))
	{
		if (IsLiquid((ContainerCode)code))
		{
			DDWORD dwFlags = pClientDE->GetObjectFlags(m_hServerObject);
			pClientDE->SetObjectFlags(m_hServerObject, dwFlags | FLAG_RAYHIT);
		}
	}

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_VolumeBrushFactory
//-------------------------------------------------------------------------------------------------

const SFX_VolumeBrushFactory SFX_VolumeBrushFactory::registerThis;

CSpecialFX* SFX_VolumeBrushFactory::MakeShape() const
{
	return new CVolumeBrushFX();
}

