// ----------------------------------------------------------------------- //
//
// MODULE  : AIState.h
//
// PURPOSE : AI State class definitions
//
// CREATED : 12/30/98
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STATE_H__
#define __AI_STATE_H__

#include "LTString.h"
#include "MusicMgr.h"

class CAISense;
class CAI;

class CAIState
{
	public : // Public methods

		// Ctors/Dtors/etc
		CAIState(CAI * pAI);
		virtual ~CAIState() {}

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Methods

		virtual void Start() {}
		virtual void End()   {}

		virtual void PreUpdate() {}
		virtual void Update() {}
		virtual void PostUpdate();


		// Handlers
		virtual bool HandleParameter(const char * const * pszArgs, int nArgs) { return false; }
		virtual bool HandleCommand(const char*const* pTokens, int nArgs) { return false; }

#ifndef _FINAL
		// For debugging
		virtual const char * GetDescription() const { return "Unknown"; }
#endif

	protected : // Protected methods


		CAI * GetAIPtr() { return m_pAI; }
		const CAI * GetAIPtr() const { return m_pAI; }

		// Simple accessors


	private :

		CAI*	m_pAI;						// Backpointer to our AI
		LTBOOL	m_bFirstUpdate;				// Is this our first update?
};

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ CAIState & state)
{
	state.Save(&out);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CAIState & state)
{
	state.Load(&in);
	return in;
}





#endif
