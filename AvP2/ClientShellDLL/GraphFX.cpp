// ----------------------------------------------------------------------- //
//
// MODULE  : GraphFX.cpp
//
// PURPOSE : Graph special FX - Implementation
//
// CREATED : 2/8/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GraphFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "VarTrack.h"

#define MAX_LINES_PER_SECOND	1000			// Max lines to be created in a second
#define MAX_TOTAL_LINES			4000			// Max lines total (all systems)
#define MAX_SYSTEM_LINES		2000			// Max lines per system
#define MAX_VIEW_DIST_SQR		(10000*10000)	// Max global distance to add lines

extern CGameClientShell* g_pGameClientShell;
extern DVector g_vWorldWindVel;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGraphFX::CGraphFX
//
//	PURPOSE:	Construct
//
// ----------------------------------------------------------------------- //

CGraphFX::CGraphFX()
	: m_bFirstUpdate(DFALSE),
	  m_fNextUpdate(0.0f),
	  m_fLastTime(0.0f),
		
	  m_vOffset(0.0f,0.0f,0.0f),
	  m_fWidth(0.0f),
	  m_fHeight(0.0f),
	  m_bIgnoreX(DFALSE),

	  m_pPoints(0),
	  m_nNumPoints(0) {}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGraphFX::~CGraphFX
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGraphFX::~CGraphFX()
{
	RemoveLines();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGraphFX::Init
//
//	PURPOSE:	Init the line system
//
// ----------------------------------------------------------------------- //

DBOOL CGraphFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseLineSystemFX::Init(psfxCreateStruct)) return DFALSE;

	// Record our creation info.

	GCREATESTRUCT * pInit = (GCREATESTRUCT*)psfxCreateStruct;
	m_vOffset = pInit->m_vOffsetPos;
	m_fWidth = pInit->m_fWidth;
	m_fHeight = pInit->m_fHeight;
	m_bIgnoreX = pInit->m_bIgnoreX;

	m_UpdateGraphCallback = pInit->m_UpdateGraphCallback;

	// Set our max viewable distance.

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGraphFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CGraphFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;

	// create the object
	DBOOL bRet = CBaseLineSystemFX::CreateObject(pClientDE);


	if (bRet && m_hObject && m_hServerObject)
	{
		// move the object to offset + server object's position.
		DVector vPos;
		g_pClientDE->GetObjectPos(m_hServerObject,&vPos);
		vPos += m_vOffset;
		g_pClientDE->SetObjectPos(m_hObject,&vPos);
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGraphFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CGraphFX::Update()
{
	if (!m_hObject || !m_pClientDE || IsWaitingForRemove() ) return DFALSE;

	const DFLOAT fTime = m_pClientDE->GetTime();


	if (m_bFirstUpdate)
	{
		m_fLastTime = fTime;
		m_bFirstUpdate = DFALSE;
	}

	// Make sure it is time to update...

	if (fTime < m_fLastTime + m_fNextUpdate)
	{
		return DTRUE;
	}


	
	// Update graph
	if( m_UpdateGraphCallback(&m_pPoints, &m_nNumPoints) )
	{
		RemoveLines();
		UpdateLines();
	}


	// Determine when next update should occur...

	m_fNextUpdate = 0.1f;
	m_fLastTime = fTime;

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGraphFX::AddLines
//
//	PURPOSE:	Add lines to the system
//
// ----------------------------------------------------------------------- //

void CGraphFX::RemoveLines()
{
	ASSERT(m_hObject);
	if (!m_hObject) return;

	HDELINE next_line = g_pClientDE->GetNextLine(m_hObject,NULL);
	while( next_line )
	{
		HDELINE remove_line = next_line;
		next_line = g_pClientDE->GetNextLine(m_hObject,remove_line);
		g_pClientDE->RemoveLine(m_hObject,remove_line);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGraphFX::AddLines
//
//	PURPOSE:	Add lines to the system
//
// ----------------------------------------------------------------------- //

void CGraphFX::UpdateLines()
{
	ASSERT( m_hObject );
	if( !m_hObject ) return;

	DELinePt last_point;
	last_point.m_Pos = DVector(-m_fWidth/2,-m_fHeight/2,0.0f);
	last_point.r = last_point.g 
		= last_point.b = last_point.a = 1.0f;

	int count = 0;
	for( GraphFXPoint * iter = m_pPoints; 
	     iter < m_pPoints + m_nNumPoints; 
		 ++iter)
	{
		const GraphFXPoint point = *iter;
		DELine new_line;
		
		new_line.m_Points[0] = last_point;

		if( !m_bIgnoreX )
			new_line.m_Points[1].m_Pos.x = (point.x - 0.5f)*m_fWidth;
		else
			new_line.m_Points[1].m_Pos.x = ((LTFLOAT)count / (LTFLOAT)m_nNumPoints - 0.5f) * m_fWidth;

		new_line.m_Points[1].m_Pos.y = (point.y - 0.5f)*m_fHeight;

		new_line.m_Points[1].r = point.r;
		new_line.m_Points[1].g = point.g;
		new_line.m_Points[1].b = point.b;
		new_line.m_Points[1].a = point.a;

		g_pClientDE->AddLine(m_hObject,&new_line);

		last_point = new_line.m_Points[1];

		++count;
	}
}


//-------------------------------------------------------------------------------------------------
// SFX_GraphFactory
//-------------------------------------------------------------------------------------------------

const SFX_GraphFactory SFX_GraphFactory::registerThis;

CSpecialFX* SFX_GraphFactory::MakeShape() const
{
	return new CGraphFX();
}

