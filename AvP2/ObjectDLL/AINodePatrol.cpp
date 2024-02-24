// ----------------------------------------------------------------------- //
//
// MODULE  : AINodePatrol.cpp
//
// PURPOSE : AINodePatrol implementation 
//
// CREATED : 9/14/00
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AINodePatrol.h"
#include "AINodeMgr.h"
#include "DebugLineSystem.h"
#include "AIUtils.h"

const char * const CAINodePatrol::szTypeName = "CAINodePatrol";

////////////////////////////////////////////////////////////////////////////
//
//      AINodePatrol
//
////////////////////////////////////////////////////////////////////////////

BEGIN_CLASS(AINodePatrol)

	ADD_REALPROP_FLAG_HELP(WaitTime, 0.0f, 0, "AI will wait this many seconds before going to next node.")
	ADD_OBJECTPROP_FLAG_HELP(NextPoint, "", 0, "AI will go to next point after reaching current point.")
	ADD_BOOLPROP_FLAG_HELP(Circular, FALSE, 0, "If true and a next node can not be found, BaseName0 will be tried.  BaseName is the base name of this node, of course." )
	ADD_BOOLPROP_FLAG_HELP(Cycle, FALSE, 0, "If true and this is the last (or first) node, the AI will turn around and follow the path in reverse." )

END_CLASS_DEFAULT(AINodePatrol, AINode, NULL, NULL)


CAINode * AINodePatrol::MakeCAINode(DDWORD dwID)
{
	return CheckCAINode(new CAINodePatrol(*this,dwID));
}


void AINodePatrol::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pServerDE || !pData) return;

	AINode::ReadProp(pData);
	
	if ( g_pServerDE->GetPropGeneric( "WaitTime", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_fWaitTime = genProp.m_Float;


	if ( g_pServerDE->GetPropGeneric( "NextPoint", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_strNextName = genProp.m_String;

	if( g_pServerDE->GetPropGeneric( "Cycle", &genProp ) == DE_OK )
	{
		m_bCycle = (genProp.m_Bool == LTTRUE);
	}

	if( g_pServerDE->GetPropGeneric( "Circular", &genProp ) == DE_OK )
	{
		m_bCircular = (genProp.m_Bool == LTTRUE);
	}
};


void AINodePatrol::FirstUpdate()
{
	AINode::FirstUpdate();

	if( m_strNextName.empty() )
	{
		// Use the base name technique.

		SequintialBaseName base_name(g_pLTServer->GetObjectName(m_hObject));

		// Don't try that circular thing with the first node!
		if( base_name.m_nValue == 0 ) 
			m_bCircular = false;

		base_name.m_nValue++;

		HOBJECT hNextNode = base_name.FindObject();

		if( hNextNode )
		{
			m_strNextName = g_pLTServer->GetObjectName(hNextNode);
		}
		else if( m_bCircular )
		{
			// We couldn't find a next node, and it is marked circular, so just set
			// the next node to be "BaseName0".
			m_strNextName = base_name.m_strName;
			m_strNextName += "0";
		}
	}

	if( !m_strNextName.empty() )
	{
		HOBJECT hNextNode = LTNULL;
		if( LT_OK == FindNamedObject(m_strNextName.c_str(), hNextNode) )
		{
			// Record our next point.
			m_hNext = hNextNode;

			// Tell next of our existance
			AINodePatrol * pNext = dynamic_cast<AINodePatrol*>( g_pServerDE->HandleToObject(hNextNode) );
			if( pNext )
			{
				pNext->m_hPrev = m_hObject;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////
//
//      CAINodePatrol
//
////////////////////////////////////////////////////////////////////////////


CAINodePatrol::CAINodePatrol(AINodePatrol & node, DDWORD dwID)
	: CAINode(node,dwID),
	  m_fWaitTime(node.m_fWaitTime),
	  m_bCycle(node.m_bCycle)
{
	node.m_nID = dwID;

	//
	// Set-up the next node information.
	//
	ASSERT( g_pAINodeMgr );
	if( !g_pAINodeMgr ) return;

	if( node.m_hNext )
	{
		// If the next node still exists as an engine object, record all important information
		// for it.
		AINodePatrol * pNextNode = dynamic_cast<AINodePatrol*>( g_pServerDE->HandleToObject(node.m_hNext) );
		if( pNextNode )
		{
			node.m_nNextID = pNextNode->m_nID;
			pNextNode->m_nPrevID = dwID;
		}
	}
	
	// Set the id's for the node manager class.
	m_NextID = node.m_nNextID;
	if( m_NextID < g_pAINodeMgr->GetNumNodes() )
	{
		// Our next node already exists in the nodemgr.  Set it's previous id to ourself.
		CAINodePatrol * pNextNode = dynamic_cast<CAINodePatrol*>( g_pAINodeMgr->GetNode(m_NextID) );
		ASSERT( pNextNode );
		if( pNextNode )
		{
			pNextNode->SetPrevID(dwID);
		}
	}


	// Repeat the above for the previous link as well.


	if( node.m_hPrev )
	{
		// If the previous node still exists as an engine object, record all important information
		// for it.
		AINodePatrol * pPrevNode = dynamic_cast<AINodePatrol*>( g_pServerDE->HandleToObject(node.m_hPrev) );
		if( pPrevNode )
		{
			node.m_nPrevID = pPrevNode->m_nID;
			pPrevNode->m_nNextID = dwID;
		}
	}

	// Set the id's for the node manager class.
	m_PrevID = node.m_nPrevID;
	if( m_PrevID < g_pAINodeMgr->GetNumNodes() )
	{
		// Our next node already exists in the nodemgr.  Set it's previous id to ourself.
		CAINodePatrol * pPrevNode = dynamic_cast<CAINodePatrol*>( g_pAINodeMgr->GetNode(m_PrevID) );
		ASSERT( pPrevNode );
		if( pPrevNode )
		{
			pPrevNode->SetNextID(dwID);
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodePatrol::CAINodePatrol
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAINodePatrol::CAINodePatrol(HMESSAGEREAD hRead)
	: CAINode( hRead )
{
	InternalLoad(hRead);
}

void CAINodePatrol::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}

void CAINodePatrol::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_fWaitTime;
	*hRead >> m_bCycle;
	*hRead >> m_NextID;
	*hRead >> m_PrevID;
}


void CAINodePatrol::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);

	*hWrite << m_fWaitTime;
	*hWrite << m_bCycle;
	*hWrite << m_NextID;
	*hWrite << m_PrevID;
}

#ifndef _FINAL

void CAINodePatrol::DrawDebug(int level)
{
	CAINode::DrawDebug(level);

	CAINode * pNextNode = m_NextID != CAINode::kInvalidID ? g_pAINodeMgr->GetNode(m_NextID) : LTNULL;
	CAINode * pPrevNode = m_PrevID != CAINode::kInvalidID ? g_pAINodeMgr->GetNode(m_PrevID) : LTNULL;

	LineSystem::GetSystem(this,"PatrolConnections")
		<< LineSystem::Clear();

	if( pNextNode )
	{
		LineSystem::GetSystem(this,"PatrolConnections")
			<< LineSystem::Line( GetPos(), pNextNode->GetPos(), Color::Blue,1.0f);
	}

	if( pPrevNode )
	{
		LineSystem::GetSystem(this,"PatrolConnections")
			<< LineSystem::Line( GetPos(), pPrevNode->GetPos(), Color::Blue,1.0f);
	}
}

void CAINodePatrol::ClearDebug() 
{
	CAINode::ClearDebug();

	LineSystem::GetSystem(this,"PatrolConnections")
		<< LineSystem::Clear();
}

#endif
