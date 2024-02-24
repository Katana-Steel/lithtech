#include "stdafx.h"

#include "SimpleAI.h"
#include "CharacterMgr.h"
#include "CharacterButeMgr.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "CharacterHitBox.h"
#include "AIUtils.h"
#include "WeaponMgr.h"
#include "Attachments.h"
#include "AI.h"
#include "AIPlayer.h"

extern std::string ExtractMusicSetting(char * szString); // in AI.cpp.

#define TRIGGER_TARGET		"TRG"
#define	TRIGGER_REMOVE		"REMOVE"

// From AI.cpp.
#ifndef _FINAL

extern bool InfoAIState();
extern bool InfoAIListGoals();

#endif

const LTFLOAT c_fSimpleAIDeactivationTime = 10.0f;

CSimpleAI::SimpleAIList CSimpleAI::sm_SimpleAI;



static bool BoxIntersection( const LTVector & pos1, const LTVector & dims1, 
							 const LTVector & pos2, const LTVector & dims2  )
{
	_ASSERT( dims1.x >= 0.0f && dims1.y >= 0.0f && dims1.z >= 0.0f );
	_ASSERT( dims2.x >= 0.0f && dims2.y >= 0.0f && dims2.z >= 0.0f );

	return (   (float)fabs(pos1.x - pos2.x) < dims1.x + dims2.x 
			&& (float)fabs(pos1.y - pos2.y) < dims1.y + dims2.y  			   
			&& (float)fabs(pos1.z - pos2.z) < dims1.z + dims2.z  );
}



BEGIN_CLASS(CSimpleAI)

	PROP_DEFINEGROUP(AttributeOverrides, PF_HIDDEN)
	
	ADD_BOOLPROP_FLAG_HELP(IgnoreAI, LTTRUE, 0,	"TRUE means this AI is only activated by players.  This does NOT apply to the Alert radius.")

	ADD_REALPROP_FLAG_HELP(DetectionRadius, 250.0f, 0,	"Attacks the first thing to enter this radius")
	ADD_REALPROP_FLAG_HELP(AlertRadius, 250.0f, 0, "When attacking, it will activate others in this radius")

	ADD_REALPROP_FLAG_HELP(MeleeDamage, 25.0f, 0, "Amount of damage this AI will do per attack")

	ADD_REALPROP_FLAG_HELP(LeashLength, 0.0f, 0, "AI will not move farther than this length away from the leash point.  Leave zero to keep AI unleashed.")
	ADD_STRINGPROP_FLAG_HELP(LeashPoint, "", PF_OBJECTLINK, "AI will not move farther than LeashLength away from this object. Leave blank to use AI's initial position.")

#ifdef _WIN32
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\Drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/Drone.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

	ADD_STRINGPROP_FLAG_HELP(AmbientTrackSet, const_cast<char*>(g_szDefault), PF_STATICLIST, "TrackSet for Ambient music mood.")
	ADD_STRINGPROP_FLAG_HELP(WarningTrackSet, const_cast<char*>(g_szDefault), PF_STATICLIST, "TrackSet for Warning music mood.")
	ADD_STRINGPROP_FLAG_HELP(HostileTrackSet, const_cast<char*>(g_szDefault), PF_STATICLIST, "TrackSet for Hostile music mood.")

	// Non-game related stuff
	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 0.0f, 0.0f, 0, "Color used for object in DEdit.") 

END_CLASS_DEFAULT_FLAGS_PLUGIN(CSimpleAI, CCharacter, NULL, NULL, CF_HIDDEN, CSimpleAIPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::CSimpleAI()
//				CSimpleAI::~CSimpleAI()
//
//	PURPOSE:	Initialization & Cleanup
//
// ----------------------------------------------------------------------- //

CSimpleAI::CSimpleAI()
{
	m_fDetectionRadiusSqr = 250.0f * 250.0f;
	m_fAlertRadiusSqr = 250.0f * 250.0f;
	m_hTarget = LTNULL;
	m_pCharTarget = LTNULL;

	m_bFirstUpdate = LTTRUE;
	m_fUpdateDelta = 0.1f;
	m_bIgnoreAI = LTTRUE;

	m_fLeashLengthSqr = 0.0f;
	m_vLeashFrom = LTVector(0,0,0);

	m_fMaxMeleeRange = 175.0f;
	m_fMinMeleeRange = 100.0f;
	m_fMeleeDamage = 25.0f;

	m_bHidden = LTFALSE;
	m_dwActiveFlags = 0;
	m_dwActiveFlags2 = 0;
	m_dwActiveUserFlags = 0;
	m_bActiveCanDamage = LTTRUE;

	m_bSentInfo = LTFALSE;


	sm_SimpleAI.push_back(this);
}

CSimpleAI::~CSimpleAI()
{
	sm_SimpleAI.remove(this);

	// This must be done here so that charactermgr
	// will recognize our type via dynamic cast.
	// Is this an MSVC bug?
	g_pCharacterMgr->Remove(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::SetTarget()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void CSimpleAI::SetTarget(HOBJECT hTarget) 
{ 
	m_hTarget = hTarget; 
	m_pCharTarget = dynamic_cast<const CCharacter *>( m_hTarget ? g_pLTServer->HandleToObject(m_hTarget) : LTNULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::SetMusicMood()
//
//	PURPOSE:	Sets the mood music, paying attention to the tracks.
//
// ----------------------------------------------------------------------- //

void CSimpleAI::SetMusicMood(CMusicMgr::EMood eMood)
{
	// Check if this state doesn't modify the mood.
	if( eMood == CMusicMgr::eMoodNone )
		return;

	// Check our objects.
	if( !g_pMusicMgr )
	{
		ASSERT( FALSE );
		return;
	}

	// Check if there is no music.
	if( !g_pMusicMgr->IsValid( ))
		return;

	// Check if the modifier is a real mood
	std::string strTrackSet;
	switch( eMood )
	{
		case CMusicMgr::eMoodAmbient:
			strTrackSet = m_strMusicAmbientTrackSet;
			break;
		case CMusicMgr::eMoodWarning:
			strTrackSet = m_strMusicWarningTrackSet;
			break;
		case CMusicMgr::eMoodHostile:
			strTrackSet = m_strMusicHostileTrackSet;
			break;
		// Invalid mood.
		default:
			ASSERT( FALSE );
			return;
	}

	// Set the modified mood.
	if( !strTrackSet.empty() )
	{
		// The track is specified, if it is silent do nothing,
		// otherwise, set it to the appropriate track.
		if( strTrackSet != g_szSilent )
			g_pMusicMgr->SetModifiedMood( eMood, strTrackSet.c_str() );
	}
	else
	{
		// Set the mood, but let it play
		// the default track.  (An empty track indicates
		// default music.)
		g_pMusicMgr->SetModifiedMood( eMood );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::EngineMessageFn()
//				CSimpleAI::ObjectMessageFn()
//
//	PURPOSE:	Message handlers
//
// ----------------------------------------------------------------------- //

uint32 CSimpleAI::EngineMessageFn(uint32 dwID, void *pData, LTFLOAT fData)
{
	switch(dwID)
	{
		case MID_INITIALUPDATE:
		{
			uint32 dwRet = CCharacter::EngineMessageFn(dwID, pData, fData);
			InitialUpdate();

			g_pLTServer->SetDeactivationTime(m_hObject, c_fSimpleAIDeactivationTime);
			g_pInterface->SetNextUpdate(m_hObject, m_fUpdateDelta + GetRandom(0.0f,m_fUpdateDelta) );
			g_pServerDE->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);

			if( m_bStartHidden )
			{
				Hidden(true);
			}

//			CacheFiles();
			return dwRet;
			break;
		}

		case MID_UPDATE:
		{
			uint32 dwRet = CCharacter::EngineMessageFn(dwID, pData, fData);
			Update();
			g_pInterface->SetNextUpdate(m_hObject, m_fUpdateDelta);
			return dwRet;
			break;
		}

		case MID_MODELSTRINGKEY:
		{
			ArgList *pArgList = (ArgList*)pData;
			if (!pArgList || !pArgList->argv || pArgList->argc == 0)
				break;

			// Let the animation class handle them too
			m_caAnimation.HandleStringKey(pArgList, (uint8)fData);

			HandleModelString(pArgList);
			break;
		}

		case MID_PRECREATE:
		{
//			CreateAttachments();
			uint32 dwRet = CCharacter::EngineMessageFn(dwID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				{
					ReadProp(pStruct);
//					PostReadProp(pStruct);
				}
			}

			return dwRet;
			break;
		}

		case MID_SAVEOBJECT:
		{
			// Let aggregates go first...
			uint32 dwRet = CCharacter::EngineMessageFn(dwID, pData, fData);
			Save((HMESSAGEWRITE)pData);
			return dwRet;
			break;
		}

		case MID_LOADOBJECT:
		{
			// Let aggregates go first...
			uint32 dwRet = CCharacter::EngineMessageFn(dwID, pData, fData);
			Load((HMESSAGEREAD)pData);
			return dwRet;
			break;
		}

		default:
			break;
	}

	return CCharacter::EngineMessageFn(dwID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::ReadProp()
//
//	PURPOSE:	Read DEdit properties
//
// ----------------------------------------------------------------------- //

LTBOOL CSimpleAI::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pInterface|| !pData) return LTFALSE;

	if (g_pLTServer->GetPropGeneric("IgnoreAI", &genProp) == LT_OK)
		m_bIgnoreAI = genProp.m_Bool;

	if (g_pLTServer->GetPropGeneric("LeashLength", &genProp) == LT_OK)
		m_fLeashLengthSqr = genProp.m_Float*genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("LeashPoint", &genProp) == LT_OK)
		if (genProp.m_String[0])
			m_strLeashFromName = genProp.m_String;

	if (g_pLTServer->GetPropGeneric("DetectionRadius", &genProp) == LT_OK)
		m_fDetectionRadiusSqr = genProp.m_Float*genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("AlertRadius", &genProp) == LT_OK)
		m_fAlertRadiusSqr = genProp.m_Float*genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("MeleeDamage", &genProp) == LT_OK)
		m_fMeleeDamage = genProp.m_Float;

	// Get the ambient music trackset.
	if( g_pLTServer->GetPropGeneric( "AmbientTrackSet", &genProp ) == DE_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_strMusicAmbientTrackSet = ExtractMusicSetting(genProp.m_String);
		}
	}

	// Get the Warning music trackset.
	if( g_pLTServer->GetPropGeneric( "WarningTrackSet", &genProp ) == DE_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_strMusicWarningTrackSet = ExtractMusicSetting(genProp.m_String);
		}
	}

	// Get the Hostile music trackset.
	if( g_pLTServer->GetPropGeneric( "HostileTrackSet", &genProp ) == DE_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_strMusicHostileTrackSet = ExtractMusicSetting(genProp.m_String);
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::InitialUpdate()
//
//	PURPOSE:	Initialize stuff
//
// ----------------------------------------------------------------------- //
void CSimpleAI::InitialUpdate()
{
	m_fUpdateDelta = GetAIUpdateDelta();

	if (!g_pLTServer || !m_hObject)
		return;

	// Just go away if you fall through the world.
	GetMovement()->SetObjectFlags(OFT_Flags, GetMovement()->GetObjectFlags(OFT_Flags) | FLAG_REMOVEIFOUTSIDE);

	// Set semi-solid status
	GetMovement()->SetObjectFlags(OFT_Flags2, GetMovement()->GetObjectFlags(OFT_Flags2) | FLAG2_SEMISOLID);

	// Make stuff unguaranteed
	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	// Don't rag-doll AI anymore...
	m_damage.SetApplyDamagePhysics(LTFALSE);

	AMMO * pAIClawsAmmo = g_pWeaponMgr->GetAmmo("AI_Claws_Ammo");
	if( pAIClawsAmmo )
	{
		m_fMeleeDamage = LTFLOAT(pAIClawsAmmo->nInstDamage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::FirstUpdate()
//
//	PURPOSE:	First "real" update
//
// ----------------------------------------------------------------------- //

void CSimpleAI::FirstUpdate()
{
	if( !m_strLeashFromName.empty() )
	{
		HOBJECT hFoundObj = LTNULL;

		if( LT_OK == FindNamedObject(m_strLeashFromName.c_str(),hFoundObj) )
		{
			g_pLTServer->GetObjectPos(hFoundObj,&m_vLeashFrom);
		}
		else
		{
#ifndef _FINAL
			g_pLTServer->CPrint("SimpleAI \"%s\" given leash from unknown object named \"%s\".",
				g_pLTServer->GetObjectName(m_hObject), m_strLeashFromName.c_str() );
			g_pLTServer->CPrint("   Using initial position as leash point.");
#endif
			g_pLTServer->GetObjectPos(m_hObject, &m_vLeashFrom);
		}
	}
	else
	{
		g_pLTServer->GetObjectPos(m_hObject, &m_vLeashFrom);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void CSimpleAI::Update()
{
	if( m_bFirstUpdate )
	{
		FirstUpdate();
		m_bFirstUpdate = LTFALSE;
	}

	// Prevent lateral drifting, except when strafing on purpose
	if (!(m_cmMovement.GetControlFlags() & (CM_FLAG_STRAFERIGHT | CM_FLAG_STRAFELEFT)))
	{
		m_cmMovement.UpdateVelocity(.001f, CM_DIR_FLAG_X_AXIS);
	}

	AvoidSimpleAI();

#ifndef _FINAL
	// Update our info string (if needed).
	if( InfoAIListGoals() || InfoAIState() )
	{
		if( !m_bSentInfo )
		{
			std::string info = g_pLTServer->GetObjectName(m_hObject);
			info += "\nSimple AI";

			UpdateInfoString( const_cast<char*>(info.c_str()) );

			m_bSentInfo = LTTRUE;
		}
	}
	else if( m_bSentInfo )
	{
		UpdateInfoString( LTNULL );
		m_bSentInfo = LTFALSE;
	}
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::IgnoreDamageMsg()
//
//	PURPOSE:	Force SimpleAI to target its most recent damager
//
// ----------------------------------------------------------------------- //
LTBOOL CSimpleAI::IgnoreDamageMsg(const DamageStruct & damage_msg)
{
	if( m_bHidden )
		return LTTRUE;

	if( damage_msg.hDamager )
	{
		CCharacter * pCharDamager = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(damage_msg.hDamager) );
		CPlayerObj * pPlayerDamager = dynamic_cast<CPlayerObj*>( g_pLTServer->HandleToObject(damage_msg.hDamager) );

		if( !pPlayerDamager && pCharDamager && g_pCharacterMgr->AreAllies(*this, *pCharDamager) )
		{
			return LTTRUE;
		}
	}

	return CCharacter::IgnoreDamageMsg(damage_msg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::ProcessDamageMsg()
//
//	PURPOSE:	Force SimpleAI to target its most recent damager
//
// ----------------------------------------------------------------------- //
void CSimpleAI::ProcessDamageMsg(const DamageStruct & damage_msg)
{
	if( damage_msg.hDamager )
	{
		// Only target other characters, be sure the other character
		// is not an ally (but they can be neutral!).
		const CCharacter * pChar = dynamic_cast<const CCharacter *>( g_pLTServer->HandleToObject(damage_msg.hDamager) );
		if( pChar && !g_pCharacterMgr->AreAllies(*this,*pChar) )
		{
			m_hTarget = damage_msg.hDamager;
			m_pCharTarget = pChar;
		}
	}

	CCharacter::ProcessDamageMsg(damage_msg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::ProcessCommand()
//
//	PURPOSE:	Handles trigger commands
//
// ----------------------------------------------------------------------- //
LTBOOL CSimpleAI::ProcessCommand(const char* const *pTokens, int nArgs)
{
	if (!pTokens || !pTokens[0] || nArgs < 1) return LTFALSE;

	if (stricmp(TRIGGER_TARGET, pTokens[0]) == 0)
	{
		if( nArgs > 1 )
		{
			ObjArray<HOBJECT,1> objArray;
			if ((g_pLTServer->FindNamedObjects(const_cast<char *>(pTokens[1]), objArray) == LT_OK)
				&& objArray.NumObjects() > 0)
			{
				m_hTarget = objArray.GetObject(0);
				m_pCharTarget = dynamic_cast<const CCharacter *>( m_hTarget ? g_pLTServer->HandleToObject(m_hTarget) : LTNULL );

				// Make sure we are "awake".
				g_pLTServer->SetDeactivationTime(m_hObject, c_fSimpleAIDeactivationTime);
			}
#ifndef _FINAL
			else
			{
				AIErrorPrint(this, "Unable to target %s", pTokens[1]);
			}
#endif
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command \"TRG\" with no arguments!");
			AIErrorPrint(m_hObject, "Usage : \"TRG target_name\"." );
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_REMOVE, pTokens[0]) == 0 )
	{
		RemoveObject();
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, "HIDDEN") )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			Hidden( LTTRUE == IsTrueChar(pTokens[0][0]) );
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command \"HIDDEN\" with %d arguments.", nArgs);
			AIErrorPrint(m_hObject, "Usage : HIDDEN [0 | 1]" );
		}
#endif
		return LTTRUE;
	}

	return CCharacter::ProcessCommand(pTokens, nArgs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::IsEnemyInRange()
//				CSimpleAI::IsPlayerInRange()
//				CSimpleAI::IsCharacterInRange()
//				CSimpleAI::IsSimpleAIInRange()
//
//	PURPOSE:	Determine if a player or AI has crossed a range threshold
//
// ----------------------------------------------------------------------- //

CCharacter * CSimpleAI::IsEnemyInRange(LTFLOAT fRangeSqr)
{
	if (m_bIgnoreAI)
		return (CCharacter *)IsPlayerInRange(fRangeSqr);
	else
		return IsCharacterInRange(fRangeSqr);
}

CPlayerObj * CSimpleAI::IsPlayerInRange(LTFLOAT fRangeSqr)
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

			if( g_pCharacterMgr->AreEnemies(*pPlayer,*this) )
			{
				const LTFLOAT fDistSqr = VEC_DISTSQR(vPos, pPlayer->GetPosition());
				if (fDistSqr < fRangeSqr)
				{
					return pPlayer;
				}
			}
		}
	}

	return LTNULL;
}

CCharacter * CSimpleAI::IsCharacterInRange(LTFLOAT fRangeSqr)
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

			AIPlayer* pAIplayer = dynamic_cast<AIPlayer*>(*iter);

			// ignore self
			if (pChar->m_hObject == m_hObject || pAIplayer)
				continue;

			if( g_pCharacterMgr->AreEnemies(*pChar,*this) )
			{
				const LTFLOAT fDistSqr = VEC_DISTSQR(vPos, pChar->GetPosition());
				if (fDistSqr < fRangeSqr)
				{
					return pChar;
				}
			}
		}
	}

	return LTNULL;
}

CSimpleAI * CSimpleAI::IsSimpleAIInRange(LTFLOAT fRangeSqr)
{
	LTVector vPos;
	CSimpleAI *pSimpleAI;

	g_pInterface->GetObjectPos(m_hObject, &vPos);

	for (SimpleAIIterator iter = sm_SimpleAI.begin();
		 iter != sm_SimpleAI.end(); ++iter)
	{
		if (*iter)
		{
			pSimpleAI = *iter;
			const LTFLOAT fDistSqr = VEC_DISTSQR(vPos, pSimpleAI->GetPosition());
			if (fDistSqr < fRangeSqr)
			{
				if (pSimpleAI->m_hObject != m_hObject)
					return pSimpleAI;
			}
		}
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::WithinLeash()
//
//	PURPOSE:	Is this SimpleAI still inside it's leash limit?
//
// ----------------------------------------------------------------------- //

LTBOOL CSimpleAI::WithinLeash(const LTVector & vPos)
{
	if( m_fLeashLengthSqr > 0.0f && m_fLeashLengthSqr < (vPos - m_vLeashFrom).MagSqr() )
		return LTFALSE;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::AvoidSimpleAI()
//
//	PURPOSE:	Uses strafing movement to push away from other SimpleAI.
//				This is important for wall-walkers who are nonsolid.
//
// ----------------------------------------------------------------------- //

void CSimpleAI::AvoidSimpleAI()
{
	// Check for clipping.
	int nNumChecked = 0;
	int nWeightedSum = 0;
	{for( SimpleAIList::iterator iter = sm_SimpleAI.begin();
	     iter != sm_SimpleAI.end(); ++iter )
	{
		++nNumChecked;
		if (*iter != this 
			&& BoxIntersection(m_cmMovement.GetPos(), m_cmMovement.GetObjectDims(),
			(*iter)->m_cmMovement.GetPos(), (*iter)->m_cmMovement.GetObjectDims()))
		{
			if ((m_cmMovement.GetPos() - (*iter)->m_cmMovement.GetPos()).Dot(m_cmMovement.GetRight()) > 0.0f)
				nWeightedSum += nNumChecked;
			else
				nWeightedSum -= nNumChecked;
		}
	}}

	{for( CCharacterMgr::PlayerIterator iter = g_pCharacterMgr->BeginPlayers();
	      iter != g_pCharacterMgr->EndPlayers(); ++iter )
	{
		++nNumChecked;
		if ( BoxIntersection(m_cmMovement.GetPos(), m_cmMovement.GetObjectDims(),
			(*iter)->GetPosition(), (*iter)->GetDims()))
		{
			if ((m_cmMovement.GetPos() - (*iter)->GetPosition()).Dot(m_cmMovement.GetRight()) > 0.0f)
				nWeightedSum += nNumChecked;
			else
				nWeightedSum -= nNumChecked;
		}
	}}

	if (nWeightedSum == 0)
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() & ~(CM_FLAG_STRAFERIGHT | CM_FLAG_STRAFELEFT));
	else if (nWeightedSum > 0)
		 m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_STRAFERIGHT);
	else
		m_cmMovement.SetControlFlags(m_cmMovement.GetControlFlags() | CM_FLAG_STRAFELEFT);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::HandleModelString()
//
//	PURPOSE:	Handles model string data ("Fire" by default)
//
// ----------------------------------------------------------------------- //

void CSimpleAI::HandleModelString(ArgList* pArgList)
{
	if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) 
		return;

	char* pKey = pArgList->argv[0];
	if (!pKey) 
		return;

	if (stricmp(pKey, WEAPON_KEY_FIRE) == 0 || stricmp(pKey, "Fire") == 0)
	{
		LTVector vDir, vPos;
		g_pInterface->GetObjectPos(m_hObject, &vPos);
		vDir = m_cmMovement.GetForward();
		vDir.Norm();
		vDir *= m_fMaxMeleeRange;

		IntersectQuery IQuery;
		IntersectInfo IInfo;
		IInfo.m_hObject = LTNULL;
		
		IQuery.m_From	= vPos;
		IQuery.m_To		= vPos + vDir;
		IQuery.m_FilterFn = LTNULL;
		IQuery.m_pUserData = LTNULL;

		IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;
		
		g_pLTServer->IntersectSegment(&IQuery, &IInfo);

		if (IInfo.m_hObject)
		{
			HOBJECT hObj;
			CCharacterHitBox *pHitBox;
			
			pHitBox = dynamic_cast<CCharacterHitBox *>(g_pInterface->HandleToObject(IInfo.m_hObject));
			if (pHitBox)
				hObj = pHitBox->GetModelObject();
			else
				hObj = IInfo.m_hObject;

			// Send damage message...
			DamageStruct damage;
			
			damage.eType	= DT_ALIEN_CLAW;
			damage.fDamage	= m_fMeleeDamage*GetDifficultyFactor();
			damage.hDamager = m_hObject;
			
			damage.DoDamage(this, hObj);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Hidden()
//
//	PURPOSE:	Hide or unhide ourself.
//
// ----------------------------------------------------------------------- //

void CSimpleAI::Hidden(bool hide, bool save_flags /* = true */)
{
	ASSERT( GetMovement() );

	if( !hide && m_bHidden )
	{
		m_bHidden = LTFALSE;

		m_cmMovement.SetObjectFlags(OFT_Flags, m_dwActiveFlags );
		m_cmMovement.SetObjectFlags(OFT_Flags2, m_dwActiveFlags2 );
		m_cmMovement.SetObjectUserFlags(m_dwActiveUserFlags );
		
		m_dwActiveFlags = 0;
		m_dwActiveFlags2 = 0;
		m_dwActiveUserFlags = 0;

		SimpleAIList::iterator iter = std::find(sm_SimpleAI.begin(), sm_SimpleAI.end(), this);
		ASSERT( iter == sm_SimpleAI.end() );
		if( iter == sm_SimpleAI.end() )
			sm_SimpleAI.push_back(this);

		g_pCharacterMgr->Add(this);

		m_damage.SetCanDamage(m_bActiveCanDamage);


		if( GetHitBox() )
		{
			if( CCharacterHitBox * pHitBox = dynamic_cast<CCharacterHitBox*>( g_pLTServer->HandleToObject( GetHitBox() ) ) )
			{
				pHitBox->SetCanBeHit(LTTRUE);
			}
		}

		g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);
		g_pLTServer->SetNextUpdate(m_hObject, GetRandom(0.0f, m_fUpdateDelta) );
	}
	else if( hide && !m_bHidden )
	{
		m_bHidden = LTTRUE;

		// Shall we save our flags?
		if( save_flags )
		{
			m_dwActiveFlags = m_cmMovement.GetObjectFlags(OFT_Flags);
			m_dwActiveFlags2 = m_cmMovement.GetObjectFlags(OFT_Flags2);
			m_dwActiveUserFlags = m_cmMovement.GetObjectUserFlags();
		}
		
		// Set all flags to zero.
		m_cmMovement.SetObjectFlags(OFT_Flags,0);
		m_cmMovement.SetObjectFlags(OFT_Flags2,0);
		m_cmMovement.SetObjectUserFlags(0);

		// Remove ourself from the character list so that no
		// one thinks we're still around.
		g_pCharacterMgr->Remove(this);

		sm_SimpleAI.remove(this);

		if( save_flags )
		{
			m_bActiveCanDamage = m_damage.GetCanDamage();
		}

		m_damage.SetCanDamage(LTFALSE);

		// Update the attachments so that they get a chance to make themselves invisible.
		GetAttachments()->Update();

		// Be sure our hit box is set to non-hittable.
		if( GetHitBox() )
		{
			if( CCharacterHitBox * pHitBox = dynamic_cast<CCharacterHitBox*>( g_pLTServer->HandleToObject( GetHitBox() ) ) )
			{
				pHitBox->SetCanBeHit(LTFALSE);
			}
		}

		g_pLTServer->SetObjectState( m_hObject, OBJSTATE_INACTIVE );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Save()
//
//	PURPOSE:	Save current state
//
// ----------------------------------------------------------------------- //

void CSimpleAI::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite)
		return;

	*hWrite << m_fDetectionRadiusSqr;
	*hWrite << m_fAlertRadiusSqr;
	*hWrite << m_hTarget;
	// m_pCharTarget will be set at load.

	*hWrite << m_bFirstUpdate;
	*hWrite << m_fUpdateDelta;
	*hWrite << m_bIgnoreAI;

	*hWrite << m_strLeashFromName;
	*hWrite << m_fLeashLengthSqr;
	*hWrite << m_vLeashFrom;

	*hWrite << m_fMaxMeleeRange;
	*hWrite << m_fMinMeleeRange;
	*hWrite << m_fMeleeDamage;

	*hWrite << m_bHidden;
	*hWrite << m_dwActiveFlags;
	*hWrite << m_dwActiveFlags2;
	*hWrite << m_dwActiveUserFlags;
	*hWrite << m_bActiveCanDamage;

	// Don't save m_bSentInfo.

	*hWrite << m_strMusicAmbientTrackSet;
	*hWrite << m_strMusicWarningTrackSet;
	*hWrite << m_strMusicHostileTrackSet;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAI::Load()
//
//	PURPOSE:	Load data
//
// ----------------------------------------------------------------------- //

void CSimpleAI::Load(HMESSAGEREAD hRead)
{
	if (!hRead)
		return;

	*hRead >> m_fDetectionRadiusSqr;
	*hRead >> m_fAlertRadiusSqr;
	*hRead >> m_hTarget;
	m_pCharTarget = dynamic_cast<const CCharacter *>( m_hTarget ? g_pLTServer->HandleToObject(m_hTarget) : LTNULL );

	*hRead >> m_bFirstUpdate;
	*hRead >> m_fUpdateDelta;
	*hRead >> m_bIgnoreAI;

	*hRead >> m_strLeashFromName;
	*hRead >> m_fLeashLengthSqr;
	*hRead >> m_vLeashFrom;

	*hRead >> m_fMaxMeleeRange;
	*hRead >> m_fMinMeleeRange;
	*hRead >> m_fMeleeDamage;

	LTBOOL bNewHidden;
	*hRead >> bNewHidden;
	*hRead >> m_dwActiveFlags;
	*hRead >> m_dwActiveFlags2;
	*hRead >> m_dwActiveUserFlags;
	*hRead >> m_bActiveCanDamage;

	*hRead >> m_strMusicAmbientTrackSet;
	*hRead >> m_strMusicWarningTrackSet;
	*hRead >> m_strMusicHostileTrackSet;

	Hidden( bNewHidden == LTTRUE, false);

	// Reset sent info so that it will be re-sent.
	m_bSentInfo = LTFALSE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////////////////////       CSimpleAIPlugin         //////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

static bool		s_bInitialized = false;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleAIPlugin::PreHook_EditStringList
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DRESULT	CSimpleAIPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength)
{
	// Make sure we have the butes file loaded.
	if( !s_bInitialized )
	{
		char szFile[256];

		// Initialize the music mgr.
		if( g_pMusicButeMgr && !g_pMusicButeMgr->IsValid( ))
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, "attributes\\music.txt" );
#else
			sprintf(szFile, "%s/%s", szRezPath, "Attributes/music.txt" );
#endif
			g_pMusicButeMgr->SetInRezFile(LTFALSE);
			g_pMusicButeMgr->Init(g_pLTServer, szFile);
		}

		s_bInitialized = true;
	}

	// Handle the music properties.
	if( stricmp("AmbientTrackSet",szPropName) == 0 ||
	    stricmp("WarningTrackSet",szPropName) == 0 ||
		stricmp("HostileTrackSet",szPropName) == 0 )
	{
		// Make sure the music butes were initialized ok.
		if( g_pMusicButeMgr && g_pMusicButeMgr->IsValid( ))
		{
			CMusicButeMgr::TrackSetList lstTrackSets;

			// Add a default entry.
			strcpy( *aszStrings, g_szDefault );
			++aszStrings;
			++(*pcStrings);

			// Add a silent entry.
			strcpy( *aszStrings, g_szSilent );
			++aszStrings;
			++(*pcStrings);

			// Get the tracksets.
			if( g_pMusicButeMgr->GetTrackSetsListForUI( lstTrackSets ))
			{
				// Add the tracksets to the listbox.
				for( CMusicButeMgr::TrackSetList::iterator iter = lstTrackSets.begin();
					 iter != lstTrackSets.end(); ++iter )
				{
					strcpy( *aszStrings, iter->c_str( ));
					++aszStrings;
					++(*pcStrings);
				}
			}
		}

		return LT_OK;
	}

	// No one wants it

	return LT_UNSUPPORTED;
}

