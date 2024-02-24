 // ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.h
//
// PURPOSE : Mark special fx class - Definition
//
// CREATED : 11/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __MARKSFX_H__
#define __MARKSFX_H__

#include "SpecialFX.h"
#include "ltlink.h"

struct MARKCREATESTRUCT : public SFXCREATESTRUCT
{
    MARKCREATESTRUCT();

    LTRotation   m_Rotation;
    LTVector     m_vPos;
    LTFLOAT      m_fScale;
    uint8       nAmmoId;
    uint8       nSurfaceType;
};

inline MARKCREATESTRUCT::MARKCREATESTRUCT()
{
    m_Rotation.Init();
	m_vPos.Init();
	m_fScale		= 0.0f;
	nAmmoId			= 0;
	nSurfaceType	= 0;
}


class CMarkSFX : public CSpecialFX
{
	public :

		CMarkSFX()
		{
            m_Rotation.Init();
			VEC_INIT(m_vPos);
			m_fScale = 1.0f;
			m_nAmmoId = 0;
			m_nSurfaceType = 0;
			m_fStartTime = 0.0f;
		}

		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

        virtual void WantRemove(LTBOOL bRemove=LTTRUE);
		virtual void SetMode(const std::string& mode);

	private :

        LTRotation   m_Rotation;
        LTVector     m_vPos;
        LTFLOAT      m_fScale;
        uint8       m_nAmmoId;
        uint8       m_nSurfaceType;
		LTFLOAT		m_fStartTime;
};

//-------------------------------------------------------------------------------------------------
// SFX_MarkFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_MarkFactory : public CSpecialFXFactory
{
	SFX_MarkFactory() : CSpecialFXFactory(SFX_MARK_ID) {;}
	static const SFX_MarkFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __MARKSFX_H__