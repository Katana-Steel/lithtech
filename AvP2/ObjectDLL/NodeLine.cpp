// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLine.cpp
//
// PURPOSE : Implementation of node line visualization debugging thingy
//
// CREATED : 2/11/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "NodeLine.h"
#include "MsgIDs.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"

BEGIN_CLASS(NodeLine)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(NodeLine, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NodeLine::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD NodeLine::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (g_pServerDE) g_pServerDE->SetNextUpdate(m_hObject, 0.01f);
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NodeLine::Setup()
//
//	PURPOSE:	Sends data down to client
//
// ----------------------------------------------------------------------- //

void NodeLine::Setup(const DVector& vSource, const DVector& vDestination)
{
	HMESSAGEWRITE hMessage = g_pServerDE->StartSpecialEffectMessage(this);
	g_pServerDE->WriteToMessageByte(hMessage, SFX_NODELINES_ID);
	g_pServerDE->WriteToMessageVector(hMessage, (DVector*)&vSource);
	g_pServerDE->WriteToMessageVector(hMessage, (DVector*)&vDestination);
	g_pServerDE->EndMessage(hMessage);
}
