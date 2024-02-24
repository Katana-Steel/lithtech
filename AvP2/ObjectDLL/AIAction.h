// ----------------------------------------------------------------------- //
//
// MODULE  : AIAction.h
//
// PURPOSE : The base action
//
// CREATED : 2/6/01
//
// ----------------------------------------------------------------------- //

#ifndef AI_ACTION_H
#define AI_ACTION_H

#include "CharacterMovementDefs.h"

// All the default functionality is for an action which will not stop until
// it has run to completion.

class CAI;
struct ArgList;

class AIAction
{
public:

	AIAction(CAI * pAI, LTBOOL bDone = LTFALSE)
		: m_pAI(pAI),
		  m_bDone(bDone)  {}

	virtual ~AIAction() {}

	virtual void Load(HMESSAGEREAD  hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	// Called when the action is first set.  Right before Update().
	virtual void Start() {}

	// This is called each frame.
	virtual void Update() {}

	// Called when the action is finished.
	virtual void End() {}

	// Called when a request to stop the action comes in.
	virtual void Interrupt()  {}

	// CanStop should be true if the Stop() request will end the action "soon".
	virtual LTBOOL IsInterruptible() const { return LTFALSE;  }

	// Is called when a MID_ANIMATION message is received.
	virtual void AnimMsg(uint8 nTracker, uint8 nLooping, uint8 nInfo) {}

	// Is called when a MID_AFFECTPHYSICS message is received.
	virtual void PhysicsMsg(LTVector * pNewOffset) {}

	// Called when a MID_MODELSTRINGKEY is received.
	virtual void ModelStringKeyMsg(ArgList* pArgList) {}

	// IsDone means the action is doing nothing else and should be removed as the current action.
	// This does not mean that the action truly "succeeded", only that the action will not do anything further.
	virtual LTBOOL IsDone() const { return m_bDone;  }

	// When the AI goes into a special movement state which doesn't get updates,
	// it will clear out the current action unless the action returns true
	// with this function.
	virtual LTBOOL UsesMovementState(CharacterMovementState cms) { return LTFALSE; }

	// This is used for debugging.  It should return a string description of the action.
	virtual const char * GetActionDescription() const = 0;

	CAI* GetAIPtr()             { return m_pAI; }
	const CAI* GetAIPtr() const { return m_pAI; }

protected :

	LTBOOL m_bDone;

private:

	CAI * m_pAI;
};

inline void AIAction::Load(HMESSAGEREAD  hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	*hRead >> m_bDone;
}

inline void AIAction::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	*hWrite << m_bDone;
}

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ AIAction & action)
{
	action.Save(&out);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, AIAction & action)
{
	action.Load(&in);
	return in;
}



#endif // AI_ACTION_H
