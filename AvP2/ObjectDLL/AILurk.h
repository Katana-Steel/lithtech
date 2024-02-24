// ----------------------------------------------------------------------- //
//
// MODULE  : Lurk.h
//
// PURPOSE : Implements the Lurk goal and state.
//
// CREATED : 8/10/00
//
// ----------------------------------------------------------------------- //

#ifndef __LURK_H__
#define __LURK_H__

#include "fsm.h"
#include "AICombatGoal.h"
#include "AIState.h"
#include "AITarget.h"
#include "AIStrategyApproachTarget.h"
#include "AIStrategyRetreat.h"
#include "AIStrategyEngageTarget.h"

class CAIStateLurk : public CAIState
{
	public :

		struct Root
		{
			enum State
			{
				eStateHide,
				eStateAdvance,
				eStateRetreat,
				eStateAttack,
				eStateLostTarget
			};

			enum Message
			{
				eCanSeeTarget,
				eCannotSeeTarget,
				eInRange,
				eOutOfRange,
				eTargetFacing,
				eTargetNotFacing,
				eTargetInvalid
			};
		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

	public : // Public methods

		CAIStateLurk(CAI * pAI);
		
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Overloaded Methods

		virtual void Start();
		virtual void Update();
		virtual void End();
		
#ifndef _FINAL
		virtual const char * GetDescription() const { return "Lurk"; }
#endif

		bool LostTarget() const { return m_RootFSM.GetState() == Root::eStateLostTarget; }
		bool IsAttacking() const { return m_RootFSM.GetState() == Root::eStateAttack; }

		virtual Root::State GetNextState(LTBOOL bCanSeeTarget);


	private :

		void SetupFSM();

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		// Called by RootFSM
		void StartHide(RootFSM *);
		void UpdateHide(RootFSM *);

		void StartAdvance(RootFSM *);
		void UpdateAdvance(RootFSM *);

		void StartRetreat(RootFSM *);
		void UpdateRetreat(RootFSM *);
		void EndRetreat(RootFSM *);

		void StartAttack(RootFSM *);
		void UpdateAttack(RootFSM *);

		RootFSM		m_RootFSM;	

		CAISenseSeeEnemy * m_pSeeEnemy;

		CAIStrategyApproachTarget m_StrategyApproachTarget;
		CAIStrategyRetreat        m_StrategyRetreat;

		LTBOOL		m_bIsCornered;
		LTFLOAT		m_fLurkAttackRange;
		LTBOOL		m_bWasAttacked;
};

class LurkGoal : public AICombatGoal
{
public :

	LurkGoal( CAI * pAI, std::string create_name );

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Start();
	virtual void End();

	virtual void Update() { AICombatGoal::Update(); lurkState.Update(); }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "Lurk"; }
#endif

	virtual LTBOOL StopCinematic() const { return LTTRUE; }
	virtual LTBOOL DefaultAlert() const { return  LTTRUE; }

	const char * GetRandomSoundName() const { return lurkState.IsAttacking() ? "RandomAttack" : "RandomIdle"; } 

	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return lurkState.IsAttacking() ? CMusicMgr::eMoodHostile : CMusicMgr::eMoodWarning; }

protected :

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

private :

	CAIStateLurk lurkState;
	LTSmartLink  m_hTarget;
};


#endif // __LURK_H__
