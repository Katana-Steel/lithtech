// ----------------------------------------------------------------------- //
//
// MODULE  : Plant.cpp
//
// PURPOSE : Implementation of the attack plant
//
// CREATED : 3/27/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Plant.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "PlayerObj.h"
#include "Character.h"
#include "Explosion.h"
#include "ServerSoundMgr.h"
#include "AIUtils.h"

// Statics

static char *s_szActivate		= "ACTIVATE";

// ----------------------------------------------------------------------- //
//
//	CLASS:		Plant
//
//	PURPOSE:	An attack plant
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(Plant)

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(Filename, "Models\\Props\\creature_plant.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins\\Props\\creature_plant.dtx", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(Filename, "Models/Props/creature_plant.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Skins/Props/creature_plant.dtx", PF_FILENAME)
#endif

	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(DebrisType, "Plant_Debris", PF_STATICLIST)
	ADD_STRINGPROP_FLAG(SurfaceOverride, "Flesh_Alien", PF_STATICLIST)
	ADD_SHADOW_FLAG(0, 0)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, 0)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)

	ADD_BOOLPROP_FLAG_HELP(IgnoreAI, LTTRUE, 0,			"TRUE attacks players ONLY")
	ADD_REALPROP_FLAG_HELP(DetectionRadius, 500.0f, 0,	"Plant fires a gas cloud when characters enter this range")
	ADD_REALPROP_FLAG_HELP(DamageRadius, 500.0f, 0,		"Area affected by the gas cloud damage")

#ifdef _WIN32
	ADD_STRINGPROP_FLAG(AttackSound, "Sounds\\Events\\plantclose1.wav", PF_FILENAME)
#else
	ADD_STRINGPROP_FLAG(AttackSound, "Sounds/Events/plantclose1.wav", PF_FILENAME)
#endif

	ADD_REALPROP(SoundRadius, 500.0f)

	ADD_BOOLPROP(PlayerUsable, LTFALSE)
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

	ADD_REALPROP_FLAG(Mass, 30.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(HitPoints, 100.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(Armor, 100.0f, PF_GROUP1)
	ADD_REALPROP_FLAG(MaxArmor, 100.0f, PF_GROUP1)
    ADD_BOOLPROP_FLAG(CanHeal, LTTRUE, PF_GROUP1)
    ADD_BOOLPROP_FLAG(CanRepair, LTTRUE, PF_GROUP1)
    ADD_BOOLPROP_FLAG(CanDamage, LTTRUE, PF_GROUP1)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1)
	
	ADD_REALPROP_FLAG(MaxDamage, 25.0f, PF_GROUP2)
	ADD_STRINGPROP_FLAG(DamageType, "Poison", PF_GROUP2)

END_CLASS_DEFAULT(Plant, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::Plant()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Plant::Plant() : Prop ()
{
	m_bPlayerUsable = LTFALSE;
	m_hstrActivateMessage = LTNULL;
	m_hstrActivateTarget = LTNULL;

	m_fDetectionRadiusSqr = 250.0f * 250.0f;
	m_fDamageRadius = 500.0f;
	m_bIgnoreAI = LTTRUE;

	m_fMaxDamage = 10.0f;

	m_hstrAttackSound = LTNULL;

#ifdef _WIN32
	m_sPoisonSound = "Sounds\\Events\\poisongasRelease";
#else
	m_sPoisonSound = "Sounds/Events/poisongasRelease";
#endif
	
	m_fSoundRadius = 500.0f;

	m_damage.SetRemoveOnDeath(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::~Plant()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Plant::~Plant()
{
	if(!g_pInterface)
		return;

	FREE_HSTRING(m_hstrActivateMessage);
	FREE_HSTRING(m_hstrActivateTarget);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Plant::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
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
			InitialUpdate();

			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);
			SetNextUpdate(0.1f, 1.1f);

			CacheFiles();
			return dwRet;
		}

		case MID_UPDATE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Update();

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
//	ROUTINE:	Plant::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Plant::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) 
		return LTFALSE;

	GenericProp genProp;

	if (g_pServerDE->GetPropGeneric("IgnoreAI", &genProp) == DE_OK)
		m_bIgnoreAI = genProp.m_Bool;

	if (g_pServerDE->GetPropGeneric("DetectionRadius", &genProp) == DE_OK)
		m_fDetectionRadiusSqr = genProp.m_Float * genProp.m_Float;

	if (g_pServerDE->GetPropGeneric("DamageRadius", &genProp) == DE_OK)
		m_fDamageRadius = genProp.m_Float;

	// Sound Stuff
	if ( g_pLTServer->GetPropGeneric( "AttackSound", &genProp ) == LT_OK )
	if ( genProp.m_String[0] )
		m_hstrAttackSound = g_pLTServer->CreateString( genProp.m_String );

	if ( g_pLTServer->GetPropGeneric( "SoundRadius", &genProp ) == LT_OK )
		m_fSoundRadius = genProp.m_Float;


	if ( g_pServerDE->GetPropGeneric( "ActivateMessage", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateMessage = g_pServerDE->CreateString( genProp.m_String );

	if ( g_pServerDE->GetPropGeneric( "ActivateTarget", &genProp ) == DE_OK )
	if ( genProp.m_String[0] )
		m_hstrActivateTarget = g_pServerDE->CreateString( genProp.m_String );
	
	if ( g_pServerDE->GetPropGeneric("PlayerUsable", &genProp ) == DE_OK )
		m_bPlayerUsable = genProp.m_Bool;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Plant::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) 
		return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Plant::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
			char* szMsg = hMsg ? g_pServerDE->GetStringData(hMsg) : LTNULL;

			if ( szMsg && !_stricmp(szMsg, s_szActivate) )
			{
				if ( m_bPlayerUsable || g_pServerDE->GetObjectClass(hSender) != g_pServerDE->GetClass("CPlayerObj") )
				{
					// We were triggered, now send our message

					if ( m_hstrActivateTarget && m_hstrActivateMessage )
					{
						SendTriggerMsgToObjects(this, m_hstrActivateTarget, m_hstrActivateMessage);
						FREE_HSTRING(m_hstrActivateMessage);
						FREE_HSTRING(m_hstrActivateTarget);
					}
				}
			}

			g_pServerDE->FreeString(hMsg);

			break;
		}

		case MID_DAMAGE:
		{
			// ignore all poison-based damage
			Explosion *pExp = dynamic_cast<Explosion *>(g_pInterface->HandleToObject(hSender));
			if (pExp)
			{
				if (pExp->GetDamageType() == DT_POISON)
					return 0;
			}

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
//	ROUTINE:	Plant::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Plant::InitialUpdate()
{
	SetupFSM();
	m_RootFSM.SetState(Root::eStateIdle);

	// enable model string keys
	g_pInterface->SetObjectFlags(m_hObject, g_pInterface->GetObjectFlags(m_hObject) | FLAG_MODELKEYS);

	m_damage.SetMass(INFINITE_MASS);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::Update()
//
//	PURPOSE:	Update state machine
//
// ----------------------------------------------------------------------- //

void Plant::Update()
{
	m_RootFSM.Update();
}


// ----------------------------------------------------------------------- //
//   State Machine & related methods
// ----------------------------------------------------------------------- //
void Plant::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateIdle, 
						  m_RootFSM.MakeCallback(*this, &Plant::UpdateIdle),
						  m_RootFSM.MakeCallback(*this, &Plant::StartIdle));

	m_RootFSM.DefineState(Root::eStateOpening,
						  m_RootFSM.MakeCallback(*this, &Plant::UpdateOpening),
						  m_RootFSM.MakeCallback(*this, &Plant::StartOpening));

//	m_RootFSM.DefineState(Root::eStateClosing,
//						  m_RootFSM.MakeCallback(*this, &Plant::UpdateClosing),
//						  m_RootFSM.MakeCallback(*this, &Plant::StartClosing));

	m_RootFSM.DefineState(Root::eStateFiring,
						  m_RootFSM.MakeCallback(*this, &Plant::UpdateFiring),
						  m_RootFSM.MakeCallback(*this, &Plant::StartFiring));

	// Default Transitions.
//	m_RootFSM.DefineTransition(Root::eTargetInvalid, Root::eStateCornered);

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateIdle, Root::eEnemyInRange, Root::eStateFiring);
	m_RootFSM.DefineTransition(Root::eStateFiring, Root::eNoEnemy, Root::eStateOpening);
	m_RootFSM.DefineTransition(Root::eStateFiring, Root::eDoneFiring, Root::eStateOpening);
	m_RootFSM.DefineTransition(Root::eStateOpening, Root::eDoneOpening, Root::eStateIdle);
}


void Plant::StartIdle(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Idle_0");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTTRUE);
}

void Plant::UpdateIdle(RootFSM * fsm)
{
	if (IsEnemyInRange(m_fDetectionRadiusSqr))
	{
		fsm->AddMessage(Root::eEnemyInRange);
		return;
	}
}

void Plant::StartOpening(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Open");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTFALSE);
}

void Plant::UpdateOpening(RootFSM * fsm)
{
	uint32 dwState	= g_pInterface->GetModelPlaybackState(m_hObject);
	if (dwState & MS_PLAYDONE)
	{
		fsm->AddMessage(Root::eDoneOpening);
		return;
	}
}

void Plant::StartFiring(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pInterface->GetAnimIndex(m_hObject, "Fire");

	g_pInterface->SetModelAnimation(m_hObject, hAnim);
	g_pInterface->SetModelLooping(m_hObject, LTFALSE);


	if (m_hstrAttackSound)
	{
		LTVector vPos;
		g_pInterface->GetObjectPos(m_hObject, &vPos);
		char* pSound = g_pLTServer->GetStringData(m_hstrAttackSound);
		g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
	}
}

void Plant::UpdateFiring(RootFSM * fsm)
{
	uint32 dwState = g_pInterface->GetModelPlaybackState(m_hObject);

	if (dwState & MS_PLAYDONE)
	{
		fsm->AddMessage(Root::eDoneFiring);
		return;
	}

	return;
}

void Plant::ReleaseCloud()
{
	if (!m_hObject)
		return;

	ObjectCreateStruct ocs;
	ocs.Clear();
	ocs.m_ObjectType = OT_NORMAL;

#ifdef _DEBUG
	sprintf(ocs.m_Name, "%s-Explosion", g_pLTServer->GetObjectName(m_hObject) );
#endif

	HCLASS hClass = g_pLTServer->GetClass("Explosion");
	Explosion *pExpObj = (Explosion *)g_pInterface->CreateObject(hClass, &ocs);

	IMPACTFX * pImpactFX = g_pFXButeMgr->GetImpactFX((char *)(LPCSTR)"Poison_Gas_Cloud");
	if (pImpactFX)
	{
		m_nFXId = pImpactFX->nId;
	}

	if(pExpObj)
	{
		EXPLOSION_CREATESTRUCT expCS;

		expCS.hFiredFrom = m_hObject;
		expCS.nImpactFXId = m_nFXId;
		expCS.fDamageRadius = m_fDamageRadius;
		expCS.fMaxDamage = m_fMaxDamage*GetDifficultyFactor();
		expCS.eDamageType = DT_POISON;
		g_pLTServer->GetObjectPos(m_hObject, &expCS.vPos);
		expCS.bRemoveWhenDone = LTTRUE;
		expCS.bHideFromShooter = LTFALSE;

		pExpObj->Setup(expCS);
	}

	if (!m_sPoisonSound.empty())
	{
		LTVector vPos;
		g_pInterface->GetObjectPos(m_hObject, &vPos);
		g_pServerSoundMgr->PlaySoundFromPos(vPos, m_sPoisonSound.c_str(), m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
	}

}


void Plant::HandleModelString(ArgList* pArgList)
{
	if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) 
		return;

	char* pKey = pArgList->argv[0];
	if (!pKey) 
		return;

	if (stricmp(pKey, "Fire") == 0)
	{
		ReleaseCloud();
	}

	Prop::HandleModelString(pArgList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Plant::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) 
		return;

	g_pServerDE->WriteToMessageDWord(hWrite, m_bPlayerUsable);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrActivateMessage);
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrActivateTarget);

	*hWrite << m_RootFSM;
	*hWrite << m_fDetectionRadiusSqr;
	*hWrite << m_fDamageRadius;
	*hWrite << m_fMaxDamage;
	*hWrite << m_nFXId;
    g_pInterface->WriteToMessageHString(hWrite, m_hstrAttackSound);
	*hWrite << m_fSoundRadius;
	*hWrite << m_sPoisonSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Plant::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) 
		return;

	m_bPlayerUsable = (LTBOOL)g_pServerDE->ReadFromMessageDWord(hRead);
	m_hstrActivateMessage = g_pServerDE->ReadFromMessageHString(hRead);
	m_hstrActivateTarget = g_pServerDE->ReadFromMessageHString(hRead);

	*hRead >> m_RootFSM;
	*hRead >> m_fDetectionRadiusSqr;
	*hRead >> m_fDamageRadius;
	*hRead >> m_fMaxDamage;
	*hRead >> m_nFXId;
    m_hstrAttackSound = g_pInterface->ReadFromMessageHString(hRead);
	*hRead >> m_fSoundRadius;
	*hRead >> m_sPoisonSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Plant::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void Plant::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) 
		return;

	char* pFile = LTNULL;
	if (m_hstrAttackSound)
	{
		pFile = pServerDE->GetStringData(m_hstrAttackSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
}
