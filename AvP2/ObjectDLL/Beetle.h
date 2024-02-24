// ----------------------------------------------------------------------- //
//
// MODULE  : BEETLE.h
//
// CREATED : 11.13.2000
// ----------------------------------------------------------------------- //

#ifndef __Beetle_H__
#define __Beetle_H__

#include "cpp_engineobjects_de.h"
#include "Prop.h"
#include "fsm.h"
#include "AIStrategySetFacing.h"
#include "Timer.h"
#include "CharacterMovement.h"

class Beetle : public Prop
{
	public :

		Beetle();
		~Beetle();

	protected :

		struct Root
		{
			enum State
			{
				eStateIdle,
				eStateFlap,
				eStateCrawl
			};

			enum Message
			{
				eInRange,
				eDoneMoving
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

		void	StartFlap(RootFSM *);
		void	UpdateFlap(RootFSM *);

		void	StartCrawl(RootFSM *);
		void	UpdateCrawl(RootFSM *);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();

		LTFLOAT		m_fDetectionRadiusSqr;
		LTFLOAT		m_fMinJumpDelay;
		LTFLOAT		m_fMaxJumpDelay;
		LTFLOAT		m_fMinJump;
		LTFLOAT		m_fMaxJump;

		CTimer		m_tmrJumpDelay;
		CharacterMovement cm_Movement;
};

#endif // __BEETLE_H__
