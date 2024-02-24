// ----------------------------------------------------------------------- //
//
// MODULE  : CHHWeaponModel.cpp
//
// PURPOSE : CHHWeaponModel implementation
//
// CREATED : 10/31/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HHWeaponModel.h"
#include "cpp_server_de.h"
#include "ServerUtilities.h"

#include "Weapon.h"
#include "ServerSoundMgr.h"
#include <stdio.h>

#define HHW_SOUND_KEY_RADIUS	1000.0f

BEGIN_CLASS(CHHWeaponModel)

	ADD_BOOLPROP_FLAG(TrueBaseObj, LTFALSE, PF_HIDDEN)

END_CLASS_DEFAULT_FLAGS(CHHWeaponModel, BaseClass, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::CHHWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CHHWeaponModel::CHHWeaponModel() : BaseClass(OT_MODEL)
{
	m_pParent			= DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::~CHHWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CHHWeaponModel::~CHHWeaponModel()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CHHWeaponModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags = FLAG_FORCEOPTIMIZEOBJECT;
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
			break;
		}

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::InitialUpdate()
{
	g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
	g_pLTServer->SetDeactivationTime(m_hObject, 0.001f);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);

	// Set the dims based on the current animation...

	HMODELANIM hAnim = g_pLTServer->GetAnimIndex(m_hObject, "HandHeld");
	g_pLTServer->SetModelLooping(m_hObject, DFALSE);

	if(hAnim != INVALID_MODEL_ANIM)
	{
		g_pLTServer->SetModelAnimation(m_hObject, hAnim);

		DVector vDims;
		g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, hAnim);
		g_pLTServer->SetObjectDims(m_hObject, &vDims);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHHWeaponModel::SetParent()
//
//	PURPOSE:	Set our parent data member
//
// ----------------------------------------------------------------------- //

void CHHWeaponModel::SetParent(CWeapon* pParent) 
{
	_ASSERT( pParent );

	m_pParent = pParent;
}



