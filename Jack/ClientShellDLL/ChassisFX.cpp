// ----------------------------------------------------------------------- //
//
// MODULE  : ChassisFX.cpp
//
// PURPOSE : ChassisFX - Implementation
//
// CREATED : 6/30/03
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ChassisFX.h"
#include "iltclient.h"
#include "SFXMsgIds.h"

ChassisFX::~ChassisFX()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	CHASSISCREATESTRUCT* pCS = (CHASSISCREATESTRUCT*)psfxCreateStruct;

	m_nTeam = pCS->nTeam;
	m_fDropZoneRadius = pCS->fDropZoneRadius;
	m_nActivateId = pCS->nActivateId;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisFX::Update
//
//	PURPOSE:	Update the DoomsdayPiece
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisFX::Update()
{
    if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ChassisFX::OnServerMessage()
//
//	PURPOSE:	We got a message!
//
// ----------------------------------------------------------------------- //

LTBOOL ChassisFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

	m_nTeam = pMsg->Readuint8();
	m_fDropZoneRadius = pMsg->Readfloat( );

	return LTTRUE;
}