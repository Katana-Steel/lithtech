// ----------------------------------------------------------------------- //
//
// MODULE  : RandomSparksFX.cpp
//
// PURPOSE : Sparks special FX - Implementation
//
// CREATED : 1/21/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "RandomSparksFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRandomSparksFX::Init
//
//	PURPOSE:	Init the sparks
//
// ----------------------------------------------------------------------- //

DBOOL CRandomSparksFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	RANDOMSPARKSCREATESTRUCT rs;

	rs.hServerObj = hServObj;
	rs.vPos = hRead->ReadCompPos();
	rs.vDir = hRead->ReadVector();
	rs.nSparks	 = hRead->ReadByte();
	rs.fDuration = hRead->ReadWord();

	return Init(&rs);
}

DBOOL CRandomSparksFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	RANDOMSPARKSCREATESTRUCT* pStruct = (RANDOMSPARKSCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vPos, pStruct->vPos);
	VEC_COPY(m_vDir, pStruct->vDir);
	m_nSparks	= pStruct->nSparks;
	m_fDuration	= pStruct->fDuration;
	m_vMinVelAdjust	= pStruct->vMinVelAdjust;
	m_vMaxVelAdjust = pStruct->vMaxVelAdjust;
	m_fRadius		= pStruct->fRadius;

	m_pTextureName = "Sprites\\SFX\\Impact\\RandomSpark.spr";

	VEC_SET(m_vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(m_vColor2, 255.0f, 255.0f, 255.0f);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRandomSparksFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CRandomSparksFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if (bRet)
	{
		bRet = AddSparks();
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRandomSparksFX::Update
//
//	PURPOSE:	Update the smoke trail (add smoke)
//
// ----------------------------------------------------------------------- //

DBOOL CRandomSparksFX::Update()
{
	if (!CBaseParticleSystemFX::Update()) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (fTime > m_fStartTime + m_fDuration)
	{
		return DFALSE;
	}
	

	// Fade sparks over duration...

	DFLOAT fScale = (m_fDuration - (fTime - m_fStartTime)) / m_fDuration;

	DFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRandomSparksFX::AddSparks
//
//	PURPOSE:	Make the sparks
//
// ----------------------------------------------------------------------- //

DBOOL CRandomSparksFX::AddSparks()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	DVector vMinOffset, vMaxOffset, vMinVel, vMaxVel;
	VEC_SET(vMinOffset, -0.5f, -0.5f, -0.5f);
	VEC_SET(vMaxOffset, 0.5f, 0.5f, 0.5f);

	DFLOAT fVelOffset = VEC_MAG(m_vDir);
	m_vDir.Norm();

	DRotation rRot;
	m_pClientDE->AlignRotation(&rRot, &m_vDir, DNULL);

	DVector vF, vU, vR;
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 0.0f, 1.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	}

	DVector vTemp;

	VEC_MULSCALAR(vMinVel, vF, fVelOffset * m_vMinVelAdjust.y); 
	VEC_MULSCALAR(vMaxVel, vF, fVelOffset * m_vMaxVelAdjust.y); 

	VEC_MULSCALAR(vTemp, vR, -fVelOffset * m_vMinVelAdjust.x); 
	VEC_ADD(vMinVel, vMinVel, vTemp);

	VEC_MULSCALAR(vTemp, vR, fVelOffset * m_vMaxVelAdjust.x); 
	VEC_ADD(vMaxVel, vMaxVel, vTemp);

	VEC_MULSCALAR(vTemp, vU, -fVelOffset * m_vMinVelAdjust.z); 
	VEC_ADD(vMinVel, vMinVel, vTemp);

	VEC_MULSCALAR(vTemp, vU, fVelOffset * m_vMaxVelAdjust.z); 
	VEC_ADD(vMaxVel, vMaxVel, vTemp);

	vMinVel -= GetVelocity();
	vMaxVel -= GetVelocity();

	int nNumSparks = GetNumParticles(m_nSparks);

	m_pClientDE->AddParticles(m_hObject, nNumSparks,
							  &vMinOffset, &vMaxOffset,		// Position offset
							  &vMinVel, &vMaxVel,			// Velocity
							  &m_vColor1, &m_vColor2,		// Color
							  m_fDuration*0.5f, m_fDuration);
	
	m_fStartTime = m_pClientDE->GetTime();

	return DTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_RandomSparksFactory
//-------------------------------------------------------------------------------------------------

const SFX_RandomSparksFactory SFX_RandomSparksFactory::registerThis;

CSpecialFX* SFX_RandomSparksFactory::MakeShape() const
{
	return new CRandomSparksFX();
}

