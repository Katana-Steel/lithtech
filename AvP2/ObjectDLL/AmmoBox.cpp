// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.cpp
//
// PURPOSE : Implementation of the Ammo Box object
//
// CREATED : 2/5/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AmmoBox.h"
#include "ServerSoundMgr.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"
#include "CommandMgr.h"
#include "SharedFXStructs.h"
#include "CharacterFuncs.h"
#include "CharacterHitBox.h"
#include "PlayerObj.h"

// Statics
const uint32 TORCH_HITS_PER_SECOND = 4;

BEGIN_CLASS(AmmoBox)

	ADD_STRINGPROP_FLAG_HELP(OnOpenCommand, "", 0, "This command string will be caled when the Ammo Box is opened.") 

	// Override base-class properties...
	ADD_REALPROP_FLAG(HitPoints, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxHitPoints, 5.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanHeal, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanRepair, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanDamage, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP1 | PF_HIDDEN)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(Filename, "Models\\Props\\ammobox.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins\\Props\\ammobox.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Models/Props/ammobox.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins/Props/ammobox.dtx", PF_FILENAME)
#endif
	

    ADD_BOOLPROP_FLAG_HELP(CanActivate, LTTRUE, 0, "Can the box be opened by being activated?")
    ADD_BOOLPROP_FLAG_HELP(CanTouchActivate, LTTRUE, 0, "Can the box be opened by being touched?")
    ADD_BOOLPROP_FLAG_HELP(StartLocked, LTFALSE, 0, "Should the box start locked?")
    ADD_BOOLPROP_FLAG_HELP(HumanOnly, LTTRUE, 0, "True=Human PickUp Box, False=Predator.")
	ADD_REALPROP_FLAG_HELP(RespawnTime, -1.0f, 0, "Any value less then or equal to 0/n will prevent the box from respawning.")

	ADD_STRINGPROP_FLAG_HELP(PickUp1, "", PF_STATICLIST, "Pickup that will be given to the player when the box is opened.")
	ADD_STRINGPROP_FLAG(PickUp2, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp3, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp4, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp5, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp6, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp7, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp8, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp9, "", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(PickUp10, "", PF_STATICLIST)

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)

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
	ADD_STRINGPROP_FLAG(SurfaceOverride, "", PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS_PLUGIN(AmmoBox, Prop, NULL, NULL, 0, CAmmoBoxPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::AmmoBox()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

AmmoBox::AmmoBox()
{
	m_bOpened		= LTFALSE;
	m_bLocked		= LTFALSE;
	m_bCanActivate	= LTFALSE;
	m_bCanTouch		= LTFALSE;
	m_bHumanOnly	= LTFALSE;
	m_fRespawnTime	= 0.0f;
	m_fRespawnDelay	= 0.0f;


	for(int i=0; i<MAX_PICKUPS ; i++)
		m_strPickupCommand[i] = "\0";

	m_strOpenCmd = "\0";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::~AmmoBox()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

AmmoBox::~AmmoBox()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_INITIALUPDATE :
		{
			Prop::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
			return LTTRUE;
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			// don't pick up hidden ammoboxes
			const DWORD dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			if (!(dwFlags & FLAG_VISIBLE))
				break;

			if(!m_bOpened && m_bCanTouch && !m_bLocked)
				SetupOpenState((HOBJECT)pData);
			break;
		}

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
//	ROUTINE:	AmmoBox::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;
	
	switch(messageID)
	{
	case MID_TRIGGER:
		{
            HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			TriggerMsg(hSender, hMsg);
            g_pLTServer->FreeString(hMsg);
		}
		break;
	
	default : break;
	}
	
	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::TriggerMsg()
//
//	PURPOSE:	Handler for door trigger messages.
//
// --------------------------------------------------------------------------- //

void AmmoBox::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
    char* pMsg = g_pLTServer->GetStringData(hMsg);
	if (!pMsg) return;

	ConParse parse;
	parse.Init(pMsg);

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "ACTIVATE") == 0)
			{
				// If we are activatable then try to open
				if(GetActivateType(m_hObject) != AT_NOT_ACTIVATABLE)
					SetupOpenState(hSender);
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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL AmmoBox::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;
	
	GenericProp genProp;
	char szProp[128];

    if (g_pLTServer->GetPropGeneric("OnOpenCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strOpenCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	for(int i = 0; i < MAX_PICKUPS; i++)
	{
		sprintf(szProp, "PickUp%d", i+1);

        if(g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
				PickupButes butes = g_pPickupButeMgr->GetPickupButes(genProp.m_String);

				m_strPickupCommand[i] = g_pLTServer->CreateString(butes.m_szMessage);
			}
		}
	}
	
	if( g_pServerDE->GetPropGeneric( "CanActivate", &genProp ) == LT_OK )
	{
		m_bCanActivate = (genProp.m_Bool == LTTRUE);
	}

	if( g_pServerDE->GetPropGeneric( "CanTouchActivate", &genProp ) == LT_OK )
	{
		m_bCanTouch = (genProp.m_Bool == LTTRUE);
	}

	if( g_pServerDE->GetPropGeneric( "StartLocked", &genProp ) == LT_OK )
	{
		m_bLocked = (genProp.m_Bool == LTTRUE);
	}

	if( g_pServerDE->GetPropGeneric( "HumanOnly", &genProp ) == LT_OK )
	{
		m_bHumanOnly = (genProp.m_Bool == LTTRUE);
	}

	if( g_pServerDE->GetPropGeneric( "RespawnTime", &genProp ) == LT_OK )
	{
		m_fRespawnDelay = genProp.m_Float;
	}
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL AmmoBox::InitialUpdate()
{
    SetNextUpdate(0.001f);

	//set the activation accordingly
	if(m_bCanActivate)
		SetActivateType(m_hObject, m_bHumanOnly?AT_MARINE_PICKUP:AT_PRED_PICKUP);
	else
		SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	
	// Make us non-solid
	dwFlags &= ~FLAG_SOLID;

	g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	// Move us to the floor here.
	// We have to wait until the first MID_UPDATE, because some
	// objects may not have loaded yet, like worldmodels.
	SetMoveToFloor( TRUE );

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Update
//
//	PURPOSE:	Do stuff
//
// ----------------------------------------------------------------------- //

LTBOOL AmmoBox::Update()
{
	// Set the next update to almost immediately
	g_pLTServer->SetNextUpdate(m_hObject, 0.1f);

	// See if it's time to respawn
	if((m_fRespawnTime > 0.0f) && (g_pLTServer->GetTime() >= m_fRespawnTime))
	{
		// Reset the respawn time and open state...
		m_fRespawnTime = 0.0f;
		m_bOpened = LTFALSE;

		//set the activation accordingly
		if(m_bCanActivate)
			SetActivateType(m_hObject, m_bHumanOnly?AT_MARINE_PICKUP:AT_PRED_PICKUP);
		else
			SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);

		//play the open animation
		HMODELANIM nAni	= g_pLTServer->GetAnimIndex(m_hObject, "closing");
		g_pLTServer->SetModelAnimation(m_hObject, nAni);
		g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
		g_pLTServer->ResetModelAnimation(m_hObject);  // Start from beginning

		//play the open sound
		g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "ammo_close");

	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::SetupOpenState
//
//	PURPOSE:	Setup the Opened state
//
// ----------------------------------------------------------------------- //

void AmmoBox::SetupOpenState(HOBJECT hObj)
{
	if (IsCharacterHitBox(hObj))
	{
		CCharacterHitBox* pHitBox = LTNULL;
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		hObj = pHitBox->GetModelObject();
	}

	//see that we are being activated by a player
	if(!IsPlayer(hObj))	return;

	// [KLS] 9/23/01 Make sure the player is alive...
	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObj);
	if (!pPlayer || pPlayer->GetState() != PS_ALIVE) return;


	if(IsAlien(hObj) || IsExosuit(hObj)) return;
	else if(IsHuman(hObj) && !m_bHumanOnly) return;
	else if(IsPredator(hObj) && m_bHumanOnly) return;

	if(!m_bLocked)
	{
		//send on unlock commnad string
		if (m_strOpenCmd)
		{
			const char* pCmd = m_strOpenCmd.CStr();

			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd);
			}
		}

		//set open flag
		m_bOpened = LTTRUE;

		//turn off activation flag
		SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);

		//send pickup messages

		for(int i = 0; i < MAX_PICKUPS; i++)
		{
			if(!m_strPickupCommand[i].IsNull())
			{
				// Send the message to the character who touched the pickup
				HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, hObj, MID_PICKUP);
				g_pLTServer->WriteToMessageString(hWrite, const_cast<char *>(m_strPickupCommand[i].CStr()));
				g_pLTServer->WriteToMessageDWord(hWrite, 0);
				g_pLTServer->WriteToMessageDWord(hWrite, 0);
				g_pLTServer->WriteToMessageDWord(hWrite, 0);
				g_pLTServer->WriteToMessageDWord(hWrite, 0);
				g_pLTServer->EndMessage(hWrite);
			}
		}

		//play the open animation
		HMODELANIM nAni	= g_pLTServer->GetAnimIndex(m_hObject, "opening");
		g_pLTServer->SetModelAnimation(m_hObject, nAni);
		g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
		g_pLTServer->ResetModelAnimation(m_hObject);  // Start from beginning

		//play the open sound
		g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "ammo_open");

		// Set the respawn info...
		if(m_fRespawnDelay > 0.0f)
			m_fRespawnTime = g_pLTServer->GetTime() + m_fRespawnDelay;
	}
	else
	{
        LTBOOL bIsDone=LTTRUE;

		//play the failed to open sound
		g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "ammo_locked");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	*hWrite << m_bOpened;
	*hWrite << m_bLocked;
	*hWrite << m_bCanActivate;
	*hWrite << m_bCanTouch;
	*hWrite << m_bHumanOnly;
	hWrite->WriteFloat(m_fRespawnTime - g_pLTServer->GetTime());
	*hWrite << m_fRespawnDelay;

	for(int i = 0; i < MAX_PICKUPS; i++)
		*hWrite << m_strPickupCommand[i];

	*hWrite << m_strOpenCmd;


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	*hRead >> m_bOpened;
	*hRead >> m_bLocked;
	*hRead >> m_bCanActivate;
	*hRead >> m_bCanTouch;
	*hRead >> m_bHumanOnly;
	m_fRespawnTime = hRead->ReadFloat() - g_pLTServer->GetTime();
	*hRead >> m_fRespawnDelay;

	for(int i = 0; i < MAX_PICKUPS; i++)
		*hRead >> m_strPickupCommand[i];

	*hRead >> m_strOpenCmd;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAmmoBoxPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CAmmoBoxPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	m_PickupButeMgrPlugin.PreHook_EditStringList(szRezPath, "Pickup",
				aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

	m_PickupButeMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

	return LT_OK;
}

