// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeLadder.cpp
//
// PURPOSE : AINodeLadder implementation
//
// CREATED : 3/30/01
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AINodeLadder.h"
#include "AINodeMgr.h"
#include "DebugLineSystem.h"
#include "AIVolume.h"
#include "AIPath.h"
#include "AIVolumeMgr.h"

#include <algorithm>

#ifdef _DEBUG
//#define LADDER_DEBUG
#endif

////////////////////////////////////////////////////////////////////////////
//
//      AINodeLadder
//
////////////////////////////////////////////////////////////////////////////


BEGIN_CLASS(AINodeLadder)

	PROP_DEFINEGROUP(NeighborLinks, PF_HIDDEN)

	ADD_NEIGHBOR_LINK(1, 0)
	ADD_NEIGHBOR_LINK(2, 0)
	ADD_NEIGHBOR_LINK(3, 0)
	ADD_NEIGHBOR_LINK(4, 0)
	ADD_NEIGHBOR_LINK(5, 0)

	ADD_OBJECTPROP_FLAG_HELP(JumpTo, "", PF_HIDDEN, "AI will jump to this point.")
	ADD_BOOLPROP_FLAG_HELP(TwoWayJump, LTFALSE, PF_HIDDEN, "If true, the AI will be able to jump from the JumpTo point as well." )

	ADD_BOOLPROP_FLAG_HELP( WallWalk, LTFALSE, PF_HIDDEN, "If true, this node will only be used by wall walkers.")
	ADD_BOOLPROP_FLAG_HELP( VolumeConnect, LTFALSE, PF_HIDDEN, "If true, this node will connect to a volume containing it.")

	ADD_OBJECTPROP_FLAG_HELP(TriggerObject, "", PF_HIDDEN, "This object will receive the TriggerMsg when the AI reaches the node if the AI was told to move to this node specifically. No message will be sent if the AI is simply pathfinding through this node on their way to another destination." )
	ADD_STRINGPROP_FLAG_HELP(TriggerMsg, "", PF_HIDDEN, "This message will be sent to the TriggerObject when the AI reaches the node if the AI was told to move to this node specifically. No message will be sent if the AI is simply pathfinding through this node on their way to another destination." )
	ADD_STRINGPROP_FLAG_HELP(SelfTriggerMsg, "", PF_HIDDEN,  "The AI will receive this message when reaching the node if they were told to move to this node specifically. No message will be sent if the AI is simply pathfinding through this node on their way to another destination." )

	ADD_OBJECTPROP_FLAG_HELP(AINodeGroup, "", 0, "The group this AINode is assigned to.  This is not required.")

	ADD_BOOLPROP_FLAG( MoveToFloor, LTFALSE, PF_HIDDEN )

	ADD_BOOLPROP_FLAG_HELP( UseBaseName, LTFALSE, 0, "If true, this node will connect to the next node with the same base name.")

END_CLASS_DEFAULT_FLAGS(AINodeLadder, AINode, NULL, NULL, 0)


CAINode * AINodeLadder::MakeCAINode(uint32 dwID)
{
	return CheckCAINode(new CAINodeLadder(*this,dwID));
}

void AINodeLadder::ReadProp(ObjectCreateStruct *pData)
{
	if (!g_pLTServer) return;

	GenericProp genProp;

	AINode::ReadProp(pData);

	if( g_pLTServer->GetPropGeneric( "UseBaseName", &genProp ) == DE_OK )
	{
		m_bUseBaseName = genProp.m_Bool;
	}

}

void AINodeLadder::FirstUpdate()
{
	AINode::FirstUpdate();

	// Connect to our next name if we are using a base name.
	if( m_bUseBaseName )
	{
		char szName[256];
		char szBaseName[256];
		SAFE_STRCPY(szName,g_pLTServer->GetObjectName(m_hObject));
		int number = 0;

		sscanf(szName,"%[^0123456789]%d",szBaseName,&number);
		number++;

		sprintf(szName,"%s%d",szBaseName,number);

		ObjArray<HOBJECT,1> objArray;
		if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( szName ),
													objArray ) 
			&& objArray.NumObjects() > 0 )
		{
			HOBJECT hNeighbor = objArray.GetObject(0);
			ASSERT(hNeighbor);

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
//      CAINodeLadder
//
////////////////////////////////////////////////////////////////////////////

const char * const CAINodeLadder::szTypeName = "CAINodeLadder";

CAINodeLadder::CAINodeLadder(AINodeLadder & node, DDWORD dwID)
	: CAINode(node,dwID)
{
}

CAINodeLadder::CAINodeLadder(HMESSAGEREAD hRead)
	: CAINode(hRead)
{
	InternalLoad(hRead);
}


LTVector CAINodeLadder::AddToPath(CAIPath * pPath, CAI * pAI, const LTVector & vStart, const LTVector & vDest) const
{
	// Only do a ladder movement if the previous node was a 
	// ladder node (hence, the ladder nodes must come in pairs!
	if( !GetPrevNode() || GetPrevNode()->IsLadder() )
	{
		CAIPathWaypoint ladder_waypt(GetID(), CAIPathWaypoint::eInstructionLadderTo);
		ladder_waypt.SetArgumentPosition(GetPos());

		pPath->AddWaypoint(ladder_waypt);

		return ladder_waypt.GetArgumentPosition();
	}

	return CAINode::AddToPath(pPath, pAI, vStart, vDest);
}

void CAINodeLadder::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}

void CAINodeLadder::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

}


void CAINodeLadder::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);
}
