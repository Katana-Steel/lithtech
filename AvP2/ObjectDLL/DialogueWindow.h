// ----------------------------------------------------------------------- //
//
// MODULE  : DialogueWindow.h
//
// PURPOSE : DialogueWindow - Definition
//
// CREATED : 4/17/99
//
// COMMENT : Copyright (c) 1999, Monolith Productions, Inc.
//
// ----------------------------------------------------------------------- //

#ifndef _DIALOGUEWINDOW_H_
#define _DIALOGUEWINDOW_H_

#include "SCDefs.h"

#ifdef _AVP2BUILD
#include "Character.h"
#define CPrBase BaseClass
#define CActor CCharacter
#else // Sanity build
#include "PrBase.h"
#include "Actor.h"
#endif

class CinematicTrigger;
class CDialogueWindow : public CPrBase
{
	public:
		
		CDialogueWindow();
		~CDialogueWindow() { Term(); }

		BOOL				Init();
		void				Term();
		BOOL				IsInitialized() { return m_bInitialized; }
		void				Finished(BYTE byDecision = 0, DWORD dwDecision = 0);
		DDWORD				EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		BOOL				IsPlaying() { return m_bPlaying; }
		BOOL				PlayDialogue(DWORD dwID, CActor* pSpeaker, BOOL bStayOpen, const char *szActorName,
											const char *szDecisions = NULL);
		BOOL				PlayDialogue(DWORD dwID, BOOL bStayOpen, const char *szActorName, 
								   const char *szDecisions, CinematicTrigger* pCinematic);
		void				StopDialogue() { if(m_pCurSpeaker) m_pCurSpeaker->KillVoicedSound(); }
		BOOL				SendMessage(DWORD dwID, BOOL bStayOpen, const char *szActorName, const char *szDecisions);
	
	private:

		BOOL				m_bInitialized;
		CActor*				m_pCurSpeaker;
		CinematicTrigger*	m_pCinematic;
		BOOL				m_bPlaying;
		BOOL				m_byDecision;
};

inline CDialogueWindow::CDialogueWindow()
{
	m_bInitialized = FALSE;
	m_pCurSpeaker = NULL;
	m_pCinematic = NULL;
	m_bPlaying = FALSE;
	m_byDecision = 0;
}


inline BOOL CDialogueWindow::SendMessage(DWORD dwID, BOOL bStayOpen, const char *szActorName, const char *szDecisions)
{
	// Bring up a dialogue window on the client (NOTE: This goes to ALL CLIENTS)
	HMESSAGEWRITE hMsg = g_pInterface->StartMessage(DNULL, SCM_PLAY_DIALOGUE);
	g_pInterface->WriteToMessageDWord(hMsg, dwID);
	g_pInterface->WriteToMessageString(hMsg, const_cast<char*>(szActorName) );
	g_pInterface->WriteToMessageByte(hMsg,bStayOpen);
	g_pInterface->WriteToMessageString(hMsg, const_cast<char*>(szDecisions) );
	g_pInterface->EndMessage(hMsg);
	m_bPlaying = TRUE;
	return TRUE;
}

extern CDialogueWindow g_DialogueWindow;

#endif
