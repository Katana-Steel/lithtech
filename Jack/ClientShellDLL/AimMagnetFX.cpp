// ----------------------------------------------------------------------- //
//
// MODULE  : AimMagnetFX.cpp
//
// PURPOSE : AimMagnet - Implementation
//
// CREATED : 3/11/03
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AimMagnetFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"

extern CGameClientShell* g_pGameClientShell;


CAimMagnetFX::~CAimMagnetFX()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAimMagnetFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CAimMagnetFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	AIMMAGNETCREATESTRUCT* pAMCS = (AIMMAGNETCREATESTRUCT*)psfxCreateStruct;

	m_nTeamId = pAMCS->m_nTeamId;
	m_hTarget = pAMCS->m_hTarget;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAimMagnetFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CAimMagnetFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAimMagnetFX::Update
//
//	PURPOSE:	Update the AimMagnet
//
// ----------------------------------------------------------------------- //

LTBOOL CAimMagnetFX::Update()
{
    if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAimMagnetFX::OnServerMessage
//
//	PURPOSE:	Handle recieving a message from the server...
//
// ----------------------------------------------------------------------- //

LTBOOL CAimMagnetFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return LTFALSE;

	m_nTeamId = pMsg->Readuint8( );
	m_hTarget = pMsg->ReadObject();

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAimMagnetFX::GetTarget
//
//	PURPOSE:	Get the handle of the target
//
// ----------------------------------------------------------------------- //

HOBJECT CAimMagnetFX::GetTarget() const
{
	if (m_hTarget)
		return m_hTarget;

	return GetServerObj();
}

