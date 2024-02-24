// ----------------------------------------------------------------------- //
//
// MODULE  : GameServerShell.cpp
//
// PURPOSE : The game's server shell - Implementation
//
// UPDATED : 3/29/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>

#include "cpp_server_de.h"
#include "GameServerShell.h"

#include "MsgIDs.h"
#include "ObjectMsgs.h"
#include "CommandIDs.h"
#include "CheatDefs.h"
#include "ServerUtilities.h"

#include "Sparam.h"
#include "CSDefs.h"
#include "AssertMgr.h"

#include "CharacterButeMgr.h"
#include "CharacterFuncs.h"
#include "PlayerObj.h"
#include "GameStartPoint.h"

#include "DialogueWindow.h"
#include "WorldProperties.h"
#include "GameBase.h"
#include "DebugLineSystem.h"
#include "ModelButeMgr.h"
#include "MissionMgr.h"
#include "LightGroup.h"
#include "MusicMgr.h"
#include "AIVolumeMgr.h"
#include "MultiplayerMgr.h"
#include "StarlightView.h"
#include "Weapons.h"
#include "ServerAssetMgr.h"

#include "ServerRes.h"

// ----------------------------------------------------------------------- //

int iIdentCode0 = 0x4212A901;
char szCopywriteString[] = "© 2001 Twentieth Century Fox Film Corporation.  All rights reserved.  Aliens versus Predator, Fox Interactive and their respective logos are either registered trademarks or trademarks of Twentieth Century Fox Film Corporation.";
int iIdentCode1 = 0x4212A901;

// ----------------------------------------------------------------------- //

#define CLIENT_PING_UPDATE_RATE		3.0f
#define UPDATENAME_INTERVAL			10.0f

// ----------------------------------------------------------------------- //
// Stuff to create a GameServerShell.

SETUP_SERVERSHELL()

// ----------------------------------------------------------------------- //

CDialogueWindow		g_DialogueWindow;

#ifndef _FINAL
CVarTrack			g_ShowDimsTrack;
CVarTrack			g_ShowTimingTrack;
CVarTrack			g_ShowTriggersTrack;
CVarTrack			g_ShowTriggersFilter;
CVarTrack			g_DamageScale;
CVarTrack			g_HealScale;
CVarTrack			g_ShowAIVolTrack;
#endif

CVarTrack			g_EE_MillerTime;

CVarTrack			g_vtLowViolence;

// ----------------------------------------------------------------------- //

CGameServerShell*	g_pGameServerShell = LTNULL;
GameStartPoint*		g_pLastPlayerStartPt = LTNULL;
ILTServer*			g_pInterface = LTNULL;

static int			g_nNumClients = 0;

static char			s_LevelCacheFileName[128];

// Our current save/load version.
// Formatted as YYYYMMDDN (year, month, day, number).  So that it will always be increasing.
uint32 CGameServerShell::m_nCurrentSaveLoadVersion = 200111210;

extern void CacheCharacterFiles( int nCharacterType ); // in Character.cpp

// ----------------------------------------------------------------------- //

IServerShell* CreateServerShell(ILTServer *pLTServer)
{
	//AfxSetAllocStop(29048);

	// Redirect our asserts as specified by the assert convar 
#ifdef _WIN32
	CAssertMgr::Enable();
#endif

	// Set our global pointer

	g_pInterface = g_pLTServer = pLTServer;

	// Set up our CRC string

#ifdef _WIN32
	g_pLTServer->SetCRCString(	"CShell.DLL; \
								Attributes\\CharacterButes.TXT; \
								Attributes\\CharacterSounds.TXT; \
								Attributes\\Debris.TXT; \
								Attributes\\FX.TXT; \
								Attributes\\ModelButes.TXT; \
								Attributes\\PickupButes.TXT; \
								Attributes\\SoundButes.TXT; \
								Attributes\\Surface.TXT; \
								Attributes\\VisionModeButes.TXT; \
								Attributes\\Weapons.TXT; \
								Models\\Weapons\\Alien\\*_pv.ABC; \
								Models\\Weapons\\Marine\\*_pv.ABC; \
								Models\\Weapons\\Exosuit\\*_pv.ABC; \
								Models\\Weapons\\Predator\\*_pv.ABC");
#else
	g_pLTServer->SetCRCString(	"cshell.dll; \
								Attributes/CharacterButes.txt; \
								Attributes/CharacterSounds.txt; \
								Attributes/Debris.txt; \
								Attributes/FX.txt; \
								Attributes/ModelButes.txt; \
								Attributes/PickupButes.txt; \
								Attributes/SoundButes.txt; \
								Attributes/Surface.txt; \
								Attributes/VisionModeButes.txt; \
								Attributes/Weapons.txt; \
								Models/Weapons/Alien/*_pv.abc; \
								Models/Weapons/Marine/*_pv.abc; \
								Models/Weapons/Exosuit/*_pv.abc; \
								Models/Weapons/Predator/*_pv.abc");
#endif

	// Set up the LT subsystem pointers

	g_pMathLT = g_pLTServer->GetMathLT();
	g_pModelLT = g_pLTServer->GetModelLT();
	g_pTransLT = g_pLTServer->GetTransformLT();
	g_pPhysicsLT = g_pLTServer->Physics();
	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);

	// Set up global console trackers...

#ifndef _FINAL
	g_ShowDimsTrack.Init(g_pLTServer, "ShowDims", LTNULL, 0.0f);
	g_ShowTimingTrack.Init(g_pLTServer, "ShowTiming", "0", 0.0f);
	g_ShowTriggersTrack.Init(g_pLTServer, "ShowTriggers", "0", 0.0f);
	g_ShowTriggersFilter.Init(g_pLTServer, "FilterTriggerMsgs", "", 0.0f);
	g_DamageScale.Init(g_pLTServer, "DamageScale", LTNULL, 1.0f);
	g_HealScale.Init(g_pLTServer, "HealScale", LTNULL, 1.0f);
	g_ShowAIVolTrack.Init(g_pLTServer, "ShowAIVol", LTNULL, 0.0f);
#endif

	g_EE_MillerTime.Init(g_pLTServer, "EE_MillerTime", LTNULL, 0.0f);

	*s_LevelCacheFileName = '\0';


	// Setup the violence setting
#ifdef _WIN32
	HSTRING hViolenceStr = g_pLTServer->FormatString(IDS_LOWVIOLENCE);
	g_vtLowViolence.Init(g_pLTServer, "LowViolence", LTNULL, (LTFLOAT)stricmp(g_pLTServer->GetStringData(hViolenceStr), "Off"));
	g_pLTServer->FreeString(hViolenceStr);
#else
	// Ignore this in Linux builds for now
#endif


	// Make sure we are using autodeactivation...

	g_pLTServer->RunGameConString("autodeactivate 1.0");

	CGameServerShell *pShell = new CGameServerShell(pLTServer);

	g_pGameServerShell = pShell;

	return (IServerShell*)pShell;
}

// ----------------------------------------------------------------------- //

void DeleteServerShell(IServerShell *pInputShell)
{
	CGameServerShell *pShell = (CGameServerShell*)pInputShell;

	// g_pGameServerShell = LTNULL; 
	// (kls - 2/22/98 - CreateSeverShell() is called BEFORE
	// DeleteServerShell() is called so we CAN'T set this to NULL)

	delete pShell;

	// Return the report hook to the normal CRT function

#ifdef _WIN32
	CAssertMgr::Disable();
#endif
}

// ----------------------------------------------------------------------- //

LTBOOL g_bInfiniteAmmo = LTFALSE;

void* CGameServerShell::GetAuthContext()
{ 
	return m_pMPMgr->GetAuthContext(); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CGameServerShell()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CGameServerShell::CGameServerShell(ILTServer *pLTServer)
{
	m_pMPMgr = new MultiplayerMgr;

	// Perform our CRC checks if this is a multiplayer game
	if (m_pMPMgr->GetGameType().IsMultiplayer())
	{
		g_pLTServer->CreateStringCRC();
	}

	m_bFirstUpdate		= LTTRUE;
	m_bAlarm			= LTFALSE;
	m_nPrevMissionId = -1;
	m_bDoAutoSave		= LTFALSE;

	m_nLastLGFlags		= LOAD_NEW_GAME;
	m_nLastSaveLoadVersion = 0;

	// Initialize all the globals...
	m_bGlobalMgrValid = m_GlobalMgr.Init(g_pLTServer);

	m_nEvacZoneObjects = 0;
	m_vEvacZonePos.Init();

	// FOX CODES TO PREVENT OPTIMIZING OUT THE VARIABLES
	iIdentCode0 = iIdentCode0;
	szCopywriteString[0] = szCopywriteString[0];
	iIdentCode1 = iIdentCode1;


	g_nNumClients = 0;
	m_bPlayerMuted = LTFALSE;

	m_bLoadingKeepAlive = LTFALSE;
	m_bHasQueenStartPoint = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::~CGameServerShell()
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

CGameServerShell::~CGameServerShell()
{
	if(m_pMPMgr)
	{
		m_pMPMgr->CleanUp();

		delete m_pMPMgr;
		m_pMPMgr = LTNULL;
	}

	m_nEvacZoneObjects = 0;
	m_vEvacZonePos.Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::LowViolence()
//
//	PURPOSE:	Is the game set to low violence?
//
// ----------------------------------------------------------------------- //

LTBOOL CGameServerShell::LowViolence()
{
	HCONVAR m_hVar = g_pLTServer->GetGameConVar("LowViolence");
	return (LTBOOL)(m_hVar && g_pLTServer->GetVarValueFloat(m_hVar));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::GetGameInfo()
//
//	PURPOSE:	Return the game info
//
// ----------------------------------------------------------------------- //

MPGameInfo* CGameServerShell::GetGameInfo() const
{
	if(!m_pMPMgr)
		return LTNULL;

	return m_pMPMgr->GetGameInfo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::GetGameType()
//
//	PURPOSE:	Return the game type used by the multiplayer manager...
//				or just return SP_STORY
//
// ----------------------------------------------------------------------- //

GameType CGameServerShell::GetGameType() const
{
	if(!m_pMPMgr)
		return GameType(SP_STORY);

	return m_pMPMgr->GetGameType();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnAddClient()
//
//	PURPOSE:	Called when a client connects to a server
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnAddClient(HCLIENT hClient)
{
	g_nNumClients++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnRemoveClient()
//
//	PURPOSE:	Called when a client disconnects from a server
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnRemoveClient(HCLIENT hClient)
{
	// Clear our client data...

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hClient);
	if (pPlayer)
	{
		pPlayer->SetClient(LTNULL);
	}

	g_nNumClients--;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnClientEnterWorld()
//
//	PURPOSE:	Add a client
//
// ----------------------------------------------------------------------- //

void CGameServerShell::VerifyClient(HCLIENT hClient, void *pClientData, uint32 &nVerifyCode)
{
	if (!hClient || !pClientData)
		return;

	// If it's a multiplayer game... check for the correct password immediately
	if(GetGameType().IsMultiplayer())
	{
		MPClientData *pData = (MPClientData*)pClientData;
		MPGameInfo *pInfo = GetGameInfo();

		if(pInfo)
		{
			// Use g_nNumClients instead of the value from MPMgr because it'll be
			// update faster (therefore giving better results).
			if(g_nNumClients > pInfo->m_nPlayerLimit)
			{
				nVerifyCode = LT_REJECTED;
				return;
			}

			if(pInfo->m_bLocked)
			{
				if(stricmp(pInfo->m_szPassword, pData->m_szPassword))
				{
					nVerifyCode = LT_ERROR;
					return;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnClientEnterWorld()
//
//	PURPOSE:	Add a client
//
// ----------------------------------------------------------------------- //

LPBASECLASS CGameServerShell::OnClientEnterWorld(HCLIENT hClient, void *pClientData, uint32 clientDataLen)
{
	if (!hClient) return LTNULL;

	CPlayerObj* pPlayer  = LTNULL;
	g_pLastPlayerStartPt = LTNULL;
	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);

	LTBOOL bFoundClient = LTFALSE;
	char sClientName[MAX_MP_CLIENT_NAME_LENGTH];
	char sClientRefName[MAX_MP_CLIENT_NAME_LENGTH];
	sClientName[0] = sClientRefName[0] = '\0';

	g_pLTServer->GetClientName(hClient, sClientName, MAX_MP_CLIENT_NAME_LENGTH - 1);

	// Search through the client refs to see if any of them match our
	// client...

	HCLIENTREF hClientRef = g_pLTServer->GetNextClientRef(LTNULL);
	while (hClientRef)
	{
		// See if this client reference is local or not...

		if (g_pLTServer->GetClientRefInfoFlags(hClientRef) & CIF_LOCAL)
		{
			bFoundClient = LTTRUE;
		}


		// Determine if there is a reference to a client with the same
		// name...

		if (!bFoundClient && g_pLTServer->GetClientRefName(hClientRef, sClientRefName, MAX_MP_CLIENT_NAME_LENGTH - 1))
		{
			if (sClientName[0] && sClientRefName[0])
			{
				if (_stricmp(sClientName, sClientRefName) == 0)
				{
					bFoundClient = LTTRUE;
				}
			}
		}


		// See if we found the right client...

		if (bFoundClient)
		{
			HOBJECT	hObject = g_pLTServer->GetClientRefObject(hClientRef);
			pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);
			break;
		}

		hClientRef = g_pLTServer->GetNextClientRef(hClientRef);
	}

	// If single-player game, we need to refresh the game info.
	// This needs to happen before the player is created.
	if( GetGameType() == SP_STORY )
	{
		m_pMPMgr->StartSinglePlayer();
	}

	// See if we need to create a player (no matches found)...
	if(!pPlayer)
	{
		pPlayer = CreatePlayer(hClient, pClientData, clientDataLen);
	}

	// Player object meet client, client meet player object...
	if(pPlayer)
	{
		pPlayer->SetClient(hClient);
		g_pLTServer->SetClientUserData(hClient, (void*)pPlayer);

		// Add this client to our local list...
		m_pMPMgr->AddClient(hClient, (MPClientData*)pClientData);

		pPlayer->Respawn(m_nLastLGFlags);

		// Set the server-side view pos to match the client
		// side view pos.
		// If this isn't done before a saved-game load is finished,
		// the player object position from the previous game will
		// be used to check visibility!
		LTVector vPlayerPos;
		ASSERT( pPlayer->m_hObject );
		if( pPlayer->m_hObject )
		{
			g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vPlayerPos);
			g_pLTServer->SetClientViewPos(hClient, &vPlayerPos);
		}

		// Send starlight messages down...
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

		g_pServerDE->FindNamedObjects("StarlightObject",objArray);

		for(uint32 i = 0; i < objArray.NumObjects(); i++)
		{
			StarLightView* pSLView = dynamic_cast<StarLightView*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));
			
			if(pSLView)
				pSLView->SendEffectMessage(hClient);
		}


		// (ALM 10/4/01) Commented this out to try and help modem players connect better...
		// there's another method of this that gets requested from the client when his
		// character FX is created. So this portion of code doesn't seem like it's even
		// needed at all.

/*		if(GetGameType().IsMultiplayer())
		{
			CPlayerObj* pPlayerObj=LTNULL;
			// Verify that all the character match up...
			for (CCharacterMgr::PlayerIterator iter = g_pCharacterMgr->BeginPlayers();
				 iter != g_pCharacterMgr->EndPlayers(); ++iter)
			{
				if (*iter)
				{
					pPlayerObj = *iter;

					pPlayerObj->SendFXButes(hClient);
				}
			}
		}
*/	}


	g_pLTServer->SetClientInfoFlags(hClient, CIF_AUTOACTIVATEOBJECTS);

	// All done...

	return pPlayer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnClientExitWorld()
//
//	PURPOSE:	remove a client
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnClientExitWorld(HCLIENT hClient)
{
	// Remove this client from our local list...

	m_pMPMgr->RemoveClient(hClient);


	// Remove the player object...

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hClient);

	if(pPlayer) 
	{
		pPlayer->RemoveObject();
	}

	g_pLTServer->SetClientUserData(hClient, LTNULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PreStartWorld()
//
//	PURPOSE:	Handle pre start world
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PreStartWorld(LTBOOL bSwitchingWorlds)
{		
	m_charMgr.PreStartWorld();
	// Terminate the musicmgr.
	g_pMusicMgr->Term();

	g_DialogueWindow.Term();

	m_CmdMgr.ClearCommands();

	LightManager::GetInstance()->Reset();

	if(GetGameType().IsMultiplayer())
	{
		// Tell the server app.
		m_pMPMgr->ServerAppPreStartWorld( );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PostStartWorld()
//
//	PURPOSE:	Handle post switch world
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PostStartWorld()
{
	HOBJECT hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
	{
		GameBase * pGameBase = dynamic_cast<GameBase*>( g_pLTServer->HandleToObject(hCurObject) );
		if( pGameBase )
		{
			pGameBase->PostStartWorld(m_nLastLGFlags);
		}
	}

	hCurObject = LTNULL;
	while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
	{
		GameBase * pGameBase = dynamic_cast<GameBase*>( g_pLTServer->HandleToObject(hCurObject) );
		if( pGameBase )
		{
			pGameBase->PostStartWorld(m_nLastLGFlags);
		}
	}

	// This must occur after all the objects get their PostStartWorld message.
	m_charMgr.PostStartWorld(m_nLastLGFlags);

	g_DialogueWindow.Init();


	if(GetGameType().IsMultiplayer())
	{
		// Tell the server app.
		m_pMPMgr->ServerAppPostStartWorld( );

		// Create the CRC for the world
		g_pLTServer->CreateWorldCRC();

		// Process any MP startpoint messages
		ProcessMPStartPointMessages();

		// Spawn in any exosuits
		SpawnMPExosuits();

		// Check our start points
		m_bHasQueenStartPoint = ValidateQueenStartPoints();
	}
	else
	{
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnMessage()
//
//	PURPOSE:	Handle messages
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnMessage(HCLIENT hSender, uint8 messageID, HMESSAGEREAD hMessage)
{
	// Let multiplayer handle messages first...
	if(m_pMPMgr->OnMessage(hSender, messageID, hMessage)) return;


	switch (messageID)
	{
		case MID_PLAYER_UPDATE :
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;

			if (pPlayer)
			{
				pPlayer->HandleMessage_UpdateClient(hMessage);
			}
		}
		break;

		case MID_PLAYER_VERIFY_CFX :
		{
			HOBJECT hChar = g_pLTServer->ReadFromMessageObject(hMessage);

			if(hChar)
			{
				CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hChar) );
				if( pChar && pChar->GetCurrentWeaponPtr())
				{
					UpdateCharacterWeaponFX(pChar, pChar->GetCurrentWeaponPtr()->GetWeaponData());
				}
			}
		}
		break;

		case MID_MPLAYER_VERIFY_CFX :
		{
			for( CCharacterMgr::PlayerIterator iter = g_pCharacterMgr->BeginPlayers();
				  iter != g_pCharacterMgr->EndPlayers(); ++iter )
			{
	  			CPlayerObj* pPlayer = *iter;

				if(pPlayer)
				{
					if(pPlayer->GetClient() != hSender)
					{
						pPlayer->SendFXButes(hSender);
					}
				}
			}
		}
		break;

		case MID_FORCE_POUNCE_JUMP:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleMessage_PounceJump(hMessage);
			}
		}
		break;

		case MID_WEAPON_FIRE:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleMessage_WeaponFire(hMessage);
			}
		}
		break;

		case MID_WEAPON_HIT:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleMessage_WeaponHit(hMessage);
			}
		}
		break;

		case MID_WEAPON_SOUND:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleMessage_WeaponSound(hMessage);
			}
		}
		break;

		case MID_WEAPON_LOOPSOUND:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleMessage_WeaponLoopSound(hMessage);
			}
		}
		break;

		case MID_WEAPON_CHANGE:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				uint8 nWeaponId = g_pLTServer->ReadFromMessageByte(hMessage);
				pPlayer->DoWeaponChange(nWeaponId);
			}
		}
		break;

		case MID_PLAYER_ACTIVATE:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleMessage_Activate(hMessage);
			}
		}
		break;

		case MID_PLAYER_CLIENTMSG:
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (pPlayer)
			{
				pPlayer->HandleClientMsg(hMessage);
			}
		}
		break;

		case MID_FRAG_SELF:
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
			if(pPlayer)
			{		
				LPBASECLASS pClass = g_pLTServer->HandleToObject(pPlayer->m_hObject);
				if (pClass)
				{
					DamageStruct damage;

					damage.eType	= DT_EXPLODE;
					damage.fDamage	= damage.kInfiniteDamage;
					damage.hDamager = pPlayer->m_hObject;
					damage.vDir.Init(0, 1, 0);

					damage.DoDamage(pClass, pPlayer->m_hObject);
				}
			}
		}
		break;

		case MID_PLAYER_RESPAWN :
		{
			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;

			if(pPlayer && pPlayer->GetState() == PS_DEAD)
			{
				LTBOOL bCanRespawn = LTTRUE;

				// Don't allow the player to respawn if the server is changing their class...
				// They'll get respawned automatically.
				if(GetGameType().IsMultiplayer())
				{
					MPMgrData *pData = m_pMPMgr->GetClientData(pPlayer->GetClient());

					if(pData && (pData->m_nNextClass < MAX_TEAMS))
						bCanRespawn = LTFALSE;
				}

				if(bCanRespawn)
					pPlayer->Respawn();
			}
		}
		break;

		case MID_PLAYER_CHEAT :
		{
			// get a pointer to the sender's player object

			void *pData = g_pLTServer->GetClientUserData(hSender);
			CPlayerObj* pPlayer = (CPlayerObj*)pData;
			if (!pPlayer) return;

			// retrieve message data

			int nCheatCode = (int) g_pLTServer->ReadFromMessageByte (hMessage);
			uint8 nData = g_pLTServer->ReadFromMessageByte (hMessage);

			// now deal with the specific cheat code

			HandleCheatCode(pPlayer, nCheatCode, nData, hMessage);
		}
		break;

		case MID_PLAYER_EXITLEVEL:
		{
			HandlePlayerExitLevel(hSender, hMessage);
		}
		break;

		case MID_SPEED_CHEAT:
		{
			m_pMPMgr->KickSpeedCheater(hSender);
		}
		break;

		case MID_GAME_PAUSE:
		{
			PauseGame(LTTRUE);
		}
		break;

		case MID_GAME_UNPAUSE:
		{
			PauseGame(LTFALSE);
		}
		break;

		case MID_LOAD_GAME :
		{
			HandleLoadGameMsg(hSender, hMessage);
		}
		break;

		case MID_SAVE_GAME :
		{
			HandleSaveGameMsg(hSender, hMessage);
		}
		break;

		case MID_SINGLEPLAYER_START :
		{
			if (GetGameType() == SP_STORY)
			{
				void *pData = g_pLTServer->GetClientUserData(hSender);
				CPlayerObj* pPlayer = (CPlayerObj*)pData;
				if (pPlayer)
				{
					pPlayer->StartLevel();
				}

				const char * szDifficultyDesc = "Unknown";
				if(GetGameType().GetDifficulty() < g_nDescDifficulty )
				{
					szDifficultyDesc = g_pDescDifficulty[GetGameType().GetDifficulty()];
				}

#ifndef _FINAL
				g_pLTServer->CPrint( "Difficulty is \"%s\".", szDifficultyDesc );
#endif
			}
		}
		break;

		case MID_CONSOLE_TRIGGER :
		{
			HandleConsoleTriggerMsg(hSender, hMessage);
		}
		break;

		case MID_CONSOLE_COMMAND :
		{
			HandleConsoleCmdMsg(hSender, hMessage);
		}
		break;

		case MID_SFX_MESSAGE:
		{
			HandleSFXMsg(hSender,hMessage);
		}
		break;

		case MID_CMD_ID_CLOAK:
		{

		}
		break;

		case MID_CMD_ID_DISC_RETRIEVE:
		{

		}
		break;

		case MID_CMD_ID_MEDI_COMP:
		{

		}
		break;

		case MID_CMD_ID_ENERGY_SIFT:
		{

		}
		break;

		case MID_CMD_ID_TORCH_SELECT:
		{

		}
		break;

		case MID_CMD_ID_HACKING_DEVICE:
		{

		}
		break;

		case MID_CMD_ID_VULNERABLE:
		{

		}
		break;

		case CSM_DIALOGUE_DONE:
		{
			// Tell our global dialogue manager that the dialogue is done
			g_DialogueWindow.Finished();
		}
		break;

		case CSM_DIALOGUE_DONE_SELECTION:
		{
			// Tell our global dialogue manager that the dialogue is done
			uint8 byDecision = g_pInterface->ReadFromMessageByte(hMessage);
			uint32 dwID = g_pInterface->ReadFromMessageDWord(hMessage);
			g_DialogueWindow.Finished(byDecision,dwID);
		}
		break;

		case CSM_DIALOGUE_STOP:
		{
			g_DialogueWindow.StopDialogue();
		}
		break;

		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleCheatCode()
//
//	PURPOSE:	Handle the various cheat codes
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleCheatCode(CPlayerObj* pPlayer, int nCheatCode, uint8 nData, HMESSAGEREAD hMessage)
{
	if (nCheatCode <= CHEAT_NONE || nCheatCode >= CHEAT_MAX) return;

	m_pMPMgr->ClientCheating(pPlayer->GetClient());

	switch (nCheatCode)
	{
		case CHEAT_GOD :
		{
			pPlayer->ToggleGodMode(nData, g_pLTServer->ReadFromMessageFloat(hMessage));
		}
		break;

		case CHEAT_AMMO :
		{
			pPlayer->FullAmmoCheat();
		}
		break;

		case CHEAT_ARMOR :
		{
			pPlayer->RepairArmorCheat();
		}
		break;

		case CHEAT_HEALTH :
		{
			pPlayer->HealCheat();
		}
		break;

		case CHEAT_TELEPORT :
		{
			if(nData && hMessage)
			{
				LTVector vPos;
				g_pLTServer->ReadFromMessageVector(hMessage, &vPos);
				pPlayer->Teleport(vPos);
			}
			else
				pPlayer->Respawn();
		}
		break;

		case CHEAT_FULL_WEAPONS :
		{
			pPlayer->FullWeaponCheat();
		}
		break;

		case CHEAT_TEARS:
		{
			g_bInfiniteAmmo = !g_bInfiniteAmmo;

			if (g_bInfiniteAmmo)
			{
				pPlayer->FullWeaponCheat();
			}
		}
		break;

		case CHEAT_KFA:
		{
			pPlayer->FullWeaponCheat();
			pPlayer->HealCheat();
			pPlayer->RepairArmorCheat();
		}
		break;

		case CHEAT_REMOVEAI:
		{
			HandleCheatRemoveAI(nData);
		}
		break;

		case CHEAT_CHARACTER:
		{
			if(nData)
			{
				g_pCharacterButeMgr->Reload(g_pLTServer);
				pPlayer->SetCharacter(pPlayer->GetCharacter());
			}
			else
			{
				pPlayer->SetCharacter(g_pLTServer->ReadFromMessageByte(hMessage));

				if(IsExosuit(pPlayer->m_hObject))
					pPlayer->AttachDefaultHead();
			}

			//be sure we have our default weapons
			pPlayer->ForceCloakOff();
			pPlayer->AquireDefaultWeapon();
		}
		break;

		case CHEAT_RELOADBUTES:
		{
			char szString[100];
			szString[0] = 0;
			hMessage->ReadStringFL(szString, sizeof(szString));

			// See if we're supposed to reload an attribute file
			if(!stricmp(szString, "character"))
			{
				g_pCharacterButeMgr->Reload(g_pLTServer);
				pPlayer->SetCharacter(pPlayer->GetCharacter());
			}
			else if(!stricmp(szString, "model"))
			{
				g_pModelButeMgr->Reload(g_pLTServer);
			}
		}
		break;

		case CHEAT_MILLERTIME:
		{
			g_EE_MillerTime.SetFloat(nData ? 1.0f : 0.0f);
		}
		break;

		default:
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::OnCommandOn()
//
//	PURPOSE:	Handle commands
//
// ----------------------------------------------------------------------- //

void CGameServerShell::OnCommandOn(HCLIENT hClient, int command)
{
	if (!hClient) return;

	void *pData = g_pLTServer->GetClientUserData(hClient);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (!pPlayer) return;


	// Don't do anything if we're dying, dead, or observing...
	if((pPlayer->GetState() == PS_OBSERVING) || (pPlayer->GetState() == PS_DEAD) || (pPlayer->GetState() == PS_DYING))
		return;


	switch (command)
	{
		case COMMAND_ID_CLOAK:
		{
			pPlayer->HandleCloakToggle();
			break;
		}

		case COMMAND_ID_DISC_RETRIEVE: 
		{
			pPlayer->HandleDiskRetrieve();
			break;
		}

		case COMMAND_ID_MEDI_COMP :
		{
			if(!pPlayer->IsDead() && IsPredator(pPlayer->GetCharacter()))
			{
				// First see if we need any health..
				LTBOOL bFullHealth = pPlayer->GetDestructible()->GetMaxHitPoints() == pPlayer->GetDestructible()->GetHitPoints();

				// See if we have enuf ammo to use the medicomp...
				AMMO*	pAmmo	= g_pWeaponMgr->GetAmmo("Medicomp_Ammo");
				uint32	nEnergy = pPlayer->GetPredatorEnergy();

				if(!bFullHealth && nEnergy >= (uint32)pAmmo->nAmmoPerShot)
				{
					WEAPON* pCurWep = pPlayer->GetPlayerWeapon();
					WEAPON* pMedicomp = g_pWeaponMgr->GetWeapon("Medicomp");
					WEAPON* pSift = g_pWeaponMgr->GetWeapon("Energy_Sift");

					if(pMedicomp && !pPlayer->NeedsWeapon(pMedicomp) && pCurWep != pSift && pCurWep != pMedicomp)
					{
						pPlayer->ChangeWeapon("Medicomp");

						pPlayer->DoWeaponChange(pMedicomp->nId);
					}
				}
				else
				{
					// Send failed sound here
					HMESSAGEWRITE hMsg = g_pInterface->StartMessage(pPlayer->GetClient(), MID_MEDI_FAIL);
					g_pInterface->EndMessage(hMsg);
				}
			}
			break;
		}

		case COMMAND_ID_ENERGY_SIFT :
		{
			if(!pPlayer->IsDead() && IsPredator(pPlayer->GetCharacter()))
			{
				if(pPlayer->GetPredatorEnergy() != pPlayer->GetMaxPredatorEnergy())
				{
					WEAPON* pCurWep = pPlayer->GetPlayerWeapon();
					WEAPON* pMedicomp = g_pWeaponMgr->GetWeapon("Medicomp");
					WEAPON* pSift = g_pWeaponMgr->GetWeapon("Energy_Sift");

					if(pSift && !pPlayer->NeedsWeapon(pSift) && pCurWep != pMedicomp && pCurWep != pSift)
					{
						pPlayer->ChangeWeapon("Energy_Sift");

						pPlayer->DoWeaponChange(pSift->nId);
					}
				}
				else
				{
					// Send failed sound here
					HMESSAGEWRITE hMsg = g_pInterface->StartMessage(pPlayer->GetClient(), MID_MEDI_FAIL);
					g_pInterface->EndMessage(hMsg);
				}
			}
			break;
		}

		case COMMAND_ID_TORCH_SELECT :
		{
			if(!pPlayer->IsDead() && IsHuman(pPlayer->GetCharacter()))
			{
				WEAPON* pWeapon = g_pWeaponMgr->GetWeapon("Blowtorch");
				WEAPON* pCurWep = pPlayer->GetPlayerWeapon();

				if(pWeapon && !pPlayer->NeedsWeapon(pWeapon) && pCurWep != pWeapon)
				{
					pPlayer->ChangeWeapon("Blowtorch");

					pPlayer->DoWeaponChange(pWeapon->nId);
				}
			}
			break;
		}

		case COMMAND_ID_HACKING_DEVICE :
		{
			if(!pPlayer->IsDead())
			{
				if(IsHuman(pPlayer->GetCharacter()))
				{
					WEAPON* pWeapon = g_pWeaponMgr->GetWeapon("MarineHackingDevice");
					WEAPON* pCurWep = pPlayer->GetPlayerWeapon();

					if(pWeapon && !pPlayer->NeedsWeapon(pWeapon) && pCurWep != pWeapon)
					{
						pPlayer->ChangeWeapon("MarineHackingDevice");

						pPlayer->DoWeaponChange(pWeapon->nId);
					}
				}
				else if(IsPredator(pPlayer->GetCharacter()))
				{
					WEAPON* pWeapon = g_pWeaponMgr->GetWeapon("PredatorHackingDevice");
					WEAPON* pCurWep = pPlayer->GetPlayerWeapon();
					WEAPON* pMedicomp = g_pWeaponMgr->GetWeapon("Medicomp");
					WEAPON* pSift = g_pWeaponMgr->GetWeapon("Energy_Sift");

					if(pWeapon && !pPlayer->NeedsWeapon(pWeapon) && pCurWep != pMedicomp && pCurWep != pSift && pCurWep != pWeapon)
					{
						pPlayer->ChangeWeapon("PredatorHackingDevice");

						pPlayer->DoWeaponChange(pWeapon->nId);
					}
				}
			}
			break;
		}

		case COMMAND_ID_FIRING:
		case COMMAND_ID_ALT_FIRING:
		{
//			pPlayer->PlayingCinematic(LTTRUE);
			break;
		}

		default:
			break;
	}


	// Check to see if initial spawn god mode needs to be turned off
	if(GetGameType().IsMultiplayer() && (m_pMPMgr->GetServerSubState() != MPMGR_SUBSTATE_SURVIVOR_INTERM))
	{
		switch(command)
		{
			case COMMAND_ID_FORWARD:
			case COMMAND_ID_BACKWARD:
			case COMMAND_ID_LEFT:
			case COMMAND_ID_RIGHT:
			case COMMAND_ID_STRAFE:
			case COMMAND_ID_STRAFE_LEFT:
			case COMMAND_ID_STRAFE_RIGHT:
			case COMMAND_ID_RUN:
			case COMMAND_ID_DUCK:
			case COMMAND_ID_JUMP:
			case COMMAND_ID_LOOKUP:
			case COMMAND_ID_LOOKDOWN:
			case COMMAND_ID_CENTERVIEW:
			case COMMAND_ID_RUNLOCK:
			case COMMAND_ID_CROUCH_TOGGLE:
			case COMMAND_ID_WALLWALK:
			case COMMAND_ID_WALLWALK_TOGGLE:
			case COMMAND_ID_PREV_HUD:
			case COMMAND_ID_NEXT_HUD:
			case COMMAND_ID_MOUSEAIMTOGGLE:
			case COMMAND_ID_CROSSHAIRTOGGLE:
			case COMMAND_ID_MESSAGE:
			case COMMAND_ID_TEAMMESSAGE:
			case COMMAND_ID_SCOREDISPLAY:
			case COMMAND_ID_PLAYERINFO:
			{
				// That's right... do nothing on these
				break;
			}

			default:
			{
				// Turn off god mode on any other key press
				if(pPlayer->InGodMode())
					pPlayer->ToggleGodMode(LTFALSE, -1.0f);

				break;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ProcessPacket()
//
//	PURPOSE:	Packet processing information from ILTServer
//
// ----------------------------------------------------------------------- //

LTRESULT CGameServerShell::ProcessPacket(char* sData, uint32 dataLen, uint8 senderAddr[4], uint16 senderPort)
{
	m_pMPMgr->ProcessPacket(sData, dataLen, senderAddr, senderPort);

	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::AddCharacterToCache()
//
//	PURPOSE:	Adds a character to the list of characters to be cached.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::AddCharacterToCache(int id)
{
	// Only cache the character if we are in a cache-able mood.
	if( g_pLTServer->GetServerFlags( ) & SS_CACHING )
	{
		// Only add the character if it isn't in the list already.
		CharactersToCacheList::const_iterator iter 
			= std::find(m_aCharactersToCache.begin(), m_aCharactersToCache.end(), id);

		if( iter == m_aCharactersToCache.end() )
		{
			m_aCharactersToCache.push_back(id);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CacheFiles()
//
//	PURPOSE:	Cache files that are used often
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CacheFiles()
{
	// Cache some specific files all the time if it's a multiplayer game...
	if(GetGameType().IsMultiplayer())
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Caching multiplayer models...");
#endif
		// Cache all the human base models
#ifdef _WIN32
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_mbase.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_mbase_death.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_mbase_kn1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_mbase_ps1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_mbase_rf1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_mbase_sdr.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_mbase_smg.abc");

		// Cache all the predator base models
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_pbase.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_pbase_death.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_pbase_ps1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_pbase_sc1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_pbase_sp1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\mp_pbase_wb1.abc");

		// Cache all the alien models
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\runner.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\drone.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\predalien.abc");
        g_pLTServer->CacheFile(FT_MODEL, "models\\characters\\mp\\preatorian.abc");

		// All multiplayer games cache the same resources...
	
		SetCacheFileName("worlds/multi/multi.txt");
#else
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_mbase.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_mbase_Death.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_mbase_Kn1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_mbase_Ps1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_mbase_Rf1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_mbase_Sdr.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_mbase_Smg.abc");

		// Cache all the predator base models
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_pbase.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_pbase_Death.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_pbase_Ps1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_pbase_Sc1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_pbase_Sp1.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/MP_pbase_Wb1.abc");

		// Cache all the alien models
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/runner.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/Drone.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/predalien.abc");
        g_pLTServer->CacheFile(FT_MODEL, "Models/Characters/MP/preatorian.abc");

		// All multiplayer games cache the same resources...
	
		SetCacheFileName("Worlds/Multi/Multi.txt");
#endif

	}
	else
	{
		// Initialize the music mgr if it isn't initialized yet.
		// This is in cache files so that the caching will be efficient.
		if( g_pMusicMgr && !g_pMusicMgr->IsValid() )
		{
			HCONVAR hSong = g_pLTServer->GetGameConVar("Song");
			if( hSong )
			{
				// Get the song string.
				char* pszSong = g_pLTServer->GetVarValueString( hSong );

				// Make sure we got something.
				if( pszSong && pszSong[0] )
				{
					// Initialize the musicmgr with the song.
					if( !g_pMusicMgr->Init( pszSong ))
					{
						TRACE( "CCharacterMgr::PostStartWorld:  Failed to initialize MusicMgr\n" );
					}
				}
			}
		}

		for( CharactersToCacheList::const_iterator iter = m_aCharactersToCache.begin();
		     iter != m_aCharactersToCache.end(); ++iter )
		{
			CacheCharacterFiles(*iter);
		}

		if( !m_aCharactersToCache.empty() )
			m_aCharactersToCache.clear();

	}


	// [KLS] Added 9/20/01 to support caching level specific files...
	// [KLS] Updated 9/26/01 to allow caching in multiplayer...

	CacheLevelSpecificFiles();	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SetCacheFileName()
//
//	PURPOSE:	Set the name of the file to be cached
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SetCacheFileName(char* pCacheFileName)
{
	*s_LevelCacheFileName = '\0';

	if (!pCacheFileName || !(*pCacheFileName)) return;

	if (strlen(pCacheFileName) < ARRAY_LEN(s_LevelCacheFileName))
	{
		SAFE_STRCPY(s_LevelCacheFileName, pCacheFileName);

#ifdef _WIN32
		strupr(s_LevelCacheFileName);
#endif
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CacheLevelSpecificFiles()
//
//	PURPOSE:	Cache files specific to the current level...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::CacheLevelSpecificFiles()
{
	if (*s_LevelCacheFileName == '\0') return;


	// Chop off any suffix, assuming that no folder names include dots
	// (which is really bad form, anyhow)

    char *pDot = strchr(s_LevelCacheFileName, '.');
    if (pDot)
	{
        *pDot = '\0';
	}


	// The asset file should be in with the world files...

    char fname[128];
    sprintf(fname, "%s.txt", s_LevelCacheFileName);

	
    // Have the asset mgr handle the loading of the assets...

 	CServerAssetMgr assetMgr;
    if (!assetMgr.Init(g_pLTServer, fname))
	{ 
#ifndef _FINAL
		g_pLTServer->CPrint("CServerAssetMgr::Init() falied for level: %s", fname);
#endif
	}

	// Rest so we don't try and cache again until the next level is loaded...

	*s_LevelCacheFileName = '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::CreatePlayer()
//
//	PURPOSE:	Create the player object, and associated it with the client.
//
// ----------------------------------------------------------------------- //

CPlayerObj* CGameServerShell::CreatePlayer(HCLIENT hClient, void *pClientData, uint32 clientDataLen)
{
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Rotation.Init();
	VEC_INIT(theStruct.m_Pos);
	theStruct.m_Flags = FLAG_GOTHRUWORLD;

	HCLASS hClass = g_pLTServer->GetClass("CPlayerObj");

	CPlayerObj* pPlayer = NULL;

	if(hClass)
	{
		// If we've got client data... then we want to setup some properties for the object
		if(pClientData && (clientDataLen == sizeof(MPClientData)))
		{
			MPClientData *pData = (MPClientData*)pClientData;
			char szProps[128];

			// Make sure this is a valid character type for multiplayer... that way people can't
			// cheat to new characters from the client
			m_pMPMgr->ValidateClientData(hClient, pData);
			m_pMPMgr->ValidateClassAndCharacter(hClient, pData);

			const CharacterButes *cb = &(g_pCharacterButeMgr->GetCharacterButes(pData->m_nCharacterSet[pData->m_nCharacterClass]));

			// Update the properties to what we want
			sprintf(szProps, "CharacterType %s; ClientID %d", cb->m_szName, g_pLTServer->GetClientID(hClient));

			GameStartPoint* pStartPoint = FindStartPoint(LTNULL, (int)pData->m_nCharacterClass);

			if(pStartPoint)
			{
				g_pLastPlayerStartPt = pStartPoint;
				g_pLTServer->GetObjectPos(pStartPoint->m_hObject, &(theStruct.m_Pos));

				LTVector vAngles = pStartPoint->GetPitchYawRoll();
				g_pMathLT->SetupEuler(theStruct.m_Rotation, vAngles.x, vAngles.y, vAngles.z);
			}

			// Now create the new player
			pPlayer = (CPlayerObj*) g_pLTServer->CreateObjectProps(hClass, &theStruct, szProps);
		}
		else
		{
			// This is the single player method of creation...
			GameStartPoint* pStartPoint = FindStartPoint(LTNULL);

			if(pStartPoint)
			{
				g_pLastPlayerStartPt = pStartPoint;
				g_pLTServer->GetObjectPos(pStartPoint->m_hObject, &(theStruct.m_Pos));
			}

			pPlayer = (CPlayerObj*) g_pLTServer->CreateObject(hClass, &theStruct);
		}
	}


	// Add the player to the character list
	g_pCharacterMgr->Add(pPlayer);

	return pPlayer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::FindStartPoint()
//
//	PURPOSE:	Find a good start point.
//
// ----------------------------------------------------------------------- //

GameStartPoint* CGameServerShell::FindStartPoint(HSTRING hStartPointName, int nClass, CPlayerObj* pSpawnPlayer, LTBOOL bQueen)
{
	// Grab the game type
	int nGameType = (int)g_pGameServerShell->GetGameType().Get();
	MPGameInfo *pInfo = GetGameInfo();


	// Get all the start points...
	ObjArray <HOBJECT, 100> objArray;
	int nObjects, i;
	GameStartPoint** pStartPtArray = LTNULL;

	// Find all the objects with the name "GameStartPoint"
	g_pLTServer->FindNamedObjects("GameStartPoint", objArray);
	nObjects = objArray.NumObjects();

	// If there aren't any start points at all, return NULL
	if(!nObjects)
	{
#ifndef _FINAL
		g_pLTServer->CPrint("PlayerObj: WARNING!! No GameStartPoints in this level! Player will start at (0, 0, 0)");
#endif
		return LTNULL;
	}

	// Otherwise, create an array of start point pointers
	pStartPtArray = new GameStartPoint* [nObjects];
	if(!pStartPtArray)
	{
#ifndef _FINAL
		g_pLTServer->CPrint("PlayerObj: WARNING!! Couldn't initialize an array for the GameStartPoint list. Player will start at (0, 0, 0)");
#endif
		return LTNULL;
	}

	// Now go through the start points and add them to a list of 'possible' start points
	GameStartPoint* pStartPt = LTNULL;
	int nCount = 0;

	for(i = 0; i < nObjects; i++)
	{
		pStartPt = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));

		if(pStartPt)
		{
			if(nClass == -1)
			{
				pStartPtArray[nCount++] = pStartPt;
			}
			else
			{
				// Check the team validation first
				LTBOOL bContinue = LTTRUE;
				int nAllow = pStartPt->GetAllowFlags();

				if(pInfo && !bQueen)
				{
					switch(nGameType)
					{
						case MP_HUNT:
						{
							if(	(!(nAllow & STARTPOINT_ALLOW_TEAM_1) && (nClass == pInfo->m_nHunt_HunterRace)) ||
								(!(nAllow & STARTPOINT_ALLOW_TEAM_2) && (nClass == pInfo->m_nHunt_PreyRace)))
								bContinue = LTFALSE;

							break;
						}

						case MP_SURVIVOR:
						{
							if(	(!(nAllow & STARTPOINT_ALLOW_TEAM_1) && (nClass == pInfo->m_nSurv_SurvivorRace)) ||
								(!(nAllow & STARTPOINT_ALLOW_TEAM_2) && (nClass == pInfo->m_nSurv_MutateRace)))
								bContinue = LTFALSE;

							break;
						}

						case MP_OVERRUN:
						case MP_EVAC:
						{
							if(	(!(nAllow & STARTPOINT_ALLOW_TEAM_1) && (nClass == pInfo->m_nTM_DefenderRace)) ||
								(!(nAllow & STARTPOINT_ALLOW_TEAM_2) && (nClass == pInfo->m_nTM_AttackerRace)))
								bContinue = LTFALSE;

							break;
						}
					}
				}

				// Make sure we do not consider exosuit start points.
				if(nAllow & STARTPOINT_ALLOW_EXOSUITS)
					bContinue = LTFALSE;

				// Check for race validation
				if(bContinue)
				{
					LTBOOL bAdd = LTFALSE;
					if(!bQueen)
					{
						if(	((nAllow & STARTPOINT_ALLOW_MARINES) && (nClass == Marine)) ||
							((nAllow & STARTPOINT_ALLOW_ALIENS) && (nClass == Alien)) ||
							((nAllow & STARTPOINT_ALLOW_PREDATORS) && (nClass == Predator)) ||
							((nAllow & STARTPOINT_ALLOW_CORPORATES) && (nClass == Corporate)))
						{
							bAdd = LTTRUE;
						}
					}
					else
					{
						if(nAllow & STARTPOINT_ALLOW_QUEENS)
							bAdd = LTTRUE;
					}

					// see if this start point passed the final test...
					if(bAdd)
						pStartPtArray[nCount++] = pStartPt;
				}
			}
		}
	}

	pStartPt = LTNULL;

	// If we didn't find any 'possible' start points, return the first in the list
	if((nCount <= 0) || (nCount > nObjects))
	{
#ifndef _FINAL
		g_pLTServer->CPrint("PlayerObj: WARNING!! Couldn't find a valid start point for this character or game type! Using the first available.");
#endif
		return dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(objArray.GetObject(0)));
	}


	// Now check which one we should go to based off the game type
	int nIndex = 0;

	switch(nGameType)
	{
		case SP_STORY:
		{
			// Find the start point with this name...
			if(hStartPointName)
			{
				for(int i = 0; i < nCount; i++)
				{
					if(pStartPtArray[i])
					{
						HSTRING hName = pStartPtArray[i]->GetName();
						if(hName)
						{
							if(g_pLTServer->CompareStringsUpper(hStartPointName, hName))
							{
								nIndex = i;
								break;
							}
						}
					}
				}
			}
			else
			{
				nIndex = 0;
			}

			break;
		}

		default:
		{
			// Find the start point that has the least players nearby
			LTVector vPos, vObjPos, vDir;
			CCharacterMgr::PlayerIterator iter;
			LTFLOAT fTemp;
			int nPlayers, i;

			// Array of distance values
			LTFLOAT fFarDist[128];
			LTFLOAT fNearDist[128];

			// Go through each start point and fill in the dist values
			for(i = 0; i < nCount; i++)
			{
				fFarDist[i] = 0.0f;
				fNearDist[i] = 100000.0f;

				if(pStartPtArray[i])
				{
					nPlayers = 0;
					g_pLTServer->GetObjectPos(pStartPtArray[i]->m_hObject, &vPos);

					// Go through the list of players
					for(iter = g_pCharacterMgr->BeginPlayers(); iter != g_pCharacterMgr->EndPlayers(); ++iter)
					{
						const CPlayerObj* pPlayer = *iter;

						if(pPlayer != pSpawnPlayer)
						{
							nPlayers++;

							// Get the position of this player and create a direction vector
							g_pLTServer->GetObjectPos(pPlayer->m_hObject, &vObjPos);
							vDir = vObjPos - vPos;

							fTemp = vDir.Mag();

							fFarDist[i] += fTemp;

							if(fTemp < fNearDist[i])
								fNearDist[i] = fTemp;
						}
					}
				}
			}

			// If there's less than 2 people... just pick a random point instead.
			if(nPlayers < 1)
			{
				nIndex = GetRandom(0, nCount - 1);
			}
			// Otherwise... find the point with the best position
			else
			{
				LTFLOAT fFDist = -1.0f;
				LTFLOAT fNDist = -1.0f;

				for(i = 0; i < nCount; i++)
				{
					// Only determine the point based off the nearest distance...
					if(fNearDist[i] >= fNDist)
					{
						fNDist = fNearDist[i];
						nIndex = i;
					}

					// Subtract a little for the FDist to give some flex room...
/*					if((fFarDist[i] >= (fFDist - 100.0f)) && (fNearDist[i] >= fNDist))
					{
						fFDist = fFarDist[i];
						fNDist = fNearDist[i];

						nIndex = i;
					}
*/
				}
			}

			break;
		}
	}

	// Set the returned start point to the index we chose... and delete the array we allocated
	pStartPt = pStartPtArray[nIndex];
	delete [] pStartPtArray;

	return pStartPt;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleCheatRemoveAI()
//
//	PURPOSE:	Handle the remove ai cheat
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleCheatRemoveAI(uint8 nData)
{
	HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
	HCLASS  hClass = g_pLTServer->GetClass("CAI");

	// Remove all the ai objects...

	LTBOOL bRemove = LTFALSE;

	HOBJECT hRemoveObj = LTNULL;
	while (hObj)
	{
		if (IsAI(hObj))
		{
			hRemoveObj = hObj;
		}

		hObj = g_pLTServer->GetNextObject(hObj);

		if (hRemoveObj)
		{
			LPBASECLASS pClass = g_pLTServer->HandleToObject(hRemoveObj);
			if (pClass)
			{
				DamageStruct damage;

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = hRemoveObj;

				damage.DoDamage(pClass, hRemoveObj);
			}

			hRemoveObj = LTNULL;
		}
	}


	hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	hRemoveObj = LTNULL;
	while (hObj)
	{
		if (IsAI(hObj))
		{
			hRemoveObj = hObj;
		}

		hObj = g_pLTServer->GetNextInactiveObject(hObj);

		if (hRemoveObj)
		{
			LPBASECLASS pClass = g_pLTServer->HandleToObject(hRemoveObj);
			if (pClass)
			{
				DamageStruct damage;

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = hRemoveObj;

				damage.DoDamage(pClass, hRemoveObj);
			}

			hRemoveObj = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleLoadGameMsg()
//
//	PURPOSE:	Handle loading a game
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleLoadGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	if( !m_bGlobalMgrValid )
	{
		ReportError(hSender, SERROR_LOADGAME);
		g_pLTServer->BPrint("Load Game Error: Global data could not be loaded.");
		return;
	}


	LTRESULT dResult = LT_OK;
	uint32 flags;

	// Read the message data...

	m_nLastLGFlags	= g_pLTServer->ReadFromMessageByte(hMessage);
	HSTRING	hLGName	= g_pLTServer->ReadFromMessageHString(hMessage);
	HSTRING hSGName	= g_pLTServer->ReadFromMessageHString(hMessage);
	HSTRING hROName	= g_pLTServer->ReadFromMessageHString(hMessage);

	// Save client-side info if loading a new level...
	m_nPrevMissionId = g_pLTServer->ReadFromMessageByte(hMessage);

	ObjectList* pKeepAliveList = g_pLTServer->CreateObjectList();


	char* pLGFileName = LTNULL;
	char* pSGFileName = LTNULL;
	char* pROFileName = LTNULL;

	if (hLGName) pLGFileName = g_pLTServer->GetStringData(hLGName);
	if (hSGName) pSGFileName = g_pLTServer->GetStringData(hSGName);
	if (hROName) pROFileName = g_pLTServer->GetStringData(hROName);

	LTBOOL bLoadWorldObjects	= LTFALSE;
	LTBOOL bSaveKeepAlives		= LTFALSE;
	LTBOOL bRestoreLevelObjects	= LTFALSE;

	// Set up the player's client data...  
	// (pPlayer needs to be scoped so previous goto's won't upset
	//  the compiler).
	{
	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
	if (pPlayer)
	{
		if(m_nPrevMissionId != (uint8)-1 && m_nLastLGFlags != LOAD_RESTORE_GAME)
		{
			//get the mission data
			MissionButes prevMissionButes = g_pMissionMgr->GetMissionButes(m_nPrevMissionId);

			if(prevMissionButes.m_bResetHealthAndArmor)
				pPlayer->ResetHealthAndArmor();
		}
	}
	}


	if (!hLGName) 
	{
		ReportError(hSender, SERROR_LOADGAME);
		g_pLTServer->BPrint("Load Game Error: Invalid world filename!");
		goto FREE_DATA;
	}

	if (m_nLastLGFlags == LOAD_NEW_GAME)
	{
		m_bAlarm			= LTFALSE;
		bLoadWorldObjects	= LTTRUE;
	}
	else if (m_nLastLGFlags == LOAD_NEW_LEVEL)
	{
		m_bAlarm			= LTFALSE;
		bLoadWorldObjects	= LTTRUE;
		bSaveKeepAlives		= LTTRUE;
	}
	else if (m_nLastLGFlags == LOAD_RESTORE_GAME)
	{
		// Alarm will be loaded from previous state
		bRestoreLevelObjects = LTTRUE;
	}
	else // Invalid flags
	{
		ReportError(hSender, SERROR_LOADGAME);
		g_pLTServer->BPrint("Load Game Error: Invalid flags!");
		goto FREE_DATA;
	}


	// Validate the filenames...
	
	if (!pLGFileName || pLGFileName[0] == ' ')
	{
		ReportError(hSender, SERROR_LOADGAME);
		g_pLTServer->BPrint("Load Game Error: Invalid world filename!");
		goto FREE_DATA;
	}

	if (bRestoreLevelObjects)
	{
		if (!pROFileName || pROFileName[0] == ' ')
		{
			ReportError(hSender, SERROR_LOADGAME);
			g_pLTServer->BPrint("Load Game Error: Invalid restore objects filename!");
			goto FREE_DATA;
		}
	}

	
	if (!pKeepAliveList)
	{	
		ReportError(hSender, SERROR_LOADGAME);
		g_pLTServer->BPrint("Load Game Error: Couldn't create keep alive list!");
		goto FREE_DATA;
	}


	if (bSaveKeepAlives)
	{
		// Build keep alives list...

		// Let the player add necessary keep alives to the list...

		void *pData = g_pLTServer->GetClientUserData(hSender);
		CPlayerObj* pPlayer = (CPlayerObj*)pData;
		if (pPlayer)
		{
			pPlayer->BuildKeepAlives(pKeepAliveList);
		}


		dResult = g_pLTServer->SaveObjects(KEEPALIVE_FILENAME, pKeepAliveList, m_nLastLGFlags, 0);
	
		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
			g_pLTServer->BPrint("Load Game Error: Couldn't save keepalives");
			goto FREE_DATA;
		}
	}


    // [KLS] Save the level name for CacheLevelSpecificFiles()...The cache
	// file is the same as the world name (CacheLevelSpecificFiles() will convert
	// the world name to the correct extention)...

	SetCacheFileName(pLGFileName);


	// Load the new level...

	flags = bLoadWorldObjects ? LOADWORLD_LOADWORLDOBJECTS : 0;
	flags |= LOADWORLD_NORELOADGEOMETRY;
	dResult = g_pLTServer->LoadWorld(pLGFileName, flags);
	
	if (dResult != LT_OK)
	{
		ReportError(hSender, SERROR_LOADGAME);
		g_pLTServer->BPrint("Load Game Error: Couldn't Load world '%s'", pLGFileName);
		goto FREE_DATA;
	}


	if (bRestoreLevelObjects)
	{
		dResult = g_pLTServer->RestoreObjects(pROFileName, m_nLastLGFlags, RESTOREOBJECTS_RESTORETIME);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
			g_pLTServer->BPrint("Load Game Error: Couldn't restore objects '%s'", pROFileName);
			goto FREE_DATA;
		}
	}


	// Load the keep alives...

	if (bSaveKeepAlives && pKeepAliveList && pKeepAliveList->m_nInList > 0)
	{
		m_bLoadingKeepAlive = LTTRUE;

		dResult = g_pLTServer->RestoreObjects(KEEPALIVE_FILENAME, m_nLastLGFlags, 0);

		m_bLoadingKeepAlive = LTFALSE;

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_LOADGAME);
			g_pLTServer->BPrint("Load Game Error: Couldn't restore keepalives");
			goto FREE_DATA;
		}
	}



	// Start the world...

	dResult = g_pLTServer->RunWorld();

	if (dResult != LT_OK)
	{
		ReportError(hSender, SERROR_LOADGAME);
		g_pLTServer->BPrint("Load Game Error: Couldn't run world!");
	}


// Waste not, want not...

FREE_DATA:

	if (pKeepAliveList)
	{
		g_pLTServer->RelinquishList(pKeepAliveList);
	}

	if (hLGName) g_pLTServer->FreeString(hLGName);
	if (hSGName) g_pLTServer->FreeString(hSGName);
	if (hROName) g_pLTServer->FreeString(hROName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleSaveGameMsg()
//
//	PURPOSE:	Handle saving a game
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleSaveGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	char* pSGFileName = LTNULL;
	HOBJECT	hObj = LTNULL;
	ObjectList* pSaveList = g_pLTServer->CreateObjectList();

	// Read the message data...

	uint8	nSGFlags = g_pLTServer->ReadFromMessageByte(hMessage);
	HSTRING	hSGName	 = g_pLTServer->ReadFromMessageHString(hMessage);

	HMESSAGEREAD hClientData = g_pLTServer->ReadFromMessageHMessageRead(hMessage);


	// Set up the player's client data...

	void *pData = g_pLTServer->GetClientUserData(hSender);
	CPlayerObj* pPlayer = (CPlayerObj*)pData;
	if (pPlayer)
	{
		pPlayer->SetClientSaveData(hClientData);
	}

	if (!hSGName) 
	{
		ReportError(hSender, SERROR_SAVEGAME);
		g_pLTServer->BPrint("Save Game Error: Invalid filename!");
		goto FREE_DATA;
	}

	pSGFileName = g_pLTServer->GetStringData(hSGName);
	if (!pSGFileName || pSGFileName[0] == ' ')
	{
		ReportError(hSender, SERROR_SAVEGAME);
		g_pLTServer->BPrint("Save Game Error: Invalid filename!");
		goto FREE_DATA;
	}


	// Save all the objects...

	if (!pSaveList) 
	{
		ReportError(hSender, SERROR_SAVEGAME);
		g_pLTServer->BPrint("Save Game Error: Allocation error!");
		goto FREE_DATA;
	}

	
	// Save depends on the global WorldProperties object being valid (i.e.,
	// every level MUST have a WorldProperties object).

	if (!g_pWorldProperties)
	{
		ReportError(hSender, SERROR_SAVEGAME);
		g_pLTServer->BPrint("Save Game Error: No WorldProperties object!  Can not save game!");
		goto FREE_DATA;
	}

	
	// Add active objects to the list...

	hObj = g_pLTServer->GetNextObject(LTNULL);
	while (hObj)
	{
		if (hObj != g_pWorldProperties->m_hObject)
		{
			g_pLTServer->AddObjectToList(pSaveList, hObj);
		}

		hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Add inactive objects to the list...

	hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
		if (hObj != g_pWorldProperties->m_hObject)
		{
			g_pLTServer->AddObjectToList(pSaveList, hObj);
		}

		hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}


	// Make sure the WorldProperties object is saved FIRST, this way all the global
	// data will be available for the other objects when they get restored.
	// (ILTServer::AddObjectsToList() adds to the front of the list, so we
	// need to add it last ;)...

	g_pLTServer->AddObjectToList(pSaveList, g_pWorldProperties->m_hObject);



	if (pSaveList && pSaveList->m_nInList > 0)
	{
		LTRESULT dResult = g_pLTServer->SaveObjects(pSGFileName, pSaveList, LOAD_RESTORE_GAME, 
												 SAVEOBJECTS_SAVEGAMECONSOLE | SAVEOBJECTS_SAVEPORTALS);

		if (dResult != LT_OK)
		{
			ReportError(hSender, SERROR_SAVEGAME);
			g_pLTServer->BPrint("Save Game Error: Couldn't save objects!");
		}
	}


// Waste not, want not...

FREE_DATA:

	if (hClientData)
	{
		g_pLTServer->EndHMessageRead(hClientData);
	}

	if (hSGName) g_pLTServer->FreeString(hSGName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ReportError()
//
//	PURPOSE:	Tell the client about a server error
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ReportError(HCLIENT hSender, uint8 nErrorType)
{	
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hSender, MID_SERVER_ERROR);
	g_pLTServer->WriteToMessageByte(hWrite, nErrorType);
	g_pLTServer->EndMessage(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Update
//
//	PURPOSE:	Update servier stuff periodically
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Update(LTFLOAT timeElapsed)
{
	// Update the command mgr...

	m_CmdMgr.Update();


	// If it's the first update...
	if (m_bFirstUpdate)
	{
		// Setup the multiplayer manager at this point
		m_pMPMgr->Setup();
		m_bFirstUpdate = LTFALSE;
	}


	// Update the music.
	g_pMusicMgr->Update();


	// See if we should show our bounding box...
	// DEVELOPMENT CODE...
#ifndef _FINAL
	static bool s_bShowingBoundingBox = false;
	if (g_ShowDimsTrack.GetFloat() > 0.0f)
	{
		UpdateBoundingBoxes();
		s_bShowingBoundingBox = true;
	}
	else if( s_bShowingBoundingBox )
	{
		s_bShowingBoundingBox = false;
		RemoveBoundingBoxes();
	}

	// Used for showing AIVolume-s.
	// DEVELOPMENT CODE...
	if( g_ShowAIVolTrack.GetFloat() > 0.0f)
	{
		g_pAIVolumeMgr->ShowVolumes();
	}
	else
	{
		g_pAIVolumeMgr->HideVolumes();
	}
#endif

	// Update multiplayer stuff... this will do nothing if it's not a mutliplayer game
	// so you don't have to check it here

	m_pMPMgr->Update();

	// check for autosave...
	if(GetGameType() == SP_STORY)
	{
		HOBJECT hPlayerObj = LTNULL;
		ObjArray <HOBJECT, 1> objArray;

		g_pLTServer->FindNamedObjects("Player", objArray);
		if(!objArray.NumObjects()) return;
		
		hPlayerObj = objArray.GetObject(0);
		if(!hPlayerObj) return;

		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hPlayerObj));
		if(    !pPlayer 
			||  pPlayer->IsDead() 
			||  pPlayer->PlayingCinematic() 
			||  pPlayer->IsSwapped() ) return;

		if(m_bDoAutoSave)
		{
			pPlayer->DoAutoSave();
			m_bDoAutoSave = LTFALSE;
		}
		else if(pPlayer->ShouldDoAutoSave())
		{
			// Set us up so we save next update..
			m_bDoAutoSave = LTTRUE;

			// Reset the player..
			pPlayer->ResetDoAutoSave();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::UpdateBoundingBoxes()
//
//	PURPOSE:	Update object bounding boxes
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL

void CGameServerShell::UpdateBoundingBoxes()
{
	HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
	HCLASS  hClass = g_pLTServer->GetClass("GameBase");

	// Active objects...

	while (hObj)
	{
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->UpdateBoundingBox();
			}
		}

		hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Inactive objects...

	hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->UpdateBoundingBox();
			}
		}

		hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}
}

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::RemoveBoundingBoxes()
//
//	PURPOSE:	Remove object bounding boxes
//
// ----------------------------------------------------------------------- //

#ifndef _FINAL

void CGameServerShell::RemoveBoundingBoxes()
{
	HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
	HCLASS  hClass = g_pLTServer->GetClass("GameBase");

	// Active objects...

	while (hObj)
	{
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->RemoveBoundingBox();
			}
		}

		hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Inactive objects...

	hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			GameBase* pObj = (GameBase*)g_pLTServer->HandleToObject(hObj);
			if (pObj)
			{
				pObj->RemoveBoundingBox();
			}
		}

		hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}
}

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ServerAppMessageFn
//
//	PURPOSE:	Server app message function
//
// ----------------------------------------------------------------------- //

LTRESULT CGameServerShell::ServerAppMessageFn(char *pMsg, int nLen)
{
	return m_pMPMgr->ServerAppMessageFn( pMsg, nLen);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PauseGame
//
//	PURPOSE:	Pause/unpause the game
//
// ----------------------------------------------------------------------- //

void CGameServerShell::PauseGame(LTBOOL bPause)
{
	uint32 nFlags = g_pLTServer->GetServerFlags();

	if (bPause)
	{
		nFlags |= SS_PAUSED;
	}
	else
	{
		nFlags &= ~SS_PAUSED;
	}
	
	g_pLTServer->SetServerFlags (nFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleConsoleCmdMsg()
//
//	PURPOSE:	Handle console cmd messages
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleConsoleCmdMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	HSTRING	hMsg = g_pLTServer->ReadFromMessageHString(hMessage);
	if (!hMsg) return;

	if (m_CmdMgr.Process(g_pLTServer->GetStringData(hMsg)))
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Sent Command '%s'", g_pLTServer->GetStringData(hMsg));
#endif
	}

	if (hMsg)
	{
		g_pLTServer->FreeString(hMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleConsoleTriggerMsg()
//
//	PURPOSE:	Handle console trigger messages
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleConsoleTriggerMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
	if (!pPlayer) return;

		// Read the message data...

	HSTRING	hstrObjName	= g_pLTServer->ReadFromMessageHString(hMessage);
	HSTRING hstrMsg		= g_pLTServer->ReadFromMessageHString(hMessage);

	char* pName = LTNULL;
	char* pMsg  = LTNULL;

	if (hstrObjName) 
	{
		pName = g_pLTServer->GetStringData(hstrObjName);
	}

	if (hstrMsg) 
	{
		pMsg = g_pLTServer->GetStringData(hstrMsg);
	}

	// Special case if we're supposed to list objects of a certain type...

	CommonLT* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	ConParse parse(pMsg);
	if(    pCommon->Parse(&parse) == LT_OK 
		&& parse.m_nArgs > 0 
		&& _stricmp(parse.m_Args[0],"LIST") == 0 )
	{
		LTBOOL bNoObjects = LTTRUE;

		if (parse.m_nArgs > 1)
		{
			const uint32 nClassNameSize = 64;
			char szClassName[nClassNameSize + 1];
			szClassName[nClassNameSize] = 0;

			const uint32 nScratchSize = 3;
			char szScratch[nScratchSize + 1];
			szScratch[nScratchSize] = 0;
			

			g_pLTServer->CPrint("Listing objects of type '%s'", parse.m_Args[1]);
			
			HCLASS  hClass = g_pLTServer->GetClass(parse.m_Args[1]);

			// Get the names of all the objects of the specified class...

			HOBJECT hObj = g_pLTServer->GetNextObject(LTNULL);
			while (hObj)
			{
				if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
				{
					const HCLASS hClass = g_pLTServer->GetObjectClass(hObj);
					g_pLTServer->GetClassName(hClass, szClassName, nClassNameSize);

					if( !szClassName[0] )
					{
						g_pLTServer->GetModelFilenames(hObj, szClassName, nClassNameSize, szScratch, nScratchSize);
					}

					g_pLTServer->CPrint("%s [%s] (active)", g_pLTServer->GetObjectName(hObj), szClassName );
					bNoObjects = LTFALSE;
				}

				hObj = g_pLTServer->GetNextObject(hObj);
			}

			hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
			while (hObj)
			{
				if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
				{
					const HCLASS hClass = g_pLTServer->GetObjectClass(hObj);
					g_pLTServer->GetClassName(hClass, szClassName, nClassNameSize);

					if( !szClassName[0] )
					{
						g_pLTServer->GetModelFilenames(hObj, szClassName, nClassNameSize, szScratch, nScratchSize);
					}

					g_pLTServer->CPrint("%s [%s] (inactive)", g_pLTServer->GetObjectName(hObj), szClassName);
					bNoObjects = LTFALSE;
				}

				hObj = g_pLTServer->GetNextInactiveObject(hObj);
			}

			if (bNoObjects)
			{
				g_pLTServer->CPrint("No objects of type '%s' exist (NOTE: object type IS case-sensitive)", parse.m_Args[1]);
			}
		}
	}
	// Send the message to all appropriate objects...
	else if (SendTriggerMsgToObjects(pPlayer, hstrObjName, hstrMsg))
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Sent '%s' Msg '%s'", pName ? pName : "Invalid Object!", pMsg ? pMsg : "Empty Message!!!");
#endif
	}
	else
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Failed to Send '%s' Msg '%s'!", pName ? pName : "Invalid Object!", pMsg ? pMsg : "Empty Message!!!");
#endif
	}

	if (hstrObjName) 
	{
		g_pLTServer->FreeString(hstrObjName);
	}

	if (hstrMsg) 
	{
		g_pLTServer->FreeString(hstrMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandlePlayerExitLevel()
//
//	PURPOSE:	Handle exit level command
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandlePlayerExitLevel(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	if (!hSender) return;

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->GetClientUserData(hSender);
	if (!pPlayer) return;

	HOBJECT hObj   = g_pLTServer->GetNextObject(LTNULL);
	HCLASS  hClass = g_pLTServer->GetClass("ExitTrigger");

	HOBJECT hRemoveObj = LTNULL;
	while (hObj)
	{
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			SendTriggerMsgToObject(pPlayer, hObj, "TRIGGER");
			return;
		}

		hObj = g_pLTServer->GetNextObject(hObj);
	}


	hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
		if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			SendTriggerMsgToObject(pPlayer, hObj, "TRIGGER");
			return;			
		}

		hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}


	// Okay, there was no exit trigger in the level...Force an exit...

	ExitLevel();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::HandleSFXMsg()
//
//	PURPOSE:	Handle any chatter from special effect objects.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::HandleSFXMsg(HCLIENT hSender, HMESSAGEREAD hMessage)
{
	uint8 sfx_id = hMessage->ReadByte();

	switch(sfx_id)
	{
		case SFX_DEBUGLINE_ID:
		{
			HOBJECT hObj = hMessage->ReadObject();
			LTBOOL  bUpdate = hMessage->ReadByte();

#ifndef _FINAL
			if( hObj )
			{
				DebugLineSystem * pSystem = dynamic_cast<DebugLineSystem*>( g_pServerDE->HandleToObject(hObj) );
				ASSERT( pSystem );
				if( pSystem )
				{
					pSystem->UpdateClient(hSender,bUpdate);
				}
			}
#endif
		}
		break;

		case SFX_CHARACTER_ID:
		{
			HOBJECT hObj = hMessage->ReadObject();
			if( hObj )
			{
				CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hObj) );
				if( pChar )
				{
					pChar->HandleCharacterSFXMessage(hSender,hMessage);
				}
			}
		}
		break;
	}
}	
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ExitLevel()
//
//	PURPOSE:	Exit the current level
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ExitLevel()
{
	// If the game is single player we want to tell the client to exit the world...
	// If it is multiplayer, we'll just ignore the request...


	// If single player, update the player summary...
	if(GetGameType() == SP_STORY)
	{
		HOBJECT hPlayerObj = LTNULL;
		ObjArray <HOBJECT, 1> objArray;

		g_pLTServer->FindNamedObjects("Player", objArray);
		if(!objArray.NumObjects()) return;
		
		hPlayerObj = objArray.GetObject(0);
		if(!hPlayerObj) return;

		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject(hPlayerObj));
		if(!pPlayer || pPlayer->IsDead()) return;


		HMESSAGEWRITE hMsg = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_EXITLEVEL);
		g_pLTServer->EndMessage(hMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ProcessMPStartPointMessages()
//
//	PURPOSE:	Process any MP startpoint messages.
//
// ----------------------------------------------------------------------- //

void CGameServerShell::ProcessMPStartPointMessages()
{
	// Grab the game type
	// Get all the start points...
	ObjArray <HOBJECT, 100> objArray;
	int nObjects, i;
	GameStartPoint** pStartPtArray = LTNULL;

	// Find all the objects with the name "GameStartPoint"
	g_pLTServer->FindNamedObjects("GameStartPoint", objArray);
	nObjects = objArray.NumObjects();

	// If there aren't any start points at all, return NULL
	if(!nObjects)
	{
		return;
	}

	// Now go through the start points and process their messages
	GameStartPoint* pStartPt = LTNULL;
	int nCount = 0;

	for(i = 0; i < nObjects; i++)
	{
		pStartPt = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));

		if(pStartPt)
		{
			// send the MP trigger message
			const char* pCmd = g_pLTServer->GetStringData(pStartPt->GetMPCommand());

			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::SpawnMPExosuits()
//
//	PURPOSE:	Spawn exosuits as needed...
//
// ----------------------------------------------------------------------- //

void CGameServerShell::SpawnMPExosuits()
{
	MPGameInfo* pInfo = GetGameInfo();

	if(!pInfo)
		return;

	if(pInfo->m_nExosuit == 0)
		return;

	// Grab the game type
	// Get all the start points...
	ObjArray <HOBJECT, 100> objArray;
	int nObjects, i;
	GameStartPoint** pStartPtArray = LTNULL;

	// Find all the objects with the name "GameStartPoint"
	g_pLTServer->FindNamedObjects("GameStartPoint", objArray);
	nObjects = objArray.NumObjects();

	// If there aren't any start points at all, return NULL
	if(!nObjects)
	{
		return;
	}

	// Otherwise, create an array of start point pointers
	pStartPtArray = new GameStartPoint* [nObjects];
	if(!pStartPtArray)
	{
#ifndef _FINAL
		g_pLTServer->CPrint("PlayerObj: WARNING!! Couldn't initialize an array for the GameStartPoint list. Player will start at (0, 0, 0)");
#endif
		return;
	}

	// Now go through the start points and 
	// see if they support exosuits.
	GameStartPoint* pStartPt = LTNULL;
	int nCount = 0;

	for(i = 0; i < nObjects; i++)
	{
		pStartPt = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));

		if(pStartPt)
		{
			int nAllow = pStartPt->GetAllowFlags();

			if(nAllow & STARTPOINT_ALLOW_EXOSUITS)
				pStartPtArray[nCount++] = pStartPt;
		}
	}

	if(nCount > 0)
	{
		int nSuitsToSpawn = nCount>pInfo->m_nExosuit?pInfo->m_nExosuit:nCount;

		while(nSuitsToSpawn)
		{
			int index = GetRandom(0, nCount-1);

			if(pStartPtArray[index])
			{
				pStartPtArray[index]->SpawnExosuit();
				pStartPtArray[index] = LTNULL;
				nSuitsToSpawn--;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::ValidateQueenStartPoints()
//
//	PURPOSE:	Process any MP startpoint messages.
//
// ----------------------------------------------------------------------- //

LTBOOL CGameServerShell::ValidateQueenStartPoints()
{
	// Grab the game type
	// Get all the start points...
	ObjArray <HOBJECT, 100> objArray;
	int nObjects, i;
	GameStartPoint** pStartPtArray = LTNULL;

	// Find all the objects with the name "GameStartPoint"
	g_pLTServer->FindNamedObjects("GameStartPoint", objArray);
	nObjects = objArray.NumObjects();

	// If there aren't any start points at all, return
	if(!nObjects)
	{
		return LTFALSE;
	}

	// Now go through the start points and see if there are any that support a queen
	GameStartPoint* pStartPt = LTNULL;
	int nCount = 0;

	for(i = 0; i < nObjects; i++)
	{
		pStartPt = dynamic_cast<GameStartPoint*>(g_pLTServer->HandleToObject(objArray.GetObject(i)));

		if(pStartPt)
		{
			if(pStartPt->GetAllowFlags() & STARTPOINT_ALLOW_QUEENS)
				return LTTRUE;
		}
	}

	// hmmm.. must not be any Queen start points on this level...
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Save
//
//	PURPOSE:	Save the global world info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	hWrite->WriteDWord(m_nCurrentSaveLoadVersion);

	m_charMgr.Save(hWrite);
	g_pMusicMgr->Save( *(( ILTMessage* )hWrite ));
	m_CmdMgr.Save(hWrite);
	g_pLTServer->WriteToMessageByte(hWrite, m_bAlarm);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::Load
//
//	PURPOSE:	Load the global world info
//
// ----------------------------------------------------------------------- //

void CGameServerShell::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	_ASSERT(hRead);
	if (!hRead) return;

	m_nLastSaveLoadVersion = hRead->ReadDWord();

	m_charMgr.Load(hRead);
	g_pMusicMgr->Load( *(( ILTMessage* )hRead ));
	m_CmdMgr.Load(hRead);
	m_bAlarm = g_pLTServer->ReadFromMessageByte(hRead);
}

