// ----------------------------------------------------------------------- //
//
// MODULE  : Prop.cpp
//
// PURPOSE : Model Prop - Definition
//
// CREATED : 10/9/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Prop.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "ObjectMsgs.h"
#include "CharacterButeMgr.h"
#include "ServerSoundMgr.h"
#include "SharedFXStructs.h"
#include "FXButeMgr.h"
#include "SoundButeMgr.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "CommandMgr.h"

BEGIN_CLASS(Prop)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, 0)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL(Scale, 1.0f, 1.0f, 1.0f)
	ADD_STRINGPROP_FLAG_HELP(StringKeyCommand, "", PF_HIDDEN, "If a string key of \"cmd\" is received, this message will be sent." )
	ADD_BOOLPROP_FLAG_HELP(OneTimeKeyCommand, LTFALSE, PF_HIDDEN, "Will only perform key command once if set to true." )
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SOLID_FLAG(0, 0)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_SHADOW_FLAG(0, 0)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)
    ADD_BOOLPROP(DetailTexture, LTFALSE)
    ADD_BOOLPROP(Chrome, LTFALSE)
	ADD_REALPROP(Alpha, 1.0f)
	ADD_COLORPROP(ObjectColor, 255.0f, 255.0f, 255.0f)
    ADD_CHROMAKEY_FLAG(LTFALSE, 0)
    ADD_BOOLPROP(Additive, LTFALSE)
    ADD_BOOLPROP(Multiply, LTFALSE)
    ADD_BOOLPROP(SpecialAttachment, LTFALSE)
    ADD_BOOLPROP(RayHit, LTTRUE)
	ADD_STRINGPROP_FLAG_HELP(TouchSound, "None", PF_STATICLIST,	"Special sound to play when the object is touched.")
	ADD_STRINGPROP_FLAG(DetailLevel, "Low", PF_STATICLIST)
    ADD_BOOLPROP(AICanShootThrough, LTFALSE)
    ADD_BOOLPROP(AICanSeeThrough, LTFALSE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Prop, GameBase, NULL, NULL, 0, CPropPlugin)


LTRESULT CPropPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char* const * aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	static CDestructibleModelPlugin	m_DestructibleModelPlugin;
	static CSoundButeMgrPlugin		m_SoundButePlugin;


	if (m_DestructibleModelPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}
	else if (_stricmp("DetailLevel", szPropName) == 0)
	{
		strcpy(aszStrings[(*pcStrings)++], "Low");
		strcpy(aszStrings[(*pcStrings)++], "Medium");
		strcpy(aszStrings[(*pcStrings)++], "High");
		return LT_OK;
	}
	else if (_stricmp("TouchSound", szPropName) == 0)
	{
		m_SoundButePlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength, "BasePropTouchSound");
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

static char s_tokenSpace[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
static char *s_pTokens[PARSE_MAXTOKENS];
static char *s_pCommandPos;

const char* const PROP_KEY_LOOP_SOUND		= "loop_bute_sound_key";
const char* const PROP_KEY_LOOP_SOUND_END	= "end_loop_sound_key";
const char* const PROP_KEY_COMMAND			= "CMD";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Prop()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Prop::Prop() : GameBase(OT_MODEL)
{
	// Move to the floor on the first update.
    SetMoveToFloor( LTTRUE );

	m_vScale.Init(1.0f, 1.0f, 1.0f);
	m_vObjectColor.Init(255.0f, 255.0f, 255.0f);
	m_fAlpha = 1.0f;

	m_dwFlags	= FLAG_DONTFOLLOWSTANDING;
	m_dwFlags2	= 0;

    m_hstrTouchSound    = LTNULL;
    m_hTouchSnd         = LTNULL;

	m_bCanShootThrough		= LTFALSE;
	m_bCanSeeThrough		= LTFALSE;
	m_bRemoveOnLinkBroken	= LTFALSE;
	
	m_hstrLoopSound	= LTNULL;
	m_hLoopSound	= LTNULL;

	m_bOneTimeKeyCommand = LTFALSE;

	AddAggregate(&m_damage);

	// Be sure to tell the damage aggregate to lay off our flag!
	m_damage.CanSetRayHit(LTFALSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::~Prop()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Prop::~Prop()
{
	if(m_hstrTouchSound)
		FREE_HSTRING(m_hstrTouchSound);

	if (m_hTouchSnd)
	{
        g_pLTServer->KillSound(m_hTouchSnd);
        m_hTouchSnd = LTNULL;
	}

	if(m_hstrLoopSound)
		FREE_HSTRING(m_hstrLoopSound);

	if (m_hLoopSound)
	{
        g_pLTServer->KillSound(m_hLoopSound);
        m_hLoopSound = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Prop::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				ReadProp(pStruct);

				m_dwFlags |= FLAG_MODELKEYS | FLAG_FULLPOSITIONRES;

				// If this prop is spawned, assume it should be visible (if they
				// specify Visible 0, our parent class will handle it ;)

				if (fData == PRECREATE_STRINGPROP)
				{
					m_dwFlags |= FLAG_VISIBLE;
				}


				pStruct->m_Flags = m_dwFlags;
				pStruct->m_Flags2 = m_dwFlags2;
			}

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);

			CacheFiles();

			return dwRet;
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

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
			break;
		}

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


		case MID_LINKBROKEN :
		{
			if(m_bRemoveOnLinkBroken)
				g_pLTServer->RemoveObject(m_hObject);			
		}
		break;

		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Prop::ObjectMessageFn( HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead )
{
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

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::ReadProp(ObjectCreateStruct *pData)
{
	if (!pData) return;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Alpha", &genProp) == LT_OK)
	{
		m_fAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ObjectColor", &genProp) == LT_OK)
	{
		m_vObjectColor = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("MoveToFloor", &genProp) == LT_OK)
	{
		// Move to floor on the next MID_UPDATE.
		SetMoveToFloor( genProp.m_Bool );
	}

    if (g_pLTServer->GetPropGeneric("Scale", &genProp) == LT_OK)
	{
		 m_vScale = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("Chrome", &genProp ) == LT_OK)
	{
		m_dwFlags |= (genProp.m_Bool ? FLAG_ENVIRONMENTMAP : 0);
	}

    if (g_pLTServer->GetPropGeneric("DetailTexture", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags |= FLAG_DETAILTEXTURE;
		}
	}

    if (g_pLTServer->GetPropGeneric("Additive", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags2 |= FLAG2_ADDITIVE;
			m_dwFlags  |= FLAG_FOGDISABLE;
		}
	}

    if (g_pLTServer->GetPropGeneric("Multiply", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags2 |= FLAG2_MULTIPLY;
			m_dwFlags  |= FLAG_FOGDISABLE;
		}
	}

	if (g_pLTServer->GetPropGeneric("SpecialAttachment", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			pData->m_UserFlags |= USRFLG_SPECIAL_ATTACHMENT;
		}
	}

    if (g_pLTServer->GetPropGeneric("RayHit", &genProp) == LT_OK)
	{
		if (genProp.m_Bool)
		{
			m_dwFlags |= FLAG_RAYHIT;

			// Set touch notify so projectiles can impact with us...
			m_dwFlags |= FLAG_TOUCH_NOTIFY;
		}
	}

    if (g_pLTServer->GetPropGeneric("DetailLevel", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			/*
			TO DO:
			FINISH THIS FOR WES...

			if (_stricmp("Low", szPropName) == 0)
			{
				m_eDetailLevel = DL_LOW;
			}
			else if (_stricmp("Medium", szPropName) == 0)
			{
				m_eDetailLevel = DL_MEDIUM;
			}
			else  // High
			{
				m_eDetailLevel = DL_HIGH;
			}
			*/
		}
	}

    if (g_pLTServer->GetPropGeneric("AICanShootThrough", &genProp) == LT_OK)
	{
		 m_bCanShootThrough = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("AICanSeeThrough", &genProp) == LT_OK)
	{
		 m_bCanSeeThrough = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("TouchSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			if(_stricmp("None", genProp.m_String) != 0)
			{
				m_hstrTouchSound = g_pLTServer->CreateString(genProp.m_String);
				m_dwFlags |= FLAG_TOUCH_NOTIFY;
			}
		}
	}

    if (g_pLTServer->GetPropGeneric("StringKeyCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_strKeyCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("OneTimeKeyCommand", &genProp) == LT_OK)
	{
		m_bOneTimeKeyCommand = genProp.m_Bool;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void Prop::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Remove if outside the world

	m_dwFlags |= FLAG_REMOVEIFOUTSIDE | FLAG_ANIMTRANSITION;

	// If this prop is one that shouldn't stop the player, set the appropriate
	// flag...

	if (m_damage.GetMass() < 500.0f /*HARDCODE a decent mass value for now*/)
	{
		m_dwFlags |= FLAG_CLIENTNONSOLID;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void Prop::InitialUpdate()
{
	// Set flags...

    uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwFlags | USRFLG_MOVEABLE);

    g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
    g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | m_dwFlags);

    g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags);
    g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags | m_dwFlags2);


	// Set object translucency...

	// This line looks a bit suspect...
	//VEC_DIVSCALAR(m_vObjectColor, m_vObjectColor, 255.0f/m_fAlpha);
	// Lets try this instead...
	VEC_DIVSCALAR(m_vObjectColor, m_vObjectColor, 255.0f);

    g_pLTServer->SetObjectColor(m_hObject, m_vObjectColor.x, m_vObjectColor.y,
								m_vObjectColor.z, m_fAlpha);


	// Set the dims based on the current animation...

    LTVector vDims;
    g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	// Set object dims based on scale value...

    LTVector vNewDims;
	vNewDims.x = m_vScale.x * vDims.x;
	vNewDims.y = m_vScale.y * vDims.y;
	vNewDims.z = m_vScale.z * vDims.z;

    g_pLTServer->ScaleObject(m_hObject, &m_vScale);
    g_pLTServer->SetObjectDims(m_hObject, &vNewDims);

	// Need at least one update so we can move to floor.
	if( GetMoveToFloor( ))
		SetNextUpdate( 0.001f, 1.0f );
	else
		SetNextUpdate( 0.0f, 0.0f );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::TriggerMsg()
//
//	PURPOSE:	Handler for prop trigger messages.
//
// --------------------------------------------------------------------------- //

void Prop::TriggerMsg(HOBJECT hSender, HSTRING hMsg)
{
	char szMsg[1024];
    strcpy(szMsg, g_pLTServer->GetStringData(hMsg));

	int nArgs;
	char* pCommand = szMsg;
    LTBOOL bMore = LTTRUE;
	while (bMore)
	{
        bMore = g_pLTServer->Parse(pCommand, &s_pCommandPos, s_tokenSpace, s_pTokens, &nArgs);
		if ( !_stricmp(s_pTokens[0], "ANIM") )
		{
            g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
            g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, s_pTokens[1]));
		}
		else if ( !_stricmp(s_pTokens[0], "ANIMLOOP") )
		{
            g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
            g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, s_pTokens[1]));
		}

		pCommand = s_pCommandPos;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleModelString
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void Prop::HandleModelString(ArgList* pArgList)
{
	if(!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) 
		return;

	char *pKey = pArgList->argv[0];
	if(!pKey) return;

	// Thing key handles playing an ImpactFX from the FX.txt at a particular socket on the model.
	// The format is 'ImpactFX <ImpactFX Name> <Socket Name>'
	if(!_stricmp(pKey, "ImpactFX"))
	{
		if(pArgList->argc < 2) return;

		LTVector vPos;
		LTRotation rRot;
		LTBOOL bUseObjOrientation = LTTRUE;

		if(pArgList->argc >= 3)
		{
			HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
			g_pLTServer->GetModelLT()->GetSocket(m_hObject, pArgList->argv[2], hSocket);

			if(hSocket != INVALID_MODEL_SOCKET)
			{
				LTransform trans;
				g_pLTServer->GetModelLT()->GetSocketTransform(m_hObject, hSocket, trans, LTTRUE);

				vPos = trans.m_Pos;
				rRot = trans.m_Rot;

				bUseObjOrientation = LTFALSE;
			}
		}

		if(bUseObjOrientation)
		{
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			g_pLTServer->GetObjectRotation(m_hObject, &rRot);
		}

		// Create the impact FX
		IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX(pArgList->argv[1]);

		if(pFX)
		{
			EXPLOSIONCREATESTRUCT cs;
			cs.nImpactFX = pFX->nId;
			cs.vPos = vPos;
			cs.rRot = rRot;
			cs.fDamageRadius = 0;

			HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&cs.vPos);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_EXPLOSION_ID);
			cs.Write(g_pLTServer, hMessage);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}
	}
	// handle sound fx key
	else if(!_stricmp(pKey, "BUTE_SOUND_KEY"))
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			//play the buted sound
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pArgList->argv[1]);
		}
	}
	//handle starting the loop sound
	else if (stricmp(pKey, PROP_KEY_LOOP_SOUND) == 0)
	{
		if (pArgList->argc > 1 && pArgList->argv[1])
		{
			// Only play if we are not already playing
			if(!m_hLoopSound)
			{
//				g_pLTServer->CPrint("Prop key loop sound!  %s", pArgList->argv[1]);

				// Play the sound here...
				m_hLoopSound = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pArgList->argv[1]);

				if(m_hstrLoopSound)
				{
					FREE_HSTRING(m_hstrLoopSound);
					m_hstrLoopSound = LTNULL;
				}

				// Record the sound name here
				m_hstrLoopSound = g_pLTServer->CreateString(pArgList->argv[1]);
			}
		}
	}
	//handle ending the loop sound
	else if (stricmp(pKey, PROP_KEY_LOOP_SOUND_END) == 0)
	{
		if(m_hLoopSound)
		{
			// Stop the sound..
			g_pLTServer->KillSound(m_hLoopSound);
			m_hLoopSound = LTNULL;

//			g_pLTServer->CPrint("Prop key loop sound end!");

			// Free the string
			if(m_hstrLoopSound)
			{
				FREE_HSTRING(m_hstrLoopSound);
				m_hstrLoopSound = LTNULL;
			}
		}
	}
	else if (stricmp(pKey, PROP_KEY_COMMAND) == 0)
	{
		LTBOOL bAddParen = LTFALSE;

		char buf[255] = "";
		sprintf(buf, "%s", pArgList->argv[1]);
		for (int i=2; i < pArgList->argc; i++)
		{
			bAddParen = LTFALSE;
			strcat(buf, " ");
			if (strstr(pArgList->argv[i], " "))
			{
				strcat(buf, "(");
				bAddParen = LTTRUE;
			}

			strcat(buf, pArgList->argv[i]);

			if (bAddParen)
			{
				strcat(buf, ")");
			}
		}

#ifdef _DEBUG
		g_pLTServer->CPrint("CHARACTER KEY COMMAND: %s", buf);
#endif
		if (buf[0] && g_pCmdMgr->IsValidCmd(buf))
		{
			g_pCmdMgr->Process(buf);
		}
	}

	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::HandleTouch
//
//	PURPOSE:	Handle touch notifies
//
// ----------------------------------------------------------------------- //

void Prop::HandleTouch(HOBJECT hObj)
{
	if (!hObj || !m_hstrTouchSound) return;

	// Make sure we only play one sound at a time...

	if (m_hTouchSnd)
	{
        LTBOOL bIsDone;
        if (g_pLTServer->IsSoundDone(m_hTouchSnd, &bIsDone) != LT_OK || bIsDone)
		{
            g_pLTServer->KillSound(m_hTouchSnd);
            m_hTouchSnd = LTNULL;
		}
	}

	// Play the touch sound...

	if (m_hstrTouchSound && !m_hTouchSnd)
	{
        char* pSound = g_pLTServer->GetStringData(m_hstrTouchSound);

		if (pSound)
			m_hTouchSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::IsEnemyInRange
//
//	PURPOSE:	Determine if a player or AI has crossed a range threshold
//
// ----------------------------------------------------------------------- //

CCharacter * Prop::IsEnemyInRange(LTFLOAT fRange)
{
	if (m_bIgnoreAI)
		return (CCharacter *)IsPlayerInRange(fRange);
	else
		return IsCharacterInRange(fRange);
}

CPlayerObj * Prop::IsPlayerInRange(LTFLOAT fRange)
{
	LTVector vPos;
	CPlayerObj * pPlayer;

	_ASSERT(g_pCharacterMgr);
	if (!g_pCharacterMgr)
		return LTFALSE;

	g_pInterface->GetObjectPos(m_hObject, &vPos);

	for (CCharacterMgr::PlayerIterator iter = g_pCharacterMgr->BeginPlayers();
	     iter != g_pCharacterMgr->EndPlayers(); ++iter)
	{
		if (*iter)
		{
			pPlayer = *iter;

			const LTFLOAT fDistSqr = VEC_DISTSQR(vPos, pPlayer->GetPosition());
			if (fDistSqr < fRange)
			{
				return pPlayer;
			}
		}
	}

	return LTNULL;
}

CCharacter * Prop::IsCharacterInRange(LTFLOAT fRange)
{
	LTVector vPos;
	CCharacter *pChar;

	_ASSERT(g_pCharacterMgr);
	if (!g_pCharacterMgr)
		return LTFALSE;

	g_pInterface->GetObjectPos(m_hObject, &vPos);

	for (CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters();
		 iter != g_pCharacterMgr->EndCharacters(); ++iter)
	{
		if (*iter)
		{
			pChar = *iter;

			// ignore self
			if (pChar->m_hObject == m_hObject)
				continue;

			const LTFLOAT fDistSqr = VEC_DISTSQR(vPos, pChar->GetPosition());
			if (fDistSqr < fRange)
			{
				return pChar;
			}
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void Prop::CacheFiles()
{
	if (m_hstrTouchSound)
	{
        char* pStr = g_pLTServer->GetStringData(m_hstrTouchSound);
		if (pStr && pStr[0])
		{
            g_pLTServer->CacheFile(FT_SOUND, pStr);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Prop::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	*hWrite << m_fAlpha;
	g_pServerDE->WriteToMessageHString(hWrite, m_hstrTouchSound);
	*hWrite << m_vScale;
	*hWrite << m_vObjectColor;
	*hWrite << m_dwFlags;
	*hWrite << m_dwFlags2;
	*hWrite << m_bIgnoreAI;

	*hWrite << m_bCanShootThrough;
	*hWrite << m_bCanSeeThrough;
	*hWrite << m_bRemoveOnLinkBroken;

	g_pServerDE->WriteToMessageHString(hWrite, m_hstrLoopSound);

	//now build the hide status flag
	uint32 nHideFlags = 0;

	//get the info and save it
	for(int i=0; i<32 ; i++)
	{
		LTBOOL bHidden;
		g_pModelLT->GetPieceHideStatus(m_hObject, (HMODELPIECE)i, bHidden);
		if(bHidden)
			nHideFlags |= (1<<i);
	}

	*hWrite << nHideFlags;	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Prop::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Prop::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	*hRead >> m_fAlpha;
	m_hstrTouchSound = g_pServerDE->ReadFromMessageHString(hRead);
	*hRead >> m_vScale;
	*hRead >> m_vObjectColor;
	*hRead >> m_dwFlags;
	*hRead >> m_dwFlags2;
	*hRead >> m_bIgnoreAI;

	*hRead >> m_bCanShootThrough;
	*hRead >> m_bCanSeeThrough;
	*hRead >> m_bRemoveOnLinkBroken;

	m_hstrLoopSound = g_pServerDE->ReadFromMessageHString(hRead);

	// Play the sound here
	if(m_hstrLoopSound)
	{
		char* szSound	= g_pLTServer->GetStringData(m_hstrLoopSound);
		m_hLoopSound	= g_pServerSoundMgr->PlaySoundFromObject(m_hObject, szSound);
	}

	//now re-build the hide status flag
	uint32 nHideFlags;
	*hRead >> nHideFlags;	

	//make the piece invisible or not
	for(int i=0; i<32 ; i++)
	{
		LTBOOL bHidden = (nHideFlags & (1<<i));
		g_pModelLT->SetPieceHideStatus(m_hObject, (HMODELPIECE)i, bHidden);
	}
}
