// ----------------------------------------------------------------------- //
//
// MODULE  : CameraFX.cpp
//
// PURPOSE : Camera special fx class - Definition
//
// CREATED : 
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "camerafx.h"


LTBOOL CCameraFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if(!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	CAMCREATESTRUCT* pCAM = (CAMCREATESTRUCT*)psfxCreateStruct;

	m_bAllowPlayerMovement	= pCAM->bAllowPlayerMovement;
	m_bWidescreen			= pCAM->bWidescreen;
	m_nXFOV					= pCAM->nXFOV;
	m_nYFOV					= pCAM->nYFOV;
	m_bIsListener			= pCAM->bIsListener;

	return LTTRUE;
}

LTBOOL CCameraFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	CAMCREATESTRUCT cam;

	cam.hServerObj = hServObj;
	cam.bAllowPlayerMovement = (LTBOOL) hRead->ReadByte();
	cam.bWidescreen = (LTBOOL) hRead->ReadByte();
	cam.nXFOV = hRead->ReadByte();
	cam.nYFOV = hRead->ReadByte();
	cam.bIsListener = (LTBOOL) hRead->ReadByte();

	return Init(&cam);
}


//-------------------------------------------------------------------------------------------------
// SFX_CameraFactory
//-------------------------------------------------------------------------------------------------

const SFX_CameraFactory SFX_CameraFactory::registerThis;

CSpecialFX* SFX_CameraFactory::MakeShape() const
{
	return new CCameraFX();
}


