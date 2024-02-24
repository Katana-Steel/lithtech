// ----------------------------------------------------------------------- //
//
// MODULE  : AIStrategy.h
//
// PURPOSE : AI strategy class definitions
//
// CREATED : 05.23.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_STRATEGY_H__
#define __AI_STRATEGY_H__

#include "float.h"
#include "ltsmartlink.h"

class CAI;

class CAIStrategy
{

	public : // Public methods

		CAIStrategy(CAI * pAI)
			: m_pAI(pAI) { ASSERT(pAI); }

		virtual void Load(HMESSAGEREAD hRead) {}
		virtual void Save(HMESSAGEWRITE hWrite) {}


		virtual void Update() {}


	protected :

		CAI * GetAIPtr() { return m_pAI; }
		const CAI * GetAIPtr() const { return m_pAI; }

	private :

		CAI *			m_pAI;
};


inline ILTMessage & operator<<(ILTMessage & out, /*const*/ CAIStrategy & strat)
{
	strat.Save(&out);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CAIStrategy & strat)
{
	strat.Load(&in);
	return in;
}




#endif // __AI_STRATEGY_H__
