// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicleFX.cpp
//
// PURPOSE : Beam special FX - Implementation
//
// CREATED : 6/8/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerVehicleFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "GameClientShell.h"
#include "VehicleButeMgr.h"

#define RADAR_SNOWMOBILE_TYPE	"Snowmobile"

//blinks per second
static float fFadeSpeed = 2.0f;

extern CGameClientShell* g_pGameClientShell;


CPlayerVehicleFX::~CPlayerVehicleFX()
{
	if (m_bBlinking)
	{
		m_bBlinking = false;
		g_pClientFXMgr->ShutdownClientFX(&m_ClientFXLink);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Init
//
//	PURPOSE:	Init the beam fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	PVCREATESTRUCT cs;

	cs.hServerObj = hServObj;
    cs.Read(pMsg);
	
	m_bBlinking = pMsg->Readbool();
	if (m_bBlinking)
		StartBlink();
	else
		ClearBlink();

	return Init(&cs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Init
//
//	PURPOSE:	Init the player vehicle fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PVCREATESTRUCT* pFX = (PVCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pFX;
	
	// Add the snowmobile object to the radar list in coop games only...

	if( IsMultiplayerGame( ) && IsCoopMultiplayerGameType( ) && g_pGameClientShell->ShouldUseRadar() )
	{
		g_pRadar->AddObject( m_hServerObject, RADAR_SNOWMOBILE_TYPE, INVALID_TEAM );
	}

     return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Update()
{
	if( !CSpecialFX::Update() || m_bWantRemove ) 
		return LTFALSE;

	if (m_bBlinking)
	{
		UpdateBlink();
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

	 PlayerVehicleFXMsgs eMsg = (PlayerVehicleFXMsgs)pMsg->Readuint8();
	 switch(eMsg)
	 {
	 case kPlayerVehicleRespawn:
		{
			LTVector vOldPos = pMsg->ReadLTVector();
			LTVector vNewPos = pMsg->ReadLTVector();
		 	Respawn(vOldPos,vNewPos);
		}
		break;
	 case kPlayerVehicleStartFade:
		 StartBlink();
		 break;
	 case kPlayerVehicleStopFade:
		 ClearBlink();
		 break;
	 }

		

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::UpdateBlink
//
//	PURPOSE:	Update the fade in/out
//
// ----------------------------------------------------------------------- //

void CPlayerVehicleFX::UpdateBlink()
{
	if (m_bFadeIn)
	{
		m_fAlpha += g_pLTClient->GetFrameTime() * fFadeSpeed;
		if (m_fAlpha > 1.0f)
		{
			m_fAlpha = 1.0f;
			m_bFadeIn = false;
		}
	}
	else
	{
		m_fAlpha -= g_pLTClient->GetFrameTime() * fFadeSpeed;
		if (m_fAlpha < 0.01f)
		{
			m_fAlpha = 0.01f;
			m_bFadeIn = true;
		}
	}

	g_pLTClient->SetObjectColor(m_hServerObject, 1.0f, m_fAlpha, m_fAlpha, 1.0f);

}

void CPlayerVehicleFX::StartBlink()
{
	m_fAlpha = 1.0f;
	m_bFadeIn = false;
	m_bBlinking = true;

	// get the flags for this ClientFX
	uint32 dwFlags = FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN;

	VEHICLE* pVehicle = g_pVehicleButeMgr->GetVehicle(m_cs.nVehicleId);

	LTVector vPos;
	// get in initial position for the effect
	g_pLTClient->GetObjectPos( m_hServerObject, &vPos );

	// prepare the create struct
	CLIENTFX_CREATESTRUCT fxInit(pVehicle->sFadeFX.c_str(),dwFlags,vPos);

	// create the client fx
	g_pClientFXMgr->CreateClientFX(&m_ClientFXLink,fxInit,true);


	g_pLTClient->SetObjectColor(m_hServerObject, 1.0f, 1.0f, 1.0f, 1.0f);
}

void CPlayerVehicleFX::ClearBlink()
{
	m_fAlpha = 1.0f;
	m_bBlinking = false;
	g_pClientFXMgr->ShutdownClientFX(&m_ClientFXLink);
	g_pLTClient->SetObjectColor(m_hServerObject, 1.0f, 1.0f, 1.0f, 1.0f);
}


void CPlayerVehicleFX::Respawn(LTVector& vOldPos, LTVector& vNewPos )
{
	ClearBlink();

	VEHICLE* pVehicle = g_pVehicleButeMgr->GetVehicle(m_cs.nVehicleId);

	// create the unspawn fx
	CLIENTFX_CREATESTRUCT unspawnFxInit(pVehicle->sUnspawnFX.c_str(),0,vOldPos);
	CLIENTFX_LINK unspawnFXLink;
	g_pClientFXMgr->CreateClientFX(&unspawnFXLink,unspawnFxInit,true);

	// create the respawn fx
	CLIENTFX_CREATESTRUCT respawnFxInit(pVehicle->sSpawnFX.c_str(),0,vNewPos);
	CLIENTFX_LINK respawnFXLink;
	g_pClientFXMgr->CreateClientFX(&respawnFXLink,respawnFxInit,true);

}