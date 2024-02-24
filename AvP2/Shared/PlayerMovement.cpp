// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMovement.cpp
//
// PURPOSE : General player movement
//
// CREATED : 2/29/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerMovement.h"
#include "CommandIds.h"
#include "MsgIDs.h"
#include "GameClientShell.h"
#include "VolumeBrushFX.h"
#include "SharedMovement.h"
#include "VarTrack.h"
#include "CameraMgrFX.h"
#include "CharacterFX.h"
#include "CharacterFuncs.h"

#include "MultiplayerClientMgr.h"

// ----------------------------------------------------------------------- //

VarTrack g_vtClientUpdateLowSendRate;
VarTrack g_vtClientUpdateSendRate;
VarTrack g_vtClientUpdateHighSendRate;

VarTrack g_vtHeadCanting;

// ----------------------------------------------------------------------- //

struct _SaveData
{
	_SaveData::_SaveData()
	{
		rRot.Init();
		fAim = 0.0f;
	}

	LTRotation	rRot;
	LTFLOAT		fAim;
};

static _SaveData SaveData;

// ----------------------------------------------------------------------- //

static HLOCALOBJ g_pCharacterOverlapData[32];

// ----------------------------------------------------------------------- //

static LTBOOL IsCameraPosInterpState(CharacterMovementState cms)
{
	switch(cms)
	{
		case CMS_IDLE:
		case CMS_WALKING:
		case CMS_RUNNING:
		case CMS_PRE_CRAWLING:
		case CMS_CRAWLING:
		case CMS_POST_CRAWLING:
			return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //

static LTBOOL IsCameraRotInterpState(CharacterMovementState cms)
{
	switch(cms)
	{
		case CMS_POUNCING:
		case CMS_CLIPPING:
			return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

static LTFLOAT GetSendRateFromState(CharacterMovementState cms)
{
	switch(cms)
	{
		case CMS_IDLE:
			return g_vtClientUpdateLowSendRate.GetFloat();

		case CMS_WALKING:
		case CMS_RUNNING:
		case CMS_CRAWLING:
		case CMS_WALLWALKING:
		case CMS_CLIMBING:
		case CMS_SWIMMING:
			return g_vtClientUpdateSendRate.GetFloat();

		case CMS_STAND_JUMP:
		case CMS_POUNCE_JUMP:
		case CMS_WALK_JUMP:
		case CMS_RUN_JUMP:
		case CMS_WWALK_JUMP:
		case CMS_STAND_FALL:
		case CMS_WALK_FALL:
		case CMS_RUN_FALL:
		case CMS_WWALK_FALL:
		case CMS_POUNCE_FALL:
		case CMS_POUNCING:
		case CMS_FACEHUG:
			return g_vtClientUpdateHighSendRate.GetFloat();
	}

	return g_vtClientUpdateSendRate.GetFloat();
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::PlayerMovement()
//
// PURPOSE:		Set everything to default values
//
// ----------------------------------------------------------------------- //

PlayerMovement::PlayerMovement()
{
	m_bMoveCode = 0;
	m_nLocalControlFlags = 0;
	m_hFollowObject = LTNULL;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::~PlayerMovement()
//
// PURPOSE:		Terminate everything if we didn't call it already
//
// ----------------------------------------------------------------------- //

PlayerMovement::~PlayerMovement()
{

}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::ReadSaveData()
//
// PURPOSE:		Read in the saved data and save it away
//
// ----------------------------------------------------------------------- //

void PlayerMovement::ReadSaveData(HMESSAGEREAD hData)
{
	*hData >> SaveData.rRot;
	*hData >> SaveData.fAim;
	*hData >> m_hFollowObject;

	*hData >> m_Vars;
	*hData >> m_cmsLast;
	*hData >> m_cmsCurrent;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteSaveData()
//
// PURPOSE:		Write out the data we want saved
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteSaveData(HMESSAGEWRITE hData)
{
	*hData << m_Vars.m_rRot;
	*hData << m_Vars.m_fAimingPitch;
	*hData << m_hFollowObject;


	*hData << m_Vars;
	*hData << m_cmsLast;
	*hData << m_cmsCurrent;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::RestoreSaveData()
//
// PURPOSE:		Reset the stored data
//
// ----------------------------------------------------------------------- //

void PlayerMovement::RestoreSaveData()
{
	SetRot(SaveData.rRot);
	m_Vars.m_fAimingPitch = SaveData.fAim;
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
	// Get the special FX manager so we can find the volume brush FX that have been created
	CSFXMgr *pFXMgr = g_pGameClientShell->GetSFXMgr();
	CVolumeBrushFX* pFX = (CVolumeBrushFX*)pFXMgr->FindSpecialFX(SFX_WEATHER_ID, hObj);

	if(!pFX)
		pFX = (CVolumeBrushFX*)pFXMgr->FindSpecialFX(SFX_VOLUMEBRUSH_ID, hObj);

	// If we did find an effect... adjust our final physics values accordingly
	if(pFX)
	{
		pInfo.m_fFriction *= pFX->GetFriction();
		if(pInfo.m_fViscosity < pFX->GetViscosity()) pInfo.m_fViscosity = pFX->GetViscosity();
		pInfo.m_vCurrent += pFX->GetCurrent();

		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	ConfirmTouchedObject
//
// PURPOSE:		Does a line intersect to make sure that there is nothing
//				between the mover and the object doing the touch notify.
//
// ----------------------------------------------------------------------- //

static void ConfirmTouchedObject(HOBJECT hMover, CollisionInfo * pInfo )
{
	if( pInfo->m_hObject && LT_NO == g_pInterface->Physics()->IsWorldObject(pInfo->m_hObject) )
	{
		// Make sure we there is nothing blocking us (a thin wall would allow
		// a touch notify, especially at low frame rates).
		IntersectQuery iQuery;
		IntersectInfo iInfo;

		// You must test from the target, to the pouncer!
		// The pouncer may be inside other geometry at the time of the touch notify.
		g_pInterface->GetObjectPos(pInfo->m_hObject,&iQuery.m_To);
		g_pInterface->GetObjectPos(hMover,&iQuery.m_From);

		// Test to the point where the object was before it started this movement.
//		LTVector vVel;
//		g_pInterface->Physics()->GetVelocity(hMover, &vVel);
//		iQuery.m_To -= vVel*0.2f;

		iQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		iQuery.m_FilterFn = LTNULL;
		iQuery.m_pUserData = LTNULL;

		if( g_pInterface->IntersectSegment(&iQuery, &iInfo) && iInfo.m_hObject != hMover )
		{
			// Change the pInfo so that it reads this new thing.
			pInfo->m_hObject = iInfo.m_hObject;
			pInfo->m_hPoly   = iInfo.m_hPoly;
			pInfo->m_Plane   = iInfo.m_Plane;
		}
	}		
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::Touch_Notify_Pounce_Jump()
//
// PURPOSE:		Handle Touch Notify for pouncing state
//
// ----------------------------------------------------------------------- //

static LTRESULT Touch_Notify_Pounce_Jump(CharacterMovement *pMovement, CollisionInfo *pInfo, DFLOAT fForceMag)
{
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	if(pInfo->m_hObject != g_pLTClient->GetClientObject() && cvVars->m_fStampTime != -2.0f && cvVars->m_fStampTime != -3.0f)
	{
		ConfirmTouchedObject( pMovement->GetObject(), pInfo );

		CharacterMovementState_Pounce_Jump_TN(pMovement, pInfo, fForceMag);

		LTVector vF = pMovement->GetForward();
		LTFLOAT fDamageAmount = (LTFLOAT)pMovement->GetCharacterButes()->m_nPounceDamage;

		// Send message to server.
		// If you modify this also modify Touch_Notify_Pouncing in ObjectDLL\\Character.cpp!!!
		HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pClientDE->WriteToMessageByte(hMessage, CP_POUNCE_DAMAGE);
		g_pClientDE->WriteToMessageByte(hMessage, DT_CRUSH);
		g_pClientDE->WriteToMessageFloat(hMessage, fDamageAmount);
		g_pClientDE->WriteToMessageVector(hMessage, &vF);
		hMessage->WriteObject(pInfo->m_hObject);
		g_pClientDE->EndMessage(hMessage);
	}

	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::Touch_Notify_Pouncing()
//
// PURPOSE:		Handle Touch Notify for pouncing state
//
// ----------------------------------------------------------------------- //

static LTRESULT Touch_Notify_Pouncing(CharacterMovement *pMovement, CollisionInfo *pInfo, DFLOAT fForceMag)
{
	if(pInfo->m_hObject != g_pLTClient->GetClientObject())
	{
		ConfirmTouchedObject( pMovement->GetObject(), pInfo );

		CharacterMovementState_Pouncing_TN(pMovement, pInfo, fForceMag);

		LTVector vF = pMovement->GetForward();
		LTFLOAT fDamageAmount = 100.0f;

		AMMO * pPounceAmmo = g_pWeaponMgr->GetAmmo("Pounce_Ammo");

		if( pPounceAmmo )
			fDamageAmount = LTFLOAT(pPounceAmmo->nInstDamage);

		// Send message to server.
		// If you modify this also modify Touch_Notify_Pouncing in ObjectDLL\\Character.cpp!!!
		HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pClientDE->WriteToMessageByte(hMessage, CP_POUNCE_DAMAGE);
		g_pClientDE->WriteToMessageByte(hMessage, DT_CRUSH);
		g_pClientDE->WriteToMessageFloat(hMessage, fDamageAmount);
		g_pClientDE->WriteToMessageVector(hMessage, &vF);
		hMessage->WriteObject(pInfo->m_hObject);
		g_pClientDE->EndMessage(hMessage);
	}

	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::Touch_Notify_Facehug()
//
// PURPOSE:		Handle Touch Notify for pouncing state
//
// ----------------------------------------------------------------------- //

static LTRESULT Touch_Notify_Facehug(CharacterMovement *pMovement, CollisionInfo *pInfo, DFLOAT fForceMag)
{
	if(pInfo->m_hObject != g_pLTClient->GetClientObject())
	{
		ConfirmTouchedObject( pMovement->GetObject(), pInfo );

		CharacterMovementState_Facehug_TN(pMovement, pInfo, fForceMag);

		LTVector vF = pMovement->GetForward();

		// Send message to server.
		// If you modify this also modify Touch_Notify_Facehug in ObjectDLL\\Character.cpp!!!
		HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pClientDE->WriteToMessageByte(hMessage, CP_DAMAGE);
		g_pClientDE->WriteToMessageByte(hMessage, DT_FACEHUG);
		g_pClientDE->WriteToMessageFloat(hMessage, 1000);
		g_pClientDE->WriteToMessageVector(hMessage, &vF);
		hMessage->WriteObject(pInfo->m_hObject);
		g_pClientDE->EndMessage(hMessage);
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::Init()
//
// PURPOSE:		Setup the player movement
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::Init(INTERFACE *pInterface, HOBJECT hObj, CharacterAnimation* pAnim)
{
	LTBOOL bRetVal = CharacterMovement::Init(pInterface, hObj, pAnim);

	SetContainerPhysicsInfoFunc(Container_Physics_Info);
	SetTouchNotifyFunc(CMS_POUNCING, Touch_Notify_Pouncing);
	SetTouchNotifyFunc(CMS_FACEHUG, Touch_Notify_Facehug);
	SetTouchNotifyFunc(CMS_POUNCE_JUMP, Touch_Notify_Pounce_Jump);
	SetTouchNotifyFunc(CMS_POUNCE_FALL, Touch_Notify_Pounce_Jump);

	// Init our client update send rate variable
	g_vtClientUpdateLowSendRate.Init(g_pLTClient, "ClientUpdateLowSendRate", NULL, 0.25f);
	g_vtClientUpdateSendRate.Init(g_pLTClient, "ClientUpdateSendRate", NULL, 0.125f);
	g_vtClientUpdateHighSendRate.Init(g_pLTClient, "ClientUpdateHighSendRate", NULL, 0.08f);

	g_vtHeadCanting.Init(g_pLTClient, "HeadCanting", NULL, 1.0f);

	m_fClientUpdateTime = 0.0f;

	return bRetVal;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::HandleMessageRead()
//
// PURPOSE:		Recieves a message from the client or server
//				Return true if the message was handled, or false if not...
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::HandleMessageRead(uint8 nType, HMESSAGEREAD hRead, uint32 dwFlags)
{
	// If we want to handle the message externally, go ahead and call that function
	if(m_fpHandleMessageRead)
		return (*m_fpHandleMessageRead)(nType, hRead, dwFlags);


	// Otherwise, do some standard handling of the message
	switch(nType)
	{
		case MID_SERVERFORCEPOS:
			return HandleMessage_ServForcePos(hRead, dwFlags);

		case MID_SERVERFORCEVELOCITY:
			return HandleMessage_ServForceVelocity(hRead, dwFlags);

		case MID_PLAYER_ORIENTATION:
			return HandleMessage_Orientation(hRead, dwFlags);

		case MID_PHYSICS_UPDATE:
			return HandleMessage_Physics_Update(hRead, dwFlags);
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::HandleMessageWrite()
//
// PURPOSE:		Recieves a message from the client or server
//				Return true if the message was handled, or false if not...
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::HandleMessageWrite(uint8 nType, HMESSAGEREAD hRead, uint32 dwFlags)
{
	// If we want to handle the message externally, go ahead and call that function
	if(m_fpHandleMessageWrite)
		return (*m_fpHandleMessageWrite)(nType, hRead, dwFlags);


	// Otherwise, do some standard handling of the message

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::Update()
//
// PURPOSE:		Handle the current state of the player movement
//
// ----------------------------------------------------------------------- //

void PlayerMovement::Update()
{
	// Get the game settings
	CGameSettings *pSettings = g_pGameClientShell->GetGameSettings();


	// Check the strafe toggle
	if( !m_hFollowObject )
	{
		// Clear the control flags
		uint32 dwCtrl = 0;

		if(!g_pGameClientShell->IsCinematic() && !g_pGameClientShell->GetInterfaceMgr()->CinematicSkipDelay())
		{
			// [KLS] 9/23/2001 If movement isn't allowed, check and see if clip movement is
			// allowed (multiplayer only)...

			LTBOOL bAllowMovement = g_pGameClientShell->GetMultiplayerMgr()->AllowMovementControl();
			if (!bAllowMovement && g_pGameClientShell->IsMultiplayerGame())
			{
				bAllowMovement = g_pGameClientShell->GetMultiplayerMgr()->AllowClipMovementControl();
			}

			if(!g_pGameClientShell->IsMultiplayerGame() || bAllowMovement)
			{
				LTBOOL bStrafe = LTFALSE;
				if(m_pInterface->IsCommandOn(COMMAND_ID_STRAFE))		bStrafe = LTTRUE;

				// Update the control flags
				if(m_pInterface->IsCommandOn(COMMAND_ID_FORWARD))		dwCtrl |= CM_FLAG_FORWARD;
				if(m_pInterface->IsCommandOn(COMMAND_ID_BACKWARD))		dwCtrl |= CM_FLAG_BACKWARD;
				if(m_pInterface->IsCommandOn(COMMAND_ID_LEFT))			dwCtrl |= bStrafe ? CM_FLAG_STRAFELEFT : CM_FLAG_LEFT;
				if(m_pInterface->IsCommandOn(COMMAND_ID_RIGHT))			dwCtrl |= bStrafe ? CM_FLAG_STRAFERIGHT : CM_FLAG_RIGHT;
				if(m_pInterface->IsCommandOn(COMMAND_ID_STRAFE_LEFT))	dwCtrl |= CM_FLAG_STRAFELEFT;
				if(m_pInterface->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))	dwCtrl |= CM_FLAG_STRAFERIGHT;
				if(m_pInterface->IsCommandOn(COMMAND_ID_RUN))			dwCtrl |= CM_FLAG_RUN;
				if(m_pInterface->IsCommandOn(COMMAND_ID_DUCK))			dwCtrl |= CM_FLAG_DUCK;
				if(m_pInterface->IsCommandOn(COMMAND_ID_WALLWALK))		dwCtrl |= CM_FLAG_WALLWALK;
				if(m_pInterface->IsCommandOn(COMMAND_ID_JUMP))			dwCtrl |= CM_FLAG_JUMP;
				if(m_pInterface->IsCommandOn(COMMAND_ID_LOOKUP))		dwCtrl |= CM_FLAG_LOOKUP;
				if(m_pInterface->IsCommandOn(COMMAND_ID_LOOKDOWN))		dwCtrl |= CM_FLAG_LOOKDOWN;
				if(m_pInterface->IsCommandOn(COMMAND_ID_FIRING))		dwCtrl |= CM_FLAG_PRIMEFIRING;
				if(m_pInterface->IsCommandOn(COMMAND_ID_ALT_FIRING))	dwCtrl |= CM_FLAG_ALTFIRING;

				// All the 'lock' keys should switch the current functionality
				if(pSettings->RunLock())		dwCtrl ^= CM_FLAG_RUN;

				if(GetMovementState() == CMS_SWIMMING)
				{
					pSettings->SetCrouchLock(LTFALSE);
					pSettings->SetWallWalkLock(LTFALSE);
				}
				else
				{
					if(pSettings->CrouchLock())		dwCtrl ^= CM_FLAG_DUCK;
					if(pSettings->WallWalkLock())	dwCtrl ^= CM_FLAG_WALLWALK;
				}


				//include any local control flags
				dwCtrl |= m_nLocalControlFlags;
			}
		}

		//re-set local control flags
		m_nLocalControlFlags = 0;

		// Now really set the control flags
		SetControlFlags(dwCtrl);

		// Update our model lighting.
		if( g_pGameClientShell->GetGameType()->IsSinglePlayer() )
		{
			LTVector vModelLighting(-1.0f,-1.0f,-1.0f);

			if(CWeaponModel * pWeaponModel = g_pGameClientShell->GetWeaponModel())
			{
				if( pWeaponModel->GetHandle() )
					g_pModelLT->GetModelLighting(pWeaponModel->GetHandle(), vModelLighting);
			}

			if( g_pLTClient->GetClientObject() )
			{
				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SFX_MESSAGE);
				g_pLTClient->WriteToMessageByte(hMessage, SFX_CHARACTER_ID );
				g_pLTClient->WriteToMessageObject(hMessage, g_pLTClient->GetClientObject() );
				g_pLTClient->WriteToMessageByte(hMessage, CFX_MODELLIGHTING );
				g_pLTClient->WriteToMessageVector(hMessage, &vModelLighting);
				g_pLTClient->EndMessage(hMessage);
			}
		}

	}
	else
	{
		LTVector vPos;
		m_pInterface->GetObjectPos(m_hFollowObject,&vPos);
		LTRotation rRot;
		m_pInterface->GetObjectRotation(m_hFollowObject,&rRot);

		SetPos(vPos, MOVEOBJECT_TELEPORT);
		SetRot(rRot);

		// Update our model lighting (assume it is same as the follow object).
		if( g_pGameClientShell->GetGameType()->IsSinglePlayer() )
		{
			LTVector vModelLighting(-1.0f,-1.0f,-1.0f);
			g_pModelLT->GetModelLighting(m_hFollowObject, vModelLighting);

			if( g_pLTClient->GetClientObject() )
			{
				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SFX_MESSAGE);
				g_pLTClient->WriteToMessageByte(hMessage, SFX_CHARACTER_ID );
				g_pLTClient->WriteToMessageObject(hMessage, g_pLTClient->GetClientObject() );
				g_pLTClient->WriteToMessageByte(hMessage, CFX_MODELLIGHTING );
				g_pLTClient->WriteToMessageVector(hMessage, &vModelLighting);
				g_pLTClient->EndMessage(hMessage);
			}
		}
	}


	// Update the speed modifiers
	UpdateSpeedMods();


	// Check for stuck situations with other characters...
	CheckCharacterOverlaps();


	// Call the base function to finish the movement update
	if(!m_hFollowObject)
		CharacterMovement::Update(m_pInterface->GetFrameTime());


	// Check for stuck situations with other characters...
	ResetCharacterOverlaps();


	// Update special camera control
	UpdateCameraControl();
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::UpdateRotation()
//
// PURPOSE:		Handles rotating the character
//
// ----------------------------------------------------------------------- //

void PlayerMovement::UpdateRotation()
{
	// Get the game settings
	CGameSettings *pSettings = g_pGameClientShell->GetGameSettings();
	LTBOOL bMouseLook = m_pInterface->IsCommandOn(COMMAND_ID_MOUSEAIMTOGGLE) || pSettings->MouseLook();

	if(bMouseLook && CanRotate() && !m_hFollowObject && g_pGameClientShell->GetMultiplayerMgr()->AllowMouseControl())
	{
		// Set the rotation based off the gameclientshell values temporarily
		LTFLOAT pOffsets[3];
		g_pLTClient->GetAxisOffsets(pOffsets);

		// Get the rotation vectors of the character
		LTVector vR, vU, vF;
		LTRotation rRot = m_Vars.m_rRot;
		m_pMath->GetRotationVectors(rRot, vR, vU, vF);

		LTFLOAT fTurnRate = 90.0f;
		LTFLOAT fTurnAmount = (pOffsets[0] * fTurnRate * MATH_PI / 180.0f);

		// Rotate around the up axis of the character for side to side rotation
		m_pMath->RotateAroundAxis(rRot, vU, fTurnAmount);
		m_pInterface->SetObjectRotation(m_Vars.m_hObject, &rRot);

		// Fill in the object rotation again
		m_pInterface->GetObjectRotation(m_Vars.m_hObject, &m_Vars.m_rRot);
	}

	// Do the standard control rotation updates...
	CharacterMovement::UpdateRotation();
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::UpdateAiming()
//
// PURPOSE:		Handle the lookup and lookdown controls of the character
//
// ----------------------------------------------------------------------- //

void PlayerMovement::UpdateAiming()
{
	// See if we're in a cinematic... if so... skip this
	HCAMERA hCamera = g_pGameClientShell->GetCinematicCamera();
	if(g_pGameClientShell->GetCameraMgr()->GetCameraActive(hCamera))
		return;

	// Get the game settings
	CGameSettings *pSettings = g_pGameClientShell->GetGameSettings();
	LTBOOL bMouseLook = m_pInterface->IsCommandOn(COMMAND_ID_MOUSEAIMTOGGLE) || pSettings->MouseLook();

	if(bMouseLook && g_pGameClientShell->GetMultiplayerMgr()->AllowMouseControl())
	{
		// Set the rotation based off the gameclientshell values temporarily
		LTFLOAT pOffsets[3];
		g_pLTClient->GetAxisOffsets(pOffsets);

		LTFLOAT fAimDir = pSettings->MouseInvertY() ? -1.0f : 1.0f;
		LTFLOAT fAimRate = 45.0f;
		LTFLOAT fAimAmount = (pOffsets[1] * fAimRate * fAimDir);

		// Add this to the final aim rotation and cap the values
		m_Vars.m_fAimingPitch += fAimAmount;
	}
	else if((m_Vars.m_fAimingPitch != 0.0f) && pSettings->AutoCenter())
	{
		if(!(m_Vars.m_dwCtrl & CM_FLAG_LOOKUP) && !(m_Vars.m_dwCtrl & CM_FLAG_LOOKDOWN))
		{
			// Bring the view back to the center
			LTFLOAT fAimRate = 45.0f;
			LTFLOAT fPitchDelta = m_Vars.m_fFrameTime * fAimRate;

			if (m_Vars.m_fAimingPitch > 0.0f) m_Vars.m_fAimingPitch -= LTMIN(fPitchDelta, m_Vars.m_fAimingPitch);
			if (m_Vars.m_fAimingPitch < 0.0f) m_Vars.m_fAimingPitch += LTMIN(fPitchDelta, -m_Vars.m_fAimingPitch);
		}
	}

	// Do the standard control aiming updates...
	CharacterMovement::UpdateAiming();
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdatePosition()
//
// PURPOSE:		Handle sending of position information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdatePosition(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_POSITION) || !m_pInterface)
		return;

	LTVector vPos;
	GetPos(vPos);

	// Write the move code to keep the client and server in sync
	m_pInterface->WriteToMessageByte(hWrite, m_bMoveCode);

	// Write the current position
	m_pInterface->WriteToMessageVector(hWrite, &vPos);

	// Write the handle of the poly we're standing on
	m_pInterface->WriteToMessageDWord(hWrite, (int32)(m_Vars.m_bStandingOn ? m_Vars.m_ciStandingOn.m_hPoly : INVALID_HPOLY));
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateRotation()
//
// PURPOSE:		Handle sending of rotation information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateRotation(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_ROTATION) || !m_pInterface)
		return;

	LTRotation rRot;
	GetRot(rRot);

	// Write the current rotation
	m_pInterface->WriteToMessageRotation(hWrite, &rRot);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateVelocity()
//
// PURPOSE:		Handle sending of rotation information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateVelocity(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_VELOCITY) || !m_pInterface)
		return;

	LTVector vVel;
	GetVelocity(vVel);

	// Write the current velocity
	m_pInterface->WriteToMessageVector(hWrite, &vVel);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateAiming()
//
// PURPOSE:		Handle sending of aiming information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateAiming(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_AIMING) || !m_pInterface)
		return;

	// Write the current aiming values
	int8 nAimingPitch = (int8)m_Vars.m_fAimingPitch;

	m_pInterface->WriteToMessageByte(hWrite, nAimingPitch);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateAllowedInput()
//
// PURPOSE:		Handle sending of allowable input information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateAllowedInput(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_INPUTALLOWED) || !m_pInterface)
		return;

	// Write the current input values
	uint8 nFlags = 0;

	if(m_Vars.m_nAllowMovement) nFlags |= 0x01;
	if(m_Vars.m_nAllowRotation) nFlags |= 0x02;

	m_pInterface->WriteToMessageByte(hWrite, nFlags);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateScale()
//
// PURPOSE:		Handle sending of scale information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateScale(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_SCALE) || !m_pInterface)
		return;

	LTFLOAT fScale = GetScale();

	// Write the current dims values
	m_pInterface->WriteToMessageFloat(hWrite, fScale);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateDims()
//
// PURPOSE:		Handle sending of dims information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateDims(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_DIMS) || !m_pInterface)
		return;

	LTVector vDims = GetObjectDims();

	// Write the current dims values
	m_pInterface->WriteToMessageVector(hWrite, &vDims);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateFlags()
//
// PURPOSE:		Handle sending of object flag information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateFlags(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_FLAGS) || !m_pInterface)
		return;

	// Write the actual object flags instead of the saved ones
	uint32 nFlags;

	m_pInterface->Common()->GetObjectFlags(m_Vars.m_hObject, OFT_Flags, nFlags);

	m_pInterface->WriteToMessageDWord(hWrite, nFlags);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateFlags2()
//
// PURPOSE:		Handle sending of object flag information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateFlags2(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_FLAGS2) || !m_pInterface)
		return;

	// Write the actual object flags instead of the saved ones
	uint32 nFlags2;

	m_pInterface->Common()->GetObjectFlags(m_Vars.m_hObject, OFT_Flags2, nFlags2);

	// We can write this as a word because there are no high level flags
	m_pInterface->WriteToMessageWord(hWrite, (uint16)nFlags2);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateMovementState()
//
// PURPOSE:		Handle sending of movement state information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateMovementState(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_MOVEMENTSTATE) || !m_pInterface)
		return;

	// Write the current movement state values
	m_pInterface->WriteToMessageByte(hWrite, (uint8)m_cmsCurrent);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::WriteMessage_UpdateCtrlFlags()
//
// PURPOSE:		Handle sending of control flags information
//
// ----------------------------------------------------------------------- //

void PlayerMovement::WriteMessage_UpdateCtrlFlags(HMESSAGEWRITE hWrite)
{
	if(!(m_nClientUpdateFlags & CLIENTUPDATE_CTRLFLAGS) || !m_pInterface)
		return;

	// Write the current control flag value
	m_pInterface->WriteToMessageWord(hWrite, (uint16)m_Vars.m_dwCtrl);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::ResetNet()
//
// PURPOSE:		Reset the net stuf
//
// ----------------------------------------------------------------------- //
/*
void PlayerMovement::ResetNet()
{
	while(!CanMove())
		AllowMovement(TRUE);

	while(!CanRotate())
		AllowRotation(TRUE);

	SetMovementState(CMS_IDLE);
	uint32 dwFlags = GetObjectFlags(OFT_Flags);
	SetObjectFlags(OFT_Flags, dwFlags | FLAG_GRAVITY);

	// Remove the net overlay
//	g_pGameClientShell->GetVisionModeMgr()->SetNetMode(LTFALSE);
}
*/

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::UpdateCameraControl()
//
// PURPOSE:		Handle updating special camera stuff
//
// ----------------------------------------------------------------------- //

void PlayerMovement::UpdateCameraControl()
{
	// Check for camera state changes
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	CameraMgr *pCameraMgr = g_pGameClientShell->GetCameraMgr();

	if(pCameraMgr)
	{
		uint32 nFlags = pCameraMgr->GetCameraFlags(hCamera);

		if(IsCameraRotInterpState(GetMovementState()))
			nFlags |= CAMERA_FLAG_INTERP_ROT;
		else
			nFlags &= ~CAMERA_FLAG_INTERP_ROT;

		if(IsCameraPosInterpState(GetMovementState()) && (GetObjectDims().y > 10.0f))
			nFlags |= CAMERA_FLAG_INTERP_POS_UP;
		else
			nFlags &= ~CAMERA_FLAG_INTERP_POS_UP;

		pCameraMgr->SetCameraFlags(hCamera, nFlags);
	}


	// See what the flags are and update the head tilt
	uint32 nFlags = GetControlFlags();

	if(!g_vtHeadCanting.GetFloat())
	{
		CameraMgrFX_SetHeadTiltAmount(g_pGameClientShell->GetPlayerCamera(), 0.0f);
	}
	else if(nFlags & CM_FLAG_STRAFELEFT)
	{
		if((nFlags & CM_FLAG_FORWARD) || (nFlags & CM_FLAG_BACKWARD))
			CameraMgrFX_SetHeadTiltAmount(g_pGameClientShell->GetPlayerCamera(), -0.5f);
		else
			CameraMgrFX_SetHeadTiltAmount(g_pGameClientShell->GetPlayerCamera(), -1.0f);
	}
	else if(nFlags & CM_FLAG_STRAFERIGHT)
	{
		if((nFlags & CM_FLAG_FORWARD) || (nFlags & CM_FLAG_BACKWARD))
			CameraMgrFX_SetHeadTiltAmount(g_pGameClientShell->GetPlayerCamera(), 0.5f);
		else
			CameraMgrFX_SetHeadTiltAmount(g_pGameClientShell->GetPlayerCamera(), 1.0f);
	}
	else
	{
		CameraMgrFX_SetHeadTiltAmount(g_pGameClientShell->GetPlayerCamera(), 0.0f);
	}


	// Do special camera update for aiming adjustments after coming out of wall walking
/*	LTRotation rRot;
	LTVector vPos, vR, vU, vF, vTemp;

	if( (GetLastMovementState() == CMS_WALLWALKING) && (GetMovementState() != CMS_WALLWALKING) &&
		!(pCameraMgr->GetCameraFlags(hCamera) & CAMERA_FLAG_FOLLOW_TARGET_POS))
	{
		// See if we're already following a target
		pCameraMgr->GetCameraPos(hCamera, vPos, LTTRUE);
		pCameraMgr->GetCameraRotation(hCamera, rRot, LTTRUE);
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

		ClientIntersectInfo info;
		ClientIntersectQuery query;
		memset(&query, 0, sizeof(query));
		query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		query.m_From = vPos;
		query.m_To = vPos + (vF * 5000.0f);

		if(g_pLTClient->IntersectSegment(&query, &info))
		{
			vPos = info.m_Point;
		}
		else
		{
			vPos = query.m_To;
		}

		pCameraMgr->SetCameraPos(hCamera, vPos);
		pCameraMgr->SetCameraFlags(hCamera, pCameraMgr->GetCameraFlags(hCamera) | CAMERA_FLAG_FOLLOW_TARGET_POS);
//		GetCharacterVars()->m_fAimingPitch = vF.y * -90.0f;
		m_Vars.m_fAimingPitch = 0.0f;
	}
	else if(pCameraMgr->GetCameraFlags(hCamera) & CAMERA_FLAG_FOLLOW_TARGET_POS)
	{
		pCameraMgr->GetCameraRotation(hCamera, rRot, LTTRUE);
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);
		vTemp = GetRight();

		if(vTemp.Dot(vR) > 0.99f)
			pCameraMgr->SetCameraFlags(hCamera, pCameraMgr->GetCameraFlags(hCamera) & ~CAMERA_FLAG_FOLLOW_TARGET_POS);
	}
*/
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::HandleMessage_ServForcePos()
//
// PURPOSE:		Handle a message of type MID_SERVFORCEPOS
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::HandleMessage_ServForcePos(HMESSAGEREAD hRead, uint32 dwFlags)
{
	// If the object isn't valid yet... then don't try to handle this message
	if(!m_Vars.m_hObject || !m_pInterface) return LTFALSE;

	// Make sure the clientside object of the server player is non-solid.  Otherwise
	// we'll collide with it.
	HOBJECT hClientObj = m_pInterface->GetClientObject();
	if( hClientObj )
	{
		uint32 dwFlags = m_pInterface->GetObjectFlags(hClientObj);
		m_pInterface->SetObjectFlags(hClientObj, dwFlags & ~FLAG_SOLID);
	}

	// Read in the position and increment value data
	LTVector vPos;
	m_bMoveCode = m_pInterface->ReadFromMessageByte(hRead);
	m_pInterface->ReadFromMessageVector(hRead, &vPos);

	// Set the dims very small for a minute so that when we move it, we can resize it from smaller dims
	LTVector vOldDims = GetObjectDims();
	SetObjectDims(CM_DIMS_VERY_SMALL);

	// Set the flags up so the object won't get any collision responses
	uint32 nFlags = GetObjectFlags(OFT_Flags);
	SetObjectFlags(OFT_Flags, nFlags | FLAG_GOTHRUWORLD, DFALSE);

	// Teleport the object to the new location
	SetPos(vPos, MOVEOBJECT_TELEPORT);

	// Reset the flags to their previous values
	SetObjectFlags(OFT_Flags, nFlags, DFALSE);

	// Reset the dims so we can see if we really fit in the area we teleported to
	SetObjectDims(vOldDims);

	// Make sure we're not moving at all
	LTVector vZero(0.0f, 0.0f, 0.0f);
	m_pPhysics->SetAcceleration(m_Vars.m_hObject, &vZero);
	m_pPhysics->SetVelocity(m_Vars.m_hObject, &vZero);


	// Set the camera to this position so it doesn't interpolate from god knows where
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	g_pGameClientShell->GetCameraMgr()->SetCameraPos(hCamera, vPos);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::HandleMessage_ServForceVelocity()
//
// PURPOSE:		Handle a message of type MID_SERVFORCEPOS
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::HandleMessage_ServForceVelocity(HMESSAGEREAD hRead, uint32 dwFlags)
{
	// If the object isn't valid yet... then don't try to handle this message
	if(!m_Vars.m_hObject || !m_pInterface) return LTFALSE;

	// Read in the position and increment value data
	LTVector vVel;
	m_pInterface->ReadFromMessageVector(hRead, &vVel);

	SetVelocity(vVel);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::HandleMessage_Physics_Update()
//
// PURPOSE:		Handle a message of type MID_PHYSICS_UPDATE
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::HandleMessage_Physics_Update(HMESSAGEREAD hRead, uint32 dwFlags)
{
	// If the object isn't valid yet... then don't try to handle this message
	if(!m_Vars.m_hObject || !m_pInterface) return LTFALSE;

	// Read the state flags so we know what to read out
	uint16 nFlags = m_pInterface->ReadFromMessageWord(hRead);

	// If we need to reset the attributes, send down the new player data
	if(nFlags & PSTATE_ATTRIBUTES)
	{
		// Read the character attribute set to use
		uint8 nCharacter = m_pInterface->ReadFromMessageByte(hRead);

		// See if this is a restore or not...
		LTBOOL bRestore = m_pInterface->ReadFromMessageByte(hRead);

		// ----------------------------------------------------------------------- //
		// Set the client up to deal with this character type

		// Set the butes to this new set
		SetCharacterButes(nCharacter);

		// Set the new default camera FOV
		CPoint ptFOV = GetCharacterButes()->m_ptFOV;
		g_pGameClientShell->GetCameraMgr()->SetCameraFOV(g_pGameClientShell->GetPlayerCamera(), (DFLOAT)ptFOV.x, (DFLOAT)ptFOV.y);
		g_pGameClientShell->GetCameraMgr()->SetCameraFOV(g_pGameClientShell->Get3rdPersonCamera(), (DFLOAT)ptFOV.x, (DFLOAT)ptFOV.y);

		// Setup the player stats
		g_pGameClientShell->GetPlayerStats()->SetCharacterButes(nCharacter);

		// Force new hud
		g_pInterfaceMgr->GetHudMgr()->Init(0,bRestore);

		// Reset the shared surfaces
		g_pInterfaceMgr->ResetSharedSurfaces();

		// Init the vision mode mgr
		g_pGameClientShell->GetVisionModeMgr()->Init();

		// Reset the mouse sensitivity
		g_pGameClientShell->SetMouseSensitivityScale();

		// Setup the head tilt effect
		CameraMgrFX_HeadTiltCreate(g_pGameClientShell->GetPlayerCamera(), GetCharacterButes()->m_fHeadTiltAngle, GetCharacterButes()->m_fHeadTiltSpeed);
	}

	// Send down the container information
	if(nFlags & PSTATE_SCALE)
	{
		LTFLOAT fScale = g_pLTClient->ReadFromMessageFloat(hRead);
		SetScale(fScale);
	}

	// Check for movements state forcing operations
	if((nFlags & PSTATE_POUNCE) || (nFlags & PSTATE_FACEHUG))
	{
		CharacterMovementState cms = GetMovementState();

		if(m_Vars.m_bStandingOn || cms == CMS_WALLWALKING)
		{
			LTVector vTargetPos, vVel;

			m_pInterface->ReadFromMessageVector(hRead, &vTargetPos);
			m_pInterface->ReadFromMessageVector(hRead, &vVel);

			if(GetCharacterButes()->m_fBasePounceSpeed > 0.0f)
			{
				vTargetPos += vVel * (vTargetPos - GetPos()).Mag() / GetCharacterButes()->m_fBasePounceSpeed;
			}

			LTVector vPos = GetPos();

			// Start us up a bit so we will not stub our toes on ramps and such...
			// But only if we are short!
			if(GetObjectDims().y <= 20.0f)
				vPos += m_Vars.m_vUp * 20.0;

			SetStart(vPos);
			SetDestination(vTargetPos, m_Vars.m_rRot);
			SetTimeStamp(m_pInterface->GetTime());
			SetVelocity(LTVector(0,0,0));
			SetAcceleration(LTVector(0,0,0));

			SetMovementState((nFlags & PSTATE_POUNCE) ? CMS_POUNCING : CMS_FACEHUG);
		}
	}

	if(nFlags & PSTATE_REMOVE_NET_FX)
	{
		AllowMovement(LTTRUE);
		AllowRotation(LTTRUE);

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
		CameraMgrFX_NetTiltSetRampDown(hCamera);

		// Remove the net overlay
		g_pGameClientShell->GetVisionModeMgr()->SetNetMode(LTFALSE);
	}

	if(nFlags & PSTATE_SET_NET_FX)
	{
		SetMovementState(CMS_PRE_FALLING);
		SetObjectFlags(OFT_Flags, GetObjectFlags(OFT_Flags) | FLAG_GRAVITY);

		LTVector vVel;
		m_pInterface->ReadFromMessageVector(hRead, &vVel);

		AllowMovement(LTFALSE);
		AllowRotation(LTFALSE);

		SetVelocity(vVel);

		HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();

		LTVector vDims = GetObjectDims();
		CameraMgrFX_NetTiltCreate(hCamera, LTVector(0.0f, 0.0f, -90.0f), LTVector(0.0f, -(vDims.y*1.5f), 0.0f));

		//set the net overlay
		g_pGameClientShell->GetVisionModeMgr()->SetNetMode(LTTRUE);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::HandleMessage_Physics_Update()
//
// PURPOSE:		Handle a message of type MID_PLAYER_ORIENTATION
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::HandleMessage_Orientation(HMESSAGEREAD hRead, uint32 dwFlags)
{
	LTVector vVec;
	LTRotation rRot;

	LTBOOL bQuat = (LTBOOL)g_pLTClient->ReadFromMessageByte(hRead);

	if(bQuat)
	{
		g_pLTClient->ReadFromMessageRotation(hRead, &rRot);
		SetRot(rRot);
	}
	else
	{
		g_pLTClient->ReadFromMessageVector(hRead, &vVec);
		SetRot(vVec);
	}


	// Set the camera to this rotation so it doesn't interpolate from some odd angle
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	rRot = GetRot();
	g_pGameClientShell->GetCameraMgr()->SetCameraRotation(hCamera, rRot);


	// [KLS] If we receive the orientation message while in clip mode, update
	// the pitch to reflect the new camera rotation...

	if (GetMovementState() == CMS_CLIPPING)
	{
		LTVector vEuler;
		m_pMath->GetEulerAngles(rRot, vEuler);
		if (vEuler.x < 0.0f) vEuler.x += MATH_CIRCLE;

		m_Vars.m_fAimingPitch = MATH_RADIANS_TO_DEGREES(vEuler.x);
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::GetClientUpdateFlags()
//
// PURPOSE:		Checks information to see what's changed and returns some
//				flags based off of that info.
//
// ----------------------------------------------------------------------- //

uint16 PlayerMovement::GetClientUpdateFlags()
{
	// If the object isn't valid yet... then don't try to do this.
	if(!m_Vars.m_hObject || !m_pInterface) return 0;

	// Init the flags
	uint16 nUpdateFlags = 0;


	// Check the position
	LTVector vPos = GetPos();
	if(!vPos.Equals(m_CUPos, 0.5f))
	{
		m_CUPos = vPos;
		nUpdateFlags |= CLIENTUPDATE_POSITION;
	}

	// Check the rotation
	LTRotation rRot = GetRot();
	if(!(rRot.Equals(m_CURot, 0.001f)))
	{
		m_CURot = rRot;
		nUpdateFlags |= CLIENTUPDATE_ROTATION;
	}

	// Check the velocity
	LTVector vVel = GetVelocity();
	if(!vVel.Equals(m_CUVel, 0.5f))
	{
		m_CUVel = vVel;
		nUpdateFlags |= CLIENTUPDATE_VELOCITY;
	}

	// Check the aiming pitch
	int8 nAim = (int8)m_Vars.m_fAimingPitch;
	if(nAim != m_CUAimingPitch)
	{
		m_CUAimingPitch = nAim;
		nUpdateFlags |= CLIENTUPDATE_AIMING;
	}

	// Check the allowed input
	uint8 nInput = 0;
	if(m_Vars.m_nAllowMovement) nInput |= 0x01;
	if(m_Vars.m_nAllowRotation) nInput |= 0x02;
	if(nInput != m_CUAllowedInput)
	{
		m_CUAllowedInput = nInput;
		nUpdateFlags |= CLIENTUPDATE_INPUTALLOWED;
	}

	// Check the scale
	LTFLOAT fScale = GetScale();
	if(fScale != m_CUScale)
	{
		m_CUScale = fScale;
		nUpdateFlags |= CLIENTUPDATE_SCALE;
	}

	// Check the dims
	LTVector vDims = GetObjectDims();
	if(!vDims.Equals(m_CUDims, 0.5f))
	{
		m_CUDims = vDims;
		nUpdateFlags |= CLIENTUPDATE_DIMS;
	}

	// Check the flags
	uint32 nFlags;
	m_pInterface->Common()->GetObjectFlags(m_Vars.m_hObject, OFT_Flags, nFlags);
	if(nFlags != m_CUFlags)
	{
		m_CUFlags = nFlags;
		nUpdateFlags |= CLIENTUPDATE_FLAGS;
	}

	// Check the second flags
	uint32 nFlags2;
	m_pInterface->Common()->GetObjectFlags(m_Vars.m_hObject, OFT_Flags2, nFlags2);
	if((int16)nFlags2 != m_CUFlags2)
	{
		m_CUFlags2 = (uint16)nFlags2;
		nUpdateFlags |= CLIENTUPDATE_FLAGS2;
	}

	// Check the movement state
	uint8 nState = (uint8)m_cmsCurrent;
	if(nState != m_CUMovementState)
	{
		m_CUMovementState = nState;
		nUpdateFlags |= CLIENTUPDATE_MOVEMENTSTATE;
	}

	// Check the control flags
	uint16 nCtrlFlags = (uint16)m_Vars.m_dwCtrl;
	if(nCtrlFlags != m_CUCtrlFlags)
	{
		m_CUCtrlFlags = nCtrlFlags;
		nUpdateFlags |= CLIENTUPDATE_CTRLFLAGS;
	}


	// Return the final flag value
	return nUpdateFlags;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::UpdateClientToServer()
//
// PURPOSE:		Send updated information to the server
//
//				NOTE!! These must be in the same order as the
//				CPlayerObj::HandleMessage_UpdateClient function on the server
//
// ----------------------------------------------------------------------- //

void PlayerMovement::UpdateClientToServer()
{
	// Make sure the object that we're going to get update information from is valid
	if(!GetObject()) return;

	// Get our current game time
	LTFLOAT fCurTime	 = g_pLTClient->GetTime();


	// Setup some time variables to see if we should send this data during a multiplayer game
	LTFLOAT fSendRate  = GetSendRateFromState(GetMovementState());
	LTFLOAT fSendDelta = (fCurTime - m_fClientUpdateTime);


	// If we're a single player game, or the delta has passed it's update time... send the data
	if(!g_pGameClientShell->IsMultiplayerGame() || (fSendDelta > fSendRate))
	{
		m_nClientUpdateFlags |= GetClientUpdateFlags();


		// Only send information if there are flags set
		if(m_nClientUpdateFlags)
		{
			// Start the player update message
			HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_UPDATE);
			g_pLTClient->WriteToMessageWord(hMessage, m_nClientUpdateFlags);

			WriteMessage_UpdateScale(hMessage);
			WriteMessage_UpdateDims(hMessage);
			WriteMessage_UpdateVelocity(hMessage);
			WriteMessage_UpdatePosition(hMessage);
			WriteMessage_UpdateRotation(hMessage);
			WriteMessage_UpdateAiming(hMessage);
			WriteMessage_UpdateAllowedInput(hMessage);
			WriteMessage_UpdateFlags(hMessage);
			WriteMessage_UpdateFlags2(hMessage);
			WriteMessage_UpdateMovementState(hMessage);
			WriteMessage_UpdateCtrlFlags(hMessage);

			g_pLTClient->EndMessage2(hMessage, 0);
		}


		// Reset our send information
		m_fClientUpdateTime = fCurTime;
		m_nClientUpdateFlags  = 0;
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::UpdateSpeedMods()
//
// PURPOSE:		Update the speed modification values
//
// ----------------------------------------------------------------------- //

void PlayerMovement::UpdateSpeedMods()
{
	if(g_pGameClientShell->GetGameType()->IsMultiplayer())
	{
		LTBOOL bIsExosuit = IsExosuit(m_Butes);
		m_Vars.m_fWalkSpeedMod = bIsExosuit?0.3f:1.0f;
		m_Vars.m_fRunSpeedMod = bIsExosuit?0.5f:1.0f;
		m_Vars.m_fCrouchSpeedMod = bIsExosuit?0.3f:1.0f;
	}
	else
	{
		m_Vars.m_fWalkSpeedMod = 1.0f;
		m_Vars.m_fRunSpeedMod = 1.0f;
		m_Vars.m_fCrouchSpeedMod = 1.0f;
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::CanPounce()
//
// PURPOSE:		Can we pounce from the current state???
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerMovement::CanPounce()
{
	if(CanMove())
	{
		switch(m_cmsCurrent)
		{
		case CMS_IDLE:
		case CMS_WALKING:
		case CMS_RUNNING:
		case CMS_STAND_LAND:						
		case CMS_WALK_LAND:						
		case CMS_RUN_LAND:						
		case CMS_WWALK_LAND:						
		case CMS_PRE_CRAWLING:					
		case CMS_CRAWLING:						
		case CMS_POST_CRAWLING:					
		case CMS_WALLWALKING:					
		case CMS_CLIMBING:						
				return LTTRUE;
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::CheckCharacterOverlaps()
//
// PURPOSE:		See if there are other characters overlapping us... and if
//				so, set them to non-solid.
//
// ----------------------------------------------------------------------- //

void PlayerMovement::CheckCharacterOverlaps()
{
	// Clear out the list of changed characters...
	memset(g_pCharacterOverlapData, 0, 32 * sizeof(HLOCALOBJ));
	int nNumOverlapped = 0;


	// Find the list of FX related to characters
	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);

	if(!pList || pList->IsEmpty())
		return;


	// Get the information on our player...
	LTVector vPos, vDims;
	GetPos(vPos);
	vDims = GetObjectDims();

	// Setup the min and max values...
	LTVector vMin = vPos - vDims;
	LTVector vMax = vPos + vDims;


	LTBOOL bOverlap;
	HLOCALOBJ hObj;
	uint32 nFlags;

	LTVector vTestPos;
	LTVector vTestDims;
	LTVector vTestMin;
	LTVector vTestMax;


	int nNumCharFX = pList->GetNumItems();

	// Now go through all the character FX and find our test object...
	for( CSpecialFXList::Iterator iter = pList->Begin();
		 iter != pList->End(); ++iter)
	{
		CCharacterFX *pFX = (CCharacterFX*)(iter->pSFX);

		if(pFX)
		{
			// Get the object test information
			hObj = pFX->GetServerObj();

			g_pLTClient->GetObjectPos(hObj, &vTestPos);
			g_pLTClient->Physics()->GetObjectDims(hObj, &vTestDims);

			vTestMin = vTestPos - vTestDims;
			vTestMax = vTestPos + vTestDims;

			// See if these boxes overlap
			bOverlap = LTTRUE;

			if((vMax.x <= vTestMin.x) || (vMin.x >= vTestMax.x))
				bOverlap = LTFALSE;
			else if((vMax.y <= vTestMin.y) || (vMin.y >= vTestMax.y))
				bOverlap = LTFALSE;
			else if((vMax.z <= vTestMin.z) || (vMin.z >= vTestMax.z))
				bOverlap = LTFALSE;

			if(bOverlap)
			{
				// See if this guy is solid...
				g_pLTClient->Common()->GetObjectFlags(hObj, OFT_Flags, nFlags);

				if(nFlags & FLAG_SOLID)
				{
					g_pLTClient->Common()->SetObjectFlags(hObj, OFT_Flags, nFlags & ~FLAG_SOLID);
					g_pCharacterOverlapData[nNumOverlapped++] = hObj;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::ResetCharacterOverlaps()
//
// PURPOSE:		If we set a particular character to non-solid, then reset
//				them to the way they were.
//
// ----------------------------------------------------------------------- //

void PlayerMovement::ResetCharacterOverlaps()
{
	uint32 nFlags;

	// Go through all the potential overlapped characters
	for(int i = 0; i < 32; i++)
	{
		// Boot us out if there aren't any more overlapping...
		if(!g_pCharacterOverlapData[i])
			return;

		// Otherwise... turn this character solid again.
		g_pLTClient->Common()->GetObjectFlags(g_pCharacterOverlapData[i], OFT_Flags, nFlags);
		g_pLTClient->Common()->SetObjectFlags(g_pCharacterOverlapData[i], OFT_Flags, nFlags | FLAG_SOLID);
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::SetMPButes()
//
// PURPOSE:		Handle setting up our new butes....
//
// ----------------------------------------------------------------------- //

void PlayerMovement::SetMPButes(uint8 nCharacter)
{
	// If the object isn't valid yet... then don't try to handle this message
	if(!m_Vars.m_hObject || !m_pInterface) return;

	// ----------------------------------------------------------------------- //
	// Set the client up to deal with this character type

	// Set the butes to this new set
	SetCharacterButes(nCharacter);

	// Set the new default camera FOV
	CPoint ptFOV = GetCharacterButes()->m_ptFOV;
	g_pGameClientShell->GetCameraMgr()->SetCameraFOV(g_pGameClientShell->GetPlayerCamera(), (DFLOAT)ptFOV.x, (DFLOAT)ptFOV.y);
	g_pGameClientShell->GetCameraMgr()->SetCameraFOV(g_pGameClientShell->Get3rdPersonCamera(), (DFLOAT)ptFOV.x, (DFLOAT)ptFOV.y);

	// Setup the player stats
	g_pGameClientShell->GetPlayerStats()->SetCharacterButes(nCharacter);

	// Force new hud
	g_pInterfaceMgr->GetHudMgr()->Init(0,LTFALSE);

	// Reset the shared surfaces
	g_pInterfaceMgr->ResetSharedSurfaces();

	// Init the vision mode mgr
	g_pGameClientShell->GetVisionModeMgr()->Init();

	// Reset the mouse sensitivity
	g_pGameClientShell->SetMouseSensitivityScale();

	// Setup the head tilt effect
	CameraMgrFX_HeadTiltCreate(g_pGameClientShell->GetPlayerCamera(), GetCharacterButes()->m_fHeadTiltAngle, GetCharacterButes()->m_fHeadTiltSpeed);
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::SetMPTeleport()
//
// PURPOSE:		Handle a teleport message...
//
// ----------------------------------------------------------------------- //

void PlayerMovement::SetMPTeleport(LTVector vPos, uint8 nMoveCode)
{
	// If the object isn't valid yet... then don't try to handle this message
	if(!m_Vars.m_hObject || !m_pInterface) return;

	// Make sure the clientside object of the server player is non-solid.  Otherwise
	// we'll collide with it.
	HOBJECT hClientObj = m_pInterface->GetClientObject();
	if( hClientObj )
	{
		uint32 dwFlags = m_pInterface->GetObjectFlags(hClientObj);
		m_pInterface->SetObjectFlags(hClientObj, dwFlags & ~FLAG_SOLID);
	}

	// Read in the position and increment value data
	m_bMoveCode = nMoveCode;

	// Set the dims very small for a minute so that when we move it, we can resize it from smaller dims
	LTVector vOldDims = GetObjectDims();
	SetObjectDims(CM_DIMS_VERY_SMALL);

	// Set the flags up so the object won't get any collision responses
	uint32 nFlags = GetObjectFlags(OFT_Flags);
	SetObjectFlags(OFT_Flags, nFlags | FLAG_GOTHRUWORLD, DFALSE);

	// Teleport the object to the new location
	SetPos(vPos, MOVEOBJECT_TELEPORT);

	// Reset the flags to their previous values
	SetObjectFlags(OFT_Flags, nFlags, DFALSE);

	// Reset the dims so we can see if we really fit in the area we teleported to
	SetObjectDims(vOldDims);

	// Make sure we're not moving at all
	LTVector vZero(0.0f, 0.0f, 0.0f);
	m_pPhysics->SetAcceleration(m_Vars.m_hObject, &vZero);
	m_pPhysics->SetVelocity(m_Vars.m_hObject, &vZero);


	// Set the camera to this position so it doesn't interpolate from god knows where
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	g_pGameClientShell->GetCameraMgr()->SetCameraPos(hCamera, vPos);

	return;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	PlayerMovement::SetMPOrientation()
//
// PURPOSE:		Set the player's orientation
//
// ----------------------------------------------------------------------- //

void PlayerMovement::SetMPOrientation(LTVector vVec)
{
	// Set our rotation...
	SetRot(vVec);

	// Set the camera to this rotation so it doesn't interpolate from some odd angle
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();
	LTRotation rRot = GetRot();
	g_pGameClientShell->GetCameraMgr()->SetCameraRotation(hCamera, rRot);
}
