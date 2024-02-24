// ----------------------------------------------------------------------- //
//
// MODULE  : AIState.cpp
//
// PURPOSE : AI State class implementations
//
// CREATED : 12/30/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIState.h"
#include "AI.h"
#include "AINodeMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIState::CAIState(CAI * pAI)
	: m_pAI(pAI),
	  m_bFirstUpdate(LTTRUE)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::PostUpdate
//
//	PURPOSE:	PostUpdate of the state
//
// ----------------------------------------------------------------------- //

void CAIState::PostUpdate()
{
	m_bFirstUpdate = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Load
//
//	PURPOSE:	Restores the state
//
// ----------------------------------------------------------------------- //

void CAIState::Load(HMESSAGEREAD hRead)
{
	_ASSERT(hRead);
	if (!hRead) return;

	*hRead >> m_bFirstUpdate;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Save
//
//	PURPOSE:	Saves the state
//
// ----------------------------------------------------------------------- //

void CAIState::Save(HMESSAGEREAD hWrite)
{
	_ASSERT(hWrite);
	if (!hWrite) return;

	*hWrite << m_bFirstUpdate;
}



