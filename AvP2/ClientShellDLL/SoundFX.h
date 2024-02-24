// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.h
//
// PURPOSE : Sound fx - Definition
//
// CREATED : 6/20/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_FX_H__
#define __SOUND_FX_H__

#include "SpecialFX.h"
#include "ClientSoundMgr.h"

struct SNDCREATESTRUCT : public SFXCREATESTRUCT
{
    SNDCREATESTRUCT();

    LTVector		vPos;
	LTBOOL			bLocal;
	LTBOOL			bLoop;
	LTFLOAT			fRadius;
	LTFLOAT			fPitchShift;
	char*			pSndName;
	uint8			nVolume;
	SoundPriority	ePriority;
};

inline SNDCREATESTRUCT::SNDCREATESTRUCT()
{
	vPos.Init();
	fRadius		= 0.0f;
	fPitchShift	= 1.0f;
	bLocal		= LTFALSE;
	bLoop		= LTFALSE;
	pSndName	= LTNULL;
	nVolume		= SMGR_DEFAULT_VOLUME;
	ePriority	= SOUNDPRIORITY_MISC_LOW;
}


class CSoundFX : public CSpecialFX
{
	public :

		CSoundFX() : CSpecialFX()
		{
			m_hSnd = LTNULL;
		}

		~CSoundFX()
		{
			Stop();
		}

		void Play();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		inline void	Stop() 
		{
			if (m_hSnd)
			{
				g_pLTClient->KillSoundLoop(m_hSnd);
				m_hSnd = LTNULL;
			}
		}

	private :

		SNDCREATESTRUCT	m_cs;
		HLTSOUND		m_hSnd;
};

//-------------------------------------------------------------------------------------------------
// SFX_SoundFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_SoundFactory : public CSpecialFXFactory
{
	SFX_SoundFactory() : CSpecialFXFactory(SFX_SOUND_ID) {;}
	static const SFX_SoundFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __SOUND_FX_H__
