// ----------------------------------------------------------------------- //
//
// MODULE  : NodeLinesFX.h
//
// PURPOSE : NodeLines special fx class - Definition
//
// CREATED : 2/10/99
//
// ----------------------------------------------------------------------- //

#ifndef __NODELINES_FX_H__
#define __NODELINES_FX_H__

#include "BaseScaleFX.h"

struct NLCREATESTRUCT : public SFXCREATESTRUCT
{
	NLCREATESTRUCT::NLCREATESTRUCT();

	DVector		vSource;
	DVector		vDestination;
};

inline NLCREATESTRUCT::NLCREATESTRUCT()
{
	vSource.Init();
	vDestination.Init();
}

class CNodeLinesFX : public CSpecialFX
{
	public :

		CNodeLinesFX() : CSpecialFX() 
		{
			VEC_INIT(m_vSource);
			VEC_INIT(m_vDestination);
			m_pFX = DNULL;
		}

		~CNodeLinesFX()
		{
			RemoveFX();
		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual void  RemoveFX();
		virtual DBOOL Update();

	protected :

		DBOOL			m_bFirstUpdate;
		DVector			m_vSource;
		DVector			m_vDestination;
		CBaseScaleFX*	m_pFX;
};

//-------------------------------------------------------------------------------------------------
// SFX_NodeLinesFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_NodeLinesFactory : public CSpecialFXFactory
{
	SFX_NodeLinesFactory() : CSpecialFXFactory(SFX_NODELINES_ID) {;}
	static const SFX_NodeLinesFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __NODELINES_FX_H__