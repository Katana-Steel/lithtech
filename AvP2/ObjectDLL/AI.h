// ----------------------------------------------------------------------- //
//
// MODULE  : AI.h
//
// PURPOSE : Generic base AI object
//
// CREATED : 9/29/97
//
// ----------------------------------------------------------------------- //

#ifndef __AI_H__
#define __AI_H__

#include "Character.h"
#include "LTString.h"
#include "Timer.h"
#include "MusicMgr.h"

#include <string>
#include <deque>

class AIGroup;
class CAISense;
class CAISenseMgr;
class CAINode;
class CAITarget;
class TeleportPoint;
class CAIMovement;
class BidMgr;
class Bidder;
class Goal;
class CAttachmentPosition;
struct AIWBM_AIWeapon;
class CAINode;
class AIAction;
struct ParseLocation;
class CAIStrategyMove;

// Defines

#define AI_MAX_WEAPONS		(4)
#define AI_MAX_OBJECTS		(4)

extern const char g_szNone[];
extern const char g_szDefault[];
extern const char g_szSilent[];

// Classes

class CAI : public CCharacter
{
	public :

		typedef std::deque<const AIWBM_AIWeapon*> AvailableWeaponList;

		enum HackForInClassConstantsInVCC
		{
			g_nNumAIWeapons = 3
		};

		enum ActionType
		{
			ACTION_AIM,
			ACTION_BACKOFF,
			ACTION_BEZIERTO,
			ACTION_CROUCH,
			ACTION_CROUCHRELOAD,
			ACTION_DODGE,
			ACTION_DRAW,
			ACTION_DROP,
			ACTION_FACEHUG,
			ACTION_FIREMELEE,
			ACTION_FIREWEAPON,
			ACTION_HOLSTER,
			ACTION_IDLE,				// this only interrupts other actions
			ACTION_JUMPTO,
			ACTION_LADDERTO,
			ACTION_MOVETO,
			ACTION_MOVEFORWARD,
			ACTION_PLAYANIM,
			ACTION_POUNCE,
			ACTION_WWFORWARD,
			ACTION_WWJUMPTO,
			ACTION_RELOAD,
			ACTION_STUN,
			ACTION_TURNTO,
			NUM_ACTIONS
		};
						

	public : // Public methods

		// Ctors/Dtors/etc

		CAI();
		virtual ~CAI();

		// Object info

		HOBJECT	GetObject() const { return m_hObject; }
		int		GetTemplateId() const { return m_nTemplateId; }

		LTVector GetWeaponPosition(const CWeapon *pWeapon) const;
		const LTVector& GetEyePosition() const { return m_vEyePos; }
		const LTVector& GetEyeForward() const { return m_vEyeForward; }
		const LTRotation& GetEyeRotation() const { return m_rEyeRotation; }
		virtual LTVector GetAimFromPosition() const;

		LTBOOL IsVectorOnRightSide(const LTVector& vSrc, const LTVector& vDest, LTFLOAT* fVectorDistance = LTNULL);
		LTBOOL IsVectorOnLeftSide(const LTVector& vSrc, const LTVector& vDest, LTFLOAT* fVectorDistance = LTNULL) { return !IsVectorOnRightSide(vSrc, vDest, fVectorDistance); }

		LTBOOL HasDangerousWeapon();
		LTBOOL WillHurtFriend();

		LTBOOL CanUseCover()	 const { return m_bUseCover; }
		LTBOOL NeverLoseTarget() const { return m_bNeverLoseTarget; }

		void  SetNeverLoseTarget(LTBOOL bNeverLoseTarget) { m_bNeverLoseTarget = bNeverLoseTarget; }
		
		const std::string & GetUnreachableTargetCmd() const { return m_strUnreachableTargetCmd; }

		LTRotation GetFlashlightRot() const;  // overrides CCharacter's GetFlashlightRot.
		void UpdateFlashlight() const;  
		virtual void SetFlashlight(LTBOOL bOn);

		virtual void HandleInjured();

		// Visiblity. Note: these functions can fill in useful values (distance, dot product, direction) of the point/object in question,
		// in order to save you from having to recompute them yourself (since they can be expensive). HOWEVER, you cannot use the values
		// if the point/object is NOT visible (since it may have skipped out of the function before comuting some of the values). ALSO,
		// in the case of checking to see if an object is visible, there is a degenerate case of the Source Position being INSIDE the
		// object, which does return TRUE, although the values will be somewhat meaningless. They will be fudged to a distance of 0,
		// a dot product of 1, and a direction of the forward vector of the AI.
		virtual LTBOOL IsTargetVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL)  const;

		virtual LTBOOL IsObjectVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL)  const;
		virtual LTBOOL IsObjectVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL)  const;
		virtual LTBOOL IsObjectVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObject, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL)  const;

		virtual LTBOOL IsObjectPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL, LTVector* pvColor = LTNULL) const;
		virtual LTBOOL IsObjectPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL, LTVector* pvColor = LTNULL) const;
		virtual LTBOOL IsObjectPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObject, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL, LTVector* pvColor = LTNULL) const;

		virtual LTBOOL IsPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL)  const;
		virtual LTBOOL IsPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL)  const;
		virtual LTBOOL IsPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vFrom, const LTVector& vTo, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr = LTNULL, LTFLOAT* pfDp = LTNULL, LTVector* pvDir = LTNULL)  const;

		// Group methods

		// Goal methods
		virtual Goal * CreateGoal(const char * szGoalName);
		std::string GetCurrentSlotName() const;
		void RemoveGoal(const std::string & slot_name);
		void AddGoal(const std::string & slot_name, Goal * pGoalToAdd, LTFLOAT fModifiedBid = 0.0f);  // pGoalToAdd should be a goal created with CreateGoal.

		// Alignment/class methods

		virtual CharacterClass  GetCharacterClass() const { return m_cc; }
		virtual void			SetCharacterClass(CharacterClass cc) { m_cc = cc; }

		// Actions
		virtual LTBOOL CanDoNewAction() const;
		AIAction * SetNextAction(ActionType eAction);

		AIAction * GetCurrentAction()             { return m_pCurrentAction; }
		const AIAction * GetCurrentAction() const { return m_pCurrentAction; }

		// Animation stuff
		virtual uint32	GetMovementAnimFlags() const { return m_dwMovementAnimFlags; }
		
		// Senses functions

		CAISenseMgr & GetSenseMgr() { ASSERT(m_pSenseMgr); return *m_pSenseMgr; }
		const CAISenseMgr & GetSenseMgr() const { ASSERT(m_pSenseMgr); return *m_pSenseMgr; }

		void ReadSenseReactions(GenericProp &genProp);

		// Enemy Target stuff

		void Target(HOBJECT hObject, const LTVector & vLastKnownPos = LTVector(0,0,0));
		CAITarget & GetTarget() { return *m_pTarget; }
		const CAITarget & GetTarget() const { return *m_pTarget; }
		LTBOOL HasTarget() const;

		LTFLOAT GetLightLevel(LTVector vPos) const;

		// Static filter functions for intersect segments

		static LTBOOL DefaultFilterFn(HOBJECT hObj, void *pUserData);
		static LTBOOL ShootThroughFilterFn(HOBJECT hObj, void *pUserData);
		static LTBOOL ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData);
		static LTBOOL SeeFilterFn(HOBJECT hObj, void *pUserData);
		static LTBOOL SeeThroughFilterFn(HOBJECT hObj, void *pUserData);
		static LTBOOL SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData);

		LTBOOL CanSeeThrough()   const { return m_bSeeThrough; }
		LTBOOL CanShootThrough() const { return m_bShootThrough; }

		// Orientation/movement methods
		LTBOOL	GetMustCrawl() const { return m_bCrouchLock; }

		void OpenDoor(HOBJECT hDoor);
		void CloseDoor(HOBJECT hDoor);

		void Forward()		{ m_dwMovementFlags = (m_dwMovementFlags | CM_FLAG_FORWARD) & ~CM_FLAG_BACKWARD;   }
		void Reverse()		{ m_dwMovementFlags = (m_dwMovementFlags & ~CM_FLAG_FORWARD) | CM_FLAG_BACKWARD;   }

		void StrafeRight()	{ m_dwMovementFlags = (m_dwMovementFlags | CM_FLAG_STRAFERIGHT) & ~CM_FLAG_STRAFELEFT; }
		void StrafeLeft()	{ m_dwMovementFlags = (m_dwMovementFlags & ~CM_FLAG_STRAFERIGHT) | CM_FLAG_STRAFELEFT; }

		void TurnRight()	{ m_dwMovementFlags = (m_dwMovementFlags | CM_FLAG_RIGHT) & ~CM_FLAG_LEFT; }
		void TurnLeft()		{ m_dwMovementFlags = (m_dwMovementFlags & ~CM_FLAG_RIGHT) | CM_FLAG_LEFT; }

		void AimUp()		{ m_dwMovementFlags = (m_dwMovementFlags | CM_FLAG_LOOKUP) & ~CM_FLAG_LOOKDOWN; }
		void AimDown()		{ m_dwMovementFlags = (m_dwMovementFlags & ~CM_FLAG_LOOKUP) | CM_FLAG_LOOKDOWN; }

		void Run()			{ m_dwMovementFlags = m_dwMovementFlags | CM_FLAG_RUN; }
		void Walk()			{ if( !m_bRunLock ) m_dwMovementFlags = m_dwMovementFlags & ~CM_FLAG_RUN; }

		void Crouch()		{ m_dwMovementFlags = (m_dwMovementFlags & ~CM_FLAG_WALLWALK) | CM_FLAG_DUCK; }
		void WallWalk()		{ m_dwMovementFlags = (m_dwMovementFlags & ~CM_FLAG_DUCK) | CM_FLAG_WALLWALK; }
		void Jump()			{ m_dwMovementFlags = m_dwMovementFlags | CM_FLAG_JUMP; }
		void StandUp()		
		{ 
			m_dwMovementFlags = (m_dwMovementFlags & ~CM_FLAG_WALLWALK) & ~CM_FLAG_DUCK; 
			if( GetMustCrawl() || GetInjuredMovement()) 
				m_dwMovementFlags = m_dwMovementFlags | CM_FLAG_DUCK;
		}

		void ClearMovementFlags()  {    m_dwMovementFlags &= ~(   CM_FLAG_DIRECTIONAL 
																| CM_FLAG_TURNING 
																| CM_FLAG_AIMING 
																| CM_FLAG_FIRING 
																| CM_FLAG_JUMP    );  }


		// Stun effect
		virtual LTBOOL	IsStunned() const { return (GetCurrentAction() && GetCurrentAction() == m_aActions[ACTION_STUN]); }
		virtual void	HandleStunEffect();
		virtual void	UpdateStunEffect();

		virtual void	HandleEMPEffect();
		virtual void	UpdateEMPEffect();

		void Stop();
		bool Forward(const LTVector & limit);
		bool Reverse(const LTVector & limit);
		bool StrafeRight(const LTVector & limit);
		bool StrafeLeft(const LTVector & limit);

		bool Turn(const LTVector & facing_direction);
		bool TurnRight(LTVector limit);
		bool TurnLeft(LTVector limit);

		virtual bool AllowSnapTurn() const { return true; } // if false, snapturn will only be used for initial rotation and turn action will always be used.
		void SnapTurn(LTVector facing_direction);

		void AimAt(const LTVector & vAimToPos);
		void ClearAim();

		void JumpTo(const LTVector & vDest); // This will put the ai into a special jump state (CMS_SPECIAL).
		void ClipTo(const LTVector & vDest, const LTVector & vNormal);


		void SetMovementStyle(const char * pStyle = "Standard");

		CAIStrategyMove * GetMovePtr()             { return m_pStrategyMove; }
		const CAIStrategyMove * GetMovePtr() const { return m_pStrategyMove; }

		LTBOOL IsRechargingExosuit() const { return m_bRechargingExosuit; }

		// Weapon methods
		bool  FireWeapon();

		virtual CWeapon*    GetCurrentWeaponPtr()       { return GetAttachments() ? m_pCurrentWeapon : LTNULL; }
		// This can't be named GetCurrentWeaponPtr because CCharacter does not have a const version of it.
		const   CWeapon*    const_GetCurrentWeaponPtr()  const { return GetAttachments() ? m_pCurrentWeapon : LTNULL; } 

		void SetCurrentWeaponButesPtr(const AIWBM_AIWeapon * pButes) { m_pCurrentWeaponButes = pButes; }
		const AIWBM_AIWeapon * GetCurrentWeaponButesPtr() const { return m_pCurrentWeaponButes; }
		const AIWBM_AIWeapon * GetIdealWeaponButesPtr() const { return m_pIdealWeaponButes; }

		void GetWeaponRotation(LTRotation* rRot) const;

		void AddWeapon(const char * szAIWeaponName, int nAmountAmmo = -1);
		void SetWeapon(const char * szAIWeaponName, bool bAddIfNotAvailable = false);
		void ClearWeapons(LTBOOL bDropWeapon = LTFALSE);

		virtual void ChooseBestWeapon(LTFLOAT target_range_sqr);
		void ChooseBestWeapon(const LTVector & target_pos)  {	ChooseBestWeapon( (target_pos - GetPosition()).MagSqr() ); }

		virtual bool CanUseWeapon(const AIWBM_AIWeapon & weapon_bute);

		LTFLOAT GetAccuracy() const { return m_fAccuracy; }

		bool BurstDelayExpired();   // This returns true if the AI's burst delay has expired.

		bool OutOfAmmo() const;   // returns true if current weapon is out of ammo.
		bool EmptyClip() const;   // returns true if the current weapon has an empty clip.

		// Damage stuff

		void SetInvincible(LTBOOL bInvincible) { m_damage.SetCanDamage(!bInvincible); }

		virtual void	StartDeath();
		virtual std::string GetDefaultDeathAni();
		virtual LTBOOL	ForceDefaultDeathAni();

		// Influences

		LTBOOL	HasOddsFear()   const { return m_bOddsFear; }
		LTBOOL	HasDamageFear() const { return !m_tmrDamageFear.Stopped(); }
		HOBJECT GetDamagedFriend() const { return m_hFriend; }

		LTBOOL  IsAlert() const;

		// Model node control stuff (head following, etc.)
		void LookAt(HOBJECT m_hObject);
		void LookAt(const LTVector & vPos);

		// Pathing node stuff
		virtual bool CanUseNode(const CAINode & node) const;
		virtual bool CanOpenDoor() const { return true; }
		
		const CAINode * GetLastNodeReached() const		{ return m_pLastNode;  }
		void  SetLastNodeReached(const CAINode * pNode) { m_pLastNode = pNode; }

		const CAINode * GetNextNode() const			{ return m_pNextNode;  }
		void	SetNextNode(const CAINode * pNode)	{ m_pNextNode = pNode; }

		void Teleport(const ParseLocation & plocation);

		void SetLeash(LTFLOAT fLength, const LTVector & vPoint);
		void SetLeash(LTFLOAT fLength, const char * szObject);
		void Unleash() { SetLeash(0.0f,LTVector(0,0,0)); }
		bool WithinLeash(const LTVector & vPos) const;
		const LTVector & GetLeashPoint() const;

		// non-Active AI's are invisible, non-solid, and nearly non-existant.
		void Active(bool make_active, bool save_flags = true);

		// For CinematicTriggers
		void StartCinematic(HOBJECT cinematic_trigger_handle);
		void EndCinematic(LTBOOL bNotifyTrigger = LTTRUE);

		const LTSmartLink & GetCinematicTrigger() const { return m_hCinematicTrigger; }

		// Misc.

		uint32 GetMovementFlags() const { return m_dwMovementFlags; }

		virtual std::string GetCharacterSetName(const std::string & root_name) const { return root_name + std::string("_AI"); }

		// Animation control
		virtual	WEAPON*	UpdateWeaponAnimation();
		virtual void	UpdateActionAnimation();

	protected : // Protected methods

		// Update methods

		virtual void PreUpdate();
		virtual void Update();
		virtual void PostUpdate();

		void UpdateEye();
		void UpdateBids();
		void UpdateInfluences();
		virtual void UpdateOnGround();
		void UpdateMusic();

		virtual CMusicMgr::EMood GetMusicMoodModifier();

		// Handler methods

		virtual void HandleTouch(HOBJECT hObj);
		virtual void HandleModelString(ArgList* pArgList);

		// Engine methods

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);
		
		virtual LTBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

		virtual void	ProcessAnimationMsg(uint8 nTracker, uint8 bLooping, uint8 nMsg);
		virtual LTBOOL	ProcessCommand(const char* const * pTokens, int nArgs);
		virtual LTBOOL	IgnoreDamageMsg(const DamageStruct & damage_msg);
		virtual void	ProcessDamageMsg(const DamageStruct & damage_msg);

		virtual void	InitialUpdate();
		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		virtual void	CacheFiles();

		virtual void	FirstUpdate();

		void UpdateAiming();

		// Weapon stuff
		void SetWeapon(int nWeaponOffset);

		AvailableWeaponList::iterator BeginAvailableWeapons()  { return m_AvailableWeapons.begin(); }
		AvailableWeaponList::iterator EndAvailableWeapons()    { return m_AvailableWeapons.end(); }


		// Precomputation stuff

		virtual bool Consistent();  // Used to make Sanity checks on the AI (like does character type match attribute template).

	private :

		// This should only be called by CAI::SetNextAction()
		AIAction * GetAction(ActionType eAction);

		// Attributes

		int				m_nTemplateId;				// ID number of template.
		CharacterClass	m_cc;						// Our character class.
		std::string		m_strWeaponNames[g_nNumAIWeapons];			// Stores name of weapon from readprop until initial update is reached.

		LTFLOAT			m_fUpdateDelta;		// Our update frequency.

		// Misc

		LTBOOL		m_bLocked;					// Are we doing something that preempts normal updates? (ie long recoil)
		LTBOOL		m_bFirstUpdate;				// Is this before/during our first update?
		std::string m_strQueuedCommands;			// Our queued commands
		
		LTBOOL		m_bUseCover;				// Should this AI use cover points, if available?
		LTBOOL		m_bUseShoulderLamp;			// If TRUE, this AI will have a shoulder lamp attachment
		LTBOOL		m_bNeverLoseTarget;			// If true, the AI will not lose their current target.

		LTBOOL		m_bHasLATarget;				// Do we have a look at target to keep track of?
		HOBJECT		m_hLATarget;				// Our smart link to the Look At target.  This cannot be a smart link!
		
		LTFLOAT		m_fLastAimAtTime;            // Used by GetFlashlightRot.

		std::string m_strDefaultDeathAnim;       // Is set to the death animation to use if we have begin given a g_szSetDeathAnim command.

		// Active stuff (used by CAI::Active to make the AI active or in-active).
		LTBOOL		m_bStartActive;				// If true the AI should start active.
		LTBOOL		m_bActive;					// True if AI is active, false if AI is not active.
		uint32		m_dwActiveFlags;			// If AI is not activated this stores the old object flag values, otherwise this is set to zero.
		uint32		m_dwActiveFlags2;			// If AI is not activated this stores the old object flag2 values, otherwise this is set to zero.
		uint32		m_dwActiveUserFlags;		// If AI is not activated this stores the old object user flag values, otherwise this is set to zero.
		LTBOOL		m_bActiveCanDamage;			// If AI is not activated this stores the AI's old can damage status.

		uint32		m_dwActiveHeadFlags;			// If exosuited AI is not activated this stores the old object flag values, otherwise this is set to zero.
		uint32		m_dwActiveHeadFlags2;			// If exosuited AI is not activated this stores the old object flag2 values, otherwise this is set to zero.
		uint32		m_dwActiveHeadUserFlags;		// If exosuited AI is not activated this stores the old object user flag values, otherwise this is set to zero.

		// Navigation
		uint32		m_dwMovementFlags;			// Flags which to be sent to CharacterMovement.
		LTBOOL		m_bWasStandingOn;		    // Used to determine when we change "standing-on" states.
		LTVector	m_vLastStandingPos;		// Used to determine if we fell very far.

		uint32		m_dwMovementAnimFlags;   // Allows movement animations to be played without using character movement.

		CAIStrategyMove * m_pStrategyMove;   // AI's have only one instance of a move strategy so that it will stay smooth between different paths.

		// Weapons
		AvailableWeaponList			m_AvailableWeapons;					// Weapons this AI can use to attack player.
		CAttachmentPosition*		m_pWeaponPosition;						// An array of pointers to the attachemnts for the weapons
		int							m_nCurrentWeapon;						// The weapon current selected by the AI, index into m_AvailableWeapons.
		CWeapon*					m_pCurrentWeapon;
		const AIWBM_AIWeapon*		m_pCurrentWeaponButes;					// Pointer to data from CAIWeaponButeMgr for current weapon.
		const AIWBM_AIWeapon*		m_pIdealWeaponButes;					// Pointer to data from CAIWeaponButeMgr for ideal weapon (first weapon in m_AvailableWeapons).
		CTimer						m_tmrBurstDelay;						// Is on if waiting for delay to end.
		CTimer						m_tmrLastBurst;						    // Used to determine how long since last burst ended.
		LTVector					m_vAimToPos;							// The position we are trying to aim at.
		LTBOOL						m_bFullyAimed;							// True if the last UpdateAim fully rotated to the desired position.

		// Object information

		LTVector		m_vEyePos;					// The position of our eye
		LTVector		m_vEyeForward;				// Our eye's forward vector
		LTRotation		m_rEyeRotation;				// Our eye's rotation.
		LTFLOAT			m_fHalfForwardFOV;				// Cosine of our forward field of view arc.
		LTFLOAT			m_fHalfUpNonFOV;				// Cosine of cone which is our blind spot above our head.

		DRotation	m_rTargetRot;				// Our target rotation
		DRotation	m_rStartRot;				// Our starting rotation
		LTVector		m_vTargetRight;				// Object's target right vector
		LTVector		m_vTargetUp;				// Object's target up vector
		LTVector		m_vTargetForward;			// Object's target forward vector
		LTBOOL		m_bRotating;				// Are we rotating?
		LTFLOAT		m_fRotationTime;			// Time for our rotation interpolation
		LTFLOAT		m_fRotationTimer;			// Timer for our rotation interpolation
		LTFLOAT		m_fRotationSpeed;			// Our rotation speed

		// Target

		CAITarget*	m_pTarget;					// Enemy Target
		LTBOOL		m_bShootThrough;			// Do we shoot through shoot-throughables at our target?
		LTBOOL		m_bSeeThrough;				// Do we see through see-throughables at any stimuli? (VERY EXPENSIVE!!!)

		// Accuracy/lag modififers
        LTFLOAT     m_fAccuracy;                 // Multiplier for the weapon path perturb.  Should be between 0 and 1.

		// Start up
		std::string		m_strInitialNode;        // AI will teleport to this node.

		// Commands

		LTString		m_hstrCmdInitial;			// Initial command (one-shot)
		LTString		m_hstrCmdActivateOn;		// ActivateOn command
		LTString		m_hstrCmdActivateOff;		// ActivateOff command
		std::string		m_strUnreachableTargetCmd;  // Called when an attack goal has never-lose-target but cannot reach target.

		// Activation

		LTBOOL		m_bActivated;				// Is our activation ON or OFF

		// Senses

		CAISenseMgr*	m_pSenseMgr;			// Sense manager

		// Influences
		CTimer			m_tmrInfluence;		// limits frequency of influence updates
		LTBOOL			m_bOddsFear;		// Is the AI scared of the enemy:friend ratio?
		LTSmartLink		m_hFriend;			// AI requesting help
		LTFLOAT			m_fFriendDistSqr;	// distance to m_hFriend
		LTBOOL			m_bFriendLocked;
		CTimer			m_tmrDamageFear;
		LTFLOAT			m_fFraction;

		// CinematicTriggers
		LTSmartLink		m_hCinematicTrigger;

		// Special movement.
		LTBOOL			m_bUsingJumpTo;        // These are needed to reset the CharacterMovement for save/load.
		LTBOOL			m_bUsingClipTo;

		// Exosuit stuff
		LTFLOAT			m_fStartRechargeLevel;  // Stop movement to recharge when energy level falls below this percentage.
		LTFLOAT			m_fStopRechargeLevel;   // Movement can be turned back on when energy level has risen above this percentage.
												// Set this value to zero to ignore exosuit energy.
		LTBOOL			m_bRechargingExosuit;   // If true, movement is disable to allow suit to recharge.

		// Actions
		AIAction*	m_pNextAction;      // Set to non-null if there is a next action to perform.
		AIAction*	m_pCurrentAction;   // The current action being performed (may be null).

		AIAction *	m_aActions[NUM_ACTIONS];	// The array of instantiated actions

		// Pathing nodes
		const CAINode *   m_pLastNode;   // The last node we reached.
		const CAINode *   m_pNextNode;   // The node we're trying to reach.

		LTFLOAT			m_fLeashLengthSqr;  // Don't move more than this distance from the LeashFrom point.  Set less than zero to keep unleashed.
		LTVector		m_vLeashFrom;		// The point to be leashed from.
		std::string		m_strLeashFromName;

		// Goals
		BidMgr	*   m_pBidMgr;      // Contains all bids and their names.
		Bidder  *   m_pCurrentBid;     // The last bidder we chose, points to a bidder in bidMgr.

		// Music mood modifiers
		std::string		m_strMusicAmbientTrackSet;
		std::string		m_strMusicWarningTrackSet;
		std::string		m_strMusicHostileTrackSet;

		// Level designer over-rides.
		LTBOOL		m_bAlertLock;
		LTBOOL		m_bRunLock;
		LTBOOL		m_bCrouchLock;
		LTBOOL		m_bForceFlashlightOn;

		// Debug stuff

		DDWORD		m_dwLastLoadFlags;			// Game restore flags


#ifndef _FINAL
		std::string m_strCurrentInfo;
		std::string m_strNameInfo;
		std::string m_strGoalInfo;
		std::string m_strActionInfo;
		std::string m_strTargetInfo;
		std::string m_strScriptInfo;
#endif
};

class CAIPlugin : public IObjectPlugin
{
	public:

		virtual ~CAIPlugin() {}

		virtual DRESULT	PreHook_EditStringList(const char* szRezPath, const char* szPropName, char* const * aszStrings, DDWORD* pcStrings, const DDWORD cMaxStrings, const DDWORD cMaxStringLength);

		virtual bool IsValidCharacterClass(CharacterClass x) const { return false; }

		virtual bool CanUseWeapon(const AIWBM_AIWeapon & butes) const;

	private :
		
};

#endif // __AI_H__
