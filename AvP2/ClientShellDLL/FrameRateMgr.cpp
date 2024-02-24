// ----------------------------------------------------------------------- //
//
// MODULE  : FrameRateMgr.cpp
//
// PURPOSE : Frame Rate Manager - Implementation
//
// CREATED : 4/25/1
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FrameRateMgr.h"
#include "GameClientShell.h"
#include "VarTrack.h"

// --------------------------------------------------------------------------------- //
// Console Variables

VarTrack	g_vtShowRate;
VarTrack	g_vtRateEnable;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFrameRateMgr::CFrameRateMgr()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
CFrameRateMgr::CFrameRateMgr()
{
	// Init the tracker
	m_Tracker.Init(0.5f);   // Value is how ofter the cycle re-set is called

	// Init the variables
	m_FRState	= FR_UNPLAYABLE;
	m_fLastTime	= g_pLTClient->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFrameRateMgr::~CFrameRateMgr()
//
//	PURPOSE:	Destruction
//
// ----------------------------------------------------------------------- //
CFrameRateMgr::~CFrameRateMgr()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFrameRateMgr::Init()
//
//	PURPOSE:	Initialization function
//
// ----------------------------------------------------------------------- //
void CFrameRateMgr::Init()
{
	// Init console
	if(!g_vtShowRate.IsInitted())
		g_vtShowRate.Init(g_pLTClient, "ShowRate", NULL, 0.0f);

	if(!g_vtRateEnable.IsInitted())
		g_vtRateEnable.Init(g_pLTClient, "RateEnable", NULL, 3.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFrameRateMgr::Update()
//
//	PURPOSE:	Update function
//
// ----------------------------------------------------------------------- //
void CFrameRateMgr::Update()
{
	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();

	// Update the tracker
	m_Tracker.Update(fTime - m_fLastTime, 1.0f);

	// Reset our timer
	m_fLastTime = fTime;

	// Get the current rate and record the level
	LTFLOAT fRate = m_Tracker.GetRate();

	if(g_pGameClientShell->IsMultiplayerGame())
	{
		if		(fRate >= 40.0f)	m_FRState = FR_EXCELLENT;
		else if	(fRate >= 30.0f)	m_FRState = FR_GOOD;
		else if	(fRate >= 20.0f)	m_FRState = FR_AVERAGE;
		else if	(fRate >= 10.0f)	m_FRState = FR_POOR;
		else						m_FRState = FR_UNPLAYABLE;
	}
	else
	{
		if		(fRate >= 30.0f)	m_FRState = FR_EXCELLENT;
		else if	(fRate >= 20.0f)	m_FRState = FR_GOOD;
		else if	(fRate >= 10.0f)	m_FRState = FR_AVERAGE;
		else if	(fRate >= 5.0f)		m_FRState = FR_POOR;
		else						m_FRState = FR_UNPLAYABLE;
	}


	// Show the rate if desired
	if(g_vtShowRate.GetFloat() > 0.0f)
	{
		const char* szLabel;

		switch (m_FRState)
		{
			case (FR_UNPLAYABLE):	szLabel="Unplayable";	break;
			case (FR_POOR):			szLabel="Poor";			break;
			case (FR_AVERAGE):		szLabel="Average";		break;
			case (FR_GOOD):			szLabel="Good";			break;
			case (FR_EXCELLENT):	szLabel="Excellent";	break;
		}

		g_pLTClient->CPrint("Frame Rate = %.2f, %s", fRate, szLabel);
	}

	// See if we are enabled, if not just set
	// the rate to max.
	int nRateEnable = (int)g_vtRateEnable.GetFloat();

	if(g_pGameClientShell->IsMultiplayerGame())
	{
		if(!(nRateEnable & 0x02))
			m_FRState = FR_EXCELLENT;
	}
	else
	{
		if(!(nRateEnable & 0x01))
			m_FRState = FR_EXCELLENT;
	}
}

