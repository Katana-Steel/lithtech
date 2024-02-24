// ----------------------------------------------------------------------- //
//
// MODULE  : AIAlarm.h
//
// PURPOSE : Implements the Alarm goal and state.
//
// CREATED : 12/06/00
//
// ----------------------------------------------------------------------- //

#ifndef __ALARM_GOAL_H__
#define __ALARM_GOAL_H__

#include "fsm.h"
#include "AICombatGoal.h"
#include "AIState.h"
#include "AITarget.h"
#include "AIStrategySetFacing.h"
#include "AIStrategyFollowPath.h"
#include "Timer.h"
#include "AISenseMgr.h"
#include "AINodeAlarm.h"

class CAIStateAlarm : public CAIState
{

	public : // Public methods

		CAIStateAlarm(CAI * pAI);

		virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Overloaded Methods

		virtual void Start();
		virtual void Update();
		virtual void End();
		
#ifndef _FINAL
		virtual const char * GetDescription() const { return "Alarm"; }
#endif

		LTBOOL IsDone();
		LTBOOL IsDoneFacing() const { return m_StrategySetFacing.Finished(); }

		LTBOOL HasNode() { return (m_pCurNode ? LTTRUE : LTFALSE); }

		HOBJECT GetNodeGroup() const { return m_hAINodeGroup; }

	private :

		struct Root
		{
			enum State
			{
				eStateFind,
				eStateSwitch,
				eStateDone
			};

			enum Message
			{
				eStimulus,				// sense activated
				eAtNode,
				eDone
			};
		};

	private :

		void	PlayAlarmAni();		// play alarm-activation animation

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		void	SetupFSM();

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		// Called by RootFSM
		void StartFind(RootFSM *);
		void UpdateFind(RootFSM *);

		void StartSwitch(RootFSM *);
		void UpdateSwitch(RootFSM *);

		void UpdateDone(RootFSM *);

		RootFSM		m_RootFSM;	

		CAIStrategySetFacing  m_StrategySetFacing;
		CAIStrategyFollowPath m_StrategyFollowPath;


		CAINodeAlarm *			m_pCurNode;

		LTSmartLink	 m_hAINodeGroup;
};

class AlarmGoal : public AICombatGoal
{
public :

	AlarmGoal( CAI * pAI, std::string create_name );

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Start();
	virtual void End();

	virtual void Update() { AICombatGoal::Update(); alarmState.Update(); }

	virtual LTBOOL StopCinematic() const { return LTTRUE; }
	virtual LTBOOL DefaultAlert() const { return  LTTRUE; }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "Alarm"; }
#endif

	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodWarning; }
	virtual const char * GetInitialAnimationSet() const { return "Alarm"; }

protected :

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

	CAISenseSeeDeadBody * m_pSeeDeadBody;

//	inherited from CombatGoal
//	CAISenseSeeEnemy * m_pSeeEnemy;
//	CAISenseHearEnemyWeaponFire * m_pHearEnemyWeaponFire;
//	CAISenseHearWeaponImpact * m_pHearWeaponImpact;

private :

	CAIStateAlarm		alarmState;
};


#endif // __ALARM_GOAL_H__
