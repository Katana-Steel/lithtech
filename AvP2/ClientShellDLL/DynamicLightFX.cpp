// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicLightFX.cpp
//
// PURPOSE : Dynamic light special FX - Implementation
//
// CREATED : 2/25/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DynamicLightFX.h"
#include "cpp_client_de.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDynamicLightFX::Init
//
//	PURPOSE:	Init the dynamic light
//
// ----------------------------------------------------------------------- //

DBOOL CDynamicLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	DLCREATESTRUCT* pDL = (DLCREATESTRUCT*)psfxCreateStruct;
	
	VEC_COPY(m_vColor, pDL->vColor);
	VEC_COPY(m_vPos, pDL->vPos);
	m_fMinRadius     = pDL->fMinRadius;
	m_fMaxRadius	 = pDL->fMaxRadius;
	m_fRampUpTime	 = pDL->fRampUpTime;
	m_fMaxTime		 = pDL->fMaxTime;
	m_fMinTime		 = pDL->fMinTime;
	m_fRampDownTime  = pDL->fRampDownTime;
	m_dwFlags		 = pDL->dwFlags;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDynamicLightFX::Reset
//
//	PURPOSE:	Reset the dynamic light
//
// ----------------------------------------------------------------------- //

void CDynamicLightFX::Reset()
{
	m_pClientDE->SetLightColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z);
	m_pClientDE->SetLightRadius(m_hObject, m_fMinRadius);

	m_fStartTime = m_pClientDE->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDynamicLightFX::CreateObject
//
//	PURPOSE:	Create object associated the dynamic light.
//
// ----------------------------------------------------------------------- //

DBOOL CDynamicLightFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	// Allow create object to be called to re-init object...

	if (m_hObject)
	{
		m_pClientDE->SetObjectFlags(m_hObject, m_dwFlags);
		m_pClientDE->SetObjectPos(m_hObject, &m_vPos);
	}
	else
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType = OT_LIGHT;
		createStruct.m_Flags = m_dwFlags;
		VEC_COPY(createStruct.m_Pos, m_vPos);

		m_hObject = m_pClientDE->CreateObject(&createStruct);
		if (!m_hObject) return DFALSE;
	}

	m_pClientDE->SetLightColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z);
	m_pClientDE->SetLightRadius(m_hObject, m_fMinRadius);

	m_fStartTime = m_pClientDE->GetTime();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDynamicLightFX::Update
//
//	PURPOSE:	Update the light
//
// ----------------------------------------------------------------------- //

DBOOL CDynamicLightFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime   = m_pClientDE->GetTime();
	DFLOAT fRadius = m_fMinRadius;

	if (fTime < m_fStartTime + m_fMinTime)
	{
		return DTRUE;
	}
	else if (fTime < m_fStartTime + m_fMinTime + m_fRampUpTime)
	{
		if (m_fRampUpTime > 0.0f)
		{
			DFLOAT fDelta  = fTime - (m_fStartTime + m_fMinTime);
			DFLOAT fOffset = fDelta * (m_fMaxRadius - m_fMinRadius)/m_fRampUpTime;
			fRadius += fOffset;
		}
	}
	else if (fTime < m_fStartTime + m_fMinTime + m_fRampUpTime + m_fMaxTime)
	{
		fRadius = m_fMaxRadius;
	}
	else if (fTime < m_fStartTime + m_fMinTime + m_fRampUpTime + m_fMaxTime + m_fRampDownTime)
	{
		if (m_fRampDownTime > 0.0f)
		{
			DFLOAT fDelta  = fTime - (m_fStartTime + m_fMinTime + m_fRampUpTime + m_fMaxTime);
			DFLOAT fOffset = fDelta * m_fMaxRadius/m_fRampDownTime;
			fRadius = m_fMaxRadius - fOffset;
		}
	}
	else
	{
		return DFALSE;
	}

	m_pClientDE->SetLightRadius(m_hObject, fRadius);

	return DTRUE;
}


//-------------------------------------------------------------------------------------------------
// SFX_DynamicLightFactory
//-------------------------------------------------------------------------------------------------

const SFX_DynamicLightFactory SFX_DynamicLightFactory::registerThis;

CSpecialFX* SFX_DynamicLightFactory::MakeShape() const
{
	return new CDynamicLightFX();
}

