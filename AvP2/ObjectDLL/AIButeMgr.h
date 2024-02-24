// ----------------------------------------------------------------------- //
//
// MODULE  : AIButeMgr.h
//
// PURPOSE : AIButeMgr definition - Controls attributes of all AIButes
//
// CREATED : 01/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __AIBUTE_MGR_H__
#define __AIBUTE_MGR_H__

#include "HierarchicalButeMgr.h"
#include "CharacterAlignment.h"
#include "AI.h" // for CAI::g_nNumAIWeapons
#include "ClientServerShared.h" // for WMKS_INVALID
#include "GameType.h"
#include <vector>
#include <string>

class CAISenseSeeEnemy;
class CAISenseSeeDeadBody;
class CAISenseSeeBlood;
class CAISenseSeeCloaked;
class CAISenseHearEnemyFootstep;
class CAISenseHearEnemyWeaponFire;
class CAISenseHearAllyWeaponFire;
class CAISenseHearTurretWeaponFire;
class CAISenseHearAllyPain;
class CAISenseHearAllyBattleCry;
class CAISenseHearEnemyTaunt;
class CAISenseHearDeath;
class CAISenseHearWeaponImpact;
class CAISenseSeeEnemyFlashlight;


#ifdef _WIN32
	const char * const g_szAIButeMgrFile = "Attributes\\AIButes.txt";
#else
	const char * const g_szAIButeMgrFile = "Attributes/AIButes.txt";
#endif

class CAIButeMgr;
class CAIWeaponButeMgr;

extern CAIButeMgr* g_pAIButeMgr;
extern CAIWeaponButeMgr* g_pAIWeaponButeMgr;


struct AIWBM_AIWeapon
{
	enum Race
	{
		Unknown,
		Human,
		Exosuit,
		Alien,
		Predator
	};

	std::string	Name;

	Race	eRace;

	std::string WeaponName;
	std::string BarrelName;

	std::string AnimReference;

	std::string strPiece;

	LTBOOL		bCanUseInjured;

	LTFLOAT		fMinIdealRange;
	LTFLOAT		fMaxIdealRange;

	LTFLOAT		fMinFireRange;
	LTFLOAT		fMaxFireRange;

	LTFLOAT		fMaxBurstInterval;
	LTFLOAT		fMinBurstInterval;

	uint32		nMaxBurstShots;
	uint32		nMinBurstShots;

	LTFLOAT		fMaxBurstDelay;
	LTFLOAT		fMinBurstDelay;

	LTFLOAT		fBurstChance;
	int			fHardMaxBurstDelay;

	LTFLOAT		fBurstSpeed;

	LTFLOAT		fAltWeaponChance;
	LTFLOAT		fAltWeaponMinDelay;
	LTFLOAT		fAltWeaponMaxDelay;
	std::string AltWeaponName;

	LTFLOAT		fAimAtHeadChance;
	int			nHideFromDEdit;

	LTBOOL		bWaitForAnimation;
	LTBOOL		bMeleeWeapon;
	LTBOOL		bUseAltFX;

	LTBOOL		bInfiniteAmmo;
	int			nMinStartingAmmo;
	int			nMaxStartingAmmo;

	LTFLOAT		fDamageMultiplier;

	DBYTE		nStartFireSound;
	DBYTE		nLoopFireSound;
	DBYTE		nStopFireSound;

	AIWBM_AIWeapon()
		: eRace(Unknown),
		 
		  // WeaponName
		  // BarrelName

		  // AnimReference

		  // strPiece

		  bCanUseInjured(LTTRUE),

		  fMinIdealRange(0.0f),
		  fMaxIdealRange(0.0f),

		  fMinFireRange(0.0f),
		  fMaxFireRange(0.0f),
		  
		  fMaxBurstInterval(0.0f),
		  fMinBurstInterval(0.0f),

		  nMaxBurstShots(0),
		  nMinBurstShots(0),

		  fMaxBurstDelay(0.0f),
		  fMinBurstDelay(0.0f),

		  fBurstChance(1.0f),
		  fHardMaxBurstDelay(-1),

		  fBurstSpeed(0.0f),
	
		  fAltWeaponChance(0.0f),
		  fAltWeaponMinDelay(0.0f),
		  fAltWeaponMaxDelay(0.0f),
		  // AltWeaponName inits self

		  fAimAtHeadChance(0),
		  nHideFromDEdit(0),
	
		  bWaitForAnimation(LTFALSE),
		  bMeleeWeapon(LTFALSE),
		  bUseAltFX(LTFALSE),

		  bInfiniteAmmo(LTTRUE),
		  nMinStartingAmmo(-1),
		  nMaxStartingAmmo(-1),

		  fDamageMultiplier(1.0f),
	
		  nStartFireSound(WMKS_INVALID),
		  nLoopFireSound(WMKS_INVALID),
		  nStopFireSound(WMKS_INVALID)  {}
};

class CAIWeaponButeMgr : public CHierarchicalButeMgr
{
	public :

		typedef std::vector<std::string> TagList;
		typedef std::vector<AIWBM_AIWeapon> WeaponList;

	public :

		virtual ~CAIWeaponButeMgr();

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile);


		int	GetIDByName(const std::string & szName) const;
		int	GetNumIDs() { return m_TagNames.size(); }

		const AIWBM_AIWeapon & GetAIWeapon(const std::string & name) const
		{
			return GetAIWeapon( GetIDByName(name) );
		}

		const AIWBM_AIWeapon & GetAIWeapon(uint32 iTemplate) const
		{ 
			static const AIWBM_AIWeapon null_value;
			_ASSERT( iTemplate < m_TagNames.size() ); 
			if( iTemplate < m_TagNames.size() )
				return m_aWeapons[iTemplate]; 
			else
				return null_value;
		}

	protected :

		void SetAIWeapon(uint32 iTemplate, const std::string & szTagName);

	private :

		TagList		m_TagNames;
		WeaponList	m_aWeapons;

};


/////////////////////////////////////////////////////////////////////////////
//      CAIButeMgr
/////////////////////////////////////////////////////////////////////////////

struct AIBM_Template
{
	std::string		strName;
	CharacterClass	eCharacterClass;
	std::string		strWeapon[CAI::g_nNumAIWeapons];
	std::string		strWeaponPosition;
	LTFLOAT			fAccuracy;
	LTFLOAT			fLurkRange;
	LTFLOAT			fForwardFOV;
	LTFLOAT			fUpNonFOV;
	LTFLOAT			fSwitchWeaponDelay;
	LTFLOAT			fSwitchWeaponChance;
	LTFLOAT			fSnipeTime;
	LTFLOAT			fLoseTargetTime;
	LTFLOAT			fFearThreshold;
	int				nHideFromDEdit;
	LTFLOAT			fDamagePlayerFactor;
	LTBOOL			bUseDumbAttack;
	LTFLOAT			fInjuredSpeed;

	AIBM_Template()
		: eCharacterClass(UNKNOWN),
		  fAccuracy(0.0f),
		  fLurkRange(0.0f),
		  fForwardFOV(0.0f),
		  fUpNonFOV(0.0f),
		  fSwitchWeaponDelay(0.0f),
		  fSwitchWeaponChance(0.0f),
		  fSnipeTime(0.0f),
		  fLoseTargetTime(5.0f),
		  nHideFromDEdit(0),
		  fDamagePlayerFactor(1.0f),
		  bUseDumbAttack(LTFALSE),
		  fInjuredSpeed(0.0f) {}
};

struct AIBM_Targeting
{
	LTFLOAT fFullSighting;
	LTFLOAT fMaxFiredCheck;
	LTFLOAT fMinAttackingDist;
	LTFLOAT	fTargetDelay;
	LTFLOAT fMaintainDuration;
	LTFLOAT fMaintainChance;
	LTFLOAT fMaintainCloakedDuration;
	LTFLOAT fMaintainCloakedChance;
	LTFLOAT fStillAttackingDuration;
	LTFLOAT fIncreaseLagTime;
	LTFLOAT fDecreaseLagTime;
	LTFLOAT fIdleLagTime;
	LTFLOAT fMaxLag;
	LTFLOAT fMinLag;

	AIBM_Targeting()
		: fFullSighting(0.0f),
		  fMaxFiredCheck(0.0f),
		  fMinAttackingDist(0.0f),
		  fTargetDelay(0.0f),
		  fMaintainDuration(0.0f),
		  fMaintainChance(0.0f),
		  fStillAttackingDuration(0.0f),
		  fIncreaseLagTime(0.0f),
		  fDecreaseLagTime(0.0f),
		  fIdleLagTime(0.0f),
		  fMaxLag(0.0f),
		  fMinLag(0.0f) { }
};

struct AIBM_Attack
{
	LTFLOAT fDodgeProbability;
	LTFLOAT fMinDodgeDelay;
	LTFLOAT fMaxDodgeDelay;
	LTFLOAT fMinDodgeDist;
	LTFLOAT fMaxDodgeDist;

	LTFLOAT fDodgeImpactDist;

	LTFLOAT fBackoffProbability;
	LTFLOAT fMinBackoffDelay;
	LTFLOAT fMaxBackoffDelay;
	LTFLOAT fMinBackoffDist;
	LTFLOAT fMaxBackoffDist;

	LTFLOAT	fRunForCoverChance;
	LTFLOAT	fMinRunForCoverDelay;
	LTFLOAT	fMaxRunForCoverDelay;

	LTFLOAT	fMinCoverDuration;
	LTFLOAT	fMaxCoverDuration;

	LTFLOAT fMinStuckFireDuration;
	LTFLOAT fMaxStuckFireDuration;

	LTFLOAT	fCrouchChance;
	LTFLOAT	fMinCrouchDelay;
	LTFLOAT	fMaxCrouchDelay;

	std::string strMeleeWeapon;
	LTFLOAT fEnterMeleeDist;
	LTFLOAT fExitMeleeDist;

	AIBM_Attack()
		: fDodgeProbability(0.0f),
		  fMinDodgeDelay(0.0f),
		  fMaxDodgeDelay(0.0f),
		  fMinDodgeDist(0.0f),
		  fMaxDodgeDist(0.0f),

		  fDodgeImpactDist(0.0f),

		  fBackoffProbability(0.0f),
		  fMinBackoffDelay(0.0f),
		  fMaxBackoffDelay(0.0f),
		  fMinBackoffDist(0.0f),
		  fMaxBackoffDist(0.0f),
	
		  fRunForCoverChance(0.0f),
		  fMinRunForCoverDelay(0.0f),
		  fMaxRunForCoverDelay(0.0f),

		  fMinCoverDuration(0.0f),
		  fMaxCoverDuration(0.0f),

		  fMinStuckFireDuration(0.0f),
		  fMaxStuckFireDuration(0.0f),

		  fCrouchChance(0.0f),
		  fMinCrouchDelay(0.0f),
		  fMaxCrouchDelay(0.0f),
	
		  // strMeleeWeapon
		  fEnterMeleeDist(0.0f),
		  fExitMeleeDist(0.0f) {}
};

struct AIBM_Patrol
{
	LTFLOAT fMinStuckDelay;
	LTFLOAT fMaxStuckDelay;

	AIBM_Patrol()
		: fMinStuckDelay(0.0f),
		  fMaxStuckDelay(0.0f) {}
};

struct AIBM_Investigate
{
	LTFLOAT fMinAnimDelay;
	LTFLOAT fMaxAnimDelay;

	LTFLOAT fMinSuspiciousDur;
	LTFLOAT fMaxSuspiciousDur;

	LTFLOAT fMinSearchDur;
	LTFLOAT fMaxSearchDur;

	LTFLOAT	fMinHoldOffDelay;
	LTFLOAT fMaxHoldOffDelay;

	AIBM_Investigate()
		: fMinAnimDelay(0),
		  fMaxAnimDelay(0), 
		  fMinSuspiciousDur(0),
		  fMaxSuspiciousDur(0),
		  fMinSearchDur(0),
		  fMaxSearchDur(0),
		  fMinHoldOffDelay(0),
		  fMaxHoldOffDelay(0) {}
};

struct AIBM_Cower
{
	LTFLOAT fMinCowerDuration;
	LTFLOAT fMaxCowerDuration;
	
	LTFLOAT fRunDistSqr;
	LTFLOAT fRunChance;
	LTFLOAT fMinSafeDuration;
	LTFLOAT fMaxSafeDuration;
	
	LTFLOAT	fSafeDistSqr;
	LTFLOAT fNotifyDistSqr;

	LTFLOAT fNotifyDelay;

	AIBM_Cower()
		: fMinCowerDuration(0.0f),
		  fMaxCowerDuration(0.0f),
		  fRunDistSqr(0.0f),
		  fRunChance(0.0f),
		  fMinSafeDuration(0.0f),
		  fMaxSafeDuration(0.0f),
		  fSafeDistSqr(0.0f),
		  fNotifyDistSqr(0.0f),
		  fNotifyDelay(0.0f) {}
};

struct AIBM_Movement
{
	LTFLOAT fMinCorrectionTime;
	LTFLOAT	fMaxCorrectionTime;
	LTFLOAT fCosMaxSnapTurnAngle;
	LTFLOAT fMaxStuckDuration;
	LTFLOAT fAvoidancePathFrequency;

	AIBM_Movement()
		: fMinCorrectionTime(0.0f),
		  fMaxCorrectionTime(0.0f),
		  fCosMaxSnapTurnAngle(1.0f),
		  fMaxStuckDuration(0.0f),
		  fAvoidancePathFrequency(0.0f) {}
};

struct AIBM_ApproachTarget
{
	LTFLOAT fMinRepath;
	LTFLOAT fMaxRepath;
	
	LTFLOAT	fMinFireTime;
	LTFLOAT	fMaxFireTime;

	LTFLOAT	fFireChance;

	AIBM_ApproachTarget()
		: fMinRepath(0.0f),
		  fMaxRepath(0.0f),
		  fMinFireTime(0.0f),
		  fMaxFireTime(0.0f),
		  fFireChance(0.0f) {}
};

struct AIBM_AlienAttack
{

	LTFLOAT fPounceChance;
	LTFLOAT fMinPounceDelay;
	LTFLOAT fMaxPounceDelay;
	LTFLOAT fPounceMinRangeSqr;
	LTFLOAT fPounceMaxRangeSqr;

	LTFLOAT fFacehugChance;
	LTFLOAT fMinFacehugDelay;
	LTFLOAT fMaxFacehugDelay;
	LTFLOAT fFacehugMinRangeSqr;
	LTFLOAT fFacehugMaxRangeSqr;

	AIBM_AlienAttack()
		: fPounceChance(0.0f),
		  fMinPounceDelay(0.0f),
		  fMaxPounceDelay(0.0f),
		  fPounceMinRangeSqr(0.0f),
		  fPounceMaxRangeSqr(0.0f),
		  fFacehugChance(0.0f),
		  fMinFacehugDelay(0.0f),
		  fMaxFacehugDelay(0.0f),
		  fFacehugMinRangeSqr(0.0f),
		  fFacehugMaxRangeSqr(0.0f) {}
};

struct AIBM_Influence
{
	LTFLOAT fUpdateTime;
	LTFLOAT fCharacterRange;
	int		nOddsLimit;
	LTFLOAT fSelfDamage;
	LTFLOAT fFriendDamage;
	LTFLOAT fFearDecay;
	LTFLOAT	fMinDamageFearDuration;
	LTFLOAT fMaxDamageFearDuration;

	AIBM_Influence()
		: fUpdateTime(0.0f),
		  fCharacterRange(0.0f),
		  nOddsLimit(-1),
		  fSelfDamage(0.0f),
		  fFriendDamage(0.0f),
		  fFearDecay(0.05f),
		  fMinDamageFearDuration(0),
		  fMaxDamageFearDuration(0)  {}
};

struct AIBM_Sense
{
	LTFLOAT fRange;
	LTFLOAT fIncreaseStimulationTime;
	LTFLOAT fDecreaseStimulationTime;
	LTFLOAT fPartialLevel;
	LTFLOAT fAlertRateModifier;

	AIBM_Sense()
		: fRange(0),
		  fIncreaseStimulationTime(0),
		  fDecreaseStimulationTime(0),
		  fPartialLevel(0),
		  fAlertRateModifier(0) {}

	virtual ~AIBM_Sense() {}
};

struct AIBM_SeeSense : public AIBM_Sense
{
	LTFLOAT fShortTimeDistance;
	LTFLOAT fLongTimeDistance;
	LTFLOAT fLongIncreaseStimulationTime;
	LTFLOAT fLightMin;
	LTFLOAT fLightMax;

	AIBM_SeeSense()
		: fShortTimeDistance(0.0f),
		  fLongTimeDistance(0.0f),
		  fLongIncreaseStimulationTime(0.0f),
		  fLightMin(0.0f),
		  fLightMax(0.0f) {}
};

struct AIBM_SeeEnemySense : public AIBM_SeeSense
{
	LTFLOAT		fCloakSensitivity;
	LTFLOAT		fCloakFalloffMinDist;
	LTFLOAT		fCloakFalloffMaxDist;
	LTFLOAT		fCloakMinAlphaAffect;
	LTFLOAT		fCloakMaxAlphaAffect;

	LTFLOAT		fFOV;
	LTFLOAT		fVerticalBlind;

	LTFLOAT		fStartFOV;
	LTFLOAT		fStartVerticalBlind;

	AIBM_SeeEnemySense()
		: fCloakSensitivity(0.0f),
		  fCloakFalloffMinDist(0.0f),
		  fCloakFalloffMaxDist(0.0f),
		  fCloakMinAlphaAffect(0.0f),
		  fCloakMaxAlphaAffect(0.0f),
		  fFOV(0.0f),
		  fVerticalBlind(0.0f),
		  fStartFOV(0.0f),
		  fStartVerticalBlind(0.0f) {}
};

struct AIBM_SeeDeadBodySense : public AIBM_SeeSense
{
};

struct AIBM_SeeBloodSense : public AIBM_SeeSense
{
};

struct AIBM_SeeCloakedSense : public AIBM_SeeSense
{
};

struct AIBM_HearSense : public AIBM_Sense
{
	LTFLOAT fMaxListeningTime;
	LTFLOAT fMaxNoiseDistance;

	AIBM_HearSense()
		: fMaxListeningTime(0),
		  fMaxNoiseDistance(0) {}

	virtual ~AIBM_HearSense() {}
};

struct AIBM_HearEnemyFootstepSense : public AIBM_HearSense
{
};

struct AIBM_HearEnemyWeaponFireSense : public AIBM_HearSense
{
};

struct AIBM_HearAllyWeaponFireSense : public AIBM_HearSense
{
};

struct AIBM_HearTurretWeaponFireSense : public AIBM_HearSense
{
};

struct AIBM_HearAllyPainSense : public AIBM_HearSense
{
};

struct AIBM_HearAllyBattleCrySense : public AIBM_HearSense
{
};

struct AIBM_HearEnemyTauntSense : public AIBM_HearSense
{
};

struct AIBM_HearDeathSense : public AIBM_HearSense
{
};

struct AIBM_HearWeaponImpactSense : public AIBM_HearSense
{
};

struct AIBM_SeeEnemyFlashlightSense : public AIBM_SeeSense
{
};

struct AIBM_AllSenses
{
	AIBM_SeeEnemySense				m_SeeEnemy;
	AIBM_SeeDeadBodySense			m_SeeDeadBody;
	AIBM_SeeBloodSense				m_SeeBlood;
	AIBM_SeeCloakedSense			m_SeeCloaked;
	AIBM_HearEnemyFootstepSense		m_HearEnemyFootstep;
	AIBM_HearEnemyWeaponFireSense	m_HearEnemyWeaponFire;
	AIBM_HearAllyWeaponFireSense	m_HearAllyWeaponFire;
	AIBM_HearTurretWeaponFireSense	m_HearTurretWeaponFire;
	AIBM_HearAllyPainSense			m_HearAllyPain;
	AIBM_HearAllyBattleCrySense		m_HearAllyBattleCry;
	AIBM_HearEnemyTauntSense		m_HearEnemyTaunt;
	AIBM_HearDeathSense				m_HearDeath;
	AIBM_HearWeaponImpactSense		m_HearWeaponImpact;
	AIBM_SeeEnemyFlashlightSense	m_SeeEnemyFlashlight;
};

// If you add a new bid, you also need to update the strings listed in
// const char * szBidName[NUM_BIDS] at the top of AIButeMgr.cpp
enum DefaultBids
{
	BID_SCRIPT,
	BID_NODECHECK,
	BID_ALARM,
	BID_SNIPE,
	BID_LURK,
	BID_ATTACKSEEENEMY,
	BID_RETREAT,
	BID_INVESTIGATE,
	BID_COWER,
	BID_ATTACKOTHER,
	BID_IDLESCRIPT,
	BID_PATROL,
	BID_IDLE,
	NUM_BIDS
};

class CAIButeMgr : public CHierarchicalButeMgr
{
	public :

		typedef std::vector<std::string> TagList;

	public :



		virtual ~CAIButeMgr() { Term(); }

		LTBOOL		Init(ILTCSBase *pInterface, const char* szAttributeFile = g_szAIButeMgrFile);
		void Term() { g_pAIButeMgr = LTNULL; }

		const CAIWeaponButeMgr & GetAIWeaponButes() const { return m_AIWeaponButeMgr; }

		int	GetTemplateIDByName(const std::string & szName) const;
		int	GetNumIDs() { return m_TagNames.size(); }

		std::string GetNameByID(uint32 iTemplate) const
		{
			_ASSERT( iTemplate < m_TagNames.size() ); 
			if( iTemplate < m_TagNames.size() )
				return m_TagNames[iTemplate]; 
			else
				return std::string();
		}

		const LTFLOAT GetDefaultBid(DefaultBids eBidName)
		{
			return m_aDefaultBids[eBidName];
		}

		const LTFLOAT GetDifficultyFactor(int nDifficulty)
		{
			ASSERT( nDifficulty >= 0 && nDifficulty <= DIFFICULTY_PC_INSANE );

			return m_aDifficultyFactor[nDifficulty];
		}

		const AIBM_Template & GetTemplate(uint32 iTemplate) const
		{ 
			static const AIBM_Template null_value;
			_ASSERT( iTemplate < m_aTemplates.size() ); 
			if( iTemplate < m_aTemplates.size() )
				return m_aTemplates[iTemplate]; 
			else
				return null_value;
		}

		// Butes

		const AIBM_SeeEnemySense & GetSense(uint32 iTemplate, CAISenseSeeEnemy * ) const 
		{ 
			static const AIBM_SeeEnemySense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_SeeEnemy; 
			else
				return null_value;
		}

		const AIBM_SeeDeadBodySense & GetSense(uint32 iTemplate, CAISenseSeeDeadBody *) const
		{
			static const AIBM_SeeDeadBodySense null_value;
			_ASSERT(iTemplate < m_aSenses.size());
			if (iTemplate < m_aSenses.size())
				return m_aSenses[iTemplate].m_SeeDeadBody;
			else
				return null_value;
		}

		const AIBM_SeeBloodSense & GetSense(uint32 iTemplate, CAISenseSeeBlood *) const
		{
			static const AIBM_SeeBloodSense null_value;
			_ASSERT(iTemplate < m_aSenses.size());
			if (iTemplate < m_aSenses.size())
				return m_aSenses[iTemplate].m_SeeBlood;
			else
				return null_value;
		}

		const AIBM_SeeCloakedSense & GetSense(uint32 iTemplate, CAISenseSeeCloaked *) const
		{
			static const AIBM_SeeCloakedSense null_value;
			_ASSERT(iTemplate < m_aSenses.size());
			if (iTemplate < m_aSenses.size())
				return m_aSenses[iTemplate].m_SeeCloaked;
			else
				return null_value;
		}

		const AIBM_HearEnemyFootstepSense & GetSense(uint32 iTemplate, CAISenseHearEnemyFootstep * ) const
		{ 
			static const AIBM_HearEnemyFootstepSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearEnemyFootstep; 
			else
				return null_value;
		}

		const AIBM_HearEnemyWeaponFireSense & GetSense(uint32 iTemplate, CAISenseHearEnemyWeaponFire * ) const 
		{ 
			static const AIBM_HearEnemyWeaponFireSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearEnemyWeaponFire; 
			else
				return null_value;
		}

		const AIBM_HearAllyWeaponFireSense & GetSense(uint32 iTemplate, CAISenseHearAllyWeaponFire * ) const 
		{ 
			static const AIBM_HearAllyWeaponFireSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearAllyWeaponFire; 
			else
				return null_value;
		}

		const AIBM_HearTurretWeaponFireSense & GetSense(uint32 iTemplate, CAISenseHearTurretWeaponFire * ) const 
		{ 
			static const AIBM_HearTurretWeaponFireSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearTurretWeaponFire; 
			else
				return null_value;
		}

		const AIBM_HearAllyPainSense & GetSense(uint32 iTemplate, CAISenseHearAllyPain * ) const 
		{ 
			static const AIBM_HearAllyPainSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearAllyPain; 
			else
				return null_value;
		}

		const AIBM_HearAllyBattleCrySense & GetSense(uint32 iTemplate, CAISenseHearAllyBattleCry * ) const 
		{ 
			static const AIBM_HearAllyBattleCrySense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearAllyBattleCry; 
			else
				return null_value;
		}

		const AIBM_HearEnemyTauntSense & GetSense(uint32 iTemplate, CAISenseHearEnemyTaunt * ) const 
		{ 
			static const AIBM_HearEnemyTauntSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearEnemyTaunt; 
			else
				return null_value;
		}

		const AIBM_HearDeathSense & GetSense(uint32 iTemplate, CAISenseHearDeath * ) const 
		{ 
			static const AIBM_HearDeathSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearDeath; 
			else
				return null_value;
		}

		const AIBM_HearWeaponImpactSense & GetSense(uint32 iTemplate, CAISenseHearWeaponImpact * ) const 
		{ 
			static const AIBM_HearWeaponImpactSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_HearWeaponImpact; 
			else
				return null_value;
		}

		const AIBM_SeeEnemyFlashlightSense & GetSense(uint32 iTemplate, CAISenseSeeEnemyFlashlight * ) const 
		{ 
			static const AIBM_SeeEnemyFlashlightSense null_value;
			_ASSERT( iTemplate < m_aSenses.size() ); 
			if( iTemplate < m_aSenses.size() )
				return m_aSenses[iTemplate].m_SeeEnemyFlashlight; 
			else
				return null_value;
		}

		const AIBM_Targeting & GetTargeting(uint32 iTemplate) const
		{ 
			static const AIBM_Targeting null_value;
			_ASSERT( iTemplate < m_aTargeting.size() ); 
			if( iTemplate < m_aTargeting.size() )
				return m_aTargeting[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_AlienAttack & GetAlienAttack(uint32 iTemplate) const
		{
			static const AIBM_AlienAttack null_value;
			_ASSERT( iTemplate < m_aAlienAttack.size() ); 
			if( iTemplate < m_aAlienAttack.size() )
				return m_aAlienAttack[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_Attack & GetAttack(uint32 iTemplate) const
		{
			static const AIBM_Attack null_value;
			_ASSERT( iTemplate < m_Attack.size() ); 
			if( iTemplate < m_Attack.size() )
				return m_Attack[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_Patrol & GetPatrol(uint32 iTemplate) const
		{
			static const AIBM_Patrol null_value;
			_ASSERT( iTemplate < m_aPatrol.size() ); 
			if( iTemplate < m_aPatrol.size() )
				return m_aPatrol[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_Investigate & GetInvestigate(uint32 iTemplate) const
		{
			static const AIBM_Investigate null_value;
			_ASSERT( iTemplate < m_aInvestigate.size() ); 
			if( iTemplate < m_aInvestigate.size() )
				return m_aInvestigate[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_Cower & GetCower(uint32 iTemplate) const
		{
			static const AIBM_Cower null_value;
			_ASSERT( iTemplate < m_aCower.size() ); 
			if( iTemplate < m_aCower.size() )
				return m_aCower[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_Movement & GetMovement(uint32 iTemplate) const
		{
			static const AIBM_Movement null_value;
			_ASSERT( iTemplate < m_aMovement.size() ); 
			if( iTemplate < m_aMovement.size() )
				return m_aMovement[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_ApproachTarget & GetApproachTarget(uint32 iTemplate) const
		{
			static const AIBM_ApproachTarget null_value;
			_ASSERT( iTemplate < m_aApproachTarget.size() ); 
			if( iTemplate < m_aApproachTarget.size() )
				return m_aApproachTarget[iTemplate]; 
			else
				return null_value;
		}

		const AIBM_Influence & GetInfluence(uint32 iTemplate) const
		{
			static const AIBM_Influence null_value;
			_ASSERT( iTemplate < m_aInfluence.size() ); 
			if( iTemplate < m_aInfluence.size() )
				return m_aInfluence[iTemplate]; 
			else
				return null_value;
		}

	protected :

		void SetTemplate(uint32 iTemplate, const std::string & szTagName);
		void SetSenses(uint32 iTemplate, const std::string & szTagName);
		void SetTargeting(uint32 iTemplate, const std::string & szTagName);
		void SetAlienAttack(uint32 iTemplate, const std::string & szTagName);
		void SetAttack(uint32 iTemplate, const std::string & szTagName);
		void SetPatrol(uint32 iTemplate, const std::string & szTagName);
		void SetInvestigate(uint32 iTemplate, const std::string & szTagName);
		void SetCower(uint32 iTemplate, const std::string & szTagName);
		void SetMovement(uint32 iTemplate, const std::string & szTagName);
		void SetApproachTarget(uint32 iTemplate, const std::string & szTagName);
		void SetInfluence(uint32 iTemplate, const std::string & szTagName);

		void SetSeeEnemySense(uint32 iTemplate, const std::string & szTagName);
		void SetSeeDeadBodySense(uint32 iTemplate, const std::string & szTagName);
		void SetSeeBloodSense(uint32 iTemplate, const std::string & szTagName);
		void SetSeeCloakedSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearEnemyFootstepSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearEnemyWeaponFireSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearAllyWeaponFireSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearTurretWeaponFireSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearAllyPainSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearAllyBattleCrySense(uint32 iTemplate, const std::string & szTagName);
		void SetHearEnemyTauntSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearDeathSense(uint32 iTemplate, const std::string & szTagName);
		void SetHearWeaponImpactSense(uint32 iTemplate, const std::string & szTagName);
		void SetSeeEnemyFlashlightSense(uint32 iTemplate, const std::string & szTagName);

	private :

		CAIWeaponButeMgr		m_AIWeaponButeMgr;

		TagList					m_TagNames;

		std::vector<AIBM_Template>			m_aTemplates;
		std::vector<AIBM_AllSenses>			m_aSenses;
		std::vector<AIBM_Targeting>			m_aTargeting;
		std::vector<AIBM_AlienAttack>		m_aAlienAttack;
		std::vector<AIBM_Attack>			m_Attack;
		std::vector<AIBM_Patrol>			m_aPatrol;
		std::vector<AIBM_Investigate>		m_aInvestigate;
		std::vector<AIBM_Cower>				m_aCower;
		std::vector<AIBM_Movement>			m_aMovement;
		std::vector<AIBM_ApproachTarget>	m_aApproachTarget;
		std::vector<AIBM_Influence>			m_aInfluence;

		LTFLOAT					m_aDefaultBids[NUM_BIDS];
		LTFLOAT					m_aDifficultyFactor[DIFFICULTY_PC_INSANE+1];
};




#endif // __AIBUTE_MGR_H__

