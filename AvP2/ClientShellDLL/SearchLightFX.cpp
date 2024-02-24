// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLightFX.cpp
//
// PURPOSE : SearchLight FX - Implementation
//
// CREATED : 6/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SearchLightFX.h"
#include "GameClientShell.h"
#include "ClientWeaponUtils.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Init
//
//	PURPOSE:	Init the search light
//
// ----------------------------------------------------------------------- //

DBOOL CSearchLightFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return DFALSE;
	if (!hMessage) return DFALSE;

	SEARCHLIGHTCREATESTRUCT sl;

	sl.hServerObj			= hServObj;
	sl.fBeamLength			= g_pClientDE->ReadFromMessageFloat(hMessage);
	sl.fBeamRadius			= g_pClientDE->ReadFromMessageFloat(hMessage);
	sl.fBeamAlpha			= g_pClientDE->ReadFromMessageFloat(hMessage);
	sl.fBeamRotTime			= g_pClientDE->ReadFromMessageFloat(hMessage);
	sl.fLightRadius			= g_pClientDE->ReadFromMessageFloat(hMessage);
	sl.bBeamAdditive		= (DBOOL) g_pClientDE->ReadFromMessageByte(hMessage);
	g_pClientDE->ReadFromMessageVector(hMessage, &(sl.vLightColor));

	sl.lens.InitFromMessage(sl.lens, hMessage);

	// Should be between 0.0 and 1.0f...

	sl.vLightColor /= 255.0f;

	return Init(&sl);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Init
//
//	PURPOSE:	Init the search light fx
//
// ----------------------------------------------------------------------- //

DBOOL CSearchLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	m_cs = *((SEARCHLIGHTCREATESTRUCT*)psfxCreateStruct);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

DBOOL CSearchLightFX::CreateObject(CClientDE* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return DFALSE;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	// Create the beam model...

	m_pClientDE->GetObjectPos(m_hServerObject, &(createStruct.m_Pos));
	createStruct.m_ObjectType = OT_MODEL;

	// GetSpecialFXAttributeString has been removed from ClientButeMgr.  7-7-00.
	CString str = ""; //g_pClientButeMgr->GetSpecialFXAttributeString("SearchBeam");
	if (str.IsEmpty()) return DFALSE;

	SAFE_STRCPY(createStruct.m_Filename, (char *)(LPCSTR)str);

	// GetSpecialFXAttributeString has been removed from ClientButeMgr.  7-7-00.
	str = ""; //g_pClientButeMgr->GetSpecialFXAttributeString("SearchSkin");
	if (!str.IsEmpty())
	{
		SAFE_STRCPY(createStruct.m_SkinName, (char *)(LPCSTR)str);
	}

	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT; 
	
	if (m_cs.bBeamAdditive)
	{
		createStruct.m_Flags2 = FLAG2_ADDITIVE;  
	}

	m_hBeam = m_pClientDE->CreateObject(&createStruct);
	if (!m_hBeam) return DFALSE;

	DFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hBeam, &r, &g, &b, &a);
	r = g = b = 1.0f;
	m_pClientDE->SetObjectColor(m_hBeam, r, g, b, m_cs.fBeamAlpha);


	// Create the dynamic light...

	if (m_cs.fLightRadius > 1.0f)
	{
		createStruct.m_ObjectType = OT_LIGHT;
		createStruct.m_Flags = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

		m_hLight = m_pClientDE->CreateObject(&createStruct);
		if (!m_hLight) return DFALSE;

		m_pClientDE->SetLightColor(m_hLight, m_cs.vLightColor.x, m_cs.vLightColor.y, m_cs.vLightColor.z);
		m_pClientDE->SetLightRadius(m_hLight, m_cs.fLightRadius);
	}


	// Create the lens flare...

	LENSFLARECREATESTRUCT lens = m_cs.lens;

	lens.hServerObj			= m_hServerObject;
	lens.bInSkyBox			= DFALSE;
	lens.bCreateSprite		= DTRUE;
	lens.bSpriteOnly		= DTRUE;
	lens.bUseObjectAngle	= DTRUE;

	if (!m_LensFlare.Init(&lens) || !m_LensFlare.CreateObject(m_pClientDE))
	{
		return DFALSE;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Update
//
//	PURPOSE:	Update the lens flare fx
//
// ----------------------------------------------------------------------- //

DBOOL CSearchLightFX::Update()
{
	if (!m_pClientDE || !m_hServerObject || IsWaitingForRemove() || !m_hBeam) return DFALSE;

	DDWORD dwFlags = 0;

	// Update the lens flare...

	m_LensFlare.Update();

	// Hide/show the fx if necessary...

	if (m_hServerObject)
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))  // Hide fx
		{
			if (m_hBeam)
			{
				dwFlags = m_pClientDE->GetObjectFlags(m_hBeam);
				m_pClientDE->SetObjectFlags(m_hBeam, dwFlags & ~FLAG_VISIBLE);
			}
			if (m_hLight)
			{
				dwFlags = m_pClientDE->GetObjectFlags(m_hLight);
				m_pClientDE->SetObjectFlags(m_hLight, dwFlags & ~FLAG_VISIBLE);
			}

			return DTRUE;
		}
		else  // Make all fx visible
		{
			if (m_hBeam)
			{
				dwFlags = m_pClientDE->GetObjectFlags(m_hBeam);
				m_pClientDE->SetObjectFlags(m_hBeam, dwFlags | FLAG_VISIBLE);
			}
			if (m_hLight)
			{
				dwFlags = m_pClientDE->GetObjectFlags(m_hLight);
				m_pClientDE->SetObjectFlags(m_hLight, dwFlags | FLAG_VISIBLE);
			}
		}
	}

	
	// Update the position/rotation of the beam...

	DVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	DRotation rRot;
	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	DVector vU, vR, vF;
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);


	// See how long to make the beam...

	DVector vDest = vPos + (vF * m_cs.fBeamLength);

	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_FilterFn	= AttackerLiquidFilterFn;
	qInfo.m_pUserData	= m_hServerObject;
	qInfo.m_From		= vPos;
	qInfo.m_To			= vDest;

	if (g_pInterface->IntersectSegment(&qInfo, &iInfo))
	{
		vDest = iInfo.m_Point;
	}

	DVector vDir = vDest - vPos;
	DFLOAT fDistance = vDir.Mag();
	vDir.Norm();
	
	DVector vNewPos = vPos + vDir * fDistance/2.0f;
	m_pClientDE->AlignRotation(&rRot, &vDir, NULL);

	if (m_cs.fBeamRotTime > 0.0f)
	{
		m_fBeamRotation += (360.0f/m_cs.fBeamRotTime * m_pClientDE->GetFrameTime());
		m_fBeamRotation = m_fBeamRotation > 360.0f ? m_fBeamRotation - 360.0f : m_fBeamRotation;
		m_pClientDE->RotateAroundAxis(&rRot, &vDir, MATH_DEGREES_TO_RADIANS(m_fBeamRotation));
	}

	m_pClientDE->SetObjectRotation(m_hBeam, &rRot);
	m_pClientDE->SetObjectPos(m_hBeam, &vNewPos);

	DVector vScale(m_cs.fBeamRadius, m_cs.fBeamRadius, fDistance);
	m_pClientDE->SetObjectScale(m_hBeam, &vScale);


	// Move the dynamic light...

	if (m_hLight)
	{
		vDest -= (vDir * 5.0f);
		m_pClientDE->SetObjectPos(m_hLight, &vDest);
	}

	return DTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_SearchLightFactory
//-------------------------------------------------------------------------------------------------

const SFX_SearchLightFactory SFX_SearchLightFactory::registerThis;

CSpecialFX* SFX_SearchLightFactory::MakeShape() const
{
	return new CSearchLightFX();
}

