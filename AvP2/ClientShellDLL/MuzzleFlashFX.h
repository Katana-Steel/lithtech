// ----------------------------------------------------------------------- //
//
// MODULE  : MuzzleFlashFX.h
//
// PURPOSE : MuzzleFlash special fx class - Definition
//
// CREATED : 12/17/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MUZZLE_FLASH_FX_H__
#define __MUZZLE_FLASH_FX_H__

#include "SpecialFX.h"
#include "MuzzleFlashParticleFX.h"
#include "DynamicLightFX.h"
#include "BaseScaleFX.h"

struct BARREL;

struct MUZZLEFLASHCREATESTRUCT : public SFXCREATESTRUCT
{
    MUZZLEFLASHCREATESTRUCT();

	BARREL*		pBarrel;		// Barrel data
	HOBJECT		hParent;		// Character or Player model if bPlayerView = TRUE
	uint8		nFlashSocket;   // Used to get the name of the flash socket from barrel data's FlashSockets array.
    LTVector     vPos;           // Initial pos
    LTRotation   rRot;           // Initial rotation
    LTBOOL       bPlayerView;    // Is this fx tied to the client-side player view
};

inline MUZZLEFLASHCREATESTRUCT::MUZZLEFLASHCREATESTRUCT()
{
    rRot.Init();
	vPos.Init();
	nFlashSocket = 0;
    hParent     = LTNULL;
	pBarrel		= LTNULL;
    bPlayerView = LTFALSE;
}


class CMuzzleFlashFX : public CSpecialFX
{
	public :

		CMuzzleFlashFX() : CSpecialFX()
		{
            m_bUsingParticles   = LTFALSE;
            m_bUsingScale       = LTFALSE;
            m_bUsingLight       = LTFALSE;
            m_bHidden           = LTFALSE;
			m_fWashoutAmount	= 0.0f;
		}

        LTBOOL Setup(MUZZLEFLASHCREATESTRUCT & cs);

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		void Hide();
		void Show(LTBOOL bNewShot);
        void SetPos(LTVector vWorldPos, LTVector vCamRelPos);
        void SetRot(LTRotation rRot);

	private :

        LTBOOL   Reset(MUZZLEFLASHCREATESTRUCT & cs);
        LTBOOL   ResetFX();
		void	ReallyHide();


		MUZZLEFLASHCREATESTRUCT		m_cs;		// Our data

		CMuzzleFlashParticleFX		m_Particle; // Particle system
		CBaseScaleFX				m_Scale;	// Model/Sprite
		CDynamicLightFX				m_Light;	// Dynamic light

        LTBOOL                      m_bUsingParticles;
        LTBOOL                      m_bUsingScale;
        LTBOOL                      m_bUsingLight;
        LTBOOL                      m_bHidden;
		LTFLOAT						m_fWashoutAmount;
};

//-------------------------------------------------------------------------------------------------
// SFX_MuzzleFlashFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_MuzzleFlashFactory : public CSpecialFXFactory
{
	SFX_MuzzleFlashFactory() : CSpecialFXFactory(SFX_MUZZLEFLASH_ID) {;}
	static const SFX_MuzzleFlashFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __MUZZLE_FLASH_FX_H__