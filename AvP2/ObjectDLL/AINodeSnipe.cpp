// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSnipe.cpp 
//
// CREATED : 10/03/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AINodeSnipe.h"
#include "ServerUtilities.h"
#include "AIUtils.h"


BEGIN_CLASS(AINodeSnipe)

	PROP_DEFINEGROUP(SnipeInfo, PF_GROUP1)

		ADD_BOOLPROP_FLAG(Crouch,				DFALSE,		PF_GROUP1)
		ADD_REALPROP_FLAG(SnipeRadius,			384.0f,		PF_GROUP1|PF_RADIUS)
														
END_CLASS_DEFAULT_FLAGS(AINodeSnipe, AINode, NULL, NULL, NULL)


AINodeSnipe::AINodeSnipe()
{
	m_fRadiusSqr = 0.0f;
	m_bCrouch = DFALSE;
}


CAINode * AINodeSnipe::MakeCAINode(DDWORD dwID)
{
	return CheckCAINode(new CAINodeSnipe(*this,dwID));
}

void AINodeSnipe::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	if (!g_pServerDE || !pData) return;

	AINode::ReadProp(pData);

	if ( g_pServerDE->GetPropGeneric( "Crouch", &genProp ) == DE_OK )
		m_bCrouch = genProp.m_Bool;

	if ( g_pServerDE->GetPropGeneric( "SnipeRadius", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_fRadiusSqr = genProp.m_Float*genProp.m_Float;
}

////////////////////////////////////////////////////////////////////////////
//
//      CAINodeCover
//
////////////////////////////////////////////////////////////////////////////

const char * const CAINodeSnipe::szTypeName = "CAINodeSnipe";


CAINodeSnipe::CAINodeSnipe(AINodeSnipe & node, DDWORD dwID)
	: CAINode(node,dwID)
{
	m_bCrouch = node.m_bCrouch;
	m_fRadiusSqr = node.m_fRadiusSqr;
}

CAINodeSnipe::CAINodeSnipe(HMESSAGEREAD hRead)
	: CAINode( hRead )
{
	InternalLoad(hRead);
}

void CAINodeSnipe::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	CAINode::Load(hRead);

	// Need to have an internal load so that
	// we can just load our own variables from
	// the constructor.
	InternalLoad(hRead);
}


void CAINodeSnipe::InternalLoad(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead ) return;

	*hRead >> m_bCrouch;
	*hRead >> m_fRadiusSqr;
}

void CAINodeSnipe::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite ) return;

	CAINode::Save(hWrite);

	*hWrite << m_bCrouch;
	*hWrite << m_fRadiusSqr;
}
