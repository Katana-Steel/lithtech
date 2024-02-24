// ----------------------------------------------------------------------- //
//
// MODULE  : MultiDebrisFX.h
//
// PURPOSE : Debris - Definition
//
// CREATED : 4/24/1
//
// ----------------------------------------------------------------------- //

#ifndef __MULTI_DEBRIS_FX_H__
#define __MULTI_DEBRIS_FX_H__

#include "SpecialFX.h"
#include "DebrisMgr.h"

class CMultiDebrisFX : public CSpecialFX
{
	public :

//      virtual LTBOOL	Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual DBOOL	Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
//		virtual LTBOOL	CreateObject(ILTClient* pClientDE);
//      virtual LTBOOL	Update();

};

//-------------------------------------------------------------------------------------------------
// SFX_DebrisFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_MultiDebrisFactory : public CSpecialFXFactory
{
	SFX_MultiDebrisFactory() : CSpecialFXFactory(SFX_MULTI_DEBRIS_ID) {;}
	static const SFX_MultiDebrisFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};


#endif // __MULTI_DEBRIS_FX_H__
