// ----------------------------------------------------------------------- //
//
// MODULE  : CAIPathMgr.cpp
//
// PURPOSE : CAIPathMgr implementation
//
// CREATED : 12/30/1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIPathMgr.h"
#include "AIPath.h"
#include "AI.h"
#include "AIVolumeMgr.h"
#include "AINodeMgr.h"
#include "AINode.h"

#ifdef _DEBUG
//#include "AIUtils.h"
//#define PATHING_DEBUG
#endif

#ifdef PATHING_DEBUG
//#define ASTAR_DEBUG
#endif

#include "AStar.h"

static uint32 s_nPathingIteration = 0;

// Utility funcitons (these do the bulk of the work).
static LTBOOL InternalFindPath(CAI* pAI, const LTVector& vPosSrc, CAINode * pSrcNode, const LTVector& vPosDest, CAINode* pDestNode, 
							   bool bOrNeighborToDestNode, CAIPath* pPath, LTFLOAT fMaxSearchCost);


class CAIStartPathingNode : public CAINode
{
	LTBOOL m_bIsLadder;
public :

	CAIStartPathingNode(const CAINode & original_node, const LTVector & vNewPos)
		: CAINode(original_node, vNewPos),
		  m_bIsLadder(original_node.IsLadder() ) {}

	virtual LTBOOL IsLadder() const { return m_bIsLadder; }


	virtual const char * GetTypeName() const { return "StartPathingNode"; }
};

// Functors

namespace /*unnamed*/
{
	const LTFLOAT impassable_cost = 1e25f;
	const LTFLOAT g_fMaxSearchCost = 1e5f;


	class NodeHeuristic
	{
		const LTVector goal;

	public:

		NodeHeuristic(const LTVector & new_goal)
			: goal(new_goal) {}

		LTFLOAT operator()(const CAINode & x) const
		{
			if( !x.IsPathingNode() && !x.IsGoal() )
				return impassable_cost;

			return x.GenerateHeuristic( goal, impassable_cost );
		}
	};


	class NodeCost
	{
		const CAI * ai_ptr;

	public:

		NodeCost(const CAI * new_ai_ptr)
			: ai_ptr(new_ai_ptr) {}

		LTFLOAT operator()(const CAINode & src_node, const CAINode & dest_node) const
		{
			// Set cost to a very high value if AI can't use the destination node.
			if( ai_ptr && !ai_ptr->CanUseNode(dest_node) )
				return impassable_cost;

			if( !dest_node.IsPathingNode() && !dest_node.IsGoal() )
			{
				return impassable_cost;
			}

			return src_node.GenerateCost( dest_node, impassable_cost );
		}
	};

} //namespace /*unnamed*/


// Globals

CAIPathMgr* g_pAIPathMgr = LTNULL;

// Statics

static CAINodeMgr s_AINodeMgr;
static CAIVolumeMgr s_AIVolumeMgr;

// Methods

CAIPathMgr::CAIPathMgr()
{
	g_pAIPathMgr = this;
}

CAIPathMgr::~CAIPathMgr()
{
	Term();
}

void CAIPathMgr::Term()
{
	s_AIVolumeMgr.Term();
	s_AINodeMgr.Term();
}

void CAIPathMgr::Init()
{
	s_AINodeMgr.Init();
	s_AIVolumeMgr.Init();
}

void CAIPathMgr::Load(HMESSAGEREAD hRead)
{
	s_AIVolumeMgr.Load(hRead); // This must be called before node mgr's load! See CAINode::Load for why.
	s_AINodeMgr.Load(hRead);

	*hRead >> s_nPathingIteration;
}

void CAIPathMgr::Save(HMESSAGEWRITE hWrite)
{
	s_AIVolumeMgr.Save(hWrite);
	s_AINodeMgr.Save(hWrite);

	*hWrite << s_nPathingIteration;
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, const LTVector& vPosDest, CAIPath* pPath, LTFLOAT fMaxSearchCost /* = -1.0f */ )
{
	_ASSERT( pAI );
	if( !pAI ) return LTFALSE;

	_ASSERT( pPath );
	if( !pPath ) return LTFALSE;

	pPath->ClearWaypoints();

	CAIPathWaypoint waypt(CAINode::kInvalidID, CAIPathWaypoint::eInstructionMoveTo);
	waypt.SetArgumentPosition(vPosDest);
	pPath->AddWaypoint( waypt );

	CAIVolume* pVolumeDest = g_pAIVolumeMgr->FindContainingVolume(vPosDest);

	const LTVector vProjectedDims(pAI->GetDims().x,0.0f,pAI->GetDims().z);
	if( !pVolumeDest )
	{
		return LTFALSE;
	}

	CAINode * pSrcNode = pAI->GetLastNodeReached() ? g_pAINodeMgr->GetNode( pAI->GetLastNodeReached()->GetID() ) : LTNULL;
	CAINode * pDestNode = g_pAINodeMgr->GetNode(pVolumeDest->GetNodeID());

	// A null source node will cause the AI to
	// try to use its volume system.
	if( pSrcNode && pSrcNode->UseVolumePathing() )
	{
		pSrcNode = LTNULL;
	}

	return InternalFindPath(pAI, pAI->GetPosition(), pSrcNode, vPosDest, pDestNode, true, pPath, fMaxSearchCost);
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, const CAINode* pDestNode, CAIPath* pPath, LTFLOAT fMaxSearchCost /* = -1.0f */ )
{
	_ASSERT( pAI );
	if( !pAI ) return LTFALSE;

	_ASSERT( pPath );
	if( !pPath ) return LTFALSE;

	pPath->ClearWaypoints();

	// This functions actually does modify pDestNode.
	// Sorry for the hack.

	CAINode * pSrcNode = pAI->GetLastNodeReached() ? g_pAINodeMgr->GetNode( pAI->GetLastNodeReached()->GetID() ) : LTNULL;

	// A null source node will cause the AI to
	// try to use its volume system.
	if( pSrcNode && pSrcNode->UseVolumePathing() )
	{
		pSrcNode = LTNULL;
	}

	return InternalFindPath(pAI, pAI->GetPosition(), pSrcNode, pDestNode->GetPos(), g_pAINodeMgr->GetNode(pDestNode->GetID()), false, pPath, fMaxSearchCost);
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, const CAIVolume* pVolumeDest, CAIPath* pPath, LTFLOAT fMaxSearchCost /* = -1.0f */ )
{
	_ASSERT( pAI );
	if( !pAI ) return LTFALSE;

	_ASSERT( pPath );
	if( !pPath ) return LTFALSE;

	_ASSERT( pVolumeDest );

	pPath->ClearWaypoints();

	CAINode * pDestNode = pVolumeDest ? g_pAINodeMgr->GetNode(pVolumeDest->GetNodeID()) : LTNULL;

	const LTVector vLocalUp(0,1,0);
	const LTVector & vAIDims = pAI->GetDims();
	const LTVector & vVolumeDims = pVolumeDest->GetDims();
	const LTVector & vCenter = pVolumeDest->GetCenter();
	LTVector vPosDest = vCenter + vLocalUp*(vLocalUp.Dot(vAIDims - vVolumeDims));

	CAINode * pSrcNode = pAI->GetLastNodeReached() ? g_pAINodeMgr->GetNode( pAI->GetLastNodeReached()->GetID() ) : LTNULL;

	// A null source node will cause the AI to
	// try to use its volume system.
	if( pSrcNode && pSrcNode->UseVolumePathing() )
	{
		pSrcNode = LTNULL;
	}
	
	return InternalFindPath(pAI, pAI->GetPosition(), pSrcNode, vPosDest, pDestNode, false, pPath, fMaxSearchCost);
}

LTBOOL CAIPathMgr::FindPath(CAI* pAI, const CCharacter * pChar, CAIPath* pPath, LTFLOAT fMaxSearchCost /* = -1.0f */ )
{
	_ASSERT( pAI );
	if( !pAI ) return LTFALSE;

	_ASSERT( pPath );
	if( !pPath ) return LTFALSE;

	_ASSERT( pChar );
	if( !pChar ) return LTFALSE;

	pPath->ClearWaypoints();


	CAINode * pDestNode = LTNULL;

	bool bUsingVolumeNode = false;

	// Try to find a volume.
	const CAIVolume* pVolumeDest = pChar->GetCurrentVolumePtr();
	if( !pVolumeDest )
	{
		pVolumeDest = pChar->GetLastVolumePtr();

		// Path to the last volume position.
		if( pVolumeDest )
		{
			CAIPathWaypoint waypt(pVolumeDest->GetNodeID(), CAIPathWaypoint::eInstructionMoveTo);
			waypt.SetArgumentPosition( pChar->GetLastVolumePos() );
			pPath->AddWaypoint( waypt );
		}
	}
	
	if( pVolumeDest )
	{
		pDestNode = g_pAINodeMgr->GetNode(pVolumeDest->GetNodeID());
		bUsingVolumeNode = true;
	}
	else if( const CAI* pTargetAI = dynamic_cast<const CAI*>(pChar) )
	{
		// If we are chasing an AI, try to go to its node. (We're really desperate at this point).
		pDestNode = pTargetAI->GetLastNodeReached() ? g_pAINodeMgr->GetNode( pTargetAI->GetLastNodeReached()->GetID() ) : LTNULL;
	}

	CAINode * pSrcNode = pAI->GetLastNodeReached() ? g_pAINodeMgr->GetNode( pAI->GetLastNodeReached()->GetID() ) : LTNULL;

	// If the starting node is a connect to volume node,
	// just null it out to tell the pathing system to
	// use to the volumes.
	if( pSrcNode && pSrcNode->UseVolumePathing() )
	{
		pSrcNode = LTNULL;
	}
	

	return InternalFindPath(pAI, pAI->GetPosition(), pSrcNode, pChar->GetPosition(), pDestNode, bUsingVolumeNode, pPath, fMaxSearchCost);
}

LTBOOL CAIPathMgr::FindSequentialPath(CAI* pAI, CAIVolume* pCurrentVolume, CAIVolume* pPreviousVolume, CAIPath* pPath, LTFLOAT fMaxSearchCost /* = -1.0f */ )
{
	_ASSERT( pAI );
	if( !pAI ) return LTFALSE;

	_ASSERT( pPath );
	if( !pPath ) return LTFALSE;

	if ( !pPreviousVolume || !pCurrentVolume )
	{
		// If there's no previous or current volume we can't do a "sequential" path.
		return LTFALSE;
	}

	if ( pCurrentVolume == pPreviousVolume )
	{
		// Can't do squat if these two are the same volume.

		return LTFALSE;
	}

	pPath->ClearWaypoints();

	// Make a list of sequintial volumes.
	static std::vector<CAIVolume*> aVolumes;
	aVolumes.clear();

	CAIVolume * pInitialVolume = pCurrentVolume;
	while( pCurrentVolume )
	{
		if( pCurrentVolume->GetNumNeighborVolumes() == 2 )
		{
			_ASSERT(   *(pCurrentVolume->BeginNeighbors()) == pPreviousVolume
					 || *(pCurrentVolume->BeginNeighbors()+1) == pPreviousVolume );

			CAIVolume * pFirstNeighbor = *(pCurrentVolume->BeginNeighbors());
			CAIVolume * pSecondNeighbor = *(pCurrentVolume->BeginNeighbors()+1);

			if( pFirstNeighbor == pPreviousVolume )
			{
				aVolumes.push_back(pCurrentVolume);
				pCurrentVolume = pSecondNeighbor;
			}
			else if( pSecondNeighbor == pPreviousVolume )
			{
				aVolumes.push_back(pCurrentVolume);
				pCurrentVolume = pFirstNeighbor;
			}
			else
			{
				aVolumes.push_back(pCurrentVolume);
				pCurrentVolume = LTNULL;
			}
		}
		else
		{
			aVolumes.push_back(pCurrentVolume);
			pCurrentVolume = LTNULL;
		}
	}

	if( aVolumes.size() < 2)
	{
		return LTFALSE;
	}


	const LTVector & my_pos = pAI->GetPosition();

	pCurrentVolume = aVolumes.back();
	aVolumes.pop_back();
	
	LTBOOL result = LTTRUE;

	while( !aVolumes.empty() && result )
	{
		CAINode * pSrcNode = g_pAINodeMgr->GetNode( aVolumes.back()->GetNodeID() );
		CAINode * pDestNode = g_pAINodeMgr->GetNode( pCurrentVolume->GetNodeID() );

		if( pSrcNode && pDestNode )
		{
			result = InternalFindPath(pAI, my_pos, pSrcNode, pDestNode->GetPos(), pDestNode, false, pPath, fMaxSearchCost);

			pCurrentVolume = aVolumes.back();
			aVolumes.pop_back();
		}
	};


	return result;
}


static LTBOOL InternalFindPath(CAI* pAI, const LTVector& vPosSrc, CAINode * pSrcNode, const LTVector& vPosDest, CAINode * pDestNode, bool bOrNeighborToDestNode, CAIPath* pPath, LTFLOAT fMaxSearchCost  )
{
	_ASSERT( pAI );
	if( !pAI ) return LTFALSE;

	_ASSERT( pPath );
	if( !pPath ) return LTFALSE;

	if ( !pDestNode )
	{
		// If there's no dest volume we're screwed.

		return LTFALSE;
	}

	if( !pAI->CanUseNode(*pDestNode) )
	{
		// We're not allowed to use this node (leash, locked, unable to wallwalk, etc)
		return LTFALSE;
	}

	if( fMaxSearchCost < 0.0f )
	{
		fMaxSearchCost = g_fMaxSearchCost;
	}

	// If pSrcNode is false, we should start with the volume system.
	bool put_back_in_volume = false;
	if ( !pSrcNode )
	{
		// Try to find a containing volume.
		CAIVolume * pVolumeSrc = pAI->GetCurrentVolumePtr();
		
		// If we aren't in a volume, try our last reached node.
		if( !pVolumeSrc ) 
		{
			// Maybe we aren't in a volume, lets use our last reached node.
			if( pAI->GetLastNodeReached() )
			{
				pSrcNode = g_pAINodeMgr->GetNode( pAI->GetLastNodeReached()->GetID() );
				
				// If that node uses volume pathing, use our last volume instead.
				if( pSrcNode->UseVolumePathing() )
				{
					pSrcNode = LTNULL;
				}
			}
		}
		
		// If we still haven't found a starting point, try to move to our last volume point.
		if( !pSrcNode && !pVolumeSrc)
		{
			// Okay, try our last volume.
			pVolumeSrc = pAI->GetLastVolumePtr();

			if( pVolumeSrc )
			{
				put_back_in_volume = true;
			}

		}

		// Now extract a node from the volume, if we are in a volume (or are returning to our last volume).
		if( !pSrcNode && pVolumeSrc )
		{
			pSrcNode = g_pAINodeMgr->GetNode(pVolumeSrc->GetNodeID());
		}

		_ASSERT( pSrcNode );
		if( !pSrcNode )
		{
			// We have to start somewhere!
			return LTFALSE;
		}
	}
	
	if ( pSrcNode == pDestNode )
	{
		// No need to path!
		return LTTRUE;
	}

#ifdef PATHING_DEBUG
	LTCounter pathing_counter;
	g_pServerDE->StartCounter(&pathing_counter);
#endif

	// Setup all the nodes

	static std::vector<CAINode*> path_heap;


	_ASSERT( pSrcNode && pDestNode );
	if( !pDestNode || !pSrcNode ) return LTFALSE;


	++s_nPathingIteration;

	pSrcNode->ResetPathing(s_nPathingIteration);

	CAIStartPathingNode start_node(*pSrcNode,vPosSrc);
	start_node.ResetPathing(s_nPathingIteration);

	ASSERT( pDestNode );

	pDestNode->ResetPathing(s_nPathingIteration);
	pDestNode->SetIsGoal(true);

	if( bOrNeighborToDestNode )
	{
		for( CAINode::NeighborIterator iter = pDestNode->BeginNeighbors();
		     iter != pDestNode->EndNeighbors(); ++iter )
		{
			(*iter)->ResetPathing(s_nPathingIteration);
			(*iter)->SetIsGoal(true);
		}
	}

	CAINode * const pEndNode = AStar::FindPath<CAINode>(start_node,NodeCost(pAI),NodeHeuristic(vPosDest),path_heap,fMaxSearchCost);

	if( !pEndNode )
	{
		return LTFALSE;
	}

	if( pEndNode->IsGoal() )
	{
		LTVector vCurrentDest = vPosDest;
		CAINode * pCurrentNode = pEndNode;
		const CAINode * pNextNodeForAI = pAI->GetNextNode();
		while( pCurrentNode )
		{
			if( pCurrentNode->GetCost() < g_fMaxSearchCost )
			{
				if( pCurrentNode != &start_node )
				{
					vCurrentDest = pCurrentNode->AddToPath(pPath,pAI,vPosSrc,vCurrentDest);
				}
				else
				{
					// The start node is temporary.  We want the pathing
					// to use the original node.
					_ASSERT( pSrcNode );
					if( pSrcNode )
					{
						vCurrentDest = pSrcNode->AddToPath(pPath,pAI,vPosSrc,vCurrentDest);
					}
				}

			}

			if( pCurrentNode == pNextNodeForAI 
				|| (     pCurrentNode->UseVolumePathing() 
				      && pAI->GetCurrentVolumePtr()
					  && pAI->GetCurrentVolumePtr() == pCurrentNode->GetContainingVolumePtr() ) )
			{
				// If the AI is already heading toward a node in the path, no need
				// to go to any previous nodes.
				// Or if the AI is in the same volume as a node in the path.
				pCurrentNode = LTNULL;
			}
			else
			{
				pCurrentNode = pCurrentNode->GetPrevNode();
			}
		}

		if( put_back_in_volume )
		{
			CAIPathWaypoint waypt(pAI->GetLastVolumePtr()->GetNodeID(), CAIPathWaypoint::eInstructionMoveTo);
			waypt.SetArgumentPosition(pAI->GetLastVolumePos());

			pPath->AddWaypoint(waypt);
		}

#ifdef PATHING_DEBUG
		uint32 pathing_ticks = g_pServerDE->EndCounter(&pathing_counter);

		AICPrint(pAI->m_hObject, "Pathing took %f seconds.",
			double(pathing_ticks)*10e-6 );
#endif 


		return pEndNode->GetCost() < g_fMaxSearchCost;
	}

	return LTFALSE;
}
