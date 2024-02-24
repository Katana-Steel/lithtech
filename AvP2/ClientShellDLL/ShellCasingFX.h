//----------------------------------------------------------
//
// MODULE  : SHELLCASINGFX.H
//
// PURPOSE : defines classes for ejected shells
//
// CREATED : 5/1/98
//
//----------------------------------------------------------

#ifndef __SHELLCASING_FX_H__
#define __SHELLCASING_FX_H__

#include "SpecialFX.h"
#include "ltlink.h"
#include "client_physics.h"


struct SHELLCREATESTRUCT : public SFXCREATESTRUCT
{
    SHELLCREATESTRUCT();

    LTRotation   rRot;
    LTVector     vStartPos;
    LTVector     vStartVel;
	LTVector     vOffset;
	uint8       nWeaponId;
    uint8       nAmmoId;
    uint32      dwFlags;
    LTBOOL       b3rdPerson;
};

inline SHELLCREATESTRUCT::SHELLCREATESTRUCT()
{
    rRot.Init();
	vStartPos.Init();
	vStartVel.Init();
	vOffset.Init();
	nWeaponId	= 0;
	nAmmoId		= 0;
	dwFlags		= 0;
    b3rdPerson  = LTFALSE;
}

class CShellCasingFX : public CSpecialFX
{
	public :

		CShellCasingFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

	private:

        LTFLOAT      m_fExpireTime;
        LTVector     m_vLastPos;
        LTVector     m_vStartVel;
        LTVector     m_vOffset;

		LTFLOAT		m_fStartTime;
		LTFLOAT		m_fStartDelay;

        LTFLOAT      m_fPitchVel;
        LTFLOAT      m_fYawVel;
        LTFLOAT      m_fPitch;
        LTFLOAT      m_fYaw;

        LTBOOL       m_bInVisible;
        int         m_nVisibleUpdate;
		int			m_nBounceCount;

        LTRotation   m_rRot;
        LTVector     m_vStartPos;
        uint8       m_nWeaponId;
        uint8       m_nAmmoId;
        LTBOOL       m_bResting;
        uint32      m_dwFlags;
        LTBOOL       m_b3rdPerson;

        LTFLOAT      m_fDieTime;
        LTVector     m_vScale;

		MovingObject m_movingObj;
};

//-------------------------------------------------------------------------------------------------
// SFX_ShellCasingFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ShellCasingFactory : public CSpecialFXFactory
{
	SFX_ShellCasingFactory() : CSpecialFXFactory(SFX_SHELLCASING_ID) {;}
	static const SFX_ShellCasingFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif  // __SHELLCASING_FX_H__

