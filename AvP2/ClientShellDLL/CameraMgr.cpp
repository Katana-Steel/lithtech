// ----------------------------------------------------------------------- //
//
// MODULE  : CameraMgr.cpp
//
// PURPOSE : General client side camera handling class definitions
//
// CREATED : 3/1/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "cameramgr.h"
#include "vartrack.h"

//*************************************************************************//

CameraMgr			*g_pCameraMgr = LTNULL;

static VarTrack		g_vtShowCameraStats;

static VarTrack		g_vtOrigPosInterpTime;
static VarTrack		g_vtOrigPosInterpOffset;

static VarTrack		g_vtOrigRotInterpScale;
static VarTrack		g_vtOrigRotInterpSmoothScale;

static VarTrack		g_vtCameraClipDistance;

static void CalcNonClipPos(LTVector & vPos, LTRotation & rRot)
{
	LTVector vTemp, vU, vR, vF;
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Make sure we aren't clipping into walls...

    LTFLOAT fClipDistance = g_vtCameraClipDistance.GetFloat();


	// Check for walls to the right...

	vTemp = (vR * fClipDistance);
	vTemp += vPos;

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	iQuery.m_From = vPos;
	iQuery.m_To   = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        LTFLOAT fDist = (fClipDistance - vTemp.Mag());

		vTemp = (vR * -fDist);
		vPos += vTemp;
	}
	else
	{
	 	// If we didn't adjust for a wall to the right, check walls to the left...

		vTemp = (vR * -fClipDistance);
		vTemp += vPos;

		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            LTFLOAT fDist = (fClipDistance - vTemp.Mag());

			vTemp = (vR * fDist);
			vPos += vTemp;
		}
	}


	// Check for ceilings...

	vTemp = vU * fClipDistance;
	vTemp += vPos;

	iQuery.m_From = vPos;
	iQuery.m_To = vTemp;

	if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		vTemp = iInfo.m_Point - vPos;
        LTFLOAT fDist = (fClipDistance - vTemp.Mag());

		vTemp = vU * -fDist;
		vPos += vTemp;
	}
	else
	{
 		// If we didn't hit any ceilings, check for floors...

		vTemp = vU * -fClipDistance;
		vTemp += vPos;

		iQuery.m_From = vPos;
		iQuery.m_To = vTemp;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{
			vTemp = iInfo.m_Point - vPos;
            LTFLOAT fDist = (fClipDistance - vTemp.Mag());

			vTemp = vU * fDist;
			vPos += vTemp;
		}
	}
}

//*************************************************************************//

CameraMgr::CameraMgr()
{
	// Set the global pointer
	g_pCameraMgr = this;

	// Clear out the area of camera pointers
	memset(m_pCameras, 0, sizeof(CAMERAINSTANCE*) * CAMERAMGR_MAX_CAMERAS);

	// Make sure the default is to allow only one active camera
	m_bAllowMultipleActiveCameras = LTFALSE;
}

//*************************************************************************//

CameraMgr::~CameraMgr()
{
	Term();
}

//*************************************************************************//

LTBOOL CameraMgr::Init()
{
	// Init the tracked variables
	if(!g_vtShowCameraStats.IsInitted())
		g_vtShowCameraStats.Init(g_pLTClient, "ShowCameraStats", LTNULL, 0.0f);

	if(!g_vtOrigPosInterpTime.IsInitted())
		g_vtOrigPosInterpTime.Init(g_pLTClient, "CameraMgrPosInterpTime", LTNULL, 1.0f);
	if(!g_vtOrigPosInterpOffset.IsInitted())
		g_vtOrigPosInterpOffset.Init(g_pLTClient, "CameraMgrPosInterpOffset", LTNULL, 0.01f);

	if(!g_vtOrigRotInterpScale.IsInitted())
		g_vtOrigRotInterpScale.Init(g_pLTClient, "CameraMgrRotInterpScale", LTNULL, 5.0f);
	if(!g_vtOrigRotInterpSmoothScale.IsInitted())
		g_vtOrigRotInterpSmoothScale.Init(g_pLTClient, "CameraMgrRotInterpSmoothScale", LTNULL, 7.5f);

	if(!g_vtCameraClipDistance.IsInitted())
		g_vtCameraClipDistance.Init(g_pLTClient, "CameraMgrClipDist", NULL, 5.0f);

	return LTTRUE;
}

//*************************************************************************//

void CameraMgr::Term()
{
	// Go through the list of cameras and delete all of them
	for(HCAMERA hIndex = 0; hIndex < CAMERAMGR_MAX_CAMERAS; hIndex++)
		DeleteCamera(hIndex);
}

//*************************************************************************//

void CameraMgr::AllowMultipleActiveCameras(LTBOOL bAllow)
{
	m_bAllowMultipleActiveCameras = bAllow;
}

//*************************************************************************//

void CameraMgr::SetCameraActive(HCAMERA hCamera, LTBOOL bActive)
{
	if(m_pCameras[hCamera])
	{
		// If we're only allowing one active camera, go through and turn them all off
		if(!m_bAllowMultipleActiveCameras)
		{
			for(HCAMERA hIndex = 0; hIndex < CAMERAMGR_MAX_CAMERAS; hIndex++)
			{
				if(m_pCameras[hIndex])
					m_pCameras[hIndex]->bActive = LTFALSE;
			}
		}

		// Now set the one we want to active
		m_pCameras[hCamera]->bActive = bActive;
	}
}

//*************************************************************************//

LTBOOL CameraMgr::GetCameraActive(HCAMERA hCamera)
{
	return m_pCameras[hCamera] ? m_pCameras[hCamera]->bActive : LTFALSE;
}

//*************************************************************************//

uint8 CameraMgr::GetActiveCameras(HCAMERA *hCameras, uint8 nMax)
{
	uint8 nNumActive = 0;

	// Go through the list of cameras and find any active ones
	for(HCAMERA hIndex = 0; hIndex < CAMERAMGR_MAX_CAMERAS; hIndex++)
	{
		if(m_pCameras[hIndex])
		{
			if(m_pCameras[hIndex]->bActive && m_pCameras[hIndex]->hCamera)
			{
				hCameras[nNumActive] = hIndex;
				nNumActive++;
			}
		}
	}

	return nNumActive;
}

//*************************************************************************//

void CameraMgr::RenderActiveCameras()
{
	// Go through the list of cameras and render any active ones
	for(HCAMERA hIndex = 0; hIndex < CAMERAMGR_MAX_CAMERAS; hIndex++)
	{
		if(m_pCameras[hIndex])
		{
			LTBOOL bRender = LTTRUE;

			// Call the pre render function if it exists
			if(m_pCameras[hIndex]->fpPreRender)
				bRender = (*m_pCameras[hIndex]->fpPreRender)(this, hIndex);

			// Render the camera if it's active and the object is valid
			if(bRender && m_pCameras[hIndex]->bActive && m_pCameras[hIndex]->hCamera)
			{
				// Set the light scale
				g_pLTClient->SetGlobalLightScale(&(m_pCameras[hIndex]->cidFinal.vLightScale));
				g_pLTClient->RenderCamera(m_pCameras[hIndex]->hCamera);
			}

			// Call the post render function if it exists
			if(m_pCameras[hIndex]->fpPostRender)
				(*m_pCameras[hIndex]->fpPostRender)(this, hIndex);
		}
	}
}

//*************************************************************************//

HCAMERA CameraMgr::NewCamera(CAMERACREATESTRUCT &camCS)
{
	// Get an empty spot to add a camera
	HCAMERA hSlot = FindOpenCameraSlot();

	// Try to allocate space for a new camera
	m_pCameras[hSlot] = new CAMERAINSTANCE;
	if(!m_pCameras[hSlot])
		return CAMERAMGR_INVALID;

	// Now try to create a new camera object
	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);
	ocs.m_ObjectType = OT_CAMERA;
	m_pCameras[hSlot]->hCamera = g_pLTClient->CreateObject(&ocs);

	// If the camera creation failed... delete the camera instance and return an invalid handle
	if(!m_pCameras[hSlot]->hCamera)
	{
		delete m_pCameras[hSlot];
		m_pCameras[hSlot] = LTNULL;
		return CAMERAMGR_INVALID;
	}

	// Otherwise, set all the camera properties
	SetCameraFlags(hSlot, camCS.dwFlags);
	SetCameraPos(hSlot, camCS.vPos);
	SetCameraRotation(hSlot, camCS.rRot);
	SetCameraObj(hSlot, camCS.hObj, camCS.szObjSocket);
	SetCameraTarget(hSlot, camCS.hTarget, camCS.szTargetSocket);
	SetCameraTargetPos(hSlot, camCS.vTargetPos);
	SetCameraFOV(hSlot, camCS.fFOVX, camCS.fFOVY);
	SetCameraRect(hSlot, camCS.rRect, camCS.bFullScreen);
	SetCameraLightAdd(hSlot, camCS.vLightAdd);
	SetCameraLightScale(hSlot, camCS.vLightScale);

	SetCameraPreRenderFunc(hSlot, camCS.fpPreRender);
	SetCameraPostRenderFunc(hSlot, camCS.fpPostRender);


	return hSlot;
}

//*************************************************************************//

void CameraMgr::DeleteCamera(HCAMERA hCamera)
{
	if(m_pCameras[hCamera])
	{
		// Delete the engine camera object it if exists
		if(m_pCameras[hCamera]->hCamera)
		{
			g_pLTClient->DeleteObject(m_pCameras[hCamera]->hCamera);
			m_pCameras[hCamera]->hCamera = LTNULL;
		}

		// Delete any effects that may exist
		ClearCameraEffects(hCamera);

		// Delete our camera instance structure
		delete m_pCameras[hCamera];
		m_pCameras[hCamera] = LTNULL;
	}
}

//*************************************************************************//

CAMERAEFFECTINSTANCE* CameraMgr::NewCameraEffect(HCAMERA hCamera, CAMERAEFFECTCREATESTRUCT &ceCS)
{
	// Find the last effect in the current list...
	CAMERAEFFECTINSTANCE *pEffect = m_pCameras[hCamera]->pEffectList;
	while(pEffect && pEffect->pNext)
		pEffect = pEffect->pNext;

	// Create a new effect to add in to the list
	CAMERAEFFECTINSTANCE *pNewEffect = new CAMERAEFFECTINSTANCE;

	if(pNewEffect)
	{
		pNewEffect->fStartTime = g_pLTClient->GetTime();
		pNewEffect->ceCS = ceCS;
		pNewEffect->pPrev = LTNULL;
		pNewEffect->pNext = LTNULL;
	}

	// If there weren't any effects in the list so far... add it as the first one
	if(!pEffect)
	{
		m_pCameras[hCamera]->pEffectList = pNewEffect;
	}
	else
	{
		// Otherwise, tack it onto the end of the list
		pNewEffect->pPrev = pEffect;
		pEffect->pNext = pNewEffect;
	}

	return pNewEffect;
}

//*************************************************************************//

void CameraMgr::DeleteCameraEffect(HCAMERA hCamera, CAMERAEFFECTINSTANCE *pEffect)
{
	// Make sure this is a valid element
	if(!pEffect) return;

	// If this effect is the top of the list, then move our list pointer to the next element
	if(pEffect == m_pCameras[hCamera]->pEffectList)
		m_pCameras[hCamera]->pEffectList = pEffect->pNext;

	// Remove the effect from the list
	if(pEffect->pPrev) pEffect->pPrev->pNext = pEffect->pNext;
	if(pEffect->pNext) pEffect->pNext->pPrev = pEffect->pPrev;
	pEffect->pPrev = pEffect->pNext = LTNULL;

	// Delete the element
	delete pEffect;
	pEffect = LTNULL;
}

//*************************************************************************//

void CameraMgr::ClearCameraEffects(HCAMERA hCamera)
{
	// Get the first element in the list
	CAMERAEFFECTINSTANCE *pEffect = m_pCameras[hCamera]->pEffectList;

	// Go through all of them and call the delete function
	while(pEffect)
	{
		DeleteCameraEffect(hCamera, pEffect);
		pEffect = m_pCameras[hCamera]->pEffectList;
	}
}

//*************************************************************************//
//
// FUNCTION:	CameraMgr::GetCameraEffectByID()
//
// PURPOSE:		Search for an effect element with the specified ID number.
//				Pass in LTNULL for pStart to begin at the start of the list, and
//				then pass in the pStart again to continue through to the end.
//
//*************************************************************************//

CAMERAEFFECTINSTANCE* CameraMgr::GetCameraEffectByID(HCAMERA hCamera, uint8 nID, CAMERAEFFECTINSTANCE *pStart)
{
	// Get a pointer to the starting element
	CAMERAEFFECTINSTANCE *pEffect = pStart ? pStart : m_pCameras[hCamera]?m_pCameras[hCamera]->pEffectList:LTNULL;

	// Go through the remaining elements and see if we find one with a matching ID
	while(pEffect)
	{
		if(pEffect->ceCS.nID == nID)
		{
			pStart = pEffect->pNext;
			break;
		}

		pEffect = pEffect->pNext;
	}

	return pEffect;
}

//*************************************************************************//
//
// FUNCTION:	CameraMgr::Update()
//
// PURPOSE:		Go through every camera in the list and update their positions,
//				rotations, and special FX
//
//*************************************************************************//

void CameraMgr::Update()
{
	// Go through the list of cameras and update all of them
	for(HCAMERA hIndex = 0; hIndex < CAMERAMGR_MAX_CAMERAS; hIndex++)
	{
		CAMERAINSTANCE *pCamera = m_pCameras[hIndex];

		// If the camera engine object exists... update it
		if(pCamera && pCamera->hCamera)
		{
			// Update the camera position based off the obj
			if(pCamera->dwFlags & CAMERA_FLAG_FOLLOW_OBJ_POS)
				UpdateToObjPosition(hIndex);

			if(pCamera->dwFlags & CAMERA_FLAG_FOLLOW_OBJ_ROT)
			{
				// Update the camera rotation based off the obj
				UpdateToObjRotation(hIndex);
			}

			if((pCamera->dwFlags & CAMERA_FLAG_FOLLOW_TARGET) || (pCamera->dwFlags & CAMERA_FLAG_FOLLOW_TARGET_POS))
			{
				// Update the camera rotation based off the target
				UpdateToTargetRotation(hIndex);
			}

			// Copy the original interpolation values into the final set
			memcpy(&pCamera->cidFinal, &pCamera->cidOriginal, sizeof(CAMERAINTERPDATA));

			// Update any special effects that might be active on this camera
			UpdateCameraEffects(hIndex);

			// Set all the values of the camera according to the final interpolation values
			g_pLTClient->SetObjectPos(pCamera->hCamera, &(pCamera->cidFinal.vPos));
			g_pLTClient->SetObjectRotation(pCamera->hCamera, &(pCamera->cidFinal.rRot));
			g_pLTClient->SetCameraFOV(pCamera->hCamera, pCamera->cidFinal.fFOVX, pCamera->cidFinal.fFOVY);
			g_pLTClient->SetCameraLightAdd(pCamera->hCamera, &(pCamera->cidFinal.vLightAdd));
			g_pLTClient->SetCameraRect(pCamera->hCamera,	pCamera->cidFinal.bFullScreen,
															pCamera->cidFinal.rRect.left,
															pCamera->cidFinal.rRect.top,
															pCamera->cidFinal.rRect.right,
															pCamera->cidFinal.rRect.bottom);

			//--------------------------------------------------------------------------------
			// Display camera debug information

			if(g_vtShowCameraStats.GetFloat())
			{
				LTVector vPos, vLightAdd, vLightScale, vR, vU, vF, vEuler;
				DRect rRect = pCamera->cidFinal.rRect;
				vPos = pCamera->cidFinal.vPos;
				vLightAdd = pCamera->cidFinal.vLightAdd;
				vLightScale = pCamera->cidFinal.vLightScale;

				g_pLTClient->GetMathLT()->GetRotationVectors(pCamera->cidFinal.rRot, vR, vU, vF);
				g_pLTClient->GetMathLT()->GetEulerAngles(pCamera->cidFinal.rRot, vEuler);

				if(vEuler.x < 0.0f) vEuler.x += MATH_CIRCLE;
				if(vEuler.y < 0.0f) vEuler.y += MATH_CIRCLE;
				if(vEuler.z < 0.0f) vEuler.z += MATH_CIRCLE;

				g_pLTClient->CPrint("Camera #%.2d:  Pos = %.2f %.2f %.2f, Dir = %.2f %.2f %.2f", hIndex, vPos.x, vPos.y, vPos.z, vF.x, vF.y, vF.z);
				g_pLTClient->CPrint("             Pitch = %.2f, Yaw %.2f, Roll %.2f", MATH_RADIANS_TO_DEGREES(vEuler.x), MATH_RADIANS_TO_DEGREES(vEuler.y), MATH_RADIANS_TO_DEGREES(vEuler.z));
				g_pLTClient->CPrint("             FOV = %.2f x %.2f, LightAdd = %.2f %.2f %.2f", pCamera->cidFinal.fFOVX, pCamera->cidFinal.fFOVY, vLightAdd.x, vLightAdd.y, vLightAdd.z);
				g_pLTClient->CPrint("             LightScale = %.2f %.2f %.2f", vLightScale.x, vLightScale.y, vLightScale.z);
				g_pLTClient->CPrint("             Rect = %d %d, %d %d, Fullscreen = %s", rRect.left, rRect.top, rRect.right, rRect.bottom, pCamera->cidFinal.bFullScreen ? "TRUE" : "FALSE");
			}
			//--------------------------------------------------------------------------------
		}
	}
}

//*************************************************************************//
//
// FUNCTION:	CameraMgr::UpdateCameraEffects()
//
// PURPOSE:		Go through the list of special FX for one camera and update
//				final settings based on them
//
//*************************************************************************//

void CameraMgr::UpdateCameraEffects(HCAMERA hCamera)
{
	// If this camera doesn't have a special effect list, just return
	if(!m_pCameras[hCamera]->pEffectList)
		return;

	// Otherwise loop through the special effect list and handle each of them one by one
	MathLT *pMath = g_pLTClient->GetMathLT();
	CAMERAINSTANCE *pCamera = m_pCameras[hCamera];
	CAMERAEFFECTINSTANCE *pEffect = pCamera->pEffectList;
	LTFLOAT fTime = g_pLTClient->GetTime();

	while(pEffect)
	{
		// Check to see if this effect has expired
		if(pEffect->ceCS.fTime && (fTime > pEffect->fStartTime + pEffect->ceCS.fTime))
		{
			CAMERAEFFECTINSTANCE *pExpired = pEffect;
			pEffect = pEffect->pNext;
			DeleteCameraEffect(hCamera, pExpired);
			continue;
		}

		// Now, see if there's a custom update function
		if(pEffect->ceCS.fnCustomUpdate)
		{
			if(!(*pEffect->ceCS.fnCustomUpdate)(hCamera, pEffect))
			{
				CAMERAEFFECTINSTANCE *pExpired = pEffect;
				pEffect = pEffect->pNext;
				DeleteCameraEffect(hCamera, pExpired);
				continue;
			}
		}


		// TODO: Otherwise, update the final interpolation data...

		// Handle the position offset of the camera
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_POS)
		{
			pCamera->cidFinal.vPos += pEffect->ceCS.vPos;
		}

		// Handle the rotation offset of the camera (using euler angles)
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_ROT_V)
		{
			LTVector vRadianAngles;
			LTRotation rOffsetRot;

			vRadianAngles.x = MATH_DEGREES_TO_RADIANS(pEffect->ceCS.vRot.x);
			vRadianAngles.y = MATH_DEGREES_TO_RADIANS(pEffect->ceCS.vRot.y);
			vRadianAngles.z = MATH_DEGREES_TO_RADIANS(pEffect->ceCS.vRot.z);

			pMath->SetupEuler(rOffsetRot, vRadianAngles);
			pCamera->cidFinal.rRot = pCamera->cidFinal.rRot * rOffsetRot;
		}

		// Handle the rotation offset of the camera
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_ROT)
		{
			pCamera->cidFinal.rRot = pCamera->cidFinal.rRot * pEffect->ceCS.rRot;
		}

		// Handle the X FOV interpolation
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_FOVX)
		{
			pCamera->cidFinal.fFOVX += pEffect->ceCS.fFOVX;
		}

		// Handle the Y FOV interpolation
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_FOVY)
		{
			pCamera->cidFinal.fFOVY += pEffect->ceCS.fFOVY;
		}

		// Handle the screen rect interpolation
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_RECT)
		{
			pCamera->cidFinal.bFullScreen = LTFALSE;
			pCamera->cidFinal.rRect.left += pEffect->ceCS.rRect.left;
			pCamera->cidFinal.rRect.top += pEffect->ceCS.rRect.top;
			pCamera->cidFinal.rRect.right += pEffect->ceCS.rRect.right;
			pCamera->cidFinal.rRect.bottom += pEffect->ceCS.rRect.bottom;
		}

		// Handle the light add of the camera
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_LIGHTADD)
		{
			if(pCamera->cidFinal.vLightAdd.x < pEffect->ceCS.vLightAdd.x)
				pCamera->cidFinal.vLightAdd.x = pEffect->ceCS.vLightAdd.x;
			if(pCamera->cidFinal.vLightAdd.y < pEffect->ceCS.vLightAdd.y)
				pCamera->cidFinal.vLightAdd.y = pEffect->ceCS.vLightAdd.y;
			if(pCamera->cidFinal.vLightAdd.z < pEffect->ceCS.vLightAdd.z)
				pCamera->cidFinal.vLightAdd.z = pEffect->ceCS.vLightAdd.z;
		}

		// Handle the light add of the camera
		if(pEffect->ceCS.dwFlags & CAMERA_FX_USE_LIGHTSCALE)
		{
			if(pCamera->cidFinal.vLightScale.x > pEffect->ceCS.vLightScale.x)
				pCamera->cidFinal.vLightScale.x = pEffect->ceCS.vLightScale.x;
			if(pCamera->cidFinal.vLightScale.y > pEffect->ceCS.vLightScale.y)
				pCamera->cidFinal.vLightScale.y = pEffect->ceCS.vLightScale.y;
			if(pCamera->cidFinal.vLightScale.z > pEffect->ceCS.vLightScale.z)
				pCamera->cidFinal.vLightScale.z = pEffect->ceCS.vLightScale.z;
		}

		// Calculate a non-clipping final position if requested.  
		// NOTE: we need to do this here after the final pos/rot have
		// be calculated...
		if (pEffect->ceCS.dwFlags & CAMERA_FX_USE_NOCLIP)
		{
			CalcNonClipPos(pCamera->cidFinal.vPos, pCamera->cidFinal.rRot);
		}

		// Go to the next effect
		pEffect = pEffect->pNext;
	}
}

//*************************************************************************//

void CameraMgr::SetCameraFlags(HCAMERA hCamera, uint32 dwFlags)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return;

	m_pCameras[hCamera]->dwFlags = dwFlags;
}

//*************************************************************************//

void CameraMgr::SetCameraPos(HCAMERA hCamera, LTVector &vPos)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	m_pCameras[hCamera]->cidOriginal.vPos = vPos;
	g_pLTClient->SetObjectPos(m_pCameras[hCamera]->hCamera, &vPos);
}

//*************************************************************************//

void CameraMgr::SetCameraRotation(HCAMERA hCamera, LTRotation &rRot)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	m_pCameras[hCamera]->cidOriginal.rRot = rRot;
	g_pLTClient->SetObjectRotation(m_pCameras[hCamera]->hCamera, &rRot);
}

//*************************************************************************//

void CameraMgr::SetCameraObj(HCAMERA hCamera, HOBJECT hObj, char *szSocket)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return;

	m_pCameras[hCamera]->cidOriginal.hObj = hObj;

	if(szSocket)
		strncpy(m_pCameras[hCamera]->cidOriginal.szObjSocket, szSocket, CAMERAMGR_MAX_SOCKET_NAME);
}

//*************************************************************************//

void CameraMgr::SetCameraTarget(HCAMERA hCamera, HOBJECT hTarget, char *szSocket)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return;

	m_pCameras[hCamera]->cidOriginal.hTarget = hTarget;

	if(szSocket)
		strncpy(m_pCameras[hCamera]->cidOriginal.szTargetSocket, szSocket, CAMERAMGR_MAX_SOCKET_NAME);
}

//*************************************************************************//

void CameraMgr::SetCameraTargetPos(HCAMERA hCamera, LTVector &vPos)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return;

	m_pCameras[hCamera]->cidOriginal.vTargetPos = vPos;
}

//*************************************************************************//

void CameraMgr::SetCameraFOV(HCAMERA hCamera, LTFLOAT fFOVX, LTFLOAT fFOVY, LTBOOL bDegrees)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	if(bDegrees)
	{
		fFOVX = MATH_DEGREES_TO_RADIANS(fFOVX);
		fFOVY = MATH_DEGREES_TO_RADIANS(fFOVY);
	}

	m_pCameras[hCamera]->cidOriginal.fFOVX = fFOVX;
	m_pCameras[hCamera]->cidOriginal.fFOVY = fFOVY;
	g_pLTClient->SetCameraFOV(m_pCameras[hCamera]->hCamera, fFOVX, fFOVY);
}

//*************************************************************************//

void CameraMgr::SetCameraRect(HCAMERA hCamera, LTRect &rect, LTBOOL bFullScreen)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	m_pCameras[hCamera]->cidOriginal.bFullScreen = bFullScreen;

	if(bFullScreen)
	{
		HSURFACE hScreen = g_pLTClient->GetScreenSurface();
		uint32 nWidth, nHeight;

		g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);
		m_pCameras[hCamera]->cidOriginal.rRect.Init(0, 0, nWidth, nHeight);
	}
	else
	{
		m_pCameras[hCamera]->cidOriginal.rRect = rect;
	}

	g_pLTClient->SetCameraRect(m_pCameras[hCamera]->hCamera, bFullScreen, rect.left, rect.top, rect.right, rect.bottom);
}

//*************************************************************************//

void CameraMgr::SetCameraLightAdd(HCAMERA hCamera, LTVector &vLightAdd)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	m_pCameras[hCamera]->cidOriginal.vLightAdd = vLightAdd;
	g_pLTClient->SetCameraLightAdd(m_pCameras[hCamera]->hCamera, &vLightAdd);
}

//*************************************************************************//

void CameraMgr::SetCameraLightScale(HCAMERA hCamera, LTVector &vLightScale)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	m_pCameras[hCamera]->cidOriginal.vLightScale = vLightScale;
}

//*************************************************************************//

void CameraMgr::SetCameraPreRenderFunc(HCAMERA hCamera, LTBOOL (*fp)(CameraMgr *pMgr, HCAMERA hCamera))
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	m_pCameras[hCamera]->fpPreRender = fp;
}

//*************************************************************************//

void CameraMgr::SetCameraPostRenderFunc(HCAMERA hCamera, void (*fp)(CameraMgr *pMgr, HCAMERA hCamera))
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	m_pCameras[hCamera]->fpPostRender = fp;
}

//*************************************************************************//

CAMERAINSTANCE* CameraMgr::GetCamera(HCAMERA hCamera)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return LTNULL;

	return m_pCameras[hCamera];
}

//*************************************************************************//

uint32 CameraMgr::GetCameraFlags(HCAMERA hCamera)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return 0;

	return m_pCameras[hCamera]->dwFlags;
}

//*************************************************************************//

HOBJECT CameraMgr::GetCameraObject(HCAMERA hCamera)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return LTNULL;

	return m_pCameras[hCamera]->hCamera;
}

//*************************************************************************//

void CameraMgr::GetCameraPos(HCAMERA hCamera, LTVector &vPos, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	if(bFX)
		vPos = m_pCameras[hCamera]->cidFinal.vPos;
//		g_pLTClient->GetObjectPos(m_pCameras[hCamera]->hCamera, &vPos);
	else
		vPos = m_pCameras[hCamera]->cidOriginal.vPos;
}

//*************************************************************************//

void CameraMgr::GetCameraRotation(HCAMERA hCamera, LTRotation &rRot, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	if(bFX)
		rRot = m_pCameras[hCamera]->cidFinal.rRot;
//		g_pLTClient->GetObjectRotation(m_pCameras[hCamera]->hCamera, &rRot);
	else
		rRot = m_pCameras[hCamera]->cidOriginal.rRot;
}

//*************************************************************************//

HOBJECT CameraMgr::GetCameraObj(HCAMERA hCamera, char *szSocket, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return LTNULL;

	if(bFX)
	{
		if(szSocket) strncpy(szSocket, m_pCameras[hCamera]->cidFinal.szObjSocket, CAMERAMGR_MAX_SOCKET_NAME);
		return m_pCameras[hCamera]->cidFinal.hObj;
	}
	else
	{
		if(szSocket) strncpy(szSocket, m_pCameras[hCamera]->cidOriginal.szObjSocket, CAMERAMGR_MAX_SOCKET_NAME);
		return m_pCameras[hCamera]->cidOriginal.hObj;
	}
}

//*************************************************************************//

HOBJECT CameraMgr::GetCameraTarget(HCAMERA hCamera, char *szSocket, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return LTNULL;

	if(bFX)
	{
		if(szSocket) strncpy(szSocket, m_pCameras[hCamera]->cidFinal.szTargetSocket, CAMERAMGR_MAX_SOCKET_NAME);
		return m_pCameras[hCamera]->cidFinal.hTarget;
	}
	else
	{
		if(szSocket) strncpy(szSocket, m_pCameras[hCamera]->cidOriginal.szTargetSocket, CAMERAMGR_MAX_SOCKET_NAME);
		return m_pCameras[hCamera]->cidOriginal.hTarget;
	}

}

//*************************************************************************//

void CameraMgr::GetCameraTargetPos(HCAMERA hCamera, LTVector &vPos, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera])
		return;

	if(bFX)
		vPos = m_pCameras[hCamera]->cidFinal.vTargetPos;
	else
		vPos = m_pCameras[hCamera]->cidOriginal.vTargetPos;
}

//*************************************************************************//

void CameraMgr::GetCameraFOV(HCAMERA hCamera, LTFLOAT &fFOVX, LTFLOAT &fFOVY, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return;

	if(bFX)
	{
		fFOVX = MATH_RADIANS_TO_DEGREES(m_pCameras[hCamera]->cidFinal.fFOVX);
		fFOVY = MATH_RADIANS_TO_DEGREES(m_pCameras[hCamera]->cidFinal.fFOVY);
//		g_pLTClient->GetCameraFOV(m_pCameras[hCamera]->hCamera, &fFOVX, &fFOVY);
	}
	else
	{
		fFOVX = MATH_RADIANS_TO_DEGREES(m_pCameras[hCamera]->cidOriginal.fFOVX);
		fFOVY = MATH_RADIANS_TO_DEGREES(m_pCameras[hCamera]->cidOriginal.fFOVY);
	}
}

//*************************************************************************//

LTBOOL CameraMgr::GetCameraRect(HCAMERA hCamera, LTRect &rect, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return LTFALSE;

	LTBOOL bFullScreen;

	if(bFX)
	{
		bFullScreen = m_pCameras[hCamera]->cidFinal.bFullScreen;
		rect = m_pCameras[hCamera]->cidFinal.rRect;
//		g_pLTClient->GetCameraRect(m_pCameras[hCamera]->hCamera, &bFullScreen, &rect.left, &rect.top, &rect.right, &rect.bottom);
	}
	else
	{
		bFullScreen = m_pCameras[hCamera]->cidOriginal.bFullScreen;
		rect = m_pCameras[hCamera]->cidOriginal.rRect;
	}

	return bFullScreen;
}

//*************************************************************************//

LTBOOL CameraMgr::GetCameraLightAdd(HCAMERA hCamera, LTVector &vLightAdd, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return LTFALSE;

	if(bFX)
		vLightAdd = m_pCameras[hCamera]->cidFinal.vLightAdd;
	else
		vLightAdd = m_pCameras[hCamera]->cidOriginal.vLightAdd;

	return (vLightAdd.x || vLightAdd.y || vLightAdd.z);
}

//*************************************************************************//

LTBOOL CameraMgr::GetCameraLightScale(HCAMERA hCamera, LTVector &vLightScale, LTBOOL bFX)
{
	// Make sure this camera exists
	if(!m_pCameras[hCamera] || !m_pCameras[hCamera]->hCamera)
		return LTFALSE;

	if(bFX)
		vLightScale = m_pCameras[hCamera]->cidFinal.vLightScale;
	else
		vLightScale = m_pCameras[hCamera]->cidOriginal.vLightScale;

	return (vLightScale.x || vLightScale.y || vLightScale.z);
}

//*************************************************************************//

HCAMERA CameraMgr::FindOpenCameraSlot()
{
	// Loop through the list of cameras looking for the first available slot
	for(HCAMERA hIndex = 0; hIndex < CAMERAMGR_MAX_CAMERAS; hIndex++)
	{
		if(!m_pCameras[hIndex])
			return hIndex;
	}

	// Otherwise, return an invalid index
	return CAMERAMGR_INVALID;
}

//*************************************************************************//

void CameraMgr::UpdateToObjPosition(HCAMERA hCamera)
{
	// Get the camera instance we're working with...
	CAMERAINSTANCE *pCamera = m_pCameras[hCamera];

	// If the obj doesn't exist... get us out of here
	if(!pCamera->cidOriginal.hObj)
		return;

	// Get the model interface
	ModelLT *pModel = g_pLTClient->GetModelLT();
	HMODELSOCKET hSocket;
	LTVector vPos;

	// If the socket does exist... use it's position
	if( (pCamera->dwFlags & CAMERA_FLAG_FOLLOW_OBJ_SOCKET_POS) && (pCamera->cidOriginal.szObjSocket[0] != '\0') &&
		(pModel->GetSocket(pCamera->cidOriginal.hObj, pCamera->cidOriginal.szObjSocket, hSocket) == LT_OK))
	{
		LTransform trans;
		pModel->GetSocketTransform(pCamera->cidOriginal.hObj, hSocket, trans, LTTRUE);
		vPos = trans.m_Pos;
	}
	else
	{
		// Otherwise, get the position of the obj
		g_pLTClient->GetObjectPos(pCamera->cidOriginal.hObj, &vPos);
	}

	// -------------------------------------------------------------------------- //
	// If the interpolation flags are on... smooth out the position of the camera
	if(pCamera->dwFlags & CAMERA_FLAG_INTERP_POS_ALL)
	{
		// Get the direction and rotation vectors
		LTVector vR, vU, vF;
		LTFLOAT fDot = 0.0f;
		LTFLOAT fOffset = g_vtOrigPosInterpOffset.GetFloat();
		LTFLOAT fTime = g_pLTClient->GetTime();
		LTVector vDist = vPos - pCamera->cidOriginal.vPos;
		g_pLTClient->GetMathLT()->GetRotationVectors(pCamera->cidOriginal.rRot, vR, vU, vF);

		if(pCamera->dwFlags & CAMERA_FLAG_INTERP_POS_RIGHT)
		{
			// Make sure the offset is great enough to do the interpolation
			fDot = vDist.Dot(vR);

			if(fDot >= fOffset || fDot <= -fOffset)
			{
				LTFLOAT fInterpScale = (fTime - pCamera->ceid.fPosR_ST) / g_vtOrigPosInterpTime.GetFloat();
				if(fInterpScale > 1.0f) fInterpScale = 1.0f;
				vPos -= (vR * fDot * (1.0f - fInterpScale));
			}
			else
			{
				pCamera->ceid.fPosR_ST = fTime;
			}
		}

		if(pCamera->dwFlags & CAMERA_FLAG_INTERP_POS_UP)
		{
			// Make sure the offset is great enough to do the interpolation
			fDot = vDist.Dot(vU);

			if(fDot >= fOffset || fDot <= -fOffset)
			{
				LTFLOAT fInterpScale = (fTime - pCamera->ceid.fPosU_ST) / g_vtOrigPosInterpTime.GetFloat();
				if(fInterpScale > 1.0f) fInterpScale = 1.0f;
				vPos -= (vU * fDot * (1.0f - fInterpScale));
			}
			else
			{
				pCamera->ceid.fPosU_ST = fTime;
			}
		}

		if(pCamera->dwFlags & CAMERA_FLAG_INTERP_POS_FORWARD)
		{
			// Make sure the offset is great enough to do the interpolation
			fDot = vDist.Dot(vF);

			if(fDot >= fOffset || fDot <= -fOffset)
			{
				LTFLOAT fInterpScale = (fTime - pCamera->ceid.fPosF_ST) / g_vtOrigPosInterpTime.GetFloat();
				if(fInterpScale > 1.0f) fInterpScale = 1.0f;
				vPos -= (vF * fDot * (1.0f - fInterpScale));
			}
			else
			{
				pCamera->ceid.fPosF_ST = fTime;
			}
		}
	}
	// -------------------------------------------------------------------------- //

	// Set the camera object to the new location
	pCamera->cidOriginal.vPos = vPos;
}

//*************************************************************************//

void CameraMgr::UpdateToObjRotation(HCAMERA hCamera)
{
	// Get the camera instance we're working with...
	CAMERAINSTANCE *pCamera = m_pCameras[hCamera];

	// If the obj doesn't exist... get us out of here
	if(!pCamera->cidOriginal.hObj)
		return;

	// Get the model interface
	ModelLT *pModel = g_pLTClient->GetModelLT();
	HMODELSOCKET hSocket;
	LTRotation rRot;
	rRot.Init();

	// If the socket does exist... use it's rotation
	if( (pCamera->dwFlags & CAMERA_FLAG_FOLLOW_OBJ_SOCKET_ROT) && (pCamera->cidOriginal.szObjSocket[0] != '\0') &&
		(pModel->GetSocket(pCamera->cidOriginal.hObj, pCamera->cidOriginal.szObjSocket, hSocket) == LT_OK))
	{
		LTransform trans;
		pModel->GetSocketTransform(pCamera->cidOriginal.hObj, hSocket, trans, LTTRUE);
		rRot = trans.m_Rot;
	}
	else
	{
		// Otherwise, get the rotation of the obj
		g_pLTClient->GetObjectRotation(pCamera->cidOriginal.hObj, &rRot);
	}

	// -------------------------------------------------------------------------- //
	// If the interpolation flags are on... smooth out the position of the camera
	if(pCamera->dwFlags & CAMERA_FLAG_INTERP_ROT)
	{
		LTVector vR, vF;
		LTVector vU1, vU2;

		g_pLTClient->GetMathLT()->GetRotationVectors(pCamera->cidOriginal.rRot, vR, vU1, vF);
		g_pLTClient->GetMathLT()->GetRotationVectors(rRot, vR, vU2, vF);

		if(vU1.Dot(vU2) < 0.999f)  // Interpolate during an up vector change
		{
			LTFLOAT fTime = g_pLTClient->GetFrameTime() * g_vtOrigRotInterpScale.GetFloat();
			pCamera->ceid.fRot_ST = 0.0f;

			LTRotation rDest;
			g_pLTClient->GetMathLT()->InterpolateRotation(rDest, pCamera->cidOriginal.rRot, rRot, fTime);
			rRot = rDest;
		}
		else if(pCamera->ceid.fRot_ST < 1.0f)  // Smooth out the rotation after an up vector change
		{
			LTFLOAT fTime = g_pLTClient->GetFrameTime() * g_vtOrigRotInterpSmoothScale.GetFloat();
			pCamera->ceid.fRot_ST += fTime;

			if(pCamera->ceid.fRot_ST > 1.0f)
				pCamera->ceid.fRot_ST = 1.0f;

			LTRotation rDest;
			g_pLTClient->GetMathLT()->InterpolateRotation(rDest, pCamera->cidOriginal.rRot, rRot, pCamera->ceid.fRot_ST);
			rRot = rDest;
		}
	}
	// -------------------------------------------------------------------------- //

	// Set the camera object to the new location
	pCamera->cidOriginal.rRot = rRot;
}

//*************************************************************************//

void CameraMgr::UpdateToTargetRotation(HCAMERA hCamera)
{
	// Get the camera instance we're working with...
	CAMERAINSTANCE *pCamera = m_pCameras[hCamera];


	// Store the position to look at all the time
	LTVector vPos;


	// The TARGET_POS flag overrides the other flags
	if(pCamera->dwFlags & CAMERA_FLAG_FOLLOW_TARGET_POS)
	{
		vPos = pCamera->cidOriginal.vTargetPos;
	}
	else
	{
		// Get the model interface
		ModelLT *pModel = g_pLTClient->GetModelLT();
		HMODELSOCKET hSocket;

		// If the socket does exist... use it's position
		if( (pCamera->dwFlags & CAMERA_FLAG_FOLLOW_TARGET_SOCKET) && (pCamera->cidOriginal.szTargetSocket[0] != '\0') &&
			(pModel->GetSocket(pCamera->cidOriginal.hTarget, pCamera->cidOriginal.szTargetSocket, hSocket) == LT_OK))
		{
			LTransform trans;
			pModel->GetSocketTransform(pCamera->cidOriginal.hTarget, hSocket, trans, LTTRUE);
			vPos = trans.m_Pos;
		}
		else
		{
			// Otherwise, get the position of the obj
			g_pLTClient->GetObjectPos(pCamera->cidOriginal.hTarget, &vPos);
		}
	}


	// Get the rotation vectors
	LTVector vR, vU, vF;
	g_pMathLT->GetRotationVectors(pCamera->cidOriginal.rRot, vR, vU, vF);

	// Figure out a new direction to be facing
	LTVector vDir = vPos - pCamera->cidOriginal.vPos;

	// Now align us to the new facing
	g_pMathLT->AlignRotation(pCamera->cidOriginal.rRot, vDir, vU);
}
