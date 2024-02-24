// ----------------------------------------------------------------------- //
//
// MODULE  : AIMeleeAttack.h
//
// PURPOSE : Implements the Melee Attack goal and state.
//
// CREATED : 8/1/00
//
// ----------------------------------------------------------------------- //

#ifndef __AI_MELEE_ATTACK_H__
#define __AI_MELEE_ATTACK_H__

#include "AIAttack.h"
#include "Timer.h"
#include "Range.h"


class CAIStateMeleeAttack : public CAIStateAttack
{

	public:

		CAIStateMeleeAttack(CAI * pAI);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		virtual void Start();

	protected :

		virtual void ApproachTarget();
		virtual void FireOnTarget();

		LTFLOAT GetJumpDelay();
		
		void PerformJumping();

	private :

		CTimer  m_tmrJumpDelay;


		CAIStrategyFollowPath m_StrategyFollowPath;
		const CAINode * m_pDestNode;

		// constant attributes
		LTFLOAT	m_fJumpProbability;
		FRange	m_rngJumpDelay;
};

class MeleeAttackGoal : public AttackGoal
{
	public:
		MeleeAttackGoal( CAI * pAI, std::string create_name )
			: AttackGoal(pAI,create_name, new CAIStateMeleeAttack(pAI)) {}

		virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodHostile; }
};


#endif //__AI_MELEE_ATTACK_H__
