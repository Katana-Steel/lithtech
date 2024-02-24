// ----------------------------------------------------------------------- //
//
// MODULE  : HackableWorldmodel.cpp
//
// PURPOSE : Implementation of the Hackable Lock object
//
// CREATED : 12/6/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HackableWorldmodel.h"
#include "ServerSoundMgr.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"
#include "CommandMgr.h"
#include "SharedFXStructs.h"
#include "FXButeMgr.h"

// Statics
const uint32 HACK_HITS_PER_SECOND = 4;

BEGIN_CLASS(HackableWorldmodel)

	ADD_OBJECTPROP_FLAG_HELP(MainTarget, "", 0, "(NOT REQUIRED) This object (has to be a Door or derived object) will recieve an 'unlock' messages automatically.")
	ADD_BOOLPROP_FLAG_HELP(TriggerWhenUnlocked, LTTRUE, 0, "This tells the lock to send a 'trigger' message to the MainTarget after sending the 'unlock' message.")
	ADD_REALPROP_FLAG_HELP(HackTime, 2.0f, 0, "Approximate amount of time the hacking device must be held to the spot to hack the lock.")
	ADD_STRINGPROP_FLAG_HELP(OnHackedCommand, "", 0, "This command string will be caled when HackableWorldmodel is hacked.")

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)

	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, PF_HIDDEN)
	ADD_REALPROP_FLAG(Mass, 10000, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP1 | PF_HIDDEN)
	PROP_DEFINEGROUP(StateFlags, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(ActivateTrigger, LTFALSE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(StartOpen, LTFALSE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(TriggerClose, LTFALSE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(RemainsOpen, LTFALSE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(ForceMove, LTFALSE, PF_GROUP4 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(BlockLight, LTFALSE, PF_HIDDEN) // Used by pre-processor
    ADD_BOOLPROP_FLAG(BoxPhysics, LTTRUE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Locked, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(IsKeyframed, LTFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Speed, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(MoveDelay, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(MoveDist, 0.0f, PF_HIDDEN)
	ADD_CHROMAKEY_FLAG(FALSE, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PortalName, "", PF_HIDDEN)

	ADD_STRINGPROP_FLAG(OpenSound, "", PF_FILENAME | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CloseSound, "", PF_FILENAME | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(LockedSound, "", PF_FILENAME | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(LoopSounds, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(SoundRadius, 0.0f, PF_RADIUS | PF_HIDDEN)

	ADD_STRINGPROP_FLAG(OpenedCommand, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ClosedCommand, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(LockedCommand, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(OpenWaitTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(CloseWaitTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ClosingSpeed, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP_FLAG(AITriggerable, 0, PF_HIDDEN)
	PROP_DEFINEGROUP(Waveform, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(Linear, LTTRUE, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(Sine, LTFALSE, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(SlowOff, LTFALSE, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(SlowOn, LTFALSE, PF_GROUP5 | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Attachments, "", PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RemoveAttachments, LTTRUE, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(AttachDir, 0.0f, 200.0f, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ShadowLights, "", PF_OBJECTLINK | PF_HIDDEN)
	ADD_LONGINTPROP_FLAG(LightFrames, 1, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DoorLink, "", PF_OBJECTLINK | PF_HIDDEN)

END_CLASS_DEFAULT(HackableWorldmodel, Door, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::HackableWorldmodel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

HackableWorldmodel::HackableWorldmodel() : Door()
{
	m_szMainTarget		= "\0";
	m_hMainTarget		= LTNULL;
	m_nHitPoints		= 0;
	m_fMaxHitPoints		= 0.0f;
	m_bEnabled			= LTTRUE;
	m_bTriggerWhenUnlocked = LTTRUE;
	m_strHackCmd		= "\0";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::~HackableWorldmodel()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

HackableWorldmodel::~HackableWorldmodel()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 HackableWorldmodel::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			Cache();
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				LTBOOL bRval = Door::EngineMessageFn(messageID, pData, fData);
				InitialUpdate();
				SetNextUpdate(0.1f, 1.1f);
				g_pLTServer->SetBlockingPriority(m_hObject, 255);
				return bRval;
			}
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

	return Door::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 HackableWorldmodel::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_DAMAGE:
		{
            uint32 dwRet = LTTRUE;

			DamageStruct damage;
			damage.InitFromMessage(hRead);

			if (damage.eType == DT_MARINE_HACKER && m_bEnabled)
			{
				//see if we have damage to take
				if(m_nHitPoints != 0)
				{
					//create the FX
					IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX("HackingFX");
					if(pFX)
					{
						EXPLOSIONCREATESTRUCT cs;
						cs.nImpactFX = pFX->nId;
						g_pLTServer->GetObjectPos(m_hObject, &cs.vPos);
						g_pLTServer->GetObjectRotation(m_hObject, &cs.rRot);
						cs.fDamageRadius = 0;

						HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&cs.vPos);
						g_pLTServer->WriteToMessageByte(hMessage, SFX_EXPLOSION_ID);
						cs.Write(g_pLTServer, hMessage);
						g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
					}

					//decrement the damage
					--m_nHitPoints;

					if (m_nHitPoints==0)
					{
						SetupUnlockState();
					}
				}
			}

			return dwRet;
		}
		break;

		case MID_TRIGGER:
		{
			//since we derive from door, triggering can cause bad things!
			HSTRING hStr = g_pLTServer->ReadFromMessageHString(hRead);

			ILTCommon* pCommon = g_pLTServer->Common();
			if (!pCommon) return LTTRUE;

			// ConParse does not destroy szMsg, so this is safe
			ConParse parse;
			parse.Init((char*)g_pLTServer->GetStringData(hStr));
			LTBOOL bSupportedMsg = LTFALSE;

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (	_stricmp(parse.m_Args[0], "VISIBLE") == 0 ||
							_stricmp(parse.m_Args[0], "SOLID") == 0   ||
							_stricmp(parse.m_Args[0], "HIDDEN") == 0  ||
							_stricmp(parse.m_Args[0], "REMOVE") == 0  ||
							_stricmp(parse.m_Args[0], "TELEPORT") == 0 )
							bSupportedMsg = LTTRUE;
				}
			}

			g_pLTServer->FreeString(hStr);

			if(!bSupportedMsg)
				return LTTRUE;
			else
				// Make sure other people can read it...
				g_pLTServer->ResetRead(hRead);
		}
		break;

		default : break;
	}

	return Door::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL HackableWorldmodel::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;
	
	GenericProp genProp;
	
    if(g_pLTServer->GetPropGeneric("MainTarget", &genProp) == LT_OK)
	{
		m_szMainTarget = genProp.m_String;
	}

    if(g_pLTServer->GetPropGeneric("TriggerWhenUnlocked", &genProp) == LT_OK)
	{
		m_bTriggerWhenUnlocked = genProp.m_Bool;
	}

	if ( g_pServerDE->GetPropGeneric( "HackTime", &genProp ) == DE_OK )
	{
		m_fMaxHitPoints = genProp.m_Float;
		m_nHitPoints = (uint32)(m_fMaxHitPoints * HACK_HITS_PER_SECOND);
	}
	
    if (g_pLTServer->GetPropGeneric("OnHackedCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strHackCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::Update
//
//	PURPOSE:	Check for updates
//
// ----------------------------------------------------------------------- //

LTBOOL HackableWorldmodel::Update()
{
	// Set the next update
	SetNextUpdate(0.0f);

	ObjArray<HOBJECT,1> objArray;

	if(!m_szMainTarget.IsNull())
	{
		if(LT_OK == g_pLTServer->FindNamedObjects(const_cast<char*>(m_szMainTarget.CStr()), objArray))
		{
			if(objArray.NumObjects())
				m_hMainTarget = objArray.GetObject(0);
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL HackableWorldmodel::InitialUpdate()
{
	SetActivateType(m_hObject, AT_HACK);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::SetupUnlockState
//
//	PURPOSE:	Setup the un locked state
//
// ----------------------------------------------------------------------- //

void HackableWorldmodel::SetupUnlockState()
{
	Door* pDoor = dynamic_cast<Door*>(g_pLTServer->HandleToObject(m_hMainTarget));

	if(pDoor && pDoor->IsLocked())
	{
		SendTriggerMsgToObject(this, pDoor->m_hObject, "Unlock");

		if(m_bTriggerWhenUnlocked)
		{
			SendTriggerMsgToObject(this, pDoor->m_hObject, "Trigger");
		}
	}
	else
	{
		if(m_hMainTarget.GetHOBJECT() && !pDoor)
		{
			// Main target is not a door, just send a trigger message
			SendTriggerMsgToObject(this, m_hMainTarget, "Trigger");
		}
	}

	//send on hack command string
	if (m_strHackCmd)
	{
		const char* pCmd = m_strHackCmd.CStr();

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}

	//be sure to turn off the activate flag
	SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);

	//play the hacked sound
	g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "HackingComplete");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void HackableWorldmodel::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	*hWrite << m_szMainTarget;
	*hWrite << m_hMainTarget;
	*hWrite << m_bTriggerWhenUnlocked;

	*hWrite << m_nHitPoints;
	*hWrite << m_fMaxHitPoints;

	*hWrite << m_bEnabled;
	*hWrite << m_strHackCmd;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void HackableWorldmodel::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	*hRead >> m_szMainTarget;
	*hRead >> m_hMainTarget;
	*hRead >> m_bTriggerWhenUnlocked;

	*hRead >> m_nHitPoints;
	*hRead >> m_fMaxHitPoints;

	*hRead >> m_bEnabled;
	*hRead >> m_strHackCmd;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableWorldmodel::Cache
//
//	PURPOSE:	Cache the sounds....
//
// ----------------------------------------------------------------------- //

void HackableWorldmodel::Cache()
{
	return;
}


