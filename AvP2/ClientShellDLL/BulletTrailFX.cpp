// ----------------------------------------------------------------------- //
//
// MODULE  : BulletTrailFX.cpp
//
// PURPOSE : SmokeTrail segment special FX - Implementation
//
// CREATED : 3/6/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BulletTrailFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"

#define MAX_TRAIL_LENGTH 3000.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBulletTrailFX::Init
//
//	PURPOSE:	Init the bullet trail 
//
// ----------------------------------------------------------------------- //

DBOOL CBulletTrailFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	BTCREATESTRUCT* pBT = (BTCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vStartPos, pBT->vStartPos);
	VEC_COPY(m_vDir, pBT->vDir);
	VEC_COPY(m_vColor1, pBT->vColor1);
	VEC_COPY(m_vColor2, pBT->vColor2);
	m_fLifeTime		= pBT->fLifeTime;
	m_fFadeTime		= pBT->fFadeTime;
	m_fRadius		= pBT->fRadius;
	m_fGravity		= pBT->fGravity;
	m_fNumParticles	= pBT->fNumParticles;

	m_pTextureName	= DEFAULT_BUBBLE_TEXTURE;

	m_fDistance				= MAX_TRAIL_LENGTH;
	m_fDistTraveled			= 0.0f;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBulletTrailFX::Update
//
//	PURPOSE:	Update the bullet trail (add bubbles)
//
// ----------------------------------------------------------------------- //

DBOOL CBulletTrailFX::Update()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		// See if we can figure out what color bubbles to make, based on the
		// container we start in...

		HLOCALOBJ objList[1];
		DDWORD dwNum = m_pClientDE->GetPointContainers(&m_vStartPos, objList, 1);

		if (dwNum > 0 && objList[0])
		{
			DDWORD dwUserFlags;
			m_pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);

			if (dwUserFlags & USRFLG_VISIBLE)
			{
				D_WORD dwCode;
				if (m_pClientDE->GetContainerCode(objList[0], &dwCode))
				{
					GetLiquidColorRange((ContainerCode)dwCode, &m_vColor1, &m_vColor2);
				}
			}
		}


		// Move the particle system to the correct position...

		m_pClientDE->SetObjectPos(m_hObject, &m_vStartPos);
		
		m_bFirstUpdate = DFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;

		VEC_INIT(m_vLastPos);

		// Find the end position...

		ClientIntersectQuery iQuery;
		ClientIntersectInfo  iInfo;

		DVector vTemp, vEndPoint;

		VEC_MULSCALAR(vTemp, m_vDir, MAX_TRAIL_LENGTH);
		VEC_ADD(vEndPoint, m_vStartPos, vTemp);

		VEC_COPY(iQuery.m_From, m_vStartPos);
		VEC_COPY(iQuery.m_To, vEndPoint);

		if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
		{		
			VEC_SUB(vEndPoint, iInfo.m_Point, m_vStartPos);
			m_fDistance = VEC_MAG(vEndPoint);
		}

		if (m_fDistance <= 0.0f || m_fFadeTime <= 0.0f) return DFALSE;

		// Calculate the trail velocity...

		m_fTrailVel = m_fDistance / m_fFadeTime;

		VEC_MULSCALAR(m_vDir, m_vDir, m_fTrailVel);
	}



	// Check to see if we should just wait for last bubble to go away...

	if (fTime > m_fStartTime + m_fFadeTime)
	{
		if (fTime > m_fLastTime + m_fLifeTime)
		{
			return DFALSE;
		}

		DFLOAT fScale = (m_fLifeTime - (fTime - m_fLastTime)) / m_fLifeTime;

		// m_pClientDE->SetParticleSystemColorScale(m_hObject, fScale);
		DFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);
		
		return DTRUE;
	}


	// Create the necessary particles...


	DFLOAT fTimeOffset = m_pClientDE->GetFrameTime();



	// Calculate distance traveled this frame...

	DFLOAT fDist = m_fTrailVel * fTimeOffset;
	if (fDist > m_fDistance) fDist = m_fDistance;
	
	m_fDistTraveled += fDist;
	if (m_fDistTraveled > m_fDistance)
	{
		fDist = m_fDistance - (m_fDistTraveled - fDist);
		if (fDist <= 0.0f) return DTRUE;
	}


	// Calculate number of particles to create...

	DFLOAT fNumParticles = fDist * m_fNumParticles / m_fDistance;


	// Calculate starting bubble position...

	DVector vCurPos, vPos, vDelta, vTemp, vDriftVel, vColor;

	VEC_MULSCALAR(vTemp, m_vDir, fTimeOffset);
	VEC_ADD(vCurPos, m_vLastPos, vTemp);


	// What is the range of colors?

	DFLOAT fRange = m_vColor2.x - m_vColor1.x;

	
	// Fill the distance between the last projectile position, and it's 
	// current position with bubbles...

	VEC_SUB(vTemp, vCurPos, m_vLastPos);
	VEC_MULSCALAR(vDelta, vTemp, 1.0f/fNumParticles);

	VEC_COPY(vPos, m_vLastPos);

	DFLOAT fLifeTime = 100.0f;

	DFLOAT fOffset = 0.0f;
	DVector vDriftOffset;
	VEC_SET(vDriftOffset, 0.0f, 0.0f, 0.0f);

	int nNumParticles = GetNumParticles((int)fNumParticles);

	for (int i=0; i < nNumParticles; i++)
	{
		// Build the individual bubbless...

		for (int j=0; j < 1; j++)
		{
			VEC_COPY(vTemp, vPos);

			VEC_SET(vDriftVel, 0.0f, GetRandom(5.0f, 6.0f), 0.0f);

			vTemp.x += GetRandom(-fOffset, fOffset);
			vTemp.y += GetRandom(-fOffset, fOffset);
			vTemp.z += GetRandom(-fOffset, fOffset);

			GetRandomColorInRange(vColor);

			m_pClientDE->AddParticle(m_hObject, &vTemp, &vDriftVel, &vColor, fLifeTime);
		}

		VEC_ADD(vPos, vPos, vDelta);
	}

	VEC_COPY(m_vLastPos, vCurPos);
	m_fLastTime = fTime;

	return DTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_BulletTrailFactory
//-------------------------------------------------------------------------------------------------

const SFX_BulletTrailFactory SFX_BulletTrailFactory::registerThis;

CSpecialFX* SFX_BulletTrailFactory::MakeShape() const
{
	return new CBulletTrailFX();
}

