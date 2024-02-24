// ----------------------------------------------------------------------- //
//
// MODULE  : CAISenseMgr.h
//
// PURPOSE : Manage all available senses
//
// CREATED : 9.15.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_SENSE_MGR_H__
#define __AI_SENSE_MGR_H__

#include "AISense.h"
#include "AISenseSee.h"
#include "AISenseHear.h"

class CCharacter;
class Turret;
class CAI;
class LMessage;


class CAISenseMgr
{
	public : // Public methods

		enum Sense
		{
			SeeEnemy			= 0,
			SeeEnemyFlashlight,
			SeeDeadBody,
			SeeBlood,
			SeeCloaked,
			HearEnemyFootstep,
			HearEnemyWeaponFire,
			HearAllyWeaponFire,
			HearTurretWeaponFire,
			HearAllyPain,
			HearAllyBattleCry,
			HearEnemyTaunt,
			HearDeath,
			HearWeaponImpact,
			MaxSenses
		};


		typedef CAISense** SenseIterator;
		typedef const CAISense* const * const_SenseIterator;

	public:

		// Ctors/dtors/etc
		CAISenseMgr();
		void Init(CAI* pAI);

		// Updates
		
		void Update();


		// Simple accessors

		// These are for the SenseMgr.  If it is not enabled, no senses
		// will be updated!
		void SetEnabled(bool val) { m_bEnabled = val; }
		bool IsEnabled() const { return m_bEnabled; }

		void AutoDisableSenses();

		CAISense* GetSensePtr(Sense sense) { return m_apSenses[sense]; }
		const CAISense* GetSensePtr(Sense sense) const { return m_apSenses[sense]; }

		SenseIterator BeginSenses()				 { return &m_apSenses[0]; }
		const_SenseIterator BeginSenses()  const { return &m_apSenses[0]; }

		SenseIterator EndSenses()				{ return m_apSenses + MaxSenses; }
		const_SenseIterator EndSenses()   const { return m_apSenses + MaxSenses; }
		

		void GetProperties(GenericProp* pgp);
		void GetAttributes(int nTemplateID);

		// Listening senses (this is a lousy implementation of the Observer pattern).
		void WeaponFire(const CharFireInfo & info, const CCharacter * pChar);
		void TurretWeaponFire(const CharFireInfo & info, const CCharacter *  pTurretTarget);
		void WeaponImpact(const CharImpactInfo & info, const CCharacter * pChar);
		void Footstep(const CharFootstepSoundInfo & info, const CCharacter * pChar);
		void Pain(const SoundButes & info, const CCharacter * pChar);
		void BattleCry(const SoundButes & info, const CCharacter * pChar);
		void Taunt(const SoundButes & info, const CCharacter * pChar);

		void Death(const SoundButes & info, const BodyProp * pChar);

		void TouchCharacter(const CCharacter & character);

		// Engine methods
		
		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

	protected : // Protected member variables

		CAI*			m_pAI;								// AI backpointer
		
		bool			m_bEnabled;							// Are we enabled?

		CAISense *		m_apSenses[MaxSenses];	// Array of pointers to all the senses
		
		// The senses
		CAISenseSeeEnemy			m_SenseSeeEnemy;
		CAISenseSeeEnemyFlashlight	m_SenseSeeEnemyFlashlight;
		CAISenseSeeDeadBody			m_SenseSeeDeadBody;
		CAISenseSeeBlood			m_SenseSeeBlood;
		CAISenseSeeCloaked			m_SenseSeeCloaked;
		CAISenseHearEnemyFootstep	m_SenseHearEnemyFootstep;
		CAISenseHearEnemyWeaponFire	m_SenseHearEnemyWeaponFire;
		CAISenseHearAllyWeaponFire	m_SenseHearAllyWeaponFire;
		CAISenseHearTurretWeaponFire	m_SenseHearTurretWeaponFire;
		CAISenseHearAllyPain		m_SenseHearAllyPain;
		CAISenseHearAllyBattleCry	m_SenseHearAllyBattleCry;
		CAISenseHearEnemyTaunt		m_SenseHearEnemyTaunt;
		CAISenseHearDeath			m_SenseHearDeath;
		CAISenseHearWeaponImpact	m_SenseHearWeaponImpact;
};

#endif // __AI_SENSE_MGR_H__
