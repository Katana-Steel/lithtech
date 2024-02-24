// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundMgr.cpp
//
// PURPOSE : ServerSoundMgr implementation - Controls sound on the server
//
// CREATED : 7/10/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerSoundMgr.h"
#include "CommonUtilities.h"
#include "SoundButeMgr.h"
#include "CharacterButeMgr.h"

// Global pointer to server sound mgr...

CServerSoundMgr*  g_pServerSoundMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CServerSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CServerSoundMgr::CServerSoundMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::~CServerSoundMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CServerSoundMgr::~CServerSoundMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CServerSoundMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	g_pServerSoundMgr = this;

	return CSoundMgr::Init(pInterface, szAttributeFile);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CServerSoundMgr::Term()
{
    g_pServerSoundMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CacheSounds()
//
//	PURPOSE:	Cache the appropriate sounds...
//
// ----------------------------------------------------------------------- //
static bool CacheSoundSet(const char * szKey, CButeMgr::CSymTabItem * pItem, void * pData)
{
	if( !g_pSoundButeMgr || !g_pLTServer || !pItem || !pData )
		return true;

	// Ignore the parent parameter.
	if( 0 == stricmp(szKey, "Parent") )
		return true;


	// Be sure we have a string type.
	if( pItem->SymType == CButeMgr::StringType )
	{
		CString strSoundButesName = *(pItem->data.s);

		// Be sure it isn't empty.
		if( !strSoundButesName.IsEmpty() )
		{
			// Get the butes.
			const SoundFiles & sound_files = g_pSoundButeMgr->GetSoundFiles( strSoundButesName );

			// Go through each sound name and try to cache it.
			for( uint32 i = 0; i < sound_files.m_nNumSounds; ++i )
			{
				if( LT_NOTFOUND == g_pLTServer->CacheFile( FT_SOUND, const_cast<char*>(sound_files.m_szSounds[i]) ) )
				{
#ifndef _FINAL
//					g_pLTServer->CPrint( "WARNING : Sound file \"%s\" not found.", sound_files.m_szSounds[i] );
#endif
				}
			}
		}
	}

	return true;
}


void CServerSoundMgr::CacheSounds(const char* szSoundDir)
{
	static int nRecursionLevel = 0;

	if (!szSoundDir ) return;

	if( ++nRecursionLevel > 100 )
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Max recursion hit in CServerSoundMgr::CacheSounds!");
#endif
		--nRecursionLevel;
		return;
	}

	// Cache all the sounds...
	m_buteMgr.GetKeys(szSoundDir,CacheSoundSet,this);

	// Walk up the inheritance tree.
	const CString parent_name = m_buteMgr.GetString(szSoundDir,"Parent",CString(""));
	if( stricmp(parent_name,szSoundDir)  == 0) 
	{
		--nRecursionLevel;
		return;
	}

	CacheSounds(parent_name);

	--nRecursionLevel;
	return;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CacheCharacterSounds()
//
//	PURPOSE:	Cache the appropriate sounds...
//
// ----------------------------------------------------------------------- //
void CServerSoundMgr::CacheCharacterSounds(int nCharSet, const char* szBute)
{
	if( !g_pSoundButeMgr || !g_pLTServer || !szBute)
		return;

	CString strSoundSetName = GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(nCharSet), szBute );

	CacheSounds(strSoundSetName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CacheSounds()
//
//	PURPOSE:	Cache the appropriate sounds...
//
// ----------------------------------------------------------------------- //
void CServerSoundMgr::CacheButeSounds(const char* szBute)
{
	if( !g_pSoundButeMgr || !g_pLTServer || !szBute)
		return;

	CacheButeSounds(g_pSoundButeMgr->GetSoundSetFromName( szBute ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CacheSounds()
//
//	PURPOSE:	Cache the appropriate sounds...
//
// ----------------------------------------------------------------------- //
void CServerSoundMgr::CacheButeSounds(int nBute)
{
	if( !g_pSoundButeMgr || !g_pLTServer)
		return;

	if(nBute == -1) return;

	// Get the butes.
	const SoundFiles & sound_files = g_pSoundButeMgr->GetSoundFiles( nBute );

	// Go through each sound name and try to cache it.
	for( uint32 i = 0; i < sound_files.m_nNumSounds; ++i )
	{
		g_pLTServer->CacheFile( FT_SOUND,  const_cast<char*>(sound_files.m_szSounds[i]) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::PlaySoundDirect()
//
//	PURPOSE:	Allows the game to *basically* bypass the sound mgr and
//				call directly into the engine sound code.  This is mainly
//				for backwards compatibility.
//
// ----------------------------------------------------------------------- //

HLTSOUND CServerSoundMgr::PlaySoundDirect(PlaySoundInfo & psi)
{
	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::PlaySound()
//
//	PURPOSE:	Play the sound associated with the sound info...
//
// ----------------------------------------------------------------------- //

HLTSOUND CServerSoundMgr::PlaySound(PlaySoundInfo & psi)
{
	// Kick out early for blank sounds... even though that should
	// never be the case but unfortunately it is.
	if(!psi.m_szSoundName[0])
		return LTNULL;


	HLTSOUND hSnd = LTNULL;

	// Play the sound...

	LTRESULT hResult = g_pLTServer->PlaySound(&psi);

	if (hResult == LT_OK)
	{
		hSnd = psi.m_hSound;
	}
	else
	{
#ifndef _FINAL
		_ASSERT(LTFALSE);
		g_pLTServer->CPrint("ERROR in CServerSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
#endif
		return LTNULL;
	}

	return hSnd;
}
