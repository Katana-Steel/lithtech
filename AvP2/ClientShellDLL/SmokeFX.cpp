// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeFX.cpp
//
// PURPOSE : Smoke special FX - Implementation
//
// CREATED : 3/2/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SmokeFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"

extern LTVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Init
//
//	PURPOSE:	Init the smoke trail
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	m_pTextureName	= "SpriteTextures\\SFX\\Particles\\Smoke1c.dtx";

	SMCREATESTRUCT* pSM = (SMCREATESTRUCT*)psfxCreateStruct;

	m_dwFlags = pSM->dwSystemFlags;

	m_vPos					= pSM->vPos;
	m_vColor1				= pSM->vColor1;
	m_vColor2				= pSM->vColor2;
	m_vMinDriftVel			= pSM->vMinDriftVel;
	m_vMaxDriftVel			= pSM->vMaxDriftVel;
	m_fVolumeRadius			= pSM->fVolumeRadius;
	m_fLifeTime				= pSM->fLifeTime;
	m_fRadius				= pSM->fRadius;
	m_fParticleCreateDelta	= pSM->fParticleCreateDelta;
	m_fMinParticleLife		= pSM->fMinParticleLife;
	m_fMaxParticleLife		= pSM->fMaxParticleLife;
	m_nNumParticles			= pSM->nNumParticles;
	m_bIgnoreWind			= pSM->bIgnoreWind;
	m_hstrTexture			= pSM->hstrTexture;
	m_bUpdatePos			= pSM->bUpdatePos;
	m_bRemoveIfNotLiquid	= pSM->bRemoveIfNotLiquid;

	m_fGravity		= 0.0f;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::CreateObject(ILTClient *pClientDE)
{
    if (!pClientDE ) return LTFALSE;

	if (m_hstrTexture)
	{
		m_pTextureName = pClientDE->GetStringData(m_hstrTexture);
	}

    LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if(bRet)
		g_pLTClient->GetObjectPos(m_hObject, &m_vLastPos);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeFX::Update
//
//	PURPOSE:	Update the smoke
//
// ----------------------------------------------------------------------- //

LTBOOL CSmokeFX::Update()
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_fStartTime < 0)
	{
		m_fStartTime = fTime;

		// Make sure we create the first set of particles on this update.

		m_fLastTime = fTime - m_fParticleCreateDelta;
	}


	// Hide/show the particle system if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

        uint32 dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			// Once last puff as disappeared, hide the system (no new puffs
			// will be added...)

			if (dwFlags & FLAG_VISIBLE)
			{
				if (fTime > m_fLastTime + m_fMaxParticleLife)
				{
					m_pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
				}
			}
			else
			{
				m_fLastTime = fTime;
			}

            return LTTRUE;
		}
		else
		{
			m_pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		}

		LTVector vPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		m_pClientDE->SetObjectPos(m_hObject, &vPos);

//		LTRotation rRot;
//		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
//		m_pClientDE->SetObjectRotation(m_hObject, &rRot);

		if(m_bUpdatePos)
		{
			//now get a list of the particles and update their position
			LTParticle *pHead=LTNULL, *pTail=LTNULL;
			g_pLTClient->GetParticles(m_hObject, &pHead, &pTail);

			while(pHead && pHead != pTail)
			{
				//get particle position
				LTVector vPartPos(0,0,0);
				g_pLTClient->GetParticlePos(m_hObject, pHead, &vPartPos);

				//adjust for new pos
				vPartPos -= vPos-m_vLastPos;

				//re-set pos
				g_pLTClient->SetParticlePos(m_hObject, pHead, &vPartPos);

				//incriment
				pHead = pHead->m_pNext;
			}

			//not re-set the last position
			m_vLastPos = vPos;
		}
	}

	// See if we need to pop our bubbles...
	if(m_bRemoveIfNotLiquid)
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


	// Check to see if we should just wait for last smoke puff to go away...

	if(m_fLifeTime != -1.0f)
	{
		if (fTime > m_fStartTime + m_fLifeTime)
		{
			if (fTime > m_fLastTime + m_fMaxParticleLife)
			{
				return LTFALSE;
			}

			return LTTRUE;
		}
	}


	// See if it is time to add some more smoke...

	if (fTime >= m_fLastTime + m_fParticleCreateDelta)
	{
        LTVector vDriftVel, vColor, vPos;

		// Determine how many particles to add...

		int nNumParticles = GetNumParticles(m_nNumParticles);

		// Build the individual smoke puffs...

		for (int j=0; j < nNumParticles; j++)
		{
			VEC_SET(vPos,  GetRandom(-m_fVolumeRadius, m_fVolumeRadius),
					-2.0f, GetRandom(-m_fVolumeRadius, m_fVolumeRadius));

			VEC_SET(vDriftVel,
					GetRandom(m_vMinDriftVel.x, m_vMaxDriftVel.x),
					GetRandom(m_vMinDriftVel.y, m_vMaxDriftVel.y),
					GetRandom(m_vMinDriftVel.z, m_vMaxDriftVel.z));

			if (!m_bIgnoreWind)
			{
				VEC_ADD(vDriftVel, vDriftVel, g_vWorldWindVel);
			}

			GetRandomColorInRange(vColor);

            LTFLOAT fLifeTime = GetRandom(m_fMinParticleLife, m_fMaxParticleLife);

			vDriftVel -= (m_vVel * 0.1f);

			m_pClientDE->AddParticle(m_hObject, &vPos, &vDriftVel, &vColor, fLifeTime);
		}

		m_fLastTime = fTime;
	}


	return CBaseParticleSystemFX::Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleShowerFX::GetPointContainer
//
//	PURPOSE:	Get the point container code...
//
// ----------------------------------------------------------------------- //```

ContainerCode CSmokeFX::GetPointContainer(LTVector &vPos, HOBJECT &hObj)
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
// SFX_SmokeFactory
//-------------------------------------------------------------------------------------------------

const SFX_SmokeFactory SFX_SmokeFactory::registerThis;

CSpecialFX* SFX_SmokeFactory::MakeShape() const
{
	return new CSmokeFX();
}


