// ----------------------------------------------------------------------- //
//
// MODULE  : CAITarget.h
//
// PURPOSE : Target information encapsulation class
//
// CREATED : 2/16/99
//
// ----------------------------------------------------------------------- //

#ifndef __AI_TARGET_H__
#define __AI_TARGET_H__

#include "ltsmartlink.h"
#include "Character.h"
#include "Timer.h"
#include "float.h"

class CAI;
class CAISenseSeeEnemy;

class CAITarget
{
	public : // Public methods

		CAITarget()
			: m_pAI(LTNULL),
			  m_pSeeEnemySense(LTNULL),

			  m_fMinFullSightStimulation(0.0f),
			  m_fMaxFireCheckTime(0.0f),
			  m_fMinAttackDistanceSqr(0.0f),
			  m_fStillAttackingDuration(0.0f),
			  m_fIncreaseLagRate(0.0f),
			  m_fDecreaseLagRate(0.0f),
			  m_fIdleLagRate(0.0f),
			  m_fMaxLag(0.0f),
			  m_fMinLag(0.0f),

			  m_bPartiallyVisible(false),
			  m_bFullyVisible(false),

			  m_vLag(0,0,0),

			  m_vLastPosition(0,0,0),
			  m_vLastVelocity(0,0,0),
			  m_fLastSpottingTime(0.0f),

			  m_fLastFireTime(0.0f),
			  m_vAttackingPoint(0,0,0),
		
			  m_bTouching(false),
			  m_bAimAtHead(false),
			  m_fMaintainTargetDuration(0.0f),
			  m_fMaintainTargetChance(0.0f),
			  m_fMaintainCloakedTargetDuration(0.0f),
			  m_fMaintainCloakedTargetChance(0.0f),
			  m_hTarget(LTNULL) {}

		void Init(const CAI* pAI);
		void GetAttributes(int nTemplateId);

		void Update();

		void HandleTouch(HOBJECT hObj);

		bool  IsFullyVisible() const { return m_bFullyVisible; }
		bool  IsPartiallyVisible() const { return m_bPartiallyVisible; }

		bool  IsMaintained() const { return (m_tmrMaintainTarget.On() == LTTRUE); }

		bool IsAttacking() const { return !m_tmrStillAttacking.Stopped(); }
		const LTVector& GetAttackingPoint() const { return m_vAttackingPoint; }

		const LTSmartLink & GetObject() const { return m_hTarget; }
		const CCharacter* GetCharacterPtr() const { return dynamic_cast<const CCharacter*>(m_hTarget.GetHOBJECT() ? g_pServerDE->HandleToObject(m_hTarget) : LTNULL ); }

		const LTVector& GetPosition() const { return m_vLastPosition; }
		LTVector GetPredictedPosition(LTFLOAT fTimeAhead) const { return m_vLastPosition + m_vLastVelocity*(m_fLastSpottingTime - g_pLTServer->GetTime() + fTimeAhead); }
		
		const LTVector & GetLag() const { return m_vLag; }

		bool  IsTouching() const { return m_bTouching; }

		bool IsValid() const { return m_hTarget.GetHOBJECT() != LTNULL; }

		bool IsFacing() const;

		bool IsAimingAtHead() const { return m_bAimAtHead; }
		void SetAimAtHead(bool val) { m_bAimAtHead = val; }

		LTVector GetTargetOffset() const;

		void SetObject(const LTSmartLink & target, const LTVector & vLastKnownPos = LTVector(FLT_MAX,FLT_MAX,FLT_MAX));

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);


	private : // Private member variables

		const CAI*	m_pAI;
		const CAISenseSeeEnemy * m_pSeeEnemySense;

		LTFLOAT m_fMinFullSightStimulation;   // Target is fully visible if SeeEnemy sense is above this stimulation level.
		LTFLOAT m_fMaxFireCheckTime;          // Consider a shot "recent" if it was less than this time ago.
		LTFLOAT m_fMinAttackDistanceSqr;      // We are being fired at if a shot comes within this distance.
		LTFLOAT m_fStillAttackingDuration;    // The length that a target is considered attacking after they last shot at us.
		LTFLOAT m_fIncreaseLagRate;           // Used for lag, an aiming perturbation.
		LTFLOAT m_fDecreaseLagRate;           // Used for lag, an aiming perturbation.
		LTFLOAT m_fIdleLagRate;				  // Used for lag, an aiming perturbation.
		LTFLOAT m_fMaxLag;                    // The aiming perturbation will never be greater than this.
		LTFLOAT m_fMinLag;                    // The maximum aiming perturbation will never be less than this.

		bool		m_bPartiallyVisible;      // True if target is partially visible (SeeEnemy has non-zero stimulation).
		bool		m_bFullyVisible;          // True if target is fully visible.
		LTSmartLink	m_hTarget;                // Our target.

		LTVector	m_vLag;                   // The current lag (aiming perturbation).

		LTVector	m_vLastPosition;          // Target's last known position.
		LTVector	m_vLastVelocity;          // Target's last known velocity.
		LTFLOAT		m_fLastSpottingTime;      // The time when the target was last fully visible.

		LTFLOAT		m_fLastFireTime;          // The time of the target's most recently processed LastFireInfo.
		CTimer		m_tmrStillAttacking;      // Is active if the target has attacked within the last m_fStillAttackingDuration seconds.
		LTVector	m_vAttackingPoint;        // The point used to determine the shot's distance.

		bool		m_bTouching;              // True if we are touching target.
		bool		m_bAimAtHead;			  // If true, GetTargetOffset will return the target's head, rather then their chest.

		LTFLOAT		m_fMaintainTargetDuration; // Target's position will be known for this long after the target is not fully visible.
		LTFLOAT		m_fMaintainTargetChance;   // Maintain target has this chance of being restarted after the duration is done.
		
		LTFLOAT		m_fMaintainCloakedTargetDuration;
		LTFLOAT		m_fMaintainCloakedTargetChance;

		CTimer		m_tmrMaintainTarget;       // While this is going the target's position is being maintained, even if the target isn't visible.
};

inline ILTMessage & operator<<(ILTMessage & out, /*const*/ CAITarget & target)
{
	target.Save(&out);
	return out;
}

inline ILTMessage & operator>>(ILTMessage & in, CAITarget & target)
{
	target.Load(&in);
	return in;
}

#endif
