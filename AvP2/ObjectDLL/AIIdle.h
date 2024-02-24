// ----------------------------------------------------------------------- //
//
// MODULE  : Idle.h
//
// PURPOSE : Implements the Idle goal and state.
//
// CREATED : 10/23/00
//
// ----------------------------------------------------------------------- //

#ifndef __IDLE_H__
#define __IDLE_H__

#include "Goal.h"
#include "AIState.h"


class IdleGoal : public Goal
{
public :

	IdleGoal( CAI * pAI, std::string create_name )
		: Goal(pAI,create_name) {}

	virtual LTFLOAT GetBid()  { return 1.0f; }
	virtual LTFLOAT GetMaximumBid() { return 1.0f; }

	virtual void Start() { Goal::Start(); GetAIPtr()->SetNextAction(CAI::ACTION_IDLE); GetAIPtr()->SetMovementStyle(); }

	const char * GetRandomSoundName() const { return "RandomIdle"; } 
	virtual const char * GetInitialAnimationSet() const { return "Idle"; }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "Idle"; }
#endif
	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodAmbient; }

};

#endif // __IDLE_H__
