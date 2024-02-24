//*********************************************************************************
//*********************************************************************************
// Project:		Aliens vs. Predator 2
// Purpose:		Retrieves attributes from the CharacterButes.txt
//*********************************************************************************
// File:		CharacterButeMgr.cpp
// Created:		Jan 24, 2000
// Updated:		May 29, 2000
// Author:		Andy Mattingly
//*********************************************************************************
//*********************************************************************************

#include "stdafx.h"
#include "CharacterButeMgr.h"
#include "ModelButeMgr.h"
#include "SurfaceMgr.h"
#include "WeaponMgr.h"

#ifdef _CLIENTBUILD
	#include "GameClientShell.h"
#else
	#include "GameServerShell.h"
#endif

//*********************************************************************************
// These may be externed in AIButeMgr.cpp.

const char *  CHARACTER_BUTES_TAG						= "Character";

const char *  CHARACTER_IGNORE_SET						= "IgnoreSet";
const char *  CHARACTER_CLASS_TYPE						= "Class";
const char *  CHARACTER_BUTE_MODEL_TYPE					= "ModelType";

const char *  CHARACTER_BUTE_DEFAULT_MODEL_DIR			= "DefaultModelDir";
const char *  CHARACTER_BUTE_DEFAULT_MODEL				= "DefaultModel";
const char *  CHARACTER_BUTE_DEFAULT_SKIN_DIR			= "DefaultSkinDir";
const char *  CHARACTER_BUTE_DEFAULT_SKIN				= "DefaultSkin%d";
const char *  CHARACTER_BUTE_WEAPON_SKIN_TYPE			= "WeaponSkinType";
const char *  CHARACTER_BUTE_ANIMLAYER					= "AnimLayer%d";
const char *  CHARACTER_BUTE_MPANIMLAYER				= "MPAnimLayer%d";
const char *  CHARACTER_BUTE_DEFAULT_DIMS				= "DefaultDims";
const char *  CHARACTER_BUTE_DEFAULT_MASS				= "DefaultMass";
const char *  CHARACTER_BUTE_CAMERA_HEIGHT_PERCENT		= "CameraHeightPercent";
const char *  CHARACTER_BUTE_SURFACE_TYPE				= "SurfaceType";
const char *  CHARACTER_BUTE_SKELETON_NAME				= "SkeletonName";
const char *  CHARACTER_BUTE_WEAPON_SET					= "WeaponSet";
const char *  CHARACTER_BUTE_MPWEAPON_SET				= "MPWeaponSet";
const char *  CHARACTER_BUTE_MPCLASSWEAPON_SET			= "MPClassWeaponSet";
const char *  CHARACTER_BUTE_MPWEAPON_STRING_RES		= "MPWeaponStringRes";
const char *  CHARACTER_BUTE_VISION_SET					= "VisionSet";
const char *  CHARACTER_BUTE_HUD_TYPE					= "HUDType";
const char *  CHARACTER_BUTE_DEFAULT_POUNCING_DIMS		= "PouncingDims";

const char *  CHARACTER_BUTE_MULTIPLAYER_MODEL			= "MultiplayerModel";
const char *  CHARACTER_BUTE_DISPLAY_NAME				= "DisplayName";
const char *  CHARACTER_BUTE_SCORING_CLASS				= "ScoringClass";
const char *  CHARACTER_BUTE_SELECT_ANIM				= "SelectAnim";
const char *  CHARACTER_BUTE_SELECT_POS					= "SelectPos";
const char *  CHARACTER_BUTE_SELECT_ROT					= "SelectRot";
const char *  CHARACTER_BUTE_SELECT_SCALE				= "SelectScale";

const char *  CHARACTER_BUTE_CHEST_OFFSET_PERCENT		= "ChestOffsetPercent";
const char *  CHARACTER_BUTE_HEAD_OFFSET_PERCENT		= "HeadOffsetPercent";

const char *  CHARACTER_BUTE_FOV						= "FOV";
const char *  CHARACTER_BUTE_AIM_RANGE					= "AimRange";
const char *  CHARACTER_BUTE_HEAD_TILT_ANGLE			= "HeadTiltAngle";
const char *  CHARACTER_BUTE_HEAD_TILT_SPEED			= "HeadTiltSpeed";

const char *  CHARACTER_BUTE_WALK_SPEED					= "WalkSpeed";
const char *  CHARACTER_BUTE_RUN_SPEED					= "RunSpeed";
const char *  CHARACTER_BUTE_NORM_JUMP_SPEED			= "NormJumpSpeed";
const char *  CHARACTER_BUTE_RUN_JUMP_SPEED				= "RunJumpSpeed";
const char *  CHARACTER_BUTE_SPRING_JUMP_SPEED			= "SpringJumpSpeed";
const char *  CHARACTER_BUTE_POUNCE_SPEED				= "PounceSpeed";
const char *  CHARACTER_BUTE_CROUCH_SPEED				= "CrouchSpeed";
const char *  CHARACTER_BUTE_LIQUID_SPEED				= "LiquidSpeed";
const char *  CHARACTER_BUTE_LADDER_SPEED				= "LadderSpeed";
const char *  CHARACTER_BUTE_CAN_CLIMB					= "CanClimb";
const char *  CHARACTER_BUTE_CAN_LOSE_LIMB				= "AllowLimbLoss";

const char *  CHARACTER_BUTE_FALL_DIST_SAFE				= "FallDistSafe";
const char *  CHARACTER_BUTE_FALL_DIST_DEATH			= "FallDistDeath";

const char *  CHARACTER_BUTE_WALKING_TURN_RATE			= "WalkingTurnRate";
const char *  CHARACTER_BUTE_RUNNING_TURN_RATE			= "RunningTurnRate";

const char *  CHARACTER_BUTE_WALKING_AIM_RATE			= "WalkingAimRate";
const char *  CHARACTER_BUTE_RUNNING_AIM_RATE			= "RunningAimRate";

const char *  CHARACTER_BUTE_WALKING_KICKBACK			= "WalkingKickBack";
const char *  CHARACTER_BUTE_RUNNING_KICKBACK			= "RunningKickBack";

const char *  CHARACTER_BUTE_BASE_STUN					= "BaseStunChance";

const char *  CHARACTER_BUTE_GROUND_ACCEL				= "GroundAccel";
const char *  CHARACTER_BUTE_AIR_ACCEL					= "AirAccel";
const char *  CHARACTER_BUTE_LIQUID_ACCEL				= "LiquidAccel";

const char *  CHARACTER_BUTE_AI_POP_DIST				= "AIPopDist";

const char *  CHARACTER_BUTE_JUMP_DIRECTION				= "JumpDirection";

const char *  CHARACTER_BUTE_WALL_WALK					= "CanWallWalk";

const char *  CHARACTER_BUTE_ACTIVATE_DISTANCE			= "ActivateDistance";

const char *  CHARACTER_BUTE_EMP_FLAGS					= "EMPFlags";

const char *  CHARACTER_BUTE_DEFAULT_HIT_POINTS			= "DefaultHitPoints";
const char *  CHARACTER_BUTE_MAX_HIT_POINTS				= "MaxHitPoints";
const char *  CHARACTER_BUTE_DEFAULT_ARMOR_POINTS		= "DefaultArmorPoints";
const char *  CHARACTER_BUTE_MAX_ARMOR_POINTS			= "MaxArmorPoints";
const char *  CHARACTER_BUTE_DAMAGE_RESISTANCE			= "DamageResistance";

const char *  CHARACTER_BUTE_HEAD_BITE_HEAL_POINTS		= "HeadBiteHealPoints";
const char *  CHARACTER_BUTE_CLAW_HEAL_POINTS			= "ClawHealPoints";

const char *  CHARACTER_BUTE_AIR_SUPPLY_TIME			= "AirSupplyTime";

const char *  CHARACTER_BUTE_RECOIL_MIN_DAMAGE			= "RecoilMinDamage";
const char *  CHARACTER_BUTE_RECOIL_CHANCE				= "RecoilChance";
const char *  CHARACTER_BUTE_RECOIL_DELAY				= "RecoilDelay";

const char *  CHARACTER_BUTE_ZOOM_LEVEL					= "ZoomLevel";

const char *  CHARACTER_BUTE_FOOTSTEP_SOUND				= "FootStepSoundDir";
const char *  CHARACTER_BUTE_FOOTSTEP_VOLUME			= "FootStepVolume";
const char *  CHARACTER_BUTE_GENERAL_SOUND				= "GeneralSoundDir";

const char *  CHARACTER_BUTE_AI_ANIM_TYPE				= "AIAnimType";

const char *  CHARACTER_BUTE_CAN_BE_NETTED				= "CanBeNetted";
const char *  CHARACTER_BUTE_CAN_BE_ONFIRE				= "CanBeOnFire";
const char *  CHARACTER_BUTE_CAN_BE_FACEHUGGED			= "CanBeFacehugged";
const char *  CHARACTER_BUTE_CAN_USE_LOOKAT				= "CanUseLookAt";

const char *  CHARACTER_BUTE_CAN_POUNCE					= "CanPounce";
const char *  CHARACTER_BUTE_POUNCE_WEAPON				= "PounceWeapon";
const char *  CHARACTER_BUTE_POUNCE_WEAPON_DAMAGE		= "PounceWeaponDamage";

const char *  CHARACTER_BUTE_HIT_BOX_SCALE				= "HitBoxScale";

//*********************************************************************************

CCharacterButeMgr* g_pCharacterButeMgr = DNULL;

//*********************************************************************************

static char s_aTagName[30];

//*********************************************************************************
// Saving and loading of the character bute structure
void t_CharacterButes::Write(HMESSAGEWRITE hWrite)
{
	int i = 0;

	g_pInterface->WriteToMessageDWord(hWrite, m_nId);

	g_pInterface->WriteToMessageByte(hWrite, m_eCharacterClass);
	g_pInterface->WriteToMessageString(hWrite, m_szParent);

	g_pInterface->WriteToMessageString(hWrite, m_szModelDir);
	g_pInterface->WriteToMessageString(hWrite, m_szSkinDir);

	g_pInterface->WriteToMessageString(hWrite, m_szName);
	g_pInterface->WriteToMessageString(hWrite, m_szModel);

	for(i = 0; i < MAX_MODEL_TEXTURES; i++)
		g_pInterface->WriteToMessageString(hWrite, m_szSkins[i]);

	g_pInterface->WriteToMessageString(hWrite, m_szWeaponSkinType);

	for(i = 0; i < MAX_ANIM_LAYERS; i++)
	{
		g_pInterface->WriteToMessageString(hWrite, m_szAnimLayers[i]);
		g_pInterface->WriteToMessageString(hWrite, m_szMPAnimLayers[i]);
	}

	g_pInterface->WriteToMessageVector(hWrite, &m_vDims);
	g_pInterface->WriteToMessageFloat(hWrite, m_fMass);
	g_pInterface->WriteToMessageFloat(hWrite, m_fCameraHeightPercent);
	g_pInterface->WriteToMessageByte(hWrite, m_nSurfaceType);
	g_pInterface->WriteToMessageDWord(hWrite, m_nSkeleton);
	g_pInterface->WriteToMessageDWord(hWrite, m_nWeaponSet);
	g_pInterface->WriteToMessageDWord(hWrite, m_nMPWeaponSet);
	g_pInterface->WriteToMessageDWord(hWrite, m_nMPClassWeaponSet);
	g_pInterface->WriteToMessageDWord(hWrite, m_nMPWeaponStringRes);
	g_pInterface->WriteToMessageString(hWrite, m_szVisionSet);
	g_pInterface->WriteToMessageDWord(hWrite, m_nHUDType);
	g_pInterface->WriteToMessageVector(hWrite, &m_vPouncingDims);

	g_pInterface->WriteToMessageFloat(hWrite, m_fChestOffsetPercent);
	g_pInterface->WriteToMessageFloat(hWrite, m_fHeadOffsetPercent);

	g_pInterface->WriteToMessageDWord(hWrite, m_ptFOV.x);
	g_pInterface->WriteToMessageDWord(hWrite, m_ptFOV.y);
	g_pInterface->WriteToMessageDWord(hWrite, m_ptAimRange.x);
	g_pInterface->WriteToMessageDWord(hWrite, m_ptAimRange.y);
	g_pInterface->WriteToMessageFloat(hWrite, m_fHeadTiltAngle);
	g_pInterface->WriteToMessageFloat(hWrite, m_fHeadTiltSpeed);

	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseWalkSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseRunSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseJumpSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseRunJumpSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseSpringJumpSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBasePounceSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseCrouchSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseLiquidSpeed);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseLadderSpeed);

	g_pInterface->WriteToMessageFloat(hWrite, m_fFallDistanceSafe);
	g_pInterface->WriteToMessageFloat(hWrite, m_fFallDistanceDeath);

	g_pInterface->WriteToMessageFloat(hWrite, m_fWalkingTurnRate);
	g_pInterface->WriteToMessageFloat(hWrite, m_fRunningTurnRate);

	g_pInterface->WriteToMessageFloat(hWrite, m_fWalkingAimRate);
	g_pInterface->WriteToMessageFloat(hWrite, m_fRunningAimRate);

	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseGroundAccel);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseAirAccel);
	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseLiquidAccel);

	g_pInterface->WriteToMessageFloat(hWrite, m_fMaxAIPopDist);

	g_pInterface->WriteToMessageVector(hWrite, &m_vJumpDirection);

	g_pInterface->WriteToMessageByte(hWrite, m_bWallWalk);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanBeNetted);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanBeOnFire);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanBeFacehugged);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanUseLookAt);

	g_pInterface->WriteToMessageFloat(hWrite, m_fActivateDist);
	g_pInterface->WriteToMessageDWord(hWrite, m_nEMPFlags);
	g_pInterface->WriteToMessageFloat(hWrite, m_fWalkingKickBack);
	g_pInterface->WriteToMessageFloat(hWrite, m_fRunningKickBack);

	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseStunChance);

	g_pInterface->WriteToMessageFloat(hWrite, m_fDefaultHitPoints);
	g_pInterface->WriteToMessageFloat(hWrite, m_fMaxHitPoints);
	g_pInterface->WriteToMessageFloat(hWrite, m_fDefaultArmorPoints);
	g_pInterface->WriteToMessageFloat(hWrite, m_fMaxArmorPoints);
	g_pInterface->WriteToMessageFloat(hWrite, m_fDamageResistance);

	g_pInterface->WriteToMessageFloat(hWrite, m_fHeadBiteHealPoints);
	g_pInterface->WriteToMessageFloat(hWrite, m_fClawHealPoints);

	g_pInterface->WriteToMessageFloat(hWrite, m_fAirSupplyTime);

	g_pInterface->WriteToMessageFloat(hWrite, m_fMinRecoilDamage);
	g_pInterface->WriteToMessageFloat(hWrite, m_fRecoilChance);
	g_pInterface->WriteToMessageFloat(hWrite, m_fMinRecoilDelay);
	g_pInterface->WriteToMessageFloat(hWrite, m_fMaxRecoilDelay);

	g_pInterface->WriteToMessageFloat(hWrite, m_fHitBoxScale);

	for(i=0 ; i<MAX_ZOOM_LEVELS ; i++)
		g_pInterface->WriteToMessageFloat(hWrite, m_fZoomLevels[i]);

	g_pInterface->WriteToMessageFloat(hWrite, m_fBaseFootStepVolume);
	g_pInterface->WriteToMessageString(hWrite, m_szFootStepSoundDir);
	g_pInterface->WriteToMessageString(hWrite, m_szGeneralSoundDir);
	g_pInterface->WriteToMessageString(hWrite, m_szAIAnimType);

	g_pInterface->WriteToMessageString(hWrite, m_szPounceWeapon);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanLoseLimb);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanClimb);
	g_pInterface->WriteToMessageByte(hWrite, m_bCanPounce);
	g_pInterface->WriteToMessageDWord(hWrite, m_nPounceDamage);
}

//*********************************************************************************

void t_CharacterButes::Read(HMESSAGEREAD hRead)
{
	int i = 0;

	m_nId = g_pInterface->ReadFromMessageDWord(hRead);

	m_eCharacterClass = CharacterClass(hRead->ReadByte());
	hRead->ReadStringFL(m_szParent, sizeof(m_szParent));

	hRead->ReadStringFL(m_szModelDir, sizeof(m_szModelDir));
	hRead->ReadStringFL(m_szSkinDir, sizeof(m_szSkinDir));

	hRead->ReadStringFL(m_szName, sizeof(m_szName));
	hRead->ReadStringFL(m_szModel, sizeof(m_szModel));

	for(i = 0; i < MAX_MODEL_TEXTURES; i++)
		hRead->ReadStringFL(m_szSkins[i], sizeof(m_szSkins[i]));

	hRead->ReadStringFL(m_szWeaponSkinType, sizeof(m_szModel));

	for(i = 0; i < MAX_ANIM_LAYERS; i++)
	{
		hRead->ReadStringFL(m_szAnimLayers[i], sizeof(m_szAnimLayers[i]));
		hRead->ReadStringFL(m_szMPAnimLayers[i], sizeof(m_szMPAnimLayers[i]));
	}

	g_pInterface->ReadFromMessageVector(hRead, &m_vDims);
	m_fMass = g_pInterface->ReadFromMessageFloat(hRead);
	m_fCameraHeightPercent = g_pInterface->ReadFromMessageFloat(hRead);
	m_nSurfaceType = g_pInterface->ReadFromMessageByte(hRead);
	m_nSkeleton = g_pInterface->ReadFromMessageDWord(hRead);
	m_nWeaponSet = g_pInterface->ReadFromMessageDWord(hRead);
	m_nMPWeaponSet = g_pInterface->ReadFromMessageDWord(hRead);
	m_nMPClassWeaponSet = g_pInterface->ReadFromMessageDWord(hRead);
	m_nMPWeaponStringRes = g_pInterface->ReadFromMessageDWord(hRead);
	hRead->ReadStringFL(m_szVisionSet, sizeof(m_szVisionSet));
	m_nHUDType = g_pInterface->ReadFromMessageDWord(hRead);
	g_pInterface->ReadFromMessageVector(hRead, &m_vPouncingDims);

	m_fChestOffsetPercent = g_pInterface->ReadFromMessageFloat(hRead);
	m_fHeadOffsetPercent = g_pInterface->ReadFromMessageFloat(hRead);

	m_ptFOV.x = g_pInterface->ReadFromMessageDWord(hRead);
	m_ptFOV.y = g_pInterface->ReadFromMessageDWord(hRead);
	m_ptAimRange.x = g_pInterface->ReadFromMessageDWord(hRead);
	m_ptAimRange.y = g_pInterface->ReadFromMessageDWord(hRead);
	m_fHeadTiltAngle = g_pInterface->ReadFromMessageFloat(hRead);
	m_fHeadTiltSpeed = g_pInterface->ReadFromMessageFloat(hRead);

	m_fBaseWalkSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseRunSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseJumpSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseRunJumpSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseSpringJumpSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBasePounceSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseCrouchSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseLiquidSpeed = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseLadderSpeed = g_pInterface->ReadFromMessageFloat(hRead);

	m_fFallDistanceSafe = g_pInterface->ReadFromMessageFloat(hRead);
	m_fFallDistanceDeath = g_pInterface->ReadFromMessageFloat(hRead);

	m_fWalkingTurnRate = g_pInterface->ReadFromMessageFloat(hRead);
	m_fRunningTurnRate = g_pInterface->ReadFromMessageFloat(hRead);

	m_fWalkingAimRate = g_pInterface->ReadFromMessageFloat(hRead);
	m_fRunningAimRate = g_pInterface->ReadFromMessageFloat(hRead);

	m_fBaseGroundAccel = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseAirAccel = g_pInterface->ReadFromMessageFloat(hRead);
	m_fBaseLiquidAccel = g_pInterface->ReadFromMessageFloat(hRead);

	m_fMaxAIPopDist = g_pInterface->ReadFromMessageFloat(hRead);

	g_pInterface->ReadFromMessageVector(hRead, &m_vJumpDirection);

	m_bWallWalk = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanBeNetted = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanBeOnFire = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanBeFacehugged = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanUseLookAt = g_pInterface->ReadFromMessageByte(hRead);

	m_fActivateDist = g_pInterface->ReadFromMessageFloat(hRead);
	m_nEMPFlags = g_pInterface->ReadFromMessageDWord(hRead);
	m_fWalkingKickBack = g_pInterface->ReadFromMessageFloat(hRead);
	m_fRunningKickBack = g_pInterface->ReadFromMessageFloat(hRead);

	m_fBaseStunChance = g_pInterface->ReadFromMessageFloat(hRead);

	m_fDefaultHitPoints = g_pInterface->ReadFromMessageFloat(hRead);
	m_fMaxHitPoints = g_pInterface->ReadFromMessageFloat(hRead);
	m_fDefaultArmorPoints = g_pInterface->ReadFromMessageFloat(hRead);
	m_fMaxArmorPoints = g_pInterface->ReadFromMessageFloat(hRead);
	m_fDamageResistance = g_pInterface->ReadFromMessageFloat(hRead);

	m_fHeadBiteHealPoints = g_pInterface->ReadFromMessageFloat(hRead);
	m_fClawHealPoints = g_pInterface->ReadFromMessageFloat(hRead);

	m_fAirSupplyTime = g_pInterface->ReadFromMessageFloat(hRead);

	m_fMinRecoilDamage = g_pInterface->ReadFromMessageFloat(hRead);
	m_fRecoilChance = g_pInterface->ReadFromMessageFloat(hRead);
	m_fMinRecoilDelay = g_pInterface->ReadFromMessageFloat(hRead);
	m_fMaxRecoilDelay = g_pInterface->ReadFromMessageFloat(hRead);

	m_fHitBoxScale = g_pInterface->ReadFromMessageFloat(hRead);

	for(i=0 ; i<MAX_ZOOM_LEVELS ; i++)
		m_fZoomLevels[i] = g_pInterface->ReadFromMessageFloat(hRead);

	m_fBaseFootStepVolume = g_pInterface->ReadFromMessageFloat(hRead);

	hRead->ReadStringFL(m_szFootStepSoundDir, sizeof(m_szFootStepSoundDir));
	hRead->ReadStringFL(m_szGeneralSoundDir, sizeof(m_szGeneralSoundDir));
	hRead->ReadStringFL(m_szAIAnimType, sizeof(m_szAIAnimType));

	hRead->ReadStringFL(m_szPounceWeapon, sizeof(m_szPounceWeapon));
	m_bCanLoseLimb = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanClimb = g_pInterface->ReadFromMessageByte(hRead);
	m_bCanPounce = g_pInterface->ReadFromMessageByte(hRead);
	m_nPounceDamage = g_pInterface->ReadFromMessageDWord(hRead);
}

//*********************************************************************************

void t_CharacterButes::Copy(const t_CharacterButes &cb)
{
	int i = 0;

	m_nId = cb.m_nId;

	m_eCharacterClass = cb.m_eCharacterClass;
	SAFE_STRCPY(m_szParent, cb.m_szParent);

	SAFE_STRCPY(m_szModelDir, cb.m_szModelDir);
	SAFE_STRCPY(m_szSkinDir, cb.m_szSkinDir);

	SAFE_STRCPY(m_szName, cb.m_szName);
	SAFE_STRCPY(m_szModel, cb.m_szModel);

	for(i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		SAFE_STRCPY(m_szSkins[i], cb.m_szSkins[i]);
	}

	SAFE_STRCPY(m_szWeaponSkinType, cb.m_szWeaponSkinType);

	for(i = 0; i < MAX_ANIM_LAYERS; i++)
	{
		SAFE_STRCPY(m_szAnimLayers[i], cb.m_szAnimLayers[i]);
		SAFE_STRCPY(m_szMPAnimLayers[i], cb.m_szMPAnimLayers[i]);
	}

	m_vDims = cb.m_vDims;
	m_fMass = cb.m_fMass;
	m_fCameraHeightPercent = cb.m_fCameraHeightPercent;
	m_nSurfaceType = cb.m_nSurfaceType;
	m_nSkeleton = cb.m_nSkeleton;
	m_nWeaponSet = cb.m_nWeaponSet;
	m_nMPWeaponSet = cb.m_nMPWeaponSet;
	m_nMPClassWeaponSet = cb.m_nMPClassWeaponSet;
	m_nMPWeaponStringRes = cb.m_nMPWeaponStringRes;
	SAFE_STRCPY(m_szVisionSet, cb.m_szVisionSet);
	m_nHUDType = cb.m_nHUDType;
	m_vPouncingDims = cb.m_vPouncingDims;

	m_fChestOffsetPercent = cb.m_fChestOffsetPercent;
	m_fHeadOffsetPercent = cb.m_fHeadOffsetPercent;

	m_ptFOV = cb.m_ptFOV;
	m_ptAimRange = cb.m_ptAimRange;
	m_fHeadTiltAngle = cb.m_fHeadTiltAngle;
	m_fHeadTiltSpeed = cb.m_fHeadTiltSpeed;

	m_fBaseWalkSpeed = cb.m_fBaseWalkSpeed;
	m_fBaseRunSpeed = cb.m_fBaseRunSpeed;
	m_fBaseJumpSpeed = cb.m_fBaseJumpSpeed;
	m_fBaseRunJumpSpeed = cb.m_fBaseRunJumpSpeed;
	m_fBaseSpringJumpSpeed = cb.m_fBaseSpringJumpSpeed;
	m_fBasePounceSpeed = cb.m_fBasePounceSpeed;
	m_fBaseCrouchSpeed = cb.m_fBaseCrouchSpeed;
	m_fBaseLiquidSpeed = cb.m_fBaseLiquidSpeed;
	m_fBaseLadderSpeed = cb.m_fBaseLadderSpeed;

	m_fFallDistanceSafe = cb.m_fFallDistanceSafe;
	m_fFallDistanceDeath = cb.m_fFallDistanceDeath;

	m_fWalkingTurnRate = cb.m_fWalkingTurnRate;
	m_fRunningTurnRate = cb.m_fRunningTurnRate;

	m_fWalkingAimRate = cb.m_fWalkingAimRate;
	m_fRunningAimRate = cb.m_fRunningAimRate;

	m_fBaseGroundAccel = cb.m_fBaseGroundAccel;
	m_fBaseAirAccel = cb.m_fBaseAirAccel;
	m_fBaseLiquidAccel = cb.m_fBaseLiquidAccel;

	m_fMaxAIPopDist = cb.m_fMaxAIPopDist;

	m_vJumpDirection = cb.m_vJumpDirection;

	m_bWallWalk = cb.m_bWallWalk;
	m_bCanBeNetted = cb.m_bCanBeNetted;
	m_bCanBeOnFire = cb.m_bCanBeOnFire;
	m_bCanBeFacehugged = cb.m_bCanBeFacehugged;
	m_bCanUseLookAt = cb.m_bCanUseLookAt;

	m_fActivateDist = cb.m_fActivateDist;
	m_nEMPFlags = cb.m_nEMPFlags;
	m_fWalkingKickBack = cb.m_fWalkingKickBack;
	m_fRunningKickBack = cb.m_fRunningKickBack;

	m_fBaseStunChance = cb.m_fBaseStunChance;

	m_fDefaultHitPoints = cb.m_fDefaultHitPoints;
	m_fMaxHitPoints = cb.m_fMaxHitPoints;
	m_fDefaultArmorPoints = cb.m_fDefaultArmorPoints;
	m_fMaxArmorPoints = cb.m_fMaxArmorPoints;
	m_fDamageResistance = cb.m_fDamageResistance;

	m_fHeadBiteHealPoints = cb.m_fHeadBiteHealPoints;
	m_fClawHealPoints = cb.m_fClawHealPoints;

	m_fAirSupplyTime = cb.m_fAirSupplyTime;

	m_fMinRecoilDamage = cb.m_fMinRecoilDamage;
	m_fRecoilChance = cb.m_fRecoilChance;
	m_fMinRecoilDelay = cb.m_fMinRecoilDelay;
	m_fMaxRecoilDelay = cb.m_fMaxRecoilDelay;

	m_fHitBoxScale = cb.m_fHitBoxScale;

	for( i = 0; i < MAX_ZOOM_LEVELS; i++)
	{
		m_fZoomLevels[i] = cb.m_fZoomLevels[i];
	}

	m_fBaseFootStepVolume = cb.m_fBaseFootStepVolume;

	SAFE_STRCPY(m_szFootStepSoundDir, cb.m_szFootStepSoundDir);
	SAFE_STRCPY(m_szGeneralSoundDir, cb.m_szGeneralSoundDir);
	SAFE_STRCPY(m_szAIAnimType, cb.m_szAIAnimType);

	SAFE_STRCPY(m_szPounceWeapon, cb.m_szPounceWeapon);
	m_bCanLoseLimb = cb.m_bCanLoseLimb;
	m_bCanClimb = cb.m_bCanClimb;
	m_bCanPounce = cb.m_bCanPounce;
	m_nPounceDamage = cb.m_nPounceDamage;
}

//*********************************************************************************

void t_CharacterButes::FillInObjectCreateStruct(ObjectCreateStruct &ocs, LTBOOL bUseMPDir) const
{
	if( m_szModel[0] )
	{
		SAFE_STRCPY(ocs.m_Filename, m_szModelDir);
		strcat(ocs.m_Filename, m_szModel);
	}
	else
	{
		ocs.m_Filename[0] = 0;
	}

	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		if( m_szSkins[i][0] )
		{
			SAFE_STRCPY(ocs.m_SkinNames[i], m_szSkinDir);
			strcat(ocs.m_SkinNames[i], m_szSkins[i]);
		}
		else
		{
			ocs.m_SkinNames[i][0] = 0;
		}
	}
}

//*********************************************************************************
// Construction/Destruction
//*********************************************************************************

CCharacterButeMgr::CCharacterButeMgr()
{
	m_pButeSets = LTNULL;
	m_nNumSets = 0;
}

CCharacterButeMgr::~CCharacterButeMgr()
{
	Term();
}

bool CCharacterButeMgr::CountTags(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CCharacterButeMgr *pButeMgr = (CCharacterButeMgr*)pData;

	if( pButeMgr->m_buteMgr.GetInt(szTagName, CHARACTER_IGNORE_SET, 1) != 0 )
	{
		++pButeMgr->m_nNumSets;
	}

	// Keep iterating.
	return true;
}

bool CCharacterButeMgr::LoadButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) 
		return true;

	CCharacterButeMgr *pButeMgr = (CCharacterButeMgr*)pData;
	

	if( pButeMgr->m_buteMgr.GetInt(szTagName, CHARACTER_IGNORE_SET, 1) != 0 )
	{
		// Load the character butes.
		pButeMgr->LoadCharacterButes( szTagName, pButeMgr->m_pButeSets[ pButeMgr->m_nNumSets ] );

		// Point to the next empty set.
		++pButeMgr->m_nNumSets;
	}

	// Keep iterating.
	return true;
}

//*********************************************************************************
//
//	ROUTINE:	CCharacterButeMgr::Init()
//	PURPOSE:	Init mgr
//
//*********************************************************************************

LTBOOL CCharacterButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
#ifndef __LINUX
	ASSERT( g_pModelButeMgr );
	ASSERT( g_pSurfaceMgr );
#endif

	if (g_pCharacterButeMgr || !szAttributeFile) return DFALSE;
	if (!Parse(pInterface, szAttributeFile)) return DFALSE;


	// Set up global pointer...
	g_pCharacterButeMgr = this;


	// Calculate the number of character attribute sets
	m_nNumSets = 0;
	m_buteMgr.GetTags(CountTags,this);


	if( m_nNumSets > 0 )
	{
		// Allocate the space to hold all this info!
		m_pButeSets = new CharacterButes[m_nNumSets];
		ASSERT( m_pButeSets );
		if( m_pButeSets )
		{
			const int nOldNumSets = m_nNumSets;

			m_nNumSets = 0; // this will be used to mark the current bute being filled.
			m_buteMgr.GetTags(LoadButes,this);

			ASSERT( m_nNumSets == nOldNumSets );
		}
	}
	else
	{
		m_pButeSets = LTNULL;
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

	return (m_pButeSets != LTNULL);
}


//*********************************************************************************
//
//	ROUTINE:	CCharacterButeMgr::Term()
//	PURPOSE:	Clean up.
//
//*********************************************************************************

void CCharacterButeMgr::Term()
{
	g_pCharacterButeMgr = DNULL;

	if(m_pButeSets)
	{
		delete[] m_pButeSets;
		m_pButeSets = LTNULL;
	}
}

//*********************************************************************************

int CCharacterButeMgr::GetSetFromModelType(const char *szName) const
{
	// Find out which character we're looking for
	for(int i = 0; i < GetNumSets(); i++)
	{
		if(!stricmp(szName, GetModelType(i)))
			return i;
	}

// Let the caller provide an error message.
//	g_pInterface->CPrint("CharacterButeMgr: Could not find set %s! Defaulting to index zero!", szName);

	return -1;
}

//*********************************************************************************

CString CCharacterButeMgr::GetDefaultModel(int nSet, const char *szDir, LTBOOL bForceSingle) const
{
	Check(nSet);

	CString szTemp = szDir ? szDir : m_pButeSets[nSet].m_szModelDir;
	szTemp += m_pButeSets[nSet].m_szModel;

	return szTemp;
}

//*********************************************************************************

CString CCharacterButeMgr::GetMultiplayerModel(int nSet, const char *szDir) const
{
	Check(nSet);

	CString szTemp = szDir ? szDir : m_pButeSets[nSet].m_szModelDir;
	szTemp += m_pButeSets[nSet].m_szMultiModel;

	return szTemp;
}

//*********************************************************************************

CString CCharacterButeMgr::GetDefaultSkin(int nSet, int nSkin, const char *szDir) const
{
	Check(nSet);
	ASSERT((nSkin >= 0) && (nSkin < MAX_MODEL_TEXTURES));

	CString szTemp = szDir ? szDir : m_pButeSets[nSet].m_szSkinDir;
	szTemp += m_pButeSets[nSet].m_szSkins[nSkin];

	return szTemp;
}

//*********************************************************************************

void CCharacterButeMgr::GetDefaultFilenames(int nSet, ObjectCreateStruct &pStruct, const char *szModelDir, const char *szSkinDir) const
{
	Check(nSet);

	CString cstrModel = GetDefaultModel(nSet, szModelDir);

	if(!cstrModel.IsEmpty())
	{
		SAFE_STRCPY(pStruct.m_Filename, cstrModel.GetBuffer());
	}
	else
		pStruct.m_Filename[0] = 0;

	for(int i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		if( m_pButeSets[nSet].m_szSkins[i][0] )
		{
			SAFE_STRCPY(pStruct.m_SkinNames[i], szSkinDir ? szSkinDir : m_pButeSets[nSet].m_szSkinDir);
			strcat(pStruct.m_SkinNames[i], m_pButeSets[nSet].m_szSkins[i]);
		}
		else
		{
			pStruct.m_SkinNames[i][0] = 0;
		}
	}
}

//*********************************************************************************

CString CCharacterButeMgr::GetWeaponSkinType(int nSet) const
{
	Check(nSet);

	CString szTemp = m_pButeSets[nSet].m_szWeaponSkinType;

	return szTemp;
}

//*********************************************************************************

const CharacterButes & CCharacterButeMgr::GetCharacterButes(const char *szName) const
{
	int nSet = GetSetFromModelType(szName);

	ASSERT( nSet >= 0 && nSet < m_nNumSets );

	if(nSet != -1)
		return m_pButeSets[nSet];

	return m_pButeSets[0];
}

//*********************************************************************************

void CCharacterButeMgr::LoadCharacterButes(const char * szTagName, CharacterButes &butes)
{
	int i = 0;

	butes.m_nId = m_nNumSets;
	SAFE_STRCPY(butes.m_szName, szTagName);

	butes.m_eCharacterClass = GetCharacterClass( GetString(szTagName, CHARACTER_CLASS_TYPE) );

	SAFE_STRCPY(butes.m_szParent, GetString(szTagName, "Parent"));


	SAFE_STRCPY(butes.m_szModelDir, GetString(szTagName, CHARACTER_BUTE_DEFAULT_MODEL_DIR));
	SAFE_STRCPY(butes.m_szSkinDir, GetString(szTagName, CHARACTER_BUTE_DEFAULT_SKIN_DIR));

	SAFE_STRCPY(butes.m_szModel, GetString(szTagName, CHARACTER_BUTE_DEFAULT_MODEL));


	// I want this to be specifically specified for each character type, because it determines
	// whether a character is available in multiplayer or not... so we'll use the base bute mgr
	// to check for this string so it won't go through the hierarchy stuff.
	if(m_buteMgr.Exist(szTagName, CHARACTER_BUTE_MULTIPLAYER_MODEL))
	{
		SAFE_STRCPY(butes.m_szMultiModel, GetString(szTagName, CHARACTER_BUTE_MULTIPLAYER_MODEL));
	}
	else
	{
		memset(butes.m_szMultiModel, 0, sizeof(butes.m_szMultiModel));
	}


	butes.m_nDisplayName = GetInt(szTagName, CHARACTER_BUTE_DISPLAY_NAME);
	butes.m_nScoringClass = GetInt(szTagName, CHARACTER_BUTE_SCORING_CLASS);

	SAFE_STRCPY(butes.m_szSelectAnim, GetString(szTagName, CHARACTER_BUTE_SELECT_ANIM));
	butes.m_vSelectPos = GetVector(szTagName, CHARACTER_BUTE_SELECT_POS);
	butes.m_fSelectRot = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_SELECT_ROT);
	butes.m_fSelectScale = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_SELECT_SCALE);

	char szBuffer[128];
	for(i = 0; i < MAX_MODEL_TEXTURES; i++)
	{
		sprintf(szBuffer, CHARACTER_BUTE_DEFAULT_SKIN, i);
		SAFE_STRCPY(butes.m_szSkins[i], GetString(szTagName, szBuffer));
	}

	SAFE_STRCPY(butes.m_szWeaponSkinType, GetString(szTagName, CHARACTER_BUTE_WEAPON_SKIN_TYPE));

	for(i = 0; i < MAX_ANIM_LAYERS; i++)
	{
		sprintf(szBuffer, CHARACTER_BUTE_ANIMLAYER, i);
		SAFE_STRCPY(butes.m_szAnimLayers[i], GetString(szTagName, szBuffer));

		sprintf(szBuffer, CHARACTER_BUTE_MPANIMLAYER, i);
		SAFE_STRCPY(butes.m_szMPAnimLayers[i], GetString(szTagName, szBuffer));
	}

	butes.m_vDims = GetVector(szTagName, CHARACTER_BUTE_DEFAULT_DIMS);
	butes.m_fMass = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_DEFAULT_MASS);
	butes.m_fCameraHeightPercent = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_CAMERA_HEIGHT_PERCENT);

	CString szSurfaceType = GetString(szTagName, CHARACTER_BUTE_SURFACE_TYPE);
	SURFACE *pSurface = g_pSurfaceMgr ? g_pSurfaceMgr->GetSurface(szSurfaceType.GetBuffer(0)) : LTNULL;
	butes.m_nSurfaceType = pSurface ? (uint8)pSurface->eType : 0;

	CString szSkeleton = GetString(szTagName, CHARACTER_BUTE_SKELETON_NAME);
	butes.m_nSkeleton = g_pModelButeMgr ? g_pModelButeMgr->GetSkeletonFromName(szSkeleton)
										: eModelSkeletonInvalid;

	CString strWeaponSet = GetString(szTagName,CHARACTER_BUTE_WEAPON_SET, CString(""));
	if( !strWeaponSet.IsEmpty() && g_pWeaponMgr )
	{
		WEAPON_SET * pWeaponSet = g_pWeaponMgr->GetWeaponSet(strWeaponSet);
		if( pWeaponSet )
		{
			butes.m_nWeaponSet = pWeaponSet->nId;
		}
	}

	strWeaponSet = GetString(szTagName,CHARACTER_BUTE_MPWEAPON_SET, CString(""));
	if( !strWeaponSet.IsEmpty() && g_pWeaponMgr )
	{
		WEAPON_SET * pWeaponSet = g_pWeaponMgr->GetWeaponSet(strWeaponSet);
		if( pWeaponSet )
		{
			butes.m_nMPWeaponSet = pWeaponSet->nId;
		}
	}

	strWeaponSet = GetString(szTagName,CHARACTER_BUTE_MPCLASSWEAPON_SET, CString(""));
	if( !strWeaponSet.IsEmpty() && g_pWeaponMgr )
	{
		WEAPON_SET * pWeaponSet = g_pWeaponMgr->GetWeaponSet(strWeaponSet);
		if( pWeaponSet )
		{
			butes.m_nMPClassWeaponSet = pWeaponSet->nId;
		}
	}

	butes.m_nMPWeaponStringRes = GetInt(szTagName, CHARACTER_BUTE_MPWEAPON_STRING_RES);
	SAFE_STRCPY(butes.m_szVisionSet, GetString(szTagName, CHARACTER_BUTE_VISION_SET));
	butes.m_nHUDType = GetInt(szTagName, CHARACTER_BUTE_HUD_TYPE);
	butes.m_vPouncingDims = GetVector(szTagName, CHARACTER_BUTE_DEFAULT_POUNCING_DIMS, CAVector(butes.m_vDims.x,butes.m_vDims.y,butes.m_vDims.z) );

	butes.m_fChestOffsetPercent = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_CHEST_OFFSET_PERCENT);
	butes.m_fHeadOffsetPercent = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_HEAD_OFFSET_PERCENT);

	butes.m_ptFOV = GetPoint(szTagName, CHARACTER_BUTE_FOV);
	butes.m_ptAimRange = GetPoint(szTagName, CHARACTER_BUTE_AIM_RANGE);
	butes.m_fHeadTiltAngle = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_HEAD_TILT_ANGLE);
	butes.m_fHeadTiltSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_HEAD_TILT_SPEED);

	butes.m_fBaseWalkSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_WALK_SPEED);
	butes.m_fBaseRunSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_RUN_SPEED);
	butes.m_fBaseJumpSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_NORM_JUMP_SPEED);
	butes.m_fBaseRunJumpSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_RUN_JUMP_SPEED);
	butes.m_fBaseSpringJumpSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_SPRING_JUMP_SPEED);
	butes.m_fBasePounceSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_POUNCE_SPEED);
	butes.m_fBaseCrouchSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_CROUCH_SPEED);
	butes.m_fBaseLiquidSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_LIQUID_SPEED);
	butes.m_fBaseLadderSpeed = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_LADDER_SPEED);

	butes.m_fFallDistanceSafe = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_FALL_DIST_SAFE);
	butes.m_fFallDistanceDeath = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_FALL_DIST_DEATH);

	butes.m_fWalkingTurnRate = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_WALKING_TURN_RATE);
	butes.m_fRunningTurnRate = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_RUNNING_TURN_RATE);

	butes.m_fWalkingAimRate = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_WALKING_AIM_RATE);
	butes.m_fRunningAimRate = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_RUNNING_AIM_RATE);

	if( butes.m_fWalkingAimRate < butes.m_fWalkingTurnRate )
	{
		butes.m_fWalkingAimRate = butes.m_fWalkingTurnRate*1.01f;
	}

	if( butes.m_fRunningAimRate < butes.m_fRunningTurnRate )
	{
		butes.m_fRunningAimRate = butes.m_fRunningTurnRate*1.01f;
	}

	butes.m_fBaseGroundAccel = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_GROUND_ACCEL);
	butes.m_fBaseAirAccel = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_AIR_ACCEL);
	butes.m_fBaseLiquidAccel = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_LIQUID_ACCEL);

	butes.m_fMaxAIPopDist = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_AI_POP_DIST, 0.0f);

	butes.m_vJumpDirection = GetVector(szTagName, CHARACTER_BUTE_JUMP_DIRECTION);

	butes.m_bWallWalk = (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_WALL_WALK, 0);
	butes.m_bCanBeNetted = (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_CAN_BE_NETTED, 0);
	butes.m_bCanBeOnFire = (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_CAN_BE_ONFIRE, 1);
	butes.m_bCanBeFacehugged = (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_CAN_BE_FACEHUGGED, 0);
	butes.m_bCanUseLookAt = (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_CAN_USE_LOOKAT, 1);

	butes.m_fActivateDist = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_ACTIVATE_DISTANCE);
	butes.m_nEMPFlags	= GetInt(szTagName, CHARACTER_BUTE_EMP_FLAGS);

	butes.m_fWalkingKickBack = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_WALKING_KICKBACK);
	butes.m_fRunningKickBack = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_RUNNING_KICKBACK);

	butes.m_fBaseStunChance = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_BASE_STUN);

	butes.m_fDefaultHitPoints = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_DEFAULT_HIT_POINTS);
	butes.m_fMaxHitPoints = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_MAX_HIT_POINTS);
	butes.m_fDefaultArmorPoints = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_DEFAULT_ARMOR_POINTS);
	butes.m_fMaxArmorPoints = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_MAX_ARMOR_POINTS);
	butes.m_fDamageResistance = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_DAMAGE_RESISTANCE);

	butes.m_fHeadBiteHealPoints = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_HEAD_BITE_HEAL_POINTS);
	butes.m_fClawHealPoints = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_CLAW_HEAL_POINTS);

	butes.m_fAirSupplyTime = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_AIR_SUPPLY_TIME);

	butes.m_fMinRecoilDamage = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_RECOIL_MIN_DAMAGE);
	butes.m_fRecoilChance = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_RECOIL_CHANCE, 0.0f);

	CARange temp  = GetRange(szTagName, CHARACTER_BUTE_RECOIL_DELAY);
	butes.m_fMinRecoilDelay = (LTFLOAT)temp.GetMin();
	butes.m_fMaxRecoilDelay = (LTFLOAT)temp.GetMax();

	butes.m_fHitBoxScale = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_HIT_BOX_SCALE, 1.2f);

	for(i = 0; i < MAX_ZOOM_LEVELS; i++)
	{
		sprintf(szBuffer, "%s%d", CHARACTER_BUTE_ZOOM_LEVEL, i);
		butes.m_fZoomLevels[i] = (LTFLOAT)GetDouble(szTagName, szBuffer);
	}

	butes.m_fBaseFootStepVolume = (LTFLOAT)GetDouble(szTagName, CHARACTER_BUTE_FOOTSTEP_VOLUME, 1.0);

	SAFE_STRCPY(butes.m_szFootStepSoundDir, GetString(szTagName, CHARACTER_BUTE_FOOTSTEP_SOUND));
	SAFE_STRCPY(butes.m_szGeneralSoundDir,	GetString(szTagName, CHARACTER_BUTE_GENERAL_SOUND));
	SAFE_STRCPY(butes.m_szAIAnimType, GetString(szTagName, CHARACTER_BUTE_AI_ANIM_TYPE));

	SAFE_STRCPY(butes.m_szPounceWeapon, GetString(szTagName, CHARACTER_BUTE_POUNCE_WEAPON));
	butes.m_bCanLoseLimb		= (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_CAN_LOSE_LIMB);
	butes.m_bCanClimb		= (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_CAN_CLIMB);
	butes.m_bCanPounce		= (LTBOOL)GetInt(szTagName, CHARACTER_BUTE_CAN_POUNCE);
	butes.m_nPounceDamage	= GetInt(szTagName, CHARACTER_BUTE_POUNCE_WEAPON_DAMAGE);
}



//*********************************************************************************
//
// CCharacterButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
//*********************************************************************************

#ifndef _CLIENTBUILD  // Server-side only

// Plugin statics

LTBOOL CCharacterButeMgrPlugin::sm_bInitted = DFALSE;
CCharacterButeMgr CCharacterButeMgrPlugin::sm_ButeMgr;

static CWeaponMgr sm_WeaponMgr;
static CModelButeMgr sm_ModelButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT	CCharacterButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength)
{
	if (!sm_bInitted)
	{
		char szFile[256];

		if( !g_pModelButeMgr )
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, MBMGR_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, MBMGR_DEFAULT_FILE);
#endif
			sm_ModelButeMgr.SetInRezFile(LTFALSE);
			sm_ModelButeMgr.Init(g_pLTServer, szFile);
		}

		if( !g_pWeaponMgr )
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, WEAPON_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, WEAPON_DEFAULT_FILE);
#endif
			sm_WeaponMgr.SetInRezFile(LTFALSE);
			sm_WeaponMgr.Init(g_pLTServer, szFile);
		}

		// Create the bute mgr if necessary
		if(!g_pCharacterButeMgr)
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, CHARACTER_BUTES_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath, CHARACTER_BUTES_DEFAULT_FILE);
#endif
			sm_ButeMgr.SetInRezFile(LTFALSE);
			sm_ButeMgr.Init(g_pLTServer, szFile);
		}

		sm_bInitted = LTTRUE;
	}

	// Add an entry for each character...
	uint32 nNumCharacters = g_pCharacterButeMgr->GetNumSets();

	typedef std::vector<std::string> CharacterNameList;
	CharacterNameList character_name_list;

	for (uint32 i = 0; i < nNumCharacters; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		CString sModelType = g_pCharacterButeMgr->GetModelType( i );
		int nFindAI = sModelType.Find( "_AI" );
		if( m_bPlayerOnly && nFindAI >= 0 )
			continue;

		if( m_bAIOnly && nFindAI < 0 )
			continue;

		int nFindSAI = sModelType.Find( "_SAI" );
		if( m_bPlayerOnly && nFindSAI >= 0 )
			continue;


		character_name_list.push_back( std::string(sModelType) );
	}

	std::sort(character_name_list.begin(), character_name_list.end());

	for( CharacterNameList::iterator iter = character_name_list.begin();
	     iter != character_name_list.end() && (*pcStrings) + 1 < cMaxStrings; ++iter )
	{
		strcpy(aszStrings[(*pcStrings)++], iter->c_str() );
	}		

	return LT_UNSUPPORTED;
}


#endif //_CLIENTBUILD
