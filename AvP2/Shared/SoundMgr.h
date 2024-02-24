// ----------------------------------------------------------------------- //
//
// MODULE  : SoundMgr.h
//
// PURPOSE : SoundMgr definition - Controls sound
//
// CREATED : 2/08/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_MGR_H__
#define __SOUND_MGR_H__

// ----------------------------------------------------------------------- //

#include "SoundTypes.h"
#include "HierarchicalButeMgr.h"
#include "ltbasedefs.h"

// ----------------------------------------------------------------------- //
// #defines...

#define SMGR_DEFAULT_VOLUME		100
#define SMGR_INVALID_CHARACTER	0xffff

// ----------------------------------------------------------------------- //

struct SoundButes;

// ----------------------------------------------------------------------- //

class CSoundMgr : public CHierarchicalButeMgr
{
	public :

		virtual LTBOOL Init(ILTCSBase *pInterface, const char* szAttributeFile);

		// Play sound from object variations

		HLTSOUND PlaySoundFromObject(HOBJECT hObject, char *pName, LTFLOAT fRadius,
			SoundPriority ePriority, uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f);

		// These will play a random sound from the sound butes.
		HLTSOUND PlaySoundFromObject(HOBJECT hObject, const char *szBute);
		HLTSOUND PlaySoundFromObject(HOBJECT hObject, uint32 nBute);

		// This plays the named sound file, using the given sound butes, set-up for as an attached sound.
		HLTSOUND PlayAttachedSound(HOBJECT hObject, const SoundButes & butes, const char * szFileName );

		// This plays the named sound file, using the given sound butes, set-up as a local sound.
		HLTSOUND PlayLocalSound(const SoundButes & butes, const char * szFileName );

		// Special play character sound from object variations

		HLTSOUND PlayCharSndFromObject(uint32 nCharBute, HOBJECT hObject, const char *szBute, LTBOOL bPreventLipSyncing = LTFALSE);


		// Play sound from position variations

		HLTSOUND PlaySoundFromPos(LTVector & vPos, char *pName, LTFLOAT fRadius,
			SoundPriority ePriority, uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f);

		HLTSOUND PlaySoundFromPos(LTVector & vPos, const char *szBute, LTFLOAT fIRad = -1.0f, LTFLOAT fORad = -1.0f);
		HLTSOUND PlaySoundFromPos(LTVector & vPos, uint32 nBute, LTFLOAT fIRad = -1.0f, LTFLOAT fORad = -1.0f);

		// Play a sound locally.
		HLTSOUND PlaySoundLocal( char const* pszSound, SoundPriority ePriority, 
			uint32 dwFlags = 0, uint8 nVolume = SMGR_DEFAULT_VOLUME, float fPitchShift = 1.0f,
			UserSoundTypes eUserSoundType = USERSOUNDTYPE_SOUNDEFFECT );

		// Helper functions

		CString		GetSoundSetName(const char* szSoundDir, const char* szSoundSetType);

		LTFLOAT		GetInnerRadiusPercent() const { return m_fInnerRadiusPercent; }

	protected :
		
		CSoundMgr();
		virtual ~CSoundMgr();
		
		virtual	HLTSOUND PlaySound(PlaySoundInfo & playSoundInfo) = 0;
		virtual	LTVector GetObjectPos(HOBJECT hObj) = 0;
		
		ILTCSBase*	m_pInterface;
		LTFLOAT		m_fInnerRadiusPercent;  // Percent of outer radius used for inner radius
};

// ----------------------------------------------------------------------- //

#endif // __SOUND_MGR_H__

