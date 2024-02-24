 // ----------------------------------------------------------------------- //
//
// MODULE  : PredTargetFx.h
//
// PURPOSE : Predator Target FX class - Definition
//
// CREATED : 6/13/2000
//
// ----------------------------------------------------------------------- //

#ifndef __PRED_TARGET_FX_H__
#define __PRED_TARGET_FX_H__

#include "SpecialFX.h"
#include "dlink.h"

enum TargetPhase
{
	TP_LOCKING_0 = 0,
	TP_LOCKING_1,
	TP_LOCKING_2,
	TP_LOCKED,
};

class CPredTargetSFX : public CSpecialFX
{
	public :

		CPredTargetSFX()
		{
			m_hImage			= LTNULL;
			m_fXPos				= 0.0f;
			m_fYPos				= 0.0f;
			m_fScale			= 0.0f;;
			m_fAngle			= 0.0f;;
			m_hLockedImage		= LTNULL;
			m_nHalfImageWidth	= 0;
			m_nHalfImageHeight	= 0;
			m_nScreenX			= 0;
			m_nScreenY			= 0;
			m_ePhase			= TP_LOCKING_0;
			m_fTime				= 0.0f;
			m_hLockSound		= LTNULL;
			memset(m_hTargetingImage, 0, sizeof(HSURFACE)*3);
		}
		~CPredTargetSFX()
		{
			//surfaces do not need to be free'd since they are
			//being managed by the interface surface manager.

			if(m_hLockSound)
			{
				g_pLTClient->KillSound(m_hLockSound);
				m_hLockSound = LTNULL;
			}
		}

		LTBOOL			Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		LTBOOL			Init(SFXCREATESTRUCT* psfxCreateStruct);
		LTBOOL			Update();
		LTBOOL			CreateObject(CClientDE* pClientDE);
		TargetPhase	UpdatePhase();
		void			DrawLockingTris();

		void	WantRemove(LTBOOL bRemove=DTRUE);
		void	PostRenderDraw();

	private :
		HSURFACE	m_hLockedImage;
		HSURFACE	m_hTargetingImage[3];
		HSURFACE	m_hImage;
		LTFLOAT		m_fXPos;
		LTFLOAT		m_fYPos;
		LTFLOAT		m_fScale;
		LTFLOAT		m_fAngle;
		uint32		m_nHalfImageWidth;
		uint32		m_nHalfImageHeight;
		uint32		m_nScreenX;
		uint32		m_nScreenY;
		TargetPhase	m_ePhase;
		LTFLOAT		m_fTime;
		HLTSOUND	m_hLockSound;
};

//-------------------------------------------------------------------------------------------------
// SFX_PredTargetFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_PredTargetFactory : public CSpecialFXFactory
{
	SFX_PredTargetFactory() : CSpecialFXFactory(SFX_PRED_TARGET_ID) {;}
	static const SFX_PredTargetFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __PRED_TARGET_FX_H__