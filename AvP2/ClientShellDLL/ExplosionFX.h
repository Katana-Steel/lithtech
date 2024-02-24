// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.h
//
// PURPOSE : Explosion special fx class - Definition
//
// CREATED : 12/29/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_FX_H__
#define __EXPLOSION_FX_H__

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class CExplosionFX : public CSpecialFX
{
	public :

		virtual DBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual DBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL CreateObject(CClientDE* pClientDE);

	protected :

		// Creation data...

		EXPLOSIONCREATESTRUCT	m_cs;		// Holds all initialization data
};

//-------------------------------------------------------------------------------------------------
// SFX_ExplosionFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_ExplosionFactory : public CSpecialFXFactory
{
	SFX_ExplosionFactory() : CSpecialFXFactory(SFX_EXPLOSION_ID) {;}
	static const SFX_ExplosionFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __EXPLOSION_FX_H__