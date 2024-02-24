// ----------------------------------------------------------------------- //
//
// MODULE  : Camera.cpp
//
// PURPOSE : Camera implementation
//
// CREATED : 5/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Camera.h"
#include "iltserver.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"

Camera::CameraList Camera::m_sCameraList;

LTBOOL IsExternalCameraActive() 
{
	if( !g_pLTServer )
		return LTFALSE;

	for( Camera::CameraIterator iter = Camera::BeginCameras();
		  iter != Camera::EndCameras(); ++iter )
	{
		if( USRFLG_CAMERA_LIVE & g_pLTServer->GetObjectUserFlags( (*iter)->m_hObject ) )
			return LTTRUE;
	}

	return LTFALSE;
}

BEGIN_CLASS(Camera)
	ADD_CAMERA_PROPERTIES(0)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(Camera, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Camera()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Camera::Camera() : BaseClass(OT_NORMAL)
{
    m_bAllowPlayerMovement  = LTFALSE;
	m_fActiveTime			= 1.0f;
    m_bOneTime              = LTTRUE;
	m_bWidescreen			= LTFALSE;
	m_nXFOV					= 75;
	m_nYFOV					= 60;
    m_bStartActive          = LTFALSE;
	m_fTurnOffTime			= 0.0f;
    m_bIsListener           = LTFALSE;

	m_sCameraList.push_back(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Camera()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Camera::~Camera()
{
	CameraList::iterator iter = std::find(m_sCameraList.begin(), m_sCameraList.end(), this);

	_ASSERT( iter != m_sCameraList.end() );

	if( iter != m_sCameraList.end() )
	{
		m_sCameraList.erase(iter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Camera::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
				pStruct->m_Flags |= FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES;
			}

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
                ReadProps(LTFALSE);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Camera::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
            HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
            g_pLTServer->FreeString(hMsg);
			break;
		}
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Camera::ReadProps(LTBOOL bCreateSFXMsg)
{
	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("AllowPlayerMovement", &genProp) == LT_OK)
	{
		m_bAllowPlayerMovement = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("ActiveTime", &genProp) == LT_OK)
	{
		m_fActiveTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("OneTime", &genProp) == LT_OK)
	{
		m_bOneTime = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Widescreen", &genProp) == LT_OK)
	{
        m_bWidescreen = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("XFOV", &genProp) == LT_OK)
	{
        m_nXFOV = (uint8)genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("YFOV", &genProp) == LT_OK)
	{
        m_nYFOV = (uint8)genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("StartActive", &genProp) == LT_OK)
	{
		m_bStartActive = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("IsListener", &genProp) == LT_OK)
	{
		m_bIsListener = genProp.m_Bool;
	}

	if (bCreateSFXMsg)
	{
		CreateSFXMsg();
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::InitialUpdate
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Camera::InitialUpdate(int nInfo)
{
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	if (m_bStartActive)
	{
		TurnOn();
	}

	// Tell the clients about us...

	CreateSFXMsg();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::CreateSFXMsg
//
//	PURPOSE:	Initialize the object
//
// ----------------------------------------------------------------------- //

void Camera::CreateSFXMsg()
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_CAMERA_ID);
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_bAllowPlayerMovement);
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_bWidescreen);
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_nXFOV);
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_nYFOV);
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_bIsListener);
    g_pLTServer->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::TriggerMsg()
//
//	PURPOSE:	Trigger function to turn camera on/off
//
// --------------------------------------------------------------------------- //

void Camera::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
    char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg) return;

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	if (_stricmp(pMsg, "ON") == 0)
	{
		TurnOn();
	}
	else if (_stricmp(pMsg, "OFF") == 0)
	{
		TurnOff();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Update()
//
//	PURPOSE:	Update the camera...
//
// --------------------------------------------------------------------------- //

void Camera::Update()
{
	// Tell all de-active objects I can see to become active...

// {MD 4/30/99}
//  g_pLTServer->PingObjects(m_hObject);

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	if(dwUsrFlags & USRFLG_CAMERA_LIVE)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
		g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME,objArray);

		int numObjects = objArray.NumObjects();

		for (int i = 0; i < numObjects; i++ )
		{
			CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));
			if (pPlayer)
			{
				HCLIENT hClient = pPlayer->GetClient();
				if (hClient)
				{
					g_pLTServer->SetClientViewPos(hClient, &vPos);
				}
			}
		}
	}

    if (m_fActiveTime > 0.0f && g_pLTServer->GetTime() > m_fTurnOffTime)
	{
		TurnOff();
	}
	else
	{
        g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::TurnOff()
//
//	PURPOSE:	Turn camera off
//
// --------------------------------------------------------------------------- //

void Camera::TurnOff()
{
    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_CAMERA_LIVE);
    g_pLTServer->SetNextUpdate(m_hObject, 0.0f);

	if (m_bOneTime)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::TurnOn()
//
//	PURPOSE:	Turn camera on
//
// --------------------------------------------------------------------------- //

void Camera::TurnOn()
{
    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUsrFlags |= USRFLG_CAMERA_LIVE;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);

	if (m_fActiveTime > 0.0f)
	{
        m_fTurnOffTime = g_pLTServer->GetTime() + m_fActiveTime;
	}

    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Camera::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_bIsListener);
    g_pLTServer->WriteToMessageByte(hWrite, m_bAllowPlayerMovement);
    g_pLTServer->WriteToMessageByte(hWrite, m_bOneTime);
    g_pLTServer->WriteToMessageByte(hWrite, m_bWidescreen);
    g_pLTServer->WriteToMessageByte(hWrite, m_nXFOV);
    g_pLTServer->WriteToMessageByte(hWrite, m_nYFOV);
    g_pLTServer->WriteToMessageByte(hWrite, m_bStartActive);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fActiveTime);
    g_pLTServer->WriteToMessageFloat(hWrite, (m_fTurnOffTime  - g_pLTServer->GetTime()) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Camera::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Camera::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_bIsListener           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bAllowPlayerMovement  = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bOneTime              = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bWidescreen           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_nXFOV                 = g_pLTServer->ReadFromMessageByte(hRead);
    m_nYFOV                 = g_pLTServer->ReadFromMessageByte(hRead);
    m_bStartActive          = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_fActiveTime           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fTurnOffTime          = g_pLTServer->ReadFromMessageFloat(hRead);
	m_fTurnOffTime += g_pLTServer->GetTime();
}
