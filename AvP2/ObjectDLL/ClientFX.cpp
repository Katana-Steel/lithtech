//------------------------------------------------------------------
//
//   MODULE  : CLIENTFX.CPP
//
//   PURPOSE : Implements class CClientFX
//
//   CREATED : On 1/25/99 At 3:59:19 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "ClientFX.h"
#include "MsgIDs.h"

BEGIN_CLASS(CClientFX)
	ADD_STRINGPROP(FxName, "")
	ADD_BOOLPROP(Loop, TRUE)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(CClientFX, BaseClass, NULL, NULL)

//------------------------------------------------------------------
//
//   FUNCTION : CClientFX()
//
//   PURPOSE  : Standard constructor
//
//------------------------------------------------------------------

CClientFX::CClientFX()
{
	m_bLoop		 = TRUE;
	m_hstrFxName = NULL;
}

//------------------------------------------------------------------
//
//   FUNCTION : CClientFX()
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CClientFX::~CClientFX()
{
	if (m_hstrFxName)
	{
		g_pServerDE->FreeString(m_hstrFxName);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : EngineMessageFn()
//
//   PURPOSE  : LithTech object message handler
//
//------------------------------------------------------------------

DDWORD CClientFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch (messageID)
	{
		case MID_PRECREATE :
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				GenericProp prop;

				g_pServerDE->GetPropGeneric("FxName", &prop);
				if (prop.m_String[0])
				{
					m_hstrFxName = g_pServerDE->CreateString(prop.m_String);
				}

				g_pServerDE->GetPropGeneric("Loop", &prop);
				m_bLoop = prop.m_Bool;
			}
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SendFXMessage();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


//------------------------------------------------------------------
//
//   FUNCTION : SendFXMessage()
//
//   PURPOSE  : Send the special fx message to the client...
//
//------------------------------------------------------------------

void CClientFX::SendFXMessage()
{
	// TEMP!!!
	// Don't create client fx - need to upgrade to use new Sanity code...
	return;


	DVector vPos;
	g_pServerDE->GetObjectPos(m_hObject, &vPos);
		
	HMESSAGEWRITE hWrite = g_pServerDE->StartMessage(DNULL, MID_CLIENTFX_CREATE);
	g_pServerDE->WriteToMessageString(hWrite, g_pServerDE->GetStringData(m_hstrFxName));
	g_pServerDE->WriteToMessageVector(hWrite, &vPos);
	g_pServerDE->WriteToMessageDWord(hWrite, m_bLoop);
	g_pServerDE->EndMessage(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CClientFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrFxName);
	g_pServerDE->WriteToMessageByte(hWrite, m_bLoop);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CClientFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	m_hstrFxName = g_pServerDE->ReadFromMessageHString(hRead);
	m_bLoop		 = (BOOL) g_pServerDE->ReadFromMessageByte(hRead);

	// Send the special fx message to the client(s)...

	SendFXMessage();
}
