// ----------------------------------------------------------------------- //
//
// MODULE  : VersionMgr.cpp
//
// PURPOSE : Implementation of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VersionMgr.h"
#include "regmgr.h"
#include "CommonUtilities.h"
#if defined(_CLIENTBUILD)
#include "clientres.h"
#endif

// {DF560590-D918-4a51-9CB1-38EBBE3879AD}
LTGUID GAMEGUID = 
{ 0xdf560590, 0xd918, 0x4a51, { 0x9c, 0xb1, 0x38, 0xeb, 0xbe, 0x38, 0x79, 0xad } };

int const GAME_HANDSHAKE_VER_MAJOR = 1;
int const GAME_HANDSHAKE_VER_MINOR = 0;
int const GAME_HANDSHAKE_VER = ((GAME_HANDSHAKE_VER_MAJOR << 8) + GAME_HANDSHAKE_VER_MINOR);

static const uint32 s_nBuildNumber = 186;
static const char* s_szNetVersion	= "CJ v1.0";

const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__1_0 = s_nBuildNumber;
const CVersionMgr::TSaveVersion CVersionMgr::kSaveVersion__CurrentBuild = s_nBuildNumber;

CVersionMgr* g_pVersionMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::CVersionMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVersionMgr::CVersionMgr()
{
	// Determine what language version the game is using...
	m_bLowViolence	= false;

	m_regMgr.Init("Monolith Productions", GAME_NAME, "1.0");

	char szString[256];

	if (m_regMgr.IsValid())
	{
		uint32 nSize = ARRAY_LEN( szString );
		m_regMgr.Get( "NetRegionCode", szString, nSize, "EN" );
		m_sNetRegion = szString;
	}

	sprintf( szString, "Build %d", s_nBuildNumber );
	m_sBuild = szString;

	m_sPatchVersion = GetNetVersion( );

	m_nCurrentSaveVersion = 0;

	g_pVersionMgr = this;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetCurrentSaveVersion()
//
//	PURPOSE:	Grab the "current" save version.  Meaning the version
//				of the save file currently being loaded.
//
// ----------------------------------------------------------------------- //

CVersionMgr::TSaveVersion CVersionMgr::GetCurrentSaveVersion() const
{
	// The only time we care about old save versions is when we are restoring a
	// previously saved game.
	if( m_nLastLGFlags == LOAD_RESTORE_GAME )
		return m_nCurrentSaveVersion;

	return CVersionMgr::kSaveVersion__CurrentBuild;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetCDKey
//
//	PURPOSE:	Gets the cdkey set in the registry.
//
// ----------------------------------------------------------------------- //
void CVersionMgr::GetCDKey( char* pszCDKey, uint32 nCDKeySize )
{
	m_regMgr.Get("CDKey", pszCDKey, nCDKeySize, "");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::SetCDKey
//
//	PURPOSE:	Sets the cdkey set into registry.
//
// ----------------------------------------------------------------------- //
void CVersionMgr::SetCDKey( char const* pszCDKey )
{
	m_regMgr.Set( "CDKey", pszCDKey ? pszCDKey : "" );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr::GetSaveVersion()
//
//	PURPOSE:	Get the current save version 
//
// ----------------------------------------------------------------------- //

const uint32 CVersionMgr::GetSaveVersion()
{
	return s_nBuildNumber;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr:GetDisplayVersion
//
//	PURPOSE:	Get the current display version 
//
// ----------------------------------------------------------------------- //

void CVersionMgr::GetDisplayVersion( char* pszDisplayVersion, uint32 nDisplayVersionLen ) const
{
#ifdef _CLIENTBUILD
#ifndef _FINAL
	_snprintf( pszDisplayVersion, nDisplayVersionLen, "%s Build %d (DO NOT DISTRIBUTE)", LoadTempString( IDS_GAME_VERSION ), s_nBuildNumber );
	pszDisplayVersion[nDisplayVersionLen-1] = 0;
#else // _FINAL
	LTStrCpy( pszDisplayVersion, LoadTempString( IDS_GAME_VERSION ), nDisplayVersionLen );
#endif // _FINAL
#endif // _CLIENTBUILD
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVersionMgr:GetNetVersion()
//
//	PURPOSE:	Get the current net version 
//
// ----------------------------------------------------------------------- //

const char* CVersionMgr::GetNetVersion() const
{
	return s_szNetVersion;
}

void CVersionMgr::Update()
{
#if defined(_CLIENTBUILD)
	if (stricmp(LoadTempString(IDS_ALLOW_GORE),"TRUE") != 0)
	{
		m_bLowViolence = true;
	}

	//override for debugging purposes
	float fVal = 0.0f;

    if (!g_pLTClient) return;

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("LowViolence");
	if(hVar)
	{
        fVal = g_pLTClient->GetVarValueFloat(hVar);
		if (fVal >= 1.0f)
		{
			m_bLowViolence = true;
		}

	}
#endif

}

