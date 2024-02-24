// ----------------------------------------------------------------------- //
//
// MODULE  : FrameRateMgr.h
//
// PURPOSE : Frame Rate Manager - Definition
//
// CREATED : 4/25/1
//
// ----------------------------------------------------------------------- //

#ifndef __FRAMERATE_MANAGER_H__
#define __FRAMERATE_MANAGER_H__

// --------------------------------------------------------------------------------- //
// Includes
#include "RateTracker.h"


// --------------------------------------------------------------------------------- //
// Data Types
enum FRState
{
	FR_UNPLAYABLE = 0,
	FR_POOR,
	FR_AVERAGE,
	FR_GOOD,
	FR_EXCELLENT,
};


// --------------------------------------------------------------------------------- //
// Class Definition
class CFrameRateMgr
{
public:

	// --------------------------------------------------------------------------------- //
	// Construction and destruction
	CFrameRateMgr();
	~CFrameRateMgr();


	// --------------------------------------------------------------------------------- //
	// Accessors
	FRState GetState() { return m_FRState; }


	// --------------------------------------------------------------------------------- //
	// Utilities
	void Update();
	void Init();

private:

	// --------------------------------------------------------------------------------- //
	// Member Functions


	// --------------------------------------------------------------------------------- //
	// Member Variables
	FRState		m_FRState;
	RateTracker m_Tracker;
	LTFLOAT		m_fLastTime;
};

#endif // __FRAMERATE_MANAGER_H__
