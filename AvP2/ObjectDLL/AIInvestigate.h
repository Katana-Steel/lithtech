// ----------------------------------------------------------------------- //
//
// MODULE  : AIInvestigate.h
//
// PURPOSE : Implements the Investigate goal and state.
//
// CREATED : 9/05/00
//
// ----------------------------------------------------------------------- //

#ifndef __INVESTIGATE_H__
#define __INVESTIGATE_H__

#include "fsm.h"
#include "Goal.h"
#include "AIStrategyFollowPath.h"
#include "Timer.h"

class CAISense;
class CAISenseSeeDeadBody;
class CAISenseHearEnemyFootstep;
class CAISenseHearEnemyTaunt;
class CAISenseHearEnemyWeaponFire;
class CAISenseHearAllyWeaponFire;
class CAISenseHearTurretWeaponFire;
class CAISenseHearWeaponImpact;
class CAISenseSeeEnemyFlashlight;
class CAISenseSeeBlood;
class CAISenseSeeCloaked;
class CAISenseHearAllyPain;
class CAISenseHearDeath;

class BodyProp;

struct AIBM_Investigate;

class InvestigateGoal : public Goal
{

public :
	
	enum SuspectType
	{
		Nothing,
		AlienFootstep,
		HumanFootstep,
		PredatorFootstep,
		Flashlight,
		AlienTaunt,
		HumanTaunt,
		PredatorTaunt,
		AlienBody,
		HumanBody,
		PredatorBody,
		EnemyWeaponFire,
		AllyPain,
		DeathCry,
		Cloaked
	};

public :

	InvestigateGoal( CAI * pAI, std::string create_name );

	virtual void Load(HMESSAGEREAD hRead);
	virtual void Save(HMESSAGEWRITE hWrite);

	virtual bool HandleCommand(const char * const * pTokens, int nArgs);
	
	virtual LTFLOAT GetBid();
	virtual LTFLOAT GetMaximumBid();

	virtual void Start();
	virtual void End();

	virtual void Update();

	virtual LTBOOL StopCinematic() const { return LTTRUE; }
	virtual LTBOOL DefaultAlert() const { return  LTTRUE; }

#ifndef _FINAL
	virtual const char * GetDescription() const;
#endif

	virtual CMusicMgr::EMood GetMusicMoodModifier( ) { return CMusicMgr::eMoodWarning; }



private :

	struct Root
	{
		enum State
		{
			eStateSuspicious,
			eStateMoveToStimulus,
			eStateSearch,
			eStateDone
		};

		enum Message
		{
			eStimulus,				// sense activated
			eFinished,
			eFail
		};
	};

private :


	CAISense * GetActiveSensePtr(); 	// Gets a pointer to the sense that has the highest level

	void	SetInvestigateAnimSet(LTBOOL bOn);
	LTBOOL  IsRunToSense(const CAISense * pSense);

	typedef FiniteStateMachine<Root::State,Root::Message> RootFSM;
	void SetupFSM();

	int  GetSenseIndex(CAISense * pSense) const;
	CAISense * GetIndexedSense(int nIndex) const;
	
	void	PrintRootFSMState(const RootFSM & fsm, Root::State state);  // only implemented for debug builds
	void    PrintRootFSMMessage(const RootFSM & fsm, Root::Message message);

	// Called by RootFSM
	void StartSuspicious(RootFSM *);
	void UpdateSuspicious(RootFSM *);

	void StartMoveToStimulus(RootFSM *);
	void UpdateMoveToStimulus(RootFSM *);

	void StartSearch(RootFSM *);
	void UpdateSearch(RootFSM *);

	RootFSM		m_RootFSM;	

	CAIStrategyFollowPath m_StrategyFollowPath;

	CAISenseHearEnemyWeaponFire * m_pHearEnemyWeaponFire;
	CAISenseHearAllyWeaponFire * m_pHearAllyWeaponFire;
	CAISenseHearTurretWeaponFire * m_pHearTurretWeaponFire;
	CAISenseHearAllyPain * m_pHearAllyPain;
	CAISenseHearDeath * m_pHearDeath;
	CAISenseHearWeaponImpact * m_pHearWeaponImpact;
	CAISenseHearEnemyFootstep * m_pHearEnemyFootstep;
	CAISenseHearEnemyTaunt * m_pHearEnemyTaunt;
	CAISenseSeeDeadBody * m_pSeeDeadBody;
	CAISenseSeeEnemyFlashlight * m_pSeeEnemyFlashlight;
	CAISenseSeeBlood * m_pSeeBlood;
	CAISenseSeeCloaked * m_pSeeCloaked;

	const AIBM_Investigate & m_butes;

	CTimer					m_tmrSearch;		// minimum search time
	CTimer					m_tmrSuspicious;	// minimum suspicious time
	CTimer					m_tmrHoldOff;		// Don't investigate while this timer is running.
	CAISense *				m_pSense;			// not saved
	LTVector				m_vDest;			// the location of the stimulus.

	SuspectType				m_eSuspectType;		// set to the race we suspect is out there, used for sounds.
	CTimer					m_tmrDelay;			// used to prevent synchronized AI's.

	LTBOOL					m_bTriggered;       // set to true if the AI received an investigate command.
	LTVector				m_vTriggeredDest; // the location the AI was trigged to investigate.
	BodyProp *				m_pDeadBody;
};


#endif // __INVESTIGATE_H__
