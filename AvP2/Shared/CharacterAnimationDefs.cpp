// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterAnimationDefs.cpp
//
// PURPOSE : General character animation defines
//
// CREATED : 4/6/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterAnimationDefs.h"

#ifndef _CLIENTBUILD


ILTMessage & operator<<(ILTMessage & out, t_TrackerInfo & tracker_info)
{
	LTBOOL bSaveTracker = ( tracker_info.m_pTracker == &tracker_info.m_Tracker );

	out << bSaveTracker;
	if( bSaveTracker )
	{
		ILTModel * pModel = g_pInterface->GetModelLT();

		pModel->WriteTracker( tracker_info.m_pTracker, &out);
	}


	out << tracker_info.m_fAimingRange;
	out << tracker_info.m_nIndex;

	out << tracker_info.m_nTrackerType;
	out << tracker_info.m_nTrackerData;

	out << tracker_info.m_szAnim;
	out << tracker_info.m_hAnim;
	out << tracker_info.m_hWeightSet;

	out << tracker_info.m_szDestAnim;
	out << tracker_info.m_hDestAnim;
	out << tracker_info.m_hDestWeightSet;

	out << tracker_info.m_fLastAimingRange;
	out << tracker_info.m_nLastIndex;

	out.WriteFloat(tracker_info.m_fRandTime - g_pInterface->GetTime());
	out << tracker_info.m_fRandDelay;

	out << tracker_info.m_nState;

	return out;
}

ILTMessage & operator>>(ILTMessage & in, t_TrackerInfo & tracker_info)
{
	LTBOOL bLoadTracker = LTFALSE;

	in >> bLoadTracker;
	if( bLoadTracker )
	{
		ILTModel * pModel = g_pInterface->GetModelLT();

		pModel->ReadTracker( tracker_info.m_pTracker, &in);
	}

	in >> tracker_info.m_fAimingRange;
	in >> tracker_info.m_nIndex;

	in >> tracker_info.m_nTrackerType;
	in >> tracker_info.m_nTrackerData;

	in >> tracker_info.m_szAnim;
	in >> tracker_info.m_hAnim;
	in >> tracker_info.m_hWeightSet;

	in >> tracker_info.m_szDestAnim;
	in >> tracker_info.m_hDestAnim;
	in >> tracker_info.m_hDestWeightSet;

	in >> tracker_info.m_fLastAimingRange;
	in >> tracker_info.m_nLastIndex;

	tracker_info.m_fRandTime = in.ReadFloat() + g_pInterface->GetTime();
	in >> tracker_info.m_fRandDelay;

	in >> tracker_info.m_nState;

	return in;
}

#endif