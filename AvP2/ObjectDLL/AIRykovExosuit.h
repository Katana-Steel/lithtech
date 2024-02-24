// ----------------------------------------------------------------------- //
//
// MODULE  : AIRykovExosuit.h
//
// PURPOSE : AIRykovExosuit object
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#ifndef __AIRYKOVEXOSUIT_H__
#define __AIRYKOVEXOSUIT_H__

#include "AI.h"
#include "AIExosuit.h"
#include "CharacterFuncs.h"
#include "fsm.h"
#include "AIStrategyFollowPath.h"
#include "AINodeSnipe.h"

class CAISenseSeeEnemy;
class CAISenseHearEnemyWeaponFire;
class CAISenseHearWeaponImpact;
class CAISenseHearEnemyFootstep;

class AIRykovExosuit : public AIExosuit
{
	public :

		AIRykovExosuit();
		virtual ~AIRykovExosuit();

		virtual void	Update();

		virtual Goal *	CreateGoal(const char * szGoalName);
		virtual bool	CanOpenDoor() const { return false; }

		virtual bool AllowSnapTurn() const { return true; }

	protected :

		virtual DDWORD	EngineMessageFn(DDWORD messageID, void *pData, LTFLOAT lData);

		struct Root
		{
			enum State
			{
				eStateFindNode,
				eStateFindTarget,
				eStateAttack
			};

			enum Message
			{
			};

		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

		virtual void	StartFindNode(RootFSM *);
		virtual void	UpdateFindNode(RootFSM *);
		
		virtual void	StartFindTarget(RootFSM *);
		virtual void	UpdateFindTarget(RootFSM *);

		virtual void	StartAttack(RootFSM *);
		virtual void	UpdateAttack(RootFSM *);

		virtual void	FireOnTarget();


		virtual void	InitialUpdate();
		virtual void	FirstUpdate();

		virtual LTBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);

		virtual void	ProcessDamageMsg(const DamageStruct & damage_msg);
		virtual LTBOOL	ProcessCommand(const char*const* pTokens, int nArgs);

		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

	private :

		void			SetupFSM();

		RootFSM						m_RootFSM;

		CAISenseSeeEnemy *				m_pSeeEnemy;
		CAISenseHearEnemyFootstep *		m_pHearEnemyFootstep;
		CAISenseHearEnemyWeaponFire *	m_pHearEnemyWeaponFire;
		CAISenseHearWeaponImpact *		m_pHearWeaponImpact;

		CAIStrategyFollowPath *		m_pStrategyFollowPath;

		CTimer						m_tmrHoldPos;
		CAINodeSnipe *				m_pCurNode;
		LTSmartLink					m_hAINodeGroup;
		LTBOOL						m_bRockets;

		std::string					m_sHealthMsgTarget;
		std::string					m_sHealthMsg;	// message to send at 50% health
		LTBOOL						m_bHealthMsgSent;
};

#endif // __AIRYKOVEXOSUIT_H__
