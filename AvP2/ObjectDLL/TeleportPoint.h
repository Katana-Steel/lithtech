// ----------------------------------------------------------------------- //
//
// MODULE  : TeleportPoint.h
//
// PURPOSE : TeleportPoint - Definition
//
// CREATED : 4/21/99
//
// ----------------------------------------------------------------------- //

#ifndef __TELEPORT_POINT_H__
#define __TELEPORT_POINT_H__

#include "cpp_engineobjects_de.h"

class TeleportPoint : public BaseClass
{
	public :

		DVector	GetPitchYawRoll() const { return m_vPitchYawRoll; }

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
	
	private :

		DVector	m_vPitchYawRoll;		// Pitch, yaw, and roll of point
};

#endif // __TELEPORT_POINT_H__
