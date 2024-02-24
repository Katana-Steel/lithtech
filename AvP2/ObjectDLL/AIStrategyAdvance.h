// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyAdvance.h
//
// PURPOSE : 
//
// CREATED : 8.15.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_ADVANCE_H__
#define __AI_STRATEGY_ADVANCE_H__

#include "AIStrategy.h"
#include "AIStrategyFollowPath.h"
#include "Timer.h"
#include "AINodeCover.h"

#ifdef _DEBUG
//#define STRATEGY_ADVANCE_DEBUG
#endif

class CAIStrategyAdvance : public CAIStrategy
{

	public : // Public methods

		// Ctors/Dtors/etc
		CAIStrategyAdvance(CAI * pAI);
		virtual ~CAIStrategyAdvance() {}

	
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Methods

		virtual void Update();

		void SetPath();

		LTBOOL IsCornered() { return m_RootFSM.GetState() == Root::eStateCornered; }

	protected:

	private :

		struct Root
		{
			enum State
			{
				eStateHiding,
				eStateRunning,
				eStateCornered
			};

			enum Message
			{
				eCanSeeTarget,			// sets running state
				eTargetCannotSee,		// verifies that the target does not have LOS
										// before this AI switches to hiding state

				eCornered,				// all destinations are closer to the target
				eNotCornered,
				eTargetInvalid
			};
		};

	private : // Private member variables

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		void SetupFSM();

#ifdef STRATEGY_ADVANCE_DEBUG
		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);
#endif

		// Called by RootFSM
		void StartHiding(RootFSM *);
		void UpdateHiding(RootFSM *);

		void StartRunning(RootFSM *);
		void UpdateRunning(RootFSM *);

		void StartCornered(RootFSM *);
		void UpdateCornered(RootFSM *);

		RootFSM					m_RootFSM;	

		CAIStrategyFollowPath   m_StrategyFollowPath;

		CTimer					m_tmrHide;
		CAINodeCover *			m_pCurNode;
		LTBOOL					m_bCover;
		CTimer					m_tmrCover;
};


#endif //__AI_STRATEGY_ADVANCE_H__
