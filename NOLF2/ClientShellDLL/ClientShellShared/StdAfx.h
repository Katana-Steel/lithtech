// stdafx.h : include file for standard system include files,
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
#include <SDL.h>
// stubdefs for silly win32 api stuff
typedef void* HINSTANCE;
typedef SDL_Window* HWND;
typedef void *HRSRC;
typedef void *HGLOBAL;
#define UINT uintptr_t
#define WINAPI

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif 

#include <string>
#include <vector>

// string split from RegMgr
template <typename B>
extern std::vector<std::basic_string<B>> split(const std::basic_string<B> &str, B sep);

#endif // __LINUX

#include <stdio.h>
#include <limits.h>
#include <memory>

#include "mfcstub.h"

#include "DebugNew.h"

#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "Globals.h"

#include "Sync.h"

#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "iltmath.h"
#include "iltsoundmgr.h"
#include "ltobjectcreate.h"
#include "ltbasetypes.h"

#include "Factory.h"

#include "iclientshell.h"
#include "ClientUtilities.h"
#include "CommonUtilities.h"
#include "GameClientShell.h"
#include "AutoMessage.h"

#endif // __STDAFX_H__
