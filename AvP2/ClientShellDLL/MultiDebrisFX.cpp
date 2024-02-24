// ----------------------------------------------------------------------- //
//
// MODULE  : MultiDebrisFX.cpp
//
// PURPOSE : Debris - Implementation
//
// CREATED : 4/24/1
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebrisFX.h"
#include "MultiDebrisFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMultiDebrisFX::Init
//
//	PURPOSE:	Init the multi-debris FX
//
// ----------------------------------------------------------------------- //

DBOOL CMultiDebrisFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	DEBRISCREATESTRUCT info;

	// Load up the base structure
	info.hServerObj = hServObj;
    g_pLTClient->ReadFromMessageRotation(hRead, &info.rRot);
    g_pLTClient->ReadFromMessageVector(hRead, &info.vPos);
    info.nDebrisId = g_pLTClient->ReadFromMessageByte(hRead);
    g_pLTClient->ReadFromMessageVector(hRead, &info.vVelOffset);

	MULTI_DEBRIS* pDebris = g_pDebrisMgr->GetMultiDebris(info.nDebrisId);

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if(!psfxMgr) return LTFALSE;

	if(pDebris)
	{
		// Loop thru all the effects and create them all
		for(int i=0; i < pDebris->nDebrisFX; i++)
		{
			DEBRIS* pFX = g_pDebrisMgr->GetDebris(pDebris->szDebrisFX[i].szString);
			if(pFX)
			{
				info.nDebrisId = pFX->nId;
				psfxMgr->CreateSFX(SFX_DEBRIS_ID, &info);
			} 
		}
	}

	// We are done so no point in sticking around
	return LTFALSE;
}

//-------------------------------------------------------------------------------------------------
// SFX_DebrisFactory
//-------------------------------------------------------------------------------------------------

const SFX_MultiDebrisFactory SFX_MultiDebrisFactory::registerThis;

CSpecialFX* SFX_MultiDebrisFactory::MakeShape() const
{
	return new CMultiDebrisFX();
}


