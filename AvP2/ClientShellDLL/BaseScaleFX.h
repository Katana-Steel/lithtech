 // ----------------------------------------------------------------------- //
//
// MODULE  : BaseScaleFX.h
//
// PURPOSE : BaseScale special fx class - Definition
//
// CREATED : 5/27/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_SCALE_FX_H__
#define __BASE_SCALE_FX_H__

#include "SpecialFX.h"
#include "client_physics.h"
#include "ltBaseDefs.h"
#include "FrameRateMgr.h"
#include <string>

extern enum FRState;

enum BSPriority
{
	BS_PRIORITY_HIGH = 0,
	BS_PRIORITY_MED,
	BS_PRIORITY_LOW,
};

struct BSCREATESTRUCT : public SFXCREATESTRUCT
{
    BSCREATESTRUCT();

    LTRotation		rRot;
    LTVector		vPos;
    LTVector		vVel;
    LTVector		vInitialScale;
    LTVector		vFinalScale;
    LTVector		vInitialColor;
    LTVector		vFinalColor;
    LTFLOAT			fLifeTime;
    LTFLOAT			fDelayTime;
    LTFLOAT			fInitialAlpha;
    LTFLOAT			fFinalAlpha;
    LTFLOAT			fMinRotateVel;
    LTFLOAT			fMaxRotateVel;
	std::string		Filename;
	std::string		Skins[MAX_MODEL_TEXTURES];
    uint32			dwFlags;
    LTBOOL			bUseUserColors;
    LTBOOL			bLoop;
    LTBOOL			bAdditive;
    LTBOOL			bMultiply;
    LTBOOL			bRotate;
    LTBOOL			bRotateLeft;
    LTBOOL			bFaceCamera;
    uint8			nRotationAxis;
    uint8			nType;
    LTFLOAT			fFadeOutTime;
	BSPriority		ePriority;
};

inline BSCREATESTRUCT::BSCREATESTRUCT()
{
	fLifeTime		= 0.0f;
	fDelayTime		= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	fMinRotateVel	= 1.0f;
	fMaxRotateVel	= 1.0f;
	fFadeOutTime	= 0.0f;
	dwFlags			= 0;
    bUseUserColors  = LTFALSE;
    bLoop           = LTFALSE;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
    bRotate         = LTFALSE;
    bFaceCamera     = LTFALSE;
	nRotationAxis	= 0;
	nType			= OT_SPRITE;
	rRot.Init();
	vPos.Init();
	vVel.Init();
	vInitialScale.Init();
	vFinalScale.Init();
	vInitialColor.Init(-1.0f, -1.0f, -1.0f);
	vFinalColor.Init(-1.0f, -1.0f, -1.0f);
	ePriority		= BS_PRIORITY_HIGH;
}


class CBaseScaleFX : public CSpecialFX
{
public :

	CBaseScaleFX()
	{
		m_rRot.Init();
		m_vPos.Init();
		m_vVel.Init();
		m_vInitialScale.Init(1.0f, 1.0f, 1.0f);
		m_vFinalScale.Init(1.0f, 1.0f, 1.0f);
		m_vInitialColor.Init(1.0f, 1.0f, 1.0f);
		m_vFinalColor.Init(1.0f, 1.0f, 1.0f);

		m_fLifeTime		= 1.0f;
		m_fCSLifeTime	= 1.0f;
		m_fInitialAlpha	= 1.0f;
		m_fFinalAlpha	= 1.0f;
		m_fRotateVel	= 1.0f;

		m_fStartTime	= 0.0f;
		m_fDelayTime	= 0.0f;
		m_fEndTime		= 0.0f;
		m_fFadeOutTime	= 0.0f;

		m_dwFlags		= 0;
		m_nType			= 0;
		m_nRotDir		= 1;
        m_bLoop         = LTFALSE;
        m_bAdditive     = LTFALSE;
        m_bMultiply     = LTFALSE;
        m_bRotate       = LTFALSE;
        m_bFaceCamera   = LTFALSE;
		m_nRotationAxis = 0;

        m_bUseUserColors = LTFALSE;
		m_bWantRemove	= LTFALSE;
		m_ePriority		= BS_PRIORITY_HIGH;
		m_FRState		= FR_EXCELLENT;
	}

    virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
    virtual LTBOOL Update();
    virtual LTBOOL CreateObject(ILTClient* pClientDE);

	virtual void	Remove() { m_bWantRemove=LTTRUE; }

    virtual LTBOOL Reset();

	void	AdjustScale(LTFLOAT fScaleMultiplier);

	void	MakeInvisible(LTBOOL);

	void	SetFinalAlpha(LTFLOAT fFinalAlpha) { m_fFinalAlpha=fFinalAlpha;} 

protected :

    LTRotation		m_rRot;
					
    LTVector		m_vPos;
    LTVector		m_vVel;
    LTVector		m_vInitialScale;
    LTVector		m_vFinalScale;
    LTVector		m_vInitialColor;
    LTVector		m_vFinalColor;
					
    LTFLOAT			m_fLifeTime;
    LTFLOAT			m_fCSLifeTime;
    LTFLOAT			m_fDelayTime;
    LTFLOAT			m_fInitialAlpha;
    LTFLOAT			m_fFinalAlpha;
    LTFLOAT			m_fRotateVel;
					
	LTBOOL			m_bWantRemove;

	std::string		m_Filename;
	std::string		m_Skins[MAX_MODEL_TEXTURES];

    uint32			m_dwFlags;
    LTBOOL			m_bUseUserColors;
    LTBOOL			m_bLoop;
    LTBOOL			m_bAdditive;
    LTBOOL			m_bMultiply;
    LTBOOL			m_bRotate;
    LTBOOL			m_bFaceCamera;
	uint8			m_nRotationAxis;

	int				m_nRotDir;
	unsigned short	m_nType;
    LTFLOAT         m_fStartTime;
    LTFLOAT         m_fEndTime;
    LTFLOAT         m_fFadeOutTime;
	MovingObject	m_movingObj;
	BSPriority		m_ePriority;
	FRState			m_FRState;

    virtual void UpdateAlpha(LTFLOAT fTimeDelta);
    virtual void UpdateScale(LTFLOAT fTimeDelta);
    virtual void UpdatePos(LTFLOAT fTimeDelta);
    virtual void UpdateRot(LTFLOAT fTimeDelta);
	virtual void SetFrameRateMod();
};

//-------------------------------------------------------------------------------------------------
// SFX_ScaleFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ScaleFactory : public CSpecialFXFactory
{
	SFX_ScaleFactory() : CSpecialFXFactory(SFX_SCALE_ID) {;}
	static const SFX_ScaleFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __BASE_SCALE_FX_H__
