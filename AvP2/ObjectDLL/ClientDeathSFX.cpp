// ----------------------------------------------------------------------- //
//
// MODULE  : CClientDeathSFX.cpp
//
// PURPOSE : CClientDeathSFX - Implementation
//
// CREATED : 6/15/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientDeathSFX.h"
#include "SFXMsgIds.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CreateClientDeathFX
//
//	PURPOSE:	Send message to client with data
//
// ----------------------------------------------------------------------- //

void CreateClientDeathFX(CLIENTDEATHFX & fxStruct)
{
	if (!g_pServerDE) return;

	// Tell the clients about the fx...

	HMESSAGEWRITE hMessage = g_pServerDE->StartInstantSpecialEffectMessage(&(fxStruct.vPos));
	g_pServerDE->WriteToMessageByte(hMessage, SFX_DEATH_ID);
	g_pServerDE->WriteToMessageDWord(hMessage, fxStruct.nCharacterSet);
	g_pServerDE->WriteToMessageByte(hMessage, fxStruct.nDeathType);
	g_pServerDE->WriteToMessageVector(hMessage, &(fxStruct.vPos));
	g_pServerDE->WriteToMessageVector(hMessage, &(fxStruct.vDeathDir));
	g_pServerDE->EndMessage(hMessage);
}
