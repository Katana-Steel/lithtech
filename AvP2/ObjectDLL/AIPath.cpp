// ----------------------------------------------------------------------- //
//
// MODULE  : AIPath.cpp
//
// PURPOSE : Path information implementation
//
// CREATED : 12.29.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIPath.h"
#include "AI.h"
#include "AINode.h"

// CAIPathWaypoint


void CAIPathWaypoint::Load(HMESSAGEREAD hRead)
{
	m_nNodeID = hRead->ReadDWord();
	m_eInstruction = Instruction(hRead->ReadByte());
	m_vPosition = hRead->ReadVector();
	m_vNormal = hRead->ReadVector();

	m_vBezierSrc = hRead->ReadVector();
	m_vBezierDest = hRead->ReadVector();

	const int object_list_size = hRead->ReadDWord();
	for( int i = 0; i < object_list_size; ++i)
	{
		m_ObjectList.push_back( hRead->ReadObject() );
	}
}

void CAIPathWaypoint::Save(HMESSAGEWRITE hWrite)
{
	hWrite->WriteDWord(m_nNodeID);
	hWrite->WriteByte(m_eInstruction);
	hWrite->WriteVector(m_vPosition);
	hWrite->WriteVector(m_vNormal);

	hWrite->WriteVector(m_vBezierSrc);
	hWrite->WriteVector(m_vBezierDest);

	hWrite->WriteDWord( m_ObjectList.size() );
	for( ObjectArgumentList::iterator iter = m_ObjectList.begin();
	     iter != m_ObjectList.end(); ++iter )
	{
		hWrite->WriteObject(*iter);
	}
}

// CAIPath

CAIPathWaypoint CAIPath::defaultNullWaypoint(CAINode::kInvalidID);

void CAIPath::Load(HMESSAGEREAD hRead)
{
	const int num_waypoints = hRead->ReadDWord();
	for( int i = 0; i < num_waypoints; ++i )
	{
		m_Waypoints.push_back( CAIPathWaypoint(CAINode::kInvalidID) );
		*hRead >> m_Waypoints.back();
	}
}

void CAIPath::Save(HMESSAGEWRITE hWrite)
{
	hWrite->WriteDWord(m_Waypoints.size());

	for( WaypointList::iterator iter = m_Waypoints.begin();
	     iter != m_Waypoints.end(); ++iter )
	{
		*hWrite << *iter;
	}
}

