// ----------------------------------------------------------------------- //
//
// MODULE  : CAISense.h
//
// PURPOSE : Sense information encapsulation class
//
// CREATED : 7.26.1999
//
// ----------------------------------------------------------------------- //

#ifndef __AI_SENSE_H__
#define __AI_SENSE_H__

#include "ltsmartlink.h"
#include "AIUtils.h"
#include "AI.h"

#include <map>
#include <vector>

class CCharacter;
struct AIBM_Sense;
class LMessage;

class CAISenseInstance
{
	public :

		CAISenseInstance( const LTSmartLink & hStimulus = LTNULL,
			            LTFLOAT fAccumRate = 0.0f, 
						LTFLOAT fDecayRate = 0.0f,
						LTFLOAT fPartialLevel = 0.0f);

		virtual ~CAISenseInstance() {}

		virtual void Update();

		LTFLOAT GetStimulation() const		    { return m_fStimulation; }
		const LTSmartLink & GetStimulus() const { return m_hStimulus; }

		bool  HitPartial() const { return m_fStimulation >= m_fPartialLevel && m_fLastStimulation < m_fPartialLevel; }

		void  Stimulate(LTFLOAT level, const LTVector & vLocation) 	{  _ASSERT( level >= 0.0f ); m_fStimulationAccum += level; m_vPos = vLocation; }
		void  FullyStimulate()		  { m_bFullyStimulate = true; }

		const LTVector & GetPos() const { return m_vPos; }
		const LTFLOAT GetUpdateTime() const { return m_fLastUpdateTime; }

		// These need to be set by the owner.
		void  SetAccumRate(LTFLOAT val) { m_fAccumRate = val; }
		void  SetDecayRate(LTFLOAT val) { m_fDecayRate = val; }
		void  SetRates(LTFLOAT accum, LTFLOAT decay) { SetAccumRate(accum); SetDecayRate(decay); }
		void  SetPartialLevel(LTFLOAT val) { m_fPartialLevel = val; }

		virtual void Save(LMessage * out);
		virtual void Load(LMessage * in);

	private :

		// constants
		LTSmartLink m_hStimulus;
		LTFLOAT  m_fAccumRate;
		LTFLOAT  m_fDecayRate;
		LTFLOAT	 m_fPartialLevel;

		// variables
		LTFLOAT  m_fStimulation;
		LTFLOAT  m_fLastStimulation;  // a copy of the stimulation level before the update.  Used by HitPartial().
		LTFLOAT  m_fStimulationAccum;
		bool     m_bFullyStimulate;

		LTFLOAT m_fLastUpdateTime;

		LTVector m_vPos;
};


inline LMessage & operator<<(LMessage & out, /*const*/ CAISenseInstance & val)
{
	val.Save(&out);
	return out;
}

inline LMessage & operator>>(LMessage & in, CAISenseInstance & val)
{
	val.Load(&in);
	return in;
}

class CAISense
{
	public : // Public methods

		typedef std::map<HOBJECT,CAISenseInstance> InstanceList;
		typedef std::vector<LTFLOAT>		TimeStampList;

		// Ctors/dtors/etc

		CAISense()
			: m_pAI(LTNULL),
			  m_bPropEnabled(LTTRUE),
			  m_bEnabled(false),
			  m_bDisabled(true),
			  m_fAccumRate(0.0f),
			  m_fDecayRate(0.0f),
			  m_fPartialLevel(0.0f),
			  m_fDistanceSqr(0.0f),

			  m_hStimulus(LTNULL),
			  m_vStimulusPos(0,0,0),
			  m_bWaitingFor1stTrigger(LTTRUE),
			  m_hstr1stTriggerObject(LTNULL),
			  m_hstr1stTriggerMessage(LTNULL),
			  m_hstrTriggerObject(LTNULL),
			  m_hstrTriggerMessage(LTNULL) {}
				

		virtual ~CAISense();

		virtual void Init(const CAI* pAI);

		// Updates

		void Update();

		virtual void UpdateStimulation(CAISenseInstance * instance) = 0;
		
		virtual bool IgnoreCharacter(const CCharacter *) { return false; }

		
		// Handlers

		void Enable(LTBOOL val = LTTRUE) { m_bPropEnabled = val; }


		LTBOOL IsEnabled() { return m_bPropEnabled && m_bEnabled; }

		void AutoDisable()
		{
			// Only disable if we don't have a trigger message to send.
			if( !m_bWaitingFor1stTrigger && !m_hstrTriggerMessage )
			{
				m_bEnabled = false;
			}
		}


		// Stimulation
		LTFLOAT GetStimulation() const;
		LTFLOAT GetStimulation(HOBJECT hStimulus) const;
		const LTSmartLink & GetStimulus() const { m_bEnabled = true; return m_hStimulus; }
		const LTVector & GetStimulusPos() const { m_bEnabled = true; return m_vStimulusPos; }

		bool HasPartialStimulus() const { return GetStimulation() > m_fPartialLevel; }
		LTFLOAT GetPartialLevel() const { return m_fPartialLevel; }

		// Helpers

		virtual const char* GetDescription() const = 0;
		virtual const char* GetPropertiesEnabledString() const = 0;
		virtual const char* GetPropertiesDistanceString() const = 0;

		virtual void GetAttributes(int iTemplate) = 0;
		void GetProperties(GenericProp* pgp);

		void Set1stTriggerObject(HSTRING str) { m_hstr1stTriggerObject = str; m_bEnabled = true; }
		void Set1stTriggerMessage(HSTRING str) { m_hstr1stTriggerMessage = str; m_bEnabled = true; }
		void SetTriggerObject(HSTRING str) { m_hstrTriggerObject = str; m_bEnabled = true; }
		void SetTriggerMessage(HSTRING str) { m_hstrTriggerMessage = str; m_bEnabled = true; }


		void Reset1stReaction() { m_bWaitingFor1stTrigger = LTTRUE; }

		// Engine junk

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);

		// This should be in protected, here for CAI's use.
		LTFLOAT GetDistance() const { return (LTFLOAT)sqrt(m_fDistanceSqr); }

	protected : 

		virtual void UpdateAllInstances();
		void SetAttributes(const AIBM_Sense & pSenseData);
		void RemoveInstance(InstanceList::iterator instance_to_remove);


		// Accessors for our inheritees.
		const CAI* GetAIPtr() const { return m_pAI; }
		LTFLOAT GetAccumRate() const { return m_fAccumRate; }
		LTFLOAT GetDecayRate() const { return m_fDecayRate; }
		LTFLOAT GetDistanceSqr() const { return m_fDistanceSqr; }
		InstanceList & GetInstanceList() { return m_Instances; }


		CAISenseInstance * GetInstance(HOBJECT hStimulus);
		const CAISenseInstance * GetInstance(HOBJECT hStimulus) const;

		 // This provides a multiplier if the AI is alert.
		LTFLOAT GetAlertRateModifier() const  {	if( m_pAI && m_pAI->IsAlert() )	return m_fAlertRateModifier; return 1.0f; }


		// protected variables
		LTSmartLink 	m_hStimulus;					// The most recent object to cause the stimulus
		LTVector		m_vStimulusPos;					// The location of the stimulation to give us our stimulus.

	private :

		const CAI*		m_pAI;							// AI backpointer

		InstanceList	m_Instances;                    // List of sense instances.

		LTBOOL			m_bPropEnabled;					// False if disabled by DEdit properties.
		mutable bool	m_bEnabled;						// Are we enabled?
		bool			m_bDisabled;					// Needed to know when to clear values, should only be set in Update().
		LTFLOAT			m_fAccumRate;					// Stored value for new instances
		LTFLOAT			m_fDecayRate;					// Stored value for new instances
		LTFLOAT			m_fPartialLevel;				// Stored value for new instances
		LTFLOAT			m_fAlertRateModifier;			// This will be applied if the AI is alert.
		LTFLOAT			m_fDistanceSqr;					// The square of the distance at which the sense can perceive its stimulus.

		LTBOOL			m_bWaitingFor1stTrigger;
		HSTRING			m_hstr1stTriggerObject;
		HSTRING			m_hstr1stTriggerMessage;
		HSTRING			m_hstrTriggerObject;
		HSTRING			m_hstrTriggerMessage;
};

inline LMessage & operator<<(LMessage & out, /*const*/ CAISense & val)
{
	val.Save(&out);
	return out;
}

inline LMessage & operator>>(LMessage & in, CAISense & val)
{
	val.Load(&in);
	return in;
}

#ifndef _FINAL
bool ShouldShowAllSenses();
#endif



#endif // __AI_SENSE_H__
