// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMoveTo.h
//
// PURPOSE : The base action
//
// CREATED : 2/6/01
//
// ----------------------------------------------------------------------- //

#ifndef AI_ACTION_MOVETO_H
#define AI_ACTION_MOVETO_H

#include "AIAction.h"

#include "Timer.h"

class CAI;

class AIActionBezierTo : public AIAction
{
	public:

		AIActionBezierTo(CAI * pAI)
			: AIAction(pAI),			  
			  m_vInitDest(0,0,0),
			  m_vInitBezierSrc(0,0,0),
			  m_vInitBezierDest(0,0,0),
			  m_vInitDestNormal(0,0,0),
			  m_vSrc(0,0,0),
			  m_vDest(0,0,0),
			  m_vBezierSrc(0,0,0),
			  m_vBezierDest(0,0,0),
			  m_vSrcNormal(0,0,0),
			  m_vDestNormal(0,0,0),
			  m_fTotalTime(0.0f),
			  m_fStartTime(0.0f),
			  m_vLastPosition(0,0,0) {}

		virtual ~AIActionBezierTo() {}

		void Init(const LTVector & vDest, const LTVector & vNormal, const LTVector & vBezierSrc, const LTVector & vBezierDest) 
		{ 
			m_vInitDest = vDest; 
			m_vInitBezierSrc = vBezierSrc;
			m_vInitBezierDest = vBezierDest;
			m_vInitDestNormal = vNormal;
		}

		virtual void Start();
		virtual void Update();
		virtual void End();
		
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual LTBOOL IsInterruptible() const;

		virtual const char * GetActionDescription() const { return "BezierTo"; }

	private :

		LTVector m_vInitDest;
		LTVector m_vInitBezierSrc;
		LTVector m_vInitBezierDest;
		LTVector m_vInitDestNormal;

		LTVector m_vSrc;
		LTVector m_vDest;
		LTVector m_vBezierSrc;
		LTVector m_vBezierDest;
		LTVector m_vDestNormal;
		LTVector m_vSrcNormal;

		LTFLOAT  m_fTotalTime;
		LTFLOAT  m_fStartTime;

		LTVector m_vLastPosition;
};


class AIActionLadderTo : public AIAction
{
public :

	AIActionLadderTo(CAI * pAI)
		: AIAction(pAI),
		  m_vDest(0,0,0),
		  m_vLadderDest(0,0,0),
		  m_vExitDest(0,0,0),
		  m_vInitLadderDest(0,0,0),
		  m_vInitExitDest(0,0,0)    {}

	void Init(const LTVector & vLadderDest, const LTVector & vExitDest)
	{
		m_vInitLadderDest = vLadderDest;
		m_vInitExitDest = vExitDest;
	}

	virtual void Load(HMESSAGEREAD  hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual void Start();
	virtual void Update();
	virtual void End();

	virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

	virtual void PhysicsMsg(LTVector * pNewOffset);

	virtual LTBOOL IsInterruptible() const { return LTFALSE;  }

	virtual const char * GetActionDescription() const { return "LadderTo"; }

private :

	LTBOOL   m_bHasAnim;

	LTVector m_vDest;
	LTVector m_vLadderDest;
	LTVector m_vExitDest;

	LTVector m_vInitLadderDest;	
	LTVector m_vInitExitDest;	
};

class AIActionMoveTo : public AIAction
{
public :

	AIActionMoveTo(CAI * pAI)
		: AIAction(pAI),
		  m_bInterrupt(LTFALSE),
		  m_bHasAnim(LTFALSE),
		  m_vDest(0,0,0),
		  m_vStartDir(0,0,0),
		  m_bReverseMovement(LTFALSE),
		  m_vInitDest(0,0,0),
		  m_bAllowReverseMovement(LTFALSE) {}

	void Init(const LTVector & vPos, LTBOOL bAllowReverseMovement = LTFALSE)
	{
		m_vInitDest = vPos;
		m_bAllowReverseMovement = bAllowReverseMovement;
	}

	virtual void Load(HMESSAGEREAD  hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual void Start();
	virtual void Update();

	virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

	virtual void PhysicsMsg(LTVector * pNewOffset);

	virtual void Interrupt() { m_bInterrupt = LTTRUE; }

	virtual LTBOOL IsInterruptible() const { return LTFALSE;  }

	virtual const char * GetActionDescription() const { return "MoveTo"; }

private :

	LTBOOL   m_bInterrupt;
	LTBOOL   m_bHasAnim;

	LTVector m_vDest;
	LTVector m_vStartDir;
	LTBOOL   m_bReverseMovement;

	LTVector m_vInitDest;	
	LTBOOL   m_bAllowReverseMovement;
};

class AIActionMoveForward : public AIAction
{
public :

	AIActionMoveForward(CAI * pAI)
		: AIAction(pAI),
		  m_bHasAnim(LTFALSE) {}

	virtual ~AIActionMoveForward() {}

	virtual void Load(HMESSAGEREAD  hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual void Start();
	virtual void Update();

	virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

	virtual void Interrupt() { m_tmrInterrupt.Start(0.4f); }
	
	virtual LTBOOL IsInterruptible() const { return LTTRUE;  }

	virtual const char * GetActionDescription() const { return "MoveForward"; }

private :

	CTimer m_tmrInterrupt;
	LTBOOL m_bHasAnim;
	CTimer m_tmrNoAnim;
};

class AIActionWWForward : public AIAction
{
public :

	AIActionWWForward(CAI * pAI)
		: AIAction(pAI),
		  m_bHasAnim(LTFALSE),
		  m_vStartingUp(0,1,0),
		  m_vLastUsedUp(0,1,0),
		  m_vInitDest(0,0,0),
		  m_vDest(0,0,0) {}

	virtual ~AIActionWWForward() {}

	void Init(const LTVector & vPos)
	{
		m_vInitDest = vPos;
	}

	virtual void Load(HMESSAGEREAD  hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual void Start();
	virtual void Update();

	virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo);

	virtual void PhysicsMsg(LTVector * pNewOffset);

	virtual const char * GetActionDescription() const { return "WWForward"; }

private :

	LTBOOL m_bHasAnim;
	CTimer m_tmrNoAnim;

	LTVector m_vStartingUp;
	LTVector m_vLastUsedUp;

	LTVector m_vInitDest;
	LTVector m_vDest;
};


#endif // AI_ACTION_MOVETO_H
