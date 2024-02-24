// ----------------------------------------------------------------------- //
//
// MODULE  : CastLineFX.h
//
// PURPOSE : CastLine special fx class - Definition
//
// CREATED : 1/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __CAST_LINE_FX_H__
#define __CAST_LINE_FX_H__

#include "BaseLineSystemFX.h"

struct CLCREATESTRUCT : public SFXCREATESTRUCT
{
	CLCREATESTRUCT::CLCREATESTRUCT();

	DVector vStartColor;
	DVector vEndColor;
	DFLOAT	fStartAlpha;
	DFLOAT	fEndAlpha;
};

inline CLCREATESTRUCT::CLCREATESTRUCT()
{
	vStartColor.Init();
	vEndColor.Init();
	fStartAlpha	= 0.0f;
	fEndAlpha	= 0.0f;
}


class CCastLineFX : public CBaseLineSystemFX
{
	public :

		CCastLineFX() : CBaseLineSystemFX() 
		{
			VEC_SET(m_vStartColor, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vEndColor, 1.0f, 1.0f, 1.0f);
			m_fStartAlpha	= 1.0f;
			m_fEndAlpha		= 1.0f;

			m_bFirstUpdate = DTRUE;
		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	protected :

		DVector		m_vStartColor;
		DVector		m_vEndColor;
		DFLOAT		m_fStartAlpha;
		DFLOAT		m_fEndAlpha;

		DBOOL		m_bFirstUpdate;
};

//-------------------------------------------------------------------------------------------------
// SFX_CastLineFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_CastLineFactory : public CSpecialFXFactory
{
	SFX_CastLineFactory() : CSpecialFXFactory(SFX_CASTLINE_ID) {;}
	static const SFX_CastLineFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __CAST_LINE_FX_H__