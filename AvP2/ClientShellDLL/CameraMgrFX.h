// ----------------------------------------------------------------------- //
//
// MODULE  : CameraMgrFX.h
//
// PURPOSE : Default FX handling functions for CameraMgr
//
// CREATED : 5/31/00
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_MGR_FX_H__
#define __CAMERA_MGR_FX_H__

// ----------------------------------------------------------------------- //

#include "cameramgr.h"

// ----------------------------------------------------------------------- //

#define CAMERAMGRFX_ID_POSOFFSET			100
#define CAMERAMGRFX_ID_AIMING				101
#define CAMERAMGRFX_ID_3RDPERSON			102
#define CAMERAMGRFX_ID_WIDESCREEN			103
#define CAMERAMGRFX_ID_FLASH				104
#define CAMERAMGRFX_ID_ZOOM					105
#define CAMERAMGRFX_ID_QUICKZOOM			106
#define CAMERAMGRFX_ID_TILT					107
#define CAMERAMGRFX_ID_HEAD_BOB				108
#define CAMERAMGRFX_ID_HEAD_TILT			109
#define CAMERAMGRFX_ID_SHAKE				110
#define CAMERAMGRFX_ID_WARBLE				111
#define CAMERAMGRFX_ID_WONKY				112
#define CAMERAMGRFX_ID_VOLUMEBRUSH			113
#define CAMERAMGRFX_ID_QUICKLUNGE			114
#define CAMERAMGRFX_ID_VISIONCYCLE			115
#define CAMERAMGRFX_ID_SCREENWASH			116
#define CAMERAMGRFX_ID_LANDBOB				117
#define CAMERAMGRFX_ID_NET_TILT				118

// ----------------------------------------------------------------------- //

#define CAMERAMGRFX_STATE_DISABLED			0
#define CAMERAMGRFX_STATE_RAMPUP			1
#define CAMERAMGRFX_STATE_ACTIVE			2
#define CAMERAMGRFX_STATE_RAMPDOWN			3

// ----------------------------------------------------------------------- //
// Console varaibles setup functions

void	CameraMgrFX_InitConsoleVariables();

struct CAMERAMGRFX
{
	uint8		nState;
};

LTBOOL	CameraMgrFX_Active(HCAMERA hCamera, uint8 nID);
uint8	CameraMgrFX_State(HCAMERA hCamera, uint8 nID);
void	CameraMgrFX_Clear(HCAMERA hCamera, uint8 nID);

// ----------------------------------------------------------------------- //
// Effect functions for handling 3rd person camera

struct CAMERAMGRFX_3RDPERSON : public CAMERAMGRFX
{
	LTFLOAT		fDist;
	LTBOOL		bCycle;
	LTBOOL		bEdit;
	LTBOOL		bChanged;
};

void	CameraMgrFX_3rdPersonToggle(HCAMERA hCamera, LTBOOL bCycle = LTFALSE);
LTBOOL	CameraMgrFX_3rdPersonUpdate(HCAMERA hCamera, void *pEffect);
void	CameraMgrFX_3rdPersonEdit(HCAMERA hCamera, LTBOOL bEdit);
LTBOOL	CameraMgrFX_3rdPersonChanged(HCAMERA hCamera);

// ----------------------------------------------------------------------- //
// Effect functions for handling widescreen view

struct CAMERAMGRFX_WIDESCREEN : public CAMERAMGRFX
{
	LTFLOAT		fScale;
};

void	CameraMgrFX_WidescreenToggle(HCAMERA hCamera);
LTBOOL	CameraMgrFX_WidescreenUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling flashing

struct CAMERAMGRFX_FLASH : public CAMERAMGRFX
{
	LTVector	vColor;
	LTVector	vPos;
	LTFLOAT		fLife;
	LTFLOAT		fRampUp;
	LTFLOAT		fRampDown;
	LTFLOAT		fScaleDist;
	LTBOOL		bLocal;
};

void	CameraMgrFX_FlashCreate(HCAMERA hCamera, LTVector vColor, LTVector vPos, LTFLOAT fLife,
			LTFLOAT fUp, LTFLOAT fDown, LTFLOAT fScaleDist, LTBOOL bLocal);
LTBOOL	CameraMgrFX_FlashUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling zooming

struct CAMERAMGRFX_ZOOM : public CAMERAMGRFX
{
	LTFLOAT		fStartMagnitude;
	LTFLOAT		fMagnitude;
	LTFLOAT		fDestMagnitude;
	LTBOOL		bFullbright;
};

void	CameraMgrFX_ZoomToggle(HCAMERA hCamera, LTFLOAT fMagnitude, LTBOOL bFullbright = LTFALSE);
LTBOOL	CameraMgrFX_ZoomUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling quick zooming

struct CAMERAMGRFX_QUICKZOOM : public CAMERAMGRFX
{
	LTFLOAT		fMagnitudeAdjust;
	LTFLOAT		fTime;
	LTFLOAT		fRollAmount;
};

void	CameraMgrFX_QuickZoomCreate(HCAMERA hCamera, LTFLOAT fMagAdjust, LTFLOAT fTime, LTFLOAT fRollAmount);
LTBOOL	CameraMgrFX_QuickZoomUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling tilting

struct CAMERAMGRFX_TILT : public CAMERAMGRFX
{
	LTVector	vAmount;
	LTVector	vTimes;

	LTVector	vStartAmount;
	LTVector	vCurrentAmount;
};

void	CameraMgrFX_TiltCreate(HCAMERA hCamera, LTVector vAmount, LTVector vTimes);
LTBOOL	CameraMgrFX_TiltUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling head bob

void	CameraMgrFX_HeadBobCreate(HCAMERA hCamera);
LTBOOL	CameraMgrFX_HeadBobUpdate(HCAMERA hCamera, void *pEffect);
void	CameraMgrFX_SetHeadBob(HCAMERA hCamera, LTFLOAT fAmount);

// ----------------------------------------------------------------------- //
// Effect functions for handling head tilting

struct CAMERAMGRFX_HEADTILT : public CAMERAMGRFX
{
	LTFLOAT		fAngle;
	LTFLOAT		fSpeed;

	LTFLOAT		fDirection;
	LTFLOAT		fRoll;
};

void	CameraMgrFX_HeadTiltCreate(HCAMERA hCamera, LTFLOAT fAngle, LTFLOAT fSpeed);
LTBOOL	CameraMgrFX_HeadTiltUpdate(HCAMERA hCamera, void *pEffect);
void	CameraMgrFX_SetHeadTiltAmount(HCAMERA hCamera, LTFLOAT fDirection = 0.0f);

// ----------------------------------------------------------------------- //
// Effect functions for handling shaking

struct CAMERAMGRFX_SHAKE : public CAMERAMGRFX
{
	LTVector	vAmount;
};

void	CameraMgrFX_ShakeCreate(HCAMERA hCamera, LTVector vAddAmount);
LTBOOL	CameraMgrFX_ShakeUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling warble vision

struct CAMERAMGRFX_WARBLE : public CAMERAMGRFX
{
	LTFLOAT		fXScale;
	LTFLOAT		fYScale;
};

void	CameraMgrFX_WarbleCreate(HCAMERA hCamera, LTFLOAT fXScale, LTFLOAT fYScale);
LTBOOL	CameraMgrFX_WarbleUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling wonky vision

struct CAMERAMGRFX_WONKY : public CAMERAMGRFX
{
	LTFLOAT		fScale;

	LTFLOAT		fPitchScale;
	LTFLOAT		fYawScale;
	LTFLOAT		fRollScale;
	LTFLOAT		fFOVScale;
	LTFLOAT		fFOVX;
	LTFLOAT		fFOVY;
};

void	CameraMgrFX_WonkyToggle(HCAMERA hCamera, LTFLOAT fScale);
LTBOOL	CameraMgrFX_WonkyUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling volume brush FX

struct CAMERAMGRFX_VOLUMEBRUSH : public CAMERAMGRFX
{
	uint8		nNumContainers;
	LTBOOL		bInContainer;
	LTBOOL		bInLiquid;
	HLTSOUND	hSound;
};

void	CameraMgrFX_VolumeBrushCreate(HCAMERA hCamera, uint8 nNumContainers);
LTBOOL	CameraMgrFX_VolumeBrushUpdate(HCAMERA hCamera, void *pEffect);
LTBOOL	CameraMgrFX_VolumeBrushInContainer(HCAMERA hCamera);
LTBOOL	CameraMgrFX_VolumeBrushInLiquid(HCAMERA hCamera);

// ----------------------------------------------------------------------- //
// Effect functions for handling camera lunge FX

struct CAMERAMGRFX_QUICKLUNGE : public CAMERAMGRFX
{
	LTVector	vPosOffset;
	LTVector	vRotOffset;
	LTFLOAT		fTime;
};

void	CameraMgrFX_QuickLungeCreate(HCAMERA hCamera, LTVector vPosOffset, LTVector vRotOffset, LTFLOAT fTime);
LTBOOL	CameraMgrFX_QuickLungeUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //
// Effect functions for handling vision mode cycling camera FX

struct CAMERAMGRFX_VISIONCYCLE : public CAMERAMGRFX
{
	uint8		nType;
};

void	CameraMgrFX_VisionCycleCreate(HCAMERA hCamera);
LTBOOL	CameraMgrFX_VisionCycleUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
// Effect functions for handling screen washout camera FX

void	CameraMgrFX_ScreenWashCreate(HCAMERA hCamera);
LTBOOL	CameraMgrFX_ScreenWashUpdate(HCAMERA hCamera, void *pEffect);
void	CameraMgrFX_SetScreenWash(HCAMERA hCamera, LTFLOAT fAmount);
LTFLOAT	CameraMgrFX_GetMaxScreenWash();

// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
// Effect functions for handling head bob

struct CAMERAMGRFX_LAND_BOB : public CAMERAMGRFX
{
	LTVector	vTiltAmount;
	LTVector	vTiltTimes;
	LTVector	vTiltStartAmount;
	LTVector	vTiltCurrentAmount;

	LTVector	vAmount;
	LTVector	vTimes;
	LTVector	vStartAmount;
	LTVector	vCurrentAmount;
};

void	CameraMgrFX_LandingBobCreate(HCAMERA hCamera, LTBOOL bSmall);
LTBOOL	CameraMgrFX_LandingBobUpdate(HCAMERA hCamera, void *pEffect);

// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
// Effect functions for handling net 

struct CAMERAMGRFX_NET_TILT : public CAMERAMGRFX
{
	LTVector	vTiltAmount;
	LTVector	vTiltTimes;
	LTVector	vTiltStartAmount;
	LTVector	vTiltCurrentAmount;

	LTVector	vAmount;
	LTVector	vTimes;
	LTVector	vStartAmount;
	LTVector	vCurrentAmount;

	LTBOOL		bRampUp;
};

void	CameraMgrFX_NetTiltCreate(HCAMERA hCamera, LTVector vRotAmount, LTVector vTiltAmount);
LTBOOL	CameraMgrFX_NetTiltUpdate(HCAMERA hCamera, void *pEffect);
void	CameraMgrFX_NetTiltSetRampDown(HCAMERA hCamera);

#endif //__CAMERA_MGR_FX_H__
