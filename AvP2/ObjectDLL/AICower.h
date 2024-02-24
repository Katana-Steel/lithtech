// ----------------------------------------------------------------------- //
//
// MODULE  : AICower.h
//
// PURPOSE : Implements the Cower goal and state.
//
// CREATED : 9/05/00
//
// ----------------------------------------------------------------------- //

#ifndef __COWER_H__
#define __COWER_H__

#include "fsm.h"
#include "Goal.h"
#include "AIStrategySetFacing.h"
#include "AIStrategyFollowPath.h"
#include "AIStrategyRetreat.h"
#include "Timer.h"
#include "AINodeCover.h"

#include <set>
#include <string>

class CAISense;
class CAISenseSeeEnemy;
class CAISenseHearEnemyWeaponFire;
class CAISenseHearWeaponImpact;
class CAISenseSeeDeadBody;
class CAISenseHearAllyBattleCry;

struct AIBM_Cower;
class CAI;

class CowerGoal : public Goal
{
public :

	typedef std::set<CAI*> AIList;

	enum CowerAnimState
	{
		Cower_Start,
		Cower,
		Cower_Idle
	};

public :

	CowerGoal( CAI * pAI, LTBOOL bUseRetreat, std::string create_name );

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Start();
	virtual void End();

	virtual void Update();

	virtual LTBOOL StopCinematic() const { return LTTRUE; }
	virtual LTBOOL DefaultAlert() const { return  LTTRUE; }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "Cower"; }
#endif
	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodWarning; }

protected :

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

private :

	struct Root
	{
		enum State
		{
			eStateFindNode,
			eStateRetreat,
			eStateCower,
			eStateSafe
		};

		enum Message
		{
			eAtNode,
			eRun,
			eSafe
		};
	};

		
	CAISense * GetActiveSensePtr(); // Gets a pointer to the sense that has the highest level
	void	PlayCowerAnim();	// start and maintain the cower animation.


	bool	StartAnimCower();      // starts the cower animation.
	bool	StartAnimCowerIdle();  // starts the cower idle animation.

	typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
	void SetupFSM();

	void	PrintRootFSMState(const RootFSM & fsm, Root::State state);
	void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

	CAISenseSeeEnemy * m_pSeeEnemy;
	CAISenseHearEnemyWeaponFire * m_pHearEnemyWeaponFire;
	CAISenseHearWeaponImpact * m_pHearWeaponImpact;
	CAISenseSeeDeadBody * m_pSeeDeadBody;
	CAISenseHearAllyBattleCry * m_pHearAllyBattleCry;

private :

	// Called by RootFSM

	void StartFindNode(RootFSM *);
	void UpdateFindNode(RootFSM *);

	void StartRetreat(RootFSM *);
	void UpdateRetreat(RootFSM *);

	void StartCower(RootFSM *);
	void UpdateCower(RootFSM *);

	void StartSafe(RootFSM *);
	void UpdateSafe(RootFSM *);

	RootFSM		m_RootFSM;	

	const AIBM_Cower & m_butes;

	CAIStrategySetFacing	m_StrategySetFacing;
	CAIStrategyFollowPath	m_StrategyFollowPath;
	CAIStrategyRetreat		m_StrategyRetreat;

	CAINodeCover *			m_pCurNode;

	LTBOOL					m_bValid;
	LTSmartLink				m_hAINodeGroup;

	CTimer					m_tmrCowerDuration;
	CTimer					m_tmrSafe;

	LTSmartLink				m_hTarget;
	LTVector				m_vRelTargetPos;
	LTVector				m_vInitTargetPos;

	LTBOOL					m_bUseRetreat;

	CowerAnimState			m_CowerAnimState;

	std::string				m_strNotifyMsg;
	AIList					m_NotifiedList;

	CTimer					m_tmrNotifyDelay;
	
	int						m_nRunSoundBute;
	CTimer					m_tmrRunSound;
};


#endif // __COWER_H__
