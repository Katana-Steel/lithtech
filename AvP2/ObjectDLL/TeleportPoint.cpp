// ----------------------------------------------------------------------- //
//
// MODULE  : TeleportPoint.cpp
//
// PURPOSE : TeleportPoint implementation
//
// CREATED : 4/21/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TeleportPoint.h"


BEGIN_CLASS(TeleportPoint)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(TeleportPoint, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TeleportPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD TeleportPoint::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				g_pServerDE->GetPropRotationEuler("Rotation", &m_vPitchYawRoll);
			}
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);;
}
