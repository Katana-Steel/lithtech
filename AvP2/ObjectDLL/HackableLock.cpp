// ----------------------------------------------------------------------- //
//
// MODULE  : HackableLock.cpp
//
// PURPOSE : Implementation of the Hackable Lock object
//
// CREATED : 12/6/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HackableLock.h"
#include "ServerSoundMgr.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"
#include "CommandMgr.h"
#include "SharedFXStructs.h"
#include "Door.h"
#include "FXButeMgr.h"

// Statics
const uint32 HACK_HITS_PER_SECOND = 4;

BEGIN_CLASS(HackableLock)

	ADD_OBJECTPROP_FLAG_HELP(MainTarget, "", 0, "(NOT REQUIRED) This object (has to be a Door or derived object) will recieve an 'unlock' messages automatically.")
	ADD_BOOLPROP_FLAG_HELP(TriggerWhenUnlocked, LTTRUE, 0, "This tells the lock to send a 'trigger' message to the MainTarget after sending the 'unlock' message.")
	ADD_REALPROP_FLAG_HELP(HackTime, 2.0f, 0, "Approximate amount of time the hacking device must be held to the spot to hack the lock.")
	ADD_STRINGPROP_FLAG_HELP(OnHackedCommand, "", 0, "This command string will be caled when HackableLock is hacked.")

	// Override base-class properties...
	ADD_REALPROP_FLAG(HitPoints, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxHitPoints, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanHeal, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanRepair, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanDamage, LTTRUE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1 | PF_HIDDEN)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(Filename, "models\\props\\hack_panel.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "skins\\props\\hacking_panel.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Models/Props/Hack_panel.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins/Props/hacking_panel.dtx", PF_FILENAME)
#endif

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)

    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)

	ADD_SHADOW_FLAG(0, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(DetailTexture, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Chrome, LTFALSE, PF_HIDDEN)
	ADD_COLORPROP_FLAG(ObjectColor, 255.0f, 255.0f, 255.0f, PF_HIDDEN)
    ADD_CHROMAKEY_FLAG(LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTTRUE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(TouchSound, "", PF_FILENAME | PF_HIDDEN)
	ADD_REALPROP_FLAG(TouchSoundRadius, 500.0, PF_RADIUS | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DetailLevel, "Low", PF_STATICLIST | PF_HIDDEN)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(SurfaceOverride, "Metal", PF_STATICLIST | PF_HIDDEN)

END_CLASS_DEFAULT(HackableLock, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableLock::HackableLock()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

HackableLock::HackableLock() : Prop()
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
//	ROUTINE:	HackableLock::~HackableLock()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

HackableLock::~HackableLock()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableLock::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 HackableLock::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
				InitialUpdate();
				LTBOOL bRval = Prop::EngineMessageFn(messageID, pData, fData);
				SetNextUpdate(0.1f, 1.1f);
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

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableLock::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 HackableLock::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
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

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableLock::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL HackableLock::ReadProp(ObjectCreateStruct *pInfo)
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
//	ROUTINE:	HackableLock::Update
//
//	PURPOSE:	Check for updates
//
// ----------------------------------------------------------------------- //

LTBOOL HackableLock::Update()
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
//	ROUTINE:	HackableLock::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL HackableLock::InitialUpdate()
{
	// Make sure we can be rayhit even if we are non-solid...
    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

	//make sure we are non-solid
	dwFlags &= ~FLAG_SOLID;

    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_RAYHIT);

	SetActivateType(m_hObject, AT_HACK);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HackableLock::SetupUnlockState
//
//	PURPOSE:	Setup the un locked state
//
// ----------------------------------------------------------------------- //

void HackableLock::SetupUnlockState()
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
//	ROUTINE:	HackableLock::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void HackableLock::Save(HMESSAGEWRITE hWrite)
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
//	ROUTINE:	HackableLock::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void HackableLock::Load(HMESSAGEREAD hRead)
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
//	ROUTINE:	HackableLock::Cache
//
//	PURPOSE:	Cache the sounds....
//
// ----------------------------------------------------------------------- //

void HackableLock::Cache()
{
	return;
}


