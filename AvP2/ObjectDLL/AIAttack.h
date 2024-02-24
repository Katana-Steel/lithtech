// ----------------------------------------------------------------------- //
//
// MODULE  : Attack.h
//
// PURPOSE : Implements the Attack goal and state.
//
// CREATED : 6/14/00
//
// ----------------------------------------------------------------------- //

#ifndef __ATTACK_H__
#define __ATTACK_H__

#include "AICombatGoal.h"
#include "AIStrategyApproachTarget.h"
#include "AIStrategyAdvance.h"
#include "AIStrategyRetreat.h"
#include "AIStrategyEngageTarget.h"

class CAI;
struct AIBM_Attack;

class AttackGoal : public AICombatGoal
{
public :

	AttackGoal( CAI * pAI, std::string create_name);
	virtual ~AttackGoal();

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Update();

	virtual void Start();
	virtual void End();

#ifndef _FINAL
	virtual const char * GetDescription() const;
#endif

	virtual LTBOOL StopCinematic() const { return LTTRUE; }
	virtual LTBOOL DefaultAlert() const { return  LTTRUE; }

	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodHostile; }

private :

	LTBOOL	FireOnTarget(LTBOOL bIfVisible, LTBOOL bIgnoreRange = LTFALSE);
	LTFLOAT	GetApproachTime();


	LTBOOL  Dodge();
	LTBOOL  Backoff();
	LTBOOL  RunForCover();
	LTBOOL  Crouch();
	LTBOOL	Melee();

	struct Root
	{
		enum State
		{
			eStateWaitForNewTarget,
			eStateApproachTarget,
			eStateGetLineOfSight,
			eStateRunForCover,
			eStateEngageTarget,
			eStateFear,
			eStateEngageMelee
		};

		enum Message
		{
			eNoTarget
		};

	};

	typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

	void	StartWaitForNewTarget(RootFSM * rootFSM);
	void	UpdateWaitForNewTarget(RootFSM * rootFSM);
	void	EndWaitForNewTarget(RootFSM * rootFSM);

	void	StartApproachTarget(RootFSM * rootFSM);
	void	UpdateApproachTarget(RootFSM * rootFSM);
	void	EndApproachTarget(RootFSM * rootFSM);

	void	StartGetLineOfSight(RootFSM * rootFSM);
	void	UpdateGetLineOfSight(RootFSM * rootFSM);
	void	EndGetLineOfSight(RootFSM * rootFSM);

	void	StartRunForCover(RootFSM * rootFSM);
	void	UpdateRunForCover(RootFSM * rootFSM);
	void	EndRunForCover(RootFSM * rootFSM);

	void	StartEngageTarget(RootFSM * rootFSM);
	void	UpdateEngageTarget(RootFSM * rootFSM);
	void	EndEngageTarget(RootFSM * rootFSM);

	void	StartFear(RootFSM *);
	void	UpdateFear(RootFSM *);
	void	EndFear(RootFSM *);

	void	StartEngageMelee(RootFSM *);
	void	UpdateEngageMelee(RootFSM *);
	void	EndEngageMelee(RootFSM *);

	void SetupFSM();

	// These are only implemented if ATTACK_DEBUG is #defined.
	void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
	void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

	CTimer		 m_tmrWaitForNewTarget;  // Is only on if the AI does not have a target.
	NewTarget    m_NewTarget;

	RootFSM	m_RootFSM;

	CAIStrategyApproachTarget	m_StrategyApproachTarget;
	CAIStrategyFollowPath		m_StrategyFollowPath;
	CAIStrategyRetreat			m_StrategyRetreat;

	LTSmartLink		m_hTarget;

	CAINodeCover *	m_pCurCoverNode;
	CAINodeCover *	m_pLastNodeReached;

	const AIBM_Attack & m_AttackButes;

	CTimer		m_tmrDodgeDelay;
	CTimer		m_tmrBackoffDelay;
	CTimer		m_tmrRunForCoverDelay;
	CTimer		m_tmrCoverDuration;
	CTimer		m_tmrCrouchDelay;
	CTimer		m_tmrMeleeRepath;
	CTimer		m_tmrStuckFire;

	LTBOOL		m_bSentUnreachableCmd;

	LTFLOAT		m_fCost;
	LTBOOL		m_bFiredOnApproach;
	LTFLOAT		m_fApproachTime;
	CTimer		m_tmrApproach;
};


#endif // __ATTACK_H__
