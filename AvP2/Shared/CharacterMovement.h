// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterMovement.h
//
// PURPOSE : General character movement
//
// CREATED : 2/1/00
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_MOVEMENT_H__
#define __CHARACTER_MOVEMENT_H__

// ----------------------------------------------------------------------- //
#include "CharacterButeMgr.h"
#include "CharacterMovementDefs.h"

// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD
	#include "clientheaders.h"
#else
	#include "serverheaders.h"
#endif

// ----------------------------------------------------------------------- //

#ifdef _CLIENTBUILD
	#define INTERFACE	CClientDE
	#define PHYSICS		ILTClientPhysics
#else
	#define INTERFACE	CServerDE
	#define PHYSICS		ILTPhysics
#endif

// ----------------------------------------------------------------------- //

class CharacterMovement
{
	public:
		// Constructor and destructors for our movement
		CharacterMovement();
		virtual ~CharacterMovement();

		// Initialization and destruction functions
		virtual LTBOOL	Init(INTERFACE *pInterface, HOBJECT hObj, CharacterAnimation* pAnim=LTNULL);
		virtual void	Term();

#ifndef _CLIENTBUILD
		// Loading and saving of the character movement class
		virtual void	Load(HMESSAGEREAD hRead);
		virtual void	Save(HMESSAGEWRITE hWrite);
#endif

#ifdef _CLIENTBUILD
		// To create the object
		virtual LTBOOL	CreateObject(uint32 dwFlags = 0, uint32 dwFlags2 = 0);
		virtual void	TermObject();
#endif

		// Client/Server message control: Handles message from the server or client
		virtual LTBOOL	HandleMessageRead(uint8 nType, HMESSAGEREAD hRead, uint32 dwFlags = 0);
		virtual LTBOOL	HandleMessageWrite(uint8 nType, HMESSAGEWRITE hWrite, uint32 dwFlags = 0);

		// Update the current state
		virtual void	Update(LTFLOAT fDeltaTime);								// Updates the character's movement state machine

		// Update certain aspects of the character movement
		virtual void	UpdateAllowMovement();
		virtual void	UpdateAllowRotation();
		virtual LTFLOAT	UpdateAcceleration(LTFLOAT fAccel, uint32 dwFlags, LTBOOL bAiming = LTFALSE);	// Returns true if acceleration is non-zero
		virtual LTFLOAT	UpdateVelocity(LTFLOAT fMaxVel, uint32 dwFlags);		// Returns true if velocity is the maximum

		// Update items the character is contained in
		virtual LTBOOL	UpdateContainers();
		virtual LTBOOL	UpdateContainerPhysics();

		virtual LTBOOL	IsInContainerType(int nContainerCode, int nSection = CM_CONTAINER_ALL) const;
		virtual LTBOOL	IsInLiquidContainer(int nSection = CM_CONTAINER_ALL) const;


#ifdef _CLIENTBUILD
		// Update the object according to other objects moving and rotating
		virtual LTRESULT	OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector &vPos);
		virtual LTRESULT	OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation &rRot);
#endif

		// Other engine message handling functions
		virtual LTRESULT	OnTouchNotify(CollisionInfo *pInfo, LTFLOAT fForceMag);

		// ----------------------------------------------------------------------- //
		// Data modification functions

		void	AllowMovement(LTBOOL bCanMove);
		void	AllowMovementReset()							{ m_Vars.m_nAllowMovement = 0; }
		void	AllowRotation(LTBOOL bCanRotate);
		void	AllowRotationReset()							{ m_Vars.m_nAllowRotation = 0; }

		void	AimingReset()									{ m_Vars.m_fAimingPitch = 0; m_Vars.m_fAimingYaw = 0; }

		void		SetObject(HOBJECT hObject)					{ m_Vars.m_hObject = hObject; }

		LTVector	SetObjectDims(uint8 nType, HOBJECT hObj = LTNULL, LTBOOL bForceDims = LTFALSE);	// Set dims based off CM_DIMS_ types; returns the sized dims
		LTVector	SetObjectDims(LTVector &vDims, HOBJECT hObj = LTNULL, LTBOOL bForceDims = LTFALSE);
		LTBOOL		CheckDimsSpace(LTVector vDims);

		void		SetScale(LTFLOAT fScale);
		void		SetControlFlags(uint32 dwFlags);

		void	SetObjectFlags(ObjFlagType nType, uint32 dwFlags = 0, LTBOOL bSave = LTTRUE);	// Sets the character object flags
		void	SetObjectUserFlags(uint32 dwFlags = 0, LTBOOL bSave = LTTRUE);					// Sets the character object's user flags
		void	SetObjectClientFlags(uint32 dwFlags = 0, LTBOOL bSave = LTTRUE);				// Sets the character object's client flags

		void	SetCharacterButes(const char *szName, LTBOOL bUseMPDir=LTTRUE);	// Sets character variables according to attrib file
		void	SetCharacterButes(int nType, LTBOOL bUseMPDir=LTTRUE);			// Sets character variables according to attrib file

		void	SetMovementState(CharacterMovementState cms);		// Set which movement function to call

		void	SetPos(const LTVector &vPos, uint32 dwFlags = 0);	// Updates the position of the character object
		void	SetRot(const LTVector &vRot);						// Updates the rotation of the character object (euler angles)
		void	SetRot(const LTRotation &rRot);						// Updates the rotation of the character object

		void	SetVelocity(const LTVector &vVel, LTBOOL bSaveOnly = LTFALSE);			// Sets the velocity of the character object
		void	SetAcceleration(const LTVector &vAccel, LTBOOL bSaveOnly = LTFALSE);	// Sets the acceleration of the character object

		void	SetPositionStamp(const LTVector &vPos)				{ m_Vars.m_vStampPos = vPos; }
		void	SetTimeStamp(LTFLOAT fTime)							{ m_Vars.m_fStampTime = fTime; }

		void	SetStart(const LTVector &vStart, const LTRotation & rStart = LTRotation(0,0,0,1))
		{
			m_Vars.m_vStart = vStart;
			m_Vars.m_rStart = rStart;
		}
		void	SetDestination(const LTVector &vDest, const LTRotation & rDest = LTRotation(0,0,0,1))
		{
			m_Vars.m_vDestination = vDest;
			m_Vars.m_rDestination = rDest;
		}

		void	SetStandingOn(HPOLY hPoly);

		void	SetEncumbered(LTBOOL bEncumbered)				{ m_Vars.m_bEncumbered = bEncumbered; }

		// ----------------------------------------------------------------------- //
		// Data retrieval functions

		LTBOOL		CanMove()									{ return (m_Vars.m_nAllowMovement == 0); }
		LTBOOL		CanRotate()									{ return (m_Vars.m_nAllowRotation == 0); }

		HOBJECT		GetObject() const							{ return m_Vars.m_hObject; }
		LTVector	GetDimsSetting(uint8 nType);				// Retrieves a dims setting value
		LTVector	GetObjectDims(HOBJECT hObj = LTNULL, LTBOOL bFull = LTFALSE) const;
		LTFLOAT		GetScale() const;
		uint32		GetControlFlags()	const					{ return m_Vars.m_dwCtrl; }

		uint32		GetObjectFlags(ObjFlagType nType) const;	// Sets the character object flags
		uint32		GetObjectUserFlags() const;					// Sets the character object's user flags
		uint32		GetObjectClientFlags() const;				// Sets the character object's client flags

		CharacterButes*			GetCharacterButes()				{ return &m_Butes; }
		const CharacterButes*	GetCharacterButes()	const 		{ return &m_Butes; }
		CharacterVars*			GetCharacterVars()				{ return &m_Vars; }
		const CharacterVars*	GetCharacterVars()	const		{ return &m_Vars; }

		CharacterMovementState	GetMovementState()		const	{ return m_cmsCurrent; }
		CharacterMovementState	GetLastMovementState()	const	{ return m_cmsLast; }

		char*	GetMovementStateName(CharacterMovementState cms) const;

		void	GetPos(LTVector &vPos) const					{ vPos = m_Vars.m_vPos; }
		void	GetRot(LTVector &vRot) const;					// Retrieves the rotation of the character object (euler angles)
		void	GetRot(LTRotation &rRot)	const				{ rRot = m_Vars.m_rRot; }
		void	GetRotVectors(LTVector &vRight, LTVector &vUp, LTVector &vForward) const
		{
			vRight = m_Vars.m_vRight; vUp = m_Vars.m_vUp; vForward = m_Vars.m_vForward; 
		}

		void	GetVelocity(LTVector &vVel) const				{ vVel = m_Vars.m_vVelocity; }
		void	GetAcceleration(LTVector &vAccel) const			{ vAccel = m_Vars.m_vAccel; }

		const LTVector   & GetPos() const						{ return m_Vars.m_vPos; }
		const LTRotation & GetRot() const						{ return m_Vars.m_rRot; }
		const LTVector	& GetRight() const						{ return m_Vars.m_vRight; }
		const LTVector	& GetUp() const							{ return m_Vars.m_vUp; }
		const LTVector	& GetForward() const					{ return m_Vars.m_vForward; }

		const LTVector	& GetVelocity() const					{ return m_Vars.m_vVelocity; }
		const LTVector	& GetAcceleration() const				{ return m_Vars.m_vAccel; }

		// Gets the maximum speed or acceleration for the requested state.
		LTFLOAT GetMaxSpeed(CharacterMovementState cms) const;
		LTFLOAT GetMaxAccel(CharacterMovementState cms) const;

		// Gets the maximum speed or acceleration for the current state.
		LTFLOAT GetMaxSpeed() const { return GetMaxSpeed(m_cmsCurrent); }
		LTFLOAT GetMaxAccel() const { return GetMaxAccel(m_cmsCurrent); }

		// Simple helper functions
		void AllowJump(LTBOOL bAllow=LTTRUE) { m_Vars.m_bAllowJump = bAllow; }

		LTBOOL	IsStandingOn() const		{ return m_Vars.m_bStandingOn; }
		LTBOOL	IsCrouched()   const		{ return m_Vars.m_bIsCrouched; }

		// ----------------------------------------------------------------------- //
		// Function modification functions

		// Set a new state handling function
		void	SetHandleStateFunc(CharacterMovementState cms, void (*fp)(CharacterMovement*));

		// Set a new touch notify function pointer
		void	SetTouchNotifyFunc(CharacterMovementState cms, LTRESULT (*fp)(CharacterMovement*, CollisionInfo*, LTFLOAT));

		// Set a new max speed/accel handling function
		void	SetMaxSpeedFunc(CharacterMovementState cms, LTFLOAT (*fp)(const CharacterMovement*));
		void	SetMaxAccelFunc(CharacterMovementState cms, LTFLOAT (*fp)(const CharacterMovement*));

		// Set a new message handling function
		void	SetHandleMessageReadFunc(LTBOOL (*fp)(uint8, HMESSAGEREAD, uint32));
		void	SetHandleMessageWriteFunc(LTBOOL (*fp)(uint8, HMESSAGEWRITE, uint32));

		// Set container information retrieval function
		void	SetContainerPhysicsInfoFunc(LTBOOL (*fp)(HOBJECT, CMContainerInfo&));

	protected:

		// Helper update functions
		virtual void	UpdateStandingOn();						// Updates the character's standing on variables
		virtual void	UpdateRotation();						// Updates the character's rotation
		virtual void	UpdateAiming();							// Updates the character's aiming direction


	// ----------------------------------------------------------------------- //
	// Member variables

	public:

		// The main interfaces to the engine functionality
		INTERFACE			*m_pInterface;
		PHYSICS				*m_pPhysics;
		CommonLT			*m_pCommon;
		MathLT				*m_pMath;


		// Object information for crouch states
		HOBJECT				m_hDimsObject;					// Object to be used for dims checks


	protected:

		// Movement attributes for this character
		CharacterButes				m_Butes;				// The movement attributes
		CharacterVars				m_Vars;					// The movement variables


		// Character movement states
		CharacterMovementState		m_cmsLast;				// The last character movement state
		CharacterMovementState		m_cmsCurrent;			// The current character movement state


		// The list of movement functions tied to the states
		void (*m_fpHandleState[CMS_MAX])(CharacterMovement*);
		LTRESULT(*m_fpTouchNotify[CMS_MAX])(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag);
		LTFLOAT (*m_fpMaxSpeed[CMS_MAX])(const CharacterMovement*);
		LTFLOAT (*m_fpMaxAccel[CMS_MAX])(const CharacterMovement*);


		// External message handling function pointer
		LTBOOL (*m_fpHandleMessageRead)(uint8 nType, HMESSAGEREAD hRead, uint32 dwFlags);
		LTBOOL (*m_fpHandleMessageWrite)(uint8 nType, HMESSAGEWRITE hWrite, uint32 dwFlags);


		// External function to handle retrieval of container physics information
		LTBOOL (*m_fpHandleContainerPhysicsInfo)(HOBJECT hObj, CMContainerInfo& pInfo);
};


// ----------------------------------------------------------------------- //
// Movement state functions

void CharacterMovementState_Idle(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Idle_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Idle_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Walking(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Walking_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Walking_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Running(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Running_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Running_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Pre_Jumping(CharacterMovement *pMovement);
void CharacterMovementState_Jump(CharacterMovement *pMovement, LTFLOAT fMaxVel = 0.0f);
LTRESULT CharacterMovementState_Jump_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag);
void CharacterMovementState_Stand_Jump(CharacterMovement *pMovement);
void CharacterMovementState_Walk_Jump(CharacterMovement *pMovement);
void CharacterMovementState_Run_Jump(CharacterMovement *pMovement);
void CharacterMovementState_WWalk_Jump(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Jumping_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Jumping_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Pre_Falling(CharacterMovement *pMovement);
void CharacterMovementState_Fall(CharacterMovement *pMovement, LTFLOAT fMaxVel = 0.0f, CharacterMovementState cms = CMS_STAND_FALL);
LTRESULT CharacterMovementState_Fall_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag);
void CharacterMovementState_Stand_Fall(CharacterMovement *pMovement);
void CharacterMovementState_Walk_Fall(CharacterMovement *pMovement);
void CharacterMovementState_Run_Fall(CharacterMovement *pMovement);
void CharacterMovementState_WWalk_Fall(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Falling_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Falling_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Land(CharacterMovement *pMovement, LTFLOAT fMaxVel = 0.0f, CharacterMovementState cms = CMS_STAND_LAND);
void CharacterMovementState_Stand_Land(CharacterMovement *pMovement);
void CharacterMovementState_Walk_Land(CharacterMovement *pMovement);
void CharacterMovementState_Run_Land(CharacterMovement *pMovement);
void CharacterMovementState_WWalk_Land(CharacterMovement *pMovement);

void CharacterMovementState_Pre_PounceJump(CharacterMovement *pMovement);
void CharacterMovementState_Pounce_Jump(CharacterMovement *pMovement);
LTRESULT CharacterMovementState_Pounce_Jump_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag);

void CharacterMovementState_Pounce_Fall(CharacterMovement *pMovement);

void CharacterMovementState_Pre_Crawling(CharacterMovement *pMovement);
void CharacterMovementState_Crawling(CharacterMovement *pMovement);
void CharacterMovementState_Post_Crawling(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Crawling_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Crawling_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_WallWalking(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_WallWalking_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_WallWalking_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Climbing(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Climbing_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Climbing_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Swimming(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Swimming_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Swimming_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Flying(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Flying_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Flying_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Special(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Special_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Special_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Clipping(CharacterMovement *pMovement);
void CharacterMovementState_Post_Clipping(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Clipping_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Clipping_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Pouncing(CharacterMovement *pMovement);
LTRESULT CharacterMovementState_Pouncing_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag);
LTFLOAT CharacterMovementState_Pounce_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Pounce_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Facehug(CharacterMovement *pMovement);
void CharacterMovementState_Facehug_Attach(CharacterMovement *pMovement);
LTRESULT CharacterMovementState_Facehug_TN(CharacterMovement *pMovement, CollisionInfo *pInfo, LTFLOAT fForceMag);
LTFLOAT CharacterMovementState_Facehug_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Facehug_MaxAccel(const CharacterMovement *pMovement);

void CharacterMovementState_Observing(CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Observing_MaxSpeed(const CharacterMovement *pMovement);
LTFLOAT CharacterMovementState_Observing_MaxAccel(const CharacterMovement *pMovement);

#endif //__CHARACTER_MOVEMENT_H__

