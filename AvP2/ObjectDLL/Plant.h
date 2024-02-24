// ----------------------------------------------------------------------- //
//
// MODULE  : Plant.h
//
// PURPOSE : An alarm object
//
// CREATED : 4/15/99
//
// ----------------------------------------------------------------------- //

#ifndef __PLANT_H__
#define __PLANT_H__

#include "cpp_engineobjects_de.h"
#include "Prop.h"
#include "fsm.h"

class Plant : public Prop
{
	public :

		Plant();
		~Plant();

		void	ReleaseCloud();

	protected :

		struct Root
		{
			enum State
			{
				eStateIdle,
				eStateOpening,
				eStateFiring
			};

			enum Message
			{
				eEnemyInRange,
				eNoEnemy,
				eDoneFiring,
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

		void	HandleModelString(ArgList *);

		void	SetupFSM();

		void	StartIdle(RootFSM *);
		void	UpdateIdle(RootFSM *);

		void	StartOpening(RootFSM *);
		void	UpdateOpening(RootFSM *);

		void	StartFiring(RootFSM *);
		void	UpdateFiring(RootFSM *);


		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();

		LTFLOAT m_fDetectionRadiusSqr;
		LTFLOAT m_fDamageRadius;
		LTFLOAT	m_fMaxDamage;
		uint8	m_nFXId;

		HSTRING m_hstrAttackSound;
		LTFLOAT	m_fSoundRadius;

		std::string m_sPoisonSound;
};

#endif // __PLANT_H__
