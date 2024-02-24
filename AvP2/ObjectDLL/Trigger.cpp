// ----------------------------------------------------------------------- //
//
// MODULE  : Trigger.cpp
//
// PURPOSE : Trigger - Implementation
//
// CREATED : 10/6/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Trigger.h"
#include "cpp_server_de.h"
#include "MsgIDs.h"
#include "ServerSoundMgr.h"
#include "ObjectMsgs.h"
#include "ClientServerShared.h"
#include "Character.h"
#include "PlayerObj.h"
#include "CharacterHitBox.h"
#include "CVarTrack.h"

#include <stdio.h>

BEGIN_CLASS(Trigger)
	ADD_VECTORPROP_VAL_FLAG(Dims, 16.0f, 16.0f, 16.0f, PF_DIMS) 
	ADD_LONGINTPROP(NumberOfActivations, 1)
	ADD_REALPROP(SendDelay, 0.0f)
	ADD_REALPROP(TriggerDelay, 0.0)

	PROP_DEFINEGROUP(Targets, PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName1, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName1, "TRIGGER", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName2, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName2, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName3, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName3, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName4, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName4, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName5, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName5, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName6, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName6, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName7, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName7, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName8, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName8, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName9, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName9, "", PF_GROUP1)
		ADD_STRINGPROP_FLAG(TargetName10, "", PF_GROUP1|PF_OBJECTLINK)
		ADD_STRINGPROP_FLAG(MessageName10, "", PF_GROUP1)

	ADD_BOOLPROP(TriggerTouch, 0)
	ADD_STRINGPROP(MessageTouch, "")
	ADD_BOOLPROP(PlayerTriggerable, 1)
	ADD_BOOLPROP(AITriggerable, 0)
	ADD_OBJECTPROP(AITriggerName, "")
	ADD_BOOLPROP(WeightedTrigger, DFALSE)
	ADD_REALPROP(Message1Weight, .5)
	ADD_BOOLPROP(TimedTrigger, DFALSE)
	ADD_REALPROP(MinTriggerTime, 0.0f)
	ADD_REALPROP(MaxTriggerTime, 10.0f)
	ADD_LONGINTPROP(ActivationCount, 1)
	ADD_BOOLPROP(Locked, 0)
	ADD_STRINGPROP_FLAG(ActivationSound, "", PF_FILENAME)
	ADD_REALPROP_FLAG(SoundRadius, 200.0f, PF_RADIUS)
	ADD_STRINGPROP_FLAG(AttachToObject, "", PF_OBJECTLINK)
END_CLASS_DEFAULT(Trigger, GameBase, NULL, NULL)


// Static global variables...

static char *g_szLock    = "LOCK"; 
static char *g_szUnLock  = "UNLOCK";
static char *g_szTrigger = "TRIGGER";
static char *g_szRemove  = "REMOVE";

#define UPDATE_DELTA					0.1f
#define TRIGGER_DEACTIVATION_TIME		0.001f

CVarTrack g_vNoTriggersInClip;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Trigger()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Trigger::Trigger() : GameBase()
{
	VEC_SET( m_vDims, 5.0f, 5.0f, 5.0f );
	m_fTriggerDelay	= 0.0f;

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		m_hstrTargetName[i] = DNULL;
		m_hstrMessageName[i] = DNULL;
	}

	m_bTriggerTouch			= DFALSE;
	m_bTouchNotifyActivation= DFALSE;
	m_hTouchObject			= DNULL;
	m_hstrMessageTouch		= DNULL;
	m_hstrActivationSound	= DNULL;
	m_hstrAttachToObject	= DNULL;
	m_bAttached				= DFALSE;
	m_fSoundRadius			= 200.0f;
	m_bActive				= DTRUE;
	m_bPlayerTriggerable	= DTRUE;
	m_bAITriggerable		= DFALSE;
	m_bLocked				= DFALSE;
	m_hstrAIName			= DNULL;
	m_bDelayingActivate		= DFALSE;
	m_fStartDelayTime		= 0.0f;
	m_fSendDelay			= 0.0f;

	m_fLastTouchTime		= -1.0f;

	m_nCurrentActivation	= 0;
	m_nActivationCount		= 1;

	m_nNumActivations		= 1;
	m_nNumTimesActivated	= 0;

	m_bWeightedTrigger		= DFALSE;
	m_fMessage1Weight		= .5f;

	m_bTimedTrigger			= DFALSE;
	m_fMinTriggerTime		= 0.0f;
	m_fMaxTriggerTime		= 1.0f;

	m_fNextTriggerTime		= 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::SetTargetName1()
//
//	PURPOSE:	Set target name 1
//
// ----------------------------------------------------------------------- //

void Trigger::SetTargetName1(char* pName)
{
	if (!pName) return;
	
	if (m_hstrTargetName[0]) g_pServerDE->FreeString(m_hstrTargetName[0]);
	m_hstrTargetName[0] = g_pServerDE->CreateString(pName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::SetMessageName1()
//
//	PURPOSE:	Set target name 1
//
// ----------------------------------------------------------------------- //

void Trigger::SetMessageName1(char* pMsg)
{
	if (!pMsg) return;
	
	if (m_hstrMessageName[0]) g_pServerDE->FreeString(m_hstrMessageName[0]);
	m_hstrMessageName[0] = g_pServerDE->CreateString(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::~Trigger()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Trigger::~Trigger()
{
	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		if (m_hstrTargetName[i])
		{
			g_pServerDE->FreeString(m_hstrTargetName[i]);
		}
		if (m_hstrMessageName[i])
		{
			g_pServerDE->FreeString(m_hstrMessageName[i]);
		}
	}

	if (m_hstrMessageTouch)
	{
		g_pServerDE->FreeString(m_hstrMessageTouch);
	}

	if (m_hstrActivationSound)
	{
		g_pServerDE->FreeString(m_hstrActivationSound);
	}

	if (m_hstrAttachToObject)
	{
		g_pServerDE->FreeString(m_hstrAttachToObject);
	}

	if (m_hstrAIName)
	{
		g_pServerDE->FreeString(m_hstrAIName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Trigger::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
				g_pServerDE->RemoveObject(m_hObject);		
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch((HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct *)pData;
			if (!pStruct) return 0;

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(pStruct);
			}

			// These get set from the saved game, so don't stomp on them!
			if( fData != PRECREATE_SAVEGAME )
			{
				pStruct->m_UserData = USRFLG_IGNORE_PROJECTILES;
				pStruct->m_fDeactivationTime = TRIGGER_DEACTIVATION_TIME;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink == m_hTouchObject)
			{
				m_hTouchObject = DNULL;
			}
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

		case MID_PARENTATTACHMENTREMOVED :
		{
			// Go away if our parent is removed...

			g_pServerDE->RemoveObject(m_hObject);
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

DDWORD Trigger::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
			HandleTriggerMsg(hSender, hRead);
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::HandleTriggerMsg
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void Trigger::HandleTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead)
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char* pMsg   = g_pServerDE->GetStringData(hMsg);

	// See if we should trigger the trigger...

	if (_stricmp(pMsg, g_szTrigger) == 0)
	{
		DoTrigger(hSender, DFALSE);
	}
	else if (_stricmp(pMsg, g_szLock) == 0)  // See if we should lock the trigger...
	{
		m_bLocked = DTRUE;
	}
	else if (_stricmp(pMsg, g_szUnLock) == 0) // See if we should unlock the trigger...
	{
		m_bLocked = DFALSE;
	}
	else if (_stricmp(pMsg, g_szRemove) == 0)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}

	g_pServerDE->FreeString(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return DFALSE;

	const int nMaxFilesize = 500;
	char buf[nMaxFilesize + 1];
	buf[0] = '\0';

	char propName[50];
	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		sprintf(propName, "TargetName%d", i+1);
		buf[0] = '\0';
		if (g_pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			if (buf[0] && strlen(buf))
			{
				m_hstrTargetName[i] = g_pServerDE->CreateString(buf);
			}
		}

		sprintf(propName, "MessageName%d", i+1);
		buf[0] = '\0';
		if (g_pServerDE->GetPropString(propName, buf, nMaxFilesize) == DE_OK)
		{
			if (buf[0] && strlen(buf))
			{
				m_hstrMessageName[i] = g_pServerDE->CreateString(buf);
			}
		}
	}

	g_pServerDE->GetPropBool("TriggerTouch", &m_bTriggerTouch);

	buf[0] = '\0';
	g_pServerDE->GetPropString("MessageTouch", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrMessageTouch = g_pServerDE->CreateString(buf);

	buf[0] = '\0';
	g_pServerDE->GetPropString("ActivationSound", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrActivationSound = g_pServerDE->CreateString(buf);

	buf[0] = '\0';
	g_pServerDE->GetPropString("AttachToObject", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrAttachToObject = g_pServerDE->CreateString(buf);

	g_pServerDE->GetPropVector("Dims", &m_vDims);
	g_pServerDE->GetPropReal("TriggerDelay", &m_fTriggerDelay);
	g_pServerDE->GetPropReal("SendDelay", &m_fSendDelay);
	g_pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);
	g_pServerDE->GetPropBool("PlayerTriggerable", &m_bPlayerTriggerable);
	g_pServerDE->GetPropBool("AITriggerable", &m_bAITriggerable);
	g_pServerDE->GetPropBool("Locked", &m_bLocked);

	g_pServerDE->GetPropReal("SoundRadius", &m_fSoundRadius);

	buf[0] = '\0';
	g_pServerDE->GetPropString("AITriggerName", buf, nMaxFilesize);
	if (buf[0] && strlen(buf)) m_hstrAIName = g_pServerDE->CreateString(buf);

	long nLongVal;
	g_pServerDE->GetPropLongInt("ActivationCount", &nLongVal);
	m_nActivationCount = nLongVal;

	g_pServerDE->GetPropLongInt("NumberOfActivations", &nLongVal);
	m_nNumActivations = nLongVal;

	g_pServerDE->GetPropBool("WeightedTrigger", &m_bWeightedTrigger);
	g_pServerDE->GetPropReal("Message1Weight", &m_fMessage1Weight);

	g_pServerDE->GetPropBool("TimedTrigger", &m_bTimedTrigger);
	g_pServerDE->GetPropReal("MinTriggerTime", &m_fMinTriggerTime);
	g_pServerDE->GetPropReal("MaxTriggerTime", &m_fMaxTriggerTime);

	m_fNextTriggerTime = g_pServerDE->GetTime() + m_fMinTriggerTime;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::ObjectTouch
//
//	PURPOSE:	Handle object touch
//
// ----------------------------------------------------------------------- //

void Trigger::ObjectTouch(HOBJECT hObj)
{
	HCLASS hClassCharacter = g_pServerDE->GetClass("CCharacter");
	HCLASS hClassObj		   = g_pServerDE->GetObjectClass(hObj);

	// Only AI and players can trigger things...

	CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hObj) );

	if(!pChar)
	{
		// HitBox's are special.
		if( CCharacterHitBox * pCharHitBox = dynamic_cast<CCharacterHitBox*>( g_pLTServer->HandleToObject(hObj) ) )
		{
			// If the character is wall walking, we need to use the character's hit box because
			// wall walkers do not have a touch notify.
			pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject( pCharHitBox->GetModelObject() ) );

			if( !pChar || pChar->GetMovement()->GetMovementState() != CMS_WALLWALKING )
				return;
		}
		else
		{
			// Ignore anything which is not a character or a hitbox.
			return;
		}
	}

	// Don't hit triggers if they're clipping
	if(g_vNoTriggersInClip.GetFloat() && (pChar->GetMovement()->GetMovementState() == CMS_CLIPPING))
		return;


	// An AI is any CCharacter which is not a CPlayerObj.  This includes CAI and CSimpleAI.
	const LTBOOL bIsPlayer = 0 != dynamic_cast<CPlayerObj*>(pChar);

	// If we're AI, make sure we can activate this trigger...
	if( !bIsPlayer )
	{
		if (m_bAITriggerable)
		{
			 // See if only a specific AI can trigger it...
			if (m_hstrAIName)
			{
				char* pAIName  = g_pServerDE->GetStringData(m_hstrAIName);
				char* pObjName = g_pServerDE->GetObjectName(hObj);

				if (pAIName && pObjName)
				{
					if ( stricmp(pAIName, pObjName) != 0 )
					{
						return;
					}
				} 
			}
		}
		else  // Not AI triggerable
		{
			return;
		}
	}
	// If we're the player, make sure we can activate this trigger...
	else
	{
		if (!m_bPlayerTriggerable )
		{
			return;
		}
	}

	// Okay ready to trigger.  Make sure we've waited long enough before triggering...

	DoTrigger(hObj, DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::DoTrigger
//
//	PURPOSE:	Determine if we can be triggered, and if so do it...
//
// ----------------------------------------------------------------------- //

void Trigger::DoTrigger(HOBJECT hObj, DBOOL bTouchNotify)
{
	DFLOAT fTime = g_pServerDE->GetTime();

	if((m_fLastTouchTime != -1.0f) && (fTime < m_fLastTouchTime + m_fTriggerDelay))
		return;

	m_fLastTouchTime = fTime;


	m_bTouchNotifyActivation = bTouchNotify;
	m_hTouchObject = hObj;

	// Make sure we don't create links with ourself...

	if (m_hObject != m_hTouchObject)
	{
		g_pServerDE->CreateInterObjectLink(m_hObject, m_hTouchObject);
	}

	if (m_bActive)
	{
		if (!m_bLocked)
		{
			RequestActivate();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::InitialUpdate
//
//	PURPOSE:	Initial update
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::InitialUpdate()
{
	g_pServerDE->SetObjectDims(m_hObject, &m_vDims);
	g_pServerDE->SetObjectFlags(m_hObject, FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD);
	g_pServerDE->SetObjectUserFlags(m_hObject, USRFLG_IGNORE_PROJECTILES);

	// If I'm not a timed trigger, my object touch notification 
	// will trigger new updates until then, I don't care...
	if (m_bTimedTrigger || m_hstrAttachToObject)
	{
		g_pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
		g_pServerDE->SetDeactivationTime(m_hObject, 0);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
		g_pServerDE->SetDeactivationTime(m_hObject, TRIGGER_DEACTIVATION_TIME);
	}

	// Init console vars
	if(!g_vNoTriggersInClip.IsInitted())
		g_vNoTriggersInClip.Init(g_pLTServer, "NoTriggersInClip", NULL, 0.0f);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::Update()
{
	// Handle timed trigger...

	if (m_bTimedTrigger)
	{
		DFLOAT fTime = g_pServerDE->GetTime();
		if (fTime > m_fNextTriggerTime)
		{
			m_fNextTriggerTime = fTime + GetRandom(m_fMinTriggerTime, m_fMaxTriggerTime);
			DoTrigger(DNULL, DFALSE);
		}
	}

	
	// Attach the trigger to the object...

	if (m_hstrAttachToObject && !m_bAttached)
	{
		AttachToObject();
		m_bAttached = DTRUE;
	}



	if (m_bDelayingActivate)
	{
		UpdateDelayingActivate();
	}
	else
	{
		m_bActive = DTRUE;

		// If not a timed trigger, my object touch notification will trigger 
		// new updates until then, I don't care.
		
		if (m_bTimedTrigger)
		{
			g_pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
			g_pServerDE->SetDeactivationTime(m_hObject, 0);
		}
		else
		{
			g_pServerDE->SetNextUpdate(m_hObject, 0.0f);
			g_pServerDE->SetDeactivationTime(m_hObject, TRIGGER_DEACTIVATION_TIME);
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Unlock
//
//	PURPOSE:	Unlock the trigger and trigger it
//
// ----------------------------------------------------------------------- //

void Trigger::Unlock()
{
	m_bLocked = DFALSE;
	RequestActivate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::RequestActivate
//
//	PURPOSE:	Request activation of the trigger
//
// ----------------------------------------------------------------------- //

void Trigger::RequestActivate()
{
	if (m_bActive)
	{
		// We might not want to activate right away (if SendDelay was > 0)
		// Let Update() determine when to actually activate the trigger...

		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
		g_pServerDE->SetDeactivationTime(m_hObject, 0.0f);
		m_fStartDelayTime = g_pServerDE->GetTime();

		m_bDelayingActivate = DTRUE;
		m_bActive			= DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::UpdateDelayingActivate
//
//	PURPOSE:	Update the delaying (and possibly activate) the trigger
//
// ----------------------------------------------------------------------- //

void Trigger::UpdateDelayingActivate()
{
	if (!m_bDelayingActivate) return;
	
	DFLOAT fTime = g_pServerDE->GetTime();

	if (fTime >= m_fStartDelayTime + m_fSendDelay)
	{
		Activate();
		m_bDelayingActivate = DFALSE;
		m_bActive			= DTRUE;
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, 0.001f);
		g_pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Activate
//
//	PURPOSE:	Activate the trigger.
//
// ----------------------------------------------------------------------- //

DBOOL Trigger::Activate()
{
	// Make us wait a bit before we can be triggered again...

	if (m_bTimedTrigger)
	{
		g_pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
		g_pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	}
	else
	{
		g_pServerDE->SetNextUpdate(m_hObject, m_fTriggerDelay);
		g_pServerDE->SetDeactivationTime(m_hObject, 0.0f);
	}
	
	
	// If this is a counter trigger, determine if we can activate or not...

	if (++m_nCurrentActivation < m_nActivationCount)
	{
		return DFALSE;
	}
	else
	{
		m_nCurrentActivation = 0;
	}


	// Only allow the object to be activated the number of specified times...

	if (m_nNumActivations > 0)
	{
		if (m_nNumTimesActivated >= m_nNumActivations)
		{
			return DFALSE;
		}

		m_nNumTimesActivated++;
	}


	if (m_hstrActivationSound)
	{
		char* pSound = g_pServerDE->GetStringData(m_hstrActivationSound);
		if (pSound && pSound[0] != '\0')
		{
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_HIGH);
		}
	}

	DBOOL bTriggerMsg1 = DTRUE;
	DBOOL bTriggerMsg2 = DTRUE;

	if (m_bWeightedTrigger)
	{
		bTriggerMsg1 = (GetRandom(0.0f, 1.0f) < m_fMessage1Weight ? DTRUE : DFALSE);
		bTriggerMsg2 = !bTriggerMsg1;
	}

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		DBOOL bOkayToSend = DTRUE;

		if (i == 0 && !bTriggerMsg1) bOkayToSend = DFALSE;
		else if (i == 1 && !bTriggerMsg2) bOkayToSend = DFALSE;

		if (bOkayToSend && m_hstrTargetName[i] && m_hstrMessageName[i])
		{
			SendTriggerMsgToObjects(this, m_hstrTargetName[i], m_hstrMessageName[i]);
		}
	}

	if (m_bTouchNotifyActivation && m_hTouchObject && m_bTriggerTouch && m_hstrMessageTouch)
	{
		char szBuffer[256];
		char *szToks[32];
		int nToks = 1;

		strcpy(szBuffer, g_pLTServer->GetStringData(m_hstrMessageTouch));
		szToks[0] = szBuffer;

		// Go through the buffer and setup token pointers
		char *szTemp = szBuffer;

		while(*szTemp)
		{
			if(*szTemp == ';')
			{
				*szTemp = '\0';
				szToks[nToks++] = szTemp + 1;
			}

			szTemp++;
		}

		// Go through the tokens and send a trigger for each of them
		for(int i = 0; i < nToks; i++)
		{
			SendTriggerMsgToObject(this, m_hTouchObject, szToks[i]);
		}

		m_bTouchNotifyActivation = DFALSE;
		m_hTouchObject = DNULL;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::AttachToObject()
//
//	PURPOSE:	Attach the trigger to an object
//
// ----------------------------------------------------------------------- //
	
void Trigger::AttachToObject()
{
	if (!m_hstrAttachToObject) return;

	char* pObjName = g_pServerDE->GetStringData(m_hstrAttachToObject);
	if (!pObjName) return;


	// Find object to attach to...

	HOBJECT hObj = DNULL;
	
	// Kai Martin - 10/26/99
	// replaced linked list of objects in favor of new
	// static array of objects.  
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

	g_pServerDE->FindNamedObjects(pObjName,objArray);

	if(!objArray.NumObjects()) return;

	hObj = objArray.GetObject(0);

	if (!hObj) return;
	
	DVector vOffset;
	VEC_INIT(vOffset);

	DRotation rOffset;
	ROT_INIT(rOffset);

	HATTACHMENT hAttachment;
	g_pServerDE->CreateAttachment(hObj, m_hObject, DNULL, &vOffset, &rOffset, &hAttachment);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Trigger::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	const LTFLOAT fTime = g_pLTServer->GetTime();

	*hWrite << m_bActive;
	*hWrite << m_fTriggerDelay;

	hWrite->WriteHString(m_hstrActivationSound);
	*hWrite << m_fSoundRadius;

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		hWrite->WriteHString(m_hstrTargetName[i]);
		hWrite->WriteHString(m_hstrMessageName[i]);
	}

	*hWrite << m_bTriggerTouch;
	*hWrite << m_bTouchNotifyActivation;
	hWrite->WriteHString(m_hstrMessageTouch);
	*hWrite << m_hTouchObject;

	*hWrite << m_bPlayerTriggerable;
	*hWrite << m_bAITriggerable;
	hWrite->WriteHString(m_hstrAIName);
		
	*hWrite << m_bLocked;

	*hWrite << m_bDelayingActivate;
	hWrite->WriteFloat(m_fStartDelayTime - fTime);
	*hWrite << m_fSendDelay;
	hWrite->WriteFloat(m_fLastTouchTime - fTime);

	*hWrite << m_nActivationCount;
	*hWrite << m_nCurrentActivation;

	*hWrite << m_nNumActivations;
	*hWrite << m_nNumTimesActivated;


	*hWrite << m_bWeightedTrigger;
	*hWrite << m_fMessage1Weight;

	*hWrite << m_bTimedTrigger;
	*hWrite << m_fMinTriggerTime;
	*hWrite << m_fMaxTriggerTime;
	hWrite->WriteFloat(m_fNextTriggerTime - fTime);

	*hWrite << m_bAttached;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Trigger::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	const LTFLOAT fTime = g_pLTServer->GetTime();

	*hRead >> m_bActive;
	*hRead >> m_fTriggerDelay;

	m_hstrActivationSound = hRead->ReadHString();
	*hRead >> m_fSoundRadius;

	for (int i=0; i < MAX_NUM_MESSAGES; i++)
	{
		m_hstrTargetName[i] = hRead->ReadHString();
		m_hstrMessageName[i] = hRead->ReadHString();
	}

	*hRead >> m_bTriggerTouch;
	*hRead >> m_bTouchNotifyActivation;
	m_hstrMessageTouch = hRead->ReadHString();
	*hRead >> m_hTouchObject;

	*hRead >> m_bPlayerTriggerable;
	*hRead >> m_bAITriggerable;
	m_hstrAIName = hRead->ReadHString();
		
	*hRead >> m_bLocked;

	*hRead >> m_bDelayingActivate;
	m_fStartDelayTime = hRead->ReadFloat() + fTime;
	*hRead >> m_fSendDelay;
	m_fLastTouchTime = hRead->ReadFloat() + fTime;

	*hRead >> m_nActivationCount;
	*hRead >> m_nCurrentActivation;

	*hRead >> m_nNumActivations;
	*hRead >> m_nNumTimesActivated;


	*hRead >> m_bWeightedTrigger;
	*hRead >> m_fMessage1Weight;

	*hRead >> m_bTimedTrigger;
	*hRead >> m_fMinTriggerTime;
	*hRead >> m_fMaxTriggerTime;
	m_fNextTriggerTime = hRead->ReadFloat() + fTime;

	*hRead >> m_bAttached;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Trigger::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void Trigger::CacheFiles()
{
	if (m_hstrActivationSound)
	{
		char* pFile = g_pServerDE->GetStringData(m_hstrActivationSound);
		if (pFile)
		{
			g_pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}

