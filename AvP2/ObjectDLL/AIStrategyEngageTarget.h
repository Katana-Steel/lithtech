// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategyEngageTarget.h
//
// PURPOSE : AI strategy class definitions
//
// CREATED : 11-9-00
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_ENGAGE_TARGET_H__
#define __AI_STRATEGY_ENGAGE_TARGET_H__

#include "AIStrategy.h"
#include "AIStrategySetFacing.h"
#include "AIStrategyApproachTarget.h"
#include "AIStrategyRetreat.h"


class CAIStrategyEngageTarget : public CAIStrategy
{
	public :

		CAIStrategyEngageTarget(CAI * pAI);

		virtual ~CAIStrategyEngageTarget() {}

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual void Update();

	private :

		CAIStrategyApproachTarget m_StrategyApproachTarget;
		CAIStrategyRetreat		  m_StrategyRetreat;
};


#endif // __AI_STRATEGY_ENGAGE_TARGET_H__
