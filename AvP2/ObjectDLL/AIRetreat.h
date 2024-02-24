// ----------------------------------------------------------------------- //
//
// MODULE  : AIRetreat.h
//
// PURPOSE : Implements the Retreat goal and state.
//
// CREATED : 9/08/00
//
// ----------------------------------------------------------------------- //

#ifndef __RETREAT_H__
#define __RETREAT_H__

#include "fsm.h"
#include "AICombatGoal.h"
#include "AIState.h"
#include "AITarget.h"
#include "AIStrategyRetreat.h"
#include "AIStrategySetFacing.h"
#include "Timer.h"

class RetreatGoal : public AICombatGoal
{
public :

	RetreatGoal( CAI * pAI, std::string create_name );

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
	virtual const char * GetDescription() const { return "Retreat"; }
#endif
	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodWarning; }
	virtual const char * GetInitialAnimationSet() const { return "Retreat"; }


protected :

	CAISenseSeeEnemy * m_pSeeEnemy;

private :

	CAIStrategyRetreat		m_StrategyRetreat;
	CAIStrategySetFacing	m_StrategySetFacing;

	LTSmartLink				m_hTarget;
};


#endif // __RETREAT_H__
