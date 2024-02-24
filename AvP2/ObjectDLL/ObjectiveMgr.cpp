// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveMgr.cpp
//
// PURPOSE : ObjectiveMgr - Implementation
//
// CREATED : 12/21/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "ObjectiveMgr.h"
#include "Camera.h"
#include "GameServerShell.h"
#include "PlayerObj.h"

BEGIN_CLASS(ObjectiveMgr)
	ADD_LONGINTPROP_FLAG_HELP(MissionOverview, -1, 0, "This is the string resource ID that will be displayed as the Mission Overview.")
	ADD_STRINGPROP_FLAG_HELP(InitialObjective0, "", PF_STATICLIST,	"Initial Objective0.")
	ADD_STRINGPROP_FLAG_HELP(InitialObjective1, "", PF_STATICLIST,	"Initial Objective1.")
	ADD_STRINGPROP_FLAG_HELP(InitialObjective2, "", PF_STATICLIST,	"Initial Objective2.")
	ADD_STRINGPROP_FLAG_HELP(InitialObjective3, "", PF_STATICLIST,	"Initial Objective3.")
	ADD_STRINGPROP_FLAG_HELP(InitialObjective4, "", PF_STATICLIST,	"Initial Objective4.")
	ADD_STRINGPROP_FLAG_HELP(InitialObjective5, "", PF_STATICLIST,	"Initial Objective5.")
END_CLASS_DEFAULT_FLAGS_PLUGIN(ObjectiveMgr, GameBase, LTNULL, LTNULL, 0, CObjectivePlugin)

extern CObjectiveButeMgr* g_pObjectiveButeMgr;

ObjectiveMgr*	g_pObjectiveMgr = LTNULL;

int ObjectiveMgr::m_count = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::ObjectiveMgr()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ObjectiveMgr::ObjectiveMgr() : GameBase()
{
	// make sure we are the only one!
	++m_count;

#ifndef _FINAL
	if( m_count != 0 && m_count != 1 )
	{
		g_pLTServer->CPrint("***** ERROR!!! %d ObjectiveMgr's found!  Fix this problem immediately! *****", m_count);
	}
#endif

	memset(m_nObjectives, 0, sizeof(uint32)*MAX_INITIAL_OBJECTIVES);
	m_bSendInitialObjectives = LTFALSE;
	
	g_pObjectiveMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::~ObjectiveMgr()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ObjectiveMgr::~ObjectiveMgr()
{
	g_pObjectiveMgr = LTNULL;
	--m_count;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectiveMgr::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
			if (!pStruct) return 0;

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(pStruct);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
				m_bSendInitialObjectives = LTTRUE;
			}
		}
		break;

		case MID_UPDATE:
		{
			if(m_bSendInitialObjectives && (!IsExternalCameraActive() && !g_pGameServerShell->GetMuteState()))
			{
				// ----------------------------------------------------------------------- //
				// see that the player is not trying to autosave us....
				HOBJECT hPlayerObj = LTNULL;
				ObjArray <HOBJECT, 1> objArray;

				g_pLTServer->FindNamedObjects("Player", objArray);
				if(!objArray.NumObjects()) break;
				
				hPlayerObj = objArray.GetObject(0);
				if(!hPlayerObj) break;

				CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hPlayerObj));
				
				if(pPlayer && pPlayer->IsWaitingForAutoSave())
				{
					SetNextUpdate(0.1f);
					break;
				}
				// ----------------------------------------------------------------------- //

				for (int i=0; i < MAX_INITIAL_OBJECTIVES; i++)
				{
					if(m_nObjectives[i] != 0)
						AddObjective(m_nObjectives[i]);
				}

				//send message to client about overview
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_OBJECTIVE_OVERVIEW);
				g_pLTServer->WriteToMessageDWord(hMessage, (uint32)m_nOverviewID);
				g_pLTServer->EndMessage(hMessage);


				m_bSendInitialObjectives = LTFALSE;
				SetNextUpdate(0.0f);
			}
			else
				SetNextUpdate(0.1f);
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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectiveMgr::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			if(HandleTriggerMsg(hSender, hRead))
				return 1;

			break;
		}
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::HandleTriggerMsg
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectiveMgr::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char* pMsg   = g_pServerDE->GetStringData(hMsg);

	CommonLT* pCommon = g_pServerDE->Common();
	if (!pCommon) return LTFALSE;	
	
	LTBOOL bHandled = LTFALSE;

	ConParse parse;
	parse.Init(const_cast<char*>(pMsg) );

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs ==2)
		{
			if (_stricmp(parse.m_Args[0], "ADD") == 0)
			{
				AddObjective(parse.m_Args[1]);
				bHandled = LTTRUE;
			}
			else if (_stricmp(parse.m_Args[0], "COMPLETE") == 0)
			{
				ChangeObjectiveState(parse.m_Args[1], OS_COMPLETE);
				bHandled = LTTRUE;
			}
			else if (_stricmp(parse.m_Args[0], "REMOVE") == 0)
			{
				ChangeObjectiveState(parse.m_Args[1], OS_REMOVED);
				bHandled = LTTRUE;
			}
			else if (_stricmp(parse.m_Args[0], "CANCEL") == 0)
			{
				ChangeObjectiveState(parse.m_Args[1], OS_CANCELED);
				bHandled = LTTRUE;
			}
			else if (_stricmp(parse.m_Args[0], "OVERVIEW") == 0)
			{
				//send message to client about overview
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_OBJECTIVE_OVERVIEW);
				g_pLTServer->WriteToMessageDWord(hMessage, (uint32)atol(parse.m_Args[1]));
				g_pLTServer->EndMessage(hMessage);

				bHandled = LTTRUE;
			}
		}
	}

	g_pServerDE->FreeString(hMsg);

	return bHandled;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectiveMgr::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return DFALSE;
	GenericProp genProp;

	const int nMaxFilesize = 500;
	char buf[nMaxFilesize + 1];
	buf[0] = '\0';

	char propName[50];

	//clear the objective dispaly
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_OBJECTIVE_RESET);
	g_pLTServer->EndMessage(hMessage);

	//read in and add all the objectives
	for (int i=0; i < MAX_INITIAL_OBJECTIVES; i++)
	{
		sprintf(propName, "InitialObjective%d", i);
		buf[0] = '\0';
		if (g_pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			if (buf[0] && stricmp(buf, "-None-") != 0)
			{
				m_nObjectives[i] = g_pObjectiveButeMgr->GetObjectiveIdFromName(buf);
			}
		}
	}

	if (g_pServerDE->GetPropGeneric("MissionOverview", &genProp) == DE_OK)
	{
		m_nOverviewID = genProp.m_Long;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectiveMgr::InitialUpdate()
{
	SetNextUpdate(0.01f);

	return LTTRUE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::AddObjective
//
//	PURPOSE:	Add an objective
//
// ----------------------------------------------------------------------- //

void ObjectiveMgr::AddObjective(uint32 nId)
{
	AddObjective(g_pObjectiveButeMgr->GetObjectiveName(nId));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::AddObjective
//
//	PURPOSE:	Add an objective
//
// ----------------------------------------------------------------------- //

void ObjectiveMgr::AddObjective(char* szObjective)
{
	//find an empty place to add our objective
	for (int i=0; i<MAX_OBJECTIVES; i++)
	{
		if(m_Objective[i].nId == 0)
		{
			m_Objective[i].nId = g_pObjectiveButeMgr->GetObjectiveIdFromName(szObjective);

			if(m_Objective[i].nId == -1)
			{
#ifndef _FINAL
				g_pLTServer->CPrint("WARNING: Objective '%s' could not be found in the ObjectiveButes.  Objective ignored!", szObjective);
				m_Objective[i].nId = 0;
#endif
				return;
			}

			_ASSERT(m_Objective[i].nId != 0);

			m_Objective[i].eState = OS_OPEN;

			//send message to client about this new objective
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_OBJECTIVE_ADD);
			g_pLTServer->WriteToMessageWord(hMessage, (uint16)m_Objective[i].nId);
			g_pLTServer->EndMessage(hMessage);

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::ChangeObjectiveState
//
//	PURPOSE:	Change the objective state
//
// ----------------------------------------------------------------------- //

void ObjectiveMgr::ChangeObjectiveState(uint32 nId, ObjectiveState eState)
{
	ChangeObjectiveState(g_pObjectiveButeMgr->GetObjectiveName(nId), eState);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::ChangeObjectiveState
//
//	PURPOSE:	Change the objective state
//
// ----------------------------------------------------------------------- //

void ObjectiveMgr::ChangeObjectiveState(char* szObjective, ObjectiveState eState)
{
	uint32 nId = g_pObjectiveButeMgr->GetObjectiveIdFromName(szObjective);

	_ASSERT(nId != 0);

	//find this objective and update its state
	for (int i=0; i<MAX_OBJECTIVES; i++)
	{
		if(m_Objective[i].nId == nId)
		{
			m_Objective[i].eState = eState;

			//send message to client about this new objective
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_OBJECTIVE_STATECHANGE);
			g_pLTServer->WriteToMessageWord(hMessage, (uint16)m_Objective[i].nId);
			g_pLTServer->WriteToMessageByte(hMessage, m_Objective[i].eState);
			g_pLTServer->EndMessage(hMessage);

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ObjectiveMgr::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	int i = 0;

	if (!hWrite) return;

	//Save out all the objective data
	for(i=0; i<MAX_OBJECTIVES; i++)
	{
		m_Objective[i].Save(hWrite);
	}

	for (i=0; i < MAX_INITIAL_OBJECTIVES; i++)
	{
		*hWrite << m_nObjectives[i];
	}
	*hWrite << m_nOverviewID;
	*hWrite << m_bSendInitialObjectives;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgr::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectiveMgr::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	int i = 0;

	if (!hRead) return;

	//load up all the objective data
	for(i=0; i<MAX_OBJECTIVES; i++)
	{
		m_Objective[i].Load(hRead);
	}

	for (i=0; i < MAX_INITIAL_OBJECTIVES; i++)
	{
		*hRead >> m_nObjectives[i];
	}
	*hRead >> m_nOverviewID;
	*hRead >> m_bSendInitialObjectives;

	if(m_bSendInitialObjectives)
		SetNextUpdate(0.01f, 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveMgrPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CObjectivePlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...
	for(int i=0; i<MAX_INITIAL_OBJECTIVES; i++)
	{
		char str[20];
		sprintf(str, "InitialObjective%d",i);

		if(_stricmp(str, szPropName) == 0)
		{
			m_ObjectiveButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
						aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
