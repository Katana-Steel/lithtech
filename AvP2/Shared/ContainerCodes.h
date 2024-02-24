// ----------------------------------------------------------------------- //
//
// MODULE  : ContainerCodes.h
//
// PURPOSE : Container code definitions
//
// CREATED : 1/29/98
//
// ----------------------------------------------------------------------- //

#ifndef __CONTAINER_CODES_H__
#define __CONTAINER_CODES_H__

// ----------------------------------------------------------------------- //

#include "ClientServerShared.h"

// ----------------------------------------------------------------------- //

#ifdef _WIN32
	#define DEFAULT_BUBBLE_TEXTURE		"SpriteTextures\\SFX\\Particles\\bubble.dtx"
#else
	#define DEFAULT_BUBBLE_TEXTURE		"SpriteTextures/SFX/Particles/bubble.dtx"
#endif

// ----------------------------------------------------------------------- //

#define MAX_CONTAINER_VISCOSITY		5000.0f

// ----------------------------------------------------------------------- //

enum ContainerCode
{
	CC_NO_CONTAINER = 0,
	CC_WATER,
	CC_CORROSIVE_FLUID,
	CC_ICE,
	CC_FREEZING_WATER,
	CC_POISON_GAS,
	CC_ELECTRICITY,
	CC_ENDLESS_FALL,
	CC_WIND,
	CC_COLDAIR,
	CC_LADDER,
	CC_WEATHER,
	CC_VOLUME,
	CC_EVACZONE,
	CC_MAX_CONTAINER_CODES
};

// ----------------------------------------------------------------------- //
// Following are flags used by the Weather container...

#define WFLAG_LIGHT_RAIN		(1<<0)
#define WFLAG_NORMAL_RAIN		(1<<1)
#define WFLAG_HEAVY_RAIN		(1<<2)
#define	WFLAG_LIGHT_SNOW		(1<<3)
#define	WFLAG_NORMAL_SNOW		(1<<4)
#define	WFLAG_HEAVY_SNOW		(1<<5)

#define	WFLAG_RAIN				(WFLAG_LIGHT_RAIN | WFLAG_NORMAL_RAIN | WFLAG_HEAVY_RAIN)
#define	WFLAG_SNOW				(WFLAG_LIGHT_SNOW | WFLAG_NORMAL_SNOW | WFLAG_HEAVY_SNOW)

// ----------------------------------------------------------------------- //

inline ContainerCode GetContainerCode(DVector vPos)
{
	ContainerCode eContainerCode = CC_NO_CONTAINER;

	HOBJECT objList[1];
	DDWORD dwNum = g_pInterface->GetPointContainers(&vPos, objList, 1);

	DDWORD dwUserFlags = 0;
	if (dwNum > 0 && objList[0])
	{
#ifdef _CLIENTBUILD
		g_pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);
#else
		dwUserFlags = g_pServerDE->GetObjectUserFlags(objList[0]);
#endif
	}

	if (dwNum > 0 && (dwUserFlags & USRFLG_VISIBLE))
	{
		D_WORD code;
		if (g_pInterface->GetContainerCode(objList[0], &code))
		{
			eContainerCode = (ContainerCode)code;
		}
	}

	return eContainerCode;
}

// ----------------------------------------------------------------------- //

inline DBOOL IsLiquid(ContainerCode code)
{
	DBOOL bRet = DFALSE;

	switch (code)
	{
		case CC_WATER:
		case CC_CORROSIVE_FLUID:
		case CC_FREEZING_WATER:
			bRet = DTRUE;
		break;

		default : break;
	}

	return bRet;
}

// ----------------------------------------------------------------------- //

inline DBOOL GetLiquidColorRange(ContainerCode code, DVector* pvC1, DVector* pvC2)
{
	if (!IsLiquid(code) || !pvC1 || !pvC2) return DFALSE;

	switch (code)
	{
		case CC_WATER:
			VEC_SET(*pvC1, 235.0f, 235.0f, 235.0f);
			VEC_SET(*pvC2, 235.0f, 235.0f, 255.0f);
		break;

		case CC_CORROSIVE_FLUID:
			VEC_SET(*pvC2, 200.0f, 200.0f, 0.0f);
			VEC_SET(*pvC2, 255.0f, 255.0f, 0.0f);
		break;
		
		case CC_FREEZING_WATER:
			VEC_SET(*pvC2, 200.0f, 200.0f, 255.0f);
			VEC_SET(*pvC2, 200.0f, 200.0f, 255.0f);
		break;

		default : break;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //

inline DBOOL IsFreeMovement(ContainerCode code)
{
	return(IsLiquid(code) || (code == CC_LADDER));
}

// ----------------------------------------------------------------------- //

#endif // __CONTAINER_CODES_H__
