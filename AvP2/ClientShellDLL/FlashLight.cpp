 // ----------------------------------------------------------------------- //
//
// MODULE  : FlashLight.cpp
//
// PURPOSE : FlashLight class - Implementation
//
// CREATED : 07/21/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FlashLight.h"
#include "GameClientShell.h"
#include "ClientUtilities.h"
#include "VarTrack.h"
#include "BaseScaleFX.h"
#include "MsgIDs.h"
#include "ClientSoundMgr.h"
#include "SurfaceFunctions.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarFLMinLightRadius;
VarTrack	g_cvarFLMaxLightRadius;
VarTrack	g_cvarFLMaxLightDist;
VarTrack	g_cvarFLFOVDist;
VarTrack	g_cvarFLLightBackOff;
VarTrack	g_cvarFLInterpSpeed;
VarTrack	g_cvarFLSrcLightDist;
VarTrack	g_cvarFLSrcLightRadius;
VarTrack	g_cvarFLFlickerRate;
VarTrack	g_cvarFLBeamMinRadius;
VarTrack	g_cvarFLBeamRadius;
VarTrack	g_cvarFLBeamAlpha;
VarTrack	g_cvarFLBeamUOffset;
VarTrack	g_cvarFLBeamROffset;
VarTrack	g_cvarFLPolyBeam;
VarTrack	g_cvarFLNumSegments;
VarTrack	g_cvarFLMinBeamLen;
VarTrack	g_cvarFLServerUpdateTime;

VarTrack	g_cvarFLMinChargeLevel;

VarTrack	g_cvarFLAngle;


LTBOOL g_nFlicker1[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
LTBOOL g_nFlicker2[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
LTBOOL g_nFlicker3[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
LTBOOL g_nFlicker4[10] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
LTBOOL g_nFlicker5[10] = { 1, 1, 1, 0, 1, 1, 1, 1, 1, 1 };
LTBOOL g_nFlicker6[10] = { 0, 1, 1, 1, 0, 1, 0, 1, 1, 1 };
LTBOOL g_nFlicker7[10] = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 0 };
LTBOOL g_nFlicker8[10] = { 0, 0, 1, 1, 1, 1, 0, 1, 1, 1 };


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::CFlashLight()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlashLight::CFlashLight()
{
	m_hServerObject			= LTNULL;
	m_hLightSocket			= INVALID_MODEL_SOCKET;

    m_bOn					= LTFALSE;
	m_bLocalPlayer			= LTFALSE;

    m_hLight				= LTNULL;
	m_hSrcLight				= LTNULL;
	m_hFlare				= LTNULL;

	m_nFlickerPattern		= 1;
	m_fFlickerTime			= 0.0f;
	m_fLastDist				= 75.0f;

	m_hSound				= LTNULL;

    m_fServerUpdateTimer	= (LTFLOAT)INT_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::~CFlashLight()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFlashLight::~CFlashLight()
{
	// clean up objects
	if (m_hLight)
	{
		g_pLTClient->DeleteObject(m_hLight);
		m_hLight = LTNULL;
	}

	if (m_hSrcLight)
	{
		g_pLTClient->DeleteObject(m_hSrcLight);
		m_hSrcLight = LTNULL;
	}

	if (m_hFlare)
	{
		g_pLTClient->DeleteObject(m_hFlare);
		m_hFlare = LTNULL;
	}

	if(m_hSound)
	{
		g_pLTClient->KillSound(m_hSound);
		m_hSound = LTNULL;
	}

	// clean up string
	if(m_cs.hstrSpriteFile)
		g_pLTClient->FreeString(m_cs.hstrSpriteFile);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Init()
//
//	PURPOSE:	Initializer
//
// ----------------------------------------------------------------------- //

void CFlashLight::Init(HOBJECT hServerObject)
{
	// store and set initial data
	m_hServerObject = hServerObject; 

	if( m_hServerObject )
	{
		const DDWORD dwCFlags = g_pLTClient->GetObjectClientFlags(m_hServerObject);
		g_pLTClient->SetObjectClientFlags(m_hServerObject, dwCFlags | CF_NOTIFYREMOVE);
	}

	m_bLocalPlayer = (m_hServerObject == g_pGameClientShell->GetPlayerMovement()->GetObject());

	g_pModelLT->GetSocket(m_hServerObject, "LeftShoulder", m_hLightSocket);

	if(!g_cvarFLMinLightRadius.IsInitted()) 
		g_cvarFLMinLightRadius.Init(g_pLTClient, "FLMinRadius", NULL, 75.0f);
	if(!g_cvarFLMaxLightRadius.IsInitted()) 
		g_cvarFLMaxLightRadius.Init(g_pLTClient, "FLMaxRadius", NULL, 250.0f);
	if(!g_cvarFLMaxLightDist.IsInitted()) 
		g_cvarFLMaxLightDist.Init(g_pLTClient, "FLMaxDist", NULL, 1000.0f);
	if(!g_cvarFLFOVDist.IsInitted())
		g_cvarFLFOVDist.Init(g_pLTClient, "FLFOVDist", NULL, 125.0f);
	if(!g_cvarFLLightBackOff.IsInitted()) 
		g_cvarFLLightBackOff.Init(g_pLTClient, "FLLightBackOff", NULL, 0.25f);
	if(!g_cvarFLInterpSpeed.IsInitted())
		g_cvarFLInterpSpeed.Init(g_pLTClient, "FLInterpSpeed", NULL, 3000.0f);
	if(!g_cvarFLSrcLightDist.IsInitted())
		g_cvarFLSrcLightDist.Init(g_pLTClient, "FLSrcLightDist", NULL, 25.0f);
	if(!g_cvarFLSrcLightRadius.IsInitted())
		g_cvarFLSrcLightRadius.Init(g_pLTClient, "FLSrcLightRadius", NULL, 100.0f);
	if(!g_cvarFLFlickerRate.IsInitted())
		g_cvarFLFlickerRate.Init(g_pLTClient, "FLFlickerRate", NULL, 0.75f);
	if(!g_cvarFLBeamMinRadius.IsInitted()) 
		g_cvarFLBeamMinRadius.Init(g_pLTClient, "FLBeamMinRadius", NULL, 1.0f);
	if(!g_cvarFLBeamRadius.IsInitted()) 
		g_cvarFLBeamRadius.Init(g_pLTClient, "FLBeamRadius", NULL, 150.0f);
	if(!g_cvarFLBeamAlpha.IsInitted()) 
		g_cvarFLBeamAlpha.Init(g_pLTClient, "FLBeamAlpha", NULL, 0.5f);
	if(!g_cvarFLBeamUOffset.IsInitted()) 
		g_cvarFLBeamUOffset.Init(g_pLTClient, "FLBeamUOffset", NULL, -2.0f);
	if(!g_cvarFLBeamROffset.IsInitted()) 
		g_cvarFLBeamROffset.Init(g_pLTClient, "FLBeamROffset", NULL, -10.0f);
	if(!g_cvarFLNumSegments.IsInitted()) 
		g_cvarFLNumSegments.Init(g_pLTClient, "FLNumSegments", NULL, 1.0f);
	if(!g_cvarFLPolyBeam.IsInitted()) 
		g_cvarFLPolyBeam.Init(g_pLTClient, "FLPolyBeam", NULL, 1.0f);
	if(!g_cvarFLMinBeamLen.IsInitted()) 
		g_cvarFLMinBeamLen.Init(g_pLTClient, "FLMinBeamLen", NULL, 200.0f);
	if(!g_cvarFLServerUpdateTime.IsInitted()) 
		g_cvarFLServerUpdateTime.Init(g_pLTClient, "FLServerUpdateTime", LTNULL, 0.20f);
	if(!g_cvarFLAngle.IsInitted()) 
		g_cvarFLAngle.Init(g_pLTClient, "FLAngle", LTNULL, 5.0f);
	if(!g_cvarFLMinChargeLevel.IsInitted()) 
		g_cvarFLMinChargeLevel.Init(g_pLTClient, "FLMinChargeLevel", LTNULL, 50.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::TurnOn()
//
//	PURPOSE:	Turn light on
//
// ----------------------------------------------------------------------- //

void CFlashLight::TurnOn()
{
	LTBOOL bCanTurnOn = LTTRUE;

	if(m_bLocalPlayer)
		bCanTurnOn = g_pInterfaceMgr->GetPlayerStats()->GetBatteryLevel() >= g_cvarFLMinChargeLevel.GetFloat();

	// see if we have enuf juice to turn on
	if(bCanTurnOn)
	{
		CreateLight();

		//don't do this if we are already on!
		if (m_hLight && m_hSrcLight && !m_bOn)
		{
			m_bOn = LTTRUE;

			uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
			g_pLTClient->SetObjectFlags(m_hLight, dwFlags | FLAG_VISIBLE);

			dwFlags = g_pLTClient->GetObjectFlags(m_hSrcLight);
			g_pLTClient->SetObjectFlags(m_hSrcLight, dwFlags | FLAG_VISIBLE);

			dwFlags = m_LightBeam.GetFlags();
			m_LightBeam.SetFlags(dwFlags | FLAG_VISIBLE);

			if(!g_pGameClientShell->IsMultiplayerGame())
			{
				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SFX_MESSAGE);
				g_pLTClient->WriteToMessageByte(hMessage, SFX_CHARACTER_ID );
				g_pLTClient->WriteToMessageObject(hMessage, m_hServerObject );
				g_pLTClient->WriteToMessageByte(hMessage, CFX_FLASHLIGHT );
				g_pLTClient->WriteToMessageByte(hMessage, FL_ON);
				g_pLTClient->EndMessage(hMessage);
			}

			if (m_hFlare)
			{
				uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
				g_pLTClient->SetObjectFlags(m_hFlare, dwFlags | FLAG_VISIBLE);
			}

			if(m_hSound)
			{
				g_pLTClient->KillSound(m_hSound);
				m_hSound = LTNULL;
			}

			m_hSound = g_pClientSoundMgr->PlaySoundFromObject(	m_hServerObject,
																"sounds\\interface\\flashlight_loop.wav",
																500.0f,
																SOUNDPRIORITY_MISC_HIGH,
																PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);

			if(m_bLocalPlayer && !g_pGameClientShell->IsMultiplayerGame())
			{
				// Send message to server
				HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
				g_pClientDE->WriteToMessageByte(hMessage, CP_FLASHLIGHT);
				g_pClientDE->WriteToMessageByte(hMessage, FL_ON);
				g_pClientDE->EndMessage(hMessage);
			}
		}

	}
	else
	{
		//not enuf juice, play the failed sound
		if(m_bLocalPlayer)
			g_pClientSoundMgr->PlaySoundLocal("failed_on");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::TurnOff()
//
//	PURPOSE:	Turn light off
//
// ----------------------------------------------------------------------- //

void CFlashLight::TurnOff()
{
	// don't turn off if we are already off!
	if (m_bOn)
	{
        m_bOn = LTFALSE;
		uint32 dwFlags=0;

		// hide the light
		if(m_hLight)
		{
			dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
			g_pLTClient->SetObjectFlags(m_hLight, dwFlags & ~FLAG_VISIBLE);
		}

		if(m_hSrcLight)
		{
			dwFlags = g_pLTClient->GetObjectFlags(m_hSrcLight);
			g_pLTClient->SetObjectFlags(m_hSrcLight, dwFlags & ~FLAG_VISIBLE);
		}

		// hide the beam
		dwFlags = m_LightBeam.GetFlags();
		m_LightBeam.SetFlags(dwFlags & ~FLAG_VISIBLE);

		if(!g_pGameClientShell->IsMultiplayerGame())
		{
			// tell the server we are off
			HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SFX_MESSAGE);
			g_pLTClient->WriteToMessageByte(hMessage, SFX_CHARACTER_ID );
			g_pLTClient->WriteToMessageObject(hMessage, m_hServerObject );
			g_pLTClient->WriteToMessageByte(hMessage, CFX_FLASHLIGHT );
			g_pLTClient->WriteToMessageByte(hMessage, FL_OFF);
			g_pLTClient->EndMessage(hMessage);
		}

		// hide the lens flare
		if(m_hFlare)
		{
			uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
			g_pLTClient->SetObjectFlags(m_hFlare, dwFlags & ~FLAG_VISIBLE);
		}


		if(m_hSound)
		{
			g_pLTClient->KillSound(m_hSound);
			m_hSound = LTNULL;
		}


		g_pClientSoundMgr->PlaySoundFromObject(	m_hServerObject,
												"sounds\\interface\\flashlight_off.wav",
												500.0f,
												SOUNDPRIORITY_MISC_HIGH);

		// Send message to server
		if(m_bLocalPlayer && !g_pGameClientShell->IsMultiplayerGame())
		{
			HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pClientDE->WriteToMessageByte(hMessage, CP_FLASHLIGHT);
			g_pClientDE->WriteToMessageByte(hMessage, FL_OFF);
			g_pClientDE->EndMessage(hMessage);
		}


		m_fLastDist = 75.0f;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::CreateLight()
//
//	PURPOSE:	Create the dynamic light
//
// ----------------------------------------------------------------------- //

void CFlashLight::CreateLight()
{
	if (!m_hLight)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType = OT_LIGHT;

		if(m_bLocalPlayer)
			createStruct.m_Flags = FLAG_VISIBLE;
		else
			createStruct.m_Flags = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, createStruct.m_Pos);

		m_hLight = g_pLTClient->CreateObject(&createStruct);
	}

	if (!m_hSrcLight)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType = OT_LIGHT;

		if(m_bLocalPlayer)
			createStruct.m_Flags = FLAG_VISIBLE;
		else
			createStruct.m_Flags = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, createStruct.m_Pos);

		m_hSrcLight = g_pLTClient->CreateObject(&createStruct);
	}

	if(!m_hFlare)
	{
		m_cs.hstrSpriteFile		= g_pLTClient->CreateString("Sprites\\sfx\\flares\\flare02.spr");
		m_cs.bInSkyBox			= LTFALSE;
		m_cs.bCreateSprite		= LTTRUE;
		m_cs.bSpriteOnly		= LTTRUE;
		m_cs.bUseObjectAngle	= LTFALSE;
		m_cs.bSpriteAdditive	= LTTRUE;
		m_cs.fSpriteOffset		= 0.0f;
		m_cs.fMinAngle			= 45.0f;
		m_cs.fMinSpriteAlpha	= 0.0f;
		m_cs.fMaxSpriteAlpha	= 1.0f;
		m_cs.fMinSpriteScale	= 0.0f;
		m_cs.fMaxSpriteScale	= 1.0f;
		m_cs.hServerObj = m_hServerObject;

		// Create the lens flare sprite...
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, createStruct.m_Pos);

		char* pFilename = g_pLTClient->GetStringData(m_cs.hstrSpriteFile);
		SAFE_STRCPY(createStruct.m_Filename, pFilename);
		createStruct.m_ObjectType = OT_SPRITE;
		createStruct.m_Flags = FLAG_NOLIGHT | FLAG_SPRITEBIAS;
		createStruct.m_Flags2 = FLAG2_ADDITIVE;  

		m_hFlare = g_pLTClient->CreateObject(&createStruct);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Update()
//
//	PURPOSE:	Update the flash light
//
// ----------------------------------------------------------------------- //

LTBOOL FLObjListFilterFn(HOBJECT hTest, void *pUserData)
{
	// If it's a world object, consider it a hit regardless of flags.
	if(g_pPhysicsLT->IsWorldObject(hTest) == LT_YES)
		return LTTRUE;

	// Don't collide with non-solid or non-visible objects
	uint32 nFlags;
	g_pLTClient->Common()->GetObjectFlags(hTest, OFT_Flags, nFlags);

	// [KLS] 9/9/01 - Changed so only non-solid objects are ignored.  The idea is
	// if an object is solid and invisible it is probably a blocker brush.  In this
	// case the FLPolyFilterFn will determine if the object should be ignored or not...
	//
	// Ignore non-solid and invisible objects.
	//
	//if(!(nFlags & FLAG_SOLID) || !(nFlags & FLAG_VISIBLE))
	//	return LTFALSE;
	if(!(nFlags & FLAG_SOLID)) return LTFALSE;
		

	// Check against the list of the objects
	HOBJECT *hList = (HOBJECT*)pUserData;
	while(hList && *hList)
	{
		if(hTest == *hList)
			return LTFALSE;

		++hList;
	}


	return LTTRUE;
}

LTBOOL FLPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	if(hPoly == INVALID_HPOLY) return LTTRUE;

	uint32 nSurfFlags;
	g_pLTClient->Common()->GetPolySurfaceFlags(hPoly, nSurfFlags);

	if (!(nSurfFlags & SURF_SOLID)) return LTFALSE;
	
	// [KLS] 9/9/01 - Added checks based on the surface flags.  If we hit
	// an invisible surface it may be a blocker brush that we still wish the
	// flashlight to shine on...
	//
	// Check to see if the surface is a blocker, but one the flashlight
	// shouldn't ignore...
	if (nSurfFlags & SURF_INVISIBLE)
	{
		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
		if (pSurf && pSurf->eType != ST_SKY && pSurf->eType != ST_INVISIBLE)
		{
			return LTTRUE;
		}

		return LTFALSE;
	}

	return LTTRUE;
}

void CFlashLight::Update()
{
	if (!m_bOn || !m_hLight || !m_hSrcLight) return;

	// Calculate light position...

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	HOBJECT hFilterList[] = { hPlayerObj, GetServerObject(), LTNULL };

	LTVector vPos, vEndPos, vUOffset, vROffset, vF, vR;
	GetLightPositions(vPos, vEndPos, vUOffset, vROffset, vF, vR);

	IntersectQuery qInfo;
	IntersectInfo iInfo;

	qInfo.m_From = vPos;
	qInfo.m_To   = vEndPos;

	qInfo.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
	qInfo.m_FilterFn = FLObjListFilterFn;
	qInfo.m_PolyFilterFn = FLPolyFilterFn;
	qInfo.m_pUserData = hFilterList;

	LTFLOAT fSqr, fMagSqr = g_cvarFLMaxLightDist.GetFloat() * g_cvarFLMaxLightDist.GetFloat();
	LTVector vTemp;

	if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
		fSqr = vTemp.MagSqr();

		if(fSqr < fMagSqr)
			fMagSqr = fSqr;
	}

	qInfo.m_To = vEndPos + (vR * g_cvarFLFOVDist.GetFloat());

	if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
		fSqr = vTemp.MagSqr();

		if(fSqr < fMagSqr)
			fMagSqr = fSqr;
	}

	qInfo.m_To = vEndPos - (vR * g_cvarFLFOVDist.GetFloat());

	if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
		fSqr = vTemp.MagSqr();

		if(fSqr < fMagSqr)
			fMagSqr = fSqr;
	}


	// Calculate the light radius.
	LTVector vDir = vEndPos - vPos;
	LTFLOAT fDist = (LTFLOAT)sqrt(fMagSqr);
	vDir.Norm();


	LTFLOAT fInterpAmount = g_cvarFLInterpSpeed.GetFloat() * g_pLTClient->GetFrameTime();

	if(fDist > m_fLastDist)
	{
		LTFLOAT fNewDist = m_fLastDist + fInterpAmount;

		fDist = (fNewDist > fDist) ? fDist : fNewDist;
	}
	else if(fDist < m_fLastDist)
	{
		LTFLOAT fNewDist = m_fLastDist - fInterpAmount;

		fDist = (fNewDist < fDist) ? fDist : fNewDist;
	}

	vEndPos = vPos + (vDir * fDist);
	m_fLastDist = fDist;


    LTFLOAT fLightRadius = g_cvarFLMinLightRadius.GetFloat() +
		((g_cvarFLMaxLightRadius.GetFloat() - g_cvarFLMinLightRadius.GetFloat()) * fDist / g_cvarFLMaxLightDist.GetFloat());


	// Move the light away from its end point so that it will light nearby polies
	// as well.
	LTVector vLightPos = vEndPos;
	LTVector vSrcLightPos = vPos + (vDir * g_cvarFLSrcLightDist.GetFloat());

	if( fLightRadius < fDist )
	{
		vLightPos -= vF * fLightRadius * g_cvarFLLightBackOff.GetFloat();
	}
	else
	{
		vLightPos -= vF * fDist * g_cvarFLLightBackOff.GetFloat();
	}

	g_pLTClient->SetObjectPos(m_hLight, &vLightPos);
	g_pLTClient->SetObjectPos(m_hSrcLight, &vSrcLightPos);

	if(m_hSound)
		g_pLTClient->SetSoundPosition(m_hSound, &vSrcLightPos);

	// Update the Server.
    m_fServerUpdateTimer += g_pLTClient->GetFrameTime();
	if (m_fServerUpdateTimer > g_cvarFLServerUpdateTime.GetFloat())
	{
		m_fServerUpdateTimer = 0.0f;

		if(!g_pGameClientShell->IsMultiplayerGame())
		{
			HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SFX_MESSAGE);
			g_pLTClient->WriteToMessageByte(hMessage, SFX_CHARACTER_ID );
			g_pLTClient->WriteToMessageObject(hMessage, m_hServerObject );
			g_pLTClient->WriteToMessageByte(hMessage, CFX_FLASHLIGHT );
			g_pLTClient->WriteToMessageByte(hMessage, FL_UPDATE);
			g_pLTClient->WriteToMessageCompVector(hMessage, &vEndPos);
			g_pLTClient->EndMessage(hMessage);
		}
	}

	g_pLTClient->SetLightRadius(m_hLight, fLightRadius);
	g_pLTClient->SetLightRadius(m_hSrcLight, g_cvarFLSrcLightRadius.GetFloat());

	LTVector vColor;

	LTFLOAT fFlickerTime = g_pLTClient->GetTime() - m_fFlickerTime;

	if(fFlickerTime > g_cvarFLFlickerRate.GetFloat())
	{
		m_nFlickerPattern = GetRandom(1, 8);
		m_fFlickerTime = g_pLTClient->GetTime();
	}

	LTFLOAT fFlickerScale = fFlickerTime / g_cvarFLFlickerRate.GetFloat();
	int nFlickIndex = (int)(fFlickerScale * 10.0f);

	if(nFlickIndex < 0) nFlickIndex = 0;
	else if(nFlickIndex > 9) nFlickIndex = 9;

	LTBOOL bFlick;

	switch(m_nFlickerPattern)
	{
		case 1:	bFlick = g_nFlicker1[nFlickIndex];	break;
		case 2:	bFlick = g_nFlicker2[nFlickIndex];	break;
		case 3:	bFlick = g_nFlicker3[nFlickIndex];	break;
		case 4:	bFlick = g_nFlicker4[nFlickIndex];	break;
		case 5:	bFlick = g_nFlicker5[nFlickIndex];	break;
		case 6:	bFlick = g_nFlicker6[nFlickIndex];	break;
		case 7:	bFlick = g_nFlicker7[nFlickIndex];	break;
		case 8:	bFlick = g_nFlicker8[nFlickIndex];	break;
	}

	if(bFlick)
		vColor = LTVector(220.0f, 220.0f, 190.0f);
	else
		vColor = LTVector(190.0f, 190.0f, 160.0f);


	LTVector vLightColor = vColor / 255.0f;

	g_pLTClient->SetLightColor(m_hLight, vLightColor.x, vLightColor.y, vLightColor.z);
	g_pLTClient->SetLightColor(m_hSrcLight, vLightColor.x, vLightColor.y, vLightColor.z);


	// Show the light beam...
	LTBOOL bShowBeam = !m_bLocalPlayer || !g_pGameClientShell->IsFirstPerson();

	if (g_cvarFLPolyBeam.GetFloat() > 0 && bShowBeam)
	{
		PLFXCREATESTRUCT pls;
		PLFXCREATESTRUCT pls2;
		
		vPos += vUOffset;
		vPos += vROffset;
		
		pls.pTexture			= "SpriteTextures\\SFX\\Polys\\Beam_1.dtx";
		pls.dwTexAddr			= LTTEXADDR_CLAMP;
		pls.vStartPos			= vPos;
		pls.vEndPos				= vEndPos;
		pls.vInnerColorStart	= vColor;
		pls.vInnerColorEnd		= vColor;
        pls.vOuterColorStart    = vColor;
        pls.vOuterColorEnd      = vColor;
		pls.fAlphaStart			= g_cvarFLBeamAlpha.GetFloat();
		pls.fAlphaEnd			= g_cvarFLBeamAlpha.GetFloat(); 
		pls.fMinWidth			= g_cvarFLBeamMinRadius.GetFloat();
		pls.fMaxWidth			= g_cvarFLBeamRadius.GetFloat();
		pls.fMinDistMult		= 1.0f;
		pls.fMaxDistMult		= 1.0f;
		pls.fLifeTime			= 1.0f;
		pls.fAlphaLifeTime		= 1.0f;
		pls.fPerturb			= 0.0f;
        pls.bAdditive           = LTTRUE;
        pls.bNoZ                = LTTRUE;
		pls.nWidthStyle			= PLWS_CONSTANT;
		pls.nNumSegments		= (int)g_cvarFLNumSegments.GetFloat();

		if(m_bLocalPlayer)
		{
			pls.bAlignUp			= LTTRUE;
		}
		else
		{
			pls.bAlignUsingRot		= LTTRUE;
		}

		LTBOOL bUpdateBeam = LTTRUE;
		
		if (m_LightBeam.HasBeenDrawn())
		{
			// Keep the light beam in the vis list...
			
			m_LightBeam.SetPos(vPos);
			
			// Hide the beam if it is too short...
			
            uint32 dwFlags = m_LightBeam.GetFlags();
			
			if (fDist < g_cvarFLMinBeamLen.GetFloat())
			{
				// Fade alpha out as beam gets shorter to help hide
				// the poly line...
				
				pls.fAlphaStart *= fDist / g_cvarFLMinBeamLen.GetFloat();
				
				if (pls.fAlphaStart < 0.1f)
				{
					dwFlags &= ~FLAG_VISIBLE;
					bUpdateBeam = LTFALSE;
				}
			}
			else
			{
				dwFlags |= FLAG_VISIBLE;
			}
			
			m_LightBeam.SetFlags(dwFlags);
			m_LightBeam.ReInit(&pls);
		}
		else
		{
			m_LightBeam.Init(&pls);
			m_LightBeam.CreateObject(g_pLTClient);
		}
		
		if (bUpdateBeam)
		{
			m_LightBeam.Update();
		}
	}
	else
	{
		// Hide the beam here...
		uint32 dwFlags = m_LightBeam.GetFlags();
		dwFlags &= ~FLAG_VISIBLE;
		m_LightBeam.SetFlags(dwFlags);
	}
	
	if(m_hFlare)
		UpdateFlare(vPos, vF);

	//see how we are doing on juice
	if(m_bLocalPlayer && g_pInterfaceMgr->GetPlayerStats()->GetBatteryLevel() == 0.0f)
	{
		TurnOff();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::UpdateFlare()
//
//	PURPOSE:	Update the lens flare FX...
//
// ----------------------------------------------------------------------- //

void CFlashLight::UpdateFlare(LTVector vFlarePos, LTVector vFlareF)
{
	DDWORD dwFlags = 0;
	
	// Only show the flare if the camera is looking at it...

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	if (!hCamera) return;

	LTVector vCamPos;
	g_pLTClient->GetObjectPos(hCamera, &vCamPos);

	//set the position of the flare
	g_pLTClient->SetObjectPos(m_hFlare, &vFlarePos);

	LTRotation rCamRot;
	LTVector vCamU, vCamR, vCamF;
	g_pLTClient->GetObjectRotation(hCamera, &rCamRot);
	g_pLTClient->GetRotationVectors(&rCamRot, &vCamU, &vCamR, &vCamF);

	LTVector vDir = vFlarePos - vCamPos;
	vDir.Norm();

	DFLOAT fCameraAngle = VEC_DOT(vDir, vCamF);
	fCameraAngle = fCameraAngle < 0.0f ? 0.0f : fCameraAngle;
	fCameraAngle = fCameraAngle > 1.0f ? 1.0f : fCameraAngle;
	fCameraAngle = MATH_RADIANS_TO_DEGREES((LTFLOAT)acos(fCameraAngle));

	DFLOAT fObjectAngle = VEC_DOT(-vDir, vFlareF);
	fObjectAngle = fObjectAngle < 0.0f ? 0.0f : fObjectAngle;
	fObjectAngle = fObjectAngle > 1.0f ? 1.0f : fObjectAngle;
	fObjectAngle = MATH_RADIANS_TO_DEGREES((LTFLOAT)acos(fObjectAngle));

	DFLOAT fMinAngle = m_cs.fMinAngle;

	if (fObjectAngle + fCameraAngle > fMinAngle*2)
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
		g_pLTClient->SetObjectFlags(m_hFlare, dwFlags & ~FLAG_VISIBLE);
	}
	else
	{
		dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);
		g_pLTClient->SetObjectFlags(m_hFlare, dwFlags | FLAG_VISIBLE);

		DFLOAT fVal = 1-(fObjectAngle + fCameraAngle)/(fMinAngle*2); 
	
		// Calculate new alpha...

		DFLOAT fMinAlpha = m_cs.fMinSpriteAlpha;
		DFLOAT fMaxAlpha = m_cs.fMaxSpriteAlpha;
		DFLOAT fAlphaRange = fMaxAlpha - fMinAlpha;

		DFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(m_hFlare, &r, &g, &b, &a);

		a = fMinAlpha + (fVal * fAlphaRange);
		g_pLTClient->SetObjectColor(m_hFlare, r, g, b, a);

		// Calculate new scale...

		DFLOAT fMinScale = m_cs.fMinSpriteScale;
		DFLOAT fMaxScale = m_cs.fMaxSpriteScale;
		DFLOAT fScaleRange = fMaxScale - fMinScale;

		DFLOAT fScale = fMinScale + (fVal * fScaleRange);
		DVector vScale(fScale, fScale, fScale);
		g_pLTClient->SetObjectScale(m_hFlare, &vScale);

		// Don't do any more processing if the alpha is 0...

		if (a < 0.001f) return;

		
		// See if we should make a "bliding" flare, and if so see if the
		// camera is looking directly at the flare...

		if (m_cs.bBlindingFlare)
		{
			DFLOAT fBlindObjAngle = m_cs.fBlindObjectAngle;
			DFLOAT fBlindCamAngle = m_cs.fBlindCameraAngle;
		
			if ((fObjectAngle > (90.0f - fBlindObjAngle)) &&
				(fCameraAngle > (90.0f - fBlindCamAngle)))
			{
				// Update the no-z flare if possible...

				DDWORD dwFlags = g_pLTClient->GetObjectFlags(m_hFlare);

				// Make sure there is a clear path from the flare to the camera...
					
				HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
				HOBJECT hFilterList[] = {hPlayerObj, g_pGameClientShell->GetPlayerMovement()->GetObject(), DNULL};

				IntersectInfo iInfo;
				IntersectQuery qInfo;
				qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
				qInfo.m_FilterFn	= ObjListFilterFn;
				qInfo.m_pUserData	= g_pGameClientShell->IsFirstPerson() ? hFilterList : DNULL;
				qInfo.m_From		= vFlarePos;
				qInfo.m_To			= vCamPos;

				if (g_pInterface->IntersectSegment(&qInfo, &iInfo))
				{
					// Doesn't look quite as good, but will clip on world/objects...

					dwFlags &= ~FLAG_SPRITE_NOZ;
					dwFlags |= FLAG_SPRITEBIAS;
				}
				else
				{
					// Calculate new flare scale...
		
					fVal = (fObjectAngle + fBlindObjAngle - 90.0f)/fBlindObjAngle; 

					DFLOAT fMaxBlindScale = m_cs.fMaxBlindScale;
					DFLOAT fMinBlindScale = m_cs.fMinBlindScale;

					fScaleRange =  fMaxBlindScale - fMinBlindScale;
					fScale = fMinBlindScale + (fVal * fScaleRange);
					vScale.Init(fScale, fScale, fScale);
					g_pLTClient->SetObjectScale(m_hFlare, &vScale);


					// This looks better, but will show through world/objects...

					dwFlags &= ~FLAG_SPRITEBIAS;
					dwFlags |= FLAG_SPRITE_NOZ;
				}
				
				g_pLTClient->SetObjectFlags(m_hFlare, dwFlags);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightPlayer::GetLightPositions()
//
//	PURPOSE:	Get the flash light position and rotation...
//
// ----------------------------------------------------------------------- //

void CFlashLightPlayer::GetLightPositions(LTVector& vStartPos, LTVector& vEndPos, LTVector& vUOffset, LTVector& vROffset, LTVector& vF, LTVector& vR)
{
	vStartPos.Init();
	vEndPos.Init();
	vUOffset.Init();
	vROffset.Init();
	vF.Init();

	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	LTRotation rRot;

	LTransform tf;
	if ( g_pGameClientShell->IsFirstPerson() ) 
	{
		// If we can't use the scoekt, ust the camera position.
		g_pGameClientShell->GetCameraMgr()->GetCameraPos(hCamera, vStartPos, LTTRUE);
	}
	else
	{
		g_pModelLT->GetSocketTransform(g_pLTClient->GetClientObject(), GetSocket(), tf, LTTRUE);
		g_pTransLT->GetPos(tf, vStartPos);
	}


	LTVector vU;
	g_pGameClientShell->GetCameraMgr()->GetCameraRotation(hCamera, rRot, LTTRUE);

	//adjust the viewing angle and get new vectors
	const LTFLOAT fTemp = MATH_DEGREES_TO_RADIANS(g_cvarFLAngle.GetFloat());
	rRot = rRot * LTRotation( (LTFLOAT)sin(fTemp*0.5f), 0.0f, 0.0f, (LTFLOAT) cos(fTemp*0.5f) );

	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	vEndPos = vStartPos + (vF * g_cvarFLMaxLightDist.GetFloat());

	if ( g_pGameClientShell->IsFirstPerson() )
	{
		vROffset = (vR * g_cvarFLBeamROffset.GetFloat());
		vUOffset = (vU * g_cvarFLBeamUOffset.GetFloat());
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightAI::Update()
//
//	PURPOSE:	Update the flash light
//
// ----------------------------------------------------------------------- //

void CFlashLightAI::Update()
{
	if ( !GetServerObject() ) return;

	uint32 dwUsrFlags = 0;
	if ( LT_OK == g_pLTClient->GetObjectUserFlags(GetServerObject(), &dwUsrFlags) && (dwUsrFlags & USRFLG_AI_FLASHLIGHT) )
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}

	CFlashLight::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightAI::GetLightPositions()
//
//	PURPOSE:	Get the flash light position and rotation...
//
// ----------------------------------------------------------------------- //

void CFlashLightAI::GetLightPositions(LTVector& vStartPos, LTVector& vEndPos, LTVector& vUOffset, LTVector& vROffset, LTVector& vF, LTVector& vR)
{
	if ( !GetServerObject() || GetSocket() == INVALID_MODEL_SOCKET) return;

	vStartPos.Init();
	vEndPos.Init();
	vUOffset.Init();
	vROffset.Init();
	vF.Init();

	LTransform tf;

	if ( LT_OK == g_pModelLT->GetSocketTransform(GetServerObject(), GetSocket(), tf, LTTRUE) )
	{
		LTVector vPos;

		if ( LT_OK == g_pTransLT->GetPos(tf, vPos) )
		{
			LTVector vU;
			LTRotation rRot = m_rAimingRot;

			// If the rotation is null, use the socket's rotation.
			if( rRot == LTRotation(0.0f,0.0f,0.0f,0.0f) )
			{
				g_pTransLT->GetRot(tf, rRot);
			}

			// Offset the rotation down a little.
			const LTFLOAT fTemp = MATH_DEGREES_TO_RADIANS(g_cvarFLAngle.GetFloat());
			rRot = rRot * LTRotation( (LTFLOAT)sin(fTemp*0.5f), 0.0f, 0.0f, (LTFLOAT) cos(fTemp*0.5f) );

			if ( LT_OK == g_pMathLT->GetRotationVectors(rRot, vR, vU, vF) )
			{

				vStartPos = vPos + vF*2.0f;
				vEndPos = vPos + vF * g_cvarFLMaxLightDist.GetFloat();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Save()
//
//	PURPOSE:	Save the flashlight info...
//
// ----------------------------------------------------------------------- //

void CFlashLight::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_bOn;
	*hWrite << m_fLastDist;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Load()
//
//	PURPOSE:	Load the flashlight info...
//
// ----------------------------------------------------------------------- //

void CFlashLight::Load(HMESSAGEREAD hRead)
{
	*hRead >> m_bOn;

	if(m_bOn)
	{
		TurnOff();
		TurnOn();
	}

	*hRead >> m_fLastDist;
}