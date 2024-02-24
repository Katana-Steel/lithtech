// ----------------------------------------------------------------------- //
//
// MODULE  : CClientWeaponSFX.cpp
//
// PURPOSE : CClientWeaponSFX - Implementation
//
// CREATED : 1/17/98
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponMgr.h"
#include "ClientWeaponSFX.h"
#include "SFXMsgIds.h"
#include "SurfaceFunctions.h"
#include "WeaponFXTypes.h"
#include "ServerMark.h"
#include "CommonUtilities.h"
#include "FXButeMgr.h"
#include "GameServerShell.h"

static void CreateServerMark(const CLIENTWEAPONFX & theStruct);
static void CreateServerExitMark(const CLIENTWEAPONFX & theStruct);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateClientWeaponFX
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CreateClientWeaponFX(CLIENTWEAPONFX & theStruct)
{
	if (!g_pLTServer) return;

	// If this is a moveable object, set the flags of fx to ignore
	// marks and smoke...

	if (IsMoveable(theStruct.hObj))
	{
		theStruct.wIgnoreFX |= WFX_MARK;	

		// Create a server-side mark if applicable...

		if ( !g_pGameServerShell->GetGameType().IsMultiplayer() && CanMarkObject(theStruct.hObj) )
		{
			AMMO* pAmmo = g_pWeaponMgr->GetAmmo(theStruct.nAmmoId);
			if (pAmmo)
			{
				if (pAmmo->pImpactFX)
				{
					if (WFX_MARK & pAmmo->pImpactFX->nFlags)
					{
						CreateServerMark((const CLIENTWEAPONFX)theStruct);
					}
				}

				// Create an exit mark if applicable...

				if (pAmmo->pFireFX)
				{
					if (WFX_EXITMARK & pAmmo->pFireFX->nFlags)
					{
						CreateServerExitMark((const CLIENTWEAPONFX)theStruct);
					}
				}
			}
		}
	}


	// DEBUG DEBUG
//	g_pLTServer->CPrint("FirePos: %.2f %.2f %.2f,  Pos: %.2f %.2f %.2f", theStruct.vFirePos.x, theStruct.vFirePos.y, theStruct.vFirePos.z, theStruct.vPos.x, theStruct.vPos.y, theStruct.vPos.z);


	// Tell all the clients who can see this fx about the fx...

	HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&(theStruct.vPos));
	g_pLTServer->WriteToMessageByte(hMessage, SFX_WEAPON_ID);
	g_pLTServer->WriteToMessageObject(hMessage, theStruct.hFiredFrom);
	g_pLTServer->WriteToMessageByte(hMessage, theStruct.nWeaponId);
	g_pLTServer->WriteToMessageByte(hMessage, theStruct.nBarrelId);
	g_pLTServer->WriteToMessageByte(hMessage, theStruct.nAmmoId);
	g_pLTServer->WriteToMessageByte(hMessage, theStruct.nSurfaceType);
	g_pLTServer->WriteToMessageByte(hMessage, theStruct.nFlashSocket);
	g_pLTServer->WriteToMessageWord(hMessage, theStruct.wIgnoreFX);
	g_pLTServer->WriteToMessageByte(hMessage, theStruct.nShooterId);
	g_pLTServer->WriteToMessageVector(hMessage, &(theStruct.vFirePos));
	g_pLTServer->WriteToMessageVector(hMessage, &(theStruct.vPos));
	g_pLTServer->WriteToMessageVector(hMessage, &(theStruct.vSurfaceNormal));
	g_pLTServer->WriteToMessageObject(hMessage, theStruct.hObj);
	// This doesn't always work correctly...
	//g_pLTServer->WriteToMessageCompPosition(hMessage, &(theStruct.vFirePos));
	//g_pLTServer->WriteToMessageCompPosition(hMessage, &(theStruct.vPos));
	//g_pLTServer->WriteToMessageCompPosition(hMessage, &(theStruct.vSurfaceNormal));
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateServerMark
//
//	PURPOSE:	Create a server side mark
//
// ----------------------------------------------------------------------- //

static void CreateServerMark(const CLIENTWEAPONFX & theStruct)
{
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(theStruct.nAmmoId);
	if (!pAmmo) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);


	DFLOAT fScaleAdjust = 1.0f;
	if (!GetImpactSprite((SurfaceType)theStruct.nSurfaceType, fScaleAdjust, 
		theStruct.nAmmoId, createStruct.m_Filename, ARRAY_LEN(createStruct.m_Filename)))
	{
		return;
	}

#ifdef _DEBUG
	strcpy(createStruct.m_Name, createStruct.m_Filename );
#endif

	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE;
	createStruct.m_Pos = theStruct.vPos;

	g_pLTServer->AlignRotation(&(createStruct.m_Rotation), &((DVector)theStruct.vSurfaceNormal), DNULL);


	HCLASS hClass = g_pLTServer->GetClass("CServerMark");
	CServerMark* pMark = (CServerMark*) g_pLTServer->CreateObject(hClass, &createStruct);
	if (!pMark) return;

	// Randomly adjust the mark's scale to add a bit o spice...

	if (pAmmo->pImpactFX)
	{
		DFLOAT fScale = fScaleAdjust * pAmmo->pImpactFX->fMarkScale;
	
		DVector vScale;
		VEC_SET(vScale, fScale, fScale, fScale);
		g_pLTServer->ScaleObject(pMark->m_hObject, &vScale);
	}

#ifdef __LINUX
	pMark->Setup(const_cast<CLIENTWEAPONFX &>(theStruct));
#else
	pMark->Setup((CLIENTWEAPONFX)theStruct);
#endif
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateExitMark
//
//	PURPOSE:	Create a server side exit mark
//
// ----------------------------------------------------------------------- //

static void CreateServerExitMark(const CLIENTWEAPONFX & theStruct)
{
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface((SurfaceType)theStruct.nSurfaceType);
	if (!pSurf || !pSurf->bCanShootThrough) return;

	int nMaxThickness = pSurf->nMaxShootThroughThickness;
	if (nMaxThickness < 1) return;

	// Determine if there is an "exit" surface...

	IntersectQuery qInfo;
	IntersectInfo iInfo;
	memset(&iInfo, 0, sizeof(iInfo));

	DVector vDir = theStruct.vPos - theStruct.vFirePos;
	vDir.Norm();

	qInfo.m_From = theStruct.vPos + (vDir * (DFLOAT)(nMaxThickness + 1));
	qInfo.m_To   = theStruct.vPos;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	SurfaceType eType = ST_UNKNOWN;

	if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
	{
		eType = GetSurfaceType(iInfo);
		if (ShowsMark(eType))
		{
			DRotation rNormRot;
			g_pLTServer->AlignRotation(&rNormRot, &(iInfo.m_Plane.m_Normal), DNULL);

			CLIENTWEAPONFX exitStruct = theStruct;
			exitStruct.vPos = iInfo.m_Point + vDir;
			exitStruct.vSurfaceNormal = iInfo.m_Plane.m_Normal;
				
			CreateServerMark((const CLIENTWEAPONFX)exitStruct);
		}
	}			
}
