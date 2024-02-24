// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyApproachTarget.h
//
// PURPOSE : 
//
// CREATED : 6.20.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_APPROACH_TARGET_H__
#define __AI_STRATEGY_APPROACH_TARGET_H__

#include "AIStrategy.h"
#include "AIStrategyFollowPath.h"
#include "AIStrategySetFacing.h"

class CAIStrategyApproachTarget : public CAIStrategy
{

	public : // Public methods

		// Ctors/Dtors/etc
		CAIStrategyApproachTarget(CAI * pAI);
		virtual ~CAIStrategyApproachTarget() {}

	
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Methods

		virtual void Update();

		void Reset() { m_StrategyFollowPath.Clear(); m_RootFSM.SetState(Root::eStateTargetVisible); }
		bool LostTarget() { return m_RootFSM.GetState() == Root::eStateTargetLost; } 

	private :

		struct Root
		{
			enum State
			{
				eStateTargetVisible,
				eStateLastTargetPosition,
				eStateNextVolume,
				eStateTargetLost
			};

			enum Message
			{
				eCanSeeTarget,
				eCannotSeeTarget,
				eAtDesiredLocation,
				eCannotReachDesiredLocation,
				eTargetInvalid
			};
		};

	private : // Private member variables

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		void SetupFSM();

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		// Called by RootFSM
		void StartTargetVisible(RootFSM *);
		void UpdateTargetVisible(RootFSM *);

		void StartLastTargetPosition(RootFSM *);
		void UpdateLastTargetPosition(RootFSM *);

		void StartNextVolume(RootFSM *);
		void UpdateNextVolume(RootFSM *);





		RootFSM		m_RootFSM;	



		CAIStrategyFollowPath   m_StrategyFollowPath;

		CTimer		m_tmrRepath;
		LTFLOAT		m_fMinRepathTime;
		LTFLOAT		m_fMaxRepathTime;

		LTVector	m_vLastKnownPos;
};


#endif //__AI_STRATEGY_APPROACH_TARGET_H__
