// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponSoundFX.h
//
// PURPOSE : Weapon Sound fx - Definition
//
// CREATED : 10/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_SOUND_FX_H__
#define __WEAPON_SOUND_FX_H__

#include "SpecialFX.h"

struct WSOUNDCREATESTRUCT : public SFXCREATESTRUCT
{
	WSOUNDCREATESTRUCT::WSOUNDCREATESTRUCT();

	DVector		vPos;
	DBYTE		nClientId;
	DBYTE		nType;
	DBYTE		nWeaponId;
	DBYTE		nBarrelId;
};

inline WSOUNDCREATESTRUCT::WSOUNDCREATESTRUCT()
{
	vPos.Init();
	nClientId	= 0;
	nType		= 0;
	nWeaponId	= 0;
	nBarrelId	= 0;
}

class CWeaponSoundFX : public CSpecialFX
{
	public :

		CWeaponSoundFX() : CSpecialFX() 
		{
			VEC_INIT(m_vPos);
			m_nClientId	= 0;
			m_nType		= 0;
			m_nWeaponId	= 0;
			m_nBarrelId	= 0;
		}

		~CWeaponSoundFX()
		{
		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update() { return DFALSE; }

	private :

		DVector	m_vPos;
		DBYTE	m_nClientId;
		DBYTE	m_nType;
		DBYTE	m_nWeaponId;
		DBYTE	m_nBarrelId;
};

//-------------------------------------------------------------------------------------------------
// SFX_WeaponSoundFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_WeaponSoundFactory : public CSpecialFXFactory
{
	SFX_WeaponSoundFactory() : CSpecialFXFactory(SFX_WEAPONSOUND_ID) {;}
	static const SFX_WeaponSoundFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __WEAPON_SOUND_FX_H__