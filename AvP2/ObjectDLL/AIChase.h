// ----------------------------------------------------------------------- //
//
// MODULE  : Chase.h
//
// PURPOSE : Implements the Chase goal and state.
//
// CREATED : 5/26/00
//
// ----------------------------------------------------------------------- //

#ifndef __CHASE_H__
#define __CHASE_H__

#include "Goal.h"
#include "AIState.h"
#include "AIStrategyApproachTarget.h"

class CAISenseSeeEnemy;
class CAISenseHearEnemyFootstep;
class CAISenseHearEnemyWeaponFire;

class CAIStateChase : public CAIState
{

	public : // Public methods

		CAIStateChase(CAI * pAI);
		
		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Overloaded Methods
		virtual void Start();

		virtual void Update();
		
	protected :


	private :

		CAIStrategyApproachTarget m_StrategyApproachTarget;
};

class ChaseGoal : public Goal
{
public :

	ChaseGoal( CAI * pAI, std::string create_name );

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Start();
	virtual void End();

	virtual void Update() { chaseState.Update(); }

	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodNone; }
#ifndef _FINAL
	virtual const char * GetDescription() const { return "Chase"; }
#endif

protected :

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

private :

	CAIStateChase chaseState;
	CAISenseSeeEnemy * seeEnemy;
	CAISenseHearEnemyFootstep * hearEnemyFootstep;
	CAISenseHearEnemyWeaponFire * hearEnemyWeaponFire;

	LTSmartLink  m_hTarget;
};

#endif // __CHASE_H__
