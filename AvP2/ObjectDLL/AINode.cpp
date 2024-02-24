// ----------------------------------------------------------------------- //
//
// MODULE  : AINode.cpp
//
// PURPOSE : AINode implementation
//
// CREATED : 12/15/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINode.h"
#include "ServerUtilities.h"
#include "AIUtils.h"
#include "DebugLineSystem.h"
#include "AINodeMgr.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AIPath.h"

#include "AINodeAlarm.h"
#include "AINodeCover.h"
#include "AINodeLadder.h"
#include "AINodePatrol.h"
#include "AINodeSnipe.h"
#include "AINodeVolume.h"
#include "AINodeWallWalk.h"


#include <algorithm>

#ifdef _DEBUG
//#define WALLWALK_DEBUG
//#define WWNODE_PROFILE
#endif

BEGIN_CLASS(AINode)

	ADD_VECTORPROP_VAL_FLAG_HELP(Dims, 16.0f, 16.0f, 16.0f, PF_DIMS, "Wall walk nodes must have a brush face within their dimensions.")
	
	ADD_BOOLPROP_FLAG_HELP(MoveToFloor, LTTRUE, 0, "Move node down to floor when game loads." )
	ADD_BOOLPROP_FLAG_HELP(StartActive, LTTRUE, 0, "If false, is equivalent to sending AINodeMgr an off message for this node." )
	
	ADD_OBJECTPROP_FLAG_HELP(TriggerObject, "", 0, "This object will receive the TriggerMsg when the AI reaches the node if the AI was told to move to this node specifically. No message will be sent if the AI is simply pathfinding through this node on their way to another destination." )
	ADD_STRINGPROP_FLAG_HELP(TriggerMsg, "", 0, "This message will be sent to the TriggerObject when the AI reaches the node if the AI was told to move to this node specifically. No message will be sent if the AI is simply pathfinding through this node on their way to another destination." )
	ADD_STRINGPROP_FLAG_HELP(SelfTriggerMsg, "", 0,  "The AI will receive this message when reaching the node if they were told to move to this node specifically. No message will be sent if the AI is simply pathfinding through this node on their way to another destination." )

	ADD_OBJECTPROP_FLAG_HELP(AINodeGroup, "", 0, "The group this AINode is assigned to.  This is not required.")
	
	ADD_BOOLPROP_FLAG_HELP( WallWalk, LTFALSE, 0, "If true, this node will only be used by wall walkers.")
	ADD_BOOLPROP_FLAG_HELP( VolumeConnect, LTTRUE, 0, "If true, this node will connect to a volume containing it.")

	PROP_DEFINEGROUP( AllowRace, PF_GROUP4 )
		ADD_BOOLPROP_FLAG_HELP( AllowAliens,     LTTRUE, PF_GROUP4, "This determines whether Aliens can use this node.")
		ADD_BOOLPROP_FLAG_HELP( AllowMarines,    LTTRUE, PF_GROUP4, "This determines whether Marines can use this node.")
		ADD_BOOLPROP_FLAG_HELP( AllowPredators,  LTTRUE, PF_GROUP4, "This determines whether Predators can use this node.")
		ADD_BOOLPROP_FLAG_HELP( AllowCorporates, LTTRUE, PF_GROUP4, "This determines whether Corporates can use this node.")

	PROP_DEFINEGROUP(NeighborLinks, PF_GROUP5)
		ADD_NEIGHBOR_LINK(1,PF_GROUP5)
		ADD_NEIGHBOR_LINK(2,PF_GROUP5)
		ADD_NEIGHBOR_LINK(3,PF_GROUP5)
		ADD_NEIGHBOR_LINK(4,PF_GROUP5)
		ADD_NEIGHBOR_LINK(5,PF_GROUP5)
		ADD_OBJECTPROP_FLAG_HELP(JumpTo, "", PF_GROUP5, "AI can jump to this point." )
		ADD_BOOLPROP_FLAG_HELP(TwoWayJump, LTFALSE, PF_GROUP5, "If true, the AI will be able to jump from the JumpTo point as well." )

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(AINode, GameBase, NULL, NULL, 0)


class AINodeJump : public AINode
{
};

BEGIN_CLASS(AINodeJump)

	ADD_OBJECTPROP_FLAG_HELP(JumpTo, "", 0, "AI can jump to this point." )
	ADD_BOOLPROP_FLAG_HELP(TwoWayJump, LTTRUE, 0, "If true, the AI will be able to jump from the JumpTo point as well." )

END_CLASS_DEFAULT_FLAGS(AINodeJump, AINode, NULL, NULL, 0)


static uint32 g_nNumPolyTicks = 0;
static uint32 g_nNumPolyCalls = 0;

class MatchesHandle
{
	const LTSmartLink & hMatchee;

public :

	explicit MatchesHandle(const LTSmartLink & matchee)
		: hMatchee(matchee) {}

	bool operator()(const AINode::NeighborLink & matcher) const
	{
		return matcher.hObject == hMatchee;
	}
};


LTBOOL IgnoreAIVolumes(HOBJECT hObj, void *pUserData)
{
	ASSERT(g_pLTServer);

	return !dynamic_cast<AIVolume*>(g_pLTServer->HandleToObject(hObj));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::ReadProp
//
//	PURPOSE:	Reads properties
//
// ----------------------------------------------------------------------- //

void AINode::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData) return;

	LTVector vAngles;
	if ( g_pLTServer->GetPropRotationEuler( "Rotation", &vAngles ) == DE_OK )
	{
		m_vInitialPitchYawRoll = vAngles;
	}

	if ( g_pLTServer->GetPropGeneric( "MoveToFloor", &genProp ) == DE_OK )
	{
		m_bMoveToFloor = genProp.m_Bool;
	}

	if ( g_pLTServer->GetPropGeneric( "StartActive", &genProp ) == DE_OK )
	{
		m_bStartActive = genProp.m_Bool;
	}

	if ( g_pLTServer->GetPropGeneric( "Dims", &genProp ) == DE_OK )
	{
		m_vDims = genProp.m_Vec;
	}

	if( g_pLTServer->GetPropGeneric( "VolumeConnect", &genProp ) == DE_OK )
	{
		m_bConnectToVolume = genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "WallWalk", &genProp ) == DE_OK )
	{
		m_bWallWalkOnly = genProp.m_Bool;
	}

	if ( g_pLTServer->GetPropGeneric( "TriggerObject", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strTriggerObject = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "TriggerMsg", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strTriggerMsg = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "SelfTriggerMsg", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strSelfTriggerMsg = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "AINodeGroup", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strAINodeGroup = genProp.m_String;

	// Read the neighbor links.
	char prop_name[15];
	for(int i = 0; i < 10; ++i )
	{
		sprintf(prop_name,"Neighbor%d",i+1);
		if( g_pServerDE->GetPropGeneric( prop_name, &genProp ) == LT_OK )	
		{
			if ( genProp.m_String[0] )
				m_NeighborNameList.push_back(NeighborName(genProp.m_String));
		}
	}

	// Read the jump link
	if ( g_pServerDE->GetPropGeneric( "JumpTo", &genProp ) == DE_OK )
	{
		if ( genProp.m_String[0] )
		{
			m_JumpToNameList.push_back( NeighborName(genProp.m_String, false) );

			if ( g_pServerDE->GetPropGeneric( "TwoWayJump", &genProp ) == DE_OK )
			{
				m_JumpToNameList.back().bTwoWay = (genProp.m_Bool == LTTRUE);
			}
		}
	}

	if ( g_pServerDE->GetPropGeneric( "UseBezier", &genProp ) == DE_OK )
	{
		m_bUseBezier = genProp.m_Bool;
	}

	if ( g_pServerDE->GetPropGeneric( "BezierPrev", &genProp ) == DE_OK )
	{
		m_vBezierPrev = genProp.m_Vec;
	}

	if ( g_pServerDE->GetPropGeneric( "BezierNext", &genProp ) == DE_OK )
	{
		m_vBezierNext = genProp.m_Vec;
	}

	if ( g_pServerDE->GetPropGeneric( "AllowAliens", &genProp ) == DE_OK )
	{
		m_bAllowAliens = genProp.m_Bool;
	}

	
	if ( g_pServerDE->GetPropGeneric( "AllowMarines", &genProp ) == DE_OK )
	{
		m_bAllowMarines = genProp.m_Bool;
	}
	
	if ( g_pServerDE->GetPropGeneric( "AllowCorporates", &genProp ) == DE_OK )
	{
		m_bAllowCorporates = genProp.m_Bool;
	}
	
	if ( g_pServerDE->GetPropGeneric( "AllowPredators", &genProp ) == DE_OK )
	{
		m_bAllowPredators = genProp.m_Bool;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::EngineMessageFn
//
//	PURPOSE:	Handles engine message functions
//
// ----------------------------------------------------------------------- //

uint32 AINode::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			
			if( dwRet )
			{
				int nInfo = (int)fData;
				if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
				{
					ReadProp((ObjectCreateStruct*)pData);
				}
			}

			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::PostStartWorld()
//
//	PURPOSE:	Called after all the objects and geometry has been loaded.
//
// ----------------------------------------------------------------------- //
void AINode::PostStartWorld(uint8 nLoadGameFlags)
{
	GameBase::PostStartWorld(nLoadGameFlags);

	if( nLoadGameFlags != LOAD_RESTORE_GAME )
	{
		FirstUpdate();

		ASSERT( g_pAINodeMgr );
		if( g_pAINodeMgr )
			g_pAINodeMgr->AddNodeObject(this);
	}
}


void AINode::FirstUpdate()
{
	// Set our dims
	g_pLTServer->SetObjectDims(m_hObject,&m_vDims);

	// Find our neighbors.
	{for( NeighborNameList::iterator name_iter = m_NeighborNameList.begin();
	     name_iter != m_NeighborNameList.end(); ++name_iter )
	{
		ObjArray<HOBJECT,1> objArray;
		if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( name_iter->strNeighbor.c_str() ),
													objArray ) 
			&& objArray.NumObjects() > 0 )
		{
			HOBJECT hNeighbor = objArray.GetObject(0);
			_ASSERT(hNeighbor);

			// Don't allow circular loops.
			if( hNeighbor != m_hObject )
			{
				// Be sure this isn't a duplicate.
				if( m_NeighborConnections.end() == std::find_if( m_NeighborConnections.begin(), m_NeighborConnections.end(), MatchesHandle(hNeighbor) ) )
				{
					m_NeighborConnections.push_back( NeighborLink(hNeighbor,true,name_iter->bTwoWay) );

					if( AINode * pNode = dynamic_cast<AINode*>( g_pLTServer->HandleToObject(hNeighbor) ) )
					{
						if( name_iter->bTwoWay )
							pNode->AddNeighborLink(m_hObject,true);
					}
					else if( AIVolume * pVolume = dynamic_cast<AIVolume*>( g_pLTServer->HandleToObject(hNeighbor) ) )
					{
						pVolume->AddNeighborLink(m_hObject);
					}
				}
			}
			else
			{
#ifndef _FINAL
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject,&vPos);
				g_pLTServer->CPrint("Node %s at (%.2f,%.2f,%.2f) points to itself!",
					g_pLTServer->GetObjectName(m_hObject),
					vPos.x, vPos.y, vPos.z );
#endif
			}
		}
		else
		{
#ifndef _FINAL
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject,&vPos);
				g_pLTServer->CPrint("Node %s at (%.2f,%.2f,%.2f) refers to an unknown object named \"%s\"!",
					g_pLTServer->GetObjectName(m_hObject),
					vPos.x, vPos.y, vPos.z,
					name_iter->strNeighbor.c_str() );
#endif
		}
	}}

	// Find our jump to neighbor.
	{for( NeighborNameList::iterator jump_iter = m_JumpToNameList.begin();
	     jump_iter != m_JumpToNameList.end(); ++jump_iter )
	{
		ObjArray<HOBJECT,1> objArray;
		if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( jump_iter->strNeighbor.c_str() ),
													objArray ) 
			&& objArray.NumObjects() > 0 )
		{
			HOBJECT hNeighbor = objArray.GetObject(0);
			_ASSERT(hNeighbor);

			// Don't allow circular loops.
			if( hNeighbor != m_hObject )
			{
				// Be sure this isn't a duplicate.
				if( m_JumpToConnections.end() == std::find_if( m_JumpToConnections.begin(), m_JumpToConnections.end(), MatchesHandle(hNeighbor) ) )
				{
					if( AINode * pNode = dynamic_cast<AINode*>( g_pLTServer->HandleToObject(hNeighbor) ) )
					{
						m_JumpToConnections.push_back( NeighborLink(hNeighbor,true,jump_iter->bTwoWay) );
						m_NeighborConnections.push_back( NeighborLink(hNeighbor,true,jump_iter->bTwoWay) );

						if( jump_iter->bTwoWay )
						{
							pNode->AddJumpLink(m_hObject,true);
							pNode->AddNeighborLink(m_hObject,true);
						}
					}
					else
					{
#ifndef _FINAL
						LTVector vPos;
						g_pLTServer->GetObjectPos(m_hObject,&vPos);
						g_pLTServer->CPrint("Node %s at (%.2f,%.2f,%.2f) jumps to a non-node!  AI's can only jump to other nodes!",
							g_pLTServer->GetObjectName(m_hObject),
							vPos.x, vPos.y, vPos.z );
#endif
					}
				}
			}
			else
			{
#ifndef _FINAL
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject,&vPos);
				g_pLTServer->CPrint("Node %s at (%.2f,%.2f,%.2f) jumps to itself!  Jump is being ignored.",
					g_pLTServer->GetObjectName(m_hObject),
					vPos.x, vPos.y, vPos.z );
#endif
			}
		}
		else
		{
#ifndef _FINAL
				LTVector vPos;
				g_pLTServer->GetObjectPos(m_hObject,&vPos);
				g_pLTServer->CPrint("Node %s at (%.2f,%.2f,%.2f) jumps to an unknown object named \"%s\"!",
					g_pLTServer->GetObjectName(m_hObject),
					vPos.x, vPos.y, vPos.z,
					jump_iter->strNeighbor.c_str() );
#endif
		}
	}}	

	// Find our bezier neighbor.
	if( m_bUseBezier )
	{
		SequintialBaseName base_name( g_pLTServer->GetObjectName(m_hObject) );

		// Try to find a next node.
		base_name.m_nValue++;

		HOBJECT hNextBezierNode = base_name.FindObject();

		if( hNextBezierNode )
		{
			if( AINode * pNode = dynamic_cast<AINode*>( g_pLTServer->HandleToObject(hNextBezierNode) ) )
			{
				// Link us to that node.
				m_BezierToConnections.push_back( NeighborLink(hNextBezierNode,true,true) );
				m_NeighborConnections.push_back( NeighborLink(hNextBezierNode,true,true) );

				// And link that node to us.
				pNode->AddBezierLink(m_hObject,true);
				pNode->AddNeighborLink(m_hObject,true);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINode::CheckCAINode
//
//	PURPOSE:	Performs a consistency check on the CAINode.  Returns null if 
//				node is not good.
//
// ----------------------------------------------------------------------- //
CAINode * AINode::CheckCAINode(CAINode * pNode)
{
	if( pNode && pNode->IsWallWalkOnly() )
	{
		if( pNode->GetNormal().MagSqr() < 0.5f )
		{
			delete pNode;
			return LTNULL;
		}
	}

	return pNode;
}

void AINode::AddNeighborLink(const NeighborLink & new_neighbor)
{
	if( new_neighbor.hObject )
	{
		NeighborConnectList::iterator neighbor_iter = std::find_if( m_NeighborConnections.begin(), m_NeighborConnections.end(), MatchesHandle(new_neighbor.hObject) );

		if( neighbor_iter == m_NeighborConnections.end() )
		{
			// Well, looks like we need to add a new link!
			m_NeighborConnections.push_back(new_neighbor);
		}
		else
		{
			// Allow more connectivity if needed (but don't decrease connectivity).
			if( new_neighbor.bConnectTo )
				neighbor_iter->bConnectTo = new_neighbor.bConnectTo;

			if( new_neighbor.bConnectFrom )
				neighbor_iter->bConnectFrom = new_neighbor.bConnectFrom;
		}
	}
}

void AINode::AddJumpLink(const NeighborLink & new_neighbor)
{
	if( new_neighbor.hObject )
	{
		NeighborConnectList::iterator jump_iter = std::find_if( m_JumpToConnections.begin(), m_JumpToConnections.end(), MatchesHandle(new_neighbor.hObject) );

		if( jump_iter == m_JumpToConnections.end() )
		{
			// Well, looks like we need to add a new link!
			m_JumpToConnections.push_back(new_neighbor);
		}
		else
		{
			// Allow more connectivity if needed (but don't decrease connectivity).
			if( new_neighbor.bConnectTo )
				jump_iter->bConnectTo = new_neighbor.bConnectTo;

			if( new_neighbor.bConnectFrom )
				jump_iter->bConnectFrom = new_neighbor.bConnectFrom;
		}
	}
}

void AINode::AddBezierLink(const NeighborLink & new_neighbor)
{
	if( new_neighbor.hObject )
	{
		NeighborConnectList::iterator bezier_iter = std::find_if( m_BezierToConnections.begin(), m_BezierToConnections.end(), MatchesHandle(new_neighbor.hObject) );

		if( bezier_iter == m_BezierToConnections.end() )
		{
			// Well, looks like we need to add a new link!
			m_BezierToConnections.push_back(new_neighbor);
		}
		else
		{
			// Allow more connectivity if needed (but don't decrease connectivity).
			if( new_neighbor.bConnectTo )
				bezier_iter->bConnectTo = new_neighbor.bConnectTo;

			if( new_neighbor.bConnectFrom )
				bezier_iter->bConnectFrom = new_neighbor.bConnectFrom;
		}
	}
}

LTBOOL AINode::JumpTo(HOBJECT hNode) const
{
	return m_JumpToConnections.end() != std::find_if( m_JumpToConnections.begin(), m_JumpToConnections.end(), MatchesHandle(hNode) );
}

LTBOOL AINode::BezierTo(HOBJECT hNode) const
{
	return m_BezierToConnections.end() != std::find_if( m_BezierToConnections.begin(), m_BezierToConnections.end(), MatchesHandle(hNode) );
}

CAINode * AINode::MakeCAINode(uint32 dwID)
{
	return CheckCAINode(new CAINode(*this,dwID));
}

////////////////////////////////////////////////////////////////////////////
//
//      CAINode
//
////////////////////////////////////////////////////////////////////////////

const char * const CAINode::szTypeName = "CAINode";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::SaveCAINode
//
//	PURPOSE:	Saves information so that a CAINode 
//				can be loaded with LoadCAINode.
//
// ----------------------------------------------------------------------- //
void CAINode::SaveCAINode(ILTMessage & out, /*const*/ CAINode & ai_node)
{
	// The node type name will be read into a 256 character buffer
	// in LoadCAINode().
	ASSERT( strlen( ai_node.GetTypeName() ) < 256 );

	// Save the node type name.
	out.WriteString( const_cast<char*>(ai_node.GetTypeName()) );

	// Save the node.
	ai_node.Save(&out);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::LoadCAINode
//
//	PURPOSE:	Factory function to new allocate an 
//              appropriate CAINode a previously saved CAINode.
//
// ----------------------------------------------------------------------- //
CAINode * CAINode::LoadCAINode(ILTMessage & in)
{
	// Load the node type name.
	char type_name[256];
	in.ReadStringFL( type_name, 256 );

	CAINode * pNewNode = 0;

	if( strcmp(CAINode::szTypeName,type_name) == 0 )
	{
		pNewNode = new CAINode(&in);
	}
	else if( strcmp(CAINodeAlarm::szTypeName, type_name) == 0 )
	{
		pNewNode = new CAINodeAlarm(&in);
	}
	else if( strcmp(CAINodeCover::szTypeName, type_name) == 0 )
	{
		pNewNode = new CAINodeCover(&in);
	}
	else if( strcmp(CAINodeLadder::szTypeName, type_name) == 0 )
	{
		pNewNode = new CAINodeLadder(&in);
	}
	else if( strcmp(CAINodePatrol::szTypeName,type_name) == 0 )
	{
		pNewNode = new CAINodePatrol(&in);
	}
	else if( strcmp(CAINodeSnipe::szTypeName, type_name) == 0 )
	{
		pNewNode = new CAINodeSnipe(&in);
	}
	else if( strcmp(CAIVolumeNode::szTypeName, type_name) == 0 )
	{
		pNewNode = new CAIVolumeNode(&in);
	}
	else if( strcmp(CAIVolumeNeighborNode::szTypeName, type_name) == 0 )
	{
		pNewNode = new CAIVolumeNeighborNode(&in);
	}
	else if( strcmp(CAINodeWallWalk::szTypeName, type_name) == 0 )
	{
		pNewNode = new CAINodeWallWalk(&in);
	}
	else
	{
#ifdef _DEBUG
		g_pLTServer->CPrint("Node type \"%s\" unknown! Did you forget to add it to CAINode::LoadCAINode?",
			type_name);
#endif
	}

	return pNewNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::CAINode()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAINode::CAINode( const char * name, const LTVector & pos, 
				  const LTVector & right, const LTVector & up, const LTVector & forward, 
				  LTBOOL wall_walk_only, uint32 dwID )
	: m_dwID(dwID),
	  m_strName(name),

	  m_vPos(pos),
	  m_vUp(up),
	  m_vRight(right),
	  m_vForward(forward),

	  m_vDims(0,0,0),

	  m_bConnectToVolume(LTTRUE),

	  m_pContainingVolumePtr(LTNULL),

	  m_bWallWalkOnly( wall_walk_only ),
	  m_vNormal(0,0,0),

	  //m_BezierToID
	  m_vBezierPrev(0,0,0),
	  m_vBezierNext(0,0,0),

	  m_bAllowAliens(LTTRUE),
	  m_bAllowMarines(LTTRUE),
	  m_bAllowPredators(LTTRUE),
	  m_bAllowCorporates(LTTRUE),

	  m_hLockOwner( LTNULL ),

	  m_bIsActive( LTTRUE ),

	  // m_hTriggerObject
	  // m_strTriggerMsg
	  // m_strSelfTriggerMsg

	  m_hAINodeGroup(LTNULL),

	  m_nLastPathingIteration(0),
	  m_bResetNeighbors(false),

	  m_bIsGoal(false),

	  m_fCost(0.0f),
	  m_fHeuristic(0.0f),

	  m_bInOpen(false),
	  m_bInClosed(false),
	  m_pPrevNode(LTNULL)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::CAINode()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAINode::CAINode( const char * name, LTBOOL wall_walk_only, uint32 dwID )
	: m_dwID(dwID),
	  m_strName(name),

	  m_vPos(0,0,0),
	  m_vUp(0,0,0),
	  m_vRight(0,0,0),
	  m_vForward(0,0,0),

	  m_vDims(0,0,0),

	  m_bConnectToVolume(LTTRUE),

	  m_pContainingVolumePtr(LTNULL),

	  m_bWallWalkOnly( wall_walk_only ),
	  m_vNormal(0,0,0),

	  //m_BezierToID
	  m_vBezierPrev(0,0,0),
	  m_vBezierNext(0,0,0),

	  m_bAllowAliens(LTTRUE),
	  m_bAllowMarines(LTTRUE),
	  m_bAllowPredators(LTTRUE),
	  m_bAllowCorporates(LTTRUE),

	  m_hLockOwner( LTNULL ),

	  m_bIsActive( LTTRUE ),

	  m_nLastPathingIteration(0),
	  m_bResetNeighbors(false),

	  // m_hTriggerObject
	  // m_strTriggerMsg
	  // m_strSelfTriggerMsg

	  m_bIsGoal(false),

	  m_fCost(0.0f),
	  m_fHeuristic(0.0f),

	  m_bInOpen(false),
	  m_bInClosed(false),
	  m_pPrevNode(LTNULL)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::CAINode()
//
//	PURPOSE:	Creates a node based on the original node but with a different position.
//
// ----------------------------------------------------------------------- //

CAINode::CAINode( const CAINode & orig_node, const LTVector & vPos)
	: m_dwID(CAINode::kInvalidID ),
	  m_strName(orig_node.m_strName),


	  m_vPos(vPos),
	  m_vUp(orig_node.m_vUp),
	  m_vRight(orig_node.m_vRight),
	  m_vForward(orig_node.m_vForward),

	  m_vDims(orig_node.m_vDims),

	  m_bConnectToVolume(orig_node.m_bConnectToVolume),

	  m_pContainingVolumePtr(orig_node.m_pContainingVolumePtr),

	  m_bWallWalkOnly(orig_node.m_bWallWalkOnly),
	  m_vNormal(orig_node.m_vNormal),

	  m_JumpToID(orig_node.m_JumpToID),

	  m_BezierToID( orig_node.m_BezierToID ),
	  m_vBezierPrev( orig_node.m_vBezierPrev ),
	  m_vBezierNext( orig_node.m_vBezierNext ),

	  m_bAllowAliens( orig_node.m_bAllowAliens ),
	  m_bAllowMarines( orig_node.m_bAllowMarines ),
	  m_bAllowPredators( orig_node.m_bAllowPredators ),
	  m_bAllowCorporates( orig_node.m_bAllowCorporates ),

	  m_hLockOwner(orig_node.m_hLockOwner),

	  m_bIsActive(orig_node.m_bIsActive),

	  m_nLastPathingIteration(orig_node.m_nLastPathingIteration),
	  m_bResetNeighbors(orig_node.m_bResetNeighbors),

	  m_hTriggerObject(orig_node.m_hTriggerObject),
	  m_strTriggerMsg(orig_node.m_strTriggerMsg),
	  m_strSelfTriggerMsg(orig_node.m_strSelfTriggerMsg),

#ifndef _FINAL
	  m_hDebugNodeBox(LTNULL),
#endif

	  m_NeighborIDList(orig_node.m_NeighborIDList),
	  m_NeighborList(orig_node.m_NeighborList),

	  m_bIsGoal(orig_node.m_bIsGoal),

	  m_fCost(orig_node.m_fCost),
	  m_fHeuristic(orig_node.m_fHeuristic),

	  m_bInOpen(orig_node.m_bInOpen),
	  m_bInClosed(orig_node.m_bInClosed),
	  m_pPrevNode(orig_node.m_pPrevNode)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::CAINode()
//
//	PURPOSE:	Initialize object from AINode
//
// ----------------------------------------------------------------------- //


CAINode::CAINode(AINode & node, uint32 dwID)
	: m_dwID(dwID),
	  m_strName( g_pLTServer->GetObjectName(node.m_hObject) ),

	  m_vPos(0,0,0),
	  m_vUp(0,0,0),
	  m_vRight(0,0,0),
	  m_vForward(0,0,0),

	  m_vDims(0,0,0),

	  m_bConnectToVolume(node.m_bConnectToVolume),

	  m_pContainingVolumePtr(LTNULL),

	  m_bWallWalkOnly( node.m_bWallWalkOnly ),
	  m_vNormal(0,0,0),

	  //m_BezierToID
	  m_vBezierPrev(0,0,0),
	  m_vBezierNext(0,0,0),

	  m_bAllowAliens( node.m_bAllowAliens ),
	  m_bAllowMarines( node.m_bAllowMarines ),
	  m_bAllowPredators( node.m_bAllowPredators ),
	  m_bAllowCorporates( node.m_bAllowCorporates ),

	  m_hLockOwner( LTNULL ),

	  m_bIsActive( node.m_bStartActive ),

	  m_nLastPathingIteration(0),
	  m_bResetNeighbors(false),

	  // m_hTriggerObject
	  m_strTriggerMsg(node.m_strTriggerMsg),
	  m_strSelfTriggerMsg(node.m_strSelfTriggerMsg),

	  m_bIsGoal(false),

	  m_fCost(0.0f),
	  m_fHeuristic(0.0f),

	  m_bInOpen(false),
	  m_bInClosed(false),
	  m_pPrevNode(LTNULL)
{
	node.m_nID = dwID;

	// Move the node to the floor first.
	// Hopefully, this won't be called until everything in the world
	// has been loaded.  If it gets called before a world model
	// is loaded, the node may end up "going through" the world model
	// becuase the world model really isn't there!
	if( node.m_bMoveToFloor ) 
	{
		MoveObjectToFloor(node.m_hObject);
	}

	g_pLTServer->GetObjectPos(node.m_hObject, &m_vPos);
	g_pLTServer->GetObjectDims(node.m_hObject, &m_vDims);

	// If we needed to move to floor, we need to adjust
	// our position down by our dims.
	if( node.m_bMoveToFloor )
	{
		m_vPos -= m_vDims.y;
	}

	DRotation rRot;
	g_pLTServer->SetupEuler(&rRot, EXPANDVEC(node.m_vInitialPitchYawRoll));
	g_pLTServer->Common()->GetRotationVectors(rRot, m_vRight, m_vUp, m_vForward);

	// Find the trigger object.
	if( !node.m_strTriggerObject.empty() )
	{
		if( 0 == stricmp(node.m_strTriggerObject.c_str(), "[NULL]") )
		{
			// The special case of a [NULL] object is handled by pre-pending the string
			// onto the self-trigger message.

			std::string strNewSelfTriggerMsg = node.m_strTriggerMsg.c_str();

			if( m_strSelfTriggerMsg.c_str() )
				strNewSelfTriggerMsg += m_strSelfTriggerMsg.c_str();

			m_strSelfTriggerMsg =  strNewSelfTriggerMsg.c_str();

			m_strTriggerMsg = "";
		}
		else
		{
			ObjArray<HOBJECT,1> objArray;
			if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( node.m_strTriggerObject.c_str() ), objArray ) )
			{
				if( objArray.NumObjects() > 0 )
				{
					// Record our next point.
					m_hTriggerObject = objArray.GetObject(0);

				}
				else
				{
#ifndef _FINAL
					g_pLTServer->CPrint("Node \"%s\" could found %d objects named \"%s\".",
						m_strName.c_str(),
						objArray.NumObjects(),
						node.m_strTriggerObject.c_str() );
#endif
				}
			}
			else
			{
#ifndef _FINAL
				g_pLTServer->CPrint("Node \"%s\" could not find object named \"%s\".",
					m_strName.c_str(),
					node.m_strTriggerObject.c_str() );
#endif
			}
		}
	}

	// Find the AINodeGroup (if any)
	if( !node.m_strAINodeGroup.empty() )
	{
		ObjArray<HOBJECT,1> objArray;
		if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( node.m_strAINodeGroup.c_str() ), objArray ) )
		{
			if( objArray.NumObjects() > 0 )
			{
				m_hAINodeGroup = objArray.GetObject(0);
			}
			else
			{
#ifndef _FINAL
				g_pLTServer->CPrint("Node \"%s\" could find %d objects named \"%s\".",
					m_strName.c_str(),
					objArray.NumObjects(),
					node.m_strAINodeGroup.c_str() );
#endif
			}
		}
		else
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Node \"%s\" could not find object named \"%s\".",
				m_strName.c_str(),
				node.m_strAINodeGroup.c_str() );
#endif
		}
	}


	if( m_bWallWalkOnly )
	{
		// Find our polies.
		LTVector vDims;
		g_pLTServer->GetObjectDims(node.m_hObject,&vDims);
	
		const uint32 nNumPolies = 32;
		HPOLY hPolies[nNumPolies];
		uint32 nNumPoliesFound = 0;

#ifdef WWNODE_PROFILE
		LTCounter poly_counter;
		g_pLTServer->StartCounter(&poly_counter);
#endif
		g_pLTServer->FindPoliesTouchingBox(m_vPos - vDims, m_vPos + vDims, hPolies, nNumPolies, &nNumPoliesFound, IgnoreAIVolumes );

#ifdef WWNODE_PROFILE
		const uint32 nPolyTicks = g_pLTServer->EndCounter(&poly_counter);

		g_nNumPolyTicks += nPolyTicks;
		++g_nNumPolyCalls;

		g_pLTServer->CPrint("Node %s touches %d polies. Took %d ticks.",
			g_pLTServer->GetObjectName(node.m_hObject), nNumPoliesFound, nPolyTicks 	);

		g_pLTServer->CPrint("Total %d for %d polies.",
			g_nNumPolyTicks, g_nNumPolyCalls );
#endif

		if( nNumPoliesFound > 0 )
		{
#ifdef WALLWALK_DEBUG
			const LTVector old_pos = m_vPos;
#endif

			// Move the node to the nearest polygon and use that poly's normal as our normal.

			// Find the nearest poly and record its normal and dist.
			LTFLOAT fNearestDist = FLT_MAX;
			LTPlane * pPlane = LTNULL;
			for( HPOLY * iter = hPolies; iter < hPolies + nNumPoliesFound; ++iter )
			{
				if( LT_OK == g_pLTServer->Common()->GetPolyInfo(*iter,&pPlane) && pPlane )
				{
					const LTFLOAT fCurrentDist = pPlane->DistTo(m_vPos);
					if( fabs(fCurrentDist) < fabs(fNearestDist) )
					{
						fNearestDist = fCurrentDist;
						m_vNormal = pPlane->m_Normal;
					}
				}
			}

			_ASSERT( m_vNormal.MagSqr() > 0 );

			// Move our position so that we sit on that polygon.
			m_vPos -= m_vNormal*fNearestDist;

#ifdef WALLWALK_DEBUG
			LineSystem::GetSystem("MovedNodes")
				<< LineSystem::Arrow(old_pos, m_vPos, Color::Green );
#endif
		}
		else // !( nNumPoliesFound > 0 )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Wall walk node \"%s\" at (%.2f,%.2f,%.2f) is not touching a polygon!",
				g_pLTServer->GetObjectName(node.m_hObject),
				m_vPos.x, m_vPos.y, m_vPos.z );
#endif
			return; // Hopefully the zero normal will be caught and this node will be deleted!
		}

	} //if( m_bWallWalkOnly )

	// Link up the neighbors.
	for( AINode::NeighborConnectList::const_iterator iter = node.m_NeighborConnections.begin();
	     iter != node.m_NeighborConnections.end(); ++iter )
	{
		_ASSERT( iter->hObject );
		if( iter->hObject )
		{
			// If the neighbor has a CAINode reference, record its ID and tell it of our new ID.
			if( AINode * pNeighborNodeObject = dynamic_cast<AINode*>( g_pServerDE->HandleToObject(iter->hObject) ) )
			{
				// Make the connection circular.  So if the other node doesn't have an ID yet,
				// it can tell us about its id when it gets processed by this same code.
				AINode::NeighborConnectList::iterator neighbor_iter = std::find_if( pNeighborNodeObject->m_NeighborConnections.begin(), pNeighborNodeObject->m_NeighborConnections.end(), MatchesHandle(node.m_hObject) );
				if( neighbor_iter == pNeighborNodeObject->m_NeighborConnections.end() )
				{
					// Add the link with the connect-to and connect-from reversed.
					pNeighborNodeObject->AddNeighborLink( AINode::NeighborLink(node.m_hObject, iter->bConnectFrom, iter->bConnectTo) );
					neighbor_iter = std::find_if( pNeighborNodeObject->m_NeighborConnections.begin(), pNeighborNodeObject->m_NeighborConnections.end(), MatchesHandle(node.m_hObject) );
					ASSERT( neighbor_iter != pNeighborNodeObject->m_NeighborConnections.end() );
					if( neighbor_iter == pNeighborNodeObject->m_NeighborConnections.end() )
						continue;
				}
				else
				{
					// Be sure the neighbor will connect to us if we wish.
					neighbor_iter->bConnectFrom = iter->bConnectTo;
				}


				// If it does have its id, we need to record its id and tell it to record ours.
				if( pNeighborNodeObject->m_nID != CAINode::kInvalidID )
				{
					// Record its ID.
					if( iter->bConnectTo )
					{
						AddNeighbor(pNeighborNodeObject->m_nID);

						if( node.BezierTo(pNeighborNodeObject->m_hObject) )
						{
							m_BezierToID.push_back(pNeighborNodeObject->m_nID);
						}

						if( node.JumpTo(pNeighborNodeObject->m_hObject) && !JumpTo(pNeighborNodeObject->m_nID) )
						{
							m_JumpToID.push_back(pNeighborNodeObject->m_nID);
						}
					}

					// Have it record our ID.
					if( iter->bConnectFrom )
					{
						CAINode * pNeighborNode = dynamic_cast<CAINode*>( g_pAINodeMgr->GetNode(pNeighborNodeObject->m_nID) );
						_ASSERT( pNeighborNode );
						if( pNeighborNode )
						{
							_ASSERT( pNeighborNode->EndNeighborIDs() == std::find( pNeighborNode->BeginNeighborIDs(), pNeighborNode->EndNeighborIDs(), GetID() ) );
							if( pNeighborNode->EndNeighborIDs() == std::find( pNeighborNode->BeginNeighborIDs(), pNeighborNode->EndNeighborIDs(), GetID() ) )
							{
								pNeighborNode->AddNeighbor( m_dwID );

								if( pNeighborNodeObject->BezierTo(node.m_hObject) )
								{
									pNeighborNode->m_BezierToID.push_back(m_dwID);
								}
								if( pNeighborNodeObject->JumpTo(node.m_hObject) && !pNeighborNode->JumpTo(m_dwID) )
								{
									pNeighborNode->m_JumpToID.push_back(m_dwID);
								}

							}
						}
					}
				}
			}
			else if( AIVolume * pNeighborVolumeObject = dynamic_cast<AIVolume*>( g_pServerDE->HandleToObject(iter->hObject) ) )
			{
				// Have the volume connect to us.  This relies on the volumes being initialized after the nodes.
				// The volume should link to us and then link us to all its neighbor connections.
				if( pNeighborVolumeObject->EndNeighborLinks() 
					  == std::find( pNeighborVolumeObject->BeginNeighborLinks(), pNeighborVolumeObject->EndNeighborLinks(), node.m_hObject) )
				{
					pNeighborVolumeObject->AddNeighborLink( node.m_hObject);
				}

			}

		} //if( iter->GetHOBJECT() )

	} // for( AINode::NeighborConnectList::const_iterator iter = node.m_NeighborConnections.begin();


	// L
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::CAINode()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAINode::CAINode(HMESSAGEREAD hRead)
	: m_dwID(kInvalidID),
	  // m_strName

	  m_vPos(0,0,0),
	  m_vUp(0,0,0),
	  m_vRight(0,0,0),
	  m_vForward(0,0,0),

	  m_vDims(0,0,0),

	  m_bConnectToVolume(LTTRUE),

	  m_pContainingVolumePtr(LTNULL),

	  m_bWallWalkOnly( LTFALSE ),
	  m_vNormal(0,0,0),

	  // m_BezierToID
	  m_vBezierPrev(0,0,0),
	  m_vBezierNext(0,0,0),

	  m_bAllowAliens(LTTRUE),
	  m_bAllowMarines(LTTRUE),
	  m_bAllowPredators(LTTRUE),
	  m_bAllowCorporates(LTTRUE),

	  m_hLockOwner( LTNULL ),

	  m_bIsActive( LTTRUE ),

	  m_nLastPathingIteration(0),
	  m_bResetNeighbors(false),

	  // m_hTriggerObject
	  // m_strTriggerMsg
	  // m_strSelfTriggerMsg

	  m_bIsGoal(false),

	  m_fCost(0.0f),
	  m_fHeuristic(0.0f),

	  m_bInOpen(false),
	  m_bInClosed(false),
	  m_pPrevNode(LTNULL)
{
	InternalLoad(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::~CAINode()
//
//	PURPOSE:	Destroy the object
//
// ----------------------------------------------------------------------- //

CAINode::~CAINode()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::AddNeighbor()
//
//	PURPOSE:	Makes a pathing connection between two nodes
//
// ----------------------------------------------------------------------- //

void  CAINode::AddNeighbor(uint32 dwNeighborID)
{
	if( dwNeighborID != m_dwID && dwNeighborID != kInvalidID)
	{
		if( m_NeighborIDList.end() == std::find(m_NeighborIDList.begin(),m_NeighborIDList.end(), dwNeighborID) )
		{
			m_NeighborIDList.push_back(dwNeighborID);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::DrawDebug()
//
//	PURPOSE:	Draw the box models and show the connections.
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL

void CAINode::DrawDebug(int level) 
{
	CAINode::ClearDebug();

	// Draw the node.

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	sprintf(theStruct.m_Name, "%s-DebugBox", GetName().c_str() );

	theStruct.m_Pos = GetPos() + GetNormal();

#ifdef _WIN32
	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.abc");
#else
	SAFE_STRCPY(theStruct.m_Filename, "Models/1x1_square.abc");
#endif

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	theStruct.m_ObjectType = OT_MODEL;

	HCLASS hClass = g_pLTServer->GetClass("BaseClass");
	LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	LTVector vScale;
	VEC_SET(vScale, 20.0f, 20.0f, 20.0f);
	g_pLTServer->ScaleObject(pModel->m_hObject, &vScale);
	
	LTVector color = GetNodeColor();
	LTFLOAT  alpha = GetNodeAlpha();

	g_pLTServer->SetObjectColor(pModel->m_hObject, color.x, color.y, color.z, alpha);

#ifndef _FINAL
	m_hDebugNodeBox = pModel->m_hObject;
#endif


	// Draw the connections.
	for( NeighborList::iterator iter =  BeginNeighbors();
		 iter !=  EndNeighbors(); ++iter )
	{
		if( (*iter)->IsActive() )
		{
			LineSystem::GetSystem(this,"Connections")
				<< LineSystem::Arrow(GetPos(), (*iter)->GetPos(), color,alpha);
		}
	}

	LineSystem::GetSystem(this,"Normal")
		<< LineSystem::Arrow(GetPos(), GetPos() + GetNormal()*25.0f, color,1.0f);
}

#endif

#ifndef _FINAL

void CAINode::ClearDebug() 
{
	if( m_hDebugNodeBox )
	{
		g_pLTServer->RemoveObject(m_hDebugNodeBox);
	}

	LineSystem::GetSystem(this,"Connections")
		<< LineSystem::Clear();

	LineSystem::GetSystem(this,"Normal")
		<< LineSystem::Clear();
}

#endif

void CAINode::ResetPathing(uint32 nPathingIteration )
{
	// If we have already been reset, nothing more to do!
	if( nPathingIteration == m_nLastPathingIteration )
		return;

	// Okay, we really do need to reset ourself!

	// Do the actual reset.
	InternalResetPathing();
	
	// Don't reset again this iteration.
	m_nLastPathingIteration = nPathingIteration;


	// Be sure our neighbors will get reset.
	m_bResetNeighbors = true;
}



void CAINode::InternalResetPathing()
{
	m_bIsGoal = false;

	m_fCost = 0.0f;
	m_fHeuristic = 0.0f;

	m_bInOpen = false;
	m_bInClosed = false;
	m_pPrevNode = LTNULL;
}

LTVector CAINode::AddToPath(CAIPath * pPath, CAI * /* un-used */, const LTVector & /* un-used */, const LTVector & /* un-used */) const
{
	_ASSERT( pPath );
	if( !pPath ) return LTVector(0,0,0);

	_ASSERT( m_dwID != CAINode::kInvalidID );


	CAIPathWaypoint waypt(m_dwID, CAIPathWaypoint::eInstructionMoveTo);
	waypt.SetArgumentPosition(GetPos());

	// Change the instructions if there are special instructions.
	const CAINode * pPrevNode = GetPrevNode();
	if( pPrevNode && pPrevNode->BezierTo(m_dwID) )
	{
		waypt.SetInstruction(CAIPathWaypoint::eInstructionBezierTo);
		waypt.SetArgumentNormal(GetNormal());
		waypt.SetArgumentBezierDest(m_vBezierPrev);
		waypt.SetArgumentBezierSrc(pPrevNode->m_vBezierNext);
	}
	else if( m_bWallWalkOnly )
	{
		waypt.SetInstruction(CAIPathWaypoint::eInstructionWallWalkTo);
		waypt.SetArgumentNormal(GetNormal());

		if( pPrevNode && pPrevNode->JumpTo(m_dwID) )
		{
			waypt.SetInstruction(CAIPathWaypoint::eInstructionWWJumpTo);
		}
	}
	else if( pPrevNode && pPrevNode->JumpTo(m_dwID) )
	{
		waypt.SetInstruction(CAIPathWaypoint::eInstructionJumpTo);
	}

	pPath->AddWaypoint(waypt);

	return GetPos();
}

CAINode::NeighborIterator CAINode::BeginNeighbors()
{
	// Be sure m_NeighborList is up to date.
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CAINode * pNode = g_pAINodeMgr->GetNode(*iter);
			_ASSERT( pNode );
			if(pNode) m_NeighborList.push_back(pNode);
		}
	}

	// Reset our neighbors.
	if( m_bResetNeighbors )
	{
		m_bResetNeighbors = false;

		for( NeighborIterator iter = m_NeighborList.begin(); iter != m_NeighborList.end(); ++iter )
		{
			(*iter)->ResetPathing(m_nLastPathingIteration);
		}
	}

	return m_NeighborList.begin();
}

CAINode::NeighborIterator CAINode::EndNeighbors()
{
	// Be sure m_NeighborList is up to date.
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CAINode * pNode = g_pAINodeMgr->GetNode(*iter);
			_ASSERT( pNode );
			if(pNode) m_NeighborList.push_back(pNode);
		}
	}

	// Reset our neighbors.
	if( m_bResetNeighbors )
	{
		m_bResetNeighbors = false;

		for( NeighborIterator iter = m_NeighborList.begin(); iter != m_NeighborList.end(); ++iter )
		{
			(*iter)->ResetPathing(m_nLastPathingIteration);
		}
	}

	return m_NeighborList.end();
}



CAINode::const_NeighborIterator CAINode::BeginNeighbors() const
{
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::const_iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CAINode * pNode = g_pAINodeMgr->GetNode(*iter);
			_ASSERT( pNode );
			if(pNode) m_NeighborList.push_back(pNode);
		}
	}

	return m_NeighborList.begin();
}

CAINode::const_NeighborIterator CAINode::EndNeighbors() const
{
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::const_iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CAINode * pNode = g_pAINodeMgr->GetNode(*iter);
			_ASSERT( pNode );
			if(pNode) m_NeighborList.push_back(pNode);
		}
	}

	return m_NeighborList.end();
}



LTBOOL CAINode::IsOccupied(HOBJECT hOwner)
{
	LTVector vOwnerPos, vDims;
	LTFLOAT fDistSqr, fTouchRadius;

	g_pInterface->GetObjectPos(hOwner, &vOwnerPos);
	g_pInterface->GetObjectDims(hOwner, &vDims);

	fTouchRadius = vDims.x * 2.0f;

	fDistSqr = VEC_DISTSQR(vOwnerPos, GetPos());
	if (fDistSqr <= fTouchRadius * fTouchRadius)
		return LTTRUE;
	
	return LTFALSE;
}
	
LTBOOL CAINode::JumpTo(uint32 dwID) const
{
	return ( m_JumpToID.end() != std::find( m_JumpToID.begin(), m_JumpToID.end(), dwID) );
}

LTBOOL CAINode::BezierTo(uint32 dwID) const
{
	return ( m_BezierToID.end() != std::find( m_BezierToID.begin(), m_BezierToID.end(), dwID) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::Save()
//
//	PURPOSE:	Save the node 
//
// ----------------------------------------------------------------------- //

void CAINode::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_dwID;
	*hWrite << m_strName;

	*hWrite << m_vPos;
	*hWrite << m_vUp;
	*hWrite << m_vRight;
	*hWrite << m_vForward;

	*hWrite << m_vDims;

	*hWrite << m_bConnectToVolume;

	hWrite->WriteDWord( m_pContainingVolumePtr ? m_pContainingVolumePtr->GetID() : CAIVolume::kInvalidID );

	*hWrite << m_bWallWalkOnly;
	*hWrite << m_vNormal;

	{for( NeighborIDList::iterator iter = m_BezierToID.begin();
	     iter != m_BezierToID.end(); ++iter )
	{
		*hWrite << *iter;
	}}
	*hWrite << m_vBezierPrev;
	*hWrite << m_vBezierNext;

	*hWrite << m_bAllowAliens;
	*hWrite << m_bAllowMarines;
	*hWrite << m_bAllowPredators;
	*hWrite << m_bAllowCorporates;

	hWrite->WriteDWord( m_JumpToID.size() );
	{for( NeighborIDList::iterator iter = m_JumpToID.begin();
	     iter != m_JumpToID.end(); ++iter )
	{
		*hWrite << *iter;
	}}

	*hWrite << m_hLockOwner;

	*hWrite << m_bIsActive;

	*hWrite << m_hTriggerObject;
	*hWrite << m_strTriggerMsg;
	*hWrite << m_strSelfTriggerMsg;

	*hWrite << m_hAINodeGroup;

	hWrite->WriteDWord(m_NeighborIDList.size());
	{for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
	     iter != m_NeighborIDList.end(); ++iter )
	{
		*hWrite << *iter;
	}}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINode::Load()
//
//	PURPOSE:	Load the node
//
// ----------------------------------------------------------------------- //

void CAINode::Load(HMESSAGEREAD hRead)
{
	ASSERT(hRead);
	if( !hRead ) return;

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}


void CAINode::InternalLoad(HMESSAGEREAD hRead)
{
	*hRead >> m_dwID;
	*hRead >> m_strName;

	*hRead >> m_vPos;
	*hRead >> m_vUp;
	*hRead >> m_vRight;
	*hRead >> m_vForward;

	*hRead >> m_vDims;

	*hRead >> m_bConnectToVolume;

	// This relies on AIVolumeMgr being called before AINodeMgr!
	// Otherwise, the volume pointers won't be set-up yet.
	m_pContainingVolumePtr = g_pAIVolumeMgr->GetVolumePtr( hRead->ReadDWord() );

	*hRead >> m_bWallWalkOnly;
	*hRead >> m_vNormal;

	{for( NeighborIDList::iterator iter = m_BezierToID.begin();
	     iter != m_BezierToID.end(); ++iter )
	{
		*hRead >> *iter;
	}}
	*hRead >> m_vBezierPrev;
	*hRead >> m_vBezierNext;

	*hRead >> m_bAllowAliens;
	*hRead >> m_bAllowMarines;
	*hRead >> m_bAllowPredators;
	*hRead >> m_bAllowCorporates;

	m_JumpToID.clear();
	m_JumpToID.assign(hRead->ReadDWord(),CAINode::kInvalidID);
	{for( NeighborIDList::iterator iter = m_JumpToID.begin();
	     iter != m_JumpToID.end(); ++iter )
	{
		*hRead >> *iter;
	}}

	*hRead >> m_hLockOwner;

	*hRead >> m_bIsActive;

	*hRead >> m_hTriggerObject;
	*hRead >> m_strTriggerMsg;
	*hRead >> m_strSelfTriggerMsg;

	*hRead >> m_hAINodeGroup;

	m_NeighborIDList.clear();
	m_NeighborIDList.assign(hRead->ReadDWord(),CAINode::kInvalidID);
	{for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
	     iter != m_NeighborIDList.end(); ++iter )
	{
		*hRead >> *iter;
	}}
}


