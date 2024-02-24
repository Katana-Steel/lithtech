// ----------------------------------------------------------------------- //
//
// MODULE  : BaseLineSystemFX.h
//
// PURPOSE : BaseLineSystem special fx class - Definition
//
// CREATED : 1/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_LINE_SYSTEM_FX_H__
#define __BASE_LINE_SYSTEM_FX_H__

#include "SpecialFX.h"

class CBaseLineSystemFX : public CSpecialFX
{
	public :

		CBaseLineSystemFX() : CSpecialFX() 
		{
			VEC_INIT(m_vPos);
			ROT_INIT(m_rRot);
			m_rRot.m_Quat[3] = 0.0f;
		}

		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL Update();
		virtual DBOOL CreateObject(CClientDE* pClientDE);

		void	SetPos(const DVector & val)      { m_vPos = val; }
		void	SetRot(const DRotation & val)    { m_rRot = val; }
	private :

		DVector   m_vPos;
		DRotation m_rRot;
};

#endif // __BASE_LINE_SYSTEM_FX_H__