// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveEvent.cpp
//
// PURPOSE : ObjectiveEvent - Implementation
//
// CREATED : 12/21/2000
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "ObjectiveEvent.h"
#include "ObjectiveMgr.h"

BEGIN_CLASS(ObjectiveEvent)
	ADD_STRINGPROP_FLAG_HELP(AddObjective0, "", PF_STATICLIST,	"Add Objective0.")
	ADD_STRINGPROP_FLAG_HELP(AddObjective1, "", PF_STATICLIST,	"Add Objective1.")
	ADD_STRINGPROP_FLAG_HELP(AddObjective2, "", PF_STATICLIST,	"Add Objective2.")
	ADD_STRINGPROP_FLAG_HELP(AddObjective3, "", PF_STATICLIST,	"Add Objective3.")
	ADD_STRINGPROP_FLAG_HELP(AddObjective4, "", PF_STATICLIST,	"Add Objective4.")
	ADD_STRINGPROP_FLAG_HELP(CompleteObjective0, "", PF_STATICLIST,	"Complete Objective0.")
	ADD_STRINGPROP_FLAG_HELP(CompleteObjective1, "", PF_STATICLIST,	"Complete Objective1.")
	ADD_STRINGPROP_FLAG_HELP(CompleteObjective2, "", PF_STATICLIST,	"Complete Objective2.")
	ADD_STRINGPROP_FLAG_HELP(CompleteObjective3, "", PF_STATICLIST,	"Complete Objective3.")
	ADD_STRINGPROP_FLAG_HELP(CompleteObjective4, "", PF_STATICLIST,	"Complete Objective4.")
	ADD_STRINGPROP_FLAG_HELP(RemoveObjective0, "", PF_STATICLIST,	"Remove Objective0.")
	ADD_STRINGPROP_FLAG_HELP(RemoveObjective1, "", PF_STATICLIST,	"Remove Objective1.")
	ADD_STRINGPROP_FLAG_HELP(RemoveObjective2, "", PF_STATICLIST,	"Remove Objective2.")
	ADD_STRINGPROP_FLAG_HELP(RemoveObjective3, "", PF_STATICLIST,	"Remove Objective3.")
	ADD_STRINGPROP_FLAG_HELP(RemoveObjective4, "", PF_STATICLIST,	"Remove Objective4.")
	ADD_STRINGPROP_FLAG_HELP(CancelObjective0, "", PF_STATICLIST,	"Cancel Objective0.")
	ADD_STRINGPROP_FLAG_HELP(CancelObjective1, "", PF_STATICLIST,	"Cancel Objective1.")
	ADD_STRINGPROP_FLAG_HELP(CancelObjective2, "", PF_STATICLIST,	"Cancel Objective2.")
	ADD_STRINGPROP_FLAG_HELP(CancelObjective3, "", PF_STATICLIST,	"Cancel Objective3.")
	ADD_STRINGPROP_FLAG_HELP(CancelObjective4, "", PF_STATICLIST,	"Cancel Objective4.")
END_CLASS_DEFAULT_FLAGS_PLUGIN(ObjectiveEvent, GameBase, LTNULL, LTNULL, 0, CObjectiveEventPlugin)

extern CObjectiveButeMgr*	g_pObjectiveButeMgr;
extern ObjectiveMgr*		g_pObjectiveMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::ObjectiveEvent()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ObjectiveEvent::ObjectiveEvent() : GameBase()
{
	memset(m_nAddObjectives, 0, sizeof(uint32)*MAX_OBJECTIVE_EVENTS);
	memset(m_nCompleteObjectives, 0, sizeof(uint32)*MAX_OBJECTIVE_EVENTS);
	memset(m_nRemoveObjectives, 0, sizeof(uint32)*MAX_OBJECTIVE_EVENTS);
	memset(m_nCancelObjectives, 0, sizeof(uint32)*MAX_OBJECTIVE_EVENTS);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::~ObjectiveEvent()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ObjectiveEvent::~ObjectiveEvent()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectiveEvent::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
			}
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
//	ROUTINE:	ObjectiveEvent::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 ObjectiveEvent::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
			HandleTriggerMsg();
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::HandleTriggerMsg
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void ObjectiveEvent::HandleTriggerMsg()
{
	int i = 0;

	if(g_pObjectiveMgr)
	{
		// Spill our guts to the Objective Manager
		for (i=0; i < MAX_OBJECTIVE_EVENTS; i++)
		{
			if(m_nAddObjectives[i] != 0)
				g_pObjectiveMgr->AddObjective(m_nAddObjectives[i]);
		}

		for ( i=0; i < MAX_OBJECTIVE_EVENTS; i++)
		{
			if(m_nCompleteObjectives[i] != 0)
				g_pObjectiveMgr->ChangeObjectiveState(m_nCompleteObjectives[i], OS_COMPLETE);
		}

		for ( i=0; i < MAX_OBJECTIVE_EVENTS; i++)
		{
			if(m_nRemoveObjectives[i] != 0)
				g_pObjectiveMgr->ChangeObjectiveState(m_nRemoveObjectives[i], OS_REMOVED);
		}

		for ( i=0; i < MAX_OBJECTIVE_EVENTS; i++)
		{
			if(m_nCancelObjectives[i] != 0)
				g_pObjectiveMgr->ChangeObjectiveState(m_nCancelObjectives[i], OS_CANCELED);
		}
	}

	// Now go away
	g_pLTServer->RemoveObject(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectiveEvent::ReadProp(ObjectCreateStruct *pData)
{
	int i = 0;

	if (!pData) return DFALSE;

	const int nMaxFilesize = 500;
	char buf[nMaxFilesize + 1];
	buf[0] = '\0';

	char propName[50];

	//read in all the data
	for (i=0; i < MAX_OBJECTIVE_EVENTS; i++)
	{
		sprintf(propName, "AddObjective%d", i);
		buf[0] = '\0';
		if (g_pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			if (buf[0] && strlen(buf))
			{
				m_nAddObjectives[i] = g_pObjectiveButeMgr->GetObjectiveIdFromName(buf);
			}
		}
	}

	for ( i=0; i < MAX_OBJECTIVE_EVENTS; i++)
	{
		sprintf(propName, "CompleteObjective%d", i);
		buf[0] = '\0';
		if (g_pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			if (buf[0] && strlen(buf))
			{
				m_nCompleteObjectives[i] = g_pObjectiveButeMgr->GetObjectiveIdFromName(buf);
			}
		}
	}

	for ( i=0; i < MAX_OBJECTIVE_EVENTS; i++)
	{
		sprintf(propName, "RemoveObjective%d", i);
		buf[0] = '\0';
		if (g_pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			if (buf[0] && strlen(buf))
			{
				m_nRemoveObjectives[i] = g_pObjectiveButeMgr->GetObjectiveIdFromName(buf);
			}
		}
	}

	for ( i=0; i < MAX_OBJECTIVE_EVENTS; i++)
	{
		sprintf(propName, "CancelObjective%d", i);
		buf[0] = '\0';
		if (g_pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			if (buf[0] && strlen(buf))
			{
				m_nCancelObjectives[i] = g_pObjectiveButeMgr->GetObjectiveIdFromName(buf);
			}
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

LTBOOL ObjectiveEvent::InitialUpdate()
{
	SetNextUpdate(0.0f);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ObjectiveEvent::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	for (int i=0; i < MAX_OBJECTIVE_EVENTS; i++)
	{
		*hWrite << m_nAddObjectives[i];
		*hWrite << m_nCompleteObjectives[i];
		*hWrite << m_nRemoveObjectives[i];
		*hWrite << m_nCancelObjectives[i];
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEvent::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ObjectiveEvent::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;


	for (int i=0; i < MAX_OBJECTIVE_EVENTS; i++)
	{
		*hRead >> m_nAddObjectives[i];
		*hRead >> m_nCompleteObjectives[i];
		*hRead >> m_nRemoveObjectives[i];
		*hRead >> m_nCancelObjectives[i];
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ObjectiveEventPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CObjectiveEventPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...
//	for(int i=0; i<MAX_INITIAL_OBJECTIVES; i++)
//	{
//		char str[20];
//		sprintf(str, "InitialObjective%d",i);

//		if(_strcmpi(str, szPropName) == 0)
//		{
			m_ObjectiveButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
						aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

			return LT_OK;
//		}
//	}

//	return LT_UNSUPPORTED;
}
