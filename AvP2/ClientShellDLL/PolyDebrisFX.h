// ----------------------------------------------------------------------- //
//
// MODULE  : PolyDebrisFX.h
//
// PURPOSE : Polygon Debris - Definition
//
// CREATED : 7/16/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __POLYGON_DEBRIS_FX_H__
#define __POLYGON_DEBRIS_FX_H__

#include "DebrisFX.h"
#include "PolyLineFX.h"
#include "FXStructs.h"

struct POLYDEBRISCREATESTRUCT : public SFXCREATESTRUCT
{
	POLYDEBRISCREATESTRUCT();

    LTVector         vPos;
    LTVector         vNormal;
    LTVector         vDir;

	CPolyDebrisFX	PolyDebrisFX;
};

inline POLYDEBRISCREATESTRUCT::POLYDEBRISCREATESTRUCT()
{
	vPos.Init();
	vNormal.Init();
	vDir.Init();
}

class CPolygonDebrisFX : public CDebrisFX
{
	public :

		CPolygonDebrisFX() : CDebrisFX()
		{
			m_nNumPolies = 0;
            memset(m_bValidPolies, 0, sizeof(LTBOOL)*MAX_DEBRIS);
		}

        virtual LTBOOL Update();
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

	protected :

		POLYDEBRISCREATESTRUCT m_cs;

		//CPolyLineFX		m_Polies;
		CPolyLineFX		m_Polies[MAX_DEBRIS];
        LTBOOL           m_bValidPolies[MAX_DEBRIS];

		int				m_nNumPolies;

        virtual LTBOOL   IsValidDebris(int i);
        virtual void    CreateDebris(int i, LTVector vPos);
        virtual LTBOOL   OkToRemoveDebris(int i);
		virtual void	RemoveDebris(int i);
		virtual void	RotateDebrisToRest(int i);
        virtual void    SetDebrisPos(int i, LTVector vPos);
        virtual LTBOOL   GetDebrisPos(int i, LTVector & vPos);
        virtual void    SetDebrisRot(int i, LTRotation rRot);
};

//-------------------------------------------------------------------------------------------------
// SFX_PolyDebrisFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_PolyDebrisFactory : public CSpecialFXFactory
{
	SFX_PolyDebrisFactory() : CSpecialFXFactory(SFX_POLYDEBRIS_ID) {;}
	static const SFX_PolyDebrisFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __POLYGON_DEBRIS_FX_H__
