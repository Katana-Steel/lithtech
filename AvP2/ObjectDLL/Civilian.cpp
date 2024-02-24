// ----------------------------------------------------------------------- //
//
// MODULE  : Civilian.cpp
//
// PURPOSE : A minimal civilian object
//
// CREATED : 12/21/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Civilian.h"
#include "cpp_server_de.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "ServerSoundMgr.h"
#include "BodyProp.h"
#include "AIUtils.h" // (std::string Load/Save)
#include "SharedFXStructs.h"
#include "FXButeMgr.h"
#include "CharacterMgr.h"

// Statics
static char *s_szActivate		= "ACTIVATE";

// ----------------------------------------------------------------------- //
//
//	CLASS:		Civilian
//
//	PURPOSE:	Idles.  Cowers when weapon fire is detected.
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(Civilian)

	ADD_STRINGPROP_FLAG(SurfaceOverride, "Flesh_Human", PF_STATICLIST)

	ADD_STRINGPROP_FLAG_HELP(CharacterType, "Harris", PF_STATICLIST, "Type of character." )
	ADD_STRINGPROP_FLAG_HELP(IdleAni, "Idle_0", 0, "Idle animation for this civilian")
	ADD_STRINGPROP_FLAG_HELP(CowerAni, "Stand_Cower", 0, "Cower animation")
	ADD_STRINGPROP_FLAG_HELP(DeathAni, "Death", 0, "Death animation")
	ADD_REALPROP_FLAG_HELP(CowerTime, 10.0f, 0, "Length of time to cower from a single stimulus")

	ADD_SHADOW_FLAG(0, 0)
	ADD_BOOLPROP_FLAG(MoveToFloor, LTTRUE, 0)
	ADD_BOOLPROP_FLAG_HELP(IgnoreAI, LTFALSE, 0, "TRUE is activated by players ONLY")

	ADD_REALPROP_FLAG_HELP(FearAlien, 1000.0f, PF_RADIUS, "Will cower if alien gets within this range.")
	ADD_REALPROP_FLAG_HELP(FearPredator, 1000.0f, PF_RADIUS, "Will cower if predator gets within this range.")
	ADD_REALPROP_FLAG_HELP(FearMarine, 0.0f, PF_RADIUS, "Will cower if marine gets within this range.")
	ADD_REALPROP_FLAG_HELP(FearCorporate, 0.0f, PF_RADIUS, "Will cower if corporate gets within this range.")

	ADD_STRINGPROP_FLAG( CowerSound, "", PF_FILENAME )
	ADD_REALPROP( SoundRadius, 1000.0f )

	ADD_STRINGPROP_FLAG(ActivateTarget, LTFALSE, PF_OBJECTLINK)
	ADD_STRINGPROP(ActivateMessage, LTFALSE)

	ADD_STRINGPROP_FLAG(DetailLevel, "Low", PF_STATICLIST | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1)
	ADD_BOOLPROP_FLAG(PlayerUsable, LTTRUE, 0)
	ADD_BOOLPROP_FLAG_HELP(EnableCower, LTTRUE, 0, "If TRUE, Civilian will cower when weapon fire is heard")

	//---------------
	// Hidden Fields
	//---------------
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_HIDDEN )
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN )
	ADD_STRINGPROP_FLAG(DebrisType, "", PF_STATICLIST | PF_HIDDEN )

	ADD_VECTORPROP_VAL_FLAG(Scale, 1.0f, 1.0f, 1.0f, PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)

    ADD_BOOLPROP_FLAG(DetailTexture, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Chrome, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_CHROMAKEY_FLAG(LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RayHit, LTTRUE, PF_HIDDEN)

	ADD_STRINGPROP_FLAG(TouchSound, "", PF_FILENAME | PF_HIDDEN)
	ADD_REALPROP_FLAG(TouchSoundRadius, 500.0, PF_RADIUS | PF_HIDDEN)


END_CLASS_DEFAULT_FLAGS_PLUGIN(Civilian, Prop, LTNULL, LTNULL, 0, CivilianPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::Civilian()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Civilian::Civilian() : Prop ()
{
	SetupFSM();

	m_bPlayerUsable = LTFALSE;
	m_hstrActivateMessage = LTNULL;
	m_hstrActivateTarget = LTNULL;
	m_hstrCowerSound = LTNULL;

	m_sIdleAni = "Idle_0";
	m_sCowerAni = "Stand_Cower";
	m_sDeathAni = "Death";

	m_bLockState = LTFALSE;
	m_bEnableCower = LTTRUE;

	m_fCowerTime = 10.0f;
	m_fTime = 0.0f;

	m_bIgnoreAI = LTTRUE;

	m_fSoundRadius = 500.0f;

	m_fMinAlienRangeSqr = 0.0f;
	m_fMinPredatorRangeSqr = 0.0f;
	m_fMinMarineRangeSqr = 0.0f;
	m_fMinCorporateRangeSqr = 0.0f;

	m_damage.SetRemoveOnDeath(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::~Civilian()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Civilian::~Civilian()
{
	if(!g_pLTServer)
		return;

	FREE_HSTRING(m_hstrActivateMessage);
	FREE_HSTRING(m_hstrActivateTarget);
	FREE_HSTRING(m_hstrCowerSound);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD Civilian::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}


			return dwRet;
			break;
		}

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			InitialUpdate();

			g_pLTServer->SetNextUpdate(m_hObject, 0.1f);

			CacheFiles();
			return dwRet;
		}

		case MID_UPDATE:
		{
			// DON'T call Prop::EngineMessageFn, object might get removed...
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			Update();

			g_pLTServer->SetNextUpdate(m_hObject, 0.1f);
	
			return dwRet;

			break;
		}

		case MID_MODELSTRINGKEY:
		{
//			HandleModelString((ArgList*)pData);
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
//	ROUTINE:	Civilian::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Civilian::ReadProp(ObjectCreateStruct *pInfo)
{
	if (!pInfo) 
		return LTFALSE;

	GenericProp genProp;

	// Civilian stuff
	if (g_pLTServer->GetPropGeneric("IgnoreAI", &genProp) == DE_OK)
		m_bIgnoreAI = genProp.m_Bool;

	if (g_pLTServer->GetPropGeneric("FearAlien", &genProp) == DE_OK)
		m_fMinAlienRangeSqr = genProp.m_Float*genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("FearPredator", &genProp) == DE_OK)
		m_fMinPredatorRangeSqr = genProp.m_Float*genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("FearMarine", &genProp) == DE_OK)
		m_fMinMarineRangeSqr = genProp.m_Float*genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("FearCorporate", &genProp) == DE_OK)
		m_fMinCorporateRangeSqr = genProp.m_Float*genProp.m_Float;

	if ( g_pLTServer->GetPropGeneric( "CharacterType", &genProp ) == LT_OK )
	{
		if(g_pCharacterButeMgr)
		{
			m_nCharacterSet = g_pCharacterButeMgr->GetSetFromModelType(genProp.m_String);

			const CharacterButes & butes = g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);
//			m_eModelSkeleton = (ModelSkeleton)butes.m_nSkeleton;
			pInfo->m_UserFlags = SurfaceToUserFlag((SurfaceType)butes.m_nSurfaceType);
		}
	}

	if (g_pLTServer->GetPropGeneric("IdleAni", &genProp) == LT_OK)
		m_sIdleAni = genProp.m_String;

	if (g_pLTServer->GetPropGeneric("CowerAni", &genProp) == LT_OK)
		m_sCowerAni = genProp.m_String;

	if (g_pLTServer->GetPropGeneric("DeathAni", &genProp) == LT_OK)
		m_sDeathAni = genProp.m_String;

	if (g_pLTServer->GetPropGeneric("CowerTime", &genProp) == LT_OK)
		m_fTime = genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("EnableCower", &genProp) == LT_OK)
		m_bEnableCower = genProp.m_Bool;

	// Sound stuff

	if (g_pLTServer->GetPropGeneric("CowerSound", &genProp) == LT_OK)
	if (genProp.m_String[0])
		m_hstrCowerSound = g_pLTServer->CreateString(genProp.m_String);

	if (g_pLTServer->GetPropGeneric( "SoundRadius", &genProp ) == LT_OK)
	{
		m_fSoundRadius = genProp.m_Float;
	}


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
//	ROUTINE:	Civilian::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void Civilian::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) 
		return;

	const CharacterButes & butes = g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);
	butes.FillInObjectCreateStruct(*pStruct);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD Civilian::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			char* szMsg = hMsg ? g_pLTServer->GetStringData(hMsg) : LTNULL;

			if ( szMsg && !_stricmp(szMsg, s_szActivate) )
			{
				if ( m_bPlayerUsable || g_pLTServer->GetObjectClass(hSender) != g_pLTServer->GetClass("CPlayerObj") )
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

			if ( szMsg && !_stricmp(szMsg, "Cower") )
			{
				m_RootFSM.SetState(Root::eStateCower);
				m_bLockState = LTTRUE;
			}

			if ( szMsg && !_stricmp(szMsg, "Idle") )
			{
				m_RootFSM.SetState(Root::eStateIdle);
				m_bLockState = LTFALSE;
			}

			g_pLTServer->FreeString(hMsg);

			break;
		}

		case MID_DAMAGE:
		{
			DDWORD dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

			if ( m_damage.IsDead() )
			{
				CreateBody(LTFALSE);
				g_pInterface->RemoveObject(m_hObject);
			}

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
//	ROUTINE:	Civilian::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Civilian::InitialUpdate()
{
	CreateSpecialFX();

	m_RootFSM.SetState(Root::eStateIdle);
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::Update()
//
//	PURPOSE:	Update state machine
//
// ----------------------------------------------------------------------- //

void Civilian::Update()
{
	m_RootFSM.Update();
}


// ----------------------------------------------------------------------- //
//   State Machine & related methods
// ----------------------------------------------------------------------- //
void Civilian::SetupFSM()
{
	m_RootFSM.DefineState(Root::eStateIdle, 
						  m_RootFSM.MakeCallback(*this, &Civilian::UpdateIdle),
						  m_RootFSM.MakeCallback(*this, &Civilian::StartIdle));

	m_RootFSM.DefineState(Root::eStateCower,
						  m_RootFSM.MakeCallback(*this, &Civilian::UpdateCower),
						  m_RootFSM.MakeCallback(*this, &Civilian::StartCower));

	// Standard Transitions
	m_RootFSM.DefineTransition(Root::eStateIdle, Root::eStimulus, Root::eStateCower);
	m_RootFSM.DefineTransition(Root::eStateCower, Root::eSafe, Root::eStateIdle);
}


void Civilian::StartIdle(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char *>(m_sIdleAni.c_str()));

	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
}

void Civilian::UpdateIdle(RootFSM * fsm)
{
	LTFLOAT fRangeSqr, fSoundRangeSqr;
	LTVector vCivPos;
	LTBOOL bCower = LTFALSE;

	if (!m_bEnableCower)
		return;

	g_pInterface->GetObjectPos(m_hObject, &vCivPos);

	// Detect weapon fire and impacts
	for (CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters(); iter != g_pCharacterMgr->EndCharacters(); ++iter)
	{
		const CCharacter * pChar = *iter;

		if (m_bIgnoreAI)
		{
			if (IsAI(pChar->m_hObject))
				continue;
		}

		if (pChar->IsDead() || (g_pLTServer->GetObjectState(pChar->m_hObject) & OBJSTATE_INACTIVE))
			continue;

		const LTFLOAT fCharRangeSqr = (vCivPos - pChar->GetPosition()).MagSqr();

		if (m_fMinAlienRangeSqr > 0.0f && pChar->GetCharacterClass() == ALIEN)
		{
			if (fCharRangeSqr < m_fMinAlienRangeSqr)
			{
				bCower = LTTRUE;
				m_fTime = g_pLTServer->GetTime();
				break;
			}
		}

		if (m_fMinPredatorRangeSqr > 0.0f && pChar->GetCharacterClass() == PREDATOR )
		{
			if (fCharRangeSqr < m_fMinPredatorRangeSqr)
			{
				bCower = LTTRUE;
				m_fTime = g_pLTServer->GetTime();
				break;
			}
		}
			
		if (m_fMinMarineRangeSqr > 0.0f && (pChar->GetCharacterClass() == MARINE ||  pChar->GetCharacterClass() == MARINE_EXOSUIT) )
		{
			if (fCharRangeSqr < m_fMinMarineRangeSqr)
			{
				bCower = LTTRUE;
				m_fTime = g_pLTServer->GetTime();
				break;
			}
		}

		if (m_fMinCorporateRangeSqr > 0.0f && (pChar->GetCharacterClass() == CORPORATE ||  pChar->GetCharacterClass() == CORPORATE_EXOSUIT || pChar->GetCharacterClass() == SYNTHETIC ) )
		{
			if (fCharRangeSqr < m_fMinCorporateRangeSqr)
			{
				bCower = LTTRUE;
				m_fTime = g_pLTServer->GetTime();
				break;
			}
		}

		BARREL *pBarrel = g_pWeaponMgr->GetBarrel(pChar->GetLastFireInfo().nBarrelId);
		if (pBarrel && (pChar->GetLastFireInfo().fTime != m_fTime))
		{
			fSoundRangeSqr = pBarrel->fFireSoundRadius;
			fSoundRangeSqr *= fSoundRangeSqr;

			fRangeSqr = (vCivPos - pChar->GetLastFireInfo().vFiredPos).MagSqr();
			if (fRangeSqr < fSoundRangeSqr)
			{
				m_fTime = pChar->GetLastFireInfo().fTime;
				bCower = LTTRUE;
				break;
			}
		}

		AMMO *pAmmo = g_pWeaponMgr->GetAmmo(pChar->GetLastFireInfo().nAmmoId);
		if (pAmmo && pAmmo->pImpactFX && (pChar->GetLastImpactInfo().fTime != m_fTime))
		{
			fSoundRangeSqr = (LTFLOAT)pAmmo->pImpactFX->nAISoundRadius;
			fSoundRangeSqr *= fSoundRangeSqr;

			fRangeSqr = (vCivPos - pChar->GetLastImpactInfo().vImpactPos).MagSqr();
			if (fRangeSqr < fSoundRangeSqr)
			{
				m_fTime = pChar->GetLastImpactInfo().fTime;
				bCower = LTTRUE;
				break;
			}
		}
	}

	if (bCower && !m_damage.IsDead())
	{
		m_RootFSM.AddMessage(Root::eStimulus);
	}
	
	return;
}

void Civilian::StartCower(RootFSM * fsm)
{
	if (!m_hObject)
		return;

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, const_cast<char *>(m_sCowerAni.c_str()));

	g_pLTServer->SetModelAnimation(m_hObject, hAnim);
	g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

	m_tmrCower.Start(m_fCowerTime);

	// Play cower sound...

	if (m_hstrCowerSound)
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		char* pSound = g_pLTServer->GetStringData(m_hstrCowerSound);
		if (pSound)
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSoundRadius, SOUNDPRIORITY_MISC_LOW);
	}

}

void Civilian::UpdateCower(RootFSM * fsm)
{
	if ((m_tmrCower.Stopped()) && !m_bLockState)
	{
		m_RootFSM.AddMessage(Root::eSafe);
	}

	return;
}

HOBJECT Civilian::CreateBody(LTBOOL bCarryOverAttachments)
{
	HCLASS hClass = g_pLTServer->GetClass("BodyProp");
	if (!hClass) return LTNULL;

	ObjectCreateStruct theStruct;
	theStruct.Clear();
	
#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-Body", g_pLTServer->GetObjectName(m_hObject) );
#endif

	g_pCharacterButeMgr->GetDefaultFilenames(m_nCharacterSet, theStruct);

	// Setup the head skin...

	g_pLTServer->GetObjectPos(m_hObject, &theStruct.m_Pos);
	theStruct.m_Pos += LTVector(0.0f, 0.1f, 0.0f);
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	BodyProp* pProp = (BodyProp *)g_pLTServer->CreateObject(hClass, &theStruct);

	_ASSERT(pProp);
	if (!pProp) return LTNULL;

	CharInfo info;
	info.hObj = m_hObject;
	info.nCharacterSet = m_nCharacterSet;
	info.pDest = &m_damage;

	pProp->Setup(&info, bCarryOverAttachments);
	return pProp->m_hObject;
}


void Civilian::CreateSpecialFX()
{
	// Create the special fx...
	CHARCREATESTRUCT cs;

	cs.nCharacterSet	= (uint8)m_nCharacterSet;
	cs.nNumTrackers		= 1;
	cs.nClientID		= 0; // Not a player

	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	cs.Write(g_pLTServer, hMessage);
	g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Civilian::Save(HMESSAGEWRITE hWrite)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) 
		return;

	g_pLTServer->WriteToMessageDWord(hWrite, m_bPlayerUsable);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrActivateMessage);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrActivateTarget);
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrCowerSound);

	*hWrite << m_fSoundRadius;
	*hWrite << m_nCharacterSet;
	*hWrite << m_sIdleAni;
	*hWrite << m_sCowerAni;
	*hWrite << m_sDeathAni;
	*hWrite << m_tmrCower;
	*hWrite << m_bLockState;
	*hWrite << m_bEnableCower;
	*hWrite << m_fMinAlienRangeSqr;
	*hWrite << m_fMinPredatorRangeSqr;
	*hWrite << m_fMinMarineRangeSqr;
	*hWrite << m_fMinCorporateRangeSqr;
	*hWrite << m_fCowerTime;
	hWrite->WriteFloat(m_fTime - g_pLTServer->GetTime());
	*hWrite << m_RootFSM;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Civilian::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) 
		return;

	m_bPlayerUsable = (LTBOOL)g_pLTServer->ReadFromMessageDWord(hRead);
	m_hstrActivateMessage = g_pLTServer->ReadFromMessageHString(hRead);
	m_hstrActivateTarget = g_pLTServer->ReadFromMessageHString(hRead);

	m_hstrCowerSound = g_pLTServer->ReadFromMessageHString(hRead);

	*hRead >> m_fSoundRadius;
	*hRead >> m_nCharacterSet;
	*hRead >> m_sIdleAni;
	*hRead >> m_sCowerAni;
	*hRead >> m_sDeathAni;
	*hRead >> m_tmrCower;
	*hRead >> m_bLockState;
	*hRead >> m_bEnableCower;
	*hRead >> m_fMinAlienRangeSqr;
	*hRead >> m_fMinPredatorRangeSqr;
	*hRead >> m_fMinMarineRangeSqr;
	*hRead >> m_fMinCorporateRangeSqr;
	*hRead >> m_fCowerTime;
	m_fTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_RootFSM;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Civilian::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void Civilian::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) 
		return;

	char* pFile = LTNULL;

/*
	if (m_hstrSpawnSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSpawnSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}
*/
}

LTRESULT CivilianPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;

	if (_stricmp("CharacterType", szPropName) == 0)
	{
		m_CharacterButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

