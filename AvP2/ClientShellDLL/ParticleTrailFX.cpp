// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleTrailFX.cpp
//
// PURPOSE : ParticleTrail special FX - Implementation
//
// CREATED : 4/27/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleTrailFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ParticleTrailSegmentFX.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "WeaponFXTypes.h"
#include "ContainerCodes.h"
#include "FXButeMgr.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailFX::Init
//
//	PURPOSE:	Init the Particle trail
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	PTCREATESTRUCT pt;

	pt.hServerObj = hServObj;
	pt.nType	  = hRead->ReadByte();

	return Init(&pt);
}

LTBOOL CParticleTrailFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PTCREATESTRUCT* pST = (PTCREATESTRUCT*)psfxCreateStruct;
	m_nType = pST->nType;
	m_nUWType = pST->nUWType;

	g_pLTClient->GetObjectPos(m_hServerObject, &m_vLastPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &m_rLastRot);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleTrailFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleTrailFX::Update()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if(!psfxMgr || !m_pClientDE) return LTFALSE;


	LTFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...
	if(!m_hServerObject || IsWaitingForRemove())
		return LTFALSE;


	// Get the current position and rotation
	LTVector vPos;
	LTRotation rRot;

	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);


	// Setup the initial trail segment data
	PTSCREATESTRUCT pts;
	pts.hServerObj = m_hServerObject;
	pts.nType = m_nType;


	// See if the object is underwater
	HLOCALOBJ objList[1];
	uint32 dwNum = g_pLTClient->GetPointContainers(&vPos, objList, 1);

	if(dwNum > 0 && objList[0])
	{
		uint32 dwUserFlags;
		g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

		if(dwUserFlags & USRFLG_VISIBLE)
		{
			uint16 dwCode;
			if(g_pLTClient->GetContainerCode(objList[0], &dwCode))
			{
				if(IsLiquid((ContainerCode)dwCode))
				{
					pts.nType = m_nUWType;
				}
			}
		}
	}


	// Get the particle trail FX structure data
	PARTICLETRAILFX *pPTFX = g_pFXButeMgr->GetParticleTrailFX(pts.nType);

	if(pPTFX)
	{
		LTVector vDelta = vPos - m_vLastPos;
		LTFLOAT fDeltaMag = vDelta.Mag();


		// If our position has changed enough... create a particle trail segment
		if(fDeltaMag >= pPTFX->fEmitDistance)
		{
	//		uint32 dwUserFlags;
	//		g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

	//		if(dwUserFlags & USRFLG_VISIBLE)
	//		{
				pts.vLastPos = m_vLastPos;
				pts.rLastRot = m_rLastRot;
				pts.vCurPos = vPos;
				pts.rCurRot = rRot;

				// Create this particle trail segment
				CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_PARTICLETRAILSEG_ID, &pts);

				// Let each Particle segment do its initial update...
				if(pFX) pFX->Update();
	//		}
		}
	}


	// Set the last position to our current
	m_vLastPos = vPos;
	m_rLastRot = rRot;


	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_ParticleTrailFactory
//-------------------------------------------------------------------------------------------------

const SFX_ParticleTrailFactory SFX_ParticleTrailFactory::registerThis;

CSpecialFX* SFX_ParticleTrailFactory::MakeShape() const
{
	return new CParticleTrailFX();
}

