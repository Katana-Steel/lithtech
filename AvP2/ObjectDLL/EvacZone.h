// ----------------------------------------------------------------------- //
//
// MODULE  : EvacZone.h
//
// PURPOSE : Evac Zone Container definition
//
// CREATED : 7/5/01
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __EVAC_ZONE_H__
#define __EVAC_ZONE_H__

#include "GameBase.h"
#include "ContainerCodes.h"

class EvacZone : public GameBase
{
	public :

		EvacZone();
		~EvacZone();

	protected :

		uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
};

#endif // __EVAC_ZONE_H__

