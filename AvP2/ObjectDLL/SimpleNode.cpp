// ----------------------------------------------------------------------- //
//
// MODULE  : SimpleNode.cpp
//
// PURPOSE : SimpleNode implementation
//
// CREATED : 12/15/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SimpleNode.h"
#include "SimpleNodeMgr.h"
#include "DebugLineSystem.h"
#include "AIUtils.h"

#include <algorithm>

#ifdef _DEBUG
#endif

BEGIN_CLASS(SimpleNode)

	ADD_VECTORPROP_VAL_FLAG_HELP(Dims, 16.0f, 16.0f, 16.0f, PF_DIMS, "The y-dims (height) of the node will be used to determine if the character is in the node.")
	ADD_REALPROP_FLAG_HELP(Radius, 32.0f, PF_RADIUS, "If the character is within the height of the node and within this radius of the center, the character is in the node.")
	ADD_VECTORPROP_VAL_FLAG_HELP(Offset, 0.0f, 0.0f, 0.0f, 0, "The attractor for the simple ai will be at this offset from the node's position." )

	ADD_BOOLPROP_FLAG_HELP(StartHidden, LTFALSE, PF_HIDDEN, "This will start the object as 'hidden 1'.")
	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(SimpleNode, GameBase, NULL, NULL, 0)



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleNode::ReadProp
//
//	PURPOSE:	Reads properties
//
// ----------------------------------------------------------------------- //

void SimpleNode::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pLTServer || !pData) return;

	if ( g_pLTServer->GetPropGeneric( "Dims", &genProp ) == DE_OK )
	{
		m_vDims = genProp.m_Vec;
	}

	if ( g_pLTServer->GetPropGeneric( "Radius", &genProp ) == DE_OK )
	{
		m_fRadius = genProp.m_Float;
	}

	if ( g_pLTServer->GetPropGeneric( "Offset", &genProp ) == DE_OK )
	{
		m_vOffset = genProp.m_Vec;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SimpleNode::EngineMessageFn
//
//	PURPOSE:	Handles engine message functions
//
// ----------------------------------------------------------------------- //

uint32 SimpleNode::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
//	ROUTINE:	SimpleNode::EngineMessageFn
//
//	PURPOSE:	Handles engine message functions
//
// ----------------------------------------------------------------------- //

void SimpleNode::PostStartWorld(uint8 nLoadGameFlags)
{
	ASSERT( g_pSimpleNodeMgr );

	GameBase::PostStartWorld(nLoadGameFlags);

	if( nLoadGameFlags != LOAD_RESTORE_GAME )
	{
		g_pSimpleNodeMgr->AddNode(*this);
		g_pLTServer->RemoveObject(m_hObject);
	}
}

////////////////////////////////////////////////////////////////////////////
//
//      CSimpleNode
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNode::CSimpleNode()
//
//	PURPOSE:	Initialize object from null data (to be used before loading from file).
//
// ----------------------------------------------------------------------- //


CSimpleNode::CSimpleNode()
	: m_dwID(),
	  // m_strName

	  m_vPos(0,0,0),
	  m_vAttractorPos(0,0,0),

	  m_vDims(0,0,0),
	  m_fRadius(0),
	  m_fRadiusSqr(0),

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
//	ROUTINE:	CSimpleNode::CSimpleNode()
//
//	PURPOSE:	Initialize object from SimpleNode
//
// ----------------------------------------------------------------------- //


CSimpleNode::CSimpleNode(const SimpleNode & node, uint32 dwID)
	: m_dwID(dwID),
	  m_strName( g_pLTServer->GetObjectName(node.m_hObject) ),

	  m_vPos(0,0,0),
	  m_vAttractorPos(node.m_vOffset),

	  m_vDims(node.m_vDims),
	  m_fRadius(node.m_fRadius),
	  m_fRadiusSqr( node.m_fRadius*node.m_fRadius ),

	  m_nLastPathingIteration(0),
	  m_bResetNeighbors(false),

	  m_bIsGoal(false),

	  m_fCost(0.0f),
	  m_fHeuristic(0.0f),

	  m_bInOpen(false),
	  m_bInClosed(false),
	  m_pPrevNode(LTNULL)
{
	g_pLTServer->GetObjectPos(node.m_hObject, &m_vPos);

	m_vAttractorPos += m_vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNode::CSimpleNode()
//
//	PURPOSE:	Creates a node based on the original node but with a different position.
//
// ----------------------------------------------------------------------- //

CSimpleNode::CSimpleNode( const CSimpleNode & orig_node, const LTVector & vPos)
	: m_dwID(CSimpleNode::kInvalidID ),
	  m_strName(orig_node.m_strName),


	  m_vPos(vPos),
	  m_vAttractorPos(orig_node.m_vAttractorPos),

	  m_vDims(orig_node.m_vDims),
	  m_fRadius(orig_node.m_fRadius),
	  m_fRadiusSqr(orig_node.m_fRadiusSqr),

	  m_nLastPathingIteration(orig_node.m_nLastPathingIteration),
	  m_bResetNeighbors(orig_node.m_bResetNeighbors),

	  m_hDebugNodeBox(LTNULL),


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
//	ROUTINE:	CSimpleNode::AddNeighbor()
//
//	PURPOSE:	Makes a pathing connection between two nodes
//
// ----------------------------------------------------------------------- //

void  CSimpleNode::AddNeighbor(uint32 dwNeighborID)
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
//	ROUTINE:	CSimpleNode::DrawDebug()
//
//	PURPOSE:	Draw the box models and show the connections.
//
// ----------------------------------------------------------------------- //
#ifndef _FINAL

void CSimpleNode::DrawDebug(int level) 
{
	CSimpleNode::ClearDebug();

	// Draw the node.

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	sprintf(theStruct.m_Name, "%s-DebugBox", GetName().c_str() );

	theStruct.m_Pos = GetPos();

#ifdef _WIN32
	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.abc");
#else
	SAFE_STRCPY(theStruct.m_Filename, "Models/1x1_square.abc");
#endif

	theStruct.m_Scale = m_vDims;

	theStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	theStruct.m_ObjectType = OT_MODEL;

	HCLASS hClass = g_pLTServer->GetClass("BaseClass");
	LPBASECLASS pModel = g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pModel) return;

	LTVector color = LTVector(1,0,0);
	LTFLOAT  alpha = 0.5f;

	g_pLTServer->SetObjectColor(pModel->m_hObject, color.x, color.y, color.z, alpha);

	m_hDebugNodeBox = pModel->m_hObject;

	// Draw the connections.
	for( NeighborList::iterator iter =  BeginNeighbors();
		 iter !=  EndNeighbors(); ++iter )
	{
		LineSystem::GetSystem(this,"Connections")
			<< LineSystem::Arrow(GetPos(), (*iter)->GetPos(), color,alpha);
	}
}

void CSimpleNode::ClearDebug() 
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

void CSimpleNode::ResetPathing(uint32 nPathingIteration )
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



void CSimpleNode::InternalResetPathing()
{
	m_bIsGoal = false;

	m_fCost = 0.0f;
	m_fHeuristic = 0.0f;

	m_bInOpen = false;
	m_bInClosed = false;
	m_pPrevNode = LTNULL;
}


CSimpleNode::NeighborIterator CSimpleNode::BeginNeighbors()
{
	// Be sure m_NeighborList is up to date.
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CSimpleNode * pNode = g_pSimpleNodeMgr->GetNode(*iter);
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

CSimpleNode::NeighborIterator CSimpleNode::EndNeighbors()
{
	// Be sure m_NeighborList is up to date.
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CSimpleNode * pNode = g_pSimpleNodeMgr->GetNode(*iter);
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



CSimpleNode::const_NeighborIterator CSimpleNode::BeginNeighbors() const
{
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::const_iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CSimpleNode * pNode = g_pSimpleNodeMgr->GetNode(*iter);
			_ASSERT( pNode );
			if(pNode) m_NeighborList.push_back(pNode);
		}
	}

	return m_NeighborList.begin();
}

CSimpleNode::const_NeighborIterator CSimpleNode::EndNeighbors() const
{
	if( m_NeighborList.size() != m_NeighborIDList.size() )
	{
		m_NeighborList.clear();
		for( NeighborIDList::const_iterator iter = m_NeighborIDList.begin();
		     iter != m_NeighborIDList.end(); ++iter )
		{
			CSimpleNode * pNode = g_pSimpleNodeMgr->GetNode(*iter);
			_ASSERT( pNode );
			if(pNode) m_NeighborList.push_back(pNode);
		}
	}

	return m_NeighborList.end();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNode::Save()
//
//	PURPOSE:	Save the node 
//
// ----------------------------------------------------------------------- //

void CSimpleNode::Save(HMESSAGEWRITE hWrite)
{
	*hWrite << m_dwID;
	*hWrite << m_strName;

	*hWrite << m_vPos;
	*hWrite << m_vAttractorPos;

	*hWrite << m_vDims;
	*hWrite << m_fRadius;
	*hWrite << m_fRadiusSqr;

	hWrite->WriteDWord(m_NeighborIDList.size());
	{for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
	     iter != m_NeighborIDList.end(); ++iter )
	{
		*hWrite << *iter;
	}}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSimpleNode::Load()
//
//	PURPOSE:	Load the node
//
// ----------------------------------------------------------------------- //

void CSimpleNode::Load(HMESSAGEREAD hRead)
{
	ASSERT(hRead);
	if( !hRead ) return;

	*hRead >> m_dwID;
	*hRead >> m_strName;

	*hRead >> m_vPos;
	*hRead >> m_vAttractorPos;

	*hRead >> m_vDims;
	*hRead >> m_fRadius;
	*hRead >> m_fRadiusSqr;

	m_NeighborIDList.clear();
	m_NeighborIDList.assign(hRead->ReadDWord(),CSimpleNode::kInvalidID);
	{for( NeighborIDList::iterator iter = m_NeighborIDList.begin();
	     iter != m_NeighborIDList.end(); ++iter )
	{
		*hRead >> *iter;
	}}
}


