// ----------------------------------------------------------------------- //
//
// MODULE  : Egg.h
//
// PURPOSE : An alarm object
//
// CREATED : 4/15/99
//
// ----------------------------------------------------------------------- //

#ifndef __EGG_H__
#define __EGG_H__

#include "cpp_engineobjects_de.h"
#include "Prop.h"
#include "fsm.h"
#include "Timer.h"

class Egg : public Prop
{
	public :

		Egg();
		~Egg();

	protected :

		// Returns LTTRUE if it has completely handled the message and should not call up
		virtual LTBOOL HandleTrigger(HOBJECT hSender, HSTRING hMsg);

		struct Root
		{
			enum State
			{
				eStateIdle,
				eStateWarning,
				eStateOpening,
				eStateDone
			};

			enum Message
			{
				eEnemyInRange,
				eEnemyOutOfRange,
				eDoneOpening
			};
		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		RootFSM	m_RootFSM;	


		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);
		virtual DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		LTBOOL	InitialUpdate();
		virtual void Update();

		LTBOOL	ReadProp(ObjectCreateStruct *pData);
		LTBOOL	Setup(ObjectCreateStruct *pData );
		void	PostPropRead(ObjectCreateStruct* pData);

		LTBOOL	m_bPlayerUsable;
		HSTRING	m_hstrActivateMessage;
		HSTRING	m_hstrActivateTarget;

	private :

		void	SetupFSM();

		void	StartIdle(RootFSM *);
		void	UpdateIdle(RootFSM *);

		void	StartWarning(RootFSM *);
		void	UpdateWarning(RootFSM *);

		void	StartOpening(RootFSM *);
		void	UpdateOpening(RootFSM *);

		void	StartDone(RootFSM *);
		void	UpdateDone(RootFSM *);

		
		LTBOOL	SpawnFacehugger();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();

		LTBOOL		m_bUseSimpleAI;

		LTFLOAT		m_fDetectionRadiusSqr;
		LTFLOAT		m_fWarningTime;
		CTimer		m_tmrWarning;

		LTFLOAT		m_fSoundRadius;

		LTVector	m_vTargetPos;
		LTSmartLink m_hTarget;

		std::string m_strOpenSound;
		std::string m_strWarningSound;
		
		HLTSOUND	m_hWarningSound;
};

#endif // __EGG_H__
