// ----------------------------------------------------------------------- //
//
// MODULE  : Egg.cpp
//
// PURPOSE : Implementation of the attack plant
//
// CREATED : 3/27/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Egg.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "CharacterMgr.h"
#include "ObjectMsgs.h"
#include "PlayerObj.h"
#include "Character.h"
#include "ServerSoundMgr.h"
#include "SpawnButeMgr.h"
#include "AIUtils.h"
#include "SimpleAI.h"


// Statics

static char *s_szActivate		= "ACTIVATE";

// ----------------------------------------------------------------------- //
//
//	CLASS:		Egg
//
//	PURPOSE:	Hatches when player gets in range.  Spawns a facehugger.
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(Egg)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(Filename, "Models\\Props\\egg.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins\\Props\\egg.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Models/Props/egg.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins/Props/egg.dtx", PF_FILENAME)
#endif
	
	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DebrisType, "Egg_Debris", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SurfaceOverride, "Flesh_Alien", PF_STATICLIST)
	ADD_SHADOW_FLAG(0, 0)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, 0)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)

	ADD_BOOLPROP_FLAG_HELP(UseSimpleAI, LTFALSE, 0, "TRUE means this egg will spawn a SimpleAIFacehugger instead of an AIFacehugger")
	ADD_BOOLPROP_FLAG_HELP(IgnoreAI, LTTRUE, 0,			"TRUE is activated by players ONLY")
	ADD_REALPROP_FLAG_HELP(DetectionRadius, 500.0f, 0,	"Egg begins to hatch when someone enters this radius")
	ADD_REALPROP_FLAG_HELP(WarningTime, 1.0f, 0, "If player does not leave the detection radius in this time, the egg will hatch")

	ADD_BOOLPROP_FLAG(PlayerUsable, LTFALSE, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ActivateTarget, LTFALSE, PF_OBJECTLINK)
	ADD_STRINGPROP(ActivateMessage, LTFALSE)

    ADD_BOOLPROP_FLAG(DetailTexture, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Chrome, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_CHROMAKEY_FLAG(LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTTRUE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(TouchSound, "", PF_FILENAME | PF_HIDDEN)
	ADD_REALPROP_FLAG(TouchSoundRadius, 500.0, PF_RADIUS | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DetailLevel, "Low", PF_STATICLIST)

	ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1)

END_CLASS_DEFAULT(Egg, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::Egg()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Egg::Egg() : Prop ()
{
	m_bUseSimpleAI = LTFALSE;
	m_bPlayerUsable = LTFALSE;
	m_hstrActivateMessage = LTNULL;
	m_hstrActivateTarget = LTNULL;

	m_fDetectionRadiusSqr = 250.0f * 250.0f;
	m_fWarningTime = 3.0f;
	m_bIgnoreAI = LTTRUE;

	m_fSoundRadius = 1000.0f;

	m_hTarget = LTNULL;

#ifdef _WIN32
	m_strOpenSound = "Sounds\\Events\\EggOpen1.wav";
	m_strWarningSound = "Sounds\\Events\\EggShake.wav";
#else
	m_strOpenSound = "Sounds/Events/EggOpen1.wav";
	m_strWarningSound = "Sounds/Events/EggShake.wav";
#endif

	m_hWarningSound = LTNULL;

	m_damage.SetRemoveOnDeath(LTTRUE);

	SetupFSM();
	m_RootFSM.SetState(Root::eStateIdle);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::~Egg()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Egg::~Egg()
{
	if(!g_pInterface)
		return;

	if (m_hWarningSound)
		g_pLTServer->KillSound(m_hWarningSound);

	FREE_HSTRING(m_hstrActivateMessage);
	FREE_HSTRING(m_hstrActivateTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Egg::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);
			SetNextUpdate(0.1f, 1.1f);

			uint32 dwFlags2;
			g_pInterface->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags2);
			g_pInterface->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags2 | FLAG2_SEMISOLID);

			CacheFiles();
			return dwRet;
		}

		case MID_UPDATE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Update();

			if (m_RootFSM.GetState() != Root::eStateDone)
				SetNextUpdate(0.1f);
	
			return dwRet;

			break;
		}

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
			break;
		}

		default: 
			break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Egg::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) 
		return LTFALSE;

	GenericProp genProp;

	// Egg stuff
	if (g_pLTServer->GetPropGeneric("UseSimpleAI", &genProp) == LT_OK)
		m_bUseSimpleAI = genProp.m_Bool;

	if (g_pLTServer->GetPropGeneric("IgnoreAI", &genProp) == LT_OK)
		m_bIgnoreAI = genProp.m_Bool;

	if (g_pLTServer->GetPropGeneric("DetectionRadius", &genProp) == LT_OK)
		m_fDetectionRadiusSqr = genProp.m_Float * genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("WarningTime", &genProp) == LT_OK)
		m_fWarningTime = genProp.m_Float * genProp.m_Float;


	// Prop stuff
	if ( g_pLTServer->GetPropGeneric( "ActivateMessage", &genProp ) == LT_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateMessage = g_pLTServer->CreateString( genProp.m_String );

	if ( g_pLTServer->GetPropGeneric( "ActivateTarget", &genProp ) == LT_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateTarget = g_pLTServer->CreateString( genProp.m_String );
	
	if ( g_pLTServer->GetPropGeneric("PlayerUsable", &genProp ) == LT_OK )
		m_bPlayerUsable = genProp.m_Bool;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Egg::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) 
		return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Egg::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pLTServer) 
		return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			const LTBOOL bDone = HandleTrigger(hSender, hMsg);
			g_pLTServer->FreeString(hMsg);

			if (bDone)
				return 0;
		}

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

			DDWORD dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if ( m_damage.IsDead() )
			{
				// handle death
			}

			// TODO: Check to see if we have been disabled

			return dwRet;
		}

		default:
		{
			break;
		}
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::HandleTrigger()
//
//	PURPOSE:	Process trigger messages
//
// ----------------------------------------------------------------------- //

LTBOOL Egg::HandleTrigger(HOBJECT hSender, HSTRING hMsg)
{
	char* szMsg = hMsg ? g_pLTServer->GetStringData(hMsg) : LTNULL;
	
	if (szMsg && !_stricmp(szMsg, s_szActivate))
	{
		if (m_bPlayerUsable || g_pLTServer->GetObjectClass(hSender) != g_pLTServer->GetClass("CPlayerObj"))
		{
			// We were triggered, now send our message
			
			if (m_hstrActivateTarget && m_hstrActivateMessage)
			{
				SendTriggerMsgToObjects(this, m_hstrActivateTarget, m_hstrActivateMessage);
				FREE_HSTRING(m_hstrActivateMessage);
				FREE_HSTRING(m_hstrActivateTarget);
				return LTTRUE;
			}
		}
	}
	
	if (szMsg && (!_stricmp(szMsg, "TRIGGER")))
	{
		CCharacter *pChar;
		pChar = IsEnemyInRange(3000.0f * 3000.0f);
		
		if (pChar)
		{
			m_hTarget = pChar->m_hObject;
			m_vTargetPos = pChar->GetPosition();
			m_RootFSM.SetNextState(Root::eStateOpening);
			return LTTRUE;
		}
	}
	
	ConParse parse;
	parse.Init(const_cast<char*>(szMsg));

	CommonLT* pCommon = g_pLTServer->Common();
	while (pCommon && (pCommon->Parse(&parse) == LT_OK))
	{
		if( parse.m_nArgs == 2 )
		{
			if(_stricmp(parse.m_Args[0], "SDR") == 0)
			{
				const LTFLOAT fRadius = (LTFLOAT)atof(parse.m_Args[1]);
				m_fDetectionRadiusSqr = fRadius * fRadius;
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Egg::InitialUpdate()
{
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::Update()
//
//	PURPOSE:	Update state machine
//
// ----------------------------------------------------------------------- //

void Egg::Update()
{
	m_RootFSM.Update();
}


// ----------------------------------------------------------------------- //
//   State Machine & related methods
// ----------------------------------------------------------------------- //
void Egg::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateIdle, 
						  m_RootFSM.MakeCallback(*this, &Egg::UpdateIdle),
						  m_RootFSM.MakeCallback(*this, &Egg::StartIdle));

	m_RootFSM.DefineState(Root::eStateWarning,
						  m_RootFSM.MakeCallback(*this, &Egg::UpdateWarning),
						  m_RootFSM.MakeCallback(*this, &Egg::StartWarning));

	m_RootFSM.DefineState(Root::eStateOpening,
						  m_RootFSM.MakeCallback(*this, &Egg::UpdateOpening),
						  m_RootFSM.MakeCallback(*this, &Egg::StartOpening));

	m_RootFSM.DefineState(Root::eStateDone,
						  m_RootFSM.MakeCallback(*this, &Egg::UpdateDone),
						  m_RootFSM.MakeCallback(*this, &Egg::StartDone));

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateIdle, Root::eEnemyInRange, Root::eStateWarning);
	m_RootFSM.DefineTransition(Root::eStateWarning, Root::eEnemyInRange, Root::eStateOpening);
	m_RootFSM.DefineTransition(Root::eStateWarning, Root::eEnemyOutOfRange, Root::eStateIdle);
	m_RootFSM.DefineTransition(Root::eStateOpening, Root::eDoneOpening, Root::eStateDone);
}


void Egg::StartIdle(RootFSM * fsm)
{
	// TODO: stop looping sound here, if present
	if (m_hWarningSound)
	{
		g_pLTServer->KillSound(m_hWarningSound);
		m_hWarningSound = LTNULL;
	}

	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "start");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTFALSE);
}

void Egg::UpdateIdle(RootFSM * fsm)
{
	CCharacter *pChar;

	pChar = IsEnemyInRange(m_fDetectionRadiusSqr);
	if (pChar)
	{
		fsm->AddMessage(Root::eEnemyInRange);
		return;
	}
}

void Egg::StartWarning(RootFSM * fsm)
{
	m_tmrWarning.Start(m_fWarningTime);

	// TODO: play warning sound here
	m_hWarningSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, const_cast<char *>(m_strWarningSound.c_str()), m_fSoundRadius, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
}

void Egg::UpdateWarning(RootFSM * fsm)
{
	CCharacter *pChar;
	pChar = IsEnemyInRange(m_fDetectionRadiusSqr);

	if (pChar)
	{
		if (m_tmrWarning.Stopped())
		{
			m_hTarget = pChar->m_hObject;
			m_vTargetPos = pChar->GetPosition();
			fsm->AddMessage(Root::eEnemyInRange);
			return;
		}
	}
	else
	{
		fsm->AddMessage(Root::eEnemyOutOfRange);
		return;
	}
}

void Egg::StartOpening(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "open");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTFALSE);

	// Kill Warning sound if present
	if (m_hWarningSound)
	{
		g_pLTServer->KillSound(m_hWarningSound);
		m_hWarningSound = LTNULL;
	}

	// Play spawn sound...
	g_pServerSoundMgr->PlaySoundFromObject(m_hObject, const_cast<char *>(m_strOpenSound.c_str()), m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
}

void Egg::UpdateOpening(RootFSM * fsm)
{
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);
	if (dwState & MS_PLAYDONE)
	{
		fsm->AddMessage(Root::eDoneOpening);
		return;
	}
}

void Egg::StartDone(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "end");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTFALSE);

	// Spawn Facehugger here
	LTBOOL bDone = SpawnFacehugger();
	
#ifdef DEBUG
	if (!bDone)
		g_pInterface->CPrint("Egg failed to spawn Facehugger");
#endif
}

void Egg::UpdateDone(RootFSM * fsm)
{
	return;
}


LTBOOL Egg::SpawnFacehugger()
{
	LTVector vPos, vDims, vUp, vRight, vForward;
	LTRotation rRot;
	SpawnButes butes;

	if (m_bUseSimpleAI)
		butes = g_pSpawnButeMgr->GetSpawnButes("SimpleAI_Facehugger");
	else
		butes = g_pSpawnButeMgr->GetSpawnButes("AI_Facehugger");

	g_pInterface->GetObjectPos(m_hObject, &vPos);
	g_pInterface->GetObjectRotation(m_hObject, &rRot);
	g_pInterface->GetObjectDims(m_hObject, &vDims);

	// Determine egg's orientation
	g_pMathLT->GetRotationVectors(rRot, vRight, vUp, vForward);
	
	HCLASS hClass = g_pLTServer->GetClass(butes.m_szClass.GetBuffer(0));
	if (!hClass) 
		return LTNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-%s", g_pLTServer->GetObjectName(m_hObject), m_bUseSimpleAI ? "SimpleAI_Facehugger" : "AI_Facehugger" );
#endif

	// Update target's position if the target still exists
	if (m_hTarget)
		g_pInterface->GetObjectPos(m_hTarget, &m_vTargetPos);

	vPos += vUp * (vDims.y * 2.0f);  // move spawned object to the top of the egg
	theStruct.m_Pos = vPos;

	// Aim the facehugger at his target
	LTVector vDir;
	vDir = m_vTargetPos - vPos;
	vDir -= vUp * vUp.Dot(vDir);
	g_pMathLT->AlignRotation(rRot, vDir, vUp);

	theStruct.m_Rotation = rRot;

	// Allocate an object...
	BaseClass * pObject = g_pInterface->CreateObjectProps(hClass, &theStruct, butes.m_szProps.GetBuffer(0));
	if (pObject)
	{
		LTVector vVel = m_vTargetPos - vPos;
		vVel += (vUp * 400.0f);
		g_pInterface->SetVelocity(pObject->m_hObject, &vVel);
	}

	if (m_bUseSimpleAI)
	{
		((CSimpleAI *)pObject)->SetTarget(m_hTarget);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Egg::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) 
		return;

	*hWrite << m_bUseSimpleAI;

	g_pInterface->WriteToMessageDWord(hWrite, m_bPlayerUsable);
	g_pInterface->WriteToMessageHString(hWrite, m_hstrActivateMessage);
	g_pInterface->WriteToMessageHString(hWrite, m_hstrActivateTarget);

	*hWrite << m_fDetectionRadiusSqr;
	*hWrite << m_fSoundRadius;
	*hWrite << m_vTargetPos;
	*hWrite << m_RootFSM;
	*hWrite << m_hTarget;
	*hWrite << m_strOpenSound;
	*hWrite << m_strWarningSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Egg::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) 
		return;

	*hRead >> m_bUseSimpleAI;

	m_bPlayerUsable = (LTBOOL)g_pInterface->ReadFromMessageDWord(hRead);
	m_hstrActivateMessage = g_pInterface->ReadFromMessageHString(hRead);
	m_hstrActivateTarget = g_pInterface->ReadFromMessageHString(hRead);

	*hRead >> m_fDetectionRadiusSqr;
	*hRead >> m_fSoundRadius;
	*hRead >> m_vTargetPos;
	*hRead >> m_RootFSM;
	*hRead >> m_hTarget;
	*hRead >> m_strOpenSound;
	*hRead >> m_strWarningSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Egg::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void Egg::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) 
		return;

	if (!m_strOpenSound.empty())
	{
		pServerDE->CacheFile(FT_SOUND, const_cast<char *>(m_strOpenSound.c_str()));
	}

	if (!m_strWarningSound.empty())
	{
		pServerDE->CacheFile(FT_SOUND, const_cast<char *>(m_strWarningSound.c_str()));
	}
}
