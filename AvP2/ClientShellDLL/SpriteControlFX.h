// ----------------------------------------------------------------------- //
//
// MODULE  : SpriteControlFX.h
//
// PURPOSE : Sprite controller
//
// CREATED : 7/16/00
//
// ----------------------------------------------------------------------- //

#ifndef __SPRITE_CONTROL_FX_H__
#define __SPRITE_CONTROL_FX_H__

//-------------------------------------------------------------------------------------------------

#include "SpecialFX.h"

//-------------------------------------------------------------------------------------------------

struct SPRCTRLCREATESTRUCT : public SFXCREATESTRUCT
{
	SPRCTRLCREATESTRUCT();

	uint8	nPlayMode;
	uint8	nStartFrame;
	uint8	nEndFrame;
	uint8	nFramerate;
};

inline SPRCTRLCREATESTRUCT::SPRCTRLCREATESTRUCT()
{
	nPlayMode		= 0;
	nStartFrame		= 0;
	nEndFrame		= 0;
	nFramerate		= 0;
}

//-------------------------------------------------------------------------------------------------

class CSpriteControlFX : public CSpecialFX
{
	public :

		CSpriteControlFX() : CSpecialFX() 
		{
			m_nPlayMode		= 0;
			m_nStartFrame	= 0;
			m_nEndFrame		= 0;
		}

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT *psfx);
		virtual LTBOOL Update();

		virtual LTBOOL OnServerMessage(HMESSAGEREAD hRead);

	protected :

		// Property variables
		uint8		m_nPlayMode;
		uint8		m_nStartFrame;
		uint8		m_nEndFrame;
		uint8		m_nFramerate;

		// Helper variables
		LTFLOAT		m_fFrameUpdateTime;
};


//-------------------------------------------------------------------------------------------------
// SFX_SpriteControlFactory
//-------------------------------------------------------------------------------------------------

#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_SpriteControlFactory : public CSpecialFXFactory
{
	SFX_SpriteControlFactory() : CSpecialFXFactory(SFX_SPRITE_CONTROL_ID) {;}
	static const SFX_SpriteControlFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __SPRITE_CONTROL_FX_H__

