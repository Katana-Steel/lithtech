// ----------------------------------------------------------------------- //
//
// MODULE  : TargetingSprite.h
//
// PURPOSE : CTargetingSprite implementation
//
// CREATED : 9/25/2000
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TargetingSprite.h"

BEGIN_CLASS(CTargetingSprite)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(CTargetingSprite, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTargetingSprite::CTargetingSprite()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CTargetingSprite::CTargetingSprite() : BaseClass(OT_SPRITE)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTargetingSprite::~CTargetingSprite()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTargetingSprite::~CTargetingSprite()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTargetingSprite::Setup()
//
//	PURPOSE:	Setup
//
// ----------------------------------------------------------------------- //

void CTargetingSprite::Setup()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTargetingSprite::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CTargetingSprite::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	return BaseClass::EngineMessageFn(messageID, pData, fData);
}
