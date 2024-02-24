// ----------------------------------------------------------------------- //
//
// MODULE  : Globals.h
//
// PURPOSE : Global data
//
// CREATED : 4/26/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#ifdef _CLIENTBUILD
#include "clientheaders.h"
extern CClientDE* g_pInterface;
#else
#include "serverheaders.h"
extern CServerDE* g_pInterface;
#endif

#include <set>
#include <string>
#include <list>

#ifdef _WIN32
	#define SERVER_OPTIONS_CONFIG_DIRECTORY		"ServerData\\"
#else
	#define SERVER_OPTIONS_CONFIG_DIRECTORY		"ServerData/"
#endif

#define SERVER_OPTIONS_CONFIG_EXTENSION		".txt"

// global constants
const int MAX_OBJECT_ARRAY_SIZE = 32;

#define TERM(var)		if (var)  { delete var; var = NULL; }
#define TERMSURF(surf)	if (surf) { g_pInterface->DeleteSurface(surf); surf = NULL; }
#define TERMSTRING(str) if (str)  { g_pInterface->FreeString(str); str = NULL; }
#define TERMOBJ(obj)	if (obj)  { g_pInterface->DeleteObject(obj); obj = NULL; }
#define	COORD_CENTER	0x80000000

inline BOOL PtInRect(RECT* pRect, int x, int y)
{
	if((x >= pRect->right) || (x < pRect->left) ||
	   (y >= pRect->bottom) || (y < pRect->top))
	   return FALSE;

	return TRUE;
}

// Helper classes

template<class TYPE>
class CRange
{
	public:

		CRange() { }
		CRange(TYPE fMin, TYPE fMax) { m_fMin = fMin; m_fMax = fMax; }

		void Set(TYPE fMin, TYPE fMax) { m_fMin = fMin; m_fMax = fMax; }

		TYPE GetMin() { return m_fMin; }
		TYPE GetMax() { return m_fMax; }

	protected:

		TYPE m_fMin;
		TYPE m_fMax;
};

// Miscellaneous functions...

template< class T >
inline T MinAbs( T a, T b)
{
    return (T)fabs(a) < (T)fabs(b) ? a : b;
}

template< class T >
inline T MaxAbs( T a, T b)
{
    return (T)fabs(a) > (T)fabs(b) ? a : b;
}

template< class T >
inline T Min( T a, T b)
{
    return a < b ? a : b;
}

template< class T >
inline T Max( T a, T b)
{
    return a > b ? a : b;
}

template< class T >
inline T Clamp( T val, T min, T max )
{
	return Min( max, Max( val, min ));
}

class CaselessGreater
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (stricmp(x.c_str(), y.c_str()) > 0 );
	}
};

class CaselessLesser
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (stricmp(x.c_str(), y.c_str()) < 0 );
	}
};


// String containers
typedef std::set<std::string,CaselessLesser> StringSet;
typedef std::list<std::string> StringList;


#endif  // _GLOBALS_H_
