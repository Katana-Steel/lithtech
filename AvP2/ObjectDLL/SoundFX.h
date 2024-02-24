// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.h
//
// PURPOSE : A start/stoppable ambient sound object.
//
// CREATED : 09/11/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_FX_H__
#define __SOUND_FX_H__

#include "cpp_engineobjects_de.h"

class SoundFX : public BaseClass
{
	public :

 		SoundFX();
		~SoundFX();

	protected :

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
        DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	protected : 

        void		HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);
        DBOOL		ReadProp(ObjectCreateStruct *pData);
        void		PostPropRead(ObjectCreateStruct *pStruct);
        DBOOL		InitialUpdate();
        DBOOL		Update();

		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void		PlaySound();
		void		CacheFiles();

		// Member Variables

		DBOOL		m_bStartOn;
		HSTRING		m_hstrSound;
		HSOUNDDE	m_hsndSound;
		float		m_fOuterRadius;
		float		m_fInnerRadius;
		DBYTE		m_nVolume;
		float		m_fPitchShift;
		DBOOL		m_bAmbient;
		DBOOL		m_bLooping;
		unsigned char m_nPriority;
};

#endif // __SOUND_FX_H__


