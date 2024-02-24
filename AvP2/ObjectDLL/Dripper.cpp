// ----------------------------------------------------------------------- //
//
// MODULE  : Dripper.cpp
//
// PURPOSE : Dripper - Implementation
//
// CREATED : 1/5/01
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Dripper.h"
#include "ObjectMsgs.h"
#include "SFXMsgIds.h"
#include "MsgIDs.h"

BEGIN_CLASS(Dripper)
    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 64.0f, 16.0f, PF_DIMS) 
	ADD_STRINGPROP_FLAG(DripEffect, "None", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Dripper, CClientSFX, NULL, NULL, 0, CDripperPlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::Dripper
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Dripper::Dripper() : CClientSFX()
{
	m_bFirstUpdate = LTTRUE;
    m_bOn = LTTRUE;

	m_vDims.Init();
	m_nDripEffect = -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::Dripper
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Dripper::~Dripper()
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Dripper::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct *)pData;
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pStruct);
			}

			pStruct->m_Flags = FLAG_FULLPOSITIONRES | FLAG_FORCECLIENTUPDATE;

			break;
		}

		case MID_INITIALUPDATE:
		{
			InitialUpdate();
			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return CClientSFX::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Dripper::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
            HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			HandleMsg(hSender, hMsg);
            g_pLTServer->FreeString(hMsg);
		}
		break;

		default : break;
	}

	return CClientSFX::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Dripper::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

    if(g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
		m_bOn = genProp.m_Bool;

    if(g_pLTServer->GetPropGeneric("Dims", &genProp) == LT_OK)
		m_vDims = genProp.m_Vec;

	if(g_pLTServer->GetPropGeneric("DripEffect", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
		{
			DRIPPEROBJFX *pFX = g_pFXButeMgr->GetDripperObjFX(genProp.m_String);
			m_nDripEffect = pFX ? pFX->nId : -1;
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::InitialUpdate
//
//	PURPOSE:	Handle first update
//
// ----------------------------------------------------------------------- //

void Dripper::InitialUpdate()
{
	// Turn the effect on or off
	uint32 dwUserFlags = m_bOn ? USRFLG_VISIBLE : 0;
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);


	// Set the dimensions of the object
	g_pLTServer->Physics()->SetObjectDims(m_hObject, &m_vDims, 0);


	// Tell the clients about the effect
	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);

	g_pLTServer->WriteToMessageByte(hMessage, SFX_DRIPPER_ID);
	g_pLTServer->WriteToMessageByte(hMessage, m_nDripEffect);
	g_pLTServer->WriteToMessageVector(hMessage, &m_vDims);
	g_pLTServer->EndMessage(hMessage);

	SetNextUpdate(0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::Update
//
//	PURPOSE:	Handle first update
//
// ----------------------------------------------------------------------- //

void Dripper::Update()
{

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void Dripper::HandleMsg(HOBJECT hSender, HSTRING hMsg)
{
    char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg) return;

	if (_stricmp(pMsg, "ON") == 0 && !m_bOn)
	{
        uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_VISIBLE);
        m_bOn = LTTRUE;
	}
	else if (_stricmp(pMsg, "OFF") == 0 && m_bOn)
	{
        uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_VISIBLE);
        m_bOn = LTFALSE;
	}
	else if (_stricmp(pMsg, "REMOVE") == 0)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Dripper::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite,m_bFirstUpdate);
	g_pLTServer->WriteToMessageByte(hWrite, m_bOn);
	g_pLTServer->WriteToMessageVector(hWrite, &m_vDims);
	g_pLTServer->WriteToMessageByte(hWrite, m_nDripEffect);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Dripper::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Dripper::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	m_bFirstUpdate = (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
    m_bOn = (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
	g_pLTServer->ReadFromMessageVector(hRead, &m_vDims);
	m_nDripEffect = g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDripperPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT	CDripperPlugin::PreHook_EditStringList(const char* szRezPath, 
												 const char* szPropName, 
												 char* const * aszStrings, 
												 uint32* pcStrings, 
												 const uint32 cMaxStrings, 
												 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if(_stricmp("DripEffect", szPropName) == 0)
	{
		m_FXButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if(!m_FXButeMgrPlugin.PopulateDripperFXStringList(aszStrings, pcStrings, 
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

