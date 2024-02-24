// ----------------------------------------------------------------------- //
//
// MODULE  : CameraMgr.h
//
// PURPOSE : General client side camera handling class definitions
//
// CREATED : 3/1/00
//
// ----------------------------------------------------------------------- //

#ifndef __CAMERA_MGR_H__
#define __CAMERA_MGR_H__

// ----------------------------------------------------------------------- //

#include "cameramgrdefs.h"
#include "iclientshell.h"

// ----------------------------------------------------------------------- //

class CameraMgr
{
	public:
		// Constructors and destructors
		CameraMgr();
		~CameraMgr();

		// Initialization and termination of the manager
		LTBOOL		Init();
		void		Term();

		// Active camera control
		void		AllowMultipleActiveCameras(LTBOOL bAllow = LTFALSE);
		void		SetCameraActive(HCAMERA hCamera, LTBOOL bActive = LTTRUE);
		LTBOOL		GetCameraActive(HCAMERA hCamera);
		uint8		GetActiveCameras(HCAMERA *hCameras, uint8 nMax);

		// Rendering control
		void		RenderActiveCameras();

		// Initialization and termination of cameras
		HCAMERA		NewCamera(CAMERACREATESTRUCT &camCS);
		void		DeleteCamera(HCAMERA hCamera);

		// Camera effect hanlding functions
		CAMERAEFFECTINSTANCE*	NewCameraEffect(HCAMERA hCamera, CAMERAEFFECTCREATESTRUCT &ceCS);
		void					DeleteCameraEffect(HCAMERA hCamera, CAMERAEFFECTINSTANCE *pEffect);
		void					ClearCameraEffects(HCAMERA hCamera);
		CAMERAEFFECTINSTANCE*	GetCameraEffectByID(HCAMERA hCamera, uint8 nID, CAMERAEFFECTINSTANCE *pStart); 

		// Update functions for camera movement, rotation, and effects
		void		Update();
		void		UpdateCameraEffects(HCAMERA hCamera);

		// Camera data setting functions
		// These functions set the base values of the camera, SFX offsets aren't taken into consideration
		void		SetCameraFlags(HCAMERA hCamera, uint32 dwFlags);
		void		SetCameraPos(HCAMERA hCamera, LTVector &vPos);
		void		SetCameraRotation(HCAMERA hCamera, LTRotation &rRot);
		void		SetCameraObj(HCAMERA hCamera, HOBJECT hObj, char *szSocket = LTNULL);
		void		SetCameraTarget(HCAMERA hCamera, HOBJECT hTarget, char *szSocket = LTNULL);
		void		SetCameraTargetPos(HCAMERA hCamera, LTVector &vPos);
		void		SetCameraFOV(HCAMERA hCamera, LTFLOAT fFOVX, LTFLOAT fFOVY, LTBOOL bDegrees = LTTRUE);
		void		SetCameraRect(HCAMERA hCamera, LTRect &rect, LTBOOL bFullScreen);
		void		SetCameraLightAdd(HCAMERA hCamera, LTVector &vLightAdd);
		void		SetCameraLightScale(HCAMERA hCamera, LTVector &vLightScale);

		void		SetCameraPreRenderFunc(HCAMERA hCamera, LTBOOL (*fp)(CameraMgr *pMgr, HCAMERA hCamera));
		void		SetCameraPostRenderFunc(HCAMERA hCamera, void (*fp)(CameraMgr *pMgr, HCAMERA hCamera));

		// Camera data retrieval functions
		// The last parameter of these functions specify whether it should get the values considering the SFX or not
		CAMERAINSTANCE*		GetCamera(HCAMERA hCamera);						// Get the instance
		HOBJECT				GetCameraObject(HCAMERA hCamera);				// Gets the camera engine object
		uint32				GetCameraFlags(HCAMERA hCamera);
		void				GetCameraPos(HCAMERA hCamera, LTVector &vPos, LTBOOL bFX = LTFALSE);
		void				GetCameraRotation(HCAMERA hCamera, LTRotation &rRot, LTBOOL bFX = LTFALSE);
		HOBJECT				GetCameraObj(HCAMERA hCamera, char *szSocket = LTNULL, LTBOOL bFX = LTFALSE);
		HOBJECT				GetCameraTarget(HCAMERA hCamera, char *szSocket = LTNULL, LTBOOL bFX = LTFALSE);
		void				GetCameraTargetPos(HCAMERA hCamera, LTVector &vPos, LTBOOL bFX = LTFALSE);
		void				GetCameraFOV(HCAMERA hCamera, LTFLOAT &fFOVX, LTFLOAT &fFOVY, LTBOOL bFX = LTFALSE);
		LTBOOL				GetCameraRect(HCAMERA hCamera, LTRect &rect, LTBOOL bFX = LTFALSE);
		LTBOOL				GetCameraLightAdd(HCAMERA hCamera, LTVector &vLightAdd, LTBOOL bFX = LTFALSE);
		LTBOOL				GetCameraLightScale(HCAMERA hCamera, LTVector &vLightScale, LTBOOL bFX = LTFALSE);

	private:
		// Camera creation helper functions
		HCAMERA		FindOpenCameraSlot();

		// Camera updating helper functions (these functions do no checks for valid objects - do it before using these)
		void		UpdateToObjPosition(HCAMERA hCamera);
		void		UpdateToObjRotation(HCAMERA hCamera);
		void		UpdateToTargetRotation(HCAMERA hCamera);

	private:
		// Camera list variables
		CAMERAINSTANCE		*m_pCameras[CAMERAMGR_MAX_CAMERAS];

		// Control parameters for the camera mgr
		LTBOOL		m_bAllowMultipleActiveCameras;
};

// ----------------------------------------------------------------------- //

extern CameraMgr *g_pCameraMgr;

// ----------------------------------------------------------------------- //

#endif
