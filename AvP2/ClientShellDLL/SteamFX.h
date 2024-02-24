// ----------------------------------------------------------------------- //
//
// MODULE  : SteamFX.h
//
// PURPOSE : Steam special fx class - Definition
//
// CREATED : 10/19/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STEAM_FX_H__
#define __STEAM_FX_H__

#include "SpecialFX.h"
#include "SmokeFX.h"
#include "SharedFXStructs.h"

class CSteamFX : public CSpecialFX
{
	public :

		CSteamFX();
		~CSteamFX()
		{
			if (m_hSound)
			{
                g_pLTClient->KillSound(m_hSound);
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

	protected :

		// Creation data...

		uint8				m_nSteamFXId;
		uint8				m_nSoundFXId;

		LTVector			m_vMinDrift;
		LTVector			m_vMaxDrift;

		CSmokeFX			m_Steam;
        HLTSOUND            m_hSound;
        uint32              m_dwLastUserFlags;

		void TweakSystem();
		void StartSound();
		void StopSound();
};

//-------------------------------------------------------------------------------------------------
// SFX_SteamFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_SteamFactory : public CSpecialFXFactory
{
	SFX_SteamFactory() : CSpecialFXFactory(SFX_STEAM_ID) {;}
	static const SFX_SteamFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __STEAM_FX_H__