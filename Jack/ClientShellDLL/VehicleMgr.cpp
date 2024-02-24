// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleMgr.cpp
//
// PURPOSE : Client side vehicle mgr - Implementation
//
// CREATED : 6/12/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VehicleMgr.h"
#include "VarTrack.h"
#include "MsgIDs.h"
#include "CharacterFX.h"
#include "CMoveMgr.h"
#include "PlayerMgr.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientSoundMgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "LTEulerAngles.h"
#include "JumpVolumeFX.h"
#include "PlayerLureFX.h"
#include "FXButeMgr.h"
#include "SurfaceFunctions.h"
#include "PlayerViewAttachmentMgr.h"
#include "VehicleButeMgr.h"

#define ROUNDFLOAT(f) ((int)((f) + ((f) > 0 ? 0.5 : -0.5)))

#define VEHICLE_SLIDE_TO_STOP_TIME	5.0f

#define VEHICLE_GEAR_0					0
#define VEHICLE_GEAR_1					1
#define VEHICLE_GEAR_2					2

extern CMoveMgr* g_pMoveMgr;
extern VarTrack g_vtMaxVehicleHeadCant;

// [KLS 6/29/02] - Added more vehicle debugging info...
#define		VEHICLE_TRACK_GENERAL	1.0f
#define		VEHICLE_TRACK_ANIMATION	2.0f
#define		VEHICLE_TRACK_VERBOSE	3.0f


// Vehicle tweaking vars...

static VarTrack	s_vtVehicleInfoTrack;
static VarTrack	s_vtVehicleVel;
static VarTrack	s_vtVehicleAccel;
static VarTrack	s_vtVehicleAccelTime;
static VarTrack	s_vtVehicleMaxDecel;
static VarTrack	s_vtVehicleDecelTime;
static VarTrack	s_vtVehicleMaxBrake;
static VarTrack	s_vtVehicleBrakeTime;
static VarTrack	s_vtVehicleStoppedPercent;
static VarTrack	s_vtVehicleMaxSpeedPercent;
static VarTrack	s_vtVehicleAccelGearChange;
static VarTrack	s_vtVehicleMinTurnAccel;
static VarTrack	s_vtVehicleTurnRate;
static VarTrack	s_vtVehicleTurnRateScale;
static VarTrack	s_vtVehicleInitialTurnRate;
static VarTrack	s_vtVehicleSurfaceSpeedUpPitch;
static VarTrack s_vtVehicleSurfaceSlowDownPitch;
static VarTrack	s_vtVehicleStartMotionPitch;
static VarTrack s_vtVehicleStopMotionPitch;
static VarTrack s_vtVehicleBrakePitch;
static VarTrack s_vtVehicleDecelPitch;


// Bicycle...

VarTrack	g_vtBicycleInterBumpTimeRange;
VarTrack	g_vtBicycleIntraBumpTimeRange;
VarTrack	g_vtBicycleBumpHeightRange;
VarTrack	g_vtBicycleBobRate;
VarTrack	g_vtBicycleBobSwayAmplitude;
VarTrack	g_vtBicycleBobRollAmplitude;
VarTrack	g_vtBicycleModelOffset;


VarTrack	g_vtVehicleImpactDamage;
VarTrack	g_vtVehicleImpactDamageMin;
VarTrack	g_vtVehicleImpactDamageMax;
VarTrack	g_vtVehicleImpactDamageVelPercent;
VarTrack	g_vtVehicleImpactPushVelPercent;
VarTrack	g_vtVehicleImpactMinAngle;
VarTrack	g_vtVehicleImpactMaxAdjustAngle;
VarTrack	g_vtVehicleImpactPushAmount;
VarTrack	g_vtVehicleImpactPushRadius;
VarTrack	g_vtVehicleImpactPushDuration;
VarTrack	g_vtVehicleImpactFXOffsetForward;
VarTrack	g_vtVehicleImpactFXOffsetUp;
VarTrack	g_vtVehicleImpactThreshold;

VarTrack	g_vtVehicleImpactAIDamageVelPercent;
VarTrack	g_vtVehicleImpactAIMinDamage;
VarTrack	g_vtVehicleImpactAIMaxDamage;
VarTrack	g_vtVehicleImpactAIDoNormalCollision;

VarTrack	g_vtVehicleFallDamageMinHeight;
VarTrack	g_vtVehicleFallDamageMaxHeight;

VarTrack	g_vtVehicleMouseMinYawBuffer;
VarTrack	g_vtVehicleMouseMaxYawBuffer;

VarTrack	g_vtVehicleMaxHeadOffsetX;
VarTrack	g_vtVehicleMaxHeadOffsetY;
VarTrack	g_vtVehicleMaxHeadOffsetZ;

VarTrack	g_vtVehicleMaxHeadYaw;

VarTrack	g_vtVehicleSlideToStopTime;

VarTrack	g_vtVehicleCollisionInfo;

VarTrack	g_vtJumpVolumeWaveType;
VarTrack	g_vtJumpVolumeMaxDirectionPercent;	// Give 100% if at or above this max
VarTrack	g_vtJumpVolumeMinDirectionPercent;	// Give 0% if at or below this min
VarTrack	g_vtJumpVolumeInfo;

VarTrack	g_vtVehicleContourPlayerViewModel;
VarTrack	g_vtVehicleContourPoints;
VarTrack	g_vtVehicleContourExtraDimsX;
VarTrack	g_vtVehicleContourExtraDimsZ;
VarTrack	g_vtVehicleCamContourExtraDimsX;
VarTrack	g_vtVehicleCamContourExtraDimsZ;
VarTrack	g_vtVehicleContourMaxRotation;
VarTrack	g_vtVehicleCamContourMaxRotation;
VarTrack	g_vtVehicleContourRate;
VarTrack	g_vtVehicleContour;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelNodeControlFn
//
//	PURPOSE:	Node control callback
//
// ----------------------------------------------------------------------- //

void ModelNodeControlFn(const NodeControlData& Data, void *pUserData)
{
	CVehicleMgr* pMgr = (CVehicleMgr*)pUserData;
	_ASSERT(pMgr);
	if (!pMgr) return;

	pMgr->HandleNodeControl(Data.m_hModel, Data.m_hNode, Data.m_pNodeTransform);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CVehicleMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVehicleMgr::CVehicleMgr()
{
	m_hVehicleModel			= LTNULL;
	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

    m_hVehicleStartSnd  = LTNULL;
    m_hVehicleAccelSnd  = LTNULL;
    m_hVehicleDecelSnd  = LTNULL;
	m_hVehicleImpactSnd = LTNULL;
	m_hIdleSnd			= LTNULL;
	m_hBrakeSnd			= LTNULL;

	m_vVehicleOffset.Init();
	m_vHeadOffset.Init();
	m_fHeadYaw = 0.0f;

    m_bVehicleTurning       = LTFALSE;
	m_nVehicleTurnDirection	= 0;

    m_bVehicleStopped    = LTTRUE;
    m_bVehicleAtMaxSpeed = LTFALSE;

	m_fHandlebarRoll = 0.0f;
	m_fContourRoll = 0.0f;
	m_bTurned = false;
	m_bHolsteredWeapon = false;

	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();
	m_vLastVehiclePYR.Init();
	m_vVehiclePYR.Init();

	m_pVehicle = LTNULL;
	m_bHasWeapon = false;

	InitWorldData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::~CVehicleMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CVehicleMgr::~CVehicleMgr()
{
	TermLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::Init
//
//	PURPOSE:	Initialize mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::Init()
{
    // Vehicle tweaking...

	s_vtVehicleInfoTrack.Init( g_pLTClient, "VehicleInfo", NULL, 0.0f );
    s_vtVehicleVel.Init( g_pLTClient, "VehicleVel", NULL, 0.0f );
	s_vtVehicleAccel.Init( g_pLTClient, "VehicleAccel", NULL, 0.0f );
	s_vtVehicleAccelTime.Init( g_pLTClient, "VehicleAccelTime", NULL, 0.0f );
	s_vtVehicleMaxDecel.Init( g_pLTClient, "VehicleDecelMax", NULL, 0.0f );
    s_vtVehicleDecelTime.Init( g_pLTClient, "VehicleDecelTime", NULL, 0.0f );
	s_vtVehicleMaxBrake.Init( g_pLTClient, "VehicleBrakeMax", NULL, 0.0 );
    s_vtVehicleBrakeTime.Init( g_pLTClient, "VehicleBrakeTime", NULL, 0.0 );
    s_vtVehicleMinTurnAccel.Init( g_pLTClient, "VehicleMinTurnAccel", NULL, 0.0f );
	s_vtVehicleStoppedPercent.Init( g_pLTClient, "VehicleStoppedPercent", NULL, 0.0 );
	s_vtVehicleMaxSpeedPercent.Init( g_pLTClient, "VehicleMaxSpeedPercent", NULL, 0.0 );
    s_vtVehicleAccelGearChange.Init( g_pLTClient, "VehicleAccelGearChange", NULL, 1.0 );
	s_vtVehicleTurnRate.Init( g_pLTClient, "VehicleTurnRate", NULL, 0.0f );
	s_vtVehicleTurnRateScale.Init( g_pLTClient, "VehicleTurnRateScale", NULL, 0.0f );
	s_vtVehicleInitialTurnRate.Init( g_pLTClient, "VehicleInitialTurnRate", NULL, 0.0f );
	s_vtVehicleSurfaceSpeedUpPitch.Init( g_pLTClient, "VehicleSurfaceSpeedUpPitch", NULL, 0.0f );
	s_vtVehicleSurfaceSlowDownPitch.Init( g_pLTClient, "VehicleSurfaceSlowDownPitch", NULL, 0.0f);
	s_vtVehicleStartMotionPitch.Init( g_pLTClient, "VehicleStartMotionPitch", NULL, 0.0f );
	s_vtVehicleStopMotionPitch.Init( g_pLTClient, "VehicleStopMotionPitch", NULL, 0.0f);
	s_vtVehicleBrakePitch.Init( g_pLTClient, "VehicleBrakePitch", NULL, 0.0f );
	s_vtVehicleDecelPitch.Init( g_pLTClient, "VehicleDecelPitch", NULL, 0.0f );
	
	// Bicycle...

	g_vtBicycleInterBumpTimeRange.Init(g_pLTClient, "BicycleInterBumpTimeRange", "0.75 1.25", 0.0f );
	g_vtBicycleIntraBumpTimeRange.Init(g_pLTClient, "BicycleIntraBumpTimeRange", "0.05 0.15", 0.0f );
	g_vtBicycleBumpHeightRange.Init(g_pLTClient, "BicycleBumpHeightRange", "0 0", 0.0f );
	g_vtBicycleBobRate.Init(g_pLTClient, "BicycleBobRate", NULL, 2.5f );
	g_vtBicycleBobSwayAmplitude.Init(g_pLTClient, "BicycleBobSwayAmplitude", NULL, 2.0f );
	g_vtBicycleBobRollAmplitude.Init(g_pLTClient, "BicycleBobRollAmplitude", NULL, -0.25f );
	g_vtBicycleModelOffset.Init(g_pLTClient, "BicycleModelOffset", "0.0 -20.0 5.0", 0.0f );


	g_vtVehicleImpactDamage.Init(g_pLTClient, "VehicleImpactDamage", LTNULL, 0.0f);
	g_vtVehicleImpactDamageMin.Init(g_pLTClient, "VehicleImpactDamageMin", LTNULL, 5.0f);
	g_vtVehicleImpactDamageMax.Init(g_pLTClient, "VehicleImpactDamageMax", LTNULL, 100.0f);
	g_vtVehicleImpactDamageVelPercent.Init(g_pLTClient, "VehicleImpactDamageVelPerent", LTNULL, 0.9f);
	g_vtVehicleImpactPushVelPercent.Init(g_pLTClient, "VehicleImpactPushVelPerent", LTNULL, 0.0f);
	g_vtVehicleImpactMinAngle.Init(g_pLTClient, "VehicleImpactMinAngle", LTNULL, 20.0f);
	g_vtVehicleImpactMaxAdjustAngle.Init(g_pLTClient, "VehicleImpactMinAdjustAngle", LTNULL, 10.0f);
	g_vtVehicleImpactPushAmount.Init(g_pLTClient, "VehicleImpactPushAmount", LTNULL, 400.0f);
	g_vtVehicleImpactPushRadius.Init(g_pLTClient, "VehicleImpactPushRadius", LTNULL, 100.0f);
	g_vtVehicleImpactPushDuration.Init(g_pLTClient, "VehicleImpactPushDuration", LTNULL, 0.25f);

	g_vtVehicleImpactFXOffsetForward.Init(g_pLTClient, "VehicleImpactFXOffsetForward", LTNULL, 0.0f);
	g_vtVehicleImpactFXOffsetUp.Init(g_pLTClient, "VehicleImpactFXOffsetUp", LTNULL, 0.0f);

	g_vtVehicleImpactThreshold.Init(g_pLTClient, "VehicleImpactThreshold", LTNULL, 0.8f);

	g_vtVehicleImpactAIDamageVelPercent.Init(g_pLTClient, "VehicleImpactAIDamageVelPerent", LTNULL, 0.1f);
 	g_vtVehicleImpactAIMinDamage.Init(g_pLTClient, "VehicleImpactAIMinDamage", LTNULL, 1.0f);
 	g_vtVehicleImpactAIMaxDamage.Init(g_pLTClient, "VehicleImpactAIMaxDamage", LTNULL, 100.0f);
 	g_vtVehicleImpactAIDoNormalCollision.Init(g_pLTClient, "VehicleImpactAIDoNormalCollision", LTNULL, 1.0f);

	g_vtVehicleFallDamageMinHeight.Init(g_pLTClient, "VehicleFallDamageMinHeight", LTNULL, 400.0f);
 	g_vtVehicleFallDamageMaxHeight.Init(g_pLTClient, "VehicleFallDamageMaxHeight", LTNULL, 800.0f);

    
	g_vtVehicleMouseMinYawBuffer.Init(g_pLTClient, "VehicleMouseMinYawBuffer", NULL, 2.0f);
	g_vtVehicleMouseMaxYawBuffer.Init(g_pLTClient, "VehicleMouseMaxYawBuffer", NULL, 35.0f);

	g_vtVehicleMaxHeadOffsetX.Init(g_pLTClient, "VehicleHeadMaxOffsetX", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetY.Init(g_pLTClient, "VehicleHeadMaxOffsetY", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetZ.Init(g_pLTClient, "VehicleHeadMaxOffsetZ", NULL, 0.0f);
	g_vtVehicleMaxHeadYaw.Init(g_pLTClient, "VehicleHeadMaxYaw", NULL, -1.0f);

	g_vtVehicleSlideToStopTime.Init( g_pLTClient, "VehicleSlideToStopTime", NULL, 5.0f );

    g_vtVehicleCollisionInfo.Init(g_pLTClient, "VehicleCollisionInfo", LTNULL, 0.0f);

	g_vtJumpVolumeWaveType.Init( g_pLTClient, "JumpVolumeWaveType", LTNULL, 3.0f );
	g_vtJumpVolumeMaxDirectionPercent.Init( g_pLTClient, "JumpVolumeMaxDirectionPercent", LTNULL, 0.97f );
	g_vtJumpVolumeMinDirectionPercent.Init( g_pLTClient, "JumpVolumeMinDirectionPercent", LTNULL, 0.35f );
	g_vtJumpVolumeInfo.Init( g_pLTClient, "JumpVolumeInfo", LTNULL, 0.0f );

	g_vtVehicleContourPlayerViewModel.Init( g_pLTClient, "VehicleContourPlayerViewModel", LTNULL, 1.0f );
	g_vtVehicleContourPoints.Init( g_pLTClient, "VehicleContourPoints", LTNULL, 2.0f );
	g_vtVehicleContourExtraDimsX.Init( g_pLTClient, "VehicleContourExtraDimsX", LTNULL, 24.0f );
	g_vtVehicleContourExtraDimsZ.Init( g_pLTClient, "VehicleContourExtraDimsZ", LTNULL, 32.0f );
	g_vtVehicleCamContourExtraDimsX.Init( g_pLTClient, "VehicleCamCountourExtraDimsX", LTNULL, 48.0f );
	g_vtVehicleCamContourExtraDimsZ.Init( g_pLTClient, "VehicleCamCountourExtraDimsZ", LTNULL, 64.0f );
	g_vtVehicleContourMaxRotation.Init( g_pLTClient, "VehicleContourMaxRotation", LTNULL, 65.0f );
	g_vtVehicleCamContourMaxRotation.Init( g_pLTClient, "VehicleCamContourMaxRotation", LTNULL, 3.5f );
	g_vtVehicleContourRate.Init( g_pLTClient, "VehicleContourRate", LTNULL, 0.5f );
	g_vtVehicleContour.Init( g_pLTClient, "VehicleContour", LTNULL, 1.0f );

	m_VehicleModelOffsetMgr.Init();

	// Init world specific data members...

	InitWorldData();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::InitWorldData
//
//	PURPOSE:	Initialize our data members that are specific to the
//				current world.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::InitWorldData()
{
	m_dwControlFlags		= 0;
	m_dwLastControlFlags	= 0;
	m_bLastBrake = false;

	m_fVehicleBaseMoveAccel = 0.0f;

	m_bVehicleStopped       = LTTRUE;
    m_bVehicleAtMaxSpeed    = LTFALSE;
    m_bVehicleTurning       = LTFALSE;
	m_nVehicleTurnDirection	= 0;

	m_ePPhysicsModel		= PPM_NORMAL;

	m_nCurGear				= VEHICLE_GEAR_0;
	m_nLastGear				= VEHICLE_GEAR_0;

	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

	m_bSetLastAngles		= false;

	m_vVehiclePYR.Init();
	m_vLastVehiclePYR.Init();
	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();

	m_eSurface				= ST_UNKNOWN;
	m_eLastSurface			= ST_UNKNOWN;

	m_fYawDiff				= 0.0f;
	m_fYawDelta				= 0.0f;
	m_eMouseTurnDirection	= TD_CENTER;
	m_bKeyboardTurning		= LTFALSE;

	m_vHeadOffset.Init();
	m_fHandlebarRoll		= 0.0f;
	m_fHeadYaw				= 0.0f;

	m_nPlayerLureId			= 0;
	m_bResetLure			= false;
	m_bResetLureFromSave	= false;
	m_hCurJumpVolume		= NULL;

	m_bHaveJumpVolumeVel	= LTFALSE;
	m_fJumpVolumeVel		= 0.0f;
	m_bTurned				= false;
	m_bHolsteredWeapon		= false;

	m_pVehicle				= LTNULL;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnEnterWorld
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnEnterWorld()
{
	InitWorldData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnExitWorld
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnExitWorld()
{
	// Turn off vehicle...

	SetNormalPhysicsModel();

	// Clean up any level specific shiznit...

	TermLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::TermLevel
//
//	PURPOSE:	Terminate any level specific stuff
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::TermLevel()
{
	if (m_hVehicleModel)
	{
        g_pLTClient->RemoveObject(m_hVehicleModel);
		m_hVehicleModel = LTNULL;
	}

	KillAllVehicleSounds();

	if (m_hVehicleImpactSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleImpactSnd);
        m_hVehicleImpactSnd = LTNULL;
	}

	m_hVehicleAttachSocket = INVALID_MODEL_SOCKET;

	// Remove any player-view attachments...

	if (g_pPVAttachmentMgr)
		g_pPVAttachmentMgr->RemovePVAttachments();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateControlFlags
//
//	PURPOSE:	Update the control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateControlFlags()
{
	// Clear control flags...

	m_dwLastControlFlags = m_dwControlFlags;
	m_dwControlFlags = 0;

	if (!g_pMoveMgr->GetObject() ||
		!g_pInterfaceMgr->AllowCameraMovement()) return;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			UpdateVehicleControlFlags();
		break;

		case PPM_LURE :
			UpdateLureControlFlags();
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleControlFlags
//
//	PURPOSE:	Update the vehicle physics model control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleControlFlags()
{
	// Determine what commands are currently on...

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RUN) ^ (g_pMoveMgr->RunLock() != LTFALSE))
	{
		m_dwControlFlags |= BC_CFLG_RUN;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))
	{
		m_dwControlFlags |= BC_CFLG_JUMP;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_REVERSE))
	{
		m_dwControlFlags |= BC_CFLG_REVERSE;
		m_dwControlFlags &= ~BC_CFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_FIRING;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_ALT_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_ALT_FIRING;
	}

	// Check for strafe left and strafe right.
	if ((m_dwControlFlags & BC_CFLG_RIGHT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if ((m_dwControlFlags & BC_CFLG_LEFT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if ( (m_dwControlFlags & BC_CFLG_FORWARD) ||
		 (m_dwControlFlags & BC_CFLG_REVERSE) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_LEFT) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) ||
		 (m_dwControlFlags & BC_CFLG_JUMP))
	{
		m_dwControlFlags |= BC_CFLG_MOVING;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateLureControlFlags
//
//	PURPOSE:	Update the Lure control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateLureControlFlags()
{
	// Determine what commands are currently on...

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_FIRING;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_ALT_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_ALT_FIRING;
	}

	// Get the playerlurefx object.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		// It may not be on the client yet.  Keep polling for it.
		return;
	}

	// See if we need to reset the lure info.
	if( m_bResetLure )
	{
		if( m_bResetLureFromSave )
		{
			pPlayerLureFX->Reset( m_ResetLureTransform );
		}
		else
		{
			// Reset the lure.
			pPlayerLureFX->Reset( );
		}

		// Reset the lure shake timer.
		m_LureTimeToNextBump.Stop( );

		// Make sure the weapon is set correctly.  Need to keep calling this since
		// we poll for the playerlurefx object.
		EnableWeapon( pPlayerLureFX->GetAllowWeapon( ), true);

		// Don't need to reset anymore.
		m_bResetLure = false;
	}
	else
	{
		// Make sure the weapon is set correctly.  Need to keep calling this since
		// we poll for the playerlurefx object.  Also, since the enable state
		// isn't counted up and down, something else, like a cinematic camera
		// could have enabled weapons behind our back.
		EnableWeapon( pPlayerLureFX->GetAllowWeapon( ), false);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateMotion
//
//	PURPOSE:	Update our motion
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateMotion()
{
	g_pMoveMgr->UpdateOnGround();

	m_eLastSurface = m_eSurface;
	m_eSurface	   = g_pMoveMgr->GetStandingOnSurface();

	// See if we moved over a new surface...

	if (m_eLastSurface != m_eSurface)
	{
		LTFLOAT fLastMult = GetSurfaceVelMult(m_eLastSurface);
		LTFLOAT fNewMult = GetSurfaceVelMult(m_eSurface);

		if (fLastMult < fNewMult)
		{
			// Speeding up...

			CameraDelta delta;
			delta.Pitch.fVar	= DEG2RAD( s_vtVehicleSurfaceSpeedUpPitch.GetFloat( ));
			delta.Pitch.fTime1	= 0.25f;
			delta.Pitch.fTime2	= 0.5;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;
			m_VehicleModelOffsetMgr.AddDelta(delta);
		}
		else if (fNewMult < fLastMult)
		{
			// Slowing down...

			CameraDelta delta;
			delta.Pitch.fVar	= DEG2RAD( s_vtVehicleSurfaceSlowDownPitch.GetFloat( ));
			delta.Pitch.fTime1	= 0.25f;
			delta.Pitch.fTime2	= 0.5;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;
			m_VehicleModelOffsetMgr.AddDelta(delta);
		}
	}

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			UpdateVehicleMotion();
		break;

		case PPM_LURE :
			UpdateLureMotion( );
			break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateLureMotion
//
//	PURPOSE:	Update our lure motion
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateLureMotion()
{
	HOBJECT hObj = g_pMoveMgr->GetObject();
	if( !hObj )
	{
		ASSERT( !"CVehicleMgr::UpdateLureMotion:  Invalid client object." );
		return;
	}

	LTVector vZero( 0.0f, 0.0f, 0.0f );

	// Don't allow any acceleration or velocity.
	g_pPhysicsLT->SetAcceleration( g_pMoveMgr->GetObject(), &vZero );
	g_pPhysicsLT->SetVelocity( g_pMoveMgr->GetObject(), &vZero );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateGear
//
//	PURPOSE:	Update the appropriate vehicle gear
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateGear()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			UpdateVehicleGear();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSound
//
//	PURPOSE:	Update our sounds
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSound()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			UpdateVehicleSounds();
		break;

		case PPM_LURE :
		case PPM_NORMAL:
		default :
		break;
	}

	// See if the impact snd is done playing...

	if (m_hVehicleImpactSnd)
	{
        if (g_pLTClient->IsDone(m_hVehicleImpactSnd))
		{
            g_pLTClient->SoundMgr()->KillSound(m_hVehicleImpactSnd);
            m_hVehicleImpactSnd = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleMotion
//
//	PURPOSE:	Update our motion when on a vehicle
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleMotion()
{
	HOBJECT hObj = g_pMoveMgr->GetObject();

    LTFLOAT fTime = g_pLTClient->GetTime();

	LTBOOL bHeadInLiquid = g_pMoveMgr->IsHeadInLiquid();
    LTBOOL bInLiquid     = (bHeadInLiquid || g_pMoveMgr->IsBodyInLiquid());
    LTBOOL bFreeMovement = (g_pMoveMgr->IsFreeMovement() ||
		g_pMoveMgr->IsBodyOnLadder() || g_pPlayerMgr->IsSpectatorMode());


	// Normally we have gravity on, but the containers might turn it off.

	g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, g_pPlayerMgr->IsSpectatorMode() ? 0 : FLAG_GRAVITY, FLAG_GRAVITY);

	
	// Test to see if we are still within a JumpVolume...
	
	UpdateInJumpVolume();


	// Can't ride vehicles underwater, in spectator mode, or when dead...

	if (bHeadInLiquid || g_pPlayerMgr->IsSpectatorMode() || g_pPlayerMgr->IsPlayerDead())
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
		cMsg.Writeuint8( CP_PHYSICSMODEL );
		cMsg.Writeuint8( PPM_NORMAL	);
		cMsg.Writeuint8( VBMGR_INVALID_ID );
	    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		// The server won't notify us, so change models...

		SetPhysicsModel(PPM_NORMAL);
		return;
	}

	// Zero out the acceleration to start with...

    LTVector zeroVec;
	zeroVec.Init();
	g_pPhysicsLT->SetAcceleration(hObj, &zeroVec);


	// Set our current rotation...

    LTRotation rRot;
	g_pPlayerMgr->GetPlayerRotation(rRot);
	g_pLTClient->SetObjectRotation(hObj, &rRot);

	// Update our motion based on containers we are in...

	UpdateContainerMotion();

    LTVector myVel = g_pMoveMgr->GetVelocity();
	
    LTVector vAccel(0, 0, 0);
	g_pPhysicsLT->GetAcceleration( hObj, &vAccel );

    LTVector moveVel = myVel;
	moveVel.y = 0;

	g_pLTClient->GetObjectRotation(hObj, &rRot);
    LTVector vForward = rRot.Forward();

	// Check if we're moving in our forward direction.
	bool bMovingForward = ( vForward.Dot( myVel ) > 0.0f );

    bool bForward = !!( m_dwControlFlags & BC_CFLG_FORWARD );
	bool bReverse = !!( m_dwControlFlags & BC_CFLG_REVERSE );
	bool bLastForward = !!(m_dwLastControlFlags & BC_CFLG_FORWARD);
	bool bLastReverse = !!(m_dwLastControlFlags & BC_CFLG_REVERSE);

	// This case happens when we jump if the user stops pressing forward
	// and we're in mid air...don't slow down!
	if(( !bForward || !bReverse ) && !g_pMoveMgr->CanDoFootstep())
	{
		bForward = bMovingForward;
		bReverse = !bMovingForward;
	}

	// Check the brake.  Only allow braking if on ground.
	bool bBrake = false;
	if( g_pMoveMgr->CanDoFootstep( ) && !m_bVehicleStopped )
	{
		if( bMovingForward && bReverse )
			bBrake = true;
		else if( !bMovingForward && bForward )
			bBrake = true;
	}

	// Check if vehicle isn't allowed to go in reverse.
	if( !m_pVehicle->bHasReverse )
	{
		bReverse = false;
	}

	// Pitch vehicle if making change of acceleration.
	if( !m_bVehicleStopped )
	{
		// Check if they applied the brake.
		if( bBrake )
		{
			// Only do the brake pitching the first frame.
			if( !m_bLastBrake )
			{
				// Add a little camera movement to show the brake is applied...
				CameraDelta delta;
				// Make use pitch toward the direction we are moving.
				delta.Pitch.fVar	= (( bMovingForward ) ? 1.0f : -1.0f ) * DEG2RAD( s_vtVehicleBrakePitch.GetFloat( ));
				delta.Pitch.fTime1	= 0.4f;
				delta.Pitch.fTime2	= 0.5;
				delta.Pitch.eWave1	= Wave_SlowOff;
				delta.Pitch.eWave2	= Wave_SlowOff;
				m_VehicleModelOffsetMgr.AddDelta( delta );
			}
		}
		// Pitch us a little if they just let off the accelerator.
		else if( !bForward && !bReverse )
		{
			// Only pitch the first frame.  If we just did a brake, ignore this.
			if( !m_bLastBrake && ( bLastReverse || bLastForward ))
			{
				// Add a little camera movement to show the vehicle is slowing down...
				CameraDelta delta;
				// Make use pitch toward the direction we are moving.
				delta.Pitch.fVar	= (( bMovingForward ) ? 1.0f : -1.0f ) * DEG2RAD( s_vtVehicleDecelPitch.GetFloat( ));
				delta.Pitch.fTime1	= 0.25f;
				delta.Pitch.fTime2	= 0.5;
				delta.Pitch.eWave1	= Wave_SlowOff;
				delta.Pitch.eWave2	= Wave_SlowOff;
				m_VehicleModelOffsetMgr.AddDelta( delta );
			}
		}
	}

	// Determine the acceleration to use...

    LTFLOAT fMaxAccel  = GetMaxAccelMag(GetSurfaceVelMult(m_eSurface));
    LTFLOAT fMaxVel    = GetMaxVelMag(GetSurfaceVelMult(m_eSurface));

	// If we have a velocity from a JumpVolume use that as the max...
	
	if( m_bHaveJumpVolumeVel )
	{
		// [KLS 9/1/02] Changed how this calculations works...
		// See if it is time to use our normal velocity...(on the 
		// ground and not in a jump volume)...

		if (g_pMoveMgr->CanDoFootstep() && !m_hCurJumpVolume)
		{
			m_fJumpVolumeVel = 0.0f;
			m_bHaveJumpVolumeVel = LTFALSE;
		}
		else  // Use the jump volume vel...
		{
			fMaxVel = m_fJumpVolumeVel;
		}
	}

	// Update our acceleration time if we're accelerating.
	if( !bBrake && ( bForward || bReverse ))
	{
		LTFLOAT fAccelTime = GetAccelTime();

		if (m_fAccelStart < 0.0f)
		{
			m_fAccelStart = fTime - (m_fVehicleBaseMoveAccel * fAccelTime / fMaxAccel);
		}

		m_fVehicleBaseMoveAccel = fMaxAccel * (fTime - m_fAccelStart) / fAccelTime;
	}
	else
	{
		m_fAccelStart = -1.0f;
	}

	if (m_bVehicleTurning && !bBrake)
	{
		if (m_fVehicleBaseMoveAccel < GetMinTurnAccel())
		{
			m_fVehicleBaseMoveAccel = GetMinTurnAccel();
		}
	}

	m_fVehicleBaseMoveAccel = m_fVehicleBaseMoveAccel > fMaxAccel ? fMaxAccel : m_fVehicleBaseMoveAccel;


	// Limit velocity...
	if (g_pMoveMgr->IsOnGround() && !g_pPlayerMgr->IsSpectatorMode())
	{
		float fCurLen = moveVel.Mag();
		if (fCurLen > fMaxVel)
		{
			myVel *= (fMaxVel/fCurLen);

			g_pPhysicsLT->SetVelocity(hObj, &myVel);
		}
	}
	else if (moveVel.Mag() > fMaxVel)
	{
		// Don't cap velocity in the y direction...

		moveVel.Normalize();
		moveVel *= fMaxVel;

		myVel.x = moveVel.x;
		myVel.z = moveVel.z;

		g_pPhysicsLT->SetVelocity(hObj, &myVel);
	}


    LTFLOAT fMoveAccel = m_fVehicleBaseMoveAccel;

	if (!bInLiquid && !bFreeMovement)  // Can only accelerate in x and z directions
	{
		vForward.y = 0.0;
		vForward.Normalize();
	}


	// If we aren't dead we can drive around

	if (!g_pPlayerMgr->IsPlayerDead())
	{
		// Don't let us go backwards, slow us down...
		if (( !bForward && !bReverse ) || bBrake)
		{
			// Make our momentum continue in the same direction.
			vAccel += (( bMovingForward ) ? 1.0f : -1.0f ) * (vForward * fMoveAccel);
			LTVector vAccelDir = vAccel;
			LTFLOAT fAccelMag = vAccel.Mag();
			
			if( fAccelMag )
			{
				vAccelDir.Normalize();
			}
            

			// Determine the acceleration to use...

            LTFLOAT fMaxDecel  = GetMaxDecel();
            LTFLOAT fDecelTime = GetDecelTime();

			if (bBrake)
			{
				fMaxDecel  = GetMaxBrake();
				fDecelTime = GetBrakeTime();
			}

			if (m_fDecelStart < 0.0f)
			{
				m_fDecelStart = fTime;
			}

            LTFLOAT fAccelAdjust = fMaxDecel * (fTime - m_fDecelStart) / fDecelTime;
			fAccelAdjust = fAccelAdjust > fAccelMag ? fAccelMag : fAccelAdjust;

			vAccel -= vAccelDir * fAccelAdjust;

			if (vAccel.Mag() < 10.0f)
			{
				vAccel.Init();
			}

			m_fVehicleBaseMoveAccel = vAccel.Mag();
		}
		else
		{
			// Accelerate.
			vAccel += ( bReverse ? -1.0f : 1.0f ) * ( vForward * fMoveAccel );

			m_fDecelStart = -1.0f;
		}
	}

	// Cap the acceleration...

	if (vAccel.Mag() > fMoveAccel)
	{
		vAccel.Normalize();
		vAccel *= fMoveAccel;
	}

	g_pPhysicsLT->SetAcceleration(hObj, &vAccel);
	
	// Add any container currents to my velocity...

    LTVector vel = g_pMoveMgr->GetVelocity();
	vel += g_pMoveMgr->GetTotalCurrent();

	g_pPhysicsLT->SetVelocity(hObj, &vel);


	// If we're dead, we can't move...

	if (g_pPlayerMgr->IsPlayerDead() || !g_pPlayerMgr->IsPlayerMovementAllowed())
	{
		g_pPhysicsLT->SetAcceleration(hObj, &zeroVec);
		g_pPhysicsLT->SetVelocity(hObj, &zeroVec);
	}

	// Update our frame brake information.
	m_bLastBrake = bBrake;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetAccelTime
//
//	PURPOSE:	Get the current vehicle's accerlation time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetAccelTime() const
{
	LTFLOAT fTime = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			fTime = s_vtVehicleAccelTime.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMinTurnAccel
//
//	PURPOSE:	Get the current vehicle's minimum turn acceleration
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetMinTurnAccel() const
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			fValue = s_vtVehicleMinTurnAccel.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxDecel
//
//	PURPOSE:	Get the current vehicle's maximum deceleration
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetMaxDecel() const
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			fValue = s_vtVehicleMaxDecel.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	LTFLOAT fMoveMulti = g_pMoveMgr->GetMoveMultiplier();
	fValue *= fMoveMulti;

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetDecelTime
//
//	PURPOSE:	Get the current vehicle's deceleration time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetDecelTime() const
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			fValue = s_vtVehicleDecelTime.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxBrake
//
//	PURPOSE:	Get the current vehicle's max break value
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetMaxBrake() const
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			fValue = s_vtVehicleMaxBrake.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	LTFLOAT fMoveMulti = g_pMoveMgr->GetMoveMultiplier();
	fValue *= fMoveMulti;

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetBrakeTime
//
//	PURPOSE:	Get the current vehicle's break time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetBrakeTime() const
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			fValue = s_vtVehicleBrakeTime.GetFloat();
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateFriction
//
//	PURPOSE:	Update fricton
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateFriction()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
			UpdateVehicleFriction( g_vtVehicleSlideToStopTime.GetFloat( ));
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleFriction
//
//	PURPOSE:	Update vehicle fricton
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleFriction(LTFLOAT fSlideToStopTime)
{
	// Dampen our velocity so we don't slide around too much...

    uint32 dwTestFlags = BC_CFLG_MOVING | BC_CFLG_JUMP;

    LTVector vZero(0, 0, 0);

	if ( !(m_dwControlFlags & dwTestFlags) && g_pMoveMgr->IsOnGround() &&
		g_pMoveMgr->GetTotalCurrent() == vZero)
	{
        LTVector vCurVel = g_pMoveMgr->GetVelocity();

        LTFLOAT fYVal = vCurVel.y;
		vCurVel.y = 0.0f;

        LTVector vVel;

		if (vCurVel.Mag() > 5.0f)
		{
            LTVector vDir = vCurVel;
			vDir.Normalize();

			if (fSlideToStopTime < 0.0f) fSlideToStopTime = 0.1f;

            LTFLOAT fAdjust = g_pGameClientShell->GetFrameTime() * (GetMaxVelMag()/fSlideToStopTime);

			vVel = (vDir * fAdjust);

			if (vVel.MagSqr() < vCurVel.MagSqr())
			{
				vVel = vCurVel - vVel;
			}
			else
			{
				vVel.Init();
			}

			vVel.y = fYVal;
		}
		else
		{
			vVel.Init();
		}

		g_pMoveMgr->SetVelocity(vVel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsVehiclePhysics
//
//	PURPOSE:	Are we doing vehicle physics
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::IsVehiclePhysics()
{
	return IsVehicleModel(m_ePPhysicsModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CanDismount
//
//	PURPOSE:	Are we allowed do get off the vehicle
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::CanDismount()
{
	if( !IsVehiclePhysics() )
		return LTFALSE;

	return (m_ePPhysicsModel != PPM_LURE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::EnableWeapon
//
//	PURPOSE:	Enable/disables the weapon.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::EnableWeapon( bool bEnable, bool bToggleHolster /*= true*/ )
{
	// Get our weapon.
	CClientWeaponMgr *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	IClientWeaponBase* pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	if( !pClientWeapon || g_pPlayerMgr->IsCarryingHeavyObject() )
		return;

	// Enable the weapon...
	if( bEnable )
	{
		// We must enable the weapon before holstering it...
		
		if (!pClientWeaponMgr->WeaponsEnabled())
		{
			pClientWeaponMgr->EnableWeapons();
		}
		
		
		// Check if we have a holstered weapon.
		if( m_bHolsteredWeapon && bToggleHolster )
		{
			g_pPlayerMgr->ToggleHolster( false );
			m_bHolsteredWeapon = false;
		}
	}
	// Disable the weapon.
	else
	{
		if( !m_bHolsteredWeapon && bToggleHolster )
		{
			// Can't holster melee weapon. (why i don't know BEP).
			if (!pClientWeapon->IsMeleeWeapon())
			{
				g_pPlayerMgr->ToggleHolster( false );
				m_bHolsteredWeapon = true;
			}
		}

		if (pClientWeaponMgr->WeaponsEnabled())
		{
			pClientWeaponMgr->DisableWeapons();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::PreSetPhysicsModel
//
//	PURPOSE:	Do any clean up from old model
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::PreSetPhysicsModel( PlayerPhysicsModel eModel )
{
	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;
	m_fContourRoll = 0.0f;
	m_bHasWeapon = false;

	HOBJECT hObj = g_pMoveMgr->GetObject();

	LTBOOL bRet = LTTRUE;

	bool bTouchNotify;

	if (IsVehicleModel(eModel))
	{
		switch (eModel)
		{
			case PPM_VEHICLE :
				EnableWeapon( false );
			break;
		
			case PPM_LURE :
			case PPM_NORMAL :
			default :
			break;
		}

		// Needed for collision physics...

		bTouchNotify = true;
		g_pPhysicsLT->SetForceIgnoreLimit(hObj, 0.0f);
	}
	else
	{
		bTouchNotify = false;
		g_pPhysicsLT->SetForceIgnoreLimit(hObj, 1000.0f);

		// Turn off the vehicle...

		KillAllVehicleSounds();

 		switch (m_ePPhysicsModel)
		{
			case PPM_VEHICLE :
			{
				// We're getting off a vehicle so play it's shutdown sound...

				if( m_pVehicle )
				{
					g_pClientSoundMgr->PlaySoundLocal( m_pVehicle->sTurnOffSnd.c_str(), SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_CLIENT );
				}
				
				PlayVehicleAni("Deselect");
				
				// Can't move until deselect is done...
				g_pMoveMgr->AllowMovement(LTFALSE);
				
				// Can't shoot the vehicle weapon anymore...
				
				if( m_bHasWeapon )
				{
					m_VehicleWeapon.SetDisable( true );
					m_VehicleWeapon.Deactivate();
				}
				
				// Wait to change modes...
				bRet = LTFALSE;
			}
			break;

			case PPM_LURE :
			{
				// Enable the weapon.
				EnableWeapon( true );
			}
			break;

			default :
			case PPM_NORMAL :
			break;
		}

	}

	g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, bTouchNotify ? FLAG_TOUCH_NOTIFY : 0, FLAG_TOUCH_NOTIFY);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetPhysicsModel
//
//	PURPOSE:	Set the physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPhysicsModel( PlayerPhysicsModel eModel, uint8 nModelId /*= (uint8)-1*/, LTBOOL bDoPreSet /*= LTTRUE*/ )
{
	if (m_ePPhysicsModel == eModel) return;

	// Do clean up...

	if (bDoPreSet && !PreSetPhysicsModel(eModel)) return;

	m_ePPhysicsModel = eModel;

	// Cache our vehicle record for quick access...

	m_pVehicle = g_pVehicleButeMgr->GetVehicle( nModelId );

 	switch (eModel)
	{
		case PPM_VEHICLE :
		{
			if( !m_pVehicle )
			{
				g_pLTClient->CPrint( "ERROR - Could not find vehicle record." );
				SetPhysicsModel( PPM_NORMAL );
				break;
			}

			SetVehiclePhysicsModel();

			if( s_vtVehicleInfoTrack.GetFloat())
			{
				g_pLTClient->CPrint( "%s Physics Mode: ON", m_pVehicle->sName.c_str() );
			}
		}
		break;

		case PPM_LURE :
		{
			SetPlayerLurePhysicsModel( );
		}
		break;

		default :
		case PPM_NORMAL :
		{
			SetNormalPhysicsModel();

			if( s_vtVehicleInfoTrack.GetFloat() )
			{
				g_pLTClient->CPrint("Normal Physics Mode: ON");
			}
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::PlayVehicleAni
//
//	PURPOSE:	Change the vehicle's animation...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::PlayVehicleAni(char* pAniName, LTBOOL bReset, LTBOOL bLoop)
{
	if (!m_hVehicleModel || !pAniName) return;

	LTBOOL bIsDone = LTFALSE;
	if (!bReset && IsCurVehicleAni(pAniName, bIsDone))
	{
		return;
	}

	uint32 dwAnim = g_pLTClient->GetAnimIndex(m_hVehicleModel, pAniName);

    g_pLTClient->SetModelLooping(m_hVehicleModel, bLoop != LTFALSE);
	g_pLTClient->SetModelAnimation(m_hVehicleModel, dwAnim);

	if (s_vtVehicleInfoTrack.GetFloat() == VEHICLE_TRACK_ANIMATION)
	{
       g_pLTClient->CPrint("Playing Ani( %s )", pAniName);
	}

	if (bReset)
	{
		g_pLTClient->ResetModelAnimation(m_hVehicleModel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsCurVehicleAni
//
//	PURPOSE:	See if this is the current vehicle animation
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::IsCurVehicleAni(char* pAniName, LTBOOL & bIsDone)
{
	if (!m_hVehicleModel || !pAniName) return LTFALSE;

	// See if the current ani is done...

	uint32 dwState = g_pLTClient->GetModelPlaybackState(m_hVehicleModel);
 	bIsDone = (dwState & MS_PLAYDONE);

	uint32 dwTestAni = g_pLTClient->GetAnimIndex(m_hVehicleModel, pAniName);
	uint32 dwAni	 = g_pLTClient->GetModelAnimation(m_hVehicleModel);

	return (dwTestAni == dwAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetVehiclePhysicsModel
//
//	PURPOSE:	Set the vehicle physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetVehiclePhysicsModel( )
{
	if( !m_pVehicle )
		return;
	
	if (!m_hVehicleStartSnd )
	{
        uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		m_hVehicleStartSnd = g_pClientSoundMgr->PlaySoundLocal( m_pVehicle->sStartUpSnd.c_str(), SOUNDPRIORITY_PLAYER_HIGH, dwFlags );
	}

	CreateVehicleModel();

	// Initialize the vehicle weapon if it has one...

	if( !m_pVehicle->sVehicleWeapon.empty() )
	{
		const WEAPON *pVehicleWeapon = g_pWeaponMgr->GetWeapon( m_pVehicle->sVehicleWeapon.c_str() );
		if( pVehicleWeapon )
		{
			// Initialize the vehicle weapon...

			m_VehicleWeapon.ResetWeapon( );
			m_VehicleWeapon.Init( *pVehicleWeapon );

			// Activate the vehicle weapon model...

			m_VehicleWeapon.SetVehicleWeaponModel( m_hVehicleModel );
			
			if( m_VehicleWeapon.Activate( ))
			{
				m_bHasWeapon = true;

				// Don't allow the player to shoot the vehicle weapon untill the select animation has finished...

				m_VehicleWeapon.SetDisable( true );
			}
		}
	}

	// Read all the values we want to set as console vars for ingame tweaking...
	
	s_vtVehicleVel.SetFloat( m_pVehicle->fVel );
	s_vtVehicleAccel.SetFloat( m_pVehicle->fAccel );
	s_vtVehicleAccelTime.SetFloat( m_pVehicle->fAccelTime );
	s_vtVehicleMaxDecel.SetFloat( m_pVehicle->fMaxDecel );
	s_vtVehicleDecelTime.SetFloat( m_pVehicle->fDecelTime );
	s_vtVehicleMaxBrake.SetFloat( m_pVehicle->fMaxBrake );
	s_vtVehicleBrakeTime.SetFloat( m_pVehicle->fBrakeTime );
	s_vtVehicleStoppedPercent.SetFloat( m_pVehicle->fStoppedPercent );
	s_vtVehicleMaxSpeedPercent.SetFloat( m_pVehicle->fMaxSpeedPercent );
	s_vtVehicleMinTurnAccel.SetFloat( m_pVehicle->fMinTurnAccel );
	s_vtVehicleTurnRate.SetFloat( m_pVehicle->fTurnRate );
	s_vtVehicleInitialTurnRate.SetFloat( m_pVehicle->fInitialTurnRate );
	s_vtVehicleSurfaceSpeedUpPitch.SetFloat( m_pVehicle->fSurfaceSpeedUpPitch );
	s_vtVehicleSurfaceSlowDownPitch.SetFloat( m_pVehicle->fSurfaceSlowDownPitch );
	s_vtVehicleStartMotionPitch.SetFloat( m_pVehicle->fStartMotionPitch );
	s_vtVehicleStopMotionPitch.SetFloat( m_pVehicle->fStopMotionPitch );
	s_vtVehicleBrakePitch.SetFloat( m_pVehicle->fBrakePitch );
	s_vtVehicleDecelPitch.SetFloat( m_pVehicle->fDecelPitch );

	m_vVehicleOffset = m_pVehicle->vOffset;

	// Reset acceleration so we don't start off moving...

	m_fVehicleBaseMoveAccel = 0.0f;

 	ShowVehicleModel(LTTRUE);

	PlayVehicleAni("Select");

	// Can't move until select ani is done...

	g_pMoveMgr->AllowMovement(LTFALSE);

	m_bSetLastAngles = false;
	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();
	m_vLastVehiclePYR.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetPlayerLurePhysicsModel
//
//	PURPOSE:	Set the playerlure physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPlayerLurePhysicsModel()
{
	// Get the playerlurefx for info.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		return;
	}

	// Do special setup for bicycle.
	if( pPlayerLureFX->GetBicycle( ))
	{
		CreateVehicleModel();
	 	ShowVehicleModel(LTTRUE);

		if (!m_hVehicleAccelSnd)
		{
			KillAllVehicleSounds();

			const char* pSound = GetAccelSnd();

			if (pSound)
			{
				uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT | PLAYSOUND_LOOP;
				m_hVehicleAccelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}
	}

	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	// No need for a vehicle weapon...
	
	m_VehicleWeapon.Deactivate();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetNormalPhysicsModel
//
//	PURPOSE:	Set the normal physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetNormalPhysicsModel()
{
	if (m_hVehicleModel)
	{
        g_pLTClient->RemoveObject(m_hVehicleModel);
		m_hVehicleModel = LTNULL;
	}

	KillAllVehicleSounds();

	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	// Remove player-view attachments

	g_pPVAttachmentMgr->RemovePVAttachments();

	// No need for a vehicle weapon...
	
	m_VehicleWeapon.Deactivate();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CreateVehicleModel
//
//	PURPOSE:	Create the player-view vehicle model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CreateVehicleModel()
{
	ObjectCreateStruct ocs;
	
	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{
			ocs.m_ObjectType	= OT_MODEL;
			ocs.m_Flags			= FLAG_VISIBLE | FLAG_REALLYCLOSE;
			ocs.m_Flags2		= FLAG2_FORCETRANSLUCENT | FLAG2_DYNAMICDIRLIGHT;

			if( !m_pVehicle )
				break;

			LTStrCpy( ocs.m_Filename, m_pVehicle->sPVModel.c_str(), ARRAY_LEN( ocs.m_Filename ));

			m_pVehicle->blrPVSkins.CopyList( 0, ocs.m_SkinNames[0], ARRAY_LEN( ocs.m_SkinNames[1] ));
			m_pVehicle->blrPVRenderStyles.CopyList( 0, ocs.m_RenderStyleNames[0], ARRAY_LEN( ocs.m_RenderStyleNames[0] ));


			if( (m_pVehicle->blrPVSkins.GetNumItems( )) &&
				(!_stricmp( m_pVehicle->blrPVSkins.GetItem( 0 ), "Hands" ) ))
			{
				// This looks for the special case where "Hands" is specified as the first
				// player view skin.  If it is, then we take the skin from the model bute
				// instead of the weapon bute.
				
				CCharacterFX* pCharFX = g_pMoveMgr->GetCharacterFX();
				ModelId nModelID = ( pCharFX ) ? pCharFX->GetModelId() : static_cast< ModelId >( 0 );
				
				LTStrCpy( ocs.m_SkinNames[ 0 ], g_pModelButeMgr->GetHandsSkinFilename( nModelID ), ARRAY_LEN( ocs.m_SkinNames[0] ));
			}
			
		}
		break;

		case PPM_LURE:
		{
			// NEED TO MOVE THESE INTO A BUTE FILE!!!

			ocs.m_ObjectType	= OT_MODEL;
			ocs.m_Flags			= FLAG_VISIBLE;
			ocs.m_Flags2		= FLAG2_DYNAMICDIRLIGHT;

			SAFE_STRCPY(ocs.m_Filename, "chars\\Models\\armtricycle.ltb");
			SAFE_STRCPY(ocs.m_SkinNames[0], "chars\\skins\\armstrong.dtx");
			SAFE_STRCPY(ocs.m_SkinNames[1], "chars\\skins\\armstronghead.dtx");
			SAFE_STRCPY(ocs.m_RenderStyleNames[0], "RS\\default.ltb");
		}
		break;

		default :
			return;
		break;
	}

	// Remove any old vehicle model.
	if( m_hVehicleModel )
	{
		g_pLTClient->RemoveObject( m_hVehicleModel );
		m_hVehicleModel = NULL;
	}

    m_hVehicleModel = g_pLTClient->CreateObject(&ocs);
	if (!m_hVehicleModel) return;
	
	// Set up node control...
	// Do this AFTER we set the filename of the model since the node control is now per node.
	// First remove any we may have previously set otherwise the matrix will be applied multiple times.

	g_pModelLT->RemoveNodeControlFn( m_hVehicleModel, ModelNodeControlFn, this );
	g_pModelLT->AddNodeControlFn(m_hVehicleModel, ModelNodeControlFn, this);
    

	// Map all the nodes in our skeleton in the bute file to the nodes in the actual .ltb model file

	m_VehicleNode1.hNode = LTNULL;
	m_VehicleNode2.hNode = LTNULL;
	m_VehicleNode3.hNode = LTNULL;

	// Do post creation stuff.
	switch( m_ePPhysicsModel )
	{
		case PPM_VEHICLE :
		{
			g_pLTClient->SetModelLooping(m_hVehicleModel, LTFALSE);


			int iNode = 0;
			HMODELNODE hCurNode = INVALID_MODEL_NODE;
			while (g_pLTClient->GetModelLT()->GetNextNode(m_hVehicleModel, hCurNode, hCurNode) == LT_OK)
			{
				char szName[64];
				*szName = 0;
				g_pLTClient->GetModelLT()->GetNodeName(m_hVehicleModel, hCurNode, szName, sizeof(szName));

				if (stricmp(szName, "Handlebars1") == 0)
				{
					m_VehicleNode1.hNode = hCurNode;
				}
				else if( stricmp( szName, "Speedometer1" ) == 0 )
				{
					m_VehicleNode2.hNode = hCurNode;
				}
			}
			
			// Create any player-view attachments 

			g_pPVAttachmentMgr->CreatePVAttachments( m_hVehicleModel );
		}
		break;

		case PPM_LURE:
		{
			g_pLTClient->SetModelLooping( m_hVehicleModel, LTFALSE );
		}
		break;

		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateModels
//
//	PURPOSE:	Update the player-view models
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateModels()
{
	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		UpdateVehicleModel();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleModel
//
//	PURPOSE:	Update the player-view vehicle model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleModel()
{
	if (!m_hVehicleModel || (m_ePPhysicsModel != PPM_VEHICLE) )
		return;

	//See if we are paused, if so we need to pause the animation...

	bool bPaused = g_pGameClientShell->IsServerPaused();
	g_pLTClient->Common()->SetObjectFlags(m_hVehicleModel, OFT_Flags, bPaused ? FLAG_PAUSED : 0, FLAG_PAUSED);

	if (bPaused) return;

	// Update our camera offset mgr...

	m_VehicleModelOffsetMgr.Update();

	uint32 dwVehicleFlags;
	g_pCommonLT->GetObjectFlags(m_hVehicleModel, OFT_Flags, dwVehicleFlags);

	// See if we're just waiting for he deselect animation to finish...

	LTBOOL bIsDone = LTFALSE;
	if (IsCurVehicleAni("Deselect", bIsDone))
	{
		if (bIsDone)
		{
			// Okay to move now...
			g_pMoveMgr->AllowMovement(LTTRUE);

			ShowVehicleModel(LTFALSE);
			SetPhysicsModel(PPM_NORMAL, VBMGR_INVALID_ID, LTFALSE);

			// Enable the weapon...
			EnableWeapon( true );

			// Tell the server we want off...NOTE: In multiplayer we may want
			// to call this in PreSetPhysicsModel()
			g_pPlayerMgr->DoActivate();
			return;
		}
	}

	// If we're in specatator mode, or 3rd-person don't show the player-view
	// vehicle model...

	if (g_pPlayerMgr->IsSpectatorMode() || !g_pPlayerMgr->IsFirstPerson())
	{
		ShowVehicleModel(LTFALSE);
		return;
	}

	ShowVehicleModel(LTTRUE);

    LTRotation rRot;

    LTVector vPitchYawRollDelta = m_VehicleModelOffsetMgr.GetPitchYawRollDelta();

	m_vVehiclePYR.x += vPitchYawRollDelta.x;
	m_vVehiclePYR.y	+= vPitchYawRollDelta.y + m_fHeadYaw;
	m_vVehiclePYR.z += vPitchYawRollDelta.z;

	rRot = LTRotation(m_vVehiclePYR.x, m_vVehiclePYR.y, m_vVehiclePYR.z);

	LTVector vNewPos = m_VehicleModelOffsetMgr.GetPosDelta();

	vNewPos += rRot.Right() * (m_vVehicleOffset.x + m_vHeadOffset.x);
	vNewPos += rRot.Up() * (m_vVehicleOffset.y + m_vHeadOffset.y);
	vNewPos += rRot.Forward() * (m_vVehicleOffset.z + m_vHeadOffset.z);

	g_pLTClient->SetObjectPosAndRotation(m_hVehicleModel, &vNewPos, &rRot);

	// Reset the vehicle angles...

	m_vVehiclePYR.Init();

	// Give player-view attachments an update...

	g_pPVAttachmentMgr->UpdatePVAttachments();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::ShowVehicleModel
//
//	PURPOSE:	Show/Hide the vehicle model (and attachments)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::ShowVehicleModel(LTBOOL bShow)
{
	if (!m_hVehicleModel) return;

	if (bShow)
	{
		g_pCommonLT->SetObjectFlags(m_hVehicleModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hVehicleModel, OFT_Flags, 0, FLAG_VISIBLE);
	}

	g_pPVAttachmentMgr->ShowPVAttachments( !!(bShow) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxVelMag
//
//	PURPOSE:	Get the max velocity for our current mode
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetMaxVelMag(LTFLOAT fMult) const
{
    LTFLOAT fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{
			fMaxVel = s_vtVehicleVel.GetFloat();
		}
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	fMaxVel *= fMult;

	LTFLOAT fMoveMulti = g_pMoveMgr->GetMoveMultiplier();

// Allow vehicles to be affected by runspeed.
//	fMoveMulti = (fMoveMulti > 1.0f ? 1.0f : fMoveMulti);

	fMaxVel *= (g_pMoveMgr->IsBodyInLiquid() ? fMoveMulti/2.0f : fMoveMulti);

	return fMaxVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxAccelMag
//
//	PURPOSE:	Get the max acceleration for our current mode
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetMaxAccelMag(LTFLOAT fMult) const
{
    LTFLOAT fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{
			fMaxVel = s_vtVehicleAccel.GetFloat();
		}
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	fMaxVel *= fMult;

	fMaxVel *= g_pMoveMgr->GetMoveAccelMultiplier();

	return fMaxVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetSurfaceVelMult
//
//	PURPOSE:	Get the velocity multiplier for the current surface
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetSurfaceVelMult(SurfaceType eSurf) const
{
	LTFLOAT fVelMultiplier = 1.0f;

	SURFACE* pSurface = g_pSurfaceMgr->GetSurface(eSurf);

	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{
			if( pSurface && m_pVehicle )
			{
				SURFACE::VehicleType *pSurfaceVehicleType = pSurface->GetVehicleType( m_pVehicle->nSurfaceVehicleType );
				if( pSurfaceVehicleType )
				{
					fVelMultiplier = pSurfaceVehicleType->fVelMult;
				}
			}
		}
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	return fVelMultiplier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleSounds
//
//	PURPOSE:	Update sounds while on the vehicle
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleSounds()
{
	// Update what gear the vehicle is in...

	UpdateGear();

	// See if the start snd is done playing...

	if (m_hVehicleStartSnd)
	{
        if (g_pLTClient->IsDone(m_hVehicleStartSnd))
		{
            g_pLTClient->SoundMgr()->KillSound(m_hVehicleStartSnd);
            m_hVehicleStartSnd = LTNULL;
		}
	}



	// See if we are going forward or backward...
    bool bForward = !!( m_dwControlFlags & BC_CFLG_FORWARD );
	bool bReverse = !!( m_dwControlFlags & BC_CFLG_REVERSE );

	// Check the brake.  Only allow braking if on ground.
	bool bBrake = false;
	if( g_pMoveMgr->CanDoFootstep( ) && !m_bVehicleStopped )
	{
		// Check if we're moving in our forward direction.
		LTVector myVel = g_pMoveMgr->GetVelocity();
		LTRotation rRot;
		g_pLTClient->GetObjectRotation(g_pMoveMgr->GetObject(), &rRot);
		LTVector vForward = rRot.Forward();
		bool bMovingForward = ( vForward.Dot( myVel ) > 0.0f );

		if( bMovingForward && bReverse )
			bBrake = true;
		else if( !bMovingForward && bForward )
			bBrake = true;
	}

	// Check if vehicle isn't allowed to go in reverse.
	if( !m_pVehicle->bHasReverse )
	{
		bReverse = false;
	}

	LTBOOL bIsDone = LTFALSE;
	LTBOOL bPlayAni = LTTRUE;

	if (IsCurVehicleAni("Select", bIsDone))
	{
		if (bIsDone)
		{
			// Okay, we can move now...
			g_pMoveMgr->AllowMovement(LTTRUE);

			if( m_bHasWeapon )
			{
				// We can now shoot...

				m_VehicleWeapon.SetDisable( false );
			}
		}
		else // Can't do anything until this is done...
		{
			return;
		}
	}
	else if (IsCurVehicleAni("Deselect", bIsDone))
	{
		return;
	}

	bIsDone = LTFALSE;

	// Play the appropriate sound, based on acceleration...

	if(( !bForward && !bReverse ) || bBrake )
	{
		if (m_hVehicleAccelSnd)
		{
            g_pLTClient->SoundMgr()->KillSound(m_hVehicleAccelSnd);
            m_hVehicleAccelSnd = LTNULL;
		}

		// Play braking ani if necessary...

		bool bVehicleBreaking = (bBrake && !m_bVehicleStopped);

		if( m_bHasWeapon )
		{
			m_VehicleWeapon.SetVehicleBrakeOn( bVehicleBreaking );
		}

		if( bVehicleBreaking ) 
		{
			if( !m_bHasWeapon || m_VehicleWeapon.GetState() != W_FIRING )
			{
				PlayVehicleAni("BrakeOn", LTFALSE);
			}

			if (!m_hBrakeSnd)
			{
				const char* pSound = GetBrakeSnd();

				if (pSound)
				{
					uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
					m_hBrakeSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
				}
			}

		}
		else
		{
			if (m_hBrakeSnd)
			{
                g_pLTClient->SoundMgr()->KillSound(m_hBrakeSnd);
                m_hBrakeSnd = LTNULL;
			}
		}

		if (m_bVehicleStopped)
		{
			if (m_hVehicleDecelSnd)
			{
                g_pLTClient->SoundMgr()->KillSound(m_hVehicleDecelSnd);
                m_hVehicleDecelSnd = LTNULL;
			}
		}
		else if (!m_hVehicleDecelSnd)
		{
			const char* pSound = GetDecelSnd();

			if (pSound)
			{
                uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
				m_hVehicleDecelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}
	}
	else // Accelerating
	{
		// Make sure we're playing the accelerate sound...

		if (!m_hVehicleAccelSnd)
		{
			KillAllVehicleSounds();

			const char* pSound = GetAccelSnd();

			if (pSound)
			{
                uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT | PLAYSOUND_LOOP;
				m_hVehicleAccelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}

		if( m_bVehicleAtMaxSpeed )
		{
			if( bPlayAni )
			{
				if( !m_bHasWeapon || m_VehicleWeapon.GetState() != W_FIRING )
				{
					PlayVehicleAni( "FullSpeed", LTFALSE, LTTRUE );
				}
			}
		}
		else
		{
			// Play accelleration ani if necessary...

			if (bPlayAni)
			{
				if( !m_bHasWeapon || m_VehicleWeapon.GetState() != W_FIRING )
				{
					PlayVehicleAni("Accelerate", LTFALSE);
				}
			}
		}
	}


	// See if we are idle...

	if (!bForward && !bReverse)
	{
		if (m_bVehicleStopped )
		{
			// If the vehicle has a weapon and it's curently firing, let the weapon handle the animation...
			
			if( !m_bHasWeapon || m_VehicleWeapon.GetState() != W_FIRING )
			{
				PlayVehicleAni("Idle_0", LTFALSE, LTTRUE);
			}
		
			if (!m_hIdleSnd)
			{
				KillAllVehicleSounds(LTNULL);

				const char* pSound = GetIdleSnd();

				if (pSound)
				{
					uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
					m_hIdleSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
				}
			}
		}
		else if( !bBrake )
		{
			if (bPlayAni)
			{
				if( !m_bHasWeapon || m_VehicleWeapon.GetState() != W_FIRING )
				{
					PlayVehicleAni("Decelerate", LTFALSE);
				}
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetIdleSnd
//
//	PURPOSE:	Get the appropriate idle sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetIdleSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{
			if( m_pVehicle )
			{
				return m_pVehicle->sIdleSnd.c_str();
			}
		}
		break;
	
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetAccelSnd
//
//	PURPOSE:	Get the appropriate acceleration sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetAccelSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{	
			if( m_pVehicle )
			{
				return m_pVehicle->sAccelSnd.c_str();
			}
		}
		break;

		case PPM_LURE :
			return "Snd\\Vehicle\\bicycle\\accel.wav";
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetDecelSnd
//
//	PURPOSE:	Get the appropriate deceleration sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetDecelSnd()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{
			if( m_pVehicle )
			{
				return m_pVehicle->sDecelSnd.c_str();
			}
		}
		break;
		
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetBrakeSnd
//
//	PURPOSE:	Get the appropriate brake sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetBrakeSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_VEHICLE :
		{
			if( m_pVehicle )
			{
				return m_pVehicle->sBrakeSnd.c_str();
			}
		}
		break;

		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetImpactSnd
//
//	PURPOSE:	Get the appropriate impact sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetImpactSnd(LTFLOAT fCurVelocityPercent, SURFACE* pSurface)
{
	if (!pSurface) return LTNULL;

	if (pSurface->eType == ST_FLESH)
	{
		if (GetRandom(0, 1) == 1)
		{
			return "Snd\\Vehicle\\vehiclehit1.wav";
		}
		else
		{
			return "Snd\\Vehicle\\vehiclehit2.wav";
		}
	}
	else
	{
		switch (m_ePPhysicsModel)
		{
			case PPM_VEHICLE :
			{
				if( !m_pVehicle )
					break;

				if (fCurVelocityPercent > 0.1f && fCurVelocityPercent < 0.4f)
				{
					return m_pVehicle->sSlowImpactSnd.c_str();
				}
				else if (fCurVelocityPercent < 0.7f)
				{
					return m_pVehicle->sMedImpactSnd.c_str();
				}
				else
				{
					return m_pVehicle->sFastImpactSnd.c_str();
				}
			}
			break;

			case PPM_LURE :
			break;

			case PPM_NORMAL:
			default :
			break;
		}
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::KillAllVehicleSounds
//
//	PURPOSE:	Kill all the vehicle sounds
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::KillAllVehicleSounds(HLTSOUND hException)
{
	if (m_hVehicleStartSnd && hException != m_hVehicleStartSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleStartSnd);
        m_hVehicleStartSnd = LTNULL;
	}

	if (m_hVehicleAccelSnd && hException != m_hVehicleAccelSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleAccelSnd);
        m_hVehicleAccelSnd = LTNULL;
	}

	if (m_hVehicleDecelSnd && hException != m_hVehicleDecelSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleDecelSnd);
        m_hVehicleDecelSnd = LTNULL;
	}

	if (m_hIdleSnd && hException != m_hIdleSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hIdleSnd);
        m_hIdleSnd = LTNULL;
	}

	if (m_hBrakeSnd && hException != m_hBrakeSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hBrakeSnd);
        m_hBrakeSnd = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnTouchNotify
//
//	PURPOSE:	Handle our object touching something...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnTouchNotify(CollisionInfo *pInfo, float forceMag)
{
	if (!pInfo->m_hObject) return;

	// Cancel out the engine's velocity adjustment on touch.  We'll handle that, thank you.
	LTVector vOldVel = g_pMoveMgr->GetVelocity();
	g_pMoveMgr->SetVelocity(vOldVel - pInfo->m_vStopVel);

	// Filter out the player object
	if (pInfo->m_hObject == g_pLTClient->GetClientObject()) return;

	// Test for a JumpVolume...
	
	uint16 nCode;
	if( g_pLTClient->GetContainerCode( pInfo->m_hObject, &nCode) )
	{
		if( nCode == CC_JUMP_VOLUME )
		{
			if( HandleJumpVolumeTouch( pInfo->m_hObject ))
			{
				return;			
			}
		}
	}
	
	// All other objects must be solid for us to collide with them...
    
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_Flags, dwFlags);
	if (!(dwFlags & FLAG_SOLID)) 
	{
		return;
	}

	LTPlane cPolyPlane;
	if (pInfo->m_hPoly != INVALID_HPOLY)
		g_pLTClient->Common()->GetPolyPlane(pInfo->m_hPoly, &cPolyPlane);
	else
	{
		cPolyPlane.m_Normal.Init();
	}

	LTVector vNormal = pInfo->m_Plane.m_Normal;

	// If flat enough, we can drive on it...

	if ((vNormal.y > 0.76f) || (cPolyPlane.m_Normal.y > 0.76f)) return;

	// Check initial stuff so we handle object collisions properly

	SurfaceType eType = ::GetSurfaceType(*pInfo);
	SURFACE* pSurface = g_pSurfaceMgr->GetSurface(eType);

	LTBOOL bIsWorld = IsMainWorld(pInfo->m_hObject);

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
        g_pLTClient->CPrint("***********************************");
        g_pLTClient->CPrint("MoveMgr Object Touch:");
        g_pLTClient->CPrint("  Stop Vel: %.2f, %.2f, %.2f", VEC_EXPAND(pInfo->m_vStopVel));
        g_pLTClient->CPrint("  (%s)", bIsWorld ? "WORLD" : "OBJECT");
        g_pLTClient->CPrint("  Normal: %.2f, %.2f, %.2f", VEC_EXPAND(vNormal));
        g_pLTClient->CPrint("  Object Type: %d", GetObjectType(pInfo->m_hObject));
		g_pLTClient->CPrint("  Surface Type: %s", pSurface ? pSurface->szName : "No surface");
	}

	vNormal.y = 0.0f; // Don't care about this anymore...


	// Determine the current percentage of our max velocity we are
	// currently moving...

	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	LTFLOAT fVelPercent = GetVehicleMovementPercent();

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
		g_pLTClient->CPrint("Calculated Velocity Percent: %.2f", fVelPercent);
	}


	// Hit objects

	if (!bIsWorld && pInfo->m_hObject)
	{
		uint32 dwUserFlags = 0;
		g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_User, dwUserFlags);

		if (dwUserFlags & USRFLG_CHARACTER)
		{
			if (fVelPercent > g_vtVehicleImpactAIDamageVelPercent.GetFloat())
			{
				// Make sure we're pointing at the correct surface type...

				pSurface = g_pSurfaceMgr->GetSurface(ST_FLESH);

				if ( HandleCharacterTouch( pInfo->m_hObject ) )
				{
					return;
				}
			}
		}
	}

	m_bWasTouched = true;
	m_cLastTouchInfo = *pInfo;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::HandleCollision
//
//	PURPOSE:	Handle colliding with something
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::HandleCollision()
{
	LTVector vNormal = m_cLastTouchInfo.m_Plane.m_Normal;

	SurfaceType eType = ::GetSurfaceType(m_cLastTouchInfo);
	SURFACE* pSurface = g_pSurfaceMgr->GetSurface(eType);

	LTBOOL bIsWorld = IsMainWorld(m_cLastTouchInfo.m_hObject);

	vNormal.y = 0.0f; // Don't care about this anymore...

	// Determine the current percentage of our max velocity we are
	// currently moving...

	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	LTFLOAT fVelPercent = GetVehicleMovementPercent();

	// Get our forward vector...

	HOBJECT hObj = g_pMoveMgr->GetObject();

    LTVector vU, vR, vF;
    LTRotation rRot;
	g_pLTClient->GetObjectRotation(hObj, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();
	
	vF.y = 0.0f; // Don't care about this...

	// Determine the direction to change our forward to...
	// Determine the angle between our forward and the new direction...

	vNormal.Normalize();
	vF.Normalize();

	// fAngle will be a value between -1 (left) and 1 (right)...

	LTFLOAT fAngle = VEC_DOT(vNormal, vR);

	if (fAngle > 0.0f)
	{
		fAngle = (MATH_PI/2.0f) * (1.0f - fAngle);
	}
	else
	{
		fAngle = (-MATH_PI/2.0f) * (1.0f + fAngle);
	}

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
	   g_pLTClient->CPrint("Impact Angle: %.2f", RAD2DEG(fAngle));
	}

	LTFLOAT fImpactAngle = (LTFLOAT) fabs(fAngle);

	float fBiggestAngle = DEG2RAD(g_vtVehicleImpactMaxAdjustAngle.GetFloat());

	if (fImpactAngle > fBiggestAngle)
	{
		fAngle = fAngle > 0.0f ? fBiggestAngle : -fBiggestAngle;
	}

	// Add this angle to the yaw...

    LTFLOAT fYaw         = g_pPlayerMgr->GetYaw();
    LTFLOAT fPlayerYaw   = g_pPlayerMgr->GetPlayerYaw();

	// Adjust the min impact angle based on our current velocity (the
	// faster you go the more hitting stuff matters ;)

	float fMinImpactAngle = DEG2RAD(g_vtVehicleImpactMinAngle.GetFloat());
	fMinImpactAngle = fMinImpactAngle + 
		((1.0f - ( fVelPercent * m_pVehicle->fImpactAngleSensitivity )) * (DEG2RAD(90.0f) - fMinImpactAngle));

	// See if we hit hard enough to play a sound and do damage...

	if (fImpactAngle > fMinImpactAngle)
	{
		LTFLOAT fPushPercent = g_vtVehicleImpactPushVelPercent.GetFloat();
		if (fVelPercent < fPushPercent) return;

		// Create an impact fx...

		LTVector vObjPos, vPos;
		g_pLTClient->GetObjectPos(hObj, &vObjPos);

		vPos = vObjPos;

		LTRotation rRot;
		g_pLTClient->GetObjectRotation( hObj, &rRot );

		// Play the FX closer to the supposed point of impact...

		vPos += (rRot.Forward() * 32.0f);

		// Play an fx based on the surface we hit...

		if( pSurface && m_pVehicle )
		{
			SURFACE::VehicleType *pSurfaceVehicleType = pSurface->GetVehicleType( m_pVehicle->nSurfaceVehicleType );
			if( pSurfaceVehicleType && !pSurfaceVehicleType->sImpactFX.empty() )
			{
				CLIENTFX_CREATESTRUCT fxCS( pSurfaceVehicleType->sImpactFX.c_str(), 0, vPos );
				fxCS.m_vTargetNorm = m_cLastTouchInfo.m_Plane.m_Normal;

				g_pClientFXMgr->CreateClientFX( NULL, fxCS, LTTRUE );
			}
		}

		// Push us away from the impact...(keep y value the same so we
		// don't get pushed up into the air)

		LTVector vDir = vVel;
		vDir.Normalize();

		float dot = VEC_DOT(vDir, m_cLastTouchInfo.m_Plane.m_Normal);
		dot *= -2.0f;

		VEC_ADDSCALED(vDir, vDir, m_cLastTouchInfo.m_Plane.m_Normal, dot);

		float fOldY = vPos.y;

		// Old algorithm
		// vPos += (vDir * -1.0f);
		// New algorithm, push vehicle along mirrored vector from point of impact,
		// the 32 accounts for the 32 added above so the pusher values don't need
		// to be adjusted...Basically we're just using the object's position instead 
		// of the fx position
		vPos = (vObjPos += (vDir * -32.0f));

		vPos.y = fOldY;

		float fPushRadius = g_vtVehicleImpactPushRadius.GetFloat();
		float fPushAmount = g_vtVehicleImpactPushAmount.GetFloat() * fVelPercent;
		float fStartDelay = 0.0f;
		float fDuration   = g_vtVehicleImpactPushDuration.GetFloat();
		g_pMoveMgr->AddPusher(vPos, fPushRadius, fStartDelay, fDuration, fPushAmount);

	
		if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
		{
			g_pLTClient->CPrint("Pusher Pos: %.2f, %.2f, %.2f Amount: %.2f", VEC_EXPAND(vPos), fPushAmount);
		}

		// Move handle bars down a bit...(head over)...

		CameraDelta delta;
		delta.Pitch.fVar	= DEG2RAD(3.0f);
		delta.Pitch.fTime1	= 0.1f;
		delta.Pitch.fTime2	= 0.3f;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);

		// Tilt steering wheel based on the surface we hit...

		LTFLOAT fOffset		= DEG2RAD(g_vtVehicleMouseMinYawBuffer.GetFloat());
		LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());
		LTFLOAT fYawPercent = (fImpactAngle / DEG2RAD(90.0f));
		m_fYawDiff = fYawPercent * fMaxOffset;
		m_fYawDiff *= (fAngle > 0.0f ? 1.0f : -1.0f);

		if (m_fYawDiff < -fOffset)
		{
			m_eMouseTurnDirection = TD_LEFT;
		}
		else if (m_fYawDiff > fOffset)
		{
			m_eMouseTurnDirection = TD_RIGHT;
		}

		// Clear velocity...

		LTVector vNewVel(0, 0, 0);

		LTFLOAT fAdjust = (1.0f - fYawPercent);
		vNewVel = vVel * fAdjust;
		g_pMoveMgr->SetVelocity(vNewVel);
		m_fVehicleBaseMoveAccel *= fAdjust;
		m_fAccelStart = -1.0f;

		// Reset the ani/sounds...

		if( !m_bHasWeapon || m_VehicleWeapon.GetState() != W_FIRING )
		{
			PlayVehicleAni("Idle_0", LTFALSE, LTTRUE);
		}
		
		KillAllVehicleSounds(LTNULL);

		
		// Play an impact sound based on the surface we hit..

		if (pSurface && !m_hVehicleImpactSnd)
		{
			const char* pSound = GetImpactSnd(fVelPercent, pSurface);
			if (pSound)
			{
				uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
				m_hVehicleImpactSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}

		// See if we hit hard enough to actually damage us...

		LTFLOAT fImpactVelPercent = g_vtVehicleImpactDamageVelPercent.GetFloat();
		if (fVelPercent > fImpactVelPercent)
		{
			// Okay, since we were going pretty damn fast, just determine
			// the damage based on the angle we hit...

			float fRange = (MATH_PI/2.0f) - fMinImpactAngle;
			if (fRange <= 0.0f) fRange = 0.01f;

			LTFLOAT fDamagePercent = 1.0f - (((MATH_PI/2.0f) - fImpactAngle) / fRange);

 			// Send damage message to server...

			float fVehicleImpactDamageMin = g_vtVehicleImpactDamageMin.GetFloat();
			float fVehicleImpactDamageMax = g_vtVehicleImpactDamageMax.GetFloat();

			float fDamageRange = fVehicleImpactDamageMax - fVehicleImpactDamageMin;
			float fDamage = fVehicleImpactDamageMin + fDamageRange * fDamagePercent;

			if (g_vtVehicleImpactDamage.GetFloat())
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
				cMsg.Writeuint8(CP_DAMAGE);
				cMsg.Writeuint8(DT_STUN);
				cMsg.Writefloat(0.0f);
				cMsg.WriteLTVector(vNormal);
				cMsg.Writeuint8(1);
				cMsg.Writefloat(1.0f);
				cMsg.WriteObject(g_pLTClient->GetClientObject());
			    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			}

			if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
			{
				g_pLTClient->CPrint("Impact Damage: %.2f", fDamage);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleLightPosRot
//
//	PURPOSE:	Get the vehicle light pos/rot
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::GetVehicleLightPosRot(LTVector & vPos, LTRotation & rRot)
{
	vPos.Init();
	rRot.Init();

	if (!m_hVehicleModel) return;

	// Get the vehicle light position/rotation

	HMODELSOCKET hSocket;

	if (g_pModelLT->GetSocket(m_hVehicleModel, "Light", hSocket) == LT_OK)
	{
		LTransform transform;
        if (g_pModelLT->GetSocketTransform(m_hVehicleModel, hSocket, transform, LTFALSE) == LT_OK)
		{
			vPos = transform.m_Pos;
			rRot = transform.m_Rot;

			HOBJECT hCamera = g_pPlayerMgr->GetCamera();
			if (!hCamera) return;

			LTVector vCamPos;
			g_pLTClient->GetObjectPos(hCamera, &vCamPos);
			g_pLTClient->GetObjectRotation(hCamera, &rRot);

			// Rotate down a bit...
			rRot.Rotate(rRot.Right(), DEG2RAD(10.0f));

			vPos += vCamPos;
		}
	}
	else
	{
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		if (!hCamera) return;

		g_pLTClient->GetObjectRotation(hCamera, &rRot);
		g_pLTClient->GetObjectPos(hCamera, &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::HandleNodeControl
//
//	PURPOSE:	Handle node control
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat)
{
	if (m_VehicleNode1.hNode && hNode == m_VehicleNode1.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode1.matTransform;
	}
	else if (m_VehicleNode2.hNode && hNode == m_VehicleNode2.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode2.matTransform;
	}
	else if (m_VehicleNode3.hNode && hNode == m_VehicleNode3.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode3.matTransform;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CalculateBankingVehicleRotation()
//
//	PURPOSE:	Calculate the new player/camera rotation values for 
//				banking vehicles.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CalculateBankingVehicleRotation(LTVector & vPlayerPYR,
										   LTVector & vPYR, LTFLOAT fYawDelta)
{
	if (s_vtVehicleInfoTrack.GetFloat() >= VEHICLE_TRACK_VERBOSE)
	{
		g_pLTClient->CPrint("VEHICLE VERBOSE INFO:");
		g_pLTClient->CPrint("Initial vPlayerPYR.y	= %.2f", vPlayerPYR.y);
		g_pLTClient->CPrint("Initial vPYR.y			= %.2f", vPYR.y);
		g_pLTClient->CPrint("Initial fYawDelta		= %.2f", fYawDelta);
		g_pLTClient->CPrint("Initial m_fYawDiff		= %.2f", m_fYawDiff);
	}

	// Don't allow pitch in camera angle.
	vPYR.x = 0.0f;

	vPlayerPYR.x = 0.0f;
	vPlayerPYR.z = vPYR.z;

	LTFLOAT fOffset		= DEG2RAD(g_vtVehicleMouseMinYawBuffer.GetFloat());
	LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());

	LTFLOAT fAdjustedYawDelta = s_vtVehicleInitialTurnRate.GetFloat() * g_pGameClientShell->GetFrameTime();;


	// If we're playing the select or deselect anis, don't allow us to turn...

	LTBOOL bIsDone = LTFALSE;
	if (IsCurVehicleAni("Deselect", bIsDone) || IsCurVehicleAni("Select", bIsDone))
	{
		fYawDelta = 0.0f;
		m_bKeyboardTurning = LTFALSE;
		m_dwControlFlags &= ~BC_CFLG_STRAFE_LEFT;
		m_dwControlFlags &= ~BC_CFLG_LEFT;
		m_dwControlFlags &= ~BC_CFLG_STRAFE_RIGHT;
		m_dwControlFlags &= ~BC_CFLG_RIGHT;
	}

	if (fYawDelta)
	{
		m_bKeyboardTurning = LTFALSE;
	}

	// See if we are turning (using the keyboard)...

	if ((m_dwControlFlags & BC_CFLG_STRAFE_LEFT) || (m_dwControlFlags & BC_CFLG_LEFT))
	{
		m_bKeyboardTurning = LTTRUE;
		fYawDelta = -fAdjustedYawDelta;
	}
	else if ((m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) || (m_dwControlFlags & BC_CFLG_RIGHT))
	{
 		m_bKeyboardTurning = LTTRUE;
		fYawDelta = fAdjustedYawDelta;
	}
	else
	{
		// If we were turning with the keyboard automatically adjust back to
		// center...

		if (m_bKeyboardTurning)
		{
			switch (m_eMouseTurnDirection)
			{
				case TD_LEFT :
				{
 					fYawDelta = fAdjustedYawDelta;
				}
				break;

				case TD_RIGHT :
				{
					fYawDelta = -fAdjustedYawDelta;
				}
				break;

				default :
				case TD_CENTER :
				{
					if (m_fYawDiff > 0.0f)
					{
						fYawDelta = -fAdjustedYawDelta;
					}
					else if (m_fYawDiff < -fOffset)
					{
						fYawDelta = fAdjustedYawDelta;
					}
				}
				break;
			}
		}
	}


	// Use our last calculated yaw delta for the amount to turn
	// (if it is greater than our min turn rate)...

	if (fYawDelta)
	{
		// [KLS 6/29/02] - If we're not turning with the keyboard, use the passed in
		// yaw delta if it is less than the calcuated fAdjustedYawDelta...

		if (!m_bKeyboardTurning)
		{
			float fPosYawDelta = (float)fabs(fYawDelta);
			fAdjustedYawDelta = ((fPosYawDelta < fAdjustedYawDelta) ? fPosYawDelta : fAdjustedYawDelta);
		}
		else if (m_fYawDelta > fAdjustedYawDelta)
		{
			fAdjustedYawDelta = m_fYawDelta;
		}

		fAdjustedYawDelta = (fYawDelta > 0.0f ? fAdjustedYawDelta : -fAdjustedYawDelta);

		if (s_vtVehicleInfoTrack.GetFloat() >= VEHICLE_TRACK_VERBOSE)
		{
			g_pLTClient->CPrint("fAdjustedYawDelta		= %.2f", fAdjustedYawDelta);
		}
		
		m_fYawDiff += fAdjustedYawDelta;
	}

	// Depending on the direction we are turning, cap our yaw difference (from center)
	// and determine if the direciton we're turning changed...

	switch (m_eMouseTurnDirection)
	{
		case TD_LEFT :
		{
			if (m_fYawDiff < -fMaxOffset)
			{
				m_fYawDiff = -fMaxOffset;
			}
			else if (m_fYawDiff > -fOffset)
			{
				m_eMouseTurnDirection = TD_CENTER;
			}
		}
		break;

		case TD_RIGHT :
		{
			if (m_fYawDiff > fMaxOffset)
			{
				m_fYawDiff = fMaxOffset;
			}
			else if (m_fYawDiff < fOffset)
			{
				m_eMouseTurnDirection = TD_CENTER;
			}
		}
		break;

		default :
		case TD_CENTER :
		{
			if (m_fYawDiff > fOffset)
			{
				m_eMouseTurnDirection = TD_RIGHT;
			}
			else if (m_fYawDiff < -fOffset)
			{
				m_eMouseTurnDirection = TD_LEFT;
			}
		}
		break;
	}

	// For now assume we aren't turning...

	m_nVehicleTurnDirection = 0;
    m_bVehicleTurning = LTFALSE;


	if (fYawDelta != 0.0f || m_eMouseTurnDirection != TD_CENTER)
	{
		m_bVehicleTurning = LTTRUE;

		if (m_eMouseTurnDirection != TD_CENTER)
		{
			m_nVehicleTurnDirection = (m_eMouseTurnDirection == TD_LEFT ? -1 : 1);
		}
		else if (fYawDelta != 0.0f)
		{
			m_nVehicleTurnDirection = (fYawDelta < 0.0f ? -1 : 1);
		}
	}

	// Calculate the player's yaw...

	if (m_bVehicleTurning)
	{
        LTFLOAT fTurnRate = s_vtVehicleTurnRate.GetFloat();

	    m_fYawDelta = fTurnRate * g_pGameClientShell->GetFrameTime();

		// Scale how fast we turn based on how close to center the
		// "steering wheel" is...
		float fSteeringOffset = ( fMaxOffset == 0.0f ) ? 1.0f : fMaxOffset;
		LTFLOAT fScale = (LTFLOAT)fabs(m_fYawDiff) / fSteeringOffset;
		fScale = WaveFn_SlowOn(fScale);

		m_fYawDelta *= fScale;

		// Scale the rate based on use input...

		m_fYawDelta *= s_vtVehicleTurnRateScale.GetFloat();

		if (s_vtVehicleInfoTrack.GetFloat() >= VEHICLE_TRACK_VERBOSE)
		{
			g_pLTClient->CPrint("Final m_fYawDelta		= %.2f", m_fYawDelta);
		}

		// Change direction of turn based on forward/backward motion.
		float fDirMult = 1.0f;
		if( m_bVehicleStopped )
		{
			bool bReverse = !!( m_dwControlFlags & BC_CFLG_REVERSE );
			if( bReverse && m_pVehicle->bHasReverse )
				fDirMult = -1.0f;
		}
		else
		{
			LTVector myVel = g_pMoveMgr->GetVelocity();
			LTRotation rRot;
			g_pLTClient->GetObjectRotation(g_pMoveMgr->GetObject(), &rRot);
			LTVector vForward = rRot.Forward();
			bool bMovingForward = ( vForward.Dot( myVel ) > 0.0f );
			if( !bMovingForward )
				fDirMult = -1.0f;
		}

		// Set the player's new yaw...
		if (!m_bVehicleStopped)
		{
			vPlayerPYR.y += (m_fYawDelta * m_nVehicleTurnDirection * fDirMult);
		}
	}

	// Keep the camera and player pitch/yaw/roll in sync...

	vPYR.y = vPlayerPYR.y;

	if (s_vtVehicleInfoTrack.GetFloat() >= VEHICLE_TRACK_VERBOSE)
	{
		g_pLTClient->CPrint("Final vPlayerPYR.y		= %.2f", vPlayerPYR.y);
		g_pLTClient->CPrint("Final vPYR.y			= %.2f", vPYR.y);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CalculateLureVehicleRotation()
//
//	PURPOSE:	Calculate the new player/camera rotation for lure.
//
// ----------------------------------------------------------------------- //

// Limits the angle to a -PI to PI range.
static inline float LimitToPosNegPi( float fAngle )
{
	// Copy the angle and make sure it's under 2 pi.
	float fNewAngle = ( float )fmod( fAngle, MATH_CIRCLE );

	if( fNewAngle > MATH_PI )
		fNewAngle = fNewAngle - MATH_CIRCLE;
	else if( fNewAngle < -MATH_PI )
		fNewAngle = fNewAngle + MATH_CIRCLE;
	
	return fNewAngle;
}


void CVehicleMgr::CalculateLureVehicleRotation(LTVector& vPlayerPYR,
										   LTVector& vPYR, float fYawDelta, float fPitchDelta )
{
	// Get the playerlurefx.  It has all of the data about the lure.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		ASSERT( !"CVehicleMgr::CalculateLureVehicleRotation: Missing PlayerLureFX." );
		return;
	}

	// Get the client player object.
	HOBJECT hObj = g_pMoveMgr->GetObject( );
	if( !hObj )
	{
		ASSERT( !"CVehicleMgr::CalculateLureVehicleRotation:  Missing client-side player object." );
		return;
	}

	// Get the rotation of the client object, which has been set by MoveToLure.
	// We need the rotation to calculate the euler angles for the restricted camera.
    LTRotation rPlayerLureRot;
	g_pLTClient->GetObjectRotation( hObj, &rPlayerLureRot );

	// Set the player's euler angles.
	EulerAngles eulerPlayerLure = Eul_FromQuat( rPlayerLureRot, EulOrdYXZr );
	vPlayerPYR.y = eulerPlayerLure.x;
	vPlayerPYR.x = eulerPlayerLure.y;
	vPlayerPYR.z = eulerPlayerLure.z;

	// Calculate the starting PRY before the delta's were applied.  They are applied
	// to vPYR before entering this function.
	LTVector vStartingPYR( vPYR.x - fPitchDelta, vPYR.y - fYawDelta, vPYR.z );

	// Copy the camera's PYR so we can restrict it before doing final calculations.
	LTVector vRestrictedCameraPYR = vPYR;

	// Scale the yaw by how close we are to the poles.  This helps avoid the gimbal lock.
	vRestrictedCameraPYR.y = vStartingPYR.y + ( fYawDelta * ( MATH_HALFPI - ( float )fabs( vRestrictedCameraPYR.x )) / MATH_HALFPI );

	// Check if the camera has restricted freedom.
	if( pPlayerLureFX->GetCameraFreedom( ) != kPlayerLureCameraFreedomUnlimited )
	{
		// Get the max pitch and yaw allowed for this lure.
		float fPitchMax = 0.0f;
		float fPitchMin = 0.0f;
		float fYawMax = 0.0f;
		float fYawMin = 0.0f;
		switch( pPlayerLureFX->GetCameraFreedom( ))
		{
			case kPlayerLureCameraFreedomLimited:
				// Convert the left/right/up/down angles to min/max's in radians.
				pPlayerLureFX->GetLimitedRanges( fYawMin, fYawMax, fPitchMax, fPitchMin );

				// Convert over to radians.  Have to negate the values for pitch since it 
				// works opposite intuitive values.
				fYawMin = ( fYawMin * MATH_PI ) / 180.0f;
				fYawMax = ( fYawMax * MATH_PI ) / 180.0f;
				fPitchMax = ( -fPitchMax * MATH_PI ) / 180.0f;
				fPitchMin = ( -fPitchMin * MATH_PI ) / 180.0f;
				break;
			case kPlayerLureCameraFreedomNone:
			default:
				break;
		}

		// Clamp the pitch and yaw.  Don't allow roll, since the player can't control roll.
		// Make sure the differences are between -PI and PI so they are usable.  Otherwise
		// we could think we have a large difference with say -PI and PI, which looks
		// like a difference of 2PI, when it's actually zero.
		LTVector vDiff = vRestrictedCameraPYR - vPlayerPYR;
		vDiff.x = LimitToPosNegPi( vDiff.x );
		vDiff.x = Clamp( vDiff.x, fPitchMin, fPitchMax );
		vDiff.y = LimitToPosNegPi( vDiff.y );
		vDiff.y = Clamp( vDiff.y, fYawMin, fYawMax );

		// Find the new change in pitch and yaw.
		vRestrictedCameraPYR = vPlayerPYR + vDiff;
	}

	// Create a rotation based on current camera orientation.  Use the roll from the playerlure.
	LTRotation rStartingCamera( vStartingPYR.x, vStartingPYR.y, vPlayerPYR.z );

	// Restrict pitch so that we don't get gimbal lock at the poles.
	static const float fRestrictedPitch = MATH_HALFPI * 0.85f;
	vRestrictedCameraPYR.x = Clamp( vRestrictedCameraPYR.x, -fRestrictedPitch, fRestrictedPitch );

	// Find the new pitch/yaw deltas using restricted camera PYR.
	float fNewPitchDelta = vRestrictedCameraPYR.x - vStartingPYR.x;
	float fNewYawDelta = vRestrictedCameraPYR.y - vStartingPYR.y;

	// Create a rotation based on user input.
	LTRotation rCameraOffset( fNewPitchDelta, fNewYawDelta, 0.0f );
	
	// Apply offsets to current camera space rather than global space.
	LTRotation rFinalCamera = rStartingCamera * rCameraOffset;

	// Find the resulting euler angles.
	EulerAngles eulerFinalCamera = Eul_FromQuat( rFinalCamera, EulOrdYXZr );
	vPYR.y = eulerFinalCamera.x;
	vPYR.x = eulerFinalCamera.y;
	vPYR.z = eulerFinalCamera.z;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CalculateVehicleRotation()
//
//	PURPOSE:	Calculate the new player/camera rotation.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CalculateVehicleRotation(LTVector & vPlayerPYR,
										   LTVector & vPYR, float fYawDelta, float fPitchDelta )
{
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		case PPM_VEHICLE :
			CalculateBankingVehicleRotation( vPlayerPYR, vPYR, fYawDelta );
			CalculateVehicleContourRotation( vPlayerPYR, vPYR );
			break;
		
		case PPM_LURE:
			CalculateLureVehicleRotation( vPlayerPYR, vPYR, fYawDelta, fPitchDelta );
			break;

		case PPM_NORMAL:
			return;
			break;

		default:
			ASSERT( !"CVehicleMgr::MoveLocalSolidObject: Invalid physics model." );
			break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CalculateVehicleCameraDisplacment()
//
//	PURPOSE:	Allows vehicles to perterb the camera placement.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CalculateVehicleCameraDisplacment( LTVector& vDisplacement )
{
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		case PPM_LURE:
			{
				PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
				if( !pPlayerLureFX )
				{
					ASSERT( !"CVehicleMgr::MoveToLure:  Missing PlayerLureFX." );
					return;
				}

				// Only do displacement stuff for bicycle.
				if( !pPlayerLureFX->GetBicycle( ))
					return;

				// Don't update the bob/sway if in spectator or 3rd person.
				if( g_pPlayerMgr->IsSpectatorMode() || !g_pPlayerMgr->IsFirstPerson( ))
					return;

				// Check if we're going over a bump.
				if( !m_LureTimeOverBump.Stopped( ))
				{
					float fPeriod = MATH_PI / m_LureTimeOverBump.GetDuration( );
					float fDisplacement = m_fLureBumpHeight * ( float )sin( m_LureTimeOverBump.GetElapseTime( ) * fPeriod );

					vDisplacement.y += fDisplacement;
				}
				// Check if it's time to go over another bump.
				else if( m_LureTimeToNextBump.Stopped( ))
				{
					float fMin, fMax;

					sscanf( g_vtBicycleInterBumpTimeRange.GetStr( ), "%f %f", &fMin, &fMax );
					m_LureTimeToNextBump.Start( GetRandom( fMin, fMax ));

					sscanf( g_vtBicycleIntraBumpTimeRange.GetStr( ), "%f %f", &fMin, &fMax );
					m_LureTimeOverBump.Start( GetRandom( fMin, fMax ));

					sscanf( g_vtBicycleBumpHeightRange.GetStr( ), "%f %f", &fMin, &fMax );
					m_fLureBumpHeight = GetRandom( fMin, fMax );
				}

				// Adjust our parameterized bob.
				m_fLureBobParameter += g_pGameClientShell->GetFrameTime( ) * g_vtBicycleBobRate.GetFloat( );
				
				// Keep in range of 0 to 1.0.
				m_fLureBobParameter = ( float )fmod( m_fLureBobParameter, 1.0f );

				// Get the camera so we can add some roll.
				HOBJECT hCamera = g_pPlayerMgr->GetCamera( );
				LTRotation rCameraRot;
				g_pLTClient->GetObjectRotation( hCamera, &rCameraRot );

				// Make our sway left/right always camera relative.
				LTVector vBobSway = rCameraRot.Right( );
				vBobSway.y = 0.0f;
				vBobSway.Normalize( );

				// Get the amount to sway and add it to the displacement.
				float fBobSway = ( float )sin( m_fLureBobParameter * MATH_CIRCLE );
				fBobSway *= g_vtBicycleBobSwayAmplitude.GetFloat( );
				vBobSway *= fBobSway;
				vDisplacement += vBobSway;
				
				// Get the amount of roll and apply it to the camera's rotation.
				float fBobRoll = ( float )sin( m_fLureBobParameter * MATH_CIRCLE );
				fBobRoll *= g_vtBicycleBobRollAmplitude.GetFloat( ) * MATH_PI / 180.0f;
				LTRotation rBobRoll( 0.0f, 0.0f, fBobRoll );
				LTRotation rFinal = rCameraRot * rBobRoll;
				g_pLTClient->SetObjectRotation( hCamera, &rFinal );

				if( !m_hVehicleModel )
				{
					ASSERT( !"CVehicleMgr::CalculateVehicleCameraDisplacment:  Missing vehicle model." );
					return;
				}

				LTVector vBicycleOffset;
				sscanf( g_vtBicycleModelOffset.GetStr( ), "%f %f %f", &vBicycleOffset.x, &vBicycleOffset.y, 
					&vBicycleOffset.z );
	
				// Offset the vehicle model based on the lure's transform.
				LTVector vLurePos;
				g_pLTClient->GetObjectPos( pPlayerLureFX->GetServerObj( ), &vLurePos );
				LTRotation rLureRot;
				g_pLTClient->GetObjectRotation( pPlayerLureFX->GetServerObj( ), &rLureRot );
				LTMatrix matLureRot;
				rLureRot.ConvertToMatrix( matLureRot );
				vBicycleOffset = matLureRot * vBicycleOffset;
				LTVector vVehicleModelPos = vLurePos + vBicycleOffset;
				g_pLTClient->SetObjectPos( m_hVehicleModel, &vVehicleModelPos );

				// Add the bob roll into the vehicle model rotation.
				LTRotation rVehicleModelRot = rLureRot * rBobRoll;
				g_pLTClient->SetObjectRotation( m_hVehicleModel, &rVehicleModelRot );
			}
			break;

		case PPM_NORMAL :
		case PPM_VEHICLE :
			return;
			break;

		default:
			ASSERT( !"CVehicleMgr::CalculateVehicleCameraDisplacment: Invalid physics model." );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::CalculateVehicleContourRotation
//
//  PURPOSE:	Calculate the rotation of the vehicle so it conforms to the ground normal
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CalculateVehicleContourRotation( LTVector &vPlayerPYR, LTVector &vPYR )
{
	if ( g_vtVehicleContour.GetFloat() < 1.0f ) 
	{
		// Don't do vehicle contouring...Rest our roll values...

		vPlayerPYR.z   = 0.0f;
		m_fContourRoll = 0.0f;
		return;
	}

	// Just use the previous rotation if we are in the air from a jumpvolume...
	
	if( m_bHaveJumpVolumeVel && !g_pMoveMgr->IsOnGround() )
	{
		vPlayerPYR	= m_vLastPlayerAngles;
		vPYR		= m_vLastCameraAngles;

		return;
	}


	// We are assuming that while on a vehicle the only roll will come from the vehicle,
	// so set the player roll to nothing.

	vPlayerPYR.z = 0.0f;

	// Get the new forward and right of the player...

	LTRotation	rPlayerRot( vPlayerPYR.x, vPlayerPYR.y, vPlayerPYR.z );
	LTVector	vPlayerF = rPlayerRot.Forward();
			
	vPlayerF.y = 0.0f;
	vPlayerF.Normalize();

	LTVector vPlayerR = vPlayerF.Cross( LTVector( 0, 1, 0 ));
	vPlayerR.Normalize();

	
	LTVector vNormal, vModelNormal;


	// Depending on how many points we want get the appropriate values...

	if( g_vtVehicleContourPoints.GetFloat() < 2.0f )
	{
		// Get the normal directly under us and use that to calculate our pitch and roll
		// NOTE: Works well when the vehicle is covering a single normal but "snaps" to the
		//		 new pitch and roll when moving from one normal to another.
	
		vNormal = g_pMoveMgr->GetGroundNormal();
	}
	else
	{
		// Get the normals under the 4 corners of the vehicle and average them toghether 
		// to get the normal we should use.  This helps smooth out the transition between 
		// two or more planes.

		HOBJECT		hObj = g_pMoveMgr->GetObject();
		LTVector	vDims, vPos;
		
		HOBJECT		hFilter[] = { g_pLTClient->GetClientObject(), hObj, LTNULL };
		
		g_pLTClient->GetObjectPos( hObj, &vPos );
		g_pPhysicsLT->GetObjectDims( hObj , &vDims );

		LTVector	vForward = vPlayerF * (vDims.z + g_vtVehicleContourExtraDimsZ.GetFloat());
		LTVector	vRight = vPlayerR * (vDims.x + g_vtVehicleContourExtraDimsX.GetFloat());
		vModelNormal = GetContouringNormal( vPos, vDims, vForward, vRight, hFilter );

		vForward = vPlayerF * (vDims.z + g_vtVehicleCamContourExtraDimsZ.GetFloat());
		vRight	= vPlayerR * (vDims.x + g_vtVehicleCamContourExtraDimsX.GetFloat());
		vNormal = GetContouringNormal( vPos, vDims, vForward, vRight, hFilter );
	}

	// Calculate how much pitch and roll we should apply...

	float fPitchPercent, fModelPitchPercent;
	float fRollPercent, fModelRollPercent;
	float fAmount, fModelAmount;

	GetContouringInfo( vPlayerF, vNormal, fAmount, fPitchPercent, fRollPercent );
	GetContouringInfo( vPlayerF, vModelNormal, fModelAmount, fModelPitchPercent, fModelRollPercent );

	// Save the roll we are applying so the canting can use it...
	m_fContourRoll = ( m_pVehicle->bContourAffectsRoll ) ? fAmount * fRollPercent : 0.0f;

	vPlayerPYR.x += fAmount * fPitchPercent;
	vPlayerPYR.z += m_fContourRoll;

	vPYR.x += vPlayerPYR.x;
	vPYR.z += vPlayerPYR.z;

	if (g_vtVehicleContourPlayerViewModel.GetFloat())
	{
		m_vVehiclePYR.x = (fModelAmount * fModelPitchPercent) - (fAmount * fPitchPercent);
		m_vVehiclePYR.z	= (fModelAmount * fModelRollPercent) - (fAmount * fRollPercent);
	}

	// [KLS 6/29/02] - Interpolate our position over time (don't just snap there)...

	if (m_bSetLastAngles)
	{
		// Calculate how far to interpoldate based on the current frame time (delta) and 
		// the time to interpolate over the entire distance (VehicleContourRate).
		// fInterpolationTime should always be a value between 0 and 1.0...

		float fInterpolationTime = LTCLAMP((g_pGameClientShell->GetFrameTime() / g_vtVehicleContourRate.GetFloat()), 0.0f, 1.0f);

		float fCameraYSave = vPYR.y;
		float fPlayerYSave = vPlayerPYR.y;
		float fVehicleYSave = m_vVehiclePYR.y;

		LTVector vInterpolated;
		VEC_LERP(vInterpolated, m_vLastCameraAngles, vPYR, fInterpolationTime);
		vPYR = vInterpolated;
		vPYR.y = fCameraYSave;

		VEC_LERP(vInterpolated, m_vLastPlayerAngles, vPlayerPYR, fInterpolationTime);
		vPlayerPYR = vInterpolated;
		vPlayerPYR.y = fPlayerYSave;

		VEC_LERP(vInterpolated, m_vLastVehiclePYR, m_vVehiclePYR, fInterpolationTime);
		m_vVehiclePYR = vInterpolated;
		m_vVehiclePYR.y = fVehicleYSave;
	}


	// Clamp all the values 
	
	float fClamp = g_vtVehicleContourMaxRotation.GetFloat();

	vPlayerPYR.x = Clamp( vPlayerPYR.x, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));
	vPlayerPYR.z = Clamp( vPlayerPYR.z, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));
	vPYR.x = Clamp( vPYR.x, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));
	vPYR.z = Clamp( vPYR.z, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));

	// These are relative to the camera so keep them small...

	fClamp = g_vtVehicleCamContourMaxRotation.GetFloat();

	if (g_vtVehicleContourPlayerViewModel.GetFloat())
	{
		m_vVehiclePYR.x = Clamp( m_vVehiclePYR.x, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));
		m_vVehiclePYR.z = Clamp( m_vVehiclePYR.z, MATH_DEGREES_TO_RADIANS( -fClamp ), MATH_DEGREES_TO_RADIANS( fClamp ));
	}

	m_vLastPlayerAngles = vPlayerPYR;
	m_vLastCameraAngles = vPYR;
	m_vLastVehiclePYR	= m_vVehiclePYR;
	m_bSetLastAngles	= true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleGear
//
//	PURPOSE:	Update the vehicle's current gear...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleGear()
{
	LTVector myVel = g_pMoveMgr->GetVelocity();

	// Print out info...

	
	if( s_vtVehicleInfoTrack.GetFloat() == VEHICLE_TRACK_GENERAL )
	{
		// How fast are we going anyway...
		
		LTVector vAccel;
		g_pPhysicsLT->GetAcceleration(g_pMoveMgr->GetObject(), &vAccel);
		
		g_pLTClient->CPrint("Vel = %.2f", myVel.Mag());
		g_pLTClient->CPrint("Accel = %.2f", vAccel.Mag());
		g_pLTClient->CPrint("HandleBarRoll = %.2f", m_fHandlebarRoll);
	}
		
	myVel.y = 0.0f;

	LTBOOL bUsingAccel = (LTBOOL) s_vtVehicleAccelGearChange.GetFloat();

	LTFLOAT fMaxVal  = bUsingAccel ? GetMaxAccelMag() : GetMaxVelMag();
	LTFLOAT fVal	 = bUsingAccel ? m_fVehicleBaseMoveAccel : myVel.Mag();

	LTBOOL bWasStopped = m_bVehicleStopped;

	m_bVehicleStopped = LTFALSE;
	if (fVal < fMaxVal * s_vtVehicleStoppedPercent.GetFloat())
	{
        m_bVehicleStopped = LTTRUE;

		myVel.Init();
		g_pMoveMgr->SetVelocity(myVel);
	}

	// Add a little movement to show we stopped...

	if (!bWasStopped && m_bVehicleStopped)
	{
		CameraDelta delta;
		delta.Pitch.fVar	= DEG2RAD( s_vtVehicleStopMotionPitch.GetFloat( ));
		delta.Pitch.fTime1	= 0.4f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}
	else if (bWasStopped && !m_bVehicleStopped)
	{
		// Show a little movement to show we started moving...

		CameraDelta delta;

		delta.Pitch.fVar	= DEG2RAD( s_vtVehicleStartMotionPitch.GetFloat( ));
		delta.Pitch.fTime1	= 0.25f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}

    m_bVehicleAtMaxSpeed = LTFALSE;
	if (fVal > fMaxVal * s_vtVehicleMaxSpeedPercent.GetFloat())
	{
        m_bVehicleAtMaxSpeed = LTTRUE;
	}

	if( m_bHasWeapon )
	{
		m_VehicleWeapon.SetVehicleAtFullSpeed( !!m_bVehicleAtMaxSpeed );
	}
	

	m_nLastGear = m_nCurGear;

	m_nCurGear = VEHICLE_GEAR_0;

	// Update the handlebars...

	UpdateVehicleHandleBars();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleHandleBars
//
//	PURPOSE:	Update the vehicle handlebars
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleHandleBars()
{
	if (!m_hVehicleModel) return;

	// Reset all our nodes matrices

	m_VehicleNode1.matTransform.Identity();
	m_VehicleNode2.matTransform.Identity();
	m_VehicleNode3.matTransform.Identity();

	// Do the rotation on the handle bar node
	//
    LTRotation rRot;
    rRot.Init();
    LTVector vAxis(0.0f, 1.0f, 0.0f);

	// Negate the roll for some reason
	rRot.Rotate(vAxis, -m_fHandlebarRoll);

	// Create a rotation matrix and apply it to the current offset matrix

	LTMatrix m1;
	rRot.ConvertToMatrix(m1);
	m_VehicleNode1.matTransform = m_VehicleNode1.matTransform * m1;


	// Do the rotation on the speedometer node...
	
    LTFLOAT fMaxRot = MATH_DEGREES_TO_RADIANS(270.0f);

	// Calculate the speed dial rotation...

	LTVector vVel = g_pMoveMgr->GetVelocity();

	LTFLOAT fMaxVel = GetMaxVelMag();
	LTFLOAT fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	LTFLOAT fPercent = 1.0f - ((fMaxVel - fCurVel) / fMaxVel);

	LTFLOAT fCurRot = fPercent*fMaxRot;
	
	rRot.Init();
	vAxis.Init(0.0f, 0.0f, 1.0f);
	rRot.Rotate(vAxis, -fCurRot);

	// Create a rotation matrix and apply it to the current offset matrix

	rRot.ConvertToMatrix(m1);
	m_VehicleNode2.matTransform = m_VehicleNode2.matTransform * m1;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::AdjustRoll
//
//	PURPOSE:	Adjust the camera roll if necessary
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::AdjustCameraRoll(LTFLOAT & fRoll)
{
	// Only adjust if we are on a vehicle...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		// Don't adjust camera when selecting/deselecting the vehicle...

		LTBOOL bIsDone = LTFALSE;
		if (IsCurVehicleAni("Deselect", bIsDone) || IsCurVehicleAni("Select", bIsDone))
		{
			return;
		}

		LTFLOAT fMaxCant = DEG2RAD(g_vtMaxVehicleHeadCant.GetFloat());

		LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());
		LTFLOAT fYawDiff = (LTFLOAT)fabs(m_fYawDiff);
		LTFLOAT fPercent = fYawDiff / fMaxOffset;

		if (m_fYawDiff > 0.0f)
		{
			fPercent = -fPercent;
		}

		// Save off roll value for updating handlebars...

		m_fHandlebarRoll = fMaxCant * fPercent;


		// Update head can't only when moving...

		if (!m_bVehicleStopped)
		{
			fRoll = m_fHandlebarRoll;

			// Move head left or right depending on amount we are turning...

			m_vHeadOffset.x = g_vtVehicleMaxHeadOffsetX.GetFloat() * fPercent;
			m_vHeadOffset.y = g_vtVehicleMaxHeadOffsetY.GetFloat() * fPercent;
			m_vHeadOffset.z = g_vtVehicleMaxHeadOffsetZ.GetFloat() * fPercent;
			m_fHeadYaw = DEG2RAD(g_vtVehicleMaxHeadYaw.GetFloat() * fPercent);
		}
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveLocalSolidObject
//
//	PURPOSE:	Handle the moving of the object if we need to.
//
// ----------------------------------------------------------------------- //
bool CVehicleMgr::MoveLocalSolidObject( )
{
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		// These guys don't handle movement.
		case PPM_NORMAL :
		case PPM_VEHICLE :
			return false;
			break;

		case PPM_LURE:
			return MoveToLure( );
			break;

		default:
			ASSERT( !"CVehicleMgr::MoveLocalSolidObject: Invalid physics model." );
			return false;
			break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveToLure
//
//	PURPOSE:	Handle the moving of the object if we need to.
//
// ----------------------------------------------------------------------- //
bool CVehicleMgr::MoveToLure( )
{
	// Get the playerlurefx.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing PlayerLureFX." );
		return false;
	}

	// Get the server lure object.
	HOBJECT hServerObj = pPlayerLureFX->GetServerObj( );
	if( !hServerObj )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing server-side player object." );
		return false;
	}

	// Get the client player object.
	HOBJECT hObj = g_pMoveMgr->GetObject( );
	if( !hObj )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing client-side player object." );
		return false;
	}

	// Our resulting pos and rotation.
	LTVector vPos;
	LTRotation rRot;

	// Apply offset if told to.
	if( pPlayerLureFX->GetRetainOffsets( ))
	{
		// Get the playerlure's transform.
		LTransform playerLureTransform;
		g_pLTClient->GetObjectPos( hServerObj, &playerLureTransform.m_Pos );
		g_pLTClient->GetObjectRotation( hServerObj, &playerLureTransform.m_Rot );
		playerLureTransform.m_Scale.Init( 1.0f, 1.0f, 1.0f );

		// Get the offset transform.
		LTransform offsetTransform = pPlayerLureFX->GetOffsetTransform( );

		// Find the offset position and rotation.
		LTransform playerTransform;
	    ILTTransform *pTransformLT = g_pLTClient->GetTransformLT();
		pTransformLT->Multiply( playerTransform, playerLureTransform, offsetTransform );

		// Stuff the result.
		vPos = playerTransform.m_Pos;
		rRot = playerTransform.m_Rot;
	}
	// Not retaining offsets.
	else
	{
		// Get the playerlure's position.
		g_pLTClient->GetObjectPos( hServerObj, &vPos );

		// Get the playerlure's rotation.
		g_pLTClient->GetObjectRotation( hServerObj, &rRot );
	}

	g_pPhysicsLT->MoveObject( hObj, &vPos, MOVEOBJECT_TELEPORT );
	g_pLTClient->SetObjectRotation( hObj, &rRot );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::GetVehicleMovementPercent
//
//  PURPOSE:	Calculate the percentage from our max velocity...
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetVehicleMovementPercent() const
{
	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	LTFLOAT fMaxVel = GetMaxVelMag();
	LTFLOAT fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	if (fMaxVel > 0.0f)
	{
		return 1.0f - ((fMaxVel - fCurVel) / fMaxVel);
	}

	return 0.0f;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::HandleJumpVolumeTouch
//
//  PURPOSE:	Handle hitting a JumpVolume...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::HandleJumpVolumeTouch( HOBJECT hJumpVolume )
{
	if( !hJumpVolume ) return false;

	// We only want one JumpVolume to effect us at a time...
	
	if( m_hCurJumpVolume )
		return true;
	
	// Grab the SpecialFX associated with this object...
	
	CJumpVolumeFX *pFX = (CJumpVolumeFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_JUMPVOLUME_ID, hJumpVolume );
	if( pFX )
	{
			
		// Dont add the full velocity amount if we aren't at the max...
				
		LTRotation	rRot;
		g_pLTClient->GetObjectRotation( g_pMoveMgr->GetObject(), &rRot );
		
		LTVector	vF = rRot.Forward();
		vF.y = 0.0f;
		vF.Normalize();

		LTVector	vDir = pFX->GetVelocity();
		vDir.y = 0.0f;
		vDir.Normalize();
		
		float	fDot = rRot.Forward().Dot( vDir );

		// Don't let the JumpVolume effct us if we're heading in the opposite direction...

		if( fDot <= 0.001f )
			return true;

		// Adjust the percentage based on the min and max values...

		float	fDirMax = g_vtJumpVolumeMaxDirectionPercent.GetFloat();
		float	fDirMin	= g_vtJumpVolumeMinDirectionPercent.GetFloat();
		float	fT = 0.0f;
		
		if( fDot >= fDirMax )
		{
			// Give us full amount
			fT = 1.0f;
		}
		else if( fDot <= fDirMin )
		{
			// Don't let it affect us at all
			return true;
		}
		else
		{
			fT = (fDot - fDirMin) / (1.0f - fDirMin);
		}

		WaveType	eType = (WaveType)(int)g_vtJumpVolumeWaveType.GetFloat();
		float		fPercent = GetVehicleMovementPercent() * GetWaveFn(eType)(fT); 

		if( g_vtJumpVolumeInfo.GetFloat() > 0.0f )
		{
			g_pLTClient->CPrint( "Movement: %.4f Dot: %.4f T: %.4f Wave: %.3f", GetVehicleMovementPercent(), fDot, fT, GetWaveFn(eType)(fT) );
			g_pLTClient->CPrint( "Percent: %.4f", fPercent );
		}
		
		LTVector vJumpVolumeVel = g_pMoveMgr->GetVelocity() + (pFX->GetVelocity() * fPercent);
		g_pMoveMgr->SetVelocity( vJumpVolumeVel );

		m_fJumpVolumeVel = vJumpVolumeVel.Mag();
		
		// The vehicle is now being affected by a JumpVolume...

		m_hCurJumpVolume		= hJumpVolume;
		m_bHaveJumpVolumeVel	= LTTRUE;

		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::UpdateInJumpVolume
//
//  PURPOSE:	Detrimines if we are still within a JumpVolume...
//				This is really only used so a JumpVolume doesn't keep 
//				adding it's velocity if we intersect it on multiple frames.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateInJumpVolume()
{
	// Make sure we are in a JumpVolume...

	if( !m_hCurJumpVolume )
		return;

	HOBJECT hObj = g_pMoveMgr->GetObject();

	LTVector	vJVPos, vObjPos, vJVDims, vObjDims;
	LTVector	vJVMin, vJVMax, vObjMin, vObjMax;

	g_pLTClient->GetObjectPos( m_hCurJumpVolume, &vJVPos );
	g_pLTClient->GetObjectPos( hObj, &vObjPos );

	g_pPhysicsLT->GetObjectDims( m_hCurJumpVolume, &vJVDims );
	g_pPhysicsLT->GetObjectDims( hObj, &vObjDims );

	// Setup the extents...
	
	vJVMin	= vJVPos - vJVDims;
	vJVMax	= vJVPos + vJVDims;
	vObjMin	= vObjPos - vObjDims;
	vObjMax = vObjPos + vObjDims;

	// If the boxes arent intersecting then we are no longer in the JumpVolume...

	if( !BoxesIntersect( vJVMin, vJVMax, vObjMin, vObjMax ))
		m_hCurJumpVolume = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::HandleCharacterTouch
//
//  PURPOSE:	Handle hitting a characer...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::HandleCharacterTouch( HOBJECT hCharacter )
{
	if( !hCharacter ) return false;

	LTFLOAT fVelPercent = GetVehicleMovementPercent();
	LTFLOAT fDamage		= g_vtVehicleImpactAIMinDamage.GetFloat();
	LTFLOAT fMaxDamage	= g_vtVehicleImpactAIMaxDamage.GetFloat();
	LTFLOAT fDamRange	= fMaxDamage - fDamage;

	fDamage += (fDamRange * fVelPercent);

	LTVector vPos;
	g_pLTClient->GetObjectPos(hCharacter, &vPos);

	// Get our forward vector...

	HOBJECT hObj = g_pMoveMgr->GetObject();
	LTRotation rRot;
	g_pLTClient->GetObjectRotation(hObj, &rRot);
	
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
	cMsg.Writeuint8(CP_DAMAGE_VEHICLE_IMPACT);
	cMsg.Writeuint8(DT_EXPLODE);
	cMsg.Writefloat(fDamage);
	cMsg.WriteLTVector(rRot.Forward());
	cMsg.WriteLTVector(vPos);
	cMsg.Writeuint8(0);
	cMsg.WriteObject(hCharacter);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	if (fDamage > (g_vtVehicleImpactAIMaxDamage.GetFloat() * 0.75f))
	{
		// Pop a little wheely...

		CameraDelta delta;
		delta.Pitch.fVar	= -DEG2RAD(5.0f);
		delta.Pitch.fTime1	= 0.1f;
		delta.Pitch.fTime2	= 0.3f;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		g_pPlayerMgr->GetCameraOffsetMgr()->AddDelta(delta);
	}

	// Should we continue processing the collision?

	if (!g_vtVehicleImpactAIDoNormalCollision.GetFloat())
	{
		return true;
	}
	
	return false;
}


LTBOOL CVehicleMgr::OnCommandOn(int command)
{
	return LTFALSE;
}

LTBOOL CVehicleMgr::OnCommandOff(int command)
{
	return LTFALSE;
}

void CVehicleMgr::AlignAngleToAxis(float &fAngle)
{
	// First make sure the yaw is in the 0 - 2pi range
	if(fAngle < 0)
	{
		fAngle = fAngle + (((float)((int)(fAngle/MATH_CIRCLE))+1)*MATH_CIRCLE);
	}
	else if(fAngle > MATH_CIRCLE)
	{
		fAngle = fAngle - (((float)((int)(fAngle/MATH_CIRCLE))+1)*MATH_CIRCLE);
	}

	// Now axis align it
	if(fAngle <= (MATH_PI/4.0f)) // PI/4
	{
		fAngle = 0;
	}
	else if(fAngle <= (MATH_PI*0.75f)) // 3PI/4
	{
		fAngle = MATH_HALFPI;
	}
	else if(fAngle <= (MATH_PI*1.25)) // 5PI/4
	{
		fAngle = MATH_PI;
	}
	else if(fAngle <= (MATH_PI*1.75)) // 7PI/4
	{
		fAngle = MATH_HALFPI*3.0f;
	}
	else
	{
		fAngle = 0;
	}

	/***********************
	Just in case, this code does the same thing as above but uses a forward 
	vector instead of an angle

		vF.y = 0;
		LTVector vAxis(0.0f,0.0f,0.0f);
		float fTheta;

		// Figure out which quadrant it's in
		if(vF.x >= 0.0f && vF.z >= 0.0f)
		{
			vAxis.z = 1.0f; // 12 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.x = 1.0f; // 3 o'clock
			}
			else
			{
				vMoveVel.z = 1.0f; // 12 o'clock
			}
		}
		else if(vF.x >= 0.0f && vF.z <= 0.0f)
		{
			vAxis.x = 1.0f; // 3 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.z = -1.0f; // 6 o'clock
			}
			else
			{
				vMoveVel.x = 1.0f; // 3 o'clock
			}
		}
		else if(vF.x <= 0.0f && vF.z >= 0.0f)
		{
			vAxis.z = -1.0f; // 6 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.x = -1.0f; // 9 o'clock
			}
			else
			{
				vMoveVel.z = -1.0f; // 6 o'clock
			}
		}
		else // (vF.x <= 0.0f && vF.z <= 0.0f)
		{
			vAxis.x = -1.0f; // 9 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.z = 1.0f; // 12 o'clock
			}
			else
			{
				vMoveVel.x = -1.0f; // 9 o'clock
			}
		}
	***********************/
}

void CVehicleMgr::UpdateContainerMotion()
{
	// Get the containerinfo from movemgr update the gravity and viscosity...

	g_pMoveMgr->UpdateContainerList();

	uint32 nNumContainers = g_pMoveMgr->GetNumContainers();
	CContainerInfo *pInfo = LTNULL;

	for( uint32 i = 0; i < nNumContainers; ++i )
	{
		pInfo = g_pMoveMgr->GetContainerInfo( i );
		if( !pInfo )
			continue;

		g_pMoveMgr->UpdateContainerViscosity( pInfo );
		g_pMoveMgr->UpdateContainerGravity( pInfo );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveVehicleObject
//
//	PURPOSE:	Allows the vehicle to do some extra handeling of moving the object...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::MoveVehicleObject( LTVector &vPos )
{ 
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		case PPM_VEHICLE :
		{
			HOBJECT hObj = g_pMoveMgr->GetObject();

			// Remember where we started
			LTVector vStartPos;
			g_pLTClient->GetObjectPos( hObj, &vStartPos );

			// We haven't been touched yet...
			m_bWasTouched = false;

			// First do movement normally to get collision information...
			
			g_pPhysicsLT->MoveObject( hObj, &vPos, 0 );

			// If we didn't get touched, we're done
			if (!m_bWasTouched)
				break;

			// Check where we ended up
			LTVector vResultPos;
			g_pLTClient->GetObjectPos( hObj, &vResultPos );

			// If we made it to our destination anyway, we're done
			if( vResultPos == vPos )
				break;

			// If we didn't make it far enough toward the destination, it was a collision
			LTVector vDesiredOfs = vPos - vStartPos;
			LTVector vActualOfs = vResultPos - vStartPos;

			float fDesiredMag = vDesiredOfs.Mag();
			if ((vActualOfs.Dot(vDesiredOfs) / fDesiredMag) < (fDesiredMag * g_vtVehicleImpactThreshold.GetFloat()))
				HandleCollision();
		}
		break;

		case PPM_NORMAL:
		case PPM_LURE:
		{
			// Simple movement call...

			g_pPhysicsLT->MoveObject( g_pMoveMgr->GetObject(), &vPos, 0 );
		}
		break;

		default:
			ASSERT( !"CVehicleMgr::MoveLocalSolidObject: Invalid physics model." );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveVehicleObject
//
//	PURPOSE:	Allows the vehicle to do some extra handeling of moving the object...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPlayerLureId( uint32 nPlayerLureId )
{
	m_nPlayerLureId = nPlayerLureId; 
	m_bResetLure = true; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::Save
//
//	PURPOSE:	Serialization
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	// Saving this flag is fine since it doesn't deal with any actualy physics info
	// and is needed so we know if we should holster the current weapon when the 
	// physics model gets reset...

	pMsg->Writebool( m_bHolsteredWeapon );

	// If we are following a player lure we need to save the original offset transform...
	if( m_ePPhysicsModel == PPM_LURE )
	{
		// Get the playerlurefx object.
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
		if( pPlayerLureFX )
		{
			const LTransform &offset = pPlayerLureFX->GetOffsetTransform();

			pMsg->Writebool( true );
			pMsg->WriteLTVector( offset.m_Pos );
			pMsg->WriteLTRotation( offset.m_Rot );
			pMsg->WriteLTVector( offset.m_Scale );
		}
		else
		{
			pMsg->Writebool( false );
		}
	}
	else
	{
		pMsg->Writebool( false );
	}
	
// IMPORTANT NOTE!!! 
// Serialization of VehicleMgr is very bad, since the rest of the code assumes it's going
// to have to reset the state of the player physics model
/*
	pMsg->Writeuint32(m_ePPhysicsModel);

	pMsg->Writefloat(m_fAccelStart);
	pMsg->Writefloat(m_fDecelStart);
	pMsg->Writeuint32(m_dwControlFlags);
	pMsg->Writeuint32(m_dwLastControlFlags);
	pMsg->Writefloat(m_fVehicleBaseMoveAccel);

	pMsg->Writeint32(m_nCurGear);
	pMsg->Writeint32(m_nLastGear);
	pMsg->Writebool(m_bTurned);

	pMsg->Writebool(m_bVehicleStopped != LTFALSE);
	pMsg->Writebool(m_bVehicleAtMaxSpeed != LTFALSE);
	pMsg->Writebool(m_bVehicleTurning != LTFALSE);
	pMsg->Writeint32(m_nVehicleTurnDirection);

	pMsg->WriteLTVector(m_vVehiclePYR);

	pMsg->Writefloat(m_fContourRoll);

	pMsg->Writeuint32(m_eSurface);
	pMsg->Writeuint32(m_eLastSurface);

	pMsg->Writefloat(m_fYawDiff);
	pMsg->Writefloat(m_fYawDelta);
	pMsg->Writefloat(m_fHandlebarRoll);
	pMsg->Writefloat(m_fHeadYaw);

	pMsg->Writeuint32(m_nPlayerLureId);

	pMsg->Writebool(m_bHaveJumpVolumeVel != LTFALSE);
	pMsg->Writefloat(m_fJumpVolumeVel);

	pMsg->Writebool(m_bSetLastAngles);
	pMsg->WriteLTVector(m_vLastVehiclePYR);
	pMsg->WriteLTVector(m_vLastPlayerAngles);
	pMsg->WriteLTVector(m_vLastCameraAngles);
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::Load
//
//	PURPOSE:	Serialization
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState)
{
	m_bHolsteredWeapon = pMsg->Readbool();

	m_bResetLureFromSave = pMsg->Readbool();
	if( m_bResetLureFromSave )
	{
		m_ResetLureTransform.m_Pos		= pMsg->ReadLTVector();
		m_ResetLureTransform.m_Rot		= pMsg->ReadLTRotation();
		m_ResetLureTransform.m_Scale	= pMsg->ReadLTVector();
	}

// IMPORTANT NOTE!!! 
// Serialization of VehicleMgr is very bad, since the rest of the code assumes it's going
// to have to reset the state of the player physics model
/*
	SetPhysicsModel((PlayerPhysicsModel)pMsg->Readuint32());

	m_fAccelStart = pMsg->Readfloat();
	m_fDecelStart = pMsg->Readfloat();
	m_dwControlFlags = pMsg->Readuint32();
	m_dwLastControlFlags = pMsg->Readuint32();
	m_fVehicleBaseMoveAccel = pMsg->Readfloat();

	m_nCurGear = pMsg->Readuint32();
	m_nLastGear = pMsg->Readuint32();
	m_bTurned = pMsg->Readbool();

	m_bVehicleStopped = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bVehicleAtMaxSpeed = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_bVehicleTurning = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_nVehicleTurnDirection = pMsg->Readuint32();

	m_vVehiclePYR = pMsg->ReadLTVector();

	m_fContourRoll = pMsg->Readfloat();

	m_eSurface = (SurfaceType)pMsg->Readuint32();
	m_eLastSurface = (SurfaceType)pMsg->Readuint32();

	m_fYawDiff = pMsg->Readfloat();
	m_fYawDelta = pMsg->Readfloat();
	m_fHandlebarRoll = pMsg->Readfloat();
	m_fHeadYaw = pMsg->Readfloat();

	m_nPlayerLureId = pMsg->Readuint32();

	m_bHaveJumpVolumeVel = pMsg->Readbool() ? LTTRUE : LTFALSE;
	m_fJumpVolumeVel = pMsg->Readfloat();

	m_bSetLastAngles = pMsg->Readbool();
	m_vLastVehiclePYR = pMsg->ReadLTVector();
	m_vLastPlayerAngles = pMsg->ReadLTVector();
	m_vLastCameraAngles = pMsg->ReadLTVector();
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SaveVehicleInfo
//
//	PURPOSE:	Write the console variables to the vehicle bute file.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SaveVehicleInfo()
{
	switch( m_ePPhysicsModel )
	{
		case PPM_VEHICLE :
		{
			if( !m_pVehicle )
				break;

			// Set the vehicle data to the console var data...

			m_pVehicle->fVel					= s_vtVehicleVel.GetFloat( );
			m_pVehicle->fAccel					= s_vtVehicleAccel.GetFloat( );
			m_pVehicle->fAccelTime				= s_vtVehicleAccelTime.GetFloat( );
			m_pVehicle->fMaxDecel				= s_vtVehicleMaxDecel.GetFloat( );
			m_pVehicle->fDecelTime				= s_vtVehicleDecelTime.GetFloat( );
			m_pVehicle->fMaxBrake				= s_vtVehicleMaxBrake.GetFloat( );
			m_pVehicle->fBrakeTime				= s_vtVehicleBrakeTime.GetFloat( );
			m_pVehicle->fStoppedPercent			= s_vtVehicleStoppedPercent.GetFloat( );
			m_pVehicle->fMaxSpeedPercent		= s_vtVehicleMaxSpeedPercent.GetFloat( );
			m_pVehicle->fMinTurnAccel			= s_vtVehicleMinTurnAccel.GetFloat( );
			m_pVehicle->fTurnRate				= s_vtVehicleTurnRate.GetFloat( );
			m_pVehicle->fInitialTurnRate		= s_vtVehicleInitialTurnRate.GetFloat( );
			m_pVehicle->fSurfaceSpeedUpPitch	= s_vtVehicleSurfaceSpeedUpPitch.GetFloat( );
			m_pVehicle->fSurfaceSlowDownPitch	= s_vtVehicleSurfaceSlowDownPitch.GetFloat( );
			m_pVehicle->fStartMotionPitch		= s_vtVehicleStartMotionPitch.GetFloat( );
			m_pVehicle->fStopMotionPitch		= s_vtVehicleStopMotionPitch.GetFloat( );
			m_pVehicle->fBrakePitch				= s_vtVehicleBrakePitch.GetFloat( );
			m_pVehicle->fDecelPitch				= s_vtVehicleDecelPitch.GetFloat( );

			m_pVehicle->vOffset				= m_vVehicleOffset;

			// Write the vehicle to the bute file...

			char szMsg[64] = {0};
			
			if( g_pVehicleButeMgr->SaveToFile( m_pVehicle ))
			{
				LTSNPrintF( szMsg, ARRAY_LEN( szMsg ), "Saved vehicle %s", m_pVehicle->sName.c_str() );
			}
			else
			{
				LTSNPrintF( szMsg, ARRAY_LEN( szMsg ), "ERROR Saving vehicle %s", m_pVehicle->sName.c_str() );
			}
			
			g_pChatMsgs->AddMessage( szMsg,kMsgCheatConfirm );
		}
		break;

		case PPM_LURE :
		case PPM_NORMAL:
		default:
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleMgr::UpdateWeaponModel()
//
//	PURPOSE:	Update the vehicle weapon
//
// ----------------------------------------------------------------------- //

WeaponState CVehicleMgr::UpdateVehicleWeapon( LTRotation const &rRot, LTVector const &vPos,
												bool bFire, FireType eFireType /* = FT_NORMAL_FIRE */ )
{
	if( !m_bHasWeapon )
		return W_INACTIVE;

	// Update the vehicle weapon...
	
	m_VehicleWeapon.SetCameraInfo( rRot, vPos );
	WeaponState eWeaponState = m_VehicleWeapon.Update( bFire, eFireType );

	return eWeaponState;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleMgr::OnModelKey()
//
//	PURPOSE:	Handle any model string keys...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::OnModelKey( HLOCALOBJ hObj, ArgList *pArgs )
{
	if( m_bHasWeapon )
	{
		// Send to the vehicle weapon to handle...

		return m_VehicleWeapon.OnModelKey( hObj, pArgs );
	}
	
	return false;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleMgr::CanShowCrosshair()
//
//	PURPOSE:	Decide if the vehicle should display the crosshair...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::CanShowCrosshair() const
{
	switch( m_ePPhysicsModel )
	{
		case PPM_VEHICLE :
		{
			if( !m_pVehicle )
				return false;

			return m_pVehicle->bCanShowCrosshair;
		}
		break;

		case PPM_LURE :
		case PPM_NORMAL :
		default :
			return true;

	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleMgr::AllowHeadBobCant()
//
//	PURPOSE:	Allow headbobmgr to adjust head bob and canting.
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::AllowHeadBobCant() const
{
	switch( m_ePPhysicsModel )
	{
		case PPM_LURE :
			return false;

		case PPM_NORMAL :
		default :
			return true;

	}
}
