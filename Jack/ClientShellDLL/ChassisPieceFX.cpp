// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisPiece.cpp
//
// PURPOSE : ChassisPiece - Implementation
//
// CREATED : 5/12/03
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ChassisPieceFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"
#include "HUDMgr.h"
#include "ChassisButeMgr.h"

extern CGameClientShell* g_pGameClientShell;


ChassisPieceFX::~ChassisPieceFX()
{
	ClearBlink();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPieceFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisPieceFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	CHASSISPIECECREATESTRUCT* pCS = (CHASSISPIECECREATESTRUCT*)psfxCreateStruct;

	m_pChassisPieceBute = pCS->pChassisPieceBute;
	m_bCarried = pCS->bCarried;
	m_nTeam = pCS->nTeam;
	m_bPlanted = pCS->bPlanted;
	m_nActivateId = pCS->nActivateId;
	m_bBlinking = pCS->bBlinking;
	m_fAlpha = 1.0f;
	m_fFadeSpeed = -1.0f;

	std::string radarType = m_pChassisPieceBute->m_sRadarType;

	switch (m_nTeam)
	{
	case 0:
		radarType += "_B";
		break;
	case 1:
		radarType += "_R";
		break;
	default:
		radarType += "_N";
		break;
	}

	g_pRadar->AddObject(m_hServerObject,radarType.c_str(), INVALID_TEAM);
	g_pLTClient->SetObjectColor(m_hServerObject,1.0f,1.0f,1.0f,m_fAlpha);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPieceFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisPieceFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPieceFX::Update
//
//	PURPOSE:	Update the DoomsdayPiece
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisPieceFX::Update()
{
    if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

	if (m_bBlinking)
	{
		UpdateBlink();
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisPieceFX::OnServerMessage()
//
//	PURPOSE:	We got a message!
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisPieceFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

	bool bRespawn = pMsg->Readbool();
	if (bRespawn)
	{
		LTVector vOldPos = pMsg->ReadLTVector();
		LTVector vNewPos = pMsg->ReadLTVector();
		m_bCarried = false;
		m_nTeam = INVALID_TEAM;
		m_bPlanted = false;
		Respawn(vOldPos,vNewPos);
	}
	else
	{
		uint8 nIndex = pMsg->Readuint8();
		m_pChassisPieceBute = ChassisButeMgr::Instance( ).GetChassisPieceButeList( )[nIndex];
		m_bCarried = pMsg->Readbool();
		m_nTeam = pMsg->Readuint8();
		m_bPlanted = pMsg->Readbool();
		uint8 nActivate = pMsg->Readuint8();
		bool bBlinking =  pMsg->Readbool();

		if (m_bBlinking != bBlinking)
		{
			if (bBlinking)
				StartBlink();
			else
				ClearBlink();
		}
	}

	std::string radarType = m_pChassisPieceBute->m_sRadarType;

	switch (m_nTeam)
	{
	case 0:
		radarType += "_B";
		break;
	case 1:
		radarType += "_R";
		break;
	default:
		radarType += "_N";
		break;

	}
	g_pRadar->ChangeRadarType(m_hServerObject,radarType.c_str());

	g_pHUDMgr->QueueUpdate(kHUDDoomsday);

	return LTTRUE;
};


void ChassisPieceFX::UpdateBlink()
{
	m_fAlpha += m_fFadeSpeed * g_pLTClient->GetFrameTime();
	if (m_fAlpha > 1.0f)
	{
		m_fAlpha = 1.0f;
		m_fFadeSpeed = -1.0f;
	}
	else if (m_fAlpha < 0.1f)
	{
		m_fAlpha = 0.1f;
		m_fFadeSpeed = 1.0f;
	}

	g_pLTClient->SetObjectColor(m_hServerObject,1.0f,1.0f,m_fAlpha,m_fAlpha);
}

void ChassisPieceFX::StartBlink()
{
	m_fAlpha = 1.0f;
	m_bBlinking = true;
	m_fFadeSpeed = -1.0f;
	g_pLTClient->SetObjectColor(m_hServerObject,1.0f,1.0f,1.0f,m_fAlpha);

	// get the flags for this ClientFX
	uint32 dwFlags = FXFLAG_LOOP | FXFLAG_NOSMOOTHSHUTDOWN;


	
	LTVector vPos;
	// get in initial position for the effect
	g_pLTClient->GetObjectPos( m_hServerObject, &vPos );

	// prepare the create struct
	CLIENTFX_CREATESTRUCT fxInit(m_pChassisPieceBute->m_sFadeFX.c_str(),dwFlags,vPos);

	// create the client fx
	g_pClientFXMgr->CreateClientFX(&m_ClientFXLink,fxInit,true);


	g_pLTClient->SetObjectColor(m_hServerObject, 1.0f, 1.0f, 1.0f, 1.0f);
}

void ChassisPieceFX::ClearBlink()
{
	m_fAlpha = 1.0f;
	m_bBlinking = false;
	m_fFadeSpeed = -1.0f;
	g_pLTClient->SetObjectColor(m_hServerObject,1.0f,1.0f,1.0f,m_fAlpha);

	g_pClientFXMgr->ShutdownClientFX(&m_ClientFXLink);
	g_pLTClient->SetObjectColor(m_hServerObject, 1.0f, 1.0f, 1.0f, 1.0f);
}


void ChassisPieceFX::Respawn(LTVector& vOldPos, LTVector& vNewPos )
{
	ClearBlink();

	// create the unspawn fx
	CLIENTFX_CREATESTRUCT unspawnFxInit(m_pChassisPieceBute->m_sUnspawnFX.c_str(),0,vOldPos);
	CLIENTFX_LINK unspawnFXLink;
	g_pClientFXMgr->CreateClientFX(&unspawnFXLink,unspawnFxInit,true);

	// create the respawn fx
	CLIENTFX_CREATESTRUCT respawnFxInit(m_pChassisPieceBute->m_sSpawnFX.c_str(),0,vNewPos);
	CLIENTFX_LINK respawnFXLink;
	g_pClientFXMgr->CreateClientFX(&respawnFXLink,respawnFxInit,true);

}