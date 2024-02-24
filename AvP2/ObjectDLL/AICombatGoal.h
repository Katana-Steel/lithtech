#ifndef __AI_COMBAT_GOAL_H__
#define __AI_COMBAT_GOAL_H__

#include "Goal.h"
#include "Timer.h"
#include "ltsmartlink.h"

class CAI;
class CAISenseSeeEnemy;
class CAISenseHearEnemyFootstep;
class CAISenseHearEnemyWeaponFire;
class CAISenseHearAllyWeaponFire;
class CAISenseHearAllyBattleCry;
class CAISenseHearWeaponImpact;

class AICombatGoal : public Goal
{
public :

	// All the types used by this class.

	struct NewTarget
	{
		enum Reason
		{
			Unknown,
			SeeEnemy,
			Damage,
			HearBattleCry,
			HearAllyWeaponFire,
			OtherSense
		};

		LTSmartLink hTarget;
		LTVector	vPos;
		Reason		eReason;
		
		NewTarget( LTSmartLink hNewTarget = LTSmartLink(), const LTVector & vNewPos = LTVector(0,0,0), Reason eNewReason = Unknown )
			: hTarget(hNewTarget),
			  vPos(vNewPos),
			  eReason(eNewReason) {}
	};


public :

	AICombatGoal(CAI * pAI, std::string sCreateName);
	virtual ~AICombatGoal() { }

	virtual void	Load(HMESSAGEREAD hRead);
	virtual void	Save(HMESSAGEWRITE hWrite);

	virtual void	Update();
	virtual void	End();

protected :


	// Returns new target info if a new target is found.
	NewTarget FindNewTarget();

	LTBOOL  HearFullWeaponImpact() const;
	LTBOOL	HearPartialWeaponImpact() const;

	LTBOOL  AllowWeaponSwitch(LTBOOL val) { return m_bAllowWeaponSwitch = val; }

private :

	const CAISenseSeeEnemy * m_pSeeEnemy;
	const CAISenseHearEnemyFootstep * m_pHearEnemyFootstep;
	const CAISenseHearEnemyWeaponFire * m_pHearEnemyWeaponFire;
	const CAISenseHearAllyBattleCry * m_pHearAllyBattleCry;
	const CAISenseHearAllyWeaponFire * m_pHearAllyWeaponFire;
	const CAISenseHearWeaponImpact * m_pHearWeaponImpact;

	LTBOOL			IsMoreDangerous(HOBJECT hDamager);

	LTSmartLink		m_hTopDamager;
	LTFLOAT			m_fTopDamage;
	LTFLOAT			m_fLastDamageTime;  // Set to the time of the last damage so that we know when we get new damage.
	CTimer			m_tmrTargetDelay;	// prevent switching targets too quickly

	LTBOOL			m_bAllowWeaponSwitch;
	CTimer			m_tmrSwitchWeapon;
	LTFLOAT			m_fSwitchWeaponDelay;
	LTFLOAT			m_fSwitchWeaponChance;

	CTimer			m_tmrAltWeapon;
};

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ AICombatGoal::NewTarget & new_target)
{
	out << new_target.hTarget;
	out << new_target.vPos;
	out.WriteDWord( new_target.eReason );

	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, AICombatGoal::NewTarget & new_target)
{
	in >> new_target.hTarget;
	in >> new_target.vPos;
	new_target.eReason = AICombatGoal::NewTarget::Reason( in.ReadDWord() );

	return in;
}


#endif // __AI_COMBAT_GOAL_H
