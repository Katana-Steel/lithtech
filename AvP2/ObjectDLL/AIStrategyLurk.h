// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyLurk.h
//
// PURPOSE : 
//
// CREATED : 7.10.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_LURK_H__
#define __AI_STRATEGY_LURK_H__

#include "fsm.h"
#include "AIStrategy.h"
#include "AIStrategyApproachTarget.h"
#include "AIStrategyFollowPath.h"


class CAIStrategyLurk : public CAIStrategy
{

	public : // Public methods

		// Ctors/Dtors/etc
		CAIStrategyLurk();
		virtual ~CAIStrategyLurk() {}

	
		virtual void Init(CAI * pAI);

//		virtual void Load(HMESSAGEREAD hRead);
//		virtual void Save(HMESSAGEWRITE hWrite);

		// Methods

		virtual void Update();


	protected:

		virtual void ApproachTarget();


	private :

		struct Root
		{
			enum State
			{
				eStateSearch,
				eStateAdvance,
				eStateRetreat,
				eStateAttack
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

	private : // Private member variables

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		void SetupFSM();

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		// Called by RootFSM
		void StartSearch(RootFSM *);
		void UpdateSearch(RootFSM *);

		void StartAdvance(RootFSM *);
		void UpdateAdvance(RootFSM *);

		void StartRetreat(RootFSM *);
		void UpdateRetreat(RootFSM *);

		void StartAttack(RootFSM *);
		void UpdateAttack(RootFSM *);

		RootFSM		m_RootFSM;	

		CAIStrategyApproachTarget m_StrategyApproachTarget;
		CAIStrategyFollowPath m_StrategyFollowPath;
};


#endif //__AI_STRATEGY_LURK_H__