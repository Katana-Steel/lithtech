// ----------------------------------------------------------------------- //
//
// MODULE  : NodeCheck.h
//
// PURPOSE : Implements the NodeCheck goal and state.
//
// CREATED : 10/23/00
//
// ----------------------------------------------------------------------- //

#ifndef __NODECHECK_H__
#define __NODECHECK_H__

#include "Goal.h"
#include "AIState.h"

#include "AIStrategyFollowPath.h"

#include <vector>

class CAINode;

class CAIStateNodeCheck : public CAIState
{
	public :

		typedef std::vector<CAINode*> NodeList;

	public :

		CAIStateNodeCheck(CAI * pAI);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);
		
		// Overloaded Methods
		virtual void Start();
		virtual void Update();
		virtual void Stop();

#ifndef _FINAL
		virtual const char * GetDescription() const;
#endif
		virtual bool HandleParameter(const char * const * pszArgs, int nArgs);

		bool HasEmptyList() const { return m_bEmptyListNotify; }
	private :

		bool	  m_bWallWalkOnly;
		LTFLOAT	  m_fRadiusSqr;
		bool	  m_bEmptyListNotify;
		NodeList  m_NodeList;

		NodeList::iterator m_CurrentNode;
		CAIStrategyFollowPath m_StrategyFollowPath;
};

class NodeCheckGoal : public Goal
{
public :

	NodeCheckGoal( CAI * pAI, std::string create_name )
		: Goal(pAI,create_name),
		  m_NodeCheckState(pAI) {}

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual bool HandleParameter(const char * const * pszArgs, int nArgs) 
	{ 
		if( Goal::HandleParameter(pszArgs,nArgs) ) return true;

		return m_NodeCheckState.HandleParameter(pszArgs,nArgs);
	}
		

	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();


	virtual void Start() { m_NodeCheckState.Start(); }
	virtual void End()   { m_NodeCheckState.End(); }

	virtual void Update() { m_NodeCheckState.Update(); }

#ifndef _FINAL
	virtual const char * GetDescription() const { return "NodeCheck"; }
#endif
	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodNone; }

private :

	CAIStateNodeCheck m_NodeCheckState;
};

#endif // __NODECHECK_H__
