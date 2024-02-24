// ----------------------------------------------------------------------- //
//
// MODULE  : Patrol.h
//
// PURPOSE : Implements the Patrol goal and state.
//
// CREATED : 5/26/00
//
// ----------------------------------------------------------------------- //

#ifndef __PATROL_H__
#define __PATROL_H__

#include "Goal.h"
#include "AIState.h"
#include "Timer.h"
#include "Range.h"

#include "AIStrategyFollowPath.h"

class CAINodePatrol;

class CAIStatePatrol : public CAIState
{

	public :

		// Ctors/Dtors/Etc
		CAIStatePatrol(CAI * pAI);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Overloaded Methods
		virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

		virtual void Start();

		virtual void Update();

#ifndef _FINAL
		virtual const char * GetDescription() const;
#endif

		// Other methods
		void SetPath(const CAINodePatrol * pNode);

		bool HasValidPath() const { return m_pPatrolNode != LTNULL; }

	private :

		struct Root
		{
			enum State
			{
				eStateFindNextNode,
				eStateWait,
				eStateGoToNode,
				eStateEndOfPath
			};

			enum Message
			{
				eFinishedState,
				eEndOfPath,
				eStuck
			};
		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

		void SetupFSM();

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		RootFSM m_RootFSM;

		
		void UpdateFindNextNode(RootFSM * rootFSM);

		void StartWait(RootFSM * rootFSM);
		void UpdateWait(RootFSM * rootFSM);

		void StartGoToNode(RootFSM * rootFSM);
		void UpdateGoToNode(RootFSM * rootFSM);

		CAIStrategyFollowPath m_StrategyFollowPath;

		const CAINodePatrol * m_pPatrolNode;
		const CAINodePatrol * m_pLastPatrolNodeReached;

		CTimer	m_tmrWait;
		bool	m_bForward;

		FRange  m_rngStuckDelay;
};

class PatrolGoal : public Goal
{
public :

	PatrolGoal( CAI * pAI, std::string create_name );

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

	virtual void Update() { Goal::Update(); m_PatrolState.Update(); }
	virtual void Start()  { Goal::Start(); m_PatrolState.Start(); }
	virtual void End()    { Goal::End(); m_PatrolState.End(); }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "Patrol"; }
#endif
	
	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodAmbient; }
	const char * GetRandomSoundName() const { return "RandomIdle"; } 
	virtual const char * GetInitialAnimationSet() const { return "Patrol"; }


	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();


private :

	CAIStatePatrol m_PatrolState;
};


#endif // __PATROL_H__
