// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayTimer.cpp
//
// PURPOSE : DisplayTimer - Implementation
//
// CREATED : 10/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DisplayTimer.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "MsgIDs.h"

BEGIN_CLASS(DisplayTimer)
	ADD_STRINGPROP(StartCommand, "")
	ADD_STRINGPROP(EndCommand, "")
	ADD_BOOLPROP(RemoveWhenDone, DTRUE)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(DisplayTimer, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::DisplayTimer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DisplayTimer::DisplayTimer() : BaseClass(OT_NORMAL)
{
	m_hstrStartCmd		= DNULL;
	m_hstrEndCmd		= DNULL;
	m_bRemoveWhenDone	= DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::~DisplayTimer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

DisplayTimer::~DisplayTimer()
{
	FREE_HSTRING(m_hstrStartCmd);
	FREE_HSTRING(m_hstrEndCmd);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD DisplayTimer::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD DisplayTimer::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	char tempStr[256];

	switch (messageID)
	{
		case MID_TRIGGER :
		{
			if (hRead->ReadHStringAsStringFL(tempStr, sizeof(tempStr)) == LT_OK)
			{
				TriggerMsg(hSender, tempStr);
			}
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void DisplayTimer::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("StartCommand", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrStartCmd = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("EndCommand", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrEndCmd = g_pServerDE->CreateString(genProp.m_String);
		}
	}

	if (g_pServerDE->GetPropGeneric("RemoveWhenDone", &genProp) == DE_OK)
	{
		m_bRemoveWhenDone = genProp.m_Bool;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void DisplayTimer::InitialUpdate()
{
	// Must be triggered on...

	g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void DisplayTimer::Update()
{
	// Testing...
	//g_pServerDE->CPrint("Time Left: %.2f", m_Timer.GetCountdownTime());

	if (m_Timer.Stopped())
	{
		HandleEnd();
		return;
	}

	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::TriggerMsg()
//
//	PURPOSE:	Process cinematic trigger messages
//
// --------------------------------------------------------------------------- //

void DisplayTimer::TriggerMsg(HOBJECT hSender, char *pMsg)
{
	if (!pMsg) return;

	CommonLT* pCommon = g_pServerDE->Common();
	if (!pCommon) return;

	ConParse parse;
	parse.Init(pMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if ((_stricmp(parse.m_Args[0], "START") == 0) ||
				(_stricmp(parse.m_Args[0], "ON") == 0))
			{
				if (parse.m_nArgs > 1)
				{
					HandleStart((DFLOAT)atof(parse.m_Args[1]));
				}
			}
			else if (_stricmp(parse.m_Args[0], "ADD") == 0)
			{
				if (parse.m_nArgs > 1)
				{
					HandleAdd((DFLOAT)atof(parse.m_Args[1]));
				}
			}
			else if ((_stricmp(parse.m_Args[0], "END") == 0) ||
					 (_stricmp(parse.m_Args[0], "OFF") == 0))
			{
				HandleEnd();
			}
			else if (_stricmp(parse.m_Args[0], "KILL") == 0)
			{
				HandleAbort();
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleStart()
//
//	PURPOSE:	Handle start message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleStart(DFLOAT fTime)
{
	if (fTime <= 0.0f) return;

	if (m_hstrStartCmd)
	{
		char* pCmd = g_pServerDE->GetStringData(m_hstrStartCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}

	m_Timer.Start(fTime);


	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();


	// Update the DisplayTimer...

	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleAdd()
//
//	PURPOSE:	Handle add message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleAdd(DFLOAT fTime)
{
	// Start the timer if it isn't going (and the timer is positive)...

	if (m_Timer.GetCountdownTime() <= 0.0f)
	{
		if (fTime > 0.0f)
		{
			HandleStart(fTime);
		}
		
		return;
	}

	
	// Add/subtract the time from the timer...

	m_Timer.Add(fTime);


	// Send message to clients telling them to update DisplayTimer...

	UpdateClients();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleEnd()
//
//	PURPOSE:	Handle the DisplayTimer ending
//
// ----------------------------------------------------------------------- //

void DisplayTimer::HandleEnd()
{
	if (m_hstrEndCmd)
	{
		char* pCmd = g_pServerDE->GetStringData(m_hstrEndCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}

	
	// Tell client to stop the timer...

	HandleAbort();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::UpdateClients()
//
//	PURPOSE:	Update the client's time
//
// --------------------------------------------------------------------------- //

void DisplayTimer::UpdateClients()
{
	// Send message to clients telling them about the DisplayTimer...

	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_DISPLAY_TIMER);
	g_pServerDE->WriteToMessageFloat(hMessage, m_Timer.GetCountdownTime());
	g_pServerDE->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Handle()
//
//	PURPOSE:	Handle abort message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleAbort()
{
	// Tell the client to stop the timer...

	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_DISPLAY_TIMER);
	g_pServerDE->WriteToMessageFloat(hMessage, 0.0f);
	g_pServerDE->EndMessage(hMessage);

	g_pServerDE->SetNextUpdate(m_hObject, 0.0f);

	if (m_bRemoveWhenDone)
	{
		g_pServerDE->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DisplayTimer::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	SAVE_HSTRING(m_hstrStartCmd);
	SAVE_HSTRING(m_hstrEndCmd);
	SAVE_BOOL(m_bRemoveWhenDone);

	m_Timer.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DisplayTimer::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	LOAD_HSTRING(m_hstrStartCmd);
	LOAD_HSTRING(m_hstrEndCmd);
	LOAD_BOOL(m_bRemoveWhenDone);

	m_Timer.Load(hRead);

	if (m_Timer.GetDuration() > 0.0f)
	{
		UpdateClients();
	}
}
