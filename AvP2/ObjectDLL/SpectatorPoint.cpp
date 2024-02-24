// ----------------------------------------------------------------------- //
//
// MODULE  : SpectatorPoint.cpp
//
// PURPOSE : SpectatorPoint implementation
//
// CREATED : 6/8/01
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SpectatorPoint.h"


BEGIN_CLASS(SpectatorPoint)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(SpectatorPoint, GameBase, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpectatorPoint::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SpectatorPoint::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				SAFE_STRCPY(pStruct->m_Name, "SpectatorPoint");
			}			
		}
		break;

		case MID_INITIALUPDATE:
		{
			if(!dwRet) break;

			SetNextUpdate(0.0f, 0.001f);
		}
		break;

		default : break;
	}

	return dwRet;
}

