// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMovement.cpp
//
// PURPOSE : General character movement
//
// CREATED : 2/1/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterButeMgr.h"
#include "CharacterMovement.h"
#include "CharacterAnimation.h"
#include "ContainerCodes.h"
#include "DamageTypes.h"

#ifdef __LINUX
	#include <math.h>
	#define _isnan isnan
#endif

// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD
	#include "MsgIDs.h"
	#include "ClientSoundMgr.h"
	#include "CameraMgrFX.h"
	#include "GameClientShell.h"
	#include "CharacterFX.h"
	#include "ProfileMgr.h"
	#include "VarTrack.h"
#else
	#include "WeaponMgr.h"
	#include "Destructible.h"
	#include "ServerSoundMgr.h"
	#include "CVarTrack.h"
	#include "GameServerShell.h"
#endif

// ----------------------------------------------------------------------- //

#define CHARACTERMOVEMENT_STANDARD_MASS			2000.0f
#define CHARACTERMOVEMENT_STANDARD_FRICTION		12.5f
#define CHARACTERMOVEMENT_WALLWALK_FRICTION		15.0f
#define CHARACTERMOVEMENT_SWIMMING_FRICTION		2.5f
#define CHARACTERMOVEMENT_LADDER_FRICTION		0.0f

#define CHARACTERMOVEMENT_VEL_CHECK_FUDGE		5.0f

#define CHARACTERMOVEMENT_MAX_RUN_AIR_VEL		500.0f

#define CHARACTERMOVEMENT_AIMING_YAW_CAP		45.0f

#define CHARACTERMOVEMENT_LIQUID_VISCOSITY		1.0f

#define CHARACTERMOVEMENT_ENCUMBERED_SCALE		0.75f

#ifdef _DEBUG
//	#define CHARACTER_MOVEMENT_DEBUG
	const LTBOOL bDebugPlayer = LTFALSE;
#endif

#ifdef _CLIENTBUILD
	static VarTrack vtWWDetectDist;
#else
	static CVarTrack vtWWDetectDist;
#endif

// ----------------------------------------------------------------------- //

char *CMSNames[CMS_MAX] =
{
	"Idle",				// CMS_IDLE
	"Walking",			// CMS_WALKING,			
	"Running",			// CMS_RUNNING,			
	"Start Jump",		// CMS_PRE_JUMPING,		
	"Start Pounce Jump", // CMS_PRE_POUNCE_JUMP,					
	"Pounce Jump",		// CMS_POUNCE_JUMP,		
	"Stand Jump",		// CMS_STAND_JUMP,			
	"Walk Jump",		// CMS_WALK_JUMP,			
	"Run Jump",			// CMS_RUN_JUMP,			
	"Wall Walk Jump",	// CMS_WWALK_JUMP,			
	"Start Fall",		// CMS_PRE_FALLING,		
	"Pounce Fall",		// CMS_POUNCE_FALL,		
	"Stand Fall",		// CMS_STAND_FALL,		
	"Walk Fall",		// CMS_WALK_FALL,			
	"Run Fall",			// CMS_RUN_FALL,			
	"Wall Walk Fall",	// CMS_WWALK_FALL,			
	"Stand Land",		// CMS_STAND_LAND,			
	"Walk Land",		// CMS_WALK_LAND,			
	"Run Land",			// CMS_RUN_LAND,			
	"Wall Walk Land",	// CMS_WWALK_LAND,			
	"Start Crawling",	// CMS_PRE_CRAWLING,		
	"Crawling",			// CMS_CRAWLING,			
	"End Crawling",		// CMS_POST_CRAWLING,		
	"Wall Walking",		// CMS_WALLWALKING,			
	"Climbing",			// CMS_CLIMBING,		
	"Swimming",			// CMS_SWIMMING,			
	"Flying",			// CMS_FLYING,				
	"Special",			// CMS_SPECIAL,				
	"Clipping",			// CMS_CLIPPING,		
	"End Clipping",		// CMS_POST_CLIPPING,		
	"Pouncing",			// CMS_POUNCING,			
	"Facehug",			// CMS_FACEHUG,			
	"Facehug Attached",	// CMS_FACEHUG_ATTACH,		
	"Observing",		// CMS_OBSERVING,
};


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //


#define REALIGN_STANDARD				0
#define REALIGN_WALLWALK_JUMP			1
#define REALIGN_STANDING_ON				2

void RealignCharacter(CharacterMovement *pMovement, int nType = REALIGN_STANDARD)
{
	CharacterVars*	cvVars	= pMovement->GetCharacterVars();
	MathLT*			pMath	= pMovement->m_pMath;

	// Reorient the character with the new gravity...
	LTVector vR, vU, vF;
	vU.Init(0.0f, 1.0f, 0.0f);

	// If we're already aligned... don't worry about it
	if(vU.Dot(cvVars->m_vUp) >= 0.9999f)
		return;


	switch(nType)
	{
		case REALIGN_WALLWALK_JUMP:
		{
			LTFLOAT ay = (LTFLOAT)fabs(cvVars->m_vForward.y);
			vF = (cvVars->m_vForward * (1.0f - ay)) + (cvVars->m_vUp * ay);

			vF.y = 0.0f;
			vF.Norm();

			break;
		}

		case REALIGN_STANDING_ON:
		{
			vU = cvVars->m_ciStandingOn.m_Plane.m_Normal;

			vF = vU.Cross(cvVars->m_vRight);
			vF.Norm();

			break;
		}

		case REALIGN_STANDARD:
		default:
		{
			if(vU.Equals(cvVars->m_vForward, 0.1f) || vU.Equals(-cvVars->m_vForward, 0.1f))
			{
				vF = vU.Cross(cvVars->m_vRight);
				vF.Norm();
			}
			else
			{
				vR = vU.Cross(cvVars->m_vForward);
				vR.Norm();

				vF = vR.Cross(vU);
			}

			break;
		}
	}


	LTRotation rRot;
	pMath->AlignRotation(rRot, vF, vU);
	pMovement->SetRot(rRot);
}

// --------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::CharacterMovement()
//
// PURPOSE:		Set everything to default values
//
// ----------------------------------------------------------------------- //

CharacterMovement::CharacterMovement()
{
	m_pInterface = LTNULL;
	m_pPhysics = LTNULL;
	m_pCommon = LTNULL;
	m_pMath = LTNULL;

	m_cmsLast = CMS_MIN;
	m_cmsCurrent = CMS_MIN;

	m_hDimsObject = LTNULL;

	memset(m_fpHandleState, 0, sizeof(void*) * CMS_MAX);
	memset(m_fpTouchNotify, 0, sizeof(void*) * CMS_MAX);
	memset(m_fpMaxSpeed, 0, sizeof(void*) * CMS_MAX);
	memset(m_fpMaxAccel, 0, sizeof(void*) * CMS_MAX);

	m_fpHandleMessageRead = LTNULL;
	m_fpHandleMessageWrite = LTNULL;

	m_fpHandleContainerPhysicsInfo = LTNULL;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::~CharacterMovement()
//
// PURPOSE:		Terminate everything if we didn't call it already
//
// ----------------------------------------------------------------------- //

CharacterMovement::~CharacterMovement()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::Init()
//
// PURPOSE:		Setup the main interfaces and pointers.  
//				Be sure to call SetCharacterButes before using movement!
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::Init(INTERFACE *pInterface, HOBJECT hObj, CharacterAnimation* pAnim)
{
	m_pInterface = pInterface;

#ifdef _CLIENTBUILD
	m_pPhysics = (ILTClientPhysics*)m_pInterface->Physics();
#else
	m_pPhysics = m_pInterface->Physics();
#endif

	m_pCommon = m_pInterface->Common();
	m_pMath = m_pInterface->GetMathLT();

	m_Vars.m_hObject = hObj;
	m_Vars.m_pCharAnim = pAnim;

	// Make sure all our interfaces got setup properly
	if(!m_pInterface || !m_pPhysics || !m_pCommon || !m_pMath)
		return LTFALSE;

	// Set the initial position and rotation
	if(m_Vars.m_hObject)
	{
		m_pInterface->GetObjectPos(m_Vars.m_hObject, &m_Vars.m_vPos);
		m_pInterface->GetObjectRotation(m_Vars.m_hObject, &m_Vars.m_rRot);
		m_pMath->GetRotationVectors(m_Vars.m_rRot, m_Vars.m_vRight, m_Vars.m_vUp, m_Vars.m_vForward);

		m_pCommon->GetObjectFlags(m_Vars.m_hObject, OFT_Flags, m_Vars.m_dwFlags);
		m_pCommon->GetObjectFlags(m_Vars.m_hObject, OFT_Flags2, m_Vars.m_dwFlags2);

		m_Vars.m_fFriction = CHARACTERMOVEMENT_STANDARD_FRICTION;
		m_pPhysics->SetFrictionCoefficient(m_Vars.m_hObject, m_Vars.m_fFriction);
		m_pPhysics->SetForceIgnoreLimit(m_Vars.m_hObject, 0.0f);
	}

	// Set the state functions
	SetHandleStateFunc(CMS_IDLE,			CharacterMovementState_Idle);
	SetMaxSpeedFunc(CMS_IDLE,				CharacterMovementState_Idle_MaxSpeed);
	SetMaxAccelFunc(CMS_IDLE,				CharacterMovementState_Idle_MaxAccel);

	SetHandleStateFunc(CMS_WALKING,			CharacterMovementState_Walking);
	SetMaxSpeedFunc(CMS_WALKING,			CharacterMovementState_Walking_MaxSpeed);
	SetMaxAccelFunc(CMS_WALKING,			CharacterMovementState_Walking_MaxAccel);

	SetHandleStateFunc(CMS_RUNNING,			CharacterMovementState_Running);
	SetMaxSpeedFunc(CMS_RUNNING,			CharacterMovementState_Running_MaxSpeed);
	SetMaxAccelFunc(CMS_RUNNING,			CharacterMovementState_Running_MaxAccel);

	SetHandleStateFunc(CMS_PRE_JUMPING,		CharacterMovementState_Pre_Jumping);
	SetMaxSpeedFunc(CMS_PRE_JUMPING,		CharacterMovementState_Jumping_MaxSpeed);
	SetMaxAccelFunc(CMS_PRE_JUMPING,		CharacterMovementState_Jumping_MaxAccel);

	SetHandleStateFunc(CMS_PRE_POUNCE_JUMP,	CharacterMovementState_Pre_PounceJump);
	SetMaxSpeedFunc(CMS_PRE_POUNCE_JUMP,	CharacterMovementState_Jumping_MaxSpeed);
	SetMaxAccelFunc(CMS_PRE_POUNCE_JUMP,	CharacterMovementState_Jumping_MaxAccel);

	SetHandleStateFunc(CMS_POUNCE_JUMP,		CharacterMovementState_Pounce_Jump);
	SetTouchNotifyFunc(CMS_POUNCE_JUMP,		CharacterMovementState_Pounce_Jump_TN);
	SetMaxSpeedFunc(CMS_POUNCE_JUMP,		CharacterMovementState_Jumping_MaxSpeed);
	SetMaxAccelFunc(CMS_POUNCE_JUMP,		CharacterMovementState_Jumping_MaxAccel);

	SetHandleStateFunc(CMS_STAND_JUMP,		CharacterMovementState_Stand_Jump);
	SetMaxSpeedFunc(CMS_STAND_JUMP,			CharacterMovementState_Jumping_MaxSpeed);
	SetMaxAccelFunc(CMS_STAND_JUMP,			CharacterMovementState_Jumping_MaxAccel);

	SetHandleStateFunc(CMS_WALK_JUMP,		CharacterMovementState_Walk_Jump);
	SetMaxSpeedFunc(CMS_WALK_JUMP,			CharacterMovementState_Jumping_MaxSpeed);
	SetMaxAccelFunc(CMS_WALK_JUMP,			CharacterMovementState_Jumping_MaxAccel);

	SetHandleStateFunc(CMS_RUN_JUMP,		CharacterMovementState_Run_Jump);
	SetMaxSpeedFunc(CMS_RUN_JUMP,			CharacterMovementState_Jumping_MaxSpeed);
	SetMaxAccelFunc(CMS_RUN_JUMP,			CharacterMovementState_Jumping_MaxAccel);

	SetHandleStateFunc(CMS_WWALK_JUMP,		CharacterMovementState_WWalk_Jump);
	SetTouchNotifyFunc(CMS_WWALK_JUMP,		CharacterMovementState_Jump_TN);
	SetMaxSpeedFunc(CMS_WWALK_JUMP,			CharacterMovementState_Jumping_MaxSpeed);
	SetMaxAccelFunc(CMS_WWALK_JUMP,			CharacterMovementState_Jumping_MaxAccel);

	SetHandleStateFunc(CMS_PRE_FALLING,		CharacterMovementState_Pre_Falling);
	SetMaxSpeedFunc(CMS_PRE_FALLING,		CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_PRE_FALLING,		CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_STAND_FALL,		CharacterMovementState_Stand_Fall);
	SetMaxSpeedFunc(CMS_STAND_FALL,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_STAND_FALL,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_WALK_FALL,		CharacterMovementState_Walk_Fall);
	SetMaxSpeedFunc(CMS_WALK_FALL,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_WALK_FALL,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_RUN_FALL,		CharacterMovementState_Run_Fall);
	SetMaxSpeedFunc(CMS_RUN_FALL,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_RUN_FALL,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_WWALK_FALL,		CharacterMovementState_WWalk_Fall);
	SetTouchNotifyFunc(CMS_WWALK_FALL,		CharacterMovementState_Fall_TN);
	SetMaxSpeedFunc(CMS_WWALK_FALL,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_WWALK_FALL,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_POUNCE_FALL,		CharacterMovementState_Pounce_Fall);
	SetTouchNotifyFunc(CMS_POUNCE_FALL,		CharacterMovementState_Pounce_Jump_TN);
	SetMaxSpeedFunc(CMS_POUNCE_FALL,		CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_POUNCE_FALL,		CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_STAND_LAND,		CharacterMovementState_Stand_Land);
	SetMaxSpeedFunc(CMS_STAND_LAND,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_STAND_LAND,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_WALK_LAND,		CharacterMovementState_Walk_Land);
	SetMaxSpeedFunc(CMS_WALK_LAND,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_WALK_LAND,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_RUN_LAND,		CharacterMovementState_Run_Land);
	SetMaxSpeedFunc(CMS_RUN_LAND,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_RUN_LAND,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_WWALK_LAND,		CharacterMovementState_WWalk_Land);
	SetMaxSpeedFunc(CMS_WWALK_LAND,			CharacterMovementState_Falling_MaxSpeed);
	SetMaxAccelFunc(CMS_WWALK_LAND,			CharacterMovementState_Falling_MaxAccel);

	SetHandleStateFunc(CMS_PRE_CRAWLING,	CharacterMovementState_Pre_Crawling);
	SetMaxSpeedFunc(CMS_PRE_CRAWLING,		CharacterMovementState_Crawling_MaxSpeed);
	SetMaxAccelFunc(CMS_PRE_CRAWLING,		CharacterMovementState_Crawling_MaxAccel);

	SetHandleStateFunc(CMS_CRAWLING,		CharacterMovementState_Crawling);
	SetMaxSpeedFunc(CMS_CRAWLING,			CharacterMovementState_Crawling_MaxSpeed);
	SetMaxAccelFunc(CMS_CRAWLING,			CharacterMovementState_Crawling_MaxAccel);

	SetHandleStateFunc(CMS_POST_CRAWLING,	CharacterMovementState_Post_Crawling);
	SetMaxSpeedFunc(CMS_POST_CRAWLING,		CharacterMovementState_Crawling_MaxSpeed);
	SetMaxAccelFunc(CMS_POST_CRAWLING,		CharacterMovementState_Crawling_MaxAccel);

	SetHandleStateFunc(CMS_WALLWALKING,		CharacterMovementState_WallWalking);
	SetMaxSpeedFunc(CMS_WALLWALKING,		CharacterMovementState_WallWalking_MaxSpeed);
	SetMaxAccelFunc(CMS_WALLWALKING,		CharacterMovementState_WallWalking_MaxAccel);

	SetHandleStateFunc(CMS_CLIMBING,		CharacterMovementState_Climbing);
	SetMaxSpeedFunc(CMS_CLIMBING,			CharacterMovementState_Climbing_MaxSpeed);
	SetMaxAccelFunc(CMS_CLIMBING,			CharacterMovementState_Climbing_MaxAccel);

	SetHandleStateFunc(CMS_SWIMMING,		CharacterMovementState_Swimming);
	SetMaxSpeedFunc(CMS_SWIMMING,			CharacterMovementState_Swimming_MaxSpeed);
	SetMaxAccelFunc(CMS_SWIMMING,			CharacterMovementState_Swimming_MaxAccel);
	
	SetHandleStateFunc(CMS_FLYING,			CharacterMovementState_Flying);
	SetMaxSpeedFunc(CMS_FLYING,				CharacterMovementState_Flying_MaxSpeed);
	SetMaxAccelFunc(CMS_FLYING,				CharacterMovementState_Flying_MaxAccel);

	SetHandleStateFunc(CMS_SPECIAL,			CharacterMovementState_Special);
	SetMaxSpeedFunc(CMS_SPECIAL,			CharacterMovementState_Special_MaxSpeed);
	SetMaxAccelFunc(CMS_SPECIAL,			CharacterMovementState_Special_MaxAccel);

	SetHandleStateFunc(CMS_CLIPPING,		CharacterMovementState_Clipping);
	SetMaxSpeedFunc(CMS_CLIPPING,			CharacterMovementState_Clipping_MaxSpeed);
	SetMaxAccelFunc(CMS_CLIPPING,			CharacterMovementState_Clipping_MaxAccel);

	SetHandleStateFunc(CMS_POST_CLIPPING,	CharacterMovementState_Post_Clipping);
	SetMaxSpeedFunc(CMS_POST_CLIPPING,		CharacterMovementState_Clipping_MaxSpeed);
	SetMaxAccelFunc(CMS_POST_CLIPPING,		CharacterMovementState_Clipping_MaxAccel);

	SetHandleStateFunc(CMS_POUNCING,		CharacterMovementState_Pouncing);
	SetTouchNotifyFunc(CMS_POUNCING,		CharacterMovementState_Pouncing_TN);
	SetMaxSpeedFunc(CMS_POUNCING,			CharacterMovementState_Pounce_MaxSpeed);
	SetMaxAccelFunc(CMS_POUNCING,			CharacterMovementState_Pounce_MaxAccel);

	SetHandleStateFunc(CMS_FACEHUG,			CharacterMovementState_Facehug);
	SetTouchNotifyFunc(CMS_FACEHUG,			CharacterMovementState_Facehug_TN);
	SetMaxSpeedFunc(CMS_FACEHUG,			CharacterMovementState_Facehug_MaxSpeed);
	SetMaxAccelFunc(CMS_FACEHUG,			CharacterMovementState_Facehug_MaxAccel);

	SetHandleStateFunc(CMS_FACEHUG_ATTACH,	CharacterMovementState_Facehug_Attach);
	SetMaxSpeedFunc(CMS_FACEHUG_ATTACH,		CharacterMovementState_Facehug_MaxSpeed);
	SetMaxAccelFunc(CMS_FACEHUG_ATTACH,		CharacterMovementState_Facehug_MaxAccel);

	SetHandleStateFunc(CMS_OBSERVING,		CharacterMovementState_Observing);
	SetMaxSpeedFunc(CMS_OBSERVING,			CharacterMovementState_Observing_MaxSpeed);
	SetMaxAccelFunc(CMS_OBSERVING,			CharacterMovementState_Observing_MaxAccel);


#ifdef _CLIENTBUILD

	// Destroy the dims object if it exists already...
	if(m_hDimsObject)
	{
		m_pInterface->DeleteObject(m_hDimsObject);
		m_hDimsObject = LTNULL;
	}

	// Create the dims object
	ObjectCreateStruct ocs;
	ocs.Clear();

	ocs.m_ObjectType = OT_NORMAL;
//	ocs.m_Flags = FLAG_SOLID;

	// Create the new object
	m_hDimsObject = m_pInterface->CreateObject(&ocs);

#endif


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::Term()
//
// PURPOSE:		Reset everything so we don't accicentally call something we
//				weren't suppose to
//
// ----------------------------------------------------------------------- //

void CharacterMovement::Term()
{
#ifdef _CLIENTBUILD
	// Call this first so the interfaces aren't invalid
	TermObject();
#endif

#ifdef _CLIENTBUILD

	// Destroy the dims object if it exists already...
	if(m_hDimsObject)
	{
		m_pInterface->DeleteObject(m_hDimsObject);
		m_hDimsObject = LTNULL;
	}

#endif

	m_pInterface = LTNULL;
	m_pPhysics = LTNULL;
	m_pCommon = LTNULL;
	m_pMath = LTNULL;

	memset(m_fpHandleState, 0, sizeof(void*) * CMS_MAX);
	memset(m_fpMaxSpeed, 0, sizeof(void*) * CMS_MAX);
	memset(m_fpMaxAccel, 0, sizeof(void*) * CMS_MAX);

	m_fpHandleMessageRead = LTNULL;
	m_fpHandleMessageWrite = LTNULL;

	m_fpHandleContainerPhysicsInfo = LTNULL;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::Load()
//
// PURPOSE:		Load information about the character movement
//
// ----------------------------------------------------------------------- //
#ifndef _CLIENTBUILD

void CharacterMovement::Load(HMESSAGEREAD hRead)
{
	*hRead >> m_Butes;
	*hRead >> m_Vars;

	*hRead >> m_cmsLast;
	*hRead >> m_cmsCurrent;
}

#endif

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::Save()
//
// PURPOSE:		Save information about the character movement
//
// ----------------------------------------------------------------------- //
#ifndef _CLIENTBUILD

void CharacterMovement::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_Butes;
	*hWrite << m_Vars;

	*hWrite << m_cmsLast;
	*hWrite << m_cmsCurrent;
}

#endif

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::CreateObject()
//
// PURPOSE:		Creates the object from the set attributes
//
// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD

LTBOOL CharacterMovement::CreateObject(uint32 dwFlags, uint32 dwFlags2)
{
	// Setup the object create structure
	ObjectCreateStruct ocs;
	ocs.Clear();

	m_Butes.FillInObjectCreateStruct(ocs);
	ocs.m_ObjectType = OT_MODEL;
	ocs.m_Flags = dwFlags ? dwFlags : FLAG_SOLID | FLAG_GRAVITY | FLAG_STAIRSTEP | FLAG_TOUCH_NOTIFY | FLAG_ANIMTRANSITION;
	ocs.m_Flags2 = dwFlags2 ? dwFlags2 : FLAG2_CYLINDERPHYSICS;
	ocs.m_Pos.Init();
	ocs.m_Rotation.Init();

	// Single player games will use full position resolution to help the server-side physics along.
	if( g_pGameClientShell->GetGameType()->IsSinglePlayer() && !dwFlags)
	{
		ocs.m_Flags |= FLAG_FULLPOSITIONRES;
	}

	// Create the new object
	HOBJECT hObject = m_pInterface->CreateObject(&ocs);

	// If the object creation failed, keep things the way they were
	if(!hObject) return LTFALSE;

	// Destroy the object if it exists already
	if(m_Vars.m_hObject)
		m_pInterface->DeleteObject(m_Vars.m_hObject);

	m_Vars.m_hObject = hObject;

	// Set the initial dimensions
	SetObjectDims(CM_DIMS_DEFAULT, LTNULL, LTTRUE);

	// Set the default friction
	m_Vars.m_fFriction = CHARACTERMOVEMENT_STANDARD_FRICTION;
	m_pPhysics->SetFrictionCoefficient(m_Vars.m_hObject, m_Vars.m_fFriction);
	m_pPhysics->SetForceIgnoreLimit(m_Vars.m_hObject, 0.0f);

	// Init the flags variables
	m_Vars.m_dwFlags = ocs.m_Flags;
	m_Vars.m_dwFlags2 = ocs.m_Flags2;

	return LTTRUE;
}

#endif

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::TermObject()
//
// PURPOSE:		Removes the object from the world
//
// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD

void CharacterMovement::TermObject()
{
	// Make sure the object exists first
	if(m_Vars.m_hObject && m_pInterface)
	{
		m_pInterface->DeleteObject(m_Vars.m_hObject);
		m_Vars.m_hObject = LTNULL;
	}
}

#endif

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::HandleMessageRead()
//
// PURPOSE:		Recieves a message from the client or server
//				Return true if the message was handled, or false if not...
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::HandleMessageRead(uint8 nType, HMESSAGEREAD hRead, uint32 dwFlags)
{
	// If we want to handle the message externally, go ahead and call that function
	if(m_fpHandleMessageRead)
		return (*m_fpHandleMessageRead)(nType, hRead, dwFlags);


	// Otherwise, do some standard handling of the message
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::HandleMessageWrite()
//
// PURPOSE:		Recieves a message from the client or server
//				Return true if the message was handled, or false if not...
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::HandleMessageWrite(uint8 nType, HMESSAGEWRITE hWrite, uint32 dwFlags)
{
	// If we want to handle the message externally, go ahead and call that function
	if(m_fpHandleMessageWrite)
		return (*m_fpHandleMessageWrite)(nType, hWrite, dwFlags);


	// Otherwise, do some standard handling of the message
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::Update()
//
// PURPOSE:		Handle the current state of the character movement
//
// ----------------------------------------------------------------------- //

void CharacterMovement::Update(LTFLOAT fDeltaTime)
{
	// If we don't have an object yet... better not do anything here
	if(!m_Vars.m_hObject) return;


	// Do some general variables updates
	m_pInterface->GetObjectPos(m_Vars.m_hObject, &m_Vars.m_vPos);
	m_pInterface->GetObjectRotation(m_Vars.m_hObject, &m_Vars.m_rRot);
	m_pMath->GetRotationVectors(m_Vars.m_rRot, m_Vars.m_vRight, m_Vars.m_vUp, m_Vars.m_vForward);

	m_pPhysics->GetVelocity(m_Vars.m_hObject, &m_Vars.m_vVelocity);
	m_pPhysics->GetAcceleration(m_Vars.m_hObject, &m_Vars.m_vAccel);

//	m_pInterface->CPrint("Velocity is (%f,%f,%f). Speed is %f.",
//		m_Vars.m_vVelocity.x, m_Vars.m_vVelocity.y, m_Vars.m_vVelocity.z,
//		m_Vars.m_vVelocity.Mag() );

	// If the velocity is rather small, we should just set it to zero
	if(m_Vars.m_vVelocity.MagSqr() < 9.0f)
		SetVelocity(LTVector(0.0f, 0.0f, 0.0f));

	// Get the time that has elapsed within the last frame update
	m_Vars.m_fFrameTime = fDeltaTime;

	// This is kind of a hack fix, but we need to cap the frame time so it seems like we
	// never update at a faster rate than 2 frames per second (without this there were bugs
	// on the initial updates of the character movement due to the fact that the game is still
	// loading and caching stuf. The frametime was too large and we were getting extreme bounces
	// of characters on their first couple of moves)
	if(m_Vars.m_fFrameTime > 0.5f) m_Vars.m_fFrameTime = 0.5f;

	// Update anything specific to the allow movement state
	UpdateAllowMovement();

	// Update anything specific to the allow rotation state
	UpdateAllowRotation();

	// Updates whether the character is standing on something
	UpdateStandingOn();

	// Update the rotation of the character
	UpdateRotation();

	// Update the aiming of the character (look up and down)
	UpdateAiming();

	// Update the container lists for body sections
#ifdef CHARACTER_MOVEMENT_DEBUG
	if(m_Vars.m_bContainersChanged = UpdateContainers())
	{
	#ifdef _CLIENTBUILD
		m_pInterface->CPrint("%.2f CharMov : Object(%s) Containers have changed!", m_pInterface->GetTime(), "Client Physics Object");
	#else
		if(bDebugPlayer || _stricmp(m_pInterface->GetObjectName(m_Vars.m_hObject), "Player"))
			m_pInterface->CPrint("%.2f CharMov : Object(%s) Containers have changed!", m_pInterface->GetTime(), m_pInterface->GetObjectName(m_Vars.m_hObject));
	#endif
	}
#else
	m_Vars.m_bContainersChanged = UpdateContainers();
#endif

	// Make sure the friction is set properly (this may get overriden in the movement states)
	m_pPhysics->SetFrictionCoefficient(m_Vars.m_hObject, m_Vars.m_fFriction * m_Vars.m_pContainers[CM_CONTAINER_LOWER].m_fFriction);

	// Call the current character movement state
	if(m_fpHandleState[m_cmsCurrent])
		(*m_fpHandleState[m_cmsCurrent])(this);


	// Now update any container physics that may be applied to the object
	UpdateContainerPhysics();


	// If we are standing on a character, be sure to bump us off!

	// First check to see that we are standing on a non-world object.
	if(    m_Vars.m_bStandingOn 
		&& m_Vars.m_ciStandingOn.m_hObject 
		&& m_Vars.m_ciStandingOn.m_hPoly == INVALID_HPOLY )
	{
		
		// Now determine if we are standing on a character.
#ifdef _CLIENTBUILD

		LTBOOL bIsCharacter = LTFALSE;

		CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);
		if( pList )
		{
			for( CSpecialFXList::Iterator iter = pList->Begin();
				 iter != pList->End() && !bIsCharacter; ++iter)
			{
				if( iter->pSFX && iter->pSFX->GetServerObj() == m_Vars.m_ciStandingOn.m_hObject )
				{
					if(g_pLTClient->GetClientObject() != m_Vars.m_ciStandingOn.m_hObject)
						bIsCharacter = LTTRUE;
				}
			}
		}
#else
		const LTBOOL bIsCharacter = IsCharacter( m_Vars.m_ciStandingOn.m_hObject );
#endif

		if( bIsCharacter )
		{
			// Bump us up and forward.
			const LTVector vOldAccel = m_Vars.m_vAccel;
			LTVector vNewAccel = m_Vars.m_vAccel;
			
			// Give us a slight push up and forward.
			vNewAccel.y = 10.0f;
			vNewAccel += m_Vars.m_vForward*10.0f;

			// Counter-act gravity.
			if( m_Vars.m_dwFlags & FLAG_GRAVITY )
			{
				LTVector vForce;
				m_pPhysics->GetGlobalForce(vForce);
				vNewAccel += -vForce;
			}


			SetAcceleration( vNewAccel );
		}

	} //if(m_Vars.m_bStandingOn && m_Vars.m_ciStandingOn.m_hObject )



//	m_pInterface->CPrint("Acceleration is (%f,%f,%f). AccelMag is %f. FrameTime is %f.",
//		m_Vars.m_vAccel.x, m_Vars.m_vAccel.y, m_Vars.m_vAccel.z,
//		m_Vars.m_vAccel.Mag(),
//		m_Vars.m_fFrameTime );

	// Otherwise, do some kind of standard update...

#ifdef _CLIENTBUILD

	// If this is a client object, get an offset of the movement that should have occured...
	MoveInfo miInfo;
	miInfo.m_hObject = m_Vars.m_hObject;
	miInfo.m_dt = m_Vars.m_fFrameTime;
	m_pPhysics->UpdateMovement(&miInfo);

	// If the movement offset is greater than a very small amount... move the object
	if((miInfo.m_Offset.MagSqr() > 0.01f) || (m_Vars.m_dwFlags2 & FLAG2_ORIENTMOVEMENT))
	{
		LTVector vNewPos = m_Vars.m_vPos + miInfo.m_Offset;
		SetPos(vNewPos, 0);
	}

#endif
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateAllowMovement()
//
// PURPOSE:		Handle anything that needs to be specifically set according
//				to the m_bAllowMovement bool in m_Vars
//
// ----------------------------------------------------------------------- //

void CharacterMovement::UpdateAllowMovement()
{
	if(!CanMove())
	{
		m_Vars.m_dwCtrl &= ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD | CM_FLAG_STRAFELEFT | CM_FLAG_STRAFERIGHT | CM_FLAG_JUMP | CM_FLAG_DUCK | CM_FLAG_WALLWALK);
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateAllowRotation()
//
// PURPOSE:		Handle anything that needs to be specifically set according
//				to the m_bAllowRotation bool in m_Vars
//
// ----------------------------------------------------------------------- //

void CharacterMovement::UpdateAllowRotation()
{
	if(!CanRotate())
	{
		m_Vars.m_dwCtrl &= ~(CM_FLAG_LEFT | CM_FLAG_RIGHT);
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateAcceleration()
//
// PURPOSE:		Updates the acceleration in various directions according to
//				the control flags
//
// ----------------------------------------------------------------------- //

LTFLOAT CharacterMovement::UpdateAcceleration(LTFLOAT fAccel, uint32 dwFlags, LTBOOL bAiming)
{
	// Setup the initial rotation to accelerate from
	LTVector vR, vU, vF, vAccel;
	LTRotation rRot = m_Vars.m_rRot;

	// Init the acceleration to zero
	vAccel = LTVector(0.0f, 0.0f, 0.0f);

	// Get the vectors of the object's facing
	if(bAiming)
	{
		// If we want to consider aiming, make sure our reference rotation is set properly
		m_pMath->RotateAroundAxis(rRot, m_Vars.m_vRight, MATH_DEGREES_TO_RADIANS(m_Vars.m_fAimingPitch));
		m_pMath->GetRotationVectors(rRot, vR, vU, vF);
	}
	else
	{
		vR = m_Vars.m_vRight;
		vU = m_Vars.m_vUp;
		vF = m_Vars.m_vForward;
	}

	// Calculate a new acceleration vector based off the control keys being used
	if(dwFlags & CM_DIR_FLAG_X_AXIS)
	{
		if(m_Vars.m_dwCtrl & CM_FLAG_STRAFERIGHT)		vAccel += (vR * fAccel);
		if(m_Vars.m_dwCtrl & CM_FLAG_STRAFELEFT)		vAccel -= (vR * fAccel);
	}

	if(dwFlags & CM_DIR_FLAG_Y_AXIS)
	{
		if(m_Vars.m_dwCtrl & CM_FLAG_JUMP)				vAccel += (vU * fAccel);
		if( (m_Vars.m_dwCtrl & CM_FLAG_DUCK) ||
			(m_Vars.m_dwCtrl & CM_FLAG_WALLWALK))		vAccel -= (vU * fAccel);
	}

	if(dwFlags & CM_DIR_FLAG_Z_AXIS)
	{
		if(m_Vars.m_dwCtrl & CM_FLAG_FORWARD)			vAccel += (vF * fAccel);
		if(m_Vars.m_dwCtrl & CM_FLAG_BACKWARD)			vAccel -= (vF * fAccel);
	}

	// Set the acceleration to our newly calculated vector
	SetAcceleration(vAccel);

	return vAccel.Mag();
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateVelocity()
//
// PURPOSE:		Caps off our velocity at fMaxVel along specific directions
//
// ----------------------------------------------------------------------- //

LTFLOAT CharacterMovement::UpdateVelocity(LTFLOAT fMaxVel, uint32 dwFlags)
{
	// Account for speed multipliers
	fMaxVel *= m_Vars.m_fSpeedMod;


	LTFLOAT fResult = 0.0f;
	if( fMaxVel > 0.0f )
	{
		// Separate the cap direction and the non-cap direction
		LTVector vCap(0.0f, 0.0f, 0.0f);
		LTVector vNonCap = m_Vars.m_vVelocity;

//#ifndef _CLIENTBUILD
		// Acceleration will need to be capped to make sure
		// we don't exceed max velocity.
		LTVector vAccelCap(0,0,0);
		LTVector vAccelNonCap = m_Vars.m_vAccel;
//#endif

		const LTVector & vU = m_Vars.m_vUp;
		const LTVector & vR = m_Vars.m_vRight;
		const LTVector & vF = m_Vars.m_vForward;

		if(dwFlags & CM_DIR_FLAG_X_AXIS)
		{
			vCap += vR*vR.Dot(m_Vars.m_vVelocity);
			vNonCap -= vR*vR.Dot(vNonCap);

//#ifndef _CLIENTBUILD
			vAccelCap += vR*vR.Dot(m_Vars.m_vAccel);
			vAccelNonCap -= vR*vR.Dot(vAccelNonCap);
//#endif
		}

		if(dwFlags & CM_DIR_FLAG_Y_AXIS)
		{
			vCap += vU*vU.Dot(m_Vars.m_vVelocity);
			vNonCap -= vU*vU.Dot(vNonCap);

//#ifndef _CLIENTBUILD
			vAccelCap += vU*vU.Dot(m_Vars.m_vAccel);
			vAccelNonCap -= vU*vU.Dot(vAccelNonCap);
//#endif
		}

		if(dwFlags & CM_DIR_FLAG_Z_AXIS)
		{
			vCap += vF*vF.Dot(m_Vars.m_vVelocity);
			vNonCap -= vF*vF.Dot(vNonCap);

//#ifndef _CLIENTBUILD
			vAccelCap += vF*vF.Dot(m_Vars.m_vAccel);
			vAccelNonCap -= vF*vF.Dot(vAccelNonCap);
//#endif
		}

		// Cap the appropriate vector
		LTFLOAT fCurSpeed = vCap.Mag();
		fResult = fCurSpeed;

		if(fCurSpeed >= fMaxVel)
		{
			vCap *= (fMaxVel / fCurSpeed);
			fResult = fMaxVel;
		}

//#ifndef _CLIENTBUILD
		// LT's friction model (air friction) ensures
		// that: 
		//   vMaxVel = vAccel / fFrictionCoeffiction.
		// So we cap vAccel to ensure vMaxVel.

		LTFLOAT fFriction = m_Vars.m_fFriction;
		if( fFriction > 0.0f )
		{
			const LTFLOAT fCurAccel = vAccelCap.Mag();
			if( fCurAccel > fFriction*fMaxVel )
			{
				vAccelCap *= fFriction*fMaxVel/fCurAccel;
				SetAcceleration(vAccelCap + vAccelNonCap);
			}
		}
//#endif
		// Add the non-cap vector into the final velocity
		// Set the velocity to our new vector
		SetVelocity(vCap + vNonCap);
	}
	else
	{
		// We've been told to stop, so just do it.
		SetVelocity(LTVector(0.0f,0.0f,0.0f));
	}

	return fResult;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateContainers()
//
// PURPOSE:		Update the containers for various sections of the character
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::UpdateContainers()
{
	// If the object doesn't exist... get out of here
	if(!m_Vars.m_hObject) return LTFALSE;

	// Get the model interface
	ModelLT *pModel = m_pInterface->GetModelLT();
	HMODELSOCKET hSocket = LTNULL;
	LTransform trans;
	LTVector vPos;
	LTBOOL bChanged = LTFALSE;
	uint32 i, j;

	// Go through each container section to see if there are sockets available
	for(i = 0; i < CM_NUM_CONTAINER_SECTIONS; i++)
	{
		// Get the player position as a default
		m_pInterface->GetObjectPos(m_Vars.m_hObject, &vPos);

		// Get a position offset for this section
		if(i == CM_CONTAINER_UPPER)
			vPos += GetUp() * (GetObjectDims().y * GetCharacterButes()->m_fCameraHeightPercent);
		else if(i == CM_CONTAINER_MID)
			vPos += GetUp() * (GetObjectDims().y * GetCharacterButes()->m_fCameraHeightPercent * 0.75f);
		else if(i == CM_CONTAINER_LOWER)
			vPos -= GetUp() * (GetObjectDims().y * 0.9f);


		// Setup some temporary container storage
		HOBJECT hContainers[CM_MAX_CONTAINERS];
		uint32 nNumContainers = 0;

		// Retrieve the position containers
		nNumContainers = m_pInterface->GetPointContainers(&vPos, hContainers, CM_MAX_CONTAINERS);

		// Clear out the physics information for each container
		m_Vars.m_pContainers[i].m_fFriction = 1.0f;
		m_Vars.m_pContainers[i].m_fViscosity = 0.0f;
		m_Vars.m_pContainers[i].m_vCurrent.Init();

		// Copy the number of containers and see if the value has changed
		if(m_Vars.m_pContainers[i].m_nNumContainers != nNumContainers)
		{
			m_Vars.m_pContainers[i].m_nNumContainers = nNumContainers;
			bChanged = LTTRUE;
		}

		// Copy the new container information and see if any of it has changed from our current containers
		for(j = 0; j < nNumContainers; j++)
		{
			if(m_Vars.m_pContainers[i].m_hContainers[j] != hContainers[j])
			{
				m_Vars.m_pContainers[i].m_hContainers[j] = hContainers[j];
				bChanged = LTTRUE;
			}

			// If the container physics update function exists... call it
			if(m_fpHandleContainerPhysicsInfo)
				(*m_fpHandleContainerPhysicsInfo)(m_Vars.m_pContainers[i].m_hContainers[j], m_Vars.m_pContainers[i]);
		}
	}

	// Return a value specifying if the containers have changed at all
	return bChanged;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateContainerPhysics()
//
// PURPOSE:		Updates the final velocity of the character based off of
//				container information
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::UpdateContainerPhysics()
{
	// If we don't have any containers... just return
	if(!m_Vars.m_pContainers[CM_CONTAINER_MID].m_nNumContainers) return LTFALSE;

	LTBOOL bLiquid = IsInLiquidContainer(CM_CONTAINER_MID);

	// Save the current velocity
	LTVector vCurVel = m_Vars.m_vVelocity;
	LTFLOAT fViscosity = bLiquid ? CHARACTERMOVEMENT_LIQUID_VISCOSITY : m_Vars.m_pContainers[CM_CONTAINER_MID].m_fViscosity;
	LTVector vCurrent = m_Vars.m_pContainers[CM_CONTAINER_MID].m_vCurrent;


	// ----------------------------------------------------------------------- //
	// Apply any container forces (currents) that are present
	vCurVel += vCurrent * m_Vars.m_fFrameTime;


	// Apply a sinking force if we're in a liquid
/*	if(bLiquid)
	{
		LTVector vForce;
		m_pPhysics->GetGlobalForce(vForce);
		vCurVel += vForce * 0.1f * m_Vars.m_fFrameTime * (m_Butes.m_fMass / CHARACTERMOVEMENT_STANDARD_MASS);
	}
*/

	// ----------------------------------------------------------------------- //
	// Do some viscosity damping on the velocity of this character
	if((fViscosity > 0.0f) && (vCurVel.MagSqr() > 0.001f))
	{
		LTVector vDir = vCurVel;

		vDir.Norm();
		vDir *= (GetMaxSpeed() * fViscosity * m_Vars.m_fFrameTime);

		if(vDir.MagSqr() < vCurVel.MagSqr())
		{
			vCurVel = vCurVel - vDir;
		}
		else
		{
			vCurVel.Init();
		}
	}


	// Apply any changes that were made
	SetVelocity(vCurVel);


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::IsInContainerType()
//
// PURPOSE:		Checks to see if a certain character section is within a
//				specified type of container
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::IsInContainerType(int nContainerCode, int nSection) const
{
	D_WORD nCode = 0;
	uint32 i, j;

	if( m_Vars.m_nForceContainerType > 0 && nContainerCode == m_Vars.m_nForceContainerType )
		return LTTRUE;

	// Check against all the character sections if needed
	if(nSection == CM_CONTAINER_ALL)
	{
		for(i = 0; i < CM_NUM_CONTAINER_SECTIONS; i++)
		{
			// Loop through all the valid containers for this section
			for(j = 0; j < m_Vars.m_pContainers[i].m_nNumContainers; j++)
			{
				// If the object is definitely a container... check the container code
				if(m_pInterface->GetContainerCode(m_Vars.m_pContainers[i].m_hContainers[j], &nCode))
				{
					if(nCode == nContainerCode)
						return LTTRUE;
				}
			}
		}
	}
	else
	{
		// If the section passed in is without of range... return false
		if((nSection < 0) || (nSection >= CM_NUM_CONTAINER_SECTIONS))
			return LTFALSE;

		// Loop through all the valid containers for this section
		for(j = 0; j < m_Vars.m_pContainers[nSection].m_nNumContainers; j++)
		{
			// If the object is definitely a container... check the container code
			if(m_pInterface->GetContainerCode(m_Vars.m_pContainers[nSection].m_hContainers[j], &nCode))
			{
				if(nCode == nContainerCode)
					return LTTRUE;
			}
		}
	}

	// Otherwise, just return false, cause we didn't find anything
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::IsInLiquidContainer()
//
// PURPOSE:		Checks to see if a certain character section is within a
//				liquid type of container
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::IsInLiquidContainer(int nSection) const
{
	D_WORD nCode = 0;
	uint32 i, j;

	if(m_Vars.m_nForceContainerType > 0 && IsLiquid((ContainerCode)m_Vars.m_nForceContainerType))
		return LTTRUE;

	// Check against all the character sections if needed
	if(nSection == CM_CONTAINER_ALL)
	{
		for(i = 0; i < CM_NUM_CONTAINER_SECTIONS; i++)
		{
			// Loop through all the valid containers for this section
			for(j = 0; j < m_Vars.m_pContainers[i].m_nNumContainers; j++)
			{
				// If the object is definitely a container... check the container code
				if(m_pInterface->GetContainerCode(m_Vars.m_pContainers[i].m_hContainers[j], &nCode))
				{
					if(IsLiquid((ContainerCode)nCode))
						return LTTRUE;
				}
			}
		}
	}
	else
	{
		// If the section passed in is without of range... return false
		if((nSection < 0) || (nSection >= CM_NUM_CONTAINER_SECTIONS))
			return LTFALSE;

		// Loop through all the valid containers for this section
		for(j = 0; j < m_Vars.m_pContainers[nSection].m_nNumContainers; j++)
		{
			// If the object is definitely a container... check the container code
			if(m_pInterface->GetContainerCode(m_Vars.m_pContainers[nSection].m_hContainers[j], &nCode))
			{
				if(IsLiquid((ContainerCode)nCode))
					return LTTRUE;
			}
		}
	}

	// Otherwise, just return false, cause we didn't find anything
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::OnObjectMove()
//
// PURPOSE:		Handle object side objects moving
//
// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD

DRESULT CharacterMovement::OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector &vPos)
{
	if(!m_Vars.m_hObject || !m_pInterface) return LT_OK;

	HOBJECT hClientObj = m_pInterface->GetClientObject();


	// Make sure the client object is non-solid on our local client
	if(hObj == hClientObj)
	{
		uint32 dwFlags;
		g_pLTClient->Common()->GetObjectFlags(hClientObj, OFT_Flags, dwFlags);

		dwFlags &= ~FLAG_SOLID;
		dwFlags |= FLAG_CLIENTNONSOLID;

		g_pLTClient->Common()->SetObjectFlags(hClientObj, OFT_Flags, dwFlags);
	}


	// If it's a solid world model moving, carry/push the player object around.
	if(!bTeleport && (hObj != hClientObj) && (hObj != m_Vars.m_hObject))
	{
		uint32 nType = m_pInterface->GetObjectType(hObj);
		uint32 dwFlags;

		m_pCommon->GetObjectFlags(hObj, OFT_Flags, dwFlags);

		if((nType == OT_WORLDMODEL) && (dwFlags & FLAG_SOLID))
			m_pPhysics->MovePushObjects(hObj, vPos, &m_Vars.m_hObject, 1);
	}

	return LT_OK;
}

#endif

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::OnObjectRotate()
//
// PURPOSE:		Handle object side objects rotating
//
// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD

DRESULT CharacterMovement::OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation &rRot)
{
	if(!m_Vars.m_hObject || !m_pInterface) return LT_OK;

	HOBJECT hClientObj = m_pInterface->GetClientObject();

	// If it's a solid world model moving, carry/push the player object around.
	if(!bTeleport && hObj != hClientObj && hObj != m_Vars.m_hObject)
	{
		uint32 nType = m_pInterface->GetObjectType(hObj);
		uint32 dwFlags;

		m_pCommon->GetObjectFlags(hObj, OFT_Flags, dwFlags);

		if((nType == OT_WORLDMODEL) && (dwFlags & FLAG_SOLID))
			m_pPhysics->RotatePushObjects(hObj, rRot, &m_Vars.m_hObject, 1);
	}

	return LT_OK;
}

#endif

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::OnTouchNotify()
//
// PURPOSE:		Handle when this object recieves a touch notfiy message from
//				the engine
//
// ----------------------------------------------------------------------- //

DRESULT CharacterMovement::OnTouchNotify(CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	// Call the current character movement state
	if(m_fpTouchNotify[m_cmsCurrent])
		return (*m_fpTouchNotify[m_cmsCurrent])(this, pInfo, fForceMag);

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::AllowMovement()
//
// PURPOSE:		Increment or decrement the allow movement value
//
// ----------------------------------------------------------------------- //

void CharacterMovement::AllowMovement(LTBOOL bCanMove)
{
	if(bCanMove)
	{
		if(m_Vars.m_nAllowMovement > 0)
			m_Vars.m_nAllowMovement--;
	}
	else
	{
		if(m_Vars.m_nAllowMovement < 255)
			m_Vars.m_nAllowMovement++;
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::AllowRotation()
//
// PURPOSE:		Increment or decrement the allow rotation value
//
// ----------------------------------------------------------------------- //

void CharacterMovement::AllowRotation(LTBOOL bCanRotate)
{
	if(bCanRotate)
	{
		if(m_Vars.m_nAllowRotation > 0)
			m_Vars.m_nAllowRotation--;
	}
	else
	{
		if(m_Vars.m_nAllowRotation < 255)
			m_Vars.m_nAllowRotation++;
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetObjectDims()
//
// PURPOSE:		Sets the dimensions of the character object
//
// ----------------------------------------------------------------------- //

LTVector CharacterMovement::SetObjectDims(uint8 nType, HOBJECT hObj, LTBOOL bForceDims)
{
	LTVector vDestDims = GetDimsSetting(nType);
	return SetObjectDims( vDestDims, hObj, bForceDims );
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetObjectDims()
//
// PURPOSE:		Sets the dimensions of the character object
//
// ----------------------------------------------------------------------- //

LTVector CharacterMovement::SetObjectDims(LTVector &vDims, HOBJECT hObj, LTBOOL bForceDims)
{
	HOBJECT hObject = hObj ? hObj : m_Vars.m_hObject;
	if(!hObject) return LTVector(0.0f, 0.0f, 0.0f);

	LTVector vCurrentDims;
	m_pPhysics->GetObjectDims( hObject, &vCurrentDims );
	LTVector vDestDims = vDims * m_Vars.m_fScale;

	// If the dims are already set to this... get out of here
	if(vDestDims.Equals(vCurrentDims, 0.1f))
		return vDestDims;

	// Setup the flags we should use when setting the dims
	uint32 dwFlags = 0;

	if(!bForceDims && ((vDestDims.x > vCurrentDims.x) || (vDestDims.y > vCurrentDims.y) || (vDestDims.z > vCurrentDims.z)))
		dwFlags = SETDIMS_PUSHOBJECTS;

	// Set the dims
	m_pPhysics->SetObjectDims(hObject, &vDestDims, dwFlags);

#ifndef _CLIENTBUILD
	// If they were standing on something, use move object to floor 
	// to make sure they are still standing on something.
	if( m_Vars.m_bStandingOn && !IsPlayer( hObject ))
		MoveObjectToFloor( hObject, LTFALSE, LTFALSE );
#endif

#ifdef CHARACTER_MOVEMENT_DEBUG
	#ifdef _CLIENTBUILD
		m_pInterface->CPrint("%.2f CharMov : Object(%s) SetDims (%f %f %f)", m_pInterface->GetTime(), "Client Physics Object", vDestDims.x, vDestDims.y, vDestDims.z);
	#else
		if(bDebugPlayer || _stricmp(m_pInterface->GetObjectName(hObject), "Player"))
			m_pInterface->CPrint("%.2f CharMov : Object(%s) SetDims (%f %f %f)", m_pInterface->GetTime(), m_pInterface->GetObjectName(hObject), vDestDims.x, vDestDims.y, vDestDims.z);
	#endif
#endif

	return vDestDims;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::CheckDimsSpace()
//
// PURPOSE:		Checks room around this character for the specifed dims size
//
// ----------------------------------------------------------------------- //

LTBOOL CharacterMovement::CheckDimsSpace(LTVector vDims)
{
	// Check our needed interfaces
	if(!m_pInterface || !m_pPhysics)
		return LTFALSE;


	// Use the dims object as the preferred method...
	if(m_hDimsObject)
	{
		LTVector vPos;
		m_pInterface->GetObjectPos(m_Vars.m_hObject, &vPos);

		m_pPhysics->MoveObject(m_hDimsObject, &vPos, MOVEOBJECT_TELEPORT);

		// Set our dims object to the same starting dims as the real object.
		// This makes sure that the dims growing has the same behavior.  Dims
		// growing first grows the along the y axis, then along the x, then the z.
		// If we don't start with the same dims, we may not push ourselves the same way.
		LTVector vOriginalDims;
		m_pPhysics->GetObjectDims( m_Vars.m_hObject, &vOriginalDims );
		SetObjectDims( vOriginalDims, m_hDimsObject );

		SetObjectFlags(OFT_Flags, m_Vars.m_dwFlags & ~FLAG_SOLID, LTFALSE);
		m_pInterface->Common()->SetObjectFlags(m_hDimsObject, OFT_Flags, FLAG_SOLID);

		LTVector vSizedDims = SetObjectDims(vDims, m_hDimsObject);

		m_pInterface->Common()->SetObjectFlags(m_hDimsObject, OFT_Flags, 0);
		SetObjectFlags(OFT_Flags, m_Vars.m_dwFlags, LTFALSE);

		if(vSizedDims.Equals(vDims, 0.1f))
			return LTTRUE;
	}
	// Set our own object dims as a standby... (not preferred)
	else
	{
		LTVector vOrigDims = GetObjectDims();
		LTVector vSizedDims = SetObjectDims(CM_DIMS_DEFAULT);

		if(vSizedDims.Equals(vDims, 0.1f))
			return LTTRUE;

		SetObjectDims(vOrigDims);
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetScale()
//
// PURPOSE:		Sets the scale of the character object
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetScale(LTFLOAT fScale)
{
	LTVector vScale(fScale, fScale, fScale);
	m_Vars.m_fScale = fScale;

#ifdef _CLIENTBUILD
	m_pInterface->SetObjectScale(m_Vars.m_hObject, &vScale);
#else
	m_pInterface->ScaleObject(m_Vars.m_hObject, &vScale);
#endif

	// Reset the dims so the scale will take affect
	LTVector vDims = GetObjectDims();
	SetObjectDims(vDims);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetControlFlags()
//
// PURPOSE:		Sets the control flags
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetControlFlags(uint32 dwFlags)
{
	// Check for opposing flags and cancel them out...
	if((dwFlags & CM_FLAG_FORWARD) && (dwFlags & CM_FLAG_BACKWARD))
		dwFlags &= ~(CM_FLAG_FORWARD | CM_FLAG_BACKWARD);

	if((dwFlags & CM_FLAG_LEFT) && (dwFlags & CM_FLAG_RIGHT))
		dwFlags &= ~(CM_FLAG_LEFT | CM_FLAG_RIGHT);

	if((dwFlags & CM_FLAG_STRAFELEFT) && (dwFlags & CM_FLAG_STRAFERIGHT))
		dwFlags &= ~(CM_FLAG_STRAFELEFT | CM_FLAG_STRAFERIGHT);

//	if((dwFlags & CM_FLAG_DUCK) && (dwFlags & CM_FLAG_JUMP))
//		dwFlags &= ~(CM_FLAG_DUCK | CM_FLAG_JUMP);

	if((dwFlags & CM_FLAG_LOOKUP) && (dwFlags & CM_FLAG_LOOKDOWN))
		dwFlags &= ~(CM_FLAG_LOOKUP | CM_FLAG_LOOKDOWN);

	if((dwFlags & CM_FLAG_PRIMEFIRING) && (dwFlags & CM_FLAG_ALTFIRING))
		dwFlags &= ~(CM_FLAG_ALTFIRING);

	if(!m_Butes.m_bWallWalk)
		dwFlags &= ~CM_FLAG_WALLWALK;

	if(m_Vars.m_bEncumbered)
	{
		dwFlags &= ~CM_FLAG_RUN;

		// Alien AI's with missing legs will set their
		// encumbered state so that they won't move too
		// fast or wall walk.
		dwFlags &= ~CM_FLAG_WALLWALK;
	}


	// Set the control flags equal to this new value
	m_Vars.m_dwCtrl = dwFlags;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetObjectFlags(), SetObjectUserFlags(), SetObjectClientFlags()
//
// PURPOSE:		Sets the flags of the character object
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetObjectFlags(ObjFlagType nType, uint32 dwFlags, LTBOOL bSave)
{
	if(!m_Vars.m_hObject) return;
	m_pCommon->SetObjectFlags(m_Vars.m_hObject, nType, dwFlags);

	if(bSave)
	{
		if(nType == OFT_Flags2)
			m_Vars.m_dwFlags2 = dwFlags;
		else
			m_Vars.m_dwFlags = dwFlags;
	}
}

// ----------------------------------------------------------------------- //

void CharacterMovement::SetObjectUserFlags(uint32 dwFlags, LTBOOL bSave)
{
	if(!m_Vars.m_hObject || !m_pInterface) return;
	m_pInterface->SetObjectUserFlags(m_Vars.m_hObject, dwFlags);

	if(bSave)
		m_Vars.m_dwUserFlags = dwFlags;
}

// ----------------------------------------------------------------------- //

 void CharacterMovement::SetObjectClientFlags(uint32 dwFlags, LTBOOL bSave)
{
#ifdef _CLIENTBUILD
	if(!m_Vars.m_hObject || !m_pInterface) return;
	m_pInterface->SetObjectClientFlags(m_Vars.m_hObject, dwFlags);

	if(bSave)
		m_Vars.m_dwClientFlags = dwFlags;
#endif
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetCharacterButes()
//
// PURPOSE:		Find the correct character name, and then set the attributes
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetCharacterButes(const char *szName, LTBOOL bUseMPDir)
{
	if(!m_pInterface) return;

	// Find out which character we're looking for
	for(int i = 0; i < g_pCharacterButeMgr->GetNumSets(); i++)
	{
		if(!stricmp(szName, g_pCharacterButeMgr->GetModelType(i)))
		{
			// If we found the character, set the butes and exit
			SetCharacterButes(i, bUseMPDir);
			return;
		}
	}

	// Otherwise, leave it the same and print out an error
	m_pInterface->CPrint("ERROR!! CharacterMovement::SetCharacterButes() --- Could not locate the character set: %s", szName);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetCharacterButes()
//
// PURPOSE:		Set the attributes from an index
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetCharacterButes(int nType, LTBOOL bUseMPDir)
{
	if(!m_pInterface) return;

	// Make sure the type is within range
	if((nType < 0) || (nType >= g_pCharacterButeMgr->GetNumSets()))
	{
		m_pInterface->CPrint("ERROR!! CharacterMovement::SetCharacterButes() --- Type %d is not within range from zero to %d", nType, g_pCharacterButeMgr->GetNumSets() - 1);
		return;
	}

	// Fill in all the attributes here
	m_Butes = g_pCharacterButeMgr->GetCharacterButes(nType);

	if(m_Vars.m_hObject)
	{
		// Set the filenames of the model
		ObjectCreateStruct theStruct;
		theStruct.Clear();

		m_Butes.FillInObjectCreateStruct(theStruct, bUseMPDir);
		m_pInterface->Common()->SetObjectFilenames(m_Vars.m_hObject, &theStruct);

		LTBOOL bSinglePlayerGame = LTTRUE;

		// check to see if we are in sp or mp
#ifdef _CLIENTBUILD
		if(g_pGameClientShell->GetGameType()->IsMultiplayer())
		{
			bSinglePlayerGame = LTFALSE;
		}
#else
		if(g_pGameServerShell->GetGameType() != SP_STORY)
		{
			bSinglePlayerGame = LTFALSE;
		}
#endif

		// Remove any models that may be left around as the result of changing files.
		if(bSinglePlayerGame)
			m_pInterface->FreeUnusedModels( );

		// make sure we are not wall walking
		if(!bSinglePlayerGame)
			SetObjectFlags(OFT_Flags2, m_Vars.m_dwFlags2 & ~FLAG2_ORIENTMOVEMENT & ~FLAG2_SPHEREPHYSICS, LTFALSE);

		// Set the default dimensions
		if(!bSinglePlayerGame)
			SetObjectDims(CM_DIMS_DEFAULT, LTNULL, LTFALSE);
		else
			SetObjectDims(CM_DIMS_DEFAULT, LTNULL, LTTRUE);
	}
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetMovementState()
//
// PURPOSE:		Switch to another movement state and save the old one
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetMovementState(CharacterMovementState cms)
{
	// Check and make sure this state has a function to handle it
	if(m_fpHandleState[cms])
	{
		// Save the previous state and set the current one
		m_cmsLast = m_cmsCurrent;
		m_cmsCurrent = cms;

#ifdef CHARACTER_MOVEMENT_DEBUG
	#ifdef _CLIENTBUILD
		m_pInterface->CPrint("%.2f CharMov : Object(%s) Character state changing from %s to %s!!", m_pInterface->GetTime(), "Client Physics Object", CMSNames[m_cmsLast], CMSNames[m_cmsCurrent]);
	#else
		if(bDebugPlayer || _stricmp(m_pInterface->GetObjectName(m_Vars.m_hObject), "Player"))
			m_pInterface->CPrint("%.2f CharMov : Object(%s) Character state changing from %s to %s!!", m_pInterface->GetTime(), m_pInterface->GetObjectName(m_Vars.m_hObject), CMSNames[m_cmsLast], CMSNames[m_cmsCurrent]);
	#endif
#endif

		// Reset the restraints to use the bute file values
		m_Vars.m_bIgnorAimRestraints = LTFALSE;
		return;
	}

	if(m_pInterface)
		// Otherwise, print out an error message cause we can't change the state
		m_pInterface->CPrint("CharacterMovement Error!! SetMovementState function is invalid!!");
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	Position and rotation functions
//
// PURPOSE:		Move and rotate the character
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetPos(const LTVector &vPos, uint32 dwFlags)
{
	if(!m_Vars.m_hObject || !m_pInterface) return;

	m_pPhysics->MoveObject(m_Vars.m_hObject, const_cast<LTVector*>(&vPos), dwFlags);
	m_pInterface->GetObjectPos(m_Vars.m_hObject, &m_Vars.m_vPos);


	// If the object is oriented by movement, we need to update the orientatoin info
	// after we move the object!
	if(  GetObjectFlags(OFT_Flags2) & FLAG2_ORIENTMOVEMENT )
	{
		m_pInterface->GetObjectRotation(m_Vars.m_hObject, &m_Vars.m_rRot);
		m_pMath->GetRotationVectors(m_Vars.m_rRot, m_Vars.m_vRight, m_Vars.m_vUp, m_Vars.m_vForward);
	}
}

// ----------------------------------------------------------------------- //

void CharacterMovement::SetRot(const LTVector &vRot)
{
	if(!m_Vars.m_hObject) return;
	LTRotation rRot;
	m_pMath->SetupEuler(rRot, vRot.x, vRot.y, vRot.z);
	SetRot(rRot);
}

// ----------------------------------------------------------------------- //

void CharacterMovement::SetRot(const LTRotation &rRot)
{
	if(!m_Vars.m_hObject || !m_pInterface) return;
#ifdef _CLIENTBUILD
	m_pInterface->SetObjectRotation(m_Vars.m_hObject, const_cast<LTRotation*>(&rRot));
#else
	m_pInterface->RotateObject(m_Vars.m_hObject, const_cast<LTRotation*>(&rRot));
#endif
	m_pInterface->GetObjectRotation(m_Vars.m_hObject, &m_Vars.m_rRot);
	m_pMath->GetRotationVectors(m_Vars.m_rRot, m_Vars.m_vRight, m_Vars.m_vUp, m_Vars.m_vForward);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	Velocity and acceleration functions
//
// PURPOSE:		
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetVelocity(const LTVector &vVel, LTBOOL bSaveOnly)
{
	if(!m_Vars.m_hObject) return;

	if(!bSaveOnly)
		m_pPhysics->SetVelocity(m_Vars.m_hObject, const_cast<LTVector*>(&vVel));

	m_Vars.m_vVelocity = vVel;
}

// ----------------------------------------------------------------------- //

void CharacterMovement::SetAcceleration(const LTVector &vAccel, LTBOOL bSaveOnly)
{
	if(!m_Vars.m_hObject) return;

	if(!bSaveOnly)
		m_pPhysics->SetAcceleration(m_Vars.m_hObject, const_cast<LTVector*>(&vAccel));

	m_Vars.m_vAccel = vAccel;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetStandingOn()
//
// PURPOSE:		Set whether the character is standing on an HPOLY
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetStandingOn(HPOLY hPoly)
{
	if(hPoly == INVALID_HPOLY)
	{
		m_Vars.m_bStandingOn = LTFALSE;
		m_Vars.m_ciStandingOn.m_hPoly = INVALID_HPOLY;
	}
	else
	{
		m_Vars.m_bStandingOn = LTTRUE;
		m_Vars.m_ciStandingOn.m_hPoly = hPoly;
	}
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetDimsSetting()
//
// PURPOSE:		Retrieves a potential dims setting
//
// ----------------------------------------------------------------------- //

LTVector CharacterMovement::GetDimsSetting(uint8 nType)
{
	if( !m_pInterface )
		return LTVector(0,0,0);

	LTVector vDims;
	HMODELANIM hAnim = INVALID_MODEL_ANIM;

	// Figure out the size of the dims to try and switch to...
	switch(nType)
	{
		case CM_DIMS_CROUCHED:
		{
			hAnim = m_pInterface->GetAnimIndex(m_Vars.m_hObject, "Dims_Crouch");

			vDims = m_Butes.m_vDims;
			vDims.y /= 2.0f;
			break;
		}

		case CM_DIMS_WALLWALK:
		{
			hAnim = m_pInterface->GetAnimIndex(m_Vars.m_hObject, "Dims_Crouch");

			LTFLOAT fBoxDims = m_Butes.m_vDims.y / 2.0f;
			vDims.Init(fBoxDims, fBoxDims, fBoxDims);
			break;
		}

		case CM_DIMS_VERY_SMALL:
		{
			vDims.Init(1.0f, 1.0f, 1.0f);
			break;
		}

		case CM_DIMS_POUNCING:
		{
			vDims = m_Butes.m_vPouncingDims;
			break;
		}

		case CM_DIMS_DEFAULT:
		default:
		{
			if( hAnim == INVALID_MODEL_ANIM )
				hAnim = m_pInterface->GetAnimIndex(m_Vars.m_hObject, "Dims_Default");

			vDims = m_Butes.m_vDims;
			break;
		}
	}

	// Record if we are crouching or not.
	m_Vars.m_bIsCrouched = ( nType == CM_DIMS_CROUCHED || nType == CM_DIMS_WALLWALK );

	// Check to see if we should use the animation dims instead
	if(m_Vars.m_hObject && (hAnim != INVALID_MODEL_ANIM))
		m_pCommon->GetModelAnimUserDims(m_Vars.m_hObject, &vDims, hAnim);

	// If we're using wall walking dims... make them square
	if(nType == CM_DIMS_WALLWALK)
	{
		vDims.x = vDims.y;
		vDims.z = vDims.y;
	}

	return (vDims * m_Vars.m_fScale);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetObjectDims()
//
// PURPOSE:		Gets the dimensions of the character object
//
// ----------------------------------------------------------------------- //

LTVector CharacterMovement::GetObjectDims(HOBJECT hObj, LTBOOL bFull) const
{
	HOBJECT hObject = hObj ? hObj : m_Vars.m_hObject;
	LTVector vDims(0.0f, 0.0f, 0.0f);

	// Make sure the object is valid
	if(hObject)
	{
		m_pPhysics->GetObjectDims(hObject, &vDims);

		// If we want the full dims, double it...
		if(bFull) vDims *= 2.0f;
	}

	return vDims;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetScale()
//
// PURPOSE:		Gets the scale of the character object
//
// ----------------------------------------------------------------------- //

LTFLOAT CharacterMovement::GetScale() const
{
	if( !m_pInterface )
		return 1.0f;

	LTVector vScale;
	m_pInterface->GetObjectScale(m_Vars.m_hObject, &vScale);

	return vScale.x;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetObjectFlags(), GetObjectUserFlags(), GetObjectClientFlags()
//
// PURPOSE:		Sets the flags of the character object
//
// ----------------------------------------------------------------------- //

uint32 CharacterMovement::GetObjectFlags(ObjFlagType nType) const
{
	if(!m_Vars.m_hObject) return 0;
	uint32 dwFlags = 0;
	m_pCommon->GetObjectFlags(m_Vars.m_hObject, nType, dwFlags);
	return dwFlags;
}

// ----------------------------------------------------------------------- //

uint32 CharacterMovement::GetObjectUserFlags() const
{
	if(!m_Vars.m_hObject || !m_pInterface) return 0;

#ifdef _CLIENTBUILD
	uint32 dwFlags = 0;
	m_pInterface->GetObjectUserFlags(m_Vars.m_hObject, &dwFlags);
	return dwFlags;
#else
	return m_pInterface->GetObjectUserFlags(m_Vars.m_hObject);
#endif
}

// ----------------------------------------------------------------------- //

uint32 CharacterMovement::GetObjectClientFlags() const
{
#ifdef _CLIENTBUILD
	if(!m_Vars.m_hObject) return 0;
	return m_pInterface->GetObjectClientFlags(m_Vars.m_hObject);
#endif

	return 0;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetMovementStateName()
//
// PURPOSE:		Gets the string name of a movement state
//
// ----------------------------------------------------------------------- //

char* CharacterMovement::GetMovementStateName(CharacterMovementState cms) const
{
	if((cms < CMS_MIN) || (cms >= CMS_MAX))
		return "None";

	return CMSNames[cms];
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetRot()
//
// PURPOSE:		Gets a vector of the character rotation angles (from -MATH_PI to MATH_PI)
//
// ----------------------------------------------------------------------- //

void CharacterMovement::GetRot(LTVector &vRot) const
{
	m_pMath->GetEulerAngles(const_cast<LTRotation&>(m_Vars.m_rRot), vRot);
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetMaxSpeed()
//
// PURPOSE:		Gets the maximum speed for the requested state.
//
// ----------------------------------------------------------------------- //

LTFLOAT CharacterMovement::GetMaxSpeed(CharacterMovementState cms) const
{
	if((cms < CMS_MIN) || (cms >= CMS_MAX))
	{
		m_pInterface->CPrint("ERROR!! CharacterMovement::GetMaxSpeed() --- Movement state was out of range: %d", cms);
		return 0.0f;
	}

	// Call the handler if it has been set.
	if(m_fpMaxSpeed[cms])
		return (*m_fpMaxSpeed[cms])(this);

	// There is no handler.  Just return 0.
	return 0.0f;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::GetMaxAccel()
//
// PURPOSE:		Gets the maximum Accel for the requested state.
//
// ----------------------------------------------------------------------- //

LTFLOAT CharacterMovement::GetMaxAccel(CharacterMovementState cms) const
{
	if((cms < CMS_MIN) || (cms >= CMS_MAX))
	{
		m_pInterface->CPrint("ERROR!! CharacterMovement::GetMaxAccel() --- Movement state was out of range: %d", cms);
		return 0.0f;
	}

	// Call the handler if it has been set.
	if(m_fpMaxAccel[cms])
		return (*m_fpMaxAccel[cms])(this);

	// There is no handler.  Just return 0.
	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetHandleStateFunc()
//
// PURPOSE:		Override the standard update function for a particular state
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetHandleStateFunc(CharacterMovementState cms, void (*fp)(CharacterMovement*))
{
	if(!m_pInterface || !fp) return;

	// Make sure the state is within the range
	if((cms < CMS_MIN) || (cms >= CMS_MAX))
	{
		m_pInterface->CPrint("ERROR!! CharacterMovement::SetHandleStateFunc() --- Movement state was out of range: %d", cms);
		return;
	}

	// Set this state to our new function
	m_fpHandleState[cms] = fp;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetHandleStateFunc()
//
// PURPOSE:		Override the standard update function for a particular state
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetTouchNotifyFunc(CharacterMovementState cms, LTRESULT (*fp)(CharacterMovement*, CollisionInfo *pInfo, LTFLOAT fForceMag))
{
	if(!m_pInterface || !fp) return;

	// Make sure the state is within the range
	if((cms < CMS_MIN) || (cms >= CMS_MAX))
	{
		m_pInterface->CPrint("ERROR!! CharacterMovement::SetTouchNotifyFunc() --- Movement state was out of range: %d", cms);
		return;
	}

	// Set this state to our new function
	m_fpTouchNotify[cms] = fp;
}


// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetMaxSpeedFunc()
//
// PURPOSE:		Override the standard max speed function for a particular state
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetMaxSpeedFunc(CharacterMovementState cms, LTFLOAT (*fp)(const CharacterMovement*))
{
	if(!m_pInterface || !fp) return;

	// Make sure the state is within the range
	if((cms < CMS_MIN) || (cms >= CMS_MAX))
	{
		m_pInterface->CPrint("ERROR!! CharacterMovement::SetMaxSpeedFunc() --- Movement state was out of range: %d", cms);
		return;
	}

	// Set this state to our new function
	m_fpMaxSpeed[cms] = fp;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetMaxAccelFunc()
//
// PURPOSE:		Override the standard max speed function for a particular state
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetMaxAccelFunc(CharacterMovementState cms, LTFLOAT (*fp)(const CharacterMovement*))
{
	if(!m_pInterface || !fp) return;

	// Make sure the state is within the range
	if((cms < CMS_MIN) || (cms >= CMS_MAX))
	{
		m_pInterface->CPrint("ERROR!! CharacterMovement::SetMaxAccelFunc() --- Movement state was out of range: %d", cms);
		return;
	}

	// Set this state to our new function
	m_fpMaxAccel[cms] = fp;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetHandleMessageReadFunc(), SetHandleMessageWriteFunc()
//
// PURPOSE:		Override the standard message functions
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetHandleMessageReadFunc(LTBOOL (*fp)(uint8, HMESSAGEREAD, uint32))
{
	if(!m_pInterface || !fp) return;

	// Set the message handling function
	m_fpHandleMessageRead = fp;
}

// ----------------------------------------------------------------------- //

void CharacterMovement::SetHandleMessageWriteFunc(LTBOOL (*fp)(uint8, HMESSAGEWRITE, uint32))
{
	if(!m_pInterface || !fp) return;

	// Set the message handling function
	m_fpHandleMessageWrite = fp;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::SetContainerPhysicsInfoFunc()
//
// PURPOSE:		Set a container physics info function
//
// ----------------------------------------------------------------------- //

void CharacterMovement::SetContainerPhysicsInfoFunc(LTBOOL (*fp)(HOBJECT, CMContainerInfo&))
{
	if(!m_pInterface || !fp) return;

	// Set the function pointer
	m_fpHandleContainerPhysicsInfo = fp;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateStandingOn()
//
// PURPOSE:		Handles setting the standing on variables
//
// ----------------------------------------------------------------------- //

// This filters on FLAG_SOLID.  The IGNORE_NONSOLID flag
// filters on (FLAG_SOLID | FLAG_RAYHIT).
LTBOOL FilterNonSolid(HOBJECT hObj, void * /*un-used*/)
{
	if( hObj )
	{
		uint32 flags;
		if( LT_OK == g_pInterface->Common()->GetObjectFlags(hObj, OFT_Flags, flags) )
		{
			if( !(flags & FLAG_SOLID) )
			{
				return LTFALSE;
			}
		}
	}

	return LTTRUE;
}

void CharacterMovement::UpdateStandingOn()
{
	// Save whether we were standing on something last frame.
	m_Vars.m_bLastStandingOn = m_Vars.m_bStandingOn;

	if( CMS_WALLWALKING != m_cmsCurrent )
	{
		// Get the current velocity and object standing on information
		m_pPhysics->GetStandingOn(m_Vars.m_hObject, &m_Vars.m_ciStandingOn);
	}
	else
	{
		// Reset our collision info.
		m_Vars.m_ciStandingOn.m_hObject = LTNULL;
		m_Vars.m_ciStandingOn.m_hPoly = INVALID_HPOLY;
		m_Vars.m_ciStandingOn.m_Plane = LTPlane(0,0,0,0);
		m_Vars.m_ciStandingOn.m_vStopVel = LTVector(0,0,0);

		// In wall walking mode, we need to drop a line
		// intersect and hope we hit the poly under our feet.
		// If we don't, we need to just fall.

		LTVector vDims;
		m_pPhysics->GetObjectDims(m_Vars.m_hObject, &vDims);

		IntersectQuery IQuery;
		IntersectInfo IInfo;


		LTFLOAT fDetectDist = 30.0f;

		if( !vtWWDetectDist.IsInitted() )
			vtWWDetectDist.Init(m_pInterface, "WWDetectDist", LTNULL, fDetectDist);

		fDetectDist = vtWWDetectDist.GetFloat();

		IQuery.m_From = m_Vars.m_vPos;
		IQuery.m_To = m_Vars.m_vPos - m_Vars.m_vUp * (vDims.y + fDetectDist);

		IQuery.m_Flags	   = IGNORE_NONSOLID | INTERSECT_HPOLY | INTERSECT_OBJECTS;

		IQuery.m_FilterFn  = FilterNonSolid;
		IQuery.m_pUserData = NULL;

		if ( m_pInterface->IntersectSegment(&IQuery, &IInfo) )
		{
			if( IInfo.m_hPoly != INVALID_HPOLY )
			{
				// Grab the surface flags of this poly
				uint32 nSurfFlags;
				m_pInterface->Common()->GetPolySurfaceFlags(IInfo.m_hPoly, nSurfFlags);

				if(!(nSurfFlags & SURF_NOWALLWALK))
				{
					m_Vars.m_ciStandingOn.m_hObject = IInfo.m_hObject;
					m_Vars.m_ciStandingOn.m_hPoly = IInfo.m_hPoly;
					m_Vars.m_ciStandingOn.m_Plane = IInfo.m_Plane;
					m_Vars.m_ciStandingOn.m_vStopVel = LTVector(0,0,0);
				}
			}
		}
	}


	// Set our standing on boolean value
	if(m_Vars.m_ciStandingOn.m_hObject)
	{
		m_Vars.m_bStandingOn = LTTRUE;
		m_Vars.m_fStandingOnCheckTime = 0.0f;
	}
	else
	{
		if(m_Vars.m_bStandingOn)
		{
			LTFLOAT fCheckTime = 0.2f;

			// Check our time to see if what we were standing on is really not there anymore
			m_Vars.m_fStandingOnCheckTime += m_Vars.m_fFrameTime;

#ifdef _CLIENTBUILD
			// Adjust the check time according to our console variable
			HCONSOLEVAR hVar;
			if(hVar = g_pLTClient->GetConsoleVar("StandingOnCheckTime"))
				fCheckTime = g_pLTClient->GetVarValueFloat(hVar);
#endif

			// If the saved check time is greater than our max check time, set the boolean
			if(m_Vars.m_fStandingOnCheckTime >= fCheckTime)
			{
				m_Vars.m_bStandingOn = LTFALSE;
			}
		}
	}

#ifdef CHARACTER_MOVEMENT_DEBUG
	if( m_Vars.m_bLastStandingOn != m_Vars.m_bStandingOn)
	{
	#ifdef _CLIENTBUILD
		m_pInterface->CPrint("%.2f CharMov :  Object(%s) StandingOn = %s  Poly = %d", m_pInterface->GetTime(), "Client Physics Object", m_Vars.m_bStandingOn ? "TRUE" : "FALSE", m_Vars.m_ciStandingOn.m_hPoly);
	#else
		if(bDebugPlayer || _stricmp(m_pInterface->GetObjectName(m_Vars.m_hObject), "Player"))
			m_pInterface->CPrint("%.2f CharMov :  Object(%s) StandingOn = %s  Poly = %d", m_pInterface->GetTime(), m_pInterface->GetObjectName(m_Vars.m_hObject), m_Vars.m_bStandingOn ? "TRUE" : "FALSE", m_Vars.m_ciStandingOn.m_hPoly);
	#endif
	}
#endif
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTBOOL IsRealignState(CharacterMovementState cms)
{
	switch(cms)
	{
		case (CMS_PRE_POUNCE_JUMP):
		case (CMS_POUNCE_JUMP):
		case (CMS_WWALK_JUMP):
		case (CMS_POUNCE_FALL):
		case (CMS_WWALK_FALL):
		case (CMS_WWALK_LAND):
		case (CMS_WALLWALKING):
		case (CMS_POUNCING):
		case (CMS_FACEHUG):
		case (CMS_FACEHUG_ATTACH):
		case (CMS_PRE_JUMPING):
		case (CMS_PRE_FALLING):
		case (CMS_PRE_CRAWLING):
		case (CMS_POST_CRAWLING):
		case (CMS_OBSERVING):
			return LTFALSE;
	}
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateRotation()
//
// PURPOSE:		Handles rotating the character
//
// ----------------------------------------------------------------------- //

void CharacterMovement::UpdateRotation()
{
	m_Vars.m_fTurnRate = (m_Vars.m_dwCtrl & CM_FLAG_RUN) ? m_Butes.m_fRunningTurnRate : m_Butes.m_fWalkingTurnRate;

#ifdef _CLIENTBUILD
	CUserProfile *pProf = g_pProfileMgr->GetCurrentProfile();
	if(pProf)
	{
		int nRate = (m_Vars.m_dwCtrl & CM_FLAG_RUN) ? pProf->m_nFastTurn : pProf->m_nNormalTurn;
		m_Vars.m_fTurnRate *= (LTFLOAT)nRate/23;
	}
#endif

	if(m_Vars.m_dwCtrl & (CM_FLAG_TURNING|CM_KICK_BACK))
	{
		// TODO: PUT THE ROTATION AMOUNT IN THE ATTRIBUTE FILE
		// TODO: MIGHT NEED TO MAKE AN UPDATE ROTATION ANGLE INSTEAD OF ROTATING FROM THE CURRENT ROTATION

		// Calculate the amount to turn for this frame / update
		LTFLOAT fTurnAmount = 0.0f;

		if(m_Vars.m_dwCtrl & CM_FLAG_LEFT)		fTurnAmount -= (m_Vars.m_fTurnRate * MATH_PI / 180.0f) * m_Vars.m_fFrameTime;
		if(m_Vars.m_dwCtrl & CM_FLAG_RIGHT)		fTurnAmount += (m_Vars.m_fTurnRate * MATH_PI / 180.0f) * m_Vars.m_fFrameTime;

		// Get the rotation vectors of the character
		LTRotation rRot = m_Vars.m_rRot;

		// Rotate around the up axis of the character for side to side rotation
		m_pMath->RotateAroundAxis(rRot, m_Vars.m_vUp, fTurnAmount);
		SetRot(rRot);
	}

#ifdef _CLIENTBUILD
	if( IsRealignState(m_cmsCurrent) && m_Vars.m_vUp.y < 0.999f )
	{
		RealignCharacter(this);
	}
#endif
}

// ----------------------------------------------------------------------- //
//
// FUNCTION:	CharacterMovement::UpdateAiming()
//
// PURPOSE:		Handle the lookup and lookdown controls of the character
//
// ----------------------------------------------------------------------- //

void CharacterMovement::UpdateAiming()
{
	if(m_Vars.m_dwCtrl & CM_FLAG_AIMING)
	{
		// TODO: PUT THE AIM AMOUNT IN THE ATTRIBUTE FILE
		// TODO: MAKE AN UPDATED AIM ANGLE INSTEAD OF AIMING FROM THE CURRENT POSITION, THAT WAY IT CAN BE LIMITED

		// Calculate the amount to aiming for this frame / update
		LTFLOAT fAimAmount = 0.0f;
		m_Vars.m_fAimRate = (m_Vars.m_dwCtrl & CM_FLAG_RUN) ? m_Butes.m_fRunningAimRate : m_Butes.m_fWalkingAimRate;

#ifdef _CLIENTBUILD
		CUserProfile *pProf = g_pProfileMgr->GetCurrentProfile();
		if(pProf)
		{
			m_Vars.m_fAimRate *= (LTFLOAT)pProf->m_nLookUp/23;
		}
#endif

		if(m_Vars.m_dwCtrl & CM_FLAG_LOOKUP)	fAimAmount -= m_Vars.m_fAimRate * m_Vars.m_fFrameTime;
		if(m_Vars.m_dwCtrl & CM_FLAG_LOOKDOWN)	fAimAmount += m_Vars.m_fAimRate * m_Vars.m_fFrameTime;

		// Add this to the final aim rotation and cap the values
		m_Vars.m_fAimingPitch += fAimAmount;
	}


	// Handle weapon kick back
	if(m_Vars.m_dwCtrl & CM_KICK_BACK)
	{
		if(m_Vars.m_vVelocity.MagSqr())
		{
			//we are moving so lets bump things up a bit!

			// Calculate the amount to aiming for this frame / update
			m_Vars.m_fAimingPitch -= (m_Vars.m_dwCtrl & CM_FLAG_RUN) ? m_Butes.m_fRunningKickBack : m_Butes.m_fWalkingKickBack;
		}
	}


	// Cap off the aiming values to the appropriate angles
	if(m_Vars.m_bIgnorAimRestraints)
	{
		// If we're in water... let us aim completely up and down
		if(m_Vars.m_fAimingPitch > 90.0f)		m_Vars.m_fAimingPitch = 90.0f;
		if(m_Vars.m_fAimingPitch < -90.0f)		m_Vars.m_fAimingPitch = -90.0f;
	}
	else
	{
		if(m_Vars.m_fAimingPitch > m_Butes.m_ptAimRange.y)		m_Vars.m_fAimingPitch = (LTFLOAT)m_Butes.m_ptAimRange.y;
		if(m_Vars.m_fAimingPitch < m_Butes.m_ptAimRange.x)		m_Vars.m_fAimingPitch = (LTFLOAT)m_Butes.m_ptAimRange.x;
	}
}


//****************************************************************************************************************
//****************************************************************************************************************
//
// THE FOLLOWING FUNCTIONS ARE THE FUNCTIONALITY OF THE MOVEMENT STATES
//
//****************************************************************************************************************
//****************************************************************************************************************

// ----------------------------------------------------------------------- //
//
// FUNCTION:	Movement state handling functions
//
// PURPOSE:		Do the current movement state
//
// ----------------------------------------------------------------------- //

void CharacterMovementState_Idle(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();

	// Set the dims to the appropriate size
	LTVector vDimsResult = pMovement->SetObjectDims(CM_DIMS_DEFAULT);

	// Check if we reached our full height.  If not, then go to crawling state.
	LTVector vDimsSought = pMovement->GetDimsSetting(CM_DIMS_DEFAULT);
	if( vDimsResult.y < vDimsSought.y )
	{
		pMovement->SetMovementState(CMS_PRE_CRAWLING); 
		return;
	}

	// Return the flags to their original state
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);

	// Init the acceleration and velocity to zero
	pMovement->UpdateAcceleration(0.0f, 0);
//	pMovement->UpdateVelocity(0.0f, CM_DIR_FLAG_NONE);

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
		{ pMovement->SetMovementState(CMS_CLIMBING); return; }
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
		{ pMovement->SetMovementState(CMS_SWIMMING); return; }

	// If our gravity isn't on... then we should be in the flying state
	if(!(cvVars->m_dwFlags & FLAG_GRAVITY))
		{ pMovement->SetMovementState(CMS_FLYING); return; }

	// Check for jump state change: If our jump flag is on, we should jump
	if(cvVars->m_dwCtrl & CM_FLAG_JUMP && cvVars->m_bAllowJump)
		{ pMovement->SetMovementState(CMS_PRE_JUMPING);	return; }

	if((cvVars->m_dwCtrl & CM_FLAG_DUCK) || (cvVars->m_dwCtrl & CM_FLAG_WALLWALK))
		{ pMovement->SetMovementState(CMS_PRE_CRAWLING); return; }

	// If we're not standing on anything, then we're either falling, or jumping
	if(!cvVars->m_bStandingOn)
		{ pMovement->SetMovementState(CMS_PRE_FALLING); return; }

	// Check the control flags to see if we need to update the velocity
	if(cvVars->m_dwCtrl & CM_FLAG_DIRECTIONAL)
	{
		if(cvVars->m_dwCtrl & CM_FLAG_RUN)
			{ pMovement->SetMovementState(CMS_RUNNING); return; }
		else
			{ pMovement->SetMovementState(CMS_WALKING); return; }
	}
}

LTFLOAT CharacterMovementState_Idle_MaxSpeed(const CharacterMovement *)
{
	return 0.0f;
}

LTFLOAT CharacterMovementState_Idle_MaxAccel(const CharacterMovement *)
{
	return 0.0f;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Walking(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the appropriate size
	pMovement->SetObjectDims(CM_DIMS_DEFAULT);

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
		{ pMovement->SetMovementState(CMS_CLIMBING); return; }
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
		{ pMovement->SetMovementState(CMS_SWIMMING); return; }

	// ----------------------------------------------------------------------- //
	// Check for jump state change: If our jump flag is on, we should jump
	if(cvVars->m_dwCtrl & CM_FLAG_JUMP && cvVars->m_bAllowJump)
		{ pMovement->SetMovementState(CMS_PRE_JUMPING); return; }

	if((cvVars->m_dwCtrl & CM_FLAG_DUCK) || (cvVars->m_dwCtrl & CM_FLAG_WALLWALK))
		{ pMovement->SetMovementState(CMS_PRE_CRAWLING); return; }

	// If we're not standing on anything, then we're either falling, or jumping
	if(!cvVars->m_bStandingOn)
		{ pMovement->SetMovementState(CMS_PRE_FALLING); return; }

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseGroundAccel * cvVars->m_fSpeedMod * cvVars->m_fWalkSpeedMod;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBaseWalkSpeed * cvVars->m_fWalkSpeedMod, CM_DIR_FLAG_XZ_PLANE);

	// Give a little bit of fudge factor here...
	if((fVelMag + CHARACTERMOVEMENT_VEL_CHECK_FUDGE) > cbButes->m_fBaseWalkSpeed*cvVars->m_fSpeedMod*cvVars->m_fWalkSpeedMod )
	{
		if(cvVars->m_dwCtrl & CM_FLAG_RUN)
			{ pMovement->SetMovementState(CMS_RUNNING); return; }
	}

	// ----------------------------------------------------------------------- //
	// Check for idle state change: If the velocity is very small... put us in the Idle state
	if((fAccelMag == 0.0f) && (fVelMag < 1.0f))
		{ pMovement->SetMovementState(CMS_IDLE); return; }
}

LTFLOAT CharacterMovementState_Walking_MaxSpeed(const CharacterMovement *pMovement)
{
	return   pMovement->GetCharacterButes()->m_fBaseWalkSpeed
		   * pMovement->GetCharacterVars()->m_fWalkSpeedMod
		   * pMovement->GetCharacterVars()->m_fSpeedMod;
}

LTFLOAT CharacterMovementState_Walking_MaxAccel(const CharacterMovement *pMovement)
{
	return   pMovement->GetCharacterButes()->m_fBaseGroundAccel
		   * pMovement->GetCharacterVars()->m_fWalkSpeedMod
		   * pMovement->GetCharacterVars()->m_fSpeedMod;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Running(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the appropriate size
	pMovement->SetObjectDims(CM_DIMS_DEFAULT);

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
		{ pMovement->SetMovementState(CMS_CLIMBING); return; }
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
		{ pMovement->SetMovementState(CMS_SWIMMING); return; }

	// ----------------------------------------------------------------------- //
	// Check for jump state change: If our jump flag is on, we should jump
	if(cvVars->m_dwCtrl & CM_FLAG_JUMP && cvVars->m_bAllowJump)
		{ pMovement->SetMovementState(CMS_PRE_JUMPING); return; }

	if((cvVars->m_dwCtrl & CM_FLAG_DUCK) || (cvVars->m_dwCtrl & CM_FLAG_WALLWALK))
		{ pMovement->SetMovementState(CMS_PRE_CRAWLING); return; }

	// If we're not standing on anything, then we're either falling, or jumping
	if(!cvVars->m_bStandingOn)
		{ pMovement->SetMovementState(CMS_PRE_FALLING); return; }

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseGroundAccel * cvVars->m_fRunSpeedMod * cvVars->m_fSpeedMod;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBaseRunSpeed * cvVars->m_fRunSpeedMod, CM_DIR_FLAG_XZ_PLANE);

	// Put us back in walk mode if we let go of the run key
	if(!(cvVars->m_dwCtrl & CM_FLAG_RUN) && (cvVars->m_dwCtrl & CM_FLAG_DIRECTIONAL))
		{ pMovement->SetMovementState(CMS_WALKING); return; }

	// ----------------------------------------------------------------------- //
	// Check for idle state change: If the velocity is very small... put us in the Idle state
	if((fAccelMag == 0.0f) && (fVelMag < 1.0f))
		{ pMovement->SetMovementState(CMS_IDLE); return; }
}

LTFLOAT CharacterMovementState_Running_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseRunSpeed
		   * pMovement->GetCharacterVars()->m_fRunSpeedMod
		   * pMovement->GetCharacterVars()->m_fSpeedMod;
}

LTFLOAT CharacterMovementState_Running_MaxAccel(const CharacterMovement *pMovement)
{
	return   pMovement->GetCharacterButes()->m_fBaseGroundAccel 
		   * pMovement->GetCharacterVars()->m_fRunSpeedMod
		   * pMovement->GetCharacterVars()->m_fSpeedMod;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Pre_Jumping(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	INTERFACE *pInterface = pMovement->m_pInterface;
	MathLT *pMath = pMovement->m_pMath;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Get the current velocity of the character
	LTVector vVel = cvVars->m_vVelocity;

	// Calculate a new direction of velocity to accelerate
	LTVector vJumpDir = cbButes->m_vJumpDirection;
	LTVector vNewDir = (cvVars->m_vRight * vJumpDir.x) + (cvVars->m_vUp * vJumpDir.y) + (cvVars->m_vForward * vJumpDir.z);
	CharacterMovementState cms = CMS_STAND_JUMP;
	LTBOOL bSlantedTest = LTFALSE;

	// Scale the direction according to the correctly jump speed
	switch(pMovement->GetLastMovementState())
	{
		case CMS_WALKING:
		{
			// Spress the acceleration...
			pMovement->SetAcceleration(LTVector(0.0f, 0.0f, 0.0f));

			vVel.y = 0.0f;
			vNewDir.Norm(cbButes->m_fBaseJumpSpeed * cvVars->m_fJumpSpeedMod);
			cms = CMS_WALK_JUMP;
			bSlantedTest = LTTRUE;
			break;
		}

		case CMS_RUNNING:
		{
			vVel.y = 0.0f;
			vNewDir.Norm(cbButes->m_fBaseRunJumpSpeed * cvVars->m_fJumpSpeedMod);
			cms = CMS_RUN_JUMP;
			bSlantedTest = LTTRUE;
			break;
		}

		case CMS_CRAWLING:
		{
			vVel.y = 0.0f;
			vNewDir.Norm(cvVars->m_fStampTime * cvVars->m_fJumpSpeedMod);
			cms = CMS_STAND_JUMP;
			bSlantedTest = LTTRUE;
			break;
		}

		case CMS_WALLWALKING:
		{
			vNewDir = cvVars->m_vStampPos;
			vNewDir.Norm(cvVars->m_fStampTime * cvVars->m_fJumpSpeedMod);
			cms = CMS_WWALK_JUMP;
			break;
		}

		default:
		{
			vVel.y = 0.0f;
			vNewDir.Norm(cbButes->m_fBaseJumpSpeed * cvVars->m_fJumpSpeedMod);
			cms = CMS_STAND_JUMP;
			bSlantedTest = LTTRUE;
			break;
		}
	}

	// Test to see if we're on a steep slant and push off of it...
	if(bSlantedTest && cvVars->m_bStandingOn)
	{
		if(cvVars->m_ciStandingOn.m_hPoly != INVALID_HPOLY)
		{
			LTPlane *pPlane;
			pInterface->Common()->GetPolyInfo(cvVars->m_ciStandingOn.m_hPoly, &pPlane);

			if(pPlane && pPlane->m_Normal.y < 0.7071)
			{
				vNewDir += (pPlane->m_Normal * 200.0f);
			}
		}
	}

	// Take care of encumbered characters...
	if(cvVars->m_bEncumbered)
		vNewDir *= CHARACTERMOVEMENT_ENCUMBERED_SCALE;

	LTVector vNewVelocity = vVel + vNewDir;

	// Get the final velocity and set it
	pMovement->SetVelocity( vNewVelocity );

	// Clear the standingon info, so we don't think we've landed already.
	cvVars->m_bStandingOn = FALSE;

	// Set the state to the normal jumping control
	pMovement->SetMovementState(cms);

	// Play a jump sound
	if(hObject)
	{
		CharacterButes* pButes = pMovement->GetCharacterButes();

		if(pButes)
		{
#ifdef _CLIENTBUILD
			g_pClientSoundMgr->PlayCharSndFromObject(pButes->m_nId, hObject, "JumpLocal");

			// Send up a message so that others can hear my jump bellow!
			HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pClientDE->WriteToMessageByte(hMessage, CP_JUMP_SOUND);
			g_pClientDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
#else
			g_pServerSoundMgr->PlayCharSndFromObject(pButes->m_nId, hObject, "Jump");
#endif
		}
	}

	// Be sure we don't repeat
	pMovement->AllowJump(LTFALSE);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Jump(CharacterMovement *pMovement, LTFLOAT fMaxVel)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();

	// Set the dims according to if we're ducking or not
	uint8 nDimsType = CM_DIMS_DEFAULT;
	if( cvVars->m_dwCtrl & CM_FLAG_DUCK )
		nDimsType = CM_DIMS_CROUCHED;
	if( cvVars->m_dwCtrl & CM_FLAG_WALLWALK )
		nDimsType = CM_DIMS_WALLWALK;

	pMovement->SetObjectDims(nDimsType);

	// Make sure the flags are set properly
	pPhysics->SetFrictionCoefficient(cvVars->m_hObject, 0.0f);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseAirAccel;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	if(fMaxVel)
	{
		LTFLOAT fVelMag = pMovement->UpdateVelocity(fMaxVel, CM_DIR_FLAG_XZ_PLANE);
	}

	// Check to see if we need to switch to the wall walking jump state
	if(cbButes->m_bWallWalk)
	{
		if((cvVars->m_dwCtrl & CM_FLAG_WALLWALK) && (pMovement->GetMovementState() != CMS_WWALK_JUMP))
		{
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
			pMovement->SetMovementState(CMS_WWALK_JUMP);
			return;
		}
		else if(!(cvVars->m_dwCtrl & CM_FLAG_WALLWALK) && (pMovement->GetMovementState() == CMS_WWALK_JUMP))
		{
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
			pMovement->SetMovementState(CMS_WALK_JUMP);
			return;
		}
	}

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
	{
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_CLIMBING);
		return;
	}
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
	{
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_SWIMMING);
		return;
	}

	LTVector vForce, vVel = cvVars->m_vVelocity;
	pPhysics->GetGlobalForce(vForce);
	// Normalize the force, so we get true projections of the velocity on the force direction.
	vForce.Norm( );

	// Record the dotproduct between the velocity and the gravity.
	float fVelDotGravity = vVel.Dot( vForce );

	// ----------------------------------------------------------------------- //
	// Check for falling state change: check the angle between our force and velocity
	// Also check if we're standing on something, which would mean we've already landed.
	if( fVelDotGravity >= -0.1f || cvVars->m_bStandingOn )
	{
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_PRE_FALLING);
		return;
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTRESULT CharacterMovementState_Jump_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	// If the HPOLY is invalid... don't do anything
	if(pInfo->m_hPoly == INVALID_HPOLY)
		return LT_OK;


	// Check the surface flags to see if we can stick to this surface...
	uint32 nSurfFlags = 0;
	pMovement->m_pInterface->Common()->GetPolySurfaceFlags(pInfo->m_hPoly, nSurfFlags);

	if(nSurfFlags & SURF_NOWALLWALK)
		return LT_OK;


	// This is kinda dumb... but I'm gonna set the stamp time to a value that I
	// can check back in the movement state to see if we need to change states.
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	cvVars->m_fStampTime = -2.0f;

	if(pMovement->GetMovementState() == CMS_WWALK_JUMP)
	{
		// Align our rotation to the surface that we hit
		MathLT *pMath = pMovement->m_pMath;

		// Get the original rotation
		LTRotation rRot = cvVars->m_rRot;
		LTVector n = pInfo->m_Plane.m_Normal;
		LTVector cr = cvVars->m_vVelocity;

		LTVector a;
		LTFLOAT theta;

		if(n.y < -0.7071f)
		{
			// Determine whether we roll or pitch around...
			if(cvVars->m_fAimingPitch < -45.0f)
				cr = cvVars->m_vForward;
			else
				cr = cvVars->m_vRight;

			// Get the angle to rotate
			theta = cvVars->m_vUp.Dot(n);
		}
		else
		{
			// Normalize the velocity
			cr.Norm();

			// Get the angle to rotate
			theta = cr.Dot(n);
		}

		// Get the axis of rotation
		a = cr.Cross(-n);
		a.Norm();

		// Bounds check
		theta = theta < -1.0f ? -1.0f : theta;
		theta = theta > 1.0f ? 1.0f : theta;

		theta = (LTFLOAT)acos(theta);

		if(!_isnan(theta))
		{
			pMath->RotateAroundAxis(rRot, a, theta);
			pMovement->SetRot(rRot);
		}
	}

	return LT_OK;
}
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Pre_PounceJump(CharacterMovement *pMovement)
{

#ifdef _CLIENTBUILD

	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	INTERFACE *pInterface = pMovement->m_pInterface;
	MathLT *pMath = pMovement->m_pMath;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();


	// Reset the flags
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags & ~FLAG_GRAVITY, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);

	// Get the current look vector
	HOBJECT hCam = g_pGameClientShell->GetPlayerCameraObject();
	LTRotation rRot;
	LTVector vR, vU, vF;

	g_pLTClient->GetObjectRotation(hCam, &rRot);
	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);


	// Cheezy way of keeping track of the fact we were wall walking.
	if(pMovement->GetLastMovementState() == CMS_WALLWALKING)
		cvVars->m_fStampTime = -1.0f;
	else
		cvVars->m_fStampTime = 0.0f;


	// See about the angle mod...
	LTFLOAT fUD = vF.Dot(cvVars->m_vUp);

	// Bounds check
	if(fUD > 1.0f) fUD = 1.0f;
	if(fUD < 0.0f) { fUD = 0.0f; vF = cvVars->m_vForward; }

	fUD = 1.0f - fUD;


	// Find the angle we want to rotate off the normal direction
	LTFLOAT fOffsetAngle = -MATH_DEGREES_TO_RADIANS(15.0f);
	LTFLOAT fRotAngle = fUD * fOffsetAngle;

	g_pMathLT->AlignRotation(rRot, vF, vU);
	g_pMathLT->RotateAroundAxis(rRot, vR, fRotAngle);
	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);


	// Get the final velocity and set it
	LTVector vNewVel = vF * cbButes->m_fBasePounceSpeed;
	pMovement->SetVelocity(vNewVel);

	// Clear the standingon info, so we don't think we've landed already.
	cvVars->m_bStandingOn = FALSE;

	// Zero out our pos stamp for later use...
	cvVars->m_vStampPos = LTVector(0,0,0);

	// Set the state to the normal jumping control
	pMovement->SetMovementState(CMS_POUNCE_JUMP);


	// Play a jump sound
	if(hObject)
	{
		// Play the pounce sound
		BARREL* pBarrel	= LTNULL;
		uint8 nType = 1;
		
		if(strstr(pMovement->GetCharacterButes()->m_szName, "Facehugger"))
		{
			g_pClientSoundMgr->PlaySoundLocal("FacehuggerPounceCry");
			nType = 2;
		}
		else if(strstr(pMovement->GetCharacterButes()->m_szName, "Predalien"))
		{
			g_pClientSoundMgr->PlaySoundLocal("PredalienPounceCry");
			nType = 3;
		}
		else
		{
			g_pClientSoundMgr->PlaySoundLocal("AlienPounceCry");
		}


		// If we are in multiplayer then send up a message so that 
		// others can hear my pounce cry!
		if(g_pGameClientShell->GetGameType()->IsMultiplayer())
		{
			HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pClientDE->WriteToMessageByte(hMessage, CP_POUNCE_SOUND);
			g_pClientDE->WriteToMessageByte(hMessage, nType);
			g_pClientDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}
	}

#endif

}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Pounce_Jump(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	MathLT *pMath = pMovement->m_pMath;

	// Set the dims according to if we're ducking or not
	uint8 nDimsType = CM_DIMS_DEFAULT;
	if( cvVars->m_dwCtrl & CM_FLAG_DUCK )
		nDimsType = CM_DIMS_CROUCHED;
	if( cvVars->m_dwCtrl & CM_FLAG_WALLWALK )
		nDimsType = CM_DIMS_WALLWALK;

	pMovement->SetObjectDims(nDimsType);

	// Make sure the flags are set properly
	pPhysics->SetFrictionCoefficient(cvVars->m_hObject, 0.0f);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseAirAccel;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBasePounceSpeed, CM_DIR_FLAG_XZ_PLANE);

	LTVector vForce, vVel = cvVars->m_vVelocity;
	pPhysics->GetGlobalForce(vForce);
	// Normalize the force, so we get true projections of the velocity on the force direction.
	vForce.Norm( );

	// Record the dotproduct between the velocity and the gravity.
	float fVelDotGravity = vVel.Dot( vForce );

	// ----------------------------------------------------------------------- //
	// Check for falling state change: check the angle between our force and velocity
	// Also check if we're standing on something, which would mean we've already landed.
	if( (fVelDotGravity >= -0.1f || cvVars->m_bStandingOn) && (cvVars->m_vStampPos.MagSqr() == 0))
	{
		// stamp our position...
		cvVars->m_vStampPos = cvVars->m_vPos;
	}


	// This is a cheezy way of telling us that we need to go into wall walk mode...
	if(cvVars->m_fStampTime == -2.0f)
	{
		// This will take care of falling damage and swaping us over to wall walking...
		CharacterMovementState_Land(pMovement, cbButes->m_fBaseCrouchSpeed * cvVars->m_fCrouchSpeedMod * 2.0f, CMS_WALLWALKING);
		cvVars->m_fStampTime = 0.0f;
		return;
	}

	if(cvVars->m_fStampTime == -3.0f)
	{
		RealignCharacter(pMovement);

		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_PRE_FALLING);
		cvVars->m_fStampTime = 0.0f;
		return;
	}

	// Check to see if we need to switch to the wall walking jump state
	if(cvVars->m_dwCtrl & CM_FLAG_WALLWALK)
		cvVars->m_fStampTime = -1.0f;
	else
		cvVars->m_fStampTime = 0.0f;

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
	{
		RealignCharacter(pMovement);

		// Reset the flags for the next state...
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_CLIMBING);
		return;
	}
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
	{
		RealignCharacter(pMovement);

		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_SWIMMING);
		return;
	}

	// We are controlling gravity manually...
	pPhysics->GetGlobalForce(vForce);

	vVel += vForce * cvVars->m_fFrameTime * 0.5f;
	pMovement->SetVelocity(vVel);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTRESULT CharacterMovementState_Pounce_Jump_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// If the HPOLY is invalid... don't do anything
	if(pInfo->m_hPoly == INVALID_HPOLY)
	{
		cvVars->m_fStampTime = -3.0f;
		return LT_OK;
	}

	// If the HPOLY is not against the direction of the pounce... then ignore it
	if(pInfo->m_Plane.m_Normal.Dot(cvVars->m_vVelocity) > 0.0f)
	{
		return LT_OK;
	}

	if(cvVars->m_fStampTime == -1.0f)
	{
		// Check the surface flags to see if we can stick to this surface...
		uint32 nSurfFlags = 0;
		pMovement->m_pInterface->Common()->GetPolySurfaceFlags(pInfo->m_hPoly, nSurfFlags);

		if(nSurfFlags & SURF_NOWALLWALK)
		{
			cvVars->m_fStampTime = -3.0f;
			return LT_OK;
		}


		// Align our rotation to the surface that we hit
		MathLT *pMath = pMovement->m_pMath;

		// Get the original rotation vectors
		LTVector n = pInfo->m_Plane.m_Normal;
		LTVector vR, vU, vF;

		LTRotation rRot = cvVars->m_rRot;
		pMath->GetRotationVectors(rRot, vR, vU, vF);

		// Get the axis of rotation
		LTVector a = vU.Cross(-n);//axis of rotation
		a.Norm();

		// Get the angle to rotate
		float theta = vU.Dot(n);//angle to rotate

		// Bounds check
		theta = theta < -1.0f ? -1.0f : theta;
		theta = theta > 1.0f ? 1.0f : theta;

		theta = (float) acos(theta);

		if(!_isnan(theta))
		{
			pMath->RotateAroundAxis(rRot, a, theta);
			pMovement->SetRot(rRot);
		}

		// This is kinda dumb... but I'm gonna set the stamp time to a value that I
		// can check back in the movement state to see if we need to change states.
		cvVars->m_fStampTime = -2.0f;
		return LT_OK;
	}

	cvVars->m_fStampTime = -3.0f;
	return LT_OK;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Pounce_Fall(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	MathLT *pMath = pMovement->m_pMath;

	// Set the dims according to if we're ducking or not
	uint8 nDimsType = CM_DIMS_DEFAULT;
	if( cvVars->m_dwCtrl & CM_FLAG_DUCK )
		nDimsType = CM_DIMS_CROUCHED;
	if( cvVars->m_dwCtrl & CM_FLAG_WALLWALK )
		nDimsType = CM_DIMS_WALLWALK;

	pMovement->SetObjectDims(nDimsType);

	// Make sure the flags are set properly
	pPhysics->SetFrictionCoefficient(cvVars->m_hObject, 0.0f);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseAirAccel;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBasePounceSpeed, CM_DIR_FLAG_XZ_PLANE);


	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
	{
		RealignCharacter(pMovement);

		// Reset the flags for the next state...
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_CLIMBING);
		return;
	}
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
	{
		RealignCharacter(pMovement);

		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_SWIMMING);
		return;
	}

	// If we're standing on an object, we've landed... so change the state to landing
	if(cvVars->m_bStandingOn || cvVars->m_fStampTime == -2.0f)
	{
		// Reset the flags for the next state...
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);

		if(cvVars->m_fStampTime == -1.0f || cvVars->m_fStampTime == -2.0f)
		{
			pMovement->SetMovementState(CMS_WWALK_LAND);
		}
		else
		{
			RealignCharacter(pMovement);

			pMovement->SetMovementState(CMS_STAND_LAND);
		}

		cvVars->m_fStampTime = 0.0f;
		return;
	}

	// Check to see if we need to switch out of the wall walking jump state
	if(cvVars->m_dwCtrl & CM_FLAG_WALLWALK)
		cvVars->m_fStampTime = -1.0f;
	else
		cvVars->m_fStampTime = 0.0f;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Stand_Jump(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterMovementState_Jump(pMovement, cbButes->m_fBaseAirAccel);
}

void CharacterMovementState_Walk_Jump(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars * cvVars = pMovement->GetCharacterVars();
	CharacterMovementState_Jump(pMovement, cbButes->m_fBaseWalkSpeed * cvVars->m_fWalkSpeedMod);
}

void CharacterMovementState_Run_Jump(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars * cvVars = pMovement->GetCharacterVars();

	LTFLOAT fSpeed = cbButes->m_fBaseRunSpeed * cvVars->m_fRunSpeedMod;
	if(fSpeed > CHARACTERMOVEMENT_MAX_RUN_AIR_VEL) fSpeed = CHARACTERMOVEMENT_MAX_RUN_AIR_VEL;

	CharacterMovementState_Jump(pMovement, fSpeed);
}

void CharacterMovementState_WWalk_Jump(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	CharacterMovementState_Jump(pMovement, cbButes->m_fBaseCrouchSpeed * cvVars->m_fCrouchSpeedMod * 2.0f);

	// This is a cheezy way of telling us that we need to go into wall walk mode...
	if(cvVars->m_fStampTime == -2.0f)
	{
		pMovement->SetMovementState(CMS_WALLWALKING);
		cvVars->m_fStampTime = 0.0f;
	}
}

LTFLOAT CharacterMovementState_Jumping_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseAirAccel;
}

LTFLOAT CharacterMovementState_Jumping_MaxAccel(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseAirAccel;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Pre_Falling(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();

	// Set the stamp position to our current position... we'll use this to
	// determine falling distance after we land.
	cvVars->m_vStampPos = cvVars->m_vPos;

	// Scale the direction according to the correctly jump speed
	switch(pMovement->GetLastMovementState())
	{
		case CMS_WALK_JUMP:			pMovement->SetMovementState(CMS_WALK_FALL);			break;
		case CMS_RUN_JUMP:			pMovement->SetMovementState(CMS_RUN_FALL);			break;
		case CMS_WWALK_JUMP:		pMovement->SetMovementState(CMS_WWALK_FALL);		break;

		default:
		{
			if(cbButes->m_bWallWalk && (cvVars->m_dwCtrl & CM_FLAG_WALLWALK))
				pMovement->SetMovementState(CMS_WWALK_FALL);
			else
				pMovement->SetMovementState(CMS_STAND_FALL);

			break;
		}
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Fall(CharacterMovement *pMovement, LTFLOAT fMaxVel, CharacterMovementState cms)
{
	// Setup some temporary attribute variables to make things clearer
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();

	// Set the dims according to if we're ducking or not
	uint8 nDimsType = CM_DIMS_DEFAULT;
	if( cvVars->m_dwCtrl & CM_FLAG_DUCK )
		nDimsType = CM_DIMS_CROUCHED;
	if( cvVars->m_dwCtrl & CM_FLAG_WALLWALK )
		nDimsType = CM_DIMS_WALLWALK;

	pMovement->SetObjectDims(nDimsType);

	// Make sure the flags are set properly
	pPhysics->SetFrictionCoefficient(cvVars->m_hObject, 0.0f);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseAirAccel;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	if(fMaxVel)
	{
		LTFLOAT fVelMag = pMovement->UpdateVelocity(fMaxVel, CM_DIR_FLAG_XZ_PLANE);
	}

	// Check to see if we need to switch out of the wall walking jump state
	if(cbButes->m_bWallWalk)
	{
		if((cvVars->m_dwCtrl & CM_FLAG_WALLWALK) && (pMovement->GetMovementState() != CMS_WWALK_FALL))
		{
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
			pMovement->SetMovementState(CMS_WWALK_FALL);
			return;
		}
		else if(!(cvVars->m_dwCtrl & CM_FLAG_WALLWALK) && (pMovement->GetMovementState() == CMS_WWALK_FALL))
		{
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
			pMovement->SetMovementState(CMS_WALK_FALL);
			return;
		}
	}

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
	{
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_CLIMBING);
		return;
	}
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
	{
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(CMS_SWIMMING);
		return;
	}

	// If we're standing on an object, we've landed... so change the state to landing
	if(cvVars->m_bStandingOn)
	{
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
		pMovement->SetMovementState(cms);
		cvVars->m_fStampTime = 0.0f;
		return;
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTRESULT CharacterMovementState_Fall_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	// If the HPOLY is invalid... don't do anything
	if(pInfo->m_hPoly == INVALID_HPOLY)
		return LT_OK;


	// Check the surface flags to see if we can stick to this surface...
	uint32 nSurfFlags = 0;
	pMovement->m_pInterface->Common()->GetPolySurfaceFlags(pInfo->m_hPoly, nSurfFlags);

	if(nSurfFlags & SURF_NOWALLWALK)
		return LT_OK;


	// This is kinda dumb... but I'm gonna set the stamp time to a value that I
	// can check back in the movement state to see if we need to change states.
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	cvVars->m_fStampTime = -2.0f;

	if(pMovement->GetMovementState() == CMS_WWALK_FALL)
	{
		// Align our rotation to the surface that we hit
		MathLT *pMath = pMovement->m_pMath;

		// Get the original rotation vectors
		LTVector n = pInfo->m_Plane.m_Normal;
		LTVector vR, vU, vF;

		LTRotation rRot = cvVars->m_rRot;
		pMath->GetRotationVectors(rRot, vR, vU, vF);

		// Get the axis of rotation
		LTVector a = vU.Cross(-n);//axis of rotation
		a.Norm();

		// Get the angle to rotate
		float theta = vU.Dot(n);//angle to rotate

		// Bounds check
		theta = theta < -1.0f ? -1.0f : theta;
		theta = theta > 1.0f ? 1.0f : theta;

		theta = (float) acos(theta);

		if(!_isnan(theta))
		{
			pMath->RotateAroundAxis(rRot, a, theta);
			pMovement->SetRot(rRot);
		}
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Stand_Fall(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterMovementState_Fall(pMovement, cbButes->m_fBaseAirAccel, CMS_STAND_LAND);
}

void CharacterMovementState_Walk_Fall(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars * cvVars = pMovement->GetCharacterVars();

	CharacterMovementState_Fall(pMovement, cbButes->m_fBaseWalkSpeed * cvVars->m_fWalkSpeedMod, CMS_WALK_LAND);
}

void CharacterMovementState_Run_Fall(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars * cvVars = pMovement->GetCharacterVars();

	LTFLOAT fSpeed = cbButes->m_fBaseRunSpeed * cvVars->m_fRunSpeedMod;
	if(fSpeed > CHARACTERMOVEMENT_MAX_RUN_AIR_VEL) fSpeed = CHARACTERMOVEMENT_MAX_RUN_AIR_VEL;

	CharacterMovementState_Fall(pMovement, fSpeed, CMS_RUN_LAND);
}

void CharacterMovementState_WWalk_Fall(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	CharacterMovementState_Fall(pMovement, cbButes->m_fBaseCrouchSpeed * cvVars->m_fCrouchSpeedMod * 2.0f, CMS_WWALK_LAND);

	// This is a cheezy way of telling us that we need to go into wall walk mode...
	if(cvVars->m_fStampTime == -2.0f)
	{
		CharacterMovementState_Land(pMovement, cbButes->m_fBaseCrouchSpeed * cvVars->m_fCrouchSpeedMod * 2.0f, CMS_WALLWALKING);
//		pMovement->SetMovementState(CMS_WALLWALKING);
		cvVars->m_fStampTime = 0.0f;
	}
}

LTFLOAT CharacterMovementState_Falling_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseAirAccel;
}

LTFLOAT CharacterMovementState_Falling_MaxAccel(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseAirAccel;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Land(CharacterMovement *pMovement, LTFLOAT fMaxVel, CharacterMovementState cms)
{
	// Setup some temporary attribute variables to make things clearer
	INTERFACE *pInterface = pMovement->m_pInterface;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(cbButes->m_fBaseGroundAccel, CM_DIR_FLAG_XZ_PLANE);
	LTFLOAT fFallDistance = 0;
	
	// Make sure we have a valid start point
	if(cvVars->m_vStampPos.MagSqr() != 0)
		fFallDistance = cvVars->m_vStampPos.y - cvVars->m_vPos.y;


	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	if(fMaxVel)
	{
		LTFLOAT fVelMag = pMovement->UpdateVelocity(fMaxVel, CM_DIR_FLAG_XZ_PLANE);
	}


	// ----------------------------------------------------------------------- //
	// Set our new movement state
	pMovement->SetMovementState(cms);


	// ----------------------------------------------------------------------- //
	// Do a camera effect if we are local.

	if(fFallDistance > 5.0f)
	{
#ifdef _CLIENTBUILD
		LTVector vDims = pMovement->GetDimsSetting(CM_DIMS_DEFAULT);

		LTBOOL bSmall = (vDims.y < 40.0);

		// Do the land bob but only if the net FX is not active
		HCAMERA hCam = g_pGameClientShell->GetPlayerCamera();
		CAMERAEFFECTINSTANCE* pInst = g_pCameraMgr->GetCameraEffectByID(hCam, CAMERAMGRFX_ID_NET_TILT, LTNULL); 

		if(!g_pGameClientShell->IsFirstLand())
		{
			if(!pInst)
			{
				CameraMgrFX_LandingBobCreate( hCam, bSmall);
			}


			// Play foot step sound here
			CCharacterFX* pFX = dynamic_cast<CCharacterFX*>(g_pGameClientShell->GetSFXMgr()->GetClientCharacterFX());
			if(pFX) pFX->ForceFootstep();

			// Play character sound here
			HOBJECT hObject = pMovement->GetObject();
			if(hObject)
			{
				CharacterButes* pButes = pMovement->GetCharacterButes();

				if(pButes)
				{
					g_pClientSoundMgr->PlayCharSndFromObject(pButes->m_nId, hObject, "LandLocal");
				}
			}

			// Snd up a message so that others can hear my jump bellow!
			HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pClientDE->WriteToMessageByte(hMessage, CP_LAND_SOUND);
			g_pClientDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
		}

#else
		// Play character sound here
		HOBJECT hObject = pMovement->GetObject();
		if(hObject)
		{
			CharacterButes* pButes = pMovement->GetCharacterButes();

			if(pButes)
			{
				g_pServerSoundMgr->PlayCharSndFromObject(pButes->m_nId, hObject, "Land");
			}
		}
#endif
	}

	// ----------------------------------------------------------------------- //
	// Check to see if we've fallen too far and damage accordingly
	if((cbButes->m_fFallDistanceSafe >= 0.0f) && (cbButes->m_fFallDistanceDeath >= 0.0f))
	{
		cvVars->m_vStampPos = cvVars->m_vPos;

		if(fFallDistance > cbButes->m_fFallDistanceSafe)
		{
			LTFLOAT fMaxDamage = cbButes->m_fMaxHitPoints + cbButes->m_fMaxArmorPoints;
			LTFLOAT fDamageScale = (fFallDistance - cbButes->m_fFallDistanceSafe) / (cbButes->m_fFallDistanceDeath - cbButes->m_fFallDistanceSafe);
			LTFLOAT fDamageAmount = fMaxDamage * fDamageScale;
			LTVector vDamageDir = -pMovement->GetUp();

#ifdef CHARACTER_MOVEMENT_DEBUG
	#ifdef _CLIENTBUILD
			pInterface->CPrint("%.2f CharMov : Object(%s) Falling Damage: Dist = %f, Safe Dist = %f, Max Dist = %f, Amount = %f, Max Amount = %f", pInterface->GetTime(), "Client Physics Object", fFallDistance, cbButes->m_fFallDistanceSafe, cbButes->m_fFallDistanceDeath, fDamageAmount, fMaxDamage);
	#else
			if(bDebugPlayer || _stricmp(pInterface->GetObjectName(cvVars->m_hObject), "Player"))
				pInterface->CPrint("%.2f CharMov : Object(%s) Falling Damage: Dist = %f, Safe Dist = %f, Max Dist = %f, Amount = %f, Max Amount = %f", pInterface->GetTime(), pInterface->GetObjectName(cvVars->m_hObject), fFallDistance, cbButes->m_fFallDistanceSafe, cbButes->m_fFallDistanceDeath, fDamageAmount, fMaxDamage);
	#endif
#endif

#ifdef _CLIENTBUILD
			// Only damage us if we are not in a cinematic...
			// Some silly players jump into cinematic triggers then if they
			// are teleported they can take massive falling damage...
			if(!g_pGameClientShell->IsCinematic())
			{
				// Send message to server to do damage
				HMESSAGEWRITE hMessage = pInterface->StartMessage(MID_PLAYER_CLIENTMSG);
				pInterface->WriteToMessageByte(hMessage, CP_DAMAGE);
				pInterface->WriteToMessageByte(hMessage, DT_CRUSH);
				pInterface->WriteToMessageFloat(hMessage, fDamageAmount);
				pInterface->WriteToMessageVector(hMessage, &vDamageDir);
				hMessage->WriteObject(pInterface->GetClientObject());
				pInterface->EndMessage(hMessage);
			}
#else
			// Do damage to our selves
			DamageStruct damage;
			damage.eType = DT_CRUSH;
			damage.fDamage = fDamageAmount;
			damage.vDir = vDamageDir;
			damage.hDamager = pMovement->GetObject();

			BaseClass *pObj = pInterface->HandleToObject(pMovement->GetObject());
			damage.DoDamage(pObj, pMovement->GetObject());
#endif
		}
	}
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Stand_Land(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterMovementState_Land(pMovement, cbButes->m_fBaseAirAccel, (cvVars->m_dwCtrl & CM_FLAG_DUCK) ? CMS_PRE_CRAWLING : CMS_IDLE);

	RealignCharacter(pMovement);
}

void CharacterMovementState_Walk_Land(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars * cvVars = pMovement->GetCharacterVars();
	CharacterMovementState_Land(pMovement, cbButes->m_fBaseWalkSpeed * cvVars->m_fWalkSpeedMod * 2.0f, CMS_WALKING);

	RealignCharacter(pMovement);
}

void CharacterMovementState_Run_Land(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars * cvVars = pMovement->GetCharacterVars();
	CharacterMovementState_Land(pMovement, cbButes->m_fBaseRunSpeed * cvVars->m_fRunSpeedMod, CMS_RUNNING);

	RealignCharacter(pMovement);
}

void CharacterMovementState_WWalk_Land(CharacterMovement *pMovement)
{
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterMovementState_Land(pMovement, cbButes->m_fBaseAirAccel, (cvVars->m_dwCtrl & CM_FLAG_WALLWALK) ? CMS_WALLWALKING : CMS_IDLE);

	RealignCharacter(pMovement);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Pre_Crawling(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	INTERFACE *pInterface = pMovement->m_pInterface;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the crouched size
	LTVector vOrigDims = pMovement->GetObjectDims();
	LTVector vSizedDims = pMovement->SetObjectDims((cbButes->m_bWallWalk && (cvVars->m_dwCtrl & CM_FLAG_WALLWALK)) ? CM_DIMS_WALLWALK : CM_DIMS_CROUCHED);
	pMovement->SetMovementState((cbButes->m_bWallWalk && (cvVars->m_dwCtrl & CM_FLAG_WALLWALK)) ? CMS_WALLWALKING : CMS_CRAWLING);

	// Setup a new position that the character will try to move to when the dims change
	LTVector vNewPos = pMovement->GetPos() - (cvVars->m_vUp * (vOrigDims.y - vSizedDims.y));
	pMovement->SetPos(vNewPos);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Crawling(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	INTERFACE *pInterface = pMovement->m_pInterface;
	MathLT *pMath = pMovement->m_pMath;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
		{ pMovement->SetMovementState(CMS_CLIMBING); return; }
	else if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
		{ pMovement->SetMovementState(CMS_SWIMMING); return; }

	// Make sure we have proper dims..
	pMovement->SetObjectDims(CM_DIMS_CROUCHED);


	// See if we should double the acceleration and velocity speeds
	LTFLOAT fMod = ((cvVars->m_dwCtrl & CM_FLAG_RUN) && (cbButes->m_bWallWalk)) ? 2.0f : 1.0f;


	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseGroundAccel * cvVars->m_fCrouchSpeedMod * cvVars->m_fSpeedMod;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel * fMod, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBaseCrouchSpeed * cvVars->m_fCrouchSpeedMod * fMod, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// Check for jump state change: If our jump flag is on, we should jump
	if(cvVars->m_dwCtrl & CM_FLAG_JUMP && cvVars->m_bAllowJump)
	{
#ifdef _CLIENTBUILD

		// Check the dims area to see if we have enough room to jump
		if(!pMovement->CheckDimsSpace(pMovement->GetDimsSetting(CM_DIMS_DEFAULT)))
			return;

		// See if it is tiem to spring or not...
		if(g_pGameClientShell->CanSpringJump())
			cvVars->m_fStampTime = cbButes->m_fBaseSpringJumpSpeed;
		else
			cvVars->m_fStampTime = cbButes->m_fBaseJumpSpeed;

#else

		cvVars->m_fStampTime = cbButes->m_fBaseSpringJumpSpeed;

#endif

		pMovement->SetMovementState(CMS_PRE_JUMPING);
		return;
	}

	// ----------------------------------------------------------------------- //

	if(!(cvVars->m_dwCtrl & CM_FLAG_DUCK) && !(cbButes->m_bWallWalk && (cvVars->m_dwCtrl & CM_FLAG_WALLWALK)))
	{
		// If our time is clear... try to reset the dims
		if(!cvVars->m_fResetDimsCheckTime)
		{
			// Check the dims area to see if we have enough room to stand
			if(pMovement->CheckDimsSpace(pMovement->GetDimsSetting(CM_DIMS_DEFAULT)))
			{
				pMovement->SetObjectDims(CM_DIMS_DEFAULT);
				pMovement->SetMovementState(CMS_POST_CRAWLING);
				cvVars->m_fResetDimsCheckTime = 0.0f;
				return;
			}

			// Add make sure we wait a little while
			cvVars->m_fResetDimsCheckTime += cvVars->m_fFrameTime;
		}
		else
		{
			LTFLOAT fCheckTime = 1.0f;

#ifdef _CLIENTBUILD
			// Adjust the check time according to our console variable
			HCONSOLEVAR hVar;
			if(hVar = g_pLTClient->GetConsoleVar("ResetDimsCheckTime"))
				fCheckTime = g_pLTClient->GetVarValueFloat(hVar);

			switch(g_pGameClientShell->GetFrameRateState())
			{
			case FR_UNPLAYABLE:	fCheckTime = 1.0f; break;
			case FR_POOR:		fCheckTime = 0.7f; break;
			case FR_AVERAGE:	fCheckTime = 0.3f; break;
			case FR_GOOD:		fCheckTime = 0.1f; break;
			case FR_EXCELLENT:	fCheckTime = 0.0f; break;
			}
#endif

			// Update our reset dims check time
			cvVars->m_fResetDimsCheckTime += cvVars->m_fFrameTime;

			if(cvVars->m_fResetDimsCheckTime >= fCheckTime)
				cvVars->m_fResetDimsCheckTime = 0.0f;
		}
	}
	else if(!cvVars->m_bStandingOn)
	{
		// If we're not standing on anything, then we're either falling, or jumping
		pMovement->SetMovementState(CMS_PRE_FALLING);
	}

	// Gotta do this check here 'cause you can be crouched in a low ceiling area
	// without the key down.  The if you hit the WW button we gotta check here.
	if(cvVars->m_bStandingOn && cbButes->m_bWallWalk && (cvVars->m_dwCtrl & CM_FLAG_WALLWALK))
	{
		if(cvVars->m_ciStandingOn.m_hPoly != INVALID_HPOLY)
		{
			// Grab the surface flags of this poly
			uint32 nSurfFlags;
			pInterface->Common()->GetPolySurfaceFlags(cvVars->m_ciStandingOn.m_hPoly, nSurfFlags);

			if(!(nSurfFlags & SURF_NOWALLWALK))
			{
				// If we're a wall walking guy... go back to that state
				pMovement->SetMovementState(CMS_WALLWALKING);
			}
		}
	}
}

LTFLOAT CharacterMovementState_Crawling_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseCrouchSpeed 
		 * pMovement->GetCharacterVars()->m_fCrouchSpeedMod 
		 * pMovement->GetCharacterVars()->m_fSpeedMod;
}

LTFLOAT CharacterMovementState_Crawling_MaxAccel(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseGroundAccel 
		 * pMovement->GetCharacterVars()->m_fCrouchSpeedMod 
		 * pMovement->GetCharacterVars()->m_fSpeedMod;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Post_Crawling(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the default size
	pMovement->SetObjectDims(CM_DIMS_DEFAULT);

	// Get the velocity and acceleration
	LTVector vVel = cvVars->m_vVelocity;
	LTVector vAccel = cvVars->m_vAccel;

	// Get the magnitudes
	LTFLOAT fVel = vVel.Mag();
	LTFLOAT fAccel = vAccel.Mag();

	// See which state we should go back to...
	if((fAccel == 0.0f) && (fVel < 1.0f))
		pMovement->SetMovementState(CMS_IDLE);
	else
		pMovement->SetMovementState(CMS_WALKING);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_WallWalking(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	MathLT *pMath = pMovement->m_pMath;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	INTERFACE *pInterface = pMovement->m_pInterface;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Make sure the flags are set properly
	pPhysics->SetFrictionCoefficient(cvVars->m_hObject, CHARACTERMOVEMENT_WALLWALK_FRICTION);
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags & ~FLAG_SHADOW, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2 | FLAG2_SPHEREPHYSICS | FLAG2_ORIENTMOVEMENT, LTFALSE);

	// See if we should double the acceleration and velocity speeds
	LTFLOAT fMod = (cvVars->m_dwCtrl & CM_FLAG_RUN) ? 2.0f : 1.0f;

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION

	LTFLOAT fAccel = cbButes->m_fBaseGroundAccel * cvVars->m_fCrouchSpeedMod * cvVars->m_fSpeedMod;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel * fMod, CM_DIR_FLAG_XZ_PLANE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY

	LTFLOAT fVelMag = 0.0f;

	if(fAccelMag)
	{
		fVelMag = pMovement->UpdateVelocity(cbButes->m_fBaseCrouchSpeed * cvVars->m_fCrouchSpeedMod * fMod, CM_DIR_FLAG_XZ_PLANE);
	}
	else
	{
		// Dampen the velocity so we don't 'skid'
		LTVector vVel = cvVars->m_vVelocity;
		vVel *= (0.5f * (1.0f - (cvVars->m_fFrameTime > 1.0f ? 1.0f : cvVars->m_fFrameTime)));
		pMovement->SetVelocity(vVel);
	}

	// ----------------------------------------------------------------------- //
	// CHECK FOR STATE EXITS

	LTBOOL bRealign = LTFALSE;
	int nRealignType = REALIGN_STANDARD;

	// Change the state to falling
	if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
	{
		pMovement->SetMovementState(CMS_SWIMMING);
		bRealign = LTTRUE;
	}
	else if(cvVars->m_dwCtrl & CM_FLAG_JUMP && cvVars->m_bAllowJump)
	{
		// Check the dims area to see if we have enough room to jump
		if(pMovement->CheckDimsSpace(pMovement->GetDimsSetting(CM_DIMS_DEFAULT)))
		{
			// Calculate a new direction of velocity to accelerate
			LTVector vJumpDir = cbButes->m_vJumpDirection;
			cvVars->m_vStampPos = (cvVars->m_vRight * vJumpDir.x) + (cvVars->m_vUp * vJumpDir.y) + (cvVars->m_vForward * vJumpDir.z);

			if(fVelMag < 5.0f)
				cvVars->m_fStampTime = cbButes->m_fBaseSpringJumpSpeed;
			else if((cvVars->m_dwCtrl & CM_FLAG_DIRECTIONAL) && (cvVars->m_dwCtrl & CM_FLAG_RUN))
				cvVars->m_fStampTime = cbButes->m_fBaseSpringJumpSpeed;
			else
				cvVars->m_fStampTime = cbButes->m_fBaseRunJumpSpeed;

			pMovement->SetMovementState(CMS_PRE_JUMPING);
			bRealign = LTTRUE;
			nRealignType = REALIGN_WALLWALK_JUMP;
		}
	}
	else if(!cvVars->m_bStandingOn || !(cvVars->m_dwCtrl & CM_FLAG_WALLWALK))
	{
		// Make sure we don't fall thru the world
//		pMovement->SetObjectDims(CM_DIMS_VERY_SMALL);
//		pMovement->SetObjectDims(CM_DIMS_CROUCHED);

		pMovement->SetMovementState(CMS_CRAWLING);
		bRealign = LTTRUE;
	}

	if(bRealign)
	{
		RealignCharacter(pMovement, nRealignType);

		// Clear the standing on.
		cvVars->m_bStandingOn = LTFALSE;

		// Return the flags to it's original state
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
	}
}

LTFLOAT CharacterMovementState_WallWalking_MaxSpeed(const CharacterMovement *pMovement)
{
	const CharacterButes * const cbButes = pMovement->GetCharacterButes();
	const CharacterVars * const cvVars = pMovement->GetCharacterVars();

	const LTFLOAT fMod = (cvVars->m_dwCtrl & CM_FLAG_RUN) ? 2.0f : 1.0f;

	return cbButes->m_fBaseCrouchSpeed * cvVars->m_fCrouchSpeedMod * cvVars->m_fSpeedMod * fMod;
}

LTFLOAT CharacterMovementState_WallWalking_MaxAccel(const CharacterMovement *pMovement)
{
	const CharacterButes * const cbButes = pMovement->GetCharacterButes();
	const CharacterVars * const cvVars = pMovement->GetCharacterVars();

	const LTFLOAT fMod = (cvVars->m_dwCtrl & CM_FLAG_RUN) ? 2.0f : 1.0f;

	return cbButes->m_fBaseGroundAccel * cvVars->m_fCrouchSpeedMod * cvVars->m_fSpeedMod * fMod;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Climbing(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	INTERFACE *pInterface = pMovement->m_pInterface;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the appropriate size
	pMovement->SetObjectDims(CM_DIMS_DEFAULT);

	// Make sure gravity is turned off for the character
	pPhysics->SetFrictionCoefficient(cvVars->m_hObject, CHARACTERMOVEMENT_LADDER_FRICTION);
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags & ~FLAG_GRAVITY, LTFALSE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseGroundAccel * cvVars->m_fSpeedMod;
	LTBOOL bUseAiming = !((cvVars->m_dwCtrl & CM_FLAG_JUMP) || (cvVars->m_dwCtrl & CM_FLAG_DUCK) || (cvVars->m_dwCtrl & CM_FLAG_WALLWALK));
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_ALL, bUseAiming);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	if(fAccelMag)
	{
		LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBaseLadderSpeed, CM_DIR_FLAG_ALL);
	}
	else
	{
		// Dampen the velocity so we don't 'skid' across the ladder
		LTVector vVel = cvVars->m_vVelocity;
		vVel *= (0.75f * (1.0f - (cvVars->m_fFrameTime > 1.0f ? 1.0f : cvVars->m_fFrameTime)));
		pMovement->SetVelocity(vVel);
	}

	// If we're not within a ladder brush anymore... return to an idle state
	if(!pMovement->IsInContainerType(CC_LADDER))
	{
		// Return the gravity to it's original state
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetMovementState(CMS_IDLE);
		return;
	}
}

LTFLOAT CharacterMovementState_Climbing_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseLadderSpeed 
		 * pMovement->GetCharacterVars()->m_fSpeedMod;
}

LTFLOAT CharacterMovementState_Climbing_MaxAccel(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseGroundAccel 
		 * pMovement->GetCharacterVars()->m_fSpeedMod;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Swimming(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the appropriate size
	uint8 nDimsType = CM_DIMS_DEFAULT;
	if( cvVars->m_dwCtrl & CM_FLAG_DUCK )
		nDimsType = CM_DIMS_CROUCHED;

	pMovement->SetObjectDims(nDimsType);

	// Ignor the aiming restraints so we can move straight up and down if needed
	cvVars->m_bIgnorAimRestraints = LTTRUE;

	// Make sure gravity is turned off for the character
	pPhysics->SetFrictionCoefficient(cvVars->m_hObject, CHARACTERMOVEMENT_SWIMMING_FRICTION);
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags & ~FLAG_GRAVITY, LTFALSE);

	// Update the movement state based off of the containers...
	if(cbButes->m_bCanClimb && pMovement->IsInContainerType(CC_LADDER))
		{ pMovement->SetMovementState(CMS_CLIMBING); return; }


	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseLiquidAccel * cvVars->m_fSpeedMod;
	LTBOOL bUseAiming = !((cvVars->m_dwCtrl & CM_FLAG_JUMP) || (cvVars->m_dwCtrl & CM_FLAG_DUCK) || (cvVars->m_dwCtrl & CM_FLAG_WALLWALK));
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_ALL, bUseAiming);


	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBaseLiquidSpeed, CM_DIR_FLAG_ALL);


	// If we're not within a water brush anymore... return to an idle state
	if(!pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
	{
		// Return the gravity to it's original state
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetMovementState(CMS_IDLE);
		return;
	}
}

LTFLOAT CharacterMovementState_Swimming_MaxSpeed(const CharacterMovement *pMovement)
{
	const CharacterButes * const cbButes = pMovement->GetCharacterButes();
	const CharacterVars * const cvVars = pMovement->GetCharacterVars();

	return cbButes->m_fBaseLiquidSpeed;
}

LTFLOAT CharacterMovementState_Swimming_MaxAccel(const CharacterMovement *pMovement)
{
	const CharacterButes * const cbButes = pMovement->GetCharacterButes();
	const CharacterVars * const cvVars = pMovement->GetCharacterVars();

	return cbButes->m_fBaseLiquidAccel * cvVars->m_fSpeedMod;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Flying(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the appropriate size
	pMovement->SetObjectDims(CM_DIMS_DEFAULT);

	// Ignor the aiming restraints so we can move straight up and down if needed
	cvVars->m_bIgnorAimRestraints = LTTRUE;

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = cbButes->m_fBaseAirAccel * cvVars->m_fSpeedMod * cvVars->m_fRunSpeedMod;
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_ALL);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	LTFLOAT fVelMag = pMovement->UpdateVelocity(cbButes->m_fBaseRunSpeed * cvVars->m_fRunSpeedMod, CM_DIR_FLAG_ALL);

	// ----------------------------------------------------------------------- //
	// Check for a state change
	if(cvVars->m_dwFlags & FLAG_GRAVITY)
		{ pMovement->SetMovementState(CMS_PRE_FALLING); return; }
}

LTFLOAT CharacterMovementState_Flying_MaxSpeed(const CharacterMovement *pMovement)
{
	return   pMovement->GetCharacterButes()->m_fBaseRunSpeed
		   * pMovement->GetCharacterVars()->m_fRunSpeedMod
		   * pMovement->GetCharacterVars()->m_fSpeedMod;
}

LTFLOAT CharacterMovementState_Flying_MaxAccel(const CharacterMovement *pMovement)
{
	return   pMovement->GetCharacterButes()->m_fBaseAirAccel 
		   * pMovement->GetCharacterVars()->m_fRunSpeedMod
		   * pMovement->GetCharacterVars()->m_fSpeedMod;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Special(CharacterMovement *pMovement)
{

}

LTFLOAT CharacterMovementState_Special_MaxSpeed(const CharacterMovement *pMovement)
{
	return 0.0f;
}

LTFLOAT CharacterMovementState_Special_MaxAccel(const CharacterMovement *pMovement)
{
	return 0.0f;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Clipping(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterButes *cbButes = pMovement->GetCharacterButes();
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Ignor the aiming restraints so we can move straight up and down if needed
	cvVars->m_bIgnorAimRestraints = LTTRUE;

	// Make sure gravity and solid is turned off for the character
	pMovement->SetObjectFlags(OFT_Flags, 0, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, 0, LTFALSE);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR ACCELERATION
	LTFLOAT fAccel = 10000.0f;

#ifdef _CLIENTBUILD
	HCONSOLEVAR hVar;
	if(hVar = g_pLTClient->GetConsoleVar("ClipSpeedAccel"))
		fAccel = g_pLTClient->GetVarValueFloat(hVar);
#endif

	LTBOOL bUseAiming = !((cvVars->m_dwCtrl & CM_FLAG_JUMP) || (cvVars->m_dwCtrl & CM_FLAG_DUCK) || (cvVars->m_dwCtrl & CM_FLAG_WALLWALK));
	LTFLOAT fAccelMag = pMovement->UpdateAcceleration(fAccel, CM_DIR_FLAG_ALL, bUseAiming);

	// ----------------------------------------------------------------------- //
	// CALCULATE AND SET OUR VELOCITY
	if(fAccelMag)
	{
		LTFLOAT fVelCap = cbButes->m_fBaseLadderSpeed;

#ifdef _CLIENTBUILD
		if(hVar = g_pLTClient->GetConsoleVar("ClipSpeed"))
			fVelCap = g_pLTClient->GetVarValueFloat(hVar);
#endif

		LTFLOAT fVelMag = pMovement->UpdateVelocity(fVelCap, CM_DIR_FLAG_ALL);
	}
	else
	{
		// Stop the velocity so we don't 'skid'
		pMovement->SetVelocity(LTVector(0.0f, 0.0f, 0.0f));
	}
}

LTFLOAT CharacterMovementState_Clipping_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBaseLadderSpeed;
}

LTFLOAT CharacterMovementState_Clipping_MaxAccel(const CharacterMovement *pMovement)
{
	return 10000.0f;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Post_Clipping(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	MathLT *pMath = pMovement->m_pMath;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Return the flags to their original state
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);

	// Set the movement state to idle
	pMovement->SetMovementState(CMS_IDLE);

	// Give us a small 'push' so we fall down
	pMovement->SetAcceleration(LTVector(0.0f, 100.0f, 0.0f));
	pMovement->SetVelocity(LTVector(0.0f, 100.0f, 0.0f));


	// Reorient the character with the global gravity...
	LTVector vR, vU, vF;
	vU.Init(0.0f, 1.0f, 0.0f);

	if(vU.Equals(cvVars->m_vForward, 0.1f) || vU.Equals(-cvVars->m_vForward, 0.1f))
	{
		vF = vU.Cross(cvVars->m_vRight);
	}
	else
	{
		vR = vU.Cross(cvVars->m_vForward);
		vF = vR.Cross(vU);
	}

	LTRotation rRot;
	pMath->AlignRotation(rRot, vF, vU);
	pMovement->SetRot(rRot);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Pouncing(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	HOBJECT hObject = pMovement->GetObject();
	MathLT *pMath = pMovement->m_pMath;
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	INTERFACE *pInterface = pMovement->m_pInterface;
	CharacterVars *cvVars = pMovement->GetCharacterVars();
	CharacterButes *cbButes = pMovement->GetCharacterButes();

	// Make sure gravity is turned off for the character
	pMovement->SetObjectFlags(OFT_Flags, (cvVars->m_dwFlags & ~FLAG_GRAVITY) | FLAG_DONTFOLLOWSTANDING, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2 & ~(FLAG2_SEMISOLID | FLAG2_ORIENTMOVEMENT | FLAG2_SPHEREPHYSICS), LTFALSE);

	// Set their dims so that they don't clip into something.
	pMovement->SetObjectDims(CM_DIMS_POUNCING);

	// Get our time information
	LTFLOAT fTime = pInterface->GetTime();
	LTFLOAT fTimeDelta = fTime - cvVars->m_fStampTime;

	// Get the percent that we should be at across the entire
	// travel distance
	LTVector vDir = cvVars->m_vDestination - cvVars->m_vStart;
	LTFLOAT fDirMag = vDir.Mag();
	LTFLOAT fTotalTravelTime = fDirMag / cbButes->m_fBasePounceSpeed;
	LTFLOAT fPercent = fTimeDelta / fTotalTravelTime;

	// See if we're doine traveling the whole path or have changed containers
	if( (cvVars->m_fStampTime == -1.0f) || (fPercent >= 1.0f) || pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
	{
		// Reset our rotation
		LTRotation rRot;
		LTVector vUp(0.0f, 1.0f, 0.0f);
		vDir.y = 0.0f;
		pMath->AlignRotation(rRot, vDir, vUp);
		pMovement->SetRot(rRot);

		// Set our movement state to falling
		pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
		pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);

		if(pMovement->IsInLiquidContainer(CM_CONTAINER_MID))
		{
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2 & ~FLAG2_ORIENTMOVEMENT & ~FLAG2_SPHEREPHYSICS, LTFALSE);
			pMovement->SetMovementState(CMS_SWIMMING);
		}
		else if(cvVars->m_dwCtrl & CM_FLAG_WALLWALK)
		{
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2 | FLAG2_ORIENTMOVEMENT | FLAG2_SPHEREPHYSICS, LTFALSE);
			pMovement->SetObjectDims(CM_DIMS_WALLWALK);
			pMovement->SetMovementState(CMS_WALLWALKING);
		}
		else
		{
			pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2 & ~FLAG2_ORIENTMOVEMENT & ~FLAG2_SPHEREPHYSICS, LTFALSE);
			pMovement->SetObjectDims(CM_DIMS_STANDING);
			if(pMovement->GetMovementState() != CMS_FACEHUG_ATTACH)
				pMovement->SetMovementState(CMS_PRE_FALLING);
		}

		return;
	}


	// Get the point across the direction that we need as a reference
	LTVector vDirPos = cvVars->m_vStart + (vDir * fPercent);

	// Align a rotation down the path
	LTRotation rAlignRot;
	LTVector vR, vU, vF;
	pMath->GetRotationVectors(cvVars->m_rDestination, vR, vU, vF);

	// Get our arc offset (if we are not wall walking.
	LTFLOAT fArc = 0.0f;
	
	if( !(cvVars->m_dwCtrl & CM_FLAG_WALLWALK) )
		fArc = sinf(MATH_PI * fPercent);

	// Calculate the offset above the path
	LTVector vOffset = vU * (fArc * fDirMag * 0.1f);

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
		pMovement->SetPos(vNewPos,0);
	}


	// Update our rotation along the path
	vDir = vNewPos - vOldPos;
	vU.Init(0.0f, 1.0f, 0.0f);
	pMath->AlignRotation(rAlignRot, vDir, vU);
	pMovement->SetRot(rAlignRot);


	// Update our aiming angle so we're always looking at the destination
	pMath->GetRotationVectors(rAlignRot, vR, vU, vF);
	vDir = cvVars->m_vDestination - vNewPos;
	vDir.Norm();

	LTFLOAT theta = vDir.Dot(vF);//angle to rotate

	// Bounds check
	theta = theta < -1.0f ? -1.0f : theta;
	theta = theta > 1.0f ? 1.0f : theta;

	cvVars->m_fAimingPitch = MATH_RADIANS_TO_DEGREES(acosf(theta));
}

LTFLOAT CharacterMovementState_Pounce_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBasePounceSpeed * pMovement->GetCharacterVars()->m_fJumpSpeedMod;
}

LTFLOAT CharacterMovementState_Pounce_MaxAccel(const CharacterMovement *pMovement)
{
	return 0.0f;
}

LTRESULT CharacterMovementState_Pouncing_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	// m_fStampTime of -1.0f indicates that we should exist this movement state.
	if( !pMovement )
		return LT_ERROR;

	CharacterVars *cvVars = pMovement->GetCharacterVars();

	if( !cvVars )
		return LT_ERROR;


	// If the HPOLY is not against the direction of the pounce... then ignore it
	if(pInfo->m_hPoly != INVALID_HPOLY)
	{
		if(pInfo->m_Plane.m_Normal.Dot(cvVars->m_vVelocity) > 0.0f)
		{
			return LT_OK;
		}
	}

	// Only stop if we hit something solid.
	if( pInfo->m_hObject && (FLAG_SOLID & g_pInterface->GetObjectFlags(pInfo->m_hObject) ) )
	{
		cvVars->m_fStampTime = -1.0f;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Facehug(CharacterMovement *pMovement)
{
	CharacterMovementState_Pouncing(pMovement);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Facehug_Attach(CharacterMovement *pMovement)
{
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags, LTFALSE);
	pMovement->SetObjectFlags(OFT_Flags2, cvVars->m_dwFlags2, LTFALSE);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTRESULT CharacterMovementState_Facehug_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag)
{
	// m_fStampTime of -1.0f indicates that we should exist this movement state.

	if( !pMovement )
		return LT_ERROR;

	CharacterVars *cvVars = pMovement->GetCharacterVars();

	if( !cvVars )
		return LT_ERROR;

	// If the HPOLY is not against the direction of the pounce... then ignore it
	if(pInfo->m_hPoly != INVALID_HPOLY)
	{
		if(pInfo->m_Plane.m_Normal.Dot(cvVars->m_vVelocity) > 0.0f)
		{
			return LT_OK;
		}
	}

	// Only stop if we hit something solid.
	if( pInfo->m_hObject && (FLAG_SOLID & g_pInterface->GetObjectFlags(pInfo->m_hObject) ) )
	{
		cvVars->m_fStampTime = -1.0f;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTFLOAT CharacterMovementState_Facehug_MaxSpeed(const CharacterMovement *pMovement)
{
	return pMovement->GetCharacterButes()->m_fBasePounceSpeed * pMovement->GetCharacterVars()->m_fJumpSpeedMod;
}

LTFLOAT CharacterMovementState_Facehug_MaxAccel(const CharacterMovement *pMovement)
{
	return 0.0f;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CharacterMovementState_Observing(CharacterMovement *pMovement)
{
	// Setup some temporary attribute variables to make things clearer
	PHYSICS *pPhysics = pMovement->m_pPhysics;
	CharacterVars *cvVars = pMovement->GetCharacterVars();

	// Set the dims to the appropriate size
	pMovement->SetObjectDims(CM_DIMS_VERY_SMALL);

	// Clear our velocity and acceleration
	pMovement->SetVelocity(LTVector(0.0f, 0.0f, 0.0f));
	pMovement->SetAcceleration(LTVector(0.0f, 0.0f, 0.0f));

	// Make sure gravity and solid is turned off for the character
	pMovement->SetObjectFlags(OFT_Flags, cvVars->m_dwFlags & ~FLAG_GRAVITY & ~FLAG_SOLID | FLAG_GOTHRUWORLD, LTFALSE);
}

LTFLOAT CharacterMovementState_Observing_MaxSpeed(const CharacterMovement *pMovement)
{
	return 0.0f;
}

LTFLOAT CharacterMovementState_Observing_MaxAccel(const CharacterMovement *pMovement)
{
	return 0.0f;
}

