// ----------------------------------------------------------------------- //
//
// MODULE  : Alarm.cpp
//
// PURPOSE : Alarm object implementation
//
// CREATED : 12/07/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "Alarm.h"
#include "cpp_engineobjects_de.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "SoundMgr.h"
#include <stdio.h>
#include "AIUtils.h"  // for operator<<(ILTMessage &, std::string &)

BEGIN_CLASS(Alarm)

	ADD_BOOLPROP_FLAG(PlayerUsable, LTTRUE, 0)
	ADD_STRINGPROP_FLAG(ActivateTarget, LTFALSE, PF_OBJECTLINK)
	ADD_STRINGPROP(ActivateMessage, LTFALSE)
	ADD_STRINGPROP_FLAG(DeactivateTarget, LTFALSE, PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG_HELP(DeactivateMessage, LTFALSE, 0, "Sent to DeactivateTarget when Alarm is shut off")

#ifdef _WIN32
	ADD_STRINGPROP_FLAG( AlarmSound, "Sounds\\Events\\ALARM.wav", PF_FILENAME )
#else
	ADD_STRINGPROP_FLAG( AlarmSound, "Sounds/Events/ALARM.wav", PF_FILENAME )
#endif

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(Alarm, BaseClass, LTNULL, LTNULL)

//static char szEnvironmentMap[] = "EnvironmentMap";

static HSOUNDDE PlayAlarm(const std::string & strSoundName)
{
	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT(playSoundInfo);

	if (!strSoundName.empty())
	{
		SAFE_STRCPY(playSoundInfo.m_szSoundName, strSoundName.c_str());
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOCAL | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
		playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_MEDIUM;
		if (g_pServerDE->PlaySound(&playSoundInfo) == LT_OK)
		{
			return playSoundInfo.m_hSound;
		}
	}

	return LTNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Alarm()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

Alarm::Alarm()
: m_hActiveSound(LTNULL),
  m_bPlayerUsable(LTFALSE) {}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// --------------------------------------------------------------------------- //

DDWORD Alarm::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ReadProps();
		}
		break;

		case MID_INITIALUPDATE:
		{
			g_pServerDE->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ReadProps()
//
//	PURPOSE:	Read in properties
//
// --------------------------------------------------------------------------- //

void Alarm::ReadProps()
{
	GenericProp genProp;

	if ( g_pServerDE->GetPropGeneric("PlayerUsable", &genProp ) == DE_OK )
		m_bPlayerUsable = genProp.m_Bool;

	if ( g_pServerDE->GetPropGeneric( "ActivateTarget", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_strActivateTarget = genProp.m_String;

	if ( g_pServerDE->GetPropGeneric( "ActivateMessage", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_strActivateMessage = genProp.m_String;

	if ( g_pServerDE->GetPropGeneric( "DeactivateTarget", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_strDeactivateTarget = genProp.m_String;

	if ( g_pServerDE->GetPropGeneric( "DeactivateMessage", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_strDeactivateMessage = genProp.m_String;


	if ( g_pServerDE->GetPropGeneric( "AlarmSound", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_strAlarmSound = genProp.m_String;

	return;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

DDWORD Alarm::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : LTNULL;

			if ( szMsg )
			{
				if ( m_bPlayerUsable || g_pServerDE->GetObjectClass(hSender) != g_pServerDE->GetClass("CPlayerObj") )
				{
					if (stricmp(szMsg, "ON") == 0)
					{
						Activate();
					}
					if (stricmp(szMsg, "OFF") == 0)
					{
						Deactivate();
					}
				}
			}

			g_pServerDE->FreeString(hMsg);

			break;
		}

		default:
			break;
	}
	
	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

void Alarm::Activate()
{
	// Already activated?
	if (g_pGameServerShell->GetAlarm())
		return;

	g_pGameServerShell->SetAlarm(LTTRUE);

	// Alarm sound will NOT override an existing one
	if (!m_hActiveSound)
	{
		m_hActiveSound = PlayAlarm(m_strAlarmSound);
	}

	// We were triggered: activate Alarm and send our message
	if ( !m_strActivateTarget.empty() && !m_strActivateMessage.empty() )
	{
		SendTriggerMsgToObjects(this, m_strActivateTarget.c_str(), m_strActivateMessage.c_str());
	}

	return;
}

void Alarm::Deactivate()
{
	g_pGameServerShell->SetAlarm(LTFALSE);

	if (m_hActiveSound)
	{
		if (g_pLTServer->KillSound(m_hActiveSound) == LT_OK)
		{
			m_hActiveSound = LTNULL;
		}
	}

	if ( !m_strDeactivateTarget.empty() && !m_strDeactivateMessage.empty() )
	{
		SendTriggerMsgToObjects(this, m_strDeactivateTarget.c_str(), m_strDeactivateMessage.c_str());
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Alarm::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	*hWrite << m_strActivateTarget;
	*hWrite << m_strActivateMessage;
	*hWrite << m_strDeactivateTarget;
	*hWrite << m_strDeactivateMessage;

	*hWrite << m_strAlarmSound;
	
	hWrite->WriteByte( m_hActiveSound != LTNULL );

	*hWrite << m_bPlayerUsable;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Alarm::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Alarm::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	*hRead >> m_strActivateTarget;
	*hRead >> m_strActivateMessage;
	*hRead >> m_strDeactivateTarget;
	*hRead >> m_strDeactivateMessage;

	*hRead >> m_strAlarmSound;
	
	if( hRead->ReadByte() )
	{
		m_hActiveSound = PlayAlarm(m_strAlarmSound);
	}

	*hRead >> m_bPlayerUsable;
}
