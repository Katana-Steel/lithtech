// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.h
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 1/10/01
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_FX_OBJ_H__
#define __SOUND_FX_OBJ_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"
#include "SoundButeMgr.h"

// ----------------------------------------------------------------------- //

class SoundFXObj : public GameBase
{
	public :

 		SoundFXObj();
		~SoundFXObj();

	protected:

		uint32		EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32		ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private:

        void		HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);
        LTBOOL		ReadProp(ObjectCreateStruct *pData);
        void		PostPropRead(ObjectCreateStruct *pStruct);
        LTBOOL		InitialUpdate();
        LTBOOL		Update();

		void		Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);
		void		PlaySound();
		void		CacheFiles();

	protected:

		// Member Variables
		LTBOOL		m_bOn;

		int			m_nSoundFX;
		HLTSOUND	m_hsndSound;

		int			m_nNumPlays;
		LTFLOAT		m_fMinDelay;
		LTFLOAT		m_fMaxDelay;
		LTFLOAT		m_fInnerRadius;
		LTFLOAT		m_fOuterRadius;
		LTBOOL		m_bWait;

		LTFLOAT		m_fCurDelay;
		LTFLOAT		m_fLastOnUpdate;
		LTFLOAT		m_fLastPlayTime;
		LTFLOAT		m_fCurPlayLength;
};

// ----------------------------------------------------------------------- //

class CSoundFXObjPlugin : public IObjectPlugin
{
	public:

		virtual LTRESULT	PreHook_EditStringList(
			const char* szRezPath, 
			const char* szPropName, 
			char* const * aszStrings, 
			uint32* pcStrings, 
			const uint32 cMaxStrings, 
			const uint32 cMaxStringLength);

	private:

		CSoundButeMgrPlugin	m_SoundButeMgrPlugin;
};

// ----------------------------------------------------------------------- //

#endif // __SOUND_FX_OBJ_H__


