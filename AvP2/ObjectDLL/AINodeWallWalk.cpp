// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeWallWalk.cpp
//
// PURPOSE : AINodeWallWalk implementation
//
// CREATED : 10/25/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AINodeWallWalk.h"
#include "AINodeMgr.h"
#include "DebugLineSystem.h"
#include "AIVolume.h"
#include "AIPath.h"
#include "AIVolumeMgr.h"
#include "AIUtils.h"

#include <algorithm>

#ifdef _DEBUG
//#define WALLWALK_DEBUG
#endif

////////////////////////////////////////////////////////////////////////////
//
//      AINodeWallWalk
//
////////////////////////////////////////////////////////////////////////////


BEGIN_CLASS(AINodeWallWalk)

	ADD_VECTORPROP_VAL_FLAG_HELP(Dims, 20.0f, 20.0f, 20.0f, PF_DIMS, "Wall walk nodes must have a brush face within their dimensions.")

	PROP_DEFINEGROUP(NeighborLinks, PF_HIDDEN)

	ADD_NEIGHBOR_LINK(1, 0)
	ADD_NEIGHBOR_LINK(2, 0)
	ADD_NEIGHBOR_LINK(3, 0)
	ADD_NEIGHBOR_LINK(4, 0)
	ADD_NEIGHBOR_LINK(5, 0)

	ADD_OBJECTPROP_FLAG_HELP(JumpTo, "", 0, "AI will jump to this point.")
	ADD_BOOLPROP_FLAG_HELP(TwoWayJump, LTTRUE, 0, "If true, the AI will be able to jump from the JumpTo point as well." )

	ADD_BOOLPROP_FLAG( WallWalk, LTTRUE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG( MoveToFloor, LTFALSE, PF_HIDDEN )

	ADD_BOOLPROP_FLAG_HELP( AdvancePoint, LTFALSE, 0, "If true, the AI will use this as a possible advance point when attacking the player.")
	ADD_BOOLPROP_FLAG_HELP( DropPoint, LTFALSE, 0, "The AI can drop from this point to an AIVolume below.")

	ADD_BOOLPROP_FLAG_HELP( UseBaseName, LTFALSE, 0, "If true, this node will connect to the next node with the same base name.")

//	ADD_BEZIER_LINK( 0 )

END_CLASS_DEFAULT_FLAGS(AINodeWallWalk, AINode, NULL, NULL, 0)


CAINode * AINodeWallWalk::MakeCAINode(uint32 dwID)
{
	return CheckCAINode(new CAINodeWallWalk(*this,dwID));
}

void AINodeWallWalk::ReadProp(ObjectCreateStruct *pData)
{
	if (!g_pLTServer) return;

	GenericProp genProp;

	AINode::ReadProp(pData);

	if( g_pLTServer->GetPropGeneric( "UseBaseName", &genProp ) == DE_OK )
	{
		m_bUseBaseName = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "DropPoint", &genProp ) == DE_OK )
	{
		m_bDropPoint = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "AdvancePoint", &genProp ) == DE_OK )
	{
		m_bAdvancePoint = genProp.m_Bool;
	}

}

void AINodeWallWalk::FirstUpdate()
{
	// Force into wall walking.
	SetWallWalk(LTTRUE);

	AINode::FirstUpdate();

	// Connect to our next name if we are using a base name.
	if( m_bUseBaseName )
	{
		SequintialBaseName base_name(g_pLTServer->GetObjectName(m_hObject));

		base_name.m_nValue++;

		HOBJECT hNeighbor = base_name.FindObject();

		if( hNeighbor )
		{
			// Be sure this isn't a duplicate.
			AddNeighborLink(hNeighbor,true);

			if( AINode * pNode = dynamic_cast<AINode*>( g_pLTServer->HandleToObject(hNeighbor) ) )
			{
				pNode->AddNeighborLink(m_hObject,true);
			}
			else if( AIVolume * pVolume = dynamic_cast<AIVolume*>( g_pLTServer->HandleToObject(hNeighbor) ) )
			{
				pVolume->AddNeighborLink(m_hObject);
			}
		}

	}
}	


////////////////////////////////////////////////////////////////////////////
//
//      CAINodeWallWalk
//
////////////////////////////////////////////////////////////////////////////

const char * const CAINodeWallWalk::szTypeName = "CAINodeWallWalk";
const LTFLOAT CAINodeWallWalk::k_fWallWalkPreference = 0.8f;

CAINodeWallWalk::CAINodeWallWalk(AINodeWallWalk & node, DDWORD dwID)
	: CAINode(node,dwID),
	  m_bDropPoint( node.m_bDropPoint ),
	  m_bAdvancePoint( node.m_bAdvancePoint ),
	  m_dwDropToVolumeID( kInvalidID )
{
}

CAINodeWallWalk::CAINodeWallWalk(HMESSAGEREAD hRead)
	: CAINode(hRead)
{
	InternalLoad(hRead);
}

void CAINodeWallWalk::FindDropVolume()
{
	if( m_bDropPoint )
	{
		CAIVolume * pInVolume = g_pAIVolumeMgr->FindFallToVolume( GetPos() + GetNormal()*15.0f );

		if( pInVolume )
		{
			m_dwDropToVolumeID = pInVolume->GetNodeID();
			AddNeighbor(m_dwDropToVolumeID);
		}
		else
		{
			m_bDropPoint = LTFALSE;
		}
	}
}

LTVector CAINodeWallWalk::AddToPath(CAIPath * pPath, CAI * pAI, const LTVector & vStart, const LTVector & vDest) const
{
	// Add a drop instruction if our next path point is to our drop destination.
	if( m_bDropPoint )
	{
		const CAIPathWaypoint & next_waypoint = pPath->GetCurrentWaypoint();

		CAINode * pNextNode = g_pAINodeMgr->GetNode( next_waypoint.GetNodeID() );
		if(	   m_dwDropToVolumeID != kInvalidID 
			&& next_waypoint.GetNodeID() == m_dwDropToVolumeID )
		{
			CAIPathWaypoint waypt(GetID(),CAIPathWaypoint::eInstructionDrop);

			pPath->AddWaypoint(waypt);
		}
	}

	return CAINode::AddToPath(pPath,pAI,vStart,vDest);
}

void CAINodeWallWalk::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}

void CAINodeWallWalk::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_bDropPoint;
	*hRead >> m_bAdvancePoint;
	*hRead >> m_dwDropToVolumeID;
}


void CAINodeWallWalk::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);

	*hWrite << m_bDropPoint;
	*hWrite << m_bAdvancePoint;
	*hWrite << m_dwDropToVolumeID;
}
