// ----------------------------------------------------------------------- //
//
// MODULE  : DialogueWindow.cpp
//
// PURPOSE : DialogueWindow - Implementation
//
// CREATED : 4/17/99
//
// COMMENT : Copyright (c) 1999, Monolith Productions, Inc.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DialogueWindow.h"
#include "CinematicTrigger.h"

BEGIN_CLASS(CDialogueWindow)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(CDialogueWindow, BaseClass, NULL, NULL, CF_HIDDEN)

DDWORD CDialogueWindow::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch (messageID)
	{
		case MID_LINKBROKEN:
		{
			ASSERT(FALSE);
			if (m_pCurSpeaker && (HOBJECT)pData == m_pCurSpeaker->m_hObject)
			{
				m_pCurSpeaker = NULL;
			}

			if (m_pCinematic && (HOBJECT)pData == m_pCinematic->m_hObject)
			{
				m_pCinematic = NULL;
			}
			break;
		}
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

BOOL CDialogueWindow::Init()
{
	if(m_bInitialized)
		return FALSE;

	m_bInitialized = TRUE;
	m_bPlaying = FALSE;
	m_byDecision = NULL;
	m_pCurSpeaker = NULL;
	m_pCinematic = NULL;
	return TRUE;
}

void CDialogueWindow::Term()
{
	if(!m_bInitialized)
		return;

	m_bInitialized = FALSE;
}

void CDialogueWindow::Finished(BYTE byDecision, DWORD dwDecision)
{
	m_bPlaying = FALSE;

	// Unlink us
	ASSERT(m_pCurSpeaker || m_pCinematic);
	if(m_pCinematic)
	{
		// Since CinematicTrigger::DialogueFinished can start another piece
		// of dialogue, we need to break our link first
		CinematicTrigger* pCin = m_pCinematic;
		g_pServerDE->BreakInterObjectLink(m_hObject, m_pCinematic->m_hObject);
		m_pCinematic = NULL;
		pCin->DialogueFinished(byDecision, dwDecision);
	}
	else
	if(m_pCurSpeaker)
	{
		m_pCurSpeaker->StopDialogue();
		g_pServerDE->BreakInterObjectLink(m_hObject, m_pCurSpeaker->m_hObject);
		m_pCurSpeaker = NULL;
	}
}

BOOL CDialogueWindow::PlayDialogue(DWORD dwID, CActor* pSpeaker, BOOL bStayOpen, 
								   const char *szActorName, const char *szDecisions)
{
	// Link us up
	ASSERT(pSpeaker);
	m_pCurSpeaker = pSpeaker;
	g_pInterface->CreateInterObjectLink(m_hObject, pSpeaker->m_hObject);

	// Bring up a dialogue window on the client (NOTE: This goes to ALL CLIENTS)
	return(SendMessage(dwID,bStayOpen,szActorName,szDecisions));
}

BOOL CDialogueWindow::PlayDialogue(DWORD dwID, BOOL bStayOpen, const char *szActorName, 
								   const char *szDecisions, CinematicTrigger* pCinematic)
{
	// Link us up
	ASSERT(pCinematic);
	m_pCinematic = pCinematic;
	g_pInterface->CreateInterObjectLink(m_hObject, m_pCinematic->m_hObject);

	// Bring up a dialogue window on the client (NOTE: This goes to ALL CLIENTS)
	return(SendMessage(dwID,bStayOpen,szActorName,szDecisions));
}

