// ----------------------------------------------------------------------- //
//
// MODULE  : CSimpleNodeMgr.cpp
//
// PURPOSE : CSimpleNodeMgr implementation
//
// CREATED : 7/8/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SimpleNodeMgr.h"
#include "SimpleNode.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AStar.h"

#include <algorithm>

// Globals

CSimpleNodeMgr* g_pSimpleNodeMgr = LTNULL;

// Externs

extern int g_cIntersectSegmentCalls;

namespace /*unnamed*/
{
	const LTFLOAT g_fMaxSearchCost = 1e5f;


	class SimpleNodeHeuristic
	{
		const LTVector goal;

	public:

		SimpleNodeHeuristic(const LTVector & new_goal)
			: goal(new_goal) {}

		LTFLOAT operator()(const CSimpleNode & x) const
		{
			return (x.GetPos() - goal).Mag();
		}
	};


	class SimpleNodeCost
	{

	public:

		LTFLOAT operator()(const CSimpleNode & src_node, const CSimpleNode & dest_node) const
		{
			return (src_node.GetPos() - dest_node.GetPos()).Mag();
		}
	};

}; //namespace unnamed

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::CSimpleNodeMgr
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CSimpleNodeMgr::CSimpleNodeMgr()
{
	g_pSimpleNodeMgr = this;

	m_bInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::Term
//
//	PURPOSE:	Terminates the SimpleNodeMgr
//
// ----------------------------------------------------------------------- //

void CSimpleNodeMgr::Term()
{
	m_bInitialized = LTFALSE;

	while( !m_apNodes.empty() )
	{
		delete m_apNodes.back();
		m_apNodes.pop_back();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::Init
//
//	PURPOSE:	Create a list of all the Nodes in the level.
//
// ----------------------------------------------------------------------- //

void CSimpleNodeMgr::Init()
{
	// Nothing to be done.
	m_bInitialized = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::FindNeighbors
//
//	PURPOSE:	Finds neighbors of a node in the list of known nodes.
//				And then adds the node.
//				
//
// ----------------------------------------------------------------------- //

void CSimpleNodeMgr::AddNode(const SimpleNode & new_node)
{

	CSimpleNode * pCSimpleNode = new CSimpleNode(new_node, m_apNodes.size());

	for( SimpleNodeList::iterator iter = m_apNodes.begin(); iter != m_apNodes.end(); ++iter )
	{
		CSimpleNode * pCurrentNode = *iter;

		if( (float)fabs( pCurrentNode->GetPos().y - pCSimpleNode->GetPos().y ) < pCurrentNode->GetHalfHeight() + pCSimpleNode->GetHalfHeight() )
		{
			const LTFLOAT x_diff = pCurrentNode->GetPos().x - pCSimpleNode->GetPos().x;
			const LTFLOAT z_diff = pCurrentNode->GetPos().z - pCSimpleNode->GetPos().z;
			const LTFLOAT total_radius = pCurrentNode->GetRadius() + pCSimpleNode->GetRadius();

			if( x_diff*x_diff + z_diff*z_diff < total_radius*total_radius )
			{
				pCSimpleNode->AddNeighbor( pCurrentNode->GetID() );
				pCurrentNode->AddNeighbor( pCSimpleNode->GetID() );
			}
		}

	}

	m_apNodes.push_back( pCSimpleNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::FindPath
//
//	PURPOSE:	Uses A-Star to find a path through the node connections.
//
// ----------------------------------------------------------------------- //

bool CSimpleNodeMgr::FindPath(const LTVector &vGoalPos, const CSimpleNode * pDest, const CSimpleNode * pSrc)
{
	static uint32 s_nSimplePathingIteration = 0;
	static std::vector<CSimpleNode*> path_heap;


	_ASSERT( pSrc && pDest );
	if( !pDest || !pSrc ) 
		return false;

	// This is a hack to get non-const versions of the source and destination nodes.
	CSimpleNode * pSrcNode = GetNode(pSrc->GetID());
	CSimpleNode * pDestNode = GetNode(pDest->GetID());

	++s_nSimplePathingIteration;

	pSrcNode->ResetPathing(s_nSimplePathingIteration);

	pDestNode->ResetPathing(s_nSimplePathingIteration);
	pDestNode->SetIsGoal(true);

	if( pSrcNode == pDestNode )
		return false;

	CSimpleNode * const pEndNode = AStar::FindPath<CSimpleNode>(*pSrcNode,SimpleNodeCost(),SimpleNodeHeuristic(vGoalPos),path_heap,g_fMaxSearchCost);

	return pEndNode && pEndNode->IsGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::Load
//
//	PURPOSE:	Restores the state of the SimpleNodeMgr
//
// ----------------------------------------------------------------------- //

void CSimpleNodeMgr::Load(HMESSAGEREAD hRead)
{
	uint32 dwNodes, iNode;

	ASSERT( hRead );
	if( !hRead ) return;

	Term();

	*hRead >> m_bInitialized;

	dwNodes = hRead->ReadDWord();
	for ( iNode = 0 ; iNode < dwNodes; iNode++ )
	{
		CSimpleNode * pCSimpleNode = new CSimpleNode();

		*hRead >> (*pCSimpleNode);
		m_apNodes.push_back(pCSimpleNode);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::Save
//
//	PURPOSE:	Saves the state of the SimpleNodeMgr
//
// ----------------------------------------------------------------------- //

void CSimpleNodeMgr::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	*hWrite << m_bInitialized;

	hWrite->WriteDWord(m_apNodes.size());
	for ( SimpleNodeList::iterator iter = m_apNodes.begin();
	      iter != m_apNodes.end(); ++iter )
	{
		// We should never have a null pointer in this list
		_ASSERT(*iter);

		*hWrite << *(*iter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::AddNodeDebug
//
//	PURPOSE:	Add the node models
//
// ----------------------------------------------------------------------- //

class SortNodesAround
{
	const LTVector & vAroundPos;
public :

	SortNodesAround( const LTVector & vPos )
		: vAroundPos(vPos) {}

	bool operator()( const CSimpleNode * pNodeA, const CSimpleNode * pNodeB)
	{
		return (pNodeA->GetPos() - vAroundPos).MagSqr() < (pNodeB->GetPos() - vAroundPos).MagSqr();
	}
};

#ifndef _FINAL

void CSimpleNodeMgr::AddNodeDebug(int level)
{
	std::vector<CSimpleNode*> sorted_nodes;


	{for(SimpleNodeList::iterator iter = m_apNodes.begin();
	    iter != m_apNodes.end(); ++iter)
	{
		sorted_nodes.push_back( (*iter) );
	}}


	CCharacterMgr::PlayerIterator player_iter = g_pCharacterMgr->BeginPlayers();
	if( player_iter != g_pCharacterMgr->EndPlayers() )
	{
		CPlayerObj * pPlayerObj = *player_iter;
		const LTVector & vPlayerPos = pPlayerObj->GetPosition();

		std::sort( sorted_nodes.begin(), sorted_nodes.end(), SortNodesAround( vPlayerPos ) );
	}

	for ( SimpleNodeList::iterator iter = m_apNodes.begin();
	      iter != m_apNodes.end(); ++iter )
	{
		(*iter)->DrawDebug(level);
	}
}

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNodeMgr::RemoveNodeDebug
//
//	PURPOSE:	Removes the node models
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL

void CSimpleNodeMgr::RemoveNodeDebug()
{
	for ( SimpleNodeList::iterator iter = m_apNodes.begin();
		  iter != m_apNodes.end(); ++iter )
	{
		(*iter)->ClearDebug();
	}
}

#endif
