// ----------------------------------------------------------------------- //
//
// MODULE  : TorchableLockCounter.cpp
//
// PURPOSE : TorchableLockCounter - Implementation
//
// CREATED : 11.20.2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TorchableLockCounter.h"
#include "TorchableLock.h"
#include "Door.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"

BEGIN_CLASS(TorchableLockCounter)
	ADD_OBJECTPROP_FLAG_HELP(MainTarget, "", 0, "This object (has to be a Door or derived object) will recieve 'lock', 'unlock', and 'trigger' messages from this counter automatically.")
	ADD_BOOLPROP_FLAG_HELP(TriggerWhenUnlocked, LTTRUE, 0, "This tells the counter to send a 'trigger' message to the MainTarget after sending the 'unlock' message.")
	ADD_STRINGPROP_FLAG_HELP(OnLockCommand, "", 0, "This command string will be caled when all the TorchableLocks are welded closed, in addition to the lock message sent to the door itself.")
	ADD_STRINGPROP_FLAG_HELP(OnOpenCommand, "", 0, "This command string will be caled when all the TorchableLocks are cut open, in addition to the unlock message sent to the door itself.") 
	PROP_DEFINEGROUP(SideA, PF_GROUP1)
		ADD_OBJECTPROP_FLAG_HELP(SideALock0, "", PF_GROUP1, "This TorchableLock is one of the objects contained in the first lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideALock1, "", PF_GROUP1, "This TorchableLock is one of the objects contained in the first lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideALock2, "", PF_GROUP1, "This TorchableLock is one of the objects contained in the first lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideALock3, "", PF_GROUP1, "This TorchableLock is one of the objects contained in the first lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideALock4, "", PF_GROUP1, "This TorchableLock is one of the objects contained in the first lock group.")
	PROP_DEFINEGROUP(SideB, PF_GROUP2)
		ADD_OBJECTPROP_FLAG_HELP(SideBLock0, "", PF_GROUP2, "This TorchableLock is one of the objects contained in the second lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideBLock1, "", PF_GROUP2, "This TorchableLock is one of the objects contained in the second lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideBLock2, "", PF_GROUP2, "This TorchableLock is one of the objects contained in the second lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideBLock3, "", PF_GROUP2, "This TorchableLock is one of the objects contained in the second lock group.")
		ADD_OBJECTPROP_FLAG_HELP(SideBLock4, "", PF_GROUP2, "This TorchableLock is one of the objects contained in the second lock group.")
END_CLASS_DEFAULT(TorchableLockCounter, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::TorchableLockCounter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TorchableLockCounter::TorchableLockCounter() : GameBase()
{
	m_szMainTarget = "\0";
	m_hMainTarget = LTNULL;

	m_strLockCmd = "\0";
	m_strOpenCmd = "\0";

	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		m_szSideA[i] = "\0";
		m_hSideA[i] = LTNULL;

		m_szSideB[i] = "\0";
		m_hSideB[i] = LTNULL;
	}

	m_bTriggerWhenUnlocked = LTTRUE;
	m_bFirstUpdate = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::~TorchableLockCounter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

TorchableLockCounter::~TorchableLockCounter()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TorchableLockCounter::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

		case MID_INITIALUPDATE:
		{
			g_pLTServer->SetNextUpdate(m_hObject, 0.01f);
		}
		break;

		case MID_UPDATE:
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 TorchableLockCounter::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
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

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::Update
//
//	PURPOSE:	Check for updates
//
// ----------------------------------------------------------------------- //

LTBOOL TorchableLockCounter::Update()
{
	// Set the next update
	g_pLTServer->SetNextUpdate(m_hObject, 0.1f);

	// Setup our object handles on the first update
	if(m_bFirstUpdate)
	{
		SetupObjectHandles();

		// Make sure we don't get in here again...
		m_bFirstUpdate = LTFALSE;
	}
	else
	{
		// Get the handle to the door we're attached to
		if(m_hMainTarget.GetHOBJECT() == LTNULL)
		{
			RemoveAllLocks();

			//now remove us
			g_pLTServer->RemoveObject(m_hObject);

			return LTTRUE;
		}

		Door* pDoor = dynamic_cast<Door*>(g_pLTServer->HandleToObject(m_hMainTarget));
		if(!pDoor) return LTTRUE;

		// Check the door state
		if(pDoor->GetState() != DOORSTATE_CLOSED)
		{
			DestroyAllLocks();
		}
		else
		{
			EnableAllLocks();
		}

		// See if the door HAS to be locked
		if(pDoor->GetState() == DOORSTATE_CLOSED && StateOfAllLocksEquals(BOLT))
		{
			if(!pDoor->IsLocked())
			{
				SendTriggerMsgToObject(this, pDoor->m_hObject, "Lock");

				//send on lock commnad string
				if (m_strLockCmd)
				{
					const char* pCmd = m_strLockCmd.CStr();

					if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
					{
						g_pCmdMgr->Process(pCmd);
					}
				}
			}
		}
		else if(StateOfAllLocksEquals(WELD))
		{
			if(pDoor->IsLocked())
			{
				SendTriggerMsgToObject(this, pDoor->m_hObject, "Unlock");

				if(m_bTriggerWhenUnlocked)
				{
					SendTriggerMsgToObject(this, pDoor->m_hObject, "Trigger");
				}
				//send on unlock commnad string
				if (m_strOpenCmd)
				{
					const char* pCmd = m_strOpenCmd.CStr();

					if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
					{
						g_pCmdMgr->Process(pCmd);
					}
				}
			}
		}
	}


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL TorchableLockCounter::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;
	char szProp[128];


    if(g_pLTServer->GetPropGeneric("MainTarget", &genProp) == LT_OK)
	{
		m_szMainTarget = genProp.m_String;
	}

    if(g_pLTServer->GetPropGeneric("TriggerWhenUnlocked", &genProp) == LT_OK)
	{
		m_bTriggerWhenUnlocked = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("OnLockCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strLockCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OnOpenCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strOpenCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		sprintf(szProp, "SideALock%d", i);

        if(g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_szSideA[i] = genProp.m_String;
		}

		sprintf(szProp, "SideBLock%d", i);

        if(g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_szSideB[i] = genProp.m_String;
		}
	}


    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::TriggerMsg()
//
//	PURPOSE:	Process EventCounter trigger messages
//
// --------------------------------------------------------------------------- //

void TorchableLockCounter::TriggerMsg(HOBJECT hSender, char* pMsg)
{
	if (!pMsg) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	ConParse parse;
	parse.Init(pMsg);

	while(pCommon->Parse(&parse) == LT_OK)
	{
		if(parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if(!_stricmp(parse.m_Args[0], "CutDamage"))
			{
				HOBJECT hObj = FindSideMatch(hSender);

				if(hObj)
				{
					LTVector vDir(0.0f, 0.0f, 0.0f);

					HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, hObj, MID_DAMAGE);
					g_pLTServer->WriteToMessageVector(hWrite, &vDir);
					g_pLTServer->WriteToMessageFloat(hWrite, 0.0f);
					g_pLTServer->WriteToMessageFloat(hWrite, 1.0f);
					g_pLTServer->WriteToMessageByte(hWrite, DT_TORCH_CUT);
					g_pLTServer->WriteToMessageObject(hWrite, hSender);
					g_pLTServer->WriteToMessageObject(hWrite, LTNULL);
					g_pLTServer->EndMessage(hWrite);
				}
			}
			else if(!_stricmp(parse.m_Args[0], "WeldDamage"))
			{
				HOBJECT hObj = FindSideMatch(hSender);

				if(hObj)
				{
					LTVector vDir(0.0f, 0.0f, 0.0f);

					HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, hObj, MID_DAMAGE);
					g_pLTServer->WriteToMessageVector(hWrite, &vDir);
					g_pLTServer->WriteToMessageFloat(hWrite, 0.0f);
					g_pLTServer->WriteToMessageFloat(hWrite, 1.0f);
					g_pLTServer->WriteToMessageByte(hWrite, DT_TORCH_WELD);
					g_pLTServer->WriteToMessageObject(hWrite, hSender);
					g_pLTServer->WriteToMessageObject(hWrite, LTNULL);
					g_pLTServer->EndMessage(hWrite);
				}
			}
		}
	}
	
	// Now force an update.
	g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
	g_pLTServer->SetDeactivationTime( m_hObject, 0.1f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TorchableLockCounter::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_szMainTarget;
	*hWrite << m_hMainTarget;

	*hWrite << m_strLockCmd;
	*hWrite << m_strOpenCmd;

	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		*hWrite << m_szSideA[i];
		*hWrite << m_hSideA[i];

		*hWrite << m_szSideB[i];
		*hWrite << m_hSideB[i];
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TorchableLockCounter::Load(HMESSAGEREAD hRead)
{
	*hRead >> m_szMainTarget;
	*hRead >> m_hMainTarget;

	*hRead >> m_strLockCmd;
	*hRead >> m_strOpenCmd;

	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		*hRead >> m_szSideA[i];
		*hRead >> m_hSideA[i];

		*hRead >> m_szSideB[i];
		*hRead >> m_hSideB[i];
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::SetupObjectHandles
//
//	PURPOSE:	Sets up the object handles
//
// ----------------------------------------------------------------------- //

void TorchableLockCounter::SetupObjectHandles()
{
	ObjArray<HOBJECT,1> objArray;
	unsigned long nTotal = 0;

	if(!m_szMainTarget.IsNull())
	{
		g_pLTServer->FindNamedObjects(const_cast<char*>(m_szMainTarget.CStr()), objArray, &nTotal);

		_ASSERT(nTotal > 0);
		if (nTotal > 0)
		{
			m_hMainTarget = objArray.GetObject(0);
		}
		else
		{
			// Don't continue.  This counter's target object doesn't exist, so the counter
			// will get removed.  Therefore SetCounterObject should not be called.
			return;
		}
	}

	HCLASS hClass = g_pLTServer->GetClass("TorchableLock");
	HCLASS hObjClass = LTNULL;

	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		if(!m_szSideA[i].IsNull())
		{
			if(LT_OK == g_pLTServer->FindNamedObjects(const_cast<char*>(m_szSideA[i].CStr()), objArray))
			{
				if(objArray.NumObjects())
				{
					m_hSideA[i] = objArray.GetObject(0);

					hObjClass = g_pLTServer->GetObjectClass(m_hSideA[i]);

					if(g_pLTServer->IsKindOf(hClass, hObjClass))
					{
						TorchableLock* pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideA[i]));
						if(pLock) pLock->SetCounterObject(m_hObject);
					}
				}
			}
		}

		if(!m_szSideB[i].IsNull())
		{
			if(LT_OK == g_pLTServer->FindNamedObjects(const_cast<char*>(m_szSideB[i].CStr()), objArray))
			{
				if(objArray.NumObjects())
				{
					m_hSideB[i] = objArray.GetObject(0);

					hObjClass = g_pLTServer->GetObjectClass(m_hSideB[i]);

					if(g_pLTServer->IsKindOf(hClass, hObjClass))
					{
						TorchableLock* pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideB[i]));
						if(pLock) pLock->SetCounterObject(m_hObject);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::FindSideMatch
//
//	PURPOSE:	Find the opposite lock
//
// ----------------------------------------------------------------------- //

HOBJECT TorchableLockCounter::FindSideMatch(HOBJECT hObj)
{
	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		if(m_hSideA[i].GetHOBJECT() == hObj)
			return m_hSideB[i].GetHOBJECT();

		if(m_hSideB[i].GetHOBJECT() == hObj)
			return m_hSideA[i].GetHOBJECT();
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::DestroyAllLocks
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void TorchableLockCounter::DestroyAllLocks()
{
	TorchableLock* pLock = LTNULL;
	TorchMode eMode = INVALID;
	LTFLOAT fRatio;

	// Go through each lock
	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		if(m_hSideA[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideA[i]));

			if(pLock)
			{
				eMode = pLock->GetMode(fRatio);

				if(!(eMode == WELD && fRatio == 1.0f))
					pLock->DestroyBolt();

				pLock->SetEnabled(LTFALSE);
			}
		}


		if(m_hSideB[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideB[i]));

			if(pLock)
			{
				eMode = pLock->GetMode(fRatio);

				if(!(eMode == WELD && fRatio == 1.0f))
					pLock->DestroyBolt();

				pLock->SetEnabled(LTFALSE);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::RemoveAllLocks
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void TorchableLockCounter::RemoveAllLocks()
{
	TorchableLock* pLock = LTNULL;

	// Go through each lock
	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		if(m_hSideA[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideA[i]));

			if(pLock)
				pLock->Remove();
		}

		if(m_hSideB[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideB[i]));

			if(pLock)
				pLock->Remove();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::EnableAllLocks
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void TorchableLockCounter::EnableAllLocks()
{
	TorchableLock* pLock = LTNULL;

	// Go through each lock
	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		if(m_hSideA[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideA[i]));

			if(pLock)
				pLock->SetEnabled(LTTRUE);
		}

		if(m_hSideB[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideB[i]));

			if(pLock)
				pLock->SetEnabled(LTTRUE);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLockCounter::StateOfAllLocksEquals
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTBOOL TorchableLockCounter::StateOfAllLocksEquals(int nState)
{
	TorchableLock* pLock = LTNULL;
	TorchMode eMode = INVALID;
	LTFLOAT fRatio;

	// Go through each lock
	for(int i = 0; i < MAX_LOCKS_PER_SIDE; i++)
	{
		if(m_hSideA[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideA[i]));

			if(pLock)
			{
				if(pLock->GetMode(fRatio) != nState)
					return LTFALSE;
			}
		}


		if(m_hSideB[i].GetHOBJECT() != LTNULL)
		{
			pLock = dynamic_cast<TorchableLock*>(g_pLTServer->HandleToObject(m_hSideB[i]));

			if(pLock)
			{
				if(pLock->GetMode(fRatio) != nState)
					return LTFALSE;
			}
		}
	}

	return LTTRUE;
}

