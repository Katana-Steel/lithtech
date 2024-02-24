// ----------------------------------------------------------------------- //
//
// MODULE  : ClientUtilities.cpp
//
// PURPOSE : Utility functions
//
// CREATED : 9/25/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientUtilities.h"
#include "GameClientShell.h"
#include "ClientRes.h"
#include "VarTrack.h"

//**************************************************************
// Display profile times for certain parts of the code...

LTCounter g_ltCounter;

#define _PROFILE_OUTPUT_

void StartProfile()
{
#ifdef _PROFILE_OUTPUT_
	g_pLTClient->StartCounter(&g_ltCounter);
#endif
}

void EndProfile(const char *szPrint)
{
#ifdef _PROFILE_OUTPUT_
	uint32 nTime = g_pLTClient->EndCounter(&g_ltCounter);
	g_pLTClient->CPrint("Profiling | %64s | TIME: %9d | MS: %4d | FPS: %.2f", szPrint ? szPrint : "Unknown", nTime, nTime / 1000, 1000000.0f / (LTFLOAT)nTime);
#endif
}

//**************************************************************


extern CGameClientShell* g_pGameClientShell;


//************************************************************************************************************
//**      THIS DATA IS ONLY USED FOR CONFIGURING CONTROLS							**************************
//**	do not add commands here unless they should be reconfigurable by the user	**************************
//************************************************************************************************************
CommandData g_CommandArray[] = 
{
	{ IDS_CONTROL_FORWARD,			COMMAND_ID_FORWARD,			IDS_ACTION_FORWARD,			COM_SHARED },
	{ IDS_CONTROL_BACKWARD,			COMMAND_ID_BACKWARD,		IDS_ACTION_BACKWARD,		COM_SHARED },
	{ IDS_CONTROL_LEFT,				COMMAND_ID_LEFT,			IDS_ACTION_LEFT,			COM_SHARED },
	{ IDS_CONTROL_RIGHT,			COMMAND_ID_RIGHT,			IDS_ACTION_RIGHT,			COM_SHARED },
	{ IDS_CONTROL_STRAFELEFT,		COMMAND_ID_STRAFE_LEFT,		IDS_ACTION_STRAFELEFT,		COM_SHARED },
	{ IDS_CONTROL_STRAFERIGHT,		COMMAND_ID_STRAFE_RIGHT,	IDS_ACTION_STRAFERIGHT,		COM_SHARED },
	{ IDS_CONTROL_STRAFE,			COMMAND_ID_STRAFE,			IDS_ACTION_STRAFE,			COM_SHARED },
	{ IDS_CONTROL_RUN,				COMMAND_ID_RUN,				IDS_ACTION_RUN,				COM_SHARED },
	{ IDS_CONTROL_RUNLOCK,			COMMAND_ID_RUNLOCK,			IDS_ACTION_RUNLOCK,			COM_SHARED },
	{ IDS_CONTROL_JUMP,				COMMAND_ID_JUMP,			IDS_ACTION_JUMP,			COM_SHARED },
	{ IDS_CONTROL_FIRE,				COMMAND_ID_FIRING,			IDS_ACTION_FIRE,			COM_SHARED },
	{ IDS_CONTROL_ALTFIRE,			COMMAND_ID_ALT_FIRING,		IDS_ACTION_ALTFIRE,			COM_SHARED },
	{ IDS_CONTROL_NEXTVISION,		COMMAND_ID_NEXT_VISIONMODE,	IDS_ACTION_NEXTVISION,		COM_SHARED },
	{ IDS_CONTROL_LOOKUP,			COMMAND_ID_LOOKUP,			IDS_ACTION_LOOKUP,			COM_SHARED },
	{ IDS_CONTROL_LOOKDOWN,			COMMAND_ID_LOOKDOWN,		IDS_ACTION_LOOKDOWN,		COM_SHARED },
	{ IDS_CONTROL_CENTERVIEW,		COMMAND_ID_CENTERVIEW,		IDS_ACTION_CENTERVIEW,		COM_SHARED },
	{ IDS_CONTROL_MOUSELOOK,		COMMAND_ID_MOUSEAIMTOGGLE,	IDS_ACTION_MOUSELOOK,		COM_SHARED },
	{ IDS_CONTROL_SCOREDISPLAY,		COMMAND_ID_SCOREDISPLAY,	IDS_ACTION_SCOREDISPLAY,	COM_SHARED },
	{ IDS_CONTROL_MESSAGE,			COMMAND_ID_MESSAGE,			IDS_ACTION_MESSAGE,			COM_SHARED },
	{ IDS_CONTROL_TEAMMESSAGE,		COMMAND_ID_TEAMMESSAGE,		IDS_ACTION_TEAMMESSAGE,		COM_SHARED },
	{ IDS_CONTROL_TAUNT,			COMMAND_ID_TAUNT,			IDS_ACTION_TAUNT,			COM_SHARED },
	{ IDS_CONTROL_CROSSHAIR,		COMMAND_ID_CROSSHAIRTOGGLE,	IDS_ACTION_CROSSHAIR,		COM_SHARED },


	{ IDS_CONTROL_DUCK,				COMMAND_ID_DUCK,			IDS_ACTION_DUCK,			COM_MARINE },
	{ IDS_CONTROL_CROUCH_TOGGLE,	COMMAND_ID_CROUCH_TOGGLE,	IDS_ACTION_CROUCH_TOGGLE,	COM_MARINE },
	{ IDS_CONTROL_ACTIVATE,			COMMAND_ID_ACTIVATE,		IDS_ACTION_ACTIVATE,		COM_MARINE },
	{ IDS_CONTROL_NEXTWEAPON,		COMMAND_ID_NEXT_WEAPON,		IDS_ACTION_NEXTWEAPON,		COM_MARINE },
	{ IDS_CONTROL_PREVWEAPON,		COMMAND_ID_PREV_WEAPON,		IDS_ACTION_PREVWEAPON,		COM_MARINE },
	{ IDS_CONTROL_RELOAD,			COMMAND_ID_RELOAD,			IDS_ACTION_RELOAD,			COM_MARINE },
	{ IDS_CONTROL_ITEM0,			COMMAND_ID_SHOULDERLAMP,	IDS_ACTION_ITEM0,			COM_MARINE },
	{ IDS_CONTROL_ITEM1,			COMMAND_ID_FLARE,			IDS_ACTION_ITEM1,			COM_MARINE },
	{ IDS_CONTROL_ITEM2,			COMMAND_ID_HACKING_DEVICE,	IDS_ACTION_ITEM2,			COM_MARINE },
	{ IDS_CONTROL_TORCH_SELECT,		COMMAND_ID_TORCH_SELECT,	IDS_ACTION_TORCH_SELECT,	COM_MARINE },
	{ IDS_CONTROL_WEAPON_M0,		COMMAND_ID_WEAPON_0,		IDS_ACTION_WEAPON0,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M1,		COMMAND_ID_WEAPON_1,		IDS_ACTION_WEAPON1,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M2,		COMMAND_ID_WEAPON_2,		IDS_ACTION_WEAPON2,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M3,		COMMAND_ID_WEAPON_3,		IDS_ACTION_WEAPON3,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M4,		COMMAND_ID_WEAPON_4,		IDS_ACTION_WEAPON4,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M5,		COMMAND_ID_WEAPON_5,		IDS_ACTION_WEAPON5,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M6,		COMMAND_ID_WEAPON_6,		IDS_ACTION_WEAPON6,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M7,		COMMAND_ID_WEAPON_7,		IDS_ACTION_WEAPON7,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M8,		COMMAND_ID_WEAPON_8,		IDS_ACTION_WEAPON8,			COM_MARINE },
	{ IDS_CONTROL_WEAPON_M9,		COMMAND_ID_WEAPON_9,		IDS_ACTION_WEAPON9,			COM_MARINE },
	{ IDS_CONTROL_LAST_WEAPON,		COMMAND_ID_LAST_WEAPON,		IDS_ACTION_LAST_WEAPON,		COM_MARINE },


	{ IDS_CONTROL_DUCK,				COMMAND_ID_DUCK,			IDS_ACTION_DUCK,			COM_PREDATOR },
	{ IDS_CONTROL_CROUCH_TOGGLE,	COMMAND_ID_CROUCH_TOGGLE,	IDS_ACTION_CROUCH_TOGGLE,	COM_PREDATOR },
	{ IDS_CONTROL_ACTIVATE,			COMMAND_ID_ACTIVATE,		IDS_ACTION_ACTIVATE,		COM_PREDATOR },
	{ IDS_CONTROL_NEXTWEAPON,		COMMAND_ID_NEXT_WEAPON,		IDS_ACTION_NEXTWEAPON,		COM_PREDATOR },
	{ IDS_CONTROL_PREVWEAPON,		COMMAND_ID_PREV_WEAPON,		IDS_ACTION_PREVWEAPON,		COM_PREDATOR },
	{ IDS_CONTROL_RELOAD,			COMMAND_ID_RELOAD,			IDS_ACTION_RELOAD,			COM_PREDATOR },
	{ IDS_CONTROL_ZOOMIN,			COMMAND_ID_ZOOM_IN,			IDS_ACTION_ZOOMIN,			COM_PREDATOR },
	{ IDS_CONTROL_ZOOMOUT,			COMMAND_ID_ZOOM_OUT,		IDS_ACTION_ZOOMOUT,			COM_PREDATOR },
	{ IDS_CONTROL_ITEM3,			COMMAND_ID_HACKING_DEVICE,	IDS_ACTION_ITEM2,			COM_PREDATOR },
	{ IDS_CONTROL_ITEM8,			COMMAND_ID_CLOAK,			IDS_ACTION_ITEM8,			COM_PREDATOR },
	{ IDS_CONTROL_ITEM9,			COMMAND_ID_DISC_RETRIEVE,	IDS_ACTION_ITEM9,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P0,		COMMAND_ID_WEAPON_0,		IDS_ACTION_WEAPON0,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P1,		COMMAND_ID_WEAPON_1,		IDS_ACTION_WEAPON1,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P2,		COMMAND_ID_WEAPON_2,		IDS_ACTION_WEAPON2,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P3,		COMMAND_ID_WEAPON_3,		IDS_ACTION_WEAPON3,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P4,		COMMAND_ID_WEAPON_4,		IDS_ACTION_WEAPON4,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P5,		COMMAND_ID_WEAPON_5,		IDS_ACTION_WEAPON5,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P6,		COMMAND_ID_WEAPON_6,		IDS_ACTION_WEAPON6,			COM_PREDATOR },
	{ IDS_CONTROL_WEAPON_P7,		COMMAND_ID_WEAPON_7,		IDS_ACTION_WEAPON7,			COM_PREDATOR },
	{ IDS_CONTROL_LAST_WEAPON,		COMMAND_ID_LAST_WEAPON,		IDS_ACTION_LAST_WEAPON,		COM_PREDATOR },
	{ IDS_CONTROL_ENERGY_SIFT,		COMMAND_ID_ENERGY_SIFT,		IDS_ACTION_ENERGY_SIFT,		COM_PREDATOR },
	{ IDS_CONTROL_MEDI_COMP,		COMMAND_ID_MEDI_COMP,		IDS_ACTION_MEDI_COMP,		COM_PREDATOR },
	{ IDS_CONTROL_PREVVISION,		COMMAND_ID_PREV_VISIONMODE,	IDS_ACTION_PREVVISION,		COM_PREDATOR },
	{ IDS_CONTROL_LEAP,				-1,							IDS_ACTION_LEAP,			COM_PREDATOR },


	{ IDS_CONTROL_POUNCE_JUMP,		COMMAND_ID_POUNCE_JUMP,		IDS_ACTION_POUNCE_JUMP,		COM_ALIEN },
	{ IDS_CONTROL_WALLWALK,			COMMAND_ID_WALLWALK,		IDS_ACTION_WALLWALK,		COM_ALIEN },
	{ IDS_CONTROL_WALLWALK_TOGGLE,	COMMAND_ID_WALLWALK_TOGGLE,	IDS_ACTION_WALLWALK_TOGGLE,	COM_ALIEN },
	{ IDS_CONTROL_DUCK,				COMMAND_ID_DUCK,			IDS_ACTION_DUCK,			COM_ALIEN },
	{ IDS_CONTROL_CROUCH_TOGGLE,	COMMAND_ID_CROUCH_TOGGLE,	IDS_ACTION_CROUCH_TOGGLE,	COM_ALIEN },

	{ IDS_CONTROL_PLAYERINFO,		-1,							IDS_ACTION_PLAYERINFO,		COM_SHARED },
	{ IDS_CONTROL_HEADBITE,			-1,							IDS_ACTION_HEADBITE,		COM_ALIEN },
	{ IDS_CONTROL_FACEHUG,			-1,							IDS_ACTION_FACEHUG,			COM_ALIEN },
	{ IDS_CONTROL_TEAR,				-1,							IDS_ACTION_TEAR,			COM_ALIEN },

	// This control must always remain as the last one in the array
	{ IDS_CONTROL_UNASSIGNED,		COMMAND_ID_UNASSIGNED,		IDS_ACTION_UNASSIGNED }
};

const int g_kNumCommands = sizeof(g_CommandArray) / sizeof(g_CommandArray[0]) -1;

//-------------------------------------------------------------------------------------------
// CommandName
//
// Retrieves the command name from a command number
// Arguments:
//		nCommand - command number
// Return:
//		String containing the name of the action
//-------------------------------------------------------------------------------------------

char* CommandName(int nCommand)
{
	static char buffer[128];

	DDWORD nStringID = 0;

	for (int i=0; i < g_kNumCommands; i++)
	{
		if (g_CommandArray[i].nCommandID == nCommand)
		{
			nStringID = g_CommandArray[i].nActionStringID;
			break;
		}
	}

	if (nStringID)
	{
		HSTRING hStr = g_pClientDE->FormatString(nStringID);
		SAFE_STRCPY(buffer, g_pClientDE->GetStringData(hStr));
		g_pClientDE->FreeString(hStr);
	}
	else
	{
		SAFE_STRCPY(buffer, "Error in CommandName()!");
	}

	return buffer;
}


//-------------------------------------------------------------------------------------------
// CropSurface
//
// Crops the given surface to smallest possible area
// Arguments:
//		hSurf - surface to be cropped
//		hBorderColor - color of area to be cropped
// Return:
//		cropped surface if successful, original surface otherwise
//-------------------------------------------------------------------------------------------
HSURFACE CropSurface ( HSURFACE hSurf, HDECOLOR hBorderColor )
{
	if (!g_pGameClientShell) return hSurf;

	if (!hSurf) return DNULL;
	
	CClientDE* pClientDE = g_pGameClientShell->GetClientDE();
	if (!pClientDE) return hSurf;

	DDWORD nWidth, nHeight;
	pClientDE->GetSurfaceDims (hSurf, &nWidth, &nHeight);

	DRect rcBorders;
	memset (&rcBorders, 0, sizeof (DRect));
	pClientDE->GetBorderSize (hSurf, hBorderColor, &rcBorders);

	if (rcBorders.left == 0 && rcBorders.top == 0 && rcBorders.right == 0 && rcBorders.bottom == 0) return hSurf;

	HSURFACE hCropped = pClientDE->CreateSurface (nWidth - rcBorders.left - rcBorders.right, nHeight - rcBorders.top - rcBorders.bottom);
	if (!hCropped) return hSurf;

	DRect rcSrc;
	rcSrc.left = rcBorders.left;
	rcSrc.top = rcBorders.top;
	rcSrc.right = nWidth - rcBorders.right;
	rcSrc.bottom = nHeight - rcBorders.bottom;

	pClientDE->DrawSurfaceToSurface (hCropped, hSurf, &rcSrc, 0, 0);

	pClientDE->DeleteSurface (hSurf);

	return hCropped;
}

static VarTrack s_cvarFirePitchShift;
void PlayWeaponSound(WEAPON *pWeapon, BARREL *pBarrel, LTVector vPos, WeaponModelKeySoundId eSoundId,
					 LTBOOL bLocal)
{
	if (!pWeapon || !pBarrel) return;

 	if (!s_cvarFirePitchShift.IsInitted())
	{
		s_cvarFirePitchShift.Init(g_pLTClient, "PitchShiftFire", NULL, -1.0f);
	}

	char* pSnd = LTNULL;

	LTFLOAT fRadius = WEAPON_SOUND_RADIUS;

	switch (eSoundId)
	{
		case WMKS_FIRE:
		{
			pSnd = pBarrel->szFireSound;
			fRadius = (LTFLOAT) pBarrel->fFireSoundRadius;
		}
		break;

		case WMKS_ALT_FIRE:
		{
			pSnd = pBarrel->szAltFireSound;
			fRadius = (LTFLOAT) pBarrel->fFireSoundRadius;
		}
		break;

		case WMKS_DRY_FIRE:
		{
			pSnd = pBarrel->szDryFireSound;
			fRadius = (LTFLOAT) pBarrel->fFireSoundRadius;
		}
		break;

		case WMKS_MISC0:
		case WMKS_MISC1:
		case WMKS_MISC2:
		{
			pSnd = pBarrel->szMiscSounds[eSoundId];
		}
		break;

		case WMKS_SELECT:
		{
			pSnd = pWeapon->szSelectSound;
		}
		break;

		case WMKS_DESELECT:
		{
			pSnd = pWeapon->szDeselectSound;
		}
		break;

		case WMKS_INVALID:
		default : break;
	}

	if (pSnd && pSnd[0])
	{
		uint32 dwFlags = 0;
		float fPitchShift = 1.0f;
		if (s_cvarFirePitchShift.GetFloat() > 0.0f)
		{
			dwFlags |= PLAYSOUND_CTRL_PITCH;
			fPitchShift = s_cvarFirePitchShift.GetFloat();
		}

 		if (bLocal)
		{
			g_pClientSoundMgr->PlaySoundLocal(pSnd, SOUNDPRIORITY_PLAYER_HIGH,
				dwFlags, SMGR_DEFAULT_VOLUME, fPitchShift);
		}
		else
		{
 			g_pClientSoundMgr->PlaySoundFromPos(vPos, pSnd,
				fRadius, SOUNDPRIORITY_PLAYER_HIGH, dwFlags,
				SMGR_DEFAULT_VOLUME, fPitchShift);
		}
	}
}

LTBOOL GetScreenPos(HCAMERA hCamera, HOBJECT hObj, LTIntPt &ptRval, LTBOOL bTargetTorso /*LTFALSE*/)
{
	//get camera's position, rotation, and rotation vectors
	LTVector vCamPos;
	LTRotation rRot;
	LTVector vF, vU, vR;
	g_pCameraMgr->GetCameraPos(hCamera, vCamPos, LTTRUE);
	g_pCameraMgr->GetCameraRotation(hCamera, rRot, LTTRUE);
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	//get object's position
	LTVector vObjPos, vObjDims;
	PhysicsLT* pPhysics = g_pLTClient->Physics();
	pPhysics->GetObjectDims(hObj, &vObjDims);

	if(bTargetTorso)
	{
		ModelLT* pModelLT   = g_pClientDE->GetModelLT();

		HMODELNODE hNode;

		if (pModelLT->GetNode(hObj, "Torso_u_node", hNode) == LT_OK)
		{
			LTransform	tTransform;

			if (pModelLT->GetNodeTransform(hObj, hNode, tTransform, LTTRUE) == LT_OK)
			{
				TransformLT* pTransLT = g_pLTClient->GetTransformLT();
				pTransLT->GetPos(tTransform, vObjPos);
			}
			else
				bTargetTorso = LTFALSE;
		}
		else
			bTargetTorso = LTFALSE;
	}

	if(!bTargetTorso)
	{
		g_pLTClient->GetObjectPos(hObj, &vObjPos);

		//adjust to mid chest
		vObjPos.y += vObjDims.y/2;
	}

	//calculate camera to object vector
	LTVector vToObject = vObjPos-vCamPos;
	vToObject.Norm();

	//get camera rect
	LTRect rcCamera;
	g_pCameraMgr->GetCameraRect(hCamera, rcCamera, LTTRUE);

	//get vectors from camera to screen edges
	LTVector vToTL, vToTR, vToBL;
	HOBJECT hCamObj = g_pCameraMgr->GetCameraObject(hCamera);
	g_pLTClient->Get3DCameraPt(hCamObj, rcCamera.left, rcCamera.top, &vToTL);
	g_pLTClient->Get3DCameraPt(hCamObj, rcCamera.right-1,rcCamera.top , &vToTR);
	g_pLTClient->Get3DCameraPt(hCamObj, rcCamera.left, rcCamera.bottom-1, &vToBL);

	//define the screen plane in world coords
	LTPlane plScreen;
	plScreen.Init( vToTR, vToTL, vToBL);

	//find the intersects in world coords
	int nInfo;
	LTVector vObj;
	if(plScreen.RayIntersect(vCamPos, vToObject, nInfo, &vObj) == LTPLANE_INTERSECT_NONE)
		return LTFALSE;

	//calculate vectors and distances
	LTVector vHorz		= vToTR-vToTL;
	LTVector vVert		= vToBL-vToTL;
	LTVector vToPoint	= vObj-vToTL;

	LTFLOAT fHorzDist = vHorz.Mag();
	LTFLOAT fVertDist = vVert.Mag();

	//normalize the horz and vert
	vHorz.Norm();
	vVert.Norm();

	//get the horz and vert distances of our object
	//from the TL corner of the plane
	LTFLOAT fHorzDot = vHorz.Dot(vToPoint);
	LTFLOAT fVertDot = vVert.Dot(vToPoint);

	//compute results
	ptRval.x = (int)(rcCamera.left+(fHorzDot/fHorzDist*(rcCamera.right-rcCamera.left)));
	ptRval.y = (int)(rcCamera.top+(fVertDot/fVertDist*(rcCamera.bottom-rcCamera.top)));

	return LTTRUE;
}


// Takes the hCamera and determines the scale factors to apply to a sprite overlay.
void GetCameraScale(HCAMERA hCamera, LTFLOAT distance, LTFLOAT &scaleX, LTFLOAT &scaleY)
{
	// Get the camera rect
	LTRect rcCamera;
	g_pCameraMgr->GetCameraRect(hCamera, rcCamera, LTTRUE);

	// Get x position as 0, 1/2 bottom
	int x_posX = 0;
	int x_posY = rcCamera.bottom/2;

	// Get y position as 0, 1/2 right
	int y_posX = rcCamera.right/2;
	int y_posY = 0;

	// Get a world point for each screen location
	LTVector v3DCamPtX, v3DCamPtY;
	HOBJECT hCamObj = g_pCameraMgr->GetCameraObject(hCamera);

	//get camera's position, rotation, and rotation vectors
	LTRotation rRot;
	LTVector vCamF, vCamU, vCamR, vCamPos;
	g_pLTClient->GetObjectPos(hCamObj, &vCamPos);
	g_pLTClient->GetObjectRotation(hCamObj, &rRot);
	g_pLTClient->GetRotationVectors(&rRot, &vCamU, &vCamR, &vCamF);

	// Move the camera to the origin to avoid rounding error
	g_pLTClient->SetObjectPos(hCamObj, &LTVector(0,0,0));

	g_pLTClient->Get3DCameraPt(hCamObj, x_posX, x_posY, &v3DCamPtX);
	g_pLTClient->Get3DCameraPt(hCamObj, y_posX, y_posY, &v3DCamPtY);

	// Now put the camera back
	g_pLTClient->SetObjectPos(hCamObj, &vCamPos);

	// Dot x position with -Camera right to get x distance
	LTFLOAT fXDist = v3DCamPtX.Dot(-vCamR);

	// scaleX
	scaleX = distance * fXDist;

	// Dot y position with Camera up to get y distance
	LTFLOAT fYDist = v3DCamPtY.Dot(vCamU);


	// ScaleY
	scaleY = distance * fYDist;
}

void GetConsoleString(char* sKey, char* sDest, char* sDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(sKey);
		if (hVar)
		{
            char* sValue = g_pLTClient->GetVarValueString(hVar);
			if (sValue)
			{
				strcpy(sDest, sValue);
				return;
			}
		}
	}

	strcpy(sDest, sDefault);
}

int GetConsoleInt(char* sKey, int nDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(sKey);
		if (hVar)
		{
            float fValue = g_pLTClient->GetVarValueFloat(hVar);
			return((int)fValue);
		}
	}

	return(nDefault);
}

LTFLOAT GetConsoleFloat(char* sKey, LTFLOAT fDefault)
{
    if (g_pLTClient)
	{
        HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(sKey);
		if (hVar)
		{
            float fValue = g_pLTClient->GetVarValueFloat(hVar);
			return(fValue);
		}
	}

	return(fDefault);
}

void WriteConsoleString(char* sKey, char* sValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
#ifdef __PSX2
        sprintf(sTemp, "+%s \"%s\"", sKey, sValue);
#else
        wsprintf(sTemp, "+%s \"%s\"", sKey, sValue);
#endif
        g_pLTClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleInt(char* sKey, int nValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
#ifdef __PSX2
        sprintf(sTemp, "+%s %i", sKey, nValue);
#else
		wsprintf(sTemp, "+%s %i", sKey, nValue);
#endif
        g_pLTClient->RunConsoleString(sTemp);
	}
}

void WriteConsoleFloat(char* sKey, LTFLOAT fValue)
{
    if (g_pLTClient)
	{
		char sTemp[256];
		sprintf(sTemp, "+%s %f", sKey, fValue);
        g_pLTClient->RunConsoleString(sTemp);
	}
};


DBOOL GetAttachmentSocketTransform(HOBJECT hObj, char* pSocketName, DVector & vPos, DRotation & rRot)
{
	if (!hObj || !pSocketName) return DFALSE;

	HOBJECT hAttachList[30];
	DDWORD dwListSize, dwNumAttachments;

	if (g_pClientDE->Common()->GetAttachments(hObj, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		for (DDWORD i=0; i < dwListSize; i++)
		{
			if (hAttachList[i])
			{
				HMODELSOCKET hSocket;

				if (g_pModelLT->GetSocket(hAttachList[i], pSocketName, hSocket) == LT_OK)
				{
					LTransform transform;
					if (g_pModelLT->GetSocketTransform(hAttachList[i], hSocket, transform, DTRUE) == LT_OK)
					{
						g_pTransLT->Get(transform, vPos, rRot);
						return DTRUE;
					}
				}	
			}
		}
	}

	return DFALSE;
}


void GetAttachmentsAlpha(HOBJECT hObj, LTFLOAT *pAlphas, uint32 nElements)
{
	if(!hObj || !pAlphas || nElements < 1) return;

	HOBJECT hAttachList[30];
	uint32 dwListSize, dwNumAttachments;
	LTFLOAT r, g, b, a;

	if(g_pLTClient->Common()->GetAttachments(hObj, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		for(uint32 i = 0; i < dwListSize; i++)
		{
			if(i >= nElements)
				return;

			if(hAttachList[i])
			{
				g_pLTClient->GetObjectColor(hAttachList[i], &r, &g, &b, &a);
				pAlphas[i] = a;
			}
		}
	}
}


void SetAttachmentsAlpha(HOBJECT hObj, LTFLOAT fAlpha, uint32 nUsrFlagTest)
{
	if(!hObj) return;

	HOBJECT hAttachList[30];
	uint32 dwListSize, dwNumAttachments;
	LTFLOAT r, g, b, a;
	uint32 nFlags;

	if (g_pLTClient->Common()->GetAttachments(hObj, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		for (uint32 i = 0; i < dwListSize; i++)
		{
			if (hAttachList[i])
			{
				g_pLTClient->GetObjectUserFlags(hAttachList[i], &nFlags);

				if(!(nFlags & nUsrFlagTest))
				{
					g_pLTClient->GetObjectColor(hAttachList[i], &r, &g, &b, &a);

					g_pLTClient->SetObjectColor(hAttachList[i], r, g, b, fAlpha);
				}
			}
		}
	}
}


void SetAttachmentsColor(HOBJECT hObj, LTVector vColor, uint32 nFlagsTest, uint32 nFlags2Test)
{
	if(!hObj) return;

	HOBJECT hAttachList[30];
	uint32 dwListSize, dwNumAttachments;
	LTFLOAT r, g, b, a;
	uint32 nFlags, nFlags2;

	if (g_pLTClient->Common()->GetAttachments(hObj, hAttachList, 
		ARRAY_LEN(hAttachList), dwListSize, dwNumAttachments) == LT_OK)
	{
		for (uint32 i = 0; i < dwListSize; i++)
		{
			if (hAttachList[i])
			{
				g_pLTClient->Common()->GetObjectFlags(hAttachList[i], OFT_Flags, nFlags);
				g_pLTClient->Common()->GetObjectFlags(hAttachList[i], OFT_Flags2, nFlags2);

				if(((nFlags & nFlagsTest) == nFlagsTest) && ((nFlags2 & nFlags2Test) == nFlags2Test))
				{
					g_pLTClient->GetObjectColor(hAttachList[i], &r, &g, &b, &a);
					g_pLTClient->SetObjectColor(hAttachList[i], vColor.x, vColor.y, vColor.z, a);

					if((vColor.x == 0.0f) && (vColor.y == 0.0f) && (vColor.z == 0.0f))
						g_pLTClient->Common()->SetObjectFlags(hAttachList[i], OFT_Flags, nFlags & ~FLAG_VISIBLE);
					else
						g_pLTClient->Common()->SetObjectFlags(hAttachList[i], OFT_Flags, nFlags | FLAG_VISIBLE);
				}
			}
		}
	}
}


