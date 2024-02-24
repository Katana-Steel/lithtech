// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyRetreat.h
//
// PURPOSE : 
//
// CREATED : 8.15.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_RETREAT_H__
#define __AI_STRATEGY_RETREAT_H__

#include "AIStrategy.h"
#include "AIStrategyFollowPath.h"
#include "AIStrategySetFacing.h"

class CAINode;

class CAIStrategyRetreat : public CAIStrategy
{

	public : // Public methods

		// Ctors/Dtors/etc
		CAIStrategyRetreat(CAI * pAI);
		virtual ~CAIStrategyRetreat() {}

	
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Methods

		void Start();
		virtual void Update();

		void	SetFlee(LTBOOL bFlee) { m_bFlee = bFlee; }
		LTBOOL	IsCornered() { return m_RootFSM.GetState() == Root::eStateCornered; }
		void	FindFriend(HOBJECT hFriend) { m_hFriend = hFriend; }

	protected:

		virtual void FireOnTarget();

		
		struct Root
		{
			enum State
			{
				eStateRunning,
				eStateRunningToCorner,
				eStateCornered
			};

			enum Message
			{
				eCornered,				// all destinations are closer to the target
				eNotCornered,
				eTargetInvalid
			};
		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

		RootFSM					m_RootFSM;	
		CAIStrategyFollowPath   m_StrategyFollowPath;
		CAIStrategySetFacing	m_StrategySetFacing;


	private : // Private member variables

		void SetupFSM();

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		// Called by RootFSM
		void StartRunning(RootFSM *);
		void UpdateRunning(RootFSM *);

		void StartRunningToCorner(RootFSM *);
		void UpdateRunningToCorner(RootFSM *);

		void StartCornered(RootFSM *);
		void UpdateCornered(RootFSM *);

		const CAINode *		m_pDestNode;	// don't save (updated automatically)

		LTBOOL					m_bFlee;
		LTSmartLink				m_hFriend;
};

#endif //__AI_STRATEGY_RETREAT_H__
