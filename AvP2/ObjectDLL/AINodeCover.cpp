// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCover.h
//
// PURPOSE : Tactical node definition 
//
// CREATED : 08/28/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeCover.h"
#include "ServerUtilities.h"
#include "AIUtils.h"
#include "AI.h"
#include "AITarget.h"

BEGIN_CLASS(AINodeCover)

	PROP_DEFINEGROUP(CoverInfo, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Crouch, LTFALSE, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Cower, LTFALSE, PF_GROUP1)
		ADD_REALPROP_FLAG_HELP(CoverRadius, 256.0f, PF_GROUP1|PF_RADIUS, "AIs inside this radius may dodge to this node.  This value does not affect advancing behavior.")

	PROP_DEFINEGROUP(NeighborLinks, PF_HIDDEN)
														
END_CLASS_DEFAULT_FLAGS(AINodeCover, AINode, NULL, NULL, NULL)


AINodeCover::AINodeCover()
{
	m_fCoverRadiusSqr = 0.0f;
	m_bCrouch = LTFALSE;
	m_bCower = LTFALSE;
}


CAINode * AINodeCover::MakeCAINode(DDWORD dwID)
{
	return CheckCAINode(new CAINodeCover(*this,dwID));
}

void AINodeCover::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pServerDE || !pData) return;

	AINode::ReadProp(pData);

	if ( g_pServerDE->GetPropGeneric( "Crouch", &genProp ) == LT_OK )
		m_bCrouch = genProp.m_Bool;

	if ( g_pServerDE->GetPropGeneric( "Cower", &genProp ) == LT_OK )
		m_bCower= genProp.m_Bool;

	if ( g_pServerDE->GetPropGeneric( "CoverRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fCoverRadiusSqr = genProp.m_Float*genProp.m_Float;
}

////////////////////////////////////////////////////////////////////////////
//
//      CAINodeCover
//
////////////////////////////////////////////////////////////////////////////

const char * const CAINodeCover::szTypeName = "CAINodeCover";


CAINodeCover::CAINodeCover(AINodeCover & node, DDWORD dwID)
	: CAINode(node,dwID)
{
	m_bCrouch = node.m_bCrouch;
	m_bCower = node.m_bCower;
	m_fCoverRadiusSqr = node.m_fCoverRadiusSqr;
}

CAINodeCover::CAINodeCover(HMESSAGEREAD hRead)
	: CAINode( hRead )
{
	InternalLoad(hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeCover::IsCoverFromTarget()
//
//	PURPOSE:	Returns true if the node provides cover from the target.
//
// ----------------------------------------------------------------------- //
LTBOOL CAINodeCover::IsCoverFromTarget(const CAI & ai) const
{
	const CAITarget & target = ai.GetTarget();

	if( !target.IsValid() )
		return LTFALSE;

	return IsCoverFromThreat(ai, target.GetObject());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAINodeCover::IsCoverFromThreat()
//
//	PURPOSE:	Is cover from a threat
//
// ----------------------------------------------------------------------- //
LTBOOL CAINodeCover::IsCoverFromThreat(const CAI & ai, HOBJECT hThreat) const
{
	if( !hThreat )
		return LTFALSE;

	LTVector vCoverFrom = GetPos();
	if( m_bCrouch )
	{
		vCoverFrom += const_cast<CharacterMovement*>(ai.GetMovement())->GetDimsSetting(CM_DIMS_CROUCHED).y;
	}
	else
	{
		vCoverFrom += ai.GetChestOffset();
	}

	++g_cIntersectSegmentCalls;
	return !TestLOSIgnoreCharacters(hThreat, NULL, vCoverFrom);
}


void CAINodeCover::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}


void CAINodeCover::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_strCoverObject;
	*hRead >> m_fCoverRadiusSqr;
	*hRead >> m_bCrouch;
	*hRead >> m_bCower;
}

void CAINodeCover::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);

	*hWrite << m_strCoverObject;
	*hWrite << m_fCoverRadiusSqr;
	*hWrite << m_bCrouch;
	*hWrite << m_bCower;
}
