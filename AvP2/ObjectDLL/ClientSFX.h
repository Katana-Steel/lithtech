// ----------------------------------------------------------------------- //
//
// MODULE  : CClientSFX.h
//
// PURPOSE : CClientSFX - Base class for client-side sfx objects - Definition
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SFX_H__
#define __CLIENT_SFX_H__

#include "GameBase.h"

class CClientSFX : public GameBase
{
	public :

		CClientSFX(DBYTE nType=OT_NORMAL);
		virtual ~CClientSFX() {}

};

#endif // __CLIENT_SFX_H__
