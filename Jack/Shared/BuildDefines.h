// ----------------------------------------------------------------------- //
//
// MODULE  : BuildDefines.h
//
// PURPOSE : Definition of build variables, like _DEMO.
//
// CREATED : 08/26/03
//
// (c) 2000-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BUILDDEFINES_H__
#define __BUILDDEFINES_H__

// Set of mutually exclusive flags to define the game sku.  If none are defined,
// then retail is assumed.

// Flags code that should be part of the MP Demo.
//#define _MPDEMO

// Flags code that should be part of the SP Demo.
//#define _SPDEMO

// Flags code that should be part of the Public Relations Demo.
//#define _PRDEMO


// Make sure the demo flag is set if any of the demo sku's are defined.
#if defined( _MPDEMO ) || defined( _SPDEMO ) || defined( _PRDEMO )
#define _DEMO
#endif // defined( _MPDEMO ) || defined( _SPDEMO ) || defined( _PRDEMO )

// Known SKU's of the game.  This is used as the registry entry.
// This is used for folder paths and registry entries.  Don't put invalid characters into the name.
// This is not to be localized.
#define GAME_NAME_RETAIL	"Contract Jack"
#define GAME_NAME_MPDEMO	"Contract Jack (MP Demo)"
#define GAME_NAME_SPDEMO	"Contract Jack (SP Demo)"
#define GAME_NAME_PRDEMO	"Contract Jack (PR Demo)"

// Known SKU executable names.
#define LAUNCHER_EXE_RETAIL		"ContractJack.exe"		
#define LAUNCHER_EXE_MPDEMO		"ContractJackMpDemo.exe"		
#define LAUNCHER_EXE_SPDEMO		"ContractJackSpDemo.exe"		
#define LAUNCHER_EXE_PRDEMO		"ContractJackPrDemo.exe"		

#ifdef _MPDEMO
#define GAME_NAME		GAME_NAME_MPDEMO
#define LAUNCHER_EXE	LAUNCHER_EXE_MPDEMO
#elif defined(_SPDEMO)
#define GAME_NAME		GAME_NAME_SPDEMO
#define LAUNCHER_EXE	LAUNCHER_EXE_SPDEMO
#elif defined(_PRDEMO)
#define GAME_NAME		GAME_NAME_PRDEMO
#define LAUNCHER_EXE	LAUNCHER_EXE_PRDEMO
#else
#define GAME_NAME		GAME_NAME_RETAIL
#define LAUNCHER_EXE	LAUNCHER_EXE_RETAIL
#endif

#endif __BUILDDEFINES_H__