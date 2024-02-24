// ----------------------------------------------------------------------- //
//
// MODULE  : AIQueen.h
//
// PURPOSE : AIQueen object
//
// CREATED : 04/09/01
//
// ----------------------------------------------------------------------- //

#ifndef __AIQUEEN_H__
#define __AIQUEEN_H__

#include "AI.h"
#include "fsm.h"
#include "AIStrategyApproachTarget.h"
#include "AIStrategyFollowPath.h"


class CAISenseSeeEnemy;
class CAISenseHearEnemyFootstep;
class CAISenseHearEnemyWeaponFire;
class CAISenseHearWeaponImpact;
class CAINodeAlarm;

class AIQueen : public CAI
{
	public :

		AIQueen();
		virtual ~AIQueen();

		virtual void	Update();

		virtual bool	CanOpenDoor() const { return false; }
		virtual Goal *	CreateGoal(const char * szGoalName);
//		virtual void	UpdateActionAnimation();

	protected :

		virtual CMusicMgr::EMood GetMusicMoodModifier();

		struct Root
		{
			enum State
			{
				eStateWaitForNewTarget,
				eStateApproachTarget,
				eStateGetLineOfSight,
				eStateEngageTarget,
				eStateRunToNode,
				eStateSummon

				// special attack state
				// "roar" state - summon additional AIs
			};

			enum Message
			{
			};

		};

		typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;

		virtual void	StartWaitForNewTarget(RootFSM * rootFSM);
		virtual void	UpdateWaitForNewTarget(RootFSM * rootFSM);
		virtual void	EndWaitForNewTarget(RootFSM * rootFSM);

		virtual void	StartApproachTarget(RootFSM * rootFSM);
		virtual void	UpdateApproachTarget(RootFSM * rootFSM);

		virtual void	StartGetLineOfSight(RootFSM * rootFSM);
		virtual void	UpdateGetLineOfSight(RootFSM * rootFSM);

		virtual void	StartEngageTarget(RootFSM * rootFSM);
		virtual void	UpdateEngageTarget(RootFSM * rootFSM);

		virtual void	StartRunToNode(RootFSM * rootFSM);
		virtual void	UpdateRunToNode(RootFSM * rootFSM);

		virtual void	StartSummon(RootFSM * rootFSM);
		virtual void	UpdateSummon(RootFSM * rootFSM);



		virtual void	InitialUpdate();
		virtual LTBOOL	ReadProp(ObjectCreateStruct *pInfo);
		virtual void	PostPropRead(ObjectCreateStruct *pStruct);
		virtual void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		virtual void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		virtual void	HandleAlienDamage() { return; }		// disable live-gibbing
		virtual LTBOOL	IgnoreDamageMsg(const DamageStruct& damage);

		virtual void	ProcessDamageMsg(const DamageStruct & damage_msg);
		virtual LTBOOL	ProcessCommand(const char*const* pTokens, int nArgs);



	private :

		void SetupFSM();

		RootFSM						m_RootFSM;

		CAIStrategyApproachTarget *	m_pStrategyApproachTarget;
		CAIStrategyFollowPath *		m_pStrategyFollowPath;

		CAISenseSeeEnemy *				m_pSeeEnemy;
		CAISenseHearEnemyFootstep *		m_pHearEnemyFootstep;
		CAISenseHearEnemyWeaponFire *	m_pHearEnemyWeaponFire;
		CAISenseHearWeaponImpact *		m_pHearWeaponImpact;

		CAINodeAlarm *				m_pCurNode;
		int							m_nSummons;		// number of times she called for help
		LTFLOAT						m_fDamageChunk;
		HOBJECT						m_hLastTarget;
		int							m_nRandomSoundBute;
		CTimer						m_tmrRandomSound;
};

#endif // __AIQUEEN_H__
