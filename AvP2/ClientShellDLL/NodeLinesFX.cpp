// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLinesFX.cpp
//
// PURPOSE : NodeLines special FX - Implementation
//
// CREATED : 1/21/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "NodeLinesFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeLinesFX::Init
//
//	PURPOSE:	Init the tracer fx
//
// ----------------------------------------------------------------------- //

DBOOL CNodeLinesFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	NLCREATESTRUCT w;

	w.hServerObj	= hServObj;

	w.vSource = hRead->ReadVector();
	w.vDestination = hRead->ReadVector();

	return Init(&w);
}

DBOOL CNodeLinesFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	NLCREATESTRUCT* pNL = (NLCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vSource, pNL->vSource);
	VEC_COPY(m_vDestination, pNL->vDestination);

	m_bFirstUpdate = DTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeLinesFX::RemoveFX
//
//	PURPOSE:	Remove all fx
//
// ----------------------------------------------------------------------- //

void CNodeLinesFX::RemoveFX()
{
	if (m_pFX)
	{
		delete m_pFX;
		m_pFX = DNULL;
	}
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeLinesFX::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

DBOOL CNodeLinesFX::Update()
{
	if(!m_pClientDE) return DFALSE;

	if (m_bFirstUpdate)
	{
		BSCREATESTRUCT ex;

		DVector vDirection = m_vDestination - m_vSource;
		DFLOAT fDistance = VEC_MAG(vDirection);

		vDirection.Norm();

		ex.vPos = m_vSource + vDirection*fDistance/2.0f;

		m_pClientDE->AlignRotation(&ex.rRot, &vDirection, NULL);
		
		VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
		VEC_SET(ex.vInitialScale, 1.0f, 1.0f, fDistance);
		VEC_SET(ex.vFinalScale, 1.0f, 1.0f, fDistance);
		VEC_SET(ex.vInitialColor, 1.0f, 0.0f, 0.0f);
		VEC_SET(ex.vFinalColor, 1.0f, 0.0f, 0.0f);
		ex.bUseUserColors = DTRUE;

		ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
		ex.fLifeTime		= 500000.0f;
		ex.fInitialAlpha	= 1.0f;
		ex.fFinalAlpha		= 1.0f;
		ex.Filename			= "Models\\1x1_square.abc";
		ex.nType			= OT_MODEL;
		//ex.pSkin			= "SpecialFX\\Explosions\\Juggernaut.dtx";

		m_pFX = new CBaseScaleFX();
		if (!m_pFX)
		{
			RemoveFX();
			return DFALSE;
		}

		m_pFX->Init(&ex);
		m_pFX->CreateObject(m_pClientDE);

		m_bFirstUpdate = DFALSE;
	}

	if (IsWaitingForRemove())
	{
		RemoveFX();
		return DFALSE;
	}

	if (m_pFX)
	{
		m_pFX->Update();
	}

	return DTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_NodeLinesFactory
//-------------------------------------------------------------------------------------------------

const SFX_NodeLinesFactory SFX_NodeLinesFactory::registerThis;

CSpecialFX* SFX_NodeLinesFactory::MakeShape() const
{
	return new CNodeLinesFX();
}

