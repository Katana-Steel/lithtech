// ----------------------------------------------------------------------- //
//
// MODULE  : CAISenseHear.h
//
// PURPOSE : Hearing based sense class
//
// CREATED : 9.15.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_SENSE_HEAR_H__
#define __AI_SENSE_HEAR_H__

#include "AIUtils.h"
#include "AISense.h"

class CCharacter;
class CAI;
class BodyProp;
struct AIBM_Sense;
struct AIBM_HearSense;
class LMessage;

struct CharFootstepSoundInfo;
struct CharFireInfo;
struct CharImpactInfo;
struct SoundButes;

class CAISenseHear : public CAISense
{
	public :

		struct SoundProps
		{
			// See CAISenseHear::StimulateInstance for an explanation
			// of this class.
			LTFLOAT  fVolume;    
			LTVector vLocation;

			SoundProps()
				: fVolume(1.0f),
				  vLocation(0,0,0) {}
		};


	public :

		CAISenseHear()
			: m_fLastCheckTime(0),
			  m_fMaxNoiseDistanceSqr(0) {}

		virtual ~CAISenseHear() {}

		virtual const char* GetDescription() const = 0;
		virtual const char* GetPropertiesEnabledString() const { return "CanHear"; }
		virtual const char* GetPropertiesDistanceString() const { return "HearDistance"; }

		virtual void UpdateStimulation(CAISenseInstance * instance) {} // All the action occurs in StimulateInstance below.

		virtual void StimulateInstance(const SoundProps & info, CAISenseInstance * pInstance);

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);


	protected :

		void SetAttributes(const AIBM_HearSense & pSenseData);

	private :

		LTFLOAT m_fLastCheckTime;
		LTFLOAT m_fMaxNoiseDistanceSqr;

};


class CAISenseHearEnemyFootstep : public CAISenseHear 
{

	public :

		void Footstep(const CharFootstepSoundInfo & info, const CCharacter * pChar);

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearEnemyFootstep"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearEnemyFootstep"; }
		virtual const char* GetPropertiesDistanceString() const { return "HearEnemyFootstepDistance"; }
};


class CAISenseHearWeaponFire : public CAISenseHear 
{

	public :

		void WeaponFire(const CharFireInfo & info, const CCharacter * pChar);

		// This is a base class for CAISenseHear[Enemy,Ally]WeaponFire, 
		// so GetDescription is not defined.
		virtual const char* GetPropertiesDistanceString() const { return "WeaponFireDistance"; }
};

class CAISenseHearEnemyWeaponFire : public CAISenseHearWeaponFire 
{

	public :

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearEnemyWeaponFire"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearEnemyWeaponFire"; }
};

class CAISenseHearAllyWeaponFire : public CAISenseHearWeaponFire 
{

	public :

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearAllyWeaponFire"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearAllyWeaponFire"; }
};

class CAISenseHearTurretWeaponFire : public CAISenseHearWeaponFire 
{

	public :

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearTurretWeaponFire"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearTurretWeaponFire"; }
};

class CAISenseHearAllyPain : public CAISenseHear
{

	public :

		void Pain(const SoundButes & info, const CCharacter * pChar);

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearAllyPain"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearAllyPain"; }
};

class CAISenseHearAllyBattleCry : public CAISenseHear
{

	public :

		void BattleCry(const SoundButes & info, const CCharacter * pChar);

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearAllyBattleCry"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearAllyBattleCry"; }
};

class CAISenseHearEnemyTaunt : public CAISenseHear
{

	public :

		void BattleCry(const SoundButes & info, const CCharacter * pChar);

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearEnemyTaunt"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearEnemyTaunt"; }
};

class CAISenseHearDeath : public CAISenseHear
{

	public :

		void Death(const SoundButes & info, const BodyProp * pBody);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearDeath"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearDeath"; }

	protected :

		virtual void UpdateAllInstances();
};


class CAISenseHearWeaponImpact : public CAISenseHear
{
	public:

		void WeaponImpact(const CharImpactInfo & info, const CCharacter * pChar);

		virtual bool IgnoreCharacter(const CCharacter * pChar);
		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearWeaponImpact"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearWeaponImpact"; }
		virtual const char* GetPropertiesDistanceString() const { return "WeaponFireDistance"; }
};

class CAISenseHearTurretFire : public CAISenseHear
{

	public :

		void TurretFire(const SoundButes & info, const BodyProp * pBody);

		virtual void GetAttributes(int nTemplateID);

		virtual const char* GetDescription() const { return "HearTurretFire"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanHearTurretFire"; }

	protected :

		virtual void UpdateAllInstances();
};

#endif  // __AI_SENSE_HEAR_H__

