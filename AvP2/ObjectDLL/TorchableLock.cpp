// ----------------------------------------------------------------------- //
//
// MODULE  : TorchableLock.cpp
//
// PURPOSE : Implementation of the Lock object
//
// CREATED : 11/16/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TorchableLock.h"
#include "ServerSoundMgr.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"
#include "CommandMgr.h"
#include "SharedFXStructs.h"
#include "FXButeMgr.h"

// Statics
const uint32 TORCH_HITS_PER_SECOND = 4;

BEGIN_CLASS(TorchableLock)

	ADD_BOOLPROP_FLAG_HELP(StartAsBolt, TRUE, 0, "If true the object will start as a visible lock, else it will be an invisible weld location." )
	ADD_BOOLPROP_FLAG_HELP(CanChangeModes, FALSE, 0, "If true a lock will become a weld location when cut or weld location will become a cutable lock." )
	ADD_REALPROP_FLAG_HELP(WeldTime, 2.0f, 0, "Approximate amount of time the torch must be held to the spot to make a weld.")
	ADD_REALPROP_FLAG_HELP(CutTime, 2.0f, 0, "Approximate amount of time the torch must be held to the spot to cut a lock.")
	ADD_STRINGPROP_FLAG_HELP(OnWeldedCommand, "", 0, "This command string will be caled when TorchableLock is welded closed")
	ADD_STRINGPROP_FLAG_HELP(OnCutCommand, "", 0, "This command string will be caled when the TorchableLock is cut open") 

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
	ADD_STRINGPROP_FLAG(Filename, "models\\props\\bolt.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "skins\\props\\bolt.dtx", PF_FILENAME)
	ADD_STRINGPROP_FLAG_HELP(AltFilename, "models\\props\\weld.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "The alt file name and skin should be used when the lock starts as a bolt and then becomes a weld spot if soldered.  This model will be used for the new weld joint.")
	ADD_STRINGPROP_FLAG(AltSkin, "skins\\props\\weld.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Models/Props/bolt.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins/Props/bolt.dtx", PF_FILENAME)
	ADD_STRINGPROP_FLAG_HELP(AltFilename, "Models/Props/weld.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "The alt file name and skin should be used when the lock starts as a bolt and then becomes a weld spot if soldered.  This model will be used for the new weld joint.")
	ADD_STRINGPROP_FLAG(AltSkin, "Skins/Props/weld.dtx", PF_FILENAME)
#endif

	ADD_VECTORPROP_VAL(AltScale,  1.0f, 1.0f, 1.0f)

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
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
	ADD_STRINGPROP_FLAG(SurfaceOverride, "Metal", PF_STATICLIST)

	// Set our default debris type...
	ADD_STRINGPROP_FLAG(DebrisType, "TorchableLock",PF_STATICLIST)

END_CLASS_DEFAULT(TorchableLock, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::TorchableLock()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

TorchableLock::TorchableLock() : Prop()
{
	m_nCutHitPoints		= 0;
	m_nWeldHitPoints	= 0;
	m_fMaxCutHitPoints	= 0.0f;
	m_fMaxWeldHitPoints	= 0.0f;

	m_eMode = INVALID;

	m_bAllowModeChange	= LTFALSE;

	m_bEnabled = LTTRUE;

	m_hCounterObject = LTNULL;

	m_strLockCmd = "\0";
	m_strOpenCmd = "\0";

	m_bCanDamage = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::~TorchableLock()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

TorchableLock::~TorchableLock()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 TorchableLock::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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

	return Prop::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::HandleDamage
//
//	PURPOSE:	Handle damage message messages
//
// ----------------------------------------------------------------------- //

void TorchableLock::HandleDamage(HOBJECT hSender, HMESSAGEREAD hRead)
{
	DamageStruct damage;
	damage.InitFromMessage(hRead);
	
	if (damage.eType == DT_TORCH_CUT && m_bEnabled)
	{
		//see if we are in the right mode and we have damage to take
		if(m_eMode == BOLT && m_nCutHitPoints != 0)
		{
			//create the FX
			IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX("TorchCutting");
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
			
			
			// Send damage info up to the counter object if it exists
			if(m_hCounterObject.GetHOBJECT() != LTNULL && m_hCounterObject.GetHOBJECT() != hSender)
			{
				SendTriggerMsgToObject(this, m_hCounterObject, "CutDamage");
			}
			
			
			//decrement the damage
			--m_nCutHitPoints;
			
			if (m_nCutHitPoints==0)
			{
				SetupCutState();
			}
		}
	}
	else if (damage.eType == DT_TORCH_WELD  && m_bEnabled)
	{
		//see if we are in the right mode and we have damage to take
		if(m_eMode == WELD && m_nWeldHitPoints != 0)
		{
			//create the FX
			IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX("TorchWelding");
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
			
			
			// Send damage info up to the counter object if it exists
			if(m_hCounterObject.GetHOBJECT() != LTNULL && m_hCounterObject.GetHOBJECT() != hSender)
			{
				SendTriggerMsgToObject(this, m_hCounterObject, "WeldDamage");
			}
			
			
			//decrement the damage
			--m_nWeldHitPoints;
			
			if (m_nWeldHitPoints==0)
			{
				SetupWeldedState();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 TorchableLock::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;
	
	switch(messageID)
	{
	case MID_DAMAGE:
		{
			if(m_bCanDamage)
			{
				HandleDamage(hSender, hRead);
			}
			
			return LTTRUE;
		}
		break;
		
	default : break;
	}
	
	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL TorchableLock::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

	if( g_pServerDE->GetPropGeneric( "StartAsBolt", &genProp ) == DE_OK )
	{
		m_eMode = (genProp.m_Bool == LTTRUE)?BOLT:WELD;
	}

	if( g_pServerDE->GetPropGeneric( "CanChangeModes", &genProp ) == DE_OK )
	{
		m_bAllowModeChange = (genProp.m_Bool == LTTRUE);
	}

	if ( g_pServerDE->GetPropGeneric( "WeldTime", &genProp ) == DE_OK )
	{
		m_fMaxWeldHitPoints = genProp.m_Float;
		m_nWeldHitPoints = (uint32)(m_fMaxWeldHitPoints * TORCH_HITS_PER_SECOND);
	}

	if ( g_pServerDE->GetPropGeneric( "CutTime", &genProp ) == DE_OK )
	{
		m_fMaxCutHitPoints = genProp.m_Float;
		m_nCutHitPoints = (uint32)(m_fMaxCutHitPoints * TORCH_HITS_PER_SECOND);
	}

	if ( g_pServerDE->GetPropGeneric( "DebrisType", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrDebrisType = genProp.m_String;

 	if ( g_pServerDE->GetPropGeneric( "AltFilename", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrAltModel = genProp.m_String;

	if ( g_pServerDE->GetPropGeneric( "AltSkin", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrAltSkin = genProp.m_String;

	if ( g_pServerDE->GetPropGeneric( "AltScale", &genProp ) == DE_OK )
		m_vAltModelScale = genProp.m_Vec;

    if (g_pLTServer->GetPropGeneric("OnWeldedCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strLockCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OnCutCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_strOpenCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

   return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL TorchableLock::InitialUpdate()
{
    g_pLTServer->SetNextUpdate(m_hObject, 0.0);

	// Make sure we can be rayhit even if we are non-solid...
    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

	// Hide the object if we are starting as a weldable spot
	if(m_eMode == WELD)
		dwFlags &= ~FLAG_VISIBLE;

	//make sure we are non-solid
	dwFlags &= ~FLAG_SOLID;

    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_RAYHIT);

	// Set the activation if there is no counter..
	if(m_hCounterObject.GetHOBJECT() == LTNULL)
		SetEnabled(LTTRUE);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::SetupUnlockState
//
//	PURPOSE:	Setup the un locked state
//
// ----------------------------------------------------------------------- //

void TorchableLock::SetupCutState(LTBOOL bForceModeChange)
{
	// Create the gib effect
	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(g_pLTServer->GetStringData(m_hstrDebrisType.HStr()));
	if (pDebris)
	{
		LTRotation rRot;
		LTVector vF, vR, vU;
		LTVector vPos;

		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);
		g_pMathLT->GetRotationVectors(rRot,vR,vU,vF);

		CreatePropDebris(vPos, vF, pDebris->nId);
	}

	// Hide the object
    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);

	//send on unlock commnad string
	if (m_strOpenCmd)
	{
		const char* pCmd = m_strOpenCmd.CStr();

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}

	//always change mode
	m_eMode = WELD;

	// See about switching modes
	if(m_bAllowModeChange || bForceModeChange)
	{
		//reset mode and hps
		m_nWeldHitPoints = (uint32)(m_fMaxWeldHitPoints * TORCH_HITS_PER_SECOND);
		m_bCanDamage = LTTRUE;

		// Send info up to the counter object if it exists
		if(m_hCounterObject.GetHOBJECT() != LTNULL)
		{
			HSTRING hStr = g_pLTServer->CreateString("WeldState");

			HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, m_hCounterObject, MID_TRIGGER);
			g_pLTServer->WriteToMessageHString(hWrite, hStr);
			g_pLTServer->EndMessage(hWrite);

			g_pLTServer->FreeString(hStr);
			hStr = LTNULL;
		}
	}
	else
	{
		SetEnabled(LTFALSE);
		m_bCanDamage = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::SetupUnlockState
//
//	PURPOSE:	Setup the locked state
//
// ----------------------------------------------------------------------- //

void TorchableLock::SetupWeldedState()
{
	// unhide the object
    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);

	// Set our model to the new abc and skins.
	ObjectCreateStruct ocs;
	ocs.Clear();
	SAFE_STRCPY(ocs.m_Filename, m_hstrAltModel.CStr());
	SAFE_STRCPY(ocs.m_SkinNames[0], m_hstrAltSkin.CStr());
	g_pLTServer->Common()->SetObjectFilenames(m_hObject, &ocs);
	g_pLTServer->ScaleObject(m_hObject, &m_vAltModelScale);

	//send on lock command string
	if (m_strLockCmd)
	{
		const char* pCmd = m_strLockCmd.CStr();

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}

	//always change mode
	m_eMode = BOLT;

	// See about switching modes
	if(m_bAllowModeChange)
	{
		//reset mode and hps
		m_nCutHitPoints = (uint32)(m_fMaxCutHitPoints * TORCH_HITS_PER_SECOND);

		// Send info up to the counter object if it exists
		if(m_hCounterObject != LTNULL)
		{
			HSTRING hStr = g_pLTServer->CreateString("CutState");

			HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, m_hCounterObject, MID_TRIGGER);
			g_pLTServer->WriteToMessageHString(hWrite, hStr);
			g_pLTServer->EndMessage(hWrite);

			g_pLTServer->FreeString(hStr);
			hStr = LTNULL;
		}
	}
	else
	{
		SetEnabled(LTFALSE);
		m_bCanDamage = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void TorchableLock::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	*hWrite << m_nCutHitPoints;
	*hWrite << m_nWeldHitPoints;
	*hWrite << m_fMaxCutHitPoints;
	*hWrite << m_fMaxWeldHitPoints;

	*hWrite << m_hstrDebrisType;

	*hWrite << m_hstrAltModel;
	*hWrite << m_hstrAltSkin;
	*hWrite << m_vAltModelScale;

	*hWrite << m_hCounterObject;

	hWrite->WriteWord(m_eMode);
	*hWrite << m_bAllowModeChange;

	*hWrite << m_bEnabled;

	*hWrite << m_strLockCmd;
	*hWrite << m_strOpenCmd;

	*hWrite << m_bCanDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void TorchableLock::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	*hRead >> m_nCutHitPoints;
	*hRead >> m_nWeldHitPoints;
	*hRead >> m_fMaxCutHitPoints;
	*hRead >> m_fMaxWeldHitPoints;

	*hRead >> m_hstrDebrisType;

	*hRead >> m_hstrAltModel;
	*hRead >> m_hstrAltSkin;
	*hRead >> m_vAltModelScale;

	*hRead >> m_hCounterObject;

	m_eMode = TorchMode(hRead->ReadWord());
	*hRead >> m_bAllowModeChange;

	*hRead >> m_bEnabled;

	*hRead >> m_strLockCmd;
	*hRead >> m_strOpenCmd;

	*hRead >> m_bCanDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::Cache
//
//	PURPOSE:	Cache the sounds....
//
// ----------------------------------------------------------------------- //

void TorchableLock::Cache()
{
	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::Cache
//
//	PURPOSE:	Cache the sounds....
//
// ----------------------------------------------------------------------- //

TorchMode TorchableLock::GetMode(LTFLOAT &fRatio)
{
	switch(m_eMode)
	{
		case(WELD):
			fRatio = (LTFLOAT)m_nWeldHitPoints/(m_fMaxWeldHitPoints*TORCH_HITS_PER_SECOND);
			break;

		case(BOLT):
			fRatio = (LTFLOAT)m_nCutHitPoints/(m_fMaxCutHitPoints*TORCH_HITS_PER_SECOND);
			break;

		default:
			fRatio = 0.0f;
			break;
	}

	return m_eMode;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::DestroyBolt
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void TorchableLock::DestroyBolt()
{
	if(m_eMode == BOLT)
	{
		SetupCutState(LTTRUE);
	}
	m_nCutHitPoints = (uint32)(m_fMaxCutHitPoints*TORCH_HITS_PER_SECOND);
	m_nWeldHitPoints = (uint32)(m_fMaxWeldHitPoints*TORCH_HITS_PER_SECOND);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TorchableLock::DestroyBolt
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void TorchableLock::SetEnabled(LTBOOL bOn)
{
	//set bool
	m_bEnabled = bOn;

	if(bOn && m_bCanDamage)
	{
		if(m_eMode == WELD)
		{
			SetActivateType(m_hObject, AT_WELD);
		}
		else
		{
			SetActivateType(m_hObject, AT_CUT);
		}
	}
	else
		SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);
}
