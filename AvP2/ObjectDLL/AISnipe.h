// ----------------------------------------------------------------------- //
//
// MODULE  : Snipe.h
//
// PURPOSE : Implements the Snipe goal and state.
//
// CREATED : 6/14/00
//
// ----------------------------------------------------------------------- //

#ifndef __SNIPE_H__
#define __SNIPE_H__

#include "AICombatGoal.h"
#include "AIState.h"
#include "AIStrategySetFacing.h"
#include "AIStrategyFollowPath.h"
#include "AINodeSnipe.h"

class CAI;
class CAISenseSeeEnemy;
class CAISenseHearEnemyWeaponFire;
class CAISenseHearWeaponImpact;
class CAISenseHearEnemyFootstep;
class CAISenseSeeEnemyFlashlight;

class CAIStateSnipe : public CAIState
{

	public : // Public methods

		// Ctors/Dtors/Etc
		CAIStateSnipe(CAI * pAI);
		virtual ~CAIStateSnipe() {}

		virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual void Start();
		virtual void Update();
		virtual void End();
		
#ifndef _FINAL
		virtual const char * GetDescription() const { return "Snipe"; }
#endif
		bool IsAttacking() const { return m_RootFSM.GetState() == Root::eStateAttack; }

	protected :

		virtual void FireOnTarget();

	private :

		void SetupFSM();

		struct Root
		{
			enum State
			{
				eStateFindNode,
				eStateFindTarget,
				eStateAttack
			};

			enum Message
			{
				eAtNode,
				eHasTarget,
				eUnderFire,
				eCannotSeeTarget,
				eNoTarget
			};

		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

	
		RootFSM	m_RootFSM;

		void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
		void	PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);


		void StartFindNode(RootFSM *);
		void UpdateFindNode(RootFSM *);

		void StartFindTarget(RootFSM *);
		void UpdateFindTarget(RootFSM *);

		void StartAttack(RootFSM *);
		void UpdateAttack(RootFSM *);
		void EndAttack(RootFSM *);


		CAISenseSeeEnemy * m_pSeeEnemy;
		CAISenseHearEnemyFootstep * m_pHearEnemyFootstep;
		CAISenseHearEnemyWeaponFire * m_pHearEnemyWeaponFire;
		CAISenseHearWeaponImpact * m_pHearWeaponImpact;
		CAISenseSeeEnemyFlashlight * m_pSeeEnemyFlashlight;

		CAIStrategyFollowPath		m_StrategyFollowPath;

		CAINodeSnipe *	m_pCurNode;
		uint32			m_nNumNodes;
		CTimer			m_tmrHoldPos;

		LTSmartLink	 m_hAINodeGroup;			// which node group should this sniper use?
};

class SnipeGoal : public AICombatGoal
{
public :

	SnipeGoal( CAI * pAI, std::string create_name );
	virtual ~SnipeGoal();

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Update() { AICombatGoal::Update(); snipeState.Update(); }

	virtual void Start();
	virtual void End();

	virtual LTBOOL StopCinematic() const { return LTTRUE; }
	virtual LTBOOL DefaultAlert() const { return  LTTRUE; }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "Snipe"; }
#endif
	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return snipeState.IsAttacking() ? CMusicMgr::eMoodHostile : CMusicMgr::eMoodWarning; }
	virtual const char * GetInitialAnimationSet() const { return "Snipe"; }


protected :

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

private :

	CAIStateSnipe snipeState;

	CAISenseSeeEnemy * m_pSeeEnemy;

	LTSmartLink  m_hTarget;
	LTVector	 m_vTargetPos;
	LTFLOAT		 m_fMinTargetRangeSqr;
};


#endif // __SNIPE_H__
