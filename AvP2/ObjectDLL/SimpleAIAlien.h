// ----------------------------------------------------------------------- //
//
// MODULE  : SimpleAIAlien.h
//
// PURPOSE : Low tick count Alien object
//
// CREATED : 03/06/01
//
// ----------------------------------------------------------------------- //

#ifndef __SIMPLE_AIALIEN_H__
#define __SIMPLE_AIALIEN_H__

#include "SimpleAI.h"
#include "CharacterAlignment.h"
#include "fsm.h"
#include "Timer.h"

struct AIBM_AlienAttack;
class  CSimpleNode;

class SimpleAIAlien : public CSimpleAI
{
	public :

		typedef std::deque<const CSimpleNode *> Path;

	public:

		SimpleAIAlien();
		virtual ~SimpleAIAlien();
		virtual CharacterClass  GetCharacterClass() const  { return ALIEN; }

		virtual void HandleInjured();

		virtual void HandleStunEffect();
		virtual void UpdateStunEffect();

		virtual void	HandleEMPEffect();
		virtual void	UpdateEMPEffect();

		virtual void	UpdateActionAnimation();

	protected:

		virtual uint32	EngineMessageFn(uint32 dwID, void *pData, LTFLOAT fData);
		virtual LTBOOL	ReadProp(ObjectCreateStruct *pStruct);
		virtual LTBOOL	ProcessCommand(const char* const* pTokens, int nArgs);

		virtual LTFLOAT GetSimpleNodeCheckDelay() { if( !m_bUsingPath ) return CCharacter::GetSimpleNodeCheckDelay(); return 0.0f; }

		struct Root
		{
			enum State
			{
				eStateIdle,
				eStateChase,
				eStateAttack
			};

			enum Message
			{
				eEnemyDetected,
				eInAttackRange,
				eOutOfAttackRange,
				eNoTarget
			};
		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
		RootFSM	m_RootFSM;	

		void			SetupFSM();

		virtual void	InitialUpdate();
		virtual void	Update();
		
		virtual LTBOOL	IgnoreDamageMsg(const DamageStruct & damage_msg);
		LTBOOL			IsTargetVertical();		// is target aligned to our up vector?

		LTBOOL			IsStuck();

		virtual void	Save(HMESSAGEWRITE hWrite);
		virtual void	Load(HMESSAGEREAD hRead);
//		virtual void	PostPropRead(ObjectCreateStruct *pStruct);


		std::string		m_strIdleAnim;
		std::string		m_strChaseAnim;
		std::string		m_strAttackAnim;

		const AIBM_AlienAttack * m_pAttackButes; // needn't be saved.

	private:


		virtual void	StartIdle(RootFSM *);
		virtual void	UpdateIdle(RootFSM *);
		virtual void	EndIdle(RootFSM *);

		virtual void	StartChase(RootFSM *);
		virtual void	UpdateChase(RootFSM *);

		virtual void	StartAttack(RootFSM *);
		virtual void	UpdateAttack(RootFSM *);

		int				m_nRandomSoundBute;		// don't save
		CTimer			m_tmrRandomSound;		// don't save
		LTBOOL			m_bRemoveOnDeactivate;

		CTimer			m_tmrStuck;				// don't save
		LTVector		m_vLastRecordedPos;
		LTBOOL			m_bCheckVert;			// check along our up vector for "stuck" status?

		Path			m_Path;
		LTBOOL			m_bUsingPath;
		const CSimpleNode * m_pTargetsLastNode;
};


class SimpleAIFacehugger : public SimpleAIAlien
{
	public:

		SimpleAIFacehugger();
		virtual void	Update();

		virtual void	Save(HMESSAGEWRITE hWrite);
		virtual void	Load(HMESSAGEREAD hRead);

	private:

		virtual void	StartAttack(RootFSM *);
		virtual void	UpdateAttack(RootFSM *);

		CTimer			m_tmrFacehugDelay;
};

#endif // __AIALIEN_H__
