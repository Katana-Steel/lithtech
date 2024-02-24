// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMovementDefs.h
//
// PURPOSE : General character movement
//
// CREATED : 2/1/00
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_MOVEMENT_DEFS_H__
#define __CHARACTER_MOVEMENT_DEFS_H__

#include "iltmessage.h"

// ----------------------------------------------------------------------- //

enum CharacterMovementState
{
	CMS_MIN = 0,
	CMS_IDLE = 0,						// Generally when the velocity is zero
	CMS_WALKING,						// Walking in any direction *
	CMS_RUNNING,						// Running in any direction *
	CMS_PRE_JUMPING,					// Start the jump process **
	CMS_PRE_POUNCE_JUMP,				// Start the pounce jump process **
	CMS_POUNCE_JUMP,					// Increasing height **
	CMS_STAND_JUMP,						// Increasing height **
	CMS_WALK_JUMP,						// Increasing height **
	CMS_RUN_JUMP,						// Increasing height **
	CMS_WWALK_JUMP,						// Increasing height **
	CMS_PRE_FALLING,					// Start the falling process
	CMS_POUNCE_FALL,					// Decreasing height **
	CMS_STAND_FALL,						// Decreasing height **
	CMS_WALK_FALL,						// Decreasing height **
	CMS_RUN_FALL,						// Decreasing height **
	CMS_WWALK_FALL,						// Decreasing height **
	CMS_STAND_LAND,						// Landed on something after a falling state
	CMS_WALK_LAND,						// Landed on something after a falling state
	CMS_RUN_LAND,						// Landed on something after a falling state
	CMS_WWALK_LAND,						// Landed on something after a falling state
	CMS_PRE_CRAWLING,					// Start the crawling state
	CMS_CRAWLING,						// Ducking down or crawling
	CMS_POST_CRAWLING,					// After the crawling state is done
	CMS_WALLWALKING,					// Walk around the edge of the walls
	CMS_CLIMBING,						// Contained within a ladder brush
	CMS_SWIMMING,						// Swimming below the surface of a liquid
	CMS_FLYING,							// Free movement through the air
	CMS_SPECIAL,						// Any special movement state
	CMS_CLIPPING,						// Clip through geometry
	CMS_POST_CLIPPING,					// What to do after we stop clipping
	CMS_POUNCING,						// Pouncing alien attack
	CMS_FACEHUG,						// During the facehug attack
	CMS_FACEHUG_ATTACH,					// During the facehug implantation
	CMS_OBSERVING,						// The observation mode state
	CMS_MAX,
};

// * - Includes when crouching
// ** - According to facing and gravity


inline ILTMessage & operator<<(ILTMessage & out, CharacterMovementState x)
{
	out.WriteByte(x);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CharacterMovementState & x)
{
	x = CharacterMovementState(in.ReadByte());
	return in;
}

// ----------------------------------------------------------------------- //

#define CM_MAX_CONTAINERS				5
#define CM_NUM_CONTAINER_SECTIONS		3

#define CM_CONTAINER_ALL				-1
#define CM_CONTAINER_UPPER				0
#define CM_CONTAINER_MID				1
#define CM_CONTAINER_LOWER				2

// ----------------------------------------------------------------------- //

#define CM_FLAG_NONE					(0 << 0)
#define CM_FLAG_FORWARD					(1 << 0)
#define CM_FLAG_BACKWARD				(1 << 1)
#define CM_FLAG_LEFT					(1 << 2)
#define CM_FLAG_RIGHT					(1 << 3)
#define CM_FLAG_STRAFELEFT				(1 << 4)
#define CM_FLAG_STRAFERIGHT				(1 << 5)
#define CM_FLAG_RUN						(1 << 6)
#define CM_FLAG_DUCK					(1 << 7)
#define CM_FLAG_WALLWALK				(1 << 8)
#define CM_FLAG_JUMP					(1 << 9)
#define CM_FLAG_LOOKUP					(1 << 10)
#define CM_FLAG_LOOKDOWN				(1 << 11)
#define CM_FLAG_PRIMEFIRING				(1 << 12)
#define CM_FLAG_ALTFIRING				(1 << 13)
#define CM_KICK_BACK					(1 << 14)

#define CM_FLAG_DIRECTIONAL				(CM_FLAG_FORWARD | CM_FLAG_BACKWARD | CM_FLAG_STRAFELEFT | CM_FLAG_STRAFERIGHT)
#define CM_FLAG_TURNING					(CM_FLAG_LEFT | CM_FLAG_RIGHT)
#define CM_FLAG_AIMING					(CM_FLAG_LOOKUP | CM_FLAG_LOOKDOWN)
#define CM_FLAG_FIRING					(CM_FLAG_PRIMEFIRING | CM_FLAG_ALTFIRING)

// ----------------------------------------------------------------------- //

#define CM_DIR_FLAG_NONE				0x00
#define CM_DIR_FLAG_X_AXIS				0x01
#define CM_DIR_FLAG_Y_AXIS				0x02
#define CM_DIR_FLAG_Z_AXIS				0x04
#define CM_DIR_FLAG_XY_PLANE			0x03
#define CM_DIR_FLAG_XZ_PLANE			0x05
#define CM_DIR_FLAG_YZ_PLANE			0x06
#define CM_DIR_FLAG_ALL					0xFF

// ----------------------------------------------------------------------- //

#define CM_DIMS_DEFAULT					0
#define CM_DIMS_STANDING				0
#define CM_DIMS_CROUCHED				1
#define CM_DIMS_WALLWALK				2
#define CM_DIMS_VERY_SMALL				3
#define CM_DIMS_POUNCING				4

// ----------------------------------------------------------------------- //

#define SET_DIR_VELOCITY(value, dir, accel, friction)\
{\
	if(dir == 1)\
	{\
		if(value < 0.0f)\
			value += friction;\
		value += accel;\
	}\
	else if(dir == -1)\
	{\
		if(value > 0.0f)\
			value -= friction;\
		value -= accel;\
	}\
	else\
	{\
		if(value > 0.0f)\
		{\
			value -= friction;\
			if(value < 0.0f)\
				value = 0.0f;\
		}\
		else if(value < 0.0f)\
		{\
			value += friction;\
			if(value > 0.0f)\
				value = 0.0f;\
		}\
	}\
}

// ----------------------------------------------------------------------- //

#define CAP_DIR_VELOCITY(value, low, hi)\
{\
	if(value < low) value = low;\
	else if(value > hi) value = hi;\
}

// ----------------------------------------------------------------------- //

typedef struct t_CMContainerInfo
{
	// Container information about various character points
	uint32				m_nNumContainers;
	HOBJECT				m_hContainers[CM_MAX_CONTAINERS];

	// Physics update information
	LTFLOAT				m_fFriction;
	LTFLOAT				m_fViscosity;
	LTVector			m_vCurrent;

}	CMContainerInfo;

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ t_CMContainerInfo & x)
{
	out << x.m_nNumContainers;

	for(uint32 i = 0; (i < x.m_nNumContainers) && (i < CM_MAX_CONTAINERS); i++)
		out << x.m_hContainers[i];

	out << x.m_fFriction;
	out << x.m_fViscosity;
	out << x.m_vCurrent;

	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, t_CMContainerInfo & x)
{
	in >> x.m_nNumContainers;

	for(uint32 i = 0; (i < x.m_nNumContainers) && (i < CM_MAX_CONTAINERS); i++)
		in >> x.m_hContainers[i];

	in >> x.m_fFriction;
	in >> x.m_fViscosity;
	in >> x.m_vCurrent;

	return in;
}

// ----------------------------------------------------------------------- //

class CharacterAnimation;


typedef struct t_CharacterVars
{
	t_CharacterVars::t_CharacterVars();

	// The main object being controlled by the character movement class
	HOBJECT				m_hObject;						// The handle to the character object
	uint32				m_dwFlags;						// The character object flags
	uint32				m_dwFlags2;						// The character object flags 2
	uint32				m_dwUserFlags;					// The character object user flags
	uint32				m_dwClientFlags;				// The character object client flags

	// The main object orientation information
	LTVector			m_vPos;							// An updated variable of the character's position
	LTRotation			m_rRot;							// An updated variable of the character's rotation
	LTVector			m_vRight;						// Updated variables of the character's local axis
	LTVector			m_vUp;
	LTVector			m_vForward;

	// Directional velocity and acceleration save values
	LTVector			m_vVelocity;					// An updated variable of the character's velocity
	LTVector			m_vAccel;						// An updated variable of the character's acceleration
	LTFLOAT				m_fFriction;					// The friction coefficient applied to this character

	// Scale information
	LTFLOAT				m_fScale;						// The current scale for this character

	// Dimension information
	LTFLOAT				m_fResetDimsCheckTime;			// Time check for whether we should reset the dims or not
	LTBOOL				m_bIsCrouched;					// Is true if dims were set to crouch or wall walk.

	// Standing on information
	CollisionInfo		m_ciStandingOn;					// An updated collision of what the character is standing on
	LTFLOAT				m_fStandingOnCheckTime;			// Time check for whether we're actually standing on something new
	LTBOOL				m_bStandingOn;					// Are we really standing on something?
	LTBOOL				m_bLastStandingOn;				// Tells us if we were standing on something last frame.

	// Time variables
	LTFLOAT				m_fFrameTime;					// The time that has occured between frames

	// Main control variables
	uint8				m_nAllowMovement;				// Should we allow the character to move? (zero == can move)
	uint8				m_nAllowRotation;				// Should we allow the character to rotate? (zero == can move)
	uint32				m_dwCtrl;						// A set of control flags for the character (sharedmovement.h)
	LTBOOL				m_bAllowJump;					// Jump repeat supression boolean

	// Movement modification variables
	LTFLOAT				m_fSpeedMod;					// A scale for all ground speed values
	LTFLOAT				m_fWalkSpeedMod;				// A scale for the walk speed.
	LTFLOAT				m_fRunSpeedMod;					// A scale for the run speed.
	LTFLOAT				m_fCrouchSpeedMod;				// A scale for the crouch speed.
	LTFLOAT				m_fJumpSpeedMod;				// A scale for all jump speed values
	LTFLOAT				m_fFootstepVolume;				// A scale for the volume of the footsteps (0 to 1)

	// Rotation and aiming variables
	LTFLOAT				m_fTurnRate;					// Turning rate in degrees per second.
	LTFLOAT				m_fAimRate;						// Aiming rate in degrees per second.
	LTFLOAT				m_fAimingPitch;					// The current pitch rotation used for aiming
	LTFLOAT				m_fAimingYaw;					// The current yaw rotation used for aiming (waist turning)
	LTBOOL				m_bIgnorAimRestraints;			// Use 90 to -90 for the aim restraints despite the bute file values

	// State stamping information
	LTVector			m_vStampPos;					// A position stamp
	LTFLOAT				m_fStampTime;					// A time stamp

	// Movement override control variables
	LTVector			m_vStart;						// Start point of states that move the character on their own
	LTVector			m_vDestination;					// Destination of states that move the character on their own
	LTRotation			m_rStart;						// Rotation at m_vStart.
	LTRotation			m_rDestination;					// Rotation at m_vDestination.

	// Container information about various character points
	CMContainerInfo		m_pContainers[CM_NUM_CONTAINER_SECTIONS];
	LTBOOL				m_bContainersChanged;
	int					m_nForceContainerType;

	// Pointer to our character animation if one exists
	CharacterAnimation*	m_pCharAnim;					// Pointer to character animation object

	// Weight specific values
	LTBOOL				m_bEncumbered;					// Is this character encumbered in any way?

}	CharacterVars;

// ----------------------------------------------------------------------- //

inline t_CharacterVars::t_CharacterVars()
{
	memset(this, 0, sizeof(t_CharacterVars));

	m_rRot.Init();

	m_fScale			= 1.0f;
	m_fSpeedMod			= 1.0f;
	m_fWalkSpeedMod		= 1.0f;
	m_fRunSpeedMod		= 1.0f;
	m_fCrouchSpeedMod	= 1.0f;
	m_fJumpSpeedMod		= 1.0f;
	m_fFootstepVolume	= 1.0f;

	m_pCharAnim			= LTNULL;

	m_bAllowJump		= LTTRUE;

	m_nForceContainerType = -1;
}

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ LTPlane & x)
{
	out << x.m_Normal;
	out << x.m_Dist;

	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, LTPlane & x)
{
	in >> x.m_Normal;
	in >> x.m_Dist;

	return in;
}

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ CollisionInfo & x)
{
	out << x.m_Plane;
	out << x.m_hObject;
	out << x.m_hPoly;
	out << x.m_vStopVel;

	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CollisionInfo & x)
{
	in >> x.m_Plane;
	in >> x.m_hObject;
	in >> x.m_hPoly;
	in >> x.m_vStopVel;

	return in;
}


inline ILTMessage & operator<<(ILTMessage & out, /*const*/ t_CharacterVars & x)
{
	out << x.m_hObject;
	out << x.m_dwFlags;
	out << x.m_dwFlags2;
	out << x.m_dwUserFlags;
	out << x.m_dwClientFlags;

	out << x.m_vPos;
	out << x.m_rRot;
	out << x.m_vRight;
	out << x.m_vUp;
	out << x.m_vForward;

	out << x.m_vVelocity;
	out << x.m_vAccel;
	out << x.m_fFriction;

	out << x.m_fScale;

	out << x.m_fResetDimsCheckTime;
	out << x.m_bIsCrouched;

	out << x.m_ciStandingOn;
	out << x.m_fStandingOnCheckTime;
	out << x.m_bStandingOn;
	out << x.m_bLastStandingOn;

	out << x.m_fFrameTime;

	out << x.m_nAllowMovement;
	out << x.m_nAllowRotation;
	out << x.m_dwCtrl;

	out << x.m_fSpeedMod;
	out << x.m_fWalkSpeedMod;
	out << x.m_fRunSpeedMod;
	out << x.m_fCrouchSpeedMod;
	out << x.m_fJumpSpeedMod;
	out << x.m_fFootstepVolume;

	out << x.m_fTurnRate;
	out << x.m_fAimingPitch;
	out << x.m_fAimingYaw;
	out << x.m_bIgnorAimRestraints;

	out << x.m_vStampPos;
	out.WriteFloat(x.m_fStampTime - g_pInterface->GetTime());

	out << x.m_vStart;
	out << x.m_vDestination;
	out << x.m_rStart;
	out << x.m_rDestination;

	for(int i = 0; i < CM_NUM_CONTAINER_SECTIONS; i++)
		out << x.m_pContainers[i];

	out.WriteDWord(uint32(x.m_nForceContainerType));
	out << x.m_bContainersChanged;

	out << x.m_bEncumbered;

	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, t_CharacterVars & x)
{
	in >> x.m_hObject;
	in >> x.m_dwFlags;
	in >> x.m_dwFlags2;
	in >> x.m_dwUserFlags;
	in >> x.m_dwClientFlags;

	in >> x.m_vPos;
	in >> x.m_rRot;
	in >> x.m_vRight;
	in >> x.m_vUp;
	in >> x.m_vForward;

	in >> x.m_vVelocity;
	in >> x.m_vAccel;
	in >> x.m_fFriction;

	in >> x.m_fScale;

	in >> x.m_fResetDimsCheckTime;
	in >> x.m_bIsCrouched;

	in >> x.m_ciStandingOn;
	in >> x.m_fStandingOnCheckTime;
	in >> x.m_bStandingOn;
	in >> x.m_bLastStandingOn;

	in >> x.m_fFrameTime;

	in >> x.m_nAllowMovement;
	in >> x.m_nAllowRotation;
	in >> x.m_dwCtrl;

	in >> x.m_fSpeedMod;
	in >> x.m_fWalkSpeedMod;
	in >> x.m_fRunSpeedMod;
	in >> x.m_fCrouchSpeedMod;
	in >> x.m_fJumpSpeedMod;
	in >> x.m_fFootstepVolume;

	in >> x.m_fTurnRate;
	in >> x.m_fAimingPitch;
	in >> x.m_fAimingYaw;
	in >> x.m_bIgnorAimRestraints;

	in >> x.m_vStampPos;
	x.m_fStampTime = in.ReadFloat() + g_pInterface->GetTime();

	in >> x.m_vStart;
	in >> x.m_vDestination;
	in >> x.m_rStart;
	in >> x.m_rDestination;

	for(int i = 0; i < CM_NUM_CONTAINER_SECTIONS; i++)
		in >> x.m_pContainers[i];

	x.m_nForceContainerType = int(in.ReadDWord());
	in >> x.m_bContainersChanged;

	in >> x.m_bEncumbered;

	return in;
}

// ----------------------------------------------------------------------- //

#endif //__CHARACTER_MOVEMENT_DEFS_H__

