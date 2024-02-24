// ----------------------------------------------------------------------- //
//
// MODULE  : AIAlienAttack.h
//
// PURPOSE : Implements the Melee Attack goal and state.
//
// CREATED : 8/1/00
//
// ----------------------------------------------------------------------- //

#ifndef __AI_ALIEN_ATTACK_H__
#define __AI_ALIEN_ATTACK_H__

#include "fsm.h"
#include "AICombatGoal.h"
#include "Timer.h"
#include "AIStrategyApproachTarget.h"

struct AIBM_AlienAttack;
class CAINodeWallWalk;

class AlienAttackGoal : public AICombatGoal
{
	public:

		AlienAttackGoal( CAI * pAI, std::string create_name );
		
		virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

		virtual void Start();
		virtual void Update();
		virtual void End();

#ifndef _FINAL
		virtual const char * GetDescription() const { return "AlienAttack"; }
#endif
		virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodHostile; }

		virtual const char * GetRandomSoundName() const { return "RandomAttack"; }

		virtual LTBOOL StopCinematic() const { return LTTRUE; }
		virtual LTBOOL DefaultAlert() const { return  LTTRUE; }

		virtual LTFLOAT GetMaximumBid();
		virtual LTFLOAT GetBid();

		virtual void	Load(HMESSAGEREAD hRead);
		virtual void	Save(HMESSAGEWRITE hWrite);

	private :

		void SetupFSM();
		void FindBetterAdvanceNode();

		struct Root
		{
			enum State
			{
				eStateWaitForNewTarget,
				eStateDisplay,
				eStateApproachTarget,
				eStateGetLineOfSight,
				eStateEngageTarget
			};

			enum Message
			{
				eNewTarget,
				eEngageTarget
			};

		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

		void	StartWaitForNewTarget(RootFSM * rootFSM);
		void	UpdateWaitForNewTarget(RootFSM * rootFSM);
		void	EndWaitForNewTarget(RootFSM * rootFSM);

		void	StartDisplay(RootFSM * rootFSM);
		void	UpdateDisplay(RootFSM * rootFSM);
		void	EndDisplay(RootFSM * rootFSM);

		void	StartApproachTarget(RootFSM * rootFSM);
		void	UpdateApproachTarget(RootFSM * rootFSM);
		void	EndApproachTarget(RootFSM * rootFSM);

		void	StartGetLineOfSight(RootFSM * rootFSM);
		void	UpdateGetLineOfSight(RootFSM * rootFSM);
		void	EndGetLineOfSight(RootFSM * rootFSM);

		void	StartEngageTarget(RootFSM * rootFSM);
		void	UpdateEngageTarget(RootFSM * rootFSM);
		void	EndEngageTarget(RootFSM * rootFSM);

		// Only implemented if ALIEN_ATTACK_DEBUG is #defined.
		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

		CTimer		 m_tmrWaitForNewTarget;  // Is only on if the AI does not have a target.
		NewTarget    m_NewTarget;

		RootFSM	m_RootFSM;

		CAIStrategyApproachTarget	m_StrategyApproachTarget;
		CAIStrategyFollowPath		m_StrategyFollowPath;

		LTBOOL	m_bDropped;
		LTBOOL  m_bSentUnreachableCmd;

		CTimer	m_tmrPounce;
		CTimer	m_tmrFacehug;

		CAINodeWallWalk * m_pAdvanceNode;

		std::string m_strDisplayAnim;

		const AIBM_AlienAttack & m_Butes; // needn't be saved.
};


class DumbAlienAttackGoal : public AICombatGoal
{
public :

	DumbAlienAttackGoal( CAI * pAI, std::string create_name )
		: AICombatGoal(pAI,create_name) {}

	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Start();
	virtual void Update();
	virtual void End();

	virtual const char * GetRandomSoundName() const { return "RandomAttack"; }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "DumbAttack"; }
#endif
	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodHostile; }

};


#endif //__AI_ALIEN_ATTACK_H__
