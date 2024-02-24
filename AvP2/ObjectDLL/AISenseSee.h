// ----------------------------------------------------------------------- //
//
// MODULE  : CAISenseSee.h
//
// PURPOSE : Vision based sense class
//
// CREATED : 9.15.2000
//
// ----------------------------------------------------------------------- //

#ifndef __AI_SENSE_SEE_H__
#define __AI_SENSE_SEE_H__

#include "AIUtils.h"
#include "AISense.h"
#include "Timer.h"

class CCharacter;
class CAI;
struct AIBM_Sense;
struct AIBM_SeeSense;
class LMessage;


class CAISenseSee : public CAISense 
{

	public :

		CAISenseSee()
			: m_fLightMin(0.0f),
			  m_fLightMax(0.0f),
			  m_vPointColor(0,0,0) {}

		virtual const char* GetDescription() const = 0;
		virtual const char* GetPropertiesEnabledString() const { return "CanSee"; }
		virtual const char* GetPropertiesDistanceString() const { return "SeeDistance"; }

		virtual LTBOOL	IsVisible(CAISenseInstance * pInstance);
		virtual void	UpdateStimulation(CAISenseInstance * instance);

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);

	protected :

		virtual LTFLOAT GetRateMultiplier(HOBJECT hStimulus, const LTVector & vStimulus) { return 1.0f; }
	
		LTFLOAT m_fLightMin;
		LTFLOAT m_fLightMax;

		LTVector m_vPointColor;
};



class CAISenseSeeEnemy : public CAISenseSee
{
	public:
		
		CAISenseSeeEnemy()
			: m_fShortTimeDistanceSqr(0.0f),
			  m_fLongTimeDistanceSqr(0.0f),
			  m_fLongDivShortTime(0.0f),

			  m_fCloakFalloffMinDist(0.0f),
			  m_fCloakFalloffRange(0.0f),
			  m_fCloakSensitivity(0.0f),
			  m_fMinAlphaCloakAffect(0.0f),
			  m_fMaxAlphaCloakAffect(0.0f),
		
			  m_fCosFOV(0.0f),
			  m_fCosStartFOV(0.0f),
	
			  m_fSinVerticalBlind(0.0f),
			  m_fSinStartVerticalBlind(0.0f) {}

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual const char* GetDescription() const { return "SeeEnemy"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanSeeEnemy"; }
		virtual const char* GetPropertiesDistanceString() const { return "SeeEnemyDistance"; }

		virtual void GetAttributes(int nTemplateID);
		virtual LTFLOAT GetRateMultiplier(HOBJECT hStimulus, const LTVector & vStimulus);

		void Touched(const CCharacter & character);

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);


	private:

		LTFLOAT m_fShortTimeDistanceSqr;
		LTFLOAT m_fLongTimeDistanceSqr;
		LTFLOAT m_fLongDivShortTime;

		LTFLOAT m_fCloakSensitivity;      // Set to zero to ignore cloaking (ie. aliens and predators).
		LTFLOAT m_fCloakFalloffMinDist;      // Cloaked targets will start to go to zero at this range.
		LTFLOAT m_fCloakFalloffRange;     // Cloaked targets are invisible at (m_fCloakFalloffMinDist + m_fCloakFalloffRange).
		LTFLOAT m_fMinAlphaCloakAffect;     // Least alpha facter can affect the rate, before cloak sensitivity has been multiplied.
		LTFLOAT m_fMaxAlphaCloakAffect;		// Most alpha facter can affect the rate, before cloak sensitivity has been multiplied.

		LTFLOAT m_fCosFOV;
		LTFLOAT m_fCosStartFOV;
	
		LTFLOAT m_fSinVerticalBlind;
		LTFLOAT m_fSinStartVerticalBlind;
};

class CAISenseSeeDeadBody : public CAISenseSee
{
	public:

		CAISenseSeeDeadBody() {};

		virtual const char* GetDescription() const { return "SeeDeadBody"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanSeeDeadBody"; }
		virtual const char* GetPropertiesDistanceString() const { return "SeeDeadBodyDistance"; }

		virtual void GetAttributes(int nTemplateID);

	protected :

		virtual void UpdateAllInstances();

};

class CAISenseSeeBlood : public CAISenseSee
{
	public:

		CAISenseSeeBlood() {};

		virtual const char* GetDescription() const { return "SeeBlood"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanSeeBlood"; }
		virtual const char* GetPropertiesDistanceString() const { return "SeeBloodDistance"; }

		virtual void GetAttributes(int nTemplateID);

	protected :

		virtual void UpdateAllInstances();
};

class CAISenseSeeEnemyFlashlight : public CAISenseSee
{
	public:

		CAISenseSeeEnemyFlashlight()
				: m_bWasVisible(LTFALSE) {};

		virtual const char* GetDescription() const { return "SeeEnemyFlashlight"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanSeeEnemyFlashlight"; }
		virtual const char* GetPropertiesDistanceString() const { return "SeeEnemyFlashlightDistance"; }
		
		virtual LTBOOL IsVisible(CAISenseInstance *pInstance);
		virtual bool IgnoreCharacter(const CCharacter *pChar);
		virtual void GetAttributes(int nTemplateID);

	private:

		// Don't need to save these...
		CTimer	m_tmrIntersect;			// don't use IntersectSegment every update
		LTBOOL	m_bWasVisible;			// was visible on the previous intersect
};

class CAISenseSeeCloaked : public CAISenseSee
{
	public:

		CAISenseSeeCloaked() {};

		virtual bool IgnoreCharacter(const CCharacter * pChar);

		virtual const char* GetDescription() const { return "SeeCloaked"; }
		virtual const char* GetPropertiesEnabledString() const { return "CanSeeCloaked"; }
		virtual const char* GetPropertiesDistanceString() const { return "SeeCloakedDistance"; }

		virtual void GetAttributes(int nTemplateID);

	protected :

		virtual void UpdateAllInstances();

};


#endif  // __AI_SENSE_SEE_H__
