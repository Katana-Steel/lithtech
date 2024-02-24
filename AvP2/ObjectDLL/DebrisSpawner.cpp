// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisSpawner.cpp
//
// PURPOSE : Debris Spawner object class implementation
//
// CREATED : 1/24/2001
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebrisSpawner.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "DebrisMgr.h"

BEGIN_CLASS(DebrisSpawner)
	ADD_STRINGPROP_FLAG_HELP(DebrisType0, "", PF_STATICLIST, "Base debris type.")
	ADD_STRINGPROP_FLAG_HELP(DebrisType1, "", PF_STATICLIST, "Additional debris type.")
	ADD_STRINGPROP_FLAG_HELP(DebrisType2, "", PF_STATICLIST, "Additional debris type.")
	ADD_STRINGPROP_FLAG_HELP(DebrisType3, "", PF_STATICLIST, "Additional debris type.")
	ADD_BOOLPROP_FLAG_HELP(StartOn, LTFALSE, 0, "Should the spawner start on?")
	ADD_BOOLPROP_FLAG_HELP(Continuous , LTFALSE, 0, "Should the spawner create debris continuously?  (uses the interval specified below)")
	ADD_REALPROP_FLAG_HELP(SpawnInterval, 1.0f, 0, "This is the time interval inbetween spawns when the spawner is in continuous mode.")
	ADD_STRINGPROP_FLAG_HELP(TriggerOnCommand, "", 0, "This command will be sent each time a Trigger ON is received.")
	ADD_STRINGPROP_FLAG_HELP(TriggerOffCommand, "", 0, "This command will be sent each time a Trigger OFF is received.")
	ADD_STRINGPROP_FLAG_HELP(SpawnCommand, "", 0, "This command will be sent each time a new debris effect is created (mostly applicable for repeating spawners)")
END_CLASS_DEFAULT_FLAGS_PLUGIN(DebrisSpawner, GameBase, NULL, NULL, 0, DebrisSpawnerPlugin)

#define UPDATE_DELTA					0.05f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::DebrisSpawner()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DebrisSpawner::DebrisSpawner()
{ 
	m_strDebris0	= LTNULL;
	m_strDebris1	= LTNULL;
	m_strDebris2	= LTNULL;
	m_strDebris3	= LTNULL;

	m_strOnCmd		= LTNULL;
	m_strOffCmd		= LTNULL;
	m_strSpawnCmd	= LTNULL;

	m_bOn			= LTFALSE;
	m_bContinuous	= LTFALSE;
	m_fInterval		= 1.0f;
	m_fSpawnTime	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::~DebrisSpawner()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

DebrisSpawner::~DebrisSpawner() 
{
	if(m_strDebris0)
	{
		g_pLTServer->FreeString(m_strDebris0);
		m_strDebris0 = LTNULL;
	}

	if(m_strDebris1)
	{
		g_pLTServer->FreeString(m_strDebris1);
		m_strDebris1 = LTNULL;
	}

	if(m_strDebris2)
	{
		g_pLTServer->FreeString(m_strDebris2);
		m_strDebris2 = LTNULL;
	}

	if(m_strDebris3)
	{
		g_pLTServer->FreeString(m_strDebris3);
		m_strDebris3 = LTNULL;
	}

	if(m_strOnCmd)
	{
		g_pLTServer->FreeString(m_strOnCmd);
		m_strOnCmd = LTNULL;
	}

	if(m_strOffCmd)
	{
		g_pLTServer->FreeString(m_strOffCmd);
		m_strOffCmd = LTNULL;
	}

	if(m_strSpawnCmd)
	{
		g_pLTServer->FreeString(m_strSpawnCmd);
		m_strSpawnCmd = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DebrisSpawner::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				SetNextUpdate(UPDATE_DELTA, UPDATE_DELTA + 1.0f);
			}
		}
		break;

		case MID_UPDATE :
		{
			Update();
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
//	ROUTINE:	DebrisSpawner::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 DebrisSpawner::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
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

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerMsg()
//
//	PURPOSE:	Handler for trigger messages.
//
// --------------------------------------------------------------------------- //

void DebrisSpawner::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
    char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg) return;

	//parse the message string
	ConParse parse;
	parse.Init(pMsg);

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "on") == 0)
			{
				//turn on the spawner
				m_bOn = LTTRUE;

				SetNextUpdate(0.001f, UPDATE_DELTA + 1.0f);

				//send the "on" command
				if (m_strOnCmd)
				{
					const char* pCmd = g_pLTServer->GetStringData(m_strOnCmd);

					if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
					{
						g_pCmdMgr->Process(pCmd);
					}
				}
			}
			else if (_stricmp(parse.m_Args[0], "off") == 0)
			{
				//turn off the spawner
				m_bOn = LTFALSE;

				SetNextUpdate(0.0f);

				//send the "off" command
				if (m_strOffCmd)
				{
					const char* pCmd = g_pLTServer->GetStringData(m_strOffCmd);

					if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
					{
						g_pCmdMgr->Process(pCmd);
					}
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL DebrisSpawner::ReadProp(ObjectCreateStruct *pInfo)
{
	GenericProp genProp;

	if (!pInfo) return LTFALSE;

	if (g_pServerDE->GetPropGeneric("DebrisType0", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strDebris0 = g_pLTServer->CreateString(genProp.m_String);
		}
	}
	if (g_pServerDE->GetPropGeneric("DebrisType1", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strDebris1 = g_pLTServer->CreateString(genProp.m_String);
		}
	}
	if (g_pServerDE->GetPropGeneric("DebrisType2", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strDebris2 = g_pLTServer->CreateString(genProp.m_String);
		}
	}
	if (g_pServerDE->GetPropGeneric("DebrisType3", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strDebris3 = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bOn = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("Continuous", &genProp) == LT_OK)
	{
		m_bContinuous = genProp.m_Bool;
	}

	if (g_pServerDE->GetPropGeneric("SpawnInterval", &genProp) == LT_OK)
	{
		m_fInterval = genProp.m_Float;
	}

	if (g_pServerDE->GetPropGeneric("TriggerOnCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strOnCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}
	if (g_pServerDE->GetPropGeneric("TriggerOffCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strOffCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}
	if (g_pServerDE->GetPropGeneric("SpawnCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strSpawnCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::Update()
//
//	PURPOSE:	Update the spawner
//
// ----------------------------------------------------------------------- //
	
LTBOOL DebrisSpawner::Update()
{
	if(m_bOn)
	{
		if(!m_bContinuous)
		{
			//create the debris and exit
			CreateDebris();
			m_bOn = LTFALSE;

			SetNextUpdate(0.0f);
		}
		else
		{
			//see if it's time to make an effect
			if(g_pLTServer->GetTime()-m_fSpawnTime >= m_fInterval)
			{
				//create the debris and reset the timer
				CreateDebris();

				m_fSpawnTime = g_pLTServer->GetTime();
			}
		}

		SetNextUpdate(UPDATE_DELTA, UPDATE_DELTA + 1.0f);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::CreateDebris()
//
//	PURPOSE:	Create the debris effect
//
// ----------------------------------------------------------------------- //
	
void DebrisSpawner::CreateDebris()
{
	//get the position and rotation of the spawner
	LTVector vPos, vNULL, vF;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject,&rRot);
	g_pMathLT->GetRotationVectors(rRot,vNULL,vNULL,vF);

	//create the debris of all types
	if(m_strDebris0)
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(g_pLTServer->GetStringData(m_strDebris0));
		if (pDebris)
			CreatePropDebris(vPos, vF, pDebris->nId);
	}

	if(m_strDebris1)
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(g_pLTServer->GetStringData(m_strDebris1));
		if (pDebris)
			CreatePropDebris(vPos, vF, pDebris->nId);
	}

	if(m_strDebris2)
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(g_pLTServer->GetStringData(m_strDebris2));
		if (pDebris)
			CreatePropDebris(vPos, vF, pDebris->nId);
	}

	if(m_strDebris3)
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(g_pLTServer->GetStringData(m_strDebris3));
		if (pDebris)
			CreatePropDebris(vPos, vF, pDebris->nId);
	}

	//send the command string
	if (m_strSpawnCmd)
	{
		const char* pCmd = g_pLTServer->GetStringData(m_strSpawnCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DebrisSpawner::Save(HMESSAGEWRITE hWrite)
{
	g_pServerDE->WriteToMessageHString(hWrite, m_strDebris0);
	g_pServerDE->WriteToMessageHString(hWrite, m_strDebris1);
	g_pServerDE->WriteToMessageHString(hWrite, m_strDebris2);
	g_pServerDE->WriteToMessageHString(hWrite, m_strDebris3);

	g_pServerDE->WriteToMessageHString(hWrite, m_strOnCmd);
	g_pServerDE->WriteToMessageHString(hWrite, m_strOffCmd);
	g_pServerDE->WriteToMessageHString(hWrite, m_strSpawnCmd);

	*hWrite << m_bOn;
	*hWrite << m_bContinuous;
	*hWrite << m_fInterval;

	//no need to save this
	// m_fSpawnTime;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawner::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DebrisSpawner::Load(HMESSAGEREAD hRead)
{
	m_strDebris0 = g_pServerDE->ReadFromMessageHString(hRead);
	m_strDebris1 = g_pServerDE->ReadFromMessageHString(hRead);
	m_strDebris2 = g_pServerDE->ReadFromMessageHString(hRead);
	m_strDebris3 = g_pServerDE->ReadFromMessageHString(hRead);

	m_strOnCmd		= g_pServerDE->ReadFromMessageHString(hRead);
	m_strOffCmd		= g_pServerDE->ReadFromMessageHString(hRead);
	m_strSpawnCmd	= g_pServerDE->ReadFromMessageHString(hRead);

	*hRead >> m_bOn;
	*hRead >> m_bContinuous;
	*hRead >> m_fInterval;

	//just initialize this one to be safe
	m_fSpawnTime = 0.0f;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DebrisSpawnerPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Fill in property string list
//
// ----------------------------------------------------------------------- //

LTRESULT DebrisSpawnerPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// Check for the debris string props here
	if(		_stricmp(szPropName, "DebrisType0") == 0 
		||	_stricmp(szPropName, "DebrisType1") == 0 
		||	_stricmp(szPropName, "DebrisType2") == 0 
		||	_stricmp(szPropName, "DebrisType3") == 0 )
	{
		m_DebrisPlugin.PreHook_EditStringList(	szRezPath, 
												"DebrisType", 
												aszStrings, 
												pcStrings, 
												cMaxStrings, 
												cMaxStringLength);
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

