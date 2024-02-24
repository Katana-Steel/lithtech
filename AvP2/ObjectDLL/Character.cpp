// ----------------------------------------------------------------------- //
//
// MODULE  : Character.cpp
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Character.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"
#include "stdio.h"
#include "BodyProp.h"
#include "VolumeBrush.h"
#include "Spawner.h"
#include "SurfaceFunctions.h"
#include "CharacterMgr.h"
#include "CVarTrack.h"
#include "ServerSoundMgr.h"
#include "modellt.h"
#include "transformlt.h"
#include "ObjectMsgs.h"
#include "DialogueWindow.h"
#include "SFXMsgIds.h"
#include "Attachments.h"
#include "SurfaceMgr.h"
#include "SharedFXStructs.h"
#include "MsgIDs.h"
#include "GameServerShell.h"
#include "AIVolumeMgr.h"
#include "CharacterHitBox.h"
#include "AIUtils.h"
#include "ModelButeMgr.h"
#include "PredDisk.h"
#include "Explosion.h"
#include "PlayerObj.h"
#include "CharacterFuncs.h"
#include "CVarTrack.h"
//#include "TargetingSprite.h"
#include "PickupObject.h"
#include "SoundButeMgr.h"
#include "MultiplayerMgr.h"
#include "ClientServerShared.h"
#include "ClientWeaponSFX.h"
#include "WeaponFXTypes.h"
#include "SimpleNodeMgr.h"
#include "SimpleNode.h"
#include "SimpleAIAlien.h"
#include "GameStartPoint.h"

#include "AI.h"
#include "AISenseMgr.h"

// [KLS] Added 9/6/01 for IsExternalCameraActive() call
#include "Camera.h"
// [KLS] End 9/6/01 changes 


#include <vector>

CVarTrack	g_cvarExoRunRate;
CVarTrack	g_cvarExoWalkRate;
CVarTrack	g_cvarExoCrouchRate;
CVarTrack	g_cvarExoJumpRate;
CVarTrack	g_cvarExoClimbRate;
CVarTrack	g_cvarExoRegenRate;
CVarTrack	g_cvarExoDepriveRate;
CVarTrack	g_cvarExoFiringRate;

CVarTrack	g_cvarChestbursterMaxScale;
CVarTrack	g_cvarChestbursterMutateTime;
CVarTrack	g_cvarChestbursterMutateDelay;

CVarTrack	g_cvarBattRechargeRate;
CVarTrack	g_cvarBattFLUseRate;
CVarTrack	g_cvarBattNVUseRate;
CVarTrack	g_cvarBattSystem;
CVarTrack	g_cvarMPUseMod;

CVarTrack	g_cvarMinDialogueTime;

CVarTrack	g_cvarDemaskChance;

CVarTrack	g_cvarAcidHitRad;
CVarTrack	g_cvarMPAcidHitRatio;
CVarTrack	g_cvarPredAcidHitRatio;
CVarTrack	g_cvarOtherAcidHitRatio;

CVarTrack	g_cvarDamageHistoryTime;

CVarTrack	g_cvarCloakDelayTime;
CVarTrack	g_cvarCloakCostPerSecond;
CVarTrack	g_cvarCloakInitialCost;

CVarTrack	g_cvarSiftMultiMod;
CVarTrack	g_cvarMediMultiMod;

CVarTrack	g_cvarNetDamage;
CVarTrack	g_cvarNetTime;

CVarTrack	g_cvarBlinkMinDelay;
CVarTrack	g_cvarBlinkMaxDelay;
CVarTrack	g_cvarBlinkOnDelay;


#ifdef _DEBUG
//#include "DebugLineSystem.h"
//#define LAST_VOLUME_DEBUG
#endif

//#define PROF_CHARACTER_UPDATE
#ifdef PROF_CHARACTER_UPDATE

CVarTrack   g_cvarMaxTickDelay;

static CTimer g_tmrTickCheck;
static LTCounter character_counter;
static uint32 g_max_update_ticks = 0;
static uint32 g_max_updatea_ticks = 0;
static uint32 updatea_ticks = 0;
static uint32 g_max_updateb_ticks = 0;
static uint32 updateb_ticks = 0;
static uint32 g_max_updatec_ticks = 0;
static uint32 updatec_ticks = 0;
static uint32 g_max_updated_ticks = 0;
static uint32 updated_ticks = 0;
static uint32 g_max_updatee_ticks = 0;
static uint32 updatee_ticks = 0;

#endif

#ifndef _FINAL
static bool ShouldShowLighting(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

	static CVarTrack vtShowModelLighting(g_pLTServer,"ShowModelLighting",0.0f);

	return vtShowModelLighting.GetFloat() > 0.0f && ShouldShow(hObject);
}
#endif

BEGIN_CLASS(CCharacter)
	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP1, 0)

	// Hide some attributes from destructible (these are set-up in attribute overrides below).
	ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(Armor, 100.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxArmor, 100.0f, PF_HIDDEN)

	ADD_STRINGPROP_FLAG_HELP(CharacterType, "Harris", PF_STATICLIST, "Type of character." )
	ADD_BOOLPROP_FLAG_HELP(MoveToFloor, LTTRUE, 0, "Move character down to floor when game loads" )
	ADD_BOOLPROP_FLAG_HELP(ShowDeadBody, LTTRUE, 0, "Create body prop when character dies.")
	ADD_STRINGPROP_FLAG_HELP(SpawnItem, "", 0, "Item spawned when character killed.")

	ADD_LONGINTPROP_FLAG(ClientID, 0, PF_HIDDEN)

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP3)

		// Attachments
		ADD_HUMANATTACHMENTS_AGGREGATE(PF_GROUP3)

		// Basic attributes
		ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1)

		ADD_STRINGPROP_FLAG_HELP(ArmorPoints, "", PF_GROUP3, "Initial armor points.")
		ADD_STRINGPROP_FLAG_HELP(HitPoints, "", PF_GROUP3,   "Initial hit points.")
/*
		ADD_STRINGPROP_FLAG(Lag,			"", PF_GROUP3|PF_RADIUS)
*/

END_CLASS_DEFAULT_FLAGS_PLUGIN(CCharacter, GameBase, NULL, NULL, CF_HIDDEN, CCharacterPlugin)


#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"
#define KEY_SET_DIMS			"SETDIMS"
#define KEY_MOVE				"MOVE"
#define KEY_PLAYSOUND			"PLAYSOUND"
#define KEY_PLAYVOICE			"PLAYVOICE"
#define KEY_COMMAND				"CMD"
#define KEY_CHESTBURSTFX		"CHESTBURSTFX"

#define TRIGGER_PLAY_SOUND		"PLAYSOUND"
#define TRIGGER_HIT_NODE		"HITNODE"
#define TRIGGER_PLAY_ANIM		"PLAYANIM"
#define TRIGGER_PLAY_ANIM_LOOP	"PLAYANIMLOOP"
#define TRIGGER_PLAY_SET		"PLAYSET"
#define TRIGGER_STOP_ANIM		"STOPANIM"
#define TRIGGER_EXPRESSION		"EXPRESSION"
#define TRIGGER_DEMASK			"DEMASK"
#define TRIGGER_GIVE_MASK		"GIVEMASK"
#define TRIGGER_EYE_FLASH		"EYEFLASH"
#define TRIGGER_CLOAK			"CL"
#define TRIGGER_EXOSUIT_DISABLE	"EXOSUITDISABLE"
#define TRIGGER_EXOSUIT_ENABLE	"EXOSUITENABLE"
#define TRIGGER_EXOSUIT_EXIT	"EXOSUITEXIT"
#define TRIGGER_AIM_YAW			"AIMYAW"
#define TRIGGER_AIM_PITCH		"AIMPITCH"
#define TRIGGER_CHARACTER		"CHARACTER"

// TRIGGER_ATTACH in Character.h
// TRIGGER_DETACH in Character.h

#define DIMS_EPSILON			0.5f

#define AIR_SUPPLY_REGEN_SCALE	15.0f
#define AIR_SUPPLY_DEGRADE_SLOW	3.0f
#define AIR_SUPPLY_DEGRADE_FAST	5.0f
#define AIR_DEPRIVED_DAMAGE		5.0f

#define CLOAK_ENERGY_USE_TIME	1.0f		// This is the amount of time it takes for one energy unit to be used
#define CLOAK_RAMPUP_TIME		2.0f		// The amount of time it takes to cloak
#define CLOAK_RAMPDOWN_TIME		1.0f		// The amount of time it takes to decloak


const int MP_EXOSUIT_HP_MULTIPLIER = 6;
const int CST_NO_IMPORTANCE = -1;
const int CST_DAMAGE_IMPORTANCE = 1;
const int CST_EXCLAMATION_IMPORTANCE = 2;
const int CST_DIALOGUE_IMPORTANCE = 3;

const int CHARACTER_BLOCKING_PRIORITY = 30;

// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ShouldSpewAcidDamage()
//
//	PURPOSE:	Should we make an acid spew explosion
//
// ----------------------------------------------------------------------- //
LTBOOL ShouldSpewAcidOrJib(DamageType dt)
{
	//Only do this if we can be damaged...
	switch (dt)
	{
		//none of these support spewing acid or jibs
	case (DT_ACID):
	case (DT_BURN):
	case (DT_EXPLODE):
	case (DT_CHOKE):
	case (DT_CRUSH):
	case (DT_ELECTRIC):
	case (DT_FREEZE):
	case (DT_POISON):
	case (DT_STUN):
	case (DT_ENDLESS_FALL):
	case (DT_EMP):
	case (DT_NAPALM):
	case (DT_FACEHUG):
		return LTFALSE;

		//everything else does
	default:
		return LTTRUE;
	}
}

// ----------------------------------------------------------------------- //

// Please update these if you modify the CloakAmountFromState function!
LTFLOAT fMinCloakingAlpha = 0.0f;
LTFLOAT fMaxCloakingAlpha = 0.15f;

static LTFLOAT CloakAmountFromState(CharacterMovementState cms, uint32 dwFlags)
{
	switch(cms)
	{
		case CMS_WALKING:
			return 0.05f;

		case CMS_CRAWLING:
		{
			if(dwFlags & CM_FLAG_DIRECTIONAL)
				return 0.025f;
			else
				return 0.0f;
		}

		case CMS_CLIMBING:
		case CMS_SWIMMING:
		{
			if(dwFlags & (CM_FLAG_DIRECTIONAL | CM_FLAG_JUMP | CM_FLAG_DUCK))
				return 0.05f;
			else
				return 0.0f;
		}

		case CMS_RUNNING:
			return 0.15f;

		case CMS_STAND_JUMP:
		case CMS_WALK_JUMP:
		case CMS_RUN_JUMP:
		case CMS_WWALK_JUMP:
		case CMS_STAND_FALL:
		case CMS_WALK_FALL:
		case CMS_RUN_FALL:
		case CMS_WWALK_FALL:
			return 0.15f;
	}

	return 0.0f;
}

static LTFLOAT FootStepStateVolumeMod(CharacterMovementState cms)
{
	// If you change this, please keep FootStepStateVolumeMod in CharacterFX.cpp
	// in sync.
	switch(cms)
	{
		case CMS_RUNNING:
			return 1.0f;

		case CMS_WALLWALKING:
		case CMS_CRAWLING:
			return 0.333f;

		default:
			return 0.5f;
	}

	return 1.0f;
}

void CacheFootStepSounds(const char * szFootStepSoundDir)
{
	char szSound[255];

	if( !g_pLTServer || !szFootStepSoundDir || !szFootStepSoundDir[0] )
	{
		return;
	}

	SurfaceList * pSurfaceList = g_pSurfaceMgr->GetSurfaceList();

	if( pSurfaceList )
	{
		SURFACE** pCur = pSurfaceList->GetItem(TLIT_FIRST);
		
		while( pCur )
		{
			if( *pCur )
			{
				if( (*pCur)->szRtFootStepSnds[0] &&  (*pCur)->szRtFootStepSnds[0][0] )
				{
					sprintf(szSound, (*pCur)->szRtFootStepSnds[0], szFootStepSoundDir);
					if( LT_NOTFOUND == g_pLTServer->CacheFile( FT_SOUND, szSound ) )
					{
					}
				}

				if( (*pCur)->szRtFootStepSnds[1] &&  (*pCur)->szRtFootStepSnds[1][0] )
				{
					sprintf(szSound, (*pCur)->szRtFootStepSnds[1], szFootStepSoundDir);
					if( LT_NOTFOUND == g_pLTServer->CacheFile( FT_SOUND, szSound ) )
					{
#ifndef _FINAL
//						g_pLTServer->CPrint( "WARNING : Sound file \"%s\" not found.",
//							(*pCur)->szRtFootStepSnds[1] );
#endif
					}
				}

				if( (*pCur)->szLtFootStepSnds[0] &&  (*pCur)->szLtFootStepSnds[0][0] )
				{
					sprintf(szSound, (*pCur)->szLtFootStepSnds[0], szFootStepSoundDir);
					if( LT_NOTFOUND == g_pLTServer->CacheFile( FT_SOUND, szSound ) )
					{
#ifndef _FINAL
//						g_pLTServer->CPrint( "WARNING : Sound file \"%s\" not found.",
//							(*pCur)->szLtFootStepSnds[0] );
#endif
					}
				}

				if( (*pCur)->szLtFootStepSnds[1] &&  (*pCur)->szLtFootStepSnds[1][0] )
				{
					sprintf(szSound, (*pCur)->szLtFootStepSnds[1], szFootStepSoundDir);
					if( LT_NOTFOUND == g_pLTServer->CacheFile( FT_SOUND, szSound ) )
					{
#ifndef _FINAL
//						g_pLTServer->CPrint( "WARNING : Sound file \"%s\" not found.",
//							(*pCur)->szLtFootStepSnds[1] );
#endif
					}
				}

			}
			
			pCur = pSurfaceList->GetItem(TLIT_NEXT);
		}
	}	
}


void CacheCharacterFiles( int nCharacterType )
{
	if( nCharacterType < 0 || nCharacterType > g_pCharacterButeMgr->GetNumSets() )
		return;

	g_pServerSoundMgr->CacheSounds( g_pCharacterButeMgr->GetGeneralSoundDir(nCharacterType) );
	g_pSurfaceMgr->CacheFootsteps( nCharacterType );

	// Cache our debris.
	// Get the initial node info...
	ModelNode		eNode		= (ModelNode)0;
	ModelSkeleton	eModelSkele = ModelSkeleton( g_pCharacterButeMgr->GetSkeletonIndex(nCharacterType) );
	const char*		pDebrisType	= g_pModelButeMgr->GetSkeletonNodeGIBType(eModelSkele, eNode);

	for(int i=1; i < g_pModelButeMgr->GetSkeletonNumNodes(eModelSkele); i++)
	{
		MULTI_DEBRIS *pMultiDebris = g_pDebrisMgr->GetMultiDebris(const_cast<char*>(pDebrisType));

		if(pMultiDebris)
		{
			for(int j=0; j<pMultiDebris->nDebrisFX; j++)
			{
				DEBRIS *pDebris = g_pDebrisMgr->GetDebris(pMultiDebris->szDebrisFX[j].szString);

				if(pDebris)
					pDebris->Cache(g_pDebrisMgr);
			}
		}

		// Move on to the next node...
		eNode		= (ModelNode)i;
		pDebrisType = g_pModelButeMgr->GetSkeletonNodeGIBType(eModelSkele, eNode);
	}
}

// ----------------------------------------------------------------------- //

extern CGameServerShell* g_pGameServerShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CCharacter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCharacter::CCharacter() 
	: GameBase(OT_MODEL),
		m_pAttachments(LTNULL),
		m_nCharacterSet( g_pCharacterButeMgr ? g_pCharacterButeMgr->GetSetFromModelType("Harris") : 0),
		m_bCharacterReset(LTTRUE),

		m_pCurrentVolume(LTNULL),
		m_pLastVolume(LTNULL),
		m_vLastVolumePos(0.0f,0.0f,0.0f),

		m_pLastSimpleNode(LTNULL),
		// m_tmrNextSimpleNodeCheck

		m_fAirSupplyTime(-1.0f),
		m_fOldAirSupplyTime(-1.0f),
		m_nCannonChargeLevel(0),

		m_bCreateBody(LTTRUE),

		m_hHitBox(LTNULL),
		m_eModelNodeLastHit(eModelNodeInvalid),

		m_fDefaultHitPts(-1.0f),
		m_fDefaultArmor(-1.0f),

		// m_LastFireInfo inits self
		// m_LastImpactInfo inits self
		// m_LastFootstepSoundInfo inits self

		m_vOldCharacterColor(0.0f,0.0f,0.0f),
		m_fOldCharacterAlpha(1.0f),
		m_bCharacterHadShadow(LTTRUE),

		// m_hstrSpawnItem doesn't need initialization

		m_hCurVcdSnd(LTNULL),
		m_nCurVcdSndImp(CST_NO_IMPORTANCE),
		m_bCurVcdSndLipSynced(LTFALSE),
		m_bCurVcdSndSubtitled(LTFALSE),

		m_fDialogueStartTime(0.0f),
		m_bPlayingTextDialogue(LTFALSE),
		m_bEMPEffect(LTFALSE),
		m_bStunEffect(LTFALSE),
		m_cSpears(0),

		m_bFlashlight(LTFALSE),
		m_vFlashlightPos(0.0f,0.0f,0.0f),

		m_vModelLighting(-1.0f,-1.0f,-1.0f),

		m_bHasFlarePouch(LTFALSE),
		m_bHasCloakDevice(LTFALSE),
		m_bHasMask(LTFALSE),
		m_bHasNightVision(LTFALSE),

		m_fCloakStartTime(0.0f),
		m_nCloakState(CLOAK_STATE_INACTIVE),
//		m_pTargetingSprite(LTNULL),
		m_fBatteryChargeLevel(MARINE_MAX_BATTERY_LEVEL),
		m_bNightvision(LTFALSE),
		m_nDesiredExpression(EXPRESSION_MIN),
		m_fLastBlinkTime(0.0f),
		m_fBlinkDelay(0.0f),
		// m_tmrMinRecoilDelay
		m_fLastAimingPitch(0.0f),
		m_fLastAimingYaw(-1.0f),
		m_nLastAimingPitchSet(0),
		m_nAimingPitchSet(0),
		m_nLastStrafeState(0xFF)
{
	AddAggregate(&m_damage);
	AddAggregate(&m_editable);

	// Move to the floor on the first MID_UPDATE.
	SetMoveToFloor(LTTRUE),

	fpUpdateExoEnergy	= LTNULL;
	m_fEnergyChange		= 0.0f;
	m_bExosuitDisabled	= LTFALSE;
	m_nOldCharacterSet	= m_nCharacterSet;
	m_fOldHealth		= 0.0f;
	m_fOldArmor			= 0.0f;

	m_nChestburstedFrom			= -1;
	m_fChestbursterMutateTime	= 0.0f;
	m_fChestbursterMutateDelay	= 0.0f;

	m_hHead				= LTNULL;
	m_bForceCrawl		= LTFALSE;
	m_fCloakDelayTime	= 0.0f;
	m_fCloakEnergyUsed	= 0.0f;

	m_nPreCreateClientID = 0;
	m_hNet				= LTNULL;
	m_fNetTime			= 0.0f;
	m_nNetDamage		= 0;
	m_bFirstNetUpdate	= LTTRUE;
	m_eNetStage			= NET_STAGE_INVALID;
	m_eLastNetStage		= NET_STAGE_INVALID;
	m_hNetStruggleSound	= LTNULL;
	m_pOldWeapon		= LTNULL;
	m_bConfirmFX		= LTFALSE;
	m_hStartpointSpawner= LTNULL;
	m_bResetAttachments	= LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Reset()
//
//	PURPOSE:	Reset (after death)
//
// ----------------------------------------------------------------------- //

void CCharacter::Reset()
{
	m_nCurVcdSndImp	= CST_NO_IMPORTANCE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::~CCharacter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacter::~CCharacter()
{
	// This should be made an automatic data member to localize this 
	// object's memory allocation...

	DestroyAttachments();

	if(m_hHead)
	{
		// remove the head attachemnt
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hHead, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hHead);
		m_hHead = LTNULL;
	}

	if(m_hNet)
	{
		// remove the net
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hNet, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hNet);
		m_hNet = LTNULL;
	}

	if(m_hNetStruggleSound)
	{
		g_pLTServer->KillSound(m_hNetStruggleSound);
		m_hNetStruggleSound = LTNULL;
	}

	KillVoicedSound();

	if (m_hstrSpawnItem)
	{
		g_pLTServer->FreeString(m_hstrSpawnItem);
	}

	if (m_hHitBox)
	{
		g_pLTServer->RemoveObject(m_hHitBox);
	}

	if( g_pCharacterMgr )
	{
		// Remove us from the active and global lists...
		g_pCharacterMgr->Remove(this);
		g_pCharacterMgr->RemoveFromGlobalList(this);
	}

	// See if we need to tell the exsuit spawner about our death...
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		if(m_hStartpointSpawner)
		{
			GameStartPoint* pStartPoint = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(m_hStartpointSpawner));

			if(pStartPoint)
			{
				pStartPoint->SetExosuitRespawn();
			}

			// Make sure we only do this once
			m_hStartpointSpawner = LTNULL;
		}
	}	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			ArgList *pArgList = (ArgList*)pData;
			if (!pArgList || !pArgList->argv || pArgList->argc == 0)
				break;

			// Let the animation class handle them too
			m_caAnimation.HandleStringKey(pArgList, (uint8)fData);

			HandleModelString(pArgList);
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			CollisionInfo ci;
			g_pLTServer->GetLastCollision(&ci);
			m_cmMovement.OnTouchNotify(&ci, 1.0f);
		}
		break;

		case MID_PRECREATE:
		{
			CreateAttachments();

			uint32 nResult = GameBase::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				{
					ReadProp(pStruct);
					PostReadProp(pStruct);
				}
			}

			return nResult;
		}
		break;

		case MID_INITIALUPDATE:
		{
			// The aggregrates must be initialized before the rest of character!
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			InitialUpdate((int)fData);

			// If we are not in single player then go ahead and 
			// cache our Predator files so we lesson the the lag
			// when a predator joins...
			if(g_pGameServerShell->GetGameType() != SP_STORY)
			{
				CacheFiles();
			}

			// Add ourself to the too be cached list.
			g_pGameServerShell->AddCharacterToCache(GetCharacter());

			if(g_pCharacterMgr)
				g_pCharacterMgr->AddToGlobalList(this);

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
			// Let aggregates go first...

			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			Save((HMESSAGEWRITE)pData);

			return dwRet;
		}
		break;

		case MID_LOADOBJECT:
		{
			// Let aggregates go first...

			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			Load((HMESSAGEREAD)pData);

			return dwRet;
		}
		break;


		case MID_DEACTIVATING:
		{
			if( IsPlayingDialogue() )
			{
				StopDialogue();
			}
			else if( IsPlayingVoicedSound() )
			{
				KillVoicedSound();
			}
		}
		break;
			
		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ObjectMessageFn()
//
//	PURPOSE:	Handler for object to object messages.
//
// --------------------------------------------------------------------------- //

uint32 CCharacter::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_ANIMATION :
		{
			// Read in the animation message data
			const uint8 nTracker = hRead->ReadByte(); 
			const uint8 bLooping = hRead->ReadByte(); 
			const uint8 nMsg = hRead->ReadByte();

			ProcessAnimationMsg(nTracker,bLooping, nMsg);
		}
		break;



		case MID_TRIGGER:
		{
			HSTRING hMsg = g_pLTServer->ReadFromMessageHString(hRead);
			const char *pMsg = g_pLTServer->GetStringData(hMsg);
			
			const LTBOOL bFullyHandled = ProcessTriggerMsg(pMsg);

			g_pLTServer->FreeString(hMsg);

			if( bFullyHandled )
			{
				return LT_OK;
			}
			else
			{
				// Let our base class handle it.

				// Reset the message so nothing goes awry.
				hRead->ResetPos();
			}
		}
		break;

		case MID_DAMAGE:
		{
			DamageStruct damage;
			damage.InitFromMessage(hRead);

			if( !IgnoreDamageMsg(damage) )
			{
				hRead->ResetPos();

				const uint32 dwRet = GameBase::ObjectMessageFn(hSender, messageID, hRead);

				ProcessDamageMsg(damage);
				return dwRet;
			}
			
			return 1;

		}
		break;

		case MID_FORCE_POUNCE:
		case MID_FORCE_FACEHUG:
		{
			//set movement state to pounce
			HOBJECT hTarget = hRead->ReadObject();
			CharacterMovementState cms = m_cmMovement.GetMovementState();

			if(m_cmMovement.GetCharacterVars()->m_bStandingOn || cms == CMS_WALLWALKING)
			{
				CCharacter * const pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hTarget) );

				LTVector vTargetPos;
				g_pLTServer->GetObjectPos(hTarget,&vTargetPos);
				LTVector vTargetDims;
				g_pLTServer->GetObjectDims(hTarget, &vTargetDims);

				LTVector vStartPos;
				g_pLTServer->GetObjectPos(m_hObject, &vStartPos);
				LTVector vStartDims;
				g_pLTServer->GetObjectDims(m_hObject,&vStartDims);
				
				const LTVector vPounceDims = m_cmMovement.GetDimsSetting(CM_DIMS_POUNCING);

				// Scootch the character up a bit to make room for the new dims.
				vStartPos += GetUp()*(vPounceDims.y - vStartDims.y)*1.1f;
				vTargetPos.y += LTMAX(pChar->GetChestOffset().y, (vPounceDims.y - vTargetDims.y)*1.1f);

				if(m_cmMovement.GetCharacterButes()->m_fBasePounceSpeed > 0.0f)
				{
					LTVector vVel;
					if( pChar )
					{
						vVel = pChar->GetMovement()->GetVelocity();
					}
					else
					{
						g_pLTServer->Physics()->GetVelocity(hTarget,&vVel);
					}

					vTargetPos += vVel*(vTargetPos-vStartPos).Mag()/m_cmMovement.GetCharacterButes()->m_fBasePounceSpeed*1.1f;
				}

				m_cmMovement.SetStart(vStartPos);
				m_cmMovement.SetDestination(vTargetPos, m_cmMovement.GetRot());
				m_cmMovement.SetTimeStamp(g_pLTServer->GetTime());

				LTVector vNull(0.0f, 0.0f, 0.0f);
				m_cmMovement.SetVelocity(vNull);
				m_cmMovement.SetAcceleration(vNull);

				m_cmMovement.SetMovementState((messageID == MID_FORCE_POUNCE) ? CMS_POUNCING : CMS_FACEHUG);

				// Be sure to only set our position after the movement state is set-up so 
				// that we catch touch notifies from the set position.
				m_cmMovement.SetPos(vStartPos);

				if(strstr(m_cmMovement.GetCharacterButes()->m_szName, "Facehugger"))
					PlayExclamationSound("FacehuggerPounceCry");
				else if(strstr(m_cmMovement.GetCharacterButes()->m_szName, "Predalien"))
					PlayExclamationSound("PredalienPounceCry");
				else
					PlayExclamationSound("AlienPounceCry");

				// Set up fire info for Pounce so HearWeaponFire can receive it.
				CharFireInfo char_info;
				char_info.vFiredPos = vTargetPos;
				char_info.vFiredDir = vTargetPos - vStartPos;

				WEAPON *pWeapon = g_pWeaponMgr->GetWeapon("Pounce");
				if (pWeapon)
					char_info.nWeaponId = pWeapon->nId;

				BARREL *pBarrel = g_pWeaponMgr->GetBarrel("AI_Pounce_Barrel");
				if (pBarrel)
					char_info.nBarrelId = pBarrel->nId;

				AMMO *pAmmo = g_pWeaponMgr->GetAmmo("AI_Pounce_Ammo");
				if (pAmmo)
					char_info.nAmmoId = pAmmo->nId;

				char_info.fTime		= g_pLTServer->GetTime();
				SetLastFireInfo(char_info);
			}
		}
		break;

		case MID_PICKUP:
		{
			char pMsg[128];
			uint32 nPUUsedAmt[4];

			hRead->ReadStringFL(pMsg, sizeof(pMsg));
			nPUUsedAmt[0] = g_pLTServer->ReadFromMessageDWord(hRead);
			nPUUsedAmt[1] = g_pLTServer->ReadFromMessageDWord(hRead);
			nPUUsedAmt[2] = g_pLTServer->ReadFromMessageDWord(hRead);
			nPUUsedAmt[3] = g_pLTServer->ReadFromMessageDWord(hRead);

			uint32 nAmt[4]={0,0,0,0};

			LTBOOL bPartialPU=LTFALSE;

			if(ProcessPowerupMsg(pMsg, hSender, nAmt, nPUUsedAmt, LTTRUE, &bPartialPU))
			{
				// Send a message back to let the powerup know it was pickup up
				HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, hSender, MID_PICKEDUP);
				g_pLTServer->WriteToMessageDWord(hWrite, nAmt[0]);
				g_pLTServer->WriteToMessageDWord(hWrite, nAmt[1]);
				g_pLTServer->WriteToMessageDWord(hWrite, nAmt[2]);
				g_pLTServer->WriteToMessageDWord(hWrite, nAmt[3]);
				g_pLTServer->WriteToMessageByte(hWrite, bPartialPU);
				g_pLTServer->EndMessage(hWrite);
			}
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
	if (!pStruct) return LTFALSE;

	if ( g_pLTServer->GetPropGeneric( "MoveToFloor", &genProp ) == LT_OK )
	{
		SetMoveToFloor( genProp.m_Bool );
	}
	if ( g_pLTServer->GetPropGeneric( "ShowDeadBody", &genProp ) == LT_OK )
	{
		m_bCreateBody = genProp.m_Bool;
	}

	if ( g_pLTServer->GetPropGeneric( "SpawnItem", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
			m_hstrSpawnItem = g_pLTServer->CreateString( genProp.m_String );
	}

	if ( g_pLTServer->GetPropGeneric( "HitPoints", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
			m_fDefaultHitPts = genProp.m_Float;
	}

	if ( g_pLTServer->GetPropGeneric( "ArmorPoints", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
			m_fDefaultArmor = genProp.m_Float;
	}

	if ( g_pLTServer->GetPropGeneric( "CharacterType", &genProp ) == LT_OK )
	{
		if(g_pCharacterButeMgr)
		{
			std::string char_set_name = GetCharacterSetName(genProp.m_String);
			m_nCharacterSet = g_pCharacterButeMgr->GetSetFromModelType(char_set_name.c_str());

			if( m_nCharacterSet > uint32(g_pCharacterButeMgr->GetNumSets()) )
			{
#ifndef _FINAL
				g_pLTServer->CPrint("Character set \"%s\" does not exist. Defaulting to set \"%s\".",
					char_set_name.c_str(), g_pCharacterButeMgr->GetModelType(0) );
#endif
				m_nCharacterSet = 0;
			}
		}
	}

	if ( g_pLTServer->GetPropGeneric( "ClientID", &genProp ) == LT_OK )
	{
		m_nPreCreateClientID = genProp.m_Long;
	}
	else
	{
		m_nPreCreateClientID = 0;
	}

	return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PostReadProp
//
//	PURPOSE:	Set values derived from ReadProp or over-ride values.
//
// ----------------------------------------------------------------------- //

void CCharacter::PostReadProp(ObjectCreateStruct *pStruct)
{
	// Set the model filename from the attributes file.
	g_pCharacterButeMgr->GetDefaultFilenames(GetCharacter(), *pStruct);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessTriggerMsg()
//
//	PURPOSE:	Process a trigger message messages.
//
// --------------------------------------------------------------------------- //

static LTBOOL BaseCommand(const char * pCommand)
{
	// This function should return true if GameBase or
	// an aggregrate would understand the command.
	return // GameBase commands
			  0 == stricmp(pCommand, "VISIBLE")
		   || 0 == stricmp(pCommand, "SOLID")
		   || 0 == stricmp(pCommand, "HIDDEN")
		   || 0 == stricmp(pCommand, "REMOVE")
		   || 0 == stricmp(pCommand, "TELEPORT")

		   // Destructible commands
		   || 0 == stricmp(pCommand, "DAMAGE")
		   || 0 == stricmp(pCommand, "DESTROY")
		   || 0 == stricmp(pCommand, "CANDAMAGE")
		   || 0 == stricmp(pCommand, "NEVERDESTROY")
		   || 0 == stricmp(pCommand, "SET_ARMOR")
		   || 0 == stricmp(pCommand, "SET_HEALTH");
}

LTBOOL CCharacter::ProcessTriggerMsg(const char* pMsg)
{

	if (!pMsg) return LTFALSE;

	// This will be set to false if a known base command is used.
	LTBOOL bFullyProcessed = LTTRUE;

	ConParse parse( const_cast<char*>(pMsg) );

	// Iterate through each script command.
	while( LT_OK == g_pLTServer->Common()->Parse(&parse) )
	{
		if (g_pCmdMgr->IsValidCmd(parse))
		{
			g_pCmdMgr->Process(parse);
		}
		else if ( !ProcessCommand(parse.m_Args, parse.m_nArgs) )
		{
			if(    parse.m_nArgs > 0 
				&& parse.m_Args[0] )
			{
#ifndef _FINAL
				if( !BaseCommand(parse.m_Args[0]) )
				{
					AIErrorPrint(m_hObject, "Unrecognized command (\"%s\"), in \"%s\".", parse.m_Args[0], pMsg);
				}
#endif
				bFullyProcessed = LTFALSE;
			}
		}
	}

	return bFullyProcessed;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

LTBOOL CCharacter::ProcessCommand(const char*const* pTokens, int nArgs)
{
	if (!pTokens || !pTokens[0] || nArgs < 1) return LTFALSE;

	// handle sound fx key
	if(stricmp(pTokens[0], "BUTE_SOUND_KEY")==0)
	{
		if (nArgs == 2)
		{
			const char* pSoundBute = pTokens[1];

			if(pSoundBute)
			{
				//play the buted sound
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, pSoundBute);
			}
#ifndef _FINAL
			else
			{
				AIErrorPrint(this, "SoundBute \"%s\" not found.", pSoundBute);
			}
#endif
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command BUTE_SOUND_KEY not understood.  Format \"BUTE_SOUND_KEY sound_type\".",
				TRIGGER_PLAY_SOUND, TRIGGER_PLAY_SOUND);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_PLAY_SOUND, pTokens[0]) == 0 )
	{
		if( nArgs == 2)
		{
			// Get sound name from message...

			const char* pSoundName = pTokens[1];

			if (pSoundName)
			{
				PlayExclamationSound(pSoundName);
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command %s not understood.  Format \"%s sound_type\".",
				TRIGGER_PLAY_SOUND, TRIGGER_PLAY_SOUND);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_HIT_NODE, pTokens[0]) == 0 )
	{
		if( nArgs == 2)
		{
			ModelSkeleton eSkeleton = (ModelSkeleton)m_cmMovement.GetCharacterButes()->m_nSkeleton;

			if(eSkeleton != eModelSkeletonInvalid)
			{
				ModelNode eNode = g_pModelButeMgr->GetSkeletonNode(eSkeleton, pTokens[1]);

				if(eNode != eModelNodeInvalid)
				{
					SetModelNodeLastHit(eNode);
				}
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command %s not understood.  Format \"%s node_name\". ",
				TRIGGER_HIT_NODE, TRIGGER_HIT_NODE);
		}
#endif
		return LTTRUE;
	}
	else if (!_stricmp(TRIGGER_PLAY_ANIM, pTokens[0]) )
	{
		if( nArgs == 2)
		{
			m_caAnimation.PlayAnim(pTokens[1]);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command %s not understood.  Format \"%s anim_name\". ",
				TRIGGER_PLAY_ANIM, TRIGGER_PLAY_ANIM);
		}
#endif
		return LTTRUE;
	}
	else if (!_stricmp(TRIGGER_PLAY_ANIM_LOOP, pTokens[0]))
	{
		if( nArgs == 2)
		{
			m_caAnimation.PlayAnim(pTokens[1], LTTRUE);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command %s not understood.  Format \"%s anim_name\". ",
				TRIGGER_PLAY_ANIM_LOOP, TRIGGER_PLAY_ANIM_LOOP);
		}
#endif
		return LTTRUE;
	}
	else if (!_stricmp(TRIGGER_PLAY_SET, pTokens[0]) )
	{
		if( nArgs == 2)
		{
			m_caAnimation.PlayTrackerSet(pTokens[1]);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command %s not understood.  Format \"%s anim_name\". ",
				TRIGGER_PLAY_SET, TRIGGER_PLAY_SET);
		}
#endif
		return LTTRUE;
	}
	else if (!_stricmp(TRIGGER_STOP_ANIM, pTokens[0]) )
	{
		m_caAnimation.StopAnim();
		return LTTRUE;
	}
	else if (!_stricmp(TRIGGER_EXPRESSION, pTokens[0]) )
	{
		if( nArgs == 2 )
		{
			if(!_stricmp("Normal", pTokens[1]))
			{
				SetAnimExpression(EXPRESSION_NORMAL);
				return LTTRUE;
			}
			else if(!_stricmp("Blink", pTokens[1]))
			{
				SetAnimExpression(EXPRESSION_BLINK);
				return LTTRUE;
			}
			else if(!_stricmp("Scared", pTokens[1]))
			{
				SetAnimExpression(EXPRESSION_SCARED);
				return LTTRUE;
			}
			else if(!_stricmp("Angry", pTokens[1]))
			{
				SetAnimExpression(EXPRESSION_ANGRY);
				return LTTRUE;
			}
			else if(!_stricmp("Curious", pTokens[1]))
			{
				SetAnimExpression(EXPRESSION_CURIOUS);
				return LTTRUE;
			}
			else if(!_stricmp("Pain", pTokens[1]))
			{
				if(g_pGameServerShell->LowViolence())
					SetAnimExpression(EXPRESSION_NORMAL);
				else
					SetAnimExpression(EXPRESSION_PAIN);

				return LTTRUE;
			}
#ifndef _FINAL
			else
			{
				AIErrorPrint(this, "Expression \"%s\" not understood.",
					pTokens[1]);
				AIErrorPrint(this, "  Known expressions:   \"Normal\", \"Blink\", \"Scared\", \"Angry\", \"Curious\", and \"Pain\"." );
			}
#endif
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command %s not understood.  Format \"%s expression_name\". ",
				TRIGGER_EXPRESSION, TRIGGER_EXPRESSION);
		}
#endif
	}
	else if (!_stricmp(TRIGGER_DEMASK, pTokens[0]) )
	{
		CreateMaskPickup();
		return LTTRUE;
	}
	else if (!_stricmp(TRIGGER_GIVE_MASK, pTokens[0]) )
	{
		SetHasMask();
		return LTTRUE;
	}
	else if (!_stricmp(TRIGGER_EYE_FLASH, pTokens[0]) )
	{
		if( nArgs == 4 )
		{
			// Calculate the color
			uint32 nColor = SETRGB((uint32)atoi(pTokens[1]), (uint32)atoi(pTokens[2]), (uint32)atoi(pTokens[3]));

			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, CFX_EYEFLASH);
			g_pLTServer->WriteToMessageDWord(hMessage, nColor);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(this, "Command %s not understood.  Format \"%s red_value green_value blue_value\". ",
				TRIGGER_EYE_FLASH, TRIGGER_EYE_FLASH);
		}
#endif
		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_CLOAK))
	{
		++pTokens;
		--nArgs;

		if (GetCharacterClass() == PREDATOR)
		{
			if (nArgs == 1)
			{
				if (0 == stricmp("on", *pTokens))
				{
					if ((GetCloakState() == CLOAK_STATE_INACTIVE) || 
						(GetCloakState() == CLOAK_STATE_RAMPDOWN))
						HandleCloakToggle();
				}
				else if (0 == stricmp("off",*pTokens))
				{
					if ((GetCloakState() == CLOAK_STATE_ACTIVE) ||
						(GetCloakState() == CLOAK_STATE_RAMPUP))
						HandleCloakToggle();
				}
			}
#ifndef _FINAL
			else
			{
				AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", TRIGGER_CLOAK);
				AIErrorPrint(m_hObject, "Usage : \"%s on\" or \"%s off\".", TRIGGER_CLOAK, TRIGGER_CLOAK );
			}
#endif
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Cloak command sent to non-predator!");
		}
#endif
		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_ATTACH))
	{
		++pTokens;
		--nArgs;

		if (nArgs == 2)
		{
			m_pAttachments->Detach(pTokens[0]);
			m_pAttachments->Attach(pTokens[0],pTokens[1]);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", TRIGGER_ATTACH);
			AIErrorPrint(m_hObject, "Usage : \"%s position attachment\".", TRIGGER_ATTACH);
		}
#endif
		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_DETACH))
	{
		++pTokens;
		--nArgs;

		if (nArgs == 1)
		{
			m_pAttachments->Detach(pTokens[0]);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", TRIGGER_DETACH);
			AIErrorPrint(m_hObject, "Usage : \"%s position\".", TRIGGER_DETACH);
		}
#endif
		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_EXOSUIT_DISABLE))
	{
		SetExosuitEnergy(0);
		m_bExosuitDisabled = LTTRUE;
		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_EXOSUIT_ENABLE))
	{
		m_bExosuitDisabled = LTFALSE;
		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_EXOSUIT_EXIT))
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

		if(pPlayer)
		{
			if(nArgs == 2)
				pPlayer->HandleExosuitEject(_stricmp(pTokens[1], "nopickup"));
			else
				pPlayer->HandleExosuitEject();
		}
		else
		{
			LTVector vPos;
			HandleExosuitEject(vPos);
		}

		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_AIM_YAW))
	{
		if (nArgs == 2)
		{
			m_cmMovement.GetCharacterVars()->m_fAimingYaw = (LTFLOAT)atof(pTokens[1]);
		}

		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_AIM_PITCH))
	{
		if (nArgs == 2)
		{
			m_cmMovement.GetCharacterVars()->m_fAimingPitch = (LTFLOAT)atof(pTokens[1]);
		}

		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, TRIGGER_CHARACTER))
	{
		if (nArgs == 2)
		{
			SetCharacter(g_pCharacterButeMgr->GetSetFromModelType(pTokens[1]));

			if(IsPlayer(m_hObject))
			{
				// Make sure the ammo counter is active...
				HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_RESET_COUNTER);
				g_pServerDE->EndMessage(hMessage);
			}
		}

		return LTTRUE;
	}


	return LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CCharacter::ProcessDamageMsg(const DamageStruct & damage)
{
	// See if we should explode some acid damage
	// If we are not an alien then just recoil.
	if(IsAlien(m_cmMovement.GetCharacterButes()))
	{
		// See if this damage type supports an acid explosion
		if(ShouldSpewAcidOrJib(damage.eType))
		{
			if(m_damage.GetCanDamage())
			{
				// See if we resisted the hit
				LTBOOL bResist = LTFALSE;

				if((m_cmMovement.GetCharacterButes()->m_fDamageResistance < 1.0f) && DamageTypeResistance(damage.eType))
					bResist = LTTRUE;

				if(!bResist)
				{
					// Ok go ahead and make the explosion
					SpewAcidDamage(damage.fDamage);

					// Now see about some jibs...
					HandleAlienDamage();
				}
			}
		}
	}
	else
	{

		if(    GetButes()->m_fRecoilChance > 0.0f
			&& damage.fDamage > GetButes()->m_fMinRecoilDamage
			&& m_caAnimation.IsValidTracker(ANIM_TRACKER_RECOIL) )
		{
			// Play the appropriate recoil animation
			TrackerInfo *pInfo = m_caAnimation.GetTrackerInfo(ANIM_TRACKER_RECOIL);

			if(m_tmrMinRecoilDelay.Stopped() )
			{
				if( pInfo->m_nIndex == -1 )
				{
					LTFLOAT fRecoilDelay = GetRandom(GetButes()->m_fMinRecoilDelay, GetButes()->m_fMaxRecoilDelay);

					if( GetRandom(0.0f,1.0f) < GetButes()->m_fRecoilChance )
					{
						pInfo->m_nIndex = (int)GetModelNodeLastHit();

						if(damage.vDir.Dot(m_cmMovement.GetForward()) > 0.0f)
							pInfo->m_nIndex += 100;

						m_caAnimation.Update();

						fRecoilDelay += LTFLOAT(m_caAnimation.GetAnimLength(ANIM_TRACKER_RECOIL))*0.001f;
					}
					
					m_tmrMinRecoilDelay.Start(fRecoilDelay);
				}
			}
		}
	}


	// Only do this in multiplayer
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		// See if we should try to eject our mask
		if(IsPredator(m_cmMovement.GetCharacterButes()) && HasMask())
		{
			// Just to make this faster... I'll check against a hardcoded value for the mask node
			if(25 == (int)GetModelNodeLastHit() && (GetRandom(0.0f, 1.0f) <= g_cvarDemaskChance.GetFloat()))
			{
				if(damage.fDamage != 0)
					CreateMaskPickup();
			}
		}
	}


	// Play a damage sound...
	if(!m_damage.IsDead())
	{
		// Play our damage sound
		const int nSoundBute = PlayDamageSound(damage.eType);
		if( nSoundBute >= 0 && g_pGameServerShell->GetGameType().IsSinglePlayer() )
		{
			// Notify the AI's about the pain sound.
			for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs();
				 iter != g_pCharacterMgr->EndAIs(); ++iter )
			{
				(*iter)->GetSenseMgr().Pain(g_pSoundButeMgr->GetSoundButes(nSoundBute),this);
			}
		}
	}
	else
	{
		// Death
		if(IsPredator(m_cmMovement.GetCharacterButes()) && HasMask())
		{
			// Predators will always loose their mask when they die.
			CreateMaskPickup(LTNULL, LTFALSE);
			ForceCloakOff();
		}

		// See if this character was an ally of the player
		CheckMissionFailed(damage.hDamager);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessPowerupMsg()
//
//	PURPOSE:	Process a powerup message
//
// --------------------------------------------------------------------------- //

LTBOOL CCharacter::ProcessPowerupMsg(char* pMsg, HOBJECT hSender, uint32* nAmt, uint32* nUsed, LTBOOL bNotifyHud, LTBOOL* pPartialPU)
{
	int i = 0, j = 0;

	// Return true so the pickup will be accepted...
	if(!pMsg || !pMsg[0]) return LTTRUE;

	LTBOOL bRet = LTFALSE;
	if(pPartialPU)
		*pPartialPU = LTFALSE;

	char szMsg[256], szToks[64];

	// Copy the string into a temporary buffer so we can get tokens from it
	strncpy(szMsg, pMsg, 256);

	// Go through the string and find the main tokens
	char *szMainToks[32];
	int nMainToks = 0;

	szMainToks[0] = strtok(szMsg, ",");

	if(szMainToks[0])
	{
		for(i = 1; i < 32; i++)
		{
			szMainToks[i] = strtok(LTNULL, ",");

			if(!szMainToks[i])
				break;
		}

		nMainToks = i;
	}

	uint32 nResidual = 0;

	// Now go through each main token and do something from it
	for(i = 0; i < nMainToks; i++)
	{
		// Copy this token into the temporary sub token string
		strncpy(szToks, szMainToks[i], 64);

		// Go through the main token and find the sub tokens
		char *szSubToks[32];
		int nSubToks = 0;

		szSubToks[0] = strtok(szToks, " ");

		if(szSubToks[0])
		{
			for(j = 1; j < 32; j++)
			{
				szSubToks[j] = strtok(LTNULL, " ");

				if(!szSubToks[j])
					break;
			}

			nSubToks = j;
		}

		if(HandlePickup(szSubToks, nSubToks, hSender, nAmt, nUsed, bNotifyHud, i, &nResidual))
		{
			bRet = LTTRUE;
		}
	}

	if(pPartialPU && nResidual)
		*pPartialPU = LTTRUE;

	// We're done... so return the bool value that says whether we need
	// to send a message to the clients
	return bRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandlePickup()
//
//	PURPOSE:	Process a powerup message
//
// --------------------------------------------------------------------------- //

LTBOOL CCharacter::HandlePickup(char **pToks, int nToks, HOBJECT hSender, uint32* nAmt, uint32* nUsed, LTBOOL bNotifyHud, int nMainTok, uint32* pResidual)
{
	// saftey first!
	if(nMainTok >= 4 || nMainTok < 0)
		nMainTok = 0;

	// Do this based off the number of tokens
	switch(nToks)
	{
		// ----------------------------------------------------------------------- //
		// Check all the single parameter tokens
		case 1:
		{
			if(_stricmp(pToks[0], "NULL") == 0)
			{
				return LTTRUE;
			}
			else if(_stricmp(pToks[0], "CLOAK_DEVICE") == 0)
			{
				if(IsPredator(m_cmMovement.GetCharacterButes()))
				{
					if(!HasCloakDevice())
					{
						SetHasCloakDevice();

						//see if we are a player and send a message
						if (IsPlayer(m_hObject))
						{
							HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_CLOAK_ADDED);
							g_pServerDE->EndMessage(hMessage);
						}

						return LTTRUE; 
					}
				}

				return LTFALSE;
			}
			else if(_stricmp(pToks[0], "EXOSUIT_DEVICE") == 0)
			{
				// See if we don't already have a flare pouch
				if(IsHuman(m_cmMovement.GetCharacterButes()))
				{
					HandleExosuitEntry(hSender);
					return LTTRUE; 
				}

				return LTFALSE;
			}
			else if(_stricmp(pToks[0], "PREDATOR_MASK") == 0)
			{
				if(IsPredator(m_cmMovement.GetCharacterButes()))
				{
					if(!HasMask())
					{
						SetHasMask();

						//see if we are a player and send a message
						if (IsPlayer(m_hObject))
						{
							HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_MASK_ADDED);
							g_pServerDE->EndMessage(hMessage);
						}

						return LTTRUE; 
					}
				}

				return LTFALSE;
			}
			else if(_stricmp(pToks[0], "NIGHT_VISION") == 0)
			{
				if(IsHuman(m_cmMovement.GetCharacterButes()))
				{
					if(!HasNightVision())
					{
						SetHasNightVision();

						//see if we are a player and send a message
						if (IsPlayer(m_hObject))
						{
							HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_NIGHT_VISION_ADDED);
							g_pServerDE->EndMessage(hMessage);
						}

						return LTTRUE; 
					}
				}

				return LTFALSE;
			}
		}
		break;

		// ----------------------------------------------------------------------- //
		// Check all the tokens with 2 parameters
		case 2:
		{
			if(_stricmp(pToks[0], "HEALTH") == 0)
			{
				if(IsHuman(m_cmMovement.GetCharacterButes()))
				{
					// Clear out any burn damage...
					m_damage.ClearProgressiveDamages(DT_NAPALM);

					LTFLOAT fAmount = (LTFLOAT)atof(pToks[1])-nUsed[nMainTok];
					LTFLOAT fNeeded = m_damage.GetMaxHitPoints() - m_damage.GetHitPoints();

					nAmt[nMainTok] = (uint32)ceil(fNeeded);

					if(m_damage.Heal(fAmount))
					{
						// if we are gonna use it all send the 0 code
						if(nAmt[nMainTok] >= fAmount)
						{
							nAmt[nMainTok] = (uint32)ceil(fAmount); 
						}
						else
						{
							// just make sure the pickup does not go away.
							if(pResidual)
								*pResidual = 1;
						}

						//see if we are a player and send a message
						if (IsPlayer(m_hObject))
						{
							HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_HEALTH_ADDED);
							g_pServerDE->EndMessage(hMessage);
						}
						return LTTRUE;
					}
				}
			}
			else if(_stricmp(pToks[0], "ARMOR") == 0)
			{
				// See if we don't already have a flare pouch
				if(IsHuman(m_cmMovement.GetCharacterButes()))
				{
					LTFLOAT fAmount = (LTFLOAT)atof(pToks[1])-nUsed[nMainTok];
					LTFLOAT fNeeded = m_damage.GetMaxArmorPoints() - m_damage.GetArmorPoints();

					nAmt[nMainTok] = (uint32)ceil(fNeeded);

					if(m_damage.Repair(fAmount))
					{
						//see if we are a player and send a message
						// if we are gonna use it all send the 0 code
						if(nAmt[nMainTok] >= fAmount)
						{
							nAmt[nMainTok] = (uint32)ceil(fAmount); 
						}
						else
						{
							// just make sure the pickup does not go away.
							if(pResidual)
								*pResidual = 1;
						}

						if (IsPlayer(m_hObject))
						{
							HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_ARMOR_ADDED);
							g_pServerDE->EndMessage(hMessage);
						}
						return LTTRUE;
					}
				}
			}
			else if(_stricmp(pToks[0], "CHARACTER") == 0)
			{
				// See if we don't already have a flare pouch
				if(IsHuman(m_cmMovement.GetCharacterButes()))
				{
					//see if we are a player and send a message
					if (IsPlayer(m_hObject))
					{
						SetCharacter(g_pCharacterButeMgr->GetSetFromModelType(pToks[1]), LTFALSE);
						((CPlayerObj*)this)->SendButesAndScaleInformation();

						//tell player to re-load his PV model skins.
						HMESSAGEWRITE hMessage = g_pServerDE->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_RELOAD_SKINS);
						g_pServerDE->EndMessage(hMessage);
					}
					return LTTRUE;
				}
			}
		}
		break;

		// ----------------------------------------------------------------------- //
		// Check all the tokens with 3 parameters
		case 3:
		{
			if(_stricmp(pToks[0], "AMMO") == 0)
			{
				uint32 nWeaponSet;
				
				if(g_pGameServerShell->GetGameType().IsMultiplayer())
				{
					if(g_pGameServerShell->GetGameInfo()->m_bClassWeapons)
						nWeaponSet = GetButes()->m_nMPClassWeaponSet;
					else
						nWeaponSet = GetButes()->m_nMPWeaponSet;
				}
				else
					nWeaponSet = GetButes()->m_nWeaponSet;

				uint8 nInSet = g_pWeaponMgr->IsAmmoInSet(nWeaponSet, pToks[1]);

				if(	(nInSet == WMGR_AVAILABLE) ||
					(_stricmp(pToks[1], "Flares")==0 && IsHuman(GetButes())))
				{
					AMMO_POOL *pPool = g_pWeaponMgr->GetAmmoPool(pToks[1]);
					if(!pPool) return LTFALSE;

					LTFLOAT fPUAmt = (LTFLOAT)atof(pToks[2])-nUsed[nMainTok];

					if(NeedsAmmo(pPool, nAmt[nMainTok]))
					{
						// cap
						if(fPUAmt < 0)
						{
							fPUAmt = 0;
						}

						// if we need more than is available then note it...
						if(nAmt[nMainTok] >= fPUAmt)
						{
							nAmt[nMainTok] = (uint32)fPUAmt; 
						}
						else
						{
							if(pResidual)
							{
								*pResidual = 1;
							}
						}

						HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_AMMO);
						g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
						g_pLTServer->WriteToMessageFloat(hMessage, fPUAmt);
						g_pLTServer->WriteToMessageByte(hMessage, bNotifyHud);
						g_pLTServer->EndMessage(hMessage);
						return LTTRUE;
					}
					else
					{
						// make sure to note if there was some we could have picked up but didn't
						if(fPUAmt > 0)
						{
							if(pResidual)
							{
								*pResidual = 1;
							}
						}
					}
				}
				else if(nInSet == WMGR_CANT_USE)
				{
					// If this is a player, make sure he knows he can't use this...
					CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

					if(pPlayer)
						g_pGameServerShell->GetMultiplayerMgr()->ClientPickupMsg(pPlayer->GetClient(), MP_PICKUP_MSG_CANT_USE_AMMO);
				}
			}
		}
		break;

		case 4:
		{
			if(_stricmp(pToks[0], "WEAPON") == 0)
			{
				uint32 nWeaponSet;

				// Copy the weapon and ammo for later use / parse.
				char szWep[64];
				char szAmmo[64];

				strcpy(szWep, pToks[1]);
				strcpy(szAmmo, pToks[2]);

				// Copy the weapon name to the buffer

				if(g_pGameServerShell->GetGameType().IsMultiplayer())
				{
					if(g_pGameServerShell->GetGameInfo()->m_bClassWeapons)
						nWeaponSet = GetButes()->m_nMPClassWeaponSet;
					else
						nWeaponSet = GetButes()->m_nMPWeaponSet;

					// now re-set the name...

					if(	(_stricmp(pToks[1], "Blowtorch")!=0)			&&
						(_stricmp(pToks[1], "MarineHackingDevice")!=0)	&&
						(_stricmp(pToks[1], "Energy_Sift")!=0)			&&
						(_stricmp(pToks[1], "Medicomp")!=0) )
					{
						strcat(szWep, "_MP");
						strcat(szAmmo, "_MP");
					}
				}
				else
					nWeaponSet = GetButes()->m_nWeaponSet;

				uint8 nInSet = g_pWeaponMgr->IsWeaponInSet(nWeaponSet, szWep);

				if(	(nInSet == WMGR_AVAILABLE) ||
					((_stricmp(pToks[1], "Blowtorch")==0) && IsHuman(m_hObject)) ||
					((_stricmp(pToks[1], "MarineHackingDevice")==0) && IsHuman(m_hObject)) ||
					((_stricmp(pToks[1], "Energy_Sift")==0) && IsPredator(m_hObject)) ||
					((_stricmp(pToks[1], "Medicomp")==0) && IsPredator(m_hObject)))
				{
					WEAPON *pWeap = g_pWeaponMgr->GetWeapon(szWep);
					if(!pWeap) return LTFALSE;

					AMMO *pAmmo = g_pWeaponMgr->GetAmmo(szAmmo);
					if(!pAmmo) return LTFALSE;

					AMMO_POOL *pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);
					if(!pPool) return LTFALSE;

					LTFLOAT fAmt = (LTFLOAT)atof(pToks[3]);
					LTFLOAT fPUAmt = fAmt-nUsed[nMainTok];

					if( NeedsWeapon(pWeap) || (fPUAmt>0 && NeedsAmmo(pPool, nAmt[nMainTok])) )
					{

						// cap
						if(fPUAmt < 0)
							fPUAmt = 0;

						// if we need more than is available then note it...
						if(nAmt[nMainTok] > fPUAmt)
						{
							nAmt[nMainTok] = (uint32)fPUAmt; 
						}

						HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_ADDWEAPON);
						g_pLTServer->WriteToMessageByte(hMessage, pWeap->nId);
						g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
						g_pLTServer->WriteToMessageFloat(hMessage, fPUAmt);
						g_pLTServer->WriteToMessageByte(hMessage, bNotifyHud);
						g_pLTServer->EndMessage(hMessage);
						return LTTRUE;
					}
				}
				else if(nInSet == WMGR_CANT_USE)
				{
					// If this is a player, make sure he knows he can't use this...
					CPlayerObj *pPlayer = dynamic_cast<CPlayerObj *>(g_pLTServer->HandleToObject(m_hObject));

					if(pPlayer)
						g_pGameServerShell->GetMultiplayerMgr()->ClientPickupMsg(pPlayer->GetClient(), MP_PICKUP_MSG_CANT_USE_WEAPON);
				}
			}
		}
		break;
	}

	return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::NeedsAmmo()
//
//	PURPOSE:	Does the character need any of this ammo
//
// --------------------------------------------------------------------------- //
LTBOOL CCharacter::NeedsAmmo(AMMO_POOL *pPool, uint32 &nAmt)
{
	if(!pPool) return LTFALSE;

	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(GetAttachments());
	if(!pAttach) return LTFALSE;

	CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());
	if(!pAttachedWeapon) return LTFALSE;

	CWeapons* pWeapons = pAttachedWeapon->GetWeapons();
	if(!pWeapons) return LTFALSE;

	uint32 nQty = pWeapons->GetAmmoPoolCount(pPool->nId);
	uint32 nMax = pPool->GetMaxAmount(m_hObject);

	if(nQty < nMax)
	{
		nAmt = nMax - nQty;
		return LTTRUE;
	}
	
	return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::NeedsWeapon()
//
//	PURPOSE:	Does the character need this weapon
//
// --------------------------------------------------------------------------- //
LTBOOL CCharacter::NeedsWeapon(WEAPON *pWeapon)
{
	if(!pWeapon) return LTFALSE;

	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(GetAttachments());

	if(!pAttach) return LTFALSE;

	CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

	if(!pAttachedWeapon) return LTFALSE;

	CWeapons* pWeapons = pAttachedWeapon->GetWeapons();
	if(!pWeapons) return LTFALSE;

	return !(pWeapons->HasWeapon(pWeapon->nId));
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	func_Container_Physics_Info()
//
// PURPOSE:		Retrieves data about the container physics
//
// ----------------------------------------------------------------------- //

static LTBOOL Container_Physics_Info(HOBJECT hObj, CMContainerInfo& pInfo)
{
	// Get the object class information
	HCLASS hVBClass = g_pLTServer->GetClass("VolumeBrush");
	HCLASS hObjClass = g_pLTServer->GetObjectClass(hObj);

	// As long as this is a volume brush or derived class, update the physics values
	if(g_pLTServer->IsKindOf(hObjClass, hVBClass))
	{
		VolumeBrush* pVolBrush = (VolumeBrush*)g_pLTServer->HandleToObject(hObj);

		if(pVolBrush)
		{
			pInfo.m_fFriction *= pVolBrush->GetFriction();
			if(pInfo.m_fViscosity < pVolBrush->GetViscosity()) pInfo.m_fViscosity = pVolBrush->GetViscosity();
			pInfo.m_vCurrent += pVolBrush->GetCurrent();

			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	Touch_Notify_Pouncing()
//
// PURPOSE:		Handle Touch Notify for pouncing state
//
// ----------------------------------------------------------------------- //

static LTRESULT Touch_Notify_Pouncing(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	ASSERT( pMovement );
	ASSERT( pInfo );
	if( !pMovement || !pInfo )
		return LT_OK;

	CharacterVars * const cvVars = pMovement->GetCharacterVars();

	// Ignore the message if we have already hit something.
	if( cvVars->m_fStampTime == -1.0f )
		return LT_OK;

	if( IsAI(pMovement->GetObject()) )
	{
		CharacterMovementState_Pouncing_TN(pMovement, pInfo, fForceMag);
	
		if( cvVars->m_fStampTime == -1.0f )
		{
			// Do damage to the object we hit.
			// If you modify this also modify Touch_Notify_Pouncing in Shared\\PlayerMovement.cpp!!!
			DamageStruct damage;
			damage.eType = DT_CRUSH;
			damage.fDamage = 100.0f;
			damage.vDir = pMovement->GetForward();
			damage.hDamager = pMovement->GetObject();

			AMMO * pPounceAmmo = g_pWeaponMgr->GetAmmo("AI_Pounce_Ammo");
			if( pPounceAmmo )
				damage.fDamage = LTFLOAT(pPounceAmmo->nInstDamage);
			
			damage.fDamage *= GetDifficultyFactor();

			CCharacter *pCharacter = dynamic_cast<CCharacter*>(pMovement->m_pInterface->HandleToObject(pMovement->GetObject()));

			if(pCharacter)
				damage.DoDamage(pCharacter, pInfo->m_hObject);
		}
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	Touch_Notify_Facehug()
//
// PURPOSE:		Handle Touch Notify for pouncing state
//
// ----------------------------------------------------------------------- //

static LTRESULT Touch_Notify_Facehug(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	ASSERT( pMovement );
	ASSERT( pInfo );
	if( !pMovement || !pInfo )
		return LT_OK;

	CCharacter *pChar = dynamic_cast<CCharacter *>(pMovement->m_pInterface->HandleToObject(pMovement->GetObject()));
	CPlayerObj *pPlayer = dynamic_cast<CPlayerObj *>(pMovement->m_pInterface->HandleToObject(pMovement->GetObject()));

	if (pChar && !pPlayer)
	{
		CharacterVars *cvVars = pMovement->GetCharacterVars();
		const LTFLOAT fOldStampTime = cvVars->m_fStampTime;

		CharacterMovementState_Facehug_TN(pMovement, pInfo, fForceMag);

		// CharacterMovementState_Facehug_TN function sets cvVars->m_fTimeStamp to -1.0f which
		// indicates a "get out of this state" condition.
		if( cvVars->m_fStampTime == -1.0f )
		{
			HOBJECT hVictim = pInfo->m_hObject;
			CCharacter *pCharVictim = dynamic_cast<CCharacter *>( g_pLTServer->HandleToObject(hVictim) );
			if( !pCharVictim )
			{
				// Ignore touch notifies against non-characters.
				cvVars->m_fStampTime = fOldStampTime;
				return LT_OK;
			}


			// Just fall if we hit any non-face huggable characters.
			if( !pCharVictim->GetButes()->m_bCanBeFacehugged )
			{
				return LT_OK;
			}


			// Do damage to the object we hit.
			// If you modify this also modify Touch_Notify_Facehug in Shared\\PlayerMovement.cpp!!!
			DamageStruct damage;
			damage.eType = DT_FACEHUG;
			damage.fDamage = 1000.0f;
			damage.vDir = pMovement->GetForward();
			damage.hDamager = pMovement->GetObject();

			damage.DoDamage(pChar, hVictim);

			// Be sure any wall-walking is turned off.
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2 & ~(FLAG2_ORIENTMOVEMENT | FLAG2_SPHEREPHYSICS), LTTRUE);

			// Make sure the movement state is set properly
			pMovement->SetMovementState(CMS_FACEHUG_ATTACH);

			// Strike back if victim is in god mode.
			CPlayerObj * pPlayerObj = dynamic_cast<CPlayerObj*>(pCharVictim);
			if( pPlayerObj && pPlayerObj->InGodMode() )
			{
				DamageStruct thunder_strike;
				thunder_strike.eType = DT_EXPLODE;
				thunder_strike.fDamage = 1000.0f;
				thunder_strike.vDir = -pMovement->GetForward();
				thunder_strike.hDamager = hVictim;

				thunder_strike.DoDamage(pPlayerObj,pMovement->GetObject());
			}
		}
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::InitialUpdate()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

void CCharacter::InitialUpdate(int nInfo)
{
	// ---------------------------------------------------------------------------
	// Required initializations (for new or saved games).
	m_cmMovement.Init(g_pLTServer, m_hObject, &m_caAnimation);
	m_cmMovement.SetContainerPhysicsInfoFunc(Container_Physics_Info);
	m_cmMovement.SetTouchNotifyFunc(CMS_POUNCING, Touch_Notify_Pouncing);
	m_cmMovement.SetTouchNotifyFunc(CMS_FACEHUG, Touch_Notify_Facehug);
	m_cmMovement.SetObjectFlags( OFT_Flags,
								 FLAG_STAIRSTEP | FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_SOLID |
								 FLAG_GRAVITY | FLAG_MODELKEYS | FLAG_RAYHIT | FLAG_VISIBLE |
								 FLAG_ANIMTRANSITION, LTTRUE );

	if(g_pGameServerShell->GetGameType().IsMultiplayer())
		m_caAnimation.Init(g_pLTServer, m_hObject, MID_ANIMATION, (1 << ANIM_TRACKER_MOVEMENT));
	else
		m_caAnimation.Init(g_pLTServer, m_hObject, MID_ANIMATION);

	// ---------------------------------------------------------------------------
	// ---------------------------------------------------------------------------

	// Init console vars
	if(!g_cvarExoRunRate.IsInitted())
		g_cvarExoRunRate.Init(g_pLTServer, "ExoRunRate", NULL, 0.0f);
	if(!g_cvarExoWalkRate.IsInitted())
		g_cvarExoWalkRate.Init(g_pLTServer, "ExoWalkRate", NULL, 0.0f);
	if(!g_cvarExoCrouchRate.IsInitted())
		g_cvarExoCrouchRate.Init(g_pLTServer, "ExoCrouchRate", NULL, 0.0f);
	if(!g_cvarExoJumpRate.IsInitted())
		g_cvarExoJumpRate.Init(g_pLTServer, "ExoJumpRate", NULL, 10.0f);
	if(!g_cvarExoClimbRate.IsInitted())
		g_cvarExoClimbRate.Init(g_pLTServer, "ExoClimbRate", NULL, 0.0f);
	if(!g_cvarExoRegenRate.IsInitted())
		g_cvarExoRegenRate.Init(g_pLTServer, "ExoRegenRate", NULL, 5.0f);
	if(!g_cvarExoDepriveRate.IsInitted())
		g_cvarExoDepriveRate.Init(g_pLTServer, "ExoDepriveRate", NULL, 0.0f);
	if(!g_cvarExoFiringRate.IsInitted())
		g_cvarExoFiringRate.Init(g_pLTServer, "ExoFiringRate", NULL, 5.0f);

	if(!g_cvarChestbursterMaxScale.IsInitted())
		g_cvarChestbursterMaxScale.Init(g_pLTServer, "ChestbursterMaxScale", NULL, 1.0f);
	if(!g_cvarChestbursterMutateTime.IsInitted())
		g_cvarChestbursterMutateTime.Init(g_pLTServer, "ChestbursterMutateTime", NULL, 12.5f);
	if(!g_cvarChestbursterMutateDelay.IsInitted())
		g_cvarChestbursterMutateDelay.Init(g_pLTServer, "ChestbursterMutateDelay", NULL, 7.5f);

	if(!g_cvarBattRechargeRate.IsInitted())
		g_cvarBattRechargeRate.Init	(g_pLTServer, "BattRechargeRate", NULL, 15.0f);
	if(!g_cvarBattFLUseRate.IsInitted())
		g_cvarBattFLUseRate.Init	(g_pLTServer, "BattFLUseRate", NULL, 2.0f);
	if(!g_cvarBattNVUseRate.IsInitted())
		g_cvarBattNVUseRate.Init	(g_pLTServer, "BattNVUseRate", NULL, 14.0f);
	if(!g_cvarBattSystem.IsInitted())
		g_cvarBattSystem.Init	(g_pLTServer, "BattSystemOn", NULL, 1.0f);
	if(!g_cvarMPUseMod.IsInitted())
		g_cvarMPUseMod.Init	(g_pLTServer, "MPBattUseMod", NULL, 0.30f);

	if(!g_cvarMinDialogueTime.IsInitted())
		g_cvarMinDialogueTime.Init(g_pLTServer, "MinDialogueTime", NULL, 2.0f);

	if(!g_cvarDemaskChance.IsInitted())
		g_cvarDemaskChance.Init(g_pLTServer, "DemaskChance", NULL, 0.25f);

	if(!g_cvarAcidHitRad.IsInitted())
		g_cvarAcidHitRad.Init(g_pLTServer, "AcidHitRad", NULL, 150.0f);
	if(!g_cvarMPAcidHitRatio.IsInitted())
		g_cvarMPAcidHitRatio.Init(g_pLTServer, "MPAcidHitRatio", NULL, 0.5f);
	if(!g_cvarPredAcidHitRatio.IsInitted())
		g_cvarPredAcidHitRatio.Init(g_pLTServer, "PredAcidHitRatio", NULL, 0.1f);
	if(!g_cvarOtherAcidHitRatio.IsInitted())
		g_cvarOtherAcidHitRatio.Init(g_pLTServer, "OtherAcidHitRatio", NULL, 0.4f);

	if(!g_cvarDamageHistoryTime.IsInitted())
		g_cvarDamageHistoryTime.Init(g_pLTServer, "CharacterDamageHistoryTime", NULL, 0.5f);

	if(!g_cvarCloakDelayTime.IsInitted())
		g_cvarCloakDelayTime.Init(g_pLTServer, "CloakDelay", NULL, 5.0f);

	if(!g_cvarCloakCostPerSecond.IsInitted())
		g_cvarCloakCostPerSecond.Init(g_pLTServer, "CloakCostPerSec", NULL, 0.1f);

	if(!g_cvarCloakInitialCost.IsInitted())
		g_cvarCloakInitialCost.Init(g_pLTServer, "CloakInitialCost", NULL, 5.0f);

	if(!g_cvarSiftMultiMod.IsInitted())
		g_cvarSiftMultiMod.Init(g_pLTServer, "SiftMod", NULL, 4.0f);

	if(!g_cvarMediMultiMod.IsInitted())
		g_cvarMediMultiMod.Init(g_pLTServer, "MediMod", NULL, 4.0f);

	if(!g_cvarNetDamage.IsInitted())
		g_cvarNetDamage.Init(g_pLTServer, "NetDamage", NULL, 100.0f);
	if(!g_cvarNetTime.IsInitted())
		g_cvarNetTime.Init(g_pLTServer, "NetTime", NULL, 12.0f);

	if(!g_cvarBlinkMinDelay.IsInitted())
		g_cvarBlinkMinDelay.Init(g_pLTServer, "BlinkMinDelay", NULL, 2.0f);
	if(!g_cvarBlinkMaxDelay.IsInitted())
		g_cvarBlinkMaxDelay.Init(g_pLTServer, "BlinkMaxDelay", NULL, 10.0f);
	if(!g_cvarBlinkOnDelay.IsInitted())
		g_cvarBlinkOnDelay.Init(g_pLTServer, "BlinkOnDelay", NULL, 0.1f);


#ifdef PROF_CHARACTER_UPDATE
	if(!g_cvarMaxTickDelay.IsInitted())
		g_cvarMaxTickDelay.Init(g_pLTServer, "MaxTickDelay", NULL, 5.0f);
#endif	
	
	// ---------------------------------------------------------------------------
	// ---------------------------------------------------------------------------
	
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	ResetCharacter();

	// Create the box used for weapon impact detection...

	CreateHitBox();

	// Set this as a moveable character object...
	uint32 nFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, nFlags | USRFLG_MOVEABLE | USRFLG_CHARACTER);

	// Create the special fx message...
	CreateSpecialFX();

	// Load animations if we are netted
	if(m_hNet)
		LoadNetAnimation();


}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateSpecialFX()
{
	// Create the special fx...
	CHARCREATESTRUCT cs;

	cs.nCharacterSet = GetCharacter();
	cs.nNumTrackers = g_pGameServerShell->GetGameType().IsMultiplayer() ? 1 : ANIM_TRACKER_MAX;
	cs.nClientID = 0;

	if(IsPlayer(m_hObject))
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

		if(pPlayer)
		{
			if(g_pGameServerShell->GetGameType().IsSinglePlayer())
				cs.nClientID = 1;
			else
				cs.nClientID = m_nPreCreateClientID;
		}
	}


	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	cs.Write(g_pLTServer, hMessage);
	g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateVolumeInfo()
//
//	PURPOSE:	Update the current and last volume info (be sure this is after m_cmMovement.Update()).
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateVolumeInfo(LTBOOL bResetVolumeInfo /* = LTFALSE */ )
{
	// m_pCurrentVolume will contain the volume we are in.  If we aren't in a volume, it is null.
	// m_pLastVolume willl hold the previous volume, if we had one.  Otherwise it should
	//    be set to our current volume.  It should only be null if we are not in a current volume
	//    and we were told to reset last volume info.
	// m_vLastVolumePos will contain our current position if we are in a volume, or our position when we were last in a volume.


	// Do a reset, if requested.
	if( bResetVolumeInfo )
	{
		m_pCurrentVolume = LTNULL;
		m_pLastVolume = LTNULL;
	}

	CAIVolume * const pOldCurrentVolume = m_pCurrentVolume;
	const LTVector & vPos = GetPosition();

	// Have we even changed volumes?
	if( m_pCurrentVolume && m_pCurrentVolume->Contains(vPos) )
	{
		// When we are not in a volume, this
		// will be our last volume position!
		m_vLastVolumePos = vPos;
		return;
	}

	if( pOldCurrentVolume )
	{
		// See if we just entered a neighbor.
		m_pCurrentVolume = LTNULL;
		for(CAIVolume::NeighborList::iterator iNeighbor = pOldCurrentVolume->BeginNeighbors();
			iNeighbor != pOldCurrentVolume->EndNeighbors() && !m_pCurrentVolume; ++iNeighbor )
		{
			if( (*iNeighbor)->Contains(vPos) )
			{
				// Aha! we have entered a neighbor!
				m_pCurrentVolume = *iNeighbor;

/*				AICPrint(m_hObject, "Entering neighbor volume \"%s\" at (%.2f,%.2f,%.2f).",
					m_pCurrentVolume->GetName(), 
					m_pCurrentVolume->GetCenter().x,
					m_pCurrentVolume->GetCenter().y,
					m_pCurrentVolume->GetCenter().z );
*/			}
		}
	}

	// If we still don't have a current volume, search all the volumes!
	if( !m_pCurrentVolume && g_pAIVolumeMgr->IsInitialized() )
	{
		// See if we have entered a volume.
		CAIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(vPos);

		if( pVolume )
		{
			m_pCurrentVolume = pVolume;

/*			AICPrint(m_hObject, "Suddenly in volume \"%s\" at (%.2f,%.2f,%.2f).",
				m_pCurrentVolume->GetName(), 
				m_pCurrentVolume->GetCenter().x,
				m_pCurrentVolume->GetCenter().y,
				m_pCurrentVolume->GetCenter().z );
*/		}
	}
	
	// Now we can set our last volume info.

	// If we had a current volume before and it has changed,
	// use that as our last volume!
	if( pOldCurrentVolume && pOldCurrentVolume != m_pCurrentVolume )
	{
		if( m_pLastVolume != pOldCurrentVolume )
		{
			m_pLastVolume = pOldCurrentVolume;
		}
	}

	// Take care of the business of having a current volume.
	if( m_pCurrentVolume )
	{
		// Record our current position, 
		// when we are no longer in a volume it will be our last position.
		m_vLastVolumePos = vPos;

		// If we don't have a last volume but we do have a current
		// volume,  use the current volume as our last volume.
		if( !m_pLastVolume  )
		{
			m_pLastVolume = m_pCurrentVolume;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateLastSimpleNode()
//
//	PURPOSE:	Check to see if we have a new "last" simple node.
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateLastSimpleNode()
{
	const LTVector & vMyPos = GetPosition();

	for( CSimpleNodeMgr::SimpleNodeList::iterator iter = g_pSimpleNodeMgr->BeginNodes();
	     iter != g_pSimpleNodeMgr->EndNodes(); ++iter )
	{
		CSimpleNode * const pSimpleNode = (*iter);

		const LTVector & vNodePos = pSimpleNode->GetPos();
		const LTFLOAT x_diff = vMyPos.x - vNodePos.x;
		const LTFLOAT z_diff = vMyPos.z - vNodePos.z;

		// Are we inside it's x-z radius?
		if(    x_diff < pSimpleNode->GetRadius()
			&& z_diff < pSimpleNode->GetRadius()
			&& x_diff*x_diff + z_diff*z_diff < pSimpleNode->GetRadiusSqr() )
		{
			const LTFLOAT y_diff = vMyPos.y - vNodePos.y;

			// Are we inside its y dims?
			if( (float)fabs(y_diff) < pSimpleNode->GetHalfHeight() )
			{
				// If we don't have a last node, or our last node is further away than this node.
				if( !m_pLastSimpleNode || ( vNodePos - vMyPos ).MagSqr() < (m_pLastSimpleNode->GetPos() - vMyPos).MagSqr() )
				{
					// Then this is our new node!
					m_pLastSimpleNode = pSimpleNode;
				}
			}
		}
	}
}

LTFLOAT CCharacter::GetSimpleNodeCheckDelay() 
{ 
	return GetRandom(0.2f, 1.2f); 
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CacheFiles()
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void CCharacter::CacheFiles()
{
	// The default model and skins will be cached internally by the engine,
	// so we just need to cache the vision mode skins here

	// Grab the base filenames
	ObjectCreateStruct pStruct;
	pStruct.Clear();
	g_pCharacterButeMgr->GetDefaultFilenames(GetCharacter(), pStruct);

	// A temp buffer for the caching
	char szBuffer[256];
	char szVisionModes[3][64];

#ifdef _WIN32
	strcpy(szVisionModes[0], "ElectroVision\\");
	strcpy(szVisionModes[1], "HeatVision\\");
	strcpy(szVisionModes[2], "NavVision\\");
#else
	strcpy(szVisionModes[0], "ElectroVision/");
	strcpy(szVisionModes[1], "HeatVision/");
	strcpy(szVisionModes[2], "NavVision/");
#endif

	// Go through each texture and append the vision mode directories
	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		if(pStruct.m_SkinNames[i][0])
		{
			CString tempDir = g_pCharacterButeMgr->GetDefaultSkinDir(m_nCharacterSet);
			CString skinDir;
			CString skinName;
			int nLength;

			for(int j = 0; j < 3; j++)
			{
				skinDir = tempDir;
				skinDir += szVisionModes[j];
				skinName = g_pCharacterButeMgr->GetDefaultSkin(m_nCharacterSet, i, skinDir.GetBuffer());

				sprintf(szBuffer, "%s", skinName.GetBuffer());
				nLength = strlen(szBuffer);
				strcpy(szBuffer + (nLength - 3), "spr");

				if(g_pLTServer->CacheFile(FT_SPRITE, szBuffer) != LT_OK)
				{
					strcpy(szBuffer + (nLength - 3), "dtx");

					if(g_pLTServer->CacheFile(FT_TEXTURE, szBuffer) != LT_OK)
					{
#ifndef _FINAL
						g_pLTServer->CPrint("CCharacter::CacheFiles!!  Error caching '%s'", szBuffer);
#endif
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandHeldWeaponFirePos()
//
//	PURPOSE:	Get the fire (flash) position of the hand-held weapon
//
// ----------------------------------------------------------------------- //
	
LTVector	CCharacter::HandHeldWeaponFirePos(const CWeapon *pWeapon) const
{
	LTRotation rRot;
	LTVector vPos;

	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (!g_pLTServer || !pWeapon || !g_pWeaponMgr) return vPos;

	char szModelFilename[256];
	g_pLTServer->GetModelFilenames( pWeapon->GetModelObject(), szModelFilename, 256, LTNULL, 0);

	if( strstr(szModelFilename,"Dummy") == LTNULL )
	{
		HATTACHMENT hAttachment;
		if (g_pLTServer->FindAttachment(m_hObject, pWeapon->GetModelObject(), &hAttachment) != LT_OK)
		{
			return vPos;
		}

		HMODELSOCKET hSocket;

		const char * szFlashSocketName = pWeapon->GetFlashSocketName();
		if( szFlashSocketName && szFlashSocketName[0] )
		{
			if (g_pModelLT->GetSocket(pWeapon->GetModelObject(), const_cast<char*>(szFlashSocketName), hSocket) == LT_OK)
			{
				LTransform transform;
				CommonLT* pCommonLT = g_pLTServer->Common();
				pCommonLT->GetAttachedModelSocketTransform(hAttachment, hSocket, transform);

				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				g_pTransLT->GetPos(transform, vPos);
			}
#ifndef _FINAL
			else
			{
				g_pLTServer->CPrint("%s could not find flash socket %s on weapon model for %s!",
					g_pLTServer->GetObjectName(m_hObject), pWeapon->GetFlashSocketName(),
					pWeapon->GetWeaponData()->szName );
			}
#endif
		}
	}
	else
	{
		const char * szFlashSocketName = pWeapon->GetFlashSocketName();
		if ( szFlashSocketName && szFlashSocketName[0] )
		{
			HMODELSOCKET hSocket;
			if( g_pModelLT->GetSocket(m_hObject, const_cast<char*>(szFlashSocketName), hSocket) == LT_OK)
			{
				LTransform transform;
				ModelLT* pModelLT = g_pLTServer->GetModelLT();
				pModelLT->GetSocketTransform(m_hObject, hSocket, transform, LTTRUE);

				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				g_pTransLT->GetPos(transform, vPos);
			}
#ifndef _FINAL
			else
			{
				g_pLTServer->CPrint("%s could not find flash socket %s on model!",
					g_pLTServer->GetObjectName(m_hObject), pWeapon->GetFlashSocketName() );
			}
#endif
		}
	}

	return vPos;
}

CWeapon* CCharacter::GetCurrentWeaponPtr()
{
	CWeapon *pWeapon = LTNULL;
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(GetAttachments());

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				pWeapon = pWeapons->GetCurWeapon();
			}
		}
	}

	return pWeapon;
}

static void SetLastVolumeInfo( const LTVector & vPos, const LTVector & vDims,
							   const LTVector & vUp, 
							   CAIVolume ** ppCurrentVolume, CAIVolume ** ppLastVolume, 
							   LTVector * pvLastVolumePos )
{
	if ( g_pAIVolumeMgr->IsInitialized() && ppCurrentVolume)
	{
		if( *ppCurrentVolume )
		{
			if( !(*ppCurrentVolume)->Contains(vPos) )
			{
				// See if we just entered a neighbor.
				for(CAIVolume::NeighborList::iterator iNeighbor = (*ppCurrentVolume)->BeginNeighbors();
				    iNeighbor != (*ppCurrentVolume)->EndNeighbors(); ++iNeighbor )
				{
					if( (*iNeighbor)->Contains(vPos) )
					{
						if(     ppLastVolume && *ppCurrentVolume 
							&& *ppLastVolume != *iNeighbor )
						{
							*ppLastVolume = *ppCurrentVolume;
						
							if( pvLastVolumePos )
								*pvLastVolumePos = vPos;
						}

						*ppCurrentVolume = *iNeighbor;
						return;
					}
				}

				if(     ppLastVolume && *ppCurrentVolume )
				{
					*ppLastVolume = *ppCurrentVolume;
				
					if( pvLastVolumePos )
						*pvLastVolumePos = vPos;
				}

				*ppCurrentVolume = LTNULL;
			}
		}

		if( !( *ppCurrentVolume ) )
		{
			// See if we have entered a volume.
			CAIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(vPos);

			if( pVolume )
			{
				*ppCurrentVolume = pVolume;
				
				if( ppLastVolume && !*ppLastVolume )
				{
					// If we don't have a previous volume make our current and
					// previous volume the same.
					*ppLastVolume = *ppCurrentVolume;
					if( pvLastVolumePos )
						*pvLastVolumePos = vPos;
				}
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Update()
//
//	PURPOSE:	Update the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Update()
{
#ifdef PROF_CHARACTER_UPDATE

	if( g_tmrTickCheck.Stopped() )
	{
		g_pLTServer->CPrint("max a = %d.", g_max_updatea_ticks );
		g_max_updatea_ticks = 0;

		g_pLTServer->CPrint("max b = %d.", g_max_updateb_ticks );
		g_max_updateb_ticks = 0;

		g_pLTServer->CPrint("max c = %d.", g_max_updatec_ticks );
		g_max_updatec_ticks = 0;

		g_pLTServer->CPrint("max d = %d.", g_max_updated_ticks );
		g_max_updated_ticks = 0;

		g_pLTServer->CPrint("max e = %d.", g_max_updatee_ticks );
		g_max_updatee_ticks = 0;

		g_pLTServer->CPrint("total max = %d.", g_max_update_ticks );
		g_max_update_ticks = 0;

		g_pLTServer->CPrint("---------------------------");

		g_tmrTickCheck.Start(g_cvarMaxTickDelay.GetFloat());
	}

	g_pLTServer->StartCounter(&character_counter);

#endif

	// Update our sounds

	UpdateSounds();

	// Make sure our hit box is in the correct position...

	UpdateHitBox();

	// Update the air level

	UpdateAirLevel();

	// Update battery level

	UpdateBatteryLevel();

	// Update the exosuit energy level

	if(fpUpdateExoEnergy)
		(this->*(fpUpdateExoEnergy))();

	// Update the dialogue delay time

	if(g_pLTServer->GetTime() >= (m_fDialogueStartTime + g_cvarMinDialogueTime.GetFloat()))
		m_fDialogueStartTime = 0.0f;

	// Update alien life cycle stuff

	UpdateChestbursterCycle();

#ifdef PROF_CHARACTER_UPDATE
	updatea_ticks = g_pLTServer->EndCounter(&character_counter);
#endif

	// Perform the movement... (Player stuff is handled client side)
	if(!IsPlayer(m_hObject))
	{
		if(m_bEMPEffect)
			UpdateEMPEffect();

		if(m_bStunEffect || IsNetted())
			UpdateStunEffect();

		m_cmMovement.Update(GetUpdateDelta());
	}

	// This must be done after the movement update!
	UpdateVolumeInfo();

	if( m_tmrNextSimpleNodeCheck.Stopped() )
	{
		UpdateLastSimpleNode();

		m_tmrNextSimpleNodeCheck.Start( GetSimpleNodeCheckDelay() );
	}

#ifdef PROF_CHARACTER_UPDATE
	updateb_ticks = g_pLTServer->EndCounter(&character_counter);
#endif

#ifdef LAST_VOLUME_DEBUG
	LineSystem::GetSystem(this,"ShowVolume")
		<< LineSystem::Clear();

	if( m_pCurrentVolume )
	{
		LineSystem::GetSystem(this,"ShowVolume")
			<< LineSystem::Arrow( GetPosition(), m_pCurrentVolume->GetCenter(), Color::Green );
	}

	if( m_pLastVolume )
	{
		LineSystem::GetSystem(this,"ShowVolume")
			<< LineSystem::Arrow( GetPosition(), m_pLastVolume->GetCenter(), Color::Red );
	}
#endif



	// Update the cloaking.
	UpdateCloakEffect();

#ifdef PROF_CHARACTER_UPDATE
	updatec_ticks = g_pLTServer->EndCounter(&character_counter);
#endif

	// Update the character animation
	UpdateAnimation();
	m_caAnimation.Update();

#ifdef PROF_CHARACTER_UPDATE
	updated_ticks = g_pLTServer->EndCounter(&character_counter);
#endif

	UpdateCharacterFX();


	// Update our net...
	if(m_hNet)
		UpdateNet();

#ifndef _FINAL
	if( ShouldShowLighting(m_hObject) )
	{
		AICPrint( m_hObject,"Model lighting (%f,%f,%f).",
			m_vModelLighting.x,m_vModelLighting.y,m_vModelLighting.z );
	}
#endif

	// See if we be dead...
	if (m_damage.IsDead())
	{
		HandleDead(LTTRUE);
		return;
	}


	// Reset the character reset variable
	m_bCharacterReset = LTFALSE;

#ifdef PROF_CHARACTER_UPDATE
	updatee_ticks = g_pLTServer->EndCounter(&character_counter);

	const uint32 update_ticks = g_pLTServer->EndCounter(&character_counter);
	if( update_ticks > g_max_update_ticks )
	{
		g_max_update_ticks = update_ticks;

		g_max_updatea_ticks = updatea_ticks;
		g_max_updateb_ticks = updateb_ticks - updatea_ticks;
		g_max_updatec_ticks = updatec_ticks - updateb_ticks;
		g_max_updated_ticks = updated_ticks - updatec_ticks;
		g_max_updatee_ticks = updatee_ticks - updated_ticks;
	}

#endif

	// see if we need to update our character FX...
	if(m_bConfirmFX)
	{
		// see that we don't come back here...
		m_bConfirmFX = LTFALSE;

		// Send an SFX message to make sure the special FX get set to the proper character set
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_RESET_ATTRIBUTES);
		g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_nCharacterSet);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST | MESSAGE_GUARANTEED);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateCharacterFX()
//
//	PURPOSE:	Update the character FX on the clients
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateCharacterFX()
{
	// MULTIPLAYER SPECIFIC UPDATES
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		// Update the strafe node control state
		uint8 nStrafeState;
		CharacterMovementState cms = m_cmMovement.GetMovementState();

		switch(cms)
		{
			case CMS_IDLE:
			case CMS_CLIMBING:
			case CMS_SWIMMING:
			case CMS_POUNCING:
			case CMS_POUNCE_JUMP:
			case CMS_FACEHUG:
			case CMS_FACEHUG_ATTACH:
				nStrafeState = 0xFF;
				break;

			case CMS_CRAWLING:
			case CMS_WALLWALKING:
				nStrafeState = 1;
				break;

			default:
				nStrafeState = 0;
				break;
		}

		if(m_nLastStrafeState != nStrafeState)
		{
			m_nLastStrafeState = nStrafeState;

			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_STRAFE_YAW);
			g_pLTServer->WriteToMessageByte(hMessage, m_nLastStrafeState);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}


		// Update the aiming pitch on the client FX to handle the node control
		if(m_fLastAimingPitch != m_cmMovement.GetCharacterVars()->m_fAimingPitch || m_nLastAimingPitchSet != m_nAimingPitchSet)
		{
			m_fLastAimingPitch = m_cmMovement.GetCharacterVars()->m_fAimingPitch;
			m_nLastAimingPitchSet = m_nAimingPitchSet;

			int8 nSendValue = (int8)(m_fLastAimingPitch / 90.0f * 100.0f);

			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_AIMING_PITCH);
			g_pLTServer->WriteToMessageByte(hMessage, nSendValue);
			g_pLTServer->WriteToMessageByte(hMessage, m_nAimingPitchSet);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}
	}


	// SINGLE PLAYER SPECIFIC UPDATES
	if(g_pGameServerShell->GetGameType().IsSinglePlayer())
	{
		// Figure out the global aiming angle from 0 to MATH_CIRCLE
		LTVector vF = GetForward();
		vF.y = 0.0f;
		vF.Norm();

		LTFLOAT fYawAngle = (LTFLOAT)acos(vF.z);
		if(vF.x < 0.0f) fYawAngle = MATH_CIRCLE - fYawAngle;

		fYawAngle = MATH_RADIANS_TO_DEGREES(fYawAngle) + m_cmMovement.GetCharacterVars()->m_fAimingYaw;

		while(fYawAngle < 0.0f) fYawAngle += 360.0f;
		while(fYawAngle >= 360.0f) fYawAngle -= 360.0f;


		// Update the aiming yaw on the client FX to handle the node control
		if(m_fLastAimingYaw != fYawAngle)
		{
			m_fLastAimingYaw = fYawAngle;

			// Send down the info
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_AIMING_YAW);
			g_pLTServer->WriteToMessageFloat(hMessage, fYawAngle);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}
	}


	// Update the flashlight FX

	// Removing to allow flashlight to track the shoulder socket.
	if(0)// IsFlashlightOn() )
	{
		LTRotation rAim = GetFlashlightRot();
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_FLASHLIGHT);
		g_pLTServer->WriteToMessageRotation(hMessage, &rAim);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateWeaponAnimation()
//
//	PURPOSE:	Update the weapon animation information
//
// ----------------------------------------------------------------------- //

WEAPON* CCharacter::UpdateWeaponAnimation()
{
	// ----------------------------------------------------------------------- //
	// Set the weapon animation layer

	// Go through all this shit in order to get the weapon name
	WEAPON *pWeap = LTNULL;
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(GetAttachments());

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				CWeapon *pWeapon = pWeapons->GetCurWeapon();

				if(pWeapon)
				{
					pWeap = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
				}
			}
		}
	}

	if(pWeap)
	{
		// Only switch this layer for aliens if they're attacking...
		if(IsAlien(m_cmMovement.GetCharacterButes()))
		{
			if((m_cmMovement.GetControlFlags() & CM_FLAG_PRIMEFIRING) || (m_cmMovement.GetControlFlags() & CM_FLAG_ALTFIRING))
				m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, pWeap->szName);	
			else
				m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");

			m_nAimingPitchSet = 0;
		}
		else
		{
			m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, pWeap->szName);	
			m_nAimingPitchSet = pWeap->nAimingPitchSet;
		}
	}
	else
	{
		m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
		m_nAimingPitchSet = 0;
	}

	return pWeap;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateActionAnimation()
//
//	PURPOSE:	Update the action animation information
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateActionAnimation()
{
	m_caAnimation.SetLayerReference(ANIMSTYLELAYER_ACTION, "Idle");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateAnimation()
//
//	PURPOSE:	Update the animation information
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateAnimation()
{
	// ----------------------------------------------------------------------- //
	// Handle the layer reference setups

	UpdateWeaponAnimation();
	UpdateActionAnimation();

	// ----------------------------------------------------------------------- //
	// Set the proper base speed

	m_caAnimation.SetBaseSpeed(m_cmMovement.GetMaxSpeed());


	// ----------------------------------------------------------------------- //
	// Calculate an aiming pitch value

	LTFLOAT fAiming = -m_cmMovement.GetCharacterVars()->m_fAimingPitch / 75.0f;
	if(fAiming < -1.0f) fAiming = -1.0f;
	if(fAiming > 1.0f) fAiming = 1.0f;

	// Set the appropriate trackers with this value
	TrackerInfo *pInfo = LTNULL;

	pInfo = m_caAnimation.GetTrackerInfo(ANIM_TRACKER_AIMING_UP);
	pInfo->m_fAimingRange = fAiming;

	pInfo = m_caAnimation.GetTrackerInfo(ANIM_TRACKER_AIMING_DOWN);
	pInfo->m_fAimingRange = fAiming;


	// ----------------------------------------------------------------------- //
	// Update our painful expression.
	pInfo = m_caAnimation.GetTrackerInfo(ANIM_TRACKER_EXPRESSION);

	if(!IsRecoiling())
	{
		pInfo->m_nIndex = m_nDesiredExpression;

		// Update our blinking...
		if(pInfo->m_nIndex == EXPRESSION_NORMAL)
		{
			LTFLOAT fTime = g_pLTServer->GetTime();

			if(fTime >= m_fLastBlinkTime + m_fBlinkDelay)
			{
				pInfo->m_nIndex = EXPRESSION_BLINK;
				m_nDesiredExpression = EXPRESSION_BLINK;
				m_fLastBlinkTime = fTime;
			}
		}
		else if(pInfo->m_nIndex == EXPRESSION_BLINK)
		{
			LTFLOAT fTime = g_pLTServer->GetTime();

			if(fTime >= m_fLastBlinkTime + g_cvarBlinkOnDelay.GetFloat())
			{
				pInfo->m_nIndex = EXPRESSION_NORMAL;
				m_nDesiredExpression = EXPRESSION_NORMAL;
				m_fLastBlinkTime = fTime;
				m_fBlinkDelay = GetRandom(g_cvarBlinkMinDelay.GetFloat(), g_cvarBlinkMaxDelay.GetFloat());
			}
		}
	}
	else
	{
		if(g_pGameServerShell->LowViolence())
			pInfo->m_nIndex = EXPRESSION_NORMAL;
		else
			pInfo->m_nIndex = EXPRESSION_PAIN;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetAnimExpression()
//
//	PURPOSE:	Sets the expression animation for this character
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::SetAnimExpression(int nExpression)
{
	// Make sure the value is within proper range
	if((nExpression < EXPRESSION_MIN) || (nExpression > EXPRESSION_MAX))
		return LTFALSE;

	// Set the appropriate tracker with this value
	if(!IsRecoiling())
	{
		TrackerInfo *pInfo = m_caAnimation.GetTrackerInfo(ANIM_TRACKER_EXPRESSION);
		pInfo->m_nIndex = nExpression;
	}

	m_nDesiredExpression = nExpression;
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleModelString(ArgList* pArgList)
{
	char* pKey = pArgList->argv[0];
	if (!pKey) return;


	if (stricmp(pKey, KEY_FOOTSTEP_SOUND) == 0 )
	{
		// Only listen to the model's footstep keys if we
		// are not the player or are in a multiplayer game.
		// (in SinglePlayer, the footsteps are sent up from the client).
		if(    !g_pGameServerShell->GetGameType().IsSinglePlayer() 
			|| !dynamic_cast<CPlayerObj*>(this) )
		{
			HandleFootstepKey();
		}
	}
	else if (stricmp(pKey, KEY_PLAYSOUND) == 0 && pArgList->argc > 1)
	{
		// Get sound name from message...

		const char* szSoundName = pArgList->argv[1];

		if (szSoundName && szSoundName[0])
		{
			// Try to get a character specific sound
			const CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(m_nCharacterSet), szSoundName );
			int nButes = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);

			// If that fails (nButes == -1), then just try to find a direct sound bute.
			if( nButes < 0 )
				nButes = g_pSoundButeMgr->GetSoundSetFromName(szSoundName);

			// And play it!
			if( nButes >= 0 )
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, nButes);
		}
	}
	else if (stricmp(pKey, KEY_PLAYVOICE) == 0 && pArgList->argc > 1)
	{
		// Get sound name from message...

		const char* szSoundName = pArgList->argv[1];

		if (szSoundName && szSoundName[0])
		{
			PlayExclamationSound(szSoundName);
		}
	}
	else if (stricmp(pKey, KEY_SET_DIMS) == 0)
	{
		if (pArgList->argc < 2) return;

		// Set up so we can set one or more dims...

		LTVector vDims;
		g_pLTServer->GetObjectDims(m_hObject, &vDims);

		if (pArgList->argv[1])
		{
			vDims.x = (LTFLOAT) atof(pArgList->argv[1]);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			vDims.y = (LTFLOAT) atof(pArgList->argv[2]);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			vDims.z = (LTFLOAT) atof(pArgList->argv[3]);
		}

		// Set the new dims

//		m_cmMovement.SetObjectDims();
	}
	else if (stricmp(pKey, KEY_MOVE) == 0)
	{
		if (pArgList->argc < 2) return;

		// Set up so we move in one or more directions

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		
		LTRotation rRot;
		LTVector vU, vR, vF;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);
		g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

		LTFLOAT fOffset;

		if (pArgList->argv[1])
		{
			// Forward...

			fOffset = (LTFLOAT) atof(pArgList->argv[1]);

			VEC_MULSCALAR(vF, vF, fOffset);
			VEC_ADD(vPos, vPos, vF);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			// Up...

			fOffset = (LTFLOAT) atof(pArgList->argv[2]);

			VEC_MULSCALAR(vU, vU, fOffset);
			VEC_ADD(vPos, vPos, vU);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			// Right...

			fOffset = (LTFLOAT) atof(pArgList->argv[3]);

			VEC_MULSCALAR(vR, vR, fOffset);
			VEC_ADD(vPos, vPos, vR);
		}

		// Set the new position

		g_pLTServer->MoveObject(m_hObject, &vPos);
	}
	else if (stricmp(pKey, KEY_COMMAND) == 0)
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
	else if (stricmp(pKey, KEY_CHESTBURSTFX) == 0)
	{
		// Send the FX message down to the client to create the chest burst blood
		// and model FX

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_CHESTBURST_FX);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleFootstepKey
//
//	PURPOSE:	Records the footstep sound info so that AI's can "hear" it.
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleFootstepKey()
{
	// Get the surface we're standing on.
	SurfaceType eStandingOnSurface = ST_UNKNOWN;
	if (  m_cmMovement.GetCharacterVars()->m_bStandingOn )
	{
		eStandingOnSurface = GetSurfaceType(m_cmMovement.GetCharacterVars()->m_ciStandingOn);
	}

	
	m_LastFootstepSoundInfo.fTime = g_pLTServer->GetTime();
	m_LastFootstepSoundInfo.eSurfaceType = eStandingOnSurface;
	m_LastFootstepSoundInfo.vLocation = m_cmMovement.GetPos();
	m_LastFootstepSoundInfo.fVolume = 1.0f*FootStepStateVolumeMod(m_cmMovement.GetMovementState());

	// Let character butes soften the sound.
	m_LastFootstepSoundInfo.fVolume *= m_cmMovement.GetCharacterButes()->m_fBaseFootStepVolume;

	// Determine our surface and do appropriate stuff with it.
	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eStandingOnSurface);
	_ASSERT(pSurf);
	if (pSurf)
	{
		// Modify the footstep volume.
		m_LastFootstepSoundInfo.fVolume *= pSurf->fMovementNoiseModifier;
	}

	// Notify the AI's about the footstep sound.
	if( g_pGameServerShell->GetGameType().IsSinglePlayer() )
	{
		for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs();
			 iter != g_pCharacterMgr->EndAIs(); ++iter )
		{
			(*iter)->GetSenseMgr().Footstep(m_LastFootstepSoundInfo,this);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateAirLevel
//
//	PURPOSE:	Update the m_fAirLevel data member
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateAirLevel()
{
	// Record the air level...
	m_fOldAirSupplyTime = m_fAirSupplyTime;
	
	// Get the total air supply time for this character
	LTFLOAT fTotalAirTime = GetButes()->m_fAirSupplyTime;

	// See if the upper section of our character is contained in water
	LTBOOL bUpperInLiquid = LTFALSE;
	LTBOOL bMidInLiquid = LTFALSE;
	HOBJECT hContainer = LTNULL;
	uint16 nCode, i;

	for(i = 0; i < GetVars()->m_pContainers[CM_CONTAINER_UPPER].m_nNumContainers; i++)
	{
		hContainer = GetVars()->m_pContainers[CM_CONTAINER_UPPER].m_hContainers[i];
		g_pLTServer->GetContainerCode(hContainer, &nCode);

		if(IsLiquid((ContainerCode)nCode))
		{
			bUpperInLiquid = LTTRUE;
			break;
		}
	}

	// Setup some temporary container storage
	HOBJECT hContainers[CM_MAX_CONTAINERS];
	uint32 nNumContainers = 0;

	// Retrieve the position containers
	LTVector vPos = m_cmMovement.GetPos();
	nNumContainers = g_pLTServer->GetPointContainers(&vPos, hContainers, CM_MAX_CONTAINERS);

	for(i = 0; i < nNumContainers; i++)
	{
		hContainer = hContainers[i];
		g_pLTServer->GetContainerCode(hContainer, &nCode);

		if(IsLiquid((ContainerCode)nCode))
		{
			bMidInLiquid = LTTRUE;
			break;
		}
	}

	// We want to see the user flags too
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	// If we're at least midway in a liquid... clear any burn damage types
	if(bMidInLiquid)
	{
		// Make sure we are not on fire since fire is put out by liquid...
		m_damage.ClearProgressiveDamages(DT_BURN);
		m_damage.ClearProgressiveDamages(DT_NAPALM);
	}

	// If the character's head is in liquid... update the air supply
	if(bUpperInLiquid)
	{
		dwUserFlags |= USRFLG_CHAR_UNDERWATER;

		if(fTotalAirTime != -1.0f)
		{
			// Check to see if we're expending effort to move somewhere
			uint32 nCtrlFlags = m_cmMovement.GetControlFlags();

			// Start taking away our air supply time
			if(nCtrlFlags & (CM_FLAG_DIRECTIONAL | CM_FLAG_DUCK | CM_FLAG_JUMP))
				m_fAirSupplyTime -= GetUpdateDelta() * ((nCtrlFlags & CM_FLAG_RUN) ? AIR_SUPPLY_DEGRADE_FAST : AIR_SUPPLY_DEGRADE_SLOW);
			else
				m_fAirSupplyTime -= GetUpdateDelta();


			if(m_fAirSupplyTime < 0)
			{
				m_fAirSupplyTime = 0.0f;

				// Send damage message...
				DFLOAT fDamage = AIR_DEPRIVED_DAMAGE * GetUpdateDelta();

				DamageStruct damage;

				damage.eType	= DT_CHOKE;
				damage.fDamage	= fDamage;
				damage.hDamager = m_hObject;

				damage.DoDamage(this, m_hObject);
			}
		}
	}
	else
	{
		dwUserFlags &= ~USRFLG_CHAR_UNDERWATER;

		if(fTotalAirTime != -1.0f)
		{
			// Regenerate time
			m_fAirSupplyTime += (GetUpdateDelta() * AIR_SUPPLY_REGEN_SCALE);

			if(m_fAirSupplyTime > fTotalAirTime)
				m_fAirSupplyTime = fTotalAirTime + 0.01f;
		}
	}

	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateOnGround
//
//	PURPOSE:	Update m_bOnGround data member
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateOnGround()
{
	// See if we're standing on any breakable objects...

	CollisionInfo Info;
	g_pLTServer->Physics()->GetStandingOn(m_hObject, &Info);

	if(Info.m_hObject && (Info.m_hPoly != INVALID_HPOLY))
	{
		if(IsKindOf(Info.m_hObject, "Breakable"))
		{
			SendTriggerMsgToObject(this, Info.m_hObject, "TOUCHNOTIFY");
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateSounds()
//
//	PURPOSE:	Update the currently playing sounds
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateSounds()
{
	// See if it is time to shut up...

	if (m_hCurVcdSnd)
	{
		LTBOOL bIsDone = LTFALSE;
		if (g_pLTServer->IsSoundDone(m_hCurVcdSnd, &bIsDone) != LT_OK || bIsDone)
		{
			KillVoicedSound();
		}
	}

	// See if we are coming out of a liquid...

/*	if (!m_bBodyInLiquid && m_bBodyWasInLiquid)
	{
		PlaySound("Chars\\Snd\\splash1.wav", m_fSoundRadius, LTFALSE);
	}
	else if (!m_bBodyWasInLiquid && m_bBodyInLiquid)  // or going into
	{
		PlaySound("Chars\\Snd\\splash2.wav", m_fSoundRadius, LTFALSE);
	}
*/
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDamageSound
//
//	PURPOSE:	Play a damage sound, returns -1 if sound not played, returns bute index if sound was played.
//
// ----------------------------------------------------------------------- //

int CCharacter::PlayDamageSound(DamageType eType)
{
	// Don't play a damage sound if something else is more important or if another
	// damage sound is playing.
	if (m_nCurVcdSndImp >= CST_DAMAGE_IMPORTANCE) 
		return -1;

	// Dont play if we are invulnerable...
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		CPlayerObj *pPObj = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

		if(pPObj)
			if(pPObj->InGodMode())
				return -1;
	}

	// Get the sound butes.
	CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(m_nCharacterSet), "Pain" );

	int nButes = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);
	if( nButes < 0 )
		return -1;

	SoundButes butes = g_pSoundButeMgr->GetSoundButes(nButes);

	// Be sure the facehugging always plays a pain sound!
	if( eType == DT_FACEHUG )
	{
		butes.m_fMaxDelay = 0.0f;
		butes.m_fPlayChance = 1.0f;
	}


	// Get the pain sound.
	std::string strSoundFile = g_pSoundButeMgr->GetRandomSoundFileWeighted(nButes);

	if( !strSoundFile.empty() && strSoundFile[0] >= '0' && strSoundFile[0] <= '9' )
	{
		strSoundFile = FullSoundName(strSoundFile);
	}


	if( !PlayVoicedSound(nButes, butes,strSoundFile.c_str(),CST_DAMAGE_IMPORTANCE) )
	{
		return -1;
	}

	return nButes;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayExclamationSound
//
//	PURPOSE:	Play a exclamation sound, returns -1 if sound not played, returns bute index if sound was played.
//
// ----------------------------------------------------------------------- //

int CCharacter::PlayExclamationSound(const char* szSoundType)
{
	// Sanity checks.
	if (!szSoundType || !szSoundType[0]) 
		return -1;

	// Don't play a sound if something more important is playing.
	// But go ahead and over-ride a different exclamation.
	if( m_nCurVcdSndImp > CST_EXCLAMATION_IMPORTANCE) 
		return -1;


	if(g_pGameServerShell->LowViolence())
	{
		// Check for low violence settings stuff

		if(!strnicmp(szSoundType, "Cower", 5) || !strnicmp(szSoundType, "Whimper", 7))
			return -1;
	}


	CString strSoundButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(m_nCharacterSet), szSoundType );


	int nButes = -1;
	
	if( strSoundButes.IsEmpty() )
	{
		nButes = g_pSoundButeMgr->GetSoundSetFromName(szSoundType);
	}
	else
	{
		nButes = g_pSoundButeMgr->GetSoundSetFromName(strSoundButes);
	}

	if( nButes < 0 )
		return -1; 

	std::string strSoundFile = g_pSoundButeMgr->GetRandomSoundFileWeighted(nButes);

	if( !strSoundFile.empty() && strSoundFile[0] >= '0' && strSoundFile[0] <= '9' )
	{
		strSoundFile = FullSoundName(strSoundFile);
	}

	const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(nButes);

	if( !PlayVoicedSound(nButes, butes,strSoundFile.c_str(),CST_EXCLAMATION_IMPORTANCE) )
		return -1;

	return nButes;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogueSound
//
//	PURPOSE:	Play a dialog sound, returns -1 if sound not played, returns bute index if sound was played.
//
// ----------------------------------------------------------------------- //

int CCharacter::PlayDialogueSound(const char* szSound)
{
	// Sanity checks.
	if (!szSound || !szSound[0]) 
		return -1;

	CString strDialogButes = g_pServerSoundMgr->GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(m_nCharacterSet), "Dialog" );
	
	int nButes = g_pSoundButeMgr->GetSoundSetFromName(strDialogButes);
	if( nButes < 0 )
		return -1;

	const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(nButes);

	// The dialog sounds should always be played!
	_ASSERT( butes.m_fMaxDelay <= 0.0f );
	_ASSERT( butes.m_fPlayChance == 1.0f );

	if( !PlayVoicedSound(nButes, butes, szSound, CST_DIALOGUE_IMPORTANCE) )
	{
		return -1;
	}

	return nButes;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayVoicedSound
//
//	PURPOSE:	Plays a sound supposedly being voiced by the character.
//				Ie. Dialog, Pain (grunts and such), AI exclamations (battle cries and such)..
//				Returns true if the sound was actually played.
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::PlayVoicedSound(int nBute, const SoundButes & orig_butes, const char* szSound, int nImportance)
{
	// Sanity checks.
	if (!szSound || !szSound[0]) 
		return LTFALSE;

	if ( m_damage.IsDead() )   
		return LTFALSE;

	// Give ourselves a modifiable copy of the butes.
	SoundButes butes = orig_butes;

	// Check to see if we should play this sound.

	// First be sure the sound isn't played too often.
	if( butes.m_fMaxDelay > 0.0f )
	{
		CTimer & sound_delay = m_SoundDelays[nBute];

		// Have we waited long enough?
		if( !sound_delay.Stopped() )
			return LTFALSE;

		// We have waited long enough, set-up the delay for the next time.
		sound_delay.Start( GetRandom( butes.m_fMinDelay, butes.m_fMaxDelay ) );
	}

	// Check the play chance.
	if(butes.m_fPlayChance < 1.0f)
	{
		if(GetRandom(0.0f,1.0f) > butes.m_fPlayChance)
			return LTFALSE;

		// Because the SoundMgr also makes this play chance check,
		// we need to make sure it won't fail to play.
		butes.m_fPlayChance = 1.0f;
	}

	// Kill current sound...
	KillVoicedSound();

	// Set-up the sound.
	butes.m_nFlags = butes.m_nFlags | PLAYSOUND_GETHANDLE;

	bool bIsVoiceOver = g_pGameServerShell->GetGameType() == SP_STORY && IsPlayer(m_hObject);

#ifdef _WIN32
	bool bIsDialogue = ((strstr(szSound,"dialogue\\") != LTNULL) || (strstr(szSound,"Dialogue\\") != LTNULL));
#else
	bool bIsDialogue = (strstr(szSound,"Dialogue/") != LTNULL);
#endif

	if( IsPlayer(m_hObject) )
	{
		butes.m_ePriority += SOUNDPRIORITYBASE_PLAYER;
	}
	else
	{
		butes.m_ePriority += SOUNDPRIORITYBASE_AI;
	}

	// Play the sound!

	LTString hStr;

	hStr = szSound;

	LTFLOAT fRadius = butes.m_fOuterRad;
	LTFLOAT fDuration = 0.0f;


	// PLAYSOUND_CLIENTLOCAL only makes sense for player (client) objects...

	if (!IsPlayer(m_hObject))
	{
		butes.m_nFlags &= ~PLAYSOUND_CLIENTLOCAL;
	}


	if (!bIsVoiceOver
		&& butes.m_bLipSync 
		&& g_pGameServerShell->GetGameType() == SP_STORY )
	{
		// Do lip sync stuff.

		// Tell the client to play the sound (lip synched)...
		// TO DO: Remove sending of sound path, send sound id instead...

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		if(m_hHead)
		{
			g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hHead);
			g_pLTServer->WriteToMessageByte(hMessage, BPFX_NODECONTROL_LIP_SYNC);
		}
		else
		{
			g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_LIP_SYNC);
		}

		g_pLTServer->WriteToMessageHString(hMessage, hStr);
		g_pLTServer->WriteToMessageFloat(hMessage, butes.m_fOuterRad );
		g_pLTServer->WriteToMessageDWord(hMessage, butes.m_nFlags );
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

		m_bCurVcdSndLipSynced = LTTRUE;
		m_bCurVcdSndSubtitled = bIsDialogue;
		// And set up the butes so that the sound has zero radius.
		// That way, it won't be sent to the client (hopefully!).
		butes.m_fOuterRad = 0.0f;
		butes.m_fInnerRad = 0.0f;
	}

	if( bIsVoiceOver )
	{
		butes.m_nFlags |= PLAYSOUND_TIMESYNC;
		m_hCurVcdSnd = g_pServerSoundMgr->PlayLocalSound(butes, szSound);
	}
	else
	{
		// [KLS] Added 9/6/01 - Always playing cinematic dialogue in client's head
		LTBOOL bPlayLocal = LTFALSE;
		if (bIsDialogue && IsExternalCameraActive())
		{
			bPlayLocal = LTTRUE;
		}

		if (bPlayLocal)
		{
			m_hCurVcdSnd = g_pServerSoundMgr->PlayLocalSound(butes, szSound);
		}
		else
		{
			m_hCurVcdSnd = g_pServerSoundMgr->PlayAttachedSound(m_hObject, butes, szSound);
		}
		// [KLS] End 9/6/01 Changes
	}

	if (m_hCurVcdSnd && !m_bCurVcdSndLipSynced && bIsDialogue)
	{
		m_bCurVcdSndSubtitled = LTTRUE;
		g_pLTServer->GetSoundDuration(m_hCurVcdSnd,&fDuration);
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_SUBTITLES);
		g_pLTServer->WriteToMessageHString(hMessage, hStr);
		g_pLTServer->WriteToMessageFloat(hMessage, fRadius );
		g_pLTServer->WriteToMessageFloat(hMessage, fDuration );
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}


	m_nCurVcdSndImp = nImportance;

	// If the sound failed, clear out the information.
	if( !m_hCurVcdSnd )
	{
		m_nCurVcdSndImp = CST_NO_IMPORTANCE;
		m_bCurVcdSndLipSynced = LTFALSE;
		m_bCurVcdSndSubtitled = LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::IsPlayingDialogue
//
//	PURPOSE:	Are we playing a dialogue?
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::IsPlayingDialogue()
{
	if( g_pLTServer->GetObjectUserFlags(m_hObject) & USRFLG_DEACTIVATED )
		return LTFALSE;

	return (m_fDialogueStartTime || m_bPlayingTextDialogue || IsPlayingVoicedSound());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::KillVoicedSound
//
//	PURPOSE:	Kill the current dialogue sound
//
// ----------------------------------------------------------------------- //

void CCharacter::KillVoicedSound()
{
	if (m_hCurVcdSnd)
	{
		g_pLTServer->KillSound(m_hCurVcdSnd);
		m_hCurVcdSnd = LTNULL;
	}

	m_nCurVcdSndImp = CST_NO_IMPORTANCE;


	// Make sure the client knows to stop lip syncing...

	

	if (m_bCurVcdSndLipSynced)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		if(m_hHead)
		{
			g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hHead);
			g_pLTServer->WriteToMessageByte(hMessage, BPFX_NODECONTROL_LIP_SYNC);
		}
		else
		{
			g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_LIP_SYNC);
		}
		g_pLTServer->WriteToMessageHString(hMessage, LTNULL);
		g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
		g_pLTServer->WriteToMessageDWord(hMessage, LTNULL );
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}
	else if (m_bCurVcdSndSubtitled)

	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_SUBTITLES);
		g_pLTServer->WriteToMessageHString(hMessage, LTNULL);
		g_pLTServer->WriteToMessageFloat(hMessage, 0.0f );
		g_pLTServer->WriteToMessageFloat(hMessage, 0.0f );
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}


	m_bCurVcdSndLipSynced = LTFALSE;
	m_bCurVcdSndSubtitled = LTFALSE;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogue
//
//	PURPOSE:	Do dialogue sound/window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::PlayDialogue(DWORD dwID, CinematicTrigger* pCinematic,
	BOOL bWindow, BOOL bStayOpen, const char *szCharOverride, const char *szDecisions,
	BYTE byMood)
{
	if (!dwID) return LTFALSE;

	// Don't play if we are not active.
	if( g_pLTServer->GetObjectUserFlags(m_hObject) & USRFLG_DEACTIVATED )
		return LTFALSE;

	HCONVAR m_hVar = g_pLTServer->GetGameConVar("EE_MillerTime");

	if(m_hVar && g_pLTServer->GetVarValueFloat(m_hVar))
	{
		dwID = 1300 + GetRandom(0, 3);
	}


	char strID[12];
	_snprintf(strID,12,"%d",dwID);
	strID[12] = 0;

	PlayDialogueSound( FullSoundName(strID).c_str() );

	if (bWindow)
	{
		return(DoDialogueWindow(pCinematic,dwID,bStayOpen,szCharOverride,szDecisions));
	}
	else
	{
		m_fDialogueStartTime = g_pLTServer->GetTime();
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogue
//
//	PURPOSE:	Do dialogue sound/window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::PlayDialogue(const char *szDialogue, CinematicTrigger* pCinematic,
	BOOL bWindow, BOOL bStayOpen, const char* szCharOverride,const char *szDecisions,
	BYTE byMood)
{
	if (!szDialogue) return LTFALSE;

	// Don't play if we are not active.
	if( g_pLTServer->GetObjectUserFlags(m_hObject) & USRFLG_DEACTIVATED )
		return LTFALSE;

	CString csSound;
	DWORD dwID = 0;

	HCONVAR m_hVar = g_pLTServer->GetGameConVar("EE_MillerTime");

	if(m_hVar && g_pLTServer->GetVarValueFloat(m_hVar))
	{
		int dwID = 1300 + GetRandom(0, 3);
		char szTemp[12];

		sprintf(szTemp, "%d", dwID);
		csSound = FullSoundName(szTemp).c_str();
	}
	else if ((szDialogue[0] >= '0') && (szDialogue[0] <= '9'))
	{
		// It's an ID
		csSound = FullSoundName(szDialogue).c_str();
	}
	else
	{
		// It's a sound
		csSound = szDialogue;
	}

	PlayDialogueSound(csSound);

	if (bWindow)
	{
		return(DoDialogueWindow(pCinematic,dwID,bStayOpen,szCharOverride,szDecisions));
	}
	else
	{
		m_fDialogueStartTime = g_pLTServer->GetTime();
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DoDialogueWindow
//
//	PURPOSE:	Bring up the dialogue window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::DoDialogueWindow(CinematicTrigger* pCinematic, DWORD dwID, 
	BOOL bStayOpen, const char *szCharOverride, const char *szDecisions)
{
	const char *szTag = "Unknown";
	if (szCharOverride)
	{
		szTag = szCharOverride;
	}

	if (g_DialogueWindow.IsPlaying())
	{
		TRACE("ERROR - Dialogue window was already up when we tried to play dialogue!\n");
		return LTFALSE;
	}

	if (!g_DialogueWindow.PlayDialogue(dwID,bStayOpen,szTag,szDecisions,pCinematic))
	{
		TRACE("ERROR - Could not play dialogue ID: %d\n",dwID);
		return LTFALSE;
	}

	m_bPlayingTextDialogue = LTTRUE;
	m_fDialogueStartTime = 0.0f;

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StopDialogue
//
//	PURPOSE:	Stop the dialogue
//
// ----------------------------------------------------------------------- //

void CCharacter::StopDialogue(LTBOOL bCinematicDone)
{
	KillVoicedSound();
	m_bPlayingTextDialogue = LTFALSE;
	m_fDialogueStartTime = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDead()
//
//	PURPOSE:	Performs necesary stuff for death.  Note that the object
//              will be removed at the end of this function call if bRemoveBody is true!! 
//				Don't do anything after calling this function.
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleDead(LTBOOL bRemoveBody)
{
	StartDeath();
	
	// Create the body prop, which should play the correct
	// death animation and sound.
	if (m_bCreateBody) 
	{
		CreateBody();

		// Now clear out the progressive damage
		m_damage.ClearProgressiveDamages();

		// Be sure all the old flags are reset
		uint32 nFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		g_pLTServer->SetObjectUserFlags(m_hObject, nFlags & ~USERFLG_CHAR_FILTER);
	}

	// Remove us.  Deletes this object!!!
	if( bRemoveBody )
	{
		RemoveObject();
	}

	// See if we need to tell the exsuit spawner about our death...
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		if(IsExosuit(m_hObject) && m_hStartpointSpawner)
		{
			GameStartPoint* pStartPoint = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(m_hStartpointSpawner));

			if(pStartPoint)
			{
				pStartPoint->SetExosuitRespawn();
			}

			// Make sure we only do this once
			m_hStartpointSpawner = LTNULL;
		}

		// Make sure our timer is reset
		m_fChestbursterMutateTime = 0.0f;
		m_fChestbursterMutateDelay = 0.0f;
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveObject()
//
//	PURPOSE:	Handle removing character objects
//
// ----------------------------------------------------------------------- //

void CCharacter::RemoveObject()
{
	// Make sure this object is removed from the global CharacterMgr

	g_pCharacterMgr->Remove(this);

	// Remove the engine object...

	g_pLTServer->RemoveObject(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateBody()
//
//	PURPOSE:	Create the body prop
//
// ----------------------------------------------------------------------- //

HOBJECT CCharacter::CreateBody(LTBOOL bCarryOverAttachments)
{
	HCLASS hClass = g_pLTServer->GetClass("BodyProp");
	if (!hClass) return LTNULL;

	std::string name = g_pLTServer->GetObjectName(m_hObject);
	name += "_dead";

	ObjectCreateStruct theStruct;
	theStruct.Clear();

	// Name our body-prop.
	strncpy( theStruct.m_Name, name.c_str(), MAX_CS_FILENAME_LEN );
	theStruct.m_Name[MAX_CS_FILENAME_LEN] = '\0';  // the char array is actually (MAX_CS_FILENAME_LEN + 1) long.

	// Setup the file names.
	g_pCharacterButeMgr->GetDefaultFilenames(m_nCharacterSet, theStruct);

	// Setup the head skin...

	g_pLTServer->GetObjectPos(m_hObject, &theStruct.m_Pos);
	theStruct.m_Pos += LTVector(0.0f, 0.1f, 0.0f);
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	BodyProp* pProp = (BodyProp *)g_pLTServer->CreateObject(hClass, &theStruct);

	_ASSERT(pProp);
	if (!pProp) return LTNULL;

	pProp->Setup(this, bCarryOverAttachments, GetDefaultDeathAni(), ForceDefaultDeathAni() );


	// Make sure the glow flag is turned off...
	uint32 nUserFlags = g_pLTServer->GetObjectUserFlags(pProp->m_hObject);
	g_pLTServer->SetObjectUserFlags(pProp->m_hObject, nUserFlags & ~USRFLG_GLOW);


	return pProp->m_hObject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateInfoString()
//
//	PURPOSE:	Updates the client side information string for this character
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateInfoString(char *szString)
{
	// DON'T do this in multiplayer... this is for single player debug only
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
		return;

	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_INFO_STRING);
	g_pLTServer->WriteToMessageString(hMessage, szString ? szString : const_cast<char *>("\0"));
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateChestbursterCycle()
//
//	PURPOSE:	Updates the life cycle of the chestburster
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateChestbursterCycle()
{
	// If the character is dead... for god sakes don't do this!!
	if(m_damage.IsDead())
		return;


	// First check to see if we are a chestburster... if not, just leave
	char const* pszType = g_pCharacterButeMgr->GetModelType(GetCharacter());

	if(_stricmp(pszType, "Chestburster") && _stricmp(pszType, "Chestburster_AI") && _stricmp(pszType, "Chestburster_MP"))
		return;

	// Ok... we should be good to do the chestburster update stuff now
	if(m_fChestbursterMutateDelay > 0.0f)
	{
		// Take some off of our delay
		m_fChestbursterMutateDelay -= GetUpdateDelta();

		// If our delay is done... switch our character
		if(m_fChestbursterMutateDelay <= 0.0f)
		{
			// Get the type of character we're supposed to mutate into
			uint32 nNewCharacter = m_nChestburstedFrom;
			LTBOOL bMulti = g_pGameServerShell->GetGameType().IsMultiplayer();

			if(m_nChestburstedFrom == -1)
			{
				nNewCharacter = g_pCharacterButeMgr->GetSetFromModelType(bMulti ? "Drone_MP" : (IsAI(m_hObject) ? "Drone_AI" : "Drone"));
			}
			else if(IsHuman(m_nChestburstedFrom))
			{
				if(g_pCharacterButeMgr->GetClass(m_nChestburstedFrom) == CORPORATE)
					nNewCharacter = g_pCharacterButeMgr->GetSetFromModelType(bMulti ? "Runner_MP" : (IsAI(m_hObject) ? "Runner_AI" : "Runner"));
				else
					nNewCharacter = g_pCharacterButeMgr->GetSetFromModelType(bMulti ? "Drone_MP" : (IsAI(m_hObject) ? "Drone_AI" : "Drone"));
			}
			else if(IsPredator(m_nChestburstedFrom))
			{
				nNewCharacter = g_pCharacterButeMgr->GetSetFromModelType(bMulti ? "Predalien_MP" : (IsAI(m_hObject) ? "Predalien_AI" : "Predalien"));
			}
			else
			{
				nNewCharacter = g_pCharacterButeMgr->GetSetFromModelType(bMulti ? "Runner_MP" : (IsAI(m_hObject) ? "Runner_AI" : "Runner"));
			}

			// Get a handle to the player object
			CPlayerObj *pPObj = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

			// [KLS] We want to make sure the player gets reset correctly in the call to
			// SetCharacter() below, so set this here...
			if (pPObj)
			{
				pPObj->ResetHealthAndArmor(LTTRUE);
			}

			// Set the new character type
			SetCharacter(nNewCharacter);

#ifdef _WIN32
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Sounds\\Weapons\\Alien\\FaceHug\\chestburst.wav", 1000.0f, SOUNDPRIORITY_MISC_HIGH);
#else
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Sounds/Weapons/Alien/FaceHug/chestburst.wav", 1000.0f, SOUNDPRIORITY_MISC_HIGH);
#endif

			if(pPObj)
			{
				pPObj->AquireDefaultWeapon();

				// Update the multiplayer mgr to send the new character information to the clients
				g_pGameServerShell->GetMultiplayerMgr()->ChangeClientCharacter(pPObj->GetClient(), (uint8)nNewCharacter, LTFALSE);
			}
		}
	}
	else
	{
		if(g_pGameServerShell->GetGameType().IsMultiplayer())
		{
			// Initialize the time if it hasn't been yet...
			if(m_fChestbursterMutateTime == 0.0f)
				m_fChestbursterMutateTime = g_pLTServer->GetTime();

			// First things first... get some info about the current state of our guy
			LTFLOAT fScale = m_cmMovement.GetScale();
			LTFLOAT fDiff = g_pLTServer->GetTime() - m_fChestbursterMutateTime;

			// Calculate the new scale we want to be at
			LTFLOAT fNewScale = (fDiff / g_cvarChestbursterMutateTime.GetFloat()) * g_cvarChestbursterMaxScale.GetFloat();

			if(fNewScale >= g_cvarChestbursterMaxScale.GetFloat())
			{
				fNewScale = g_cvarChestbursterMaxScale.GetFloat();

				m_fChestbursterMutateTime = 0.0f;
				m_fChestbursterMutateDelay = g_cvarChestbursterMutateDelay.GetFloat();
			}

			// Only scale us if our new scale is greater than our current... we don't
			// want to end up shrinking if we lose health
			if(fNewScale >= (fScale + 0.25f))
			{
				// Set the scale for our sever object
				m_cmMovement.SetScale(fNewScale);

				// If we're a player, send information to our client
				CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));

				if(pPlayer)
				{
					pPlayer->SendScaleInformation();
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateHitBox()
//
//	PURPOSE:	Create our hit box
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateHitBox()
{
	if (m_hHitBox) return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	
#ifdef _DEBUG
	sprintf(theStruct.m_Name,"%s-HitBox", g_pLTServer->GetObjectName(m_hObject));
#endif

	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);
	
	// Allocate an object...

	CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pHitBox) return;

	m_hHitBox = pHitBox->m_hObject;

	pHitBox->Init(m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateHitBox()
//
//	PURPOSE:	Update our hit box position
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateHitBox()
{
	if (!m_hHitBox)
	{
#ifdef _DEBUG
#ifndef _DEMO
		AICPrint(m_hObject, "Warning: CharacterHitBox missing");
#endif
#endif
		return;
	}

	CCharacterHitBox* pHitBox = static_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
	if (pHitBox)
	{
		pHitBox->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SpawnItem()
//
//	PURPOSE:	Spawn the specified item
//
// ----------------------------------------------------------------------- //

void CCharacter::SpawnItem(char* pItem, LTVector & vPos, LTRotation & rRot)
{
	if (!pItem) return;

	LPBASECLASS pObj = SpawnObject(pItem, vPos, rRot);

	if (pObj && pObj->m_hObject)
	{
		// It is not wise to use the acceleration as that will make objects move faster during slower
		// frame rates.
//		LTVector vAccel;
//		VEC_SET(vAccel, GetRandom(0.0f, 300.0f), GetRandom(100.0f, 200.0f), GetRandom(0.0f, 300.0f));
//		g_pLTServer->SetAcceleration(pObj->m_hObject, &vAccel);

		LTVector vVel;
		VEC_SET(vVel, GetRandom(0.0f, 100.0f), GetRandom(200.0f, 400.0f), GetRandom(0.0f, 100.0f));
		g_pLTServer->SetVelocity(pObj->m_hObject, &vVel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StartDeath()
//
//	PURPOSE:	Start dying
//
// ----------------------------------------------------------------------- //
	
void CCharacter::StartDeath()
{
	// Spawn any special item we were instructed to

	if (m_hstrSpawnItem)
	{
		char* pItem = g_pLTServer->GetStringData(m_hstrSpawnItem);
		if (pItem)
		{
			// Add gravity to the item...

			char buf[300];
			sprintf(buf, "%s Gravity 1", pItem);

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			LTRotation rRot;
			ROT_INIT(rRot);

			SpawnItem(buf, vPos, rRot);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetDefaultDeathAni()
//
//	PURPOSE:	return the character's default death animation.
//
// ----------------------------------------------------------------------- //

std::string CCharacter::GetDefaultDeathAni()
{
	const ModelSkeleton eSkeleton = GetModelSkeleton();
	const std::string strBaseName = g_pModelButeMgr->GetSkeletonDefaultDeathAni(eSkeleton);

	if (IsNetted())
	{
		if (INVALID_ANI != g_pLTServer->GetAnimIndex(m_hObject, "Net_Thrw_Death"))
		{
			return "Net_Thrw_Death";
		}
	}
	if( GetInjuredMovement() )
	{
		if( INVALID_ANI != g_pLTServer->GetAnimIndex( m_hObject, "Injured_Death") )
			return "Injured_Death";		
	}

	char strNodeFragment[8];
	strNodeFragment[0] = 0;

	if(m_eModelNodeLastHit != eModelNodeInvalid)
	{
		sprintf(strNodeFragment, "_%s", g_pModelButeMgr->GetSkeletonNodeDeathAniFrag(eSkeleton, m_eModelNodeLastHit));
	}

	char strDirFragment[8];
	strDirFragment[0] = 0;

	if( m_damage.GetDeathDir().Dot( GetForward() ) > c_fCos80 )
	{
		strcpy(strDirFragment, "_b");
	}

	char strCrouchedFragment[8];
	strCrouchedFragment[0] = 0;

	if( m_cmMovement.IsCrouched() )
	{
		strcpy(strCrouchedFragment, "_c");

		// Override the node fragment...
		if(_stricmp(strNodeFragment, "_h") != 0)
			strcpy(strNodeFragment, "_t");
		else
			// don't have a back head so make sure Dir is empty
			strDirFragment[0] = 0;
	}



	std::string strResult = strBaseName + strCrouchedFragment + strDirFragment + strNodeFragment;

	if( INVALID_ANI != g_pLTServer->GetAnimIndex( m_hObject, const_cast<char*>(strResult.c_str()) ) )
		return strResult;		

	// If the above failed, we do not have a death animation for our current state.
	// The following is a set of arbitrary fallbacks.

	// Try it without direction.
	if( strDirFragment[0] )
	{
		strResult = strBaseName + strCrouchedFragment + strNodeFragment;

		if( INVALID_ANI != g_pLTServer->GetAnimIndex( m_hObject, const_cast<char*>(strResult.c_str()) ) )
			return strResult;		
	}

	// Try it without the node (prefer crouching over the node),
	// then try it with the node but without the crouching.
	if( strCrouchedFragment[0] )
	{
		strResult = strBaseName + strCrouchedFragment;

		if( INVALID_ANI != g_pLTServer->GetAnimIndex( m_hObject, const_cast<char*>(strResult.c_str()) ) )
			return strResult;		

		strCrouchedFragment[0] = 0;

		// Try it with the node but without the crouching.
		strResult = strBaseName + strNodeFragment;
		if( INVALID_ANI != g_pLTServer->GetAnimIndex( m_hObject, const_cast<char*>(strResult.c_str()) ) )
			return strResult;		
	}


	// Couldn't find a valid animation, just do the default.
	return strBaseName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetChestOffset()
//
//	PURPOSE:	Update the offset from our position to our chest
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::GetChestOffset() const
{
	LTVector vOffset;
	VEC_INIT(vOffset);

	LTVector vDims;
	g_pLTServer->GetObjectDims(m_hObject, &vDims);

	// Just make the default offset a bit above the waist...

	vOffset.y = vDims.y * GetButes()->m_fChestOffsetPercent;

	return vOffset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetHeadOffset()
//
//	PURPOSE:	Update the offset from our position to our head
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::GetHeadOffset() const
{
	LTVector vOffset;
	VEC_INIT(vOffset);

	LTVector vDims;
	g_pLTServer->GetObjectDims(m_hObject, &vDims);

	// Just make the default offset a bit below the top....

	vOffset.y = vDims.y * GetButes()->m_fHeadOffsetPercent;

	return vOffset;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateAttachments()
{
	if ( !m_pAttachments ) 
	{
		m_pAttachments = new CHumanAttachments();

		if(m_pAttachments)
		{
			AddAggregate(m_pAttachments);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DestroyAttachments
//
//	PURPOSE:	Destroys our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CCharacter::DestroyAttachments()
{
	if ( m_pAttachments ) 
	{
		delete m_pAttachments; 
		m_pAttachments = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferAttachments
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our 
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

CAttachments* CCharacter::TransferAttachments()
{
	if(m_hHead)
	{
		// remove the head attachemnt
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hHead, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hHead);
		m_hHead = LTNULL;
	}

	if(m_hNet)
	{
		// remove the net
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hNet, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hNet);
		m_hNet = LTNULL;

		// Gib the net
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris("Nets_ammo");
		if (pDebris)
		{
			LTVector vVel;
			LTVector vPos;

			g_pLTServer->GetVelocity(m_hObject, &vVel);
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			vVel.Norm();

			CreatePropDebris(vPos, vVel, pDebris->nId);
		}

		// Reset the net stage stuff
		m_eNetStage = NET_STAGE_INVALID;
		m_eLastNetStage = NET_STAGE_INVALID;

		// Remove movement restrictions
		m_caAnimation.StopAnim();
		m_caAnimation.Enable(LTTRUE);
	}

	if (m_pAttachments)
	{
		m_pAttachments->HandleDeath();
	}

	CAttachments* pAtt = m_pAttachments;
	if (m_pAttachments)
	{
		RemoveAggregate(m_pAttachments);
		m_pAttachments = LTNULL;
	}

	HOBJECT hAttachList[30];
	uint32 dwListSize, dwNumAttachments;

	// At this point there should be no attachments so go ahead and nuke em...
	if (g_pLTServer->Common()->GetAttachments(m_hObject, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		if(dwNumAttachments != 0)
		{
#ifndef _FINAL
			g_pLTServer->CPrint("WARNING: Character still has attachments (%d) upon death.", dwNumAttachments);
#endif
			for (uint32 i = 0; i < dwNumAttachments; i++)
			{
				HATTACHMENT hAttachment;
				if (g_pLTServer->FindAttachment(m_hObject, hAttachList[i], &hAttachment) == LT_OK)
				{
					g_pLTServer->RemoveAttachment(hAttachment);

				}
				g_pLTServer->RemoveObject(hAttachList[i]);
			}
		}
	}

	return pAtt;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ResetCharacter
//
//	PURPOSE:	Resets the character model and attributes
//
// ----------------------------------------------------------------------- //

void CCharacter::ResetCharacter()
{
	if(!m_hObject)
		return;

//	LTBOOL bWasOrIsExosuit = IsExosuit(m_nCharacterSet);
//	
//	if(IsExosuit(m_cmMovement.GetCharacterButes()) && !IsHuman(m_nCharacterSet))
//		bWasOrIsExosuit = LTFALSE;

//	if(g_pGameServerShell->GetGameType().IsMultiplayer())
//	{
//		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
//		g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
//	}

	// [KLS] 10/3/01 - Make sure the attachments are removed and re-created just
	// in case we changed model skeletons...
	if(g_pGameServerShell->GetGameType().IsMultiplayer() && m_bResetAttachments)
	{
		if (m_pAttachments)
		{
			RemoveAggregate(m_pAttachments);
			DestroyAttachments();
		}
	}

	// Set the attributes to the correct settings
	m_cmMovement.SetCharacterButes(m_nCharacterSet);
	m_cmMovement.SetScale(1.0f);

	// [KLS] 10/3/01 - Recreate/Init the attachments as necessary...
	if(g_pGameServerShell->GetGameType().IsMultiplayer() && m_bResetAttachments)
	{
		CreateAttachments();
		if (m_pAttachments)
		{
			m_pAttachments->Init(m_hObject);
		}
	}

	// reset the flag
	m_bResetAttachments = LTTRUE;

	// Reset all the animation stuff
	m_caAnimation.ClearLayers();

	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		for(int i = 0; i < MAX_ANIM_LAYERS; i++)
		{
			char *szLayer = m_cmMovement.GetCharacterButes()->m_szMPAnimLayers[i];

			if(szLayer[0])
				m_caAnimation.AddLayer(szLayer, i);
		}
	}
	else
	{
		for(int i = 0; i < MAX_ANIM_LAYERS; i++)
		{
			char *szLayer = m_cmMovement.GetCharacterButes()->m_szAnimLayers[i];

			if(szLayer[0])
				m_caAnimation.AddLayer(szLayer, i);
		}
	}

	// Set the animation type to our basic AI animations
	m_caAnimation.SetLayerReference(ANIMSTYLELAYER_STYLE, "Standard");
	m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");
	m_caAnimation.SetLayerReference(ANIMSTYLELAYER_ACTION, "Idle");


	// Reset the user flags and make sure the surface type user flag is set properly
	uint32 dwInitUserFlags = USRFLG_MOVEABLE | USRFLG_CHARACTER;

	uint32 dwSTFlag = SurfaceToUserFlag((SurfaceType)m_cmMovement.GetCharacterButes()->m_nSurfaceType);
	uint32 dwUserFlags = UserFlagSurfaceMask(dwInitUserFlags);

	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | dwSTFlag);


	// Make sure that other functions know if we reset the character on this update
	m_bCharacterReset = LTTRUE;


	// Make sure the airlevel is set at the max
	m_fAirSupplyTime = GetButes()->m_fAirSupplyTime;


	// Make sure the charge level is re-set
	m_nCannonChargeLevel = 0;
	SetMaxChargeLevel();

	// Make sure our timers are reset
	m_fChestbursterMutateTime	= 0.0f;
	m_fChestbursterMutateDelay	= 0.0f;

	// Set function pointer for exosuit energy
#ifndef __LINUX
	fpUpdateExoEnergy = IsExosuit(m_cmMovement.GetCharacterButes()) ? UpdateExoEnergy : LTNULL;
#else
	if (IsExosuit(m_cmMovement.GetCharacterButes()))
		fpUpdateExoEnergy = &CCharacter::UpdateExoEnergy;
	else
		fpUpdateExoEnergy = LTNULL;
#endif

	m_cmMovement.SetObjectFlags(OFT_Flags2, m_cmMovement.GetObjectFlags(OFT_Flags2) | FLAG2_DYNAMICDIRLIGHT, LTTRUE);

	// If we're an alien... set the chromakey flag
	if(IsAlien(m_cmMovement.GetCharacterButes()))
		m_cmMovement.SetObjectFlags(OFT_Flags2, m_cmMovement.GetObjectFlags(OFT_Flags2) | FLAG2_CHROMAKEY, LTTRUE);

	// Also... if we're not a wall walker then only use Y rotations
	if(!m_cmMovement.GetCharacterButes()->m_bWallWalk)
		m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) | FLAG_YROTATION, LTTRUE);


	// Make sure the damage aggregate stuff is reset
	m_damage.ClearProgressiveDamages();
	m_damage.SetCanCrush(LTTRUE);

	// Don't base the blocking priority on the mass.
	m_damage.SetMassBasedBlockingPriority( LTFALSE );
	m_damage.SetBlockingPriority( CHARACTER_BLOCKING_PRIORITY );

	m_damage.SetMass(g_pCharacterButeMgr->GetDefaultMass(m_nCharacterSet));

	if( !IsPlayer(m_hObject) )
	{
		m_damage.SetHitPoints((m_fDefaultHitPts > 0.0f) ? m_fDefaultHitPts : g_pCharacterButeMgr->GetDefaultHitPoints(m_nCharacterSet));
		m_damage.SetMaxHitPoints((m_fDefaultHitPts > 0.0f) ? m_fDefaultHitPts : g_pCharacterButeMgr->GetMaxHitPoints(m_nCharacterSet));
	
		m_damage.SetArmorPoints((m_fDefaultArmor > 0.0f) ? m_fDefaultArmor : g_pCharacterButeMgr->GetDefaultArmorPoints(m_nCharacterSet));
		m_damage.SetMaxArmorPoints((m_fDefaultArmor > 0.0f) ? m_fDefaultArmor : g_pCharacterButeMgr->GetMaxArmorPoints(m_nCharacterSet));

		m_damage.SetHitPoints( m_damage.GetHitPoints()*GetDifficultyFactor() );
		m_damage.SetMaxHitPoints( m_damage.GetMaxHitPoints()*GetDifficultyFactor() );

		m_damage.SetArmorPoints( m_damage.GetArmorPoints()*GetDifficultyFactor(), LTTRUE);
//		m_damage.SetMaxArmorPoints( m_damage.GetMaxArmorPoints()*GetDifficultyFactor() );
	}
	else
	{
		// see if we need to reset the health and armor
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(this);

		if(pPlayer && pPlayer->IsResetHealthAndArmor())
		{
			// turn the reset off...
			pPlayer->ResetHealthAndArmor(LTFALSE);
			pPlayer->ResetPlayerHealth();
		}
	}

	// Reset our hit nodes.
	if( m_hHitBox )
	{
		CCharacterHitBox * pHitBox = dynamic_cast<CCharacterHitBox*>( g_pLTServer->HandleToObject(m_hHitBox) );
		if( pHitBox )
			pHitBox->ResetSkeleton();
	}

	// Send an SFX message to make sure the special FX get set to the proper character set
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_RESET_ATTRIBUTES);
	g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_nCharacterSet);
	g_pLTServer->EndMessage(hMessage);//, MESSAGE_NAGGLEFAST | MESSAGE_GUARANTEED);


	// Make sure this object is added to the global CharacterMgr...
	// The Add function will check to see if this object already exists... so no worries.

	g_pCharacterMgr->Add(this);


	// Setup the required attachments
	if(m_pAttachments)
	{
		m_pAttachments->ResetRequirements();

		// Make sure to deal with the sprites...
		CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		HOBJECT hWeapon = pAttachedWeapon?pAttachedWeapon->GetModel():LTNULL;

		HOBJECT hAttachList[30];
		uint32 dwListSize, dwNumAttachments;

		if (g_pLTServer->Common()->GetAttachments(m_hObject, hAttachList, 
			ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
		{
			for (uint32 i = 0; i < dwNumAttachments; i++)
			{
				if(hAttachList[i] != hWeapon)
				{
					HATTACHMENT hAttachment;

					if (g_pLTServer->FindAttachment(m_hObject, hAttachList[i], &hAttachment) == LT_OK)
					{
						g_pLTServer->RemoveAttachment(hAttachment);
					}

					g_pLTServer->RemoveObject(hAttachList[i]);

#ifndef _FINAL
					g_pLTServer->CPrint("WARNING: Character still has unaccounted for attachments at reset.");
#endif
				}
			}
		}

		m_pAttachments->AddRequirements(m_nCharacterSet);
	}

	// Setup the model pieces
	SetupModelPieces();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetupModelPieces
//
//	PURPOSE:	Make sure the appropriate model pieces are shown
//
// ----------------------------------------------------------------------- //

void CCharacter::SetupModelPieces()
{
	// First go through and make all the pieces visible...
	for(int i = 0; i < 32; i++)
	{
		g_pModelLT->SetPieceHideStatus(m_hObject, (HMODELPIECE)i, LTFALSE);
	}


	// Now hide any pieces that shouldn't be shown
	HMODELPIECE hPiece;

	if(IsPredator(m_cmMovement.GetCharacterButes()))
	{
		if(HasMask())
			g_pModelLT->GetPiece(m_hObject, "Head", hPiece);
		else
			g_pModelLT->GetPiece(m_hObject, "Head_Mask", hPiece);

		g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateExoEnergy
//
//	PURPOSE:	Adjust the amount of energy based on movement...
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateExoEnergy()
{
	//get the control flags
	uint32 nFlags = m_cmMovement.GetControlFlags();
	uint32 nState = m_cmMovement.GetMovementState();

	//get our current energy
	uint32 nEnergy = GetExosuitEnergy();

	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		// just recharge if we are idle
		if(nState == CMS_IDLE)
			m_fEnergyChange += (GetUpdateDelta() * g_cvarExoRegenRate.GetFloat());

		DecrementExosuitEnergy();
		return;
	}


	// If the character is in liquid... update the air supply
	if(nFlags & (CM_FLAG_DIRECTIONAL | CM_FLAG_DUCK | CM_FLAG_JUMP) || nState != CMS_IDLE)
	{
		// Start taking away our energy supply time
		if(nFlags & CM_FLAG_DIRECTIONAL)
		{
			//im either walking or running
			if(nFlags & CM_FLAG_RUN)
			{
				//bill em for running
				m_fEnergyChange -= g_cvarExoRunRate.GetFloat()*GetUpdateDelta();
			}
			else
			{
				//tag em for walking
				m_fEnergyChange -= g_cvarExoWalkRate.GetFloat()*GetUpdateDelta();
			}
		}

		if(nFlags & CM_FLAG_DUCK)
		{
			m_fEnergyChange -= g_cvarExoCrouchRate.GetFloat()*GetUpdateDelta();
		}

		switch(nState)
		{
			case CMS_PRE_JUMPING:
			{
				m_fEnergyChange -= g_cvarExoJumpRate.GetFloat();
				break;
			}

			case CMS_CLIMBING:
			{
				m_fEnergyChange -= g_cvarExoClimbRate.GetFloat()*GetUpdateDelta();
				break;
			}

			case CMS_SWIMMING:
			{ 
				if(!(nFlags & CM_FLAG_DIRECTIONAL) && !(nFlags & CM_FLAG_DUCK) && !(nFlags & CM_FLAG_JUMP))
				{
					if(!m_bExosuitDisabled)
						m_fEnergyChange += (GetUpdateDelta() * g_cvarExoRegenRate.GetFloat());
				}

				if((nFlags & CM_FLAG_DUCK) || (nFlags & CM_FLAG_JUMP))
					m_fEnergyChange -= g_cvarExoWalkRate.GetFloat()*GetUpdateDelta();

				break; 
			}
		}

		if(nEnergy == 0 && m_fEnergyChange < 0.0f)
		{
			m_fEnergyChange = 0.0f;

			// Send damage message...
			DFLOAT fDamage = g_cvarExoDepriveRate.GetFloat() * GetUpdateDelta();

			// Don't do 0 damage or you get pain sounds.
			if(fDamage != 0.0f)
			{
				DamageStruct damage;

				damage.eType	= DT_CHOKE;
				damage.fDamage	= fDamage;
				damage.hDamager = m_hObject;

				damage.DoDamage(this, m_hObject);
			}
		}
	}
	else
	{
		// Regenerate time
		if(!m_bExosuitDisabled)
			m_fEnergyChange += (GetUpdateDelta() * g_cvarExoRegenRate.GetFloat());
	}

	if(m_fEnergyChange != 0)
		DecrementExosuitEnergy();
}


// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //


CCharacterPlugin::~CCharacterPlugin()
{
	delete m_pCharacterButeMgrPlugin;
	m_pCharacterButeMgrPlugin = LTNULL;

	delete m_pHumanAttachmentsPlugin;
	m_pHumanAttachmentsPlugin = LTNULL;
}

LTRESULT	CCharacterPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return LT_UNSUPPORTED;

	if( !m_pCharacterButeMgrPlugin )
	{
		m_pCharacterButeMgrPlugin = new CCharacterButeMgrPlugin();
	}

	if( !m_pHumanAttachmentsPlugin )
	{
		m_pHumanAttachmentsPlugin = new CHumanAttachmentsPlugin();
	}

	if(m_pHumanAttachmentsPlugin->PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		return LT_OK;

	if (_stricmp("CharacterType", szPropName) == 0)
	{
		m_pCharacterButeMgrPlugin->PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		if( pcStrings && *pcStrings > 0)
			return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacter::ComputeDamageModifier(ModelNode eModelNode) const
{
	return g_pModelButeMgr->GetSkeletonNodeDamageFactor(GetModelSkeleton(), eModelNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleCharacterSFXMessage
//
//	PURPOSE:	Handle character specific sfx messages from client to us (like the flashlight).
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleCharacterSFXMessage(HCLIENT hSender, HMESSAGEREAD hRead) 
{ 
	const uint8 cfx_id = hRead->ReadByte();

	switch(cfx_id)
	{
		case CFX_FLASHLIGHT:
		{
			// FL_ON = 0, FL_OFF = 1, FL_UPDATE = 2
			const uint8 nStatus = hRead->ReadByte();

			switch (nStatus)
			{
				case 0:
				{
					SetFlashlight(TRUE);
					break;
				}
				case 1:
				{
					SetFlashlight(FALSE);
					break;
				}
				case 2:
				{
					LTVector vPos = hRead->ReadCompVector();
					SetFlashlightPos(vPos);
					break;
				}
			}
		}
		break;

		case CFX_MODELLIGHTING:
		{
			m_vModelLighting = hRead->ReadVector();
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleEMPEffect
//
//	PURPOSE:	Handle EMP effect...
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleEMPEffect() 
{ 
	uint32 nEMPFlags = g_pCharacterButeMgr->GetEMPFlags(m_nCharacterSet);

	if(!m_bEMPEffect && (nEMPFlags & EMP_FLAG_STUN))
	{
		m_cmMovement.AllowMovement(LTFALSE);
		m_cmMovement.AllowRotation(LTFALSE);
		m_bEMPEffect = LTTRUE;
	}

	//set predator energy to 0
	DecrementPredatorEnergy(GetPredatorEnergy());

	//turn off cloaking
	ForceCloakOff(LTTRUE);

	//clear the battery power
	m_fBatteryChargeLevel = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleStunEffect
//
//	PURPOSE:	Handle Stun effect...
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleStunEffect() 
{ 
	if(!m_bStunEffect)
	{
		m_cmMovement.AllowMovement(LTFALSE);
		m_cmMovement.AllowRotation(LTFALSE);
		m_bStunEffect = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateEMPEffect
//
//	PURPOSE:	Update the EMP effect...
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateEMPEffect() 
{ 
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	if(!(dwUserFlags & USRFLG_CHAR_EMPEFFECT))
	{
		//reset time
		uint32 nEMPFlags = g_pCharacterButeMgr->GetEMPFlags(m_nCharacterSet);

		if(nEMPFlags & EMP_FLAG_STUN)
		{
			m_cmMovement.AllowMovement(LTTRUE);
			m_cmMovement.AllowRotation(LTTRUE);
		}

		m_bEMPEffect = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateStunEffect
//
//	PURPOSE:	Update the Stun effect...
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateStunEffect() 
{ 
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	if(!(dwUserFlags & USRFLG_CHAR_STUNNED))
	{
		m_cmMovement.AllowMovement(LTTRUE);
		m_cmMovement.AllowRotation(LTTRUE);

		m_bStunEffect = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetHasCloakDevice
//
//	PURPOSE:	Set the cloaking information
//
// ----------------------------------------------------------------------- //

void CCharacter::SetHasCloakDevice(LTBOOL bHas) 
{
	// Only Predator's can use a cloaking device
	if(IsPredator(m_cmMovement.GetCharacterButes()))
		m_bHasCloakDevice = bHas;
	else
		m_bHasCloakDevice = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetHasMask
//
//	PURPOSE:	Set the mask
//
// ----------------------------------------------------------------------- //

void CCharacter::SetHasMask(LTBOOL bHas, LTBOOL bMessage) 
{
	// Only Predator's can use the mask
	if(!IsPredator(m_cmMovement.GetCharacterButes()))
	{
		m_bHasMask = LTFALSE;
		return;
	}

	// Set our variable
	m_bHasMask = bHas;

	// Setup the model pieces
	SetupModelPieces();

	// If this is a player... send a message to the client
	CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(this);

	if(pPlayer && pPlayer->GetClient())
	{
		uint8 nFlags = (m_bHasMask ? 0x01 : 0x00) | (bMessage ? 0x02 : 0x00);

		HMESSAGEWRITE hMsg = g_pInterface->StartMessage(pPlayer->GetClient(), MID_PREDATORMASK);
		g_pInterface->WriteToMessageByte(hMsg, nFlags);
		g_pInterface->EndMessage(hMsg);
	}

	// Make sure to deal with the sprites...
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(GetAttachments());
	if(!pAttach) return;

	CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());
	if(!pAttachedWeapon) return;

	CWeapons* pWeapons = pAttachedWeapon->GetWeapons();
	if(!pWeapons) return;

	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeapons->GetCurWeaponId());
	if (!pWeaponData) return;

	SetTargetingSprite(m_bHasMask && pWeaponData->nTargetingType==1);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetHasNightVision
//
//	PURPOSE:	Set night vision
//
// ----------------------------------------------------------------------- //

void CCharacter::SetHasNightVision(LTBOOL bHas) 
{
	// Only Predator's can use the mask
	if(!IsHuman(m_cmMovement.GetCharacterButes()))
	{
		m_bHasNightVision = LTFALSE;
		return;
	}

	// Set our variable
	m_bHasNightVision = bHas;

	// If this is a player... send a message to the client
	CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(this);

	if(pPlayer)
	{
		uint8 nFlags = (m_bHasNightVision ? 0x01 : 0x00);

		HMESSAGEWRITE hMsg = g_pInterface->StartMessage(pPlayer->GetClient(), MID_NIGHTVISION);
		g_pInterface->WriteToMessageByte(hMsg, nFlags);
		g_pInterface->EndMessage(hMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateMaskPickup
//
//	PURPOSE:	Creates a mask pickup and sends it flying
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateMaskPickup(LTVector *vVel, LTBOOL bMessage)
{
	// If we're not a Predator... then we shouldn't be in here at all
	if(!IsPredator(m_cmMovement.GetCharacterButes()) || !HasMask())
		return;

	// See if multiplayer server options would turn this off
	if(g_pGameServerShell->GetGameType().IsMultiplayer() && !g_pGameServerShell->GetGameInfo()->m_bMaskLoss)
		return;


	// Get the position and rotation
	LTVector vPos, vDims;
	LTRotation rRot;

	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pLTServer->GetObjectDims(m_hObject, &vDims);
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	vPos.y += vDims.y;


	char *szName = m_cmMovement.GetCharacterButes()->m_szName;
	char szType[64];

	if(strstr(szName, "Light"))
		strcpy(szType, "Light_Predator_Mask");
	else if(strstr(szName, "Heavy"))
		strcpy(szType, "Heavy_Predator_Mask");
	else if(strstr(szName, "MHawk"))
		strcpy(szType, "MHawk_Predator_Mask");
	else if(strstr(szName, "Predator"))		// This one must be last
		strcpy(szType, "Predator_Mask");


	char szSpawn[128];
	sprintf(szSpawn, "PickupObject Name PredMask; Pickup %s; ForceNoRespawn 1", szType);


	// Create the object
	BaseClass* pObj = SpawnObject(szSpawn, vPos, rRot);

	if(pObj && pObj->m_hObject)
	{
		LTVector vR, vU, vF;
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

		LTVector vDir;
		
		if(vVel)
		{
			vDir = *vVel;
		}
		else
		{
			// Calculate a velocity
			vDir = vF * GetRandom(175.0f, 250.0f);
			vDir += vR * GetRandom(-75.0f, 75.0f);
			vDir += vU * 100.0f;
		}

		g_pLTServer->SetVelocity(pObj->m_hObject, &vDir);

		// Set our mask value
		SetHasMask(LTFALSE, bMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::EyeFlash
//
//	PURPOSE:	Plays the eye flash for the predator
//
// ----------------------------------------------------------------------- //

void CCharacter::EyeFlash(uint8 nColorDef)
{
	// If we're not a Predator... then we shouldn't be in here at all
	if(!IsPredator(m_cmMovement.GetCharacterButes()) || !HasMask())
		return;


	// Setup the color
	uint32 nColor;
	
	switch(nColorDef)
	{
		case EYEFLASH_COLOR_RED:
			nColor = SETRGB(255, 0, 0);
			break;

		case EYEFLASH_COLOR_BLUE:
			nColor = SETRGB(0, 0, 255);
			break;

		case EYEFLASH_COLOR_GREEN:
		default:
			nColor = SETRGB(0, 255, 0);
			break;
	}


	// Send the message to the clients
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_EYEFLASH);
	g_pLTServer->WriteToMessageDWord(hMessage, nColor);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleCloakToggle
//
//	PURPOSE:	Set the cloaking information
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleCloakToggle() 
{ 
	// If this character doesn't have a cloak device... just return and make
	// sure the effect is inactive
	if(!HasCloakDevice() && IsPlayer(m_hObject))
	{
		m_nCloakState = CLOAK_STATE_INACTIVE;
		return;
	}

	// Now see if we are dealyed
	if(g_pLTServer->GetTime()-m_fCloakDelayTime < g_cvarCloakDelayTime.GetFloat())
	{
		// Make a noise
		g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakfail");

		return;
	}

	// Now see if we are in water
	if (m_cmMovement.IsInLiquidContainer(CM_CONTAINER_LOWER))
	{
		// Make a noise
		g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakfail");

		return;
	}

	// Handle the states differently
	switch(m_nCloakState)
	{
		case CLOAK_STATE_RAMPUP:
		{
			LTFLOAT fPercent = ((g_pLTServer->GetTime() - m_fCloakStartTime) / CLOAK_RAMPUP_TIME);
			m_fCloakStartTime = g_pLTServer->GetTime() - (CLOAK_RAMPDOWN_TIME * (1.0f - fPercent));
			m_nCloakState = CLOAK_STATE_RAMPDOWN;

			// Make a noise
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakoff");
			break;
		}

		case CLOAK_STATE_ACTIVE:
		{
			m_fCloakStartTime = g_pLTServer->GetTime();
			m_nCloakState = CLOAK_STATE_RAMPDOWN;

			// Make a noise
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakoff");
			break;
		}

		case CLOAK_STATE_RAMPDOWN:
		{
			// See if we have enuf juice to turn on...
			if( GetPredatorEnergy() >= (uint32)g_cvarCloakInitialCost.GetFloat())
			{
				LTFLOAT fPercent = ((g_pLTServer->GetTime() - m_fCloakStartTime) / CLOAK_RAMPDOWN_TIME);
				m_fCloakStartTime = g_pLTServer->GetTime() - (CLOAK_RAMPUP_TIME * (1.0f - fPercent));
				m_nCloakState = CLOAK_STATE_RAMPUP;

				// Make a noise
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakon");

					// Ding us for the energy cost...
				DecrementPredatorEnergy((uint32)g_cvarCloakInitialCost.GetFloat());
			}
			else
			{
				// No juice so play a fail sound...
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakfail");
			}
			break;
		}

		default:
		case CLOAK_STATE_INACTIVE:
		{
			// See if we have enuf juice to turn on...
			if( GetPredatorEnergy() >= (uint32)g_cvarCloakInitialCost.GetFloat())
			{
				// Make sure our user flag is set... this will be unset by the update
				// function when the cloaking is done
				uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
				g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_CHAR_CLOAKED);

				m_fCloakStartTime = g_pLTServer->GetTime();
				m_nCloakState = CLOAK_STATE_RAMPUP;

				// Make a noise
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakon");

				// Ding us for the energy cost...
				DecrementPredatorEnergy((uint32)g_cvarCloakInitialCost.GetFloat());
			}
			else
			{
				// No juice so play a fail sound...
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakfail");
			}
			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ForceCloakOff
//
//	PURPOSE:	Force a predator to turn off cloaking
//
// ----------------------------------------------------------------------- //

void CCharacter::ForceCloakOff(LTBOOL bDelay) 
{ 
	// Handle the states differently
	switch(m_nCloakState)
	{
		case CLOAK_STATE_RAMPUP:
		{
			LTFLOAT fPercent = ((g_pLTServer->GetTime() - m_fCloakStartTime) / CLOAK_RAMPUP_TIME);
			m_fCloakStartTime = g_pLTServer->GetTime() - (CLOAK_RAMPDOWN_TIME * (1.0f - fPercent));
			m_nCloakState = CLOAK_STATE_RAMPDOWN;

			if(bDelay)
			{
				// Time stamp the cloak dealy
				m_fCloakDelayTime = g_pLTServer->GetTime();

				// Play the cloak charge sound
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakcharge");
			}
			break;
		}

		case CLOAK_STATE_ACTIVE:
		{
			m_fCloakStartTime = g_pLTServer->GetTime();
			m_nCloakState = CLOAK_STATE_RAMPDOWN;

			if(bDelay)
			{
				// Time stamp the cloak dealy
				m_fCloakDelayTime = g_pLTServer->GetTime();

				// Play the cloak charge sound
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakcharge");
			}
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ResetCloak
//
//	PURPOSE:	Force a predator to turn off cloaking with no FX
//
// ----------------------------------------------------------------------- //

void CCharacter::ResetCloak() 
{ 
	m_nCloakState = CLOAK_STATE_INACTIVE;

	// Make sure the user flags are set properly
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_CHAR_CLOAKED);

	// Turn on Predator's shadow
	m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) | FLAG_SHADOW, LTFALSE);

	// Get the color of our character and fade him out
	LTFLOAT r, g, b, a;
	g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);
	g_pLTServer->SetObjectColor(m_hObject, r, g, b, 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateCloakEffect
//
//	PURPOSE:	Update the cloaking effect...
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateCloakEffect() 
{ 
	if (m_cmMovement.IsInLiquidContainer(CM_CONTAINER_LOWER))
	{
		if(m_nCloakState != CLOAK_STATE_INACTIVE && m_nCloakState != CLOAK_STATE_RAMPDOWN)
		{
			ForceCloakOff();

			// Make a noise
			g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "predcloakoff");
		}
	}

	// Handle the states differently
	switch(m_nCloakState)
	{
		case CLOAK_STATE_INACTIVE:
		{
			// Make sure the user flags are set properly
			uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
			g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_CHAR_CLOAKED);

			// Turn on Predator's shadow
			m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) | FLAG_SHADOW, LTFALSE);
			break;
		}

		case CLOAK_STATE_RAMPUP:
		{
			// Make sure the user flags are set properly
			uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
			g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_CHAR_CLOAKED);

			// Get the time difference
			LTFLOAT fTimeDelta = g_pLTServer->GetTime() - m_fCloakStartTime;
			LTFLOAT fDestAlpha = 1.0f - (fTimeDelta / CLOAK_RAMPUP_TIME);

			// Change states if needed
			if(fDestAlpha < 0.0f)
			{
				m_nCloakState = CLOAK_STATE_ACTIVE;
				return;
			}

			// Get the color of our character and fade him out
			LTFLOAT r, g, b, a;
			g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);
			g_pLTServer->SetObjectColor(m_hObject, r, g, b, fDestAlpha);

			// Update the energy used
			m_fCloakEnergyUsed += g_cvarCloakCostPerSecond.GetFloat() * GetUpdateDelta();
			break;
		}

		case CLOAK_STATE_ACTIVE:
		{
			// Make sure the user flags are set properly
			uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
			g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_CHAR_CLOAKED);

			// Turn off Predator's shadow
			m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) & ~FLAG_SHADOW, LTFALSE);

			LTFLOAT fDestAlpha = CloakAmountFromState(m_cmMovement.GetMovementState(), m_cmMovement.GetControlFlags());

			// Set the color of our character to fade him out
			LTFLOAT r, g, b, a;
			g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);
			g_pLTServer->SetObjectColor(m_hObject, r, g, b, fDestAlpha);

			// Update the energy used
			m_fCloakEnergyUsed += g_cvarCloakCostPerSecond.GetFloat() * GetUpdateDelta();
			break;
		}

		case CLOAK_STATE_RAMPDOWN:
		{
			// Make sure the user flags are set properly
			uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
			g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | USRFLG_CHAR_CLOAKED);

			// Get the time difference
			LTFLOAT fTimeDelta = g_pLTServer->GetTime() - m_fCloakStartTime;
			LTFLOAT fDestAlpha = (fTimeDelta / CLOAK_RAMPDOWN_TIME);

			// Get the color of our character 
			LTFLOAT r, g, b, a;
			g_pLTServer->GetObjectColor(m_hObject, &r, &g, &b, &a);

			// Change states if needed
			if(fDestAlpha > 1.0f)
			{
				m_nCloakState = CLOAK_STATE_INACTIVE;
				fDestAlpha = 1.0f;
			}

			// Fade him out
			g_pLTServer->SetObjectColor(m_hObject, r, g, b, fDestAlpha);
			break;
		}
	}

	// Ok update the energy pool
	if(m_fCloakEnergyUsed >= 1.0f)
	{
		DecrementPredatorEnergy((int)m_fCloakEnergyUsed);

		m_fCloakEnergyUsed -= (int)m_fCloakEnergyUsed;

		// Now check if there is any energy left
		if(GetPredatorEnergy() == 0)
			ForceCloakOff();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDiskRetrieve
//
//	PURPOSE:	Handle retrieving my disk...
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleDiskRetrieve() 
{ 
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

	g_pLTServer->FindNamedObjects("PredatorDisk",objArray);

	int numObjects = objArray.NumObjects();
    if (!numObjects) return;

	CPredDisk *pDisk = LTNULL;
	
	for (int i = 0; i < numObjects; i++)
	{
		pDisk = dynamic_cast<CPredDisk*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));

		if(pDisk)
		{
			if(pDisk->GetFiredFrom() == m_hObject)
			{
				pDisk->HandleDiskRetrieve();
				return;
			}
		}
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetPredatorEnergy
//
//	PURPOSE:	Returns the amount on energy in the predator ammo pool
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetPredatorEnergy()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				return pWeapons->GetAmmoPoolCount(g_pWeaponMgr->GetAmmoPool("Predator_Energy")->nId);
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetPredatorEnergy
//
//	PURPOSE:	Returns the amount on energy in the predator ammo pool
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetMaxPredatorEnergy()
{
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Predator_Energy");

	if(pPool)
		return pPool->GetMaxAmount(m_hObject);

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DecrementPredatorEnergy
//
//	PURPOSE:	Reduces the amount of predator energy by nAmount
//
// ----------------------------------------------------------------------- //

void CCharacter::DecrementPredatorEnergy(int nAmount)
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				pWeapons->DecrementAmmoPool(g_pWeaponMgr->GetAmmoPool("Predator_Energy")->nId, nAmount);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetExoEnergy
//
//	PURPOSE:	Returns the amount on energy in the Exosuit ammo pool
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetExosuitEnergy()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				return pWeapons->GetAmmoPoolCount(g_pWeaponMgr->GetAmmoPool("Exosuit_Energy")->nId);
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetExoEnergy
//
//	PURPOSE:	Returns the maximum amount of energy in the Exosuit.
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetMaxExosuitEnergy()
{ 
	AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Energy");

	if(!pPool) return 0;

	return pPool->GetMaxAmount(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DecrementExoEnergy
//
//	PURPOSE:	Reduces the amount of Exosuit energy
//
// ----------------------------------------------------------------------- //

void CCharacter::DecrementExosuitEnergy()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			int nAmount = (int)m_fEnergyChange;

			if(pWeapons)
			{
				AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Energy");

				if(!pPool) return;

				pWeapons->DecrementAmmoPool(pPool->nId, -nAmount);
			}

			//reset energy
			m_fEnergyChange += -nAmount;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetExosuitEnergy
//
//	PURPOSE:	Sets the amount of Exosuit energy
//
// ----------------------------------------------------------------------- //

void CCharacter::SetExosuitEnergy(int nAmount)
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Energy");

				if(!pPool) return;

				pWeapons->SetAmmo(pPool->nId, nAmount==-1? pPool->GetMaxAmount(m_hObject): nAmount);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ResetExosuitEnergy
//
//	PURPOSE:	Reduces the amount of Exosuit energy
//
// ----------------------------------------------------------------------- //

void CCharacter::ResetExosuitEnergy()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Energy");

			if(!pPool) return;

			int nMaxAmount = pPool->GetMaxAmount(m_hObject);
			
			if(pWeapons)
			{
				pWeapons->SetAmmoPool("Exosuit_Energy", nMaxAmount);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetExoBulletAmmo
//
//	PURPOSE:	Returns the amount of exosuit bullet ammo
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetExoBulletAmmo()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				return pWeapons->GetAmmoPoolCount(g_pWeaponMgr->GetAmmoPool("Exosuit_Bullets")->nId);
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetExoBulletAmmo
//
//	PURPOSE:	Sets the amount of Exosuit bullet ammo
//
// ----------------------------------------------------------------------- //

void CCharacter::SetExoBulletAmmo(int nAmount)
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	nAmount = nAmount==-1? 150: nAmount;

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Bullets"); if(!pPool) return;

				pWeapons->SetAmmo(pPool->nId, nAmount);

				if(IsPlayer(m_hObject))
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_INFOCHANGE);
					g_pLTServer->WriteToMessageByte(hMessage, IC_AMMO_ID);
					g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
					g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)nAmount);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetExoFlamerAmmo
//
//	PURPOSE:	Returns the amount of exosuit flamer ammo
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetExoFlamerAmmo()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				return pWeapons->GetAmmoPoolCount(g_pWeaponMgr->GetAmmoPool("Exosuit_Napalm")->nId);
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetExoFlamerAmmo
//
//	PURPOSE:	Sets the amount of Exosuit flamer ammo
//
// ----------------------------------------------------------------------- //

void CCharacter::SetExoFlamerAmmo(int nAmount)
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	nAmount = nAmount==-1? 100: nAmount;

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Napalm"); if(!pPool) return;

				pWeapons->SetAmmo(pPool->nId, nAmount);

				if(IsPlayer(m_hObject))
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_INFOCHANGE);
					g_pLTServer->WriteToMessageByte(hMessage, IC_AMMO_ID);
					g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
					g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)nAmount);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetExoPlasmaAmmo
//
//	PURPOSE:	Returns the amount of exosuit plasma ammo
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetExoPlasmaAmmo()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				return pWeapons->GetAmmoPoolCount(g_pWeaponMgr->GetAmmoPool("Exosuit_Plasma")->nId);
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetExoPlasmaAmmo
//
//	PURPOSE:	Sets the amount of Exosuit plasma ammo
//
// ----------------------------------------------------------------------- //

void CCharacter::SetExoPlasmaAmmo(int nAmount)
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Plasma"); if(!pPool) return;

				pWeapons->SetAmmo(pPool->nId, nAmount);

				if(IsPlayer(m_hObject))
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_INFOCHANGE);
					g_pLTServer->WriteToMessageByte(hMessage, IC_AMMO_ID);
					g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
					g_pLTServer->WriteToMessageFloat(hMessage, nAmount==-1? pPool->GetMaxAmount(m_hObject): (LTFLOAT)nAmount);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetExoRocketAmmo
//
//	PURPOSE:	Returns the amount of exosuit rocket ammo
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::GetExoRocketAmmo()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				return pWeapons->GetAmmoPoolCount(g_pWeaponMgr->GetAmmoPool("Exosuit_Rockets")->nId);
			}
		}
	}
	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetExoRocketAmmo
//
//	PURPOSE:	Sets the amount of Exosuit rocket ammo
//
// ----------------------------------------------------------------------- //

void CCharacter::SetExoRocketAmmo(int nAmount)
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	nAmount = nAmount==-1? 10: nAmount;

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Rockets"); if(!pPool) return;

				pWeapons->SetAmmo(pPool->nId, nAmount);

				if(IsPlayer(m_hObject))
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PLAYER_INFOCHANGE);
					g_pLTServer->WriteToMessageByte(hMessage, IC_AMMO_ID);
					g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
					g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)nAmount);
					g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::AddSpear
//
//	PURPOSE:	Stick a spear into us
//
// ----------------------------------------------------------------------- //

void CCharacter::AddSpear(HOBJECT hObject, HOBJECT hSpear, ModelNode eModelNode, const LTRotation& rRot)
{
	if ( m_cSpears < kMaxSpears )
	{
		ModelSkeleton nSkele = (ModelSkeleton)m_cmMovement.GetCharacterButes()->m_nSkeleton;
		
		if(nSkele == eModelSkeletonInvalid)
		{
			g_pLTServer->RemoveObject(hSpear);
			return;			
		}

		// Get the node transform because we need to make rotation relative

		LTVector vPos;
		LTRotation rRotNode;
		if(GetNodePosition(eModelNode, vPos, nSkele, &rRotNode))
		{
			LTRotation rAttachment = ~rRotNode*rRot;
			LTVector vAttachment, vNull;
			g_pMathLT->GetRotationVectors(rAttachment, vNull, vNull, vAttachment);
			vAttachment *= -10.0f;

			HATTACHMENT hAttachment;

			char* szNode = (char *)g_pModelButeMgr->GetSkeletonNodeName(nSkele, eModelNode);

			if ( LT_OK == g_pLTServer->CreateAttachment(hObject, hSpear, szNode, &vAttachment, &rAttachment, &hAttachment) )
			{
				m_aSpears[m_cSpears].hObject = hSpear;
				m_aSpears[m_cSpears].eModelNode = eModelNode;
				m_aSpears[m_cSpears].rRot = rRot;

				//now that we are attached it's okay to be non-solid
				uint32 dwFlags =  g_pLTServer->GetObjectFlags(hSpear);
				g_pLTServer->SetObjectFlags(hSpear, dwFlags & ~FLAG_SOLID);

				//make sure we show up...
				uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(hSpear);
				g_pLTServer->SetObjectUserFlags(hSpear, dwUsrFlags | USRFLG_SPECIAL_ATTACHMENT);

				m_cSpears++;

				return;
			}
		}
	}

	// Unless we actually stuck the spear into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hSpear);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::AddNet
//
//	PURPOSE:	Stick a net into us
//
// ----------------------------------------------------------------------- //

void CCharacter::AttachNet(HOBJECT hNet)
{
	uint32 nSkele = m_cmMovement.GetCharacterButes()->m_nSkeleton;
	
	if(nSkele == eModelSkeletonInvalid)
	{
		g_pLTServer->RemoveObject(hNet);
		return;			
	}

	// ----------------------------------------------------------------------- //
	ModelNode eTorsoNode;
	LTVector vPos;
	LTRotation rRotNode;
	char* szNode = "net_socket";

	// Find the net socket node...
	eTorsoNode = g_pModelButeMgr->GetSkeletonNode((ModelSkeleton)nSkele, szNode);

	if(!GetNodePosition(eTorsoNode, vPos, (ModelSkeleton)nSkele, &rRotNode))
	{
		//  hum... try the torso node...
		szNode = "Torso_u_node";
		eTorsoNode = g_pModelButeMgr->GetSkeletonNode((ModelSkeleton)nSkele, szNode);
		if(!GetNodePosition(eTorsoNode, vPos, (ModelSkeleton)nSkele, &rRotNode))
		{
			// wow, big problems with netting this model...
			g_pLTServer->RemoveObject(hNet);
			return;			
		}
	}
	// ----------------------------------------------------------------------- //

	// Go ahead and make the attachment.
	HATTACHMENT hAttachment;
	LTRotation rRot, rAttachment;

	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	rAttachment = ~rRotNode*rRot;

	g_pLTServer->CreateAttachment(m_hObject, hNet, szNode, &LTVector(0,0,0), &rAttachment, &hAttachment);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferSpears
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

void CCharacter::TransferSpears(BodyProp* pBody)
{
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		HATTACHMENT hAttachment;
		HOBJECT hSpear = m_aSpears[iSpear].hObject;
		if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
		{
			if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
			{
				// Attach it to the body prop

				pBody->AddSpear(hSpear, m_aSpears[iSpear].rRot, m_aSpears[iSpear].eModelNode);
                continue;
			}
		}

		// If any of this failed, just remove the spear

		g_pLTServer->RemoveObject(hSpear);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetAimRotation
//
//	PURPOSE:	Returns the forward aiming direction.
//
// ----------------------------------------------------------------------- //

LTRotation CCharacter::GetAimRotation() const
{
	LTRotation aim_rotation = m_cmMovement.GetRot();

	g_pLTServer->GetMathLT()->RotateAroundAxis( aim_rotation, const_cast<LTVector&>(GetUp()),
		MATH_DEGREES_TO_RADIANS(m_cmMovement.GetCharacterVars()->m_fAimingYaw) );

	g_pLTServer->GetMathLT()->RotateAroundAxis( aim_rotation, const_cast<LTVector&>(GetRight()),
		MATH_DEGREES_TO_RADIANS(m_cmMovement.GetCharacterVars()->m_fAimingPitch) );

	return aim_rotation;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetAimForward
//
//	PURPOSE:	Returns the forward aiming direction.
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::GetAimForward() const
{
	LTVector vAimForward, vR, vU;
	LTRotation rRot = GetAimRotation();
	g_pLTServer->GetMathLT()->GetRotationVectors(rRot,vR,vU,vAimForward);

	return vAimForward;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetAimUp
//
//	PURPOSE:	Returns the up aiming direction.
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::GetAimUp() const
{
	LTVector vAimUp, vR, vF;
	LTRotation rRot = GetAimRotation();
	g_pLTServer->GetMathLT()->GetRotationVectors(rRot,vR,vAimUp,vF);

	return vAimUp;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferNet
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //
/*
void CCharacter::TransferNet(BodyProp* pBody)
{
	if(m_NetData.hObject)
	{
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, m_NetData.hObject, &hAttachment) )
		{
			if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
			{
				// Attach it to the body prop

				pBody->AddNet(m_NetData.hObject, m_NetData.rRot, m_NetData.eModelNode);
			}
		}
	}
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveNet
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

void CCharacter::RemoveNet()
{
	if(m_hNet)
	{
		// remove the net
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hNet, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hNet);
		m_hNet = LTNULL;
	}

	// If we are a player then tell the client to get on it...
	if(IsPlayer(m_hObject))
	{
		HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PHYSICS_UPDATE);
		if(!hWrite) return;
		
		// Write the state flags to the message first, so we know what to read out on the other side
		g_pLTServer->WriteToMessageWord(hWrite, (uint16)PSTATE_REMOVE_NET_FX);
		g_pLTServer->EndMessage2(hWrite, MESSAGE_GUARANTEED);
	}
	else
	{
		// Let us move again
		m_cmMovement.AllowMovement(LTTRUE);
		m_cmMovement.AllowRotation(LTTRUE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleNetSlash
//
//	PURPOSE:	Handle slashing at our net
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleNetSlash(uint32 nDamage)
{
	if(m_hNet)
	{
		// Add to the net damage
		m_nNetDamage += nDamage;

		if(m_nNetDamage > (uint32)g_cvarNetDamage.GetFloat())
			HandleNetEscape();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetMaxChargeLevel
//
//	PURPOSE:	Handle setting the max charge level for the Predator
//				shoulder cannon.
//
// ----------------------------------------------------------------------- //

void CCharacter::SetMaxChargeLevel()
{
	int i = 0;

	//start fresh
	m_nMaxChargeLevel=0;

	WEAPON* pWep = g_pWeaponMgr->GetWeapon("Shoulder_Cannon");

	if(!pWep) return;

	BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWep->aBarrelTypes[PRIMARY_BARREL]);

	for(i=0; pBarrel ; i++)
	{
		pBarrel = g_pWeaponMgr->GetBarrel(pBarrel->szNextBarrel);
	}

	m_nMaxChargeLevel = i;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateTargetingSprite
//
//	PURPOSE:	Handle updating our targeting sprite object
//
// ----------------------------------------------------------------------- //
/*
void CCharacter::UpdateTargetingSprite(LTBOOL bOn, LTVector vPos, LTVector vNorm, LTVector vCamF)
{
	//see if we have a targeting sprite to begin with...
	if(!m_pTargetingSprite) return;

	//if we are not on hide us and be done with it...
	if(!bOn)
	{
        uint32 dwFlags = g_pLTServer->GetObjectFlags(m_pTargetingSprite->m_hObject);
        g_pLTServer->SetObjectFlags(m_pTargetingSprite->m_hObject, dwFlags & ~FLAG_VISIBLE);
		return;
	}

	//ok this is a full update make us visible and set our position and rotation...

	//make visible
    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_pTargetingSprite->m_hObject);
    g_pLTServer->SetObjectFlags(m_pTargetingSprite->m_hObject, dwFlags | FLAG_VISIBLE);

	//set rotation
	LTRotation rRot;
	LTVector vU;
	if(vNorm.y)
	{
		if(vNorm.y == 1.0f || vNorm.y == -1.0f)
		{
			vU = vNorm.Cross(vNorm.y>0?vCamF:-vCamF);
		}
		else
		{
			vU = vNorm.Cross(LTVector(0,1,0));
		}
//		vU.Norm();
		vU = vU.Cross(vNorm);
//		vU.Norm();
	}
	else
		vU = LTVector(0,1,0);

	g_pLTServer->AlignRotation(&rRot, &vNorm, &vU);
//	g_pLTServer->RotateAroundAxis(&rRot, &vNorm, MATH_HALFPI);
	g_pLTServer->SetObjectRotation(m_pTargetingSprite->m_hObject, &rRot);

	//set position
	g_pLTServer->SetObjectPos(m_pTargetingSprite->m_hObject, &vPos);
}
*/
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetTargetingSprite
//
//	PURPOSE:	Send special FX message
//
// ----------------------------------------------------------------------- //
void CCharacter::SetTargetingSprite(LTBOOL bOn)
{
	//send the special effects message to turn on or off the targeting
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_TARGETING_SPRITE);
	g_pLTServer->WriteToMessageByte(hMessage, bOn);
	g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetTargetingSprite
//
//	PURPOSE:	Send special FX message
//
// ----------------------------------------------------------------------- //
void CCharacter::SetFlameThrowerFX(LTBOOL bOn)
{
	//send the special effects message to turn on or off the targeting
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_FLAME_FX);
	g_pLTServer->WriteToMessageByte(hMessage, bOn);
	g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::InitTargetingSprite
//
//	PURPOSE:	Create our targeting sprite
//
// ----------------------------------------------------------------------- //
/*
void CCharacter::InitTargetingSprite(char* szSprite, LTVector vScale)
{
	//first check to see if we already have a targeting sprite
	if(m_pTargetingSprite)
	{
		//remove the old one
		g_pLTServer->RemoveObject(m_pTargetingSprite->m_hObject);
		m_pTargetingSprite = LTNULL;
	}

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	SAFE_STRCPY(createStruct.m_Filename, szSprite);
	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_ROTATEABLESPRITE;
	createStruct.m_Pos = LTVector(0,0,0);

	HCLASS hClass = g_pLTServer->GetClass("CTargetingSprite");

	m_pTargetingSprite = (CTargetingSprite*) g_pLTServer->CreateObject(hClass, &createStruct);

	if (!m_pTargetingSprite) return;

	g_pLTServer->ScaleObject(m_pTargetingSprite->m_hObject, &vScale);
}
*/
 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetFlashlight
//
//	PURPOSE:	Set the flashlight on or off
//
// ----------------------------------------------------------------------- //

void CCharacter::SetFlashlight(LTBOOL bOn)
{
	if (bOn)
	{
		// Verify that the Character actually has a shoulderlamp to turn on. (AI Only)
		if(IsPlayer(m_hObject))
		{
			m_bFlashlight = LTTRUE; 
		}
		else
		{
			CAttachmentPosition *pAP = GetAttachments()->GetAttachmentPosition("LeftShoulder");
			if (pAP)
			{
				const char *pName = pAP->GetAttachmentName().c_str();
				if (strcmp(pName, "ShoulderLamp") == 0)
				{
					g_pLTServer->SetObjectUserFlags(m_hObject, g_pLTServer->GetObjectUserFlags(m_hObject) | USRFLG_AI_FLASHLIGHT);
					m_bFlashlight = LTTRUE; 
				}
			}
		}
	}
	else
	{
		g_pLTServer->SetObjectUserFlags(m_hObject, g_pLTServer->GetObjectUserFlags(m_hObject) & ~USRFLG_AI_FLASHLIGHT);
		m_bFlashlight = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveTargetingSprite
//
//	PURPOSE:	Remove oir TargetingSprite
//
// ----------------------------------------------------------------------- //
/*
void CCharacter::RemoveTargetingSprite()
{
	if(m_pTargetingSprite)
	{
		g_pLTServer->RemoveObject(m_pTargetingSprite->m_hObject);
		m_pTargetingSprite = LTNULL;
	}
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleEnergySift
//
//	PURPOSE:	Handle updating the pred energy
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleEnergySift()
{
	AMMO*	pAmmo	= g_pWeaponMgr->GetAmmo("Energy_Sift_Ammo");
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel("Energy_Sift_Barrel");
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon("Energy_Sift");

	if(!pAmmo || !pBarrel || !pWeapon) return;

	// Restore some energy...
	DecrementPredatorEnergy(-(pAmmo->nAmmoPerShot));

	// now do the FX
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(GetAttachments());

	if(!pAttach) return;

	CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

	if(!pAttachedWeapon) return;

	CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

	if(!pWeapons) return;

	CWeapon* pCurWeapon = pWeapons->GetCurWeapon();

	if(!pCurWeapon) return;

	{
		// Play the effect on all the clients
		LTVector vNULL(0,0,0);

		CLIENTWEAPONFX fxStruct;

		fxStruct.hFiredFrom		= m_hObject;
		fxStruct.vSurfaceNormal	= vNULL;
		fxStruct.vFirePos		= m_cmMovement.GetPos();
		fxStruct.vPos			= m_cmMovement.GetPos();
		fxStruct.hObj			= LTNULL;
		fxStruct.nWeaponId		= pWeapon->nId;
		fxStruct.nAmmoId		= pAmmo->nId;
		fxStruct.nBarrelId		= pBarrel->nId;
		fxStruct.nSurfaceType	= ST_UNKNOWN;
		fxStruct.nFlashSocket	= pCurWeapon->GetCurrentFlashSocket();;
		fxStruct.wIgnoreFX		= WFX_MUZZLEONLY;

		// If this is a player object, get the client id...

		if (IsPlayer(m_hObject))
		{
			CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(m_hObject));
			if (pPlayer)
			{
				fxStruct.nShooterId	= (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
			}
		}

		CreateClientWeaponFX(fxStruct);
	}

	// Turn off cloaking
	ForceCloakOff(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleMedicomp
//
//	PURPOSE:	Handle a medicomp charge
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleMedicomp()
{
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo("Medicomp_Ammo");

	if(pAmmo)
	{
		//looks good so go ahead and heal up
		DecrementPredatorEnergy(pAmmo->nAmmoPerShot);
		m_damage.Heal(m_damage.GetMaxHitPoints());

		//turn off cloaking
		ForceCloakOff(LTTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleExosuitEntry
//
//	PURPOSE:	Handle getting into the exosuit
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleExosuitEntry(HOBJECT hSender)
{
	if(!hSender)
		return;

	// this is really only for players...
	CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(this);
	if(!pPlayer)
		return;

	if(m_damage.IsDead())
		return;

	m_bResetAttachments = LTFALSE;
	PickupObject* pPU = dynamic_cast<PickupObject*>(g_pLTServer->HandleToObject(hSender));

	//save our old character set, health and armor and weapon
	m_nOldCharacterSet  = m_nCharacterSet;
	m_fOldHealth		= m_damage.GetHitPoints();
	m_fOldArmor			= m_damage.GetArmorPoints();

	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(GetAttachments());
	if(!pAttach) return;
	CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());
	if(!pAttachedWeapon) return;
	CWeapons* pWeapons = pAttachedWeapon->GetWeapons();
	if(!pWeapons) return;
	CWeapon* pWeapon = pWeapons->GetCurWeapon();
	if(!pWeapon) return;

	m_pOldWeapon = const_cast<WEAPON*>(pWeapon->GetWeaponData());

	// This stuff really deserves its own function in CAI, but since it's small
	// it will be left here for now.
	SetFlashlight(LTFALSE);

	//now change to the Exosuit
	int nCharId = g_pCharacterButeMgr->GetSetFromModelType("Exosuit");
	CCharacter::SetCharacter(nCharId, LTTRUE);

	// Make sure we always have and start with the default weapon...
	int nWeapon = pPlayer->AquireDefaultWeapon(LTFALSE);

	//set position and rotation
	LTRotation rRot;
	LTVector vR, vU, vF;

	g_pLTServer->GetObjectRotation(hSender, &rRot);
	g_pMathLT->GetRotationVectors(rRot,vR,vU,vF);
	g_pLTServer->AlignRotation(&rRot, &vF, LTNULL);

	LTVector vVec;
	g_pMathLT->GetEulerAngles(rRot, vVec);

	uint8 nMoveCode = pPlayer->IncMoveCode();

	LTVector vPos;
	g_pLTServer->GetObjectPos(hSender, &vPos);

	// Setup the object flags for teleporting to the location
	uint32 dwFlags;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | FLAG_GOTHRUWORLD);

	g_pLTServer->TeleportObject(m_hObject, &vPos);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags & ~FLAG_GOTHRUWORLD);

	// Make sure that anyone else that's at this location gets punished!
	pPlayer->TeleFragObjects(vPos);

	// Update their view position so they get the sfx message.
	g_pLTServer->SetClientViewPos(pPlayer->GetClient(), &vPos);

	if(pPU && pPU->m_pUserData)
	{
		GetExoPUData(pPU);
	}
	else
	{
		//set our armor and health to initial values
		m_damage.SetHitPoints(pPU->GetHitPoints());
		m_damage.SetMaxHitPoints(g_pCharacterButeMgr->GetMaxHitPoints(m_nCharacterSet)*MP_EXOSUIT_HP_MULTIPLIER);
		m_damage.SetMaxArmorPoints(0);
		m_damage.SetArmorPoints(0);

		//set ammo values to initial values
		if( g_pGameServerShell->GetGameType() != SP_STORY)
			SetExoInitialAmmo();

		//be sure to reset the energy
		ResetExosuitEnergy();
	}

	// Make sure we don't have any wacky dims
	LTVector vStandDims = m_cmMovement.GetDimsSetting(CM_DIMS_DEFAULT);
	g_pLTServer->SetObjectDims(m_hObject, &vStandDims);

	// Now do the MP respawn thing...
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(pPlayer->GetClient(), MID_MP_EXO_RESPAWN);
	// Character ID
	g_pLTServer->WriteToMessageByte(hMessage, (uint8)GetCharacter());
	// Move code
	g_pLTServer->WriteToMessageByte(hMessage, nMoveCode);
	// Player position
	g_pLTServer->WriteToMessageVector(hMessage, &vPos);
	// Player orientation
	g_pLTServer->WriteToMessageVector(hMessage, &vVec);
	// This is our new weapon
	g_pLTServer->WriteToMessageByte(hMessage, (uint8)g_pWeaponMgr->GetCommandId(nWeapon));
	// Send the new interface stuff
	pPlayer->SendMPInterfaceUpdate(hMessage);
	// And away we go...
	g_pLTServer->EndMessage(hMessage);

	// record the spawn point info, only needed in MP...
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		PickupObject* pPUObject = dynamic_cast<PickupObject*>(g_pLTServer->HandleToObject(hSender));

		if(pPUObject)
		{
			m_hStartpointSpawner = pPUObject->GetStartpointObject();
		}
	}

	//play the entry sound
	g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "GetInExo");

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleExosuitEject
//
//	PURPOSE:	Handle getting out of the exosuit
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::HandleExosuitEject(LTVector &vNewPos, LTBOOL bCreatePickup)
{
	// this is really only for players...
	CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(this);
	if(!pPlayer)
		return LTFALSE;

	// bail out if the player was killed prior to ejecting
	if(m_damage.IsDead())
		return LTFALSE;

	m_bResetAttachments = LTFALSE;
	LTBOOL rVal = LTFALSE;

	LTVector vF		= m_cmMovement.GetForward();
	LTVector vPos	= m_cmMovement.GetPos();

	LTVector vStandDims = m_cmMovement.GetDimsSetting(CM_DIMS_DEFAULT);

	LTVector vDims;
	g_pLTServer->GetObjectDims(m_hObject, &vDims);

	if(!(vDims.Equals(vStandDims, 1.0)))
		return LTFALSE;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, vPos - vF*vDims.z*2.1f);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	
	if (!g_pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		LTVector vTestDims = vDims;

		vPos -= vF*vDims.z*2.1f;

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		theStruct.m_ObjectType	= OT_NORMAL;
		theStruct.m_Flags		= FLAG_SOLID;
		theStruct.m_Pos			= vPos;

//		11/21/2000 - MarkS - removed hardcoded dependency (see Lambert)
//		SAFE_STRCPY(theStruct.m_Filename, "avp3\\models\\1x1_square.abc");
//		SAFE_STRCPY(theStruct.m_SkinName, "avp3\\skins\\1x1_square.dtx");

#ifdef _DEBUG
		sprintf(theStruct.m_Name, "%s-EjectCheck", g_pLTServer->GetObjectName(m_hObject) );
#endif

#ifdef _WIN32
		SAFE_STRCPY(theStruct.m_Filename, "models\\1x1_square.abc");
		SAFE_STRCPY(theStruct.m_SkinName, "skins\\1x1_square.dtx");
#else
		SAFE_STRCPY(theStruct.m_Filename, "Models/1x1_square.abc");
		SAFE_STRCPY(theStruct.m_SkinName, "Skins/1x1_square.dtx");
#endif

		HCLASS hClass = g_pServerDE->GetClass("BaseClass");
		BaseClass* pClass = g_pLTServer->CreateObject(hClass, &theStruct);

		g_pLTServer->SetObjectDims(pClass->m_hObject, &vDims);

		if(vDims.Equals(vTestDims, 1.0))
		{
			LTVector vLoc;
			g_pLTServer->GetObjectPos(pClass->m_hObject, &vLoc);

			if(vPos.Equals(vLoc, 1.0))
			{
				//Go back to our normal character set
				LTFLOAT nHitPoints = m_damage.GetHitPoints();
				CCharacter::SetCharacter(m_nOldCharacterSet, LTTRUE);
				vNewPos = vPos;
				rVal = LTTRUE;

				// get our alignment vector
				LTRotation rRot = m_cmMovement.GetRot();
				LTVector vVec;
				g_pMathLT->GetEulerAngles(rRot, vVec);


				if(bCreatePickup)
				{
					//spawn a new powerup
					char pStr[255];
					sprintf(pStr,"PickupObject Pickup Exosuit_Device");
					PickupObject* pExoClass = dynamic_cast<PickupObject*>(SpawnObject( pStr, m_cmMovement.GetPos(), rRot));

					if(pExoClass)
					{
						// Be sure we don't rotate...
						pExoClass->SetAllowMovement(LTFALSE);
						pExoClass->SetAllowRotation(LTFALSE);

						// Record the start point info...
						pExoClass->SetStartpointObject(m_hStartpointSpawner);
						m_hStartpointSpawner = LTNULL;

						// Remember our stats...
						SetExoPUData(pExoClass, nHitPoints);
					}
				}

				//re-set our health and armor
				if(m_fOldHealth > 0.0f)
				{
					//we have good old data
					m_damage.SetHitPoints(m_fOldHealth);
					m_damage.SetMaxHitPoints(g_pCharacterButeMgr->GetMaxHitPoints(m_nCharacterSet));
					m_damage.SetArmorPoints(m_fOldArmor);
					m_damage.SetMaxArmorPoints(g_pCharacterButeMgr->GetMaxArmorPoints(m_nCharacterSet));
				}
				else
				{
					//must have started as an exosuit, load up fresh data
					m_damage.SetHitPoints(g_pCharacterButeMgr->GetMaxHitPoints(m_nCharacterSet));
					m_damage.SetArmorPoints(g_pCharacterButeMgr->GetMaxArmorPoints(m_nCharacterSet));
				}

				// Setup the object flags for teleporting to the location
				uint32 dwFlags;
				g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
				g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | FLAG_GOTHRUWORLD);

				g_pLTServer->TeleportObject(m_hObject, &vPos);
				g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

				g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags & ~FLAG_GOTHRUWORLD);

				// Update their view position so they get the sfx message.
				g_pLTServer->SetClientViewPos(pPlayer->GetClient(), &vPos);

				// Get thenew move code
				uint8 nMoveCode = pPlayer->IncMoveCode();

				//be sure we have our weapon
				int nWeapon = 0;

				if(m_pOldWeapon && !NeedsWeapon(m_pOldWeapon))
				{
					pPlayer->ChangeWeapon(m_pOldWeapon->szName, LTFALSE);

					uint32 nWeaponSet;
					if(g_pGameServerShell->GetGameInfo()->m_bClassWeapons)
						nWeaponSet = g_pCharacterButeMgr->GetMPClassWeaponSet(GetCharacter());
					else
						nWeaponSet = g_pCharacterButeMgr->GetMPWeaponSet(GetCharacter());

					WEAPON_SET *pWeaponSet = g_pWeaponMgr->GetWeaponSet(nWeaponSet);

					if(pWeaponSet)
					{
						int i;
						for(i=0 ; i<pWeaponSet->nNumWeapons ; i++)
						{
							WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeaponSet->szWeaponName[i]);
							if (pWeaponData && pWeaponData->nId == m_pOldWeapon->nId)
							{
								nWeapon = i;
							}
						}
					}
				}
				else
				{
					nWeapon = pPlayer->AquireDefaultWeapon(LTFALSE);
				}

				// Now set this to null for the cheaters..
				m_pOldWeapon = LTNULL;


				// Now do the MP respawn thing...
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(pPlayer->GetClient(), MID_MP_EXO_RESPAWN);
				// Character ID
				g_pLTServer->WriteToMessageByte(hMessage, (uint8)GetCharacter());
				// Move code
				g_pLTServer->WriteToMessageByte(hMessage, nMoveCode);
				// Player position
				g_pLTServer->WriteToMessageVector(hMessage, &vPos);
				// Player orientation
				g_pLTServer->WriteToMessageVector(hMessage, &vVec);
				// This is our new weapon
				g_pLTServer->WriteToMessageByte(hMessage, (uint8)g_pWeaponMgr->GetCommandId(nWeapon));
				// Send the new interface stuff
				pPlayer->SendMPInterfaceUpdate(hMessage);
				// And away we go...
				g_pLTServer->EndMessage(hMessage);

				// remove the head attachemnt
				HATTACHMENT hAttachment;
				if (g_pServerDE->FindAttachment(m_hObject, m_hHead, &hAttachment) == DE_OK)
				{
					g_pServerDE->RemoveAttachment(hAttachment);
				}
				g_pServerDE->RemoveObject(m_hHead);
				m_hHead = LTNULL;

				//play the exit sound
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "GetOutExo");
			}
		}

		g_pLTServer->RemoveObject(pClass->m_hObject);
	}

	return rVal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetExoPUData
//
//	PURPOSE:	Set the save data for the PU
//
// ----------------------------------------------------------------------- //

void CCharacter::SetExoPUData(PickupObject* pPickUp, LTFLOAT fHPs)
{
	//shouldn't neet this but just in case
	
	// gcc won't allow a (void *) deletion
	if(pPickUp->m_pUserData)
		delete pPickUp->m_pUserData;

	pPickUp->m_pUserData = new ExosuitData;

	ExosuitData* pData = pPickUp->m_pUserData;

	if(pData)
	{
		pData->nBulletAmmo	= GetExoBulletAmmo();
		pData->nFlamerAmmo	= GetExoFlamerAmmo();
		pData->nPlasmaAmmo	= GetExoPlasmaAmmo();
		pData->nRocketAmmo	= GetExoRocketAmmo();
		pData->nEnergy		= GetExosuitEnergy();
		pData->bDisabled	= m_bExosuitDisabled;
	}
	//finally set the hps
	pPickUp->SetHitpoints(fHPs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetExoPUData
//
//	PURPOSE:	Gets the save data for the PU
//
// ----------------------------------------------------------------------- //

void CCharacter::GetExoPUData(PickupObject* pPickUp)
{
	if(!pPickUp) return;

	ExosuitData* pData = pPickUp->m_pUserData;

	if(pData)
	{
		//set the ammo amounts
		SetExoBulletAmmo(pData->nBulletAmmo);
		SetExoFlamerAmmo(pData->nFlamerAmmo);
		SetExoPlasmaAmmo(pData->nPlasmaAmmo);
		SetExoRocketAmmo(pData->nRocketAmmo);
		SetExosuitEnergy(pData->nEnergy);
		m_bExosuitDisabled = pData->bDisabled;
	}

	//finally set the hps and re-set the armor
	m_damage.SetHitPoints(pPickUp->GetHitPoints());
	m_damage.SetMaxHitPoints(g_pCharacterButeMgr->GetMaxHitPoints(m_nCharacterSet)*MP_EXOSUIT_HP_MULTIPLIER);
	m_damage.SetArmorPoints(0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetExoInitialAmmo
//
//	PURPOSE:	Set all values to max...
//
// ----------------------------------------------------------------------- //

void CCharacter::SetExoInitialAmmo()
{
	//set the ammo amounts
	SetExoBulletAmmo(-1);
	SetExoFlamerAmmo(-1);
	SetExoPlasmaAmmo(-1);
	SetExoRocketAmmo(-1);
	SetExosuitEnergy(-1);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateBatteryLevel
//
//	PURPOSE:	Update the battery system
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateBatteryLevel()
{
	if(g_cvarBattSystem.GetFloat() == 1.0f)
	{
		if(!m_bFlashlight && !m_bNightvision)
		{
			// recharge time
			if(m_fBatteryChargeLevel == MARINE_MAX_BATTERY_LEVEL) return;

			m_fBatteryChargeLevel += (GetUpdateDelta() * g_cvarBattRechargeRate.GetFloat());

			// test for bounds
			if(m_fBatteryChargeLevel > MARINE_MAX_BATTERY_LEVEL)
				m_fBatteryChargeLevel = MARINE_MAX_BATTERY_LEVEL;

			return;
		}

		if(m_bFlashlight)
		{
			// use some battery
			if( g_pGameServerShell->GetGameType() == SP_STORY)
				m_fBatteryChargeLevel -= (GetUpdateDelta() * g_cvarBattFLUseRate.GetFloat());
			else
				m_fBatteryChargeLevel -= (GetUpdateDelta() * g_cvarBattFLUseRate.GetFloat() * g_cvarMPUseMod.GetFloat());
		}

		if(m_bNightvision)
		{
			// use some battery
			if( g_pGameServerShell->GetGameType() == SP_STORY)
				m_fBatteryChargeLevel -= (GetUpdateDelta() * g_cvarBattNVUseRate.GetFloat());
			else
				m_fBatteryChargeLevel -= (GetUpdateDelta() * g_cvarBattNVUseRate.GetFloat() * g_cvarMPUseMod.GetFloat());

		}

		// test for bounds
		if(m_fBatteryChargeLevel < 0.0f)
			m_fBatteryChargeLevel = 0.0f;
	}
	else
	{
		//system is disabled
		m_fBatteryChargeLevel = MARINE_MAX_BATTERY_LEVEL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CheckMissionFailed()
//
//	PURPOSE:	Send a message to the client if the player killed his own ally
//
// ----------------------------------------------------------------------- //

void CCharacter::CheckMissionFailed(HOBJECT hDamager)
{
	// Do nothing if the dying character is a player
	if (IsPlayer(m_hObject))
		return;

	if (!IsPlayer(hDamager))
		return;

	// Don't do this for Aliens... sheez. They could care less who they kill.
	if (IsAlien(hDamager))
		return;

	CCharacter * pChar = dynamic_cast<CCharacter *>(g_pInterface->HandleToObject(hDamager));
	if (pChar)
	{
		if (g_pCharacterMgr->AreAllies(*pChar, *this))
		{
			CharacterClass cVictim	= pChar->GetCharacterClass();
			CharacterClass cLocal	= GetCharacterClass();

			// See if we are incognito...
			cLocal = cLocal == CORPORATE?MARINE:cLocal;

			// If we killed our own kind then you know what you have to do!
			if(cLocal == cVictim)
			{
				HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_MISSION_FAILED);
				g_pInterface->EndMessage(hMsg);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SpewAcidDamage()
//
//	PURPOSE:	Make an acid damage area
//
// ----------------------------------------------------------------------- //

void CCharacter::SpewAcidDamage(LTFLOAT fDamage)
{
	if( g_pGameServerShell->GetGameType() == SP_STORY && IsPlayer(m_hObject) )
		return;
	
	HCLASS hClass = g_pLTServer->GetClass("Explosion");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-AcidExplosion", g_pLTServer->GetObjectName(m_hObject) );
#endif

	Explosion* pExplosion = (Explosion*)g_pLTServer->CreateObject(hClass, &theStruct);

	if (pExplosion)
	{
		EXPLOSION_CREATESTRUCT expCS;

		LTFLOAT fDamageMod = GetAcidDamageMod();

		if(g_pGameServerShell->GetGameType().IsMultiplayer())
			expCS.bHideFromShooter	= LTTRUE;
		else
			expCS.bHideFromShooter	= LTFALSE;

		expCS.bRemoveWhenDone	= LTTRUE;
		expCS.eDamageType		= DT_ACID;
		expCS.fDamageRadius		= g_cvarAcidHitRad.GetFloat();
		expCS.fMaxDamage		= fDamage * fDamageMod * GetDifficultyFactor();
		expCS.hFiredFrom		= m_hObject;
		expCS.nImpactFXId		= FXBMGR_INVALID_ID;
		expCS.vPos				= m_cmMovement.GetPos();

		pExplosion->Setup(expCS);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetAcidDamageMod()
//
//	PURPOSE:	Make an acid damage area
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacter::GetAcidDamageMod()
{
	if( g_pGameServerShell->GetGameType() == SP_STORY )
	{
		CPlayerObj * pPlayer;

		for( CCharacterMgr::PlayerIterator iter = g_pCharacterMgr->BeginPlayers();
			 iter != g_pCharacterMgr->EndPlayers(); ++iter )
		{
			if(*iter)
			{
				pPlayer = *iter;

				if(IsPredator(pPlayer->m_hObject))
					return g_cvarPredAcidHitRatio.GetFloat();
				else
					return g_cvarOtherAcidHitRatio.GetFloat();

			}
		}
	}

	return g_cvarMPAcidHitRatio.GetFloat();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleAlienDamage()
//
//	PURPOSE:	Gib parts if it's time...
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleAlienDamage()
{
	if(g_pGameServerShell->LowViolence())
		return;


	// Don't do any more of this if I am a player
	if(!IsPlayer(m_hObject))
	{
		ModelSkeleton eModelSkele = GetModelSkeleton();

		if(m_eModelNodeLastHit != eModelNodeInvalid)
		{
			if(g_pModelButeMgr->GetSkeletonNodeCanLiveExplode(eModelSkele, m_eModelNodeLastHit))
			{
				LTFLOAT fHistDamage = m_damage.GetHistoryDamageAmount(g_cvarDamageHistoryTime.GetFloat(), (uint32)m_eModelNodeLastHit);
				LTFLOAT fFactor = g_pModelButeMgr->GetSkeletonNodeDamageFactor(eModelSkele, m_eModelNodeLastHit);

				fHistDamage /= fFactor;

				if(g_pModelButeMgr->ExplodeNode(eModelSkele, m_eModelNodeLastHit, fHistDamage))
				{
					ExplodeLimb(m_eModelNodeLastHit, eModelSkele);

					if( !m_damage.IsDead() )
					{
						if(g_pModelButeMgr->GetSkeletonNodeForceCrawl(eModelSkele, m_eModelNodeLastHit))
						{
							m_bForceCrawl = LTTRUE;
							m_cmMovement.SetEncumbered(LTTRUE);
							HandleInjured();
						}
					}

					// Tell the clients to create a blood spray!
					ModelNode eNode = g_pModelButeMgr->GetSkeletonNodeBleeder(eModelSkele, m_eModelNodeLastHit);

					if(eNode != eModelNodeInvalid)
					{
						HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
						g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
						g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
						g_pLTServer->WriteToMessageByte(hMessage, CFX_BLEEDER);
						g_pLTServer->WriteToMessageWord(hMessage, eNode);
						g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ExplodeLimb()
//
//	PURPOSE:	Explode a limb
//
// ----------------------------------------------------------------------- //

void CCharacter::ExplodeLimb(ModelNode eParentNode, ModelSkeleton eModelSkele)
{
	int i = 0;

	if(eParentNode == eModelNodeInvalid) return;

	// Get the name of the piece so we can make sure to not allow it on
	// the other limbs
	const char* pExplPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkele, eParentNode);
	if(pExplPiece[0] == '\0') return;

	HMODELPIECE hPiece = INVALID_MODEL_PIECE;

	// Make the piece invisible
	if(g_pModelLT->GetPiece(m_hObject, const_cast<char*>(pExplPiece), hPiece) == LT_OK)
	{
		// First get the hide status... if it's already invisible, just get out of here.
		LTBOOL bHidden;
		g_pModelLT->GetPieceHideStatus(m_hObject, hPiece, bHidden);
		if(bHidden) return;
	}


	// Start a stack to go through multiple limb branches
	ModelNode pNodeStack[32];
	int nStackAmount = -1;

	ModelNode *pChildren = g_pModelButeMgr->GetSkeletonNodeChildren(eModelSkele, eParentNode);
	int nChildren = g_pModelButeMgr->GetSkeletonNodeNumChildren(eModelSkele, eParentNode);

	for(i = 0; i < nChildren; i++)
	{
		++nStackAmount;
		_ASSERT(nStackAmount < 32);

		pNodeStack[nStackAmount] = pChildren[i];
	}


	// Find more children if we can, and create limbs for any appropriate branches
	ModelNode mnTemp = eModelNodeInvalid;
	const char* pPiece = LTNULL;

	while(nStackAmount > -1)
	{
		// Get the current model node to check
		mnTemp = pNodeStack[nStackAmount];
		--nStackAmount;

		// Get the name of the piece associated with this node
		pPiece = g_pModelButeMgr->GetSkeletonNodePiece(eModelSkele, mnTemp);
		if(pPiece[0] == '\0') continue;

		// If this is the same as our explosion piece... check for more children
		if(!_stricmp(pPiece, pExplPiece))
		{
			pChildren = g_pModelButeMgr->GetSkeletonNodeChildren(eModelSkele, mnTemp);
			nChildren = g_pModelButeMgr->GetSkeletonNodeNumChildren(eModelSkele, mnTemp);

			for(i = 0; i < nChildren; i++)
			{
				++nStackAmount;
				_ASSERT(nStackAmount < 32);

				pNodeStack[nStackAmount] = pChildren[i];
			}
		}
		else
		{
			// recurse here...
			ExplodeLimb(mnTemp, eModelSkele);
		}
	}


	// Set the explode piece invisible now that we created all the limbs
	// Don't hide players' pieces, they can heal
	if(hPiece != INVALID_MODEL_PIECE && !IsPlayer(m_hObject))
		g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);

	// Now try the sub pieces
	char pTempPiece[255];

	for(i=1; hPiece != INVALID_MODEL_NODE; i++)
	{
		sprintf(pTempPiece, "%s%s%d", pExplPiece, "_OBJ", i);

		if(g_pModelLT->GetPiece(m_hObject, pTempPiece, hPiece) == LT_OK)
			g_pModelLT->SetPieceHideStatus(m_hObject, hPiece, LTTRUE);	
	}

	// Now create some appropriate GIBs at the location of our parent node
	CreateGibs(eParentNode, eModelSkele);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateGibs()
//
//	PURPOSE:	Create the gibs props
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateGibs(ModelNode eNode, ModelSkeleton eModelSkele)
{
	LTVector vPos;
	GetNodePosition(eNode, vPos, eModelSkele);

	const char* pDebrisType = g_pModelButeMgr->GetSkeletonNodeGIBType(eModelSkele, eNode);

	if(pDebrisType)
	{
		MULTI_DEBRIS *debris = g_pDebrisMgr->GetMultiDebris(const_cast<char*>(pDebrisType));

		if(debris)
		{
			LTVector vVel;
			g_pLTServer->GetVelocity(m_hObject, &vVel);

			CLIENTDEBRIS fxStruct;

			fxStruct.rRot.Init();
			fxStruct.vPos = vPos;
			fxStruct.nDebrisId = debris->nId;
			fxStruct.vVelOffset = vVel;

			CreateMultiDebris(fxStruct);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetNodePosition()
//
//	PURPOSE:	Get the node position... or it's parent if this node
//				is represented by a socket instead
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::GetNodePosition(ModelNode eNode, LTVector &vPos, ModelSkeleton eModelSkele, LTRotation* pRot)
{
	HMODELNODE hNode = INVALID_MODEL_NODE;
	ModelNode pPosNode = eNode;
	const char* pNode = LTNULL;
	LTBOOL bFoundNodePos = LTFALSE;
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;

	// Be sure our input is valid.
	_ASSERT( eModelSkele != eModelSkeletonInvalid );
	_ASSERT( eNode != eModelNodeInvalid );
	if( eModelSkele == eModelSkeletonInvalid || eNode == eModelNodeInvalid )
	{
		return LTFALSE;
	}

	// Get the node's name.
	const char * szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkele, eNode);
	_ASSERT( szNodeName && szNodeName[0] );
	if( !szNodeName || !szNodeName[0] )
		return LTFALSE;

	while(pPosNode != eModelNodeInvalid)
	{
		// Get the name of the node
		pNode = g_pModelButeMgr->GetSkeletonNodeName(eModelSkele, pPosNode);

		// Make the piece visible on the new limb and invisible on the current bodyprop
		if(g_pModelLT->GetNode(m_hObject, const_cast<char*>(pNode), hNode) == LT_OK)
		{
			LTransform trans;
			g_pModelLT->GetNodeTransform(m_hObject, hNode, trans, LTTRUE);
			g_pLTServer->GetTransformLT()->GetPos(trans, vPos);

			if(pRot)
				g_pLTServer->GetTransformLT()->GetRot(trans, *pRot);

			bFoundNodePos = LTTRUE;
			break;
		}
		else if (g_pModelLT->GetSocket(m_hObject, const_cast<char*>(szNodeName), hSocket) == LT_OK)
		{
			LTransform	tTransform;

			if (g_pModelLT->GetSocketTransform(m_hObject, hSocket, tTransform, LTTRUE) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTServer->GetTransformLT();
				pTransLT->GetPos(tTransform, vPos);

				if(pRot)
					g_pLTServer->GetTransformLT()->GetRot(tTransform, *pRot);

				bFoundNodePos = LTTRUE;
				break;
			}
		}
		else
		{
			// Go back a node in the tree
			pPosNode = g_pModelButeMgr->GetSkeletonNodeParent(eModelSkele, pPosNode);
		}

		// Get the hit location of this node... if it's all the way back to the torso
		// or something unknown... then just exit, we'll use the object position instead
		HitLocation hlTemp = g_pModelButeMgr->GetSkeletonNodeHitLocation(eModelSkele, pPosNode);
		if(hlTemp == HL_TORSO || hlTemp == HL_UNKNOWN)
		{
			g_pLTServer->GetObjectPos(m_hObject, &vPos);
			break;
		}
	}

	return bFoundNodePos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleNetHit()
//
//	PURPOSE:	Handle getting hit by a predator's net
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleNetHit(LTVector vDir, HOBJECT hShooter)
{
	// Load up the animations
	if(!LoadNetAnimation() || !m_damage.GetCanDamage())
	{
#ifndef _FINAL
		// This model is not ready to be netted...
		g_pLTServer->CPrint("Net Hit Failed: Invalid or missing net animations.");
#endif
		// Make some debris so we don't look too stupid
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris("Nets_ammo");
		if (pDebris)
		{
			LTVector vVel;
			LTVector vPos;

			g_pLTServer->GetVelocity(m_hObject, &vVel);
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			vVel.Norm();

			CreatePropDebris(vPos, vVel, pDebris->nId);
		}
		return;
	}

	// Create the net object
	m_hNet = CreateNet();

	// Sanity...
	if(!m_hNet) return;

	// Attach the net
	AttachNet(m_hNet);

	// Record some data
	m_fNetTime			= g_pLTServer->GetTime();
	m_nNetDamage		= 0;
	m_bFirstNetUpdate	= LTTRUE;

	// Put all the other animations on hold
	m_caAnimation.Enable(LTFALSE);

	// If we are a player then tell the client to get on it...
	// else do the movement on the server...
	LTVector vVel = (vDir+LTVector(0.0f,1.0f, 0.0f))*600.0f;

	if(IsPlayer(m_hObject))
	{
		HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(((CPlayerObj*)this)->GetClient(), MID_PHYSICS_UPDATE);
		if(!hWrite) return;
		
		// Write the state flags to the message first, so we know what to read out on the other side
		g_pLTServer->WriteToMessageWord(hWrite, (uint16)PSTATE_SET_NET_FX);
		g_pLTServer->WriteToMessageVector(hWrite, &vVel);
		g_pLTServer->EndMessage2(hWrite, MESSAGE_GUARANTEED);
	}
	else
	{
		// See that the AI does its thing.
		HandleStunEffect();

		// Send us flying!
		m_cmMovement.AllowMovement(LTFALSE);
		m_cmMovement.AllowRotation(LTFALSE);
		m_cmMovement.SetVelocity(vVel);
		m_cmMovement.SetMovementState(CMS_PRE_FALLING);
		m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) | FLAG_GRAVITY);
		m_cmMovement.SetObjectFlags(OFT_Flags2, m_cmMovement.GetObjectFlags(OFT_Flags2) & ~(FLAG2_ORIENTMOVEMENT | FLAG2_SPHEREPHYSICS));

		// Make sure the AI knows who hit them...
		const CCharacter * pChar = dynamic_cast<const CCharacter*>( g_pLTServer->HandleToObject(hShooter) );
		CAI * pAi = dynamic_cast<CAI*>( g_pLTServer->HandleToObject(m_hObject) );
		if( pAi && pChar && g_pCharacterMgr->AreEnemies(*this, *pChar) )
		{
			pAi->GetSenseMgr().TouchCharacter(*pChar);
		}
		else
		{
			// Make sure the SimpleAI knows who hit them...
			SimpleAIAlien* pSimpleAI = dynamic_cast<SimpleAIAlien*>( g_pLTServer->HandleToObject(m_hObject));

			if( pSimpleAI && pChar && g_pCharacterMgr->AreEnemies(*this, *pChar) )
			{
				pSimpleAI->SetTarget(hShooter);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleNetHit()
//
//	PURPOSE:	Handle getting hit by a predator's net
//
// ----------------------------------------------------------------------- //

HOBJECT CCharacter::CreateNet()
{
	//load up the create struct
	ObjectCreateStruct createStruct;
	createStruct.Clear();

	HCLASS hClass = g_pServerDE->GetClass("Prop");
	if (!hClass) return LTNULL;

#ifdef _DEBUG
	sprintf(createStruct.m_Name, "%s-Net", g_pLTServer->GetObjectName(m_hObject) );
#endif

#ifdef _WIN32
	SAFE_STRCPY(createStruct.m_Filename, "Models\\SFX\\Projectiles\\prednet2.abc");
	SAFE_STRCPY(createStruct.m_SkinNames[0], "Skins\\SFX\\Projectiles\\prednet2.dtx");
#else
	SAFE_STRCPY(createStruct.m_Filename, "Models/SFX/Projectiles/prednet2.abc");
	SAFE_STRCPY(createStruct.m_SkinNames[0], "Skins/SFX/Projectiles/prednet2.dtx");
#endif

	createStruct.m_ObjectType	= OT_MODEL;
	createStruct.m_Flags		= FLAG_VISIBLE;

	// Allocate an object...
	Prop* pProp;
	pProp = (Prop*)g_pLTServer->CreateObject(hClass, &createStruct);
	if (!pProp) return LTNULL;

	// Make sure we can't be damaged
	pProp->GetDestructible()->SetCanDamage(LTFALSE);

	//now set up our dims based on our crouch dims
	//make it a square based on two times the y dim
	LTVector vScale;
	uint32 nIndex = g_pLTServer->GetAnimIndex(m_hObject, "Dims_Crouch");
	g_pLTServer->Common()->GetModelAnimUserDims(m_hObject, &vScale, nIndex);
	vScale.x = vScale.y;
	vScale.z = vScale.y;
	g_pLTServer->ScaleObject(pProp->m_hObject, &(vScale*3));
	
	// Looks like all is good...
	return pProp->m_hObject;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateNet()
//
//	PURPOSE:	Update the status of our net attachment.
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateNet()
{
	// See if it is time to go...
	if(g_pLTServer->GetTime() - m_fNetTime > g_cvarNetTime.GetFloat() && m_eNetStage != NET_STAGE_GETUP)
	{
		HandleNetEscape();
	}

	// Do the initial up date thing
	if(m_bFirstNetUpdate)
	{
		// First update so set the flags and color...
		uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hNet);
		g_pLTServer->SetObjectFlags(m_hNet, dwFlags | FLAG_VISIBLE);

		uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hNet);
		g_pLTServer->SetObjectUserFlags(m_hNet, dwUsrFlags | USRFLG_SPECIAL_ATTACHMENT);

		LTFLOAT fR, fG, fB, fA;
		g_pLTServer->GetObjectColor(m_hNet, &fR, &fG, &fB, &fA);
		g_pLTServer->SetObjectColor(m_hNet, fR, fG, fB, 0.99f);

		// Set our initial stage
		m_eNetStage = NET_STAGE_START;
		m_eLastNetStage = NET_STAGE_INVALID;

		// Don't come back here next time...
		m_bFirstNetUpdate = LTFALSE;
	}

	// Update our animation state
	TrackerInfo* pInfo	= m_caAnimation.GetTrackerInfo(ANIM_TRACKER_MOVEMENT);
	HMODELANIM dwAni;
	g_pModelLT->GetCurAnim(pInfo->m_pTracker, dwAni);
	uint32 dwState;
	g_pModelLT->GetPlaybackState(pInfo->m_pTracker, dwState);

	const int nStage = m_eNetStage;
	const NetStage eLastNetStage = m_eLastNetStage;

	m_eLastNetStage = m_eNetStage;

	switch(nStage)
	{
	case (NET_STAGE_START):
		{
			// If we are not playing our animation then play it...
			if(eLastNetStage != NET_STAGE_START)
			{
				m_caAnimation.PlayAnim(m_NetAnims[NET_STAGE_START]);
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Net_Impact");
			}
			else if(dwState & MS_PLAYDONE || !m_caAnimation.UsingAltAnim())
			{
				// See if we are standing on somthing
				if(m_cmMovement.IsStandingOn())
				{
					m_eNetStage = NET_STAGE_END;
				}
			}
			break;
		}
	case (NET_STAGE_END):
		{
			// If we are not playing our animation then play it...
			if( eLastNetStage != NET_STAGE_END)
			{
				m_caAnimation.PlayAnim(m_NetAnims[NET_STAGE_END]);
			}
			else if(dwState & MS_PLAYDONE  || !m_caAnimation.UsingAltAnim())
			{
				// All done so let's switch to the next stage
				m_eNetStage = NET_STAGE_STRUGGLE;
			}
			break;
		}
	case (NET_STAGE_STRUGGLE):
		{
			// If we are not playing our animation then play it...
			if(eLastNetStage != NET_STAGE_STRUGGLE)
			{
				m_caAnimation.PlayAnim(m_NetAnims[NET_STAGE_STRUGGLE], LTTRUE);
				m_hNetStruggleSound = g_pServerSoundMgr->PlayCharSndFromObject(GetButes()->m_nId, m_hObject, "Net_Struggle");
			}
			break;
		}
	case (NET_STAGE_GETUP):
		{
			// If we are not playing our animation then play it...
			if( eLastNetStage != NET_STAGE_GETUP)
			{
				m_caAnimation.PlayAnim(m_NetAnims[NET_STAGE_GETUP]);
				if(m_hNetStruggleSound)
				{
					g_pLTServer->KillSound(m_hNetStruggleSound);
					m_hNetStruggleSound = LTNULL;
				}
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Net_Escape");
			}
			else if(dwState & MS_PLAYDONE || !m_caAnimation.UsingAltAnim())
			{
				// All done so let's switch to the next stage
				m_eNetStage = NET_STAGE_INVALID;

				// Remove movement restrictions
				m_caAnimation.Enable(LTTRUE);

				// Lost the net attachment
				RemoveNet();

				// Make sure we free up any stun effects..
				SimpleAIAlien* pAI = dynamic_cast<SimpleAIAlien*>( g_pLTServer->HandleToObject(m_hObject));

				if(pAI)
					UpdateStunEffect();

			}
			break;
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleNetEscape()
//
//	PURPOSE:	Handle escaping from the net
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleNetEscape()
{
	// Gib the net
	DEBRIS* pDebris = g_pDebrisMgr->GetDebris("Nets_ammo");
	if (pDebris)
	{
		LTVector vVel;
		LTVector vPos;

		g_pLTServer->GetVelocity(m_hObject, &vVel);
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		vVel.Norm();

		CreatePropDebris(vPos, vVel, pDebris->nId);
	}

	// Play the stand up anim
	m_eNetStage = NET_STAGE_GETUP;

	// Hide the net
	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hNet);
	g_pLTServer->SetObjectFlags(m_hNet, dwFlags & ~FLAG_VISIBLE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::LoadNetAnimation()
//
//	PURPOSE:	Load up the net animations
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::LoadNetAnimation()
{
	m_NetAnims[NET_STAGE_START]		= g_pLTServer->GetAnimIndex(m_hObject, "Net_thrw_start");
	m_NetAnims[NET_STAGE_END]		= g_pLTServer->GetAnimIndex(m_hObject, "Net_thrw_end");
	m_NetAnims[NET_STAGE_STRUGGLE]	= g_pLTServer->GetAnimIndex(m_hObject, "Net_thrw_struggle");
	m_NetAnims[NET_STAGE_GETUP]		= g_pLTServer->GetAnimIndex(m_hObject, "Net_thrw_getup");

	for(int i=0; i <  MAX_NET_ANIMS; i++)
	{
		if(m_NetAnims[i] == INVALID_ANI)
			return LTFALSE;
	}

	// If we get here then all is good...
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetLastFireInfo
//
//	PURPOSE:	Records last fire info and notifies AI's about sound.
//
// ----------------------------------------------------------------------- //
void CCharacter::SetLastFireInfo(const CharFireInfo & info)
{
	m_LastFireInfo = info;

	// Notify all the AI's.
	if( g_pGameServerShell->GetGameType().IsSinglePlayer() )
	{
		for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs();
			 iter != g_pCharacterMgr->EndAIs(); ++iter )
		{
			(*iter)->GetSenseMgr().WeaponFire(info,this);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetLastImpactInfo
//
//	PURPOSE:	Records last impact info and notifies AI's about sound.
//
// ----------------------------------------------------------------------- //
void CCharacter::SetLastImpactInfo(const CharImpactInfo & info)
{
	m_LastImpactInfo = info;

	// Notify all the AI's.
	if( g_pGameServerShell->GetGameType().IsSinglePlayer() )
	{
		for( CCharacterMgr::AIIterator iter = g_pCharacterMgr->BeginAIs();
			 iter != g_pCharacterMgr->EndAIs(); ++iter )
		{
			(*iter)->GetSenseMgr().WeaponImpact(info,this);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CacheDebris
//
//	PURPOSE:	Cache our debris files...
//
// ----------------------------------------------------------------------- //
void CCharacter::CacheDebris()
{
	// All this is now in CacheCharacterFiles.
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HasDiscAmmo
//
//	PURPOSE:	See if we have any disc ammo
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::HasDiscAmmo()
{ 
	CHumanAttachments* pAttach = dynamic_cast<CHumanAttachments*>(m_pAttachments);

	if(pAttach)
	{
		CAttachmentWeapon* pAttachedWeapon = dynamic_cast<CAttachmentWeapon*>(pAttach->GetRightHand().GetAttachment());

		if(pAttachedWeapon)
		{
			CWeapons* pWeapons = pAttachedWeapon->GetWeapons();

			if(pWeapons)
			{
				uint32 nAmmo = pWeapons->GetAmmoPoolCount(g_pWeaponMgr->GetAmmoPool("Discs")->nId);
				return (nAmmo != 0);
			}
		}
	}
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RespawnExosuit
//
//	PURPOSE:	Respawn the exosuit
//
// ----------------------------------------------------------------------- //

void CCharacter::RespawnExosuit()
{ 
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		if(m_hStartpointSpawner)
		{
			GameStartPoint* pStartPoint = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(m_hStartpointSpawner));

			if(pStartPoint)
			{
				pStartPoint->SetExosuitRespawn();
			}

			// Make sure we only do this once
			m_hStartpointSpawner = LTNULL;
		}
	}	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;
	
	//no save needed on these
	// m_damage;
	// m_pAttachments;
	// m_editable;

	*hWrite << m_nCharacterSet;			
	*hWrite << m_bCharacterReset;		
	*hWrite << m_nOldCharacterSet;		
	*hWrite << m_fOldHealth;			
	*hWrite << m_fOldArmor;				

	m_cmMovement.Save(hWrite);				
	m_caAnimation.Save(hWrite);				

	*hWrite << m_dwFlags;		

	*hWrite << m_fAirSupplyTime;
	*hWrite << m_fOldAirSupplyTime;

	*hWrite << m_nCannonChargeLevel;
	*hWrite << m_nMaxChargeLevel;

	*hWrite << m_fBatteryChargeLevel;

	hWrite->WriteDWord( m_pCurrentVolume ? m_pCurrentVolume->GetID() : CAIVolume::kInvalidID );
	hWrite->WriteDWord( m_pLastVolume ? m_pLastVolume->GetID() : CAIVolume::kInvalidID );

	*hWrite << m_vLastVolumePos;

	hWrite->WriteDWord( m_pLastSimpleNode ? m_pLastSimpleNode->GetID() : CSimpleNode::kInvalidID );
	*hWrite << m_tmrNextSimpleNodeCheck;

	*hWrite << m_bCreateBody;	

	*hWrite << m_hHitBox;			

	*hWrite << m_fDefaultHitPts;	
	*hWrite << m_fDefaultArmor;		

	hWrite->WriteWord(m_eModelNodeLastHit);

	m_LastFireInfo.Save(hWrite);			
	m_LastImpactInfo.Save(hWrite);			
	m_LastFootstepSoundInfo.Save(hWrite);	

	*hWrite << m_vOldCharacterColor;
	*hWrite << m_fOldCharacterAlpha;
	*hWrite << m_bCharacterHadShadow;
	*hWrite << m_hstrSpawnItem;		

	*hWrite << m_bEMPEffect;		
	*hWrite << m_bStunEffect;		

	*hWrite << m_bHasFlarePouch;
	*hWrite << m_bHasCloakDevice;
	*hWrite << m_bHasMask;
	*hWrite << m_bHasNightVision;

	hWrite->WriteFloat(m_fCloakStartTime - g_pLTServer->GetTime());
	*hWrite << m_nCloakState;		
	*hWrite << m_fEnergyChange;		

	hWrite->WriteFloat(m_fDialogueStartTime - g_pLTServer->GetTime());
	*hWrite << m_bPlayingTextDialogue;	

	*hWrite << m_cSpears;				
	for ( uint32 i = 0 ; i < kMaxSpears ; i++ )
		m_aSpears[i].Save(hWrite);

	*hWrite << m_bFlashlight;			
	*hWrite << m_vFlashlightPos;		

	*hWrite << m_vModelLighting;

	*hWrite << m_nChestburstedFrom;		
	hWrite->WriteFloat(m_fChestbursterMutateTime - g_pLTServer->GetTime());
	*hWrite << m_fChestbursterMutateDelay;

	*hWrite << m_bNightvision;
	hWrite->WriteDWord( uint32(m_nDesiredExpression) );
	hWrite->WriteFloat(m_fLastBlinkTime - g_pLTServer->GetTime());
	*hWrite << m_fBlinkDelay;
	*hWrite << m_tmrMinRecoilDelay;

	*hWrite << m_fLastAimingPitch;
	// Don't save m_fLastAimingYaw.
	*hWrite << m_nLastAimingPitchSet;
	*hWrite << m_nAimingPitchSet;

	*hWrite << m_nLastStrafeState;

	*hWrite << m_hHead;
	*hWrite << m_bForceCrawl;

	hWrite->WriteFloat(m_fCloakDelayTime - g_pLTServer->GetTime());
	*hWrite << m_fCloakEnergyUsed;

	*hWrite << m_nPreCreateClientID;

	*hWrite << m_hNet;
	hWrite->WriteFloat(m_fNetTime - g_pLTServer->GetTime());
	*hWrite << m_nNetDamage;
	*hWrite << m_bFirstNetUpdate;
	hWrite->WriteWord(m_eNetStage);
	hWrite->WriteWord(m_eLastNetStage);

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...
	
	// m_hCurVcdSnd;		
	// m_nCurVcdSndImp;	
	// m_pTargetingSprite;	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	//no load needed on these
	// m_damage;
	// m_pAttachments;
	// m_editable;

	*hRead >> m_nCharacterSet;			
	*hRead >> m_bCharacterReset;		
	*hRead >> m_nOldCharacterSet;		
	*hRead >> m_fOldHealth;			
	*hRead >> m_fOldArmor;				
	
	m_cmMovement.Load(hRead);			
	m_caAnimation.Load(hRead);			
	
	*hRead >> m_dwFlags;		
	
	*hRead >> m_fAirSupplyTime;
	*hRead >> m_fOldAirSupplyTime;
	
	*hRead >> m_nCannonChargeLevel;
	*hRead >> m_nMaxChargeLevel;
	
	*hRead >> m_fBatteryChargeLevel;

	const uint32 current_vol_id = hRead->ReadDWord();
	const uint32 last_vol_id = hRead->ReadDWord();
	m_pCurrentVolume = g_pAIVolumeMgr->GetVolumePtr(current_vol_id);
	m_pLastVolume = g_pAIVolumeMgr->GetVolumePtr(last_vol_id);

	*hRead >> m_vLastVolumePos;
	
	const uint32 last_simple_node_id = hRead->ReadDWord();
	m_pLastSimpleNode = g_pSimpleNodeMgr->GetNode(last_simple_node_id);

	*hRead >> m_tmrNextSimpleNodeCheck;

	*hRead >> m_bCreateBody;	
	
	*hRead >> m_hHitBox;			
	
	*hRead >> m_fDefaultHitPts;	
	*hRead >> m_fDefaultArmor;		
	
	m_eModelNodeLastHit = ModelNode(hRead->ReadWord());

	m_LastFireInfo.Load(hRead);			
	m_LastImpactInfo.Load(hRead);			
	m_LastFootstepSoundInfo.Load(hRead);

	*hRead >> m_vOldCharacterColor;
	*hRead >> m_fOldCharacterAlpha;
	*hRead >> m_bCharacterHadShadow;
	*hRead >> m_hstrSpawnItem;		

	*hRead >> m_bEMPEffect;		
	*hRead >> m_bStunEffect;		

	*hRead >> m_bHasFlarePouch;
	*hRead >> m_bHasCloakDevice;
	*hRead >> m_bHasMask;
	*hRead >> m_bHasNightVision;

	m_fCloakStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_nCloakState;		
	*hRead >> m_fEnergyChange;		

	m_fDialogueStartTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_bPlayingTextDialogue;	

	*hRead >> m_cSpears;				
	for ( uint32 i = 0 ; i < kMaxSpears ; i++ )
		m_aSpears[i].Load(hRead);

	*hRead >> m_bFlashlight;			
	*hRead >> m_vFlashlightPos;		

	*hRead >> m_vModelLighting;

	*hRead >> m_nChestburstedFrom;		
	m_fChestbursterMutateTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_fChestbursterMutateDelay;

	*hRead >> m_bNightvision;
	m_nDesiredExpression = int( hRead->ReadDWord() );
	m_fLastBlinkTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_fBlinkDelay;
	*hRead >> m_tmrMinRecoilDelay;

	*hRead >> m_fLastAimingPitch;
	m_fLastAimingYaw = -1.0f;
	*hRead >> m_nLastAimingPitchSet;
	*hRead >> m_nAimingPitchSet;

	*hRead >> m_nLastStrafeState;

	*hRead >> m_hHead;
	*hRead >> m_bForceCrawl;

	m_fCloakDelayTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_fCloakEnergyUsed;

	*hRead >> m_nPreCreateClientID;

	*hRead >> m_hNet;
	m_fNetTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_nNetDamage;
	*hRead >> m_bFirstNetUpdate;
	m_eNetStage = NetStage(hRead->ReadWord());
	m_eLastNetStage = NetStage(hRead->ReadWord());

	// Load animations if we are netted
	if(m_hNet)
		LoadNetAnimation();

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...
	
	// m_hCurVcdSnd;		
	// m_nCurVcdSndImp;	
	// m_pTargetingSprite;	
	
	// Make sure this object is added to the global CharacterMgr...

	g_pCharacterMgr->Add(this);

	// We need to reset our hit box, as it doesn't save the node info.
	if( m_hHitBox )
	{
		CCharacterHitBox * pHitBox = dynamic_cast<CCharacterHitBox*>( g_pLTServer->HandleToObject(m_hHitBox) );
		if( pHitBox )
			pHitBox->ResetSkeleton();
	}

	// Set function pointer for exosuit energy
#ifndef __LINUX
	fpUpdateExoEnergy = IsExosuit(m_cmMovement.GetCharacterButes()) ? UpdateExoEnergy : LTNULL;
#else
	if (IsExosuit(m_cmMovement.GetCharacterButes()))
		fpUpdateExoEnergy = &CCharacter::UpdateExoEnergy;
	else
		fpUpdateExoEnergy = LTNULL;
#endif

	// Make sure we update our butes next update...
	if(IsPlayer(m_hObject))
		m_bConfirmFX = LTTRUE;
}
