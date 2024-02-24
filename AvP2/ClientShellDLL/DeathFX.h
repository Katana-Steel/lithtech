// ----------------------------------------------------------------------- //
//
// MODULE  : DeathFX.h
//
// PURPOSE : Death special fx class - Definition
//
// CREATED : 6/14/98
//
// ----------------------------------------------------------------------- //

#ifndef __DEATH_FX_H__
#define __DEATH_FX_H__

#include "SpecialFX.h"
#include "ContainerCodes.h"
#include "CharacterAlignment.h"
#include "GibFX.h"

struct DEATHCREATESTRUCT : public SFXCREATESTRUCT
{
	DEATHCREATESTRUCT::DEATHCREATESTRUCT();

	DDWORD		nCharacterSet;
	DBYTE		nDeathType;
	DVector		vPos;
	DVector		vDir;
};

inline DEATHCREATESTRUCT::DEATHCREATESTRUCT()
{
	nCharacterSet = -1;
	nDeathType		= 0;
	vPos.Init();
	vDir.Init();
}


class CDeathFX : public CSpecialFX
{
	public :

		CDeathFX() : CSpecialFX() 
		{
			m_eCode				= CC_NO_CONTAINER;
			m_nDeathType		= 0;
			m_nCharacterSet		= 1;
			VEC_INIT(m_vPos);
			VEC_INIT(m_vDir);
		}

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);
		virtual DBOOL Update() { return DFALSE; }

	protected :
	
		ContainerCode	m_eCode;			// Container effect is in
		DVector			m_vPos;				// Effect position
		DVector			m_vDir;				// Direction damage came from
		DBYTE			m_nDeathType;		// Type of death
		DDWORD			m_nCharacterSet;	// Character Set
	
		void CreateDeathFX();
		void CreateHumanDeathFX();

		void SetupGibTypes(GIBCREATESTRUCT & gib);
};

//-------------------------------------------------------------------------------------------------
// SFX_DeathFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_DeathFactory : public CSpecialFXFactory
{
	SFX_DeathFactory() : CSpecialFXFactory(SFX_DEATH_ID) {;}
	static const SFX_DeathFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __DEATH_FX_H__