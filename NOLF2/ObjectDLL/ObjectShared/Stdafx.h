// Stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifndef __LINUX
// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  

// This removes warnings in Vis C about the CodeWarrior pragma 'force_active'
//
#pragma warning( disable : 4068 )

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#else

#define HMODULE void*
#define UINT uintptr_t
// from regmgr
#include <vector>
#include <string>
template<typename B>
std::vector<std::basic_string<B>> split(const std::basic_string<B> &src, B sep);

#endif
#include <stdio.h>
#include <limits.h>
#include <memory>

#include "mfcstub.h"

#include "DebugNew.h"

#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "Globals.h"

#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "iltmath.h"
#include "iltsoundmgr.h"
#include "ltobjectcreate.h"
#include "ltbasetypes.h"

#include "ltobjref.h"

#include "Factory.h"

#include "ServerUtilities.h"
#include "GameServerShell.h"

#include "AutoMessage.h"

#endif // __STDAFX_H__