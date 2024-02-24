// ----------------------------------------------------------------------- //
//
// MODULE  : CameraMgrFX.cpp
//
// PURPOSE : Default FX handling functions for CameraMgr
//
// CREATED : 5/31/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "cameramgrfx.h"
#include "commandids.h"
#include "vartrack.h"

#include "gameclientshell.h"
#include "volumebrushfx.h"
#include "clientsoundmgr.h"
#include "ContainerCodes.h"

// ----------------------------------------------------------------------- //
// Variables that the FX need to use in order to update properly

static VarTrack		g_vt3rdPersonRamp;
static VarTrack		g_vt3rdPersonFOVX;
static VarTrack		g_vt3rdPersonFOVY;
static VarTrack		g_vt3rdPersonDist;
static VarTrack		g_vt3rdPersonFade;
static VarTrack		g_vt3rdPersonBack;
static VarTrack		g_vt3rdPersonAxis;
static VarTrack		g_vt3rdPersonCycl;
static VarTrack		g_vt3rdPersonMult;
static VarTrack		g_vt3rdPersonSpin;
static VarTrack		g_vt3rdPersonKeys;

static VarTrack		g_vtWidescreenRamp;
static VarTrack		g_vtWidescreenXSet;
static VarTrack		g_vtWidescreenYSet;

static VarTrack		g_vtZoomRamp;
static VarTrack		g_vtZoomLODMag;
static VarTrack		g_vtZoomLODScale;
static VarTrack		g_vtZoomSensMag;
static VarTrack		g_vtZoomSensScale;

static VarTrack		g_vtShakeMaxAmount;
static VarTrack		g_vtShakeDecayAmount;

static VarTrack		g_vtWarbleFOVPhase;

static VarTrack		g_vtWonkyPitch;
static VarTrack		g_vtWonkyYaw;
static VarTrack		g_vtWonkyRoll;
static VarTrack		g_vtWonkyPitchPhase;
static VarTrack		g_vtWonkyYawPhase;
static VarTrack		g_vtWonkyRollPhase;
static VarTrack		g_vtWonkyFOVX;
static VarTrack		g_vtWonkyFOVY;
static VarTrack		g_vtWonkyFOVPhase;
static VarTrack		g_vtWonkyRampUp;
static VarTrack		g_vtWonkyRampDown;

static VarTrack		g_vtVisionTime;

static VarTrack		g_vtScreenWashRestoreTime;
static VarTrack		g_vtMaxScreenWash;

static VarTrack		g_vtLandBobAmount;
static VarTrack		g_vtLandBobAmountSmall;
static VarTrack		g_vtLandBobRampUpTime;
static VarTrack		g_vtLandBobRampDownTime;

// ----------------------------------------------------------------------- //

void CameraMgrFX_InitConsoleVariables()
{
	// Init 3rd person camera variables
	if(!g_vt3rdPersonRamp.IsInitted())
		g_vt3rdPersonRamp.Init(g_pLTClient, "3rdPersonRamp", LTNULL, 1.0f);
	if(!g_vt3rdPersonFOVX.IsInitted())
		g_vt3rdPersonFOVX.Init(g_pLTClient, "3rdPersonFOVX", LTNULL, 75.0f);
	if(!g_vt3rdPersonFOVY.IsInitted())
		g_vt3rdPersonFOVY.Init(g_pLTClient, "3rdPersonFOVY", LTNULL, 60.0f);
	if(!g_vt3rdPersonDist.IsInitted())
		g_vt3rdPersonDist.Init(g_pLTClient, "3rdPersonDist", LTNULL, 150.0f);
	if(!g_vt3rdPersonFade.IsInitted())
		g_vt3rdPersonFade.Init(g_pLTClient, "3rdPersonFade", LTNULL, 75.0f);
	if(!g_vt3rdPersonBack.IsInitted())
		g_vt3rdPersonBack.Init(g_pLTClient, "3rdPersonBack", LTNULL, 5.0f);
	if(!g_vt3rdPersonAxis.IsInitted())
		g_vt3rdPersonAxis.Init(g_pLTClient, "3rdPersonAxis", LTNULL, 0.0f);
	if(!g_vt3rdPersonCycl.IsInitted())
		g_vt3rdPersonCycl.Init(g_pLTClient, "3rdPersonCycl", LTNULL, 0.0f);
	if(!g_vt3rdPersonMult.IsInitted())
		g_vt3rdPersonMult.Init(g_pLTClient, "3rdPersonMult", LTNULL, 50.0f);
	if(!g_vt3rdPersonSpin.IsInitted())
		g_vt3rdPersonSpin.Init(g_pLTClient, "3rdPersonSpin", LTNULL, 15.0f);
	if(!g_vt3rdPersonKeys.IsInitted())
		g_vt3rdPersonKeys.Init(g_pLTClient, "3rdPersonKeys", LTNULL, 45.0f);

	// Init wide screen camera variables
	if(!g_vtWidescreenRamp.IsInitted())
		g_vtWidescreenRamp.Init(g_pLTClient, "WidescreenRamp", LTNULL, 1.0f);
	if(!g_vtWidescreenXSet.IsInitted())
		g_vtWidescreenXSet.Init(g_pLTClient, "WidescreenXSet", LTNULL, 1.0f);
	if(!g_vtWidescreenYSet.IsInitted())
		g_vtWidescreenYSet.Init(g_pLTClient, "WidescreenYSet", LTNULL, 0.75f);

	// Init zoom camera variables
	if(!g_vtZoomRamp.IsInitted())
		g_vtZoomRamp.Init(g_pLTClient, "ZoomRamp", LTNULL, 0.25f);
	if(!g_vtZoomLODMag.IsInitted())
		g_vtZoomLODMag.Init(g_pLTClient, "ZoomLODMag", LTNULL, 8.0f);
	if(!g_vtZoomLODScale.IsInitted())
		g_vtZoomLODScale.Init(g_pLTClient, "ZoomLODScale", LTNULL, -3.0f);
	if(!g_vtZoomSensMag.IsInitted())
		g_vtZoomSensMag.Init(g_pLTClient, "ZoomSensMag", LTNULL, 8.0f);
	if(!g_vtZoomSensScale.IsInitted())
		g_vtZoomSensScale.Init(g_pLTClient, "ZoomSensScale", LTNULL, 0.2f);

	// Shake camera fx variables
	if(!g_vtShakeMaxAmount.IsInitted())
		g_vtShakeMaxAmount.Init(g_pLTClient, "ShakeMaxAmount", LTNULL, 20.0f);
	if(!g_vtShakeDecayAmount.IsInitted())
		g_vtShakeDecayAmount.Init(g_pLTClient, "ShakeDecayAmount", LTNULL, 3.5f);

	// Warble camera fx variables
	if(!g_vtWarbleFOVPhase.IsInitted())
		g_vtWarbleFOVPhase.Init(g_pLTClient, "WarbleFOVPhase", LTNULL, 0.75f);

	// Wonky camera fx variables
	if(!g_vtWonkyPitch.IsInitted())
		g_vtWonkyPitch.Init(g_pLTClient, "WonkyPitch", LTNULL, 15.0f);
	if(!g_vtWonkyYaw.IsInitted())
		g_vtWonkyYaw.Init(g_pLTClient, "WonkyYaw", LTNULL, 10.0f);
	if(!g_vtWonkyRoll.IsInitted())
		g_vtWonkyRoll.Init(g_pLTClient, "WonkyRoll", LTNULL, 12.5f);
	if(!g_vtWonkyPitchPhase.IsInitted())
		g_vtWonkyPitchPhase.Init(g_pLTClient, "WonkyPitchPhase", LTNULL, 2.5f);
	if(!g_vtWonkyYawPhase.IsInitted())
		g_vtWonkyYawPhase.Init(g_pLTClient, "WonkyYawPhase", LTNULL, 1.25f);
	if(!g_vtWonkyRollPhase.IsInitted())
		g_vtWonkyRollPhase.Init(g_pLTClient, "WonkyRollPhase", LTNULL, 1.25f);
	if(!g_vtWonkyFOVX.IsInitted())
		g_vtWonkyFOVX.Init(g_pLTClient, "WonkyFOVX", LTNULL, 15.0f);
	if(!g_vtWonkyFOVY.IsInitted())
		g_vtWonkyFOVY.Init(g_pLTClient, "WonkyFOVY", LTNULL, 7.5f);
	if(!g_vtWonkyFOVPhase.IsInitted())
		g_vtWonkyFOVPhase.Init(g_pLTClient, "WonkyFOVPhase", LTNULL, 1.0f);
	if(!g_vtWonkyRampUp.IsInitted())
		g_vtWonkyRampUp.Init(g_pLTClient, "WonkyRampUp", LTNULL, 5.0f);
	if(!g_vtWonkyRampDown.IsInitted())
		g_vtWonkyRampDown.Init(g_pLTClient, "WonkyRampDown", LTNULL, 2.0f);

	// Vision cycle camera fx variables
	if(!g_vtVisionTime.IsInitted())
		g_vtVisionTime.Init(g_pLTClient, "VisionTime", LTNULL, 0.2f);

	// Screen Wash camera fx variables
	if(!g_vtScreenWashRestoreTime.IsInitted())
		g_vtScreenWashRestoreTime.Init(g_pLTClient, "WashRestoreTime", LTNULL, 1.5f);
	if(!g_vtMaxScreenWash.IsInitted())
		g_vtMaxScreenWash.Init(g_pLTClient, "MaxWash", LTNULL, 0.75f);

	// Land Bob camera fx variables
	if(!g_vtLandBobAmount.IsInitted())
		g_vtLandBobAmount.Init(g_pLTClient, "LBAmount", LTNULL, -20.0f);
	if(!g_vtLandBobAmountSmall.IsInitted())
		g_vtLandBobAmountSmall.Init(g_pLTClient, "LBAmountSmall", LTNULL, -4.0f);
	if(!g_vtLandBobRampUpTime.IsInitted())
		g_vtLandBobRampUpTime.Init(g_pLTClient, "LBRampUpTime", LTNULL, 0.1f);
	if(!g_vtLandBobRampDownTime.IsInitted())
		g_vtLandBobRampDownTime.Init(g_pLTClient, "LBRampDownTime", LTNULL, 0.2f);
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_Active(HCAMERA hCamera, uint8 nID)
{
	return (g_pCameraMgr->GetCameraEffectByID(hCamera, nID, LTNULL) != LTNULL);
}

// ----------------------------------------------------------------------- //

uint8 CameraMgrFX_State(HCAMERA hCamera, uint8 nID)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, nID, LTNULL);

	// If an effect doesn't exist, then just return
	if(!pEffect)
		return CAMERAMGRFX_STATE_DISABLED;

	// Create a user data structure
	CAMERAMGRFX *pData = (CAMERAMGRFX*)pEffect->ceCS.pUserData;

	return pData ? (uint8)pData->nState : CAMERAMGRFX_STATE_DISABLED;
}

// ----------------------------------------------------------------------- //

void CameraMgrFX_Clear(HCAMERA hCamera, uint8 nID)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, nID, LTNULL);

	// If an effect doesn't exist, then just return
	if(!pEffect)
		return;

	g_pCameraMgr->DeleteCameraEffect(hCamera, pEffect);
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

void CameraMgrFX_3rdPersonToggle(HCAMERA hCamera, LTBOOL bCycle)
{
	// Find the 3rd person effect on this camera if one exists already
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_3RDPERSON, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_3RDPERSON *pData = new CAMERAMGRFX_3RDPERSON;
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->fDist = 0.0f;
		pData->bCycle = bCycle;
		pData->bEdit = LTFALSE;
		pData->bChanged = LTFALSE;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_3RDPERSON;
		ceCS.dwFlags = CAMERA_FX_USE_FOVX | CAMERA_FX_USE_FOVY | CAMERA_FX_USE_POS | CAMERA_FX_USE_ROT | CAMERA_FX_USE_NOCLIP;
		ceCS.fnCustomUpdate = CameraMgrFX_3rdPersonUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, we want to switch some information in the user data
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_3RDPERSON *pData = (CAMERAMGRFX_3RDPERSON*)pEffect->ceCS.pUserData;

		// If the cycle value is different, then just set that
		if(pData->bCycle != bCycle)
		{
			pData->bCycle = bCycle;
			g_vt3rdPersonCycl.SetFloat(g_vt3rdPersonAxis.GetFloat());
		}
		else
		{
			// Switch the state to the one we need to be handling
			switch(pData->nState)
			{
				case CAMERAMGRFX_STATE_DISABLED:
					pData->nState = CAMERAMGRFX_STATE_RAMPUP;
					break;

				case CAMERAMGRFX_STATE_RAMPUP:
					pData->nState = CAMERAMGRFX_STATE_RAMPDOWN;
					pEffect->fStartTime = g_pLTClient->GetTime() - (g_vt3rdPersonRamp.GetFloat() - (g_pLTClient->GetTime() - pEffect->fStartTime));
					break;

				case CAMERAMGRFX_STATE_ACTIVE:
					pData->nState = CAMERAMGRFX_STATE_RAMPDOWN;
					pEffect->fStartTime = g_pLTClient->GetTime();
					break;

				case CAMERAMGRFX_STATE_RAMPDOWN:
					pData->nState = CAMERAMGRFX_STATE_RAMPUP;
					pEffect->fStartTime = g_pLTClient->GetTime() - (g_vt3rdPersonRamp.GetFloat() - (g_pLTClient->GetTime() - pEffect->fStartTime));
					break;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_3rdPersonUpdate(HCAMERA hCamera, void *pEffect)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return LTTRUE;


	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_3RDPERSON *pData = (CAMERAMGRFX_3RDPERSON*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = 1.0f;

	// Do the ramp up of the position and FOV
	if(pData->nState == CAMERAMGRFX_STATE_RAMPUP)
	{
		fRamp = (fTime - pFX->fStartTime) / g_vt3rdPersonRamp.GetFloat();

		// If we've ramped completely out... set our distance and new state
		if(fRamp >= 1.0f)
		{
			fRamp = 1.0f;
			pData->fDist = g_vt3rdPersonDist.GetFloat();
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
		else
		{
			pData->fDist = g_vt3rdPersonDist.GetFloat() * sinf(fRamp * MATH_HALFPI);
		}
	}
	else if(pData->nState == CAMERAMGRFX_STATE_RAMPDOWN)
	{
		fRamp = 1.0f - ((fTime - pFX->fStartTime) / g_vt3rdPersonRamp.GetFloat());
		
		// If we've ramped completely in... set our distance and new state
		if(fRamp <= 0.0f)
		{
			pData->fDist = 0.0;
			pData->nState = CAMERAMGRFX_STATE_DISABLED;

			uint32 dwUserFlags;
			LTFLOAT r, g, b, a;
			g_pLTClient->GetObjectUserFlags(g_pLTClient->GetClientObject(), &dwUserFlags);

			if(!(dwUserFlags & USRFLG_CHAR_CLOAKED))
			{
				g_pLTClient->GetObjectColor(g_pLTClient->GetClientObject(), &r, &g, &b, &a);

				if(g_pGameClientShell->GetPlayerState() == PS_ALIVE)
					g_pLTClient->SetObjectColor(g_pLTClient->GetClientObject(), r, g, b, 1.0f);
			}

			return LTFALSE;
		}
		else
		{
			pData->fDist = g_vt3rdPersonDist.GetFloat() * sinf(fRamp * MATH_HALFPI);
		}
	}
	else
	{
		pData->fDist = g_vt3rdPersonDist.GetFloat();
	}


	// Find any offset special fx and get a camera position to test from
	LTVector vStartPos = pCamera->cidOriginal.vPos;
	CAMERAEFFECTINSTANCE *pMatch = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_POSOFFSET, LTNULL);

	while(pMatch)
	{
		vStartPos += pMatch->ceCS.vPos;

		if(!pMatch->pNext)
			break;

		pMatch = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_POSOFFSET, pMatch->pNext);
	}


	// Find any aiming special fx and get a rotation to offset by
	LTVector vOffsetRot = pCamera->cidOriginal.vRot;
	pMatch = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_AIMING, LTNULL);

	while(pMatch)
	{
		vOffsetRot += pMatch->ceCS.vRot;

		if(!pMatch->pNext)
			break;

		pMatch = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_AIMING, pMatch->pNext);
	}


	// If we should be cycling the axis of the rotation... don't allow other axis controls
	if(pData->bCycle)
	{
		LTFLOAT fNewAxis = g_vt3rdPersonCycl.GetFloat();
		fNewAxis += (g_vt3rdPersonSpin.GetFloat() * g_pLTClient->GetFrameTime());

		// Cap it at or after 360
		while(fNewAxis >= 360.0f)
			fNewAxis -= 360.0f;

		g_vt3rdPersonCycl.SetFloat(fNewAxis);
	}
	// If we're editing this camera rotation, update the rotation amount
	else
	{
		if(pData->bEdit)
		{
			LTFLOAT fNewAxis = g_vt3rdPersonAxis.GetFloat();
			LTFLOAT fScale = g_pLTClient->IsCommandOn(COMMAND_ID_RUN) ? 2.0f : 1.0f;
			LTFLOAT pOffsets[NUM_AXIS_OFFSETS];
			g_pLTClient->GetAxisOffsets(pOffsets);

			if(pOffsets[0])
				fNewAxis += (pOffsets[0] * g_vt3rdPersonMult.GetFloat());

			if(g_pLTClient->IsCommandOn(COMMAND_ID_LEFT))
				fNewAxis += (g_vt3rdPersonKeys.GetFloat() * g_pLTClient->GetFrameTime() * fScale);
			if(g_pLTClient->IsCommandOn(COMMAND_ID_RIGHT))
				fNewAxis -= (g_vt3rdPersonKeys.GetFloat() * g_pLTClient->GetFrameTime() * fScale);

			if(fNewAxis != g_vt3rdPersonAxis.GetFloat())
			{
				// Cap it between 0 and 360
				while((fNewAxis >= 360.0f) || (fNewAxis < 0.0f))
				{
					if(fNewAxis >= 360.0f)
						fNewAxis -= 360.0f;
					else if(fNewAxis < 0.0f)
						fNewAxis += 360.0f;
				}

				g_vt3rdPersonAxis.SetFloat(fNewAxis);
				pData->bChanged = LTTRUE;
			}
		}
		else
		{
			pData->bChanged = LTFALSE;
		}
	}


	// Setup a temporary rotation so we know what angle the camera needs to be moved
	LTVector vRadianAngles;
	LTRotation rOffsetRot;
	LTRotation rAxisRot;

	vRadianAngles.x = MATH_DEGREES_TO_RADIANS(vOffsetRot.x);
	vRadianAngles.y = MATH_DEGREES_TO_RADIANS(vOffsetRot.y + (pData->bCycle ? g_vt3rdPersonCycl.GetFloat() : g_vt3rdPersonAxis.GetFloat()));
	vRadianAngles.z = MATH_DEGREES_TO_RADIANS(vOffsetRot.z);
	g_pLTClient->GetMathLT()->SetupEuler(rOffsetRot, vRadianAngles);

	vRadianAngles.x = MATH_DEGREES_TO_RADIANS(-vOffsetRot.x);
	vRadianAngles.y = MATH_DEGREES_TO_RADIANS(-vOffsetRot.y);
	vRadianAngles.z = MATH_DEGREES_TO_RADIANS(-vOffsetRot.z);
	g_pLTClient->GetMathLT()->SetupEuler(rAxisRot, vRadianAngles);

	pFX->ceCS.rRot = rAxisRot * rOffsetRot;
	rOffsetRot = pCamera->cidOriginal.rRot * rOffsetRot;


	// Take the distance and move us back some from our original rotation
	LTVector vR, vU, vF;
	g_pLTClient->GetMathLT()->GetRotationVectors(rOffsetRot, vR, vU, vF);

	LTVector vEndPos;
	vEndPos = vStartPos + (vF * -pData->fDist);


	// Do an intersect backwards from our test position so we know where the
	// camera should be placed for it's final offset
	ClientIntersectInfo ii;
	ClientIntersectQuery iq;

	iq.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iq.m_From = vStartPos;
	iq.m_To = vEndPos;

	if(g_pLTClient->IntersectSegment(&iq, &ii))
	{
		LTVector vDir = vStartPos - vEndPos;
		vDir.Norm(g_vt3rdPersonBack.GetFloat());

		pFX->ceCS.vPos = ii.m_Point - iq.m_From + vDir;
	}
	else
	{
		pFX->ceCS.vPos = iq.m_To - iq.m_From;
	}

	// Handle the transparency of the object if the position is too close
	LTFLOAT r, g, b, a, fPercent;
	fPercent = pFX->ceCS.vPos.Mag() / g_vt3rdPersonFade.GetFloat();
	if(fPercent < 0.25f) fPercent = 0.25f;
	if(fPercent > 1.0f) fPercent = 1.0f;

	uint32 dwUserFlags;
	g_pLTClient->GetObjectUserFlags(g_pLTClient->GetClientObject(), &dwUserFlags);

	if(!(dwUserFlags & USRFLG_CHAR_CLOAKED))
	{
		g_pLTClient->GetObjectColor(g_pLTClient->GetClientObject(), &r, &g, &b, &a);

		if(g_pGameClientShell->GetPlayerState() == PS_ALIVE)
			g_pLTClient->SetObjectColor(g_pLTClient->GetClientObject(), r, g, b, fPercent);
		else
			g_pLTClient->SetObjectColor(g_pLTClient->GetClientObject(), r, g, b, 1.0f);
	}

	// Interpolate the field of view variables
	LTFLOAT fFOVX = MATH_DEGREES_TO_RADIANS(g_vt3rdPersonFOVX.GetFloat());
	LTFLOAT fFOVY = MATH_DEGREES_TO_RADIANS(g_vt3rdPersonFOVY.GetFloat());

	pFX->ceCS.fFOVX = (fFOVX - pCamera->cidOriginal.fFOVX) * sinf(fRamp * MATH_HALFPI);
	pFX->ceCS.fFOVY = (fFOVY - pCamera->cidOriginal.fFOVY) * sinf(fRamp * MATH_HALFPI);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CameraMgrFX_3rdPersonEdit(HCAMERA hCamera, LTBOOL bEdit)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_3RDPERSON, LTNULL);

	// If an effect doesn't exist, then just return
	if(!pEffect)
		return;

	// Create a user data structure
	CAMERAMGRFX_3RDPERSON *pData = (CAMERAMGRFX_3RDPERSON*)pEffect->ceCS.pUserData;

	if(pData)
		pData->bEdit = bEdit;
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_3rdPersonChanged(HCAMERA hCamera)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_3RDPERSON, LTNULL);

	// If an effect doesn't exist, then just return
	if(!pEffect)
		return LTFALSE;

	// Create a user data structure
	CAMERAMGRFX_3RDPERSON *pData = (CAMERAMGRFX_3RDPERSON*)pEffect->ceCS.pUserData;
	return pData->bChanged;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_WidescreenToggle(HCAMERA hCamera)
{
	// Find the widescreen effect on this camera if one exists already
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_WIDESCREEN, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_WIDESCREEN *pData = new CAMERAMGRFX_WIDESCREEN;
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->fScale = 0.0f;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_WIDESCREEN;
		ceCS.dwFlags = CAMERA_FX_USE_RECT;
		ceCS.fnCustomUpdate = CameraMgrFX_WidescreenUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, we want to switch some information in the user data
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_WIDESCREEN *pData = (CAMERAMGRFX_WIDESCREEN*)pEffect->ceCS.pUserData;

		// Switch the state to the one we need to be handling
		switch(pData->nState)
		{
			case CAMERAMGRFX_STATE_DISABLED:
				pData->nState = CAMERAMGRFX_STATE_RAMPUP;
				break;

			case CAMERAMGRFX_STATE_RAMPUP:
				pData->nState = CAMERAMGRFX_STATE_RAMPDOWN;
				pEffect->fStartTime = g_pLTClient->GetTime() - (g_vtWidescreenRamp.GetFloat() - (g_pLTClient->GetTime() - pEffect->fStartTime));
				break;

			case CAMERAMGRFX_STATE_ACTIVE:
				pData->nState = CAMERAMGRFX_STATE_RAMPDOWN;
				pEffect->fStartTime = g_pLTClient->GetTime();
				break;

			case CAMERAMGRFX_STATE_RAMPDOWN:
				pData->nState = CAMERAMGRFX_STATE_RAMPUP;
				pEffect->fStartTime = g_pLTClient->GetTime() - (g_vtWidescreenRamp.GetFloat() - (g_pLTClient->GetTime() - pEffect->fStartTime));
				break;
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_WidescreenUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_WIDESCREEN *pData = (CAMERAMGRFX_WIDESCREEN*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = 1.0f;

	// Do the ramp up of the scale
	if(pData->nState == CAMERAMGRFX_STATE_RAMPUP)
	{
		fRamp = (fTime - pFX->fStartTime) / g_vtWidescreenRamp.GetFloat();

		// If we've ramped completely out... set our new state
		if(fRamp >= 1.0f)
		{
			fRamp = 1.0f;
			pData->fScale = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
		else
		{
			pData->fScale = sinf(fRamp * MATH_HALFPI);
		}
	}
	else if(pData->nState == CAMERAMGRFX_STATE_RAMPDOWN)
	{
		fRamp = 1.0f - ((fTime - pFX->fStartTime) / g_vtWidescreenRamp.GetFloat());
		
		// If we've ramped completely in... set our new state
		if(fRamp <= 0.0f)
		{
			pData->fScale = 0.0;
			pData->nState = CAMERAMGRFX_STATE_DISABLED;
			return LTFALSE;
		}
		else
		{
			pData->fScale = sinf(fRamp * MATH_HALFPI);
		}
	}
	else
	{
		pData->fScale = 1.0f;
	}

	// Create a rect which will be our final rendering rect
	LTRect rRect;
	g_pCameraMgr->SetCameraRect(hCamera, rRect, LTTRUE);
	rRect = pCamera->cidOriginal.rRect;


	// Calculate a total offset required for the final transition
	int32 nRWidth = rRect.right - rRect.left;
	int32 nRHeight = rRect.bottom - rRect.top;
	int32 nXOffset = (nRWidth - (uint32)(nRWidth * g_vtWidescreenXSet.GetFloat())) / 2;
	int32 nYOffset = (nRHeight - (uint32)(nRHeight * g_vtWidescreenYSet.GetFloat())) / 2;

	// Scale these values
	nXOffset = (uint32)(nXOffset * pData->fScale);
	nYOffset = (uint32)(nYOffset * pData->fScale);

	// Create a final rect with offset interpolation values
	pFX->ceCS.rRect.Init(nXOffset, nYOffset, -nXOffset, -nYOffset);

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_FlashCreate(HCAMERA hCamera, LTVector vColor, LTVector vPos, LTFLOAT fLife,
			LTFLOAT fUp, LTFLOAT fDown, LTFLOAT fScaleDist, LTBOOL bLocal)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;


	CAMERAEFFECTINSTANCE *pMatch = LTNULL;
	CAMERAMGRFX_FLASH *pMatchData = LTNULL;

	// Make sure the color is ok...
	if((vColor.x > 1.0f) || (vColor.y > 1.0f) || (vColor.z > 1.0f) ||
		(vColor.x < 0.0f) || (vColor.y < 0.0f) || (vColor.z < 0.0f))
	{
		g_pLTClient->CPrint("CameraMgrFX Warning!  Color passed into FlashCreate is invalid!  Values = %f %f %f", vColor.x, vColor.y, vColor.z);
		return;
	}

	// Don't allow multiple ones of the same color for local flashes
	if(bLocal)
	{
		// Find any flash special fx and see if it's a duplicate color value
		CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_FLASH, LTNULL);

		while(pEffect)
		{
			if(pEffect->ceCS.pUserData)
			{
				pMatchData = (CAMERAMGRFX_FLASH*)pEffect->ceCS.pUserData;
				if(pMatchData->vColor == vColor)
				{
					pMatch = pEffect;
					break;
				}
			}

			if(!pEffect->pNext)
				break;

			pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_FLASH, pEffect->pNext);
		}
	}


	// If we found a duplicate colored flash... just setup that same flash with new values
	if(pMatch)
	{
		pMatch->fStartTime = g_pLTClient->GetTime();
		pMatchData->fLife = (fLife > pMatchData->fLife) ? fLife : pMatchData->fLife;
		pMatchData->fRampUp = 0.0f;
		pMatchData->fRampDown = (fDown > pMatchData->fRampDown) ? fDown : pMatchData->fRampDown;
	}
	// Otherwise, we need to create a new one... so go ahead and do that
	else
	{
		// Create a user data structure
		CAMERAMGRFX_FLASH *pData = new CAMERAMGRFX_FLASH;
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->vColor = vColor;
		pData->vPos = vPos;
		pData->fLife = fLife;
		pData->fRampUp = fUp;
		pData->fRampDown = fDown;
		pData->fScaleDist = fScaleDist;
		pData->bLocal = bLocal;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_FLASH;
		ceCS.dwFlags = CAMERA_FX_USE_LIGHTADD;
		ceCS.fnCustomUpdate = CameraMgrFX_FlashUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_FlashUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_FLASH *pData = (CAMERAMGRFX_FLASH*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = 1.0f;

	// Do the ramp up of the flash
	if(fTime < (pFX->fStartTime + pData->fRampUp))
//	if(pData->nState == CAMERAMGRFX_STATE_RAMPUP)
	{
		fRamp = (fTime - pFX->fStartTime) / pData->fRampUp;

		// If we've ramped completely out... set our new state
		if(fRamp >= 1.0f)
		{
			fRamp = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
	}
	else if(fTime > (pFX->fStartTime + pData->fRampUp + pData->fLife))
//	else if(pData->nState == CAMERAMGRFX_STATE_RAMPDOWN)
	{
		LTFLOAT fTempStart = pFX->fStartTime + pData->fRampUp + pData->fLife;
		fRamp = 1.0f - ((fTime - fTempStart) / pData->fRampDown);

		// If we've ramped completely in... set our new state
		if(fRamp <= 0.0f)
		{
			pData->nState = CAMERAMGRFX_STATE_DISABLED;
			return LTFALSE;
		}
	}
	else
//	else if(pData->nState == CAMERAMGRFX_STATE_ACTIVE)
	{
		if(fTime > (pFX->fStartTime + pData->fRampUp + pData->fLife))
		{
			pData->nState = CAMERAMGRFX_STATE_RAMPDOWN;
		}
	}

	// Setup the final scale value
	LTFLOAT fScale = 1.0f;

	// If it's not local to the camera... check some more data
	if(!pData->bLocal)
	{
		if(pData->fScaleDist < 100000.0f)
		{
			LTVector vCamPos;
			g_pCameraMgr->GetCameraPos(hCamera, vCamPos);

			// Check to make sure we can see this effect at all
			ClientIntersectQuery iQuery;
			ClientIntersectInfo iInfo;
			memset(&iQuery, 0, sizeof(iQuery));
			iQuery.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS;
			iQuery.m_From = vCamPos;
			iQuery.m_To = pData->vPos;

			if(g_pLTClient->IntersectSegment(&iQuery, &iInfo))
			{
				// Something is in the way.
				pFX->ceCS.vLightAdd = LTVector(0.0f, 0.0f, 0.0f);
				return LTTRUE;
			}


			// If the effect is too far away... just ignor it
			LTVector vDist = pData->vPos - vCamPos;
			LTFLOAT fMag = vDist.Mag();

			if(fMag > pData->fScaleDist)
			{
				// We're too far away
				pFX->ceCS.vLightAdd = LTVector(0.0f, 0.0f, 0.0f);
				return LTTRUE;
			}

			// Otherwise, scale down the effect some based off the dist
			fScale *= (1.0f - (fMag / pData->fScaleDist));


			// Now check the angle of the camera and the flash position
			// and scale again from that
			LTRotation rRot;
			LTVector vR, vU, vF;

			g_pCameraMgr->GetCameraRotation(hCamera, rRot);
			g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

			vDist.Norm();
			LTFLOAT fAngle = vDist.Dot(vF);
			fScale *= ((fAngle + 1.0f) / 2.0f);
		}
		else
		{
			fScale = 1.0f;
		}
	}


	// Create the final light add value
	pFX->ceCS.vLightAdd = pData->vColor * fScale * fRamp;


	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_ZoomToggle(HCAMERA hCamera, LTFLOAT fMagnitude, LTBOOL bFullbright)
{
	// Find the zoom effect on this camera if one exists already
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_ZOOM, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// If the magnitude is zero... just return cause there's no need to create an effect
		if(fMagnitude <= 1.0f)
			return;

		// Create a user data structure
		CAMERAMGRFX_ZOOM *pData = new CAMERAMGRFX_ZOOM;
		pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		pData->fStartMagnitude = 1.0f;
		pData->fMagnitude = 1.0f;
		pData->fDestMagnitude = fMagnitude;
		pData->bFullbright = bFullbright;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_ZOOM;
		ceCS.dwFlags = CAMERA_FX_USE_FOVX | CAMERA_FX_USE_FOVY;
		ceCS.fnCustomUpdate = CameraMgrFX_ZoomUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, we want to switch some information in the user data
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_ZOOM *pData = (CAMERAMGRFX_ZOOM*)pEffect->ceCS.pUserData;

		// Make sure the magnitude isn't less than 1
		if(fMagnitude < 1.0f)
			fMagnitude = 1.0f;

		// If the magnitude is different... set it to a new one
		if(pData->fDestMagnitude != fMagnitude)
		{
			pData->fStartMagnitude = pData->fMagnitude;
			pData->fDestMagnitude = fMagnitude;
			pData->bFullbright = bFullbright;
			pEffect->fStartTime = g_pLTClient->GetTime();
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_ZoomUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_ZOOM *pData = (CAMERAMGRFX_ZOOM*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = (fTime - pFX->fStartTime) / g_vtZoomRamp.GetFloat();

	// If we've ramped completely... set it to the max
	if(fRamp > 1.0f) fRamp = 1.0f;

	// Set our current magnitude
	pData->fMagnitude = pData->fStartMagnitude + ((pData->fDestMagnitude - pData->fStartMagnitude) * fRamp);

	// Cap the fMagnitude if it drops below 1
	if(pData->fMagnitude < 1.0f)
		pData->fMagnitude = 1.0f;

	// If the offsets are zero... then clear this effect
	if((pData->fDestMagnitude <= 1.0f) && (pData->fMagnitude <= 1.0f))
	{
		g_pLTClient->RunConsoleString("LMFullBright 0");
		g_pLTClient->RunConsoleString("ModelZoomScale 1.0");

		g_pGameClientShell->SetMouseSensitivityScale();

		pData->nState = CAMERAMGRFX_STATE_DISABLED;
		return LTFALSE;
	}

	// Find 'A' assuming that our original FOV represents an 'O' of 1
	LTFLOAT fAX = 1.0f / (LTFLOAT)tan(pCamera->cidOriginal.fFOVX / 2.0f);
	LTFLOAT fFOVXDest = (LTFLOAT)atan(1.0f / pData->fMagnitude / fAX);

	LTFLOAT fOY = fAX * (LTFLOAT)tan(pCamera->cidOriginal.fFOVY / 2.0f);
	LTFLOAT fFOVYDest = (LTFLOAT)atan(fOY / pData->fMagnitude / fAX);

	// Set the final FOV offset values
	pFX->ceCS.fFOVX = ((fFOVXDest * 2.0f) - pCamera->cidOriginal.fFOVX);
	pFX->ceCS.fFOVY = ((fFOVYDest * 2.0f) - pCamera->cidOriginal.fFOVY);


	// ----------------------------------------------------------------------- //
	// Calculate a new FOV scale
	char strConsole[128];
	sprintf(strConsole, "ModelZoomScale %f", pData->fMagnitude);
	g_pLTClient->RunConsoleString(strConsole);


	// ----------------------------------------------------------------------- //
	// See if we want to force fullbright
	if(pData->bFullbright)
	{
		g_pLTClient->RunConsoleString("LMFullBright 1");
	}


	// ----------------------------------------------------------------------- //
	// Calculate new mouse sensitivity
	LTFLOAT fSensScale = g_vtZoomSensScale.GetFloat() / (pData->fMagnitude / g_vtZoomSensMag.GetFloat());
	g_pGameClientShell->SetMouseSensitivityScale(fSensScale);


	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_QuickZoomCreate(HCAMERA hCamera, LTFLOAT fMagAdjust, LTFLOAT fTime, LTFLOAT fRollAmount)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;


	// If the magnitude or time is not valid... just return cause there's no need to create an effect
	if((fMagAdjust <= 1.0f) || (fTime <= 0.0f))
		return;

	// Find the quick zoom effect on this camera if one exists already
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_QUICKZOOM, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_QUICKZOOM *pData = new CAMERAMGRFX_QUICKZOOM;
		pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		pData->fMagnitudeAdjust = fMagAdjust;
		pData->fTime = fTime;
		pData->fRollAmount = GetRandom(-fRollAmount, fRollAmount);

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_QUICKZOOM;
		ceCS.dwFlags = CAMERA_FX_USE_FOVX | CAMERA_FX_USE_FOVY | CAMERA_FX_USE_ROT;
		ceCS.fnCustomUpdate = CameraMgrFX_QuickZoomUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just reset the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_QUICKZOOM *pData = (CAMERAMGRFX_QUICKZOOM*)pEffect->ceCS.pUserData;

		// If the magnitude is different... set it to a new one
		pEffect->fStartTime = g_pLTClient->GetTime();
		pData->fMagnitudeAdjust = fMagAdjust;
		pData->fTime = fTime;
		pData->fRollAmount = GetRandom(-fRollAmount, fRollAmount);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_QuickZoomUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_QUICKZOOM *pData = (CAMERAMGRFX_QUICKZOOM*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = (fTime - pFX->fStartTime) / pData->fTime;

	// If we've done the entire zoom... so just return false to destroy the effect
	if(fRamp >= 1.0f)
	{
		pData->nState = CAMERAMGRFX_STATE_DISABLED;
		return LTFALSE;
	}

	// Recalculate the ramp value to account for the ramp up and ramp down
	if(fRamp > 0.5f)
		fRamp = 0.5f - (fRamp - 0.5f);

	fRamp *= 2.0f;

	// Set our current magnitude
	LTFLOAT fMag = 1.0f + ((pData->fMagnitudeAdjust - 1.0f) * fRamp);


	// Find 'A' assuming that our original FOV represents an 'O' of 1
	LTFLOAT fAX = 1.0f / (LTFLOAT)tan(pCamera->cidOriginal.fFOVX / 2.0f);
	LTFLOAT fFOVXDest = (LTFLOAT)atan(1.0f / fMag / fAX);

	LTFLOAT fOY = fAX * (LTFLOAT)tan(pCamera->cidOriginal.fFOVY / 2.0f);
	LTFLOAT fFOVYDest = (LTFLOAT)atan(fOY / fMag / fAX);

	// Set the final FOV offset values
	pFX->ceCS.fFOVX = ((fFOVXDest * 2.0f) - pCamera->cidOriginal.fFOVX);
	pFX->ceCS.fFOVY = ((fFOVYDest * 2.0f) - pCamera->cidOriginal.fFOVY);


	// Calculate the roll amount
	LTVector vRot(0.0f, 0.0f, 0.0f);
	vRot.z = MATH_DEGREES_TO_RADIANS(pData->fRollAmount * fRamp);
	g_pLTClient->GetMathLT()->SetupEuler(pFX->ceCS.rRot, vRot);


	// Calculate a new FOV scale
	LTFLOAT fFOVScale = (fMag / g_vtZoomLODMag.GetFloat()) * g_vtZoomLODScale.GetFloat();

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_TiltCreate(HCAMERA hCamera, LTVector vAmount, LTVector vTimes)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;


	CAMERAEFFECTINSTANCE *pMatch = LTNULL;
	CAMERAMGRFX_TILT *pMatchData = LTNULL;

	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_TILT, LTNULL);

	// Make the amounts in radians instead of degrees
	vAmount.x = MATH_DEGREES_TO_RADIANS(vAmount.x);
	vAmount.y = MATH_DEGREES_TO_RADIANS(vAmount.y);
	vAmount.z = MATH_DEGREES_TO_RADIANS(vAmount.z);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_TILT *pData = new CAMERAMGRFX_TILT;
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->vAmount = vAmount;
		pData->vTimes = vTimes;
		pData->vStartAmount = LTVector(0.0f, 0.0f, 0.0f);
		pData->vCurrentAmount = LTVector(0.0f, 0.0f, 0.0f);

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_TILT;
		ceCS.dwFlags = CAMERA_FX_USE_ROT;
		ceCS.fnCustomUpdate = CameraMgrFX_TiltUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just reset the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_TILT *pData = (CAMERAMGRFX_TILT*)pEffect->ceCS.pUserData;

		// If the magnitude is different... set it to a new one
		pEffect->fStartTime = g_pLTClient->GetTime();
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->vAmount = vAmount;
		pData->vTimes = vTimes;
		pData->vStartAmount = LTVector(0.0f, 0.0f, 0.0f);
//		pData->vStartAmount = pData->vCurrentAmount;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_TiltUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_TILT *pData = (CAMERAMGRFX_TILT*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = 1.0f;

	// Do the ramp up of the tilt
	if(fTime < (pFX->fStartTime + pData->vTimes.x))
	{
		fRamp = (fTime - pFX->fStartTime) / pData->vTimes.x;

		// If we've ramped completely out... set our new state
		if(fRamp >= 1.0f)
		{
			fRamp = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
	}


	// Do the ramp up of the flash
	if(fTime < (pFX->fStartTime + pData->vTimes.x))		// CAMERAMGRFX_STATE_RAMPUP
	{
		fRamp = (fTime - pFX->fStartTime) / pData->vTimes.x;

		// If we've ramped completely out... set our new state
		if(fRamp >= 1.0f)
		{
			fRamp = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
	}
	else if(fTime > (pFX->fStartTime + pData->vTimes.x + pData->vTimes.y))	// CAMERAMGRFX_STATE_RAMPDOWN
	{
		LTFLOAT fTempStart = pFX->fStartTime + pData->vTimes.x + pData->vTimes.y;
		fRamp = 1.0f - ((fTime - fTempStart) / pData->vTimes.z);

		// If we've ramped completely in... set our new state
		if(fRamp <= 0.0f)
		{
			pData->nState = CAMERAMGRFX_STATE_DISABLED;
			return LTFALSE;
		}
	}

	// Create the final light add value
	pData->vCurrentAmount = pData->vStartAmount + ((pData->vAmount - pData->vStartAmount) * fRamp);
	g_pLTClient->GetMathLT()->SetupEuler(pFX->ceCS.rRot, pData->vCurrentAmount);

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_HeadBobCreate(HCAMERA hCamera)
{
	// Create a user data structure
	CAMERAEFFECTCREATESTRUCT ceCS;
	ceCS.nID = CAMERAMGRFX_ID_HEAD_BOB;
	ceCS.dwFlags = CAMERA_FX_USE_POS;
	ceCS.fnCustomUpdate = CameraMgrFX_HeadBobUpdate;
	ceCS.pUserData = LTNULL;

	g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
	return;
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_HeadBobUpdate(HCAMERA hCamera, void *pEffect)
{
	//for now don't do anything

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CameraMgrFX_SetHeadBob(HCAMERA hCamera, LTFLOAT fAmount)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_HEAD_BOB, LTNULL);

	if(!pEffect) return;

	LTRotation rRot;
	g_pCameraMgr->GetCameraRotation(hCamera, rRot);

	LTVector vR, vU, vF;
	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	//set the new offset
	pEffect->ceCS.vPos = vU * fAmount;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_HeadTiltCreate(HCAMERA hCamera, LTFLOAT fAngle, LTFLOAT fSpeed)
{
	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_HEAD_TILT, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_HEADTILT *pData = new CAMERAMGRFX_HEADTILT;
		pData->fAngle = fAngle;
		pData->fSpeed = fSpeed;
		pData->fDirection = 0.0f;
		pData->fRoll = 0.0f;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_HEAD_TILT;
		ceCS.dwFlags = CAMERA_FX_USE_ROT;
		ceCS.fnCustomUpdate = CameraMgrFX_HeadTiltUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just update the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_HEADTILT *pData = (CAMERAMGRFX_HEADTILT*)pEffect->ceCS.pUserData;
		pData->fAngle = fAngle;
		pData->fSpeed = fSpeed;
	}

	return;
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_HeadTiltUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_HEADTILT *pData = (CAMERAMGRFX_HEADTILT*)pFX->ceCS.pUserData;


	LTFLOAT fDestAngle = pData->fAngle * pData->fDirection;

	// Calculate the new tilt value
	if(pData->fRoll > fDestAngle)
	{
		pData->fRoll -= pData->fSpeed * g_pLTClient->GetFrameTime();

		if(pData->fRoll < fDestAngle)
			pData->fRoll = fDestAngle;
	}
	else if(pData->fRoll < fDestAngle)
	{
		pData->fRoll += pData->fSpeed * g_pLTClient->GetFrameTime();

		if(pData->fRoll > fDestAngle)
			pData->fRoll = fDestAngle;
	}


	// Set the rotation
	pFX->ceCS.rRot.Init();
	g_pLTClient->GetMathLT()->EulerRotateZ(pFX->ceCS.rRot, MATH_DEGREES_TO_RADIANS(pData->fRoll));


	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CameraMgrFX_SetHeadTiltAmount(HCAMERA hCamera, LTFLOAT fDirection)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_HEAD_TILT, LTNULL);
	if(!pEffect) return;


	CAMERAMGRFX_HEADTILT *pData = (CAMERAMGRFX_HEADTILT*)pEffect->ceCS.pUserData;
	pData->fDirection = fDirection;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_ShakeCreate(HCAMERA hCamera, LTVector vAddAmount)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;

	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_SHAKE, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_SHAKE *pData = new CAMERAMGRFX_SHAKE;
		pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		pData->vAmount = vAddAmount;

		LTFLOAT fMax = g_vtShakeMaxAmount.GetFloat();
		if(pData->vAmount.x > fMax) pData->vAmount.x = fMax;
		if(pData->vAmount.y > fMax) pData->vAmount.y = fMax;
		if(pData->vAmount.z > fMax) pData->vAmount.z = fMax;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_SHAKE;
		ceCS.dwFlags = CAMERA_FX_USE_POS;
		ceCS.fnCustomUpdate = CameraMgrFX_ShakeUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just update the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_SHAKE *pData = (CAMERAMGRFX_SHAKE*)pEffect->ceCS.pUserData;

		// If the magnitude is different... set it to a new one
		pEffect->fStartTime = g_pLTClient->GetTime();
		pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		pData->vAmount += vAddAmount;

		LTFLOAT fMax = g_vtShakeMaxAmount.GetFloat();
		if(pData->vAmount.x > fMax) pData->vAmount.x = fMax;
		if(pData->vAmount.y > fMax) pData->vAmount.y = fMax;
		if(pData->vAmount.z > fMax) pData->vAmount.z = fMax;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_ShakeUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_SHAKE *pData = (CAMERAMGRFX_SHAKE*)pFX->ceCS.pUserData;


	// Calculate the decay amount
	LTFLOAT fDecayAmount = g_vtShakeDecayAmount.GetFloat() * g_pLTClient->GetFrameTime();

	pData->vAmount.x -= fDecayAmount;
	pData->vAmount.y -= fDecayAmount;
	pData->vAmount.z -= fDecayAmount;

	if(pData->vAmount.x < 0.0f) pData->vAmount.x = 0.0f;
	if(pData->vAmount.y < 0.0f) pData->vAmount.y = 0.0f;
	if(pData->vAmount.z < 0.0f) pData->vAmount.z = 0.0f;

	if(pData->vAmount.x <= 0.0f && pData->vAmount.y <= 0.0f && pData->vAmount.z <= 0.0f)
		return LTFALSE;


	// Apply the screen shake to the fx position
    LTFLOAT faddX = GetRandom(-1.0f, 1.0f) * pData->vAmount.x * 3.0f;
    LTFLOAT faddY = GetRandom(-1.0f, 1.0f) * pData->vAmount.y * 3.0f;
    LTFLOAT faddZ = GetRandom(-1.0f, 1.0f) * pData->vAmount.z * 3.0f;
	LTVector vAdd(faddX, faddY, faddZ);

	pFX->ceCS.vPos = vAdd;


	// See if we need to adjust the weapon position
	CWeaponModel *pWeapon = g_pGameClientShell->GetWeaponModel();

	if(pWeapon)
	{
		HLOCALOBJ hWeapon = pWeapon->GetHandle();

		if(hWeapon)
		{
			LTVector vPos;
			g_pLTClient->GetObjectPos(hWeapon, &vPos);

			vAdd += 0.95f;
			vPos += vAdd;

			g_pLTClient->SetObjectPos(hWeapon, &vPos);
		}
	}


	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_WarbleCreate(HCAMERA hCamera, LTFLOAT fXScale, LTFLOAT fYScale)
{
	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_WARBLE, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_WARBLE *pData = new CAMERAMGRFX_WARBLE;
		pData->fXScale = fXScale;
		pData->fYScale = fYScale;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_WARBLE;
		ceCS.dwFlags = CAMERA_FX_USE_FOVX | CAMERA_FX_USE_FOVY;
		ceCS.fnCustomUpdate = CameraMgrFX_WarbleUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);

		return;
	}

	// Otherwise, just update the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_WARBLE *pData = (CAMERAMGRFX_WARBLE*)pEffect->ceCS.pUserData;
		pData->fXScale = fXScale;
		pData->fYScale = fYScale;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_WarbleUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_WARBLE *pData = (CAMERAMGRFX_WARBLE*)pFX->ceCS.pUserData;


	LTFLOAT fCalc1 = (g_pLTClient->GetTime() - pFX->fStartTime) * MATH_PI;
	LTFLOAT fCalc2 = fCalc1 - MATH_HALFPI;

	LTFLOAT fFOVXScale = sinf(fCalc1 / g_vtWarbleFOVPhase.GetFloat()) * pData->fXScale;
	LTFLOAT fFOVYScale = sinf(fCalc2 / g_vtWarbleFOVPhase.GetFloat()) * pData->fYScale;


	// Fill in the final FX values
	pFX->ceCS.fFOVX = MATH_DEGREES_TO_RADIANS(fFOVXScale);
	pFX->ceCS.fFOVY = MATH_DEGREES_TO_RADIANS(fFOVYScale);

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_WonkyToggle(HCAMERA hCamera, LTFLOAT fScale)
{
	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_WONKY, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_WONKY *pData = new CAMERAMGRFX_WONKY;
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->fScale = fScale;

		pData->fPitchScale = 0.0f;
		pData->fYawScale = 0.0f;
		pData->fRollScale = 0.0f;
		pData->fFOVScale = 0.0f;
		pData->fFOVX = 0.0f;
		pData->fFOVY = 0.0f;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_WONKY;
		ceCS.dwFlags = CAMERA_FX_USE_FOVX | CAMERA_FX_USE_FOVY | CAMERA_FX_USE_ROT;
		ceCS.fnCustomUpdate = CameraMgrFX_WonkyUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);

		return;
	}

	// Otherwise, just update the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_WONKY *pData = (CAMERAMGRFX_WONKY*)pEffect->ceCS.pUserData;

		switch(pData->nState)
		{
			case CAMERAMGRFX_STATE_RAMPUP:
			case CAMERAMGRFX_STATE_ACTIVE:
			{
				pData->nState = CAMERAMGRFX_STATE_RAMPDOWN;
				pEffect->fStartTime = g_pLTClient->GetTime();
				break;
			}

			case CAMERAMGRFX_STATE_RAMPDOWN:
			{
				pData->nState = CAMERAMGRFX_STATE_RAMPUP;
				pData->fScale = fScale;

				pData->fPitchScale = 0.0f;
				pData->fYawScale = 0.0f;
				pData->fRollScale = 0.0f;
				pData->fFOVScale = 0.0f;
				pData->fFOVX = 0.0f;
				pData->fFOVY = 0.0f;

				pEffect->fStartTime = g_pLTClient->GetTime();
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_WonkyUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_WONKY *pData = (CAMERAMGRFX_WONKY*)pFX->ceCS.pUserData;


	// Some variables to calculate information vith
	LTFLOAT fCalc = 0.0f;


	// Do something different for each state
	switch(pData->nState)
	{
		case CAMERAMGRFX_STATE_RAMPUP:
		{
			LTFLOAT fNewScale = pData->fScale;

			if((g_pLTClient->GetTime() - pFX->fStartTime) > g_vtWonkyRampUp.GetFloat())
			{
				pData->nState = CAMERAMGRFX_STATE_ACTIVE;
			}
			else
			{
				fNewScale = (g_pLTClient->GetTime() - pFX->fStartTime) / g_vtWonkyRampUp.GetFloat() * pData->fScale;
			}

			fCalc = (g_pLTClient->GetTime() - pFX->fStartTime) * MATH_PI;

			pData->fPitchScale = sinf(fCalc / g_vtWonkyPitchPhase.GetFloat()) * fNewScale;
			pData->fYawScale = sinf(fCalc / g_vtWonkyYawPhase.GetFloat()) * fNewScale;
			pData->fRollScale = sinf(fCalc / g_vtWonkyRollPhase.GetFloat()) * fNewScale;
			pData->fFOVScale = sinf(fCalc / g_vtWonkyFOVPhase.GetFloat()) * fNewScale;

			break;
		}

		case CAMERAMGRFX_STATE_ACTIVE:
		{
			fCalc = (g_pLTClient->GetTime() - pFX->fStartTime) * MATH_PI;

			pData->fPitchScale = sinf(fCalc / g_vtWonkyPitchPhase.GetFloat()) * pData->fScale;
			pData->fYawScale = sinf(fCalc / g_vtWonkyYawPhase.GetFloat()) * pData->fScale;
			pData->fRollScale = sinf(fCalc / g_vtWonkyRollPhase.GetFloat()) * pData->fScale;
			pData->fFOVScale = sinf(fCalc / g_vtWonkyFOVPhase.GetFloat()) * pData->fScale;

			break;
		}

		case CAMERAMGRFX_STATE_RAMPDOWN:
		{
			// See if we're done here
			if((g_pLTClient->GetTime() - pFX->fStartTime) > g_vtWonkyRampDown.GetFloat())
			{
				return LTFALSE;
			}
			else
			{
				fCalc = 1.0f - ((g_pLTClient->GetTime() - pFX->fStartTime) / g_vtWonkyRampDown.GetFloat());

				pData->fPitchScale *= fCalc;
				pData->fYawScale *= fCalc;
				pData->fRollScale *= fCalc;
				pData->fFOVScale *= fCalc;
			}

			break;
		}
	}


	// Setup the rotation values
	LTFLOAT fP, fY, fR;
	fP = MATH_DEGREES_TO_RADIANS(g_vtWonkyPitch.GetFloat() * pData->fPitchScale);
	fY = MATH_DEGREES_TO_RADIANS(g_vtWonkyYaw.GetFloat() * pData->fYawScale);
	fR = MATH_DEGREES_TO_RADIANS(g_vtWonkyRoll.GetFloat() * pData->fRollScale);


	// Fill in the final FX values
	g_pMathLT->SetupEuler(pFX->ceCS.rRot, fP, fY, fR);

	pData->fFOVX = g_vtWonkyFOVX.GetFloat() * pData->fFOVScale;
	pData->fFOVY = g_vtWonkyFOVY.GetFloat() * pData->fFOVScale;

	pFX->ceCS.fFOVX = MATH_DEGREES_TO_RADIANS(pData->fFOVX);
	pFX->ceCS.fFOVY = MATH_DEGREES_TO_RADIANS(pData->fFOVY);

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_VolumeBrushCreate(HCAMERA hCamera, uint8 nNumContainers)
{
	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_VOLUMEBRUSH, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_VOLUMEBRUSH *pData = new CAMERAMGRFX_VOLUMEBRUSH;
		pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		pData->nNumContainers = nNumContainers;
		pData->bInContainer = LTFALSE;
		pData->bInLiquid = LTFALSE;
		pData->hSound = LTNULL;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_VOLUMEBRUSH;
		ceCS.dwFlags = CAMERA_FX_USE_LIGHTADD | CAMERA_FX_USE_LIGHTSCALE;
		ceCS.fnCustomUpdate = CameraMgrFX_VolumeBrushUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL	CameraMgrFX_VolumeBrushUpdate(HCAMERA hCamera, void *pEffect)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return LTTRUE;

	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_VOLUMEBRUSH *pData = (CAMERAMGRFX_VOLUMEBRUSH*)pFX->ceCS.pUserData;

	LTBOOL bInContainer = LTFALSE;
	LTBOOL bInLiquid = LTFALSE;


	// Get the complete position of the camera (after FX and everything)
	LTVector vPos;
	g_pCameraMgr->GetCameraPos(hCamera, vPos, LTTRUE);


	// Get a pointer to the special FX manager
	CSFXMgr *pSFXMgr = g_pGameClientShell->GetSFXMgr();
	if(!pSFXMgr) return LTTRUE;


	// Now get the container list at this position
	HLOCALOBJ objList[1];
	uint32 nListSize = g_pLTClient->GetPointContainers(&vPos, objList, 1);

	uint16 nCode = 0;
	uint32 nUsrFlgs = 0;

	// Go through the list of containers and update our values appropriately
	for(uint32 i = 0; i < nListSize; i++)
	{
		// Vertify it is a container
		if(g_pLTClient->GetContainerCode(objList[i], &nCode))
		{
			// Verifiy that we want to use this container for the FX
			g_pLTClient->GetObjectUserFlags(objList[i], &nUsrFlgs);

			if(!(nUsrFlgs & USRFLG_VISIBLE))
				continue;


			// Get the FX information (check weather brushes and normal volume brushes)
			CVolumeBrushFX* pVBFX = (CVolumeBrushFX*)pSFXMgr->FindSpecialFX(SFX_WEATHER_ID, objList[i]);
			if(!pVBFX) pVBFX = (CVolumeBrushFX*)pSFXMgr->FindSpecialFX(SFX_VOLUMEBRUSH_ID, objList[i]);
			if(!pVBFX) continue;

			bInContainer = LTTRUE;

			// Get the information from the FX
			LTBOOL bNormalViewMode = (pSFXMgr->GetMode() == "Normal");
			LTBOOL bFog = pVBFX->IsFogEnable() && bNormalViewMode;
			LTVector vFogColor = pVBFX->GetFogColor();

			// If we want to use fog... set up the console variables
			if(bFog)
			{
				// Grab the current scale value...
				LTFLOAT fScale = 1.0f;

/*				CGameSettings *pSettings = g_pGameClientShell->GetInterfaceMgr()->GetSettings();

				if(pSettings)
				{
					fScale = pSettings->GetFloatVar("FogPerformanceScale") / 100.0f;
				}
*/
				char buf[30];
				// turn off vFog since it is not compatible with
				// our normal distance fog.  It will get turned back on
				// when we call ResetGlobalFog()

				sprintf(buf, "vFog 0");
				g_pClientDE->RunConsoleString(buf);

				sprintf(buf, "FogEnable %d", (int)bFog);
				g_pLTClient->RunConsoleString(buf);

				sprintf(buf, "FogNearZ %d", (int)(pVBFX->GetMinFogNearZ() + ((pVBFX->GetFogNearZ() - pVBFX->GetMinFogNearZ()) * fScale)));
				g_pLTClient->RunConsoleString(buf);

				sprintf(buf, "FogFarZ %d", (int)(pVBFX->GetMinFogFarZ() + ((pVBFX->GetFogFarZ() - pVBFX->GetMinFogFarZ()) * fScale)));
				g_pLTClient->RunConsoleString(buf);

				sprintf(buf, "FogR %d", (int)vFogColor.x);
				g_pLTClient->RunConsoleString(buf);

				sprintf(buf, "FogG %d", (int)vFogColor.y);
				g_pLTClient->RunConsoleString(buf);

				sprintf(buf, "FogB %d", (int)vFogColor.z);
				g_pLTClient->RunConsoleString(buf);

				// Now set the void color to match...
				g_pGameClientShell->SetVoidColor(vFogColor);
			}

			// Set the camera FX values
			if(bNormalViewMode)
			{
				pFX->ceCS.vLightScale = pVBFX->GetTintColor();
			}
			else
			{
				pFX->ceCS.vLightScale = LTVector(1.0f, 1.0f, 1.0f);
			}

			pFX->ceCS.vLightAdd = pVBFX->GetLightAdd();


			// See if this is a liquid volume
			if(IsLiquid((ContainerCode)nCode))
				bInLiquid = LTTRUE;
		}
	}


	// Check to see if we've just entered a container
	if(!pData->bInContainer && bInContainer)
	{

	}
	// Otherwise, see if we just exited a container
	else if(pData->bInContainer && !bInContainer)
	{
		// Reset the fog back to what it was...
		if(pSFXMgr->GetMode() == "Normal")
			g_pGameClientShell->ResetGlobalFog();

		// Clear out our FX variables
		pFX->ceCS.vLightScale = LTVector(1.0f, 1.0f, 1.0f);
		pFX->ceCS.vLightAdd = LTVector(0.0f, 0.0f, 0.0f);
	}


	// Set our save variable
	pData->bInContainer = bInContainer;
	pData->bInLiquid = bInLiquid;


	// Check to see whether we should play an underwater sound or not
	if(pData->bInLiquid && !pData->hSound)
	{
		pData->hSound = g_pClientSoundMgr->PlaySoundLocal("Sounds\\Characters\\unwater.wav", SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP);
	}
	else if(!pData->bInLiquid && pData->hSound)
	{
		g_pLTClient->KillSound(pData->hSound);
		pData->hSound = LTNULL;
	}


	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_VolumeBrushInContainer(HCAMERA hCamera)
{
	// Find any vb special fx
	CAMERAEFFECTINSTANCE *pFX = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_VOLUMEBRUSH, LTNULL);

	// If an effect doesn't exist, return false
	if(!pFX) return LTFALSE;

	CAMERAMGRFX_VOLUMEBRUSH *pData = (CAMERAMGRFX_VOLUMEBRUSH*)pFX->ceCS.pUserData;
	return pData->bInContainer;
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_VolumeBrushInLiquid(HCAMERA hCamera)
{
	// Find any vb special fx
	CAMERAEFFECTINSTANCE *pFX = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_VOLUMEBRUSH, LTNULL);

	// If an effect doesn't exist, return false
	if(!pFX) return LTFALSE;

	CAMERAMGRFX_VOLUMEBRUSH *pData = (CAMERAMGRFX_VOLUMEBRUSH*)pFX->ceCS.pUserData;
	return pData->bInLiquid;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_QuickLungeCreate(HCAMERA hCamera, LTVector vPosOffset, LTVector vRotOffset, LTFLOAT fTime)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;


	// If the time is not valid... just return cause there's no need to create an effect
	if(fTime <= 0.0f)
		return;

	// Find the quick zoom effect on this camera if one exists already
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_QUICKLUNGE, LTNULL);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_QUICKLUNGE *pData = new CAMERAMGRFX_QUICKLUNGE;
		pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		pData->vPosOffset = vPosOffset;
		pData->vRotOffset = vRotOffset;
		pData->fTime = fTime;

		pData->vRotOffset.x = MATH_DEGREES_TO_RADIANS(pData->vRotOffset.x);
		pData->vRotOffset.y = MATH_DEGREES_TO_RADIANS(pData->vRotOffset.y);
		pData->vRotOffset.z = MATH_DEGREES_TO_RADIANS(pData->vRotOffset.z);

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_QUICKLUNGE;
		ceCS.dwFlags = CAMERA_FX_USE_POS | CAMERA_FX_USE_ROT | CAMERA_FX_USE_FOVX | CAMERA_FX_USE_FOVY;
		ceCS.fnCustomUpdate = CameraMgrFX_QuickLungeUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just reset the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_QUICKLUNGE *pData = (CAMERAMGRFX_QUICKLUNGE*)pEffect->ceCS.pUserData;

		pEffect->fStartTime = g_pLTClient->GetTime();
		pData->vPosOffset = vPosOffset;
		pData->vRotOffset = vRotOffset;
		pData->fTime = fTime;

		pData->vRotOffset.x = MATH_DEGREES_TO_RADIANS(pData->vRotOffset.x);
		pData->vRotOffset.y = MATH_DEGREES_TO_RADIANS(pData->vRotOffset.y);
		pData->vRotOffset.z = MATH_DEGREES_TO_RADIANS(pData->vRotOffset.z);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_QuickLungeUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_QUICKLUNGE *pData = (CAMERAMGRFX_QUICKLUNGE*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = (fTime - pFX->fStartTime) / pData->fTime;

	// If we've done the entire lunge... just return false to destroy the effect
	if(fRamp >= 1.0f)
	{
		pData->nState = CAMERAMGRFX_STATE_DISABLED;
		return LTFALSE;
	}

	// Recalculate the ramp value to account for the ramp up and ramp down
	if(fRamp > 0.5f)
		fRamp = 0.5f - (fRamp - 0.5f);

	fRamp *= 2.0f;

	// Create a final position and rotation offset
	LTRotation rRot;
	g_pCameraMgr->GetCameraRotation(hCamera, rRot);

	LTVector vR, vU, vF;
	g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

	pFX->ceCS.vPos = ((vR * pData->vPosOffset.x) + (vU * pData->vPosOffset.y)) * fRamp;

	LTVector vRotOffset = pData->vRotOffset * fRamp;
	g_pLTClient->GetMathLT()->SetupEuler(pFX->ceCS.rRot, vRotOffset);

	pFX->ceCS.fFOVX = MATH_DEGREES_TO_RADIANS(pData->vPosOffset.z) * fRamp;
	pFX->ceCS.fFOVY = MATH_DEGREES_TO_RADIANS(pData->vPosOffset.z) * fRamp;

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

#define VISIONCYCLE_TYPE_MARINE			0
#define VISIONCYCLE_TYPE_PREDATOR		1
#define VISIONCYCLE_TYPE_ALIEN			2

#define VISIONCYCLE_FLAGS_MARINE		(0)
#define VISIONCYCLE_FLAGS_PREDATOR		(CAMERA_FX_USE_LIGHTADD | CAMERA_FX_USE_RECT)
#define VISIONCYCLE_FLAGS_ALIEN			(CAMERA_FX_USE_LIGHTADD | CAMERA_FX_USE_RECT)

// ----------------------------------------------------------------------- //

void CameraMgrFX_VisionCycleCreate(HCAMERA hCamera)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;


	// Find the quick zoom effect on this camera if one exists already
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_VISIONCYCLE, LTNULL);

	// Get the character type
	uint8 nType = VISIONCYCLE_TYPE_MARINE;

	switch(g_pGameClientShell->GetPlayerMovement()->GetCharacterButes()->m_eCharacterClass)
	{
		case PREDATOR:	nType = VISIONCYCLE_TYPE_PREDATOR;		break;
		case ALIEN:		nType = VISIONCYCLE_TYPE_ALIEN;			break;
	}

	// Get the FX flags
	uint32 dwFlags = 0;
	char *szSound = LTNULL;

	switch(nType)
	{
		case VISIONCYCLE_TYPE_MARINE:
		{
			dwFlags = VISIONCYCLE_FLAGS_MARINE;
			break;
		}

		case VISIONCYCLE_TYPE_PREDATOR:
		{
			dwFlags = VISIONCYCLE_FLAGS_PREDATOR;
			szSound = "PredatorVisionModeCycle";
			g_pInterfaceMgr->SetPredVisionTransition(LTTRUE);
			break;
		}

		case VISIONCYCLE_TYPE_ALIEN:
		{
			dwFlags = VISIONCYCLE_FLAGS_ALIEN;
			szSound = "alien_vision_switch";
			break;
		}
	}

	if(!dwFlags) return;
	if(szSound) g_pClientSoundMgr->PlaySoundLocal(szSound);


	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_VISIONCYCLE *pData = new CAMERAMGRFX_VISIONCYCLE;
		pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		pData->nType = nType;

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_VISIONCYCLE;
		ceCS.dwFlags = dwFlags;
		ceCS.fnCustomUpdate = CameraMgrFX_VisionCycleUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just reset the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_VISIONCYCLE *pData = (CAMERAMGRFX_VISIONCYCLE*)pEffect->ceCS.pUserData;

		pEffect->fStartTime = g_pLTClient->GetTime();
		pEffect->ceCS.dwFlags = dwFlags;
		pData->nType = nType;
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_VisionCycleUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_VISIONCYCLE *pData = (CAMERAMGRFX_VISIONCYCLE*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fRamp = (fTime - pFX->fStartTime) / g_vtVisionTime.GetFloat();

	// If we've done the entire effect... just return false to destroy the effect
	if(fRamp >= 1.0f)
	{
		pData->nState = CAMERAMGRFX_STATE_DISABLED;
		switch(pData->nType)
		{
			case VISIONCYCLE_TYPE_MARINE:
			{
				break;
			}

			case VISIONCYCLE_TYPE_PREDATOR:
			{
				g_pInterfaceMgr->SetPredVisionTransition(LTFALSE);
				break;
			}

			case VISIONCYCLE_TYPE_ALIEN:
			{
				break;
			}
		}
		return LTFALSE;
	}

	// Do something different for each type
	switch(pData->nType)
	{
		case VISIONCYCLE_TYPE_MARINE:
		{
			break;
		}

		case VISIONCYCLE_TYPE_PREDATOR:
		{
			pFX->ceCS.vLightAdd = LTVector(1.0f, 1.0f, 1.0f) * (1.0f - fRamp);

			// Create a rect which will be our final rendering rect
			LTRect rRect;
			g_pCameraMgr->GetCameraRect(hCamera, rRect, LTTRUE);
			rRect = pCamera->cidOriginal.rRect;

			// Calculate a total offset required for the final transition
			int32 nWidth = rRect.right - rRect.left;

			// Create a final rect with offset interpolation values
			pFX->ceCS.rRect.Init(0, 0, (int)((LTFLOAT)(-nWidth) * (1.0f - fRamp)), 0);

			break;
		}

		case VISIONCYCLE_TYPE_ALIEN:
		{
			pFX->ceCS.vLightAdd = LTVector(1.0f, 1.0f, 1.0f) * (1.0f - fRamp);

			// Create a rect which will be our final rendering rect
			LTRect rRect;
			g_pCameraMgr->SetCameraRect(hCamera, rRect, LTTRUE);
			rRect = pCamera->cidOriginal.rRect;

			// Calculate a total offset required for the final transition
			int32 nHalfHeight = (rRect.bottom - rRect.top) / 2;

			LTFLOAT fScale = (LTFLOAT)sqrt(sinf(fRamp * MATH_HALFPI));
			int32 nOffset = (int)((float)nHalfHeight * (1.0f - fScale));

			// Create a final rect with offset interpolation values
			pFX->ceCS.rRect.Init(0, nOffset, 0, -nOffset);

			break;
		}
	}

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_ScreenWashCreate(HCAMERA hCamera)
{
	// Create the camera description to create
	CAMERAEFFECTCREATESTRUCT ceCS;
	ceCS.nID = CAMERAMGRFX_ID_SCREENWASH;
	ceCS.dwFlags = CAMERA_FX_USE_LIGHTADD;
	ceCS.fnCustomUpdate = CameraMgrFX_ScreenWashUpdate;
	ceCS.pUserData = LTNULL;

	g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
	return;
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_ScreenWashUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetFrameTime();

	LTVector vAdd = pFX->ceCS.vLightAdd;

	vAdd.x -= fTime/g_vtScreenWashRestoreTime.GetFloat();

	vAdd.y = vAdd.z = vAdd.x = vAdd.x>=0.0f?vAdd.x:0.0f;

	pFX->ceCS.vLightAdd = vAdd;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CameraMgrFX_SetScreenWash(HCAMERA hCamera, LTFLOAT fAmount)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_SCREENWASH, LTNULL);

	if(!pEffect) return;

	if(fAmount > g_vtMaxScreenWash.GetFloat()) fAmount = g_vtMaxScreenWash.GetFloat();

	if(fAmount > pEffect->ceCS.vLightAdd.x)
		pEffect->ceCS.vLightAdd = LTVector(fAmount, fAmount, fAmount);
}

// ----------------------------------------------------------------------- //

LTFLOAT CameraMgrFX_GetMaxScreenWash()
{
	return g_vtMaxScreenWash.GetFloat();
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_LandingBobCreate(HCAMERA hCamera, LTBOOL bSmall)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;

	//get xyz amount vector and times
	LTVector vAmount(0.000000f, g_vtLandBobAmount.GetFloat(), 0.000000f);

	if(bSmall)
		vAmount.y = g_vtLandBobAmountSmall.GetFloat();

	LTVector vTimes(g_vtLandBobRampUpTime.GetFloat(), 0.000000f, g_vtLandBobRampDownTime.GetFloat());

	//get rotation amount vector and times
	LTVector vTiltAmount(0.000000f, 0.000000f, 0.000000f);
	LTVector vTiltTimes(0.100000f, 0.000000f, 0.200000f);

	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_LANDBOB, LTNULL);

	// Make the amounts in radians instead of degrees
	vTiltAmount.x = MATH_DEGREES_TO_RADIANS(vTiltAmount.x);
	vTiltAmount.y = MATH_DEGREES_TO_RADIANS(vTiltAmount.y);
	vTiltAmount.z = MATH_DEGREES_TO_RADIANS(vTiltAmount.z);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_LAND_BOB *pData = new CAMERAMGRFX_LAND_BOB;
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;

		pData->vAmount = vAmount;
		pData->vTimes = vTimes;
		pData->vStartAmount			= LTVector(0.0f, 0.0f, 0.0f);
		pData->vCurrentAmount		= LTVector(0.0f, 0.0f, 0.0f);

		pData->vTiltAmount = vTiltAmount;
		pData->vTiltTimes = vTiltTimes;
		pData->vTiltStartAmount		= LTVector(0.0f, 0.0f, 0.0f);
		pData->vTiltCurrentAmount	= LTVector(0.0f, 0.0f, 0.0f);

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_LANDBOB;
		ceCS.dwFlags = CAMERA_FX_USE_POS | CAMERA_FX_USE_ROT;
		ceCS.fnCustomUpdate = CameraMgrFX_LandingBobUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just reset the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_LAND_BOB *pData = (CAMERAMGRFX_LAND_BOB*)pEffect->ceCS.pUserData;

		pEffect->fStartTime = g_pLTClient->GetTime();
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;

		pData->vTiltAmount		= vTiltAmount;
		pData->vTiltTimes		= vTiltTimes;
		pData->vTiltStartAmount = LTVector(0.0f, 0.0f, 0.0f);

		pData->vAmount		= vAmount;
		pData->vTimes		= vTimes;
		pData->vStartAmount = LTVector(0.0f, 0.0f, 0.0f);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_LandingBobUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_LAND_BOB *pData = (CAMERAMGRFX_LAND_BOB*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fTiltRamp = 1.0f;
	LTFLOAT fRamp = 1.0f;

	LTBOOL bStillTilting = LTTRUE;

	//****************************************************************//

	// Do the ramp up of the tilt
	if(fTime < (pFX->fStartTime + pData->vTiltTimes.x))
	{
		fTiltRamp = (fTime - pFX->fStartTime) / pData->vTiltTimes.x;

		// If we've ramped completely out... set our new state
		if(fTiltRamp >= 1.0f)
		{
			fTiltRamp = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
	}


	// Do the ramp up of the flash
	if(fTime < (pFX->fStartTime + pData->vTiltTimes.x))		// CAMERAMGRFX_STATE_RAMPUP
	{
		fTiltRamp = (fTime - pFX->fStartTime) / pData->vTiltTimes.x;

		// If we've ramped completely out... set our new state
		if(fTiltRamp >= 1.0f)
		{
			fTiltRamp = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
	}
	else if(fTime > (pFX->fStartTime + pData->vTiltTimes.x + pData->vTiltTimes.y))	// CAMERAMGRFX_STATE_RAMPDOWN
	{
		LTFLOAT fTempStart = pFX->fStartTime + pData->vTiltTimes.x + pData->vTiltTimes.y;
		fTiltRamp = 1.0f - ((fTime - fTempStart) / pData->vTiltTimes.z);

		// If we've ramped completely in... set our new state
		if(fTiltRamp <= 0.0f)
		{
			pData->nState = CAMERAMGRFX_STATE_DISABLED;
			bStillTilting = LTFALSE;
		}
	}


	//****************************************************************//
	
	// Do the ramp up of the tilt
	if(fTime < (pFX->fStartTime + pData->vTimes.x))
	{
		fRamp = (fTime - pFX->fStartTime) / pData->vTimes.x;

		// If we've ramped completely out... set our new state
		if(fRamp >= 1.0f)
		{
			fRamp = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
	}


	// Do the ramp up of the flash
	if(fTime < (pFX->fStartTime + pData->vTimes.x))		// CAMERAMGRFX_STATE_RAMPUP
	{
		fRamp = (fTime - pFX->fStartTime) / pData->vTimes.x;

		// If we've ramped completely out... set our new state
		if(fRamp >= 1.0f)
		{
			fRamp = 1.0f;
			pData->nState = CAMERAMGRFX_STATE_ACTIVE;
		}
	}
	else if(fTime > (pFX->fStartTime + pData->vTimes.x + pData->vTimes.y))	// CAMERAMGRFX_STATE_RAMPDOWN
	{
		LTFLOAT fTempStart = pFX->fStartTime + pData->vTimes.x + pData->vTimes.y;
		fRamp = 1.0f - ((fTime - fTempStart) / pData->vTimes.z);

		// If we've ramped completely in... set our new state
		if(fRamp <= 0.0f)
		{
			pData->nState = CAMERAMGRFX_STATE_DISABLED;

			if(!bStillTilting)
				return LTFALSE;
		}
	}

	//****************************************************************//

	LTVector vU, vR, vF;
	g_pLTClient->GetRotationVectors(&pCamera->cidOriginal.rRot, &vU, &vR, &vF);

	// Create the final value
	pData->vTiltCurrentAmount = pData->vTiltStartAmount + ((pData->vTiltAmount - pData->vTiltStartAmount) * fTiltRamp);
	g_pLTClient->GetMathLT()->SetupEuler(pFX->ceCS.rRot, pData->vTiltCurrentAmount);

	pData->vCurrentAmount = pData->vStartAmount + ((pData->vAmount - pData->vStartAmount) * fRamp);
	pFX->ceCS.vPos = vR*pData->vCurrentAmount.x + vU*pData->vCurrentAmount.y + vF*pData->vCurrentAmount.z;

	return LTTRUE;
}

// *********************************************************************** //
// *********************************************************************** //
// *********************************************************************** //

void CameraMgrFX_NetTiltCreate(HCAMERA hCamera, LTVector vRotAmount, LTVector vPosAmount)
{
	// See if this camera is active...
	if(!g_pCameraMgr->GetCameraActive(hCamera))
		return;

	LTVector vTimes(0.25f, 4.0f, 0.25f);

	// Find any tilt special fx
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_NET_TILT, LTNULL);

	// Make the amounts in radians instead of degrees
	vRotAmount.x = MATH_DEGREES_TO_RADIANS(vRotAmount.x);
	vRotAmount.y = MATH_DEGREES_TO_RADIANS(vRotAmount.y);
	vRotAmount.z = MATH_DEGREES_TO_RADIANS(vRotAmount.z);

	// If an effect doesn't exist, then create one and return
	if(!pEffect)
	{
		// Create a user data structure
		CAMERAMGRFX_NET_TILT *pData = new CAMERAMGRFX_NET_TILT;
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->bRampUp = LTTRUE;

		pData->vAmount = vPosAmount;
		pData->vTimes = vTimes;
		pData->vStartAmount			= LTVector(0.0f, 0.0f, 0.0f);
		pData->vCurrentAmount		= LTVector(0.0f, 0.0f, 0.0f);

		pData->vTiltAmount = vRotAmount;
		pData->vTiltTimes = vTimes;
		pData->vTiltStartAmount		= LTVector(0.0f, 0.0f, 0.0f);
		pData->vTiltCurrentAmount	= LTVector(0.0f, 0.0f, 0.0f);

		// Create the camera description to create
		CAMERAEFFECTCREATESTRUCT ceCS;
		ceCS.nID = CAMERAMGRFX_ID_NET_TILT;
		ceCS.dwFlags = CAMERA_FX_USE_POS | CAMERA_FX_USE_ROT;
		ceCS.fnCustomUpdate = CameraMgrFX_NetTiltUpdate;
		ceCS.pUserData = pData;

		g_pCameraMgr->NewCameraEffect(hCamera, ceCS);
		return;
	}

	// Otherwise, just reset the current effect
	if(pEffect->ceCS.pUserData)
	{
		CAMERAMGRFX_NET_TILT *pData = (CAMERAMGRFX_NET_TILT*)pEffect->ceCS.pUserData;

		pEffect->fStartTime = g_pLTClient->GetTime();
		pData->nState = CAMERAMGRFX_STATE_RAMPUP;
		pData->bRampUp = LTTRUE;

		pData->vTiltAmount		= vRotAmount;
		pData->vTiltTimes		= vTimes;
		pData->vTiltStartAmount = LTVector(0.0f, 0.0f, 0.0f);

		pData->vAmount		= vPosAmount;
		pData->vTimes		= vTimes;
		pData->vStartAmount = LTVector(0.0f, 0.0f, 0.0f);
	}
}

// ----------------------------------------------------------------------- //

LTBOOL CameraMgrFX_NetTiltUpdate(HCAMERA hCamera, void *pEffect)
{
	// Make sure everything is valid
	if(!pEffect) return LTTRUE;

	// Cast the pointers to the data so we can use them
	CAMERAINSTANCE *pCamera = g_pCameraMgr->GetCamera(hCamera);
	CAMERAEFFECTINSTANCE *pFX = (CAMERAEFFECTINSTANCE*)pEffect;
	CAMERAMGRFX_NET_TILT *pData = (CAMERAMGRFX_NET_TILT*)pFX->ceCS.pUserData;

	// Get the current time
	LTFLOAT fTime = g_pLTClient->GetTime();
	LTFLOAT fTiltRamp = 1.0f;
	LTFLOAT fRamp = 1.0f;

	LTBOOL bStillTilting = LTTRUE;

	//****************************************************************//

	// Do the ramp up of the tilt
	if(pData->bRampUp)
	{
		if(fTime < (pFX->fStartTime + pData->vTiltTimes.x))
		{
			fTiltRamp = (fTime - pFX->fStartTime) / pData->vTiltTimes.x;

			// If we've ramped completely out... set our new state
			if(fTiltRamp >= 1.0f)
			{
				fTiltRamp = 1.0f;
				pData->nState = CAMERAMGRFX_STATE_ACTIVE;
			}
		}


		// Do the ramp up of the flash
		if(fTime < (pFX->fStartTime + pData->vTiltTimes.x))		// CAMERAMGRFX_STATE_RAMPUP
		{
			fTiltRamp = (fTime - pFX->fStartTime) / pData->vTiltTimes.x;

			// If we've ramped completely out... set our new state
			if(fTiltRamp >= 1.0f)
			{
				fTiltRamp = 1.0f;
				pData->nState = CAMERAMGRFX_STATE_ACTIVE;
			}
		}
	}
	else	// CAMERAMGRFX_STATE_RAMPDOWN
	{
		fTiltRamp = 1.0f - ((fTime - pFX->fStartTime) / pData->vTiltTimes.z);

		// If we've ramped completely in... set our new state
		if(fTiltRamp <= 0.0f)
		{
			pData->nState = CAMERAMGRFX_STATE_DISABLED;
			bStillTilting = LTFALSE;
		}
	}


	//****************************************************************//
	
	// Do the ramp up of the tilt
	if(pData->bRampUp)
	{
		if(fTime < (pFX->fStartTime + pData->vTimes.x))
		{
			fRamp = (fTime - pFX->fStartTime) / pData->vTimes.x;

			// If we've ramped completely out... set our new state
			if(fRamp >= 1.0f)
			{
				fRamp = 1.0f;
				pData->nState = CAMERAMGRFX_STATE_ACTIVE;
			}
		}


		// Do the ramp up of the flash
		if(fTime < (pFX->fStartTime + pData->vTimes.x))		// CAMERAMGRFX_STATE_RAMPUP
		{
			fRamp = (fTime - pFX->fStartTime) / pData->vTimes.x;

			// If we've ramped completely out... set our new state
			if(fRamp >= 1.0f)
			{
				fRamp = 1.0f;
				pData->nState = CAMERAMGRFX_STATE_ACTIVE;
			}
		}
	}
	else // CAMERAMGRFX_STATE_RAMPDOWN
	{
		fRamp = 1.0f - ((fTime - pFX->fStartTime) / pData->vTimes.z);

		// If we've ramped completely in... set our new state
		if(fRamp <= 0.0f)
		{
			pData->nState = CAMERAMGRFX_STATE_DISABLED;

			if(!bStillTilting)
				return LTFALSE;
		}
	}

	//****************************************************************//

	LTVector vRotOffset = pData->vTiltAmount * fRamp;
	g_pLTClient->GetMathLT()->SetupEuler(pFX->ceCS.rRot, vRotOffset);

	LTVector vU, vR, vF;
	g_pLTClient->GetRotationVectors(&pCamera->cidOriginal.rRot, &vU, &vR, &vF);

	// Create the final value
//	pData->vTiltCurrentAmount = pData->vTiltStartAmount + ((pData->vTiltAmount - pData->vTiltStartAmount) * fTiltRamp);
//	g_pLTClient->GetMathLT()->SetupEuler(pFX->ceCS.rRot, pData->vTiltCurrentAmount);

	pData->vCurrentAmount = pData->vStartAmount + ((pData->vAmount - pData->vStartAmount) * fRamp);
	pFX->ceCS.vPos = vR*pData->vCurrentAmount.x + vU*pData->vCurrentAmount.y + vF*pData->vCurrentAmount.z;

	return LTTRUE;
}

void CameraMgrFX_NetTiltSetRampDown(HCAMERA hCamera)
{
	CAMERAEFFECTINSTANCE *pEffect = g_pCameraMgr->GetCameraEffectByID(hCamera, CAMERAMGRFX_ID_NET_TILT, LTNULL);

	if(!pEffect) return;

	CAMERAMGRFX_NET_TILT *pData = (CAMERAMGRFX_NET_TILT*)pEffect->ceCS.pUserData;

	pEffect->fStartTime = g_pLTClient->GetTime();
	pData->bRampUp = LTFALSE;
}


