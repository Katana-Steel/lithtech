// ----------------------------------------------------------------------- //
//
// MODULE  : EventCounter.cpp
//
// PURPOSE : EventCounter - Implementation
//
// CREATED : 03/22/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "EventCounter.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"

BEGIN_CLASS(EventCounter)
	ADD_LONGINTPROP_FLAG(StartingValue, 0, 0)
	PROP_DEFINEGROUP(Event1, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(Event1Value, -1, PF_GROUP1)
		ADD_STRINGPROP_FLAG(Event1IncToValCmd, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(Event1DecToValCmd, "", PF_GROUP1)
	PROP_DEFINEGROUP(Event2, PF_GROUP2)
		ADD_LONGINTPROP_FLAG(Event2Value, -1, PF_GROUP2)
		ADD_STRINGPROP_FLAG(Event2IncToValCmd, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(Event2DecToValCmd, "", PF_GROUP2)
	PROP_DEFINEGROUP(Event3, PF_GROUP3)
		ADD_LONGINTPROP_FLAG(Event3Value, -1, PF_GROUP3)
		ADD_STRINGPROP_FLAG(Event3IncToValCmd, "", PF_GROUP3)
		ADD_STRINGPROP_FLAG(Event3DecToValCmd, "", PF_GROUP3)
	PROP_DEFINEGROUP(Event4, PF_GROUP4)
		ADD_LONGINTPROP_FLAG(Event4Value, -1, PF_GROUP4)
		ADD_STRINGPROP_FLAG(Event4IncToValCmd, "", PF_GROUP4)
		ADD_STRINGPROP_FLAG(Event4DecToValCmd, "", PF_GROUP4)
	PROP_DEFINEGROUP(Event5, PF_GROUP5)
		ADD_LONGINTPROP_FLAG(Event5Value, -1, PF_GROUP5)
		ADD_STRINGPROP_FLAG(Event5IncToValCmd, "", PF_GROUP5)
		ADD_STRINGPROP_FLAG(Event5DecToValCmd, "", PF_GROUP5)
	PROP_DEFINEGROUP(Event6, PF_GROUP6)
		ADD_LONGINTPROP_FLAG(Event6Value, -1, PF_GROUP6)
		ADD_STRINGPROP_FLAG(Event6IncToValCmd, "", PF_GROUP6)
		ADD_STRINGPROP_FLAG(Event6DecToValCmd, "", PF_GROUP6)
END_CLASS_DEFAULT(EventCounter, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::EventCounter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

EventCounter::EventCounter() : GameBase()
{
    m_bLocked = LTFALSE;
	m_nCurVal = 0;

	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::~EventCounter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

EventCounter::~EventCounter()
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 EventCounter::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;

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

	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 EventCounter::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;

	switch (messageID)
	{
		case MID_TRIGGER:
		{
            HSTRING hStr = g_pLTServer->ReadFromMessageHString(hRead);
			if (!hStr) return 0;

            char* pMsg = g_pLTServer->GetStringData(hStr);
			TriggerMsg(hSender, pMsg);

            g_pLTServer->FreeString(hStr);
		}
		break;

		default : break;
	}

	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::TriggerMsg()
//
//	PURPOSE:	Process EventCounter trigger messages
//
// --------------------------------------------------------------------------- //

void EventCounter::TriggerMsg(HOBJECT hSender, char* pMsg)
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;

	if (!pMsg) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	ConParse parse;
	parse.Init(pMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "INC") == 0 ||
				_stricmp(parse.m_Args[0], "INCREMENT") == 0)
			{
				int nCount = 1;
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					nCount = atol(parse.m_Args[1]);
				}
				
				nCount = (nCount < 1 ? 1 : nCount);

				for (int i=0; i < nCount; i++)
				{
					Increment();
				}
			}
			else if (_stricmp(parse.m_Args[0], "DEC") == 0 ||
					 _stricmp(parse.m_Args[0], "DECREMENT") == 0)
			{
				int nCount = 1;
				if (parse.m_nArgs > 1 && parse.m_Args[1])
				{
					nCount = atol(parse.m_Args[1]);
				}
				
				nCount = (nCount < 1 ? 1 : nCount);

				for (int i=0; i < nCount; i++)
				{
					Decrement();
				}
			}
			else if (_stricmp(parse.m_Args[0], "LOCK") == 0)
			{
                m_bLocked = LTTRUE;
			}
			else if (_stricmp(parse.m_Args[0], "UNLOCK") == 0)
			{
                m_bLocked = LTFALSE;
			}
		}
	}

	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Increment
//
//	PURPOSE:	Increment the event counter
//
// ----------------------------------------------------------------------- //

void EventCounter::Increment()
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;

	if (m_bLocked) return;

	m_nCurVal++;

	// See if we should process any commands...

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		if (m_EventData[i].nValue == m_nCurVal)
		{
			if (m_EventData[i].hstrIncToValCmd)
			{
                char* pCmd = g_pLTServer->GetStringData(m_EventData[i].hstrIncToValCmd);
				if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
				{
					g_pCmdMgr->Process(pCmd);
				}
			}
		}
	}

	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Decrement
//
//	PURPOSE:	Decrement the event counter
//
// ----------------------------------------------------------------------- //

void EventCounter::Decrement()
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;

	if (m_bLocked || m_nCurVal <= 0) return;

	m_nCurVal--;

	// See if we should process any commands...

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		if (m_EventData[i].nValue == m_nCurVal)
		{
			if (m_EventData[i].hstrDecToValCmd)
			{
                char* pCmd = g_pLTServer->GetStringData(m_EventData[i].hstrDecToValCmd);
				if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
				{
					g_pCmdMgr->Process(pCmd);
				}
			}
		}
	}

	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL EventCounter::ReadProp(ObjectCreateStruct *pInfo)
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;


    if (!pInfo) return LTFALSE;

	GenericProp genProp;
	char szProp[128];

	for (int i = 0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		sprintf(szProp, "Event%dValue", i + 1);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_EventData[i].nValue = genProp.m_Long;
		}

		sprintf(szProp, "Event%dIncToValCmd", i + 1);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_EventData[i].hstrIncToValCmd = g_pLTServer->CreateString(genProp.m_String);
			}
		}

		sprintf(szProp, "Event%dDecToValCmd", i + 1);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_EventData[i].hstrDecToValCmd = g_pLTServer->CreateString(genProp.m_String);
			}
		}
	}

    if (g_pLTServer->GetPropGeneric("StartingValue", &genProp) == LT_OK)
	{
		m_nCurVal = genProp.m_Long;
	}

	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void EventCounter::Save(HMESSAGEWRITE hWrite)
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;


	SAVE_BOOL(m_bLocked);
	SAVE_INT(m_nCurVal);

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		m_EventData[i].Save(hWrite);
	}


	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void EventCounter::Load(HMESSAGEREAD hRead)
{
	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;


	LOAD_BOOL(m_bLocked);
	LOAD_INT(m_nCurVal);

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		m_EventData[i].Load(hRead);
	}


	if((uint32)m_pFirstAggregate == (uint32)-1)
		m_pFirstAggregate = m_pFirstAggregate;
}