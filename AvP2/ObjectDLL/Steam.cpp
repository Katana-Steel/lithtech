// ----------------------------------------------------------------------- //
//
// MODULE  : Steam.cpp
//
// PURPOSE : Steam implementation
//
// CREATED : 10/19/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Steam.h"
#include "SFXMsgIds.h"
#include "ObjectMsgs.h"
#include "Destructible.h"

BEGIN_CLASS(Steam)
	ADD_STRINGPROP_FLAG(SteamFXName, "", PF_STATICLIST)
	ADD_REALPROP_FLAG(Lifetime, -1.0f, 0)
    ADD_BOOLPROP_FLAG(StartActive, LTFALSE, 0)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Steam, GameBase, NULL, NULL, 0, CSteamPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Steam()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Steam::Steam() : GameBase(OT_NORMAL)
{
	m_fDamageRange	= 0.0f;
	m_fDamage		= 0.0f;
	m_nSteamFXId	= -1;
	m_fLifetime		= -1.0f;
    m_bStartActive  = LTFALSE;
    m_bOn           = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::~Steam()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Steam::~Steam()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Steam::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES;
			}

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProps();
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Steam::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
            HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
            g_pLTServer->FreeString(hMsg);
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void Steam::ReadProps()
{
	GenericProp genProp;

	g_pFXButeMgr->ReadSteamFXProp("SteamFXName", m_nSteamFXId);

    if (g_pLTServer->GetPropGeneric("Lifetime", &genProp) == LT_OK)
	{
		m_fLifetime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("StartActive", &genProp) == LT_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::InitialUpdate
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Steam::InitialUpdate()
{
	// Tell the clients about us...

	STEAMFX *pSteam = g_pFXButeMgr->GetSteamFX(m_nSteamFXId);

	if(pSteam)
	{
		m_fDamageRange = pSteam->fDamageDist;
		m_fDamage = pSteam->fDamage;
	}

	CreateSFXMsg();

	if (m_bStartActive)
	{
		TurnOn();
	}
	else
	{
		SetNextUpdate(0.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Setup
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Steam::Setup(STEAMCREATESTRUCT* pSC, LTFLOAT fLifetime, LTBOOL bStartActive)
{
	if (!pSC) return;

	m_bStartActive	= bStartActive;
	m_fLifetime		= fLifetime;

	// Make sure we reset any necessary values...

	InitialUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Update
//
//	PURPOSE:	Update the damage
//
// ----------------------------------------------------------------------- //

void Steam::Update()
{
	if(m_fDamage > 0.0f)
	{
		// Do damage...
        DoDamage(m_fDamage * g_pLTServer->GetFrameTime());
	}

	// See if we are timed...

	if(m_fLifetime > 0.0f)
	{
		if(m_Timer.Stopped())
		{
			TurnOff();
			return;
		}
	}

	SetNextUpdate(0.001f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::DoDamage
//
//	PURPOSE:	Damage objects (if necessary)
//
// ----------------------------------------------------------------------- //

inline LTBOOL FilterFn_CharactersOnly(HOBJECT hTest, void *pUserData)
{
	// Test the user flags to see if this is a character
	uint32 nUserFlags;
	nUserFlags = g_pLTServer->GetObjectUserFlags(hTest);

	if(nUserFlags & USRFLG_CHARACTER)
		return LTTRUE;

	return LTFALSE;
}

void Steam::DoDamage(LTFLOAT fDamageAmount)
{
	if(fDamageAmount <= 0.0f || m_fDamageRange <= 0.0f)
		return;

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

    LTVector vU, vR, vF;
    g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// See if there is an object to damage...

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From		= vPos;
	IQuery.m_To			= vPos + (vF * m_fDamageRange);
	IQuery.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn	= FilterFn_CharactersOnly;

    if(g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if(IInfo.m_hObject)
		{
			DamageStruct damage;

			damage.eType	= DT_BURN;
			damage.fDamage	= fDamageAmount;
			damage.hDamager = m_hObject;
			damage.vDir		= vF;

			damage.DoDamage(this, IInfo.m_hObject);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::CreateSFXMsg
//
//	PURPOSE:	Create our special fx message
//
// ----------------------------------------------------------------------- //

void Steam::CreateSFXMsg()
{
	STEAMFX *pSteam = g_pFXButeMgr->GetSteamFX(m_nSteamFXId);

	if(pSteam)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_STEAM_ID);
		g_pLTServer->WriteToMessageByte(hMessage, pSteam->nSmokeFX);
		g_pLTServer->WriteToMessageByte(hMessage, pSteam->nSoundFX);
		g_pLTServer->EndMessage(hMessage);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::TriggerMsg()
//
//	PURPOSE:	Trigger function to turn Steam on/off
//
// --------------------------------------------------------------------------- //

void Steam::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
    char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg) return;

	if (_stricmp(pMsg, "ON") == 0)
	{
		TurnOn();
	}
	else if (_stricmp(pMsg, "OFF") == 0)
	{
		TurnOff();
	}
	else if (_stricmp(pMsg, "REMOVE") == 0)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::TurnOff()
//
//	PURPOSE:	Turn Steam off
//
// --------------------------------------------------------------------------- //

void Steam::TurnOff()
{
	if (!m_bOn) return;

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);
	SetNextUpdate(0.0f);

    m_bOn = LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::TurnOn()
//
//	PURPOSE:	Turn Steam on
//
// --------------------------------------------------------------------------- //

void Steam::TurnOn()
{
	if (m_bOn) return;

	uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUsrFlags |= USRFLG_VISIBLE;
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);

	// If we do damage, make sure we do updates...

	if (m_fDamage > 0.0f || m_fLifetime > 0.0f)
	{
		if (m_fLifetime > 0.0f)
		{
			m_Timer.Start(m_fLifetime);
		}

		SetNextUpdate(0.001f, 1.001f);
	}
	else
	{
		SetNextUpdate(0.0f);
	}

    m_bOn = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Steam::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	g_pLTServer->WriteToMessageByte(hWrite, m_nSteamFXId);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDamageRange);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fLifetime);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartActive);
    g_pLTServer->WriteToMessageByte(hWrite, m_bOn);

	m_Timer.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Steam::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Steam::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	m_nSteamFXId	= g_pLTServer->ReadFromMessageByte(hRead);
    m_fDamageRange  = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fDamage       = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fLifetime     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bStartActive  = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bOn           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);

	m_Timer.Load(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSteamPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT	CSteamPlugin::PreHook_EditStringList(const char* szRezPath, 
												 const char* szPropName, 
												 char* const * aszStrings, 
												 uint32* pcStrings, 
												 const uint32 cMaxStrings, 
												 const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_stricmp("SteamFXName", szPropName) == 0)
	{
		m_FXButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if (!m_FXButeMgrPlugin.PopulateSteamFXStringList(aszStrings, pcStrings, 
			 cMaxStrings, cMaxStringLength)) return LT_UNSUPPORTED;

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
