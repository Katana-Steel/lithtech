// *********************************************************************** //
//
//	MODULE:		UTILS (.H)
//
//	PURPOSE:	Gernal purpose utilities
//
//	HISTORY:	02/15/95 [blg] This file was created
//
//	NOTICE:		Copyright (c) 1995, Monolith Productions, Inc.
//
// *********************************************************************** //

#ifndef _UTILS_H_
#define _UTILS_H_

#pragma warning( disable : 4786 )

#include "shlwapi.h"
#include <string>

// Macros

#define PACKVERSION(major,minor) MAKELONG(minor,major)

// Prototypes...

BOOL	ExistProcess(const char* sExe, int thresh = 0, HANDLE* phProcess = NULL);
DWORD	GetDllVersion(LPCTSTR lpszDllName);

BOOL	DirExist( char const* strPath );

class CaselessLesser
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (stricmp(x.c_str(), y.c_str()) < 0 );
	}
};

// EOF...

#endif
