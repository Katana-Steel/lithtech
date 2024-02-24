// ----------------------------------------------------------------------- //
//
// MODULE  : TargetingSprite.h
//
// PURPOSE : CTargetingSprite definition - Server-side targeting mark fx
//
// CREATED : 9/25/2000
//
// ----------------------------------------------------------------------- //

#ifndef __TARGETING_SPRITE_H__
#define __TARGETING_SPRITE_H__

#include "cpp_engineobjects_de.h"

class CTargetingSprite : public BaseClass
{
	public :

		CTargetingSprite();
		virtual ~CTargetingSprite();

		void Setup();
		
	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
};

#endif // __TARGETING_SPRITE_H__

