// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AINodeSniper.h"
#include "AIUtils.h"

LINKFROM_MODULE( AINodeSniper );


BEGIN_CLASS(AINodeSniper)
	ADD_REALPROP_FLAG(Fov,						60.0f,			0|PF_FIELDOFVIEW)
	ADD_REALPROP_FLAG(ThreatRadius,				256.0f,			0|PF_RADIUS)
END_CLASS_DEFAULT_FLAGS(AINodeSniper, AINodeUseObject, NULL, NULL, 0)

CMDMGR_BEGIN_REGISTER_CLASS(AINodeSniper)
CMDMGR_END_REGISTER_CLASS(AINodeSniper, AINodeUseObject)

AINodeSniper::AINodeSniper()
{
	m_fFovDp = 0.0f;
	m_fThreatRadiusSqr = 0.0f;
}

AINodeSniper::~AINodeSniper()
{
}

void AINodeSniper::ReadProp(ObjectCreateStruct* pocs)
{
	super::ReadProp(pocs);

    if ( g_pLTServer->GetPropGeneric( "Fov", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fFovDp = FOV2DP(g_gp.m_Float);

    if ( g_pLTServer->GetPropGeneric( "ThreatRadius", &g_gp ) == LT_OK )
		if ( g_gp.m_String[0] )
			m_fThreatRadiusSqr = g_gp.m_Float*g_gp.m_Float;
}

void AINodeSniper::Init()
{
	super::Init();
}

EnumNodeStatus AINodeSniper::GetStatus( CAI* pAI, const LTVector& vPos, HOBJECT hThreat) const
{
	if( !hThreat )
	{
		return kStatus_Ok;
	}

	LTVector vThreatPos;
    g_pLTServer->GetObjectPos(hThreat, &vThreatPos);

	float fDistSqr = VEC_DISTSQR(m_vPos, vThreatPos);
	if ( fDistSqr < m_fThreatRadiusSqr )
	{
		return kStatus_ThreatInsideRadius;
	}

	LTVector vThreatDir = vThreatPos - m_vPos;
	vThreatDir.Normalize();

	if ( vThreatDir.Dot(m_vForward) > m_fFovDp )
	{
		return kStatus_Ok;
	}
	else
	{
		return kStatus_ThreatOutsideFOV;
	}
}

void AINodeSniper::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_fFovDp);
	SAVE_FLOAT(m_fThreatRadiusSqr);
}

void AINodeSniper::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT(m_fFovDp);
	LOAD_FLOAT(m_fThreatRadiusSqr);
}

