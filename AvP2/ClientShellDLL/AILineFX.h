// ----------------------------------------------------------------------- //
//
// MODULE  : AILineFX.h
//
// PURPOSE : AILine special fx class -- Provides line system for debugging AI's.
//
// CREATED : 3/27/00
//
// ----------------------------------------------------------------------- //

#ifndef __AILINE_FX_H__
#define __AILINE_FX_H__

#include "BaseLineSystemFX.h"

struct AILCREATESTRUCT : public SFXCREATESTRUCT
{
	AILCREATESTRUCT()
		: vSource(0.0f,0.0f,0.0f),
		  vDest(0.0f,0.0f,0.0f),
		  vColor(1.0f,1.0f,1.0f),
		  fAlpha(0.0f) {}

	DVector vSource;
	DVector vDest;
	DVector vColor;
	DFLOAT	fAlpha;
};



class CAILineFX : public CBaseLineSystemFX
{
	public :

		CAILineFX() 
			: m_bFirstUpdate(DTRUE) {}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();

	protected :

		AILCREATESTRUCT m_InitData;

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

class SFX_AILineFactory : public CSpecialFXFactory
{
	SFX_AILineFactory() : CSpecialFXFactory(SFX_AILINE_ID) {;}
	static const SFX_AILineFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __AILINE_FX_H__