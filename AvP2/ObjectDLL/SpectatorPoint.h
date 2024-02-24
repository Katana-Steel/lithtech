// ----------------------------------------------------------------------- //
//
// MODULE  : SpectatorPoint.h
//
// PURPOSE : SpectatorPoint - Definition
//
// CREATED : 6/8/01
//
// ----------------------------------------------------------------------- //

#ifndef __SPECTATOR_POINT_H__
#define __SPECTATOR_POINT_H__

// ----------------------------------------------------------------------- //

#include "GameBase.h"

// ----------------------------------------------------------------------- //

class SpectatorPoint : public GameBase
{
	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
};


#endif // __SPECTATOR_POINT_H__

