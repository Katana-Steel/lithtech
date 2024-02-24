// ----------------------------------------------------------------------- //
//
// MODULE  : GameStartPoint.cpp
//
// PURPOSE : GameStartPoint - Definition
//
// CREATED : 9/30/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameStartPoint.h"
#include "cpp_server_de.h"
#include "CharacterFuncs.h"
#include "GameServerShell.h"
#include "ServerSoundMgr.h"
#include "DebrisMgr.h"
#include "PickupObject.h"
#include "Spawner.h"
#include "CVarTrack.h"

#include "SurfaceMgr.h"

extern int MP_EXOSUIT_HP_MULTIPLIER;

// ----------------------------------------------------------------------- //

uint32 GameStartPoint::m_dwCounter = 0;
CVarTrack	g_ExoRespawnDelay;

// ----------------------------------------------------------------------- //

BEGIN_CLASS(GameStartPoint)
	
	ADD_STRINGPROP_FLAG_HELP(CharacterType, "Harris", PF_STATICLIST, "This will be the kind of character that the player will start as when spawned (in single player story mode only). These are read out of the CharacterButes.txt attribute file.")

	ADD_BOOLPROP_FLAG_HELP(AllowAliens, LTTRUE, 0, "This determines whether Aliens can spawn at this point or not in a multiplayer game.")
	ADD_BOOLPROP_FLAG_HELP(AllowMarines, LTTRUE, 0, "This determines whether Marines can spawn at this point or not in a multiplayer game.")
	ADD_BOOLPROP_FLAG_HELP(AllowPredators, LTTRUE, 0, "This determines whether Predators can spawn at this point or not in a multiplayer game.")
	ADD_BOOLPROP_FLAG_HELP(AllowCorporates, LTTRUE, 0, "This determines whether Corporates can spawn at this point or not in a multiplayer game.")
	ADD_BOOLPROP_FLAG_HELP(AllowQueens, LTFALSE, 0, "This determines whether Alien Queens can spawn at this point or not in a multiplayer game.")
	ADD_BOOLPROP_FLAG_HELP(AllowExosuits, LTFALSE, 0, "This determines whether an Exosuit can spawn at this point or not in a multiplayer game.")
	ADD_BOOLPROP_FLAG_HELP(AllowTeam1, LTTRUE, 0, "For game modes that require team selection. (Hunt = Hunters; Survivor = Survivors; Overrun = Defenders; Evac = Defenders)")
	ADD_BOOLPROP_FLAG_HELP(AllowTeam2, LTTRUE, 0, "For game modes that require team selection. (Hunt = Prey; Survivor = Mutants; Overrun = Attackers; Evac = Attackers)")
	ADD_BOOLPROP_FLAG_HELP(AllowTeam3, LTTRUE, 0, "Currently unused. (for future expansion)")
	ADD_BOOLPROP_FLAG_HELP(AllowTeam4, LTTRUE, 0, "Currently unused. (for future expansion)")
	ADD_OBJECTPROP_FLAG_HELP(TriggerTarget, "", 0, "This is a link to the object which will get a trigger message when a player spawns from this point.")
	ADD_STRINGPROP_FLAG_HELP(TriggerMessage, "", 0, "This is the message that the trigger target receives.")

	ADD_OBJECTPROP_FLAG_HELP(StartMissionTarget, "", 0, "This is a link to the object which will get a trigger message when a player starts a mission (or loads via custom level).")
	ADD_STRINGPROP_FLAG_HELP(StartMissionMessage, "", 0, "This is the message that the start mission target receives.")

	ADD_BOOLPROP_FLAG_HELP(HasOpeningCinematic, LTFALSE, 0, "Does this level start with an opening cinematic?  /n If this value is true then sounds will be muted until the cinematic begins.")

	ADD_STRINGPROP_FLAG_HELP(MPStartCommand, "", 0, "This is the message that gets sent every time a this /n level is loaded in a multiplayer game..")

END_CLASS_DEFAULT_FLAGS_PLUGIN(GameStartPoint, GameBase, NULL, NULL, CF_ALWAYSLOAD, CGameStartPointPlugin)

// ----------------------------------------------------------------------- //

extern void CacheCharacterFiles( int nCharacterType ); // in Character.cpp

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::GameStartPoint
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

GameStartPoint::GameStartPoint() : GameBase()
{ 
	m_nCharacterSet			= g_pCharacterButeMgr->GetSetFromModelType("Harris");
	m_nAllowFlags			= STARTPOINT_ALLOW_ALL;
	m_hstrName				= LTNULL;
	m_hstrTriggerTarget		= LTNULL;
	m_hstrTriggerMessage	= LTNULL;
	m_hstrMissionTarget		= LTNULL;
	m_hstrMissionMessage	= LTNULL;
	m_bHasOpenCine			= LTFALSE;
	m_hstrMPTriggerMessage		= LTNULL;
	m_vPitchYawRoll.Init();
	m_fRespawnStartTime		= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::~GameStartPoint
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

GameStartPoint::~GameStartPoint()
{ 
	if(m_hstrName)
	{
		g_pLTServer->FreeString(m_hstrName);
		m_hstrName = LTNULL;
	}

	if(m_hstrTriggerTarget)
	{
		g_pLTServer->FreeString(m_hstrTriggerTarget);
		m_hstrTriggerTarget = LTNULL;
	}

	if(m_hstrTriggerMessage)
	{
		g_pLTServer->FreeString(m_hstrTriggerMessage);
		m_hstrTriggerMessage = LTNULL;
	}

	if(m_hstrMPTriggerMessage)
	{
		g_pLTServer->FreeString(m_hstrMPTriggerMessage);
		m_hstrMPTriggerMessage = LTNULL;
	}

	if(m_hstrMissionTarget)
	{
		g_pLTServer->FreeString(m_hstrMissionTarget);
		m_hstrMissionTarget = LTNULL;
	}

	if(m_hstrMissionMessage)
	{
		g_pLTServer->FreeString(m_hstrMissionMessage);
		m_hstrMissionMessage = LTNULL;
	}

	--m_dwCounter;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GameStartPoint::EngineMessageFn(uint32 messageID, void *pData, DFLOAT fData)
{
	uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE)
				{
					ReadProp(pStruct);
				}

				SAFE_STRCPY(pStruct->m_Name, "GameStartPoint");
			}			
		}
		break;

		case MID_INITIALUPDATE:
		{
			if(!dwRet) break;

			// Only count valid start points.
			++m_dwCounter;

			SetNextUpdate(0.0f, 0.001f);

			CacheFiles();

			if( g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY )
			{
#ifndef _FINAL
				if( m_dwCounter > 1 )
				{
					g_pLTServer->CPrint("GameStartPoint ERROR!  %d start points found in a single player game!", m_dwCounter);
				}
#endif
			}

			if(!g_ExoRespawnDelay.IsInitted())
				g_ExoRespawnDelay.Init(g_pLTServer, "ExosuitRespawnDelay", DNULL, 15.0f);

		}
		break;

		case MID_UPDATE:
			{
				// see if it is time to respawn our exosuit
				if(g_pLTServer->GetTime() - m_fRespawnStartTime > g_ExoRespawnDelay.GetFloat())
				{
					// respawn away
					SpawnExosuit();

					// turn off updating
					SetNextUpdate(0,0);
				}
				else
				{
					// keep updating
					SetNextUpdate(0.1f, 1.0f);
				}
			}
			break;

		default : break;
	}

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL GameStartPoint::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

	if (g_pLTServer->GetPropGeneric("CharacterType", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) 
		{
			m_nCharacterSet = g_pCharacterButeMgr->GetSetFromModelType(genProp.m_String);
		}
	}
	
	if (g_pLTServer->GetPropGeneric("AllowAliens", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_ALIENS;
	}

	if (g_pLTServer->GetPropGeneric("AllowMarines", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_MARINES;
	}

	if (g_pLTServer->GetPropGeneric("AllowPredators", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_PREDATORS;
	}

	if (g_pLTServer->GetPropGeneric("AllowCorporates", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_CORPORATES;
	}

	if (g_pLTServer->GetPropGeneric("AllowQueens", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_QUEENS;
	}

	if (g_pLTServer->GetPropGeneric("AllowExosuits", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_EXOSUITS;
	}

	if (g_pLTServer->GetPropGeneric("AllowTeam1", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_TEAM_1;
	}

	if (g_pLTServer->GetPropGeneric("AllowTeam2", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_TEAM_2;
	}

	if (g_pLTServer->GetPropGeneric("AllowTeam3", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_TEAM_3;
	}

	if (g_pLTServer->GetPropGeneric("AllowTeam4", &genProp) == DE_OK)
	{
		if (!genProp.m_Bool)
			m_nAllowFlags &= ~STARTPOINT_ALLOW_TEAM_4;
	}

	if (g_pLTServer->GetPropGeneric("HasOpeningCinematic", &genProp) == DE_OK)
	{
		m_bHasOpenCine = genProp.m_Bool;
	}

	if (g_pLTServer->GetPropGeneric("Name", &genProp) == DE_OK)
	{
		if (genProp.m_String[0]) 
		{
			m_hstrName = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("TriggerTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("TriggerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("MPStartCommand", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrMPTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("StartMissionTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrMissionTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	if (g_pLTServer->GetPropGeneric("StartMissionMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrMissionMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

	g_pLTServer->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::CacheFiles
//
//	PURPOSE:	Cache files associated with this object
//
// ----------------------------------------------------------------------- //

void GameStartPoint::CacheFiles()
{
	if (!m_hObject) return;

	// Don't cache if the world already loaded...
	if (!(g_pLTServer->GetServerFlags() & SS_CACHING)) return;


	if (m_dwCounter == 1)
	{
		if (g_pSurfaceMgr)
		{
#ifndef _FINAL
			StartTimingCounter();
#endif
			g_pSurfaceMgr->CacheAll();
#ifndef _FINAL
			EndTimingCounter("GameStartPoint::CacheSurfaceFiles()");
#endif
		}

		// Cache certain FX graphics
		LTBOOL bPassed = LTTRUE;

		if(g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY)
		{
			if(IsSynthetic(m_nCharacterSet) || IsHuman(m_nCharacterSet))
			{
#ifdef _WIN32
				if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\nvoverlay.spr") != LT_OK)		bPassed = LTFALSE;
				if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\railgunoverlay.spr") != LT_OK)	bPassed = LTFALSE;
#else
				if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/nvoverlay.spr") != LT_OK)		bPassed = LTFALSE;
				if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/railgunoverlay.spr") != LT_OK)	bPassed = LTFALSE;
#endif

				// Special human stuff...
				if( g_pWeaponMgr && g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY )
				{
					CString szAppend = g_pCharacterButeMgr->GetWeaponSkinType(m_nCharacterSet);

					WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Blowtorch");
					if(pWeap) pWeap->Cache(g_pWeaponMgr, &szAppend);

					pWeap = g_pWeaponMgr->GetWeapon("Flare_Pouch_Weapon");
					if(pWeap) pWeap->Cache(g_pWeaponMgr, &szAppend);

					pWeap = g_pWeaponMgr->GetWeapon("MarineHackingDevice");
					if(pWeap) pWeap->Cache(g_pWeaponMgr, &szAppend);
				}
			}
			else if(IsAlien(m_nCharacterSet))
			{
#ifdef _WIN32
				if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\alienoverlay.spr") != LT_OK)		bPassed = LTFALSE;
#else
				if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/alienoverlay.spr") != LT_OK)		bPassed = LTFALSE;
#endif
				// Always cache the pounce weapon...  And tear...
				CString szAppend = g_pCharacterButeMgr->GetWeaponSkinType(m_nCharacterSet);

				WEAPON* pWeap = g_pWeaponMgr->GetWeapon("PounceJump");
				if(pWeap) pWeap->Cache(g_pWeaponMgr, &szAppend);

				pWeap = g_pWeaponMgr->GetWeapon("tear");
				if(pWeap) pWeap->Cache(g_pWeaponMgr, &szAppend);
			}
			else if(IsPredator(m_nCharacterSet))
			{
#ifdef _WIN32
				if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\eoverlay.spr") != LT_OK)			bPassed = LTFALSE;
				if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\h2overlay.spr") != LT_OK)		bPassed = LTFALSE;
				if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\predzoom.spr") != LT_OK)			bPassed = LTFALSE;
#else
				if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/eoverlay.spr") != LT_OK)			bPassed = LTFALSE;
				if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/h2overlay.spr") != LT_OK)		bPassed = LTFALSE;
				if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/predzoom.spr") != LT_OK)			bPassed = LTFALSE;
#endif

				// Special human stuff...
				if( g_pWeaponMgr && g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY )
				{
					CString szAppend = g_pCharacterButeMgr->GetWeaponSkinType(m_nCharacterSet);

					WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Energy_Sift");
					if(pWeap) pWeap->Cache(g_pWeaponMgr, &szAppend);

					pWeap = g_pWeaponMgr->GetWeapon("Medicomp");
					if(pWeap) pWeap->Cache(g_pWeaponMgr, &szAppend);
				}
			}
		}
		else
		{
			// Cache all overlay stuff for MP
#ifdef _WIN32
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aClaws_drone_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aClaws_runner_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aClaws_predalien_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aClaws_praetorian_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aClaws_queen_pv.dtx") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aTail_drone_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aTail_runner_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aTail_predalien_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aTail_praetorian_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Alien\\aTail_queen_pv.dtx") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\nvoverlay.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\railgunoverlay.spr") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\alienoverlay.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\eoverlay.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\h2overlay.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\predzoom.spr") != LT_OK)			bPassed = LTFALSE;
			
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Marine\\mhands_white_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Marine\\mhands_black_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Marine\\mhands_hazmat_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Marine\\mhands_guard_pv.dtx")!= LT_OK)	bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pRightHand_pred_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pLeftHand_pred_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pRightHand_light_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pLeftHand_light_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pRightHand_mhawk_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pLeftHand_mhawk_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pRightHand_heavy_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins\\Weapons\\Predator\\pLeftHand_heavy_pv.dtx")!= LT_OK)	bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\MarBld1.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\MarBld2.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\MarBld3.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\MarBld4.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\MarBld5.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\MarBld6.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\PrdBld1.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\PrdBld2.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\PrdBld3.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\PrdBld4.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\PrdBld5.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\PrdBld6.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\AlnBld1.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\AlnBld2.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\AlnBld3.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\AlnBld4.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\AlnBld5.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\AlnBld6.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\BloodSplat1.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\BloodSplat2.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\BloodSplat3.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\BloodSplat4.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\cloak.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\lasercursor.spr") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\sfx\\particles\\OnFire0.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\sfx\\particles\\OnFire1.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\sfx\\particles\\OnFire2.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\sfx\\particles\\aura.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\sfx\\particles\\heat.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\sfx\\particles\\e_aura.spr") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\interface\\playerinfo_on.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\interface\\playerselect.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\interface\\playerselect_h.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\interface\\playerinfo_off.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\interface\\invulnerable_off.wav") != LT_OK)	bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\powerups\\pu_armor.wav") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\powerups\\pu_health.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "sounds\\powerups\\destroy.wav") != LT_OK)			bPassed = LTFALSE;

#else
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aClaws_drone_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aClaws_runner_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aClaws_predalien_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aClaws_praetorian_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aClaws_queen_pv.dtx") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aTail_drone_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aTail_runner_pv.dtx") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aTail_predalien_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aTail_praetorian_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Alien/aTail_queen_pv.dtx") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/nvoverlay.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/railgunoverlay.spr") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/alienoverlay.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/eoverlay.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/h2overlay.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/predzoom.spr") != LT_OK)			bPassed = LTFALSE;
			
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Marine/mhands_white_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Marine/mhands_black_pv.dtx") != LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Marine/mhands_hazmat_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Marine/mhands_guard_pv.dtx")!= LT_OK)	bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pRightHand_pred_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pLeftHand_pred_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pRightHand_light_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pLeftHand_light_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pRightHand_mhawk_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pLeftHand_mhawk_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pRightHand_heavy_pv.dtx")!= LT_OK)	bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_TEXTURE, "Skins/Weapons/Predator/pLeftHand_heavy_pv.dtx")!= LT_OK)	bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/MarBld1.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/MarBld2.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/MarBld3.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/MarBld4.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/MarBld5.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/MarBld6.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/PrdBld1.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/PrdBld2.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/PrdBld3.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/PrdBld4.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/PrdBld5.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/PrdBld6.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/AlnBld1.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/AlnBld2.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/AlnBld3.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/AlnBld4.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/AlnBld5.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/AlnBld6.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/BloodSplat1.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/BloodSplat2.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/BloodSplat3.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/BloodSplat4.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/cloak.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/lasercursor.spr") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/SFX/Particles/OnFire0.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/SFX/Particles/OnFire1.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/SFX/Particles/OnFire2.spr") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/SFX/Particles/aura.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/SFX/Particles/heat.spr") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/SFX/Particles/e_aura.spr") != LT_OK)		bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Interface/playerinfo_on.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Interface/playerselect.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Interface/playerselect_h.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Interface/playerinfo_off.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Interface/invulnerable_off.wav") != LT_OK)	bPassed = LTFALSE;

			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Powerups/pu_armor.wav") != LT_OK)			bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Powerups/pu_health.wav") != LT_OK)		bPassed = LTFALSE;
			if(g_pLTServer->CacheFile(FT_SOUND, "Sounds/Powerups/destroy.wav") != LT_OK)			bPassed = LTFALSE;
#endif	// _WIN32


			DEBRIS* pDebris = g_pDebrisMgr->GetDebris("Pickup_Destroy");
			if(pDebris) pDebris->Cache(g_pDebrisMgr);

			// This will error on the weapons that need special skins parsed in
			// but it's only one texture per some models and it's not worth looping
			// through all the MP characters and caching the same other files
			// over and over and over....
			WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Flare_Pouch_Weapon");
			if(pWeap) pWeap->Cache(g_pWeaponMgr, LTNULL);

			pWeap = g_pWeaponMgr->GetWeapon("PounceJump");
			if(pWeap) pWeap->Cache(g_pWeaponMgr, LTNULL);
			
			pWeap = g_pWeaponMgr->GetWeapon("tear");
			if(pWeap) pWeap->Cache(g_pWeaponMgr, LTNULL);

			pWeap = g_pWeaponMgr->GetWeapon("Energy_Sift");
			if(pWeap) pWeap->Cache(g_pWeaponMgr, LTNULL);
			
			pWeap = g_pWeaponMgr->GetWeapon("Medicomp");
			if(pWeap) pWeap->Cache(g_pWeaponMgr, LTNULL);
		}

		// Always cahce these....

#ifdef _WIN32
		if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\onfireoverlay.spr") != LT_OK)		bPassed = LTFALSE;
		if(g_pLTServer->CacheFile(FT_SPRITE, "sprites\\cloak.spr") != LT_OK)				bPassed = LTFALSE;
#else
		if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/onfireoverlay.spr") != LT_OK)		bPassed = LTFALSE;
		if(g_pLTServer->CacheFile(FT_SPRITE, "Sprites/cloak.spr") != LT_OK)				bPassed = LTFALSE;
#endif

#ifndef _FINAL
		if(!bPassed)
			g_pLTServer->CPrint("GameStartPoint::CacheFiles!!  An FX graphic did not get cached properly!");
#endif
		// Now cache all the weapon stuff...
		if( g_pWeaponMgr && g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY )
		{
			CString szAppend = g_pCharacterButeMgr->GetWeaponSkinType(m_nCharacterSet);

			const CharacterButes & butes = g_pCharacterButeMgr->GetCharacterButes(m_nCharacterSet);

			WEAPON_SET* pSet = g_pWeaponMgr->GetWeaponSet(butes.m_nWeaponSet);

			if(pSet)
			{
				for(int i=0; i<pSet->nNumWeapons; i++)
				{
					WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(pSet->szWeaponName[i]);
					if (pWeaponData)
					{
						pWeaponData->Cache(g_pWeaponMgr, &szAppend);
					}
				}
			}
		}
		else if(g_pWeaponMgr)
		{
			// In multiplayer cache everything!
			int nNumWeapons = g_pWeaponMgr->GetNumWeapons();

			for(int i=0 ; i<nNumWeapons ; i++)
			{
				WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(i);
				if (pWeaponData && pWeaponData->bMPCache)
				{
					pWeaponData->Cache(g_pWeaponMgr);
				}
			}
		}

		// Cache our character sounds in single player...
		if(g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY)
		{
			CacheCharacterFiles(m_nCharacterSet);
		}
		else
		{
			for(int i=0 ; i < g_pCharacterButeMgr->GetNumSets() ; i++)
			{
				CString str = g_pCharacterButeMgr->GetMultiplayerModel(i);

				LTBOOL bSpecial=LTFALSE;
				const char* szName = g_pCharacterButeMgr->GetName(i);

				if(stricmp(szName,"Queen")==0)
					bSpecial = LTTRUE;
				else if(stricmp(szName,"Exosuit")==0)
					bSpecial = LTTRUE;

				if(strstr(str.GetBuffer(), ".abc") || bSpecial)
				{
					// cache the sounds here...
					CacheCharacterFiles(i);

					// cache the vison mode skins here...
					ObjectCreateStruct pStruct;
					pStruct.Clear();
					g_pCharacterButeMgr->GetDefaultFilenames(i, pStruct);

					// cache the model itself...
					if(pStruct.m_Filename[0]) g_pLTServer->CacheFile(FT_MODEL, pStruct.m_Filename);

					// cache all the normal skins
					if(pStruct.m_SkinNames[0][0]) g_pLTServer->CacheFile(FT_TEXTURE, pStruct.m_SkinNames[0]);
					if(pStruct.m_SkinNames[1][0]) g_pLTServer->CacheFile(FT_TEXTURE, pStruct.m_SkinNames[1]);
					if(pStruct.m_SkinNames[2][0]) g_pLTServer->CacheFile(FT_TEXTURE, pStruct.m_SkinNames[2]);
					if(pStruct.m_SkinNames[3][0]) g_pLTServer->CacheFile(FT_TEXTURE, pStruct.m_SkinNames[3]);

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
					for(int j = 0; j < MAX_MODEL_TEXTURES; j++)
					{
						if(pStruct.m_SkinNames[j][0])
						{
							CString tempDir = g_pCharacterButeMgr->GetDefaultSkinDir(i);
							CString skinDir;
							CString skinName;
							int nLength;

							for(int k = 0; k < 3; k++)
							{
								skinDir = tempDir;
								skinDir += szVisionModes[k];
								skinName = g_pCharacterButeMgr->GetDefaultSkin(i, j, skinDir.GetBuffer());

								sprintf(szBuffer, "%s", skinName.GetBuffer());
								nLength = strlen(szBuffer);
								strcpy(szBuffer + (nLength - 3), "spr");

								if(g_pLTServer->CacheFile(FT_SPRITE, szBuffer) != LT_OK)
								{
									strcpy(szBuffer + (nLength - 3), "dtx");

									g_pLTServer->CacheFile(FT_TEXTURE, szBuffer);
								}//if(g_pLTServer->CacheFile(FT_SPRITE, szBuffer) != LT_OK)
							}//for(int k = 0; k < 3; k++)
						}//if(pStruct.m_SkinNames[j][0])
					}//for(int j = 0; j < MAX_MODEL_TEXTURES; i++)
				}//f(strstr(str.GetBuffer(), ".abc"))
			}//for(int i=0 ; i < g_pCharacterButeMgr->GetNumSets() ; i++)
		}//if(g_pGameServerShell && g_pGameServerShell->GetGameType() == SP_STORY)
	}//if (m_dwCounter == 1)
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::SpawnExosuit
//
//	PURPOSE:	Spawn in an exosuit if necessary....
//
// ----------------------------------------------------------------------- //

void GameStartPoint::SpawnExosuit()
{
	// see if we are the right kind of startpoint...
	if(!(m_nAllowFlags & STARTPOINT_ALLOW_EXOSUITS)) return;

	//spawn a new powerup
	char pStr[255];
	sprintf(pStr,"PickupObject Pickup Exosuit_Device");
	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	PickupObject* pExoClass = dynamic_cast<PickupObject*>(SpawnObject( pStr, vPos, rRot));

	if(pExoClass)
	{
		// Be sure we don't rotate...
		pExoClass->SetAllowMovement(LTFALSE);
		pExoClass->SetTeleportToFloor(LTTRUE);
		pExoClass->SetStartpointObject(m_hObject);

		// Now set our max hps
		int nId = g_pCharacterButeMgr->GetSetFromModelType("Exosuit");
		pExoClass->SetHitpoints(g_pCharacterButeMgr->GetMaxHitPoints(nId)*6);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameStartPoint::SetExosuitRespawn
//
//	PURPOSE:	Set us up to respawn an exosuit....
//
// ----------------------------------------------------------------------- //

void GameStartPoint::SetExosuitRespawn()
{
	// time stamp
	m_fRespawnStartTime = g_pLTServer->GetTime();

	// make sure we get updated
	SetNextUpdate(0.1f, 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameStartPointPlugin::PreHook_EditStringList()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DRESULT	CGameStartPointPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_stricmp("CharacterType", szPropName) == 0)
	{
		m_CharacterButeMgrPlugin.SetPlayerOnly();
		m_CharacterButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, 
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
