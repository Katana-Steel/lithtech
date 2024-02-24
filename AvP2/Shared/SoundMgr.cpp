// ----------------------------------------------------------------------- //
//
// MODULE  : SoundMgr.cpp
//
// PURPOSE : SoundMgr - Implementation
//
// CREATED : 02/05/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SoundMgr.h"
#include "CommonUtilities.h"
#include "SoundButeMgr.h"
#include "CharacterButeMgr.h"

#ifndef _CLIENTBUILD
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#endif

#define SMGR_DEFAULT_INNERRADIUSPERCENT		0.1f
#define SMGR_BASE_SOUND_ID					1000
#define SMGR_PATH_TAG						"Paths"


// Global pointer to sound mgr...

static char s_aTagName[30];
static char s_aAttName[100];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::CSoundMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSoundMgr::CSoundMgr()
{
	m_pInterface = LTNULL;
	m_fInnerRadiusPercent = SMGR_DEFAULT_INNERRADIUSPERCENT;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::~CSoundMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSoundMgr::~CSoundMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSoundMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
	m_pInterface = pInterface;

    return (szAttributeFile && Parse(pInterface, szAttributeFile));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::GetSoundSetName()
//
//	PURPOSE:	Get a particular sound set from a tag...
//
// ----------------------------------------------------------------------- //

CString CSoundMgr::GetSoundSetName(const char* szSoundDir, const char* szSoundSetType)
{
	CString strRet;
	if (!szSoundDir || !szSoundSetType) return strRet;

	return GetString(szSoundDir, szSoundSetType, CString(""));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object.
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlaySoundFromObject(HOBJECT hObject, char *pName, LTFLOAT fRadius,
                                        SoundPriority ePriority, uint32 dwFlags, uint8 nVolume,
										float fPitchShift)
{
    if (!pName || !hObject) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB | PLAYSOUND_ATTACHED;
	psi.m_dwFlags |= dwFlags;

	if (psi.m_dwFlags & PLAYSOUND_CLIENTLOCAL)
	{
		psi.m_dwFlags &= ~PLAYSOUND_3D;
	}

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_fOuterRadius	= fRadius;
	psi.m_fInnerRadius	= fRadius * m_fInnerRadiusPercent;
	psi.m_nVolume		= nVolume;
	psi.m_hObject		= hObject;
	psi.m_fPitchShift	= fPitchShift;
	psi.m_vPosition		= GetObjectPos(hObject);

	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlaySoundFromObject(HOBJECT hObject, const char *szBute)
{
    if (!szBute || !hObject) return LTNULL;

	//get the set index and call the other play sound function
	int nBute = g_pSoundButeMgr->GetSoundSetFromName(szBute);

	if(nBute == -1) return LTNULL;

	return PlaySoundFromObject( hObject, nBute );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlaySoundFromObject(HOBJECT hObject, uint32 nBute)
{
    if (!hObject) return LTNULL;

	SoundButes butes = g_pSoundButeMgr->GetSoundButes(nBute);
	const char * szFile = g_pSoundButeMgr->GetRandomSoundFileWeighted(nBute);

	if(!szFile) return LTNULL;

	return PlayAttachedSound(hObject,butes,szFile);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays named sound attached to object using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlayAttachedSound(HOBJECT hObject, const SoundButes & butes, const char * szFileName )
{
	if(!szFileName) return LTNULL;

	// check for the play chance
	if(butes.m_fPlayChance < 1.0f)
	{
		if(GetRandom(0.0f,1.0f) > butes.m_fPlayChance)
			return LTNULL;
	}

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = butes.m_nFlags;

	if( hObject )
		psi.m_dwFlags |= PLAYSOUND_ATTACHED;

	strncpy(psi.m_szSoundName, szFileName, _MAX_PATH);
	psi.m_nPriority		= butes.m_ePriority;
	psi.m_fOuterRadius	= butes.m_fOuterRad;
	psi.m_fInnerRadius	= butes.m_fInnerRad;
	psi.m_nVolume		= butes.m_nVolume;
	psi.m_hObject		= hObject;
	psi.m_fPitchShift	= butes.m_fPitch;
	psi.m_vPosition		= GetObjectPos(hObject);

	return PlaySound(psi);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlayLocalSound()
//
//	PURPOSE:	Plays a sound locally.
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlayLocalSound(const SoundButes & butes, const char * szFileName )
{
	if(!szFileName) return LTNULL;

	// check for the play chance
	if(butes.m_fPlayChance < 1.0f)
	{
		if(GetRandom(0.0f,1.0f) > butes.m_fPlayChance)
			return LTNULL;
	}

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = butes.m_nFlags;

	psi.m_dwFlags &= ~(PLAYSOUND_ATTACHED | PLAYSOUND_CLIENTLOCAL | PLAYSOUND_3D);
	psi.m_dwFlags |= PLAYSOUND_LOCAL;

	strncpy(psi.m_szSoundName, szFileName, _MAX_PATH);
	psi.m_nPriority		= butes.m_ePriority;
	psi.m_fOuterRadius	= butes.m_fOuterRad;
	psi.m_fInnerRadius	= butes.m_fInnerRad;
	psi.m_nVolume		= butes.m_nVolume;
	psi.m_fPitchShift	= butes.m_fPitch;

	return PlaySound(psi);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromObject()
//
//	PURPOSE:	Plays sound attached to object using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlayCharSndFromObject(uint32 nCharBute, HOBJECT hObject, const char *szBute, LTBOOL bPreventLipSyncing /* = LTFALSE */)
{
    if (!szBute || !hObject) return LTNULL;

	//get the set index and call the other play sound function


	CString strSoundSetName = GetSoundSetName( g_pCharacterButeMgr->GetGeneralSoundDir(nCharBute), szBute );

	int nBute = g_pSoundButeMgr->GetSoundSetFromName( strSoundSetName );
	if(nBute == -1) return LTNULL;

	const char * szSoundFileName = g_pSoundButeMgr->GetRandomSoundFileWeighted(nBute);
	const SoundButes & butes = g_pSoundButeMgr->GetSoundButes(nBute);

	return PlayAttachedSound(hObject, butes, szSoundFileName);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlaySoundFromPos(LTVector & vPos, char *pName, LTFLOAT fRadius,
                                     SoundPriority ePriority, uint32 dwFlags,
									 uint8 nVolume, float fPitchShift)
{
    if (!pName) return LTNULL;

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;
	psi.m_dwFlags |= dwFlags;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	strncpy(psi.m_szSoundName, pName, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_fOuterRadius	= fRadius;
	psi.m_fInnerRadius	= fRadius * m_fInnerRadiusPercent;
	psi.m_nVolume		= nVolume;
	psi.m_vPosition		= vPos;
	psi.m_fPitchShift	= fPitchShift;

	return PlaySound(psi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position using bute
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlaySoundFromPos(LTVector & vPos, const char *szBute, LTFLOAT fIRad, LTFLOAT fORad)
{
    if (!szBute) return LTNULL;

	//get the set index and call the other play sound function
	int nBute = g_pSoundButeMgr->GetSoundSetFromName(szBute);

	if(nBute == -1) return LTNULL;

	//get the set index and call the other play sound function
	return PlaySoundFromPos(vPos, nBute, fIRad, fORad);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundFromPos()
//
//	PURPOSE:	Plays sound from a specific position
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlaySoundFromPos(LTVector & vPos, uint32 nBute, LTFLOAT fIRad, LTFLOAT fORad)
{
	if(nBute == -1) return LTNULL;

	SoundButes butes = g_pSoundButeMgr->GetSoundButes(nBute);

	// check for the play chance
	if(butes.m_fPlayChance < 1.0f)
	{
		if(GetRandom(0.0f,1.0f) > butes.m_fPlayChance)
			return LTNULL;
	}

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = butes.m_nFlags;

	const char *szSound = g_pSoundButeMgr->GetRandomSoundFile(nBute);

	if(szSound)
	{
		strncpy(psi.m_szSoundName, szSound, _MAX_PATH);
		psi.m_nPriority		= butes.m_ePriority;
		psi.m_fOuterRadius	= (fORad == -1.0f) ? butes.m_fOuterRad : fORad;
		psi.m_fInnerRadius	= (fIRad == -1.0f) ? butes.m_fInnerRad : fIRad;
		psi.m_nVolume		= butes.m_nVolume;
		psi.m_fPitchShift	= butes.m_fPitch;
		psi.m_vPosition		= vPos;

		return PlaySound(psi);
	}

	return LTNULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMgr::PlaySoundLocal()
//
//	PURPOSE:	Plays sound from localling inside the player's head.
//
// ----------------------------------------------------------------------- //

HLTSOUND CSoundMgr::PlaySoundLocal( char const* pszSound, SoundPriority ePriority, uint32 dwFlags,
									 uint8 nVolume, float fPitchShift, UserSoundTypes eUserSoundType )
{
	// Check inputs.
    if( !pszSound )
	{
		ASSERT( FALSE );
		return NULL;
	}

	PlaySoundInfo psi;
	PLAYSOUNDINFO_INIT(psi);

	psi.m_dwFlags = PLAYSOUND_LOCAL;
	psi.m_dwFlags |= dwFlags;

	if (nVolume < 100)
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_VOL;
	}

	if( eUserSoundType != 0 )
	{
		psi.m_dwFlags |= PLAYSOUND_CTRL_TYPE;
	}

	strncpy(psi.m_szSoundName, pszSound, _MAX_PATH);
	psi.m_nPriority		= ePriority;
	psi.m_nVolume		= nVolume;
	psi.m_fPitchShift	= fPitchShift;
	psi.m_nUserSoundType = eUserSoundType;

	return PlaySound(psi);
}
