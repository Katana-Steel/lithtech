// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFX.h
//
// PURPOSE : Debris - Definition
//
// CREATED : 5/31/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEBRIS_FX_H__
#define __DEBRIS_FX_H__

#include "SpecialFX.h"
#include "client_physics.h"
#include "DebrisMgr.h"
#include "FrameRateMgr.h"

#define MAX_DEBRIS 20

struct DEBRISCREATESTRUCT : public SFXCREATESTRUCT
{
    DEBRISCREATESTRUCT();

    LTRotation   rRot;
    LTVector     vPos;
    LTVector     vMinVel;
    LTVector     vMaxVel;
    LTVector     vMinDOffset;
    LTVector     vMaxDOffset;
    LTVector     vVelOffset;
    LTFLOAT      fMinLifeTime;
    LTFLOAT      fMaxLifeTime;
    LTFLOAT      fFadeTime;
    uint8       nNumDebris;
    LTBOOL       bRotate;
    uint8       nDebrisId;
    uint8       nMinBounce;
    uint8       nMaxBounce;
    LTFLOAT      fMinScale;
    LTFLOAT      fMaxScale;
    LTFLOAT      fGravityScale;
    LTFLOAT      fAlpha;
    LTBOOL       bPlayBounceSound;
    LTBOOL       bPlayExplodeSound;
    LTBOOL       bForceRemove;
    LTBOOL       bDirOffsetOnly;
};

inline DEBRISCREATESTRUCT::DEBRISCREATESTRUCT()
{
	rRot.Init();
	vPos.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vMinDOffset.Init();
	vMaxDOffset.Init();
	vVelOffset.Init();
	fMinLifeTime		= 0.0f;
	fMaxLifeTime		= 0.0f;
	fFadeTime			= 0.0f;
	nNumDebris			= 0;
    bRotate             = LTFALSE;
	nDebrisId			= DEBRISMGR_INVALID_ID;
	nMinBounce			= 0;
	nMaxBounce			= 0;
	fMinScale			= 0.0f;
	fMaxScale			= 0.0f;
	fAlpha				= 1.0f;
	fGravityScale		= 1.0f;
    bPlayBounceSound    = LTTRUE;
    bPlayExplodeSound   = LTTRUE;
    bForceRemove        = LTFALSE;
}

class CDebrisFX : public CSpecialFX
{
	public :

		CDebrisFX() : CSpecialFX()
		{
            m_bFirstUpdate  = LTTRUE;
			m_fLastTime		= -1.0f;
			m_fStartTime	= -1.0f;
			m_FRState		= FR_UNPLAYABLE;

			memset(m_Emitters, 0, sizeof(MovingObject)*MAX_DEBRIS);
            memset(m_ActiveEmitters, 0, sizeof(LTBOOL)*MAX_DEBRIS);
            memset(m_BounceCount, 0, sizeof(uint8)*MAX_DEBRIS);
			memset(m_hDebris, 0, sizeof(HOBJECT)*MAX_DEBRIS);
            memset(m_fPitch, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
            memset(m_fYaw, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
            memset(m_fRoll, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
            memset(m_fPitchVel, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
            memset(m_fYawVel, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
            memset(m_fRollVel, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
            memset(m_fDebrisLife, 0, sizeof(LTFLOAT)*MAX_DEBRIS);
            memset(m_nDebrisFlags, 0, sizeof(uint16)*MAX_DEBRIS);
		}

		~CDebrisFX()
		{
			for (int i=0; i < m_ds.nNumDebris; i++)
			{
				if (m_hDebris[i] && m_pClientDE)
				{
					m_pClientDE->DeleteObject(m_hDebris[i]);
				}
			}
		}

        virtual LTBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL	Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL	CreateObject(ILTClient* pClientDE);
        virtual LTBOOL	Update();

	protected :

		DEBRISCREATESTRUCT	m_ds;

        LTFLOAT			m_fStartTime;   // When did we start this crazy thing
        LTFLOAT			m_fLastTime;    // Last time we created some particles
        LTBOOL			m_bFirstUpdate; // First update
		FRState			m_FRState;		// Our current fram rate state

		MovingObject	m_Emitters[MAX_DEBRIS];				// Debris emitters
        LTBOOL			m_ActiveEmitters[MAX_DEBRIS];       // Active?
        uint8			m_BounceCount[MAX_DEBRIS];          // Number of bounces
		HOBJECT			m_hDebris[MAX_DEBRIS];
        LTFLOAT			m_fDebrisLife[MAX_DEBRIS];
        uint16			m_nDebrisFlags[MAX_DEBRIS];


		// Emitter rotation stuff...

        LTFLOAT			m_fPitch[MAX_DEBRIS];
        LTFLOAT			m_fYaw[MAX_DEBRIS];
        LTFLOAT			m_fRoll[MAX_DEBRIS];
        LTFLOAT			m_fPitchVel[MAX_DEBRIS];
        LTFLOAT			m_fYawVel[MAX_DEBRIS];
        LTFLOAT			m_fRollVel[MAX_DEBRIS];

        virtual LTBOOL	UpdateEmitter(MovingObject* pObject, LTBOOL & bRemove, int i=0);

        virtual LTBOOL	IsValidDebris(int i);
        virtual void	CreateDebris(int i, LTVector vPos) {}
        virtual LTBOOL	OkToRemoveDebris(int i);
		virtual void	RemoveDebris(int i);
		virtual void	RotateDebrisToRest(int i);
        virtual void	SetDebrisPos(int i, LTVector vPos);
        virtual LTBOOL	GetDebrisPos(int i, LTVector & vPos);
        virtual void	SetDebrisRot(int i, LTRotation rRot);
		virtual void	SetDebrisAlpha(int i, LTFLOAT fAlpha);
		virtual void	SetFrameRateMod();

};

//-------------------------------------------------------------------------------------------------
// SFX_DebrisFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_DebrisFactory : public CSpecialFXFactory
{
	SFX_DebrisFactory() : CSpecialFXFactory(SFX_DEBRIS_ID) {;}
	static const SFX_DebrisFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __DEBRIS_FX_H__
