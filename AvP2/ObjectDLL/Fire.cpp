// ----------------------------------------------------------------------- //
//
// MODULE  : Fire.cpp
//
// PURPOSE : Fire - Implementation
//
// CREATED : 5/6/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Fire.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(Fire)
	ADD_STRINGPROP_FLAG(FireObjFX, "", PF_STATICLIST)
    ADD_BOOLPROP(StartOn, LTTRUE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Fire, CClientSFX, NULL, NULL, 0, CFirePlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Fire
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

Fire::Fire() : CClientSFX()
{
    m_bOn           = LTTRUE;
	m_nFireObjFXId	= -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Fire
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Fire::~Fire()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Fire::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
			InitialUpdate((int)fData);
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
//	ROUTINE:	Fire::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Fire::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	Fire::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Fire::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

	g_pFXButeMgr->ReadFireObjFXProp("FireObjFX", m_nFireObjFXId);

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bOn = genProp.m_Bool;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

void Fire::InitialUpdate(int nInfo)
{
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    uint32 dwUserFlags = m_bOn ? USRFLG_VISIBLE : 0;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Tell the clients about the Fire...

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_FIRE_ID);
	g_pLTServer->WriteToMessageByte(hMessage, m_nFireObjFXId);
    g_pLTServer->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::HandleMsg()
//
//	PURPOSE:	Handle trigger messages
//
// --------------------------------------------------------------------------- //

void Fire::HandleMsg(HOBJECT hSender, HSTRING hMsg)
{
    char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg) return;

	if (_stricmp(pMsg, "ON") == 0 && !m_bOn)
	{
        uint32 dwUserFlags = USRFLG_VISIBLE;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTTRUE;
	}
	else if (_stricmp(pMsg, "OFF") == 0 && m_bOn)
	{
        uint32 dwUserFlags = 0;
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
        m_bOn = LTFALSE;
	}
	else if (_stricmp(pMsg, "REMOVE") == 0)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Fire::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    *hWrite << m_bOn;
	*hWrite << m_nFireObjFXId;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Fire::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Fire::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    *hRead >> m_bOn;
	*hRead >> m_nFireObjFXId;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFirePlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT	CFirePlugin::PreHook_EditStringList(const char* szRezPath, 
												 const char* szPropName, 
												 char* const * aszStrings, 
												 uint32* pcStrings, 
												 const uint32 cMaxStrings, 
												 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_stricmp("FireObjFX", szPropName) == 0)
	{
		m_FXButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_FXButeMgrPlugin.PopulateFireFXStringList(aszStrings, pcStrings, 
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
