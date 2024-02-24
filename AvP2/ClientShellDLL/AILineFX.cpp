// ----------------------------------------------------------------------- //
//
// MODULE  : AILineFX.cpp
//
// PURPOSE : AILine special FX - Implementation
//
// CREATED : 1/17/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AILineFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAILineFX::Init
//
//	PURPOSE:	Init the cast line
//
// ----------------------------------------------------------------------- //

LTBOOL CAILineFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	if( !hRead ) return LTFALSE;
	if (!CSpecialFX::Init(hServObj, hRead)) return LTFALSE;

	AILCREATESTRUCT createStruct;

	createStruct.hServerObj = hServObj;

	const int num_lines = hRead->ReadWord();
	for(int i = 0; i < num_lines; ++i)
	{
		DebugLine new_line;
		*hRead >> new_line;

		createStruct.lines.push_back(new_line);
	}

	return Init(&createStruct);
}

LTBOOL CAILineFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	AILCREATESTRUCT* pAIL = (AILCREATESTRUCT*)psfxCreateStruct;
	m_InitData = *pAIL;

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAILineFX::Update
//
//	PURPOSE:	Update the cast line (recalculate end point)
//
// ----------------------------------------------------------------------- //

LTBOOL CAILineFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return LTFALSE;

	if ( IsWaitingForRemove() )
	{
		return LTFALSE;
	}

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = false;

		// Find the central point.
		LTVector central_point = LTVector(0.0f,0.0f,0.0f);
		{ for(AILCREATESTRUCT::LineList::const_iterator iter = m_InitData.lines.begin();
		    iter != m_InitData.lines.end(); ++iter )
		{
			central_point += iter->vSource;
			central_point += iter->vDest;
		}}

		if( !m_InitData.lines.empty() )
		{
			central_point /= float( 2*m_InitData.lines.size() );
		}

		
		// Put the line system at central point.
		m_pClientDE->SetObjectPos(m_hObject, &central_point);

		// Add all the lines (translated by the line system's location).
		{ for(AILCREATESTRUCT::LineList::const_iterator iter = m_InitData.lines.begin();
		    iter != m_InitData.lines.end(); ++iter )
		{
			const DebugLine & debug_line = *iter;
			LTLine lt_line;

			lt_line.m_Points[0].m_Pos = debug_line.vSource - central_point;
			lt_line.m_Points[0].r = debug_line.vColor.x;
			lt_line.m_Points[0].g = debug_line.vColor.y;
			lt_line.m_Points[0].b = debug_line.vColor.z;
			lt_line.m_Points[0].a = debug_line.fAlpha;

			lt_line.m_Points[1].m_Pos = debug_line.vDest - central_point;
			lt_line.m_Points[1].r = debug_line.vColor.x;
			lt_line.m_Points[1].g = debug_line.vColor.y;
			lt_line.m_Points[1].b = debug_line.vColor.z;
			lt_line.m_Points[1].a = debug_line.fAlpha;

			m_pClientDE->AddLine(m_hObject, &lt_line);
		}}
	}

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_CastLineFactory
//-------------------------------------------------------------------------------------------------

const SFX_AILineFactory SFX_AILineFactory::registerThis;

CSpecialFX* SFX_AILineFactory::MakeShape() const
{
	return new CAILineFX();
}

