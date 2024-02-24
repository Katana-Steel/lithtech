// ----------------------------------------------------------------------- //
//
// MODULE  : CameraMgrDefs.h
//
// PURPOSE : General client side camera handling class definitions
//
// CREATED : 3/1/00
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_MGR_DEFS_H__
#define __CAMERA_MGR_DEFS_H__

// ----------------------------------------------------------------------- //

#include "ltbasedefs.h"

//*************************************************************************//

#define CAMERAMGR_MAX_CAMERAS					64
#define CAMERAMGR_MAX_SOCKET_NAME				64
#define CAMERAMGR_INVALID						-1

//*************************************************************************//

#define CAMERA_FLAG_NONE						0x0000

#define CAMERA_FLAG_FOLLOW_OBJ_POS				0x0001
#define CAMERA_FLAG_FOLLOW_OBJ_SOCKET_POS		0x0002
#define CAMERA_FLAG_FOLLOW_OBJ_ROT				0x0004
#define CAMERA_FLAG_FOLLOW_OBJ_SOCKET_ROT		0x0008
#define CAMERA_FLAG_FOLLOW_TARGET				0x0010
#define CAMERA_FLAG_FOLLOW_TARGET_SOCKET		0x0020
#define CAMERA_FLAG_FOLLOW_TARGET_POS			0x0040	// The Target_Pos flag overrides other target flags

#define CAMERA_FLAG_FOLLOW_OBJ_ALL				0x000F
#define CAMERA_FLAG_FOLLOW_TARGET_ALL			0x0030

#define CAMERA_FLAG_INTERP_POS_RIGHT			0x0100
#define CAMERA_FLAG_INTERP_POS_UP				0x0200
#define CAMERA_FLAG_INTERP_POS_FORWARD			0x0400
#define CAMERA_FLAG_INTERP_ROT					0x0800

#define CAMERA_FLAG_INTERP_POS_ALL				0x0700
#define CAMERA_FLAG_INTERP_ALL					0x0F00

#define CAMERA_FLAG_ALL							0xFFFF

//*************************************************************************//

#define CAMERA_FX_USE_NONE						0x0000
#define CAMERA_FX_USE_TIME						0x0001
#define CAMERA_FX_USE_PITCH						0x0002
#define CAMERA_FX_USE_FREQUENCY					0x0004
#define CAMERA_FX_USE_POS						0x0008
#define CAMERA_FX_USE_ROT_V						0x0010
#define CAMERA_FX_USE_ROT						0x0020
#define CAMERA_FX_USE_OBJ						0x0040
#define CAMERA_FX_USE_TARGET					0x0080
#define CAMERA_FX_USE_FOVX						0x0100
#define CAMERA_FX_USE_FOVY						0x0200
#define CAMERA_FX_USE_RECT						0x0400
#define CAMERA_FX_USE_LIGHTADD					0x0800
#define CAMERA_FX_USE_LIGHTSCALE				0x1000
#define CAMERA_FX_USE_NOCLIP					0x2000
#define CAMERA_FX_USE_ALL						0xFFFF

#define CAMERA_FX_INTERP_LINEAR					1
#define CAMERA_FX_INTERP_SINE					2
#define CAMERA_FX_INTERP_COSINE					3
#define CAMERA_FX_INTERP_TANGENT				4

#define CAMERA_FX_INTERP_VAR_MIN				0
#define CAMERA_FX_INTERP_VAR_VECTOR				0
#define CAMERA_FX_INTERP_VAR_ROTATION			1
#define CAMERA_FX_INTERP_VAR_FLOAT				2
#define CAMERA_FX_INTERP_VAR_INT32				3
#define CAMERA_FX_INTERP_VAR_RECT				4
#define CAMERA_FX_INTERP_VAR_MAX				4

//*************************************************************************//

typedef int		HCAMERA;

//*************************************************************************//

struct CAMERAEXTRAINTERPDATA
{
	CAMERAEXTRAINTERPDATA::CAMERAEXTRAINTERPDATA();

	LTFLOAT		fPosR_ST;			// Camera position right offset interpolation start time
	LTFLOAT		fPosU_ST;			// Camera position up offset interpolation start time
	LTFLOAT		fPosF_ST;			// Camera position forward offset interpolation start time

	LTFLOAT		fRot_ST;			// Camera rotation offset interpolation smoothing time
};

// ----------------------------------------------------------------------- //

inline CAMERAEXTRAINTERPDATA::CAMERAEXTRAINTERPDATA()
{
	memset(this, 0, sizeof(CAMERAEXTRAINTERPDATA));
}

//*************************************************************************//

struct CAMERAINTERPDATA
{
	CAMERAINTERPDATA::CAMERAINTERPDATA();

	LTVector	vPos;				// Camera position data
	LTVector	vRot;				// Camera rotation data (angles in degrees) - Used for effects only
	LTRotation	rRot;				// Camera rotation data

	HOBJECT		hObj;				// Object to follow (will not follow anything if this is NULL)
	char		szObjSocket[CAMERAMGR_MAX_SOCKET_NAME];		// Socket on the object to reference (will use object's center if this is NULL)

	HOBJECT		hTarget;			// Object to look at (will not follow anything if this is NULL)
	char		szTargetSocket[CAMERAMGR_MAX_SOCKET_NAME];	// Socket on the target to reference (will use target's center if this is NULL)

	LTVector	vTargetPos;			// Position to look at (the proper flag must be set)

	LTFLOAT		fFOVX;				// Horizontal FOV value (in degrees)
	LTFLOAT		fFOVY;				// Vertical FOV value (in degrees)
	LTBOOL		bFullScreen;		// Should the camera render fullscreen?
	LTRect		rRect;				// Viewing rectangle to use when rendering the camera (ignored if bFullScreen is TRUE)
	LTVector	vLightAdd;			// Extra light to add to the camera rendering
	LTVector	vLightScale;		// Extra light to scale to the camera rendering
};

// ----------------------------------------------------------------------- //

inline CAMERAINTERPDATA::CAMERAINTERPDATA()
{
	memset(this, 0, sizeof(CAMERAINTERPDATA));
	rRot.Init();
	vLightScale = LTVector(1.0f, 1.0f, 1.0f);
}

//*************************************************************************//
// A structure to define a camera special effect.
//*************************************************************************//

struct CAMERAEFFECTCREATESTRUCT : public CAMERAINTERPDATA
{
	CAMERAEFFECTCREATESTRUCT::CAMERAEFFECTCREATESTRUCT();

	uint8		nID;				// A custom ID number used to find, remove, modify, etc. certain FX
	uint32		dwFlags;			// Flags on how to use this effect (CAMERA_FX_USE_ defs)
	uint8		nInterpType;		// The type of interpolation that the effect uses (CAMERA_FX_INTERP_ defs)

	LTBOOL		(*fnCustomInterp)(void *pResult, void *pStart, void *pEnd, LTFLOAT fT, uint16 nVarType);
	LTBOOL		(*fnCustomUpdate)(HCAMERA hCamera, void *pEffect);
	void		*pUserData;			// The pointer to the user data for use in the custom functions

	LTFLOAT		fTime;				// The amount of time this effect should last (zero is infinite)
	LTFLOAT		fPitch;				// A pitch of the effect interp style being used
	LTFLOAT		fFrequency;			// A frequency of the effect over time
};

// ----------------------------------------------------------------------- //

inline CAMERAEFFECTCREATESTRUCT::CAMERAEFFECTCREATESTRUCT()
{
	nID				= 0;
	dwFlags			= 0;
	nInterpType		= 0;

	fnCustomInterp	= LTNULL;
	fnCustomUpdate	= LTNULL;
	pUserData		= LTNULL;

	fTime			= 0.0f;
	fPitch			= 0.0f;
	fFrequency		= 0.0f;
}

//*************************************************************************//
// A structure to hold an instance of a special effect.
//*************************************************************************//

struct CAMERAEFFECTINSTANCE
{
	CAMERAEFFECTINSTANCE::CAMERAEFFECTINSTANCE();

	LTFLOAT		fStartTime;				// The time that the effect got created
	CAMERAEFFECTCREATESTRUCT	ceCS;	// The details of the special effect

	CAMERAEFFECTINSTANCE		*pPrev;	// Previous effect in a list
	CAMERAEFFECTINSTANCE		*pNext;	// Next effect in a list
};

// ----------------------------------------------------------------------- //

inline CAMERAEFFECTINSTANCE::CAMERAEFFECTINSTANCE()
{
	fStartTime	= 0.0f;

	pPrev		= LTNULL;
	pNext		= LTNULL;
}

//*************************************************************************//
// Structure used when creating a new camera through CameraMgr
//*************************************************************************//

class CameraMgr;

struct CAMERACREATESTRUCT : public CAMERAINTERPDATA
{
	CAMERACREATESTRUCT::CAMERACREATESTRUCT();

	uint32		dwFlags;			// The flags specify how to control the camera (CAMERA_FLAG_ defs)

	// These functions will be called whether the camera is active or not... so depending
	// on what you want to do inside these, you might need to check to see if the camera
	// passed in is actually active or not.

	// If LTFALSE is returned for the PreRender... the camera will not get rendered
	LTBOOL		(*fpPreRender)(CameraMgr *pMgr, HCAMERA hCamera);	// A function for pre rendering setup
	void		(*fpPostRender)(CameraMgr *pMgr, HCAMERA hCamera);	// A function for post rendering cleanup
};

// ----------------------------------------------------------------------- //

inline CAMERACREATESTRUCT::CAMERACREATESTRUCT()
{
	dwFlags			= CAMERA_FLAG_FOLLOW_OBJ_ALL;

	fpPreRender		= LTNULL;
	fpPostRender	= LTNULL;
}

//*************************************************************************//
// The structure that is used when maintaining each camera within CameraMgr.
// You can obtain a pointer to one of these after a camera has been created.
//*************************************************************************//

struct CAMERAINSTANCE
{
	CAMERAINSTANCE::CAMERAINSTANCE();

	uint32					dwFlags;		// The flags specify how to control the camera (CAMERA_FLAG_ defs)
	HOBJECT					hCamera;		// The engine object of the camera

	CAMERAINTERPDATA		cidOriginal;	// The cameras base values
	CAMERAINTERPDATA		cidFinal;		// The cameras values after any effects are calculated

	CAMERAEXTRAINTERPDATA	ceid;			// The interpolation times for the original camera data

	CAMERAEFFECTINSTANCE	*pEffectList;	// The list of special FX that this camera is using

	LTBOOL					bActive;		// Is this camera active (ie. should it be updating and rendering)?

	// These functions will be called whether the camera is active or not... so depending
	// on what you want to do inside these, you might need to check to see if the camera
	// passed in is actually active or not.

	// If LTFALSE is returned for the PreRender... the camera will not get rendered
	LTBOOL		(*fpPreRender)(CameraMgr *pMgr, HCAMERA hCamera);	// A function for pre rendering setup
	void		(*fpPostRender)(CameraMgr *pMgr, HCAMERA hCamera);	// A function for post rendering cleanup
};

// ----------------------------------------------------------------------- //

inline CAMERAINSTANCE::CAMERAINSTANCE()
{
	dwFlags			= CAMERA_FLAG_FOLLOW_OBJ_POS | CAMERA_FLAG_FOLLOW_OBJ_ROT;
	hCamera			= LTNULL;

	pEffectList		= LTNULL;
	bActive			= LTFALSE;

	fpPreRender		= LTNULL;
	fpPostRender	= LTNULL;
}

//*************************************************************************//

#endif
