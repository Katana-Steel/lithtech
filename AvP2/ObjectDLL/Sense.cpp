// ----------------------------------------------------------------------- //
//
// MODULE  : CSense.cpp
//
// PURPOSE : Sense information aggregrate
//
// CREATED : 2.23.00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Sense.h"
#include "iltmessage.h"

#ifdef _DEBUG
//#define SENSE_DEBUG
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSenseInstance::CSenseInstance
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

CSenseInstance::CSenseInstance(DFLOAT fAccumRate /* = 0.0f */, DFLOAT fDecayRate /* = 0.0f */, DFLOAT fThreshold /* = 0.0f */)
	: m_fPartialThreshold(fThreshold),
	  m_fAccumRate(fAccumRate),
	  m_fDecayRate(fDecayRate),

	  m_bFirstUpdate(DTRUE),
	  m_bEnabled(DTRUE),

	  m_fStimulation(0.0f),
	  m_fLastUpdateTime( g_pLTServer->GetTime() ) {}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSenseInstance::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CSenseInstance::Save(LMessage * pMsg)
{
	if (!pMsg) return;

	*pMsg << m_fPartialThreshold;
	*pMsg << m_fAccumRate;
	*pMsg << m_fDecayRate;

	*pMsg << m_bFirstUpdate;
	*pMsg << m_bEnabled;

	*pMsg << m_fStimulation;
	pMsg->WriteFloat( g_pLTServer->GetTime() - m_fLastUpdateTime );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSense::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CSenseInstance::Load(LMessage * pMsg)
{
	if (!pMsg) return;

	*pMsg >> m_fPartialThreshold;
	*pMsg >> m_fAccumRate;
	*pMsg >> m_fDecayRate;

	*pMsg >> m_bFirstUpdate;
	*pMsg >> m_bEnabled;

	*pMsg >> m_fStimulation;
	m_fLastUpdateTime = g_pLTServer->GetTime() - pMsg->ReadFloat();
}

void ReadDummySenseInstance(LMessage * pMsg)
{
	LTFLOAT fDummy;
	LTBOOL bDummy;

	if( !pMsg ) return;

	*pMsg >> fDummy; // m_fPartialThreshold;
	*pMsg >> fDummy; // m_fAccumRate;
	*pMsg >> fDummy; // m_fDecayRate;

	*pMsg >> bDummy; // m_bFirstUpdate;
	*pMsg >> bDummy; // m_bEnabled;

	*pMsg >> fDummy; // m_fStimulation;
	*pMsg >> fDummy; // m_fLastUpdateTime = g_pLTServer->GetTime() - pMsg->ReadFloat();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSenseInstance::AgeStimulation
//
//	PURPOSE:	Manage the stimulation levels (add accumulations and decay).
//
// ----------------------------------------------------------------------- //

void CSenseInstance::Stimulate(DFLOAT level)
{
	if( !m_bEnabled ) return;


	const DFLOAT fFrameTime = g_pServerDE->GetTime() - m_fLastUpdateTime;
	m_fLastUpdateTime = g_pServerDE->GetTime();

	// Decay the old stimulation level.
	// This must be done even if the stimulation is zero, so that
	// if there is some accumulation, the adding of the decay rate
	// will cancel it out.  Otherwise the initial stimulus gets 
	// both the accum and decay rate.
	m_fStimulation -= fFrameTime*m_fDecayRate;

	// Add the new stimulations.
	if( level < 0.0f )
	{
		// A negative stimulation indicates full stimulation.
		m_fStimulation = 1.0f;
	}
	else
	{
		ASSERT( level >= 0.0f && m_fAccumRate >= 0.0f );

		// Adding the decay rate prevents a constant stimuli from decaying,
		//  ie. if m_fStimulationAccum == 1.0f on each update, the sense
		//      will be fully triggered after m_fAccumRate seconds.
		m_fStimulation += (m_fAccumRate+m_fDecayRate)*fFrameTime*level;
	}

#ifdef SENSE_DEBUG
	g_pServerDE->CPrint("Simulation : %f, Accum : %f, FrameTime : %f, AccumRate : %f, DecayRate : %f",
		       m_fStimulation, level, fFrameTime, m_fAccumRate, m_fDecayRate);
#endif

	
	// Clamp our stimulation level.
	m_fStimulation = LTCLAMP( m_fStimulation, 0.0f, 1.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSenseInstance::Disable
//
//	PURPOSE:	Disables the sense.  Class's state is left alone so that 
//              the information is still accessible.  Enable should reset
//              the state.
//
// ----------------------------------------------------------------------- //

void CSenseInstance::Disable()
{
	m_bEnabled = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSenseInstance::Enable
//
//	PURPOSE:	Enables the sense and resets its internal state.
//
// ----------------------------------------------------------------------- //

void CSenseInstance::Enable()
{
	m_bEnabled = DTRUE;

	m_fStimulation = 0.0f;
	m_fLastUpdateTime = g_pLTServer->GetTime();
}


