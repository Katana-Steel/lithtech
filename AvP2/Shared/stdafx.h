// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

#pragma warning( disable : 4786 )
#pragma warning(disable : 4503)

#ifdef _WIN32

	#define WIN32_LEAN_AND_MEAN

	#include <windows.h>

#endif

#include <stdio.h>
#include <limits.h>
#include "mfcstub.h"

#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"

#include "modellt.h"
#include "transformlt.h"
#include "physics_lt.h"
#include "math_lt.h"

#include "Globals.h"
#include "CommonUtilities.h"
#include "MemoryUtils.h"
#include "PerfInfo.h"

#endif // __STDAFX_H__
