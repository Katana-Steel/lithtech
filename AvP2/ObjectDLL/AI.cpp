// ----------------------------------------------------------------------- //
//
// MODULE  : AI.cpp
//
// PURPOSE : Generic base AI object - Implementation
//
// CREATED : 9/29/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "AI.h"

#include "Destructible.h"
#include "Weapons.h"
#include "ObjectMsgs.h"
#include "VolumeBrushTypes.h"
#include "HHWeaponModel.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "AIButeMgr.h"
#include "SurfaceFunctions.h"
#include "Attachments.h"
#include "CommandMgr.h"
#include "AIPathMgr.h"
#include "AITarget.h"
#include "AISenseMgr.h"
#include "TeleportPoint.h"
#include "AIUtils.h"
#include "BidMgr.h"
#include "AINodeMgr.h"
#include "AINode.h"
#include "ParseUtils.h"
#include "LightObject.h"
#include "BodyProp.h"
#include "DebugLineSystem.h"
#include "CinematicTrigger.h"
#include "AIVolume.h"
#include "AINodeVolume.h"
#include "Door.h"
#include "FXButeMgr.h"
#include "CharacterHitBox.h"
#include "AIScriptCommands.h" // for CAIScriptCommand::GetCommandDescription
#include "CharacterFuncs.h"

#include "RCMessage.h"
#include "ServerSoundMgr.h"
#include "MsgIDs.h"

#include "AIChase.h"
#include "AIPatrol.h"
#include "AIAttack.h"
#include "AILurk.h"
#include "AIScript.h"
#include "AIInvestigate.h"
#include "AISnipe.h"
#include "AICower.h"
#include "AIAlarm.h"
#include "AIIdle.h"
#include "AINodeCheck.h"

#include "AIAction.h"
#include "AIActionMoveTo.h"
#include "AIActionMisc.h"

#include <algorithm>

static std::string ReadGoalProp( GenericProp * pGenProp, int number);

const char g_szEyeSocketName[] = "Head";

const char g_szNone[] = "<none>";
const char g_szDefault[] = "<default>";
const char g_szSilent[] = "<no_affect>";

const char g_szActive[] = "ACTIVE";
const char g_szAddGoal[] = "AG";
const char g_szAlertLock[] = "ALERT";
const char g_szAddGoalBidParam[] = "GoalBid";
const char g_szAddGoalNameParam[] = "GoalName";
const char g_szAddWeapon[] = "AW";
const char g_szCrouch[] = "CROUCH";
const char g_szDropWeapons[] = "DW";
const char g_szFlashlight[] = "SL";
const char g_szFlashlightLock[] = "SLLOCK";
const char g_szHidden[] = "HIDDEN"; // This is so that the AI will catch "hidden 0/1" messages.
const char g_szIdleScript[] = "ISCR";
const char * g_szInvestigate = "INV";
const char g_szKillGoal[] = "KG";
const char g_szKillScripts[] = "KILLSCRIPTS";
const char g_szKillAction[] = "KILLACTION";
const char g_szLookAt[] = "LA";
const char g_szListGoals[] = "LISTGOALS";
const char g_szLeash[] = "LSH";
const char g_szNeverLoseTarget[] = "NVRLSTRGT";
const char g_szRemove[] = "REMOVE";
const char g_szResetSense[] = "RESETSENSE";
const char g_szRotate[] = "ROT";
const char g_szRun[] = "RUN";
const char g_szRunLock[] = "RUNLOCK";
const char g_szCrouchLock[] = "CROUCHLOCK";
const char g_szSay[] = "SAY";
const char g_szScript[] = "SCR";
const char g_szSeeThrough[] = "SEETHROUGH";
const char g_szSetClass[] = "SETCLASS";
const char g_szSetDeathAnim[] = "SDA";
const char g_szSetGoalPriority[] = "SGP";
const char g_szSetMovementStyle[] = "SMS";
const char g_szShootThrough[] = "SHOOTTHROUGH";
const char g_szStand[] = "STAND";
const char g_szSetWeapon[] = "SW";
const char g_szTeleport[] = "TLP";
const char g_szTarget[] = "TRG";
const char g_szWalk[] = "WALK";
const char g_szWallWalk[] = "WALLWALK";

void CharacterMovementState_AIJump(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_AIJump_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_AIJump_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_AIClip(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_AIClip_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_AIClip_MaxAccel(const CharacterMovement *pMovement);

#ifdef _DEBUG
//#define ZERO_TURN_RADIUS
//#define ZERO_AIM_RADIUS
//#define AI_DEBUG
//#include "AIUtils.h"
//#define BID_DEBUG
//#define AIM_DEBUG
//#define AI_WEAPON_DEBUG
//#define AIMFROM_PROFILE
//#define EXACT_MOVEMENT_DEBUG
#endif


#define ADD_GOAL(number, group) \
		ADD_STRINGPROP_FLAG_HELP(GoalType_##number##, const_cast<char*>(g_szNone), (group) | PF_DYNAMICLIST, "Type of goal.") \
		ADD_OBJECTPROP_FLAG_HELP(Link_##number##, "", group, "Link to an object used by the goal (can be null).") \
		ADD_STRINGPROP_FLAG_HELP(Parameters_##number##, "", group, "Other parameters for goal (ie. TARGET=Player).")

/*
		ADD_STRINGPROP_FLAG_HELP(GoalName_##number##, "Goal_"#number, group, "Unique name for goal.") \
		ADD_REALPROP_FLAG_HELP(BidValue_##number##, 0.0f, group, "Bid (priority) of goal.") \
*/

#define ADD_GOAL_DEFAULTS(number, group, type, priority) \
		ADD_STRINGPROP_FLAG_HELP(GoalType_##number##, type, (group) | PF_DYNAMICLIST, "Type of goal.") \
		ADD_OBJECTPROP_FLAG_HELP(Link_##number##, "", group, "Link to an object used by the goal (can be null).") \
		ADD_STRINGPROP_FLAG_HELP(Parameters_##number##, "", group, "Other parameters for goal (ie. TARGET=Player).")

/*	   
		ADD_STRINGPROP_FLAG_HELP(GoalName_##number##, "Goal_"#number, group, "Unique name for goal.") \
		ADD_REALPROP_FLAG_HELP(BidValue_##number##, priority, group, "Bid (priority) of goal.") \
*/

// Define our properties (what is available in DEdit)...

BEGIN_CLASS(CAI)

	ADD_STRINGPROP_FLAG(AttributeTemplate, "", PF_STATICLIST)

	ADD_STRINGPROP_FLAG_HELP( StartingNode, "", PF_OBJECTLINK, "AI will start at this node")

	ADD_REALPROP_FLAG_HELP( LeashLength, 0.0f, 0, "AI will not move farther than this length away from the leash point.  Leave zero to keep AI unleashed.")
	ADD_STRINGPROP_FLAG_HELP( LeashPoint, "", PF_OBJECTLINK, "AI will not move farther than LeashLength away from this object. Leave blank to use AI's initial position.")

	// Overrides

	ADD_STRINGPROP_FLAG(SpawnItem, "", PF_HIDDEN)

	// Sense Reactions

	PROP_DEFINEGROUP(StartReactions, PF_GROUP6)

		ADD_STRINGPROP_FLAG(1stSeeEnemyTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stSeeEnemyMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(SeeEnemyTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(SeeEnemyMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stSeeCloakedTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stSeeCloakedMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(SeeCloakedTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(SeeCloakedMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stSeeEnemyFlashlightTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stSeeEnemyFlashlightMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(SeeEnemyFlashlightTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(SeeEnemyFlashlightMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stSeeDeadBodyTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stSeeDeadBodyMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(SeeDeadBodyTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(SeeDeadBodyMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearEnemyFootstepTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearEnemyFootstepMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearEnemyFootstepTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearEnemyFootstepMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearEnemyWeaponFireTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearEnemyWeaponFireMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearEnemyWeaponFireTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearEnemyWeaponFireMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearAllyWeaponFireTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearAllyWeaponFireMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearAllyWeaponFireTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearAllyWeaponFireMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearTurretWeaponFireTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearTurretWeaponFireMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearTurretWeaponFireTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearTurretWeaponFireMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearAllyPainTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearAllyPainMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearAllyPainTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearAllyPainMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearDeathTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearDeathMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearDeathTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearDeathMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearAllyBattleCryTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearAllyBattleCryMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearAllyBattleCryTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearAllyBattleCryMessage, LTFALSE, PF_GROUP6) \

		ADD_STRINGPROP_FLAG(1stHearEnemyTauntTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(1stHearEnemyTauntMessage, LTFALSE, PF_GROUP6) \
		ADD_STRINGPROP_FLAG(HearEnemyTauntTarget, LTFALSE, PF_GROUP6 | PF_OBJECTLINK) \
		ADD_STRINGPROP_FLAG(HearEnemyTauntMessage, LTFALSE, PF_GROUP6) \

	ADD_STRINGPROP_FLAG_HELP(Weapon, const_cast<char*>(g_szDefault), PF_STATICLIST, "AI's primary weapon.")
	ADD_STRINGPROP_FLAG_HELP(SecondaryWeapon, const_cast<char*>(g_szDefault), PF_STATICLIST, "If AI cannot use primary weapon, use this weapon.")
	ADD_STRINGPROP_FLAG_HELP(TertiaryWeapon, const_cast<char*>(g_szDefault), PF_STATICLIST, "If AI cannot use primary or secondary weapon, use this weapon.")

	// Attribute Overrides
		ADD_STRINGPROP_FLAG(Accuracy,		"", PF_GROUP3)
		ADD_BOOLPROP_FLAG_HELP(CanShootThrough,	LTTRUE, PF_GROUP3, "AI will attempt to shoot through things markd AICanShootThrough.")
		ADD_BOOLPROP_FLAG_HELP(CanSeeThrough,	LTTRUE, PF_GROUP3, "AI will attempt to See through things markd AICanSeeThrough.")

/*
		ADD_SENSEPROPS(PF_GROUP3)
*/

	// Goals and Commands
	ADD_BOOLPROP_FLAG_HELP(StartActive, LTTRUE, 0, "If false, the AI will start in-active (invisible, non-solid, non-responding, etc.)" )

	PROP_DEFINEGROUP(Commands, PF_GROUP4)
		
		ADD_STRINGPROP_FLAG_HELP(Initial,		"", PF_GROUP4, "This message is sent to the AI on its first update (first active update)." )
		ADD_STRINGPROP_FLAG_HELP(ActivateOn,		"", PF_GROUP4, "This message is sent to the AI he is activated (player activates, or receives an activate trigger).")
		ADD_STRINGPROP_FLAG_HELP(ActivateOff,	"", PF_GROUP4, "This message is sent to the AI he is de-activated (player de-activates, or receives a second activate trigger).")
		ADD_BOOLPROP_FLAG_HELP(UseCover,	LTFALSE,	PF_GROUP4, "AI will use cover nodes if available." )
		ADD_BOOLPROP_FLAG_HELP(NeverLoseTarget,	LTFALSE,	PF_GROUP4, "Once AI targets somebody it will not lose the target (but it may still switch targets)." )
		ADD_STRINGPROP_FLAG_HELP(UnreachableTargetCommand, "", PF_GROUP4, "If the AI has NeverLoseTarget and cannot reach the target, it will send this command to itself.")
		ADD_BOOLPROP_FLAG_HELP(CanHearEnemyTaunt,	LTFALSE,	PF_GROUP4, "AI will investigate tuants if this is on." )
		ADD_BOOLPROP_FLAG_HELP(CanHearTurretWeaponFire,	LTFALSE,	PF_GROUP4, "AI will investigate what the turret is firing at if this is on." )

#ifndef __LINUX
	PROP_DEFINEGROUP(Goals, PF_GROUP5) 
		ADD_GOAL_DEFAULTS(1, PF_GROUP5, "Attack", 1000.0f )
		ADD_GOAL_DEFAULTS(2, PF_GROUP5, "Investigate", 150.0f )
		ADD_GOAL(3,PF_GROUP5)
		ADD_GOAL(4,PF_GROUP5)
		ADD_GOAL(5,PF_GROUP5)
		ADD_GOAL(6,PF_GROUP5)
		ADD_GOAL(7,PF_GROUP5)
		ADD_GOAL(8,PF_GROUP5)
		ADD_GOAL(9,PF_GROUP5)
		ADD_GOAL(10,PF_GROUP5)
		ADD_GOAL(11,PF_GROUP5)
		ADD_GOAL(12,PF_GROUP5)
		ADD_GOAL(13,PF_GROUP5)
		ADD_GOAL(14,PF_GROUP5)
		ADD_GOAL(15,PF_GROUP5)
#endif

	ADD_COLORPROP_FLAG_HELP(DEditColor, 255.0f, 0.0f, 0.0f, 0, "Color used for object in DEdit.") 

#ifdef _WIN32	
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models\\Characters\\X.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#else
	ADD_STRINGPROP_FLAG_HELP(DEditDims, "Models/Characters/X.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME, "This is a dummy ABC file that just helps to see the dimensions of an object.")
#endif

	ADD_STRINGPROP_FLAG_HELP(AmbientTrackSet, const_cast<char*>(g_szDefault), PF_STATICLIST, "TrackSet for Ambient music mood.")
	ADD_STRINGPROP_FLAG_HELP(WarningTrackSet, const_cast<char*>(g_szDefault), PF_STATICLIST, "TrackSet for Warning music mood.")
	ADD_STRINGPROP_FLAG_HELP(HostileTrackSet, const_cast<char*>(g_szDefault), PF_STATICLIST, "TrackSet for Hostile music mood.")

	ADD_BOOLPROP_FLAG_HELP(StartHidden, LTFALSE, PF_HIDDEN, "This will start the object as 'hidden 1'.")

END_CLASS_DEFAULT_FLAGS_PLUGIN(CAI, CCharacter, NULL, NULL, CF_HIDDEN, CAIPlugin)

#ifndef _FINAL

bool InfoAIState()
{
	ASSERT( g_pLTServer );

	static CVarTrack g_vtInfoAIState(g_pLTServer,"InfoAIState",0.0f);

	return g_vtInfoAIState.GetFloat() > 0.0f;
}

bool InfoAIListGoals()
{
	ASSERT( g_pLTServer );

	static CVarTrack g_vtInfoAIListGoals(g_pLTServer,"InfoAIListGoals",0.0f);
	
	return g_vtInfoAIListGoals.GetFloat() > 0.0f;
}


static bool ShouldShowPos(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

	static CVarTrack vtShowAIPos(g_pLTServer,"ShowAIPos",0.0f);

	return vtShowAIPos.GetFloat() > 0.0f && ShouldShow(hObject);
}

static bool ShouldShowFire(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

	static CVarTrack vtShowAIFiring(g_pLTServer,"ShowAIFiring",0.0f);

	return vtShowAIFiring.GetFloat() > 0.0f && ShouldShow(hObject);
}

static LTFLOAT ShowActionLevel(HOBJECT hObject)
{
	ASSERT( g_pLTServer );

	static CVarTrack vtShowAIAction(g_pLTServer,"ShowAIAction",0.0f);

	if( !ShouldShow(hObject) )
		return 0.0f;

	return vtShowAIAction.GetFloat();
}

#endif

std::string ExtractMusicSetting(char * szString)
{
	if( 0 == stricmp(szString,g_szSilent) )
	{
		return g_szSilent;
	}
	else if(   0 == stricmp(szString, g_szDefault)
			|| 0 == stricmp(szString, g_szNone) )
	{
		return std::string();
	}

	// Extract the trackset.
	char* szSong = strtok( szString, " " );
	if( szSong )
	{
		char* szMood = strtok( NULL, " " );
		if( szMood )
		{
			char* szTrackSet = strtok( NULL, " " );
			if( szTrackSet )
			{
				return std::string(szTrackSet);
			}
		}
	}

	_ASSERT(0);
	return std::string();
}

// Filter functions

static HOBJECT s_hAI = LTNULL;

LTBOOL CAI::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
	CCharacterHitBox *pHitBox = LTNULL;
	BodyProp *pBodyProp = LTNULL;

	if ( !hObj ) return LTFALSE;
	if ( hObj == s_hAI ) return LTFALSE;

	pHitBox = dynamic_cast<CCharacterHitBox *>(g_pLTServer->HandleToObject(hObj));
	if (!pHitBox)
		pBodyProp = dynamic_cast<BodyProp *>(g_pLTServer->HandleToObject(hObj));

	if ((pHitBox || pBodyProp) && (hObj != pUserData))
	{
		return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

LTBOOL CAI::ShootThroughFilterFn(HOBJECT hObj, void *pUserData)
{
	if ( !hObj ) return LTFALSE;
	if ( hObj == s_hAI ) return LTFALSE;

	GameBase * pObject = dynamic_cast<GameBase *>( g_pLTServer->HandleToObject( hObj ) );
	if( pObject && pObject->CanShootThrough() )
	{
		return LTFALSE;
	}

	return DefaultFilterFn(hObj, pUserData);
}

LTBOOL CAI::ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// No longer used.
/*	if ( INVALID_HPOLY == hPoly ) return LTFALSE;

	
	// Check to see if this is a poly from the main world.
	// If it is, don't ignore it!
	HOBJECT hPolyObject = LTNULL;
	g_pLTServer->GetHPolyObject(hPoly,hPolyObject);
	
	if( g_pLTServer->Physics()->IsWorldObject(hPolyObject) )
		return LTTRUE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if ( pSurf &&  pSurf->bCanShootThrough ) 
	{
		return LTFALSE;
	}

*/
	return LTTRUE;
}

LTBOOL CAI::SeeFilterFn(HOBJECT hObj, void *pUserData)
{
	if ( !hObj ) return LTFALSE;
	if ( hObj == s_hAI ) return LTFALSE;

	CCharacter *pCharacter = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(hObj) );
	if ((pCharacter) && (hObj != pUserData))
	{
		return LTFALSE;
	}

	return DefaultFilterFn(hObj, pUserData);
}

LTBOOL CAI::SeeThroughFilterFn(HOBJECT hObj, void *pUserData)
{
	if ( !hObj ) return LTFALSE;
	if ( hObj == s_hAI ) return LTFALSE;

	GameBase * pObject = dynamic_cast<GameBase *>( g_pLTServer->HandleToObject( hObj ) );
	if( pObject && pObject->CanSeeThrough() )
	{
		return LTFALSE;
	}

	return SeeFilterFn(hObj, pUserData);

}

LTBOOL CAI::SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// No longer used.
/*	if ( INVALID_HPOLY == hPoly ) return LTFALSE;

	// Check to see if this is a poly from the main world.
	// If it is, don't ignore it!
	HOBJECT hPolyObject = LTNULL;
	g_pLTServer->GetHPolyObject(hPoly,hPolyObject);
	
	if( g_pLTServer->Physics()->IsWorldObject(hPolyObject) )
		return LTTRUE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if(pSurf)
		if(pSurf->bCanSeeThrough)
			return LTFALSE;
*/
	return LTTRUE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

CAI::CAI()
  :	m_nTemplateId(-1),
  	m_cc(UNKNOWN),
	//m_strWeaponNames

	m_fUpdateDelta(0.1f),

	m_bLocked(LTFALSE),
	m_bFirstUpdate(LTTRUE),
	//m_strQueuedCommands
	m_bUseCover(LTFALSE),
	m_bUseShoulderLamp(LTFALSE),
	m_bNeverLoseTarget(LTFALSE),
	
	m_bHasLATarget(LTFALSE),
	m_hLATarget(LTNULL),

	m_fLastAimAtTime(-1.0f),
	//m_strDefaultDeathAnim

	m_bStartActive(LTTRUE),
	m_bActive(LTTRUE),
	m_dwActiveFlags(0),
	m_dwActiveFlags2(0),
	m_dwActiveUserFlags(0),

	m_dwActiveHeadFlags(0),
	m_dwActiveHeadFlags2(0),
	m_dwActiveHeadUserFlags(0),

	m_dwMovementFlags(CM_FLAG_NONE),
	m_bWasStandingOn(LTTRUE),
	m_vLastStandingPos(0,0,0),
	m_dwMovementAnimFlags(CM_FLAG_NONE),
	m_pStrategyMove(LTNULL),

	//m_AvailableWeapons
	m_pWeaponPosition(LTNULL),
	m_nCurrentWeapon(-1),
	m_pCurrentWeapon(LTNULL),
	m_pCurrentWeaponButes(LTNULL),
	m_pIdealWeaponButes(LTNULL),
	// m_tmrBurstDelay
	// m_tmrLastBurst
	m_vAimToPos(0,0,0),
	m_bFullyAimed(LTFALSE),


	m_vEyePos(0.0f,0.0f,0.0f),
	m_vEyeForward(0.0f,0.0f,0.0f),
	m_rEyeRotation(0.0f,0.0f,0.0f,1.0f),
	m_fHalfForwardFOV(0.0f),
	m_fHalfUpNonFOV(0.0f),

	m_rTargetRot(0.0f,0.0f,0.0f,1.0f),
	m_rStartRot(0.0f,0.0f,0.0f,1.0f),
	m_vTargetRight(0.0f,0.0f,0.0f),
	m_vTargetUp(0.0f,0.0f,0.0f),
	m_vTargetForward(0.0f,0.0f,0.0f),
	m_bRotating(LTFALSE),
	m_fRotationTime(-1.0f),
	m_fRotationTimer(1.0f),
	m_fRotationSpeed(1.0f),

	m_pTarget( new CAITarget() ),
	m_bShootThrough(LTTRUE),
	m_bSeeThrough(LTTRUE),

	m_fAccuracy(-FLT_MAX), //will be set in PostPropRead().

	//m_hstrCmdInitial
	//m_hstrCmdActivateOn
	//m_hstrCmdActivateOff
	//m_strUnreachableTargetCmd

	m_bActivated(LTFALSE),

	m_pSenseMgr( new CAISenseMgr() ),

	// m_tmrInfluence
	m_bOddsFear(LTFALSE),
	m_hFriend(LTNULL),
	m_fFriendDistSqr(0.0f),
	m_bFriendLocked(LTFALSE),
	// m_tmrDamageFear
	m_fFraction(0.0f),

	// m_hCinematicTrigger

	m_fStartRechargeLevel(0.0f),
	m_fStopRechargeLevel(0.0f),
	m_bRechargingExosuit(LTFALSE),

	m_pNextAction(LTNULL),
	m_pCurrentAction(LTNULL),

	m_pLastNode(LTNULL),
	m_pNextNode(LTNULL),
	m_fLeashLengthSqr(0.0f),
	m_vLeashFrom(0,0,0),

	m_pBidMgr( new BidMgr() ),
	m_pCurrentBid( LTNULL ),

	m_bAlertLock(LTFALSE),
	m_bRunLock(LTFALSE),
	m_bCrouchLock(LTFALSE),
	m_bForceFlashlightOn(LTTRUE),

	m_dwLastLoadFlags(0)
{
	for (int i = 0; i < NUM_ACTIONS; i++)
		m_aActions[i] = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::~CAI()
//
//	PURPOSE:	Deallocate data
//
// ----------------------------------------------------------------------- //

CAI::~CAI()
{
	if (!g_pLTServer) return;

	delete m_pTarget;
	m_pTarget = LTNULL;

	delete m_pSenseMgr;
	m_pSenseMgr = LTNULL;

	delete m_pBidMgr;
	m_pBidMgr = LTNULL;

	for (int i = 0; i < NUM_ACTIONS; i++)
	{
		if (m_aActions[i])
			delete m_aActions[i];
	}

	delete m_pStrategyMove;
	m_pStrategyMove = LTNULL;

	// This must be done here so that charactermgr
	// will recognize our type via dynamic cast.
	// Is this an MSVC bug?
	g_pCharacterMgr->Remove(this);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CAI::EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{

			// CCharacter's Update needs to be called first so that 
			// m_cmMovement.Update is called.  That way, we get 
			// nice fresh values for GetUp, GetPosition, etc.
			DDWORD dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			Update();

			return dwRet;
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
			break;
		}

		case MID_AFFECTPHYSICS:
		{
			if( m_pCurrentAction )
				m_pCurrentAction->PhysicsMsg((LTVector*)pData);

			break;
		}

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			DDWORD dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			if (!g_pLTServer) return dwRet;

			// This needs to always be done.  
			// It needs to be done after CCharacter's initial update.
			delete m_pStrategyMove;
			m_pStrategyMove = new CAIStrategyMove(this);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();	

				if( !Consistent() )
				{
#ifndef _FINAL
					g_pLTServer->CPrint("%s being removed.",
						g_pLTServer->GetObjectName(m_hObject) );
					g_pLTServer->RemoveObject(m_hObject);
#endif
					return LT_ERROR;
				}

				if( nInfo == INITIALUPDATE_WORLDFILE )
				{
					// Cache our weapons.
					for( AvailableWeaponList::iterator iter = m_AvailableWeapons.begin(); 
					     iter != m_AvailableWeapons.end(); ++iter )
					{
						WEAPON * pWeaponData = LTNULL;
						
						// Try to get the weapon data.
						if( !(*iter)->WeaponName.empty() )
						{
							pWeaponData = g_pWeaponMgr->GetWeapon((*iter)->WeaponName.c_str());
						}
						else
						{
							uint8 nBarrelId = WMGR_INVALID_ID;
							uint8 nAmmoId = WMGR_INVALID_ID;
							uint8 nAmmoPoolId = WMGR_INVALID_ID;

							g_pWeaponMgr->ReadBarrel((*iter)->BarrelName.c_str(), nBarrelId, nAmmoId, nAmmoPoolId);

							if( nBarrelId != WMGR_INVALID_ID )
							{
								const uint8 nWeaponId = g_pWeaponMgr->FindWeaponWithBarrel(nBarrelId);
								pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
							}
						}

						// If we have the weapon data, cache it!
						if( pWeaponData )
						{
							pWeaponData->Cache(g_pWeaponMgr);
						}
					}
				}

				if( m_bStartActive )
				{
					g_pLTServer->SetNextUpdate(m_hObject, m_fUpdateDelta + GetRandom(0.0f, m_fUpdateDelta) );
					g_pLTServer->SetDeactivationTime(m_hObject, c_fAIDeactivationTime);
					g_pServerDE->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);
				}
				else
				{
					Active(false);
				}

			}

			// Initialize our member stuff.
			m_pTarget->Init(this);
			m_pSenseMgr->Init(this);

			CacheFiles();

			return dwRet;
			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
			
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}
			return dwRet;
			break;
		}

		case MID_SAVEOBJECT:
		{
			// Let our base class get the message first, so that our aggregrates will be saved!
			const DDWORD dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			Save((HMESSAGEWRITE)pData, (DDWORD)fData);

			return dwRet;
			break;
		}

		case MID_LOADOBJECT:
		{
			// Let our base class get the message first, so that our aggregrates will be loaded!
			const DDWORD dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			Load((HMESSAGEREAD)pData, (DDWORD)fData);

			return dwRet;
			break;
		}

		case MID_LINKBROKEN:
		{
			HOBJECT hBreaker = (HOBJECT) pData;

			if( m_bHasLATarget && hBreaker == m_hLATarget )
			{
				// Clear our look at, because our object just
				// went away.
				LookAt(LTNULL);
			}
		}
		break;

		default: 
		{
			break;
		}
	}

	return CCharacter::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData) return LTFALSE;

	// If we have an attribute template, fill in the info
	if ( g_pLTServer->GetPropGeneric( "AttributeTemplate", &genProp ) == DE_OK )
	{
		if ( genProp.m_String[0] )
		{
			std::string template_name = genProp.m_String;
			
			// Support legacy stuff.
			if( "AlienAIAttrib" == template_name ) template_name = "BasicAlien";
			if( "PredatorAIAttrib" == template_name ) template_name = "BasicPredator";
			if( "MarineAIAttrib" == template_name ) template_name = "BasicMarine";
			if( "CorporateAIAttrib" == template_name ) template_name = "BasicCorporate";

			m_nTemplateId = g_pAIButeMgr->GetTemplateIDByName(template_name.c_str());
			if ( m_nTemplateId < 0 )
			{
#ifndef _FINAL
				g_pLTServer->CPrint("Bad AI Attribute Template referenced! : %s", genProp.m_String );
#endif
			}
		}
		else
		{
#ifndef _FINAL
			g_pLTServer->CPrint("No attribute template specified for AI!");
#endif
		}
	} //if ( g_pLTServer->GetPropGeneric( "AttributeTemplate", &genProp ) == DE_OK )

	//	Sense Reactions
	ReadSenseReactions(genProp);

	// Commands

	if ( g_pLTServer->GetPropGeneric( "StartActive", &genProp ) == DE_OK )
	{
		m_bStartActive = genProp.m_Bool;
	}

	if ( g_pLTServer->GetPropGeneric( "StartingNode", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strInitialNode = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "Initial", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrCmdInitial = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "ActivateOn", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrCmdActivateOn = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "ActivateOff", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrCmdActivateOff = genProp.m_String;

	if (g_pLTServer->GetPropGeneric("UseCover", &genProp) == DE_OK)
		m_bUseCover = genProp.m_Bool;

	if (g_pLTServer->GetPropGeneric("UseShoulderLamp", &genProp) == DE_OK)
	{
		m_bUseShoulderLamp = genProp.m_Bool;
	}

	if (g_pLTServer->GetPropGeneric("NeverLoseTarget", &genProp) == DE_OK)
		m_bNeverLoseTarget = genProp.m_Bool;

	if (g_pLTServer->GetPropGeneric("UnreachableTargetCommand", &genProp) == DE_OK)
	{
		if ( genProp.m_String[0] )
			m_strUnreachableTargetCmd = genProp.m_String;
	}
	
	if (g_pLTServer->GetPropGeneric("LeashLength", &genProp) == DE_OK)
	{
		m_fLeashLengthSqr = genProp.m_Float*genProp.m_Float;
	}

	if (g_pLTServer->GetPropGeneric("LeashPoint", &genProp) == DE_OK)
	{
		if ( genProp.m_String[0] )
			m_strLeashFromName = genProp.m_String;
	}

	// Overrides

	if ( g_pLTServer->GetPropGeneric("Weapon", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strWeaponNames[0] = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric("SecondaryWeapon", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strWeaponNames[1] = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric("TertiaryWeapon", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strWeaponNames[2] = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric("Accuracy", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_fAccuracy = genProp.m_Float;

	if (g_pLTServer->GetPropGeneric("CanShootThrough", &genProp) == DE_OK)
		m_bShootThrough = genProp.m_Bool;

	if (g_pLTServer->GetPropGeneric("CanSeeThrough", &genProp) == DE_OK)
		m_bSeeThrough = genProp.m_Bool;

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

	// Let the SenseMgr get the sense properties

	m_pSenseMgr->GetProperties(&genProp);

	// Handle the goals
	for( int i = 0; i < 15; ++i )
	{
		std::string goal_command = ReadGoalProp(&genProp,i+1);
		if( !goal_command.empty() )
		{
			if( !m_strQueuedCommands.empty() ) m_strQueuedCommands += "; ";
			m_strQueuedCommands += goal_command;
		}
	}

	return LTTRUE;
}

void CAI::ReadSenseReactions(GenericProp &genProp)
{
	CAISense * pSense = LTNULL;

	// SeeEnemy
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemy);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stSeeEnemyTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));
			
		if ( g_pLTServer->GetPropGeneric( "1stSeeEnemyMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
				
		if ( g_pLTServer->GetPropGeneric( "SeeEnemyTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));
					
		if ( g_pLTServer->GetPropGeneric( "SeeEnemyMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// SeeCloaked
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::SeeCloaked);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stSeeCloakedTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));
			
		if ( g_pLTServer->GetPropGeneric( "1stSeeCloakedMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
				
		if ( g_pLTServer->GetPropGeneric( "SeeCloakedTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));
					
		if ( g_pLTServer->GetPropGeneric( "SeeCloakedMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// SeeEnemyFlashlight
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::SeeEnemyFlashlight);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stSeeEnemyFlashlightTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stSeeEnemyFlashlightMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "SeeEnemyFlashlightTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "SeeEnemyFlashlightMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// SeeDeadBody
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::SeeDeadBody);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stSeeDeadBodyTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stSeeDeadBodyMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "SeeDeadBodyTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "SeeDeadBodyMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// HearEnemyFootstep
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyFootstep);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearEnemyFootstepTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearEnemyFootstepMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearEnemyFootstepTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearEnemyFootstepMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// HearEnemyWeaponFire
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyWeaponFire);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearEnemyWeaponFireTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearEnemyWeaponFireMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearEnemyWeaponFireTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearEnemyWeaponFireMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// HearAllyWeaponFire
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyWeaponFire);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearAllyWeaponFireTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearAllyWeaponFireMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearAllyWeaponFireTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearAllyWeaponFireMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}


	// HearTurretWeaponFire
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearTurretWeaponFire);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearTurretWeaponFireTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearTurretWeaponFireMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearTurretWeaponFireTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearTurretWeaponFireMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}


	// HearAllyPain
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyPain);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearAllyPainTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearAllyPainMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearAllyPainTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearAllyPainMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// HearAllyBattleCry
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearAllyBattleCry);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearAllyBattleCryTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearAllyBattleCryMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearAllyBattleCryTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearAllyBattleCryMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// HearEnemyTaunt
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearEnemyTaunt);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearEnemyTauntTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearEnemyTauntMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearEnemyTauntTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearEnemyTauntMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}

	// HearDeath
	pSense = GetSenseMgr().GetSensePtr(CAISenseMgr::HearDeath);
	if (pSense)
	{
		if ( g_pLTServer->GetPropGeneric( "1stHearDeathTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "1stHearDeathMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->Set1stTriggerMessage(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearDeathTarget", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerObject(g_pLTServer->CreateString(genProp.m_String));

		if ( g_pLTServer->GetPropGeneric( "HearDeathMessage", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			pSense->SetTriggerMessage(g_pLTServer->CreateString(genProp.m_String));
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void CAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!g_pLTServer || !pStruct) return;

	// Read our attributes
	if( m_nTemplateId >= 0 )
	{
		const AIBM_Template & butes = g_pAIButeMgr->GetTemplate(m_nTemplateId);
		m_cc = butes.eCharacterClass;

		for( int i = 0; i < g_nNumAIWeapons; ++i )
		{
			if( m_strWeaponNames[i].empty() || m_strWeaponNames[i] == g_szDefault )
			{
				m_strWeaponNames[i] = butes.strWeapon[i];
			}
		}

		if( m_fAccuracy == -FLT_MAX )
			m_fAccuracy = butes.fAccuracy;

		m_fHalfForwardFOV = (LTFLOAT)cos( butes.fForwardFOV/2.0f );
		m_fHalfUpNonFOV = (LTFLOAT)cos( butes.fUpNonFOV/2.0f );
		// Let the SenseMgr get the sense attributes

		m_pSenseMgr->GetAttributes(m_nTemplateId);
		m_pTarget->GetAttributes(m_nTemplateId);

	}

	// Add all our editables

//	m_editable.AddFloatProp("SoundRadius",	&m_fSoundRadius);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::InitialUpdate()
//
//	PURPOSE:	Initialize the AI routines
//
// ----------------------------------------------------------------------- //

void CAI::InitialUpdate()
{
	m_fUpdateDelta = GetAIUpdateDelta();

	if (!g_pLTServer || !m_hObject) return;

	// Just go away if you fall through the world.
	GetMovement()->SetObjectFlags(OFT_Flags, GetMovement()->GetObjectFlags(OFT_Flags) | FLAG_REMOVEIFOUTSIDE);

	// The cylinder physics is a little slower (rough measurements said about 100 ticks more),
	// but gives much better behaviour when bumping into corners are coming over lips of inclined
	// planes.
	GetMovement()->SetObjectFlags( OFT_Flags2, GetMovement()->GetObjectFlags(OFT_Flags2) | FLAG2_SEMISOLID | FLAG2_CYLINDERPHYSICS );

	// Make stuff unguaranteed

	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	// Don't rag-doll AI anymore...

	m_damage.SetApplyDamagePhysics(LTFALSE);

	g_pLTServer->GetObjectRotation(m_hObject, &m_rTargetRot);
	g_pMathLT->GetRotationVectors(m_rTargetRot, m_vTargetRight, m_vTargetUp, m_vTargetForward);

	Goal *pGoal = CreateGoal("Idle");
	AddGoal("Idle", pGoal);

	if (m_bUseShoulderLamp)
		GetAttachments()->Attach("LeftShoulder", "ShoulderLamp");

	// Register ourself to be teleported.
	if( !m_strInitialNode.empty() )
	{
		g_pAINodeMgr->AddTeleportee(this, m_strInitialNode);
	}

	if( m_hstrCmdActivateOff || m_hstrCmdActivateOn )
	{
		SetActivateType(m_hObject, AT_NORMAL);
	}
	else
	{
		SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);
	}

	// Give us our weapons.
	// MID_INITIALUPDATE needs to be sent down to CCharacter by this time (to update the aggregrates)!
	for(int i = 0; i < g_nNumAIWeapons; ++i )
	{
		if( !m_strWeaponNames[i].empty() 
			&& m_strWeaponNames[i] != g_szNone )
		{
			AddWeapon(m_strWeaponNames[i].c_str());
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FirstUpdate()
//
//	PURPOSE:	Called on first MID_UPDATE.
//
// ----------------------------------------------------------------------- //

void CAI::FirstUpdate()
{
	// Move us to our node.
	// Yes, InitialUpdate did this already,
	// but we quite likely need to get our movement
	// state set up again.
	if( !m_strInitialNode.empty() )
	{
		const ParseLocation result(m_strInitialNode.c_str(), *this);

		if( result.pNode )
		{
			Teleport(result);
		}
		else
		{
#ifndef _FINAL
			// Print error message.
			AIErrorPrint(this,"Could not find node named \"%s\".  Staying at current location.",
				m_strInitialNode.c_str() );
#endif
		}

		m_strInitialNode.erase();
	}

	// Set up our leash.
	if( m_fLeashLengthSqr > 0.0f )
	{
		SetLeash((LTFLOAT)sqrt(m_fLeashLengthSqr), m_strLeashFromName.c_str());

		// First try a node.
/*		CAINode * pNode = g_pAINodeMgr->GetNode(m_strLeashFromName.c_str());
		if( pNode )
		{
			m_vLeashFrom = pNode->GetPos();
		}
		else
		{
			// The node doesn't exist, see if it is an object.
			ObjArray<HOBJECT,1> objArray;
			if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( m_strLeashFromName.c_str() ),
														objArray ) 
				&& objArray.NumObjects() > 0 )
			{
				HOBJECT hLeashObject = objArray.GetObject(0);
				
				g_pLTServer->GetObjectPos(hLeashObject, &m_vLeashFrom);
			}
			else
			{
				// It wasn't an object or node, just use our starting position.
				g_pLTServer->GetObjectPos(m_hObject, &m_vLeashFrom);
			}
		}
*/
		m_strLeashFromName.erase();
	}

	// Set up our exosuit stuff.
	if( IsExosuit(GetButes()) )
	{
		m_fStartRechargeLevel = 0.1f;  // PLH TODO : bute this.
		m_fStopRechargeLevel = 0.8f;  //  PLH TODO : bute this.

		ResetExosuitEnergy();
	}

#ifndef _FINAL
	// Record our name.
	m_strNameInfo = g_pLTServer->GetObjectName(m_hObject);
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CacheFiles()
//
//	PURPOSE:	Cache resources used by this AI
//
// ----------------------------------------------------------------------- //

void CAI::CacheFiles()
{
	if (!g_pLTServer || !m_hObject) return;

	if( !( g_pLTServer->GetServerFlags( ) & SS_CACHING ))
		return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CAI::HandleTouch(HOBJECT hObj)
{
	if (!g_pLTServer) return;

	m_pTarget->HandleTouch(hObj);

	const CCharacter * pChar = dynamic_cast<const CCharacter*>( g_pLTServer->HandleToObject(hObj) );
	if( pChar && g_pCharacterMgr->AreEnemies(*this, *pChar) )
	{
		m_pSenseMgr->TouchCharacter(*pChar);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessAnimationMsg()
//
//	PURPOSE:	Process an animation system message.
//
// --------------------------------------------------------------------------- //

void CAI::ProcessAnimationMsg(uint8 nTracker, uint8 bLooping, uint8 nMsg)
{
	CCharacter::ProcessAnimationMsg(nTracker, bLooping, nMsg);

	// Pass it on to the current action.
	if( m_pCurrentAction )
	{
		m_pCurrentAction->AnimMsg(nTracker, bLooping, nMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IgnoreDamageMsg()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IgnoreDamageMsg(const DamageStruct & damage_msg)
{
	if( !m_bActive )
		return LTTRUE;

	if( damage_msg.hDamager )
	{
		CCharacter * pCharDamager = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(damage_msg.hDamager) );
		CPlayerObj * pPlayerDamager = dynamic_cast<CPlayerObj*>( pCharDamager );
		if( pCharDamager && !pPlayerDamager && g_pCharacterMgr->AreAllies(*this, *pCharDamager) )
		{
			return LTTRUE;
		}
		else if( damage_msg.eType == DT_ACID && dynamic_cast<BodyProp*>( g_pLTServer->HandleToObject(damage_msg.hDamager) ) )
		{
			// Don't take damage from acid pools of body props (alien blood acid pools).
			return LTTRUE;
		}
	}
	

	return CCharacter::IgnoreDamageMsg(damage_msg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleDamage()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void CAI::ProcessDamageMsg(const DamageStruct& damage)
{
	if (!g_pLTServer || !m_hObject) return;

	CCharacter::ProcessDamageMsg(damage);

	if ( !m_damage.IsDead() )
	{
		// Deal with AI's set to never destroy.
		if( m_damage.GetNeverDestroy() )
		{
			if( m_damage.GetHitPoints() - damage.fDamage < 0.0f )
			{
				m_damage.SetHitPoints(g_pCharacterButeMgr->GetMaxHitPoints(GetCharacter()));
				m_damage.SetArmorPoints(g_pCharacterButeMgr->GetMaxArmorPoints(GetCharacter()));
			}
		}

		// Handle damage fear influence
		const AIBM_Influence & influence = g_pAIButeMgr->GetInfluence(GetTemplateId());

		// Get the most recent damage as a percentage of current health
		if ((GetHitPoints() > 0.0f) && m_tmrDamageFear.Stopped() )
		{
			m_fFraction = m_damage.GetLastDamage() / GetHitPoints();
			m_fFraction = m_fFraction * influence.fSelfDamage;

			if (m_fFraction > GetRandom(0.0f, 1.0f))
			{
				m_tmrDamageFear.Start( GetRandom( influence.fMinDamageFearDuration, influence.fMaxDamageFearDuration) );
			}
		}

	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ProcessCommand()
//
//	PURPOSE:	Handles a command.
//
// --------------------------------------------------------------------------- //

LTBOOL CAI::ProcessCommand(const char * const * pTokens, int nArgs)
{
	// Wake us back up if we get a message

	g_pLTServer->SetDeactivationTime(m_hObject, c_fAIDeactivationTime);

	// Queue it up if we haven't had an update yet (unless it is the active command!).

	if (    m_bFirstUpdate 
		 && 0 != stricmp(pTokens[0], g_szActive) 
		 && 0 != stricmp(pTokens[0], g_szHidden) 
		 && 0 != stricmp(pTokens[0], g_szRemove) )
	{
		// If this isn't the first command in the queue, prefix it with a separator

		if ( !m_strQueuedCommands.empty() )
		{
			m_strQueuedCommands += ';';
		}

		// Dump all the tokens into the queue, separated with one space

		for ( int iArg = 0 ; iArg < nArgs ; iArg++ )
		{
			m_strQueuedCommands += '(';
			m_strQueuedCommands += pTokens[iArg];
			m_strQueuedCommands += ") ";
		}

		return LTTRUE;
	}

	// Attach and detach are special cases, as they can
	// screw up our weapons!
	if( !m_AvailableWeapons.empty() )
	{
		if(    0 == _stricmp(*pTokens, TRIGGER_ATTACH) 
			|| 0 == _stricmp(*pTokens, TRIGGER_DETACH) )
		{
			if( nArgs > 1 && 0 == _stricmp(pTokens[1], "RightHand") )
			{
				ClearWeapons();
			}
		}
	}

	// Let our parent have a crack at it.
	if ( CCharacter::ProcessCommand(pTokens, nArgs) ) 
	{
		return LTTRUE;
	}

	// Let the goal have a whack at it
	if(    m_pCurrentBid 
		&& m_pCurrentBid->GetGoalPtr() 
		&& m_pCurrentBid->GetGoalPtr()->HandleCommand(pTokens, nArgs) )
	{
		return LTTRUE;
	}

	// Ignore all commands without a name, what the heck are those anyhow?
	if( nArgs < 1 )
		return LTTRUE;


	// Handle being activated (by the player).
	if ( stricmp(*pTokens, c_szActivate) == 0 )
	{
		m_bActivated = !m_bActivated;

		if ( m_bActivated && m_hstrCmdActivateOn )
		{
			SendTriggerMsgToObject(this, m_hObject, m_hstrCmdActivateOn);
		}
		else if ( !m_bActivated && m_hstrCmdActivateOff )
		{
			SendTriggerMsgToObject(this, m_hObject, m_hstrCmdActivateOff);
		}

		return LTTRUE;
	}
	else if( _stricmp(*pTokens, g_szAddGoal) == 0)
	{
		// Parse the bid command.  Format :
		// AG goal_type [goalbid bid_value] [goalname name_str] [(goal_param1; goal_param2; ...)]

		++pTokens;
		--nArgs;

		_ASSERT(nArgs >= 1 && nArgs <= 4);
		if( nArgs >= 1 && nArgs <= 4 )
		{
			Goal * pGoal = CreateGoal(pTokens[0]);	// goal type is required
			const char * szSlotName = pTokens[0];
			++pTokens;
			--nArgs;

			// TODO: get this working properly someday
			LTFLOAT fModifiedBidVal = 0.0f; //g_pAIButeMgr->GetDefaultBid(szSlotName);

			if (pGoal)
			{
				// handle optional parameters
				while( nArgs >= 1 )
				{
					if( _stricmp(*pTokens, g_szAddGoalBidParam) == 0)
					{
						++pTokens;
						--nArgs;

						if( *pTokens )
							fModifiedBidVal = (LTFLOAT)atof(*pTokens);
					}
					else if( _stricmp(*pTokens, g_szAddGoalNameParam) == 0)
					{
						++pTokens;
						--nArgs;

						if( *pTokens )
							szSlotName = pTokens[0];
					}
					else
					{
						if (*pTokens)
							pGoal->HandleGoalParameters(*pTokens++);
						else
							++pTokens;

						--nArgs;
					}
				
				}
				
				AddGoal( szSlotName, pGoal, fModifiedBidVal );
			}

		} //if( nArgs >= 2 )
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szAddGoal, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s goal_name",g_szAddGoal);
			AIErrorPrint(m_hObject, "  or  : %s goal_type priority goal_name ( goal_paramater1; goal_parameter2; ... )",g_szAddGoal,  nArgs);
		}
#endif

		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szSetGoalPriority) )
	{
		++pTokens;
		--nArgs;

		// Format:
		// SGP goal_name new_priority

		if( nArgs == 2 )
		{
			const char *  szSlotName = *pTokens++;
			--nArgs;

			const LTFLOAT fNewBid = (LTFLOAT)atof(*pTokens++);
			--nArgs;

			m_pBidMgr->SetBid(szSlotName,fNewBid);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szSetGoalPriority, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s goal_name new_priority", g_szSetGoalPriority, nArgs);
		}
#endif

		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szKillGoal) )
	{
		++pTokens;
		--nArgs;

		// Format:
		// KG goal_name

		if( nArgs == 1 )
		{
			RemoveGoal(*pTokens);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szKillGoal, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s goal_name", g_szKillGoal, nArgs);
		}
#endif

		return LTTRUE;
	}
#ifndef _FINAL
	else if ( 0 == _stricmp(*pTokens, g_szListGoals) )
	{
		for( BidMgr::const_BidIterator iter = m_pBidMgr->BeginBids();
		     iter != m_pBidMgr->EndBids(); ++iter )
		{
			const std::string & name = iter->first;
			const Bidder & bidder = iter->second;

			AICPrint(m_hObject, "%s %s %4.1f  Name: %s",
//			AICPrint(m_hObject, "%s %s %4.1f %4.1f %s",
				&bidder == m_pCurrentBid ? "*" : " ",
				bidder.GetGoalPtr() ? bidder.GetGoalPtr()->GetDescription() : "No goal!",
//				bidder.GetBaseBid(), 
				bidder.GetBid(), 
				name.c_str() );
		}

		if( m_pBidMgr->BeginBids() == m_pBidMgr->EndBids() )
		{
			AICPrint(m_hObject, "Has no goals.");
		}

		return LTTRUE;

	}
#endif
	else if (    0 == _stricmp(*pTokens, g_szScript) 
		      || 0 == _stricmp(*pTokens, g_szIdleScript)  )
	{
		const bool bIsIdleScript = (0 == _stricmp(*pTokens, g_szIdleScript));
		const char * const szOriginalCommand = *pTokens;
		++pTokens;
		--nArgs;

		// Format:
		// SCRIPT (command1 command1_params; command2 command2_params; ...)
		

		if( nArgs == 1 )
		{
			const char * szSlotName = "Script";
			if( bIsIdleScript )
				szSlotName = "IdleScript";
			
			Goal * pGoal = CreateGoal(szSlotName);
			_ASSERT( pGoal );
			if( pGoal ) 
			{
				pGoal->HandleGoalParameters(*pTokens);
				AddGoal(szSlotName,pGoal);
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", szOriginalCommand, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s (command1 command1_params; command2 command2_params; ...)", szOriginalCommand );
		}
#endif

		return LTTRUE;
	}
	else if ( 0 == stricmp(*pTokens, g_szTarget) )
	{
		++pTokens;
		--nArgs;

		// Format:
		// Target [Player | target_name]
		

		if( nArgs == 1 )
		{
			if( 0 == stricmp( *pTokens, "Player" ) )
			{
				CPlayerObj * pPlayer =  GetClosestPlayerPtr(*this);
				_ASSERT( pPlayer );
				if( pPlayer )
				{
					Target(pPlayer->m_hObject, pPlayer->GetPosition());
				}
			}
			else if( 0 == stricmp( *pTokens, "none" ) )
			{
				CPlayerObj * pPlayer =  GetClosestPlayerPtr(*this);
				_ASSERT( pPlayer );
				if( pPlayer )
				{
					Target(LTNULL);
				}
			}
			else
			{
				HOBJECT hTarget = LTNULL;
				const LTRESULT find_result = FindNamedObject(*pTokens,hTarget);
				if(  LT_OK == find_result )
				{
					LTVector vPos;
					g_pLTServer->GetObjectPos(hTarget,&vPos);

					Target( hTarget, vPos );
				}
#ifndef _FINAL
				else if( LT_ERROR == find_result )
				{
					AIErrorPrint(m_hObject, "Multiple objects are named %s.", *pTokens);
				}
				else if( LT_NOTFOUND == find_result )
				{
					AIErrorPrint(m_hObject, "Object named %s does not exist.", *pTokens);
				}
				else
				{
					_ASSERT(0);
					AIErrorPrint(m_hObject, "Unknown result from target command!");
				}
#endif
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szTarget, nArgs);
			AIErrorPrint(m_hObject, "Usage : \"%s target_name\" or \"%s none\".", g_szTarget, g_szTarget );
		}
#endif

		return LTTRUE;
	}
	else if ( 0 == _stricmp(*pTokens, g_szTeleport) )
	{
		++pTokens;
		--nArgs;


		if( nArgs == 1 )
		{
			const ParseLocation result(*pTokens, *this);
			if( !result.error.empty() )
			{
#ifndef _FINAL
				AIErrorPrint(m_hObject, "Command %s could not find location:", g_szTeleport);
				AIErrorPrint(m_hObject, const_cast<char*>(result.error.c_str()) );
#endif
				return LTTRUE;
			}
			else 
			{
				// Clear out the current goal so that it doesn't keep running without
				// knowing that it just teleported.
				if( m_pCurrentBid )
				{
					if( m_pCurrentBid->GetGoalPtr() ) m_pCurrentBid->GetGoalPtr()->End();
					m_pCurrentBid = LTNULL;
				}

				if( m_pCurrentAction )
				{
					m_pCurrentAction->End();
				}

				Teleport(result);
			}

			return LTTRUE;
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szTeleport, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s (# # #) or %s node_name or %s object_name", g_szTeleport, g_szTeleport, g_szTeleport );
		}
#endif
		return LTTRUE;
	}
	else if ( 0 == _stricmp(*pTokens, g_szRotate) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			LTVector vDest(0,0,0);

			const ParseLocation result(*pTokens, *this);
			if( !result.error.empty() )
			{
#ifndef _FINAL
				AIErrorPrint(m_hObject, "Command %s could not find location:", g_szRotate);
				AIErrorPrint(m_hObject, const_cast<char*>(result.error.c_str()) );
#endif
			}
			else if( result.hObject )
			{
				vDest = result.vPosition;
			}
			else if( result.bTarget )
			{
				vDest = m_pTarget->GetPosition();
			}
			else
			{
				vDest = result.vPosition;
			}


			if( vDest.x != FLT_MAX )
			{
				vDest += GetUp()*(GetUp().Dot(GetPosition() - vDest));

				LTRotation rRot;

#ifdef __LINUX
				LTVector vTemp = vDest - GetPosition();
				LTVector vUp = GetUp();
				g_pMathLT->AlignRotation(rRot, vDest, vUp);
#else
				g_pMathLT->AlignRotation( rRot, const_cast<LTVector&>(vDest - GetPosition()), const_cast<LTVector&>(GetUp()) );
#endif
				m_cmMovement.SetRot(rRot);
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szTeleport, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s [(# # #) | node_name | object_name]", g_szTeleport, g_szTeleport );
		}
#endif
		return LTTRUE;
	}
#ifndef _FINAL
	else if ( 0 == _stricmp(*pTokens, g_szAddToScript) || 0 == _stricmp(*pTokens, g_szInsertIntoScript) )
	{
		++pTokens;
		--nArgs;

		// Just eat the command.
		if( ShouldShowScript(m_hObject) )
		{
			AICPrint(m_hObject, "Ignoring additional script commands  : (%s)",
				*pTokens);
		}

		return LTTRUE;
	}
#endif
	else if ( 0 == _stricmp(*pTokens, g_szSay) )
	{
		++pTokens;
		--nArgs;
	
		if ( nArgs == 1 )
		{
			PlayDialogueSound( FullSoundName(*pTokens).c_str() );
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s.", g_szSay );
			AIErrorPrint(m_hObject, "Usage : %s sound_name", g_szSay );
		}
#endif
		return LTTRUE;
	}
	else if ( 0 == _stricmp(*pTokens, g_szLookAt) )
	{
		++pTokens;
		--nArgs;
	
		if( !GetButes()->m_bCanUseLookAt )
		{
#ifndef _FINAL
			AIErrorPrint(m_hObject, "\"%s\" command sent to an AI which can not use it.  Ignoring command.", g_szLookAt );
#endif
			return LTTRUE;
		}

		if ( nArgs == 1 )
		{
			const ParseLocation result(*pTokens, *this);

			if( !result.error.empty() )
			{
#ifndef _FINAL
				AIErrorPrint(m_hObject, "Incorrect location for %s.", g_szLookAt );
				AIErrorPrint(m_hObject, const_cast<char*>(result.error.c_str()) );
#endif
			}
			else if( result.bOff )
			{
				LookAt(LTNULL);
			}
			else if( result.bTarget )
			{
				LookAt(m_pTarget->GetPosition());
			}
			else if( result.hObject )
			{
				LookAt(result.hObject);
			}
			else
			{
				LookAt(result.vPosition);
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s.", g_szLookAt );
			AIErrorPrint(m_hObject, "Usage : %s [object_name | (# # #) | target | default ]", g_szLookAt );
		}
#endif

		return LTTRUE;
	}
	else if(    0 == _stricmp(*pTokens, g_szActive) )
	{
		++pTokens;
		--nArgs;

		// If you change this, please hit g_szHidden as well.
		if( nArgs == 1 )
		{
			Active( LTTRUE == IsTrueChar(pTokens[0][0]) );
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szActive, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s [0 | 1]", g_szActive );
		}
#endif
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szHidden)  )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			Active( LTTRUE != IsTrueChar(pTokens[0][0]) );
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szHidden, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s [0 | 1]", g_szHidden );
		}
#endif
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szAddWeapon) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			AddWeapon(*pTokens);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szAddWeapon, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s weapon_name", g_szAddWeapon );
		}
#endif
		return LTTRUE;
	}
	else if (_stricmp(*pTokens, g_szLeash) == 0)
	{
		++pTokens;
		--nArgs;

		LTFLOAT fLength = 0.0f;
		const char * szObject = LTNULL;

		if (nArgs >= 1)
		{
			fLength = (LTFLOAT) atof (*pTokens);
			
			--nArgs;
			++pTokens;

			if (nArgs == 1)
			{
				szObject = *pTokens;
			}

			SetLeash(fLength, szObject);
		}
		else
		{
#ifndef _FINAL
			AICPrint(m_hObject, "Given command %s with %d arguments.", g_szLeash, nArgs);
			AICPrint(m_hObject, "Usage : %s length object", g_szLeash);
#endif
		}

		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szSetWeapon) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			ClearWeapons();
			SetWeapon(*pTokens,LTTRUE);
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szSetWeapon, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s weapon_name", g_szSetWeapon );
		}
#endif
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szDropWeapons) )
	{
		++pTokens;
		--nArgs;

		// This doesn't work so well, the
		// AI usually picks the weapon right back up!
		ClearWeapons(LTTRUE);

		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szFlashlight) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			if( 0 == stricmp("on", *pTokens) )
			{
				SetFlashlight( LTTRUE );
#ifndef _FINAL
				if (!IsFlashlightOn())
					AIErrorPrint(m_hObject, "ShoulderLamp failed to turn on: missing attachment");
#endif
			}
			else
			{
				m_bForceFlashlightOn = LTFALSE;
				SetFlashlight( LTFALSE );

#ifndef _FINAL
				if( 0 != stricmp("off",*pTokens) )
				{
					AIErrorPrint(m_hObject,"Command %s sent with unknown on/off string \"%s\".  Assuming off.",
						g_szFlashlight, *pTokens);
				}
#endif
			}

		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szFlashlight, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s {on,off}", g_szFlashlight );
		}
#endif

		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szFlashlightLock) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			if( 0 == stricmp("on", *pTokens) )
			{
				m_bForceFlashlightOn = LTTRUE;
				SetFlashlight( LTTRUE );
#ifndef _FINAL
				if (!IsFlashlightOn())
					AIErrorPrint(m_hObject, "ShoulderLamp failed to turn on: missing attachment");
#endif
			}
			else
			{
				m_bForceFlashlightOn = LTFALSE;
				SetFlashlight( LTFALSE );

#ifndef _FINAL
				if( 0 != stricmp("off",*pTokens) )
				{
					AIErrorPrint(m_hObject,"Command %s sent with unknown on/off string \"%s\".  Assuming off.",
						g_szFlashlightLock, *pTokens);
				}
#endif
			}

		}
		else if( nArgs == 0 )
		{
			m_bForceFlashlightOn = LTTRUE;
			SetFlashlight( LTTRUE );
#ifndef _FINAL
			if (!IsFlashlightOn())
				AIErrorPrint(m_hObject, "ShoulderLamp failed to turn on: missing attachment");
#endif
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Given command %s with %d arguments.", g_szFlashlightLock, nArgs);
			AIErrorPrint(m_hObject, "Usage : %s {on,off}", g_szFlashlightLock );
		}
#endif
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szRun) )
	{
		++pTokens;
		--nArgs;

		Run();
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szWalk) )
	{
		++pTokens;
		--nArgs;

		m_bRunLock = LTFALSE;
		Walk();
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szCrouch) )
	{
		++pTokens;
		--nArgs;

		Crouch();
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szWallWalk) )
	{
		++pTokens;
		--nArgs;

		WallWalk();
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szStand) )
	{
		++pTokens;
		--nArgs;

		m_bCrouchLock = LTFALSE;
		StandUp();
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szSetMovementStyle) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			if( 0 == _stricmp(*pTokens,"default") )
			{
				SetMovementStyle();
			}
			else
			{
				SetMovementStyle(*pTokens);
			}
		}
		else
		{
			SetMovementStyle();
#ifndef _FINAL
			AIErrorPrint(m_hObject, "Incorrect format for %s, assuming \"%s default\".", g_szSetMovementStyle, g_szSetMovementStyle);
			AIErrorPrint(m_hObject, "Usage : %s [style_name | default ]", g_szSetMovementStyle );
#endif
		}
		
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szKillScripts) )
	{
		++pTokens;
		--nArgs;

		// This will contain the name of all script goals.
		typedef std::vector<std::string> BidNameContainer;
		BidNameContainer astrBidsToRemove;

		// Find all script goals.
		{for( BidMgr::const_BidIterator iter = m_pBidMgr->BeginBids();
		     iter != m_pBidMgr->EndBids(); ++iter )
		{
			const std::string & strBidName = iter->first;
			const Bidder & bidder = iter->second;
			if( dynamic_cast<ScriptGoal*>( bidder.GetGoalPtr() ) )
			{
				astrBidsToRemove.push_back( strBidName );
			}
		}}
		
		// Remove all script goals.
		{for( BidNameContainer::const_iterator iter = astrBidsToRemove.begin();
		      iter != astrBidsToRemove.end(); ++iter )
		{
			RemoveGoal(*iter);
		}}
		  
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szKillAction) )
	{
		++pTokens;
		--nArgs;


		if( m_pCurrentAction )
		{
#ifndef _FINAL
			AIErrorPrint(m_hObject, "Killing action \"%s\".", m_pCurrentAction->GetActionDescription() );
#endif
			// End the current action
			m_pCurrentAction->End();

			// We now have no current actions!
			m_pCurrentAction = LTNULL;
		}

		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens, g_szNeverLoseTarget) )
	{
		++pTokens;
		--nArgs;

		if( nArgs <= 1 )
		{
			if( nArgs == 1 && 0 == stricmp(*pTokens,"off") )
			{
				m_bNeverLoseTarget = LTFALSE;
			}
			else
			{
				m_bNeverLoseTarget = LTTRUE;
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szNeverLoseTarget);
			AIErrorPrint(m_hObject, "Usage : \"%s on\" or \"%s off\".", g_szNeverLoseTarget, g_szNeverLoseTarget );
		}
#endif
		return LTTRUE;
	}
	else if( 0 == _stricmp(*pTokens,g_szSetDeathAnim) )
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			if( 0 == stricmp(*pTokens,"none") )
			{
				m_strDefaultDeathAnim = "";
			}
			else
			{
				m_strDefaultDeathAnim = *pTokens;

				// Be sure we were given a valid animation.
				if( INVALID_ANI == g_pLTServer->GetAnimIndex( m_hObject, const_cast<char*>(m_strDefaultDeathAnim.c_str()) ) )
				{
#ifndef _FINAL
					AIErrorPrint(m_hObject, "Does not have a death animation named \"%s\".  Using default death animations.", m_strDefaultDeathAnim.c_str() );
#endif
					m_strDefaultDeathAnim = "";
				}
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szSetDeathAnim);
			AIErrorPrint(m_hObject, "Usage : \"%s none\" or \"%s animation_name\".", g_szSetDeathAnim, g_szSetDeathAnim );
		}
#endif
		return LTTRUE;
	}
	else if ( 0 == _stricmp(pTokens[0], g_szShootThrough) )
	{
		if ( pTokens[1] && *pTokens[1] )
		{
			m_bShootThrough = IsTrueChar(*pTokens[1]);

			return LTTRUE;
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szShootThrough);
			AIErrorPrint(m_hObject, "Usage : \"%s 0\" or \"%s 1\".", g_szShootThrough, g_szShootThrough );
		}
#endif
	}
	else if ( 0 == _stricmp(pTokens[0], g_szSeeThrough) )
	{
		if ( pTokens[1] && *pTokens[1] )
		{
			m_bSeeThrough = IsTrueChar(*pTokens[1]);

			return LTTRUE;
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szSeeThrough);
			AIErrorPrint(m_hObject, "Usage : \"%s 0\" or \"%s 1\".", g_szSeeThrough, g_szSeeThrough );
		}
#endif
	}
	else if ( 0 == _stricmp(pTokens[0], g_szRemove) )
	{
		RemoveObject();

		return LTTRUE;
	}
	else if ( 0 == _stricmp(pTokens[0], g_szAlertLock) )
	{
		++pTokens;
		--nArgs;

		if( nArgs <= 1 )
		{
			if( nArgs == 1 && 0 == stricmp(*pTokens,"off") )
			{
				m_bAlertLock = LTFALSE;
			}
			else
			{
				m_bAlertLock = LTTRUE;
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szAlertLock);
			AIErrorPrint(m_hObject, "Usage : \"%s on\" or \"%s off\".", g_szAlertLock, g_szAlertLock );
		}
#endif
		return LTTRUE;
	}
	else if ( 0 == _stricmp(pTokens[0], g_szRunLock) )
	{
		++pTokens;
		--nArgs;

		if( nArgs <= 1 )
		{
			if( nArgs == 1 && 0 == stricmp(*pTokens,"off") )
			{
				m_bRunLock = LTFALSE;
			}
			else
			{
				m_bRunLock = LTTRUE;

				Run();
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szRunLock);
			AIErrorPrint(m_hObject, "Usage : \"%s on\" or \"%s off\".", g_szRunLock, g_szRunLock );
		}
#endif
		return LTTRUE;
	}
	else if ( 0 == _stricmp(pTokens[0], g_szCrouchLock) )
	{
		++pTokens;
		--nArgs;

		if( nArgs <= 1 )
		{
			if( nArgs == 1 && 0 == stricmp(*pTokens,"off") )
			{
				m_bCrouchLock = LTFALSE;
			}
			else
			{
				m_bCrouchLock = LTTRUE;

				Crouch();
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szCrouchLock);
			AIErrorPrint(m_hObject, "Usage : \"%s on\" or \"%s off\".", g_szCrouchLock, g_szCrouchLock );
		}
#endif
		return LTTRUE;
	}
	else if ( 0 == _stricmp(pTokens[0], g_szInvestigate) )
	{
		// This command needs to be passed down to the investigate goal, but only
		// if there isn't a higher priority goal running.
		for( BidMgr::BidIterator iter = m_pBidMgr->BeginBids(); iter != m_pBidMgr->EndBids(); ++iter )
		{
			InvestigateGoal * pInvestigateGoal = dynamic_cast<InvestigateGoal *>( (*iter).second.GetGoalPtr() );
			if( pInvestigateGoal )
			{
				// Only pass on the command if we don't have a more important goal.
				if( !m_pCurrentBid || m_pCurrentBid->GetBid() < pInvestigateGoal->GetMaximumBid() )
					pInvestigateGoal->HandleCommand(pTokens,nArgs);

				break;
			}
		}

		return LTTRUE;
	}
	
	else if (0 == _stricmp(*pTokens, g_szSetClass))
	{
		++pTokens;
		--nArgs;

#ifndef _FINAL
		if (nArgs != 1)
		{
			AIErrorPrint(m_hObject,"Usage: SetClass [unknown | marine | predator | alien | corporate | synthetic]");
		}
#endif
		if (nArgs == 1)
		{
			if (0 == _stricmp(*pTokens, "UNKNOWN"))
				SetCharacterClass(UNKNOWN);
			else if (0 == _stricmp(*pTokens, "MARINE"))
				SetCharacterClass(MARINE);
			else if (0 == _stricmp(*pTokens, "PREDATOR"))
				SetCharacterClass(PREDATOR);
			else if (0 == _stricmp(*pTokens, "ALIEN"))
				SetCharacterClass(ALIEN);
			else if (0 == _stricmp(*pTokens, "CORPORATE"))
				SetCharacterClass(CORPORATE);
			else if (0 == _stricmp(*pTokens, "SYNTHETIC"))
				SetCharacterClass(SYNTHETIC);

			CCharacterMgr::CharacterIterator iter;
			for (iter = g_pCharacterMgr->BeginCharacters(); iter != g_pCharacterMgr->EndCharacters(); ++iter)
			{
				g_pCharacterMgr->SetRelationship(*this, **iter, g_pCharacterMgr->FindRelationship(*this, **iter));
			}
		}

		return LTTRUE;
	}
	else if (0 == _stricmp(*pTokens, g_szResetSense))
	{
		++pTokens;
		--nArgs;

		if( nArgs == 1 )
		{
			if( 0 == _stricmp(*pTokens,"ALL") )
			{
				for( CAISenseMgr::SenseIterator iter = m_pSenseMgr->BeginSenses(); 
				     iter != m_pSenseMgr->EndSenses(); ++iter )
				{
					(*iter)->Reset1stReaction();
				}
			}
			else
			{
				bool bFoundSense = false;
				for( CAISenseMgr::SenseIterator iter = m_pSenseMgr->BeginSenses(); 
				     iter != m_pSenseMgr->EndSenses(); ++iter )
				{
					if( 0 == stricmp(*pTokens, (*iter)->GetDescription()) )
					{
						bFoundSense = true;
						(*iter)->Reset1stReaction();
						break;
					}
				}

#ifndef _FINAL
				if( !bFoundSense )
				{
					AIErrorPrint(m_hObject, "Sense \"%s\" not found.  Ignoring command \"%s %s\".",
						*pTokens, g_szResetSense, *pTokens);
				}
#endif
			}
		}
#ifndef _FINAL
		else
		{
			AIErrorPrint(m_hObject, "Incorrect format for %s. Ignoring command.", g_szCrouchLock);
			AIErrorPrint(m_hObject, "Usage : \"%s all\" or \"%s SenseName\" where SenseName is the name of a sense.", g_szCrouchLock, g_szCrouchLock );
		}
#endif
		return LTTRUE;
	}

	return LTFALSE;

} //LTBOOL CAI::ProcessCommand(const char * const * pTokens, int nArgs)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PreUpdate
//
//	PURPOSE:	Does our Preupdate
//
// ----------------------------------------------------------------------- //

void CAI::PreUpdate()
{
	ClearMovementFlags();
	m_dwMovementAnimFlags = GetMovement()->GetControlFlags();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Update()
//
//	PURPOSE:	Update the AI
//
// ----------------------------------------------------------------------- //
static bool IsFallingState(CharacterMovementState eState)
{
	return false;

	return eState == CMS_PRE_FALLING
		|| eState == CMS_STAND_FALL
		|| eState == CMS_WALK_FALL
		|| eState == CMS_RUN_FALL
		|| eState == CMS_WWALK_FALL
		|| eState == CMS_STAND_LAND
		|| eState == CMS_WALK_LAND
		|| eState == CMS_RUN_LAND
		|| eState == CMS_WWALK_LAND;
}

static bool IsAlienLungeState(CharacterMovementState eState)
{
	return eState == CMS_FACEHUG_ATTACH 
		|| eState == CMS_POUNCING 
		|| eState == CMS_FACEHUG;
}

static bool IsSpecialMovementState(CharacterMovementState eState)
{
	return IsAlienLungeState(eState)
		|| eState == CMS_SPECIAL
		|| IsFallingState(eState);
}

void CAI::Update()
{
//	const uint32 nStartISegCalls = g_cIntersectSegmentCalls;

//	LineSystem::GetSystem(this,"Facings")
//		<< LineSystem::Clear()
//		<< LineSystem::Arrow( GetPosition(), GetPosition() + GetRight()*200.0f, Color::Cyan )
//		<< LineSystem::Arrow( GetPosition(), GetPosition() + GetUp()*200.0f, Color::Cyan )
//		<< LineSystem::Arrow( GetPosition(), GetPosition() + GetForward()*200.0f, Color::Cyan );


	
	// PreUpdate

	PreUpdate();

	if( m_bFirstUpdate )
	{
		FirstUpdate();
	}

	// Only do the update if we're active.
	const bool bInSpecialMovementState = IsSpecialMovementState(GetMovement()->GetMovementState());
	if( m_bActive 
		&& !bInSpecialMovementState
		&& !IsNetted() )
	{
		m_bUsingJumpTo = LTFALSE;
		m_bUsingClipTo = LTFALSE;

		// Show our position.
#ifndef _FINAL
		if( ShouldShowPos(m_hObject) )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			AICPrint( m_hObject,"pos. is (%.2f,%.2f,%.2f).",
					  vPos.x,vPos.y,vPos.z );
		}
#endif
		// Update our ground info

		UpdateOnGround();

		// Update our wee little eyes
		// (well, just their position).
		UpdateEye();


		// Send the initial message to ourselves if we have one
		if ( m_hstrCmdInitial )
		{
			SendTriggerMsgToObject(this, m_hObject, m_hstrCmdInitial);

			m_hstrCmdInitial = LTNULL;
		}
	/*
		// Update our debug iseg junk

		static int s_cTotalCalls = 0;
		static int s_cTotalUpdates = 0;
		s_cTotalCalls += g_cIntersectSegmentCalls;
		s_cTotalUpdates++;
		g_pLTServer->CPrint("iseg calls = %3.3d avg = %f", g_cIntersectSegmentCalls, (float)s_cTotalCalls/(float)s_cTotalUpdates);
		g_cIntersectSegmentCalls = 0;
	*/

		// Handle game restore stuff

		if (m_dwLastLoadFlags == LOAD_RESTORE_GAME || m_dwLastLoadFlags == LOAD_NEW_LEVEL)
		{
			m_dwLastLoadFlags = 0;
		}


		// Don't do any of this business if we're dead or locked

		if ( !m_damage.IsDead() && !m_bLocked )
		{
			// Update our senses.
			m_pSenseMgr->Update();

			if( CanDoNewAction() )
			{
				// Be sure this is called less often
				// than the goal updates!
				// Also be sure this is called before anything
				// that uses senses (like CAITarget!).
				m_pSenseMgr->AutoDisableSenses();
			}

			// Update any target we're locked in on.
			m_pTarget->Update();

#ifndef _FINAL
			// If we do not have a next action, go to the idle action.
			if( m_pTarget->IsValid() )
			{
				m_strTargetInfo = "\nT : ";
				m_strTargetInfo += g_pLTServer->GetObjectName(m_pTarget->GetObject());
			}
			else
			{
				m_strTargetInfo = "";
			}
#endif



			// Update the influences on this AI
			UpdateInfluences();

			// Clean up an action which just finished.
			if( m_pCurrentAction && m_pCurrentAction->IsDone() )
			{
#ifndef _FINAL
				if( ShowActionLevel(m_hObject) > 0.0f )
				{
					AICPrint(m_hObject, "Ending action \"%s\".", m_pCurrentAction->GetActionDescription() );
				}
#endif
				// End the current action
				m_pCurrentAction->End();

				// We now have no current actions!
				m_pCurrentAction = LTNULL;

#ifndef _FINAL
				if( !m_pNextAction )
				{
					// If we do not have a next action, go to the idle action.
					char szTimeStamp[12];
					sprintf(szTimeStamp, "\n%4.2f ", g_pLTServer->GetTime());
					m_strActionInfo = szTimeStamp;
					m_strActionInfo += "A : ";
					m_strActionInfo += "Idle";
				}
#endif
			}

			// If we can do new actions, update the goals.
			if( CanDoNewAction() )
			{
				// Update the bid system (may change current bid).
				UpdateBids();

				// Update the current goal.
				if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() )
				{
					m_pCurrentBid->GetGoalPtr()->Update();

#ifndef _FINAL
					// Update our info string.
					if( m_pCurrentBid )
					{
						if( ScriptGoal * pScriptGoal = dynamic_cast<ScriptGoal*>(m_pCurrentBid->GetGoalPtr()) )
						{
							m_strScriptInfo = "\nS : ";

							if( pScriptGoal->GetCurrentCommand() )
							{
								m_strScriptInfo += pScriptGoal->GetCurrentCommand()->GetCommandDescription();
							}
							else
							{
								m_strScriptInfo += "Empty Script";
							}
						}
						else
						{
							m_strScriptInfo = "";
						}
					}
#endif
				}
			}


			// Start a new action if we have one.
			if( !m_pCurrentAction && m_pNextAction )
			{
				// Transfer m_pNextAction to m_pCurrentAction
				m_pCurrentAction = m_pNextAction;
				m_pNextAction = LTNULL;

				// Start the new current action.
				if( m_pCurrentAction )
				{
#ifndef _FINAL
					if( ShowActionLevel(m_hObject) > 0.0f )
					{
						AICPrint(m_hObject, "Starting action \"%s\".", m_pCurrentAction->GetActionDescription() );
					}
#endif
					m_pCurrentAction->Start();

#ifndef _FINAL
					// Update our display info.
					char szTimeStamp[12];
					sprintf(szTimeStamp, "\n%4.2f ", g_pLTServer->GetTime());
					m_strActionInfo = szTimeStamp;
					m_strActionInfo += "A : ";
					m_strActionInfo += m_pCurrentAction->GetActionDescription();
#endif
				}
			}

			// Update the current action if we have one.
			if( m_pCurrentAction && !m_pCurrentAction->IsDone() )
			{
#ifndef _FINAL
				if( ShowActionLevel(m_hObject) > 1.0f )
				{
					AICPrint(m_hObject, "Updating action \"%s\".", m_pCurrentAction->GetActionDescription() );
				}
#endif
				m_pCurrentAction->Update();
			}

			// Update our movement (this will be a second time) so that any
			// changes to movement will take place immediately.
			m_cmMovement.Update(GetUpdateDelta());
		}

		// Do our aiming.
		UpdateAiming();

		// If we have a flashlight...
		if(IsFlashlightOn())
			UpdateFlashlight();

		// Stop any movement if we are recharging.
		if( m_bRechargingExosuit )
		{
			m_dwMovementFlags &= ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD);
			m_dwMovementFlags &= ~(CM_FLAG_STRAFERIGHT | CM_FLAG_STRAFELEFT);
		}

		// Be sure we are crouching if required.
		if( GetInjuredMovement() || GetMustCrawl() )
		{
			m_dwMovementFlags |= CM_FLAG_DUCK;
		}

		// Set our movement flags.
		m_cmMovement.SetControlFlags( m_dwMovementFlags );

		// update exosuit energy management.
		if( m_fStopRechargeLevel > 0.0f )
		{
			const uint32 max_energy = GetMaxExosuitEnergy();
			if( max_energy > 0 )
			{
				const uint32 energy = GetExosuitEnergy();

				if( !m_bRechargingExosuit && energy < m_fStartRechargeLevel*max_energy )
				{
					m_bRechargingExosuit = LTTRUE;
				}
				else if( m_bRechargingExosuit && energy > m_fStopRechargeLevel*max_energy )
				{
					m_bRechargingExosuit = LTFALSE;
				}
			}
		}

	} //if( m_bActive && .....)
	else
	{
		// Clear out our current action.
		if( m_pCurrentAction )
		{
			// Some actions put us in this situation, let those actions survive.
			if( !bInSpecialMovementState || !m_pCurrentAction->UsesMovementState(m_cmMovement.GetMovementState()) )
			{
				m_pCurrentAction->End();
				m_pCurrentAction = LTNULL;
			}
		}

		if( m_pNextAction )
		{
			// Try to return ourselves to a sane state.
			m_pNextAction = LTNULL;
		}

		SetNextAction(CAI::ACTION_IDLE);

		if(  IsAlienLungeState(GetMovement()->GetMovementState()) )
		{
			// These particular states invalidate our last reached
			// node information.
			m_pNextNode = LTNULL;
			m_pLastNode = LTNULL;
		}

		if( IsFallingState(GetMovement()->GetMovementState()) )
		{
			UpdateOnGround();
		}

	}

	// Keep our music going.
	if( m_bActive )
		UpdateMusic();
	
	// Set the time for our next update
	if( m_bActive )
	{
		g_pLTServer->SetNextUpdate(m_hObject, m_fUpdateDelta);
	}


#ifndef _FINAL
	// Update our info string.
	if( m_bActive && (InfoAIState() || InfoAIListGoals()) )
	{
		std::string info = m_strNameInfo;

		if( InfoAIState() )
		{
			info += m_strGoalInfo + m_strActionInfo + m_strTargetInfo + m_strScriptInfo;
		}

		if( InfoAIListGoals() )
		{
			// Iterate through goals and add their description to info string.
			char szTemp[256];
			for( BidMgr::const_BidIterator iter = m_pBidMgr->BeginBids();
				 iter != m_pBidMgr->EndBids(); ++iter )
			{
				const Bidder & bidder = iter->second;

				sprintf( szTemp, "\n%s %.0f(%.0f) ", 
					&bidder == m_pCurrentBid ? "-->" : "",
					bidder.GetBid(), 
					bidder.GetMaximumBid() );

				info += szTemp;
				info += bidder.GetGoalPtr() ? bidder.GetGoalPtr()->GetDescription() : "Invalid Goal!";

			}
		}

		if( info != m_strCurrentInfo )
		{
			m_strCurrentInfo = info;
			UpdateInfoString( const_cast<char*>(m_strCurrentInfo.c_str()) );
		}
	}
	else
	{
		if( !m_strCurrentInfo.empty() )
		{
			m_strCurrentInfo = "";
			UpdateInfoString("");
		}
	}
#endif
	// Post update

	PostUpdate();

//	AICPrint(m_hObject, "Intersect Segment called %d.", g_cIntersectSegmentCalls - nStartISegCalls);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostUpdate
//
//	PURPOSE:	Does our postupdate
//
// ----------------------------------------------------------------------- //

void CAI::PostUpdate()
{
	// Set first update flag

	m_bFirstUpdate = LTFALSE;

	// Do any queued commands
	if ( !m_strQueuedCommands.empty() )
	{
		SendTriggerMsgToObject(this, m_hObject, m_strQueuedCommands.c_str());
		m_strQueuedCommands = "";
	}

	if( m_hCinematicTrigger )
	{
		g_pLTServer->SetDeactivationTime(m_hObject, c_fAIDeactivationTime);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateBids
//
//	PURPOSE:	Updates bid system and changes goal if new higher bid is found
//
// ----------------------------------------------------------------------- //

void CAI::UpdateBids()
{
	// Garbage collect our goal pointers.
	m_pBidMgr->CleanUpGoals();

	Bidder * new_highest_bidder = LTNULL;
	LTFLOAT highest_bid;

	if (m_pCurrentBid)
		highest_bid = m_pCurrentBid->GetBid();
	else
		highest_bid = 0.0f;

	for(BidMgr::BidList::iterator iter = m_pBidMgr->BeginBids();
	    iter != m_pBidMgr->EndBids(); ++iter )
	{
		Bidder & current_bidder = iter->second;

		if( current_bidder.GetMaximumBid() >= highest_bid )
		{
			const LTFLOAT current_bid = current_bidder.GetBid();
			if(  current_bid > highest_bid )
			{
				highest_bid = current_bid;
				new_highest_bidder = &current_bidder;
			}
		}
	}	

	// Set the new winner, if we have one.
	if( new_highest_bidder )
	{
		// End the current bid.  Erase it if need be.
		if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() )
		{
			m_pCurrentBid->GetGoalPtr()->End();

			if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() && m_pCurrentBid->GetGoalPtr()->DestroyOnEnd() )
				m_pBidMgr->RemoveBid( *m_pCurrentBid );
		}

		// Set the new current bid.
		m_pCurrentBid = new_highest_bidder;

		// Start the new bid.
		if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() )
		{
			// Start the goal
			m_pCurrentBid->GetGoalPtr()->Start();

#ifndef _FINAL
			// The previous start could have removed the current goal, so 
			// be sure to check m_pCurrentBid again.
			if( m_pCurrentBid )
			{
				// Update our debug info.
				char szTimeStamp[12];
				sprintf(szTimeStamp, "\n%4.1f ", g_pLTServer->GetTime() );
				m_strGoalInfo = szTimeStamp;
				m_strGoalInfo += "G : ";
				m_strGoalInfo += m_pCurrentBid->GetGoalPtr() ? m_pCurrentBid->GetGoalPtr()->GetDescription() : "Broken Goal!";
			}
#endif
		}

#ifdef BID_DEBUG
		AICPrint(m_hObject, "Goal \"%s\" won at %f.",
			(m_pCurrentBid && m_pCurrentBid->GetGoalPtr())
			   ? m_pCurrentBid->GetGoalPtr()->GetCreateName().c_str()
			   : "<removed when started>",
			highest_bid );
#endif
	}
	// if the current bid is invalid, don't let it live!
	else if( m_pCurrentBid && highest_bid <= 0.0f )
	{
		// End the current bid.  Erase it if need be.
		if( m_pCurrentBid->GetGoalPtr() )
		{
			m_pCurrentBid->GetGoalPtr()->End();

			if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() && m_pCurrentBid->GetGoalPtr()->DestroyOnEnd() )
				m_pBidMgr->RemoveBid( *m_pCurrentBid );
		}

		m_pCurrentBid = LTNULL;

#ifdef BID_DEBUG
		AICPrint(m_hObject, "All goals are invalid.");
#endif

		if (IsFlashlightOn())
			SetFlashlight(LTFALSE);
	}


	// Hack to allow for interuptable cinematic triggers.  The cinematic
	// is ended if the AI does not have an active script goal.
	if( m_hCinematicTrigger && m_pCurrentBid )
	{
		if( m_pCurrentBid->GetGoalPtr()->StopCinematic()  )
		{
			EndCinematic();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::UpdateMusic
//
//	PURPOSE:	Updates the music.
//
// ----------------------------------------------------------------------- //

CMusicMgr::EMood CAI::GetMusicMoodModifier()
{
	if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() )
		return m_pCurrentBid->GetGoalPtr()->GetMusicMoodModifier();

	return CMusicMgr::eMoodNone;
}

void CAI::UpdateMusic()
{
	// Get the music mood modifier.
	CMusicMgr::EMood eMood = GetMusicMoodModifier();

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


//------------------------------------------------------------------------------------
// CAI::UpdateInfluences()
//
// This method sets flags based on external information that the AI does not receive
// through its senses.  For example, it observes the number of friends and enemies in
// the region.
//------------------------------------------------------------------------------------
void CAI::UpdateInfluences()
{
	const AIBM_Influence & influence = g_pAIButeMgr->GetInfluence(GetTemplateId());
	const LTFLOAT fCharRangeSqr = influence.fCharacterRange * influence.fCharacterRange;

	LTFLOAT fFraction = 0.0f;
	int nEnemies = 0, nFriends = 0;

	// Limit frequency of updates
	if (m_tmrInfluence.Stopped())
		m_tmrInfluence.Start(influence.fUpdateTime);
	else
		return;

	// Reset friend distance, if no injured friends exist
	if (!m_hFriend)
		m_fFriendDistSqr = fCharRangeSqr;

	if(    influence.nOddsLimit < 1000
		|| influence.fFriendDamage > 0.0f )
	{
		for (CCharacterMgr::CharacterIterator iter = g_pCharacterMgr->BeginCharacters();
		iter != g_pCharacterMgr->EndCharacters(); ++iter)
		{
			const CCharacter * pChar = *iter;
			
			// ignore self
			if (pChar == this)
				continue;
			
			// Only check for nearby friends/enemies
			const LTFLOAT fDistSqr = VEC_DISTSQR(pChar->GetPosition(), GetPosition());
			if (fDistSqr < fCharRangeSqr)
			{
				if (g_pCharacterMgr->AreEnemies(*pChar, *this))
				{
					nEnemies++;
				}
				else
				{
					nFriends++;
					
					if (!m_bFriendLocked && influence.fFriendDamage > 0.0f )
					{
						// check for the nearest injured friend
						fFraction = (pChar->GetHitPoints() / pChar->GetMaxHitPoints());
						if (fFraction < influence.fFriendDamage)
						{
							if (fDistSqr < m_fFriendDistSqr)
							{
								m_hFriend = pChar->m_hObject;
								m_fFriendDistSqr = fDistSqr;
							}
						}
					}
				}
			}

			// Goals are responsible for setting this value back to LTFALSE
			// when the AI is done assisting the injured friend.  Otherwise 
			// it doesn't switch friends until the current one dies.
			if (m_hFriend)
				m_bFriendLocked = LTTRUE;
		}

		// Is the AI afraid when outnumbered?
		if ((nEnemies - nFriends) > influence.nOddsLimit)
			m_bOddsFear = LTTRUE;
		else
			m_bOddsFear = LTFALSE;
	}

	// Is the AI afraid because it is injured?

	// 01/05/01 - MarkS
	// Changed this to cause fear when an AI takes a lot of damage in a single shot

//	fFraction = (GetHitPoints() / GetMaxHitPoints());
//	if (fFraction < influence.fSelfDamage)
//		m_bDamageFear = LTTRUE;
//	else
//		m_bDamageFear = LTFALSE;


	// We only need to have a damage fear
	// if this fraction is greater than zero.
	// It gets set in ProcessDamageMsg.
	// It is lowered in the code below.
/*  This is all done in handle damage now.
	if (m_fFraction > 0.0f )
	{
		// See if we should be afraid.
		if (m_fFraction > GetRandom(0.0f, 1.0f))
		{
			if( m_tmrDamageFear.Stopped() )
				m_tmrDamageFear.Start( GetRandom( influence.fMinDamageFearDuration, influence.fMaxDamageFearDuration) );

			// Reduce the fraction, eventually
			// the fraction will fall to a point
			// where the GetRandom check will
			// fail and the AI will stop being afraid.
			m_fFraction -= influence.fFearDecay;

			_ASSERT( m_fFraction < 2.0f );
		}
		else
		{
			// Be sure the fraction is set to zero
			// so that we want set m_bDamageFear to 
			// true in the next influence update!
			m_fFraction = 0.0f;
		}
	}
*/
			
	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateOnGround
//
//	PURPOSE:	Deals with AI's jumping and falling.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateOnGround()
{
	CCharacter::UpdateOnGround();

	// Detect a change in our "standing-on" condition.
	if( m_bWasStandingOn )
	{
		if( !m_cmMovement.GetCharacterVars()->m_bStandingOn )
		{
			m_bWasStandingOn = LTFALSE;
			m_vLastStandingPos = GetPosition();
		}

	}
	else
	{
		if( m_cmMovement.GetCharacterVars()->m_bStandingOn )
		{
			m_bWasStandingOn = LTTRUE;

			const LTFLOAT fMinFallingDistSqr = 50.0f*50.0f; 
			if( (GetPosition() - m_vLastStandingPos).MagSqr() > fMinFallingDistSqr )
			{
				m_pLastNode = LTNULL;
				m_pNextNode = LTNULL;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::StartDeath
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAI::StartDeath()
{
	// This will create the body prop and drop weapon powerups...
	CCharacter::StartDeath();

	// Play music to accompany player's death.
	g_pMusicMgr->PlayAIDeathEvent( );

	// Clear our current weapon.
//	SetWeapon(-1);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetDefaultDeathAni
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

std::string CAI::GetDefaultDeathAni()
{
	if( !m_strDefaultDeathAnim.empty() )
	{
		return m_strDefaultDeathAnim;
	}

	return CCharacter::GetDefaultDeathAni();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ForceDefaultDeathAni
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::ForceDefaultDeathAni()
{
	return !m_strDefaultDeathAnim.empty();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsAlert
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTBOOL  CAI::IsAlert() const
{
	if( !m_bAlertLock )
	{
		if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() )
		{
			return m_pCurrentBid->GetGoalPtr()->DefaultAlert();
		}
	}

	return m_bAlertLock;
}



LTBOOL CAI::WillHurtFriend()
{
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	if (!m_pCurrentWeapon)
		return LTFALSE;

	const LTVector vFirePos = GetWeaponPosition(m_pCurrentWeapon);
	const LTVector vPath = GetAimForward();

	iQuery.m_From = GetWeaponPosition(m_pCurrentWeapon);

	if (GetTarget().IsValid())
		iQuery.m_To = GetTarget().GetPosition();
	else
		iQuery.m_To = vFirePos + vPath*LTFLOAT(m_pCurrentWeapon->GetBarrelData()->nRange);

	iQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iQuery.m_FilterFn = CAI::DefaultFilterFn;
	iQuery.m_pUserData = LTNULL;

	g_cIntersectSegmentCalls++;
	if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )
	{
#ifdef AI_WEAPON_DEBUG
		AICPrint(m_hObject,"Would hit self if fire weapon \"%s\".",
			m_pCurrentWeaponButes->Name.c_str());
#endif
		// Don't fire if we intersected an ally
		if( iInfo.m_hObject )
		{
			const CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(iInfo.m_hObject) );
			if( pChar && g_pCharacterMgr->AreAllies(*this,*pChar) )
			{
				return LTTRUE;
			}
		}

		// Don't fire if it is suicidal
		if( m_pCurrentWeapon->GetAmmoData()->nAreaDamageRadius > 0 )
		{
			if( (iInfo.m_Point - GetPosition()).MagSqr() < LTFLOAT(m_pCurrentWeapon->GetAmmoData()->nAreaDamageRadius)*LTFLOAT(m_pCurrentWeapon->GetAmmoData()->nAreaDamageRadius) )
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}



bool CAI::BurstDelayExpired()
{

	// Don't fire while netted!
	if( IsNetted() )
		return false;

	if( !m_pCurrentWeaponButes )
		return false;

	// If the timer isn't on, then consider it expired!
	if( !m_tmrBurstDelay.On() )
		return true;

	// Wait for burst delay to finish.
	if( m_tmrBurstDelay.Stopped() )
	{
		// Okay, the burst delay timer has stopped since the last call.
		if( m_tmrBurstDelay.On()  )
		{
			if(     GetRandom(0.0f,1.0f) < m_pCurrentWeaponButes->fBurstChance 
				|| (    m_pCurrentWeaponButes->fHardMaxBurstDelay > 0
					 && m_tmrLastBurst.GetElapseTime() > m_pCurrentWeaponButes->fHardMaxBurstDelay ) )
			{
				// Turn off the burst delay timer.
				m_tmrBurstDelay.Stop();

				// Set our aim at head chance.
				// (This is a per burst thing, maybe it should be in AIActionFireWeapon?)
				if( m_pCurrentWeaponButes->fAimAtHeadChance > 0.0f )
					m_pTarget->SetAimAtHead( GetRandom(0.0f,1.0f) < m_pCurrentWeaponButes->fAimAtHeadChance );

				return true;
			}
			else
			{
				// We failed our chance to fire, restart the delay timer.
				m_tmrBurstDelay.Start( GetRandom(m_pCurrentWeaponButes->fMinBurstDelay,m_pCurrentWeaponButes->fMaxBurstDelay) );
			}
		}
	}

	return false;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetWeaponPosition()
//
//	PURPOSE:	Is the position of our "Weapon" (could be anything)
//
// ----------------------------------------------------------------------- //

LTVector CAI::GetWeaponPosition(const CWeapon* pWeapon) const
{
	return HandHeldWeaponFirePos(pWeapon);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetAimFromPosition()
//
//	PURPOSE:	Is the position we should use to determine how to aim our weapon (usually the neck node).
//
// ----------------------------------------------------------------------- //

LTVector CAI::GetAimFromPosition() const
{
	return GetPosition() + GetChestOffset();
/*
#ifdef AIMFROM_PROFILE
	LTCounter counter;
	g_pLTServer->StartCounter(&counter);
#endif

	const char szNodeName[] = "Neck_Node";

	HMODELNODE hNode = INVALID_MODEL_NODE;
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	LTransform transform;

	ModelLT* pModelLT = g_pLTServer->GetModelLT();

	if( LT_OK == pModelLT->GetNode(m_hObject, const_cast<char*>(szNodeName), hNode) )
	{
		pModelLT->GetNodeTransform(m_hObject, hNode, transform, DTRUE);
	}
	else if( LT_OK == pModelLT->GetSocket(m_hObject, const_cast<char*>(szNodeName), hSocket) )
	{
		pModelLT->GetSocketTransform(m_hObject, hSocket, transform, DTRUE);
	}
	else
	{
		// Could find aiming node, so just return our position.
		return GetPosition();
	}

	LTVector   vPos;
	LTRotation rRot;
	TransformLT* pTransLT = g_pLTServer->GetTransformLT();
	pTransLT->GetPos(transform, vPos);
	
#ifdef AIMFROM_PROFILE
	const uint32 ticks = g_pLTServer->EndCounter(&counter);
	AICPrint(m_hObject, "AimFromPosition took %d ticks.", ticks );
#endif

	return vPos;
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsObjectVisible*()
//
//	PURPOSE:	Is the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsObjectVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)  const
{
	return IsObjectVisible(ofn, pfn, GetEyePosition(), hObj, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}	

LTBOOL CAI::IsTargetVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, LTFLOAT* pfDistanceSqr /* = LTNULL */, LTFLOAT* pfDp /* = LTNULL */, LTVector* pvDir /* = LTNULL */)  const
{
	if( !m_pTarget->IsValid() || !m_pCurrentWeapon || !GetAttachments() )
		return LTFALSE;

	// Want to use target's actual position, not their last known position.
	LTVector vTargetPosition;
	g_pLTServer->GetObjectPos(m_pTarget->GetObject(), &vTargetPosition);

	return IsObjectPositionVisible( ofn, pfn, GetAimFromPosition(), 
		                            m_pTarget->GetObject(), vTargetPosition + m_pTarget->GetTargetOffset(), 
									LTFLOAT( m_pCurrentWeapon->GetBarrelData()->nRangeSqr), LTFALSE, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/) const
{
	return IsObjectVisible(ofn, pfn, GetWeaponPosition(pWeapon), hObj, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/) const
{
	LTVector vObjectPosition;
	g_pLTServer->GetObjectPos(hObj, &vObjectPosition);

	return IsObjectPositionVisible(ofn, pfn, vPosition, hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsObjectPositionVisible*()
//
//	PURPOSE:	Is a given position on the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsObjectPositionVisibleFromEye( ObjectFilterFn ofn, PolyFilterFn pfn, 
										    HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, 
											LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/, LTVector* pvColor /*= LTNULL*/)  const
{
	return IsObjectPositionVisible(ofn, pfn, GetEyePosition(), hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir, pvColor);
}	

LTBOOL CAI::IsObjectPositionVisibleFromWeapon( ObjectFilterFn ofn, PolyFilterFn pfn, 
											   CWeapon* pWeapon, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, 
											   LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/, LTVector* pvColor /*= LTNULL*/)  const
{
	return IsObjectPositionVisible(ofn, pfn, GetWeaponPosition(pWeapon), hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir, pvColor);
}

LTBOOL CAI::IsObjectPositionVisible( ObjectFilterFn ofn, PolyFilterFn pfn, 
									const LTVector& vSourcePosition, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, 
									LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/, LTVector* pvColor /*= LTNULL*/)  const
{
	if (!g_pLTServer || !m_hObject || !hObj) return LTFALSE;

	LTVector vObjectDims;
	g_pLTServer->GetObjectDims(hObj, &vObjectDims);

	// See if the position is inside the object...

	if ( vSourcePosition.x > vObjectPosition.x - vObjectDims.x && vSourcePosition.x < vObjectPosition.x + vObjectDims.x &&
	     vSourcePosition.y > vObjectPosition.y - vObjectDims.y && vSourcePosition.y < vObjectPosition.y + vObjectDims.y &&
	     vSourcePosition.z > vObjectPosition.z - vObjectDims.z && vSourcePosition.z < vObjectPosition.z + vObjectDims.z)
	{
		// Gotta fudge some of these values

		if ( pfDp )	*pfDp = 1.0;
		if ( pfDistanceSqr ) *pfDistanceSqr = 0.0f;
		if ( pvDir ) *pvDir = GetEyeForward();
		if ( pvColor ) *pvColor = LTVector(255.0,255.0,255.0);
		return LTTRUE;
	}

	const LTVector vDir = vObjectPosition - vSourcePosition;
	const LTFLOAT fDistanceSqr = vDir.MagSqr();

	// Make sure it is close enough
	
	if ( fDistanceSqr >= fVisibleDistanceSqr)
	{
		return LTFALSE;
	}

	// Make sure it is in our FOV

	/*const*/ LTVector vDirNorm = vDir;
	vDirNorm.Norm();

	if ( bFOV )
	{
		// First check horizontal FOV.

		if (vDirNorm.Dot(GetEyeForward()) <= m_fHalfForwardFOV ) 
			return LTFALSE;

		// Now check vertical FOV.
		// Note that this only excludes things within a cone above our heads.

		if (vDirNorm.Dot(GetAimUp()) >= m_fHalfUpNonFOV) 
			return LTFALSE;
	}


	IntersectQuery iQuery;
	IntersectInfo iInfo;

	VEC_COPY(iQuery.m_From, vSourcePosition);
	VEC_COPY(iQuery.m_To, vObjectPosition);
	iQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iQuery.m_FilterFn = ofn;
	iQuery.m_PolyFilterFn = pfn;
	iQuery.m_pUserData = hObj;

	s_hAI = m_hObject;

	const bool bShootThrough = (ofn && ofn == CAI::ShootThroughFilterFn);
	const bool bSeeThrough = (ofn && ofn == CAI::SeeThroughFilterFn);

	LTBOOL bDone = false;
	int nIterations = 0;
	const int c_nMaxIterations = 10;
	while( !bDone && nIterations < c_nMaxIterations)
	{
		g_cIntersectSegmentCalls++;
		++nIterations;
		bDone = true;

		if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )
		{
			// Is it the object that we hit?
			if (iInfo.m_hObject == hObj)
			{
				if ( pfDp ) *pfDp = vDirNorm.Dot(GetEyeForward());
				if ( pfDistanceSqr ) *pfDistanceSqr = fDistanceSqr;
				if ( pvDir ) *pvDir = vDirNorm;
				if ( pvColor ) 
				{
					CCharacter * pChar = dynamic_cast<CCharacter*>( g_pLTServer->HandleToObject(iInfo.m_hObject) );

					if( pChar && pChar->GetModelLighting().x > 0.0f )
					{
						*pvColor = pChar->GetModelLighting();
					}
					else
					{
						*pvColor = ::GetModelLighting(iInfo.m_Point);
					}
				}

				return LTTRUE;
			}

			// Maybe we want to continue.
			if( bShootThrough || bSeeThrough)
			{
				const SurfaceType eSurfType = GetSurfaceType(iInfo); 

				const bool bHitWorld = (LT_YES == g_pLTServer->Physics()->IsWorldObject(iInfo.m_hObject));

				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
				
				if (!bHitWorld && pSurf )
				{
					if(    (bShootThrough && pSurf->bCanShootThrough)
						|| (bSeeThrough && pSurf->bCanSeeThrough) )
					{
						iQuery.m_From = iInfo.m_Point;

						// Keep doing intersects if the impact point is not too near the impact point.
						bDone = iQuery.m_From.Equals(iQuery.m_To,1.0f);
					}
				}
			} //if( bShootThrough || bSeeThrough)

		} // if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )

	} //while( !bDone && nIterations < c_nMaxIterations)

	_ASSERT( nIterations < c_nMaxIterations );
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsPositionVisible*()
//
//	PURPOSE:	Is the test position visible to us
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsPositionVisibleFromEye( ObjectFilterFn ofn, PolyFilterFn pfn, 
									  const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, 
									  LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)  const
{
	return IsPositionVisible(ofn, pfn, GetEyePosition(), vPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}	

LTBOOL CAI::IsPositionVisibleFromWeapon( ObjectFilterFn ofn, PolyFilterFn pfn, 
										 CWeapon* pWeapon, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, 
										 LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)  const
{
	return IsPositionVisible(ofn, pfn, GetWeaponPosition(pWeapon), vPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsPositionVisible( ObjectFilterFn ofn, PolyFilterFn pfn, 
							   const LTVector& vFrom, const LTVector& vTo, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, 
							   LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)  const
{
	if (!g_pLTServer || !m_hObject) return LTFALSE;

	LTVector vDir;
	VEC_SUB(vDir, vTo, vFrom);
	LTVector vDirNorm = vDir;
	vDirNorm.Norm();
	LTFLOAT fDistanceSqr = VEC_MAGSQR(vDir);

	// Make sure it is in our FOV

	if ( bFOV )
	{
		// First check horizontal FOV.

		if (vDirNorm.Dot(GetEyeForward()) <= m_fHalfForwardFOV ) 
			return LTFALSE;

		// Now check vertical FOV.
		// Note that this only excludes things within a cone above our heads.

		if (vDirNorm.Dot(GetAimUp()) >= m_fHalfUpNonFOV) 
			return LTFALSE;
	}

	// Make sure it is close enough
	
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	VEC_COPY(iQuery.m_From, vFrom);
	VEC_COPY(iQuery.m_To, vTo);
	iQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iQuery.m_FilterFn = ofn;
	iQuery.m_PolyFilterFn = pfn;

	s_hAI = m_hObject;

	const bool bShootThrough = (ofn && ofn == CAI::ShootThroughFilterFn);
	const bool bSeeThrough = (ofn && ofn == CAI::SeeThroughFilterFn);

	LTBOOL bReachedDest = LTTRUE;
	LTBOOL bDone = false;
	int nIterations = 0;
	const int c_nMaxIterations = 10;
	while( !bDone && nIterations < c_nMaxIterations)
	{
		g_cIntersectSegmentCalls++;
		++nIterations;
		bDone = true;

		if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )
		{
			bReachedDest = LTFALSE;

			// Maybe we want to continue.
			if( bShootThrough || bSeeThrough)
			{
				const SurfaceType eSurfType = GetSurfaceType(iInfo); 

				const bool bHitWorld = (LT_YES == g_pLTServer->Physics()->IsWorldObject(iInfo.m_hObject));

				SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
				
				if (!bHitWorld && pSurf )
				{
					if(    (bShootThrough && pSurf->bCanShootThrough)
						|| (bSeeThrough && pSurf->bCanSeeThrough) )
					{
						iQuery.m_From = iInfo.m_Point;

						// Keep doing intersects if the impact point is not too near the impact point.
						bDone = iQuery.m_From.Equals(iQuery.m_To,1.0f);

						// Assume the next intersect segment will work.
						bReachedDest = LTFALSE;
					}
				}
			} //if( bShootThrough || bSeeThrough)

		} // if( g_pLTServer->IntersectSegment(&iQuery, &iInfo) )

	} //while( !bDone && nIterations < c_nMaxIterations)

	_ASSERT( nIterations < c_nMaxIterations );

	if ( pfDp ) *pfDp = vDirNorm.Dot(GetEyeForward());
	if ( pfDistanceSqr ) *pfDistanceSqr = fDistanceSqr;
	if ( pvDir ) *pvDir = vDirNorm;

	return bReachedDest;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsVectorOnRightSide()
//
//	PURPOSE:	Does the vector between 'src' and 'dest' pass our right side? 
//              Actually, we look at a RAY from vSrc passing through vDest.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsVectorOnRightSide(const LTVector& vSrc, const LTVector& vDest, LTFLOAT* fVectorDistance /*= LTNULL*/)
{
	LTVector vPn = GetForward();			// Plane normal is ai's forward-vector
	LTVector vRo = vSrc - GetPosition();		// Translate to origin (so D of plane = 0)
	LTVector vRd = (vDest - vSrc);		// Get unit vector with ray's direction
	vRd.Norm();

	// Test to see if ray is orthogonal to our forward vector or if it's 
	// coming from behind us

	LTFLOAT fPnDotRd = vPn.Dot(vRd);

	if ( fPnDotRd == 0.0f )
	{
		// Shot is parallel to the plane (this will occur almost never), so there
		// really isn't an answer to the question -- just randomly return true

		return (LTTRUE);
	}
	else if ( fPnDotRd > 0.0f )
	{
		// Shot is coming from behind us, we should reverse the plane's normal

		vPn = -vPn;
		fPnDotRd = vPn.Dot(vRd);
	}

	// Calculate distance from origin of ray to plane (along the ray)

	LTFLOAT ft = - (vPn.Dot(vRo)/fPnDotRd);

	// Calculate real intersection point given that distance

	LTVector vIntersectPoint = vSrc + vRd*ft;

	// Determine what side of us the intersection point is on

	LTFLOAT fRightVectorProjection = GetRight().Dot(vIntersectPoint - GetPosition());

	// Fill in the intersection point distance if we requested it

	if ( fVectorDistance )
	{
		*fVectorDistance = VEC_DIST(vIntersectPoint, GetPosition());
	}

	return (fRightVectorProjection > 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::OpenDoor()
//
//	PURPOSE:	Attempts to open a door
//
// ----------------------------------------------------------------------- //

void CAI::OpenDoor(HOBJECT hDoor)
{
	SendTriggerMsgToObject(this, hDoor, "ACTIVATE");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CloseDoor()
//
//	PURPOSE:	Attempts to close a door
//
// ----------------------------------------------------------------------- //

void CAI::CloseDoor(HOBJECT hDoor)
{
	SendTriggerMsgToObject(this, hDoor, "ACTIVATE");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //
	
void CAI::HandleModelString(ArgList* pArgList)
{
	if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if( stricmp(pKey, c_szKeyBodySlump) == 0 )
	{
		// Hitting the ground noise

		CollisionInfo Info;
		g_pLTServer->GetStandingOn(m_hObject, &Info);

		SurfaceType eSurface = GetSurfaceType(Info);

		// Play the noise

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
		_ASSERT(pSurf);
		if (pSurf && pSurf->szBodyFallSnd[0])
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyFallSnd, pSurf->fBodyFallSndRadius, SOUNDPRIORITY_MISC_LOW);
		}
	}
	else if( stricmp(pKey, WEAPON_KEY_FIRE) == 0 || stricmp(pKey, "Fire") == 0)
	{
		FireWeapon();
	}

	// Pass it down to the current action.
	if( m_pCurrentAction )
		m_pCurrentAction->ModelStringKeyMsg(pArgList);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAI::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	if (!g_pLTServer || !hWrite) return;

	hWrite->WriteDWord(m_nTemplateId);
	*hWrite << m_cc;

	{for( int i = 0; i < g_nNumAIWeapons; ++i )
	{
		*hWrite << m_strWeaponNames[i];
	}}

	*hWrite << m_fUpdateDelta;

	*hWrite << m_bLocked;
	*hWrite << m_bFirstUpdate;
	*hWrite << m_strQueuedCommands;

	*hWrite << m_bUseCover;
	*hWrite << m_bUseShoulderLamp;
	*hWrite << m_bNeverLoseTarget;

	*hWrite << m_bHasLATarget;
	*hWrite << m_hLATarget;

	hWrite->WriteFloat(m_fLastAimAtTime - g_pLTServer->GetTime());
	*hWrite << m_strDefaultDeathAnim;

	*hWrite << m_bStartActive;
	*hWrite << m_bActive;
	*hWrite << m_dwActiveFlags;
	*hWrite << m_dwActiveFlags2;
	*hWrite << m_dwActiveUserFlags;
	*hWrite << m_bActiveCanDamage;

	*hWrite << m_dwActiveHeadFlags;
	*hWrite << m_dwActiveHeadFlags2;
	*hWrite << m_dwActiveHeadUserFlags;

	*hWrite << m_dwMovementFlags;
	*hWrite << m_bWasStandingOn;
	*hWrite << m_vLastStandingPos;

	*hWrite << m_dwMovementAnimFlags;

	if( m_pStrategyMove )
	{
		hWrite->WriteByte(LTTRUE);
		*hWrite << *m_pStrategyMove; 
	}

	hWrite->WriteDWord( m_AvailableWeapons.size() );
	for( AvailableWeaponList::iterator iter = m_AvailableWeapons.begin();
	     iter != m_AvailableWeapons.end(); ++iter )
	{
		hWrite->WriteDWord( g_pAIWeaponButeMgr->GetIDByName( (*iter)->Name.c_str() ) );
	}

	if( m_pWeaponPosition )
	{
		*hWrite << std::string( m_pWeaponPosition->GetName() );
	}
	else
	{
		// An empty string signifies a null m_pWeaponPosition.
		*hWrite << std::string();
	}
	
	hWrite->WriteDWord(m_nCurrentWeapon);
	// m_pCurrentWeapon will be set off m_nCurrentWeapon in CAI::Load
	// m_pCurrentWeaponButes will be set off m_nCurrentWeapon in CAI::Load
	
	if( m_pIdealWeaponButes )
	{
		AvailableWeaponList::iterator ideal_iter = std::find(m_AvailableWeapons.begin(),m_AvailableWeapons.end(), m_pIdealWeaponButes);

		ASSERT( ideal_iter != m_AvailableWeapons.end() );
		if( ideal_iter != m_AvailableWeapons.end() )
		{
			hWrite->WriteDWord( m_AvailableWeapons.begin() - ideal_iter );
		}
	}
	else
	{
		// An offset >= m_AvailableWeapons.size() indicates a null m_pIdealWeaponButes.
		hWrite->WriteDWord( m_AvailableWeapons.size() );
	}

	*hWrite << m_tmrBurstDelay;
	*hWrite << m_tmrLastBurst;
	*hWrite << m_vAimToPos;
	*hWrite << m_bFullyAimed;

	*hWrite << m_vEyePos;
	*hWrite << m_vEyeForward;
	*hWrite << m_rEyeRotation;
	*hWrite << m_fHalfForwardFOV;
	*hWrite << m_fHalfUpNonFOV;

	*hWrite << m_rTargetRot;
	*hWrite << m_rStartRot;
	*hWrite << m_vTargetRight;
	*hWrite << m_vTargetUp;
	*hWrite << m_vTargetForward;
	*hWrite << m_bRotating;
	*hWrite << m_fRotationTime;
	*hWrite << m_fRotationTimer;
	*hWrite << m_fRotationSpeed;

	m_pTarget->Save(hWrite);
	*hWrite << m_bShootThrough;
	*hWrite << m_bSeeThrough;

	*hWrite << m_fAccuracy;

	*hWrite << m_strInitialNode;

	*hWrite << m_hstrCmdInitial;
	*hWrite << m_hstrCmdActivateOn;
	*hWrite << m_hstrCmdActivateOff;
	*hWrite << m_strUnreachableTargetCmd;

	*hWrite << m_bActivated;

	m_pSenseMgr->Save(hWrite);

	*hWrite << m_tmrInfluence;
	*hWrite << m_bOddsFear;
	*hWrite << m_hFriend;
	*hWrite << m_fFriendDistSqr;
	*hWrite << m_bFriendLocked;
	*hWrite << m_tmrDamageFear;
	*hWrite << m_fFraction;

	*hWrite << m_hCinematicTrigger;

	*hWrite << m_bUsingJumpTo;
	*hWrite << m_bUsingClipTo;

	*hWrite << m_fStartRechargeLevel;
	*hWrite << m_fStopRechargeLevel;
	*hWrite << m_bRechargingExosuit;

	// m_pNextAction done after m_aActions
	// m_pCurrentAction done after m_aActions

	{for(int i = 0; i < NUM_ACTIONS; ++i )
	{
		hWrite->WriteByte( m_aActions[i] != LTNULL );
		if( m_aActions[i] )
		{
			*hWrite << *m_aActions[i];
		}
	}}

	ASSERT( NUM_ACTIONS < 256);
	if( m_pNextAction )
	{
		// If this fails, nNextActionIdx will equal NUM_ACTIONS.
		const uint8 nNextActionIdx = (std::find(m_aActions,m_aActions + NUM_ACTIONS, m_pNextAction) - m_aActions);
		hWrite->WriteByte( nNextActionIdx );
	}
	else
	{
		hWrite->WriteByte(NUM_ACTIONS);
	}

	if( m_pCurrentAction )
	{
		// If this fails, nCurrentActionIdx will equal NUM_ACTIONS.
		const uint8 nCurrentActionIdx = (std::find(m_aActions,m_aActions + NUM_ACTIONS, m_pCurrentAction) - m_aActions);
		hWrite->WriteByte( nCurrentActionIdx );
	}
	else
	{
		hWrite->WriteByte(NUM_ACTIONS);
	}

	hWrite->WriteDWord(m_pLastNode ? m_pLastNode->GetID() : CAINode::kInvalidID );
	hWrite->WriteDWord(m_pNextNode ? m_pNextNode->GetID() : CAINode::kInvalidID );

	*hWrite << m_fLeashLengthSqr;
	*hWrite << m_vLeashFrom;
	*hWrite << m_strLeashFromName;

	m_pBidMgr->Save(hWrite);
	*hWrite << m_pBidMgr->GetSlotName(m_pCurrentBid);

	*hWrite << m_bAlertLock;
	*hWrite << m_bRunLock;
	*hWrite << m_bCrouchLock;
	*hWrite << m_bForceFlashlightOn;

	*hWrite << m_strMusicAmbientTrackSet;
	*hWrite << m_strMusicWarningTrackSet;
	*hWrite << m_strMusicHostileTrackSet;

	// m_dwLastLoadFlags needn't be saved.

	// m_strCurrentInfo needn't be saved.
#ifndef _FINAL
	*hWrite << m_strNameInfo; // This must be saved because it is set only during FirstUpdate.
#else
	// Save a dummy string so that save/load
	// will be compatible between final and release
	// and debug builds.
	*hWrite << std::string("PeterIsGreat");
#endif
	// m_strGoalInfo needn't be saved.
	// m_strActionInfo needn't be saved.
	// m_strTargetInfo needn't be saved.
	// m_strScriptInfo needn't be saved.

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAI::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	if (!g_pLTServer || !hRead) return;

	m_nTemplateId = hRead->ReadDWord();
	*hRead >> m_cc;

	for( int i = 0; i < g_nNumAIWeapons; ++i )
	{
		*hRead >> m_strWeaponNames[i];
	}


	*hRead >> m_fUpdateDelta;

	*hRead >> m_bLocked;
	*hRead >> m_bFirstUpdate;
	*hRead >> m_strQueuedCommands;

	*hRead >> m_bUseCover;
	*hRead >> m_bUseShoulderLamp;
	*hRead >> m_bNeverLoseTarget;

	*hRead >> m_bHasLATarget;
	*hRead >> m_hLATarget;

	m_fLastAimAtTime = hRead->ReadFloat() + g_pLTServer->GetTime();
	*hRead >> m_strDefaultDeathAnim;
	
	LTBOOL bNewActive = LTFALSE;
	*hRead >> m_bStartActive;
	*hRead >> bNewActive;
	*hRead >> m_dwActiveFlags;
	*hRead >> m_dwActiveFlags2;
	*hRead >> m_dwActiveUserFlags;
	*hRead >> m_bActiveCanDamage;

	*hRead >> m_dwActiveHeadFlags;
	*hRead >> m_dwActiveHeadFlags2;
	*hRead >> m_dwActiveHeadUserFlags;

	Active( bNewActive == LTTRUE, false);

	*hRead >> m_dwMovementFlags;
	*hRead >> m_bWasStandingOn;
	*hRead >> m_vLastStandingPos;

	*hRead >> m_dwMovementAnimFlags;

	if( hRead->ReadByte() )
	{
		delete m_pStrategyMove;
		m_pStrategyMove = new CAIStrategyMove(this);
		*hRead >> *m_pStrategyMove;
	}

	m_AvailableWeapons.clear();
	const int num_available_weapons = hRead->ReadDWord();
	{for( int i = 0; i < num_available_weapons; ++i )
	{
		m_AvailableWeapons.push_back( &g_pAIWeaponButeMgr->GetAIWeapon( hRead->ReadDWord() ) );
	}}
	
	std::string weapon_position;
	*hRead >> weapon_position;

	if( !weapon_position.empty() )
	{
		m_pWeaponPosition = m_pAttachments->GetAttachmentPosition(weapon_position.c_str());
	}
	else
	{
		m_pWeaponPosition = LTNULL;
	}

	m_nCurrentWeapon = hRead->ReadDWord();
	ASSERT( m_nCurrentWeapon < int(m_AvailableWeapons.size()) );
	if( m_nCurrentWeapon >= 0 ) 
	{
		m_pCurrentWeapon = CCharacter::GetCurrentWeaponPtr();  // CCharacter goes through the attachments to get the current weapon.
		m_pCurrentWeaponButes = m_AvailableWeapons[m_nCurrentWeapon];
	}
	else
	{
		m_pCurrentWeapon = LTNULL;
		m_pCurrentWeaponButes = LTNULL;
	}

	const uint32 nIdealOffset = hRead->ReadDWord();
	if( nIdealOffset < m_AvailableWeapons.size() )
	{
		m_pIdealWeaponButes = m_AvailableWeapons[nIdealOffset];
	}
	else
	{
		m_pIdealWeaponButes = LTNULL;
	}

	*hRead >> m_tmrBurstDelay;
	*hRead >> m_tmrLastBurst;
	*hRead >> m_vAimToPos;
	*hRead >> m_bFullyAimed;

	*hRead >> m_vEyePos;
	*hRead >> m_vEyeForward;
	*hRead >> m_rEyeRotation;
	*hRead >> m_fHalfForwardFOV;
	*hRead >> m_fHalfUpNonFOV;

	*hRead >> m_rTargetRot;
	*hRead >> m_rStartRot;
	*hRead >> m_vTargetRight;
	*hRead >> m_vTargetUp;
	*hRead >> m_vTargetForward;
	*hRead >> m_bRotating;
	*hRead >> m_fRotationTime;
	*hRead >> m_fRotationTimer;
	*hRead >> m_fRotationSpeed;

	m_pTarget->Load(hRead);
	*hRead >> m_bShootThrough;
	*hRead >> m_bSeeThrough;

	*hRead >> m_fAccuracy;

	*hRead >> m_strInitialNode;

	*hRead >> m_hstrCmdInitial;
	*hRead >> m_hstrCmdActivateOn;
	*hRead >> m_hstrCmdActivateOff;
	*hRead >> m_strUnreachableTargetCmd;

	*hRead >> m_bActivated;

	m_pSenseMgr->Load(hRead);

	*hRead >> m_tmrInfluence;
	*hRead >> m_bOddsFear;
	*hRead >> m_hFriend;
	*hRead >> m_fFriendDistSqr;
	*hRead >> m_bFriendLocked;
	*hRead >> m_tmrDamageFear;
	*hRead >> m_fFraction;

	*hRead >> m_hCinematicTrigger;

	*hRead >> m_bUsingJumpTo;
	*hRead >> m_bUsingClipTo;

	*hRead >> m_fStartRechargeLevel;
	*hRead >> m_fStopRechargeLevel;
	*hRead >> m_bRechargingExosuit;

	// m_pNextAction done after m_aActions
	// m_pCurrentAction done after m_aActions
	{for(int i = 0; i < NUM_ACTIONS; ++i )
	{
		if( hRead->ReadByte() )
		{
			m_aActions[i] = GetAction(ActionType(i));
			*hRead >> *m_aActions[i];
		}
		else
		{
			m_aActions[i] = LTNULL;
		}
	}}

	const uint8 nNextActionIdx = hRead->ReadByte();
	if( nNextActionIdx < NUM_ACTIONS )
	{
		m_pNextAction = m_aActions[nNextActionIdx];
	}
	else
	{
		m_pNextAction = LTNULL;
	}

	const uint8 nCurrentActionIdx = hRead->ReadByte();
	if( nCurrentActionIdx < NUM_ACTIONS )
	{
		m_pCurrentAction = m_aActions[nCurrentActionIdx];
	}
	else
	{
		m_pCurrentAction = LTNULL;
	}

	const uint32 last_node_id = hRead->ReadDWord();
	if( last_node_id != CAINode::kInvalidID )
		m_pLastNode = g_pAINodeMgr->GetNode(last_node_id);

	const uint32 next_node_id = hRead->ReadDWord();
	if( next_node_id != CAINode::kInvalidID )
		m_pNextNode = g_pAINodeMgr->GetNode(next_node_id);

	*hRead >> m_fLeashLengthSqr;
	*hRead >> m_vLeashFrom;
	*hRead >> m_strLeashFromName;

	m_pBidMgr->Load(hRead,this);
	std::string last_bidder_slot_name;
	*hRead >> last_bidder_slot_name;
	m_pCurrentBid = m_pBidMgr->GetBidder(last_bidder_slot_name);

	*hRead >> m_bAlertLock;
	*hRead >> m_bRunLock;
	*hRead >> m_bCrouchLock;
	*hRead >> m_bForceFlashlightOn;

	*hRead >> m_strMusicAmbientTrackSet;
	*hRead >> m_strMusicWarningTrackSet;
	*hRead >> m_strMusicHostileTrackSet;

	// m_dwLastLoadFlags needn't be saved.

#ifndef _FINAL
	*hRead >> m_strNameInfo;
#else
	// Do this so that save/load files are compatible between
	// final and realease and debug builds.
	std::string temp_string;
	*hRead >> temp_string;
#endif

	if( m_bUsingJumpTo )
	{
		m_cmMovement.SetHandleStateFunc(CMS_SPECIAL,			CharacterMovementState_AIJump);
		m_cmMovement.SetMaxSpeedFunc(CMS_SPECIAL,				CharacterMovementState_AIJump_MaxSpeed);
		m_cmMovement.SetMaxAccelFunc(CMS_SPECIAL,				CharacterMovementState_AIJump_MaxAccel);
	}

	if( m_bUsingClipTo )
	{
		m_cmMovement.SetHandleStateFunc(CMS_SPECIAL,			CharacterMovementState_AIClip);
		m_cmMovement.SetMaxSpeedFunc(CMS_SPECIAL,				CharacterMovementState_AIClip_MaxSpeed);
		m_cmMovement.SetMaxAccelFunc(CMS_SPECIAL,				CharacterMovementState_AIClip_MaxAccel);
	}

	if( m_hstrCmdActivateOff || m_hstrCmdActivateOn )
	{
		SetActivateType(m_hObject, AT_NORMAL);
	}
	else
	{
		SetActivateType(m_hObject, AT_NOT_ACTIVATABLE);
	}

	
#ifndef _FINAL
	m_strCurrentInfo.clear();
#endif

	// This needs to be done now that we have loaded m_cc.
	g_pCharacterMgr->Add(this);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*Target
//
//	PURPOSE:	Target methods
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::HasTarget() const
{ 
	return m_pTarget->IsValid(); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetLightLevel
//
//	PURPOSE:	Returns the light level at a given point
//
// ----------------------------------------------------------------------- //

LTFLOAT CAI::GetLightLevel(LTVector vPos) const
{
	const LTVector vColor = ::GetModelLighting(vPos);
	return std::max(vColor.x, std::max(vColor.y, vColor.z) )/255.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Stop
//
//	PURPOSE:	Stops the AI's movement.
//
// ----------------------------------------------------------------------- //
void CAI::Stop()
{
	LTVector vZero(0,0,0);

	m_cmMovement.SetVelocity(vZero);
	m_cmMovement.SetAcceleration(vZero);

	ClearMovementFlags();
	m_cmMovement.SetControlFlags( m_dwMovementFlags );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Forward,Reverse,StrafeLeft,StrafeRight
//
//	PURPOSE:	Goes in a direction without moving past limit.
//				Most of the working code is in DoExactLateralMovement.
//
// ----------------------------------------------------------------------- //

static const char * FindMovementDir(CharacterMovement * pMovement, const LTVector & direction)
{
	if( pMovement->GetForward() == direction )
		return "Forward";
	else if( -pMovement->GetForward() == direction )
		return "Reverse";
	else if( pMovement->GetRight() == direction )
		return "StrafeRight";
	else if( -pMovement->GetRight() == direction )
		return "StrafeLeft";

	return "Unknown direction";
}

static bool DoExactLateralMovement(CharacterMovement * pMovement, const LTVector & limit, const LTVector & direction, LTFLOAT direction_dims, uint32 direction_flags, bool * pbReachedGoal)
{
	const LTVector & my_pos = pMovement->GetPos();
	const LTVector & my_vel = pMovement->GetVelocity();

	const LTFLOAT frame_time = pMovement->GetCharacterVars()->m_fFrameTime;

	const LTVector proj_limit = direction*direction.Dot(limit - my_pos);
	const LTVector needed_vel = proj_limit / frame_time;

	if( fabs(proj_limit.Dot(direction)) < direction_dims )
	{
#ifdef EXACT_MOVEMENT_DEBUG
		AICPrint(pMovement->GetObject(),"%s teleported.",
			FindMovementDir(pMovement,direction) );
#endif
		pMovement->UpdateVelocity(0.0001f,direction_flags);
		pMovement->SetPos( my_pos + proj_limit );

		if( pbReachedGoal ) *pbReachedGoal = true;
		return true;
	}
	else if( needed_vel.MagSqr() < my_vel.MagSqr() )
	{
		pMovement->UpdateVelocity( needed_vel.Mag(), direction_flags );

#ifdef EXACT_MOVEMENT_DEBUG
		AICPrint(pMovement->GetObject(),"%s Capping velocity to %f.", 
			FindMovementDir(pMovement,direction), needed_vel.Mag() );
#endif
		return true;
	}


	return false;
}

bool CAI::Forward( const LTVector & limit )
{
	if( m_bRechargingExosuit )
		return false;

	bool reached_goal = false;
	if( DoExactLateralMovement(GetMovement(),limit,GetForward(), GetDims().z, CM_DIR_FLAG_Z_AXIS, &reached_goal) )
	{
		m_dwMovementAnimFlags = (m_dwMovementAnimFlags | CM_FLAG_FORWARD) & ~CM_FLAG_BACKWARD;
		return reached_goal;
	}

	Forward();
	return reached_goal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Reverse
//
//	PURPOSE:	Goes reverse without shooting past limit.
//
// ----------------------------------------------------------------------- //
bool CAI::Reverse(const LTVector & limit)
{
	if( m_bRechargingExosuit )
		return false;

	bool reached_goal = false;
	if( DoExactLateralMovement(GetMovement(),limit,-GetForward(), GetDims().z, CM_DIR_FLAG_Z_AXIS, &reached_goal) )
	{
		m_dwMovementAnimFlags = (m_dwMovementAnimFlags & ~CM_FLAG_FORWARD) | CM_FLAG_BACKWARD;
		return reached_goal;
	}

	Reverse();
	return reached_goal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::StrafeRight
//
//	PURPOSE:	Strafes right without shooting past limit.
//
// ----------------------------------------------------------------------- //
bool CAI::StrafeRight(const LTVector & limit)
{
	if( m_bRechargingExosuit )
		return false;

	bool reached_goal = false;
	if( DoExactLateralMovement(GetMovement(),limit,GetRight(), GetDims().x, CM_DIR_FLAG_X_AXIS, &reached_goal) )
	{
		m_dwMovementAnimFlags = (m_dwMovementAnimFlags | CM_FLAG_STRAFERIGHT) & ~CM_FLAG_STRAFELEFT;
		return reached_goal;
	}

	StrafeRight();
	return reached_goal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::StrafeLeft
//
//	PURPOSE:	Strafes right without shooting past limit.
//
// ----------------------------------------------------------------------- //
bool CAI::StrafeLeft(const LTVector & limit)
{
	if( m_bRechargingExosuit )
		return false;

	bool reached_goal = false;
	if( DoExactLateralMovement(GetMovement(),limit,-GetRight(), GetDims().x, CM_DIR_FLAG_X_AXIS, &reached_goal) )
	{
		m_dwMovementAnimFlags = (m_dwMovementAnimFlags & ~CM_FLAG_STRAFERIGHT) | CM_FLAG_STRAFELEFT;
		return reached_goal;
	}

	StrafeLeft();
	return reached_goal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Turn
//
//	PURPOSE:	Calls appropriate Turn[Right,Left] routine to face direction.
//
// ----------------------------------------------------------------------- //
bool CAI::Turn( const LTVector & direction )
{
	if( GetRight().Dot(direction) > FLT_EPSILON )
		return TurnRight(direction);
	else if( GetRight().Dot(direction) < -FLT_EPSILON )
		return TurnLeft(direction);
	else if( GetForward().Dot(direction) < 0.0f )
		return TurnRight(direction);  // If the point is directly behind me, turn right.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Turn[Right,Left]
//
//	PURPOSE:	Turns AI right or left until facing along limit vector.
//
// ----------------------------------------------------------------------- //
static bool DoTurn( CAI * pAI, CharacterMovement * pMovement, 
				       bool to_the_right,
				       const LTVector & limit )
{
	const LTFLOAT fTurnRate = MATH_DEGREES_TO_RADIANS(pMovement->GetCharacterVars()->m_fTurnRate);
	const LTFLOAT fMaxTurn = fTurnRate * pMovement->GetCharacterVars()->m_fFrameTime;
	
	LTFLOAT limit_dot_forward = limit.Dot(pAI->GetForward());
	
	// Need to account for floating point error.
	if( limit_dot_forward < -1.0f )
		limit_dot_forward = -1.0f;

	if( limit_dot_forward > 1.0f )
		return true;

	const LTFLOAT requested_angle = (float)acos(limit_dot_forward);

	if( requested_angle > fMaxTurn )
	{
		// We are being asked to turn too far, so just use movement flags.
		if( to_the_right )
			pAI->TurnRight();
		else
			pAI->TurnLeft();

		return false;
	}

	

	LTRotation rRot = pMovement->GetRot();
	LTVector vUp = pMovement->GetUp();

	if( to_the_right )
		g_pMathLT->RotateAroundAxis(rRot, vUp, requested_angle);
	else
		g_pMathLT->RotateAroundAxis(rRot, vUp, -requested_angle);

	pMovement->SetRot(rRot);

	return true;
}


bool CAI::TurnRight( LTVector limit )
{
	if( IsNetted() )
		return false;


#ifdef ZERO_TURN_RADIUS
	SnapTurn(limit);
#endif

	// Remove all up/down aspects from our limit vector.
	limit -= GetUp()*GetUp().Dot(limit);
	limit.Norm();

	// If the point is above or below us, don't bother
	// trying to turn that direction and just say
	// we did.
	if( limit.MagSqr() < 0.5f )
		return true;

	if( DoTurn(this, GetMovement(), true, limit) )
	{
		return true;
	}

	return false;
}
	

bool CAI::TurnLeft( LTVector limit )
{
	if( IsNetted() )
		return false;

#ifdef ZERO_TURN_RADIUS
	SnapTurn(limit);
#endif

	// Remove all up/down aspects from our limit vector.
	limit -= GetUp()*GetUp().Dot(limit);
	limit.Norm();

	// If the point is above or below us, don't bother
	// trying to turn that direction and just say
	// we did.
	if( limit.MagSqr() < 0.5f )
		return true;

	if( DoTurn(this, GetMovement(), false, limit) )
	{
		return true;
	}

	return false;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Turn[Right,Left]
//
//	PURPOSE:	Turns AI right or left until facing along limit vector.
//
// ----------------------------------------------------------------------- //

void CAI::SnapTurn(LTVector facing_direction)
{
	if( IsNetted() || !m_cmMovement.CanRotate() )
		return;

	LTRotation rRot;
	const LTVector & my_up = m_cmMovement.GetCharacterVars()->m_vUp;
	
	facing_direction -= my_up*my_up.Dot(facing_direction);

	if( facing_direction.MagSqr() > 0.001f )
	{
		g_pMathLT->AlignRotation(rRot, facing_direction, const_cast<LTVector&>(my_up) );
		m_cmMovement.SetRot(rRot);
	}

	// Let our aiming have a whack at aiming with a our new rotation.
	UpdateAiming();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Aim
//
//	PURPOSE:	Tries to aim along direction.  Returns true if successful.
//
// ----------------------------------------------------------------------- //

// The constructor of this structure does all the work of aiming at a controlled rate.
// The results are stored in the memeber variables.  
// All the angles are in degrees!!!! It is that way because CharacterMovement works in degrees.
struct AimResult
{

	LTFLOAT m_fAimingAngle;  // What the aiming angle should be set to this frame.

	bool m_bCompletedAim;    // True if the AI has completed aiming along the desired direction.
	bool m_bOutOfRange;      // True if the desired direction is out of the AI's aiming range.

	AimResult( const LTVector & vDesiredDirection, LTFLOAT fCurrentAngle, LTFLOAT fMinIncrementalAim, LTFLOAT fMaxIncrementalAim, 
	  		   LTFLOAT fMinAngle, LTFLOAT fMaxAngle, const LTVector & vPositiveDir, const LTVector & vPlaneNormal );
};

AimResult::AimResult( const LTVector & vDesiredDirection, LTFLOAT fCurrentAngle, LTFLOAT fMinIncrementalAim, LTFLOAT fMaxIncrementalAim,
					  LTFLOAT fMinAngle, LTFLOAT fMaxAngle, const LTVector & vPositiveDir, const LTVector & vPlaneNormal)
	: m_fAimingAngle(0.0f),
	  m_bCompletedAim(true),
	  m_bOutOfRange(false)
{
	// Remove any left/right part from our limit.
	LTVector limit = vDesiredDirection;
	limit -= vPlaneNormal*vPlaneNormal.Dot(limit);
	limit.Norm();

	// Jump out if we can't normalize our limit (it is normal to our aiming plane).
	if( limit.MagSqr() < 0.5f )
	{
		m_fAimingAngle = 0.0f;
		m_bCompletedAim = true;
		return;
	}

	//
	// Set our pitch.
	//
	LTFLOAT limit_dot_pdir = limit.Dot(vPositiveDir);

	// Need to account for floating point error.
	if( limit_dot_pdir > 1.0f )
		limit_dot_pdir = 1.0f;
	else if( limit_dot_pdir < -1.0f )
		limit_dot_pdir = -1.0f;


	// Find the desired pitch, and the angle we need to turn to reach that pitch.
	// Note that a negative pitch is up and a positive pitch is down (becuase
	// we use the right pointing vector in a left-handed system).
	m_fAimingAngle = MATH_RADIANS_TO_DEGREES( (LTFLOAT)asin(limit_dot_pdir) );

	const LTFLOAT requested_delta = m_fAimingAngle - fCurrentAngle;

#ifdef AIM_DEBUG
//	AICPrint(m_hObject,"Aim rate is %f, frame time is %f, max aim is %f, \n"
//		"desired pitch is %f, desired delta is %f, current pitch is %f (%f degrees).",
//		fAimRate, m_cmMovement.GetCharacterVars()->m_fFrameTimeGetFrameTime(), fMaxAim, requested_pitch, requested_delta,
//		m_cmMovement.GetCharacterVars()->m_fAimingPitch, MATH_DEGREES_TO_RADIANS(m_cmMovement.GetCharacterVars()->m_fAimingPitch) );
#endif


	// Be sure we stay within our incremental limits.
	if( (LTFLOAT)fabs(requested_delta) < fMinIncrementalAim )
	{
		// The amount is so small we can ignore it.
		// Just set the aiming angle to be the same as the current angle.
		m_fAimingAngle = fCurrentAngle;
	}
	else if( requested_delta > fMaxIncrementalAim )
	{
		// We are being asked to aim too far this update, 
		// so set the aiming angle to the max we can do.
		m_fAimingAngle = fCurrentAngle + fMaxIncrementalAim;
		m_bCompletedAim = false;
	}
	else if( requested_delta < -fMaxIncrementalAim )
	{
		// We are being asked to aim too far this update, 
		// so set the aiming angle to the max we can do.
		m_fAimingAngle = fCurrentAngle - fMaxIncrementalAim;
		m_bCompletedAim = false;
	}

	// Check that desired angle can be reached.
	if( m_fAimingAngle < fMinAngle )
	{
		m_fAimingAngle = fMinAngle;
		m_bCompletedAim = false;
		m_bOutOfRange = true;
	}
	else if( m_fAimingAngle > fMaxAngle )
	{
		m_fAimingAngle = fMaxAngle;
		m_bCompletedAim = false;
		m_bOutOfRange = true;
	}


#ifdef AIM_DEBUG
//	AICPrint(m_hObject,"Aiming pitch is being set to %f radians or %f degrees. m_bCompletedAim is %s. m_bOutOfRange is %s.",
//		result.m_fAimingAngle, MATH_RADIANS_TO_DEGREES(result.m_fAimingAngle), 
//		result.m_bCompletedAim ? "true" : "false",
//		result.m_bOutOfRange   ? "true" : "false"  );
#endif

}

void CAI::ClearAim()
{
	m_fLastAimAtTime = -1.0f;
	m_vAimToPos = LTVector(0,0,0);
	m_bFullyAimed = LTFALSE;

	m_cmMovement.GetCharacterVars()->m_fAimingPitch = 0.0f;
	m_cmMovement.GetCharacterVars()->m_fAimingYaw = 0.0f;
}

void CAI::AimAt(const LTVector & vAimToPos)
{
	// This is being used for the shoulder lamps.
	m_fLastAimAtTime = g_pLTServer->GetTime();

	// Record the position (is used for weapon fire).
	m_vAimToPos = vAimToPos;
	m_bFullyAimed = LTFALSE;
}

void CAI::UpdateAiming()
{

	// Don't do any model twisting if we don't have a 
	// weapon or if the weapon is a melee weapon.
	if( !m_pCurrentWeaponButes || m_pCurrentWeaponButes->bMeleeWeapon )
	{
		m_cmMovement.GetCharacterVars()->m_fAimingPitch = 0.0f;
		m_cmMovement.GetCharacterVars()->m_fAimingYaw = 0.0f;
		m_bFullyAimed = LTTRUE;
		return;
	}

	// If we aren't really aiming at anything just slowly return us to neutral.
	if( m_fLastAimAtTime < 0.0f )
	{
		// Are we already at neutral?
		if(    m_cmMovement.GetCharacterVars()->m_fAimingPitch == 0.0f
			&& m_cmMovement.GetCharacterVars()->m_fAimingYaw == 0.0f    )
		{
			return;
		}

		// Determine our aiming rate.
		const CharacterVars * pVars = m_cmMovement.GetCharacterVars();
		const CharacterButes * pButes = m_cmMovement.GetCharacterButes();

		// AI's currently snap to their aiming point. Uncomment this and use as fMaxIncrementalChange
		// to let them do a more smooth aiming.
		const LTFLOAT fAimRate = (pVars->m_dwCtrl & CM_FLAG_RUN) ? pButes->m_fRunningAimRate : pButes->m_fWalkingAimRate;
		const LTFLOAT fMaxIncrementalAim = fAimRate * GetVars()->m_fFrameTime;

		// Lower our pitch to zero, making sure not to move faster than our maximum allowed amount.
		if( (float)fabs(m_cmMovement.GetCharacterVars()->m_fAimingPitch) > fMaxIncrementalAim )
		{
			m_cmMovement.GetCharacterVars()->m_fAimingPitch -= fMaxIncrementalAim;
		}
		else
		{
			m_cmMovement.GetCharacterVars()->m_fAimingPitch = 0.0f;
		}

		// Lower our yaw to zero, making sure not to move faster than our maximum allowed amount.
		if( (float)fabs(m_cmMovement.GetCharacterVars()->m_fAimingYaw) > fMaxIncrementalAim )
		{
			m_cmMovement.GetCharacterVars()->m_fAimingYaw -= fMaxIncrementalAim;
		}
		else
		{
			m_cmMovement.GetCharacterVars()->m_fAimingYaw = 0.0f;
		}

		return;
	}

	// Set the AI's aiming variables.

	const LTVector vAimDirection =  m_vAimToPos - GetAimFromPosition();

	// Determine our aiming rate.
	const CharacterVars * pVars = m_cmMovement.GetCharacterVars();
	const CharacterButes * pButes = m_cmMovement.GetCharacterButes();

	const LTFLOAT fAimRate = (pVars->m_dwCtrl & CM_FLAG_RUN) ? pButes->m_fRunningAimRate : pButes->m_fWalkingAimRate;
	const LTFLOAT fMinIncrementalAim = 1.0f;
	const LTFLOAT fMaxIncrementalAim = fAimRate * GetVars()->m_fFrameTime;


	// Set our pitch (up/down).
	const LTFLOAT current_pitch = m_cmMovement.GetCharacterVars()->m_fAimingPitch;

	const AimResult pitch_result( vAimDirection, current_pitch, fMinIncrementalAim, fMaxIncrementalAim, 
								   -90.0f, 90.0f,
								  -GetUp(), GetRight() );

	m_cmMovement.GetCharacterVars()->m_fAimingPitch = pitch_result.m_fAimingAngle;


	// Set our yaw (left/right).
	const LTFLOAT current_yaw = m_cmMovement.GetCharacterVars()->m_fAimingYaw;

	const AimResult yaw_result( vAimDirection, current_yaw, fMinIncrementalAim, fMaxIncrementalAim,
		                          -45.0f, 45.0f,
								  GetRight(), -GetUp() );

	m_cmMovement.GetCharacterVars()->m_fAimingYaw = yaw_result.m_fAimingAngle;

	m_bFullyAimed = pitch_result.m_bCompletedAim && yaw_result.m_bCompletedAim;

//	AICPrint(m_hObject, "Aiming pitch %f, yaw %f",
//		pitch_result.m_fAimingAngle, yaw_result.m_fAimingAngle );

	// Return whether we successfully aimed along our desired direction.
	// Not used anymore.  PLH  3-20-01
//	return pitch_result.m_bCompletedAim && yaw_result.m_bCompletedAim;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetCurrentWeapon
//
//	PURPOSE:	Sets the AI's weapon.
//
// ----------------------------------------------------------------------- //
void CAI::SetMovementStyle(const char * szStyle /* = std::string("Standard") */)
{
	const char * szOldStyle = GetAnimation()->GetLayerReference(ANIMSTYLELAYER_STYLE);
	if( szStyle && szOldStyle && 0 == stricmp(szStyle,szOldStyle) )
		return;

	// Set the animation layer reference.
	if( !GetAnimation()->SetLayerReference(ANIMSTYLELAYER_STYLE, szStyle) )
	{
		// If we're here, the new style failed.  So set it to standard.
		GetAnimation()->SetLayerReference(ANIMSTYLELAYER_STYLE, "Standard");
	}


	return;

/*
**
**  This is how the movement speeds were set-up.  It won't work anymore because
**  AIButeMgr i
/*	const CharacterButes & char_butes = *GetMovement()->GetCharacterButes();
	CharacterVars & char_vars = *GetMovement()->GetCharacterVars();
	const std::string tag_name = g_pAIButeMgr->GetNameByID(m_nTemplateId);

	char buf1[CHARACTER_BUTES_STRING_SIZE];


	// Update the movement speeds.
	sprintf(buf1,"%sWalkSpeed",style_name.c_str());
	LTFLOAT new_walk_speed = g_pAIButeMgr->GetDouble(tag_name.c_str(),buf1,char_butes.m_fBaseWalkSpeed);
	char_vars.m_fWalkSpeedMod = char_butes.m_fBaseWalkSpeed > 0.0f ? new_walk_speed / char_butes.m_fBaseWalkSpeed : 0.0f;

	sprintf(buf1,"%sRunSpeed",style_name.c_str());
	LTFLOAT new_run_speed = g_pAIButeMgr->GetDouble(tag_name.c_str(),buf1,char_butes.m_fBaseRunSpeed);
	char_vars.m_fRunSpeedMod = char_butes.m_fBaseRunSpeed > 0.0f ? new_run_speed / char_butes.m_fBaseRunSpeed : 0.0f;

	sprintf(buf1,"%sCrouchSpeed",style_name.c_str());
	LTFLOAT new_crouch_speed = g_pAIButeMgr->GetDouble(tag_name.c_str(),buf1,char_butes.m_fBaseCrouchSpeed);
	char_vars.m_fCrouchSpeedMod = char_butes.m_fBaseCrouchSpeed > 0.0f ? new_crouch_speed / char_butes.m_fBaseCrouchSpeed : 0.0f;
*/

}


void CAI::HandleStunEffect()
{
	if (!IsStunned())
	{
		m_pNextAction = LTNULL;
		if( m_pCurrentAction )
			m_pCurrentAction->End();
		m_pCurrentAction = LTNULL;
		
		SetNextAction(CAI::ACTION_STUN);
	}
}

void CAI::UpdateStunEffect() 
{
	// Movement flags are handled in AIActionStun
	return;
}

void CAI::HandleEMPEffect()
{
	uint32 nEMPFlags = g_pCharacterButeMgr->GetEMPFlags(GetCharacter());

	if(!IsStunned() && nEMPFlags & EMP_FLAG_STUN)
	{
		m_pNextAction = LTNULL;
		if( m_pCurrentAction )
			m_pCurrentAction->End();
		m_pCurrentAction = LTNULL;
		
		SetNextAction(CAI::ACTION_STUN);

		// No more wall walking!
		GetMovement()->SetControlFlags(m_cmMovement.GetControlFlags() & ~CM_FLAG_WALLWALK);

		// De-cloak the predator.
		if( HasCloakDevice() )
		{
			ForceCloakOff();
		}
	}
}

void CAI::UpdateEMPEffect() 
{
	// Movement flags are handled in AIActionStun
	return;
}
		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FireWeapon
//
//	PURPOSE:	Fires the AI's current weapon.
//
// ----------------------------------------------------------------------- //

bool CAI::FireWeapon()
{
	bool bFiredWeapon = false;

//	const uint32 nStartISegCalls = g_cIntersectSegmentCalls;

	// "Fire" is to support legacy stuff.  Everything should be using WEAPON_KEY_FIRE.
	_ASSERT( m_pCurrentWeapon && m_pCurrentWeaponButes);

	if( !m_pCurrentWeapon || !m_pCurrentWeaponButes || !GetAttachments() )
		return bFiredWeapon;

	if( !m_pCurrentWeaponButes->strPiece.empty() )
	{
		HMODELPIECE hPiece = INVALID_MODEL_PIECE;
		if( LT_OK == g_pModelLT->GetPiece(m_hObject, const_cast<char*>(m_pCurrentWeaponButes->strPiece.c_str()), hPiece))
		{
			// Get the hide status... if it's invisible, we have no ammo!
			LTBOOL bHidden = LTFALSE;
			if( LT_OK == g_pModelLT->GetPieceHideStatus(m_hObject, hPiece, bHidden) )
			{
				if(bHidden) 
					return false;
			}
		}
	}

	// Find the muzzle position.
	LTVector vFirePos = GetPosition();
	LTVector vPath(0,0,0);

	// Use the AI's center point if its a melee weapon.
	if( !m_pCurrentWeaponButes->bMeleeWeapon )
	{
		vFirePos = GetWeaponPosition(m_pCurrentWeapon);
		
		// Fine tune our path if we are fully aimed!
		if( m_bFullyAimed )
		{
			vPath = m_vAimToPos - vFirePos;

			// Adjust the path for more accurate vector weapons.
			const AMMO * pAmmoData = m_pCurrentWeapon->GetAmmoData();
			if(     pAmmoData 
				&& (pAmmoData->eType == PROJECTILE) 
				&& pAmmoData->pProjectileFX
				&& (pAmmoData->pProjectileFX->dwObjectFlags & FLAG_GRAVITY) )
			{
				vPath = m_vAimToPos - GetAimFromPosition();
			}
		}
		else
		{
			vPath  = GetAimForward();
		}
	}
	else
	{
		vPath = m_pTarget->GetPosition() - vFirePos;
	}

		
	// Send the fire message.
	WFireInfo fireInfo;
	fireInfo.hFiredFrom = m_hObject;
	fireInfo.hTestObj	= m_pTarget->GetObject();
	fireInfo.vPath		= vPath;
	fireInfo.vFirePos	= vFirePos;
	fireInfo.vFlashPos	= vFirePos;
	fireInfo.nFlashSocket = m_pCurrentWeapon->GetCurrentFlashSocket();
	fireInfo.nSeed = GetRandom(2,255);
	fireInfo.bAltFire = m_pCurrentWeaponButes->bUseAltFX;
	fireInfo.fPerturbR	= (1.0f - GetAccuracy());
	fireInfo.fPerturbU	= (1.0f - GetAccuracy());
	fireInfo.fDamageMultiplier = m_pCurrentWeaponButes->fDamageMultiplier;
	fireInfo.fDamageMultiplier *= GetDifficultyFactor();

// REMOVED: 08/28/01 - MarkS
// WillHurtFriend() is now checked before each SetNextAction(CAI::ACTION_FIREWEAPON) call.

//	if (!WillHurtFriend(*this,*m_pCurrentWeapon,fireInfo))
	{ 
		// And now we fire our weapon!!!!!!!!!!!!!!!!
		m_pCurrentWeapon->Fire(fireInfo);
		
		bFiredWeapon = true;

#ifndef _FINAL
		if (ShouldShowFire(m_hObject))
		{
			LineSystem::GetSystem(this,"WeaponTrajectory")
				<< LineSystem::Clear()
				<< LineSystem::Line(fireInfo.vFirePos, fireInfo.vFirePos + fireInfo.vPath*LTFLOAT(m_pCurrentWeapon->GetBarrelData()->nRange), m_bFullyAimed ? Color::Red : Color::Yellow )
				<< LineSystem::Line(GetAimFromPosition(), m_vAimToPos, Color::Blue );
			
			AICPrint(this,"Firing weapon \"%s\".",
				m_pCurrentWeaponButes->Name.c_str());
		}
#endif
	}

	// Reset the burst delay.
	m_tmrBurstDelay.Start( GetRandom(m_pCurrentWeaponButes->fMinBurstDelay,m_pCurrentWeaponButes->fMaxBurstDelay) );
	m_tmrLastBurst.Start();

//	AICPrint(m_hObject, "Intersect Segment called %d.", g_cIntersectSegmentCalls - nStartISegCalls);

	return bFiredWeapon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetWeaponRotation
//
//	PURPOSE:	Gets the orientation of a weapon attachment
//
// ----------------------------------------------------------------------- //

void CAI::GetWeaponRotation(LTRotation* rRot) const
{
	if ( m_pWeaponPosition )
	{
		HMODELSOCKET hWeaponSocket;
		LTransform transform;
		g_pModelLT->GetSocket(m_hObject, const_cast<char *>(m_pWeaponPosition->GetName().c_str()), hWeaponSocket);
		g_pModelLT->GetSocketTransform(m_hObject, hWeaponSocket, transform, LTTRUE);
		g_pTransLT->GetRot(transform, *rRot);
	}
	else
	{
		*rRot = GetAimRotation();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetCurrentWeapon
//
//	PURPOSE:	Sets the AI's weapon.
//
// ----------------------------------------------------------------------- //


namespace
{
	struct MatchAIWeaponNamed
	{
		const char * const szNameToMatch;

		MatchAIWeaponNamed(const char * name_to_match)
			: szNameToMatch(name_to_match) {}

		bool operator()(const AIWBM_AIWeapon * pTestee) const
		{
			return 0 == stricmp(pTestee->Name.c_str(),szNameToMatch);
		}
	};
}

void CAI::SetWeapon(const char * szAIWeaponName, bool bAddIfNotAvailable /*= false */)
{
	AvailableWeaponList::iterator found_weapon = 
		std::find_if( m_AvailableWeapons.begin(), m_AvailableWeapons.end(), MatchAIWeaponNamed(szAIWeaponName) );

	if( found_weapon != m_AvailableWeapons.end() )
	{
		SetWeapon(found_weapon - m_AvailableWeapons.begin());
	}
	else if( bAddIfNotAvailable )
	{
		AddWeapon(szAIWeaponName);
		SetWeapon(m_AvailableWeapons.size()-1);
	}
}

void CAI::AddWeapon(const char * szAIWeaponName, int nAmmoAmount /* = -1 */)
{
	// This must be called after the aggregrate is set up, which is after the object's initial update.

	bool first_weapon = false;
	if( m_AvailableWeapons.empty() )
	{
		first_weapon = true;
	}

	if( first_weapon )
	{
		const AIWBM_AIWeapon & weapon_bute = g_pAIWeaponButeMgr->GetAIWeapon(szAIWeaponName);

		if( weapon_bute.WeaponName.empty() && weapon_bute.BarrelName.empty() )
		{
#ifndef _FINAL
			AIErrorPrint(m_hObject,"There is no AIWeapon named \"%s\"! See AIButes.txt.", szAIWeaponName );
#endif
			return;
		}

		if( !CanUseWeapon(weapon_bute) )
		{
#ifndef _FINAL
			AIErrorPrint(m_hObject, "Was given weapon \"%s\" which is not useable by this race.",
				weapon_bute.Name.c_str() );
#endif
			return;
		}

		// Attach the first weapon.
		// (This is really backwards,  any major bugs might best be resolved be writing a new weapon management system).
		const std::string weapon_position = g_pAIButeMgr->GetTemplate(m_nTemplateId).strWeaponPosition;

		if( !weapon_position.empty() && !m_pWeaponPosition)
		{
			// Use the weapon name, if we have one. 
			// The attachments need a valid weapon name.  If we add a special AI only barrel and
			// tie it to a weapon through AIButes, we need to make sure the attachments know which
			// weapon it is.
			if( !weapon_bute.WeaponName.empty() )
			{
				m_pAttachments->Attach( weapon_position.c_str(), weapon_bute.WeaponName.c_str() );
			}
			else
			{
				// Otherwise, the attachment code can work with a barrel so long as the barrel is 
				// associated with a weapon in Weaopns.txt (either named as default barrel or part of a sequence
				// of barrels which starts as a default barrel).

				m_pAttachments->Attach( weapon_position.c_str(), weapon_bute.BarrelName.c_str() );
			}

			m_pWeaponPosition = m_pAttachments->GetAttachmentPosition(weapon_position.c_str());
		}
	}

	if( m_pWeaponPosition )
	{
		CAttachmentWeapon * const pWeaponAttachment = dynamic_cast<CAttachmentWeapon*>( m_pWeaponPosition->GetAttachment() );
		_ASSERT( pWeaponAttachment );
		if( !pWeaponAttachment ) return;

		CWeapons * const pWeapons = pWeaponAttachment->GetWeapons();

		const AIWBM_AIWeapon * pWeaponBute = &g_pAIWeaponButeMgr->GetAIWeapon(szAIWeaponName);
		while( pWeaponBute )
		{
			if( !CanUseWeapon(*pWeaponBute) )
			{
#ifndef _FINAL
				AIErrorPrint(m_hObject, "Was given weapon \"%s\" which is not useable by this race.",
					pWeaponBute->Name.c_str() );
#endif
				pWeaponBute = LTNULL;
			}
			else if( m_AvailableWeapons.end() == std::find(m_AvailableWeapons.begin(), m_AvailableWeapons.end(), pWeaponBute) )
			{
				// Get the weapon from the weapon butes.
				if( !pWeaponBute->Name.empty() )
				{
					if( !pWeaponBute->BarrelName.empty() )
					{
						m_AvailableWeapons.push_back( pWeaponBute );
					}
					else
					{
#ifndef _FINAL
						AIErrorPrint(m_hObject,"AIWeapon named \"%s\" has no barrel! See AIButes.txt.",
							pWeaponBute->Name.c_str() );
#endif
						pWeaponBute = LTNULL;
					}
#ifdef SWITCH_WEAPON_DEBUG
					AICPrint(m_hObject,"Added weapon %s.", pWeaponBute->Name.c_str() );
#endif
				}
				else
				{
					pWeaponBute = LTNULL;
				}

				if( pWeaponBute )
				{
					// Obtain the weapon.
					uint8 nWeaponId = WMGR_INVALID_ID;
					uint8 nBarrelId = WMGR_INVALID_ID;
					uint8 nAmmoId = WMGR_INVALID_ID;
					uint8 nAmmoPoolId = WMGR_INVALID_ID;

					g_pWeaponMgr->ReadBarrel(pWeaponBute->BarrelName.c_str(), nBarrelId, nAmmoId, nAmmoPoolId);
					if( !pWeaponBute->WeaponName.empty() )
					{
						WEAPON * pWeaponData = g_pWeaponMgr->GetWeapon(pWeaponBute->WeaponName.c_str());
						if( pWeaponData )
						{
							nWeaponId = pWeaponData->nId;
						}
					}

					if( nWeaponId == WMGR_INVALID_ID && nBarrelId != WMGR_INVALID_ID )
					{
						nWeaponId = g_pWeaponMgr->FindWeaponWithBarrel(nBarrelId);
					}

					int nAmmo = -1;
					if( pWeaponBute->nMaxStartingAmmo > 0 )
						nAmmo = GetRandom( pWeaponBute->nMinStartingAmmo, pWeaponBute->nMaxStartingAmmo);

					pWeapons->ObtainWeapon( nWeaponId, nBarrelId, nAmmoPoolId );
					pWeapons->SetAmmo(nAmmoPoolId, nAmmo );
					pWeapons->SetInfinitePool(nAmmoPoolId, pWeaponBute->bInfiniteAmmo );

					// Go on to an alternate weapon (if it exists).
					if( !pWeaponBute->AltWeaponName.empty() && pWeaponBute->fAltWeaponChance > 0.0f )
					{
						pWeaponBute = &g_pAIWeaponButeMgr->GetAIWeapon(szAIWeaponName);
					}
					else
					{
						pWeaponBute = LTNULL;
					}
				}
			}
			else
			{
				pWeaponBute = LTNULL;
			}

		} //while( pWeaponBute )
	
		if( first_weapon && !m_AvailableWeapons.empty() )
		{
			SetWeapon(0);
			m_pIdealWeaponButes = m_pCurrentWeaponButes;
		}
	}
}

void CAI::ClearWeapons( LTBOOL bDropWeapon /* = LTFALSE */ )
{
	SetWeapon( -1 );

	m_AvailableWeapons.clear();
	m_pIdealWeaponButes = LTNULL;

	if( m_pWeaponPosition )
	{
		if( bDropWeapon )
		{
			m_pAttachments->DropAttachment(m_pWeaponPosition);
		}
		else
		{
			m_pAttachments->Detach( m_pWeaponPosition );
		}

		m_pWeaponPosition = LTNULL;
	}
}


bool CAI::CanUseWeapon(const AIWBM_AIWeapon & weapon_bute)
{
	switch( weapon_bute.eRace )
	{
		case AIWBM_AIWeapon::Alien :
		{
			return (GetCharacterClass() == ALIEN);
		}
		break;

		case AIWBM_AIWeapon::Predator :
		{
			return (GetCharacterClass() == PREDATOR);
		}
		break;

		case AIWBM_AIWeapon::Human :
		{
			return (   GetCharacterClass() == MARINE 
				    || GetCharacterClass() == CORPORATE
					|| GetCharacterClass() == CORPORATE
					|| GetCharacterClass() == SYNTHETIC );
		}
		break;

		case AIWBM_AIWeapon::Exosuit :
		{
			return (   GetCharacterClass() == MARINE_EXOSUIT
					|| GetCharacterClass() == CORPORATE_EXOSUIT );
		}
		break;
	};

	return false;
}

static bool HasAmmo(const AIWBM_AIWeapon & weapon_bute, /*const*/ CAttachmentPosition & attachment_position, HOBJECT hCharacter, LTBOOL bInjuredMovement)
{
	_ASSERT( g_pWeaponMgr);
	if( !g_pWeaponMgr ) return false;

	if( bInjuredMovement && !weapon_bute.bCanUseInjured )
		return false;

	if( !weapon_bute.strPiece.empty() )
	{
		HMODELPIECE hPiece = INVALID_MODEL_PIECE;
		if( LT_OK == g_pModelLT->GetPiece(hCharacter, const_cast<char*>(weapon_bute.strPiece.c_str()), hPiece))
		{
			// Get the hide status... if it's invisible, we have no ammo!
			LTBOOL bHidden = LTFALSE;
			if( LT_OK == g_pModelLT->GetPieceHideStatus(hCharacter, hPiece, bHidden) )
			{
				if(bHidden) 
					return false;
			}
		}
	}

	CAttachmentWeapon * const pWeaponAttachment = dynamic_cast<CAttachmentWeapon*>( attachment_position.GetAttachment() );
	_ASSERT( pWeaponAttachment );
	if( !pWeaponAttachment ) return false;

	CWeapons * const pWeapons = pWeaponAttachment->GetWeapons();

	uint8 nWeaponId = WMGR_INVALID_ID;
	uint8 nBarrelId = WMGR_INVALID_ID;
	uint8 nAmmoId = WMGR_INVALID_ID;
	uint8 nAmmoPoolId = WMGR_INVALID_ID;

	g_pWeaponMgr->ReadBarrel(weapon_bute.BarrelName.c_str(), nBarrelId, nAmmoId, nAmmoPoolId);

	return (LTTRUE == pWeapons->IsPoolInfinite(nAmmoPoolId)) || (pWeapons->GetAmmoPoolCount(nAmmoPoolId) > 0);
}

bool CAI::OutOfAmmo() const
{
	if( m_pWeaponPosition && m_pCurrentWeaponButes )
		return !HasAmmo(*m_pCurrentWeaponButes, *m_pWeaponPosition, m_hObject, GetInjuredMovement() );

	return true;
}

bool CAI::EmptyClip() const
{
	if( m_pCurrentWeapon && GetAttachments() )
		return ( m_pCurrentWeapon->GetAmmoInClip() <= 0.0f );

	return true;
}

void CAI::ChooseBestWeapon(LTFLOAT target_range_sqr)
{
	if( !m_pWeaponPosition) return;

	// Check to see if we need a new weapon.
	if(    m_pCurrentWeaponButes 
	    && target_range_sqr < m_pCurrentWeaponButes->fMaxIdealRange*m_pCurrentWeaponButes->fMaxIdealRange
		&& target_range_sqr > m_pCurrentWeaponButes->fMinIdealRange*m_pCurrentWeaponButes->fMinIdealRange 
		&& HasAmmo(*m_pCurrentWeaponButes, *m_pWeaponPosition, m_hObject, GetInjuredMovement() ) )
	{
		return;
	}

	// Change our ideal weapon if it is out of ammo.
	if( m_pIdealWeaponButes && !HasAmmo(*m_pIdealWeaponButes, *m_pWeaponPosition, m_hObject, GetInjuredMovement() ) )
	{
		m_pIdealWeaponButes = LTNULL;
		
		// Drop the weapon.
		// This doesn't work so well, the AI just picks the weapon up again!
//		SetWeapon(0);
//		m_pAttachments->DropAttachment(m_pWeaponPosition,LTFALSE);

		// Un-select the weapon we just dropped (kinda crazy, huh?).
		SetWeapon(-1);

		// Remove that weapon.  The ideal weapon should _always_ be the
		// first weapon in available weapons.
		// Make sure there are no pointers into the container at this point!
		m_AvailableWeapons.pop_front();

		// Try to find another ideal weapon.
		for( AvailableWeaponList::iterator iter = BeginAvailableWeapons(); 
			 iter != EndAvailableWeapons(); ++iter )
		{
			if( HasAmmo(**iter, *m_pWeaponPosition, m_hObject, GetInjuredMovement() ) )
			{
				m_pIdealWeaponButes = *iter;
				break;
			}
		}
	}

	const AIWBM_AIWeapon * best_weapon = LTNULL;
	int best_weapon_offset = -1;

	// Find first weapon for which target is within ideal range.
	{
	for( AvailableWeaponList::iterator iter = BeginAvailableWeapons(); 
		 iter != EndAvailableWeapons() && !best_weapon; ++iter )
	{
		const AIWBM_AIWeapon * const current_weapon = *iter;
		if(    target_range_sqr < current_weapon->fMaxIdealRange*current_weapon->fMaxIdealRange
			&& target_range_sqr > current_weapon->fMinIdealRange*current_weapon->fMinIdealRange )
		{
			if( HasAmmo(*current_weapon,*m_pWeaponPosition, m_hObject, GetInjuredMovement() ) )
			{
				best_weapon = *iter;
				best_weapon_offset = iter - BeginAvailableWeapons();
			}
		}
	}
	}

	// If that didn't work, find first weapon for which target is within firing range.
	{
	for( AvailableWeaponList::iterator iter = BeginAvailableWeapons(); 
		 iter != EndAvailableWeapons() && !best_weapon; ++iter )
	{
		const AIWBM_AIWeapon * const current_weapon = *iter;
		if(    target_range_sqr < current_weapon->fMaxFireRange*current_weapon->fMaxFireRange
			&& target_range_sqr > current_weapon->fMinFireRange*current_weapon->fMinFireRange )
		{
			if( HasAmmo(*current_weapon,*m_pWeaponPosition, m_hObject, GetInjuredMovement() ) )
			{
				best_weapon = *iter;
				best_weapon_offset = iter - BeginAvailableWeapons();
			}
		}
	}
	}

	// And if that didn't work, find the one whose limit is nearest the target's range!
	{
	LTFLOAT nearest_range_sqr = FLT_MAX;

	for( AvailableWeaponList::iterator iter = BeginAvailableWeapons(); 
		 iter != EndAvailableWeapons() && !best_weapon; ++iter )
	{
		const AIWBM_AIWeapon * const current_weapon = *iter;

		const LTFLOAT max_difference = (float)fabs(target_range_sqr - current_weapon->fMaxFireRange*current_weapon->fMaxFireRange);
		const LTFLOAT min_difference = (float)fabs(target_range_sqr - current_weapon->fMinFireRange*current_weapon->fMinFireRange);
		if(  min_difference < nearest_range_sqr || max_difference < nearest_range_sqr )
		{
			if( HasAmmo(*current_weapon,*m_pWeaponPosition, m_hObject, GetInjuredMovement() ) )
			{
				nearest_range_sqr = std::min(min_difference,max_difference);
				best_weapon = *iter;
				best_weapon_offset = iter - BeginAvailableWeapons();
			}
		}
	}
	}

	if( best_weapon && best_weapon != m_pCurrentWeaponButes )
	{
		SetWeapon(best_weapon_offset);
	}
}


void CAI::SetWeapon(int nWeaponOffset)
{
	if( !GetAttachments() )
	{
		return;
	}

	if( m_pCurrentWeapon && m_nCurrentWeapon == nWeaponOffset )
	{
		return;
	}

	m_pCurrentWeapon = LTNULL;
	m_nCurrentWeapon = -1;
	m_pCurrentWeaponButes = LTNULL;

	if( !m_pWeaponPosition )
		return;


	CAttachmentWeapon * const pWeaponAttachment = dynamic_cast<CAttachmentWeapon*>( m_pWeaponPosition->GetAttachment() );
	_ASSERT( pWeaponAttachment );
	if( !pWeaponAttachment ) 
		return;

	CHHWeaponModel * const pHHWeaponModel = dynamic_cast<CHHWeaponModel*>( g_pLTServer->HandleToObject(pWeaponAttachment->GetModel()) );
	_ASSERT( pHHWeaponModel );
	if( !pHHWeaponModel ) 
		return;


	CWeapons * const pWeapons = pWeaponAttachment->GetWeapons();
	pWeapons->DeselectCurWeapon();

	if( nWeaponOffset < 0 || (uint32)nWeaponOffset >= m_AvailableWeapons.size() )
	{
		return;
	}


	m_nCurrentWeapon = nWeaponOffset;
	m_pCurrentWeaponButes = m_AvailableWeapons[m_nCurrentWeapon];

	_ASSERT( m_pCurrentWeaponButes );

	uint8 nWeaponId = WMGR_INVALID_ID;
	uint8 nBarrelId = WMGR_INVALID_ID;
	uint8 nAmmoId = WMGR_INVALID_ID;
	uint8 nAmmoPoolId = WMGR_INVALID_ID;

	if( !m_pCurrentWeaponButes )
		return;

	g_pWeaponMgr->ReadBarrel(m_pCurrentWeaponButes->BarrelName.c_str(), nBarrelId, nAmmoId, nAmmoPoolId);
	if( !m_pCurrentWeaponButes->WeaponName.empty() )
	{
		WEAPON * pWeaponData = g_pWeaponMgr->GetWeapon(m_pCurrentWeaponButes->WeaponName.c_str());
		if( pWeaponData )
		{
			nWeaponId = pWeaponData->nId;
		}
	}

	if( nWeaponId == WMGR_INVALID_ID && nBarrelId != WMGR_INVALID_ID )
	{
		nWeaponId = g_pWeaponMgr->FindWeaponWithBarrel(nBarrelId);
	}

  	if( pWeapons->ChangeWeapon(nWeaponId) )
	{
		m_pCurrentWeapon = pWeapons->GetCurWeapon();
		_ASSERT( m_pCurrentWeapon );

		if( m_pCurrentWeapon )
		{
			m_pCurrentWeapon->SetBarrelId(nBarrelId);
			m_pCurrentWeapon->SetAmmoId(nAmmoId);

			m_pCurrentWeapon->ReloadClip();

			m_pTarget->SetAimAtHead( false );

			if( m_pCurrentWeaponButes->fAimAtHeadChance > 0.0f )
				m_pTarget->SetAimAtHead( GetRandom(0.0f,1.0f) < m_pCurrentWeaponButes->fAimAtHeadChance );

			m_tmrBurstDelay.Stop();
		}
	}
	else
	{
		_ASSERT(0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Consistent
//
//	PURPOSE:	Does sanity checks for the AI.
//
// ----------------------------------------------------------------------- //

bool CAI::Consistent()
{
	_ASSERT( g_pAIButeMgr );
	if( !g_pAIButeMgr ) return false;

	_ASSERT( GetButes() );
	if( !GetButes() ) return false;

	// Be sure the character types are correct.
	const AIBM_Template & ai_butes = g_pAIButeMgr->GetTemplate( m_nTemplateId );
	if(    ai_butes.eCharacterClass != GetButes()->m_eCharacterClass )
	{
		// The corporate type translates into several types in the character butes system.
		if( !(   ai_butes.eCharacterClass == CORPORATE
			  && ( GetButes()->m_eCharacterClass == SYNTHETIC || GetButes()->m_eCharacterClass == CORPORATE_EXOSUIT ) ) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint( "Error!! AI \"%s\" has a CharacterType of \"%s\" and a non-matching AttributeTemplate of \"%s\".",
				g_pLTServer->GetObjectName(m_hObject),
				GetButes()->m_szName, ai_butes.strName.c_str() );
#endif
			return false;
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Target
//
//	PURPOSE:	Targets an enemy
//
// ----------------------------------------------------------------------- //

void CAI::Target(HOBJECT hObject, const LTVector & vLastKnownPos /*=LTVector(0,0,0)*/)
{
	if ( hObject && !IsCharacter(hObject) ) 
	{
		_ASSERT(LTFALSE);
		return;
	}

	m_pTarget->SetObject(hObject, vLastKnownPos);
	m_pTarget->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateEye
//
//	PURPOSE:	Updates the eye position.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateEye()
{
	m_vEyePos = GetPosition() + GetHeadOffset();
	m_vEyeForward = GetAimForward();
	m_rEyeRotation = GetAimRotation();

/*  Get socket transform is an expensive operation, trying to remove as many as possible. PLH 8-29-01 

	// Get the eye node.
	HMODELSOCKET hEyeSocket;
	if( LT_OK != g_pModelLT->GetSocket(m_hObject, const_cast<char*>(g_szEyeSocketName), hEyeSocket) )
	{
		m_vEyePos = m_cmMovement.GetPos();
		m_vEyeForward = m_cmMovement.GetForward();
		m_rEyeRotation = m_cmMovement.GetRot();
		return;
	}

	// Pull the eye's position and forward direction from
	// the node.
	LTransform transform;
	g_pModelLT->GetSocketTransform(m_hObject, hEyeSocket, transform, LTTRUE);


	LTVector vEyePos;
	g_pTransLT->GetPos(transform, m_vEyePos);


	g_pTransLT->GetRot(transform, m_rEyeRotation);
	
	LTVector vRight, vUp;
	g_pMathLT->GetRotationVectors(m_rEyeRotation, vRight, vUp, m_vEyeForward);
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CreateGoal
//
//	PURPOSE:	Creates a goal from a string name
//
// ----------------------------------------------------------------------- //

Goal * CAI::CreateGoal(const char * szGoalName)
{
	ASSERT( szGoalName );

	Goal * pGoal = LTNULL;

	if( stricmp(szGoalName, "CHASE")  == 0)
	{
		pGoal = new ChaseGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "PATROL") == 0 )
	{
		pGoal = new PatrolGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "ATTACK") == 0 )
	{
		pGoal = new AttackGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "LURK") == 0 )
	{
		pGoal = new LurkGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "SCRIPT") == 0 )
	{
		pGoal = new ScriptGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "IDLESCRIPT") == 0 )
	{
		pGoal = new ScriptGoal(this, szGoalName, LTTRUE);
	}
	else if( stricmp(szGoalName, "INVESTIGATE") == 0 )
	{
		pGoal = new InvestigateGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "RETREAT") == 0 )
	{
		pGoal = new CowerGoal(this, LTTRUE, szGoalName);
	}
	else if( stricmp(szGoalName, "SNIPE") == 0 )
	{
		pGoal = new SnipeGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "COWER") == 0 )
	{
		pGoal = new CowerGoal(this, LTFALSE, szGoalName);
	}
	else if( stricmp(szGoalName, "ALARM") == 0 )
	{
		pGoal = new AlarmGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "Idle") == 0 )
	{
		pGoal = new IdleGoal(this, szGoalName);
	}
	else if( stricmp(szGoalName, "Cinematic") == 0 )
	{
		// This is just an idle script.
		pGoal = new ScriptGoal(this, szGoalName, LTTRUE);
	}
	else if( stricmp(szGoalName, "NodeCheck") == 0 )
	{
		pGoal = new NodeCheckGoal(this, szGoalName);
	}

	return pGoal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetCurrentSlotName
//
//	PURPOSE:	Gets the slot name of the current goal, returns empty
//              string if there isn't a current goal.
//
// ----------------------------------------------------------------------- //

std::string CAI::GetCurrentSlotName() const
{
	static std::string empty_string;

	if( !m_pCurrentBid ) return "";

	return m_pBidMgr->GetSlotName(m_pCurrentBid);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::RemoveGoal
//
//	PURPOSE:	Removes a goal, if it exists.
//
// ----------------------------------------------------------------------- //

void CAI::RemoveGoal(const std::string & slot_name)
{
	if( m_pCurrentBid == m_pBidMgr->GetBidder(slot_name) )
	{
		if( m_pCurrentBid && m_pCurrentBid->GetGoalPtr() )
			m_pCurrentBid->GetGoalPtr()->End();

		m_pCurrentBid = LTNULL;
	}

	m_pBidMgr->RemoveBid(slot_name);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::AddGoal
//
//	PURPOSE:	Safely adds a goal.  
//				Handles problem of adding a goal on top of another goal.
//
// ----------------------------------------------------------------------- //

void CAI::AddGoal(const std::string & slot_name, Goal * pGoalToAdd, LTFLOAT fModifiedBid /*= 0.0f*/)
{
	// Be sure any other goal with the same name is removed.
	RemoveGoal(slot_name);

	m_pBidMgr->AddBid(slot_name, pGoalToAdd, fModifiedBid );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetAction
//
//	PURPOSE:	Gets an existing AIAction * from our instantiated list, or
//				allocates a new one, if needed.
//
// ----------------------------------------------------------------------- //
AIAction * CAI::GetAction(ActionType eAction)
{
	if (!m_aActions[eAction])
	{
		// first use -- allocate it
		// Actions stay instantiated until the AI is destroyed
		switch(eAction)
		{
			case ACTION_AIM:
				m_aActions[eAction] = new AIActionAim(this);
				break;
			case ACTION_BACKOFF:
				m_aActions[eAction] = new AIActionBackoff(this);
				break;
			case ACTION_BEZIERTO:
				m_aActions[eAction] = new AIActionBezierTo(this);
				break;
			case ACTION_CROUCH:
				m_aActions[eAction] = new AIActionCrouch(this);
				break;
			case ACTION_CROUCHRELOAD:
				m_aActions[eAction] = new AIActionCrouchReload(this);
				break;
			case ACTION_DODGE:
				m_aActions[eAction] = new AIActionDodge(this);
				break;
			case ACTION_DROP:
				m_aActions[eAction] = new AIActionDrop(this);
				break;
			case ACTION_FACEHUG:
				m_aActions[eAction] = new AIActionFacehug(this);
				break;
			case ACTION_FIREMELEE:
				m_aActions[eAction] = new AIActionFireMelee(this);
				break;
			case ACTION_FIREWEAPON:
				m_aActions[eAction] = new AIActionFireWeapon(this);
				break;
			case ACTION_IDLE:
				break;		// this will cause interruptible actions to stop
			case ACTION_JUMPTO:
				m_aActions[eAction] = new AIActionJumpTo(this);
				break;
			case ACTION_LADDERTO:
				m_aActions[eAction] = new AIActionLadderTo(this);
				break;
			case ACTION_MOVETO:
				m_aActions[eAction] = new AIActionMoveTo(this);
				break;
			case ACTION_MOVEFORWARD:
				m_aActions[eAction] = new AIActionMoveForward(this);
				break;
			case ACTION_PLAYANIM:
				m_aActions[eAction] = new AIActionPlayAnim(this);
				break;
			case ACTION_POUNCE:
				m_aActions[eAction] = new AIActionPounce(this);
				break;
			case ACTION_RELOAD:
				m_aActions[eAction] = new AIActionReload(this);
				break;
			case ACTION_STUN:
				m_aActions[eAction] = new AIActionStun(this);
				break;
			case ACTION_WWFORWARD:
				m_aActions[eAction] = new AIActionWWForward(this);
				break;
			case ACTION_WWJUMPTO:
				m_aActions[eAction] = new AIActionWWJumpTo(this);
				break;
			case ACTION_TURNTO:
				m_aActions[eAction] = new AIActionTurnTo(this);
				break;
			default:
				break;
		}
	}

	return m_aActions[eAction];
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CanDoAction
//
//	PURPOSE:	Return true if the AI is able to perform a new action.
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::CanDoNewAction() const
{
	// Be sure we are done with our current action.
	if(    m_pCurrentAction 
		&& !m_pCurrentAction->IsDone() 
		&& !m_pCurrentAction->IsInterruptible() )
		return LTFALSE;

	// First come, first serve!
	if( m_pNextAction )
		return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetNextAction
//
//	PURPOSE:	Pulls an AIAction* from the enum, and passes it over. Returns
//				the AIAction* if one was found, otherwise returns LTNULL.
//
// ----------------------------------------------------------------------- //
AIAction * CAI::SetNextAction(ActionType eAction)
{
	// Make sure we can accept a new action
	if (!CanDoNewAction() )
		return LTNULL;

	// Idle "action" is used to force an interrupt.  No class is actually instantiated.
	if (eAction == ACTION_IDLE)
	{
		if (m_pCurrentAction && (m_pCurrentAction->IsInterruptible()))
			m_pCurrentAction->Interrupt();

#ifndef _FINAL
		if( ShowActionLevel(m_hObject) > 0.0f )
		{
			AICPrint(m_hObject, "Next action is \"Idle\".");
		}
#endif
		return LTNULL;
	}

	// Get the requested action.
	AIAction * pNewAction = GetAction(eAction);
	if (!pNewAction)
		return LTNULL;

	// Store the action away, it will be started when the current one is done.
	m_pNextAction = pNewAction;

	// Request the current one to stop.
	if( m_pCurrentAction && !m_pCurrentAction->IsDone()) 
		m_pCurrentAction->Interrupt();

#ifndef _FINAL
	if( ShowActionLevel(m_hObject) > 0.0f )
	{
		AICPrint(m_hObject, "Next action is \"%s\".", m_pNextAction->GetActionDescription() );
	}
#endif

	return m_pNextAction;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::LookAt
//
//	PURPOSE:	AI will use head to follow the object, set to null to turn off.
//
// ----------------------------------------------------------------------- //

void CAI::LookAt(HOBJECT hLookee)
{
	if( !GetButes()->m_bCanUseLookAt )
		return;

	// Don't repeat if we are already looking at him.
	if( m_bHasLATarget && m_hLATarget && m_hLATarget == hLookee )
		return;

	//do some house work here
	m_bHasLATarget = hLookee?LTTRUE:LTFALSE;
	m_hLATarget = hLookee;
	
	// We need the message immediately so that we can tell the client as soon
	// as possible to stop using the hLookee handle.
	g_pLTServer->CreateInterObjectLink(m_hObject, hLookee);

	RCMessage pMsg;
	if( !pMsg.null() )
	{
		pMsg->WriteByte(SFX_CHARACTER_ID);
		pMsg->WriteObject(m_hObject);
		pMsg->WriteByte(CFX_NODECONTROL_HEAD_FOLLOW_OBJ);
		pMsg->WriteObject(hLookee);
		pMsg->WriteHString(LTString("Head_node"));
		pMsg->WriteByte( hLookee != LTNULL );

		g_pLTServer->SendToClient(*pMsg, MID_SFX_MESSAGE, LTNULL, MESSAGE_NAGGLEFAST | MESSAGE_NOTHROTTLE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::LookAt
//
//	PURPOSE:	AI will rotate head to look at vPos, call LookAt(NULL) to turn off.
//
// ----------------------------------------------------------------------- //

void CAI::LookAt(const LTVector & vPos)
{
	if( !GetButes()->m_bCanUseLookAt )
		return;

	if( m_bHasLATarget && m_hLATarget )
	{
		g_pLTServer->BreakInterObjectLink(m_hObject,m_hLATarget);
		m_hLATarget = LTNULL;
		m_bHasLATarget = LTNULL;
	}

	RCMessage pMsg;
	if( !pMsg.null() )
	{
		pMsg->WriteByte(SFX_CHARACTER_ID);
		pMsg->WriteObject(m_hObject);
		pMsg->WriteByte(CFX_NODECONTROL_HEAD_FOLLOW_POS);
		pMsg->WriteVector(const_cast<LTVector&>(vPos));
		pMsg->WriteHString(LTString("Head_node"));
		pMsg->WriteByte( TRUE );

		g_pLTServer->SendToClient(*pMsg, MID_SFX_MESSAGE, LTNULL, 0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CanUseNode
//
//	PURPOSE:	Pathing uses this determine if a node can be used.
//
// ----------------------------------------------------------------------- //

bool CAI::CanUseNode(const CAINode & node) const
{
	// Is node de-activated?
	if( !node.IsActive() )
		return false;

	// Is it a wall walk node?
	if( node.IsWallWalkOnly() && !GetButes()->m_bWallWalk )
		return false;

	// Is it outside our leash?
	if (!WithinLeash(node.GetPos()))
		return false;
	
	// Can our "race" use it?
	if( !node.AllowAliens() )
	{
		if( GetButes()->m_eCharacterClass == ALIEN )
		{
			return false;
		}
	}

	if( !node.AllowCorporates() )
	{
		if(    GetButes()->m_eCharacterClass ==  CORPORATE
			|| GetButes()->m_eCharacterClass ==  SYNTHETIC
			|| GetButes()->m_eCharacterClass ==  CORPORATE_EXOSUIT )
		{
			return false;
		}
	}

	if( !node.AllowMarines() )
	{
		if(    GetButes()->m_eCharacterClass == MARINE
			|| GetButes()->m_eCharacterClass == MARINE_EXOSUIT )
		{
			return false;
		}
	}

	if( !node.AllowPredators() )
	{
		if( GetButes()->m_eCharacterClass == PREDATOR )
		{
			return false;
		}
	}


	// Deal with any special volume neighbor node zaniness.
	if( const CAIVolumeNeighborNode * pVolumeNeighborNode = dynamic_cast<const CAIVolumeNeighborNode*>( &node  ) )
	{
		// Deal with a non-ai-triggerable door.
		if( pVolumeNeighborNode->GetContainingVolumePtr() && pVolumeNeighborNode->GetContainingVolumePtr()->HasDoors() )
		{
			const CAIVolume & door_volume = *pVolumeNeighborNode->GetContainingVolumePtr();

			bool can_open = CanOpenDoor();
			{for( CAIVolume::const_DoorIterator iter = door_volume.BeginDoors();
				 can_open && iter != door_volume.EndDoors(); ++iter )
			{
				if( !iter->isAITriggerable ) 
				{
					can_open = false;
				}
				else
				{
					Door * pDoor = dynamic_cast<Door*>( g_pLTServer->HandleToObject(iter->handle) );
					if( pDoor )
					{
						if( pDoor->IsLocked() )
							can_open = false;
					}
				}
			}}

			if( !can_open )
			{
				// Check to see if any door is closed.
				bool door_closed = false;
				for( CAIVolume::const_DoorIterator iter = door_volume.BeginDoors();
					 !door_closed && iter != door_volume.EndDoors(); ++iter )
				{
					Door * pDoor = dynamic_cast<Door*>( g_pLTServer->HandleToObject(iter->handle) );
					if(     pDoor 
						&& (    pDoor->GetState() == DOORSTATE_CLOSED
							 || pDoor->GetState() == DOORSTATE_CLOSING ) )
					{
						door_closed = true;
					}
				}

				// Whew!  If the door can not be opened and it is closed then we can't use it!
				if( door_closed )
					return false;
			}

		} //if( pVolumeNeighborNode->GetVolumePtr() && pVolumeNeighborNode->GetVolumePtr()->HasDoors() )

	} //if( const CAIVolumeNeighborNode * pVolumeNeighborNode = dynamic_cast<const CAIVolumeNeighborNode*>( &dest_node  ) )


	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Teleport
//
//	PURPOSE:	Teleports AI to a parsed location.
//				WARNING: Most goals are not expecting this command!  
//				  If you call this, be sure the goal doesn't keep running
//				  after it is called without being aware of the teleport!
//
// ----------------------------------------------------------------------- //

void CAI::Teleport(const ParseLocation & plocation)
{
	LTBOOL  bMoveObjectToFloor = LTFALSE;

	LTVector vDest = GetPosition();
	LTVector vFacing = GetForward();

	if( plocation.pNode )
	{
		if( plocation.pNode->IsWallWalkOnly() && GetButes()->m_bWallWalk )
		{
			// Deal with a wall walk node.
			const LTVector & vNormal = plocation.pNode->GetNormal();
			_ASSERT(vNormal.MagSqr() > 0.5f);

			const LTFLOAT fHeight = GetMovement()->GetDimsSetting(CM_DIMS_WALLWALK).y;

			vDest = plocation.vPosition;
			vDest += vNormal*fHeight; //vNormal*fHeight*0.9f;

			WallWalk();
			m_cmMovement.SetObjectDims(CM_DIMS_WALLWALK);
			m_cmMovement.SetMovementState(CMS_WALLWALKING);

			LTRotation rRot;
			LTVector vAlignDir = vNormal.Cross( GetRight() );
			if( vAlignDir.MagSqr() < 0.5f )
			{
				vAlignDir = vNormal.Cross( GetUp() );
			}

			g_pLTServer->GetMathLT()->AlignRotation(rRot, vAlignDir, const_cast<LTVector&>(vNormal) );
			m_cmMovement.SetRot(rRot);

			bMoveObjectToFloor = LTFALSE;
		}
		else
		{
			const LTFLOAT fHeight = GetMovement()->GetDimsSetting(CM_DIMS_STANDING).y;
			vDest = plocation.vPosition;
			vDest.y += fHeight + 1.0f;

			bMoveObjectToFloor = LTTRUE;
		}

		vFacing = plocation.pNode->GetForward();
	}
	else if( plocation.hObject )
	{
		vDest = plocation.vPosition;

		LTRotation rRot;
		g_pLTServer->GetObjectRotation(plocation.hObject, &rRot);

		LTVector vU,vR;
		g_pMathLT->GetRotationVectors(rRot,vR,vU,vFacing);
	}
	else if( plocation.bTarget )
	{
#ifndef _FINAL
		AIErrorPrint(m_hObject, "AI can not teleport to target!");
#endif
		return;
	}

	// Clear everything out that isn't expecting a teleport.
	m_pNextAction = LTNULL;
	if( m_pCurrentAction )
		m_pCurrentAction->End();
	m_pCurrentAction = LTNULL;

	Stop();

	// Do the teleport.
	m_cmMovement.SetPos(vDest,MOVEOBJECT_TELEPORT);
	
	if( bMoveObjectToFloor )
		MoveObjectToFloor(m_hObject, LTFALSE, LTFALSE);

	SnapTurn(vFacing);

	// Reset our pathing information.
	UpdateVolumeInfo(LTTRUE);

	m_pNextNode = LTNULL;
	m_pLastNode = plocation.pNode;

}

		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetLeash
//
//	PURPOSE:	Sets up a leash.
//
// ----------------------------------------------------------------------- //

void CAI::SetLeash(LTFLOAT fLength, const LTVector & vPoint)
{
	m_fLeashLengthSqr = fLength * fLength;

	m_vLeashFrom = vPoint;
}

void CAI::SetLeash(LTFLOAT fLength, const char * szObject)
{
	LTVector vPos;

	if (!szObject || !szObject[0] || (_stricmp(szObject, "here") == 0))
	{
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
	}
	else
	{
		CAINode * pNode = g_pAINodeMgr->GetNode(szObject);
		if (pNode)
		{
			vPos = pNode->GetPos();
		}
		else
		{
			HOBJECT hObject = LTNULL;
			if( LT_OK == FindNamedObject(szObject,hObject) )
			{
				g_pLTServer->GetObjectPos(hObject, &vPos);
			}
			else
			{
				return;
			}
		}
	}

	SetLeash(fLength, vPos);
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::WithinLeash
//
//	PURPOSE:	Returns false if point is outside of leash
//
// ----------------------------------------------------------------------- //

bool CAI::WithinLeash(const LTVector & vPos) const
{
	if( m_fLeashLengthSqr > 0.0f )
	{
		LTVector vDist = vPos - m_vLeashFrom;
		vDist.y = 0.0f;

		if( m_fLeashLengthSqr < vDist.MagSqr() )
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::WithinLeash
//
//	PURPOSE:	Returns false if point is outside of leash
//
// ----------------------------------------------------------------------- //

const LTVector & CAI::GetLeashPoint() const
{
	if( m_fLeashLengthSqr > 0.0f )
		return m_vLeashFrom;

	return GetPosition();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HasDangerousWeapon
//
//	PURPOSE:	Determine if the weapon we are holding is dangerous
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::HasDangerousWeapon()
{
	if ( !GetCurrentWeaponPtr() )
	{
		// No gun... so no stimulus

		return LTFALSE;
	}
	else
	{
		return LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetFlashlightRot
//
//	PURPOSE:	Determines the correct flashlight rotation.
//
// ----------------------------------------------------------------------- //


LTRotation CAI::GetFlashlightRot() const
{
	// Hard coded constants.  Just gotta love 'em!
	const LTFLOAT fMaxFollowAimTime = 3.0f;
	const LTFLOAT fInterpolationTime = 1.0f;

	// Other useful values.
	const LTFLOAT fCurrentTime = g_pLTServer->GetTime();
	const LTFLOAT fTimeDiff = fCurrentTime - m_fLastAimAtTime;

	// 06/26/01 - MarkS
	// Removed: was causing flashlight to aim straight out, then snap
	// to the downward position.  Looks better without this.
//	if( m_fLastAimAtTime < 0.0f )
//	{
//		return LTRotation(0,0,0,0);
//	}


	if( fTimeDiff > fMaxFollowAimTime )
	{
		if( fTimeDiff > fInterpolationTime )
		{
			return GetEyeRotation();
		}

		// Do a linear interpolation in LithTech's backwards way.
		LTRotation rRot;

#ifdef __LINUX
		LTRotation rAim = GetAimRotation();
		LTRotation rEye = GetEyeRotation();
		g_pMathLT->InterpolateRotation(rRot, rAim, rEye, (fTimeDiff - fMaxFollowAimTime)/fInterpolationTime);
#else
		g_pMathLT->InterpolateRotation(rRot,const_cast<LTRotation&>(GetAimRotation()),const_cast<LTRotation&>(GetEyeRotation()), (fTimeDiff - fMaxFollowAimTime)/fInterpolationTime );
#endif
		return rRot;
	}

	return GetAimRotation();
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateFlashlight
//
//	PURPOSE:	Determines the correct flashlight rotation.
//
// ----------------------------------------------------------------------- //


void CAI::UpdateFlashlight() const
{
	// Hard coded constants.  Just gotta love 'em!
	const LTFLOAT fMaxFollowAimTime = 3.0f;
	const LTFLOAT fInterpolationTime = 1.0f;

	// Other useful values.
	const LTFLOAT fCurrentTime = g_pLTServer->GetTime();
	const LTFLOAT fTimeDiff = fCurrentTime - m_fLastAimAtTime;

	// 06/26/01 - MarkS
	// Removed: was causing flashlight to aim straight out, then snap
	// to the downward position.  Looks better without this.
//	if( m_fLastAimAtTime < 0.0f )
//	{
//		return LTRotation(0,0,0,0);
//	}


	LTRotation rRot;

	if( fTimeDiff > fMaxFollowAimTime )
		rRot = LTRotation(0,0,0,0);
	else
		rRot = GetAimRotation();

//	if( fTimeDiff < fInterpolationTime )
//	{
//		// Do a linear interpolation in LithTech's backwards way.
//		LTRotation rRot;
//		g_pMathLT->InterpolateRotation(rRot,const_cast<LTRotation&>(GetAimRotation()),const_cast<LTRotation&>(GetEyeRotation()), (fTimeDiff - fMaxFollowAimTime)/fInterpolationTime );
//	}

	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_FLASHLIGHT);
	g_pLTServer->WriteToMessageRotation(hMessage, &rRot);
	g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetFlashlight
//
//	PURPOSE:	Ignores command if m_bForceFlashlightOn is true, otherwise
//				calls up to CCharacter::SetFlashlight
//
// ----------------------------------------------------------------------- //

void CAI::SetFlashlight(LTBOOL bOn)
{
	// Ignore a turn off command if the flashlight
	// is forced on.
	if( m_bForceFlashlightOn && !bOn )
	{
		return;
	}

	// Otherwise, let character handle turning the light on or off.
	CCharacter::SetFlashlight(bOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleInjured
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CAI::HandleInjured()
{
	CCharacter::HandleInjured();

	// Slow them down.
	const AIBM_Template & butes = g_pAIButeMgr->GetTemplate( GetTemplateId() );

	if( butes.fInjuredSpeed > 0.0f )
	{
		const LTFLOAT fWalkSpeed = m_cmMovement.GetMaxSpeed(CMS_WALKING);

		CharacterVars * pVars = m_cmMovement.GetCharacterVars();

		if( fWalkSpeed > 0.0f  )
		{
			LTFLOAT fNewSpeedMod = butes.fInjuredSpeed / fWalkSpeed;
			pVars->m_fSpeedMod *= fNewSpeedMod;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateWeaponAnimation
//
//	PURPOSE:	Update the weapon animation information
//
// ----------------------------------------------------------------------- //

WEAPON* CAI::UpdateWeaponAnimation()
{
	// Do this instead of CCharacter's function... since AI's handle
	// weapon selection a little differently.

	// ----------------------------------------------------------------------- //
	// Set the weapon animation layer

	LTBOOL bWeaponValid = LTFALSE;
	WEAPON *pWeap = LTNULL;


	const AIWBM_AIWeapon * pWeaponButes = GetCurrentWeaponButesPtr();
	if( pWeaponButes )
	{
		if( pWeaponButes->eRace != AIWBM_AIWeapon::Alien )
		{
			if( !pWeaponButes->AnimReference.empty() )
			{
				bWeaponValid = m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, pWeaponButes->AnimReference.c_str() );
			}
			else
			{
				const CWeapon *pWeapon = GetCurrentWeaponPtr();
				if( pWeapon )
				{
					pWeap = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
					
					if(pWeap)
					{
						bWeaponValid = m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, pWeap->szName);
					}
				}
			}
		}
		else
		{
			// Melee weapons should be dealt with by the goals (AIAlienAttack is the only one currently).  PLH 3-16-01
			bWeaponValid = LTTRUE;
		}
	}

	if(!bWeaponValid)
		m_caAnimation.SetLayerReference(ANIMSTYLELAYER_WEAPON, "None");	

	return pWeap;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateActionAnimation
//
//	PURPOSE:	Update the action animation information
//
// ----------------------------------------------------------------------- //

void CAI::UpdateActionAnimation()
{
//	CCharacter::UpdateActionAnimation();

	if( !m_pCurrentAction && !m_pNextAction )
	{
		if( GetInjuredMovement() )
			m_caAnimation.SetLayerReference(ANIMSTYLELAYER_ACTION, "InjuredIdle");
		else if( GetMustCrawl() )
			m_caAnimation.SetLayerReference(ANIMSTYLELAYER_ACTION, "Crouch");
		else
			m_caAnimation.SetLayerReference(ANIMSTYLELAYER_ACTION, "Idle");
	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
////////////////////       CAIPlugin               //////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

static bool		s_bInitialized = false;
static CAIButeMgr	s_AIButeMgr;
static CAIWeaponButeMgr     s_AIWeaponButeMgr;
static CWeaponMgrPlugin		s_WeaponMgrPlugin;
static CCharacterButeMgr   s_CharacterButeMgr;
static CModelButeMgr		s_ModelButeMgr;
static CSurfaceMgr			s_SurfaceMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlugin::PreHook_EditStringList
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

DRESULT	CAIPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength)
{
	//
	// It's one of "our" properties.
	//

	// Make sure we have the butes file loaded.
	if( !s_bInitialized )
	{
		// Load AIButes.txt .
		char szFile[256];
#ifdef _WIN32
		sprintf(szFile, "%s\\%s", szRezPath,g_szAIButeMgrFile);
#else
		sprintf(szFile, "%s/%s", szRezPath,g_szAIButeMgrFile);
#endif
		s_AIButeMgr.SetInRezFile(LTFALSE);
		s_AIButeMgr.Init(g_pLTServer, szFile);


		// Initialize the AIWeaponButeMgr.
		s_AIWeaponButeMgr.SetInRezFile(LTFALSE);
		s_AIWeaponButeMgr.Init(g_pLTServer, szFile);

		// Initialize the WeaponMgr (loads Weapons.txt).
		s_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);


		// Load CharacterButes.txt .
		if( !g_pCharacterButeMgr )
		{
			if( !g_pModelButeMgr )
			{
#ifdef _WIN32
				sprintf(szFile, "%s\\%s", szRezPath,MBMGR_DEFAULT_FILE);
#else
				sprintf(szFile, "%s/%s", szRezPath,MBMGR_DEFAULT_FILE);
#endif
				s_ModelButeMgr.SetInRezFile(LTFALSE);
				s_ModelButeMgr.Init(g_pLTServer, szFile);
			}

			if ( !g_pSurfaceMgr )
			{
#ifdef _WIN32
				sprintf(szFile, "%s\\%s", szRezPath,SRFMGR_DEFAULT_FILE);
#else
				sprintf(szFile, "%s/%s", szRezPath,SRFMGR_DEFAULT_FILE);
#endif
				s_SurfaceMgr.SetInRezFile(LTFALSE);
				s_SurfaceMgr.Init(g_pLTServer, szFile);
			}

#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath,CHARACTER_BUTES_DEFAULT_FILE);
#else
			sprintf(szFile, "%s/%s", szRezPath,CHARACTER_BUTES_DEFAULT_FILE);
#endif
			s_CharacterButeMgr.SetInRezFile(LTFALSE);
			s_CharacterButeMgr.Init(g_pLTServer, szFile);
		}

		// Initialize the music mgr.
		if( g_pMusicButeMgr && !g_pMusicButeMgr->IsValid( ))
		{
#ifdef _WIN32
			sprintf(szFile, "%s\\%s", szRezPath, "Attributes/music.txt" );
#else
			sprintf(szFile, "%s/%s", szRezPath, "Attributes/music.txt" );
#endif
			g_pMusicButeMgr->SetInRezFile(LTFALSE);
			g_pMusicButeMgr->Init(g_pLTServer, szFile);
		}

		s_bInitialized = true;
	}

	// Do AttributeTemplate property.
	if( stricmp("AttributeTemplate",szPropName) == 0 )
	{
		const int original_cStrings = *pcStrings;

		const int num_ids = s_AIButeMgr.GetNumIDs();
		for(int i = 0; i < num_ids && *pcStrings < cMaxStrings; ++i )
		{
			const AIBM_Template & butes = s_AIButeMgr.GetTemplate(i);
			ASSERT( butes.strName.size()+1 < cMaxStringLength);

			if( IsValidCharacterClass(butes.eCharacterClass) 
				&& butes.nHideFromDEdit == 0 )
			{
				if(   !butes.strName.empty()
					&& butes.strName.size()+1 < cMaxStringLength)
				{
					strcpy( *aszStrings, butes.strName.c_str() );
					++aszStrings;
					++(*pcStrings);
				}
			}
		}

		if( *pcStrings > (uint32)original_cStrings ) return LT_OK;
	}
	else if(    stricmp("Weapon",szPropName) == 0
		     || stricmp("SecondaryWeapon",szPropName) == 0
			 || stricmp("TertiaryWeapon",szPropName) == 0 )
	{
		// Add <default>, to go to default.
		strcpy(*aszStrings, g_szDefault);
		++aszStrings;
		++(*pcStrings);

		// Add <none>.
		strcpy(*aszStrings,g_szNone);
		++aszStrings;
		++(*pcStrings);

		// Add weapons from weapons bute file.
		const int num_ids = s_AIWeaponButeMgr.GetNumIDs();
		for(int i = 0; i < num_ids && *pcStrings < cMaxStrings; ++i )
		{
			const AIWBM_AIWeapon & butes = s_AIWeaponButeMgr.GetAIWeapon(i);
			ASSERT( butes.Name.size()+1 < cMaxStringLength);

			if( !butes.Name.empty() 
				&& butes.Name.size()+1 < cMaxStringLength
				&& butes.nHideFromDEdit == 0 
				&& CanUseWeapon(butes) )
			{
				strcpy( *aszStrings, butes.Name.c_str() );
				++aszStrings;
				++(*pcStrings);
			}
		}

		return LT_OK;
	}
	else if( stricmp("CharacterType",szPropName) == 0 )
	{
		const int original_cStrings = *pcStrings;

		typedef std::vector<std::string> CharacterNameList;
		CharacterNameList character_name_list;

		const int num_ids = g_pCharacterButeMgr->GetNumSets();
		for(int i = 0; i < num_ids && *pcStrings < cMaxStrings; ++i )
		{
			CString cstrButeName = g_pCharacterButeMgr->GetModelType(i);
			const int tag_offset = cstrButeName.Find("_AI");

			if( tag_offset > 0 && IsValidCharacterClass(g_pCharacterButeMgr->GetClass(i)) )
			{
				if( (uint32)tag_offset + 1 < cMaxStringLength)
				{
					character_name_list.push_back( std::string(cstrButeName.Left(tag_offset)) );
				}
			}
		}

		std::sort(character_name_list.begin(), character_name_list.end());

		for( CharacterNameList::iterator iter = character_name_list.begin();
			 iter != character_name_list.end() && (*pcStrings) + 1 < cMaxStrings; ++iter )
		{
			strcpy(aszStrings[(*pcStrings)++], iter->c_str() );
		}		

		if( *pcStrings > (uint32)original_cStrings )
			return LT_OK;
	}
	else if( strstr(szPropName,"GoalType") != 0 )
	{
		strcpy( *aszStrings, g_szNone );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Alarm" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Attack" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Cower" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Idle" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "IdleScript" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Investigate" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Lurk" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Patrol" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Retreat" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Script" );
		++aszStrings;
		++(*pcStrings);

		strcpy( *aszStrings, "Snipe" );
		++aszStrings;
		++(*pcStrings);

		return LT_OK;
	}
	// Handle the music properties.
	else if( stricmp("AmbientTrackSet",szPropName) == 0 ||
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

bool CAIPlugin::CanUseWeapon(const AIWBM_AIWeapon & butes) const
{ 
	return false; 
}

static std::string ReadGoalProp( GenericProp * pGenProp, int number)
{
	ASSERT( pGenProp );
	if( !pGenProp ) return "";

	char prop_name[15];

	// The stored goal creation string.
	std::string queued_command = "AG ";

	// Read the goal type, if it is blank return an empty string.
	sprintf(prop_name, "GoalType_%d", number);
	if ( g_pLTServer->GetPropGeneric(prop_name, pGenProp ) == DE_OK && pGenProp->m_String[0] )
	{
		// If the goal type is "<none>" a goal was not specified, return an empty string.
		if( stricmp(pGenProp->m_String,g_szNone) == 0 )
			return "";
		
		queued_command += pGenProp->m_String;
		queued_command += " ";
		
		// Read the parameters.
		queued_command += "(";
		
		// The link parameter has its own property.
		sprintf(prop_name, "Link_%d", number);
		if ( g_pLTServer->GetPropGeneric(prop_name, pGenProp ) == DE_OK && pGenProp->m_String[0] )
		{
			queued_command += "LINK ";
			queued_command += pGenProp->m_String;
			queued_command += "; ";
		}
		
		// Read the other parameters.
		sprintf(prop_name, "Parameters_%d", number);
		if ( g_pLTServer->GetPropGeneric(prop_name, pGenProp ) == DE_OK && pGenProp->m_String[0] )
		{
			queued_command += pGenProp->m_String;
			queued_command += " ";
		}
		
		// Finish the parameters.
		queued_command += ")";
	}
	else
	{
		// Didn't have a goal type specified.
		return "";
	}

	return queued_command;
}

void CAI::Active(bool make_active, bool save_flags /* = true */)
{
	ASSERT( GetMovement() );

	if( make_active && !m_bActive )
	{
		m_bActive = LTTRUE;

		m_cmMovement.SetObjectFlags(OFT_Flags, m_dwActiveFlags );
		m_cmMovement.SetObjectFlags(OFT_Flags2, m_dwActiveFlags2 );
		m_cmMovement.SetObjectUserFlags(m_dwActiveUserFlags );
		
		m_dwActiveFlags = 0;
		m_dwActiveFlags2 = 0;
		m_dwActiveUserFlags = 0;

		g_pCharacterMgr->Add(this);

		m_damage.SetCanDamage(m_bActiveCanDamage);


		if( m_hHead )
		{
			g_pLTServer->Common()->SetObjectFlags(m_hHead, OFT_Flags, m_dwActiveHeadFlags);
			g_pLTServer->Common()->SetObjectFlags(m_hHead, OFT_Flags2, m_dwActiveHeadFlags2);
			g_pLTServer->SetObjectUserFlags(m_hHead, m_dwActiveHeadUserFlags);
		}

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
	else if( !make_active && m_bActive )
	{
		m_bActive = LTFALSE;

		// Be sure our goals and actions are cleared out.
		if( m_pCurrentBid )
		{
			if( m_pCurrentBid->GetGoalPtr() ) m_pCurrentBid->GetGoalPtr()->End();
			m_pCurrentBid = LTNULL;
		}

		if( m_pCurrentAction )
		{
			m_pCurrentAction->End();
			m_pCurrentAction = LTNULL;
		}
		
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


		if( save_flags )
		{
			m_bActiveCanDamage = m_damage.GetCanDamage();
		}

		m_damage.SetCanDamage(LTFALSE);

		// Update the attachments so that they get a chance to make themselves invisible.
		GetAttachments()->Update();

		if( m_hHead )
		{
			if( save_flags )
			{
				g_pLTServer->Common()->GetObjectFlags(m_hHead, OFT_Flags, m_dwActiveHeadFlags);
				g_pLTServer->Common()->GetObjectFlags(m_hHead, OFT_Flags2, m_dwActiveHeadFlags2);
				m_dwActiveHeadUserFlags = g_pLTServer->GetObjectUserFlags(m_hHead);
			}

			g_pLTServer->Common()->SetObjectFlags(m_hHead, OFT_Flags, 0);
			g_pLTServer->Common()->SetObjectFlags(m_hHead, OFT_Flags2, 0);
			g_pLTServer->SetObjectUserFlags(m_hHead, 0);
		}

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


void CAI::StartCinematic(HOBJECT cinematic_trigger_handle)
{
	// Be sure any other cinematic is stopped.
	EndCinematic();

	g_pLTServer->SetDeactivationTime(m_hObject, c_fAIDeactivationTime);

	// Be sure we have a cinematic trigger.
	m_hCinematicTrigger = cinematic_trigger_handle;
}

void CAI::EndCinematic(LTBOOL bNotifyTrigger /* = LTTRUE */ )
{
	if( bNotifyTrigger )
	{
		if( m_hCinematicTrigger )
		{
			CinematicTrigger * pCinematicTrigger = dynamic_cast<CinematicTrigger*>( g_pLTServer->HandleToObject(m_hCinematicTrigger) );
			if( pCinematicTrigger )
			{
				pCinematicTrigger->AIStop();
			}
		}
	}

	m_hCinematicTrigger = LTNULL;
}

//void CAI::HandleEMPEffect()
//{
//	CCharacter::HandleEMPEffect();
//
//	if(m_nCurrentWeapon >= 0)
//	{
//		CWeapons* pWeapons = m_apWeapons[m_nCurrentWeapon]->GetParent();
//
//		if(pWeapons)
//			pWeapons->SetAmmoPool( "Predator_Energy" , 0);
//	}
//}


void CAI::JumpTo(const LTVector & vDest)
{
	ASSERT( !m_bUsingClipTo );

	// If you change these, please change CAI::Load as well.
	m_cmMovement.SetHandleStateFunc(CMS_SPECIAL,			CharacterMovementState_AIJump);
	m_cmMovement.SetMaxSpeedFunc(CMS_SPECIAL,				CharacterMovementState_AIJump_MaxSpeed);
	m_cmMovement.SetMaxAccelFunc(CMS_SPECIAL,				CharacterMovementState_AIJump_MaxAccel);

	const CharacterMovementState cms = m_cmMovement.GetMovementState();

	if(m_cmMovement.GetCharacterVars()->m_bStandingOn || cms == CMS_WALLWALKING)
	{
		m_cmMovement.SetStart( m_cmMovement.GetPos() );
		m_cmMovement.SetDestination( vDest );
		m_cmMovement.SetTimeStamp(g_pLTServer->GetTime());

		LTVector vNull(0.0f, 0.0f, 0.0f);
		m_cmMovement.SetVelocity(vNull);
		m_cmMovement.SetAcceleration(vNull);

		m_cmMovement.SetMovementState(CMS_SPECIAL);

		m_bUsingJumpTo = LTTRUE;
	}

}

void CharacterMovementState_AIJump(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	MathLT *pMath = pMovement->m_pMath;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	INTERFACE *pInterface = pMovement->m_pInterface;
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();

	// Make sure gravity is turned off for the character
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags & ~FLAG_GRAVITY & ~FLAG_SOLID, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);


	// Get our time information
	LTFLOAT fTime = pInterface->GetTime();
	LTFLOAT fTimeDelta = fTime - cvVars->m_fStampTime;

	// Get the percent that we should be at across the entire
	// travel distance
	LTVector vDir = cvVars->m_vDestination - cvVars->m_vStart;
	const LTFLOAT fDirMag = vDir.Mag();
	const LTFLOAT fJumpSpeed = (cbButes->m_fBaseJumpSpeed*cvVars->m_fJumpSpeedMod);
	const LTFLOAT fTotalTravelTime = fJumpSpeed > 0.0f ? fDirMag / fJumpSpeed : fDirMag;
	const LTFLOAT fPercent = fTimeDelta / fTotalTravelTime;

	// See if we're doine traveling the whole path or have changed containers
	if(fPercent >= 1.0f )
	{
		// Reset our rotation
		LTRotation rRot;
		LTVector vUp(0.0f, 1.0f, 0.0f);
		vDir.y = 0.0f;
		pMath->AlignRotation(rRot, vDir, vUp);
		pMovement->SetRot(rRot);

		pMovement->SetPos(cvVars->m_vDestination);

		// Set allow jump so that we can determine if the jump succeeded.
		pMovement->AllowJump( fPercent >= 1.0f );

		// Set our movement state to falling
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);

		if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
			pMovement->SetMovementState(CMS_SWIMMING);
		else
			pMovement->SetMovementState(CMS_PRE_FALLING);

		return;
	}


	// Get the point across the direction that we need as a reference
	LTVector vDirPos = cvVars->m_vStart + (vDir * fPercent);

	// Align a rotation down the path
	LTRotation rAlignRot;
	LTVector vR, vU, vF;
	vU.Init(0.0f, 1.0f, 0.0f);
	pMath->AlignRotation(rAlignRot, vDir, vU);
	pMath->GetRotationVectors(rAlignRot, vR, vU, vF);

	// Get our arc offset
	LTFLOAT fArc = sinf(MATH_PI * fPercent);

	// Calculate the offset above the path
	LTVector vOffset = vU * (fArc * fDirMag * 0.25f);

	// Update the position
	LTVector vOldPos = cvVars->m_vPos;
	LTVector vNewPos = vDirPos + vOffset;
	const LTVector vChangePos = vNewPos - vOldPos;
	ASSERT( cvVars->m_fFrameTime );
	if( cvVars->m_fFrameTime > 0.0f )
	{
		const LTVector vVelocity = vChangePos/cvVars->m_fFrameTime;
		pMovement->SetVelocity(vVelocity);
	}
	else
	{
		pMovement->SetPos(vNewPos);
	}

	// Update our rotation along the path
	vDir = vNewPos - vOldPos;
	vDir.y = 0.0f;
	vU.Init(0.0f, 1.0f, 0.0f);
	pMath->AlignRotation(rAlignRot, vDir, vU);
	pMovement->SetRot(rAlignRot);

	// Update our aiming angle so we're always looking at the destination
	pMath->GetRotationVectors(rAlignRot, vR, vU, vF);
	vDir = cvVars->m_vDestination - vNewPos;
	vDir.Norm();

	LTFLOAT fAngle	= vDir.Dot(vF);

	// Bounds check
	fAngle = fAngle < -1.0f ? -1.0f : fAngle;
	fAngle = fAngle > 1.0f ? 1.0f : fAngle;

	cvVars->m_fAimingPitch = MATH_RADIANS_TO_DEGREES(acosf(fAngle));
}

LTFLOAT CharacterMovementState_AIJump_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseJumpSpeed * pMovement->GetCharacterVars()->m_fJumpSpeedMod;
}

LTFLOAT CharacterMovementState_AIJump_MaxAccel(const CharacterMovement *pMovement)
{
	return 0.0f;
}


void CAI::ClipTo(const LTVector & vDest, const LTVector & vNormal)
{
	ASSERT( !m_bUsingJumpTo );

	// If you change these, please change CAI::Load as well.
	m_cmMovement.SetHandleStateFunc(CMS_SPECIAL,			CharacterMovementState_AIClip);
	m_cmMovement.SetMaxSpeedFunc(CMS_SPECIAL,				CharacterMovementState_AIClip_MaxSpeed);
	m_cmMovement.SetMaxAccelFunc(CMS_SPECIAL,				CharacterMovementState_AIClip_MaxAccel);

	LTVector vNewForward = vDest - GetPosition();
	vNewForward -= vNormal*vNormal.Dot(vNewForward);

	LTRotation rDest;
	g_pLTServer->GetMathLT()->AlignRotation(rDest, vNewForward, const_cast<LTVector&>(vNormal) );

	m_cmMovement.SetStart( m_cmMovement.GetPos(),  m_cmMovement.GetRot() );
	m_cmMovement.SetDestination( vDest, rDest );
	m_cmMovement.SetTimeStamp(g_pLTServer->GetTime());

	LTVector vNull(0.0f, 0.0f, 0.0f);
	m_cmMovement.SetVelocity(vNull);
	m_cmMovement.SetAcceleration(vNull);


	m_cmMovement.SetMovementState(CMS_SPECIAL);

	m_bUsingClipTo = LTTRUE;

}

void CharacterMovementState_AIClip(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	MathLT *pMath = pMovement->m_pMath;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	INTERFACE *pInterface = pMovement->m_pInterface;
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();

	// Make sure gravity is turned off for the character
	const uint32 dwFlags = FLAG_GOTHRUWORLD | FLAG_VISIBLE;
	const uint32 dwFlags2 = cbButes->m_bWallWalk ? (FLAG2_ORIENTMOVEMENT | FLAG2_SPHEREPHYSICS) : 0;
	pMovement->SetObjectFlags(OFT_Flags, dwFlags, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, dwFlags2, LTFALSE);

	// Be sure we aren't trying to move.
	pMovement->UpdateVelocity(0.0f,CM_DIR_FLAG_ALL);
	pMovement->UpdateAcceleration(0.0f,CM_DIR_FLAG_ALL);

	// Get our time information
	LTFLOAT fTime = pInterface->GetTime();
	LTFLOAT fTimeDelta = fTime - cvVars->m_fStampTime;

	// Get the percent that we should be at across the entire
	// travel distance
	const LTVector vDir = cvVars->m_vDestination - cvVars->m_vStart;

	const LTFLOAT fDirMag = vDir.Mag();
	const LTFLOAT fClipSpeed = cbButes->m_fBasePounceSpeed;
	const LTFLOAT fTotalTravelTime = fClipSpeed > 0.0f ? fDirMag / fClipSpeed : fDirMag;
	const LTFLOAT fPercent = fTimeDelta / fTotalTravelTime;

	// See if we're doine traveling the whole path or have changed containers
	if(fPercent >= 1.0f)
	{
		pMovement->SetPos(cvVars->m_vDestination);
		pMovement->SetRot(cvVars->m_rDestination);

		// Set our movement state to Idle
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);

		if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
			pMovement->SetMovementState(CMS_SWIMMING);
		else
		{
			if( cbButes->m_bWallWalk )
			{
				pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2 | FLAG2_ORIENTMOVEMENT | FLAG2_SPHEREPHYSICS, LTFALSE);
				pMovement->SetObjectDims(CM_DIMS_WALLWALK);
				pMovement->SetMovementState(CMS_WALLWALKING);
			}
			else
			{
				pMovement->SetMovementState(CMS_PRE_FALLING);
			}
		}

		return;
	}


	// Get the point across the direction that we need as a reference
	LTVector vDirPos = cvVars->m_vStart + (vDir * fPercent);


	// Update our rotation along the path
	LTRotation rAlignRot;
	pMath->InterpolateRotation(rAlignRot, cvVars->m_rStart, cvVars->m_rDestination, fPercent);
	pMovement->SetRot(rAlignRot);

	// Update the position
	pPhysics->MoveObject(hObject, &vDirPos, 0);

}

LTFLOAT CharacterMovementState_AIClip_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBasePounceSpeed;
}

LTFLOAT CharacterMovementState_AIClip_MaxAccel(const CharacterMovement *pMovement)
{
	return 0.0f;
}

