// ----------------------------------------------------------------------- //
//
// MODULE  : Goal.h
//
// PURPOSE : Implements the Goal class.
//
// CREATED : 5/19/00
//
// ----------------------------------------------------------------------- //

#ifndef __GOAL_H__
#define __GOAL_H__

#include "MusicMgr.h"
#include "Timer.h"

#include <string>

class CAIState;
class CAI;

class Goal
{
public :

	Goal(CAI * new_ai_ptr, std::string new_create_name)
		: aiPtr(new_ai_ptr),
		  createName(new_create_name),
		  m_bActive(false),
		  m_nRandomSoundBute(-1) {}

	virtual ~Goal() {}

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual void Start();
	virtual void End() {  m_bActive = false; }

	virtual bool DestroyOnEnd() const { return false; }

	void HandleGoalParameters(const char * szParameters);

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs) { return false; }
	virtual bool HandleCommand(const char * const * pTokens, int nArgs)   { return false; }

	virtual void Update();

	virtual LTFLOAT GetBid() = 0;
	virtual LTFLOAT GetMaximumBid() = 0;

	const std::string & GetCreateName() const { return createName; }

	const bool IsActive() const { return m_bActive; }

	virtual const char * GetRandomSoundName() const { return LTNULL; } 
	virtual const char * GetInitialAnimationSet() const { return LTNULL; }

	virtual LTBOOL StopCinematic() const { return LTFALSE; }
	virtual LTBOOL DefaultAlert() const { return LTFALSE; }

	virtual CMusicMgr::EMood GetMusicMoodModifier( ) = 0;

	// For Debugging
#ifndef _FINAL
	virtual const char * GetDescription() const = 0;
#endif

protected :

	CAI * GetAIPtr()           { return aiPtr; }
	const CAI * GetAIPtr() const { return aiPtr; }


private :

	CAI * aiPtr;
	
	std::string createName;
	bool m_bActive;

	int    m_nRandomSoundBute;
	CTimer m_tmrRandomSound;

};

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ Goal & goal)
{
	goal.Save(&out);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, Goal & goal)
{
	goal.Load(&in);
	return in;
}



#endif // __GOAL_H__
