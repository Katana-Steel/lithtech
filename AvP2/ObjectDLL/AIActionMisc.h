// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMisc.h
//
// PURPOSE : Specific Action subclass definitions
//
// CREATED : 2/9/01
//
// ----------------------------------------------------------------------- //

#ifndef AI_ACTION_MISC_H
#define AI_ACTION_MISC_H

#include "AIAction.h"
#include "Timer.h"

#include <string>

class CAI;
class CAINodeCover;

// Please insert new actions in alphabetical order.

class AIActionAim : public AIAction
{
	public:

		AIActionAim(CAI * pAI)
			: AIAction(pAI),
		      m_bCompletedAim(LTFALSE) {}

		virtual ~AIActionAim() {}

		virtual void Start();
		virtual void Update();
		
		virtual void Interrupt()         { m_bDone = LTTRUE; }
		virtual LTBOOL IsInterruptible() const { return m_bCompletedAim; }

		virtual const char * GetActionDescription() const { return "Aim"; }

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

	private :

		LTBOOL m_bCompletedAim;
};


class AIActionBackoff : public AIAction
{
	public:

		AIActionBackoff(CAI * pAI)
			: AIAction(pAI),
			  m_vNextBackoffDest(0,0,0),
			  m_vInitialPos(0,0,0),
			  m_vFinalPos(0,0,0),
			  m_bHasAnim(LTFALSE) {}

		virtual ~AIActionBackoff() {}

		void Init(const LTVector & vBackoffDest) { m_vNextBackoffDest = vBackoffDest; }

		virtual void Start();
		virtual void Update();

		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "Backoff"; }

	private :

		LTVector	m_vNextBackoffDest;
		LTVector	m_vInitialPos;
		LTVector	m_vFinalPos;
		LTBOOL	 m_bHasAnim;
};



class AIActionCrouch : public AIAction
{
	public:

		AIActionCrouch(CAI * pAI)
			: AIAction(pAI),
			  m_pNextCoverNode(LTNULL),
			  m_pCurrentCoverNode(LTNULL) {}

		virtual ~AIActionCrouch() {}

		void Init( const CAINodeCover * pNextCoverNode ) { m_pNextCoverNode = pNextCoverNode; }

		virtual void Start();
		virtual void Update();
		virtual void End();

		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "Crouch"; }

	private :

		const CAINodeCover * m_pNextCoverNode;
		const CAINodeCover * m_pCurrentCoverNode;
};

class AIActionCrouchReload : public AIAction
{
	public:

		AIActionCrouchReload(CAI * pAI)
			: AIAction(pAI) {}

		virtual ~AIActionCrouchReload() {}

		virtual void Start();
		virtual void Update();

		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

		virtual const char * GetActionDescription() const { return "CrouchReload"; }

};

class AIActionDodge : public AIAction
{
	public:

		AIActionDodge(CAI * pAI)
			: AIAction(pAI),
			  m_vNextDodgeDest(0,0,0),
			  m_vInitialPos(0,0,0),
			  m_vFinalPos(0,0,0),
			  m_bDodgeRight(LTTRUE),
			  m_bHasAnim(LTFALSE) {}

		virtual ~AIActionDodge() {}

		void Init(LTBOOL bDodgeRight, const LTVector & vDodgeDest) { m_bDodgeRight = bDodgeRight; m_vNextDodgeDest = vDodgeDest; }

		virtual void Start();
		virtual void Update();

		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "Dodge"; }

	private :

		LTVector	m_vNextDodgeDest;
		LTVector	m_vInitialPos;
		LTVector	m_vFinalPos;
		LTBOOL   m_bDodgeRight;
		LTBOOL	 m_bHasAnim;
};

class AIActionDrop : public AIAction
{
	public:

		AIActionDrop(CAI * pAI)
			: AIAction(pAI) {}

		virtual ~AIActionDrop() {}

		virtual void Start();
		
		virtual LTBOOL IsDone() const;

		virtual const char * GetActionDescription() const { return "Drop"; }
};

class AIActionFacehug : public AIAction
{
	public:

		AIActionFacehug(CAI * pAI)
			: AIAction(pAI) {}

		virtual ~AIActionFacehug() {}

		virtual void Start();
		virtual void Update();
		
		virtual LTBOOL UsesMovementState(CharacterMovementState cms) { return cms == CMS_FACEHUG 
			                                                               || cms == CMS_FACEHUG_ATTACH; }

		virtual const char * GetActionDescription() const { return "Facehug"; }
};

class AIActionFireMelee : public AIAction
{
	public:

		AIActionFireMelee(CAI * pAI)
			: AIAction(pAI),
			  m_bHasAnim(LTFALSE),
			  m_fLastShotTime(-1.0f),
			  m_nBurstShotsLeft(0) {}

		virtual ~AIActionFireMelee() {}

		virtual void Start();
		virtual void Update();
		virtual void End();

		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);
		virtual void PhysicsMsg(LTVector * pNewOffset);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "FireMelee"; }

	private:

		LTBOOL	m_bHasAnim;
		LTFLOAT m_fLastShotTime;
		int		m_nBurstShotsLeft;
};

class AIActionFireWeapon : public AIAction
{
	public:

		AIActionFireWeapon(CAI * pAI)
			: AIAction(pAI),
			  m_bHasAnim(LTFALSE),
			  m_fLastShotTime(-1.0f),
			  m_nBurstShotsLeft(0),
			  m_hWeaponSound(LTNULL),
			  m_bTriedToPlaySounds(LTFALSE) {}

		virtual ~AIActionFireWeapon();

		virtual void Start();
		virtual void Update();
		virtual void End();

		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);


		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "FireWeapon"; }

	private:

		LTBOOL		m_bHasAnim;
		LTFLOAT		m_fLastShotTime;
		int			m_nBurstShotsLeft;
		LTBOOL		m_bTriedToPlaySounds;
		HLTSOUND	m_hWeaponSound;
};

class AIActionJumpTo : public AIAction
{
	public:

		AIActionJumpTo(CAI * pAI)
			: AIAction(pAI),
		      m_vDest(0,0,0) {}

		virtual ~AIActionJumpTo() {}

		void Init(const LTVector & vDest) { m_vDest = vDest; }

		virtual void Start();
		virtual void Update();
		
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual LTBOOL UsesMovementState(CharacterMovementState cms) { return cms == CMS_SPECIAL; }

		virtual const char * GetActionDescription() const { return "JumpTo"; }

	private :

		LTVector m_vDest;

};


class AIActionPlayAnim : public AIAction
{
	public:

		enum 
		{
			FLAG_LOOP  = ( 1 << 0 ),   // Loop the animation (makes action interruptible).
			FLAG_INTER = ( 1 << 1 ),   // Be an interruptible action, even though you aren't looping!
			FLAG_FCTRG = ( 1 << 2 ),   // Face target while playing animation.
			FLAG_RESET = ( 1 << 3 )    // Reset the animation if it is already being played.
		};

	public:

		AIActionPlayAnim(CAI * pAI)
			: AIAction(pAI),
			  m_bNextLoop(LTFALSE),
			  m_bNextInterruptible(LTFALSE),
			  m_bLoop(LTFALSE),
			  m_bInterrupt(LTFALSE),
			  m_bInterruptible(LTFALSE) {}

		void Init(const std::string & strAnim, uint32 nFlags = 0)
		{
			m_sAnim = strAnim;
			m_bNextLoop = (0 != (nFlags & FLAG_LOOP));
			m_bNextInterruptible = (0 != (nFlags & FLAG_INTER));
			m_bNextFaceTarget = (0 != (nFlags & FLAG_FCTRG));
			m_bReset = (0 != (nFlags & FLAG_RESET));
		}

		virtual ~AIActionPlayAnim() {}

		virtual void Start();
		virtual void Update();
		virtual void End();

		// This message handles everything so we don't need an Update()
		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

		virtual void Interrupt() { m_bInterrupt = LTTRUE; if( m_bInterruptible ) m_bDone = LTTRUE; }
		virtual LTBOOL IsInterruptible() const { return m_bInterruptible; }

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "PlayAnim"; }

	private:

		std::string m_sAnim;

		LTBOOL		m_bNextLoop;
		LTBOOL		m_bNextInterruptible;
		LTBOOL		m_bNextFaceTarget;

		LTBOOL		m_bLoop;
		LTBOOL		m_bInterrupt;		// true when the Action gets interrupted
		LTBOOL		m_bInterruptible;
		LTBOOL		m_bFaceTarget;
		LTBOOL		m_bReset;
};

class AIActionPounce : public AIAction
{
	public:

		AIActionPounce(CAI * pAI)
			: AIAction(pAI),
			  m_bStartedPounce(LTFALSE),
			  m_bHasAnim(LTFALSE) {}

		virtual ~AIActionPounce() {}

		virtual void Start();
		virtual void Update();
		
		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual LTBOOL UsesMovementState(CharacterMovementState cms) { return cms == CMS_POUNCING; } 

		virtual const char * GetActionDescription() const { return "Pounce"; }

	private :

		LTBOOL m_bStartedPounce;
		LTBOOL m_bHasAnim;
};

class AIActionReload : public AIAction
{
	public:

		AIActionReload(CAI * pAI)
			: AIAction(pAI) {}

		virtual ~AIActionReload() {}

		virtual void Start();
		virtual void Update();

		virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

		virtual void Load(HMESSAGEREAD hRead)   { AIAction::Load(hRead);   }
		virtual void Save(HMESSAGEWRITE hWrite) { AIAction::Save(hWrite);  }

		virtual const char * GetActionDescription() const { return "Reload"; }

};

class AIActionStun : public AIAction
{
	public:

		AIActionStun(CAI * pAI)
			: AIAction(pAI) {}

		virtual ~AIActionStun() {}

		virtual void Start();
		virtual void Update();
		virtual void End();

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "Stun"; }
};

class AIActionTurnTo : public AIAction
{
	public:

		AIActionTurnTo(CAI * pAI)
			: AIAction(pAI),
			  m_vDirection(1,0,0),
			  m_rInitialRot(0,0,0,1),
			  m_rFinalRot(0,0,0,1),
			  m_bRightTurn(LTTRUE),
			  m_fTurnLength(0.0f),
			  m_bHasAnim(LTFALSE) {}

		virtual ~AIActionTurnTo() {}

		void Init(const LTVector & vDirection) { m_vDirection = vDirection; }

		virtual void Start();
		virtual void Update();

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual const char * GetActionDescription() const { return "TurnTo"; }

	private :

		LTVector m_vDirection;
		LTRotation m_rInitialRot;
		LTRotation m_rFinalRot;
		LTBOOL   m_bRightTurn;
		LTFLOAT  m_fTurnLength;
		LTFLOAT  m_fStartTime;
		LTBOOL	 m_bHasAnim;
};

class AIActionWWJumpTo : public AIAction
{
	public:

		AIActionWWJumpTo(CAI * pAI)
			: AIAction(pAI),
		      m_vDest(0,0,0),
			  m_vNormal(0,0,0) {}

		virtual ~AIActionWWJumpTo() {}

		void Init(const LTVector & vDest, const LTVector & vNormal) 
		{ 
			m_vDest   = vDest; 
			m_vNormal = vNormal;
		}

		virtual void Start();
		virtual void Update();
		
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual LTBOOL UsesMovementState(CharacterMovementState cms) { return cms == CMS_SPECIAL; } 

		virtual const char * GetActionDescription() const { return "WWJumpTo"; }

	private :

		LTVector m_vDest;
		LTVector m_vNormal;
};

#endif // AI_ACTION_MISC_H
