// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSoundMgr.cpp
//
// PURPOSE : ClientSoundMgr implementation - Controls sound on the client
//
// CREATED : 7/10/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientSoundMgr.h"
#include "CommonUtilities.h"
#include "VarTrack.h"
#include "SoundButeMgr.h"
#include "CharacterButeMgr.h"


// Global pointer to client sound mgr...

CClientSoundMgr*  g_pClientSoundMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::CServerSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CClientSoundMgr::CClientSoundMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSoundMgr::~CServerSoundMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientSoundMgr::~CClientSoundMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CClientSoundMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	g_pClientSoundMgr = this;

	return CSoundMgr::Init(pInterface, szAttributeFile);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CClientSoundMgr::Term()
{
    g_pClientSoundMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays sound inside the player's head using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlaySoundLocal(const char *szBute)
{
    if (!szBute) return LTNULL;

	//get the set index and call the other play sound function
	int nBute = g_pSoundButeMgr->GetSoundSetFromName(szBute);

	if(nBute == -1) return LTNULL;

	//get the set index and call the other play sound function
	return PlaySoundLocal( nBute );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays sound inside the player's head using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlaySoundLocal(uint32 nBute)
{
	SoundButes butes = g_pSoundButeMgr->GetSoundButes(nBute);

	if(butes.m_fPlayChance < 1.0f)
	{
		if(GetRandom(0.0f,1.0f) > butes.m_fPlayChance)
			return LTNULL;
	}

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = butes.m_nFlags;

	const char * szSoundName = g_pSoundButeMgr->GetRandomSoundFileWeighted(nBute);
	if( !szSoundName )
		return LTNULL;

	strncpy(psi.m_szSoundName, szSoundName, _MAX_PATH);
	psi.m_szSoundName[_MAX_PATH - 1] = '\0';
	
	psi.m_nPriority		= butes.m_ePriority;
	psi.m_nVolume		= butes.m_nVolume;
	psi.m_fPitchShift	= butes.m_fPitch;

    return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays sound inside the player's head using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlayCharSoundLocal(uint32 nCharBute, const char *szBute)
{
    if (!szBute) return LTNULL;

	//get the set index and call the other play sound function
	CString strSoundSetName = GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(nCharBute), szBute );

	int nBute = g_pSoundButeMgr->GetSoundSetFromName( strSoundSetName );

	if(nBute == -1) return LTNULL;

	//get the set index and call the other play sound function
	return PlaySoundLocal( nBute );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays sound inside the player's head using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlaySoundLocal(const CString& csName, 
	SoundPriority ePriority, uint32 dwFlags, uint8 nVolume, float fPitchShift)
{
	return PlaySoundLocal((char *)(LPCSTR)csName, ePriority, dwFlags, nVolume, fPitchShift);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySoundLocal
//
//	PURPOSE:	Plays sound inside the player's head
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlaySoundLocal(char *pName, SoundPriority ePriority,
	uint32 dwFlags, uint8 nVolume, float fPitchShift)
{
    if (!pName) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_dwFlags |= dwFlags;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_nVolume		= nVolume;
	psi.m_fPitchShift	= fPitchShift;

    return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSoundMgr::PlaySound()
//
//	PURPOSE:	Play the sound associated with the sound info
//
// ----------------------------------------------------------------------- //

HLTSOUND CClientSoundMgr::PlaySound(PlaySoundInfo & psi)
{
	HLTSOUND hSnd = LTNULL;

	// Play the sound...

	LTRESULT hResult = g_pLTClient->PlaySound(&psi);

	if (hResult == LT_OK)
	{
		hSnd = psi.m_hSound;
	}
	else
	{
		_ASSERT(LTFALSE);
		g_pLTClient->CPrint("ERROR in CClientSoundMgr::PlaySound() - Couldn't play sound '%s'", psi.m_szSoundName);
		return LTNULL;
	}

	return hSnd;
}
