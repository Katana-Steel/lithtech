// ----------------------------------------------------------------------- //
//
// MODULE  : STRIKER.h
//
// CREATED : 11.13.2000
// ----------------------------------------------------------------------- //

#ifndef __Striker_H__
#define __Striker_H__

#include "cpp_engineobjects_de.h"
#include "Prop.h"
#include "fsm.h"
#include "Timer.h"
#include "CharacterMovement.h"

#include <list>

LTBOOL StrikerFilterFn(HOBJECT hObject, void *pUserData);

class Striker : public Prop
{
	public :

		typedef std::list<Striker*> StrikerList;

		enum IdleAction
		{
			eIdleAnim = 0,
			eIdleTurn,
			eIdleMove,
			NumIdleActions
		};

	public :

		Striker();
		~Striker();

	protected :

		struct Root
		{
			enum State
			{
				eStateIdle,
				eStateChase,
				eStateAttack,
				eStateDead
			};

			enum Message
			{
				eEnemyDetected,
				eInAttackRange,
				eOutOfAttackRange,
				eNoTarget,
				eDead
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

		LTBOOL  WithinLeash(const LTVector & vPos);

		void	FirstUpdate();
		void	SetupFSM();

		void	StartIdle(RootFSM *);
		void	UpdateIdle(RootFSM *);

		void	StartChase(RootFSM *);
		void	UpdateChase(RootFSM *);

		void	StartAttack(RootFSM *);
		void	UpdateAttack(RootFSM *);

		void	StartDead(RootFSM *);
		void	UpdateDead(RootFSM *);

		void	HandleFootstepKey();
		virtual void HandleModelString(ArgList *);
		void	HandleDeath();
		void	SetRandomSound(const char * szSoundName);


		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();

		LTBOOL		m_bFirstUpdate;

		LTFLOAT		m_fLeashLengthSqr;
		LTVector	m_vLeashFrom;
		std::string m_strLeashFromName; // does not need to be saved.

		IdleAction	m_eIdleAction;
		CTimer		m_tmrIdleAction;
		LTBOOL		m_bIdleTurnRight;

		LTFLOAT		m_fDetectionRadiusSqr;
		LTFLOAT		m_fChaseSpeed;
		LTFLOAT		m_fDamage;
		LTFLOAT		m_fClawRange;
		LTSmartLink	m_hTarget;

		CharacterMovement cm_Movement;

		LTBOOL		m_bLeftFoot;
		int			m_nRandomSoundBute;
		CTimer		m_tmrRandomSound;

		static  StrikerList sm_Strikers;
};

#endif // __STRIKER_H__
