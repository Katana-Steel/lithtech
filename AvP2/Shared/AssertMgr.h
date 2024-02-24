// ----------------------------------------------------------------------- //
//
// MODULE  : AssertMgr.h
//
// PURPOSE : AssertMgr definition
//
// CREATED : 05.06.1999
//
// ----------------------------------------------------------------------- //

#ifndef __ASSERT_MGR_H__
#define __ASSERT_MGR_H__

class CAssertMgr
{
	public : // Public methods

		static void Enable();
		static void Disable();

	protected : // Protected methods

		static int ReportHook(int nReportType, char* szMessage, int* pnReturnValue);

	protected : // Protected member variables

		static DBOOL			m_bEnabled;

#ifdef _WIN32
		static _CRT_REPORT_HOOK	m_crhPrevious;
#endif

};

#endif 
