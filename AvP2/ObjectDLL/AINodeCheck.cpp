// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCheck.cpp
//
// PURPOSE : Implements the NodeCheck goal and state.
//
// CREATED : 9/05/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AINodeCheck.h"
#include "AI.h"
#include "AINodeMgr.h"
#include "AINode.h"
#include "AIUtils.h"
#include "AIButeMgr.h"

#include <algorithm>

#ifdef _DEBUG
//#define MILD_NODECHECK_DEBUG
//#include "DebugLineSystem.h"
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::CAIStateNodeCheck
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CAIStateNodeCheck::CAIStateNodeCheck(CAI * pAI)
	: CAIState(pAI),
	  m_bWallWalkOnly(false),
	  m_fRadiusSqr(1.0e12f),
	  m_bEmptyListNotify(false),
	  m_StrategyFollowPath(pAI)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::HandleParameter
//
//	PURPOSE:	Sets data from name/value pairs
//
// ----------------------------------------------------------------------- //

bool CAIStateNodeCheck::HandleParameter(const char * const * pszArgs, int nArgs)
{
	if( CAIState::HandleParameter(pszArgs, nArgs) ) return true;

	if( 0 == stricmp(*pszArgs, "WallWalkOnly") )
	{
		m_bWallWalkOnly = true;
	}
	else
	{
		m_fRadiusSqr = (LTFLOAT)atof(*pszArgs);
		m_fRadiusSqr *= m_fRadiusSqr;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::Start
//
//	PURPOSE:	Called when state is started.
//
// ----------------------------------------------------------------------- //

void CAIStateNodeCheck::Start()
{
	const LTVector vSourcePos = GetAIPtr()->GetPosition();

	m_NodeList.clear();
	for( CAINodeMgr::NodeList::iterator iter = g_pAINodeMgr->BeginNodes();
		 iter != g_pAINodeMgr->EndNodes(); ++iter )
	{
		if( !m_bWallWalkOnly || ((*iter)->IsWallWalkOnly()) )
		{
			if( ((*iter)->GetPos() - vSourcePos).MagSqr() < m_fRadiusSqr )
			{
				if( GetAIPtr()->CanUseNode(*(*iter)) )
				{
					m_NodeList.push_back(*iter);
				}
			}
		}
	}

	std::random_shuffle(m_NodeList.begin(),m_NodeList.end());
	m_CurrentNode = m_NodeList.begin();
	m_bEmptyListNotify = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::Upate
//
//	PURPOSE:	Update function.
//
// ----------------------------------------------------------------------- //

void CAIStateNodeCheck::Update()
{
	CAIState::Update();

	if( m_NodeList.empty() && !m_bEmptyListNotify)
	{
		// Tell the user that we have no nodes (either there are none
		// in the radius given to us, or they were all removed becuase
		// we couldn't path to them).
		m_bEmptyListNotify = true;

#ifndef _FINAL
		AIErrorPrint(GetAIPtr(),"In goal CheckNodes with no nodes to check!");
#endif
		return;
	}

	if( !m_StrategyFollowPath.IsSet() )
	{
#ifndef _FINAL
		// Check to for being stuck and notify user.
		if( m_StrategyFollowPath.IsStuck() )
		{
			AIErrorPrint(GetAIPtr(),"Stuck at (%.2f,%.2f,%.2f) while going to node \"%s\" at (%.2f,%.2f,%.2f).",
				GetAIPtr()->GetPosition().x,GetAIPtr()->GetPosition().y,GetAIPtr()->GetPosition().z,
				(*m_CurrentNode)->GetName().c_str(),
				(*m_CurrentNode)->GetPos().x,(*m_CurrentNode)->GetPos().y,(*m_CurrentNode)->GetPos().z );
		}
#endif

		// Increment m_CurrentNode.  If at end of list, set-up next permutation
		// of elements and path again.
		// Becuase of the call to erase below, it could be that m_CurrentNode
		// is already at end.  So we need to check before we increment it!
		if(      m_CurrentNode == m_NodeList.end() 
			|| ++m_CurrentNode == m_NodeList.end() )
		{
			std::random_shuffle(m_NodeList.begin(),m_NodeList.end());
			m_CurrentNode = m_NodeList.begin();
		}


		// Now try to set the path.
		// If we can't set a path to the current node, remove it from the list.
		if( m_CurrentNode != m_NodeList.end() )
		{
			if( !m_StrategyFollowPath.Set(*m_CurrentNode) )
			{
#ifndef _FINAL
				AIErrorPrint(GetAIPtr(),"Could not path to node \"%s\" at (%.2f,%.2f,%.2f).  Removing from list.",
					(*m_CurrentNode)->GetName().c_str(),
					(*m_CurrentNode)->GetPos().x, (*m_CurrentNode)->GetPos().y, (*m_CurrentNode)->GetPos().z );
#endif
				m_CurrentNode = m_NodeList.erase(m_CurrentNode);
				if( m_NodeList.empty() )
					m_CurrentNode = m_NodeList.begin();

				return;
			}
		}
		else
		{
			// The node list is empty, 
			// the next call to Update will handle this situation.
			return;
		}


		// Alright, everything is set.  Tell the user
		// which node we are attempting to get to.
#ifdef MILD_NODECHECK_DEBUG
		AICPrint(GetAIPtr(),"Heading to node \"%s\" at (%.2f,%.2f,%.2f).",
			(*m_CurrentNode)->GetName().CStr(),
			(*m_CurrentNode)->GetPos().x, (*m_CurrentNode)->GetPos().y, (*m_CurrentNode)->GetPos().z );

#endif
	}

#ifdef MILD_NODECHECK_DEBUG
	if( m_CurrentNode != m_NodeList.end() )
	{
		LineSystem::GetSystem(this,"ShowCurrentNode")
			<< LineSystem::Clear()
			<< LineSystem::Box((*m_CurrentNode)->GetPos(),LTVector(16,16,16),Color::Purple)
			<< LineSystem::Arrow(GetAIPtr()->GetPosition(),(*m_CurrentNode)->GetPos(),Color::Purple);
	}
#endif

	m_StrategyFollowPath.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::Stop
//
//	PURPOSE:	Called when state is stopped.
//
// ----------------------------------------------------------------------- //

void CAIStateNodeCheck::Stop()
{
#ifdef MILD_NODECHECK_DEBUG
	LineSystem::GetSystem(this,"ShowCurrentNode")
		<< LineSystem::Clear();
#endif
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::GetDescription
//
//	PURPOSE:	Returns the description of the goal.
//
// ----------------------------------------------------------------------- //

#ifndef _FINAL
const char * CAIStateNodeCheck::GetDescription() const
{ 
	static char szDescription[50];

	sprintf(szDescription,"NodeCheck(%f%s)",
		sqrt(m_fRadiusSqr), m_bWallWalkOnly ? ",WW":"" );

	return szDescription;
}
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::Load
//
//	PURPOSE:	Restores the State
//
// ----------------------------------------------------------------------- //

void CAIStateNodeCheck::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);

	CAIState::Load(hRead);

	*hRead >> m_bWallWalkOnly;
	*hRead >> m_fRadiusSqr;
	*hRead >> m_bEmptyListNotify;

	uint32 node_list_size = hRead->ReadDWord();
	for(uint32 i = 0; i < node_list_size; ++i )
	{
		m_NodeList.push_back( g_pAINodeMgr->GetNode( hRead->ReadDWord() ) );
	}

	m_CurrentNode = m_NodeList.begin() + hRead->ReadDWord();
	if( (uint32)(m_CurrentNode - m_NodeList.begin()) >= m_NodeList.size() )
	{
		m_CurrentNode = m_NodeList.end();
	}

	*hRead >> m_StrategyFollowPath;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIStateNodeCheck::Save
//
//	PURPOSE:	Saves the State
//
// ----------------------------------------------------------------------- //

void CAIStateNodeCheck::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);

	CAIState::Save(hWrite);

	*hWrite << m_bWallWalkOnly;
	*hWrite << m_fRadiusSqr;
	*hWrite << m_bEmptyListNotify;

	hWrite->WriteDWord( m_NodeList.size() );
	for( NodeList::iterator iter = m_NodeList.begin();
	     iter != m_NodeList.end(); ++iter )
	{
		hWrite->WriteDWord( (*iter)->GetID() );
	}

	hWrite->WriteDWord(m_CurrentNode - m_NodeList.begin());

	*hWrite << m_StrategyFollowPath;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	NodeCheckGoal::Load/Save
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

LTFLOAT NodeCheckGoal::GetBid()
{ 
	if (m_NodeCheckState.HasEmptyList())
		return 0.0f;
	
	return g_pAIButeMgr->GetDefaultBid(BID_NODECHECK);
}

LTFLOAT NodeCheckGoal::GetMaximumBid()
{ 
	return g_pAIButeMgr->GetDefaultBid(BID_NODECHECK);
}

void NodeCheckGoal::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	Goal::Load(hRead);

	*hRead >> m_NodeCheckState;
}


void NodeCheckGoal::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	Goal::Save(hWrite);
	
	*hWrite << m_NodeCheckState;
}

