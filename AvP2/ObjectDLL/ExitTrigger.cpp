// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.h
//
// PURPOSE : ExitTrigger - Implementation
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ExitTrigger.h"
#include "cpp_server_de.h"
#include "GameServerShell.h"
#include "ServerUtilities.h"
#include "MsgIDs.h"
#include "CharacterHitBox.h"

// This is used to be sure only one instance of the exit trigger exists per level.
static int g_nNumExitTriggers = 0;

BEGIN_CLASS(ExitTrigger)
	ADD_STRINGPROP_FLAG(ExitData, "", PF_STATICLIST)
	ADD_REALPROP_FLAG(TriggerDelay, 0.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(NumberOfActivations, 1, PF_HIDDEN)
	ADD_REALPROP_FLAG(SendDelay, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_HIDDEN)
	PROP_DEFINEGROUP(Targets, PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName1, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName1, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName2, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName2, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName3, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName3, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName4, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName4, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName5, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName5, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName6, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName6, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName7, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName7, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName8, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName8, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName9, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName9, "", PF_GROUP1|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(TargetName10, "", PF_GROUP1|PF_OBJECTLINK|PF_HIDDEN)
		ADD_STRINGPROP_FLAG(MessageName10, "", PF_GROUP1|PF_HIDDEN)
	ADD_BOOLPROP_FLAG(TriggerTouch, 0 , PF_HIDDEN)
	ADD_STRINGPROP_FLAG(MessageTouch, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(AITriggerName, "", PF_HIDDEN)
	ADD_BOOLPROP_FLAG(PlayerTriggerable, 1, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(WeightedTrigger, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Message1Weight, .5, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(TimedTrigger, DFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(MinTriggerTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxTriggerTime, 10.0f, PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(ActivationCount, 1, PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS_PLUGIN(ExitTrigger, Trigger, LTNULL, LTNULL, 0, CExitTrigerPlugin)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ExitTrigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

ExitTrigger::ExitTrigger()
{
#ifndef _FINAL
	if (g_nNumExitTriggers > 0 )
	{
		g_pServerDE->CPrint("ExitTrigger ERROR!  %d ExitTrigger objects detected!", g_nNumExitTriggers + 1);
	}
#endif
	++g_nNumExitTriggers;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::~ExitTrigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

ExitTrigger::~ExitTrigger()
{
	--g_nNumExitTriggers;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ExitTrigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
				
			DDWORD dwRet = Trigger::EngineMessageFn(messageID, pData, fData);
			
			PostPropRead((ObjectCreateStruct*)pData);
			
			return dwRet;
		}

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

	return Trigger::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL ExitTrigger::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return DFALSE;

	GenericProp genProp;

	if ( g_pLTServer->GetPropGeneric( "ExitData", &genProp ) == DE_OK )
	{
		if(g_pMissionMgr)
		{
			MissionButes missionData = g_pMissionMgr->GetMissionButes(genProp.m_String);
			m_nMissionData = missionData.m_nId;
		}
	}
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::PostPropRead
//
//	PURPOSE:	Handle extra initialization
//
// ----------------------------------------------------------------------- //

void ExitTrigger::PostPropRead(ObjectCreateStruct *pStruct)
{
	m_bPlayerTriggerable = DTRUE;
	m_bAITriggerable	 = DFALSE;
	m_hstrAIName		 = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Activate
//
//	PURPOSE:	Handle being activated
//
// ----------------------------------------------------------------------- //

DBOOL ExitTrigger::Activate()
{
	if (!Trigger::Activate()) return DFALSE;
		
	if(m_bTouchNotifyActivation && m_hTouchObject && !IsPlayer(m_hTouchObject))
	{
		// since we are not a player let's see if we are a hitbox
		// for some reason if players are wallwalking then triggers 
		// can be activated by hitboxes...  go figure!
		CCharacterHitBox * pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hTouchObject));

		if(!pCharHitBox) return LTFALSE;

		if(!IsPlayer(pCharHitBox->GetModelObject())) return LTFALSE;
	}

	// Tell the client to fade the screen and exit...

	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(DNULL, MID_PLAYER_EXIT_LEVEL);
	g_pServerDE->WriteToMessageByte(hMessage, (uint8)m_nMissionData); 
	g_pServerDE->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	g_pServerDE->WriteToMessageByte(hWrite, (uint8)m_nMissionData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExitTrigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ExitTrigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	m_nMissionData = (uint32)g_pServerDE->ReadFromMessageByte(hRead);
}

// ----------------------------------------------------------------------- //

LTRESULT CExitTrigerPlugin::PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength)
{
	if(!aszStrings || !pcStrings) return LT_UNSUPPORTED;

	if(stricmp("ExitData", szPropName) == 0)
	{
		sm_ButeMgr.PreHook_EditStringList(	szRezPath, 
											szPropName, 
											aszStrings, 
											pcStrings, 
											cMaxStrings, 
											cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

