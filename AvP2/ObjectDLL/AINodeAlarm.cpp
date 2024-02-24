// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeAlarm.h
//
// PURPOSE : Tactical node definition 
//
// CREATED : 08/28/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeAlarm.h"
#include "ServerUtilities.h"
#include "AIUtils.h"


BEGIN_CLASS(AINodeAlarm)

	ADD_OBJECTPROP_FLAG_HELP(AlarmObject, "", 0, "Point this to an Alarm object")
	ADD_STRINGPROP_FLAG_HELP(AlarmMsg, "ON", 0, "The message to be sent to the Alarm (ON or OFF).")

	ADD_STRINGPROP_FLAG_HELP(SelfTriggerMsg, "", 0, "AI will receive this trigger message when he reaches this alarm node.")
	
	ADD_BOOLPROP_FLAG( WallWalk, LTFALSE, PF_HIDDEN)
	ADD_BOOLPROP_FLAG( VolumeConnect, LTTRUE, PF_HIDDEN)

	PROP_DEFINEGROUP(NeighborLinks, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(AINodeAlarm, AINode, LTNULL, LTNULL, LTNULL)


AINodeAlarm::AINodeAlarm()
{
	m_lstrAlarmObject = LTNULL;
	m_lstrAlarmMsg = LTNULL;
}


CAINode * AINodeAlarm::MakeCAINode(DDWORD dwID)
{
	return CheckCAINode(new CAINodeAlarm(*this,dwID));
}

void AINodeAlarm::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pServerDE || !pData) return;

	AINode::ReadProp(pData);

	if ( g_pLTServer->GetPropGeneric( "AlarmObject", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_lstrAlarmObject = genProp.m_String;

	if ( g_pLTServer->GetPropGeneric( "AlarmMsg", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_lstrAlarmMsg = genProp.m_String;
}

////////////////////////////////////////////////////////////////////////////
//
//      CAINodeAlarm
//
////////////////////////////////////////////////////////////////////////////

const char * const CAINodeAlarm::szTypeName = "CAINodeAlarm";


CAINodeAlarm::CAINodeAlarm(AINodeAlarm & node, DDWORD dwID)
	: CAINode(node,dwID)
{
	m_lstrAlarmMsg = node.m_lstrAlarmMsg;

	// Find the alarm object.
	if( !node.m_lstrAlarmObject.IsNull() )
	{
		ObjArray<HOBJECT,1> objArray;
		if( LT_OK == g_pServerDE->FindNamedObjects( const_cast<char*>( node.m_lstrAlarmObject.CStr() ), objArray ) )
		{
			if( objArray.NumObjects() > 0 )
			{
				// Record our next point.
				m_hAlarmObject = objArray.GetObject(0);

			}
			else
			{
//				g_pLTServer->CPrint("Node \"%s\" could found %d objects named \"%s\".",
//					m_lstrName.CStr(),
//					objArray.NumObjects(),
//					node.node.m_lstrAlarmObject.CStr() );
			}
		}
		else
		{
//			g_pLTServer->CPrint("Node \"%s\" could not find object named \"%s\".",
//				m_lstrName.CStr(),
//				node.m_lstrAlarmObject.CStr() );
		}
	}
}

CAINodeAlarm::CAINodeAlarm(HMESSAGEREAD hRead)
	: CAINode( hRead )
{
	InternalLoad(hRead);
}


void CAINodeAlarm::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}


void CAINodeAlarm::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_hAlarmObject;
	*hRead >> m_lstrAlarmMsg;
}

void CAINodeAlarm::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);

	*hWrite << m_hAlarmObject;
	*hWrite << m_lstrAlarmMsg;
}
