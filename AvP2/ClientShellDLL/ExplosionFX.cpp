// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 12/29/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ExplosionFX.h"
#include "FXButeMgr.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "WeaponFXTypes.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the particle system fx
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
	if (!CSpecialFX::Init(hServObj, hMessage)) return DFALSE;
	if (!hMessage) return DFALSE;

	EXPLOSIONCREATESTRUCT cs;

	cs.hServerObj = hServObj;
	cs.Read(g_pLTClient, hMessage);

	return Init(&cs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	// Set up our creation struct...

	EXPLOSIONCREATESTRUCT* pCS = (EXPLOSIONCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pCS;

	IMPACTFX *pImpactFX = g_pFXButeMgr->GetImpactFX(m_cs.nImpactFX);
	if (pImpactFX)
	{
		if(pImpactFX->nFlags & WFX_3RDPERSONONLY)
		{
			if(m_cs.hFiredFrom == g_pLTClient->GetClientObject() && g_pGameClientShell->IsFirstPerson())
				return LTFALSE;
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::CreateObject
//
//	PURPOSE:	Create object associated with the explosion
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	// Create the specified explosion...

	IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX(m_cs.nImpactFX);
	if (pImpactFX)
	{
		// Determine what surface we're on????

		SurfaceType eSurfaceType = ST_UNKNOWN;


		// Determine what container we're in...

		ContainerCode eCode;
		HLOCALOBJ objList[1];
		DVector vTestPos = m_cs.vPos;
		DDWORD dwNum = g_pClientDE->GetPointContainers(&vTestPos, objList, 1);

		if (dwNum > 0 && objList[0])
		{
			DDWORD dwUserFlags;
			g_pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);

			if (dwUserFlags & USRFLG_VISIBLE)
			{
				D_WORD dwCode;
				if (g_pClientDE->GetContainerCode(objList[0], &dwCode))
				{
					eCode = (ContainerCode)dwCode;
				}
			}
		}

		// Figure out what surface normal to use...

		DVector vU, vR, vF;
		g_pClientDE->GetRotationVectors(&(m_cs.rRot), &vU, &vR, &vF);
		

		IFXCS cs;
		cs.eCode		= eCode;
		cs.eSurfType	= eSurfaceType;
		cs.rSurfRot		= m_cs.rRot;
		cs.vDir.Init(0, 0, 0);
		cs.vPos			= m_cs.vPos;
		cs.vSurfNormal	= vF;
		cs.fTintRange	= m_cs.fDamageRadius * 5.0f;
		cs.bPlaySound	= DTRUE;

		g_pFXButeMgr->CreateImpactFX(pImpactFX, cs);
	}

	return DFALSE;
}


//-------------------------------------------------------------------------------------------------
// SFX_ExplosionFactory
//-------------------------------------------------------------------------------------------------

const SFX_ExplosionFactory SFX_ExplosionFactory::registerThis;

CSpecialFX* SFX_ExplosionFactory::MakeShape() const
{
	return new CExplosionFX();
}

