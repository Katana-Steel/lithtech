// ----------------------------------------------------------------------- //
//
// MODULE  : PickupObject.cpp
//
// PURPOSE : SpriteControl implementation
//
// CREATED : 8/2/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PickupObject.h"
#include "cpp_server_de.h"
#include "SFXMsgIds.h"
#include "ObjectMsgs.h"
#include "ServerUtilities.h"
#include "ServerSoundMgr.h"
#include "Destructible.h"
#include "SharedFXStructs.h"
#include "FXButeMgr.h"
#include "GameServerShell.h"
#include "MultiplayerMgrDefs.h"
#include "float.h"
#include "PlayerObj.h"
#include "GameStartPoint.h"

// ----------------------------------------------------------------------- //

#define PICKUPOBJECT_EXPIRE_TIME		30.0f
//#define PICKUPOBJECT_DEBUG

// ----------------------------------------------------------------------- //

BEGIN_CLASS(PickupObject)
	ADD_STRINGPROP_FLAG_HELP(Pickup, "", PF_STATICLIST, "The type of pickup object from PickupButes.txt!")
	ADD_BOOLPROP_FLAG_HELP(ForceNoRespawn, LTFALSE, 0, "This forces the pickup to not respawn at all.")
	ADD_BOOLPROP_FLAG_HELP(ForceNoMoveToGround, LTFALSE, 0, "This forces the pickup to not move to the ground on creation.")
	ADD_STRINGPROP_FLAG_HELP(PickupMessage, "", 0, "This is a message that gets sent when the object is picked up. (ie. 'msg object0 trigger')")
END_CLASS_DEFAULT_FLAGS_PLUGIN(PickupObject, GameBase, LTNULL, LTNULL, 0, CPickupObjectPlugin)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::PickupObject()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PickupObject::PickupObject() : GameBase(OT_MODEL)
{
	m_fCreateTime			= 0.0f;
	m_fCurrentHitPoints		= 1.0f;
	m_fRespawnTime			= 0.0f;
	m_fPickupDelay			= 0.0f;
	m_bInfiniteRespawns		= LTFALSE;
	m_bSliding				= LTFALSE;
	m_bForceNoRespawn		= LTFALSE;
	m_bNeedSebtle			= LTTRUE;
	m_bForceNoMoveToGround	= LTFALSE;
	m_nUsedAmount[0]		= 0;
	m_nUsedAmount[1]		= 0;
	m_nUsedAmount[2]		= 0;
	m_nUsedAmount[3]		= 0;
	m_bAllowMovement		= LTTRUE;
	m_pUserData				= LTNULL;
	m_hstrPickupMsg			= LTNULL;
	m_hAnim					= 0;
	m_hSender				= LTNULL;
	m_bAllowRotation		= LTTRUE;
	m_hStartpointSpawner	= LTNULL;
	m_fLastTouchTime		= 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::~PickupObject()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

PickupObject::~PickupObject()
{
	if(m_pUserData)
	{
		delete m_pUserData;
		m_pUserData = LTNULL;
	}

	if(m_hstrPickupMsg)
		g_pLTServer->FreeString(m_hstrPickupMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::EngineMessageFn
//
//	PURPOSE:	Handle engine EngineMessageFn messages
//
// ----------------------------------------------------------------------- //

uint32 PickupObject::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData)
{
	uint32 rVal = GameBase::EngineMessageFn(messageID, pData, lData);

	switch (messageID)
	{
		case MID_PRECREATE:
		{
			if( lData != INITIALUPDATE_SAVEGAME )
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

				if(pStruct)
					ReadProp(pStruct);
			}

			return rVal;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (lData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			if(m_puButes.m_bCanTouch)
			{
				TouchNotify((HOBJECT)pData);
			}

			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)lData);
			break;
		}

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)lData);
			break;
		}

		default: break;
	}

	return rVal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 PickupObject::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			// Get the trigger message
			HSTRING hStr = g_pLTServer->ReadFromMessageHString(hRead);

			if(m_puButes.m_bCanActivate)
			{
				// See if it's an Activate message from a character
				if(!_stricmp("Activate", g_pLTServer->GetStringData(hStr)))
					TouchNotify(hSender);
			}

			if(TriggerMsg(hSender, g_pLTServer->GetStringData(hStr)))
			{
				g_pLTServer->FreeString(hStr);
				return LT_OK;
			}

			// Make sure other people can read it...
			g_pLTServer->FreeString(hStr);
            g_pLTServer->ResetRead(hRead);

			break;
		}

		case MID_DAMAGE:
		{
			// Only be damaged if the butes allow it and this is not a single player game.
			if(m_puButes.m_bCanDamage && !g_pGameServerShell->GetGameType().IsSinglePlayer())
			{
				Damage(hSender, hRead);
			}

			break;
		}

		case MID_PICKEDUP:
		{
			uint32 nAmt[4];
			LTBOOL bPartialPU;
			nAmt[0] = g_pLTServer->ReadFromMessageDWord(hRead);
			nAmt[1] = g_pLTServer->ReadFromMessageDWord(hRead);
			nAmt[2] = g_pLTServer->ReadFromMessageDWord(hRead);
			nAmt[3] = g_pLTServer->ReadFromMessageDWord(hRead);
			bPartialPU = g_pLTServer->ReadFromMessageByte(hRead);

			PickedUp(hSender, nAmt, bPartialPU);

			break;
		}
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::TriggerMsg()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

LTBOOL PickupObject::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (!szMsg) return LTFALSE;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return LTFALSE;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "HIDDEN") == 0)
			{
				if (parse.m_nArgs == 2 && parse.m_Args[1])
				{
					uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

					if ((_stricmp(parse.m_Args[1], "1") == 0) ||
						(_stricmp(parse.m_Args[1], "TRUE") == 0))
					{
						dwFlags &= ~FLAG_VISIBLE;
						dwFlags &= ~FLAG_SOLID;
						dwFlags &= ~FLAG_RAYHIT;
					}
					else if ((_stricmp(parse.m_Args[1], "0") == 0) ||
							(_stricmp(parse.m_Args[1], "FALSE") == 0))
					{
						dwFlags |= FLAG_VISIBLE | FLAG_RAYHIT;

						if(m_puButes.m_bSolid)
							dwFlags |= FLAG_SOLID;
					}

					g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
				}
				return LTTRUE;
			}
		}
	}
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::ReadProp
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL PickupObject::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;

	// ----------------------------------------------------------------------- //
	// Fill in the sprite filename to create
	if(g_pLTServer->GetPropGeneric("Pickup", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
		{
			// Get the pickup data out of the attribute manager
			m_puButes = g_pPickupButeMgr->GetPickupButes(genProp.m_String);
		}
	}

	if(g_pLTServer->GetPropGeneric("ForceNoRespawn", &genProp) == LT_OK)
	{
		m_bForceNoRespawn = genProp.m_Bool;
	}

	if(g_pLTServer->GetPropGeneric("ForceNoMoveToGround", &genProp) == LT_OK)
	{
		m_bForceNoMoveToGround = genProp.m_Bool;
	}

	// Fill in the object create structure
	if(!pData->m_Name[0])
		SAFE_STRCPY(pData->m_Name, "PickupObject");

	SAFE_STRCPY(pData->m_Filename, m_puButes.m_szModel);

	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		SAFE_STRCPY(pData->m_SkinNames[i], m_puButes.m_szSkins[i]);
	}

	pData->m_ObjectType = OT_MODEL;
	pData->m_Flags = FLAG_VISIBLE | FLAG_RAYHIT | FLAG_TOUCH_NOTIFY | FLAG_REMOVEIFOUTSIDE | FLAG_GRAVITY | FLAG_NOSLIDING;

	// Reset our pickup delay
	m_fPickupDelay = g_pLTServer->GetTime() + m_puButes.m_fPickupDelay;


	// Fill in the message to send when the object gets picked up
	if(g_pLTServer->GetPropGeneric("PickupMessage", &genProp) == LT_OK)
		if (genProp.m_String[0])
			m_hstrPickupMsg = g_pLTServer->CreateString(genProp.m_String);


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::InitialUpdate
//
//	PURPOSE:	Do everything we need to do first
//
// ----------------------------------------------------------------------- //

LTBOOL PickupObject::InitialUpdate()
{
	// Set the next update to almost immediately
	g_pLTServer->SetNextUpdate(m_hObject, 0.001f);

	// Setup the model scale
	LTVector vScale(m_puButes.m_fScale, m_puButes.m_fScale, m_puButes.m_fScale);
	g_pLTServer->ScaleObject(m_hObject, &vScale);

	// Set the animation and dims of the object
	m_hAnim = g_pLTServer->GetAnimIndex(m_hObject, "PowerUp");

	if(m_hAnim == INVALID_MODEL_ANIM) m_hAnim = 0;

	g_pLTServer->SetModelAnimation(m_hObject, m_hAnim);

	LTVector vDims;
	g_pLTServer->Common()->GetModelAnimUserDims(m_hObject, &vDims, m_hAnim);

	vDims *= m_puButes.m_fScale;

	g_pLTServer->Physics()->SetObjectDims(m_hObject, &vDims, 0);


	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

	if(m_puButes.m_bSolid)
	{
		// Make object solid
		dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

		// Only set this if we are visible
		if(dwFlags & FLAG_VISIBLE)
			g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_SOLID);
	}

	if(m_puButes.m_bCanActivate)
	{
		//set the proper activate type
		switch(m_puButes.m_nType)
		{	
		case (0):	SetActivateType(m_hObject, AT_MARINE_PICKUP);	break;
		case (1):	SetActivateType(m_hObject, AT_PRED_PICKUP);		break;
		case (2):	SetActivateType(m_hObject, AT_EXO_PICKUP);		break;
		default:	break;
		}
	}


	// Send the inital SFX message
	uint8 nType = (uint8)(g_pGameServerShell->GetGameType().IsMultiplayer() ? m_puButes.m_nMPFXType : m_puButes.m_nFXType);

	HMESSAGEWRITE hWrite = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hWrite, SFX_PICKUPOBJECT_ID);
	g_pLTServer->WriteToMessageByte(hWrite, nType);
	g_pLTServer->EndMessage(hWrite);


	// See if we need to set the glow flag on this object
	PickupFXButes puFX = g_pPickupButeMgr->GetPickupFXButes(nType);

	if(puFX.m_bGlow)
	{
		uint32 nUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		g_pLTServer->SetObjectUserFlags(m_hObject, nUserFlags | USRFLG_GLOW);
	}


	// Initialize the current hit points
	m_fCurrentHitPoints = m_puButes.m_fHitPoints;
	m_bInfiniteRespawns = (m_puButes.m_nNumRespawns == -1);


	// Set the time of creation
	m_fCreateTime = g_pLTServer->GetTime();


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Update
//
//	PURPOSE:	Do stuff
//
// ----------------------------------------------------------------------- //

LTBOOL PickupObject::Update()
{
	// Assume we won't need another update.
	BOOL bDoNextUpdate = FALSE;

	// This will be set with the largest time we can wait before needing
	// another update.
	float fUpdateTime = FLT_MAX;

	// Check if we should move to the ground.
	if( !m_bForceNoMoveToGround && m_puButes.m_bMoveToGround )
	{
		// Default to not being on the ground.
		LTBOOL bHitGround = LTFALSE;

		// Check if we're on something.
		CollisionInfo Info;
		g_pLTServer->GetStandingOn(m_hObject, &Info);
		if( Info.m_hObject )
		{
			// Move the object and check if we hit the ground.
			bHitGround = MoveAndAlignToFloor( );
		}

		// Check if we haven't hit the ground or slowed down enough.
		if( !bHitGround )
		{
			// We need to keep updating until we find the floor and slow down.
			fUpdateTime = Min( fUpdateTime, 0.001f );
			bDoNextUpdate = TRUE;
		}
	}


	// If we're forcing no respawn... that most likely means we were dropped
	// from a character.  In that case, make sure this pickup object goes away
	// after a certain amount of time... but we'll make this only happen in MP.
	if(g_pGameServerShell->GetGameType().IsMultiplayer() && m_bForceNoRespawn)
	{
		// Get the time left until we expire.
		float fTimeToExpire = m_fCreateTime + PICKUPOBJECT_EXPIRE_TIME - g_pLTServer->GetTime();

		// Check if we're still waiting to expire.
		if( fTimeToExpire > 0.0f )
		{
			// We'll need another update when we expire.
			bDoNextUpdate = TRUE;

			// Find the largest amount of time before our next update.
			fUpdateTime = Min( fUpdateTime, fTimeToExpire );
		}
		// We've expired.
		else
		{
			// Create a destroy effect
			char *szDestroyFX = m_puButes.m_szMPDestroyFX;

			if(szDestroyFX[0])
			{
				IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX(szDestroyFX);

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
			}

			g_pLTServer->RemoveObject(m_hObject);
			return LTFALSE;
		}
	}


	// See if it's time to respawn
	if(m_fRespawnTime > 0.0f)
	{
		// Force an update every half second...
		fUpdateTime = Min( fUpdateTime, 0.5f );
		bDoNextUpdate = TRUE;

		if(g_pLTServer->GetTime() >= m_fRespawnTime)
		{
			// Reset the respawn time
			m_fRespawnTime = 0.0f;

			// If we don't want to respawn... remove this pickup
			if(!m_puButes.m_nNumRespawns && !m_bInfiniteRespawns)
			{
				g_pLTServer->RemoveObject(m_hObject);
				return LTFALSE;
			}

			// Make the object visible when it gets an update
			uint32 dwFlags;
			g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
			dwFlags |= FLAG_VISIBLE | FLAG_RAYHIT;
			g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);

			// Reset our pickup delay
			m_fPickupDelay = g_pLTServer->GetTime() + m_puButes.m_fPickupDelay;

			// Reset the current hit points
			m_fCurrentHitPoints = m_puButes.m_fHitPoints;

			// Decrement the number of respawns
			m_puButes.m_nNumRespawns--;

			// Reset the used counter
			m_nUsedAmount[0] = 0;
			m_nUsedAmount[1] = 0;
			m_nUsedAmount[2] = 0;
			m_nUsedAmount[3] = 0;

			// Get the location of the object
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			// Play the respawn sound
			g_pServerSoundMgr->PlaySoundFromPos(vPos, m_puButes.m_szRespawnSound, 600.0f, SOUNDPRIORITY_MISC_HIGH);

			// Create a respawn effect
			char *szRespawnFX = g_pGameServerShell->GetGameType().IsMultiplayer() ? m_puButes.m_szMPRespawnFX : m_puButes.m_szRespawnFX;

			if(szRespawnFX[0])
			{
				IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX(szRespawnFX);

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
			}


#ifdef PICKUPOBJECT_DEBUG
			// Debug information!
			g_pLTServer->CPrint("PickupObject: %s has respawned at (%.2f %.2f %.2f)!",
				m_puButes.m_szName, vPos.x, vPos.y, vPos.z);
#endif
		}
	}

	// Check if we need to do another update.
	if( bDoNextUpdate )
	{
		// Set the next update to almost immediately
		SetNextUpdate( fUpdateTime, 1.0f );
	}
	else
	{
		// Set our deactivation time.
		SetNextUpdate( 0.0f, 0.0f );
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::TouchNotify
//
//	PURPOSE:	Do stuff
//
// ----------------------------------------------------------------------- //

LTBOOL PickupObject::TouchNotify(HOBJECT hSender)
{
	// see if we already dealt with this guy...
	if(hSender == m_hSender && g_pLTServer->GetTime() - m_fLastTouchTime < 0.5)
		return LTFALSE;

	// If the object isn't around right now, just exit
	uint32 dwFlags;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);

	if(!(dwFlags & FLAG_VISIBLE))
	{
//		// Make sure we go away if we need to...
//		if(!m_puButes.m_nNumRespawns && !m_bInfiniteRespawns)
//		{
//			g_pLTServer->RemoveObject(m_hObject);
//		}

		return LTFALSE;
	}


	// If there is a pickup delay... don't let it be picked up yet
	if(g_pLTServer->GetTime() < m_fPickupDelay)
		return LTFALSE;

	// If it wasn't a character... just return
	if(!IsCharacter(hSender))
		return LTFALSE;

	// If we're only allowed for players... check that
	if(m_puButes.m_bPlayerOnly)
	{
		if(!IsPlayer(hSender))
			return LTFALSE;
	}

	// [KLS] 9/23/01 Make sure if it is a player object that it is alive...
	if (IsPlayer(hSender))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
		if (!pPlayer || pPlayer->GetState() != PS_ALIVE) return LTFALSE;
	}

	// Send the message to the character who touched the pickup
	HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(this, hSender, MID_PICKUP);
	g_pLTServer->WriteToMessageString(hWrite, m_puButes.m_szMessage);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nUsedAmount[0]);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nUsedAmount[1]);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nUsedAmount[2]);
	g_pLTServer->WriteToMessageDWord(hWrite, m_nUsedAmount[3]);
	g_pLTServer->EndMessage(hWrite);

	// Record who picked us up and when
	m_hSender = hSender;
	m_fLastTouchTime = g_pLTServer->GetTime();

	// We'll need another update all the time
	SetNextUpdate( 0.001f );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Damage
//
//	PURPOSE:	Do stuff
//
// ----------------------------------------------------------------------- //

LTBOOL PickupObject::Damage(HOBJECT hSender, HMESSAGEREAD hRead)
{
	// If the object isn't around right now, just exit
	uint32 dwFlags;
	g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);

	if(!(dwFlags & FLAG_VISIBLE))
		return LTFALSE;


	// Setup our damage structure
	DamageStruct damage;
	damage.InitFromMessage(hRead);

	if(damage.fDamage)
	{
		// Get the location of the object
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		// Subtract the damage amount from our current hit points
		m_fCurrentHitPoints -= damage.fDamage;

		// If our hit points got to zero or below... kill the object
		if(m_fCurrentHitPoints <= 0.0f)
		{
			LTFLOAT fTimeScale = 1.0f;

			if(g_pGameServerShell->GetGameType().IsMultiplayer())
				fTimeScale = g_pGameServerShell->GetGameInfo()->m_fPowerupRespawnScale;

			// Set the respawn time to the time it takes to respawn
			m_fRespawnTime = g_pLTServer->GetTime() + (m_puButes.m_fRespawnTime * fTimeScale);

			// Make the object invisible (non-active)
			dwFlags &= ~FLAG_VISIBLE;
			dwFlags &= ~FLAG_RAYHIT;
			g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);

			// Create a destroy effect
			char *szDestroyFX = g_pGameServerShell->GetGameType().IsMultiplayer() ? m_puButes.m_szMPDestroyFX : m_puButes.m_szDestroyFX;

			if(szDestroyFX[0])
			{
				IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX(szDestroyFX);

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
			}

			// If we're single player... make it always only pick up once
			if(m_bForceNoRespawn || g_pGameServerShell->GetGameType().IsSinglePlayer())
			{
				m_puButes.m_nNumRespawns = 0;
				m_bInfiniteRespawns = LTFALSE;
			}

			// If we don't want to respawn... remove this pickup
			if(!m_puButes.m_nNumRespawns && !m_bInfiniteRespawns)
			{
				// Set the respawn time to the time it takes to respawn
				m_fRespawnTime = g_pLTServer->GetTime() + 0.001f;
			}


			// We'll need another update all the time
			SetNextUpdate( 0.001f );

			// If we have a startpoint, tell it to respawn another pickup...
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


			// Debug information!
#ifdef PICKUPOBJECT_DEBUG
			g_pLTServer->CPrint("PickupObject: %s at (%.2f %.2f %.2f) was destroyed!",
					m_puButes.m_szName, vPos.x, vPos.y, vPos.z);
#endif
		}
		else
		{
			// Debug information!
#ifdef PICKUPOBJECT_DEBUG
			g_pLTServer->CPrint("PickupObject: %s at (%.2f %.2f %.2f) was damaged by %.2f points!",
					m_puButes.m_szName, vPos.x, vPos.y, vPos.z, damage.fDamage);
#endif
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::PickedUp
//
//	PURPOSE:	Do stuff
//
// ----------------------------------------------------------------------- //

LTBOOL PickupObject::PickedUp(HOBJECT hSender, uint32* nAmt, LTBOOL bPartialPU)
{
	// see if we are all used up
	LTBOOL bPlaySound = LTTRUE;

	if(!bPartialPU)
	{
		LTFLOAT fTimeScale = 1.0f;

		if(g_pGameServerShell->GetGameType().IsMultiplayer())
			fTimeScale = g_pGameServerShell->GetGameInfo()->m_fPowerupRespawnScale;

		// Set the respawn time to the time it takes to respawn
		m_fRespawnTime = g_pLTServer->GetTime() + (m_puButes.m_fRespawnTime * fTimeScale);

		// Make the object invisible (non-active)
		uint32 dwFlags;
		g_pLTServer->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		dwFlags &= ~(FLAG_VISIBLE | FLAG_RAYHIT);
		g_pLTServer->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);

		// If we're single player... make it always only pick up once
		if(m_bForceNoRespawn || g_pGameServerShell->GetGameType().IsSinglePlayer())
		{
			m_puButes.m_nNumRespawns = 0;
			m_bInfiniteRespawns = LTFALSE;
		}

		// If we don't want to respawn... remove this pickup
		if(!m_puButes.m_nNumRespawns && !m_bInfiniteRespawns)
		{
			g_pLTServer->RemoveObject(m_hObject);
		}

		// We need another update immediately.
		SetNextUpdate( 0.001f );
	}
	else
	{
		m_nUsedAmount[0] += nAmt[0];
		m_nUsedAmount[1] += nAmt[1];
		m_nUsedAmount[2] += nAmt[2];
		m_nUsedAmount[3] += nAmt[3];

		uint32 nUsed = nAmt[0] + nAmt[1] + nAmt[2] + nAmt[3];

		if(nUsed == 0)
			bPlaySound = LTFALSE;
	}


	// Get the location of the object
	LTVector vPos;
	if(hSender)
		g_pLTServer->GetObjectPos(hSender, &vPos);
	else
		g_pLTServer->GetObjectPos(m_hObject, &vPos);


	// Play the respawn sound
	if(bPlaySound)
		g_pServerSoundMgr->PlaySoundFromPos(vPos, m_puButes.m_szPickupSound, 600.0f, SOUNDPRIORITY_MISC_HIGH);

	// Create a pickup effect
	char *szPickupFX = g_pGameServerShell->GetGameType().IsMultiplayer() ? m_puButes.m_szMPPickupFX : m_puButes.m_szPickupFX;

	if(szPickupFX[0])
	{
		IMPACTFX *pFX = g_pFXButeMgr->GetImpactFX(szPickupFX);

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
	}


	// Debug information!
#ifdef PICKUPOBJECT_DEBUG
	g_pLTServer->CPrint("PickupObject: Character %s picked up %s at (%.2f %.2f %.2f)!",
			g_pLTServer->GetObjectName(hSender),
			m_puButes.m_szName, vPos.x, vPos.y, vPos.z);
#endif


	// Send the pickup message
	SendTriggerMsgToObject(this, LTNULL, m_hstrPickupMsg);


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::PostStartWorld()
//
//	PURPOSE:	Called after all the objects and geometry has been loaded.
//
// ----------------------------------------------------------------------- //
void PickupObject::PostStartWorld(uint8 nLoadGameFlags)
{
	if( nLoadGameFlags != LOAD_RESTORE_GAME )
	{
		// Move the object to the floor if the butes tell us and the level designer
		// hasn't overridden it.
		if( !m_bForceNoMoveToGround && m_puButes.m_bMoveToGround )
			MoveAndAlignToFloor( );
	}

	// Call up to base.
	GameBase::PostStartWorld( nLoadGameFlags );
}


		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::PostStartWorld()
//
//	PURPOSE:	Moves the object to the floor with alignment to normal.
//
//  RETURN:		LTTRUE if hit the ground.
//
// ----------------------------------------------------------------------- //
LTBOOL PickupObject::MoveAndAlignToFloor( )
{
	// Save our required dims.
	LTVector vDims;
	g_pLTServer->GetObjectDims( m_hObject, &vDims );

	// Set our dims to something small but enough to hit stuff.
	g_pLTServer->SetObjectDims( m_hObject, &LTVector( 5.0f, vDims.y, 5.0f ));

	// Move the object to the floor.  If no floor, then leave it alone.
	IntersectInfo IInfo;
	LTBOOL bHitGround = MoveObjectToFloor( m_hObject, IInfo, LTFALSE, LTTRUE );

	// Restore our real dims.
	g_pLTServer->SetObjectDims( m_hObject, &vDims );

	// Check if we hit ground before.
	if( bHitGround )
	{
		// Stop the movement.
		g_pLTServer->SetVelocity(m_hObject, &LTVector(0,0,0));

		// Get the rotation of the object when it hit the ground.
		LTVector vF, vU, vR;
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);
		g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

		// Make the forward vector parallel with the ground.
		vR = IInfo.m_Plane.m_Normal.Cross( vF );
		vF = vR.Cross( IInfo.m_Plane.m_Normal );

		// Align the object with the normal of the object we hit.
		g_pLTServer->AlignRotation(&rRot, &vF, &IInfo.m_Plane.m_Normal);
		if(m_bAllowRotation)
			g_pLTServer->SetObjectRotation(m_hObject, &rRot);

		// Turn gravity off if we hit the world
		if(IInfo.m_hObject && (g_pLTServer->Physics()->IsWorldObject(IInfo.m_hObject) == LT_YES))
		{
			uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
			g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_GRAVITY);

			// Don't do move to floor checks in the update anymore.
			m_bForceNoMoveToGround = LTTRUE;
		}
	}

	return bHitGround;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PickupObject::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	m_puButes.Write(hWrite);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fCreateTime - g_pLTServer->GetTime());
	g_pLTServer->WriteToMessageFloat(hWrite, m_fCurrentHitPoints);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fRespawnTime - g_pLTServer->GetTime());
	g_pLTServer->WriteToMessageFloat(hWrite, m_fPickupDelay - g_pLTServer->GetTime());
	g_pLTServer->WriteToMessageByte(hWrite, m_bInfiniteRespawns);
	g_pLTServer->WriteToMessageByte(hWrite, m_bSliding);
	g_pLTServer->WriteToMessageByte(hWrite, m_bNeedSebtle);
	g_pLTServer->WriteToMessageByte(hWrite, m_bForceNoRespawn);
	g_pLTServer->WriteToMessageByte(hWrite, m_bForceNoMoveToGround);
	*hWrite << m_nUsedAmount[0];
	*hWrite << m_nUsedAmount[1];
	*hWrite << m_nUsedAmount[2];
	*hWrite << m_nUsedAmount[3];
	*hWrite << m_bAllowMovement;
	g_pLTServer->WriteToMessageHString(hWrite, m_hstrPickupMsg);	

	*hWrite << m_hAnim;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupObject::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	m_puButes.Read(hRead);
	m_fCreateTime = g_pLTServer->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();
	m_fCurrentHitPoints = g_pLTServer->ReadFromMessageFloat(hRead);
	m_fRespawnTime = g_pLTServer->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();
	m_fPickupDelay = g_pLTServer->ReadFromMessageFloat(hRead) + g_pLTServer->GetTime();
	m_bInfiniteRespawns = g_pLTServer->ReadFromMessageByte(hRead);
	m_bSliding = g_pLTServer->ReadFromMessageByte(hRead);
	m_bNeedSebtle = g_pLTServer->ReadFromMessageByte(hRead);
	m_bForceNoRespawn = g_pLTServer->ReadFromMessageByte(hRead);
	m_bForceNoMoveToGround = g_pLTServer->ReadFromMessageByte(hRead);
	*hRead >> m_nUsedAmount[0];

	if( g_pGameServerShell->GetLastSaveLoadVersion() >= 200110240)
	{
		*hRead >> m_nUsedAmount[1];
		*hRead >> m_nUsedAmount[2];
		*hRead >> m_nUsedAmount[3];
	}

	*hRead >> m_bAllowMovement;
//	*hRead >> m_bCharacterSpawned;
	m_hstrPickupMsg = g_pLTServer->ReadFromMessageHString(hRead);	

	//reset the filenames
	ObjectCreateStruct ocs;

	SAFE_STRCPY(ocs.m_Filename, m_puButes.m_szModel);

	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		SAFE_STRCPY(ocs.m_SkinNames[i], m_puButes.m_szSkins[i]);
	}
	g_pLTServer->Common()->SetObjectFilenames(m_hObject, &ocs);

	*hRead >> m_hAnim;
	g_pLTServer->SetModelAnimation(m_hObject, m_hAnim);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupObjectPlugin::PreHook_EditStringList()
//
//	PURPOSE:	Update the static list properties
//
// ----------------------------------------------------------------------- //

LTRESULT CPickupObjectPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	// See if we can handle the property...
	if(_stricmp("Pickup", szPropName) == 0)
	{
		m_PickupButeMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
					aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		m_PickupButeMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

