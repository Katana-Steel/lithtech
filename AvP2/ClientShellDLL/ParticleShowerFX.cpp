// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleShower.cpp
//
// PURPOSE : ParticleShower special FX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleShowerFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "FXButeMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleShowerFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
	if (!hMessage) return LTFALSE;

	PARTICLESHOWERCREATESTRUCT ps;

	ps.hServerObj = hServObj;

	//get the FX index
    uint8 nFXIndex = g_pLTClient->ReadFromMessageByte(hMessage);

	CPShowerFX*	pPShowerFX = g_pFXButeMgr->GetPShowerFX((int)nFXIndex);

	if(!pPShowerFX) return LTFALSE;

	LTVector vPos, vVel;
	g_pLTClient->Physics()->GetVelocity(hServObj, &vVel);
	g_pLTClient->GetObjectPos(hServObj, &vPos);

	//normalize velocity
	vVel.Norm();


	ps.vPos				= vPos + (-vVel * pPShowerFX->fDirOffset);
	ps.vDir				= (-vVel * GetRandom(pPShowerFX->fMinVel, pPShowerFX->fMaxVel));
	ps.vColor1			= pPShowerFX->vColor1;
	ps.vColor2			= pPShowerFX->vColor2;
	ps.pTexture			= pPShowerFX->szTexture;
	ps.nParticles		= GetRandom(pPShowerFX->nMinParticles, pPShowerFX->nMaxParticles);
	ps.fDuration		= GetRandom(pPShowerFX->fMinDuration, pPShowerFX->fMaxDuration);
    ps.fFadeTime		= pPShowerFX->fFadeTime;
	ps.fEmissionRadius	= pPShowerFX->fEmissionRadius;
	ps.fRadius			= pPShowerFX->fRadius;
	ps.fGravity			= pPShowerFX->fGravity;
	ps.bAdditive		= pPShowerFX->bAdditive;
	ps.bMultiply		= pPShowerFX->bMultiply;

	return Init(&ps);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::Init
//
//	PURPOSE:	Init the sparks
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleShowerFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *(PARTICLESHOWERCREATESTRUCT*)psfxCreateStruct;

	m_vColor1		= m_cs.vColor1;
	m_vColor2		= m_cs.vColor2;
	m_fGravity		= m_cs.fGravity;
	m_fRadius		= m_cs.fRadius;
	m_vPos			= m_cs.vPos;
	m_pTextureName	= m_cs.pTexture;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleShowerFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return LTFALSE;

	if (CBaseParticleSystemFX::CreateObject(pClientDE))
	{
		return AddParticles();
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::Update
//
//	PURPOSE:	Update the particle shower
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleShowerFX::Update()
{
	if (!m_hObject || !g_pLTClient) return LTFALSE;

	DFLOAT fTime = g_pLTClient->GetTime();

	// Check to see if we should go away...

	if (fTime > m_fStartTime + m_cs.fDuration + m_cs.fFadeTime)
	{
		return LTFALSE;
	}
	

	if (fTime > m_fStartTime + m_cs.fDuration)
	{
		//time to fade
        LTFLOAT fScale = 1 - (fTime-m_fStartTime-m_cs.fDuration) / m_cs.fFadeTime;



		if(m_cs.bAdditive)
		{
			LTParticle *pHead;
			LTParticle *pTail;
			g_pLTClient->GetParticles(m_hObject, &pHead, &pTail);
			do
			{
				if (pHead)
				{
					pHead->m_Color.x *= fScale;
					pHead->m_Color.y *= fScale;
					pHead->m_Color.z *= fScale;
					
					pHead = pHead->m_pNext;
				}
			}while (pHead && (pHead != pTail) );

		}
		else
		{
			LTFLOAT r, g, b, a;
			m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
			m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);
		}


	}

	// See if we need to pop our bubbles...
	if(m_cs.bRemoveIfNotLiquid)
	{
		LTVector	vPos, vSysPos;

		// Get the base system pos
		g_pLTClient->GetObjectPos(m_hObject, &vSysPos);

		HOBJECT	hContainer = LTNULL;
		ContainerCode cc = GetPointContainer(vSysPos, hContainer);

		if(!IsLiquid(cc))
			return LTFALSE;

		if(hContainer)
		{
			LTVector vWaterDims, vWaterPos;
			g_pLTClient->GetObjectPos(hContainer, &vWaterPos);
			g_pLTClient->Physics()->GetObjectDims(hContainer, &vWaterDims);

			LTParticle *pHead;
			LTParticle *pTail;
			g_pLTClient->GetParticles(m_hObject, &pHead, &pTail);
			do
			{
				if (pHead)
				{
					// Get our position and see if it is in liquid...
					g_pLTClient->GetParticlePos(m_hObject, pHead, &vPos);

					if((vPos+vSysPos).y > (vWaterPos+vWaterDims).y)
						pHead->m_Size = 0.0f;

					pHead = pHead->m_pNext;
				}
			}while (pHead && (pHead != pTail) );
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::AddParticles
//
//	PURPOSE:	Make the particles
//
// ----------------------------------------------------------------------- //```

LTBOOL CParticleShowerFX::AddParticles()
{
	if(!m_hObject || !g_pLTClient) return LTFALSE;

	LTVector vMinOffset, vMaxOffset, vMinVel, vMaxVel;
	VEC_SET(vMinOffset, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius, -m_cs.fEmissionRadius);
	VEC_SET(vMaxOffset, m_cs.fEmissionRadius, m_cs.fEmissionRadius, m_cs.fEmissionRadius);

	DFLOAT fVelOffset = m_cs.vDir.Mag();
	m_cs.vDir.Norm();

	LTRotation rRot;
	g_pLTClient->AlignRotation(&rRot, &(m_cs.vDir), LTNULL);

	LTVector vF, vU, vR;
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

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

	LTVector vTemp;

	vMinVel = vF * (fVelOffset * .025f); 
	vMaxVel = vF * fVelOffset; 

	vMinVel += vR * -fVelOffset;
	vMaxVel += vR * fVelOffset;
	vMinVel += vU * -fVelOffset;
	vMaxVel += vU * fVelOffset;

	int nParticles = GetNumParticles(m_cs.nParticles);

	g_pLTClient->AddParticles(m_hObject, nParticles,
							  &vMinOffset, &vMaxOffset,		// Position offset
							  &vMinVel, &vMaxVel,			// Velocity
							  &m_vColor1, &m_vColor2,		// Color
							  m_cs.fDuration+m_cs.fFadeTime, m_cs.fDuration+m_cs.fFadeTime);


	// Go through and set random sizes
	LTParticle *pHead = LTNULL, *pTail = LTNULL;
	
	if(g_pLTClient->GetParticles(m_hObject, &pHead, &pTail))
	{
		while(pHead && pHead != pTail)
		{
			pHead->m_Size = GetRandom(m_cs.fMinSize, m_cs.fMaxSize);
			pHead = pHead->m_pNext;
		}
	}


	m_fStartTime = g_pLTClient->GetTime();

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::GetPointContainer
//
//	PURPOSE:	Get the point container code...
//
// ----------------------------------------------------------------------- //```

ContainerCode CParticleShowerFX::GetPointContainer(LTVector &vPos, HOBJECT &hObj)
{
	HLOCALOBJ objList[1];
	uint32 dwNum = g_pLTClient->GetPointContainers(&vPos, objList, 1);
	
	if (dwNum > 0 && objList[0])
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);
		
		if (dwUserFlags & USRFLG_VISIBLE)
		{
            uint16 dwCode;
            if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				hObj = objList[0];
				return (ContainerCode)dwCode;
			}
		}
	}

	return CC_NO_CONTAINER;
}

//-------------------------------------------------------------------------------------------------
// SFX_ParticleShowerFactory
//-------------------------------------------------------------------------------------------------

const SFX_ParticleShowerFactory SFX_ParticleShowerFactory::registerThis;

CSpecialFX* SFX_ParticleShowerFactory::MakeShape() const
{
	return new CParticleShowerFX();
}

