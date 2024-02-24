// ----------------------------------------------------------------------- //
//
// MODULE  : CinematicTrigger.h
//
// PURPOSE : CinematicTrigger - Definition
//
// CREATED : 2/17/99
//
// COMMENT : Copyright (c) 1999-2000, Monolith Productions, Inc.
//
// ----------------------------------------------------------------------- //

#ifndef __CINEMATIC_TRIGGER_H__
#define __CINEMATIC_TRIGGER_H__

#include "GameBase.h"
#include "ltsmartlink.h"

#include <vector>
#include <list>

#define MAX_CT_MESSAGES	20

class CinematicTrigger : public GameBase
{
	public:

		typedef std::vector<LTSmartLink> ParticipantList;

		typedef std::list<CinematicTrigger*> CinematicTriggerList;
		typedef CinematicTriggerList::const_iterator CinematicTriggerIterator;

		static CinematicTriggerIterator BeginCinematicTriggers() { return m_sCinematicTriggers.begin(); }
		static CinematicTriggerIterator EndCinematicTriggers()   { return m_sCinematicTriggers.end(); }


	public:
		CinematicTrigger();
		~CinematicTrigger();

        LTBOOL		DialogueFinished(BYTE byDecision, DWORD dwDecisionID);

        LTBOOL      HasCamera() const { return !!(m_hCamera); }

		void		Stop() { HandleOff(); }
		void		AIStop() { if( m_bAIInterruptible ) Stop(); }

	protected:

        LTBOOL      ReadProp(ObjectCreateStruct *pStruct);
        uint32      EngineMessageFn(uint32 messageID, void *pData, float lData);
        uint32      ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
		void		HandleLinkBroken(HOBJECT hLink);

	private:

        void        Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void        Load(HMESSAGEREAD hWrite, uint32 dwLoadFlags);

		void		TriggerMsg(const LTSmartLink & hSender, const char *pMsg);

        LTBOOL      Update();
        LTBOOL      UpdateDialogue();
		void		HandleOn();
		void		HandleOff();
		void		SendMessage();
		void		SendReplyMessage(int nReply);
        LTBOOL      StartDialogue(int nDecision = 0);
		HOBJECT		PlayedBy(const char *pszName);
        LTBOOL      IsActor(HOBJECT hObject);

		void		CreateCamera(ObjectCreateStruct *pStruct);
		void		CreateKeyFramer(ObjectCreateStruct *pStruct);

		HOBJECT		FindWhoPlaysDecision(uint8 byDecision);
		void		EnableClientSounds();

        LTFLOAT     m_fDelay[MAX_CT_MESSAGES];
		HSTRING		m_hstrDialogue[MAX_CT_MESSAGES];
		HSTRING		m_hstrWhoPlaysDialogue[MAX_CT_MESSAGES];
		HSTRING		m_hstrExpressionString[MAX_CT_MESSAGES];
		HSTRING		m_hstrTargetName[MAX_CT_MESSAGES];
		HSTRING		m_hstrMessageName[MAX_CT_MESSAGES];
		HSTRING		m_hstrWhoPlaysDecisions[MAX_CT_MESSAGES];
		HSTRING		m_hstrDecisions[MAX_CT_MESSAGES];
		HSTRING		m_hstrReplies[MAX_CT_MESSAGES];
		HSTRING		m_hstrRepliesTarget[MAX_CT_MESSAGES];
		HSTRING		m_hstrRepliesMsg[MAX_CT_MESSAGES];
        LTBOOL      m_bWindow[MAX_CT_MESSAGES];

        LTBOOL      m_bOn;
        LTBOOL      m_bNotified;
        LTBOOL      m_bCreateCamera;
        LTBOOL      m_bCreateKeyFramer;
        LTBOOL      m_bStartOn;
        LTBOOL      m_bOneTimeOnly;
        LTBOOL      m_bCanSkip;
		LTBOOL		m_bAIInterruptible;
        LTBOOL      m_bAllWindows;
        LTBOOL      m_bDialogueDone;
		LTBOOL		m_bLeaveCameraOn;

        uint8       m_byDecision;
        uint8       m_byLastReply;
        uint8       m_nCurMessage;
        LTFLOAT     m_fNextDialogueStart;

		LTSmartLink m_hCurSpeaker;
		LTSmartLink m_hLastSpeaker;
		LTSmartLink m_hCamera;
		LTSmartLink m_hKeyFramer;

		HSTRING		m_hstrCleanUpTriggerTarget;
		HSTRING		m_hstrCleanUpTriggerMsg;
		HSTRING		m_hstrDialogueDoneTarget;
		HSTRING		m_hstrDialogueDoneMsg;
		HSTRING		m_hstrDialogueStartTarget;
		HSTRING		m_hstrDialogueStartMsg;

		ParticipantList  m_Participants;

		static CinematicTriggerList m_sCinematicTriggers;
};

#endif // __CINEMATIC_TRIGGER_H__
