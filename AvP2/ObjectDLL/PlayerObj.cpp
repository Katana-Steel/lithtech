// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.cpp
//
// PURPOSE : Player object implementation
//
// CREATED : 9/18/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "CommandIDs.h"
#include "ServerUtilities.h"
#include "HHWeaponModel.h"
#include "SurfaceFunctions.h"
#include "physics_lt.h"
#include "TeleportPoint.h"
#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "Spawner.h"
#include "Attachments.h"
#include "VolumeBrush.h"
#include "SurfaceMgr.h"
#include "Breakable.h"
#include "AINodeMgr.h"
#include "CharacterButeMgr.h"
#include "ServerSoundMgr.h"
#include "CharacterFuncs.h"
#include "DebugLineSystem.h"
#include "AIPlayer.h"
#include "ProjectileTypes.h"
#include "PickupButeMgr.h"
#include "ClientLightFX.h"
#include "MusicMgr.h"
#include "GameStartPoint.h"
#include "MultiplayerMgr.h"
#include "CharacterHitBox.h"
#include "CVarTrack.h"
#include "GameServerShell.h"
#include "SCDefs.h"
#include "AIUtils.h" // for AIErrorPrint
#include "MultiSpawner.h"
#include "AISenseMgr.h"
#include "SpectatorPoint.h"
#include "SimpleNodeMgr.h"
#include "Prop.h"
#include "Camera.h"
#include "CinematicTrigger.h"

static 	LTBOOL g_bShowNodes = LTFALSE;
static  LTBOOL g_bShowSimple = LTFALSE;

extern char s_FileBuffer[MAX_CS_FILENAME_LEN];

BEGIN_CLASS(CPlayerObj)
END_CLASS_DEFAULT_FLAGS(CPlayerObj, CCharacter, NULL, NULL, CF_HIDDEN)

// Defines...

#define UPDATE_DELTA						0.001f

#define TRIGGER_MUSIC						"MUSIC"
#define TRIGGER_MISSIONFAILED				"MISSIONFAILED"
#define TRIGGER_TRAITOR						"TRAITOR"
#define TRIGGER_FACEOBJECT					"FACEOBJECT"
#define TRIGGER_TELEPORT					"TELEPORT"
#define TRIGGER_TELEPORTNR					"TELEPORTNR"
#define TRIGGER_RESETINVENTORY				"RESETINVENTORY"
#define TRIGGER_CONSOLE						"CONSOLE"
#define TRIGGER_SWAP						"SWAP"
#define TRIGGER_PICKUP						"PICKUP"
#define TRIGGER_FADEIN						"FADEIN"
#define TRIGGER_FADEOUT						"FADEOUT"
#define TRIGGER_OBSERVE						"OBSERVE"
#define TRIGGER_RESPAWN						"RESPAWN"
#define TRIGGER_TELETYPE					"TELETYPE"
#define TRIGGER_HINT						"HINT"
#define TRIGGER_MD							"MOTION_TRACKER"
#define TRIGGER_ENERGY_CHARGE				"ENERGY_CHARGE"
#define	TRIGGER_CREDITS						"CREDITS"
#define TRIGGER_PICKUP_NOTELL				"PICKUP_NOTELL"

#define DEFAULT_LEASHLEN					500.0f


CVarTrack	g_LeashLenTrack;

#ifndef _FINAL
CVarTrack	g_ShowNodesTrack;
CVarTrack	g_ShowSimpleTrack;
CVarTrack	g_ClearLines;
#endif

CVarTrack	g_InvulnerableTime;
CVarTrack	g_InvulnerableLeash;

CVarTrack	g_PounceStunRatio;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CPlayerObj
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CPlayerObj::CPlayerObj() : CCharacter()
{
	// Default the character to the Marine
	m_nWeaponStatus			= WS_NONE;

	m_pAttachments	= DNULL;

	m_bFirstUpdate			= LTTRUE;
	m_ClientMoveCode		= 0;
	m_fOldHitPts			= -1;
	m_fOldMaxHitPts			= -1;
	m_fOldMaxArmor			= -1;
	m_fOldArmor				= -1;

	m_cc					= MARINE;

	m_hClient				= DNULL;
	m_fLeashLen				= DEFAULT_LEASHLEN;
	m_eState				= PS_DEAD;
	m_bGodMode				= DFALSE;
	m_bAllowInput			= LTTRUE;
	m_b3rdPersonView		= DFALSE;

	m_PStateChangeFlags		= 0;

	m_dwLastLoadFlags			= 0;
	m_hClientSaveData			= DNULL;

	m_hstrStartLevelTriggerTarget	= DNULL;
	m_hstrStartLevelTriggerMessage	= DNULL;

	m_hstrStartMissionTriggerTarget	= DNULL;
	m_hstrStartMissionTriggerMessage	= DNULL;

	m_bLevelStarted			= DFALSE;
	m_bWaitingForAutoSave	= DFALSE;

	m_pnOldAmmo				= DNULL;

	DBYTE nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();

	if (nNumAmmoPools > 0)
	{
		m_pnOldAmmo = new int[nNumAmmoPools];
		memset(m_pnOldAmmo, 0, nNumAmmoPools);
	}

	m_bSwapped		= LTFALSE;
	m_fMinHealth		= 0.0f;
	m_bInCinematic		= LTFALSE;
	m_fOldBatteryLevel	= 0.0f;
	m_fIdleTime			= 0.0f;

	m_vSpawnPosition.Init();
	m_fSpawnTime		= 0.0f;

	m_fLastHistory		= 0.0f;

	m_nObservePoint		= 0;
	m_bNewGame			= LTTRUE;
	m_bDoAutoSave		= LTFALSE;
	m_nSaveCount		= 0;
	m_bResetHealthAndArmor = LTFALSE;
	m_bPlayingLoopFireSound	= LTFALSE;
	m_nLoopKeyId		= 0xff;
	m_bSpawnInState		= LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::~CPlayerObj
//
//	PURPOSE:	deallocate object
//
// ----------------------------------------------------------------------- //

CPlayerObj::~CPlayerObj()
{
	if (m_hstrStartLevelTriggerTarget)
	{
		g_pLTServer->FreeString(m_hstrStartLevelTriggerTarget);
		m_hstrStartLevelTriggerTarget = DNULL;
	}

	if (m_hstrStartLevelTriggerMessage)
	{
		g_pLTServer->FreeString(m_hstrStartLevelTriggerMessage);
		m_hstrStartLevelTriggerMessage = DNULL;
	}
	
	if (m_hstrStartMissionTriggerTarget)
	{
		g_pLTServer->FreeString(m_hstrStartMissionTriggerTarget);
		m_hstrStartMissionTriggerTarget = DNULL;
	}

	if (m_hstrStartMissionTriggerMessage)
	{
		g_pLTServer->FreeString(m_hstrStartMissionTriggerMessage);
		m_hstrStartMissionTriggerMessage = DNULL;
	}

	if (m_pnOldAmmo)
	{
		delete [] m_pnOldAmmo;
	}

	if( m_hAIPlayer )
	{
		g_pLTServer->RemoveObject( m_hAIPlayer );
		m_hAIPlayer = LTNULL;
	}

	// This must be done here so that charactermgr
	// will recognize our type via dynamic cast.
	// Is this an MSVC bug?
	g_pCharacterMgr->Remove(this);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CPlayerObj::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_GETFORCEUPDATEOBJECTS:
		{
			SetForceUpdateList((ForceUpdate*)pData);
		}
		break;

		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
			PostPropRead((ObjectCreateStruct*)pData);

			// If we are loading from a keepalive file, then we don't really need to
			// setup our models, since ResetCharacter will handle that for us.  Plus
			// keepalive saves may be loading models/skins from a different race that
			// was used in the last level, but not needed in this level.
			// The final PRECREATE_SAVEGAME check should be redundant.
			if(    g_pGameServerShell 
				&& g_pGameServerShell->IsLoadingKeepAlive() 
				&& fData == PRECREATE_SAVEGAME )
			{
				ObjectCreateStruct* pStruct = ( ObjectCreateStruct* )pData;
				pStruct->m_Filename[0] = 0;
				pStruct->m_SkinName[0] = 0;
				// Go through each texture and load the normal texture
				for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
				{
					pStruct->m_SkinNames[i][0] = 0;
				}
			}

			return dwRet;
		}

		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			InitialUpdate((int)fData);

			return dwRet;
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}


	return CCharacter::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CPlayerObj::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_FORCE_POUNCE:
		{
			HOBJECT hTarget = hRead->ReadObject();

			LTVector vPos, vVel;

			// Get the target position and offset it...
			g_pLTServer->GetObjectPos(hTarget,&vPos);

			CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hTarget) );
			if( pChar )
			{
				vPos += pChar->GetHeadOffset();
			}

			// Get it's velocity...
			g_pInterface->Physics()->GetVelocity(hTarget, &vVel);

			//send message to player to change state
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(m_hClient, MID_PHYSICS_UPDATE);
			if(!hWrite) break;

			// Write the state flags to the message first, so we know what to read out on the other side
			g_pLTServer->WriteToMessageWord(hWrite, (uint16)PSTATE_POUNCE);
			g_pLTServer->WriteToMessageVector(hWrite, &vPos);
			g_pLTServer->WriteToMessageVector(hWrite, &vVel);

			g_pLTServer->EndMessage2(hWrite, MESSAGE_GUARANTEED);
		}
		break;

		case MID_FORCE_FACEHUG:
		{
			HOBJECT hTarget = hRead->ReadObject();

			LTVector vPos, vVel;

			// Get the target position and offset it...
			g_pLTServer->GetObjectPos(hTarget,&vPos);

			CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hTarget) );
			if( pChar )
			{
				vPos += pChar->GetHeadOffset();
			}

			// Get it's velocity...
			g_pInterface->Physics()->GetVelocity(hTarget, &vVel);

			//send message to player to change state
			HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(m_hClient, MID_PHYSICS_UPDATE);
			if(!hWrite) break;

			// Write the state flags to the message first, so we know what to read out on the other side
			g_pLTServer->WriteToMessageWord(hWrite, (uint16)PSTATE_FACEHUG);
			g_pLTServer->WriteToMessageVector(hWrite, &vPos);
			g_pLTServer->WriteToMessageVector(hWrite, &vVel);

			g_pLTServer->EndMessage2(hWrite, MESSAGE_GUARANTEED);
		}
		break;

	}
	
	return CCharacter::ObjectMessageFn(hSender, messageID, hRead);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessAnimationMsg()
//
//	PURPOSE:	Process an animation system message.
//
// --------------------------------------------------------------------------- //

void CPlayerObj::ProcessAnimationMsg(uint8 nTracker, uint8 bLooping, uint8 nMsg)
{
	CCharacter::ProcessAnimationMsg(nTracker, bLooping, nMsg);

	// If we've got a weapon state and we hit a break... go to no weapon state
	if((nTracker == ANIM_TRACKER_MOVEMENT) && (nMsg == ANIMMSG_ANIM_BREAK))
	{
		switch(m_nWeaponStatus)
		{
			case WS_NONE:		m_nWeaponStatus = WS_NONE;		break;
			case WS_RELOADING:	m_nWeaponStatus = WS_NONE;		break;
			case WS_SELECT:		m_nWeaponStatus = WS_NONE;		break;
			case WS_DESELECT:	m_nWeaponStatus = WS_SELECT;	break;
			case WS_MELEE:		m_nWeaponStatus = WS_NONE;		break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PostPropRead
//
//	PURPOSE:	Handle post-property initialization
//
// ----------------------------------------------------------------------- //

void CPlayerObj::PostPropRead(ObjectCreateStruct *pStruct)
{
	if(pStruct)
	{
		SAFE_STRCPY(pStruct->m_Name, DEFAULT_PLAYERNAME);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::InitialUpdate(int nInfo)
{
	if(g_pLastPlayerStartPt && (g_pGameServerShell->GetGameType() == SP_STORY))
	{
		m_nCharacterSet = g_pLastPlayerStartPt->GetCharacterSet();
	}

	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED | NETFLAG_ROTUNGUARANTEED | NETFLAG_ANIMUNGUARANTEED);

	// Set up console vars used to tweak player movement...

	g_LeashLenTrack.Init(g_pLTServer, "LeashLen", DNULL, DEFAULT_LEASHLEN);

#ifndef _FINAL
	g_ShowNodesTrack.Init(g_pLTServer, "ShowNodes", DNULL, 0.0f);
	g_ShowSimpleTrack.Init(g_pLTServer, "ShowSimple", DNULL, 0.0f);
	g_ClearLines.Init(g_pLTServer, "ClearLines", DNULL, 0.0f);
#endif

	g_InvulnerableTime.Init(g_pLTServer, "InvulnerableTime", DNULL, 15.0f);
	g_InvulnerableLeash.Init(g_pLTServer, "InvulnerableLeash", DNULL, 200.0f);

	g_PounceStunRatio.Init(g_pLTServer, "PounceStunMod", DNULL, 33.0f);

	if (nInfo == INITIALUPDATE_SAVEGAME) return LTTRUE;


	g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);
	g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

	m_damage.SetMass( g_pCharacterButeMgr->GetDefaultMass(GetCharacter()) );

	ResetPlayerHealth();

	// Turn off solid and gravity.
	// This will be updated as soon as we receive an update from the client.
	m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) & ~(FLAG_SOLID | FLAG_GRAVITY), LTTRUE);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::Update()
{
	if (!g_pGameServerShell) return DFALSE;

	if( m_hAIPlayer && !m_bSwapped)
	{
		g_pLTServer->SetObjectFlags( m_hAIPlayer, g_pLTServer->GetObjectFlags(m_hAIPlayer) | FLAG_FORCECLIENTUPDATE);
		g_pLTServer->TeleportObject( m_hAIPlayer, const_cast<LTVector*>(&GetPosition()) );
	}
	else if( m_hAIPlayer && m_bSwapped)
	{
		// Teleport me to the ai player loc...
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hAIPlayer, &vPos);	
		g_pLTServer->SetObjectPos(m_hObject, &vPos);	
	}

	// We need to do this before checking our m_hClient data member since
	// there are cases (load/save) where m_hClient gets set to null, but 
	// the player object is still valid (and needs to get update called
	// until the data member is set back to a valid value)...

	g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);

	if (!m_hClient || m_bWaitingForAutoSave) return DFALSE;

	GameType eGameType = g_pGameServerShell->GetGameType();

	
	// If we are starting a level from scratch, inform the player
	// summary...

	if(eGameType == SP_STORY && m_bFirstUpdate)
	{
		if (m_dwLastLoadFlags != LOAD_RESTORE_GAME)
		{
//			m_PlayerSummary.HandleLevelStart();
		}
	}
	

	// Check to see if we just reloaded a saved game...

	if (m_dwLastLoadFlags == LOAD_RESTORE_GAME || m_dwLastLoadFlags == LOAD_NEW_LEVEL)
	{
		HandleGameRestore();
		m_dwLastLoadFlags = 0;
	}


	//  If the level hasn't been started yet (single player only), make sure
	//  our velocity isn't updated...

	if (eGameType == SP_STORY)
	{
		if (m_bFirstUpdate)
		{
			UpdateClientPhysics(); // (to make sure they have their model around)
			TeleportClientToServerPos(1.0f);
			m_bFirstUpdate = DFALSE;
		}

		if (!m_bLevelStarted)
		{
			m_cmMovement.SetVelocity(LTVector(0.0f, 0.0f, 0.0f));
			m_cmMovement.SetAcceleration(LTVector(0.0f, 0.0f, 0.0f));

			return LTFALSE;
		}
	}

	// Update our containers with the character movement class

	m_cmMovement.UpdateContainers();


	// Keep the client updated....

	UpdateClientPhysics();


	// Update Interface...

	UpdateInterface();


	// Let the client know our position...

	UpdateClientViewPos();


	// Update our console vars (have they changed?)...

	UpdateConsoleVars();



	uint32 dwUserFlags	= g_pLTServer->GetObjectUserFlags(m_hObject);
	uint32 dwFlags		= m_cmMovement.GetObjectFlags(OFT_Flags);


	// Multiplayer specific updates

	if(eGameType.IsMultiplayer())
	{
		// Make sure we have our visible flag set
		if(m_eState == PS_ALIVE && !(dwFlags & FLAG_VISIBLE))
			m_cmMovement.SetObjectFlags(OFT_Flags, dwFlags | FLAG_VISIBLE);

		// Force the players to god mode if the server is in a state other than playing...
		if(	(g_pGameServerShell->GetMultiplayerMgr()->GetServerState() != MPMGR_STATE_PLAYING) ||
			(g_pGameServerShell->GetMultiplayerMgr()->GetServerSubState() == MPMGR_SUBSTATE_SURVIVOR_ROUND) ||
			(g_pGameServerShell->GetMultiplayerMgr()->GetServerSubState() == MPMGR_SUBSTATE_SURVIVOR_INTERM) ||
			(g_pGameServerShell->GetMultiplayerMgr()->GetServerSubState() == MPMGR_SUBSTATE_TM_ROUND))
		{
			if(!InGodMode())
			{
				ToggleGodMode(LTTRUE, -1.0f);
			}
		}
		// Special observing state stuff...
		else if(m_eState == PS_OBSERVING)
		{
			if(!InGodMode())
				ToggleGodMode(LTTRUE, -1.0f);

			m_cmMovement.SetVelocity(LTVector(0.0f, 0.0f, 0.0f));
			m_cmMovement.SetAcceleration(LTVector(0.0f, 0.0f, 0.0f));
		}
		// Check our invulnerable leash
		else if(InGodMode() && !(dwUserFlags & USRFLG_CHAR_FACEHUG))
		{
			LTVector vTemp;
			g_pLTServer->GetObjectPos(m_hObject, &vTemp);

			vTemp = vTemp - m_vSpawnPosition;

			if(	(vTemp.Mag() >= g_InvulnerableLeash.GetFloat()) ||
				(g_pLTServer->GetTime() >= (m_fSpawnTime + g_InvulnerableTime.GetFloat())))
			{
				ToggleGodMode(LTFALSE, -1.0f, LTTRUE);

				// turn off our flag
				m_bSpawnInState = LTFALSE;
			}
		}

		UpdateHistory();
	}


	if(dwUserFlags & USRFLG_CHAR_FACEHUG)
	{
		if(!InGodMode())
		{
			ToggleGodMode(LTTRUE, -1.0f);
		}
	}


	// If we're outside the world (and not in spectator mode)...wake-up, 
	// time to die...
	if(!m_bSwapped)
	{
		LTVector vPos, vMin, vMax;
		g_pLTServer->GetWorldBox(vMin, vMax);
		g_pLTServer->GetObjectPos(m_hObject, &vPos);	

		if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
			vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
		{
			DamageStruct damage;

			damage.eType	= DT_EXPLODE;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager = m_hObject;
			damage.vDir.Init(0, 1, 0);

			damage.DoDamage(this, m_hObject);
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateWeaponAnimation()
//
//	PURPOSE:	Update the weapon animation information
//
// ----------------------------------------------------------------------- //

WEAPON* CPlayerObj::UpdateWeaponAnimation()
{
	WEAPON* pWeap = CCharacter::UpdateWeaponAnimation();

	if(g_pLTServer->GetTime() >= m_fIdleTime)
		m_nAimingPitchSet = 0;

	return pWeap;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateExtWeaponAnimation()
//
//	PURPOSE:	Update the extented action animation information
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateExtWeaponAnimation()
{
	// Determine if we're in a special state
	char *szAction = "None";

	if(m_nWeaponStatus == WS_RELOADING)
	{
		szAction = "Reload";
		m_nWeaponStatus = WS_NONE;
	}
	else if(m_nWeaponStatus == WS_MELEE)
	{
		szAction = "Melee";
		m_nWeaponStatus = WS_NONE;
	}


	// Check for other states that don't support special animations
	CharacterMovementState cms = m_cmMovement.GetMovementState();

	switch(cms)
	{
		case CMS_STAND_JUMP:
		case CMS_WALK_JUMP:
		case CMS_RUN_JUMP:
		case CMS_WWALK_JUMP:
		case CMS_STAND_FALL:
		case CMS_WALK_FALL:
		case CMS_RUN_FALL:
		case CMS_WWALK_FALL:
		case CMS_POUNCE_FALL:
		case CMS_STAND_LAND:
		case CMS_WALK_LAND:
		case CMS_RUN_LAND:
		case CMS_WWALK_LAND:
		{
			szAction = "None";
			break;
		}
	}

	// Check for states that don't involve character movement state
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	uint32 nEMPFlags = g_pCharacterButeMgr->GetEMPFlags(m_nCharacterSet);

	if((dwUserFlags & USRFLG_CHAR_STUNNED) || ((dwUserFlags & USRFLG_CHAR_EMPEFFECT) && (nEMPFlags & EMP_FLAG_STUN)))
	{
		szAction = "None";
	}

	m_caAnimation.SetLayerReference(ANIMSTYLELAYER_EXT_ACTION, szAction);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateActionAnimation()
//
//	PURPOSE:	Update the action animation information
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateActionAnimation()
{
	const char *szAction = "Idle";


	// Now find an animation action based off the movement state
	CharacterMovementState cms = m_cmMovement.GetMovementState();
	uint32 nFlags = m_cmMovement.GetControlFlags();

	LTBOOL bFacehugger = (LTBOOL)(strstr(m_cmMovement.GetCharacterButes()->m_szName, "Facehugger"));
	LTBOOL bAlien = (m_cmMovement.GetCharacterButes()->m_eCharacterClass == ALIEN);
	LTBOOL bRun = (m_cmMovement.GetControlFlags() & CM_FLAG_RUN);
	

	switch(cms)
	{
		case CMS_IDLE:
		{
			if(!IsAlien(m_cmMovement.GetCharacterButes()) && (g_pLTServer->GetTime() < m_fIdleTime))
				szAction = "Aim";

			break;
		}

		case CMS_WALKING:
		{
			if((nFlags & CM_FLAG_FORWARD) && !(nFlags & CM_FLAG_BACKWARD))
				szAction = "WalkF";
			else if(!(nFlags & CM_FLAG_FORWARD) && (nFlags & CM_FLAG_BACKWARD))
				szAction = "WalkB";
			else if((nFlags & CM_FLAG_STRAFELEFT) && !(nFlags & CM_FLAG_STRAFERIGHT))
				szAction = bFacehugger ? "WalkL" : "WalkF";
			else if(!(nFlags & CM_FLAG_STRAFELEFT) && (nFlags & CM_FLAG_STRAFERIGHT))
				szAction = bFacehugger ? "WalkR" : "WalkF";

			break;
		}

		case CMS_RUNNING:
		{
			if((nFlags & CM_FLAG_FORWARD) && !(nFlags & CM_FLAG_BACKWARD))
				szAction = "RunF";
			else if(!(nFlags & CM_FLAG_FORWARD) && (nFlags & CM_FLAG_BACKWARD))
				szAction = "RunB";
			else if((nFlags & CM_FLAG_STRAFELEFT) && !(nFlags & CM_FLAG_STRAFERIGHT))
				szAction = bFacehugger ? "RunL" : "RunF";
			else if(!(nFlags & CM_FLAG_STRAFELEFT) && (nFlags & CM_FLAG_STRAFERIGHT))
				szAction = bFacehugger ? "RunR" : "RunF";

			break;
		}

		case CMS_CLIMBING:
		{
			if((nFlags & CM_FLAG_FORWARD) || (nFlags & CM_FLAG_BACKWARD) || (nFlags & CM_FLAG_JUMP) || (nFlags & CM_FLAG_DUCK))
				szAction = "WalkF";

			break;
		}

		case CMS_CRAWLING:
		case CMS_WALLWALKING:
		{
			if((nFlags & CM_FLAG_FORWARD) && !(nFlags & CM_FLAG_BACKWARD))
				szAction = bFacehugger ? "WalkF" : ((bAlien && bRun) ? "CRunF" : "CWalkF");
			else if(!(nFlags & CM_FLAG_FORWARD) && (nFlags & CM_FLAG_BACKWARD))
				szAction = bFacehugger ? "WalkB" : ((bAlien && bRun) ? "CRunB" : "CWalkB");
			else if((nFlags & CM_FLAG_STRAFELEFT) && !(nFlags & CM_FLAG_STRAFERIGHT))
				szAction = bFacehugger ? "WalkL" : ((bAlien && bRun) ? "CRunF" : "CWalkF");
			else if(!(nFlags & CM_FLAG_STRAFELEFT) && (nFlags & CM_FLAG_STRAFERIGHT))
				szAction = bFacehugger ? "WalkR" : ((bAlien && bRun) ? "CRunF" : "CWalkF");
			else
			{
				if(bFacehugger)
				{
					szAction = "Idle";
				}
				else
				{
					szAction = "Crouch";

					if(!IsAlien(m_cmMovement.GetCharacterButes()) && (g_pLTServer->GetTime() < m_fIdleTime))
						szAction = "CAim";
				}
			}

			break;
		}

		case CMS_STAND_JUMP:
		case CMS_WALK_JUMP:
		case CMS_RUN_JUMP:
		case CMS_WWALK_JUMP:
		{
			szAction = "Jump";
			break;
		}

		case CMS_STAND_FALL:
		case CMS_WALK_FALL:
		case CMS_RUN_FALL:
		case CMS_WWALK_FALL:
		case CMS_POUNCE_FALL:
		{
			szAction = "Fall";
			break;
		}

		case CMS_STAND_LAND:
		case CMS_WALK_LAND:
		case CMS_RUN_LAND:
		case CMS_WWALK_LAND:
		{
			szAction = "Land";
			break;
		}

		case CMS_POUNCING:
		case CMS_POUNCE_JUMP:
		case CMS_FACEHUG:
		{
			szAction = "Pounce";
			break;
		}

		case CMS_FACEHUG_ATTACH:
		{
			szAction = "Facehug";
			break;
		}

		case CMS_SWIMMING:
		{
			if(bFacehugger)
			{
				szAction = "Swim";
			}
			else
			{
				if(!(nFlags & CM_FLAG_FORWARD) && (nFlags & CM_FLAG_BACKWARD))
					szAction = "SwimB";
				else
					szAction = "SwimF";
			}

			break;
		}
	}


	// Check for states that don't involve character movement state
	uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	uint32 nEMPFlags = g_pCharacterButeMgr->GetEMPFlags(m_nCharacterSet);

	if((dwUserFlags & USRFLG_CHAR_STUNNED) || ((dwUserFlags & USRFLG_CHAR_EMPEFFECT) && (nEMPFlags & EMP_FLAG_STUN)))
	{
		szAction = "Stunned";
	}


	// Set the action layer reference
	m_caAnimation.SetLayerReference(ANIMSTYLELAYER_ACTION, szAction);


	// Reset the idle time if we're not in the idle state
	if((cms != CMS_IDLE) && (cms != CMS_CRAWLING))
	{
		if(!(nFlags & CM_FLAG_FORWARD) && !(nFlags & CM_FLAG_BACKWARD) && !(nFlags & CM_FLAG_STRAFELEFT) && !(nFlags & CM_FLAG_STRAFERIGHT))
			m_fIdleTime = g_pLTServer->GetTime() + 5.0f;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateAnimation()
//
//	PURPOSE:	Update the animation information
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateAnimation()
{
	CCharacter::UpdateAnimation();

	UpdateExtWeaponAnimation();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsObjectInList
//
//	PURPOSE:	See if the object is in the list
//
// ----------------------------------------------------------------------- //

DBOOL IsObjectInList(HOBJECT *theList, DDWORD listSize, HOBJECT hTest)
{
	DDWORD i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
			return LTTRUE;
	}
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientPhysics
//
//	PURPOSE:	Determine what physics related messages to send to the client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientPhysics(LTBOOL bRestore)
{
	if (!m_hClient || !m_hObject || !m_PStateChangeFlags) return;


	// Setup the start of the physics update message
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(m_hClient, MID_PHYSICS_UPDATE);
	if(!hWrite) return;

	// Write the state flags to the message first, so we know what to read out on the other side
	g_pLTServer->WriteToMessageWord(hWrite, (uint16)m_PStateChangeFlags);


	// If we need to reset the attributes, send down the new player data
	if(m_PStateChangeFlags & PSTATE_ATTRIBUTES)
	{
		g_pLTServer->WriteToMessageByte(hWrite, (uint8)GetCharacter());
		g_pLTServer->WriteToMessageByte(hWrite, bRestore);
	}

	// Send down the container information
	if(m_PStateChangeFlags & PSTATE_SCALE)
	{
		LTFLOAT fScale = m_cmMovement.GetScale();
		g_pLTServer->WriteToMessageFloat(hWrite, fScale);
	}

	// Send down a new gravity direction
	if(m_PStateChangeFlags & PSTATE_GRAVITY)
	{
//		g_pLTServer->WriteToMessageByte(hWrite, (uint8)m_bLocalGravity);
//		g_pLTServer->WriteToMessageVector(hWrite, (uint8)m_vGravity);
	}


	// Finish up the message and reset the change flags so we don't send this info every update
	g_pLTServer->EndMessage2(hWrite, MESSAGE_GUARANTEED);
	m_PStateChangeFlags = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleGodMode()
//
//	PURPOSE:	Turns god mode on and off
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ToggleGodMode(LTBOOL bOn, LTFLOAT fMinHealth, LTBOOL bSound, LTBOOL bLeaveSolid)
{
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		if(bSound)
		{
#ifdef _WIN32
			if(bOn && !m_bGodMode)
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "sounds\\interface\\invulnerable_on.wav", 750.0f, SOUNDPRIORITY_MISC_HIGH);
			else if(!bOn && m_bGodMode)
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "sounds\\interface\\invulnerable_off.wav", 750.0f, SOUNDPRIORITY_MISC_HIGH);
#else
			if(bOn && !m_bGodMode)
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Sounds/Interface/invulnerable_on.wav", 750.0f, SOUNDPRIORITY_MISC_HIGH);
			else if(!bOn && m_bGodMode)
				g_pServerSoundMgr->PlaySoundFromObject(m_hObject, "Sounds/Interface/invulnerable_off.wav", 750.0f, SOUNDPRIORITY_MISC_HIGH);
#endif
		}
	}


	m_bGodMode = bOn;
	m_damage.SetCanDamage(!bOn, fMinHealth);


	// Set the hit box so it will or won't take hits
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		if(GetHitBox())
		{
			if(CCharacterHitBox *pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(GetHitBox())))
			{
				pHitBox->SetCanBeHit(!bOn);

				uint32 nFlags = m_cmMovement.GetObjectFlags(OFT_Flags);

				if(bOn & !bLeaveSolid)
					nFlags &= ~FLAG_SOLID;
				else
					nFlags |= FLAG_SOLID;

				// Update to the new flag values
				m_cmMovement.SetObjectFlags(OFT_Flags, nFlags, LTTRUE);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HealCheat()
//
//	PURPOSE:	Increase hit points
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HealCheat()
{
	if (m_damage.IsDead()) return;

	m_damage.Heal(m_damage.GetMaxHitPoints());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetCharacter(int nCharacterSet)
//
//	PURPOSE:	Changes the character set of the player.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetCharacter(int nCharacterSet, LTBOOL bReset)
{
	//if we have a head, lose it
	if(m_hHead)
	{
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hHead, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hHead);
		m_hHead = DNULL;
	}

	if(bReset)
	{
		m_PStateChangeFlags |= PSTATE_ATTRIBUTES | PSTATE_SCALE;
	}

	CCharacter::SetCharacter(nCharacterSet, bReset);

	if(bReset)
	{
		UpdateClientPhysics();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RepairArmorCheat()
//
//	PURPOSE:	Repair our armor
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RepairArmorCheat()
{
	if (m_damage.IsDead()) return;

	m_damage.Repair(m_damage.GetMaxArmorPoints());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullAmmoCheat()
//
//	PURPOSE:	Give us all ammo
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullAmmoCheat()
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (m_damage.IsDead()) return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	SetHasFlarePouch();
	SetHasCloakDevice();
	SetHasMask(LTTRUE, LTFALSE);
	SetHasNightVision();

	if(IsHuman(m_hObject))
	{
		WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Flare_Pouch_Weapon");
		if(pWeap)
		{
			if (pPlayerAttachments)
				pPlayerAttachments->ObtainWeapon(pWeap->nId);
		}
	}


	pPlayerAttachments->HandleCheatFullAmmo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullWeaponCheat()
//
//	PURPOSE:	Give us all weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullWeaponCheat()
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (m_damage.IsDead()) return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	SetHasFlarePouch();
	SetHasCloakDevice();
	SetHasMask(LTTRUE, LTFALSE);
	SetHasNightVision();

	if(IsHuman(m_hObject))
	{
		WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Blowtorch");
		if(pWeap && pPlayerAttachments)
			pPlayerAttachments->ObtainWeapon(pWeap->nId);

		pWeap = g_pWeaponMgr->GetWeapon("Flare_Pouch_Weapon");
		if(pWeap && pPlayerAttachments)
			pPlayerAttachments->ObtainWeapon(pWeap->nId);

		pWeap = g_pWeaponMgr->GetWeapon("MarineHackingDevice");
		if(pWeap && pPlayerAttachments)
			pPlayerAttachments->ObtainWeapon(pWeap->nId);
	}

	if(IsPredator(m_hObject))
	{
		WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Energy_Sift");
		if(pWeap && pPlayerAttachments)
			pPlayerAttachments->ObtainWeapon(pWeap->nId);

		pWeap = g_pWeaponMgr->GetWeapon("Medicomp");
		if(pWeap && pPlayerAttachments)
			pPlayerAttachments->ObtainWeapon(pWeap->nId);
	}

	//load up the battery power
	m_fBatteryChargeLevel = MARINE_MAX_BATTERY_LEVEL;

	if (pPlayerAttachments)
		pPlayerAttachments->HandleCheatFullWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullModsCheat()
//
//	PURPOSE:	Give us all mods for currently carried weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullModsCheat()
{
	if (m_damage.IsDead()) return;

//	dynamic_cast<CPlayerAttachments*>(m_pAttachments)->HandleCheatFullMods();
}


// ----------------------------------------------------------------------- //

static LTBOOL SpawnPointFilter(HOBJECT hObj, void *pUserData)
{
	if(!hObj) return LTFALSE;

	// Force return on non-solids... even the one's with RAYHIT set
	uint32 nFlags;
	g_pLTServer->Common()->GetObjectFlags(hObj, OFT_Flags, nFlags);

	if(!(nFlags & FLAG_SOLID))
		return LTFALSE;

	// Ignore characters and players
	HCLASS clsCharacter = g_pLTServer->GetClass("CCharacter");

	if(g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), clsCharacter))
		return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Respawn()
//
//	PURPOSE:	Respawn the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Respawn(uint8 nServerLoadGameFlags)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (!m_hClient || !g_pWeaponMgr || m_dwLastLoadFlags == LOAD_RESTORE_GAME) return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	HMESSAGEWRITE hMessage;

	// Make sure we're an ok character for multiplayer...
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		if(!g_pGameServerShell->GetMultiplayerMgr()->ValidateRespawn(this))
			return;
	}
	else
	{
		// Restore disc ammo if it was lost on a previous single player level
		if (GetCharacterClass() == PREDATOR)
		{
			WEAPON *pWeapon = g_pWeaponMgr->GetWeapon("Disc");
			if (pWeapon && pPlayerAttachments)
			{
				CWeapons * const pWeapons = pPlayerAttachments->GetWeapons();
				if (pWeapons && pWeapons->HasWeapon(pWeapon->nId))
				{
					pWeapons->SetAmmoPool("Discs", 1);
				}
			}
		}
	}

	m_cc = g_pCharacterButeMgr->GetClass(m_nCharacterSet);


	// Make sure the attachments are created all the time
	CreateAttachments();
	if(m_pAttachments)
	{
		m_pAttachments->Init(m_hObject);
	}


	// Are we starting a new game...
	if(g_pGameServerShell->GetGameType() != SP_STORY)
		m_bNewGame = LTFALSE;
	else
		m_bNewGame = (nServerLoadGameFlags == LOAD_NEW_GAME);

	// Force the cloak off
	ResetCloak();

	// Get a start point...
	int nClass = g_pCharacterButeMgr->GetClass(m_nCharacterSet);

	// Get the special case queen character set...
	int nQueenCharSet = g_pCharacterButeMgr->GetSetFromModelType("queen");

	switch(nClass)
	{
		case MARINE:	nClass = Marine;	break;
		case ALIEN:		nClass = Alien;		break;
		case PREDATOR:	nClass = Predator;	break;
		case CORPORATE:	nClass = Corporate;	break;
	}

	GameStartPoint* pStartPt = g_pGameServerShell->FindStartPoint(LTNULL, nClass, this, (m_nCharacterSet == (uint32)nQueenCharSet));

	LTVector vPos(0.0f, 0.0f, 0.0f);
	LTVector vVec(0.0f, 0.0f, 0.0f);

	if(pStartPt)
	{
		// Set our starting position...
		g_pLTServer->GetObjectPos(pStartPt->m_hObject, &vPos);
		vVec = pStartPt->GetPitchYawRoll();


		// If we're in single player story mode, set the character to what the start point says
		if(g_pGameServerShell->GetGameType() == SP_STORY)
		{
			SetCharacter(pStartPt->GetCharacterSet());

			// Since the character may not be in the level yet, we need to reset our
			// special fx as well.
			CreateSpecialFX();

			m_bLevelStarted = LTFALSE;

			// See about un-muting sounds..
			if(!pStartPt->HasOpeningCinematic())
			{
				// Tell the client to go ahead with the sounds...
				if (m_hClient)
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_UNMUTE_SOUND);
					g_pLTServer->EndMessage(hMessage);
				}
			}
			else
			{
				// Tell the server it about our mute state...
				g_pGameServerShell->SetMuteState(LTTRUE);
			}
		}
		else
		{
			SetCharacter(m_nCharacterSet);

			// Always allow sounds in MP
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_UNMUTE_SOUND);
			g_pLTServer->EndMessage(hMessage);
		}


		// Get start point trigger info...
		HSTRING hstrTarget = pStartPt->GetTriggerTarget();
		HSTRING hstrMessage = pStartPt->GetTriggerMessage();

		if (hstrTarget && hstrMessage)
		{
			if (m_hstrStartLevelTriggerTarget)
			{
				g_pLTServer->FreeString(m_hstrStartLevelTriggerTarget);
			}

			if (m_hstrStartLevelTriggerMessage)
			{
				g_pLTServer->FreeString(m_hstrStartLevelTriggerMessage);
			}
				
			m_hstrStartLevelTriggerTarget	= g_pLTServer->CopyString(hstrTarget);
			m_hstrStartLevelTriggerMessage	= g_pLTServer->CopyString(hstrMessage);
		}

		// Get start point's start mission trigger info...
		if (m_hstrStartMissionTriggerTarget)
		{
			g_pLTServer->FreeString(m_hstrStartMissionTriggerTarget);
			m_hstrStartMissionTriggerTarget = LTNULL;
		}

		if (m_hstrStartMissionTriggerMessage)
		{
			g_pLTServer->FreeString(m_hstrStartMissionTriggerMessage);
			m_hstrStartMissionTriggerMessage = LTNULL;
		}

		if( g_pGameServerShell->GetGameType().IsStartMission() )
		{
			m_hstrStartMissionTriggerTarget = g_pLTServer->CopyString( pStartPt->GetMissionTarget());
			m_hstrStartMissionTriggerMessage = g_pLTServer->CopyString( pStartPt->GetMissionMessage());
		}
	} 


	// Reposition the destination spawn location to be resting on the floor
	LTVector vDims = m_cmMovement.GetDimsSetting(CM_DIMS_DEFAULT);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_Direction = LTVector(0.0f, -1.0f, 0.0f);

	IQuery.m_Flags	   = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	IQuery.m_FilterFn  = SpawnPointFilter;
	IQuery.m_pUserData = NULL;

    if(g_pLTServer->CastRay(&IQuery, &IInfo))
	{
        LTFLOAT fDist = vPos.y - IInfo.m_Point.y;

		if(fDist > vDims.y)
		{
			vPos.y -= (fDist - (vDims.y + 1.0f));
		}
	}


	// Setup the object flags for teleporting to the location
	uint32 dwFlags;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | FLAG_GOTHRUWORLD);

	g_pLTServer->TeleportObject(m_hObject, &vPos);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags & ~FLAG_GOTHRUWORLD);


	LTRotation rRot;
	g_pMathLT->SetupEuler(rRot, vVec.x, vVec.y, vVec.z);
	m_cmMovement.SetRot(rRot);



	// Make sure that anyone else that's at this location gets punished!
	TeleFragObjects(vPos);


	// Set the initial spawn invulnerability
	if(g_pGameServerShell->GetGameType().IsMultiplayer())
	{
		// Get the current position of the player...
		m_vSpawnPosition = vPos;
		m_fSpawnTime = g_pLTServer->GetTime();

		ToggleGodMode(LTTRUE, -1.0f, LTTRUE, LTTRUE);

		// set our spawn-in flag
		m_bSpawnInState = LTTRUE;

		// Reset the inventory here so it'll get reset below with the default weapons
		hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_RESET_INVENTORY_ID);
		g_pLTServer->EndMessage(hMessage);

		// Make sure all the net stuff is cleaned up...
		m_eNetStage = NET_STAGE_INVALID;
		m_eLastNetStage = NET_STAGE_INVALID;

		// Let the animation go...
		m_caAnimation.StopAnim();
		m_caAnimation.Enable(LTTRUE);
	}


	// Inform the client of the correct camera/player orientation...
	hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_ORIENTATION);
	g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
	g_pLTServer->WriteToMessageVector(hMessage, &vVec);
	g_pLTServer->EndMessage(hMessage);


	// If we're starting a new game, or we're not in story mode... reset everything
	if(g_pGameServerShell->GetGameType() != SP_STORY || m_bNewGame)
	{
		// Reset the character
		Reset();

		// Reset all the weapons
		if (pPlayerAttachments)
			pPlayerAttachments->ResetAllWeapons();
	}

	// Make sure we always have and start with the default weapon...
	AquireDefaultWeapon();

	UpdateInterface(LTTRUE, LTTRUE);


	// This MUST be called after ChangeState and changing weapons if we
	// want the weapons to be correctly auto-saved...
	if(g_pGameServerShell->GetGameType() == SP_STORY)
	{
#ifndef _DEMO
		// Set autosave flag
		m_bDoAutoSave = LTTRUE;
#endif
	}
	else  // Multiplayer...
	{
		m_bWaitingForAutoSave = DFALSE;

		// Update their view position so they get the sfx message.
		g_pLTServer->SetClientViewPos(m_hClient, &vPos);

		// Give the player some default equipment
		SetHasFlarePouch();
		SetHasCloakDevice();
		SetHasMask(LTTRUE, LTFALSE);
		SetHasNightVision();

		//load up the battery power
		m_fBatteryChargeLevel = MARINE_MAX_BATTERY_LEVEL;
	}

	if(g_pGameServerShell->GetGameType() == SP_STORY)
	{
		UpdateClientPhysics(); // (To make sure they have their model around)
		TeleportClientToServerPos(1.0f);
		ChangeState(PS_ALIVE);

		// Inform the client that we've respawned...
		hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_RESPAWN);
		g_pLTServer->EndMessage(hMessage);
	}
	else
	{
		// Get our position
		LTVector myPos;
		g_pLTServer->GetObjectPos(m_hObject, &myPos);

		// Tell the player about the new move code.
		++m_ClientMoveCode;

		// Set the new player state
		m_eState = PS_ALIVE;

		// Now do the MP respawn thing...
		hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_RESPAWN);
		g_pLTServer->WriteToMessageByte(hMessage, (uint8)GetCharacter());
		g_pLTServer->WriteToMessageByte(hMessage, m_ClientMoveCode);
		g_pLTServer->WriteToMessageVector(hMessage, &myPos);
		g_pLTServer->EndMessage(hMessage);
	}


	// Remove any models we may have loaded from a keepalive save.
	if(g_pGameServerShell->GetGameType() == SP_STORY)
		g_pLTServer->FreeUnusedModels( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Teleport()
//
//	PURPOSE:	Teleport the player to the specified point
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Teleport(LTVector &vPos)
{
	g_pLTServer->TeleportObject(m_hObject, &vPos);
	UpdateClientPhysics();
	TeleportClientToServerPos();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Teleport()
//
//	PURPOSE:	Teleport the player to the specified point
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Teleport(const char* pTeleportPointName, LTBOOL bRotate)
{
	if (!pTeleportPointName) return;

	TeleportPoint* pTeleportPt = DNULL;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	g_pLTServer->FindNamedObjects(const_cast<char*>(pTeleportPointName),objArray);

	if(objArray.NumObjects())
		pTeleportPt = dynamic_cast<TeleportPoint*>(g_pLTServer->HandleToObject(objArray.GetObject(0)));
	else
		return;

	if (!pTeleportPt) return;


	// Set our starting values...

	LTVector vPos;
	g_pLTServer->GetObjectPos(pTeleportPt->m_hObject, &vPos);


	// Inform the client of the correct camera/player orientation...

	if(bRotate)
	{
		LTVector vVec;
		vVec = pTeleportPt->GetPitchYawRoll();

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_ORIENTATION);
		g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
		g_pLTServer->WriteToMessageVector(hMessage, &vVec);
		g_pLTServer->EndMessage(hMessage);
	}


	g_pLTServer->TeleportObject(m_hObject, &vPos);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	UpdateClientPhysics();
	TeleportClientToServerPos();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoAutoSave()
//
//	PURPOSE:	Tell the client to auto-save the game...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoAutoSave()
{
	if (!m_hClient) return;
	
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_AUTOSAVE);
	g_pLTServer->EndMessage(hMessage);

	// Wait until the save occurs to process updates...

	m_bWaitingForAutoSave = LTTRUE;
	m_bDoAutoSave = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Reset()
//
//	PURPOSE:	Reset (after death)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Reset()
{
	CCharacter::Reset();

	m_damage.SetMaxHitPoints(g_pCharacterButeMgr->GetMaxHitPoints(GetCharacter()));
	m_damage.SetMaxArmorPoints(g_pCharacterButeMgr->GetMaxArmorPoints(GetCharacter()));

	m_damage.Reset(g_pCharacterButeMgr->GetDefaultHitPoints(GetCharacter()), 
		           g_pCharacterButeMgr->GetDefaultArmorPoints(GetCharacter()));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AquireDefaultWeapon()
//
//	PURPOSE:	Give us the default weapon
//
// ----------------------------------------------------------------------- //

int CPlayerObj::AquireDefaultWeapon(LTBOOL bChangeWeapon)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;
	int i = 0;
	uint32 nWeaponSet;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	LTBOOL bMultiplayer = g_pGameServerShell->GetGameType().IsMultiplayer();
	
	if(bMultiplayer)
	{
		if(g_pGameServerShell->GetGameInfo()->m_bClassWeapons)
			nWeaponSet = g_pCharacterButeMgr->GetMPClassWeaponSet(GetCharacter());
		else
			nWeaponSet = g_pCharacterButeMgr->GetMPWeaponSet(GetCharacter());
	}
	else
		nWeaponSet = g_pCharacterButeMgr->GetWeaponSet(GetCharacter());


	WEAPON_SET *pWeaponSet = g_pWeaponMgr->GetWeaponSet(nWeaponSet);

	if(!pWeaponSet) return 0;

	for(i=0 ; i<pWeaponSet->nNumWeapons ; i++)
	{
		if(pWeaponSet->bDefaultWeapon[i])
		{
			WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pWeaponSet->szWeaponName[i]);
			if (pWeaponData)
			{
				BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWeaponData->nDefaultBarrelType);

				if(pBarrel)
				{
					AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);
					if (pAmmoData)
					{
						AMMO_POOL* pPoolData = g_pWeaponMgr->GetAmmoPool(pAmmoData->szAmmoPool);
						
						int nAmt = pPoolData?pPoolData->nDefaultAmount:1;

						if(pPlayerAttachments)
							pPlayerAttachments->ObtainWeapon(pWeaponData->nId, pBarrel->nId, pPoolData->nId, nAmt, LTTRUE, LTFALSE);
					}
				}
			}
		}
	}

	CWeapons *pWeapons = LTNULL;
	if (pPlayerAttachments)
		pWeapons = pPlayerAttachments->GetWeapons();

	for(i=0 ; i<pWeaponSet->nNumWeapons ; i++)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(pWeaponSet->szWeaponName[i]);

		if(!pWeapon) break;

		BARREL* pBarrel = g_pWeaponMgr->GetBarrel(pWeapon->aBarrelTypes[PRIMARY_BARREL]);

		if(!pBarrel) break;

		if(pBarrel->bInfiniteAmmo) break;

		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pBarrel->nAmmoType);

		if(!pAmmo) break;

		if(pWeapons && pWeapons->GetAmmoCount(pAmmo->nId))
			break;
	}

	if(IsHuman( GetCharacter()) || IsSynthetic(GetCharacter()) )
	{
		SetHasFlarePouch();
		
		//load up on flares
		WEAPON* pWep = g_pWeaponMgr->GetWeapon("Flare_Pouch_Weapon");
		if(pWep)
		{
			AMMO* pAmmo = g_pWeaponMgr->GetAmmo("Flare_Pouch_Ammo");
			if(pAmmo)
			{
				AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);
				if(pPool)
				{
					HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_ADDWEAPON);
					g_pLTServer->WriteToMessageByte(hMessage, pWep->nId);
					g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
					g_pLTServer->WriteToMessageFloat(hMessage, 0);
					g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
					g_pLTServer->EndMessage(hMessage);
				}
			}
		}
	}

	// Give up the predator default stuff...
	if( IsPredator(GetCharacter()))
	{
		// In all cases, give full energy to start with...
		AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Predator_Energy");
		if(pPool)
		{
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_AMMO);
			g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
			g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)pPool->GetMaxAmount(m_hObject));
			g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
			g_pLTServer->EndMessage(hMessage);
		}

		// Multiplayer stuff...
		if(bMultiplayer)
		{
			//load up the medicomp....
			WEAPON* pWep = g_pWeaponMgr->GetWeapon("Medicomp");
			if(pWep)
			{
				AMMO* pAmmo = g_pWeaponMgr->GetAmmo("Medicomp_Ammo");
				if(pAmmo)
				{
					AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);
					if(pPool)
					{
						HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_ADDWEAPON);
						g_pLTServer->WriteToMessageByte(hMessage, pWep->nId);
						g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
						g_pLTServer->WriteToMessageFloat(hMessage, 0);
						g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
						g_pLTServer->EndMessage(hMessage);
					}
				}
			}

			//load up the charger...
			pWep = g_pWeaponMgr->GetWeapon("Energy_Sift");
			if(pWep)
			{
				AMMO* pAmmo = g_pWeaponMgr->GetAmmo("Energy_Sift_Ammo");
				if(pAmmo)
				{
					AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);
					if(pPool)
					{
						HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_ADDWEAPON);
						g_pLTServer->WriteToMessageByte(hMessage, pWep->nId);
						g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
						g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)pAmmo->GetMaxPoolAmount(m_hObject));
						g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
						g_pLTServer->EndMessage(hMessage);
					}
				}
			}
		}
		else
		{
			// Single player stuff...

			//load up the Hacker...
			WEAPON* pWep = g_pWeaponMgr->GetWeapon("PredatorHackingDevice");
			if(pWep)
			{
				// Any ammo will do since there is no ammo given here...
				AMMO* pAmmo = g_pWeaponMgr->GetAmmo("PredatorHackingDevice_Ammo");
				if(pAmmo)
				{
					AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool(pAmmo->szAmmoPool);
					if(pPool)
					{
						HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_ADDWEAPON);
						g_pLTServer->WriteToMessageByte(hMessage, pWep->nId);
						g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
						g_pLTServer->WriteToMessageFloat(hMessage, 0);
						g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
						g_pLTServer->EndMessage(hMessage);
					}
				}
			}
		}
	}

	// Give the exosuit full energy by default...
	if(IsExosuit(GetCharacter()))
	{
		AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Exosuit_Energy");
		if(pPool)
		{
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, m_hObject, MID_AMMO);
			g_pLTServer->WriteToMessageByte(hMessage, pPool->nId);
			g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)pPool->GetMaxAmount(m_hObject));
			g_pLTServer->WriteToMessageByte(hMessage, LTFALSE);
			g_pLTServer->EndMessage(hMessage);
		}
	}

	
	if(bChangeWeapon)
		ChangeWeapon(g_pWeaponMgr->GetCommandId(i), LTFALSE, LTFALSE);

	return i;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDead()
//
//	PURPOSE:	Tell client I died
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDead(DBOOL)
{
	if (!m_hObject) return;

	if((m_eState != PS_DEAD) && (m_eState != PS_DYING) && (m_eState != PS_OBSERVING))
	{
		// Go ahead and force invisible here in hopes of making sure that nobody
		// ever see 'two' of you.
		m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) & ~FLAG_VISIBLE, LTTRUE);


		// HandleDead will remove this object.
		CCharacter::HandleDead(DFALSE);

		// This will send the second message down to the client
		// telling them that we are in fact dead (the PS_DYING setup message
		// should have been sent by this point)
		ChangeState(PS_DEAD);

		// If this is MP do something specail...
//		if(0)//g_pGameServerShell->GetGameType() != SP_STORY)
//		{
//			int nQueenCharSet = g_pCharacterButeMgr->GetSetFromModelType("queen");
//
//			if(m_nCharacterSet == (uint32)nQueenCharSet)
//			{
//				g_pGameServerShell->GetMultiplayerMgr()->ResetQueen(m_hClient);
//			}
//
//			// Just to be sure...
//			m_cmMovement.SetObjectFlags(OFT_Flags, m_cmMovement.GetObjectFlags(OFT_Flags) & ~FLAG_VISIBLE, LTTRUE);
//		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::StartDeath()
//
//	PURPOSE:	Tell client I'm dying
//
// ----------------------------------------------------------------------- //

void CPlayerObj::StartDeath()
{
	// This will create the body prop and drop weapon powerups...
	CCharacter::StartDeath();

	// This sends the initial dying message down to the client so we
	// can setup to start of our death
	ChangeState(PS_DYING);

	// Play music to accompany player's death.
	g_pMusicMgr->PlayPlayerDeathEvent( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeState()
//
//	PURPOSE:	Notify Client of changed state
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeState(PlayerState eState, HOBJECT hSwapObject /* = LTNULL */ )
{
	if (!m_hClient) return;

	m_eState = eState;

	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_STATE_CHANGE);
	g_pLTServer->WriteToMessageByte(hMessage, m_eState);

	if( eState == PS_SWAPPED )
		g_pLTServer->WriteToMessageObject(hMessage, hSwapObject);

	if( eState == PS_DYING)
		g_pLTServer->WriteToMessageByte(hMessage, m_damage.GetLastDamageType());

	g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CreateAIPlayer()
//
//	PURPOSE:	Creates a copy of the player that can be scripted for
//				cut-scenes and such.
//
// ----------------------------------------------------------------------- //

static HOBJECT CreateAIPlayer(const CharacterButes & butes)
{
	if(g_pGameServerShell->GetGameType() != SP_STORY)
		return LTNULL;

	// Fail if the AI bute does not exist.
	CString cstrAIButeName;
	cstrAIButeName.Format("%s_AI", butes.m_szName);
	if( g_pCharacterButeMgr->GetSetFromModelType(cstrAIButeName) < 0 )
	{
		return LTNULL;
	}

	HOBJECT hResult = LTNULL;

	ObjectCreateStruct theStruct;

	const char * szAttributeTemplate = LTNULL;
	switch( butes.m_eCharacterClass )
	{
	default:
		_ASSERT( 0 );
	case MARINE:
		szAttributeTemplate = "BasicMarine";
		break;

	case CORPORATE:
		szAttributeTemplate = "BasicCorporate";
		break;

	case PREDATOR:
		szAttributeTemplate = "BasicPredator";
		break;

	case ALIEN:
		szAttributeTemplate = "BasicAlien";
		break;

	case MARINE_EXOSUIT:
		szAttributeTemplate = "BasicMarineExosuit";
		break;
	case CORPORATE_EXOSUIT:
		szAttributeTemplate = "BasicExosuit";
		break;
	case SYNTHETIC:
		szAttributeTemplate = "BasicCorporate";
		break;
	}

	char szProps[256];
	sprintf(szProps,"Name AIPlayer; CharacterType %s; AttributeTemplate %s; "
		            "Weapon %s; SecondaryWeapon %s; TertiaryWeapon %s", 
					butes.m_szName,
					szAttributeTemplate,
					g_szNone,
					g_szNone,
					g_szNone );

	HCLASS hClass = g_pLTServer->GetClass("AIPlayer");
	AIPlayer * pAIPlayer = dynamic_cast<AIPlayer*>(g_pLTServer->CreateObjectProps(hClass, &theStruct, szProps));
	_ASSERT( pAIPlayer );
	if( pAIPlayer )
	{
		hResult = pAIPlayer->m_hObject;
		
		pAIPlayer->SetInvincible(LTTRUE);

		pAIPlayer->Active(false);
	}

	return hResult;
}

#ifndef _FINAL
void VerifyNodes(HOBJECT hObj, const CharacterButes* pButes, ModelSkeleton eModelSkeleton)
{
	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	const char* szNodeName = DNULL;

	HMODELNODE hNode = INVALID_MODEL_NODE;
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	const char* pNode = LTNULL;

	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		// Get the name of the node
		pNode = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, (ModelNode)iNode);

		if(g_pModelLT->GetNode(hObj, const_cast<char*>(pNode), hNode) == LT_OK)
		{
			continue;
		}
		else if (g_pModelLT->GetSocket(hObj, const_cast<char*>(pNode), hSocket) == LT_OK)
		{
			continue;
		}
		else
		{
			g_pLTServer->CPrint("Model %s: Missing Node or socket %s", pButes->m_szName, pNode);
		}
	}
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetCharacter()
//
//	PURPOSE:	Reset the player values
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetCharacter()
{
	// Reset the model...
	CCharacter::ResetCharacter();

#ifndef _FINAL
	// Good saftey check here...
	VerifyNodes(m_hObject, GetButes(), (ModelSkeleton)GetButes()->m_nSkeleton);
#endif

	if(m_bNewGame)
		ResetPlayerHealth();
	
	m_cc = GetButes()->m_eCharacterClass;

	// This needs to happen here becuase
	// CharacterMgr relies on m_cc to determine
	// our relationship with other AI's.
	g_pCharacterMgr->Add(this);
	
	// Un swap us if we are swapped.
	if( m_bSwapped )
	{
		Swap(LTFALSE);
	}
	
	if( m_hAIPlayer )
	{
		g_pLTServer->RemoveObject( m_hAIPlayer );
	}
	
	m_hAIPlayer =  CreateAIPlayer(*GetButes());
	
	// Cache all the files I am going to need...
	// This is for predators and only in single player
	// all character skins are chached in MP automatically
	if(IsPredator(m_hObject) && g_pGameServerShell->GetGameType() == SP_STORY)
	{
		for(CCharacterMgr::GlobalCharacterIterator iter = g_pCharacterMgr->BeginGlobalCharacters();
		iter != g_pCharacterMgr->EndGlobalCharacters(); ++iter )
		{
			CCharacter * const pChar = *iter;
			
			if(pChar)
				pChar->CacheFiles();
		}
	}

	// Now go through all the multispawners and load up those textures...
	if(g_pGameServerShell->GetGameType() == SP_STORY)
		CacheMultispawners();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ComputeDamageModifier()
//
//	PURPOSE:	Does not damage modification if single-player mode.
//
// ----------------------------------------------------------------------- //

LTFLOAT	CPlayerObj::ComputeDamageModifier(ModelNode eModelNode) const
{
	const LTFLOAT fResult = CCharacter::ComputeDamageModifier(eModelNode);

	if( fResult > 0.0f && g_pGameServerShell->GetGameType() == SP_STORY)
		return 1.0f;

	return fResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetPlayerHealth()
//
//	PURPOSE:	Reset the player health and armor
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetPlayerHealth()
{
	if(!m_hClient) return;

	m_damage.SetMaxHitPoints(g_pCharacterButeMgr->GetMaxHitPoints(GetCharacter()));
	m_damage.SetHitPoints(g_pCharacterButeMgr->GetDefaultHitPoints(GetCharacter()));
	m_damage.SetMaxArmorPoints(g_pCharacterButeMgr->GetMaxArmorPoints(GetCharacter()));
	m_damage.SetArmorPoints(g_pCharacterButeMgr->GetDefaultArmorPoints(GetCharacter()));

	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
	g_pLTServer->WriteToMessageByte(hMessage, IC_MAX_ARMOR_ID);
	g_pLTServer->WriteToMessageFloat(hMessage, m_damage.GetMaxArmorPoints());
	g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

	hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
	g_pLTServer->WriteToMessageByte(hMessage, IC_MAX_HEALTH_ID);
	g_pLTServer->WriteToMessageFloat(hMessage, m_damage.GetMaxHitPoints());
	g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeWeapon
//
//	PURPOSE:	Tell the client to change the weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeWeapon(DBYTE nCommandId, DBOOL bAuto, LTBOOL bPlaySelectSound)
{
	if (!m_hClient) return;

	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_WEAPON_CHANGE);
	g_pLTServer->WriteToMessageByte(hMessage, nCommandId);
	g_pLTServer->WriteToMessageByte(hMessage, bAuto);
	g_pLTServer->WriteToMessageByte(hMessage, bPlaySelectSound);
	g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeWeapon
//
//	PURPOSE:	Tell the client to change the weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeWeapon(const char *szWeapon, LTBOOL bDeselect)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (!m_hClient || !m_pAttachments) return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(szWeapon);

	if(!pWeapon) return;

	if (pPlayerAttachments)
		pPlayerAttachments->ObtainWeapon(pWeapon->nId);

	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_FORCE_WEAPON_CHANGE);
	g_pLTServer->WriteToMessageByte(hMessage, pWeapon->nId);
	g_pLTServer->WriteToMessageByte(hMessage, bDeselect);
	g_pLTServer->WriteToMessageByte(hMessage, bDeselect); // Same goes for sound...  Don't ask!
	g_pLTServer->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoWeaponChange
//
//	PURPOSE:	Change our weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponChange(DBYTE nWeaponId)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (!m_hClient || !m_pAttachments) return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);
	if (pPlayerAttachments)
		pPlayerAttachments->ChangeWeapon(nWeaponId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateInterface
//
//	PURPOSE:	Tell the client of about any changes
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateInterface(DBOOL bForceUpdate, LTBOOL bGuaranteed)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (!m_hClient || !g_pWeaponMgr || !m_pAttachments) return; //m_damage.IsDead() || 

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	// Create the send flags
	uint32 nSendFlags = MESSAGE_NAGGLE;

	if(bGuaranteed)
		nSendFlags = MESSAGE_GUARANTEED;


	// See if the ammo has changed...
	DBYTE nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();

	for (int i=0; i < nNumAmmoPools; i++)
	{
		int nAmmo = 0;

		if (pPlayerAttachments)
			nAmmo = pPlayerAttachments->GetAmmoPoolCount(i);

		if (m_pnOldAmmo)
		{
			if (m_pnOldAmmo[i] != nAmmo || bForceUpdate)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
				g_pLTServer->WriteToMessageByte(hMessage, IC_AMMO_ID);
				g_pLTServer->WriteToMessageByte(hMessage, i);
				g_pLTServer->WriteToMessageFloat(hMessage, (DFLOAT)nAmmo);
				g_pLTServer->EndMessage2(hMessage, nSendFlags);
			}

			m_pnOldAmmo[i] = nAmmo;
		}
	}

	// See if max health has changed...

	if (m_fOldMaxHitPts != m_damage.GetMaxHitPoints() || bForceUpdate)
	{
		m_fOldMaxHitPts = m_damage.GetMaxHitPoints();

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_MAX_HEALTH_ID);
		g_pLTServer->WriteToMessageFloat(hMessage, m_fOldMaxHitPts);
		g_pLTServer->EndMessage2(hMessage, nSendFlags);
	}

	// See if health has changed...

	if (m_fOldHitPts != m_damage.GetHitPoints() || bForceUpdate)
	{
		m_fOldHitPts = m_damage.GetHitPoints();

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_HEALTH_ID);
		g_pLTServer->WriteToMessageFloat(hMessage, m_fOldHitPts);
		g_pLTServer->EndMessage2(hMessage, nSendFlags);
	}


	// See if max armor has changed...

	if (m_fOldMaxArmor != m_damage.GetMaxArmorPoints() || bForceUpdate)
	{
		m_fOldMaxArmor = m_damage.GetMaxArmorPoints();

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_MAX_ARMOR_ID);
		g_pLTServer->WriteToMessageFloat(hMessage, m_fOldMaxArmor);
		g_pLTServer->EndMessage2(hMessage, nSendFlags);
	}

	// See if armor has changed...

	if (m_fOldArmor != m_damage.GetArmorPoints() || bForceUpdate)
	{
		m_fOldArmor = m_damage.GetArmorPoints();

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_ARMOR_ID);
		g_pLTServer->WriteToMessageFloat(hMessage, m_fOldArmor);
		g_pLTServer->EndMessage2(hMessage, nSendFlags);
	}

	
	// See if air level has changed...
	LTFLOAT fTotalAirTime = GetButes()->m_fAirSupplyTime;

	if ((fTotalAirTime != -1.0f) && ((m_fAirSupplyTime != m_fOldAirSupplyTime) || bForceUpdate))
	{
		LTFLOAT fPercent = m_fAirSupplyTime / fTotalAirTime;

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_AIRLEVEL_ID);
		g_pLTServer->WriteToMessageFloat(hMessage, fPercent);
		g_pLTServer->EndMessage2(hMessage, nSendFlags);

		m_fOldAirSupplyTime = m_fAirSupplyTime;
	}

	// See if charge level has changed...

	if (bForceUpdate)
	{
		LTFLOAT fLevel = (LTFLOAT)m_nCannonChargeLevel;

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_CANNON_CHARGE_ID);
		g_pLTServer->WriteToMessageFloat(hMessage, fLevel);
		g_pLTServer->EndMessage2(hMessage, nSendFlags);
	}

	// See if battery charge level has changed...

	if (m_fOldBatteryLevel != m_fBatteryChargeLevel || bForceUpdate)
	{
		// reset our old marker
		m_fOldBatteryLevel = m_fBatteryChargeLevel;

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
		g_pLTServer->WriteToMessageByte(hMessage, IC_BATTERY_CHARGE_ID);
		g_pLTServer->WriteToMessageFloat(hMessage, m_fBatteryChargeLevel);
		g_pLTServer->EndMessage2(hMessage, nSendFlags);
	}
}



LTBOOL ActivateFilterFn(HOBJECT hObj, void *pUserData)
{
	// If you modify this, please update its copy in ClientShellDLL\InterfaceMgr.cpp

	if(!hObj) return LTFALSE;
	if(g_pPhysicsLT->IsWorldObject(hObj) == LT_YES) return LTTRUE;

	// If we're a player object... ignore it (this doesn't need to be in the InterfaceMgr copy)
	if(IsPlayer(hObj)) return LTFALSE;


	// Ignore non-solid, non-activatable objects.
	if( GetActivateType(hObj) == AT_NOT_ACTIVATABLE )
	{
		const uint32 nFlags = g_pLTServer->GetObjectFlags(hObj);

		if( !(nFlags & FLAG_SOLID) )
			return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Activate()
//
//	PURPOSE:	Activate the object in front of us (return true if an
//				object was activated)
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::Activate(LTVector vPos, LTVector vDir)
{
	// First see if a cinematic is running...If so, try and stop it...

	if (PlayingCinematic(LTTRUE))
	{
		return DFALSE;
	}

	if(g_pGameServerShell->GetGameType().IsMultiplayer() && IsExosuit(m_cmMovement.GetCharacterButes()))
	{
		//time to get out of the suit
		HandleExosuitEject();

		return LTFALSE;
	}
	// Cast ray to see if there is an object to activate...
			
	LTVector vDims, vTemp, vPos2;
	g_pLTServer->GetObjectDims(m_hObject, &vDims);


	DFLOAT fDist = (vDims.x + vDims.z)/2.0f + g_pCharacterButeMgr->GetActivateDistance(GetCharacter());
	VEC_MULSCALAR(vTemp, vDir, fDist);
	VEC_ADD(vPos2, vPos, vTemp);
    
	IntersectQuery IQuery;
	memset(&IQuery, 0, sizeof(IQuery));

	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To = vPos2;
	IQuery.m_FilterFn  = ActivateFilterFn;
    
	IQuery.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS;

	if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if (LT_YES == g_pLTServer->Physics()->IsWorldObject(IInfo.m_hObject))
		{
			if (IInfo.m_hPoly != INVALID_HPOLY)
			{
				SurfaceType eType = GetSurfaceType(IInfo.m_hPoly);
				SURFACE *pSurf = g_pSurfaceMgr->GetSurface(eType);

				// See if the surface we tried to activate has an activation 
				// sound...If so, play it...

				if (pSurf && pSurf->szActivationSnd[0] && pSurf->fActivationSndRadius > 0)
				{
					g_pServerSoundMgr->PlaySoundFromPos(IInfo.m_Point, pSurf->szActivationSnd, 
						pSurf->fActivationSndRadius, SOUNDPRIORITY_PLAYER_LOW);
				}
			}
		}
		else if (IInfo.m_hObject)
		{
			HSTRING hStr;
			hStr = g_pLTServer->CreateString("ACTIVATE");

			SendTriggerMsgToObject(this, IInfo.m_hObject, hStr);
			g_pLTServer->FreeString(hStr);

			return LTTRUE;
		}
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayingCinematic()
//
//	PURPOSE:	See if we are playing a cinematic (and stop it if specified)
//
// ----------------------------------------------------------------------- //

DBOOL CPlayerObj::PlayingCinematic(DBOOL bStopCinematic)
{
	// Search for cinematic object if we need to stop it...

	LTBOOL bExternalCameraActive = IsExternalCameraActive();
	if (bExternalCameraActive && bStopCinematic)
	{
		// Stop all the CinematicTriggers that are currently active...
		HSTRING hstrMessage = g_pLTServer->CreateString("OFF");

		for( CinematicTrigger::CinematicTriggerIterator iter = CinematicTrigger::BeginCinematicTriggers();
		     iter != CinematicTrigger::EndCinematicTriggers(); ++iter )
		{
			if( (*iter)->m_hObject )
			{
				SendTriggerMsgToObject(this, (*iter)->m_hObject, hstrMessage);
			}
		}

		bExternalCameraActive = IsExternalCameraActive();

		g_pLTServer->FreeString(hstrMessage);
	}

	return bExternalCameraActive;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CPlayerObj::ProcessDamageMsg(const DamageStruct & damage_msg)
{
	if (!m_hClient) return;

	CCharacter::ProcessDamageMsg(damage_msg);

	// Tell the client about the damage...

	DFLOAT fDamage = m_damage.GetLastDamage();
	if (fDamage > 0.0f)
	{
		DFLOAT fPercent = fDamage / m_damage.GetMaxHitPoints();
		if(fPercent > 1.0f) fPercent = 1.0f;

		LTVector vDir;
		VEC_COPY(vDir, m_damage.GetLastDamageDir());
		vDir.Norm();
		VEC_MULSCALAR(vDir, vDir, fPercent);

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_DAMAGE);
		g_pLTServer->WriteToMessageVector(hMessage, &vDir);
		g_pLTServer->WriteToMessageByte(hMessage, m_damage.GetLastDamageType());
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleGameRestore
//
//	PURPOSE:	Setup the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleGameRestore()
{
	CPlayerAttachments* pPlayerAttachments;

	if (!g_pWeaponMgr) return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	// Make sure we are using the correct model/skin...
	// This shouldn't be necessary anymore,  the client
	// restores its own model/skin and so does the server.
	// If this is needed, the player's health and armor should
	// not be reset!
	//ResetCharacter();
	g_pCharacterMgr->Add(this);

	m_PStateChangeFlags |= PSTATE_ATTRIBUTES;
	if( IsNetted() ) m_PStateChangeFlags |= PSTATE_SET_NET_FX;
	UpdateClientPhysics(LTTRUE);

	// Let the client know what state we are in...
	ChangeState(m_eState);

	// Make sure we're displaying the default weapon...
	CWeapons* pWeapons = LTNULL;
	if (pPlayerAttachments)
		pWeapons = pPlayerAttachments->GetWeapons();

	if(pWeapons)
	{
		CWeapon* pWeapon = pWeapons->GetCurWeapon();

		if(pWeapon && pWeapon->GetWeaponData())
			ChangeWeapon( pWeapon->GetWeaponData()->szName, LTFALSE);
	}

	// Make sure the interface is accurate...
	UpdateInterface(LTTRUE, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::BuildKeepAlives
//
//	PURPOSE:	Add the objects that should be keep alive
//				between levels to this list.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::BuildKeepAlives(ObjectList* pList)
{
	if (!pList || !m_hObject) return;

	LTVector vZero;
	VEC_INIT(vZero);

	// Since we must be loading a level....Hide and make non-solid...

	m_cmMovement.SetObjectFlags( OFT_Flags, 0, LTFALSE);
	m_cmMovement.SetObjectFlags( OFT_Flags2, 0, LTFALSE);
	m_cmMovement.SetVelocity(LTVector(0.0f, 0.0f, 0.0f));
	m_cmMovement.SetAcceleration(LTVector(0.0f, 0.0f, 0.0f));


	// Build keep alives...
	g_pLTServer->AddObjectToList(pList, GetHitBox());
	g_pLTServer->AddObjectToList(pList, m_hObject);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetForceUpdateList
//
//	PURPOSE:	Add all the objects that ALWAYS need to be kept around on 
//				the client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetForceUpdateList(ForceUpdate* pFU)
{
	if (!pFU || !pFU->m_Objects) return;

	for( Camera::CameraIterator iter = Camera::BeginCameras();
	     iter != Camera::EndCameras(); ++iter )
	{
		if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
		{
			pFU->m_Objects[pFU->m_nObjects++] = (*iter)->m_hObject;
		}
		else
		{
			break;
		}
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

DBOOL CPlayerObj::ProcessCommand(const char*const* pTokens, int nArgs)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (!pTokens || nArgs < 1) return DFALSE;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	// Let base class have a whack at it...

	if (CCharacter::ProcessCommand(pTokens, nArgs)) return LTTRUE;


	if (stricmp(TRIGGER_MUSIC, pTokens[0]) == 0)
	{
		if( !g_pMusicMgr->HandleCommand( pTokens + 1, nArgs - 1 ) )
		{
#ifndef _FINAL
			AIErrorPrint(m_hObject, "Music command \"%s\" not understood! Full command was :", pTokens + 1 );

			for( int i = 0; i < (nArgs - 1); ++i )
			{
				AIErrorPrint(m_hObject, "    %s", pTokens + 1 + i );
			}
#endif
		}

		return LTTRUE;

	}
	if (stricmp(TRIGGER_FACEOBJECT, pTokens[0]) == 0)
	{
		if (nArgs > 1)
		{
			const char* pObjName = pTokens[1];
			if (pObjName)
			{
				ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
				int numObjects;

				g_pLTServer->FindNamedObjects(const_cast<char*>(pObjName),objArray);
				numObjects = objArray.NumObjects();

				if (!numObjects) return LTFALSE;

				HOBJECT hObj = numObjects ? objArray.GetObject(0) : DNULL;

				if (hObj)
				{
					// Look at the object...
					LTVector vDir, vPos, vTargetPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);
					g_pLTServer->GetObjectPos(hObj, &vTargetPos);

					vTargetPos.y = vPos.y; // Don't look up/down.

					VEC_SUB(vDir, vTargetPos, vPos);
					vDir.Norm();

					DRotation rRot;
					g_pLTServer->AlignRotation(&rRot, &vDir, NULL);
					g_pLTServer->SetObjectRotation(m_hObject, &rRot);
				}
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Command %s given with no arguments!", TRIGGER_FACEOBJECT);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_TELEPORT, pTokens[0]) == 0)
	{
		if(nArgs == 4)
		{
			LTVector vTemp = LTVector((float)atof(pTokens[1]), (float)atof(pTokens[2]), (float)atof(pTokens[3]));
			Teleport(vTemp);
		}
		else if(nArgs == 2)
		{
			Teleport(pTokens[1]);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Command %s given without 2 or 4 arguments!", TRIGGER_TELEPORT);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_TELEPORTNR, pTokens[0]) == 0)
	{
		if(nArgs == 4)
		{
			LTVector vTemp = LTVector((float)atof(pTokens[1]), (float)atof(pTokens[2]), (float)atof(pTokens[3]));
			Teleport(vTemp);
		}
		else if(nArgs == 2)
		{
			Teleport(pTokens[1], LTFALSE);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Command %s given without 2 or 4 arguments!", TRIGGER_TELEPORTNR);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_SWAP, pTokens[0]) == 0)
	{
		if( nArgs == 2 )
		{
			if( 0 == stricmp(pTokens[1], "on") )
			{
				Swap(LTTRUE);
			}
			else 
			{
#ifndef _FINAL
				if( 0 != stricmp(pTokens[1], "off") )
				{
					g_pLTServer->CPrint("Player received trigger command \"%s %s\", assuming \"%s off\".",
						pTokens[0], pTokens[1], TRIGGER_SWAP );
				}
#endif
				Swap(LTFALSE);
			}
		}
		else if( nArgs == 0 )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Player received trigger command \"%s\" without any arguments, assuming \"%s off\".",
				pTokens[0], TRIGGER_SWAP );
#endif
			Swap(LTFALSE);
		}
		else
		{
#ifndef _FINAL
			AIErrorPrint(m_hObject, "Received trigger command \"%s\" with more than one argument, assuming \"%s off\".",
				pTokens[0], TRIGGER_SWAP );
#endif
			Swap(LTFALSE);
		}
					
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_PICKUP, pTokens[0]) == 0)
	{
		char szBuffer[256];
		strcpy(szBuffer, pTokens[1]);
		int nSet = g_pPickupButeMgr->GetPickupSetFromName(szBuffer);

		uint32 nPUUsedAmt[4]={0,0,0,0};
		uint32 nAmt[4]={0,0,0,0};

		if(nSet == -1)
		{
			char szBuffer[256];
			strcpy(szBuffer, pTokens[1]);
			ProcessPowerupMsg(szBuffer, LTNULL, nAmt, nPUUsedAmt);
		}
		else
		{
			for(int i = 1; i < nArgs; i++)
			{
				strcpy(szBuffer, pTokens[i]);
				PickupButes butes = g_pPickupButeMgr->GetPickupButes(szBuffer);

				ProcessPowerupMsg(butes.m_szMessage, LTNULL, nAmt, nPUUsedAmt);
			}
		}

		return LTTRUE;
	}
	else if (stricmp(TRIGGER_PICKUP_NOTELL, pTokens[0]) == 0)
	{
		char szBuffer[256];
		strcpy(szBuffer, pTokens[1]);
		int nSet = g_pPickupButeMgr->GetPickupSetFromName(szBuffer);

		uint32 nPUUsedAmt[4]={0,0,0,0};
		uint32 nAmt[4]={0,0,0,0};

		if(nSet == -1)
		{
			char szBuffer[256];
			strcpy(szBuffer, pTokens[1]);
			ProcessPowerupMsg(szBuffer, LTNULL, nAmt, nPUUsedAmt, LTFALSE);
		}
		else
		{
			for(int i = 1; i < nArgs; i++)
			{
				strcpy(szBuffer, pTokens[i]);
				PickupButes butes = g_pPickupButeMgr->GetPickupButes(szBuffer);

				ProcessPowerupMsg(butes.m_szMessage, LTNULL, nAmt, nPUUsedAmt, LTFALSE);
			}
		}

		return LTTRUE;
	}
	else if (stricmp(TRIGGER_RESETINVENTORY, pTokens[0]) == 0)
	{
		if (pPlayerAttachments)
		{
			// Clear our weapons/ammo...

			pPlayerAttachments->ResetAllWeapons();

			// Tell the client to clear out all our inventory...

			if (m_hClient)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
				g_pLTServer->WriteToMessageByte(hMessage, IC_RESET_INVENTORY_ID);
				g_pLTServer->EndMessage(hMessage);
			}

			// Clear out our misc inventory...
			SetHasFlarePouch(LTFALSE);
			SetHasCloakDevice(LTFALSE);
			SetHasMask(LTFALSE, LTFALSE);
			SetHasNightVision(LTFALSE);

			// Well...give us at least *one* weapon ;)

			AquireDefaultWeapon();
		}

		return LTTRUE;
	}
	else if (stricmp(TRIGGER_CONSOLE, pTokens[0]) == 0)
	{
		// Find any objects with the default player name ("WorldProperties0")
		ObjArray <HOBJECT, 1> objArray;
		g_pLTServer->FindNamedObjects("WorldProperties0", objArray);
		if(objArray.NumObjects() == 0) return LTTRUE;

		HOBJECT hObject = objArray.GetObject(0);

		// Now send off the message to the object. 
		char buf[256];
		memset(buf, 0, sizeof(char)*256);

		for(int i = 1; i < nArgs; i++)
		{
			strcat(buf, const_cast<char*>(pTokens[i]));
			strcat(buf, " ");
		}

		HSTRING hStr = g_pLTServer->CreateString(buf);

		HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, hObject, MID_TRIGGER);
		g_pLTServer->WriteToMessageHString(hWrite, hStr);
		g_pLTServer->EndMessage(hWrite);

		g_pLTServer->FreeString(hStr);
		hStr = LTNULL;


		return LTTRUE;
	}
	else if (stricmp(TRIGGER_FADEIN, pTokens[0]) == 0)
	{
		if(nArgs > 1)
		{
			// Tell the client to do the fade....

			if (m_hClient)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_FADEIN);
				hMessage->WriteFloat((float)atof(pTokens[1]));
				g_pLTServer->EndMessage(hMessage);
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Command %s given with no arguments!", TRIGGER_FADEIN);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_FADEOUT, pTokens[0]) == 0)
	{
		if(nArgs > 1)
		{
			// Tell the client to do the fade....

			if (m_hClient)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_FADEOUT);
				hMessage->WriteFloat((float)atof(pTokens[1]));
				g_pLTServer->EndMessage(hMessage);
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Command %s given with no arguments!", TRIGGER_FADEOUT);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_MD, pTokens[0]) == 0)
	{
		if(nArgs > 1)
		{
			// Tell the client to do the fade....

			if (m_hClient)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_MD);
				hMessage->WriteByte(stricmp("ON",pTokens[1])==0);
				g_pLTServer->EndMessage(hMessage);
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Command %s given with no arguments!", TRIGGER_FADEOUT);
		}
#endif
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_ENERGY_CHARGE, pTokens[0]) == 0)
	{
		AMMO_POOL* pPool = g_pWeaponMgr->GetAmmoPool("Predator_Energy");

		if(pPool)
		{
			DecrementPredatorEnergy(-pPool->GetMaxAmount(LTNULL));
		}

		return LTTRUE;
	}
	else if (stricmp(TRIGGER_MISSIONFAILED, pTokens[0]) == 0)
	{
		HMESSAGEWRITE hMsg = g_pInterface->StartMessage(LTNULL, SCM_MISSION_FAILED);
		g_pInterface->EndMessage(hMsg);

		return LTTRUE;
	}
	else if (stricmp(TRIGGER_OBSERVE, pTokens[0]) == 0)
	{
		SetObservePoint_Random();
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_RESPAWN, pTokens[0]) == 0)
	{
		Respawn();
		return LTTRUE;
	}
	else if (stricmp(TRIGGER_TELETYPE, pTokens[0]) == 0)
	{
		if(nArgs > 1)
		{
			if(m_hClient)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_TELETYPE);
				hMessage->WriteWord((uint16)atoi(pTokens[1]));
				hMessage->WriteByte(0);
				g_pLTServer->EndMessage(hMessage);
			}
		}

		return LTTRUE;
	}
	else if (stricmp(TRIGGER_HINT, pTokens[0]) == 0)
	{
		if(nArgs > 1)
		{
			if(m_hClient)
			{
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_TELETYPE);
				hMessage->WriteWord((uint16)atoi(pTokens[1]));
				hMessage->WriteByte(1);
				g_pLTServer->EndMessage(hMessage);
			}
		}

		if(nArgs > 2)
		{
			// TODO: Insert an objective for this hint...
		}

		return LTTRUE;
	}
	else if (stricmp(TRIGGER_CREDITS, pTokens[0]) == 0)
	{
		if (nArgs > 1)
		{
			return HandleCreditsMessage(pTokens[1]);
		}
	}
	if (stricmp("REMOVE", pTokens[0]) == 0)
	{
#ifndef _FINAL
		g_pLTServer->CPrint("PLAYER ERROR: SENT REMOVE TRIGGER, COMMAND IGNORED!");
#endif
		return LTTRUE;
	}

	return LTFALSE;
}



// ----------------------------------------------------------------------- //
//s
//	ROUTINE:	CPlayerObj::UpdateConsoleVars()
//
//	PURPOSE:	Check console commands that pertain to the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateConsoleVars()
{
#ifndef _FINAL
	LTFLOAT fShowNodesLevel = g_ShowNodesTrack.GetFloat();

	if (fShowNodesLevel > 0 && !g_bShowNodes)
	{
		g_pAINodeMgr->AddNodeDebug((int)fShowNodesLevel);
		g_bShowNodes = LTTRUE;
	}
	else if (fShowNodesLevel <= 0 && g_bShowNodes)
	{
		g_pAINodeMgr->RemoveNodeDebug();
		g_bShowNodes = DFALSE;
	}

	LTFLOAT fShowSimpleLevel = g_ShowSimpleTrack.GetFloat();

	if (fShowSimpleLevel > 0 && !g_bShowSimple)
	{
		g_pSimpleNodeMgr->AddNodeDebug((int)fShowSimpleLevel);
		g_bShowSimple = LTTRUE;
	}
	else if (fShowSimpleLevel <= 0 && g_bShowSimple)
	{
		g_pSimpleNodeMgr->RemoveNodeDebug();
		g_bShowSimple = DFALSE;
	}

	if(g_ClearLines.GetFloat() > 0)
	{
		// Clear all the lines.
		LineSystem::ClearAll();

		g_ClearLines.SetFloat(0.0f);
	}

#endif

	// Check var trackers...

	SetLeashLen(g_LeashLenTrack.GetFloat());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientViewPos()
//
//	PURPOSE:	Update where the client's view is
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientViewPos()
{
	if (!m_hClient) return;

	if( m_bSwapped ) return;

	if (IsExternalCameraActive())
	{
		//only do this the first time through
		if(!m_bInCinematic)
		{
			// If we're in a cinematic don't allow the player to be damaged...

			//save our old min health
			m_fMinHealth = m_damage.GetMinHealth();
			m_bInCinematic = LTTRUE;
			m_damage.SetCanDamage(DFALSE);
		}

		// Make sure we aren't moving...

		if (!m_bAllowInput)
		{
			m_cmMovement.SetVelocity(LTVector(0.0f, 0.0f, 0.0f));
			m_cmMovement.SetAcceleration(LTVector(0.0f, 0.0f, 0.0f));
		}
	}
	else
	{
		if(m_bInCinematic || (!m_bGodMode && !m_damage.GetCanDamage()))
		{
			// Okay, now we can be damaged...
			if (!m_bGodMode)
			{
				m_damage.SetCanDamage(LTTRUE);
			}
			else
			{
				//be sure we re-set to the proper flavour of god mode
				m_damage.SetCanDamage(DFALSE, m_fMinHealth);
			}
			m_bInCinematic = LTFALSE;
		}

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->SetClientViewPos(m_hClient, &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::StartLevel()
//
//	PURPOSE:	Trigger any beginning of level events...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::StartLevel()
{
	if (m_hstrStartLevelTriggerTarget && m_hstrStartLevelTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrStartLevelTriggerTarget, m_hstrStartLevelTriggerMessage);
	}

	if (m_hstrStartMissionTriggerTarget && m_hstrStartMissionTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrStartMissionTriggerTarget, m_hstrStartMissionTriggerMessage);
	}

	// Okay, this is a one-time only trigger, so remove the messages...

	if (m_hstrStartLevelTriggerTarget)
	{
		g_pLTServer->FreeString(m_hstrStartLevelTriggerTarget);
		m_hstrStartLevelTriggerTarget = DNULL;
	}

	if (m_hstrStartLevelTriggerMessage)
	{
		g_pLTServer->FreeString(m_hstrStartLevelTriggerMessage);
		m_hstrStartLevelTriggerMessage = DNULL;
	}

	if (m_hstrStartMissionTriggerTarget)
	{
		g_pLTServer->FreeString(m_hstrStartMissionTriggerTarget);
		m_hstrStartMissionTriggerTarget = DNULL;
	}

	if (m_hstrStartMissionTriggerMessage)
	{
		g_pLTServer->FreeString(m_hstrStartMissionTriggerMessage);
		m_hstrStartMissionTriggerMessage = DNULL;
	}

	m_bLevelStarted = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMessage_UpdateClient()
//
//	PURPOSE:	Process the client update information
//
//				NOTE!! These must be in the same order as the
//				PlayerMovement::UpdateClientToServer function on the client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMessage_UpdateClient(HMESSAGEREAD hRead)
{
	// Retrieve the update flags from the message
	uint16 nFlags = g_pLTServer->ReadFromMessageWord(hRead);

	// If the flags are empty... just return
	if(!nFlags) return;


	// Check for scaling information
	if(nFlags & CLIENTUPDATE_SCALE)
	{
		LTFLOAT fScale = g_pLTServer->ReadFromMessageFloat(hRead);

		// Update to the new scale
		m_cmMovement.SetScale(fScale);
	}


	// Check for dims information
	if(nFlags & CLIENTUPDATE_DIMS)
	{
		LTVector vDims;
		g_pLTServer->ReadFromMessageVector(hRead, &vDims);

		// Update to the new dimensions
		m_cmMovement.SetObjectDims(vDims);
	}


	// Check for rotation update information
	if(nFlags & CLIENTUPDATE_VELOCITY)
	{
		LTVector vVel;

		// Get the new rotation information
		g_pLTServer->ReadFromMessageVector(hRead, &vVel);

		// Update to the new velocity
		m_cmMovement.SetVelocity(vVel, LTFALSE);
	}


	// Check for position update information
	if(nFlags & CLIENTUPDATE_POSITION)
	{
		LTVector vCurPos, vPos;

		// Get the current position information
		g_pLTServer->GetObjectPos(m_hObject, &vCurPos);

		// Get the new position information
		uint8 nMoveCode = g_pLTServer->ReadFromMessageByte(hRead);
		g_pLTServer->ReadFromMessageVector(hRead, &vPos);

		// Polygon that we're standing on
		HPOLY hPoly = (HPOLY)g_pLTServer->ReadFromMessageDWord(hRead);
		m_cmMovement.SetStandingOn(hPoly);

		// If they're more than a little different... then update them
		if(g_pGameServerShell->GetGameType().IsSinglePlayer())
		{
			// First move object without teleporting so it can handle collisions.
			m_cmMovement.SetPos(vPos, 0);

			// Make sure it made it.  If it didn't, then teleport it there.
			LTVector vEndPos;
			g_pLTServer->GetObjectPos(m_hObject, &vEndPos);
			if( !vEndPos.Equals( vPos ))
			{
				m_cmMovement.SetPos(vPos, MOVEOBJECT_TELEPORT);
			}
		}
		else
		{
			// Make sure our move code is equal
			if(nMoveCode == m_ClientMoveCode)
			{
				// Get the current movement state...
				CharacterMovementState cms = m_cmMovement.GetMovementState();

				// Don't use the leash if we're in certain states (kinda sucks... but I dunno
				// what else to do about it)
				LTBOOL bNoLeash = (cms == CMS_POUNCING) || (cms == CMS_FACEHUG) || (cms == CMS_POUNCE_JUMP) ;

				// Check the leash for our player object and make sure to teleport if we need to
				if(!bNoLeash && ((vCurPos - vPos).Mag() > m_fLeashLen))
					m_cmMovement.SetPos(vPos, MOVEOBJECT_TELEPORT);
				else if(!vCurPos.Equals(vPos, 0.1f))
					m_cmMovement.SetPos(vPos);
			}
		}

		// See if we're standing on a Breakable object...
		if(hPoly != INVALID_HPOLY)
		{
			HOBJECT hObj = DNULL;
			if (g_pLTServer->GetHPolyObject(hPoly, hObj) == LT_OK)
			{
				Breakable* pBreak = dynamic_cast<Breakable*>(g_pLTServer->HandleToObject(hObj));
				if (pBreak)
				{
					pBreak->Break(m_hObject);
				}
			}
		}
	}


	// Check for rotation update information
	if(nFlags & CLIENTUPDATE_ROTATION)
	{
		LTRotation rRot;

		// Get the new rotation information
		g_pLTServer->ReadFromMessageRotation(hRead, &rRot);

		// Update to the new rotation
		m_cmMovement.SetRot(rRot);
	}


	// Check for aiming update information
	if(nFlags & CLIENTUPDATE_AIMING)
	{
		LTFLOAT fAimingPitch = (LTFLOAT)(int8)g_pLTServer->ReadFromMessageByte(hRead);

		// Update to the new aiming value
		m_cmMovement.GetCharacterVars()->m_fAimingPitch = fAimingPitch;
	}


	// Check for allowable input information
	if(nFlags & CLIENTUPDATE_INPUTALLOWED)
	{
		uint8 nFlags = g_pLTServer->ReadFromMessageByte(hRead);

		// Update to the new input values
		m_cmMovement.GetCharacterVars()->m_nAllowMovement = (LTBOOL)(nFlags & 0x01);
		m_cmMovement.GetCharacterVars()->m_nAllowRotation = (LTBOOL)(nFlags & 0x02);
	}


	// Check for object flag value information
	if(nFlags & CLIENTUPDATE_FLAGS)
	{
		uint32 nFlags = g_pLTServer->ReadFromMessageDWord(hRead);

		// Preserve the server specific flags
		nFlags |= m_cmMovement.GetObjectFlags(OFT_Flags) & ~CLIENT_FLAGMASK;

		// Set any server specific flags
		nFlags |= FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE | FLAG_PORTALVISIBLE;
		nFlags &= ~FLAG_GRAVITY;

		// Check for special state flags
		if((m_eState == PS_DEAD) || (m_eState == PS_OBSERVING) || (m_eState == PS_SWAPPED))
			nFlags &= ~(FLAG_VISIBLE | FLAG_SOLID);

		if(GetHitBox())
		{
			if(CCharacterHitBox *pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(GetHitBox())))
			{
				if(g_pGameServerShell->GetGameType().IsMultiplayer() && InGodMode())
				{
					pHitBox->SetCanBeHit(LTFALSE);

					if(!m_bSpawnInState)
						nFlags &= ~FLAG_SOLID;
				}
				else
					pHitBox->SetCanBeHit((m_eState != PS_DEAD) && (m_eState != PS_OBSERVING));
			}
		}


		// Update to the new flag values
		m_cmMovement.SetObjectFlags(OFT_Flags, nFlags, LTTRUE);
	}


	// Check for object flag value information
	if(nFlags & CLIENTUPDATE_FLAGS2)
	{
		uint32 nFlags2 = (uint32)g_pLTServer->ReadFromMessageWord(hRead);

		// Set any server specific flags
		nFlags2 &= ~(FLAG2_CYLINDERPHYSICS | FLAG2_SPHEREPHYSICS);

		// Update to the new flag values
		m_cmMovement.SetObjectFlags(OFT_Flags2, nFlags2, LTTRUE);
	}


	// Check for movement state information
	if(nFlags & CLIENTUPDATE_MOVEMENTSTATE)
	{
		uint8 nState = g_pLTServer->ReadFromMessageByte(hRead);

		// Update to the new flag values
		m_cmMovement.SetMovementState((CharacterMovementState)nState);
	}


	// Check for control flag information
	if(nFlags & CLIENTUPDATE_CTRLFLAGS)
	{
		uint32 dwCtrlFlags = (uint32)g_pLTServer->ReadFromMessageWord(hRead);

		// Update to the new control flags
		m_cmMovement.SetControlFlags(dwCtrlFlags);
	}



	// If the player is swapped, then we don't want to allow the client to have control
	// over him... so just clear all the appropriate settings
	if(m_bSwapped)
	{
		m_cmMovement.SetObjectFlags(OFT_Flags, 0, LTFALSE);
		m_cmMovement.SetObjectFlags(OFT_Flags2, 0, LTFALSE);

		m_cmMovement.SetControlFlags(0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMessage_Activate
//
//	PURPOSE:	Handle player activation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMessage_Activate(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	LTVector vPos, vDir;
	g_pLTServer->ReadFromMessageVector(hRead, &vPos);
	g_pLTServer->ReadFromMessageVector(hRead, &vDir);

	Activate(vPos, vDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMessage_WeaponFire
//
//	PURPOSE:	Handle player firing weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMessage_WeaponFire(HMESSAGEREAD hRead)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	if (!g_pWeaponMgr || !m_hClient || m_bCharacterReset) return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	LTVector vFlashPos, vFirePos, vDir;
	g_pLTServer->ReadFromMessageVector(hRead, &vFlashPos);
	g_pLTServer->ReadFromMessageVector(hRead, &vFirePos);
	g_pLTServer->ReadFromMessageVector(hRead, &vDir);
	DBYTE nRandomSeed	= g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nWeaponId		= g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nBarrelId		= g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nAmmoId		= g_pLTServer->ReadFromMessageByte(hRead);
	DBOOL bAltFire		= (DBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	DFLOAT fPerturb		= (DFLOAT) g_pLTServer->ReadFromMessageByte(hRead);
	HOBJECT hTarget	= g_pLTServer->ReadFromMessageObject(hRead);
	fPerturb /= 255.0f;



	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
	AMMO* pAmmoData = g_pWeaponMgr->GetAmmo(nAmmoId);

	if (!pWeaponData || !pAmmoData) return;

	CWeapon* pWeapon = LTNULL;

	//special check for flare pouch
	if(strcmp(pAmmoData->szName, "Flare_Pouch_Ammo")==0)
	{
		if(!HasFlarePouch())
			return;

		if(pPlayerAttachments)
		{
			CWeapons* pWeapons = pPlayerAttachments->GetWeapons();

			if(!pWeapons) return;

			pWeapon = pWeapons->GetWeapon(nWeaponId);
		}
		else
		{
			pWeapon = LTNULL;
		}
	}
	else
	{
		if(pPlayerAttachments)
		{
			pWeapon = pPlayerAttachments->GetWeapon();
		}
		else
		{
			pWeapon = LTNULL;
		}
	}
	if (!pWeapon || nWeaponId != pWeapon->GetId()) return;


	// If we aren't dead, and we aren't in spectator mode, let us fire.
	
	if (m_damage.IsDead())
	{
		return;
	}


	// Set the ammo type in the weapon...

	pWeapon->SetAmmoId(nAmmoId);
	pWeapon->SetBarrelId(nBarrelId);


	WFireInfo fireInfo;
	fireInfo.hFiredFrom = m_hObject;
	fireInfo.vPath		= vDir;
	fireInfo.vFirePos	= vFirePos;
	fireInfo.vFlashPos	= vFlashPos;
	fireInfo.nFlashSocket = pWeapon->GetCurrentFlashSocket();
	fireInfo.nSeed		= nRandomSeed;
	fireInfo.bAltFire	= bAltFire;
	fireInfo.fPerturbR	= fPerturb;
	fireInfo.fPerturbU	= fPerturb;
	fireInfo.hTestObj	= hTarget;


	// If we're in 3rd person view, use the hand held weapon fire pos.

	if (m_b3rdPersonView)
	{
		fireInfo.vFirePos  = HandHeldWeaponFirePos(pWeapon);
		fireInfo.vFlashPos = fireInfo.vFirePos;
	}


	pWeapon->Fire(fireInfo);


	// Update number of shots fired...

//	m_PlayerSummary.IncShotsFired();



	// If this is a projectile weapon, tell clients to play the fire sound (vector
	// weapons do this in the weapon fx message)...

	if (pAmmoData->eType == PROJECTILE)
	{
		DBYTE nClientID = (DBYTE) g_pLTServer->GetClientID(m_hClient);

		HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vFirePos);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_WEAPONSOUND_ID);
		g_pLTServer->WriteToMessageByte(hMessage, bAltFire ? WMKS_ALT_FIRE : WMKS_FIRE);
		g_pLTServer->WriteToMessageByte(hMessage, nWeaponId);
		g_pLTServer->WriteToMessageByte(hMessage, nBarrelId);
		g_pLTServer->WriteToMessageByte(hMessage, nClientID);
		g_pLTServer->WriteToMessageCompPosition(hMessage, &vFirePos);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
	}


	// Reset the idle time
	m_fIdleTime = g_pLTServer->GetTime() + 5.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMessage_PounceJump
//
//	PURPOSE:	Handle player pounce jumping
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMessage_PounceJump(HMESSAGEREAD hRead)
{
	if (!g_pWeaponMgr || !m_hClient) return;

	LTVector vTargetPos;
	g_pLTServer->ReadFromMessageVector(hRead, &vTargetPos);

	//send message to player to change state
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(m_hClient, MID_PHYSICS_UPDATE);
	if(!hWrite) return;

	// Write the state flags to the message first, so we know what to read out on the other side
	g_pLTServer->WriteToMessageWord(hWrite, (uint16)PSTATE_POUNCE);
	g_pLTServer->WriteToMessageVector(hWrite, &vTargetPos);
	g_pLTServer->WriteToMessageVector(hWrite, &LTVector(0,0,0));

	g_pLTServer->EndMessage2(hWrite, MESSAGE_GUARANTEED);


	// Play the pounce sound
	if(strstr(m_cmMovement.GetCharacterButes()->m_szName, "Facehugger"))
		PlayExclamationSound("FacehuggerPounceCry");
	else if(strstr(m_cmMovement.GetCharacterButes()->m_szName, "Predalien"))
		PlayExclamationSound("PredalienPounceCry");
	else
		PlayExclamationSound("AlienPounceCry");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMessage_WeaponHit
//
//	PURPOSE:	Handle player hit message, that is this player
//				has claimed to have made a hit.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMessage_WeaponHit(HMESSAGEREAD hRead)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	// don't process any more hits if we are resetting.
	if(m_bCharacterReset)
		return;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	LTVector vFlashPos, vFirePos, vDir, vTargetPos;

	g_pLTServer->ReadFromMessageVector(hRead, &vFlashPos);
	g_pLTServer->ReadFromMessageVector(hRead, &vFirePos);
	g_pLTServer->ReadFromMessageVector(hRead, &vDir);
	DBYTE nWeaponId		= g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nBarrelId		= g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nAmmoId		= g_pLTServer->ReadFromMessageByte(hRead);
	LTBOOL bAltFire		= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
	HOBJECT hTarget		= g_pLTServer->ReadFromMessageObject(hRead);
	ModelNode eNodeHit	= (ModelNode)g_pLTServer->ReadFromMessageByte(hRead);
	LTBOOL bHidePVFX	= (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);

	// These last two are for validation
	LTFLOAT fTimeStamp	= g_pLTServer->ReadFromMessageFloat(hRead);
	g_pLTServer->ReadFromMessageVector(hRead, &vTargetPos);


	WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
	AMMO*	pAmmoData	= g_pWeaponMgr->GetAmmo(nAmmoId);

	CWeapon* pWeapon = NULL;

	if (pPlayerAttachments)
		pWeapon = pPlayerAttachments->GetWeapon();

	// Test our attachments pointer and be sure the weapon
	// we claim to be firing is the one we actually have.
	if (!pWeapon || nWeaponId != pWeapon->GetId()) return;

	// Do a bit of sanity checking before we go on...
	if (!pWeaponData || !pAmmoData) return;

	// If we aren't dead, and we aren't in spectator mode, let us fire.
	
	if (m_damage.IsDead()) return;


	//-----------------------------------------------------------------------//
	// Ok here is where we need to do our target validation...
	CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hTarget));

	if(pPlayer && pPlayer->ValidatePosition(vTargetPos, fTimeStamp))
	{
		// Set the ammo type in the weapon...

		pWeapon->SetAmmoId(nAmmoId);
		pWeapon->SetBarrelId(nBarrelId);


		// Load up the fire info and get ready to rock!
		WFireInfo fireInfo;
		fireInfo.hFiredFrom = m_hObject;
		fireInfo.vPath		= vDir;
		fireInfo.vFirePos	= vFirePos;
		fireInfo.vFlashPos	= vFlashPos;
		fireInfo.nFlashSocket = pWeapon->GetCurrentFlashSocket();
		fireInfo.bAltFire	= bAltFire;
		fireInfo.hTestObj	= hTarget;
		fireInfo.eNodeHit	= eNodeHit;


		// If we're in 3rd person view, use the hand held weapon fire pos.
		if (m_b3rdPersonView)
		{
			fireInfo.vFirePos  = HandHeldWeaponFirePos(pWeapon);
			fireInfo.vFlashPos = fireInfo.vFirePos;
		}


		// Ready, Aim, FIRE!
		pWeapon->HitFire(fireInfo, bHidePVFX);
	}


	// Reset the idle time
	m_fIdleTime = g_pLTServer->GetTime() + 5.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMessage_WeaponSound
//
//	PURPOSE:	Handle weapon sound message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMessage_WeaponSound(HMESSAGEREAD hRead)
{
	if (!hRead || !m_hClient) return;

	DBYTE nType		= g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nWeaponId = g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nBarrelId	= g_pLTServer->ReadFromMessageByte(hRead);

	DBYTE nClientID = (DBYTE) g_pLTServer->GetClientID(m_hClient);

	LTVector vPos = m_cmMovement.GetPos();

	HMESSAGEWRITE hMessage = g_pLTServer->StartInstantSpecialEffectMessage(&vPos);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_WEAPONSOUND_ID);
	g_pLTServer->WriteToMessageByte(hMessage, nType);
	g_pLTServer->WriteToMessageByte(hMessage, nWeaponId);
	g_pLTServer->WriteToMessageByte(hMessage, nBarrelId);
	g_pLTServer->WriteToMessageByte(hMessage, nClientID);
	g_pLTServer->WriteToMessageCompPosition(hMessage, &vPos);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMessage_WeaponLoopSound
//
//	PURPOSE:	Handle weapon sound message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMessage_WeaponLoopSound(HMESSAGEREAD hRead)
{
	if(!hRead || !m_hClient || IsDead()) return;

	// read in the message
	DBYTE nType		= g_pLTServer->ReadFromMessageByte(hRead);
	DBYTE nBarrelId	= g_pLTServer->ReadFromMessageByte(hRead);

	// see if we got the off code
	if(nBarrelId == 0xff) 
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_KILL_LOOP_FIRE);
		g_pLTServer->EndMessage(hMessage);

		// now record the fact that we are playing a sound.
		m_bPlayingLoopFireSound = LTFALSE;
		m_nLoopKeyId = 0xff;

		// exit here
		return;
	}

	// send the special effects message to the clients...
	// only send the new message if we are not already playing
	// a sound or the sound itself has changed.
	if(!m_bPlayingLoopFireSound || m_nLoopKeyId != nType)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
		g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
		g_pLTServer->WriteToMessageByte(hMessage, CFX_PLAY_LOOP_FIRE);
		g_pLTServer->WriteToMessageByte(hMessage, nType);
		g_pLTServer->WriteToMessageByte(hMessage, nBarrelId);
		g_pLTServer->EndMessage(hMessage);

		// now record the fact that we are playing a sound.
		m_bPlayingLoopFireSound = LTTRUE;
		m_nLoopKeyId = nType;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleClientMsg
//
//	PURPOSE:	Handle message from our client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleClientMsg(HMESSAGEREAD hRead)
{
	if (!hRead) return;
	
	switch (g_pLTServer->ReadFromMessageByte(hRead))
	{
	case CP_POUNCE_SOUND :
		{
			// 1 = Normal, 2 = Facehugger, 3 = Predalien (yeah, yeah... hack)
			uint8 nType = g_pLTServer->ReadFromMessageByte(hRead);

			// Send a message to the clients to play the pounce sound
			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, CFX_POUNCE_SOUND);
			g_pLTServer->WriteToMessageByte(hMessage, nType);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
		}
		break;
	case CP_JUMP_SOUND :
		{
			if( !g_pGameServerShell->GetGameType().IsSinglePlayer() )
			{
				// Send a message to the clients to play the pounce sound
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
				g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
				g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
				g_pLTServer->WriteToMessageByte(hMessage, CFX_JUMP_SOUND);
				g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
			}
			else
			{
				HandleFootstepKey();
			}
		}
		break;
	case CP_LAND_SOUND :
		{
			if( !g_pGameServerShell->GetGameType().IsSinglePlayer() )
			{
				// Send a message to the clients to play the pounce sound
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
				g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
				g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
				g_pLTServer->WriteToMessageByte(hMessage, CFX_LAND_SOUND);
				g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
			}
			else
			{
				HandleFootstepKey();
			}

		}
		break;

	case CP_FOOTSTEP_SOUND :
		{
			if( !g_pGameServerShell->GetGameType().IsSinglePlayer() )
			{
				// Send a message to the clients to play the pounce sound
				HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
				g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
				g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
				g_pLTServer->WriteToMessageByte(hMessage, CFX_FOOTSTEP_SOUND);
				g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLE);
			}
			else
			{
				HandleFootstepKey();
			}

		}
		break;

	case CP_POUNCE_DAMAGE :
		{
			//Only do this if we are not already dead...
			if(!IsDead() && IsAlien(m_hObject))
			{
				DamageStruct damage;

				damage.eType	= (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
				damage.fDamage  = g_pLTServer->ReadFromMessageFloat(hRead);
				g_pLTServer->ReadFromMessageVector(hRead, &(damage.vDir));
				HOBJECT hVictim = hRead->ReadObject();

				// See if we hit a character
				CCharacterHitBox* pHitBox = LTNULL;		
				CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hVictim));

				if(pChar)
				{
					pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(pChar->GetHitBox()));
				}
				else
				{
					pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(hVictim));
				}

				if(pHitBox)
				{
					if(!pChar)
						pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(pHitBox->GetModelObject()));

					if(pChar)
					{
						// If we're a facehugger we may want to do something different
						int nVictimSet = pChar->GetCharacter();
						hVictim = pChar->m_hObject;

						if(	strstr(m_cmMovement.GetCharacterButes()->m_szName, "Facehugger") &&
							g_pCharacterButeMgr->GetCanBeFacehugged(nVictimSet))
						{
							damage.eType = DT_FACEHUG;
							damage.fDamage = 5000.0f;

							ModelNode eNodeHit = g_pModelButeMgr->GetSkeletonNode(pChar->GetModelSkeleton(), "Head_node");
							pChar->SetModelNodeLastHit(eNodeHit);
						}
						else
						{
							ModelNode eNodeHit = g_pModelButeMgr->GetSkeletonNode(pChar->GetModelSkeleton(), "Torso_u_node");
							pChar->SetModelNodeLastHit(eNodeHit);
						}
					}
				}

				// do the instant damage...
				damage.hDamager = m_hObject;
				damage.DoDamage(this, hVictim);

				// do the progressive damage...
				damage.fDuration	= damage.fDamage / g_PounceStunRatio.GetFloat();
				damage.eType		= DT_STUN;
				damage.fDamage		= 0.0f;
				damage.DoDamage(this, hVictim);


				// Set up fire info for Pounce so HearWeaponFire can receive it.
				CharFireInfo char_info;
				g_pLTServer->GetObjectPos(m_hObject, &char_info.vFiredPos);
				char_info.vFiredDir = damage.vDir;

				WEAPON *pWeapon = g_pWeaponMgr->GetWeapon("Pounce");
				if (pWeapon)
					char_info.nWeaponId = pWeapon->nId;

				BARREL *pBarrel = g_pWeaponMgr->GetBarrel("Pounce_Barrel");
				if (pBarrel)
					char_info.nBarrelId = pBarrel->nId;

				AMMO *pAmmo = g_pWeaponMgr->GetAmmo("Pounce_Ammo");
				if (pAmmo)
					char_info.nAmmoId = pAmmo->nId;

				char_info.fTime		= g_pLTServer->GetTime();
				SetLastFireInfo(char_info);
			}

		}
		break;

	case CP_DAMAGE :
		{
			DamageStruct damage;

			damage.eType	= (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
			damage.fDamage  = g_pLTServer->ReadFromMessageFloat(hRead);
			g_pLTServer->ReadFromMessageVector(hRead, &(damage.vDir));
			HOBJECT hVictim = hRead->ReadObject();

			damage.hDamager = m_hObject;

			damage.DoDamage(this, hVictim);
		}
		break;

	case CP_CANNON_CHARGE :
		{
			if(IsPredator(m_hObject))
			{
				//see if we are already charged
				if(m_nCannonChargeLevel < m_nMaxChargeLevel)
				{
					//see if we have the energy to charge
					uint32 nEnergy = GetPredatorEnergy();

					AMMO* pAmmo = LTNULL;
					
					if(g_pGameServerShell->GetGameType().IsMultiplayer())
						pAmmo = g_pWeaponMgr->GetAmmo("Shoulder_Cannon_Ammo_MP");
					else
						pAmmo = g_pWeaponMgr->GetAmmo("Shoulder_Cannon_Ammo");


					if(pAmmo && ((uint32)pAmmo->nAmmoPerShot <= nEnergy))
					{
						//incriment charge level
						++m_nCannonChargeLevel;

						//relieve the energy pool
						DecrementPredatorEnergy(pAmmo->nAmmoPerShot);

						//update the player stats
						UpdateInterface(LTTRUE);
					}
				}
			}
		}
		break;

	case CP_FLASHLIGHT :
		{
			// FL_ON = 0, FL_OFF = 1, FL_UPDATE = 2
			int nStatus = g_pLTServer->ReadFromMessageByte(hRead);

			switch (nStatus)
			{
				case FL_ON:
				{
					SetFlashlight(TRUE);
					break;
				}
				case FL_OFF:
				{
					SetFlashlight(FALSE);
					break;
				}
				case 2:
				{
					LTVector vPos;
					g_pLTServer->ReadFromMessageVector(hRead, &vPos);
					SetFlashlightPos(vPos);
					break;
				}
			}

			break;
		}

	case CP_VISIONMODE :
		{
			// FL_ON = 0, FL_OFF = 1
			int nStatus = g_pLTServer->ReadFromMessageByte(hRead);

			switch (nStatus)
			{
				case VM_NIGHTV_ON:
				{
					SetNightVision(TRUE);
					break;
				}
				case VM_NIGHTV_OFF:
				{
					SetNightVision(FALSE);
					break;
				}
				case VM_PRED_HEAT:
				{
					EyeFlash(EYEFLASH_COLOR_BLUE);
					break;
				}
				case VM_PRED_ELEC:
				{
					EyeFlash(EYEFLASH_COLOR_RED);
					break;
				}
				case VM_PRED_NAV:
				{
					EyeFlash(EYEFLASH_COLOR_GREEN);
					break;
				}
			}

			break;
		}

	case CP_ENERGY_SIFT :
		{
			if(IsPredator(m_hObject))
				HandleEnergySift();
			break;
		}

	case CP_MEDICOMP :
		{
			if(IsPredator(m_hObject))
				HandleMedicomp();
			break;
		}

	case CP_HOTBOMB_DETONATE :
		{
			ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

			g_pServerDE->FindNamedObjects("HotbombProjectile",objArray);

			for(uint32 i = 0; i < objArray.NumObjects(); i++)
			{
				CStickyHotbomb* pBomb = dynamic_cast<CStickyHotbomb*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));
				
				if(pBomb)
					pBomb->TriggerDetonate(m_hObject);
			}

			break;
		}

	case CP_WEAPON_STATUS:
		{
			m_nWeaponStatus = g_pLTServer->ReadFromMessageByte(hRead);
			UpdateExtWeaponAnimation();
			break;
		}

	default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleportClientToServerPos()
//
//	PURPOSE:	This sends a message to the client telling it to move to 
//				our position.  Used when loading games and respawning.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleportClientToServerPos(LTFLOAT fTime)
{
	if (!m_hClient) return;

	LTVector myPos;
	g_pLTServer->GetObjectPos(m_hObject, &myPos);


	// Tell the player about the new move code.
	++m_ClientMoveCode;

	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(m_hClient, MID_SERVERFORCEPOS);
	g_pLTServer->WriteToMessageByte(hWrite, m_ClientMoveCode);
	g_pLTServer->WriteToMessageVector(hWrite, &myPos);
	g_pLTServer->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleFragObjects
//
//	PURPOSE:	TeleFrag any player object's at this position
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleFragObjects(LTVector & vPos)
{
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	int numObjects, i;

	// Find any objects with the default player name ("Player")
	g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);
	numObjects = objArray.NumObjects();
	if(!numObjects) return;

	HOBJECT hObject = LTNULL;

	// Go through each object found and try to kill it
	for(i = 0; i < numObjects; i++)
	{
		hObject = objArray.GetObject(i);

		// Make sure not to FRAG ourself
		if(hObject != m_hObject)
		{
			// Ignore any players that are in god mode
			CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hObject));

			if(pPlayer && pPlayer->InGodMode())
				continue;


			LTVector vPlayerDims;
			LTVector vObjPos, vObjDims;

			g_pLTServer->GetObjectDims(m_hObject, &vPlayerDims);
			g_pLTServer->GetObjectPos(hObject, &vObjPos);
			g_pLTServer->GetObjectDims(hObject, &vObjDims);

			LTVector vPosDelta = vObjPos - vPos;
			vPosDelta.x = (vPosDelta.x > 0.0f) ? vPosDelta.x : -vPosDelta.x;
			vPosDelta.y = (vPosDelta.y > 0.0f) ? vPosDelta.y : -vPosDelta.y;
			vPosDelta.z = (vPosDelta.z > 0.0f) ? vPosDelta.z : -vPosDelta.z;

			LTVector vDimsComb = vPlayerDims + vObjDims;

			// See if there's any overlap of the dims
			if((vDimsComb.x > vPosDelta.x) && (vDimsComb.y > vPosDelta.y) && (vDimsComb.z > vPosDelta.z))
			{
				DamageStruct damage;

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = m_hObject;
				damage.vDir.Init(0, 1, 0);
				damage.bIgnoreFF = LTTRUE;

				damage.DoDamage(this, hObject);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CPlayerObj::CreateAttachments()
{
	if(!m_pAttachments) 
	{
		m_pAttachments = new CPlayerAttachments(LTFALSE);

		if(m_pAttachments)
		{
			AddAggregate(m_pAttachments);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleEMPEffect
//
//	PURPOSE:	Sets the predator energy amount to 0
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleEMPEffect()
{
	CCharacter::HandleEMPEffect();

	//just be sure the interface is updated
	UpdateInterface(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleEnergySift
//
//	PURPOSE:	Handle updating the pred energy
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleEnergySift()
{
	CCharacter::HandleEnergySift();

	//just be sure the interface is updated
	UpdateInterface(LTTRUE);

	// Alert any AI about our siftage...
	CharFireInfo char_info;
	g_pLTServer->GetObjectPos(m_hObject, &char_info.vFiredPos);
	char_info.vFiredDir = GetForward();

	WEAPON *pWeapon = g_pWeaponMgr->GetWeapon("Energy_Sift");
	if (pWeapon)
		char_info.nWeaponId = pWeapon->nId;

	BARREL *pBarrel = g_pWeaponMgr->GetBarrel("Energy_Sift_Barrel");
	if (pBarrel)
		char_info.nBarrelId = pBarrel->nId;

	AMMO *pAmmo = g_pWeaponMgr->GetAmmo("Energy_Sift_Ammo");
	if (pAmmo)
		char_info.nAmmoId = pAmmo->nId;

	char_info.fTime		= g_pLTServer->GetTime();
	SetLastFireInfo(char_info);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleMedicomp
//
//	PURPOSE:	Handle a medicomp charge
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleMedicomp()
{
	CCharacter::HandleMedicomp();

	//just be sure the interface is updated
	UpdateInterface(LTTRUE);

	// Alert any AI about our surgery...
	CharFireInfo char_info;
	g_pLTServer->GetObjectPos(m_hObject, &char_info.vFiredPos);
	char_info.vFiredDir = GetForward();

	WEAPON *pWeapon = g_pWeaponMgr->GetWeapon("Medicomp");
	if (pWeapon)
		char_info.nWeaponId = pWeapon->nId;

	BARREL *pBarrel = g_pWeaponMgr->GetBarrel("Medicomp_Barrel");
	if (pBarrel)
		char_info.nBarrelId = pBarrel->nId;

	AMMO *pAmmo = g_pWeaponMgr->GetAmmo("Medicomp_Ammo");
	if (pAmmo)
		char_info.nAmmoId = pAmmo->nId;

	char_info.fTime		= g_pLTServer->GetTime();
	SetLastFireInfo(char_info);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CreateExosuitHead
//
//	PURPOSE:	Handle making the exosuit head attachment
//
// ----------------------------------------------------------------------- //

HOBJECT CPlayerObj::CreateExosuitHead(LTBOOL bDefault)
{
	//see if this is a predator or human
	CharacterButes butes;

	if(bDefault)
		butes = g_pCharacterButeMgr->GetCharacterButes("Harris");
	else
		butes = g_pCharacterButeMgr->GetCharacterButes(m_nOldCharacterSet);

	//only create a head if we are a human or predator
	HCLASS hClass = g_pServerDE->GetClass("Prop");
	if (!hClass) return LTNULL;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

#ifdef _DEBUG
	sprintf(theStruct.m_Name, "%s-ExosuitHead", g_pLTServer->GetObjectName(m_hObject) );
#endif

	g_pCharacterButeMgr->GetDefaultFilenames(m_nOldCharacterSet,theStruct);
	theStruct.m_ObjectType = OT_MODEL;

	// Allocate an object...
	Prop* pProp;
	pProp = (Prop*)g_pServerDE->CreateObject(hClass, &theStruct);
	if (!pProp) return LTNULL;

	if(g_pGameServerShell->GetGameType() == SP_STORY)
	{
		// Cool, now send a special FX message so we can trick the clients
		// into thinking this is a body prop.
		BODYPROPCREATESTRUCT cs;
		cs.nCharacterSet = (uint8)m_nOldCharacterSet;
		cs.hPlayerObj = m_hObject;
		cs.bBloodTrail = LTFALSE;

		HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(pProp);
		g_pLTServer->WriteToMessageByte(hMessage, SFX_BODYPROP_ID);
		cs.Write(g_pLTServer, hMessage);
		g_pLTServer->EndMessage(hMessage);
	}


	// Get the limb_head animation since it is centered on the head and 
	// has the proper dims we need.
	HMODELANIM	hAnim = g_pServerDE->GetAnimIndex(pProp->m_hObject, "limb_head");
	LTVector vDims;
	LTAnimTracker* pTracker;
	g_pLTServer->GetModelLT()->GetMainTracker(pProp->m_hObject, pTracker);
	g_pLTServer->GetModelLT()->SetCurAnim(pTracker, hAnim);
	g_pLTServer->Common()->GetModelAnimUserDims(pProp->m_hObject, &vDims, hAnim);
	g_pLTServer->SetObjectDims(pProp->m_hObject, &vDims);

	uint32 dwFlags = 0; 
	dwFlags |= FLAG_VISIBLE | FLAG_GOTHRUWORLD | FLAG_NOSLIDING;	// Make sure we are visible
	dwFlags &= ~(FLAG_SOLID | FLAG_TOUCH_NOTIFY);
	
	// Make sure we can't take damage
	pProp->GetDestructible()->SetCanDamage(LTFALSE);

	uint32 dwUsrFlags = g_pServerDE->GetObjectUserFlags(pProp->m_hObject);

	// Set our surface type...
	uint32 dwHeadUsrFlags = g_pLTServer->GetObjectUserFlags(pProp->m_hObject);
	SurfaceType nHeadSurfType = UserFlagToSurface(dwHeadUsrFlags);

	dwUsrFlags |= SurfaceToUserFlag(nHeadSurfType);

	g_pServerDE->SetObjectUserFlags(pProp->m_hObject, dwUsrFlags);
	g_pServerDE->SetObjectFlags(pProp->m_hObject, dwFlags);

	//hide all pieces but the head
	HMODELPIECE hPiece;
	g_pLTServer->GetModelLT()->GetPiece(pProp->m_hObject, "Head", hPiece);

	//hide 'em all
	for(HMODELPIECE hTemp=0 ; hTemp<32 ; hTemp++)
		g_pLTServer->GetModelLT()->SetPieceHideStatus(pProp->m_hObject, hTemp, LTTRUE);

	//unhide head...
	g_pLTServer->GetModelLT()->SetPieceHideStatus(pProp->m_hObject, hPiece, LTFALSE);

	return pProp->m_hObject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleExosuitEntry
//
//	PURPOSE:	Handle getting into the exosuit
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleExosuitEntry(HOBJECT hSender)
{
	//call the base class
	CCharacter::HandleExosuitEntry(hSender);

	// Now create our body prop to use as a head in the suit
	m_hHead = CreateExosuitHead();

	// and attach it
	HATTACHMENT pAttachment;
	// Get socket.
	HMODELSOCKET hSocket = NULL;
	LTransform tf;
	LTRotation rRot;
	g_pModelLT->GetSocket(m_hObject,"Body_Prop",hSocket);
	g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTFALSE);
	g_pTransLT->GetRot(tf,rRot);

	DRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, m_hHead, "Body_Prop", &LTVector(0,0,0), &rRot, &pAttachment);

	if (dRes != DE_OK)
	{
		g_pServerDE->RemoveObject(m_hHead);
		m_hHead = DNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AttachDefaultHead
//
//	PURPOSE:	Attach a default head
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AttachDefaultHead()
{
	// Now create our body prop to use as a head in the suit
	m_hHead = CreateExosuitHead(LTTRUE);

	// and attach it
	HATTACHMENT pAttachment;
	// Get socket.
	HMODELSOCKET hSocket = NULL;
	LTransform tf;
	LTRotation rRot;
	g_pModelLT->GetSocket(m_hObject,"Body_Prop",hSocket);
	g_pModelLT->GetSocketTransform(m_hObject, hSocket, tf, LTFALSE);
	g_pTransLT->GetRot(tf,rRot);

	g_pLTServer->CreateAttachment(m_hObject, m_hHead, "Body_Prop", &LTVector(0,0,0), &rRot, &pAttachment);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleExosuitEject
//
//	PURPOSE:	Handle getting out of the exosuit
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleExosuitEject(LTBOOL bCreatePickup)
{
	LTVector vPos;

	//call the base class
	CCharacter::HandleExosuitEject(vPos, bCreatePickup);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Swap
//
//	PURPOSE:	Swaps player with AIPlayer
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Swap(DBOOL bChangeToAIPlayer)
{
	if( bChangeToAIPlayer == m_bSwapped )
		return;

	_ASSERT( m_hAIPlayer );
	if( !m_hAIPlayer )
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Attempted to swap to non-existant AIPlayer.  Does %s_AI exist in CharacterButes.txt?",
			GetButes()->m_szName);
#endif
		return;
	}

	AIPlayer * pAIPlayer = dynamic_cast<AIPlayer*>( g_pLTServer->HandleToObject( m_hAIPlayer ) );
	_ASSERT( pAIPlayer );
	if( !pAIPlayer )
		return;

	if( bChangeToAIPlayer )
	{
		GetMovement()->SetObjectFlags(OFT_Flags, 0, LTFALSE);
		GetMovement()->SetObjectFlags(OFT_Flags2, 0, LTFALSE);

		m_fMinHealth = m_damage.GetMinHealth();
		m_bInCinematic = LTTRUE;
		m_damage.SetCanDamage(DFALSE);

		// Update the attachments so that they get a chance to make themselves invisible.
		GetAttachments()->Update();

		pAIPlayer->SetupForSwap( *this );

		pAIPlayer->Active( true );

		ChangeState(PS_SWAPPED, m_hAIPlayer);
	}
	else
	{
		GetMovement()->SetObjectUserFlags( pAIPlayer->GetMovement()->GetObjectUserFlags() );

		SetHasMask( pAIPlayer->HasMask() );
		SetHasNightVision( pAIPlayer->HasNightVision() );
		SetHasCloakDevice( pAIPlayer->HasCloakDevice() );
		SetCloakState( pAIPlayer->GetCloakState() );

		GetMovement()->SetObjectFlags(OFT_Flags, GetVars()->m_dwFlags, LTFALSE);
		GetMovement()->SetObjectFlags(OFT_Flags2, GetVars()->m_dwFlags2, LTFALSE);

		ChangeState(PS_ALIVE);

		//nuke the old AI Player and make a new one
		pAIPlayer->Active( false );
		g_pLTServer->RemoveObject( m_hAIPlayer );
		m_hAIPlayer =  CreateAIPlayer(*GetButes());
	}

	m_bSwapped = bChangeToAIPlayer;

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::IgnoreDamageMsg
//
//	PURPOSE:	See about ignoring damage
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::IgnoreDamageMsg(const DamageStruct& damage)
{
	if( IsAlien(m_cmMovement.GetCharacterButes()) && damage.eType == DT_ACID)
	{
		return LTTRUE;
	}

	return CCharacter::IgnoreDamageMsg(damage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!hWrite) return;

	// If we were waiting for auto save, we don't need to wait anymore...

	m_bWaitingForAutoSave = DFALSE;

	SAVE_FLOAT(m_fOldHitPts);
	SAVE_FLOAT(m_fOldMaxHitPts);
	SAVE_FLOAT(m_fOldMaxArmor);
	SAVE_FLOAT(m_fOldArmor);

	*hWrite << m_cc;

	// Don't save m_ClientMoveCode

	SAVE_BYTE(m_eState);

	SAVE_BYTE(m_b3rdPersonView);
	SAVE_DWORD(m_nSavedFlags);

	SAVE_BYTE(m_bGodMode);

	SAVE_BYTE(m_bAllowInput);

	SAVE_WORD(m_nClientChangeFlags);

	LTBOOL bHasClientSaveData = (m_hClientSaveData != LTNULL);
	SAVE_BYTE(bHasClientSaveData);

	if(bHasClientSaveData)
	{
		g_pLTServer->WriteToMessageHMessageRead(hWrite, m_hClientSaveData);
	}

	SAVE_HSTRING(m_hstrStartLevelTriggerTarget);
	SAVE_HSTRING(m_hstrStartLevelTriggerMessage);

	SAVE_HSTRING(m_hstrStartMissionTriggerTarget);
	SAVE_HSTRING(m_hstrStartMissionTriggerMessage);

	SAVE_BYTE(m_nWeaponStatus);

	SAVE_BYTE( m_bSwapped );

	*hWrite << m_hAIPlayer;

	*hWrite << m_fOldBatteryLevel;
	hWrite->WriteFloat(m_fIdleTime);
	*hWrite << m_bResetHealthAndArmor;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!hRead) return;

	
	m_dwLastLoadFlags = dwLoadFlags;

	LOAD_FLOAT(m_fOldHitPts);
	LOAD_FLOAT(m_fOldMaxHitPts);
	LOAD_FLOAT(m_fOldMaxArmor);
	LOAD_FLOAT(m_fOldArmor);

	*hRead >> m_cc;

	LOAD_BYTE_CAST(m_eState, PlayerState);

	LOAD_BYTE(m_b3rdPersonView);
	LOAD_DWORD(m_nSavedFlags);

	LOAD_BYTE(m_bGodMode);

	LOAD_BYTE(m_bAllowInput);

	LOAD_WORD(m_nClientChangeFlags);

	LTBOOL bClientSaveData;
	LOAD_BYTE(bClientSaveData);

	if(bClientSaveData)
	{
		m_hClientSaveData = g_pLTServer->ReadFromMessageHMessageRead(hRead);
	}
	else
	{
		m_hClientSaveData = LTNULL;
	}

	LOAD_HSTRING(m_hstrStartLevelTriggerTarget);
	LOAD_HSTRING(m_hstrStartLevelTriggerMessage);

	LOAD_HSTRING(m_hstrStartMissionTriggerTarget);
	LOAD_HSTRING(m_hstrStartMissionTriggerMessage);

	LOAD_BYTE(m_nWeaponStatus);

	LOAD_BYTE( m_bSwapped );

	if( m_hAIPlayer )
	{
		g_pLTServer->RemoveObject(m_hAIPlayer);
	}

	*hRead >> m_hAIPlayer;

	*hRead >> m_fOldBatteryLevel;
	m_fIdleTime = hRead->ReadFloat() + g_pLTServer->GetTime();

	// Load client data associated with this player...
	if (m_hClientSaveData && (m_dwLastLoadFlags == LOAD_RESTORE_GAME))
	{
		// Our m_hClient hasn't been set yet so tell all clients (just the one)
		// about this data...WILL ONLY WORK IN SINGLE PLAYER GAMES!!!!

		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(DNULL, MID_PLAYER_LOADCLIENT);
		g_pLTServer->WriteToMessageHMessageRead(hMessage, m_hClientSaveData);
		g_pLTServer->EndHMessageRead(m_hClientSaveData);
		g_pLTServer->EndMessage(hMessage);
	}
	*hRead >> m_bResetHealthAndArmor;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMusicMessage()
//
//	PURPOSE:	Process a music message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleMusicMessage(const char* szMsg)
{
    if (!szMsg) return LTFALSE;

	// Send message to client containing the music trigger information

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_MUSIC);

	// ILTServer::CreateString does not destroy szMsg so this is safe
	HSTRING hstrMusic = g_pLTServer->CreateString((char*)szMsg);
    g_pLTServer->WriteToMessageHString(hMessage, hstrMusic);
	FREE_HSTRING(hstrMusic);

    g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleCreditsMessage()
//
//	PURPOSE:	Process a Credits message
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::HandleCreditsMessage(const char* szMsg)
{
	if (!m_hClient)
		return LTFALSE;

    if (!szMsg || !szMsg[0]) 
		return LTFALSE;

    if (!g_pGameServerShell->GetGameType().IsSinglePlayer()) 
		return LTFALSE;

	uint8 nMsg = 255;

	if (_stricmp("OFF", szMsg) == 0)
		nMsg = 0;
	else if (_stricmp("ON", szMsg) == 0)
		nMsg = 1;
	else if (_stricmp("INTRO", szMsg) == 0)
		nMsg = 2;


	if (nMsg < 255)
	{
		HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_CREDITS);
		g_pLTServer->WriteToMessageByte(hMessage, nMsg);
		g_pLTServer->EndMessage2(hMessage, MESSAGE_GUARANTEED | MESSAGE_NAGGLE);
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateHistory()
//
//	PURPOSE:	Update the history if needed...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::UpdateHistory()
{
	LTFLOAT fTime = g_pLTServer->GetTime();

	// Only update the history every so often...
	if(fTime - m_fLastHistory > 0.1f)
	{
		// Mark the time
		m_fLastHistory = fTime;

		// Scooch down a bit...
		for(int i=(NUM_HISTORY-1); i; --i)
		{
			m_StampHistory[i] = m_StampHistory[i-1];
		}

		// Now make our mark
		m_StampHistory[0].fTime = fTime;
		g_pLTServer->GetObjectPos(m_hObject, &m_StampHistory[0].vPos);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ValidatePosition()
//
//	PURPOSE:	Validate an incoming shot...
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerObj::ValidatePosition(LTVector& vPos, LTFLOAT& fTime)
{
	int i = 0;

	// Ok let's test here...
	LTFLOAT fRunSpeed = m_cmMovement.GetMaxSpeed(CMS_RUNNING);
	
	// Make speed relitive to time then square
	fRunSpeed *= 0.15f;
	fRunSpeed *= fRunSpeed;

	// Need to give the players the benefit of the doubt due to 
	// client side prediction and server lag...
	fRunSpeed *= 3.0f;

	// Loop through the history and find the closest time stamp
	for(i=1; i<NUM_HISTORY; ++i)
	{
		if(m_StampHistory[i].fTime <=  fTime)
		{
			LTFLOAT fDist = (vPos - m_StampHistory[i].vPos).MagSqr();

			if(fDist > fRunSpeed)
			{
				// Hmm... too far from where I say I was...
#ifdef _DEBUG
				g_pLTServer->CPrint("Invalid hit (Within History): Distance = %.2f, Max Allowed = %.2f", fDist, fRunSpeed);
#endif
				return LTFALSE;
			}
			return LTTRUE;
		}
	}

	// Hmm if we get here then the shooter must be laggin hard, test
	// against the last history
	LTFLOAT fDist = (vPos - m_StampHistory[i].vPos).MagSqr();

	if(fDist > fRunSpeed)
	{
		// Hmm... too far from where I say I was...
#ifdef _DEBUG
		g_pLTServer->CPrint("Invalid hit (Exceeded History): Distance = %.2f, Max Allowed = %.2f", fDist, fRunSpeed);
#endif
		return LTFALSE;
	}
	return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CacheMultispawners()
//
//	PURPOSE:	Cache the skins for multispawner characters...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::CacheMultispawners()
{
	// Loop thru all active and inactive objects...
	// Find the multispawners...
	// Figure out what they are going to spawn...
	// Load up all the skins for that character type...

	for( MultiSpawner::MultiSpawnerIterator iter = MultiSpawner::BeginMultiSpawners();
	     iter != MultiSpawner::EndMultiSpawners(); ++iter )
	{
		MultiSpawner * pSpawner = *iter;
		if( pSpawner )
		{
			// Found a spawner!
			const SpawnButes & butes = g_pSpawnButeMgr->GetSpawnButes(pSpawner->GetObjectPropsId());

			// See if it is going to pump out a character
			if( !butes.m_szCharacterClass.IsEmpty() )
			{
				int nIndex = g_pCharacterButeMgr->GetSetFromModelType(butes.m_szCharacterClass);
			
				if(nIndex > 0)
				{
					CacheFiles(nIndex);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CacheFiles()
//
//	PURPOSE:	Cache resources used by this character type
//
// ----------------------------------------------------------------------- //

void CPlayerObj::CacheFiles(int nCharSet)
{
	int i = 0;

	// The default model and skins will be cached internally by the engine,
	// so we just need to cache the vision mode skins here

	// Grab the base filenames
	ObjectCreateStruct pStruct;
	pStruct.Clear();
	g_pCharacterButeMgr->GetDefaultFilenames(nCharSet, pStruct);

	// A temp buffer for the caching
	char szBuffer[256];
	char szVisionModes[3][64];

	// Go through each texture and load the normal texture
	for(i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		if(pStruct.m_SkinNames[i][0])
		{
			if(g_pLTServer->CacheFile(FT_TEXTURE, pStruct.m_SkinNames[i]) != LT_OK)
			{
#ifndef _FINAL
				g_pLTServer->CPrint("CPlayerObj::CacheFiles!!  Error caching '%s'", szBuffer);
#endif
			}
		}
	}

	// Only do the rest of this if I am a predator...
	if(!IsPredator(m_hObject)) return;

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
	for(i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		if(pStruct.m_SkinNames[i][0])
		{
			CString tempDir = g_pCharacterButeMgr->GetDefaultSkinDir(nCharSet);
			CString skinDir;
			CString skinName;
			int nLength;

			for(int j = 0; j < 3; j++)
			{
				skinDir = tempDir;
				skinDir += szVisionModes[j];
				skinName = g_pCharacterButeMgr->GetDefaultSkin(nCharSet, i, skinDir.GetBuffer());

				sprintf(szBuffer, "%s", skinName.GetBuffer());
				nLength = strlen(szBuffer);
				strcpy(szBuffer + (nLength - 3), "spr");

				if(g_pLTServer->CacheFile(FT_SPRITE, szBuffer) != LT_OK)
				{
					strcpy(szBuffer + (nLength - 3), "dtx");

					if(g_pLTServer->CacheFile(FT_TEXTURE, szBuffer) != LT_OK)
					{
#ifndef _FINAL
						g_pLTServer->CPrint("CPlayerObj::CacheFiles!!  Error caching '%s'", szBuffer);
#endif
					}
				}
			}
		}
	}
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetObservePoint_Prev()
//
//	PURPOSE:	Set the previous observation point
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SetObservePoint_Prev()
{
	// Make sure we're in observe mode...
	if(m_eState != PS_OBSERVING)
	{
		ChangeState(PS_OBSERVING);
	}

	// Set the observe location
	m_nObservePoint--;
	SetObservePoint(m_nObservePoint, LTFALSE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetObservePoint_Next()
//
//	PURPOSE:	Set the next observation point
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SetObservePoint_Next()
{
	// Make sure we're in observe mode...
	if(m_eState != PS_OBSERVING)
	{
		ChangeState(PS_OBSERVING);
	}

	// Set the observe location
	m_nObservePoint++;
	SetObservePoint(m_nObservePoint, LTFALSE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetObservePoint_Random()
//
//	PURPOSE:	Set a random observation point
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SetObservePoint_Random()
{
	// Make sure we're in observe mode...
	if(m_eState != PS_OBSERVING)
	{
		ChangeState(PS_OBSERVING);
	}

	// Set the observe location
	SetObservePoint(m_nObservePoint, LTTRUE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetObservePoint_Current()
//
//	PURPOSE:	Set the current observation point
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SetObservePoint_Current()
{
	// Make sure we're in observe mode...
	if(m_eState != PS_OBSERVING)
	{
		ChangeState(PS_OBSERVING);
	}

	// Set the observe location
	SetObservePoint(m_nObservePoint, LTFALSE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetObservePoint()
//
//	PURPOSE:	Set the player to a specific observation point
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SetObservePoint(int &nPoint, LTBOOL bRandom)
{
	// Make sure we're in observe mode...
	if(m_eState != PS_OBSERVING)
	{
		ChangeState(PS_OBSERVING);
	}


	// Get all the start points...
	ObjArray <HOBJECT, 100> objArray;
	SpectatorPoint** pStartPtArray = LTNULL;
	int nObjects;

	// Find all the objects with the name "SpectatorPoint"
	g_pLTServer->FindNamedObjects("SpectatorPoint", objArray);
	nObjects = objArray.NumObjects();

	// If there aren't any start points at all, return NULL
	if(!nObjects)
	{
#ifndef _FINAL
		g_pLTServer->CPrint("PlayerObj: WARNING!! No SpectatorPoints in this level! Using GameStartPoints instead.");
#endif
		// Use the "GameStartPoint" objects instead...
		g_pLTServer->FindNamedObjects("GameStartPoint", objArray);
		nObjects = objArray.NumObjects();

		if(!nObjects)
		{
#ifndef _FINAL
			g_pLTServer->CPrint("PlayerObj: WARNING!! Damn, no GameStartPoints either... you're level is fucked!");
#endif
			return;
		}
	}


	// Now make sure this point exists
	if(bRandom && (nObjects > 1))
	{
		int nOldPoint = nPoint;

		while(nOldPoint == nPoint)
		{
			nPoint = GetRandom(0, nObjects - 1);
		}
	}
	else
	{
		if(nPoint < 0)
			nPoint = nObjects - 1;
		else if(nPoint >= nObjects)
			nPoint = 0;
	}


	// Get the actual point handle
	HOBJECT hPtObject = objArray.GetObject(nPoint);
	SpectatorPoint *pPt = dynamic_cast<SpectatorPoint*>(g_pLTServer->HandleToObject(hPtObject));
	GameStartPoint *pSt = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(hPtObject));

	if(pPt || pSt)
	{
		HMESSAGEWRITE hMessage;
		LTVector vPos;
		LTRotation rRot;

		g_pLTServer->GetObjectPos(hPtObject, &vPos);
		g_pLTServer->GetObjectRotation(hPtObject, &rRot);


		// Set the position
		g_pLTServer->SetObjectPos(m_hObject, &vPos);
		TeleportClientToServerPos();


		// Inform the client of the correct camera/player orientation...
		hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_ORIENTATION);
		g_pLTServer->WriteToMessageByte(hMessage, LTTRUE);
		g_pLTServer->WriteToMessageRotation(hMessage, &rRot);
		g_pLTServer->EndMessage(hMessage);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetPlayerWeapon()
//
//	PURPOSE:	Get the current player weapon data...
//
// --------------------------------------------------------------------------- //

WEAPON* CPlayerObj::GetPlayerWeapon()
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;
	CWeapons* pWeapons = LTNULL;


	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	// Make sure we're displaying the default weapon...
	if (pPlayerAttachments)
		CWeapons* pWeapons = pPlayerAttachments->GetWeapons();

	if(pWeapons)
	{
		CWeapon* pWeapon = pWeapons->GetCurWeapon();
		if (pWeapon)
			return const_cast<WEAPON*>(pWeapon->GetWeaponData());
	}

	return LTNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendFXButes()
//
//	PURPOSE:	Resend the FX butes...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::SendFXButes(HCLIENT hClient)
{
	// Confirm who we are since we may have changed since the game started...
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_RESET_ATTRIBUTES);
	g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_nCharacterSet);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST | MESSAGE_GUARANTEED);

	// Now confirm the MP info...
	g_pGameServerShell->GetMultiplayerMgr()->VerifyClientData(hClient);

	// Confirm our weapon FX...
	UpdateCharacterWeaponFX(this, GetPlayerWeapon());
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::MPRespawn()
//
//	PURPOSE:	Do a combo reset character and respawn...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::MPRespawn(uint8 nCharId)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	//if we have a head, lose it
	if(m_hHead)
	{
		HATTACHMENT hAttachment;
		if (g_pServerDE->FindAttachment(m_hObject, m_hHead, &hAttachment) == DE_OK)
		{
			g_pServerDE->RemoveAttachment(hAttachment);
		}
		g_pServerDE->RemoveObject(m_hHead);
		m_hHead = DNULL;
	}

	// Tell other characters about our change
	CCharacter::SetCharacter(nCharId, LTTRUE);

	// Make sure all is well with this character
	int nNewChar = -1;
	if(!g_pGameServerShell->GetMultiplayerMgr()->ValidateRespawn(this, &nNewChar))
		return;

	// If validate respawn changed us we need to tell
	// the others again...
	if(nNewChar != -1)
		// Tell other characters about our change
		CCharacter::SetCharacter(nCharId, LTTRUE);

	m_cc = g_pCharacterButeMgr->GetClass(m_nCharacterSet);

	// Force the cloak off
	ResetCloak();

	// Get a start point...
	int nClass = g_pCharacterButeMgr->GetClass(m_nCharacterSet);

	// Get the special case queen character set...
	int nQueenCharSet = g_pCharacterButeMgr->GetSetFromModelType("queen");

	switch(nClass)
	{
		case MARINE:	nClass = Marine;	break;
		case ALIEN:		nClass = Alien;		break;
		case PREDATOR:	nClass = Predator;	break;
		case CORPORATE:	nClass = Corporate;	break;
	}

	GameStartPoint* pStartPt = g_pGameServerShell->FindStartPoint(LTNULL, nClass, this, (m_nCharacterSet == (uint32)nQueenCharSet));

	LTVector vPos(0.0f, 0.0f, 0.0f);
	LTVector vVec(0.0f, 0.0f, 0.0f);

	if(pStartPt)
	{
		// Set our starting position...
		g_pLTServer->GetObjectPos(pStartPt->m_hObject, &vPos);
		vVec = pStartPt->GetPitchYawRoll();
	} 


	// Reposition the destination spawn location to be resting on the floor
	LTVector vDims = m_cmMovement.GetDimsSetting(CM_DIMS_DEFAULT);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_Direction = LTVector(0.0f, -1.0f, 0.0f);

	IQuery.m_Flags	   = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	IQuery.m_FilterFn  = SpawnPointFilter;
	IQuery.m_pUserData = NULL;

    if(g_pLTServer->CastRay(&IQuery, &IInfo))
	{
        LTFLOAT fDist = vPos.y - IInfo.m_Point.y;

		if(fDist > vDims.y)
		{
			vPos.y -= (fDist - (vDims.y + 1.0f));
		}
	}


	// Setup the object flags for teleporting to the location
	uint32 dwFlags;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | FLAG_GOTHRUWORLD);

	g_pLTServer->TeleportObject(m_hObject, &vPos);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags & ~FLAG_GOTHRUWORLD);

	LTRotation rRot;
	g_pMathLT->SetupEuler(rRot, vVec.x, vVec.y, vVec.z);
	m_cmMovement.SetRot(rRot);

	// Make sure that anyone else that's at this location gets punished!
	TeleFragObjects(vPos);

	// Set the initial spawn invulnerability
	{
		// Get the current position of the player...
		m_vSpawnPosition = vPos;
		m_fSpawnTime = g_pLTServer->GetTime();

		ToggleGodMode(LTTRUE, -1.0f, LTTRUE, LTTRUE);

		// set our spawn-in flag
		m_bSpawnInState = LTTRUE;
	}

	// Make sure all the net stuff is cleaned up...
	m_eNetStage = NET_STAGE_INVALID;
	m_eLastNetStage = NET_STAGE_INVALID;

	// Let the animation go...
	m_caAnimation.StopAnim();
	m_caAnimation.Enable(LTTRUE);

	// Reset the character hps and armor
	Reset();

	// Reset all the weapons

	if(pPlayerAttachments)
		pPlayerAttachments->ResetAllWeapons();

	m_bWaitingForAutoSave = DFALSE;

	// Update their view position so they get the sfx message.
	g_pLTServer->SetClientViewPos(m_hClient, &vPos);

	// Give the player some default equipment
	SetHasFlarePouch();
	SetHasCloakDevice();
	SetHasMask(LTTRUE, LTFALSE);
	SetHasNightVision();

	//load up the battery power
	m_fBatteryChargeLevel = MARINE_MAX_BATTERY_LEVEL;

	// Get our position
	LTVector myPos;
	g_pLTServer->GetObjectPos(m_hObject, &myPos);

	// Tell the player about the new move code.
	++m_ClientMoveCode;

	// Set the new player state
	m_eState = PS_ALIVE;

	// Go ahead and tell the player to reset their inventory
	// we need to do this because ADW below will give a bunch of weapons and ammo
	// that we need to keep track of.
	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(m_hClient, MID_PLAYER_INFOCHANGE);
	g_pLTServer->WriteToMessageByte(hMessage, IC_RESET_INVENTORY_ID);
	g_pLTServer->EndMessage(hMessage);

	// Make sure we always have and start with the default weapon...
	int nWeapon = AquireDefaultWeapon(LTFALSE);

	// Now do the MP respawn thing...
	hMessage = g_pLTServer->StartMessage(m_hClient, MID_MP_RESPAWN);
	// Character ID
	g_pLTServer->WriteToMessageByte(hMessage, (uint8)GetCharacter());
	// Move code
	g_pLTServer->WriteToMessageByte(hMessage, m_ClientMoveCode);
	// Player position
	g_pLTServer->WriteToMessageVector(hMessage, &myPos);
	// Player orientation
	g_pLTServer->WriteToMessageVector(hMessage, &vVec);
	// This is our new weapon
	g_pLTServer->WriteToMessageByte(hMessage, (uint8)g_pWeaponMgr->GetCommandId(nWeapon));
	// Send the new interface stuff
	SendMPInterfaceUpdate(hMessage);
	// And away we go...
	g_pLTServer->EndMessage(hMessage);

	// Give the player some default equipment
	SetHasFlarePouch();
	SetHasCloakDevice();

	SetHasMask(LTTRUE, LTFALSE);
	SetHasNightVision();

	UpdateInterface(LTTRUE, LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateInterface
//
//	PURPOSE:	Tell the client of about any changes
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SendMPInterfaceUpdate(HMESSAGEWRITE hMessage)
{
	CPlayerAttachments* pPlayerAttachments = LTNULL;
	int i;
	int nAmmo = 0;

	pPlayerAttachments = dynamic_cast<CPlayerAttachments*>(m_pAttachments);

	// See if the ammo has changed...
	DBYTE nNumAmmoPools = g_pWeaponMgr->GetNumAmmoPools();

	// See how many ammos we need to update
	uint8 nCount = 0;
	for (i=0; i < nNumAmmoPools; i++)
	{
		if (pPlayerAttachments)
			nAmmo = pPlayerAttachments->GetAmmoPoolCount(i);
		
		if(nAmmo > 0)
			nCount++;
	}

	// Write the count
	g_pLTServer->WriteToMessageByte(hMessage, nCount);

	// Now write the amounts
	for (i=0; i < nNumAmmoPools; i++)
	{
		if (pPlayerAttachments)
			nAmmo = pPlayerAttachments->GetAmmoPoolCount(i);

		if(nAmmo > 0)
		{
			g_pLTServer->WriteToMessageByte(hMessage, i);
			g_pLTServer->WriteToMessageFloat(hMessage, (DFLOAT)nAmmo);
		}
	}


	// Write our MaxHealth
	m_fOldMaxHitPts = m_damage.GetMaxHitPoints();
	g_pLTServer->WriteToMessageFloat(hMessage, m_fOldMaxHitPts);

	// Write our normal health
	m_fOldHitPts = m_damage.GetHitPoints();
	g_pLTServer->WriteToMessageFloat(hMessage, m_fOldHitPts);

	// Write our MaxArmor..
	m_fOldMaxArmor = m_damage.GetMaxArmorPoints();
	g_pLTServer->WriteToMessageFloat(hMessage, m_fOldMaxArmor);

	// Write our normal armor
	m_fOldArmor = m_damage.GetArmorPoints();
	g_pLTServer->WriteToMessageFloat(hMessage, m_fOldArmor);

	// Write our airlevel
	LTFLOAT fTotalAirTime = GetButes()->m_fAirSupplyTime;
	LTFLOAT fPercent = m_fAirSupplyTime / fTotalAirTime;
	g_pLTServer->WriteToMessageFloat(hMessage, fPercent);

	// Write our SC charge level
	LTFLOAT fLevel = (LTFLOAT)m_nCannonChargeLevel;
	g_pLTServer->WriteToMessageFloat(hMessage, fLevel);

	// Write our battery level
	g_pLTServer->WriteToMessageFloat(hMessage, m_fBatteryChargeLevel);
}

