// ----------------------------------------------------------------------- //
//
// MODULE  : CSense.h
//
// PURPOSE : Sense information aggregrate
//
// CREATED : 2.23.00
//
// ----------------------------------------------------------------------- //

#ifndef __SENSE_H__
#define __SENSE_H__


class LMessage;

class CSenseInstance
{
	public :

		CSenseInstance(DFLOAT fAccumRate = 0.0f, DFLOAT fDecayRate = 0.0f, DFLOAT fThreshold = 0.0f);
		virtual ~CSenseInstance() {}

		virtual void  Enable();
		virtual void  Disable();

		DBOOL IsEnabled() const { return m_bEnabled; }
		DBOOL IsPartial() const { return m_fStimulation >= m_fPartialThreshold; }
		DBOOL IsFull()    const { return m_fStimulation >= 1.0f; }

		DFLOAT GetStimulationLevel() const { return m_fStimulation; }

		// These need to be set by the owner.
		void  SetThreshold(DFLOAT val) { m_fPartialThreshold = val; }
		void  SetAccumRate(DFLOAT val) { m_fAccumRate = val; }
		void  SetDecayRate(DFLOAT val) { m_fDecayRate = val; }

		virtual void Save(LMessage * out);

		virtual void Load(LMessage * in);

	protected :

		void  Stimulate(DFLOAT level);
		void  FullyStimulate()		  { Stimulate(-1.0f); }

	private :

		void AgeStimulation();


		// constants
		DFLOAT  m_fPartialThreshold;
		DFLOAT  m_fAccumRate;
		DFLOAT  m_fDecayRate;

		// variables
		DBOOL	m_bFirstUpdate;
		DBOOL   m_bEnabled;

		DFLOAT  m_fStimulation;
		LTFLOAT m_fLastUpdateTime;
};


inline LMessage & operator<<(LMessage & out, /*const*/ CSenseInstance & val)
{
	val.Save(&out);
	return out;
}

inline LMessage & operator>>(LMessage & in, CSenseInstance & val)
{
	val.Load(&in);
	return in;
}

void ReadDummySenseInstance(LMessage * pMsg);

#endif //__SENSE_H__
