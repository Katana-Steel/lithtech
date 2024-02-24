// ----------------------------------------------------------------------- //
//
// MODULE  : EvacZone.cpp
//
// PURPOSE : Evac Zone Container definition
//
// CREATED : 7/5/01
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "EvacZone.h"

#include "GameServerShell.h"

BEGIN_CLASS(EvacZone)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT(EvacZone, GameBase, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EvacZone::EvacZone()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

EvacZone::EvacZone() : GameBase(OT_CONTAINER)
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EvacZone::~EvacZone
//
//	PURPOSE:	Deallocate
//
// ----------------------------------------------------------------------- //

EvacZone::~EvacZone()
{ 
	g_pGameServerShell->DecEvacZoneObjects();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EvacZone::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 EvacZone::EngineMessageFn(uint32 messageID, void *pData, DFLOAT fData)
{
	uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
				SAFE_STRCPY(pStruct->m_SkinName, pStruct->m_Name);

				pStruct->m_Flags = FLAG_CONTAINER | FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD | FLAG_FORCECLIENTUPDATE;
				pStruct->m_ObjectType = OT_CONTAINER;
				pStruct->m_ContainerCode = (D_WORD)CC_EVACZONE;
			}			

			break;
		}

		case MID_INITIALUPDATE:
		{
			if(!dwRet) break;

			LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			g_pGameServerShell->IncEvacZoneObjects();
			g_pGameServerShell->SetEvacZonePos(vPos);

			SetNextUpdate(0.0f, 0.001f);

			break;
		}

		default : break;
	}

	return dwRet;
}


