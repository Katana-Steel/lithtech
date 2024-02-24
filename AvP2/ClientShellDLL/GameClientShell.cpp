// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.cpp
//
// PURPOSE : Game Client Shell - Implementation
//
// CREATED : 9/18/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "CommandIds.h"
#include "WeaponModel.h"
#include "ClientUtilities.h"
#include "vkdefs.h"
#include "ClientRes.h"
#include "SoundTypes.h"
#include "Music.h"
#include "VolumeBrushFX.h"
#include "client_physics.h"
#include "CameraFX.h"
#include "WinUtil.h"
#include "WeaponStringDefs.h"
#include "math_lt.h"
#include "physics_lt.h"
#include "VarTrack.h"
#include "GameButes.h"
#include "AssertMgr.h"
#include "SystemDependant.h"
#include "LithTrackMgr.h"
#include "PlayerMovement.h"
#include "CheatMgr.h"
#include "CameraMgrFX.h"
#include "ClientButeMgr.h"
#include "CharacterFX.h"
#include "CharacterFuncs.h"
#include "ProfileMgr.h"
#include "StoryElement.h"
#include "ServerOptionMgr.h"
#include "GameSpyClientMgr.h"
#include "MultiplayerClientMgr.h"
#include "FrameRateMgr.h"
#include "SaveGameData.h"
#include "TeleType.h"
#include "VisionModeButeMGR.h"
#include "FolderMulti.h"
#include "PerformanceMgr.h"
#include "SCDefs.h"
#include "FXButeMgr.h"

// For Deg2Rad()
#include "ServerUtilities.h"

#include "ViewModeMGR.h"


#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>

static _timeb g_StartTimeB;
static _timeb g_EndTimeB;
static uint32 nStartTicks;

// ----------------------------------------------------------------------- //

int iIdentCode0 = 0x4212A901;
char szCopywriteString[] = "© 2001 Twentieth Century Fox Film Corporation.  All rights reserved.  Aliens versus Predator, Fox Interactive and their respective logos are either registered trademarks or trademarks of Twentieth Century Fox Film Corporation.";
int iIdentCode1 = 0x4212A901;

// ----------------------------------------------------------------------- //

// Our current save/load version.
// Formatted as YYYYMMDDN (year, month, day, number).  So that it will always be increasing.
uint32 CGameClientShell::m_nCurrentSaveLoadVersion = 200108140;

#ifdef STRICT
	WNDPROC g_pfnMainWndProc = NULL;
#else
	FARPROC	g_pfnMainWndProc = NULL;
#endif 

#define min(a,b)	((a) < (b) ? (a) : (b))
#define max(a,b)	((a) > (b) ? (a) : (b))

#define MAX_FRAME_DELTA							0.1f

#define RESPAWN_WAIT_TIME						5.0f
#define MULTIPLAYER_RESPAWN_WAIT_TIME			1.0f

#define FORCE_RESPAWN_WAIT_TIME					10.0f
#define MULTIPLAYER_FORCE_RESPAWN_WAIT_TIME		9.0f


// IKCHAIN TESTING CODE!!
//#define _IK_CHAIN_TEST_

#ifdef _IK_CHAIN_TEST_
	#include "IKChain.h"
	IKChain_Constrained	g_ikChain;
#endif


DDWORD				g_dwSpecial			= 2342349;
DBOOL				g_bScreenShotMode	= LTFALSE;
HDECOLOR			g_hColorTransparent = DNULL;

HWND				g_hMainWnd = NULL;
RECT*				g_prcClip = NULL;

MultiplayerClientMgr* g_pMPMgr = NULL;

CheatMgr			m_CheatMgr;
TeleType			m_TeleType;

CClientDE*			g_pInterface = DNULL;
CGameClientShell*	g_pGameClientShell = DNULL;
LTVector				g_vWorldWindVel(0.0f, 0.0f, 0.0f);
PhysicsState		g_normalPhysicsState;
PhysicsState		g_waterPhysicsState;

VarTrack			g_vtActivateOverride;

VarTrack			g_vtDisableScreenFlash;

VarTrack			g_vtCamDamage;
VarTrack			g_vtCamDamagePitch;
VarTrack			g_vtCamDamageRoll;
VarTrack			g_vtCamDamageTime1;
VarTrack			g_vtCamDamageTime2;
VarTrack			g_vtCamDamageVal;
VarTrack			g_vtCamDamagePitchMin;
VarTrack			g_vtCamDamageRollMin;

VarTrack			g_varScreenFadeTime;
VarTrack			g_varDoScreenFade;

VarTrack			g_vtFireJitterDecayTime;
VarTrack			g_vtFireJitterMaxPitchDelta;

VarTrack			g_vtLocalClientObjUpdate;

VarTrack			g_vtInterfceFOVX;
VarTrack			g_vtInterfceFOVY;

VarTrack			g_vtGlowRatio;
VarTrack			g_vtInfoString;
VarTrack			g_vtMPInfoString;

const LTFLOAT g_vtPounceDelay = 2.0f;
const LTFLOAT g_vtAlienSpringDelay = 2.0f;
const LTFLOAT g_vtPredatorSpringDelay = 2.0f;
const LTFLOAT g_vtHumanSpringDelay = 2.0f;

VarTrack			g_vtHuntingWarble;
VarTrack			g_vtOrientOverlay;
VarTrack			g_vtOrientSpeed;

VarTrack			g_vtEnableHints;

VarTrack			g_vtFogPerformanceScale;



static LTFLOAT		s_fDemoTime		= 0.0f;

static LTFLOAT		s_fOrientAngle	= 0.0f;

static DBYTE		s_nLastCamType = CT_FULLSCREEN;
static LTFLOAT		s_fLastMouseSens = 0.0f;
static DamageType	s_dtLastDamageType = DT_UNSPECIFIED;

static LTBOOL		s_bRenderCamera = LTFALSE;

void DefaultModelHook(ModelHookData *pData, void *pUser);
BOOL HookWindow();
void UnhookWindow();


LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Setup..

SETUP_CLIENTSHELL();

IClientShell* CreateClientShell(ILTClient *pClientDE)
{
	// Hook up the AssertMgr

	CAssertMgr::Enable();

	// Get our ClientDE pointer

	g_pInterface = g_pLTClient = pClientDE;
	_ASSERT(g_pLTClient);

	PERFINFO_START( );

	CGameClientShell* pShell = new CGameClientShell;
	_ASSERT(pShell);

	PERFINFO_REPORT( );

	// Init our LT subsystems

    g_pMathLT = g_pLTClient->GetMathLT();
    g_pModelLT = g_pLTClient->GetModelLT();
    g_pTransLT = g_pLTClient->GetTransformLT();
    g_pPhysicsLT = g_pLTClient->Physics();

	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);

    return ((IClientShell*)pShell);
}

void DeleteClientShell(IClientShell *pInputShell)
{
	// Delete our client shell

	if (pInputShell)
	{
		delete ((CGameClientShell*)pInputShell);
	}

	// Unhook the AssertMgr and let the CRT handle asserts once again

	CAssertMgr::Disable();
}


static DBOOL LoadLeakFile(ClientDE *g_pLTClient, char *pFilename);

void LeakFileFn(int argc, char **argv)
{
	if (argc < 1)
	{
		g_pLTClient->CPrint("LeakFile <filename>");
		return;
	}

	if (LoadLeakFile(g_pLTClient, argv[0]))
	{
		g_pLTClient->CPrint("Leak file %s loaded successfully!", argv[0]);
	}
	else
	{
		g_pLTClient->CPrint("Unable to load leak file %s", argv[0]);
	}
}

DBOOL ConnectToTcpIpAddress(CClientDE* g_pLTClient, char* sAddress);

void ConnectFn(int argc, char **argv)
{
	if (argc <= 0)
	{
		g_pLTClient->CPrint("Connect <tcpip address> (use '*' for local net)");
		return;
	}

	ConnectToTcpIpAddress(g_pLTClient, argv[0]);
}

void FragSelfFn(int argc, char **argv)
{
	HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_FRAG_SELF);
	g_pLTClient->EndMessage(hWrite);
}

void CheatFn(int argc, char **argv)
{
	if(g_pCheatMgr && (argc > 0))
		g_pCheatMgr->HandleCheat(argc, argv);
}

void ReloadWeaponAttributesFn(int argc, char **argv)
{
	g_pWeaponMgr->Reload(g_pLTClient);
	g_pLTClient->CPrint("Reloaded weapons attributes file...");
}

void ReloadSurfacesAttributesFn(int argc, char **argv)
{
	g_pSurfaceMgr->Reload(g_pLTClient);
	g_pLTClient->CPrint("Reloaded surface attributes file...");
}

void ReloadFXAttributesFn(int argc, char **argv)
{
	g_pFXButeMgr->Reload(g_pLTClient);
	g_pLTClient->CPrint("Reloaded fx attributes file...");

	// Make sure we re-load the weapons and surface data, it has probably
	// changed...
	ReloadWeaponAttributesFn(0, 0);
	ReloadSurfacesAttributesFn(0, 0);
}

void RecordFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->HandleRecord(argc, argv);
	}
}

void PlayDemoFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->HandlePlaydemo(argc, argv);
	}
}

void ExitLevelFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->HandleExitLevel(argc, argv);
	}
}

void ExitGame(DBOOL bResponse, DDWORD nUserData)
{
	if (bResponse)
	{
		g_pLTClient->Shutdown();
	}
}

void InitSoundFn(int argc, char **argv)
{
	if( g_pGameClientShell )
		g_pGameClientShell->InitSound( );
}

void MusicFn(int argc, char **argv)
{
	if (!g_pGameClientShell->GetMusic()->IsInitialized())
	{
        g_pLTClient->CPrint("Direct Music hasn't been initialized!");
	}

	if (argc < 2)
	{
        g_pLTClient->CPrint("Music <command>");
        g_pLTClient->CPrint("  Commands: (syntax -> Command <Required> [Optional])");
        g_pLTClient->CPrint("    I  <intensity number to set> [when to enact change]");
        g_pLTClient->CPrint("    PS <segment name> [when to begin playing]");
        g_pLTClient->CPrint("    PM <motif style> <motif name> [when to begin playing]");
        g_pLTClient->CPrint("    V  <volume adjustment in db>");
        g_pLTClient->CPrint("    SS <segment name> [when to stop]");
        g_pLTClient->CPrint("    SM <motif name> [when to stop]");
        g_pLTClient->CPrint(" ");
        g_pLTClient->CPrint("  Enact Change Values:");
        g_pLTClient->CPrint("    Default     - Will use the default value that is defined in the Control File or by DirectMusic");
        g_pLTClient->CPrint("    Immediately - Will happen immediately");
        g_pLTClient->CPrint("    Beat        - Will happen on the next beat");
        g_pLTClient->CPrint("    Measure     - Will happen on the next measure");
        g_pLTClient->CPrint("    Grid        - Will happen on the next grid");
        g_pLTClient->CPrint("    Segment     - Will happen on the next segment transition");

		return;
	}

	// Build the command...

	char buf[512];
	buf[0] = '\0';
	sprintf(buf, "Music");
	for (int i=0; i < argc; i++)
	{
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}

	g_pGameClientShell->GetMusic()->ProcessMusicMessage(buf);
}

void TriggerFn(int argc, char **argv)
{
	if (g_pGameClientShell->IsMultiplayerGame())
		return;

	if (argc < 2)
	{
		g_pLTClient->CPrint("Trigger <objectname> <message>");
		return;
	}

	// Create a continuous string from the delimited parameters
	CString szTemp = argv[1];

	for(int i = 2; i < argc; i++)
	{
		szTemp += " (";
		szTemp += argv[i];
		szTemp += ")";
	}

	// Send message to server...
	HSTRING hstrObjName = g_pLTClient->CreateString(argv[0]);
	HSTRING hstrMsg		= g_pLTClient->CreateString(szTemp.GetBuffer(0));

	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_CONSOLE_TRIGGER);
	g_pLTClient->WriteToMessageHString(hMessage, hstrObjName);
	g_pLTClient->WriteToMessageHString(hMessage, hstrMsg);
	g_pLTClient->EndMessage(hMessage);

	g_pLTClient->FreeString(hstrObjName);
	g_pLTClient->FreeString(hstrMsg);
}

void ListFn(int argc, char **argv)
{
	if (argc < 1)
	{
		g_pClientDE->CPrint("List <classtype>");
		return;
	}

	// Send message to server...

	char buf[100];
	sprintf(buf, "List %s", argv[0]);

	HSTRING hstrPlayer = g_pClientDE->CreateString("Player");
	HSTRING hstrMsg = g_pClientDE->CreateString(buf);

	HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_CONSOLE_TRIGGER);
	g_pClientDE->WriteToMessageHString(hMessage, hstrPlayer);
	g_pClientDE->WriteToMessageHString(hMessage, hstrMsg);
	g_pClientDE->EndMessage(hMessage);

	g_pClientDE->FreeString(hstrMsg);
	g_pClientDE->FreeString(hstrPlayer);
}

void GameTimeFn(int argc, char **argv)
{
	if( g_pClientDE )
	{
		const LTFLOAT fTime = g_pClientDE->GetGameTime();
		g_pClientDE->CPrint("%f", fTime);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Pre and Post render update functions
//
//	PURPOSE:	These specify actions that are camera specific updates for
//				pre and post render states
//
// --------------------------------------------------------------------------- //

static LTBOOL PreRender_1stPerson(CameraMgr *pMgr, HCAMERA hCamera)
{
	// Get the camera
	CAMERAINSTANCE *pCamera = pMgr->GetCamera(hCamera);

	if(pCamera && !pCamera->bActive)
		return LTFALSE;

	// Get access to the weapon model
	CWeaponModel *pWeaponModel = g_pGameClientShell->GetWeaponModel();

	LTBOOL bObserving = (g_pGameClientShell->GetPlayerState() == PS_OBSERVING);


	// Make sure the weapon model is visible during 1st person states
	if(pWeaponModel)
	{
		if(!GetConsoleInt("DrawViewModel", 1) || CameraMgrFX_Active(hCamera, CAMERAMGRFX_ID_ZOOM) || bObserving || g_pGameClientShell->IsFacehugged())
		{
			if(pWeaponModel->IsVisible())
				pWeaponModel->SetVisible(LTFALSE);
		}
		else if(!pWeaponModel->IsVisible())
			pWeaponModel->SetVisible(LTTRUE);
	}

	// Get the client object (server player object)
	HOBJECT hPlayerObj = g_pLTClient->GetClientObject();

	// Make sure the client object is invisible
	uint32 nFlags, nUserFlags;
	g_pLTClient->Common()->GetObjectFlags(hPlayerObj, OFT_Flags, nFlags);
	g_pLTClient->GetObjectUserFlags(hPlayerObj, &nUserFlags);

	// This was moved outside of the if statement below in order to consider
	// cloaking effects
//	LTFLOAT r, g, b, a;
//	g_pLTClient->GetObjectColor(hPlayerObj, &r, &g, &b, &a);
//	g_pLTClient->SetObjectColor(hPlayerObj, r, g, b, (nUserFlags & USRFLG_CHAR_CLOAKED) ? 0.0f : 1.0f);

	if(nFlags & FLAG_VISIBLE)
	{
		g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, nFlags & ~FLAG_VISIBLE);
		g_pGameClientShell->HideShowAttachments(hPlayerObj);
	}


	// Get acces to the interface manager and make sure the cursor is displayed
	CInterfaceMgr *pInterface = g_pGameClientShell->GetInterfaceMgr();
	pInterface->EnableCrosshair(!bObserving);
	pInterface->SetDrawInterface(!bObserving);
	pInterface->DrawPlayerStats(!bObserving);
	pInterface->DrawCloseCaption(!bObserving);


	// Set the listener back...
	g_pLTClient->SetListener(LTTRUE, LTNULL, LTNULL);


	// Return LTTRUE so the camera gets rendered
	return LTTRUE;
}

// --------------------------------------------------------------------------- //

static void PostRender_1stPerson(CameraMgr *pMgr, HCAMERA hCamera)
{

}

// --------------------------------------------------------------------------- //

static LTBOOL PreRender_3rdPerson(CameraMgr *pMgr, HCAMERA hCamera)
{
	// Get the camera
	CAMERAINSTANCE *pCamera = pMgr->GetCamera(hCamera);

	if(pCamera && !pCamera->bActive)
		return LTFALSE;

	// Get access to the weapon model
	CWeaponModel *pWeaponModel = g_pGameClientShell->GetWeaponModel();

	// Make sure the weapon model is invisible
	if(pWeaponModel && pWeaponModel->IsVisible())
	{
		// do the model part
		pWeaponModel->SetVisible(LTFALSE);
	}

	// Get the client object (server player object)
	HOBJECT hPlayerObj = g_pLTClient->GetClientObject();

	uint32 nFlags;
	g_pLTClient->Common()->GetObjectFlags(hPlayerObj, OFT_Flags, nFlags);

	// Make sure the client object is invisible
	if(g_pGameClientShell->GetPlayerState() == PS_ALIVE)
	{
		if(!(nFlags & FLAG_VISIBLE))
		{
			g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, nFlags | FLAG_VISIBLE);
			g_pGameClientShell->HideShowAttachments(hPlayerObj);
		}
	}
	else if(nFlags & FLAG_VISIBLE)
	{
		LTFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(hPlayerObj, &r, &g, &b, &a);
		g_pLTClient->SetObjectColor(hPlayerObj, r, g, b, 1.0f);
		g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, nFlags & ~FLAG_VISIBLE);
		g_pGameClientShell->HideShowAttachments(hPlayerObj);
	}

	// Get acces to the interface manager and make sure the cursor is not displayed
	CInterfaceMgr *pInterface = g_pGameClientShell->GetInterfaceMgr();
	pInterface->EnableCrosshair(LTFALSE);
	pInterface->SetDrawInterface(LTTRUE);
	pInterface->DrawPlayerStats(LTFALSE);
	pInterface->DrawCloseCaption(LTTRUE);


	// Set the listener back...
	g_pLTClient->SetListener(LTTRUE, LTNULL, LTNULL);


	// Return LTTRUE so the camera gets rendered
	return LTTRUE;
}

// --------------------------------------------------------------------------- //

static void PostRender_3rdPerson(CameraMgr *pMgr, HCAMERA hCamera)
{
	// Get the camera
	CAMERAINSTANCE *pCamera = pMgr->GetCamera(hCamera);

	if(pCamera && !pCamera->bActive)
		return;

	// If our 3rd person SFX is done... set us back to the 1st person camera
	if(!CameraMgrFX_Active(hCamera, CAMERAMGRFX_ID_3RDPERSON))
	{
		pMgr->SetCameraActive(g_pGameClientShell->GetPlayerCamera());
	}
}

// --------------------------------------------------------------------------- //

static LTBOOL PreRender_Cinematic(CameraMgr *pMgr, HCAMERA hCamera)
{
	// Get the camera
	CAMERAINSTANCE *pCamera = pMgr->GetCamera(hCamera);
	if(!pCamera) return LTFALSE;

	// Get access to the camera SFX list
	CSpecialFXList *pCameraList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CAMERA_ID);
	CCameraFX *pFX = LTNULL;
	LTBOOL bFoundActive = LTFALSE;
	HOBJECT hObj = LTNULL;
	uint32 dwFlags = 0;

	if(pCameraList)
	{
		// Go through the cameras and see if any of them are active
		for( CSpecialFXList::Iterator iter = pCameraList->Begin();
			 iter != pCameraList->End(); ++iter)
		{
			pFX = (CCameraFX*)iter->pSFX;
			if(!pFX) continue;

			// Get a handle to the server object
			hObj = pFX->GetServerObj();

			if(hObj)
			{
				// Get the user flags to determine the active state
				g_pLTClient->GetObjectUserFlags(hObj, &dwFlags);

				if(dwFlags & USRFLG_CAMERA_LIVE)
				{
					// Set this so we know that we found a good camera to use
					bFoundActive = LTTRUE;
					break;
				}
			}
		}
	}

	// If we didn't find an active server camera object, make sure we get set back
	// to first person mode
	if(!bFoundActive)
	{
		if(pCamera->bActive)
		{
			pMgr->SetCameraActive(g_pGameClientShell->GetPlayerCamera());

			if(CameraMgrFX_Active(hCamera, CAMERAMGRFX_ID_WIDESCREEN))
				CameraMgrFX_WidescreenToggle(hCamera);
			if(CameraMgrFX_Active(g_pGameClientShell->GetPlayerCamera(), CAMERAMGRFX_ID_WIDESCREEN))
				CameraMgrFX_WidescreenToggle(g_pGameClientShell->GetPlayerCamera());

			// Make sure to put vision modes back the way they were
			ViewModeMGR* pModeMgr = g_pGameClientShell->GetVisionModeMgr();
			if(pModeMgr)
			{
//				if(_stricmp(pModeMgr->GetLastModeName().c_str(), "Normal") != 0)
//					pModeMgr->SetMenuMode(LTFALSE);
				pModeMgr->SetCineMode(LTFALSE);
			}

			g_pGameClientShell->GetPlayerMovement()->AllowMovementReset();
			g_pGameClientShell->GetPlayerMovement()->AllowRotationReset();

			g_pLTClient->SetListener(LTTRUE, LTNULL, LTNULL);
		}

		return LTFALSE;
	}

	// Otherwise, if we did find an active server camera object... make sure
	// everything on the client is setup to use it properly
	if(hObj && pFX)
	{
		// Make sure the cinematic camera is active
		if(!pMgr->GetCameraActive(hCamera))
			pMgr->SetCameraActive(hCamera);

		// Make sure we're following the appropriate server object
		pMgr->SetCameraObj(hCamera, hObj);
		pMgr->SetCameraFOV(hCamera, (LTFLOAT)pFX->GetXFOV(), (LTFLOAT)pFX->GetYFOV());
		pMgr->Update();

		// See if we need to be in widescreen mode
		if(pFX->IsWidescreen() && !CameraMgrFX_Active(hCamera, CAMERAMGRFX_ID_WIDESCREEN))
			CameraMgrFX_WidescreenToggle(hCamera);
		if(pFX->IsWidescreen() && !CameraMgrFX_Active(g_pGameClientShell->GetPlayerCamera(), CAMERAMGRFX_ID_WIDESCREEN))
			CameraMgrFX_WidescreenToggle(g_pGameClientShell->GetPlayerCamera());

		// Make sure vision modes are turned off
		ViewModeMGR* pModeMgr = g_pGameClientShell->GetVisionModeMgr();
		if(pModeMgr)
		{
//			if(_stricmp(pModeMgr->GetCurrentModeName().c_str(), "Normal"))
				pModeMgr->SetCineMode(LTTRUE);
		}


		// See if we're supposed to stop the player movement
		if(!pFX->AllowPlayerMovement())
		{
			if(g_pGameClientShell->GetPlayerMovement()->CanMove())
				g_pGameClientShell->GetPlayerMovement()->AllowMovement(LTFALSE);
			if(g_pGameClientShell->GetPlayerMovement()->CanRotate())
				g_pGameClientShell->GetPlayerMovement()->AllowRotation(LTFALSE);
		}

		// Make sure the listener position is setup
		if(pFX->IsListener())
		{
			LTVector vPos;
			LTRotation rRot;
			g_pLTClient->GetObjectPos(hObj, &vPos);
			g_pLTClient->GetObjectRotation(hObj, &rRot);
			g_pLTClient->SetListener(LTFALSE, &vPos, &rRot);
		}
	}

	// ---------------------------------------------------------------------- //
	// Handle all the normal display updates

	// Get access to the weapon model
	CWeaponModel *pWeaponModel = g_pGameClientShell->GetWeaponModel();

	// Make sure the weapon model is invisible
	if(pWeaponModel && pWeaponModel->IsVisible())
		pWeaponModel->SetVisible(LTFALSE);

	// Get the client object (server player object)
	HOBJECT hPlayerObj = g_pLTClient->GetClientObject();

	uint32 nFlags;
	g_pLTClient->Common()->GetObjectFlags(hPlayerObj, OFT_Flags, nFlags);

	// Make sure the client object is invisible
	if(g_pGameClientShell->GetPlayerState() == PS_ALIVE)
	{
		g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, nFlags | FLAG_VISIBLE);
		g_pGameClientShell->HideShowAttachments(hPlayerObj);
	}
	else
	{
		g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, nFlags & ~FLAG_VISIBLE);
		g_pGameClientShell->HideShowAttachments(hPlayerObj);
	}

	// Get access to the interface manager and make sure the cursor is not displayed
	CInterfaceMgr *pInterface = g_pGameClientShell->GetInterfaceMgr();
	pInterface->EnableCrosshair(LTFALSE);
	pInterface->SetDrawInterface(LTFALSE);
	pInterface->DrawPlayerStats(LTFALSE);
	pInterface->DrawCloseCaption(LTTRUE);

	// Return LTTRUE so the camera gets rendered
	return LTTRUE;
}

// --------------------------------------------------------------------------- //

static void PostRender_Cinematic(CameraMgr *pMgr, HCAMERA hCamera)
{

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsRendererEqual()
//
//	PURPOSE:	Returns TRUE if two renderers are the same
//
// ----------------------------------------------------------------------- //

static DBOOL IsRendererEqual(RMode *pRenderer1, RMode *pRenderer2)
{
	_ASSERT(pRenderer1);
	_ASSERT(pRenderer2);

	if (_mbsicmp((const unsigned char*)pRenderer1->m_RenderDLL, (const unsigned char*)pRenderer2->m_RenderDLL) != 0)
	{
		return DFALSE;
	}

	if (_mbsicmp((const unsigned char*)pRenderer1->m_InternalName, (const unsigned char*)pRenderer2->m_InternalName) != 0)
	{
		return DFALSE;
	}

//	if (_mbsicmp((const unsigned char*)pRenderer1->m_Description, (const unsigned char*)pRenderer2->m_Description) != 0)
//	{
//		return DFALSE;
//	}

	if (pRenderer1->m_Width != pRenderer2->m_Width)
	{
		return DFALSE;
	}

	if (pRenderer1->m_Height != pRenderer2->m_Height)
	{
		return DFALSE;
	}

	if (pRenderer1->m_BitDepth != pRenderer2->m_BitDepth)
	{
		return DFALSE;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CGameClientShell()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CGameClientShell::CGameClientShell()
{
	//AfxSetAllocStop(37803);

	g_pGameClientShell			= this;

	g_vWorldWindVel.Init();

	m_hPlayerCamera				= 0;
	m_hPlayerCameraHeight		= LTNULL;
	m_hPlayerCameraAiming		= LTNULL;

	m_h3rdPersonCamera			= 0;
	m_h3rdPersonCameraHeight	= LTNULL;
	m_h3rdPersonCameraAiming	= LTNULL;

	m_vVoidColor.Init(0.0f, 0.0f, 0.0f);

	m_bMainWindowMinimized		= LTFALSE;

	m_bGamePaused				= LTFALSE;
	m_resSoundInit				= LT_ERROR;

	memset(m_strCurrentWorldName, 0, 256);

	m_bInWorld					= LTFALSE;
	m_bStartedLevel				= LTFALSE;

	m_bPanSky					= LTFALSE;
	m_fPanSkyOffsetX			= 1.0f;
	m_fPanSkyOffsetZ			= 1.0f;
	m_fPanSkyScaleX				= 1.0f;
	m_fPanSkyScaleZ				= 1.0f;
	m_fCurSkyXOffset			= 0.0f;
	m_fCurSkyZOffset			= 0.0f;

	m_bFirstUpdate				= LTFALSE;
	m_bRestoringGame			= LTFALSE;
	m_bQuickSave				= LTFALSE;

	m_fEarliestRespawnTime		= 0.0f;
	m_fForceRespawnTime			= 0.0f;

	m_bMainWindowFocus			= LTFALSE;

	m_bPlayerPosSet				= LTFALSE;
	m_bEMPEffect				= LTFALSE;
	m_bStunEffect				= LTFALSE;
	m_bFaceHugEffect			= LTFALSE;

	m_fNextSoundReverbTime		= 0.0f;
	m_bUseReverb				= LTFALSE;
	m_fReverbLevel				= 0.0f;
	m_vLastReverbPos.Init();

	m_pStoryElement				= LTNULL;

	// NOLF
	m_hInterfaceCamera			= LTNULL;
	m_pMPMgr					= LTNULL;

	m_nLastSaveLoadVersion		= 0;

	m_fLastSpringJumpTime		= 0.0f;
	m_fLastPounceTime			= 0.0f;


	m_hFacehugger				= LTNULL;
	m_fFacehugTime				= 0.0f;
	m_bFirstLand				= LTTRUE;

	m_fMusicVolume				= 100;
	m_fSFXVolume				= 100;

	m_fInitialServerTime		= 0;
	m_fInitialLocalTime			= 0;
	m_nSpeedCheatCounter		= 0;


	// FOX CODES TO PREVENT OPTIMIZING OUT THE VARIABLES
	iIdentCode0 = iIdentCode0;
	szCopywriteString[0] = szCopywriteString[0];
	iIdentCode1 = iIdentCode1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::~CGameClientShell()
//
//	PURPOSE:	Destruction
//
// ----------------------------------------------------------------------- //
			
CGameClientShell::~CGameClientShell()
{
	DeleteInterfaceCamera();

	if (g_prcClip)
	{
		delete g_prcClip;
		g_prcClip = DNULL;
	}

	if(m_pStoryElement)
	{
		delete m_pStoryElement;
		m_pStoryElement = LTNULL;
	}

	if(m_pMPMgr)
	{
		delete m_pMPMgr;
		m_pMPMgr = LTNULL;
		g_pMPMgr = LTNULL;
	}

	g_pGameClientShell = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::InitSound
//
//	PURPOSE:	Initialize the sounds
//
// ----------------------------------------------------------------------- //

void CGameClientShell::InitSound()
{
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

	Sound3DProvider *pSound3DProviderList, *pSound3DProvider;
	InitSoundInfo soundInfo;
	ReverbProperties reverbProperties;
	HCONSOLEVAR hVar;
	DDWORD dwProviderID;
	char sz3dSoundProviderName[_MAX_PATH + 1];
	int nError;

	m_resSoundInit = LT_ERROR;

	DDWORD dwAdvancedOptions = m_InterfaceMgr.GetAdvancedOptions();
	if (!(dwAdvancedOptions & AO_SOUND)) return;

	INITSOUNDINFO_INIT(soundInfo);

	// Reload the sounds if there are any...

	soundInfo.m_dwFlags	= INITSOUNDINFOFLAG_RELOADSOUNDS;

	// Get the 3d sound provider id....

	hVar = g_pLTClient->GetConsoleVar("3DSoundProvider");
	if (hVar)
	{
		dwProviderID = ( DDWORD )g_pLTClient->GetVarValueFloat( hVar );
	}
	else
	{
		dwProviderID = SOUND3DPROVIDERID_NONE;
	}

	// Can also be set by provider name, in which case the id will be set to 
	// UNKNOWN...

	if ( dwProviderID == SOUND3DPROVIDERID_NONE || 
		 dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
	{
		sz3dSoundProviderName[0] = 0;
		hVar = g_pLTClient->GetConsoleVar("3DSoundProviderName");
		if ( hVar )
		{
			SAFE_STRCPY( sz3dSoundProviderName, g_pLTClient->GetVarValueString( hVar ));
			dwProviderID = SOUND3DPROVIDERID_UNKNOWN;
		}
	}

	// See if the provider exists....

	if ( dwProviderID != SOUND3DPROVIDERID_NONE )
	{
		g_pLTClient->GetSound3DProviderLists( pSound3DProviderList, LTFALSE );
		if ( !pSound3DProviderList )
		{
			m_resSoundInit = LT_NO3DSOUNDPROVIDER;
			return;
		}

		pSound3DProvider = pSound3DProviderList;
		while ( pSound3DProvider )
		{
			// If the provider is selected by name, then compare the names.
			if (  dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
			{
				if ( _mbscmp(( const unsigned char * )sz3dSoundProviderName, ( const unsigned char * )pSound3DProvider->m_szProvider ) == 0 )
					break;
			}
			// Or compare by the id's.
			else if ( pSound3DProvider->m_dwProviderID == dwProviderID )
				break;

			// Not this one, try next one.
			pSound3DProvider = pSound3DProvider->m_pNextProvider;
		}

		// Check if we found one.
		if (pSound3DProvider)
		{
			// Use this provider.
			SAFE_STRCPY( soundInfo.m_sz3DProvider, pSound3DProvider->m_szProvider);

			// Get the maximum number of 3d voices to use.
			hVar = g_pLTClient->GetConsoleVar("Max3DVoices");
			if (hVar)
			{
				soundInfo.m_nNum3DVoices = (DBYTE)g_pLTClient->GetVarValueFloat(hVar);
			}
			else
			{
				soundInfo.m_nNum3DVoices = 16;
			}
		}

		g_pLTClient->ReleaseSound3DProviderList(pSound3DProviderList);
	}

	// Get the maximum number of sw voices to use.
	hVar = g_pLTClient->GetConsoleVar("maxswvoices");
	if (hVar)
	{
		soundInfo.m_nNumSWVoices = (DBYTE)g_pLTClient->GetVarValueFloat(hVar);
	}
	else
	{
		g_pLTClient->RunConsoleString("maxswvoices 32");
		soundInfo.m_nNumSWVoices = 32;
	}

	soundInfo.m_nSampleRate		= 22050;
	soundInfo.m_nBitsPerSample	= pSettings->Sound16Bit() ? 16 : 8;
	soundInfo.m_nVolume			= (unsigned short)pSettings->SoundVolume();
	
	if ( !pSettings->Sound16Bit( ) )
	{
		soundInfo.m_dwFlags |= INITSOUNDINFOFLAG_CONVERT16TO8;
	}

	soundInfo.m_fDistanceFactor = 1.0f / 64.0f;

	// Go initialize the sounds...

	m_bUseReverb = LTFALSE;
	if ((m_resSoundInit = g_pLTClient->InitSound(&soundInfo)) == LT_OK)
	{
		if (soundInfo.m_dwResults & INITSOUNDINFORESULTS_REVERB)
		{
			m_bUseReverb = LTTRUE;
		}

		hVar = g_pLTClient->GetConsoleVar("ReverbLevel");
		if (hVar)
		{
			m_fReverbLevel = g_pLTClient->GetVarValueFloat(hVar);
		}
		else
		{
			m_fReverbLevel = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_DEFAULTLEVEL);
		}

		reverbProperties.m_dwParams = REVERBPARAM_VOLUME;
		reverbProperties.m_fVolume = m_fReverbLevel;
		g_pLTClient->SetReverbProperties(&reverbProperties);
	}
	else
	{
		if (m_resSoundInit == LT_NO3DSOUNDPROVIDER)
		{
			nError = IDS_INVALID3DSOUNDPROVIDER;
		}
		else
		{
			nError = IDS_SOUNDNOTINITED;
		}

		// DoMessageBox( nError, TH_ALIGN_CENTER );
	}


	// Call this so the sound will get updated to the game settings...
	GetGameSettings()->ImplementSoundVolume();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PlayTauntSound
//
//	PURPOSE:	Plays a taunt sound from a character object
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PlayTauntSound(uint32 nClientID)
{
	// Find the list of FX related to characters
	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);

	if(!pList || pList->IsEmpty())
		return;

	int nNumCharFX = pList->GetNumItems();

	// Now go through all the character FX and find our test object...
	for( CSpecialFXList::Iterator iter = pList->Begin();
		 iter != pList->End(); ++iter)
	{
		CCharacterFX *pFX = (CCharacterFX*)(iter->pSFX);

		if(pFX && (pFX->m_cs.nClientID == nClientID))
		{
			if(IsMultiplayerGame())
				g_pClientSoundMgr->PlayCharSndFromObject(pFX->m_cs.nCharacterSet, pFX->GetServerObj(), "Taunt");
			else
				g_pClientSoundMgr->PlayCharSndFromObject(pFX->m_cs.nCharacterSet, pFX->GetServerObj(), "TauntSP");

			return;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PreChangeGameState
//
//	PURPOSE:	Handle pre setting of game state
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::PreChangeGameState(GameState eNewState)
{ 
	switch (eNewState)
	{
		case GS_LOADINGLEVEL:
		case GS_FOLDER :
		{
			// Change the vision modes
			m_ModeMGR.SetMenuMode(LTTRUE);
			m_Music.Pause(LTDMEnactDefault);
		}
		break;

		case GS_PLAYING:
		{
			m_ModeMGR.SetMenuMode(LTFALSE);
			m_Music.UnPause();
		}

		default : break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PostChangeGameState
//
//	PURPOSE:	Handle post setting of game state
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::PostChangeGameState(GameState eOldState)
{ 
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CSPrint
//
//	PURPOSE:	Displays a line of text on the client
//
// ----------------------------------------------------------------------- //

void CGameClientShell::CSPrint(char* msg, ...)
{
	// parse the message

	char pMsg[256];
	va_list marker;
	va_start (marker, msg);
	int nSuccess = vsprintf (pMsg, msg, marker);
	va_end (marker);

	if (nSuccess < 0) return;
	
	// now display the message

	g_pMessageMgr->AddMessage(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEngineInitialized
//
//	PURPOSE:	Called after engine is fully initialized
//				Handle object initialization here
//
// ----------------------------------------------------------------------- //

DDWORD CGameClientShell::OnEngineInitialized(RMode *pMode, DGUID *pAppGuid)
{
	//CWinUtil::DebugBreak();

	PERFINFO_START( );

	*pAppGuid = AVP2GUID;

	char strTimeDiff[64];
	float fStartTime = CWinUtil::GetTime();

	char errorBuf[256];

	if (!g_hMainWnd)
	{
		HookWindow();
	}

	PERFINFO_REPORT( );

	// Initialize all the global bute mgrs...

	if (!m_GlobalMgr.Init(g_pLTClient))
	{
		return LT_ERROR;
	}

	PERFINFO_REPORT( );

    g_vtInterfceFOVX.Init(g_pLTClient, "FovXInterface", NULL, 90.0f);
    g_vtInterfceFOVY.Init(g_pLTClient, "FovYInterface", NULL, 75.0f);

	g_vtGlowRatio.Init(g_pLTClient, "GlowRatio", NULL, 1.0f);
	g_vtInfoString.Init(g_pLTClient, "InfoString", NULL, 0.0f);
	g_vtInfoString.Init(g_pLTClient, "MPInfoString", NULL, 1.0f);

	g_vtActivateOverride.Init(g_pLTClient, "ActivateOverride", " ", 0.0f);

	g_vtDisableScreenFlash.Init(g_pLTClient, "NoScreenFlash", NULL, 0.0f);

	g_vtCamDamage.Init(g_pLTClient, "CamDamage", NULL, 1.0f);
	g_vtCamDamagePitch.Init(g_pLTClient, "CamDamagePitch", NULL, 1.0f);
	g_vtCamDamageRoll.Init(g_pLTClient, "CamDamageRoll", NULL, 1.0f);
	g_vtCamDamageTime1.Init(g_pLTClient, "CamDamageTime1", NULL, 0.1f);
	g_vtCamDamageTime2.Init(g_pLTClient, "CamDamageTime2", NULL, 0.25f);
	g_vtCamDamageVal.Init(g_pLTClient, "CamDamageVal", NULL, 5.0f);
	g_vtCamDamagePitchMin.Init(g_pLTClient, "CamDamagePitchMin", NULL, 0.7f);
	g_vtCamDamageRollMin.Init(g_pLTClient, "CamDamageRollMin", NULL, 0.7f);

	g_varDoScreenFade.Init(g_pLTClient, "ScreenFade", NULL, 1.0f);
	g_varScreenFadeTime.Init(g_pLTClient, "ScreenFadeTime", NULL, 1.5f);

	// Default these to use values specified in the weapon...(just here for 
	// tweaking values...)
	g_vtFireJitterDecayTime.Init(g_pLTClient, "FireJitterDecayTime", NULL, -1.0f);
	g_vtFireJitterMaxPitchDelta.Init(g_pLTClient, "FireJitterMaxPitchDelta", NULL, -1.0f);

	// This variable controls whether this client should locally update it's client obj's
	// position and rotation so that no lag is visible in third person views
	g_vtLocalClientObjUpdate.Init(g_pLTClient, "LocalClientObjUpdate", NULL, 1.0f);

	g_vtHuntingWarble.Init(g_pLTClient, "HuntingWarble", NULL, 0.0f);
	g_vtOrientOverlay.Init(g_pLTClient, "OrientOverlay", NULL, 1.0f);
	g_vtOrientSpeed.Init(g_pLTClient, "OrientSpeed", NULL, 3.0f);

	g_vtEnableHints.Init(g_pLTClient, "EnableHints", NULL, 1.0f);

	g_vtFogPerformanceScale.Init(g_pLTClient, "FogPerformanceScale", NULL, 100.0f);


	HCONSOLEVAR hIsSet = g_pLTClient->GetConsoleVar("UpdateRateInitted");
	if(hIsSet && g_pLTClient->GetVarValueFloat(hIsSet) == 1.0f)
	{
		// Ok it's already initialized.
	}
	else
	{
		// Initialize the update rate.
		g_pLTClient->RunConsoleString("+UpdateRateInitted 1");
		g_pLTClient->RunConsoleString("+UpdateRate 6");
	}


	m_CheatMgr.Init();

	//init the head bob mgr
	m_HeadBobMgr.Init();

	g_pLTClient->RegisterConsoleProgram("Cheat", CheatFn);
	g_pLTClient->RegisterConsoleProgram("LeakFile", LeakFileFn);
	g_pLTClient->RegisterConsoleProgram("Connect", ConnectFn);
	g_pLTClient->RegisterConsoleProgram("FragSelf", FragSelfFn);
	g_pLTClient->RegisterConsoleProgram("ReloadWeapons", ReloadWeaponAttributesFn);
	g_pLTClient->RegisterConsoleProgram("ReloadSurfaces", ReloadSurfacesAttributesFn);
	g_pLTClient->RegisterConsoleProgram("ReloadFX", ReloadFXAttributesFn);
	g_pLTClient->RegisterConsoleProgram("Record", RecordFn);
	g_pLTClient->RegisterConsoleProgram("PlayDemo", PlayDemoFn);
	g_pLTClient->RegisterConsoleProgram("InitSound", InitSoundFn);
    g_pLTClient->RegisterConsoleProgram("Music", MusicFn);
	g_pLTClient->RegisterConsoleProgram("ExitLevel", ExitLevelFn);
	g_pLTClient->RegisterConsoleProgram("Trigger", TriggerFn);
	g_pClientDE->RegisterConsoleProgram("List", ListFn);
	g_pClientDE->RegisterConsoleProgram("GameTime", GameTimeFn);

	g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, this);

	// Make sure the save directory exists...

	if (!CWinUtil::DirExist("Save"))
	{
		CWinUtil::CreateDir("Save");
	}


	// Set the default chrome texture
	g_pLTClient->RunConsoleString("EnvMap WorldTextures\\Chrome.dtx");


	// Add to NumRuns count...
	
	float nNumRuns = 0.0f;
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("NumRuns");
	if (hVar)
	{
		nNumRuns = g_pLTClient->GetVarValueFloat(hVar);
	}
	nNumRuns++;
	
	char strConsole[64];
	sprintf (strConsole, "+NumRuns %f", nNumRuns);
	g_pLTClient->RunConsoleString(strConsole);


	// 09/04/01 - MarkS
	// Moved this here, rather than InterfaceMgr.  The performance settings need to be
	// read in before the renderer gets started.
	g_pPerformanceMgr = new CPerformanceMgr;
	if( !g_pPerformanceMgr )
		return DFALSE;

	if (!g_pPerformanceMgr->Init()) 
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CPerformanceMgr::Init():  Could not initialize PerformanceMgr!");
		g_pPerformanceMgr->Term();
		delete g_pPerformanceMgr;
		return DFALSE;
	}


	// Initialize the renderer
    LTRESULT hResult = g_pLTClient->SetRenderMode(pMode);
	if (hResult != LT_OK)
	{
        g_pLTClient->DebugOut("%s Error: Couldn't set render mode!\n", GAME_NAME);

		RMode rMode;

		// If an error occurred, try 640x480x16...

		rMode.m_Width		= 640;
		rMode.m_Height		= 480;
		rMode.m_BitDepth	= 16;
		rMode.m_bHardware	= pMode->m_bHardware;

		sprintf(rMode.m_RenderDLL, "%s", pMode->m_RenderDLL);
		sprintf(rMode.m_InternalName, "%s", pMode->m_InternalName);
		sprintf(rMode.m_Description, "%s", pMode->m_Description);

        g_pLTClient->DebugOut("Setting render mode to 640x480x16...\n");

        if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
		{
			// Okay, that didn't work, looks like we're stuck with software...

            rMode.m_bHardware = LTFALSE;

			sprintf(rMode.m_RenderDLL, "d3d.ren");
			sprintf(rMode.m_InternalName, "");
			sprintf(rMode.m_Description, "");

            g_pLTClient->DebugOut("Setting render mode to D3D Emulation mode...\n");

            if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
			{
                g_pLTClient->DebugOut("%s Error: Couldn't set to D3D Emulation mode.\nShutting down %s...\n", GAME_NAME, GAME_NAME);
                g_pLTClient->ShutdownWithMessage("%s Error: Couldn't set D3D Emulation mode.\nShutting down %s...\n", GAME_NAME, GAME_NAME);
				return LT_ERROR;
			}
		}
	}



	// Setup the global transparency color
	
	g_hColorTransparent = SETRGB_T(0,0,0);


	// Interface stuff...

	if (!m_InterfaceMgr.Init())
	{
		sprintf(errorBuf, "ERROR in CGameClientShell::OnEngineInitialized()\n\nCouldn't initialize InterfaceMgr! You may have corrupt or missing resources. Common files to check are 'autoexec.cfg', 'default.prf', and necessary art resources.");
		g_pLTClient->ShutdownWithMessage(errorBuf);
		return LT_ERROR;
	}

	PERFINFO_REPORT( );

//	if (!m_PlayerSummary.Init(g_pLTClient))
//	{
//		sprintf(errorBuf, "ERROR in CGameClientShell::OnEngineInitialized()\n\nCouldn't initialize PlayerSummaryMgr!");
//		g_pLTClient->ShutdownWithMessage(errorBuf);
//		return LT_ERROR;
//	}

	//init this here because it needs to access layout mgr through interface mgr
	m_weaponModel.Init();

	PERFINFO_REPORT( );

	// Initialize the camera mgr
	m_CameraMgr.Init();

	PERFINFO_REPORT( );


	// Create the interface camera
	CreateInterfaceCamera();


	// Create the main cameras using the camera mgr
	CAMERACREATESTRUCT camCS;

	// Create the cinematic camera
	camCS.fFOVX = 75.0f;
	camCS.fFOVY = 60.0f;
	camCS.bFullScreen = LTTRUE;
	camCS.dwFlags = CAMERA_FLAG_FOLLOW_OBJ_POS | CAMERA_FLAG_FOLLOW_OBJ_ROT;
	camCS.fpPreRender = PreRender_Cinematic;
	camCS.fpPostRender = PostRender_Cinematic;
	m_hCinematicCamera = m_CameraMgr.NewCamera(camCS);

	// Create the 1st person player camera
//	strncpy(camCS.szObjSocket, "Camera", CAMERAMGR_MAX_SOCKET_NAME);
	camCS.dwFlags |= CAMERA_FLAG_FOLLOW_OBJ_SOCKET_POS;
	camCS.fpPreRender = PreRender_1stPerson;
	camCS.fpPostRender = PostRender_1stPerson;
	m_hPlayerCamera = m_CameraMgr.NewCamera(camCS);

	// Create the 3rd person player camera
	camCS.fpPreRender = PreRender_3rdPerson;
	camCS.fpPostRender = PostRender_3rdPerson;
	m_h3rdPersonCamera = m_CameraMgr.NewCamera(camCS);

	m_CameraMgr.SetCameraActive(m_hPlayerCamera);

	// Create the camera aiming special effect
	CAMERAEFFECTCREATESTRUCT camECS;
	camECS.nID = CAMERAMGRFX_ID_AIMING;
	camECS.dwFlags = CAMERA_FX_USE_ROT_V;
	m_hPlayerCameraAiming = m_CameraMgr.NewCameraEffect(m_hPlayerCamera, camECS);
	m_h3rdPersonCameraAiming = m_CameraMgr.NewCameraEffect(m_h3rdPersonCamera, camECS);

	// Create the camera height special effect
	camECS.nID = CAMERAMGRFX_ID_POSOFFSET;
	camECS.dwFlags = CAMERA_FX_USE_POS;
	m_hPlayerCameraHeight = m_CameraMgr.NewCameraEffect(m_hPlayerCamera, camECS);
	m_h3rdPersonCameraHeight = m_CameraMgr.NewCameraEffect(m_h3rdPersonCamera, camECS);

	// Init the console variables for the standard FX
	CameraMgrFX_InitConsoleVariables();

	// Added in the volume brush FX for each camera
	CameraMgrFX_VolumeBrushCreate(m_hCinematicCamera, 1);
	CameraMgrFX_VolumeBrushCreate(m_hPlayerCamera, 1);
	CameraMgrFX_VolumeBrushCreate(m_h3rdPersonCamera, 1);

	PERFINFO_REPORT( );

	// Add in the global screen wash camera effect
	CameraMgrFX_ScreenWashCreate(m_hPlayerCamera);

	// Add in the global head bob camera effect
	CameraMgrFX_HeadBobCreate(m_hPlayerCamera);

	// Initialize the physics states...

	VEC_SET(g_normalPhysicsState.m_vGravityAccel, 0.0f, -1000.0f, 0.0f);
	g_normalPhysicsState.m_fVelocityDampen = 0.5f;

	VEC_SET(g_waterPhysicsState.m_vGravityAccel, 0.0f, -500.0f, 0.0f);
	g_waterPhysicsState.m_fVelocityDampen = 0.25f;


	DDWORD dwAdvancedOptions = m_InterfaceMgr.GetAdvancedOptions();


	// Setup the music stuff...

	if (!m_Music.IsInitialized( ) && (dwAdvancedOptions & AO_MUSIC))
	{
        m_Music.Init(g_pLTClient);
	}

	PERFINFO_REPORT( );

	// Init the special fx mgr...

	if (!m_sfxMgr.Init(g_pLTClient)) 
	{
		g_pLTClient->ShutdownWithMessage("Could not initialize SFXMgr!");
		return LT_ERROR;
	}

	PERFINFO_REPORT( );

	// Okay, start the game...

	if (hVar = g_pLTClient->GetConsoleVar("NumConsoleLines"))
	{
		// UNCOMMENT this for final builds...Show console output
		// for debugging...
		// g_pLTClient->RunConsoleString("+NumConsoleLines 0");
	}
	
	if (hVar = g_pLTClient->GetConsoleVar("runworld"))
	{
		if (!m_InterfaceMgr.LoadCustomLevel(g_pLTClient->GetVarValueString(hVar)))
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_NOLOADLEVEL);
			g_pLTClient->ShutdownWithMessage(g_pLTClient->GetStringData(hString));
			g_pLTClient->FreeString(hString);
			return LT_ERROR;
		}
	}
	else 
	{
		// Get the possible launcher app action.  The user can use the launcher
		// to jump to specific menus.
		CString sLauncherAction;
		GetConsoleString( "launcheraction", sLauncherAction.GetBuffer( 64 ), "" );
		sLauncherAction.ReleaseBuffer( );

		// Assume no action was specified by the launcher.
		BOOL bMenuSelected = FALSE;

		// There's something specified for the action.
		if( !sLauncherAction.IsEmpty( ))
		{
			// Check if they want to host a game.
			if( sLauncherAction.CompareNoCase( "host" ) == 0 )
			{
				m_InterfaceMgr.SwitchToFolder( FOLDER_ID_HOST );
				bMenuSelected = TRUE;
			}
			// Check if they want to join a game.
			else if( sLauncherAction.CompareNoCase( "join" ) == 0 )
			{
				m_InterfaceMgr.SwitchToFolder( FOLDER_ID_JOIN );
				bMenuSelected = TRUE;
			}
		}

		// Check if no menu was selected by the launcher.
		if( !bMenuSelected )
		{
			// Cannot skip title screen for legal reasons...
			m_InterfaceMgr.ChangeState(GS_SPLASHSCREEN);
		}
	}


	PERFINFO_REPORT( );

	// Determine how long it took to initialize the game...

	sprintf(strTimeDiff, "Game initialized in %f seconds.\n", CWinUtil::GetTime() - fStartTime);
	CWinUtil::DebugOut(strTimeDiff);

	
	// Init the track manager
	g_pTrackMgr = new LithTrackMgr;
	g_pTrackMgr->Init(g_pLTClient);


	// Init the multiplayer manager
	m_pMPMgr = new MultiplayerClientMgr;
	m_pMPMgr->Init();
	g_pMPMgr = m_pMPMgr;

	// Init the frame rate manager
	m_FrameRateMgr.Init();

	m_TeleType.Init();

	PERFINFO_REPORT( );

	{ PERFINFO_START( ); }


	// Setup the violence setting
	HSTRING hViolenceStr = g_pLTClient->FormatString(IDS_LOWVIOLENCE);
	WriteConsoleInt("LowViolence", stricmp(g_pLTClient->GetStringData(hViolenceStr), "Off"));
	g_pLTClient->FreeString(hViolenceStr);


	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CreateInterfaceCamera()
//
//	PURPOSE:	Removes the interface camera...
//				
// ----------------------------------------------------------------------- //

void CGameClientShell::CreateInterfaceCamera()
{
	if(m_hInterfaceCamera)
		DeleteInterfaceCamera();

	// Grab the screen dimensions...
	uint32	dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	// Setup the create structure
	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_ObjectType = OT_CAMERA;

	// Do the object creation...
    m_hInterfaceCamera = g_pLTClient->CreateObject(&theStruct);
	_ASSERT(m_hInterfaceCamera);


	// Setup the initial parameters for it...
    g_pLTClient->SetCameraRect(m_hInterfaceCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraFOV(m_hInterfaceCamera, DEG2RAD(g_vtInterfceFOVX.GetFloat()), DEG2RAD(g_vtInterfceFOVY.GetFloat()));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DeleteInterfaceCamera()
//
//	PURPOSE:	Removes the interface camera...
//				
// ----------------------------------------------------------------------- //

void CGameClientShell::DeleteInterfaceCamera()
{
	if(m_hInterfaceCamera)
	{
		g_pLTClient->DeleteObject(m_hInterfaceCamera);
		m_hInterfaceCamera = NULL;
	}
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SaveGameRenderMode()
//
//	PURPOSE:	Initializes the in-game render mode structure
//				
//
// ----------------------------------------------------------------------- //
void CGameClientShell::SaveGameRenderMode(RMode *pMode)
{
	HCONSOLEVAR	hVar;

	hVar				= g_pLTClient->GetConsoleVar("GameScreenWidth");
	m_rGameMode.m_Width	= (DDWORD) g_pLTClient->GetVarValueFloat(hVar);
	
	hVar					= g_pLTClient->GetConsoleVar("GameScreenHeight");
	m_rGameMode.m_Height	= (DDWORD) g_pLTClient->GetVarValueFloat(hVar);

	hVar					= g_pLTClient->GetConsoleVar("GameBitDepth");
	m_rGameMode.m_BitDepth	= m_rMenuMode.m_BitDepth = (DDWORD) g_pLTClient->GetVarValueFloat(hVar);

	//set elements of the member structure
	m_rGameMode.m_bHardware	= pMode->m_bHardware;

	sprintf(m_rGameMode.m_RenderDLL, "%s", pMode->m_RenderDLL);
	sprintf(m_rGameMode.m_InternalName, "%s", pMode->m_InternalName);
	sprintf(m_rGameMode.m_Description, "%s", pMode->m_Description);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetMenuRenderMode()
//
//	PURPOSE:	Initializes render mode structure and sets render mode.
//				
//
// ----------------------------------------------------------------------- //
void CGameClientShell::SetMenuRenderMode(RMode *pMode)
{
	HCONSOLEVAR	hVar;

	hVar					= g_pLTClient->GetConsoleVar("ScalingMenus");
	LTBOOL bScalingMenus	= (LTBOOL) g_pLTClient->GetVarValueFloat(hVar);

	if(bScalingMenus)
	{
		//set menu to match last in-game res mode
		m_rMenuMode.m_Width		= m_rGameMode.m_Width;
		m_rMenuMode.m_Height	= m_rGameMode.m_Height;
	}
	else
	{
		//set elements of the member structure
		m_rMenuMode.m_Width		= 640;
		m_rMenuMode.m_Height	= 480;
	}

	m_rMenuMode.m_BitDepth	= m_rGameMode.m_BitDepth;
	m_rMenuMode.m_bHardware	= pMode->m_bHardware;

	sprintf(m_rMenuMode.m_RenderDLL, "%s", pMode->m_RenderDLL);
	sprintf(m_rMenuMode.m_InternalName, "%s", pMode->m_InternalName);
	sprintf(m_rMenuMode.m_Description, "%s", pMode->m_Description);

	if (g_pLTClient->SetRenderMode(&m_rMenuMode) != LT_OK)
	{
		// Okay, that didn't work, looks like we're stuck with software...
		m_rMenuMode.m_bHardware = LTFALSE;

		sprintf(m_rMenuMode.m_RenderDLL, "d3d.ren");
		sprintf(m_rMenuMode.m_InternalName, "");
		sprintf(m_rMenuMode.m_Description, "");

		g_pLTClient->DebugOut("Setting render mode to D3D Emulation mode...\n");

		if (g_pLTClient->SetRenderMode(&m_rMenuMode) != LT_OK)
		{
			g_pLTClient->DebugOut("%s Error: Couldn't set to D3D Emulation mode.\nShutting down %s...\n", GAME_NAME, GAME_NAME);
			g_pLTClient->ShutdownWithMessage("%s Error: Couldn't set D3D Emulation mode.\nShutting down %s...\n", GAME_NAME, GAME_NAME);
		} 
	}

	//calculate and save offsets for menus
	g_pInterfaceMgr->SetMenuOffset();
}

*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEngineTerm()
//
//	PURPOSE:	Called before the engine terminates itself
//				Handle object destruction here
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnEngineTerm()
{
	UnhookWindow();

	m_TeleType.Term();

	if(g_pTrackMgr)
	{
		g_pTrackMgr->Term();
		delete g_pTrackMgr;
		g_pTrackMgr = DNULL;
	}

	g_pCameraMgr->Term();

	m_InterfaceMgr.Term();

	m_Music.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnEvent(DDWORD dwEventID, DDWORD dwParam)
{
	switch(dwEventID)
	{
		case LTEVENT_RENDERTERM:
		{
			m_bMainWindowFocus = FALSE;
		}
		break;

		case LTEVENT_RENDERINIT:
		{
			m_bMainWindowFocus = TRUE;

			if (!g_hMainWnd)
			{
				HookWindow();
			}

			// Clip the cursor if we're NOT in a window...

			HCONSOLEVAR hVar = g_pInterface->GetConsoleVar("Windowed");
			BOOL bClip = TRUE;
			if (hVar)
			{
				float fVal = g_pInterface->GetVarValueFloat(hVar);
				if (fVal == 1.0f)
				{
					bClip = FALSE;
				}
			}

			if (bClip)
			{
				if (!g_prcClip)
				{
					g_prcClip = new RECT;
				}

				GetWindowRect(g_hMainWnd, g_prcClip);
				ClipCursor(g_prcClip);
			}
		}
		break;
	}


	m_InterfaceMgr.OnEvent(dwEventID, dwParam);
}

// ----------------------------------------------------------------------- //

DRESULT CGameClientShell::OnObjectMove(HOBJECT hObj, DBOOL bTeleport, LTVector *pPos)
{
	CHudMgr* pHudMgr = m_InterfaceMgr.GetHudMgr();

	if(pHudMgr)
		pHudMgr->OnObjectMove(hObj, bTeleport, *pPos);

	return m_PlayerMovement.OnObjectMove(hObj, bTeleport, *pPos);
}

// ----------------------------------------------------------------------- //

DRESULT CGameClientShell::OnObjectRotate(HOBJECT hObj, DBOOL bTeleport, LTRotation *pNewRot)
{
//	return LT_OK;
	return m_PlayerMovement.OnObjectRotate(hObj, bTeleport, *pNewRot);
}

// ----------------------------------------------------------------------- //

DRESULT	CGameClientShell::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag)
{
	// See if we should handle a touch notify for the player character
	if(hMain == m_PlayerMovement.GetObject())
		return m_PlayerMovement.OnTouchNotify(pInfo, forceMag);

	// Handle touch notify messages for special FX
	m_sfxMgr.OnTouchNotify(hMain, pInfo, forceMag);
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PreLoadWorld()
//
//	PURPOSE:	Called before world loads
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PreLoadWorld(char *pWorldName)
{
	if (IsMainWindowMinimized())
	{
		RestoreMainWindow();
	}

	SAFE_STRCPY(m_strCurrentWorldName, pWorldName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEnterWorld()
//
//	PURPOSE:	Handle entering world
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnEnterWorld()
{
	m_PlayerMovement.Init(g_pLTClient, DNULL);
	m_PlayerMovement.SetCharacterButes("Harris");
	m_PlayerMovement.CreateObject();

	// ----------------------------------------------------------------------- //
	// Turn our sounds off for a bit..

	// Store the old music volume
	m_fMusicVolume = m_InterfaceMgr.GetSettings()->MusicVolume();
	m_fSFXVolume = m_InterfaceMgr.GetSettings()->SFXVolume();

	if((!m_bRestoringGame && !GetGameType()->IsMultiplayer()) || !(m_InterfaceMgr.GetAdvancedOptions() & AO_MUSIC))
	{
		m_InterfaceMgr.GetSettings()->SetFloatVar("MusicVolume", 0.0f);
		m_InterfaceMgr.GetSettings()->ImplementMusicVolume();
	}

	if(!m_bRestoringGame && !GetGameType()->IsMultiplayer())
	{
		m_InterfaceMgr.GetSettings()->SetFloatVar("sfxvolume", 0.0f);
		m_InterfaceMgr.GetSettings()->ImplementSoundVolume();
	}

	// ----------------------------------------------------------------------- //

	// Make sure we dont play the silly land FX every time we spawn in!
	m_bFirstLand = LTTRUE;

	m_CameraMgr.SetCameraObj(m_hPlayerCamera, m_PlayerMovement.GetObject());
	m_CameraMgr.SetCameraObj(m_h3rdPersonCamera, m_PlayerMovement.GetObject());

	m_FlashLight.Init(m_PlayerMovement.GetObject());

	m_bFirstUpdate = LTTRUE;
	m_ePlayerState = PS_UNKNOWN;

	m_bPlayerPosSet = LTFALSE;
	m_bEMPEffect	= LTFALSE;
	m_bStunEffect	= LTFALSE;
	m_bFaceHugEffect= LTFALSE;
	m_bInWorld		= LTTRUE;

	m_InterfaceMgr.AddToClearScreenCount();

	g_pLTClient->ClearInput();
	m_HeadBobMgr.OnEnterWorld();

	m_InterfaceMgr.OnEnterWorld(m_bRestoringGame);

	SetExternalCamera(LTFALSE);

	m_PlayerMovement.SetAllClientUpdateFlags();

	m_vLastReverbPos.Init();

	// Make sure the player is able to move and rotate again
	m_PlayerMovement.AllowMovementReset();
	m_PlayerMovement.AllowRotationReset();
	SetInputState(LTTRUE);

	// Make sure the player is facing straight forward

	m_PlayerMovement.AimingReset();

	if(m_bRestoringGame)
		m_PlayerMovement.RestoreSaveData();


	//reset the net
	CameraMgrFX_Clear(m_hPlayerCamera, CAMERAMGRFX_ID_NET_TILT);


	//be sure to reset the crouch toggles
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();

	if (pSettings && !m_bRestoringGame)
	{
		pSettings->SetCrouchLock(LTFALSE);
		pSettings->SetWallWalkLock(LTFALSE);
	}

	//reset the restoring flag
	m_bRestoringGame		= LTFALSE;


	// IKCHAIN TESTING CODE!!
#ifdef _IK_CHAIN_TEST_
	g_ikChain.Init(10, 50.0f, 0.99f, 0.125f);
#endif

	if(GetGameType()->IsMultiplayer())
	{
		// reset the speed monitor
		m_fInitialServerTime	= g_pLTClient->GetGameTime();
		m_fInitialLocalTime		= g_pLTClient->GetTime();
		m_nSpeedCheatCounter	= 0;

		_ftime( &g_StartTimeB );
		_ftime( &g_EndTimeB );

		nStartTicks = GetTickCount();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnExitWorld()
{
	m_PlayerMovement.Term();

	m_bInWorld		= LTFALSE;
	m_bStartedLevel	= LTFALSE;
	
	memset(m_strCurrentWorldName, 0, 256);


	m_sfxMgr.RemoveAll();					// Remove all the sfx

	m_weaponModel.Reset();
	m_FlashLight.TurnOff();

	m_ModeMGR.LevelExit();

	m_InterfaceMgr.OnExitWorld();
	m_TeleType.Stop();

	m_Music.TermLevel();


	// IKCHAIN TESTING CODE!!
#ifdef _IK_CHAIN_TEST_
	g_ikChain.Term();
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PreUpdate()
//
//	PURPOSE:	Handle client pre-updates
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PreUpdate()
{
	// Conditions in which we don't want to clear the screen

	if ((m_ePlayerState == PS_UNKNOWN && m_bInWorld))
	{
		return;
	}
	
	m_InterfaceMgr.PreUpdate();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CGameClientShell::Update()
{
	// Update the frame rate manager first
	m_FrameRateMgr.Update();


	g_pLTClient->RunConsoleString("LightMap 1");


	// Set up the time for this frame...
	m_fFrameTime = g_pLTClient->GetFrameTime();
	if (m_fFrameTime > MAX_FRAME_DELTA)
	{
		m_fFrameTime = MAX_FRAME_DELTA;
	}


	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (pSettings)
	{
		// Update the mouse sensitivity if the variable changed
		LTFLOAT fNewSens = pSettings->GetFloatVar("MouseSensitivity");

		if(s_fLastMouseSens != fNewSens)
		{
			s_fLastMouseSens = fNewSens;
			SetMouseSensitivityScale();
		}
	}


	// Update client-side physics structs...
	SetPhysicsStateTimeStep(&g_normalPhysicsState, m_fFrameTime);
	SetPhysicsStateTimeStep(&g_waterPhysicsState, m_fFrameTime);


	// Update the track manager
	g_pTrackMgr->Update();

	if(g_pTrackMgr->Editing())
		SetExternalCamera(LTTRUE);
//	else
//		SetExternalCamera(LTFALSE);

	
	// Update the interface (don't do anything if the interface mgr
	// handles the update...)

	s_bRenderCamera = LTTRUE;

	if (m_InterfaceMgr.Update())
	{
		if (!(IsPlayerInWorld() && g_pGameClientShell->GetGameType()->IsMultiplayer()))
			return;

		s_bRenderCamera = LTFALSE;
	}


	if (m_ePlayerState == PS_DEAD)
	{
		// Make sure the chooser is closed...
		CWeaponChooser*	pChooser = m_InterfaceMgr.GetWeaponChooser();
		if(pChooser && pChooser->IsOpen())
			pChooser->Close();

		// See about respawning...
		if (g_pLTClient->GetTime() >= m_fForceRespawnTime)
		{
			if (g_pGameClientShell->GetGameType()->IsSinglePlayer())
			{
				m_InterfaceMgr.MissionFailed(IDS_SP_YOUWEREKILLED);
			}
			else
			{
				HandleRespawn();
			}
		}
	}
	

	// At this point we only want to proceed if the player is in the world...

	if (IsPlayerInWorld() && m_InterfaceMgr.GetGameState() != GS_UNDEFINED)
	{
		UpdatePlaying();

		if(m_GameType != SP_STORY)
			UpdateSpeedMonitor();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlaying()
//
//	PURPOSE:	Handle updating playing (normal) game state
//
// ----------------------------------------------------------------------- //
//static void * temp;
static void* (temp) = (void*)&QueryPerformanceCounter;


void CGameClientShell::UpdatePlaying()
{
	if(!m_bInWorld)
		return;


	// Handle first update...

	if (m_bFirstUpdate)
	{
		FirstUpdate();
	}

	if(GetGameType()->IsMultiplayer())
	{
		// The final performance counter must be before the initial
		// performance counter!
		uint32 nEndTicks = GetTickCount();
		_ftime(&g_EndTimeB);

		const uint32 nTimeDelta = (g_EndTimeB.time - g_StartTimeB.time)*1000 + (g_EndTimeB.millitm - g_StartTimeB.millitm);

		if( nTimeDelta > 500 )
		{
			uint32 nDelta = nEndTicks - nStartTicks;

			// This is the time in milliseconds.
			if( nDelta*5 > nTimeDelta*6 )
			{
				// You hAxOr!

				// send the Message to the server
				HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_SPEED_CHEAT);
				g_pLTClient->EndMessage(hMsg);
			}

			// Reset the timers, the initial _ftime call must be before
			// the performance counter call!
			_ftime( &g_StartTimeB );
			nStartTicks = GetTickCount();
		}
	}

	// update any effects
	UpdateEMPEffect();
	UpdateStunEffect();
	UpdateFaceHugEffect();
	UpdateFacehugged();

	UpdateOrientationIndicator();

	if(!UpdateInfoString())
		m_pMPMgr->SetInfoString("\0");


	// update wonky vision if needed
	if(m_bEMPEffect || m_bStunEffect)
	{
		if(!CameraMgrFX_Active(m_hPlayerCamera, CAMERAMGRFX_ID_WONKY))
			CameraMgrFX_WonkyToggle(m_hPlayerCamera, 1.0f);
	}
	else if(!m_bEMPEffect && !m_bStunEffect)
	{
		if(CameraMgrFX_Active(m_hPlayerCamera, CAMERAMGRFX_ID_WONKY))
		{
			//See that we are not already ramping down...
			CAMERAEFFECTINSTANCE*pFX = g_pCameraMgr->GetCameraEffectByID(m_hPlayerCamera, CAMERAMGRFX_ID_WONKY, LTNULL); 

			if(pFX)
			{
				CAMERAMGRFX_WONKY *pData = (CAMERAMGRFX_WONKY*)pFX->ceCS.pUserData;

				if(pData && pData->nState != CAMERAMGRFX_STATE_RAMPDOWN)
					CameraMgrFX_WonkyToggle(m_hPlayerCamera, 1.0f);
			}
		}
	}


	// Update player movement...
	m_PlayerMovement.Update();

	
	// Update reverb...
	UpdateSoundReverb();


	// Tell the player to "start" the level...
	if (!m_bStartedLevel)
	{
		m_bStartedLevel = LTTRUE;
		StartLevel();
	}


	// Update Player...
	UpdatePlayer();


	// If we're in alien hunting vision mode... use the warble camera effect
	if(VisionMode::g_pButeMgr->IsAlienHuntingVision(GetVisionModeMgr()->GetCurrentModeName()))
	{
		if(!g_pCameraMgr->GetCameraEffectByID(m_hPlayerCamera, CAMERAMGRFX_ID_WARBLE, LTNULL))
			CameraMgrFX_WarbleCreate(m_hPlayerCamera, g_vtHuntingWarble.GetFloat(), g_vtHuntingWarble.GetFloat());
	}
	else
	{
		if(g_pCameraMgr->GetCameraEffectByID(m_hPlayerCamera, CAMERAMGRFX_ID_WARBLE, LTNULL))
			CameraMgrFX_Clear(m_hPlayerCamera, CAMERAMGRFX_ID_WARBLE);
	}


	// Update sky-texture panning...
	if (m_bPanSky && !m_bGamePaused)
	{
		m_fCurSkyXOffset += m_fFrameTime * m_fPanSkyOffsetX;
		m_fCurSkyZOffset += m_fFrameTime * m_fPanSkyOffsetZ;

		g_pLTClient->SetGlobalPanInfo(GLOBALPAN_SKYSHADOW, m_fCurSkyXOffset, m_fCurSkyZOffset, m_fPanSkyScaleX, m_fPanSkyScaleZ);
	}


	// Update cheats...(if a cheat is in effect, just return)...
	if(g_pCheatMgr->UpdateCheats()) return;


	// Update the camera's position...
	// Update the player camera settings and the camera manager

	// [KLS] Added 9/23/2001 Check for clipping movement when in the
	// observing state...

	if(m_ePlayerState == PS_OBSERVING && 
	   m_PlayerMovement.GetMovementState() != CMS_CLIPPING)
	{
		if(m_hPlayerCameraAiming)
			m_hPlayerCameraAiming->ceCS.vRot.Init();

		if(m_h3rdPersonCameraHeight)
			m_h3rdPersonCameraHeight->ceCS.vPos.Init();
	}
	else
	{
		if(m_hPlayerCameraAiming)
			m_hPlayerCameraAiming->ceCS.vRot.x = m_PlayerMovement.GetCharacterVars()->m_fAimingPitch;
		if(m_h3rdPersonCameraAiming)
			m_h3rdPersonCameraAiming->ceCS.vRot.x = m_PlayerMovement.GetCharacterVars()->m_fAimingPitch;

		LTVector vUp = m_PlayerMovement.GetUp() * (m_PlayerMovement.GetObjectDims().y * m_PlayerMovement.GetCharacterButes()->m_fCameraHeightPercent);

		if(m_hPlayerCameraHeight)
			m_hPlayerCameraHeight->ceCS.vPos = vUp;
		if(m_h3rdPersonCameraHeight)
			m_h3rdPersonCameraHeight->ceCS.vPos = vUp;
	}


	// Update head-bob/head-cant
	m_HeadBobMgr.Update();


	// Update the camera
	m_CameraMgr.Update();


	// Make sure the player gets updated
	m_PlayerMovement.UpdateClientToServer();


	// Update any overlays...
	m_InterfaceMgr.UpdateOverlays();


	// Update the vision mode manager
	m_ModeMGR.Update(m_ePlayerState == PS_ALIVE);



	// IKCHAIN TESTING CODE!!
#ifdef _IK_CHAIN_TEST_
	LTVector vIKChainPos, vIKR, vIKU, vIKF;
	LTRotation rIKChainRot;

	m_CameraMgr.GetCameraPos(m_hPlayerCamera, vIKChainPos);
	m_CameraMgr.GetCameraRotation(m_hPlayerCamera, rIKChainRot, LTTRUE);

	g_pMathLT->GetRotationVectors(rIKChainRot, vIKR, vIKU, vIKF);

	g_ikChain.Update(vIKChainPos, vIKF, g_pLTClient->GetFrameTime());
#endif



	// Render the camera...
	RenderCamera();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PostUpdate()
//
//	PURPOSE:	Handle post updates - after the scene is rendered
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PostUpdate()
{
	if (m_bQuickSave)
	{
		char strFilename[128];
		sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_FILENAME);

		// if the quick save exists..  archive it..
		if(CWinUtil::FileExist(strFilename))
		{
			// rename the save...
			char strNewFilename[128];
			sprintf (strNewFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_BACKUP_FILENAME);

			// remove the old file and rename the new one...
			remove(strNewFilename);
			rename(strFilename,strNewFilename);
		}

		SaveGame(strFilename);
		m_bQuickSave = LTFALSE;
	}

	// Conditions where we don't want to flip...

	if (m_ePlayerState == PS_UNKNOWN && m_bInWorld)
	{
		return;
	}
	
	
	m_InterfaceMgr.PostUpdate();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateSoundReverb()
//
//	PURPOSE:	Update the sound reverb
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateSoundReverb( )
{
	if (!m_bUseReverb) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	float fReverbLevel;
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("ReverbLevel");
	if (hVar)
	{
		fReverbLevel = g_pLTClient->GetVarValueFloat(hVar);
	}
	else
	{
		fReverbLevel = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_DEFAULTLEVEL);
	}

	// Check if reverb was off and is still off...

	if (fReverbLevel < 0.001f && m_fReverbLevel < 0.001f) return;

	m_fReverbLevel = fReverbLevel;

	// Check if it's time yet...

	if (g_pLTClient->GetTime() < m_fNextSoundReverbTime) return;

	// Update timer...

	float fUpdatePeriod = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_UPDATEPERIOD);
	m_fNextSoundReverbTime = g_pLTClient->GetTime() + fUpdatePeriod;

	HOBJECT hFilterList[] = {hPlayerObj, m_PlayerMovement.GetObject(), DNULL};

	ClientIntersectInfo info;
	ClientIntersectQuery query;
	memset(&query, 0, sizeof(query));
	query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	query.m_FilterFn = ObjListFilterFn;
	query.m_pUserData = hFilterList;

	g_pLTClient->GetObjectPos(hPlayerObj, &query.m_From);

	// Make sure the player moved far enough to check reverb again...

	float fPlayerMoveDist = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAYERMOVEDIST);
	if (VEC_DIST(query.m_From, m_vLastReverbPos) < fPlayerMoveDist) return;

	float fReverbSegmentLen = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_INTERSECTSEGMENTLEN);
	VEC_COPY( m_vLastReverbPos, query.m_From );

	LTVector vPos[6], vSegs[6];
	VEC_SET( vSegs[0], query.m_From.x + fReverbSegmentLen, query.m_From.y, query.m_From.z );
	VEC_SET( vSegs[1], query.m_From.x - fReverbSegmentLen, query.m_From.y, query.m_From.z );
	VEC_SET( vSegs[2], query.m_From.x, query.m_From.y + fReverbSegmentLen, query.m_From.z );
	VEC_SET( vSegs[3], query.m_From.x, query.m_From.y - fReverbSegmentLen, query.m_From.z );
	VEC_SET( vSegs[4], query.m_From.x, query.m_From.y, query.m_From.z + fReverbSegmentLen );
	VEC_SET( vSegs[5], query.m_From.x, query.m_From.y, query.m_From.z - fReverbSegmentLen );

	DBOOL bOpen = LTFALSE;
	for (int i = 0; i < 6; i++)
	{
		VEC_COPY( query.m_To, vSegs[i] );

		if ( g_pLTClient->IntersectSegment( &query, &info ))
		{
			VEC_COPY( vPos[i], info.m_Point );
			if (info.m_SurfaceFlags == ST_AIR || info.m_SurfaceFlags == ST_SKY)
			{
				bOpen = LTTRUE;
			}
		}
		else
		{
			VEC_COPY( vPos[i], vSegs[i] );
			bOpen = LTTRUE;
		}
	}

	float fVolume = VEC_DIST( vPos[0], vPos[1] );
	fVolume *= VEC_DIST( vPos[2], vPos[3] );
	fVolume *= VEC_DIST( vPos[4], vPos[5] );


	ReverbProperties reverbProperties;

	// Use room types that are not completely enclosed rooms...

	if ( bOpen )
	{
		float fPipeSpace  = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPESPACE);
		float fPlainSpace = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINSPACE);
		float fArenaSpace = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENASPACE);

		if ( fVolume < fPipeSpace*fPipeSpace*fPipeSpace )
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_SEWERPIPE;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEDAMPING);
		}
		else if ( fVolume < fPlainSpace*fPlainSpace*fPlainSpace )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_PLAIN;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINDAMPING);
		}
		else if ( fVolume < fArenaSpace*fArenaSpace*fArenaSpace )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_ARENA;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENAREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENADECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENAVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENADAMPING);
		}
		else
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_MOUNTAINS;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSDAMPING);
		}
	} 
	else  // Use room types that are enclosed rooms
	{
		float fStoneRoomSpace	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMSPACE);
		float fHallwaySpace		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYSPACE);
		float fConcertHallSpace = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLSPACE);

		if ( fVolume < fStoneRoomSpace*fStoneRoomSpace*fStoneRoomSpace)
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_STONEROOM;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMDAMPING);
		}
		else if ( fVolume < fHallwaySpace*fHallwaySpace*fHallwaySpace )
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_HALLWAY;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYDAMPING);
		}
		else if ( fVolume < fConcertHallSpace*fConcertHallSpace*fConcertHallSpace )
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_CONCERTHALL;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLDAMPING);
		}
		else
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_AUDITORIUM;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMDAMPING);
		}
	}

	// Override to water if in it...

//	if (IsLiquid(m_eCurContainerCode))
//	{
//		reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_UNDERWATER;
//	}

	reverbProperties.m_dwParams = REVERBPARAM_ALL;
	g_pLTClient->SetReverbProperties(&reverbProperties);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartLevel()
//
//	PURPOSE:	Tell the player to start the level
//
// ----------------------------------------------------------------------- //

void CGameClientShell::StartLevel()
{
	if (IsMultiplayerGame()) return;

	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SINGLEPLAYER_START);
	g_pLTClient->EndMessage(hMessage);

	// Set Initial cheats...

	if (g_pCheatMgr)
	{
		DBYTE nNumCheats = g_pClientButeMgr->GetNumCheatAttributes();
		CString strCheat;

		for (DBYTE i=0; i < nNumCheats; i++)
		{
			strCheat = g_pClientButeMgr->GetCheat(i);
			if (strCheat.GetLength() > 1)
			{
				int nToks = 0;
				char *szToks[64];
				char *szBuf = strCheat.GetBuffer(0);

				szToks[nToks++] = strtok(szBuf, " ");

				while(1)
				{
					if(!(szToks[nToks] = strtok(DNULL, " ")))
						break;

					nToks++;
				}

				g_pCheatMgr->HandleCheat(nToks, szToks);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetExternalCamera()
//
//	PURPOSE:	Turn on/off external camera mode
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SetExternalCamera(DBOOL bExternal)
{
	// If we want to be in 3rd person and our 1st person camera is active, switch
	// to our 3rd person camera and toggle the 3rd person effect
	if(bExternal && m_CameraMgr.GetCameraActive(m_hPlayerCamera))
	{
		m_CameraMgr.SetCameraActive(m_h3rdPersonCamera);
		CameraMgrFX_3rdPersonToggle(m_h3rdPersonCamera);
	}
	// If we want to be in 1st person and our 3rd person camera is active, toggle
	// the 3rd person effect and the camera will switch automatically when the
	// effect is done
	else if(!bExternal && m_CameraMgr.GetCameraActive(m_h3rdPersonCamera))
	{
		CameraMgrFX_3rdPersonToggle(m_h3rdPersonCamera);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetExternalCamera()
//
//	PURPOSE:	Turn on/off external camera mode
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::HandleCameraEffect(ArgList *pArgList)
{
	// Extract the type of effect
	char* szType = pArgList->argv[1];
	if(!szType) return LTFALSE;

	// Handle a quick zoom effect (head bite)
	if(stricmp(szType, "Bite") == 0)
	{
		CameraMgrFX_QuickZoomCreate(m_hPlayerCamera, 3.0f, 0.2f, 5.0f);
		return LTTRUE;
	}
	else if(stricmp(szType, "HeadBite") == 0)
	{
		CameraMgrFX_QuickZoomCreate(m_hPlayerCamera, 5.0f, 0.25f, 45.0f);
		return LTTRUE;
	}
	else if(stricmp(szType, "HeadBiteFlash") == 0)
	{
		FlashScreen(LTVector(1.0f, 0.0f, 0.0f), LTVector(0.0f, 0.0f, 0.0f), 0.2f, 0.1f, 0.35f, 0.0f, LTTRUE);
		return LTTRUE;
	}
	else if(stricmp(szType, "CameraLunge") == 0)
	{
		LTVector vPosOffset(0.0f, -10.0f, 17.5f);
		LTVector vRotOffset(2.5f, -3.75f, 1.0f);
		LTFLOAT fTime = 0.3f;

		CameraMgrFX_QuickLungeCreate(m_hPlayerCamera, vPosOffset, vRotOffset, fTime);
		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateWeaponModel()
//
//	PURPOSE:	Update the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateWeaponModel()
{
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	LTBOOL bIsCinematic = m_CameraMgr.GetCameraActive(m_hCinematicCamera);
	LTBOOL bCanUpdate = m_pMPMgr->AllowMovementControl();

	LTRotation rRot;
	LTVector vPos;

	g_pLTClient->GetObjectPos(m_CameraMgr.GetCameraObject(m_hPlayerCamera), &vPos);
	g_pLTClient->GetObjectRotation(m_CameraMgr.GetCameraObject(m_hPlayerCamera), &rRot);


	if(bCanUpdate && (m_ePlayerState == PS_ALIVE) && !bIsCinematic)
	{
		if(m_CameraMgr.GetCameraActive(m_h3rdPersonCamera))
		{
			// Use the gun's flash orientation...
			GetAttachmentSocketTransform(hPlayerObj, "Flash", vPos, rRot);
		}

		FireType eFireType = FT_INVALID;
		DBOOL bFire = LTFALSE;

		DDWORD dwFlags = m_PlayerMovement.GetControlFlags();

		LTBOOL bFirstPerson = m_CameraMgr.GetCameraActive(m_hPlayerCamera);

		if ((dwFlags & CM_FLAG_PRIMEFIRING) || (dwFlags & CM_FLAG_ALTFIRING))
		{
			bFire	  = LTTRUE;
			eFireType = (dwFlags & CM_FLAG_ALTFIRING) ? FT_ALT_FIRE : FT_NORMAL_FIRE;
		}

		// Update the model position and state...
		WeaponState eWeaponState = m_weaponModel.UpdateWeaponModel(rRot, vPos, bFire, eFireType);

		// Do fire camera jitter...
		if (FiredWeapon(eWeaponState) && bFirstPerson)
		{
			BARREL* pBarrelData = m_weaponModel.GetBarrel();

			if(pBarrelData)
			{
				LTVector vRecoil;
				vRecoil.x = GetRandom(pBarrelData->vMinRecoilAmount.x, pBarrelData->vMaxRecoilAmount.x);
				vRecoil.y = GetRandom(pBarrelData->vMinRecoilAmount.y, pBarrelData->vMaxRecoilAmount.y);
				vRecoil.z = GetRandom(pBarrelData->vMinRecoilAmount.z, pBarrelData->vMaxRecoilAmount.z);

				CameraMgrFX_TiltCreate(m_hPlayerCamera, vRecoil, pBarrelData->vRecoilTimes);
			}
		}
	}
	else
	{
		m_weaponModel.UpdateWeaponModel(rRot, vPos, LTFALSE, FT_NORMAL_FIRE, LTTRUE);

		// be sure the model is not playing a looping sound
		m_weaponModel.KillLoopSound();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ChangeWeapon(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

	DBYTE nCommandId = g_pLTClient->ReadFromMessageByte(hMessage);
	DBYTE bAuto      = (DBOOL) g_pLTClient->ReadFromMessageByte(hMessage);
	DBYTE bPlaySelSound = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);

	DBYTE nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return;

//	DBYTE nAmmoId	= 0;//g_pWeaponMgr->GetDefaultAmmoType(nWeaponId);//pWeapon->nDefaultAmmoType;

	DBOOL bChange = LTTRUE;

	// If this is an auto weapon change and this is a multiplayer game, see 
	// if the user really wants us to switch or not (we'll always switch in
	// single player games)...

	if (bAuto && IsMultiplayerGame())
	{
		bChange = LTFALSE;

		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("AutoWeaponSwitch");
		if (hVar)
		{
			bChange = (DBOOL) g_pLTClient->GetVarValueFloat(hVar);
		}
	}

	if (bChange)
	{
		// Force a change to the approprite weapon... 

		CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
		if (pStats)
		{
			ChangeWeapon(nWeaponId, LTTRUE, bPlaySelSound);//, nAmmoId, pStats->GetAmmoCount(nAmmoId));
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ForceChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ForceChangeWeapon(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

	DBYTE	nWeaponId	= g_pLTClient->ReadFromMessageByte(hMessage);
	LTBOOL	bDeselect	= g_pLTClient->ReadFromMessageByte(hMessage);
	LTBOOL	bPlaySound	= g_pLTClient->ReadFromMessageByte(hMessage);

	if (!m_weaponModel.GetHandle())
	{
		m_weaponModel.Create(g_pLTClient, nWeaponId, bPlaySound);
	}

	// Force a change to the approprite weapon... 

	m_weaponModel.ForceChangeWeapon(nWeaponId, bDeselect);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ChangeWeapon(DBYTE nWeaponId, LTBOOL bForce, LTBOOL bPlaySelSound)
{
	DBOOL bSameWeapon = (nWeaponId == m_weaponModel.GetWeaponId());

	// Turn off zooming...

	DBYTE nOldWeaponId = m_weaponModel.GetWeaponId();
	if (!bSameWeapon && g_pWeaponMgr->IsValidWeapon(nOldWeaponId))
	{
//		HandleZoomChange(nOldWeaponId, LTTRUE);
	}
	
	if (!m_weaponModel.GetHandle() || !bSameWeapon || bForce)
	{
		m_weaponModel.Create(g_pLTClient, nWeaponId, bPlaySelSound);//, nAmmoId);
	}

	// Update the Interface...

	//unfortunate hack here, only happens once per weapon change
//	BARREL* pBarrel = m_weaponModel.GetBarrel();
//	WEAPON* pWeapon = m_weaponModel.GetWeapon();

//	if(pBarrel)
//	{
//		if(strcmp(pBarrel->szName, "SADAR_Barrel")==0)
//			m_InterfaceMgr.InitTargetingMgr(0);
//		else
//			if(pWeapon)
//				m_InterfaceMgr.InitTargetingMgr(pWeapon->nTargetingType);
//	}
//	else
//	{
//		if(pWeapon)
//			m_InterfaceMgr.InitTargetingMgr(pWeapon->nTargetingType);
//	}

	//Update the HUD
	m_InterfaceMgr.GetHudMgr()->NewWeapon(m_weaponModel.GetWeapon());

//	m_InterfaceMgr.UpdateWeaponStats(nWeaponId, nAmmoId, dwAmmo);

	// Tell the server to change weapons...

	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_WEAPON_CHANGE);
	g_pLTClient->WriteToMessageByte(hMessage, nWeaponId);
	g_pLTClient->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnCommandOn(int command)
{
	// Make sure we're not in a cinematic
	if ((command != COMMAND_ID_ACTIVATE) && (g_pCameraMgr->GetCameraActive(m_hCinematicCamera)))
		return;


	// See if the multiplayer manager has control
	if (m_pMPMgr->OnCommandOn(command))
		return;


	// Let the interface handle the command first...
	if (m_InterfaceMgr.OnCommandOn(command))
		return;


	// Handle respawning if we need to
	if ((command != COMMAND_ID_SCOREDISPLAY) && (command != COMMAND_ID_MESSAGE) && (command != COMMAND_ID_TEAMMESSAGE))
	{
		HandleRespawn();
	}


	// Make sure we're in the world...
	if (!IsPlayerInWorld() || IsPlayerDead()) 
		return;


	// Check for weapon change...
	if (g_pWeaponMgr->GetFirstWeaponCommandId() <= command && 
		command <= g_pWeaponMgr->GetLastWeaponCommandId())
	{
		if(!m_InterfaceMgr.IsAlienTargeting())
		{
			m_weaponModel.ChangeWeapon(command);

			CWeaponChooser*	pChooser = m_InterfaceMgr.GetWeaponChooser();
			if(pChooser && pChooser->IsOpen())
				pChooser->Close();
		}
		return;
	}


	// Take appropriate action

	switch (command)
	{	
		case COMMAND_ID_RUNLOCK:
		{
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
			if (pSettings)
			{
				pSettings->SetRunLock(!pSettings->RunLock());
			}
		}
		break;

		case COMMAND_ID_CROUCH_TOGGLE:
		{
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
			if (pSettings)
			{
				pSettings->SetCrouchLock(!pSettings->CrouchLock());
			}
		}
		break;

		case COMMAND_ID_WALLWALK_TOGGLE:
		{
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
			if (pSettings)
			{
				pSettings->SetWallWalkLock(!pSettings->WallWalkLock());
			}
		}
		break;

//		case COMMAND_ID_ENERGY_SIFT:
//		{
//			// Handle using the energy sift...
//			HandleEnergySift();			
//		}
//		break;
//
//		case COMMAND_ID_MEDI_COMP:
//		{
//			// Handle using the medicomp...
//			HandleMediComp();			
//		}
//		break;

		case COMMAND_ID_POUNCE_JUMP:
		{
			// Handle pounce jumping...
			HandlePounceJump();			
		}
		break;

		case COMMAND_ID_FORWARD : 
		case COMMAND_ID_BACKWARD : 
		case COMMAND_ID_STRAFE_LEFT : 
		case COMMAND_ID_STRAFE_RIGHT : 
		{
			CCharacterFX* pFX = dynamic_cast<CCharacterFX*>(m_sfxMgr.GetClientCharacterFX());
			if(pFX) pFX->ForceFootstep();

			// Plays first footstep sound in multiplayer single player is handled 
			// in CCharacterFX::PlayFootstepSound
			if (!g_pGameClientShell->GetGameType()->IsSinglePlayer())
			{
				HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
				g_pClientDE->WriteToMessageByte(hMessage, CP_FOOTSTEP_SOUND);
				g_pClientDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
			}
		}
		break;

		case COMMAND_ID_LEFT : 
		case COMMAND_ID_RIGHT : 
		{
			if(g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE))
			{
				CCharacterFX* pFX = dynamic_cast<CCharacterFX*>(m_sfxMgr.GetClientCharacterFX());
				if(pFX) pFX->ForceFootstep();

				// Plays first footstep sound in multiplayer single player is handled 
				// in CCharacterFX::PlayFootstepSound
				if (!g_pGameClientShell->GetGameType()->IsSinglePlayer())
				{
					HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
					g_pClientDE->WriteToMessageByte(hMessage, CP_FOOTSTEP_SOUND);
					g_pClientDE->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
				}
			}
		}
		break;

		case COMMAND_ID_ACTIVATE : 
		{
			if(!m_pStoryElement)
				DoActivate();
			else
				EndStoryObject();
		}
		break;

		case COMMAND_ID_RELOAD : 
		{
			if(m_weaponModel.GetState() == W_IDLE)
				m_weaponModel.ReloadClip();
		}
		break;

		case COMMAND_ID_SHOULDERLAMP : 
		{
			if( IsHuman(GetPlayerStats()->GetButeSet()) )
			{
				if(g_pGameClientShell->GetGameType()->IsSinglePlayer())
				{
					// do the silly special case check...
					if(	stricmp("Convict_Harris", g_pCharacterButeMgr->GetName(GetPlayerStats()->GetButeSet())) == 0 ||
						stricmp("NoHands", g_pCharacterButeMgr->GetName(GetPlayerStats()->GetButeSet())) == 0 )
							break;
				}
				
				// ok turn it on...
				m_FlashLight.Toggle();
			}
		}
		break;

		case COMMAND_ID_PREV_VISIONMODE : 
		{
			if( m_ModeMGR.CanChangeMode() )
			{
				//if we are human, play the mode change sound
				if( IsHuman(GetPlayerStats()->GetButeSet()) || IsExosuit(GetPlayerStats()->GetButeSet()))
				{
					if(!m_ModeMGR.PrevMode())
						g_pClientSoundMgr->PlaySoundLocal("failed_on");
					else
					{
						if(_stricmp(m_ModeMGR.GetCurrentModeName().c_str(), "Normal") == 0)
							g_pClientSoundMgr->PlaySoundLocal("vision_switch_off");
						else
							g_pClientSoundMgr->PlaySoundLocal("vision_switch_on");
					}
				}
				else
					m_ModeMGR.PrevMode();
			}
		}
		break;

		case COMMAND_ID_NEXT_VISIONMODE : 
		{
			if( m_ModeMGR.CanChangeMode() )
			{
				//if we are human, play the mode change sound
				if( IsHuman(GetPlayerStats()->GetButeSet()) || IsExosuit(GetPlayerStats()->GetButeSet()))
				{
					if(!m_ModeMGR.NextMode())
						g_pClientSoundMgr->PlaySoundLocal("failed_on");
					else
					{
						if(_stricmp(m_ModeMGR.GetCurrentModeName().c_str(), "Normal") == 0)
							g_pClientSoundMgr->PlaySoundLocal("vision_switch_off");
						else
							g_pClientSoundMgr->PlaySoundLocal("vision_switch_on");
					}
				}
				else
					m_ModeMGR.NextMode();
			}
		}
		break;

		case COMMAND_ID_ZOOM_IN : 
		{
			m_InterfaceMgr.GetPlayerStats()->ZoomIn();
		}
		break;

		case COMMAND_ID_ZOOM_OUT : 
		{
			m_InterfaceMgr.GetPlayerStats()->ZoomOut();
		}
		break;

		case COMMAND_ID_CENTERVIEW :
		{
			m_PlayerMovement.GetCharacterVars()->m_fAimingPitch = 0.0f;
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if(m_pStoryElement)
			{
				EndStoryObject();
			}
			else
			{
				// If we're a facehugger... force a pounce
				if(m_PlayerMovement.GetCharacterButes()->m_bCanPounce)
				{
					if(strstr(m_PlayerMovement.GetCharacterButes()->m_szName, "Facehugger"))
						HandlePounceJump();
				}

				HandleRespawn();
			}
		}
		break;

		case COMMAND_ID_ALT_FIRING:
		{
			if(m_pStoryElement)
			{
				EndStoryObject();
			}
			else
			{
				// If we're a facehugger... force a pounce
				if(m_PlayerMovement.GetCharacterButes()->m_bCanPounce)
				{
					if(strstr(m_PlayerMovement.GetCharacterButes()->m_szName, "Facehugger"))
						HandlePounceJump();
				}

				LTBOOL bIsFirstPerson = m_CameraMgr.GetCameraActive(m_hPlayerCamera);

				if (!IsPlayerDead() && bIsFirstPerson)
					m_weaponModel.OnAltFireButtonDown();
			}

			break;
		}

		case COMMAND_ID_DISPLAYSPECIAL :
		{
			g_pLTClient->CPrint("%d", g_dwSpecial);
		}
		break;

		case COMMAND_ID_CHASEVIEWTOGGLE :
		{
			if(m_ePlayerState == PS_ALIVE)
			{
				if(CameraMgrFX_State(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON) == CAMERAMGRFX_STATE_ACTIVE)
				{
					CameraMgrFX_3rdPersonEdit(m_h3rdPersonCamera, LTTRUE);
					m_PlayerMovement.AllowRotation(LTFALSE);
				}
				else
				{
					SetExternalCamera(m_CameraMgr.GetCameraActive(m_hPlayerCamera));
				}
			}
		}
		break;

		case COMMAND_ID_TAUNT :
		{
			if(g_pGameClientShell->GetGameType()->IsMultiplayer())
			{
				HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_MPMGR_TAUNT);
				g_pLTClient->EndMessage(hMsg);
			}
		}
		break;

		default :
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnCommandOff(int command)
{
	// Make sure we're not swapped.
	if (m_ePlayerState == PS_SWAPPED )
		return;

	// See if the multiplayer manager has control
	if (m_pMPMgr->OnCommandOff(command))
		return;

	// Let the interface handle the command first...
	if (m_InterfaceMgr.OnCommandOff(command))
		return;


	switch (command)
	{
		case COMMAND_ID_JUMP:
		{
			m_PlayerMovement.AllowJump(LTTRUE);
		}
		break;

		case COMMAND_ID_FLARE	: 
		{
			if( IsHuman(GetPlayerStats()->GetButeSet()) )
			{
				if(GetPlayerStats()->GetMarineFlares())
				{
					SendFlareTossMessage(); 
					g_pClientSoundMgr->PlaySoundLocal("flaretoss");
				}
				else
				{
					//no flares, play the fail sound
					g_pClientSoundMgr->PlaySoundLocal("emptyflare");
				}
			}
			break;
		}

		case COMMAND_ID_FIRING:
		{
			LTBOOL bIsFirstPerson = m_CameraMgr.GetCameraActive(m_hPlayerCamera);

			if (!IsPlayerDead() && bIsFirstPerson)
				m_weaponModel.OnFireButtonUp();

			break;
		}

		case COMMAND_ID_ALT_FIRING:
		{
			LTBOOL bIsFirstPerson = m_CameraMgr.GetCameraActive(m_hPlayerCamera);

			if (!IsPlayerDead() && bIsFirstPerson)
				m_weaponModel.OnAltFireButtonUp();

			break;
		}
			
		case COMMAND_ID_CHASEVIEWTOGGLE :
		{
			if(m_ePlayerState == PS_ALIVE)
			{
				if(CameraMgrFX_State(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON) == CAMERAMGRFX_STATE_ACTIVE)
				{
					CameraMgrFX_3rdPersonEdit(m_h3rdPersonCamera, LTFALSE);
					m_PlayerMovement.AllowRotation(LTTRUE);
					
					if(!CameraMgrFX_3rdPersonChanged(m_h3rdPersonCamera))
						SetExternalCamera(LTFALSE);
				}
			}
			
			break;
		}
			
		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnKeyDown(int key, int rep)
//
//	PURPOSE:	Handle key down notification
//				Try to avoid using OnKeyDown and OnKeyUp as they
//				are not portable functions
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnKeyDown(int key, int rep)
{
	if (IsMultiplayerGame())
	{
		if (VK_F1 == key)
		{
			m_pMPMgr->OnCommandOn(COMMAND_ID_PLAYERINFO);
		}

		if (VK_F3 == key)
		{
		// Draw the multiplayer stats if we need to
			LTBOOL bDrawMPStats =	m_GameType.IsMultiplayer() &&
									m_InterfaceMgr.GetGameState() == GS_PLAYING &&
									!m_InterfaceMgr.GetMessageMgr()->IsEditing();

			m_pMPMgr->ShowRulesDisplay(bDrawMPStats);
		}

		// [KLS] 9/23/2001 Allow clip-mode in multiplayer...

		// We only care about the clip mode toggle key...
		//
		// Don't allow toggling of clip mode if viewing the player display, note
		// the player display can be viewed while in the observing state...

		if (VK_F2 == key && !m_pMPMgr->IsPlayerDisplay())
		{
			// Only allow clip-mode while in the observing state...

			if (GetPlayerState() == PS_OBSERVING)
			{
				if (m_PlayerMovement.GetMovementState() != CMS_CLIPPING)
				{
					// Toggle clip-mode on...

					// Make sure the player is able to move and rotate again...

					m_PlayerMovement.AllowMovementReset();
					m_PlayerMovement.AllowRotationReset();

					// Show the clip mode display and hide the observe display...

					m_pMPMgr->ShowObserveDisplay(LTFALSE);
					m_pMPMgr->ShowClipModeDisplay(LTTRUE);

					// Do Normal clip mode toggling...

					m_PlayerMovement.SetMovementState(CMS_CLIPPING);

					// Set the pitch to match the camera...

					LTRotation rRot;
					g_pCameraMgr->GetCameraRotation(m_hPlayerCamera, rRot);

					LTVector vEuler;
					g_pMathLT->GetEulerAngles(rRot, vEuler);
					if (vEuler.x < 0.0f) vEuler.x += MATH_CIRCLE;

					m_PlayerMovement.GetCharacterVars()->m_fAimingPitch = MATH_RADIANS_TO_DEGREES(vEuler.x);
				}
				else  // Toggle clip-mode off...
				{
					// Do Normal clip mode toggling...

					m_PlayerMovement.SetMovementState(CMS_POST_CLIPPING);

					// Make sure the character can't move...

					m_PlayerMovement.AllowMovement(LTFALSE);
					m_PlayerMovement.AllowRotation(LTFALSE);

					// Set the observing state...

					m_PlayerMovement.SetMovementState(CMS_OBSERVING);
					m_PlayerMovement.SetObjectDims(CM_DIMS_VERY_SMALL);

					// Show the observe display and hide the clip-mode display...

					m_pMPMgr->ShowObserveDisplay(LTTRUE);
					m_pMPMgr->ShowClipModeDisplay(LTFALSE);

					// Go to a random observation point...

					m_pMPMgr->SetObservePoint(MP_OBSERVE_RANDOM);
				}
			}
			else if(m_PlayerMovement.GetMovementState() == CMS_CLIPPING)
			{
				// Toggle off clip mode if we somehow are still in it (this should
				// never happen)...

				m_PlayerMovement.SetMovementState(CMS_POST_CLIPPING);

				// Make sure the clip display is hidden...

				m_pMPMgr->ShowClipModeDisplay(LTFALSE);

				// Turn interface back on...

				m_InterfaceMgr.DrawPlayerStats(LTTRUE);
				m_InterfaceMgr.DrawCloseCaption(LTTRUE);
			}
		}
		// End [KLS] 9/23/2001 changes... 
	}
#ifndef _FINAL
#ifndef _DEMO
	else  // Single player...
	{
		if (key == VK_F2)
		{
			if(m_PlayerMovement.GetMovementState() != CMS_CLIPPING)
			{
				m_PlayerMovement.SetMovementState(CMS_CLIPPING);
				m_InterfaceMgr.DrawPlayerStats(LTFALSE);
				m_InterfaceMgr.DrawCloseCaption(LTFALSE);
			}
			else
			{
				m_PlayerMovement.SetMovementState(CMS_POST_CLIPPING);
				m_InterfaceMgr.DrawPlayerStats(LTTRUE);
				m_InterfaceMgr.DrawCloseCaption(LTTRUE);
			}
		}
	}
#endif //_DEMO
#endif //_FINAL


	// Do some special demo related stuff
/*#ifdef _DEMO

	if(key == VK_F3)
	{
		g_pProfileMgr->GetCurrentProfile()->RestoreDefaults();
		m_InterfaceMgr.GetMessageMgr()->AddMessage("E3 Demo!!  Default controls have been restored...");
	}
	else if((key == VK_F4) && !IsMultiplayerGame())
	{
		HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
		g_pLTClient->WriteToMessageByte(hMsg, CHEAT_HEALTH);
		g_pLTClient->WriteToMessageByte(hMsg, LTTRUE);
		g_pLTClient->EndMessage(hMsg);

		hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
		g_pLTClient->WriteToMessageByte(hMsg, CHEAT_ARMOR);
		g_pLTClient->WriteToMessageByte(hMsg, LTTRUE);
		g_pLTClient->EndMessage(hMsg);

		m_InterfaceMgr.GetMessageMgr()->AddMessage("E3 Demo!!  Free health powerup...");
	}

#endif
*/

	// Make sure we're not in a cinematic
	if((key != g_pClientButeMgr->GetCinematicSkipKey()) && (g_pCameraMgr->GetCameraActive(m_hCinematicCamera)))
		return;


	// Let the multiplayer manager handle it...
	if(IsMultiplayerGame() && !m_InterfaceMgr.GetMessageMgr()->IsEditing())
	{
		if(m_pMPMgr->OnKeyDown(key, rep))
			return;
	}

	// If we're dead 
	if(!IsMultiplayerGame() && IsPlayerDead() && (g_pLTClient->GetTime() >= m_fEarliestRespawnTime))
	{
		if(m_InterfaceMgr.GetGameState() == GS_PLAYING)
		{
			m_InterfaceMgr.MissionFailed(IDS_SP_YOUWEREKILLED);
		}
	}

	// [KLS] 9/9/01 Let the TeleType have a go at it (for single player hints...)
	if(m_TeleType.OnKeyDown(key, rep))
		return;

	// Let the interface mgr have a go at it...
	if(m_InterfaceMgr.OnKeyDown(key, rep))
		return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnKeyUp(int key, int rep)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnKeyUp(int key)
{
	// Let the multiplayer manager handle it...
	if(IsMultiplayerGame() && !m_InterfaceMgr.GetMessageMgr()->IsEditing())
	{
		if (VK_F1 == key)
		{
			m_pMPMgr->OnCommandOff(COMMAND_ID_PLAYERINFO);
		}

		if (VK_F3 == key)
		{
			m_pMPMgr->ShowRulesDisplay(LTFALSE);
		}

		if(m_pMPMgr->OnKeyUp(key))
			return;
	}


	// Let the interface mgr have a go at it...
	if(m_InterfaceMgr.OnKeyUp(key))
		return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnMessage(DBYTE messageID, HMESSAGEREAD hMessage)
{
	// Let multiplayer and interface handle messages first...

	if(m_InterfaceMgr.OnMessage(messageID, hMessage)) return;
	if(m_pMPMgr->OnMessage(messageID, hMessage)) return;


	switch(messageID)
	{
		case MID_SERVERFORCEPOS:
		{
			m_PlayerMovement.HandleMessageRead(messageID, hMessage, 0);
			m_bPlayerPosSet = LTTRUE;
		}
		break;

		case MID_SERVERFORCEVELOCITY:
		case MID_PLAYER_ORIENTATION:
		case MID_PHYSICS_UPDATE:
		{
			m_PlayerMovement.HandleMessageRead(messageID, hMessage, 0);
		}
		break;

		case MID_PLAYER_CONSOLE_STRING:
		{
			char str[128] = "\0";
			char arg[64] = "\0";
			uint8 nArgs;
			
			hMessage->ReadByteFL(nArgs);

			for(int i = 0; i < nArgs; i++)
			{
				hMessage->ReadStringFL(arg, sizeof(arg));

				if(str[0])
					sprintf(str, "%s %s", str, arg);
				else
					sprintf(str, "%s", arg);
			}

			g_pLTClient->RunConsoleString(str);
		}
		break;

		case MID_WEAPON_CHANGE :
		{
			ChangeWeapon(hMessage);
		}
		break;

		case MID_FORCE_WEAPON_CHANGE :
		{
			ForceChangeWeapon(hMessage);
		}
		break;

		case STC_BPRINT :
		{
			char msg[50];
			hMessage->ReadStringFL(msg, sizeof(msg));
			CSPrint(msg);
		}
		break;

		case MID_SHAKE_SCREEN :
		{
			LTVector vAmount;
			g_pLTClient->ReadFromMessageVector(hMessage, &vAmount);
			ShakeScreen(vAmount);
		}
		break;

		case MID_SFX_MESSAGE :
		{
			m_sfxMgr.OnSFXMessage(hMessage);
		}
		break;

		case MID_STARLIGHT:
		{
			m_ModeMGR.MakeStarlightObject(hMessage);
		}
		break;

		case MID_PLAYER_EXITLEVEL :
		{	
			HandleExitLevel(hMessage);
		}
		break;

		case MID_PLAYER_STATE_CHANGE :
		{
			HandlePlayerStateChange(hMessage);
		}
		break;

		case MID_PLAYER_AUTOSAVE :
		{
			AutoSave(hMessage);
		}
		break;

		case MID_PLAYER_DAMAGE :
		{
			HandlePlayerDamage(hMessage);
		}
		break;

		case MID_MUSIC:
		{
			if( m_Music.IsInitialized( ))
				m_Music.ProcessMusicMessage( hMessage );
			break;
		}

		case MID_PLAYER_LOADCLIENT :
		{
			UnpackClientSaveMsg(hMessage);
		}
		break;

		case MID_SERVER_ERROR :
		{
			HandleServerError(hMessage);
		}
		break;

		case MID_CHANGE_FOG :
		{
			if(_stricmp(m_ModeMGR.GetCurrentModeName().c_str(), "Normal") == 0)
				ResetGlobalFog();
		}
		break;

		case MID_PLAYER_DO_STORYOBJECT :
		{
			StartStoryObject(hMessage);
		}
		break;

		case MID_PLAYER_CACHE_STORYOBJECT :
		{
			HSTRING hStr = g_pLTClient->ReadFromMessageHString(hMessage);
			if(hStr)
			{
				m_StoryImageList.push_back(g_pInterfaceResMgr->GetSharedSurface(g_pLTClient->GetStringData(hStr)));
				g_pLTClient->FreeString(hStr);
			}
		}
		break;

		case MID_PLAYER_MDTRACKER_REMOVE :
		{
			CHudMgr* pHudMgr = m_InterfaceMgr.GetHudMgr();

			if(pHudMgr)
				pHudMgr->RemoveTracker();
		}
		break;

		case MID_PLAYER_MDTRACKER :
		{
			StartObjectTracker(hMessage);
		}
		break;

		case MID_PLAYER_FADEIN :
		{
			LTFLOAT fTime = g_pLTClient->ReadFromMessageFloat(hMessage);
			m_InterfaceMgr.StartScreenFadeIn(fTime);
		}
		break;

		case MID_PLAYER_UNMUTE_SOUND :
		{
			if(m_InterfaceMgr.GetAdvancedOptions() & AO_MUSIC)
			{
				m_InterfaceMgr.GetSettings()->SetFloatVar("MusicVolume", m_fMusicVolume);
				m_InterfaceMgr.GetSettings()->ImplementMusicVolume();
			}

			m_InterfaceMgr.GetSettings()->SetFloatVar("sfxvolume", m_fSFXVolume);
			m_InterfaceMgr.GetSettings()->ImplementSoundVolume();
		}
		break;

		case MID_PLAYER_FADEOUT :
		{
			LTFLOAT fTime = g_pLTClient->ReadFromMessageFloat(hMessage);
			m_InterfaceMgr.StartScreenFadeOut(fTime);
		}
		break;

		case MID_PLAYER_MD :
		{
			LTBOOL bOn = g_pLTClient->ReadFromMessageByte(hMessage);
			m_InterfaceMgr.GetHudMgr()->SetMDEnabled(bOn);
		}
		break;

		case MID_PLAYER_RESPAWN:
		{
			if(m_GameType != SP_STORY)
			{
				// handle the big respawn message...

				// first read in the character info...
				uint8 nCharacterId = g_pLTClient->ReadFromMessageByte(hMessage);
				m_PlayerMovement.SetMPButes(nCharacterId);

				// now set the new position..
				uint8 nMoveCode = g_pLTClient->ReadFromMessageByte(hMessage);
				LTVector vPos;
				g_pLTClient->ReadFromMessageVector(hMessage, &vPos);
				m_PlayerMovement.SetMPTeleport(vPos, nMoveCode);
				m_bPlayerPosSet = LTTRUE;

				// now change our state to alive...
				{
					m_ePlayerState = PS_ALIVE;

					m_InterfaceMgr.SetDrawInterface(LTTRUE);
					m_InterfaceMgr.DrawPlayerStats(LTTRUE);
					m_InterfaceMgr.DrawCloseCaption(LTTRUE);

					// Make sure the 3rd person effect no longer exists
					CameraMgrFX_Clear(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON);
					ClearFacehugged();

					// Then make sure we've got the 1st person camera active
					m_CameraMgr.SetCameraActive(m_hPlayerCamera);

					// Send some data to the server letting them know we got the message...
					m_PlayerMovement.AddClientUpdateFlag(CLIENTUPDATE_FLAGS);
				}
			}

			// Make sure the player is able to move and rotate again
			m_PlayerMovement.AllowMovementReset();
			m_PlayerMovement.AllowRotationReset();

			m_PlayerMovement.SetMovementState(CMS_IDLE);

			m_PlayerMovement.GetCharacterVars()->m_fAimingPitch = 0.0f;


			// Reset our crouch and wallwalk toggles again (just to make sure)
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();

			if(pSettings)
			{
				pSettings->SetCrouchLock(LTFALSE);
				pSettings->SetWallWalkLock(LTFALSE);
			}


			// Reset the net/fire camera FX and overlay
			m_ModeMGR.SetNetMode(LTFALSE);
			m_ModeMGR.SetOnFireMode(LTFALSE);

			CameraMgrFX_Clear(m_hPlayerCamera, CAMERAMGRFX_ID_NET_TILT);
			CameraMgrFX_Clear(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON);
			ClearFacehugged();

			m_CameraMgr.SetCameraActive(m_hPlayerCamera);
		}
		break;

		case MID_MP_RESPAWN:
		{
			// handle the big respawn message...

			// read in the character info...
			uint8 nCharacterId = g_pLTClient->ReadFromMessageByte(hMessage);
			m_PlayerMovement.SetMPButes(nCharacterId);

			// now set the new position..
			uint8 nMoveCode = g_pLTClient->ReadFromMessageByte(hMessage);
			LTVector vPos;
			g_pLTClient->ReadFromMessageVector(hMessage, &vPos);
			m_PlayerMovement.SetMPTeleport(vPos, nMoveCode);
			m_bPlayerPosSet = LTTRUE;

			// now set our rotation
			LTVector vRot;
			g_pLTClient->ReadFromMessageVector(hMessage, &vRot);
			m_PlayerMovement.SetMPOrientation(vRot);


			// now change our state to alive...
			{
				m_ePlayerState = PS_ALIVE;

				m_InterfaceMgr.SetDrawInterface(LTTRUE);
				m_InterfaceMgr.DrawPlayerStats(LTTRUE);
				m_InterfaceMgr.DrawCloseCaption(LTTRUE);

				// Make sure the 3rd person effect no longer exists
				CameraMgrFX_Clear(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON);
				ClearFacehugged();

				// Then make sure we've got the 1st person camera active
				m_CameraMgr.SetCameraActive(m_hPlayerCamera);

				// Send some data to the server letting them know we got the message...
				m_PlayerMovement.AddClientUpdateFlag(CLIENTUPDATE_FLAGS);
			}

			// Make sure the player is able to move and rotate again
			m_PlayerMovement.AllowMovementReset();
			m_PlayerMovement.AllowRotationReset();

			m_PlayerMovement.SetMovementState(CMS_IDLE);

			m_PlayerMovement.GetCharacterVars()->m_fAimingPitch = 0.0f;

			// now get our new weapon
			uint8 nCommandId = g_pLTClient->ReadFromMessageByte(hMessage);
			MPChangeWeapon(nCommandId);

			// read in our interface stuff
			ReadMPInterfaceUpdate(hMessage);

			// make sure we have our pred mask
			if(IsPredator(nCharacterId))
				m_InterfaceMgr.GetPlayerStats()->SetHasMask(LTTRUE);

			// make sure we have our night vision
			if(IsExosuit(nCharacterId) || IsHuman(nCharacterId))
				m_InterfaceMgr.GetPlayerStats()->SetHasNightVision(LTTRUE);

			// Reset our crouch and wallwalk toggles again (just to make sure)
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();

			if(pSettings)
			{
				pSettings->SetCrouchLock(LTFALSE);
				pSettings->SetWallWalkLock(LTFALSE);
			}


			// Reset the net/fire camera FX and overlay
			m_ModeMGR.SetNetMode(LTFALSE);
			m_ModeMGR.SetOnFireMode(LTFALSE);

			CameraMgrFX_Clear(m_hPlayerCamera, CAMERAMGRFX_ID_NET_TILT);
			CameraMgrFX_Clear(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON);
			ClearFacehugged();

			m_CameraMgr.SetCameraActive(m_hPlayerCamera);
		}
		break;

		case MID_MP_EXO_RESPAWN:
		{
			// handle the big respawn message...

			// read in the character info...
			uint8 nCharacterId = g_pLTClient->ReadFromMessageByte(hMessage);
			m_PlayerMovement.SetMPButes(nCharacterId);

			// now set the new position..
			uint8 nMoveCode = g_pLTClient->ReadFromMessageByte(hMessage);
			LTVector vPos;
			g_pLTClient->ReadFromMessageVector(hMessage, &vPos);
			m_PlayerMovement.SetMPTeleport(vPos, nMoveCode);
			m_bPlayerPosSet = LTTRUE;

			// now set our rotation
			LTVector vRot;
			g_pLTClient->ReadFromMessageVector(hMessage, &vRot);
			m_PlayerMovement.SetMPOrientation(vRot);
			m_PlayerMovement.GetCharacterVars()->m_fAimingPitch = 0.0f;

			// now get our new weapon
			uint8 nCommandId = g_pLTClient->ReadFromMessageByte(hMessage);
			MPChangeWeapon(nCommandId);

			// read in our interface stuff
			ReadMPInterfaceUpdate(hMessage);

			// make sure we have our night vision
			m_InterfaceMgr.GetPlayerStats()->SetHasNightVision(LTTRUE);

			// Reset our crouch and wallwalk toggles again (just to make sure)
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();

			if(pSettings)
			{
				pSettings->SetCrouchLock(LTFALSE);
				pSettings->SetWallWalkLock(LTFALSE);
			}

			// Reset the net/fire camera FX and overlay
			m_ModeMGR.SetNetMode(LTFALSE);
			m_ModeMGR.SetOnFireMode(LTFALSE);

			ClearFacehugged();
		}
		break;

		case MID_PLAYER_TELETYPE:
		{
			uint16 nID = (uint16)hMessage->ReadWord();
			uint8 nStyle = (uint8)hMessage->ReadByte();

			// If it's a hint and they're turned off... then just break
			if((nStyle == 1) && !g_vtEnableHints.GetFloat())
				break;

			// Start the teletype display...
			m_TeleType.Start(nID, nStyle);
		}
		break;

		case SCM_FLASHLIGHT:
		{
			LTBOOL bShow = g_pInterface->ReadFromMessageByte(hMessage);
			if (bShow)
				m_FlashLight.TurnOn();
			else
				m_FlashLight.TurnOff();
		}
		break;
			

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ShakeScreen()
//
//	PURPOSE:	Shanke, rattle, and roll
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ShakeScreen(LTVector vShake)
{
	if(m_CameraMgr.GetCameraActive(m_hCinematicCamera))
		CameraMgrFX_ShakeCreate(m_hCinematicCamera, vShake);
	else
		CameraMgrFX_ShakeCreate(m_hPlayerCamera, vShake);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::FlashScreen()
//
//	PURPOSE:	Tint screen
//
// ----------------------------------------------------------------------- //

void CGameClientShell::FlashScreen(LTVector vTintColor, LTVector vPos, LTFLOAT fTime, LTFLOAT fRampUp,
								LTFLOAT fRampDown, LTFLOAT fScaleDist, LTBOOL bFirstPersonOnly)
{
	// Check to see if this is disabled...
	if(g_vtDisableScreenFlash.GetFloat())
		return;


	if(g_pCameraMgr->GetCameraActive(m_hPlayerCamera))
		CameraMgrFX_FlashCreate(m_hPlayerCamera, vTintColor, vPos, fTime, fRampUp, fRampDown, fScaleDist, bFirstPersonOnly);

	if(!bFirstPersonOnly)
	{
		if(g_pCameraMgr->GetCameraActive(m_h3rdPersonCamera))
			CameraMgrFX_FlashCreate(m_h3rdPersonCamera, vTintColor, vPos, fTime, fRampUp, fRampDown, fScaleDist, LTFALSE);

		if(g_pCameraMgr->GetCameraActive(m_hCinematicCamera))
			CameraMgrFX_FlashCreate(m_hCinematicCamera, vTintColor, vPos, fTime, fRampUp, fRampDown, fScaleDist, LTFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PauseGame()
//
//	PURPOSE:	Pauses/Unpauses the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PauseGame(DBOOL bPause, DBOOL bPauseSound)
{
	m_bGamePaused = bPause;

	if (!IsMultiplayerGame())
	{
		HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(bPause ? MID_GAME_PAUSE : MID_GAME_UNPAUSE);
		g_pLTClient->EndMessage(hMessage);
	}

	if (bPause && bPauseSound)
	{
		g_pLTClient->PauseSounds();
	}
	else
	{
		g_pLTClient->ResumeSounds();
	}

	SetInputState(!bPause);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetInputState()
//
//	PURPOSE:	Allows/disallows input
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SetInputState(DBOOL bAllowInput)
{
	g_pLTClient->SetInputState(bAllowInput);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandlePlayerStateChange()
//
//	PURPOSE:	Update player state change
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandlePlayerStateChange(HMESSAGEREAD hMessage)
{
	const PlayerState eOldState = m_ePlayerState;
	m_ePlayerState = (PlayerState) g_pLTClient->ReadFromMessageByte(hMessage);


	// [KLS] 10/7/01 - Make sure the observe and clip mode displays aren't being 
	// shown when we change states (these displays are only used inside the 
	// PS_OBSERVE state so they should never be active across state changes...
	if (IsMultiplayerGame())
	{
		m_pMPMgr->ShowObserveDisplay(LTFALSE);
		m_pMPMgr->ShowClipModeDisplay(LTFALSE);
	}


	switch (m_ePlayerState)
	{
		case PS_DYING:
		{
			// record our last damage type
			DamageType eType = (DamageType) g_pLTClient->ReadFromMessageByte(hMessage);
			s_dtLastDamageType = eType;

			if (IsMultiplayerGame())
			{
				m_fEarliestRespawnTime = g_pLTClient->GetTime() + MULTIPLAYER_RESPAWN_WAIT_TIME;
				m_fForceRespawnTime = m_fEarliestRespawnTime + MULTIPLAYER_FORCE_RESPAWN_WAIT_TIME;
			}
			else
			{
				m_fEarliestRespawnTime = g_pLTClient->GetTime() + RESPAWN_WAIT_TIME;
				m_fForceRespawnTime = m_fEarliestRespawnTime + FORCE_RESPAWN_WAIT_TIME;
			}

			m_weaponModel.Reset();


			// Don't do the 3rd person camera if a facehugger killed us...
			if(s_dtLastDamageType == DT_FACEHUG)
			{
				SetFacehugged();
				m_fForceRespawnTime += FORCE_RESPAWN_WAIT_TIME;
			}
			else
			{
				// Make sure we're in the 3rd person death camera mode
				m_CameraMgr.SetCameraActive(m_h3rdPersonCamera);
				CameraMgrFX_3rdPersonToggle(m_h3rdPersonCamera, LTTRUE);
			}


			// Make sure the character can't move
			m_PlayerMovement.AllowMovement(LTFALSE);
			m_PlayerMovement.AllowRotation(LTFALSE);

			// Send some data to the server letting them know we got the message...
			m_PlayerMovement.AddClientUpdateFlag(CLIENTUPDATE_FLAGS);

			// Make sure the flashlight turns off
			m_FlashLight.TurnOff();

			// Turn off vision modes
			m_ModeMGR.SetNormalMode();

			break;
		}

		case PS_SWAPPED:
		{
			HOBJECT hFollowObject = g_pLTClient->ReadFromMessageObject(hMessage);
			m_PlayerMovement.FollowObject(hFollowObject);

			m_PlayerMovement.SetObjectFlags(OFT_Flags, FLAG_GOTHRUWORLD, LTFALSE);
			m_PlayerMovement.SetObjectFlags(OFT_Flags2, 0, LTFALSE);

			if( g_pLTClient->GetClientObject() )
				HideShowAttachments(g_pLTClient->GetClientObject());

			break;
		}

		case PS_ALIVE:
		{
			if( eOldState != PS_SWAPPED )
			{
				m_InterfaceMgr.SetDrawInterface(LTTRUE);
				m_InterfaceMgr.DrawPlayerStats(LTTRUE);
				m_InterfaceMgr.DrawCloseCaption(LTTRUE);

				// Make sure the 3rd person effect no longer exists
				CameraMgrFX_Clear(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON);
				ClearFacehugged();

				// Then make sure we've got the 1st person camera active
				m_CameraMgr.SetCameraActive(m_hPlayerCamera);

				// Send some data to the server letting them know we got the message...
				m_PlayerMovement.AddClientUpdateFlag(CLIENTUPDATE_FLAGS);
			}
			else
			{
				m_PlayerMovement.FollowObject(LTNULL);

				m_PlayerMovement.SetObjectFlags(OFT_Flags, m_PlayerMovement.GetCharacterVars()->m_dwFlags, LTFALSE);
				m_PlayerMovement.SetObjectFlags(OFT_Flags2, m_PlayerMovement.GetCharacterVars()->m_dwFlags2, LTFALSE);

				// Make sure we don't fall thru the world, silly LDs.
				m_PlayerMovement.SetObjectDims(CM_DIMS_VERY_SMALL);
				m_PlayerMovement.SetObjectDims(CM_DIMS_DEFAULT);
			}

			break;
		}

		case PS_DEAD:
		{
			if(IsMultiplayerGame() && !m_hFacehugger && s_dtLastDamageType == DT_FACEHUG)
			{
				SetFacehugged();
				m_fForceRespawnTime += FORCE_RESPAWN_WAIT_TIME;
			}

			// Reset our crouch and wallwalk toggles
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();

			if(pSettings)
			{
				pSettings->SetCrouchLock(LTFALSE);
				pSettings->SetWallWalkLock(LTFALSE);
			}

			break;
		}

		case PS_OBSERVING:
		{
			// Make sure the character can't move
			m_PlayerMovement.AllowMovement(LTFALSE);
			m_PlayerMovement.AllowRotation(LTFALSE);

			// Set the observing state
			m_PlayerMovement.SetMovementState(CMS_OBSERVING);
			m_PlayerMovement.SetObjectDims(CM_DIMS_VERY_SMALL);

			// Send some data to the server letting them know we got the message...
			m_PlayerMovement.AddClientUpdateFlag(CLIENTUPDATE_FLAGS);


			// Make sure we go back to the first person camera... that's the one that
			// handles observe mode properly.
			m_CameraMgr.SetCameraActive(m_hPlayerCamera);

			// Reset the net camera FX
			CameraMgrFX_Clear(m_hPlayerCamera, CAMERAMGRFX_ID_NET_TILT);
			CameraMgrFX_Clear(m_h3rdPersonCamera, CAMERAMGRFX_ID_3RDPERSON);
			ClearFacehugged();

			// Turn off vision modes
			m_ModeMGR.SetNormalMode();
			m_ModeMGR.SetNetMode(LTFALSE);
			m_ModeMGR.SetOnFireMode(LTFALSE);

			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AutoSave()
//
//	PURPOSE:	Autosave the game
//
// ----------------------------------------------------------------------- //

void CGameClientShell::AutoSave(HMESSAGEREAD hMessage)
{
	if (m_ePlayerState != PS_ALIVE) return;

	// First make sure we have somewhere to put our save...
	char saveDir[128];
	sprintf(saveDir,"%s\\%s",SAVE_DIR,g_pProfileMgr->GetCurrentProfileName());

	if (!CWinUtil::DirExist(saveDir))
	{
		CWinUtil::CreateDir(saveDir);
	}
	
	CSaveGameData savedGame;
	FillSavedGameData(&savedGame);

	char strSaveGame[256];
	savedGame.BuildSaveString(strSaveGame);

	char strIniName[128];
	sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
	CWinUtil::WinWritePrivateProfileString (GAME_NAME, "Reload", strSaveGame, strIniName);

	char strFilename[128];
	sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), RELOADLEVEL_FILENAME);
	SaveGame(strFilename);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandlePlayerDamage
//
//	PURPOSE:	Handle the player getting damaged
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandlePlayerDamage(HMESSAGEREAD hMessage)
{
	if(!hMessage)
		return;

	// ----------------------------------------------------------------------- //
	// Read in the information about the damage
	LTVector vDir;
	g_pLTClient->ReadFromMessageVector(hMessage, &vDir);
	DamageType eType = (DamageType) g_pLTClient->ReadFromMessageByte(hMessage);

	s_dtLastDamageType = eType;

	if(g_bScreenShotMode || IsPlayerDead() || (m_ePlayerState == PS_OBSERVING))
		return;

	// ----------------------------------------------------------------------- //
	// Handle the screen tinting
	LTFLOAT fPercent = vDir.Mag();

	LTFLOAT fMult = 0.5f + fPercent;
	fMult = fMult > 1.0f ? 1.0f : fMult;

	LTFLOAT fRampUp = 0.2f, fTintTime = 0.1f;
	LTFLOAT fRampDown = 0.5f + fPercent * 2.0f;

	LTVector vTintColor;
	GetCameraTintColor(eType, vTintColor);
	vTintColor *= fMult;

	LTVector vCamPos;
	g_pCameraMgr->GetCameraPos(m_hPlayerCamera, vCamPos);

	FlashScreen(vTintColor, vCamPos, fTintTime, fRampUp, fRampDown, 1000.0f, LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleServerError()
//
//	PURPOSE:	Handle any error messages sent from the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleServerError(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

	DBYTE nError = g_pLTClient->ReadFromMessageByte(hMessage);
	switch (nError)
	{
		case SERROR_SAVEGAME :
		{
			//DoMessageBox(IDS_SAVEGAMEFAILED, TH_ALIGN_CENTER);
		}
		break;

		case SERROR_LOADGAME :
		{
			//DoMessageBox(IDS_NOLOADLEVEL, TH_ALIGN_CENTER);

			m_InterfaceMgr.ChangeState(GS_MENU);
		}
		break;
		
		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SpecialEffectNotify()
//
//	PURPOSE:	Handle creation of a special fx
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SpecialEffectNotify(HLOCALOBJ hObj, HMESSAGEREAD hMessage)
{
	if (hObj)
	{
		DDWORD dwCFlags = g_pLTClient->GetObjectClientFlags(hObj);
		g_pLTClient->SetObjectClientFlags(hObj, dwCFlags | CF_NOTIFYREMOVE);
	}

	m_sfxMgr.HandleSFXMsg(hObj, hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnObjectRemove()
//
//	PURPOSE:	Handle removal of a server created object...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnObjectRemove(HLOCALOBJ hObj)
{
	if (!hObj) return;

	m_sfxMgr.RemoveSpecialFX(hObj);

	CHudMgr* pHudMgr = m_InterfaceMgr.GetHudMgr();

	if(pHudMgr)
		pHudMgr->OnObjectRemove(hObj);

	// See if this was our follow object...
	HOBJECT hFollowObj = m_PlayerMovement.GetFollowObject();

	if(hFollowObj == hObj)
		m_PlayerMovement.FollowObject(LTNULL);

	if (m_InterfaceMgr.GetPlayerStats()->GetAutoTarget() == hObj)
	{
		m_InterfaceMgr.GetPlayerStats()->SetAutoTarget(LTNULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleRecord()
//
//	PURPOSE:	Console command handlers for recording demos
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleRecord(int argc, char **argv)
{
	if(argc < 2)
	{
		g_pLTClient->CPrint("Record <world name> <filename>");
		return;
	}

	if(!DoLoadWorld(argv[0], NULL, NULL, LOAD_NEW_GAME, argv[1], NULL))
	{
		g_pLTClient->CPrint("Error starting world");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleRecord()
//
//	PURPOSE:	Console command handlers for playing demos
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandlePlaydemo(int argc, char **argv)
{
	if(argc < 1)
	{
		g_pLTClient->CPrint("Playdemo <filename>");
		return;
	}

	if(!DoLoadWorld("asdf", NULL, NULL, LOAD_NEW_GAME, NULL, argv[0]))
	{
		g_pLTClient->CPrint("Error starting world");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleExitLevel()
//
//	PURPOSE:	Handle ExitLevel console command
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleExitLevel(int argc, char **argv)
{
	// Tell the server to exit the level...

	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_EXITLEVEL);
	g_pLTClient->EndMessage(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleExitLevel()
//
//	PURPOSE:	Update player state change
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleExitLevel(HMESSAGEREAD hMessage)
{
	// Nothing in the message...currently...

	ExitLevel();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ExitLevel()
//
//	PURPOSE:	Exit this level and go to the next level
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ExitLevel()
{
	g_pLTClient->ClearInput(); // Start next level with a clean slate

	// We are officially no longer in a world...
	m_bInWorld = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LoadWorld()
//
//	PURPOSE:	Handles loading a world (with AutoSave)
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::LoadWorld(char* pWorldFile, char* pCurWorldSaveFile,
								  char* pRestoreObjectsFile, DBYTE nFlags)
{
	// Auto save the newly loaded level...
	return DoLoadWorld(pWorldFile, pCurWorldSaveFile, pRestoreObjectsFile, nFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DoLoadWorld()
//
//	PURPOSE:	Does actual work of loading a world
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::DoLoadWorld(char* pWorldFile, char* pCurWorldSaveFile,
								    char* pRestoreObjectsFile, DBYTE nFlags,
									char *pRecordFile, char *pPlaydemoFile)
{
	if (!pWorldFile) return LTFALSE;


	// Change to the loading level state...

	m_InterfaceMgr.ChangeState(GS_LOADINGLEVEL);

	// Clean up any old story object images...
	for(ImageList::iterator iter = m_StoryImageList.begin();
	    iter != m_StoryImageList.end(); ++iter )
	{
		g_pInterfaceResMgr->FreeSharedSurface(*iter);
	}
	m_StoryImageList.clear();



	// Check for special case of not being connected to a server or going to 
	// single player mode from multiplayer...

	int nGameMode = 0;
	g_pLTClient->GetGameMode(&nGameMode);

	if(!g_pLTClient->IsConnected() || (nGameMode != STARTGAME_NORMAL && nGameMode != GAMEMODE_NONE))
	{
		// Reset the game type to single player story
		m_GameType.Set(SP_STORY);

		// Turn off prediction for single player.  This avoids movement lag
		// with objects on lifts.
		g_pLTClient->RunConsoleString("prediction 0");

		// Fill in the start game request
		StartGameRequest request;
		memset(&request, 0, sizeof(StartGameRequest));

		request.m_pGameInfo = &m_GameType;
		request.m_GameInfoLen = sizeof(GameType);
		request.m_Type = STARTGAME_NORMAL;

		if(pRecordFile)
		{
			SAFE_STRCPY(request.m_RecordFilename, pRecordFile);
		}

		if(pWorldFile)
		{
			SAFE_STRCPY(request.m_WorldName, pWorldFile);
		}

		if(pPlaydemoFile)
		{
			SAFE_STRCPY(request.m_PlaybackFilename, pPlaydemoFile);
		}

		DRESULT dr = g_pLTClient->StartGame(&request);
		if (dr != LT_OK)
		{
			g_pInterfaceMgr->LoadFailed();
			return LTFALSE;
		}

		if(pPlaydemoFile)
		{
			// If StartGameRequest::m_PlaybackFilename is filled in the engine fills in m_WorldName.
			pWorldFile = request.m_WorldName;
		}
	}
	else if( nGameMode == STARTGAME_NORMAL )
	{
		g_pLTClient->SetGameInfo(&m_GameType, sizeof(GameType) );
	}

	// Send a message to the server shell with the needed info...

	HSTRING hWorldFile			= g_pLTClient->CreateString(pWorldFile);
	HSTRING hCurWorldSaveFile	= g_pLTClient->CreateString(pCurWorldSaveFile ? pCurWorldSaveFile : " ");
	HSTRING hRestoreObjectsFile	= g_pLTClient->CreateString(pRestoreObjectsFile ? pRestoreObjectsFile : " ");


	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_LOAD_GAME);
	g_pLTClient->WriteToMessageByte(hMessage, nFlags);
	g_pLTClient->WriteToMessageHString(hMessage, hWorldFile);
	g_pLTClient->WriteToMessageHString(hMessage, hCurWorldSaveFile);
	g_pLTClient->WriteToMessageHString(hMessage, hRestoreObjectsFile);
	g_pLTClient->WriteToMessageByte(hMessage, (uint8)g_pInterfaceMgr->GetPrevMissionIndex());

//	BuildClientSaveMsg(hMessage);

	g_pLTClient->EndMessage(hMessage);

	g_pLTClient->FreeString(hWorldFile);
	g_pLTClient->FreeString(hCurWorldSaveFile);
	g_pLTClient->FreeString(hRestoreObjectsFile);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LoadGame()
//
//	PURPOSE:	Handles loading a saved game
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::LoadGame(char* pWorld, char* pObjectsFile)
{
	if (!pWorld || !pObjectsFile) return LTFALSE;

	return DoLoadWorld(pWorld, DNULL, pObjectsFile, LOAD_RESTORE_GAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SaveGame()
//
//	PURPOSE:	Handles saving a game...
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::SaveGame(char* pObjectsFile)
{
	if (!pObjectsFile) return LTFALSE;
	
	// First make sure we have somewhere to put our save...
	char saveDir[128];
	sprintf(saveDir,"%s\\%s",SAVE_DIR,g_pProfileMgr->GetCurrentProfileName());

	if (!CWinUtil::DirExist(saveDir))
	{
		CWinUtil::CreateDir(saveDir);
	}
	
	DBYTE nFlags = 0;

	// Save the level objects...

	HSTRING hSaveObjectsName = g_pLTClient->CreateString(pObjectsFile);

	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SAVE_GAME);
	g_pLTClient->WriteToMessageByte(hMessage, nFlags);
	g_pLTClient->WriteToMessageHString(hMessage, hSaveObjectsName);

	BuildClientSaveMsg(hMessage);

	g_pLTClient->EndMessage(hMessage);

	g_pLTClient->FreeString(hSaveObjectsName);

	uint8 nSpecies = (uint8)GetPlayerSpecies();
	int nDifficulty = g_pGameClientShell->GetGameType()->GetDifficulty();

	char *pSimpleName = strrchr(pObjectsFile,'\\');
	pSimpleName++;

	char strSaveGame[256];
	strSaveGame[255] = 0;
	_snprintf (strSaveGame, 255, "%s|%s|%d|%d",m_strCurrentWorldName, pSimpleName, nSpecies, nDifficulty);

	char strIniName[128];
	strIniName[127] = 0;
	_snprintf (strIniName, 127, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);

	CWinUtil::WinWritePrivateProfileString (GAME_NAME, "Continue", strSaveGame, strIniName);



	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsJoystickEnabled()
//
//	PURPOSE:	Determines whether or not there is a joystick device 
//				enabled
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::IsJoystickEnabled()
{
	// first attempt to find a joystick device

	char strJoystick[128];
	memset (strJoystick, 0, 128);
	DRESULT result = g_pLTClient->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
	if (result != LT_OK) return LTFALSE;

	// ok - we found the device and have a name...see if it's enabled

	DBOOL bEnabled = LTFALSE;
	g_pLTClient->IsDeviceEnabled (strJoystick, &bEnabled);

	return bEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::EnableJoystick()
//
//	PURPOSE:	Attempts to find and enable a joystick device
//
// ----------------------------------------------------------------------- //

DBOOL CGameClientShell::EnableJoystick()
{
	// first attempt to find a joystick device

	char strJoystick[128];
	memset(strJoystick, 0, 128);
	DRESULT result = g_pLTClient->GetDeviceName(DEVICETYPE_JOYSTICK, strJoystick, 127);
	if (result != LT_OK) return LTFALSE;

	// ok, now try to enable the device

	char strConsole[256];
	sprintf(strConsole, "EnableDevice \"%s\"", strJoystick);
	g_pLTClient->RunConsoleString(strConsole);

	DBOOL bEnabled = LTFALSE;
	g_pLTClient->IsDeviceEnabled(strJoystick, &bEnabled);

	return bEnabled;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::FirstUpdate
//
//	PURPOSE:	Handle first update (each level)
//
// --------------------------------------------------------------------------- //

void CGameClientShell::FirstUpdate()
{
	if (!m_bFirstUpdate) return;

	char buf[200];
	m_bFirstUpdate = LTFALSE;

	// Reset the vision mode mgr.  It will be re-initialized below.
	m_ModeMGR.SetNormalMode();

	// Set up the level-start screen fade...

	if (g_varDoScreenFade.GetFloat())
	{
		m_InterfaceMgr.StartScreenFadeIn(g_varScreenFadeTime.GetFloat());
	}


	// Initialize model warble sheeyot...

	g_pLTClient->RunConsoleString("+ModelWarble 0");
	g_pLTClient->RunConsoleString("+WarbleSpeed 15");
	g_pLTClient->RunConsoleString("+WarbleScale .95");

	
	// Set up the panning sky values

	m_bPanSky = (DBOOL) g_pLTClient->GetServerConVarValueFloat("PanSky");
	m_fPanSkyOffsetX = g_pLTClient->GetServerConVarValueFloat("PanSkyOffsetX");
	m_fPanSkyOffsetZ = g_pLTClient->GetServerConVarValueFloat("PanSkyOffsetX");
	m_fPanSkyScaleX = g_pLTClient->GetServerConVarValueFloat("PanSkyScaleX");
	m_fPanSkyScaleZ = g_pLTClient->GetServerConVarValueFloat("PanSkyScaleZ");
	char* pTexture  = g_pLTClient->GetServerConVarValueString("PanSkyTexture");

	if (m_bPanSky)
	{
		g_pLTClient->SetGlobalPanTexture(GLOBALPAN_SKYSHADOW, pTexture);
	}


	// Set misc console vars...

	MirrorSConVar("AllSkyPortals", "AllSkyPortals");
	MirrorSConVar("LMAnimStatic", "LMAnimStatic");


	// Set up the environment map (chrome) texture...

	char* pEnvMap = g_pLTClient->GetServerConVarValueString("EnvironmentMap");
	char* pVal = ((!pEnvMap || !pEnvMap[0]) ? "WorldTextures\\Chrome.dtx" : pEnvMap);
	sprintf(buf, "EnvMap %s", pVal);
	g_pLTClient->RunConsoleString(buf);


	// Set up the global (per level) wind values...

	g_vWorldWindVel.x = g_pLTClient->GetServerConVarValueFloat("WindX");
	g_vWorldWindVel.y = g_pLTClient->GetServerConVarValueFloat("WindY");
	g_vWorldWindVel.z = g_pLTClient->GetServerConVarValueFloat("WindZ");


	// Set up the global (per level) fog values...

	ResetGlobalFog();


	// Make sure the multipass gouraud setting is correct

	LTFLOAT fServVal = g_pLTClient->GetServerConVarValueFloat("MultiGouraud");
	sprintf(buf, "MulitpassGouraud %s", fServVal);
	g_pLTClient->RunConsoleString(buf);
//	g_pLTClient->RunConsoleString("rebindtextures 1");


	// Initialize the music playlists...

	if (m_Music.IsInitialized() && !IsMultiplayerGame( ))
	{
		// Initialize music for the current level...

		char* pMusicControlFile = g_pLTClient->GetServerConVarValueString("MusicControlFile");

		if( pMusicControlFile )
		{
			CMusicState MusicState;
			char szExt[MAX_PATH];

			_splitpath( pMusicControlFile, NULL, MusicState.szDirectory, MusicState.szControlFile, szExt );
			strcat( MusicState.szControlFile, szExt );

			m_Music.RestoreMusicState(MusicState);
		}
	}


	// Re-initialize the vision mode manager so that it can 
	// undo the badness of ResetGlobalFog.
	m_ModeMGR.Init();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::MirrorSConVar
//
//	PURPOSE:	Takes the value of the server-side variable specified by 
//				pSVarName and sets its value into the client-sdie variable 
///				specified by pCVarName.
//
// --------------------------------------------------------------------------- //

void CGameClientShell::MirrorSConVar(char *pSVarName, char *pCVarName)
{
	char buf[512];
	float fVal;

	fVal = g_pLTClient->GetServerConVarValueFloat(pSVarName);
	sprintf(buf, "%s %f", pCVarName, fVal);
	g_pLTClient->RunConsoleString(buf);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::MirrorSConVarScale
//
//	PURPOSE:	Takes the value of the server-side variable specified by pMinSVarName
//				and pSVarName and then sets its value into the client-sdie variable 
///				specified by pCVarName using the scale value.
//
// --------------------------------------------------------------------------- //

void CGameClientShell::MirrorSConVarScale(char *pMinSVarName, char *pSVarName, char *pCVarName, LTFLOAT fScale)
{
	char buf[512];
	float fVal1, fVal2;

	fVal1 = g_pLTClient->GetServerConVarValueFloat(pMinSVarName);
	fVal2 = g_pLTClient->GetServerConVarValueFloat(pSVarName);

	if(fVal1 > fVal2)
		fVal1 = fVal2;

	sprintf(buf, "%s %f", pCVarName, (fVal1 + ((fVal2 - fVal1) * fScale)));
	g_pLTClient->RunConsoleString(buf);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ResetGlobalFog
//
//	PURPOSE:	Reset the global fog values based on the saved values...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ResetGlobalFog()
{
	// Grab the current scale value...
	LTFLOAT fScale = 1.0f;//(g_vtFogPerformanceScale.GetFloat() / 100.0f);


	// Set the FarZ for the level...

	MirrorSConVarScale("MinFarZ", "FarZ", "FarZ", fScale);


	// See if fog should be disabled

	DDWORD dwAdvancedOptions = m_InterfaceMgr.GetAdvancedOptions();

	if (!(dwAdvancedOptions & AO_FOG))
	{
		g_pLTClient->RunConsoleString("FogEnable 0");
		return;
	}

	MirrorSConVar("FogEnable", "FogEnable");
	MirrorSConVarScale("MinFogNearZ", "FogNearZ", "FogNearZ", fScale);
	MirrorSConVarScale("MinFogFarZ", "FogFarZ", "FogFarZ", fScale);

	MirrorSConVar("FogR", "FogR");
	MirrorSConVar("FogG", "FogG");
	MirrorSConVar("FogB", "FogB");

	MirrorSConVar("SkyFogEnable", "SkyFogEnable");
	MirrorSConVarScale("MinSkyFogNearZ", "SkyFogNearZ", "SkyFogNearZ", fScale);
	MirrorSConVarScale("MinSkyFogFarZ", "SkyFogFarZ", "SkyFogFarZ", fScale);

	// VFog....

	MirrorSConVar("SC_VFog", "VFog");
	MirrorSConVar("SC_VFogMinY", "VFogMinY");
	MirrorSConVar("SC_VFogMaxY", "VFogMaxY");
	MirrorSConVar("SC_VFogDensity", "VFogDensity");
	MirrorSConVar("SC_VFogMax", "VFogMax");
	MirrorSConVar("SC_VFogMaxYVal", "VFogMaxYVal");
	MirrorSConVar("SC_VFogMinYVal", "VFogMinYVal");


	// Go ahead and set the model add to zero here too...

	g_pLTClient->RunConsoleString("ModelAdd 0 0 0");
	g_pLTClient->RunConsoleString("WMAmbient 0 0 0");


	// Update the void color
	m_vVoidColor.x = g_pLTClient->GetServerConVarValueFloat("VoidR");
	m_vVoidColor.y = g_pLTClient->GetServerConVarValueFloat("VoidG");
	m_vVoidColor.z = g_pLTClient->GetServerConVarValueFloat("VoidB");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::RenderCamera()
//
//	PURPOSE:	Sets up the client and renders the camera
//
// ----------------------------------------------------------------------- //

void CGameClientShell::RenderCamera(DBOOL bDrawInterface)
{
//	StartProfile();

	// Update any client-side special effects...
	m_sfxMgr.UpdateSpecialFX();

//	EndProfile("GCS::RenderCamera -> m_sfxMgr.UpdateSpecialFX()");


	// Update the flash light...
	m_FlashLight.Update();


	// Make sure the weapon is updated before we render the camera...
	UpdateWeaponModel();


	if(s_bRenderCamera)
	{
		g_pLTClient->Start3D();

	//	StartProfile();

		// Render the track manager camera if we're editing
		if(g_pTrackMgr->Editing() && g_pTrackMgr->GetCamera())
			g_pLTClient->RenderCamera(g_pTrackMgr->GetCamera());
		else
			m_CameraMgr.RenderActiveCameras();

	//	EndProfile("GCS::RenderCamera -> m_CameraMgr.RenderActiveCameras()");


		g_pLTClient->StartOptimized2D();


		// Draw the multiplayer stats if we need to
		LTBOOL bDrawMPStats =	m_GameType.IsMultiplayer() &&
								g_pLTClient->IsCommandOn(COMMAND_ID_SCOREDISPLAY) &&
								m_InterfaceMgr.GetGameState() == GS_PLAYING;

		m_pMPMgr->ShowStatsDisplay(bDrawMPStats);

		if(!m_pMPMgr->Update())
		{
	//		StartProfile();

			if(!g_pTrackMgr->Draw())
				m_InterfaceMgr.Draw();

	//		EndProfile("GCS::RenderCamera -> m_InterfaceMgr.Draw()");

			if(m_pStoryElement)
			{
				m_pStoryElement->Update();
			}
			else
			{
				// Only draw the teletype stuff if the objectives are not displayed.
				if(!g_pLTClient->IsCommandOn(COMMAND_ID_SCOREDISPLAY))
					m_TeleType.Update();
			}
		}


		// Handle the message display
		m_InterfaceMgr.GetMessageMgr()->Draw();


		// Display any necessary debugging info...
		if(g_pCheatMgr)
			g_pCheatMgr->UpdateDebugText();


		g_pLTClient->EndOptimized2D();
		g_pLTClient->End3D();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlayer()
//
//	PURPOSE:	Update the player
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdatePlayer()
{
	HLOCALOBJ hClientObj = g_pLTClient->GetClientObject();
	if(!hClientObj || m_ePlayerState == PS_DEAD) return;


	// Hide/Show our attachments...

	HideShowAttachments(hClientObj);



	// Make sure the rendered player object is right where it should be

	if(g_vtLocalClientObjUpdate.GetFloat())
	{
		HOBJECT hPhysicsObj;
		LTRotation myRot;
		LTVector myPos;
		
		if (!(hPhysicsObj = m_PlayerMovement.GetObject())) return;

		g_pLTClient->GetObjectPos(hPhysicsObj, &myPos);
		g_pLTClient->SetObjectPos(hClientObj, &myPos);

		g_pLTClient->GetObjectRotation(hPhysicsObj, &myRot);
		g_pLTClient->SetObjectRotation(hClientObj, &myRot);
	}

	g_pLTClient->ProcessAttachments(hClientObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HideShowAttachments()
//
//	PURPOSE:	Recursively hide/show attachments...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HideShowAttachments(HOBJECT hObj)
{
	if (!hObj) return;

	HLOCALOBJ attachList[20];
	DDWORD dwListSize = 0;
	DDWORD dwNumAttach = 0;

	g_pLTClient->GetAttachments(hObj, attachList, 20, &dwListSize, &dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	for (int i=0; i < nNum; i++)
	{
		DDWORD dwUsrFlags;
		g_pLTClient->GetObjectUserFlags(attachList[i], &dwUsrFlags);
		
		if (dwUsrFlags & USRFLG_ATTACH_HIDE1SHOW3)
		{
			DDWORD dwFlags = g_pLTClient->GetObjectFlags(attachList[i]);

			if(m_CameraMgr.GetCameraActive(m_hPlayerCamera))
			{
				if (dwFlags & FLAG_VISIBLE)
				{
					dwFlags &= ~FLAG_VISIBLE;
					g_pLTClient->SetObjectFlags(attachList[i], dwFlags);
				}
			}
			else
			{
				if (!(dwFlags & FLAG_VISIBLE))
				{
					dwFlags |= FLAG_VISIBLE;
					g_pLTClient->SetObjectFlags(attachList[i], dwFlags);
				}
			}
		}
		else if (dwUsrFlags & USRFLG_ATTACH_HIDE1)
		{
			DDWORD dwFlags = g_pLTClient->GetObjectFlags(attachList[i]);

			if(m_CameraMgr.GetCameraActive(m_hPlayerCamera))
			{
				if (dwFlags & FLAG_VISIBLE)
				{
					dwFlags &= ~FLAG_VISIBLE;
					g_pLTClient->SetObjectFlags(attachList[i], dwFlags);
				}
			}
		}

		// Hide/Show this attachment's attachments...

		HideShowAttachments(attachList[i]);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::BuildClientSaveMsg
//
//	PURPOSE:	Save all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CGameClientShell::BuildClientSaveMsg(HMESSAGEWRITE hMessage)
{
	if (!hMessage) return;

	HMESSAGEWRITE hData = g_pLTClient->StartHMessageWrite();
	
	g_pLTClient->WriteToMessageDWord(hData, m_nCurrentSaveLoadVersion);

	// Save complex data members...

	m_InterfaceMgr.Save(hData);
	m_FlashLight.Save(hData);
	m_ModeMGR.Save(hData);
 
	// Save all necessary data members...

	g_pLTClient->WriteToMessageByte(hData, m_ePlayerState);

	m_PlayerMovement.WriteSaveData(hData);

	g_pLTClient->WriteToMessageHMessageWrite(hMessage, hData);
	g_pLTClient->EndHMessageWrite(hData);

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UnpackClientSaveMsg
//
//	PURPOSE:	Load all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UnpackClientSaveMsg(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

	m_bRestoringGame = LTTRUE;


	HMESSAGEREAD hData = g_pLTClient->ReadFromMessageHMessageRead(hMessage);

	m_nLastSaveLoadVersion = g_pLTClient->ReadFromMessageDWord(hData);

	// Load complex data members...

	m_InterfaceMgr.Load(hData);
	m_FlashLight.Load(hData);
	m_ModeMGR.Load(hData);


	// Load data members...

	m_ePlayerState		= (PlayerState) g_pLTClient->ReadFromMessageByte(hData);

	m_PlayerMovement.ReadSaveData(hData);

	g_pLTClient->EndHMessageRead(hData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleRespawn
//
//	PURPOSE:	Handle player respawn
//
// --------------------------------------------------------------------------- //

void CGameClientShell::HandleRespawn()
{
	if (!IsPlayerDead()) return;
	if (!IsMultiplayerGame()) return;
	if (IsFacehugged()) return;

	if (g_pLTClient->GetTime() < m_fEarliestRespawnTime) return;

	// Reset the silly land hack...
	m_bFirstLand = LTTRUE;

	// Send a message to the server telling it that it's ok to respawn us now...
	HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_RESPAWN);
	g_pLTClient->EndMessage(hMsg);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::QuickSave
//
//	PURPOSE:	Quick save the game
//
// --------------------------------------------------------------------------- //

DBOOL CGameClientShell::QuickSave()
{
	if( m_GameType != SP_STORY || 
		IsPlayerDead() || 
		m_InterfaceMgr.IsFadingOut() || 
		m_InterfaceMgr.IsFadingIn() || 
		CameraMgrFX_Active(GetPlayerCamera(), CAMERAMGRFX_ID_NET_TILT) ||
		(m_weaponModel.GetState() != W_IDLE && m_weaponModel.GetState() != W_SPINNING))
		return LTFALSE;

	if( !CanSaveGame() )
	{
		HSTRING hString = g_pLTClient->FormatString(IDS_SAVEGAMEFAILED_NOTPOSSIBLE);
		g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,DNULL);
		g_pLTClient->FreeString(hString);

		// only reset cursor if we are in game..
		if(m_InterfaceMgr.GetGameState() == GS_PLAYING || m_InterfaceMgr.GetGameState() == GS_PAUSED)
			g_pInterfaceMgr->UseCursor(LTFALSE);

		return LTFALSE;
	}

	// First make sure we have somewhere to put our save...
	char saveDir[128];
	sprintf(saveDir,"%s\\%s",SAVE_DIR,g_pProfileMgr->GetCurrentProfileName());

	if (!CWinUtil::DirExist(saveDir))
	{
		CWinUtil::CreateDir(saveDir);
	}
	
	// If a quicksave exists then we need to save off the savegame string here and assign
	// it to the archive save...
	char strFilename[128];
	sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_FILENAME);

	if(CWinUtil::FileExist(strFilename))
	{
		char strSaveGameSetting[256];
		memset(strSaveGameSetting, 0, 256);
		char strIniName[128];
		sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
		CWinUtil::WinGetPrivateProfileString(GAME_NAME, "Quick", "", strSaveGameSetting, 256, strIniName);

		// re-save the settings...
		CWinUtil::WinWritePrivateProfileString (GAME_NAME, "QuickBackup", strSaveGameSetting, strIniName);
	}


	CSaveGameData savedGame;
	FillSavedGameData(&savedGame);

	char strSaveGame[256];
	savedGame.BuildSaveString(strSaveGame);

	char strIniName[128];
	sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
	CWinUtil::WinWritePrivateProfileString (GAME_NAME, "Quick", strSaveGame, strIniName);


	m_bQuickSave = LTTRUE;

	return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::QuickLoad
//
//	PURPOSE:	Quick load the game
//
// --------------------------------------------------------------------------- //

DBOOL CGameClientShell::QuickLoad()
{
	if(m_GameType != SP_STORY || m_InterfaceMgr.IsFadingOut() || m_InterfaceMgr.IsFadingIn() )
		return LTFALSE;

	char strSaveGameSetting[256];
	memset(strSaveGameSetting, 0, 256);
	char strIniName[128];
	sprintf (strIniName, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), SAVEGAMEINI_FILENAME);
	CWinUtil::WinGetPrivateProfileString(GAME_NAME, "Quick", "", strSaveGameSetting, 256, strIniName);

	if (!*strSaveGameSetting)
	{
		HSTRING hString = g_pLTClient->FormatString(IDS_NOQUICKSAVEGAME);
		g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,DNULL);

		return LTFALSE;
	}

	CSaveGameData load_game_data;
	load_game_data.ParseSaveString(strSaveGameSetting);

	g_pProfileMgr->SetSpecies(load_game_data.eSpecies);

	g_pGameClientShell->GetGameType()->SetDifficulty(load_game_data.nDifficulty);


	// Reset the vision mode here...
	m_ModeMGR.SetNormalMode();

	char strFilename[128];
	sprintf (strFilename, "%s\\%s\\%s", SAVE_DIR, g_pProfileMgr->GetCurrentProfileName(), QUICKSAVE_FILENAME);
	if (!LoadGame(load_game_data.szWorldName, strFilename))
	{
		HSTRING hString = g_pLTClient->FormatString(IDS_LOADGAMEFAILED);
		g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,DNULL);

		// only reset cursor if we are in game..
		if(m_InterfaceMgr.GetGameState() == GS_PLAYING || m_InterfaceMgr.GetGameState() == GS_PAUSED)
			g_pInterfaceMgr->UseCursor(LTFALSE);

		return LTFALSE;
	}


	return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HostMultiplayerGame()
//
//	PURPOSE:	Start a multiplayer game with specific options
//
// --------------------------------------------------------------------------- //

LTRESULT CGameClientShell::HostMultiplayerGame(CServerOptions *pOptions)
{
	// The options must be valid
	if(!pOptions) return MP_HOST_ERROR_INVALID_OPTIONS;

#ifndef _DEMO
	// We must have at least one map selected
	if(pOptions->MapListEmpty())
		return MP_HOST_ERROR_NO_MAPS;
#endif

	// Check if the hosted game is using the dedicated serverapp.
	if( pOptions->bDedicated )
	{
		if( !LauncherServerApp( *pOptions ))
			return MP_HOST_ERROR_STARTGAME_FAILED;

		return LT_OK;
	}

	// Set the game type before the loading screen gets initted...
	m_GameType.Set(pOptions->nGameType);
	m_GameType.SetDifficulty(DIFFICULTY_PC_MEDIUM);


	// Change to the loading level state...
	GameState gsPrev = m_InterfaceMgr.GetGameState();
	m_InterfaceMgr.ChangeState(GS_LOADINGLEVEL);


	// Init the engine networking support
	g_pLTClient->InitNetworking(LTNULL, 0);


	// Go through the service list and select TCPIP
	NetService *pServiceList, *pRef;
	g_pLTClient->GetServiceList(pServiceList);

	for(pRef = pServiceList; pRef; pRef = pRef->m_pNext)
	{
		if(pRef->m_dwFlags == NETSERVICE_TCPIP)
		{
			g_pLTClient->SelectService(pRef->m_handle);
			break;
		}
	}

	// Turn on prediction for multiplayer player.
	g_pLTClient->RunConsoleString( "prediction 1" );

	// Make a cleared game request structure
	StartGameRequest request;
	memset(&request, 0, sizeof(StartGameRequest));


	// Fill in the basic game request info
	request.m_Type = STARTGAME_HOST;

	// fix port for lan only game
	uint32 nPort = pOptions->nPortID;
	if (pOptions->bLANConnection)
	{
		uint32 nLANPort = GetValidLANPort(nPort);

		if (nLANPort != nPort) 
		{
			g_pLTClient->CPrint("port changed to: %d",nLANPort);
			nPort = nLANPort;
		}
	}

	request.m_HostInfo.m_Port = nPort;
	request.m_HostInfo.m_dwFlags = 0;


	// Setup the game info to pass over to the server
	MPGameInfo gi;
	pOptions->FillInHostInfo(request, gi);

	gi.m_nGameType = m_GameType;

	request.m_pGameInfo = &gi;
	request.m_GameInfoLen = sizeof(gi);


	// Load up our client data
	MPClientData cd;

	// Setup the player data to pass over for this client
	if(!pOptions->bDedicated)
	{
		// Get the profile
		CUserProfile *pProf = g_pProfileMgr->GetCurrentProfile();

		strncpy(cd.m_szName, pProf->m_sPlayerName.c_str(), MAX_MP_CLIENT_NAME_LENGTH - 1);
		strncpy(cd.m_szPassword, pOptions->szPassword, MAX_MP_PASSWORD_LENGTH - 1);
		cd.m_nCharacterClass = pProf->m_nRace < MAX_TEAMS ? pProf->m_nRace : Marine;

		for(int i = 0; i < MAX_TEAMS; i++)
			cd.m_nCharacterSet[i] = pProf->m_nChar[i];

		cd.m_bIgnoreTaunts = pProf->m_bIgnoreTaunts;

		request.m_pClientData = &cd;
		request.m_ClientDataLen = sizeof(cd);
	}


	// Try to start it up!! Varoom varoom!
	DRESULT dr = g_pLTClient->StartGame(&request);

	if(dr != LT_OK)
	{
		g_pInterfaceMgr->LoadFailed();
		m_InterfaceMgr.ChangeState(gsPrev);
		return MP_HOST_ERROR_STARTGAME_FAILED;
	}


	// Make sure this client knows he's should be starting to play the game...
	if(!pOptions->bDedicated)
		g_pInterfaceMgr->ChangeState(GS_PLAYING);

	return LT_OK;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::JoinMultiplayerGame()
//
//	PURPOSE:	Join up to an already existing multiplayer game
//
// --------------------------------------------------------------------------- //

LTRESULT CGameClientShell::JoinMultiplayerGame(CGameSpyServer *pServer, const char *szPassword)
{
	// The options must be valid
	if(!pServer || !pServer->GetAddress()) return MP_JOIN_ERROR_INVALID_SERVER;

	// Check to see if the game is full
	if(pServer->GetIntValue("numplayers", 0) >= pServer->GetIntValue("maxplayers", 0))
		return MP_JOIN_ERROR_GAME_FULL;


	// Grab the demo version of the server we selected
	char* sGameVer = pServer->GetStringValue("gamever");
	LTBOOL bVersionOk = LTFALSE;

	HSTRING hTemp = g_pLTClient->FormatString(IDS_VERSION);

	if(sGameVer && !stricmp(sGameVer, g_pLTClient->GetStringData(hTemp)))
		bVersionOk = LTTRUE;

	g_pLTClient->FreeString(hTemp);

	if(!bVersionOk)
		return MP_JOIN_ERROR_WRONG_VERSION;


	// Call the base functionality to join...
	return JoinMultiplayerGameAtIP(pServer->GetAddress(), (uint16)pServer->GetIntValue("hostport", 0), szPassword);
}

// --------------------------------------------------------------------------- //

LTRESULT CGameClientShell::JoinMultiplayerGameAtIP(const char *szIP, uint16 port, const char *szPassword)
{
	// The options must be valid
	if(!szIP) return MP_JOIN_ERROR_INVALID_SERVER;


	// Set the game type to DM so we get the multiplayer load screen
	m_GameType.Set(MP_DM);

	// Change to the loading level state...
	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);


	// Init the engine networking support
	g_pLTClient->InitNetworking(LTNULL, 0);


	// Turn on prediction for multiplayer player.
	g_pLTClient->RunConsoleString( "prediction 1" );

	// Make a cleared game request structure
	StartGameRequest request;
	memset(&request, 0, sizeof(StartGameRequest));

	// Fill in the basic game request info
	request.m_Type = STARTGAME_CLIENTTCP;
	sprintf(request.m_TCPAddress, "%s:%d", szIP, port);



	// Update the client data
	CUserProfile *pProf = g_pProfileMgr->GetCurrentProfile();

	MPClientData cd;
	strncpy(cd.m_szName, pProf->m_sPlayerName.c_str(), MAX_MP_CLIENT_NAME_LENGTH - 1);
	strncpy(cd.m_szPassword, szPassword ? szPassword : "AVP2", MAX_MP_PASSWORD_LENGTH - 1);
	cd.m_nCharacterClass = pProf->m_nRace < MAX_TEAMS ? pProf->m_nRace : Marine;

	for(int i = 0; i < MAX_TEAMS; i++)
		cd.m_nCharacterSet[i] = pProf->m_nChar[i];

	cd.m_bIgnoreTaunts = pProf->m_bIgnoreTaunts;

	request.m_pClientData = &cd;
	request.m_ClientDataLen = sizeof(cd);


	// Try to start it up!! Varoom varoom!
	DRESULT dr = g_pLTClient->StartGame(&request);

	if(dr != LT_OK)
	{
		g_pInterfaceMgr->LoadFailed();
		g_pInterfaceMgr->ChangeState(GS_FOLDER);
		g_pInterfaceMgr->GetFolderMgr()->SetCurrentFolder(FOLDER_ID_MAIN);
		return MP_JOIN_ERROR_STARTGAME_FAILED;
	}


	// Make sure this client knows he's should be starting to play the game...
	g_pInterfaceMgr->ChangeState(GS_PLAYING);

	return LT_OK;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LauncherServerApp()
//
//	PURPOSE:	Launches the serverapp.
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::LauncherServerApp( CServerOptions& serverOptions )
{
	PROCESS_INFORMATION procInfo;
	STARTUPINFO startInfo;
	CString sCmdLine;
	RMode rMode;

	// Save the current render mode.  We'll need to restore it if the serverapp
	// launching fails.
	g_pClientDE->GetRenderMode( &rMode );

	// Shutdown the renderer, minimize it, and hide it...
	g_pClientDE->ShutdownRender( RSHUTDOWN_MINIMIZEWINDOW | RSHUTDOWN_HIDEWINDOW );

	// Initialize the startup info.
	memset( &startInfo, 0, sizeof( STARTUPINFO ));
	startInfo.cb = sizeof( STARTUPINFO );

	// Setup the command line.
	sCmdLine.Format( "Avp2Serv.exe -config %s", serverOptions.szCfgName );

	// Start the server app.
	if( !CreateProcess( "Avp2Serv.exe", ( char* )( char const* )sCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, 
		&startInfo, &procInfo ))
	{
		// Serverapp failed.  Restore the render mode.
		g_pClientDE->SetRenderMode( &rMode );
		return FALSE;
	}

	// We're done with this process.
	g_pLTClient->Shutdown();

	return TRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsMultiplayerGame()
//
//	PURPOSE:	See if we are playing a multiplayer game
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::IsMultiplayerGame()
{
	int nGameMode = 0;
	g_pLTClient->GetGameMode(&nGameMode);
	if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE) return LTFALSE;

	return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsPlayerInWorld()
//
//	PURPOSE:	See if the player is in the world
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::IsPlayerInWorld()
{
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();

	if (!m_bPlayerPosSet || !m_bInWorld || m_ePlayerState == PS_UNKNOWN || !hPlayerObj) return LTFALSE;

	return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetNiceWorldName
//
//	PURPOSE:	Get the nice (level designer set) world name...
//
// --------------------------------------------------------------------------- //

DBOOL CGameClientShell::GetNiceWorldName(char* pWorldFile, char* pRetName, int nRetLen)
{
	if (!pWorldFile || !pRetName || nRetLen < 2) return LTFALSE;

	char buf[_MAX_PATH];
	buf[0] = '\0';
	DWORD len;

	char buf2[_MAX_PATH];
	sprintf(buf2, "%s.dat", pWorldFile);

	DRESULT dRes = g_pLTClient->GetWorldInfoString(buf2, buf, _MAX_PATH, &len);

	if (dRes != LT_OK || !buf[0] || len < 1)
	{
		// try pre-pending "worlds\" to the filename to see if it will find it then...
		// sprintf (buf2, "worlds\\%s.dat", pWorldFile);
		// dRes = g_pLTClient->GetWorldInfoString(buf2, buf, _MAX_PATH, &len);

		if (dRes != LT_OK || !buf[0] || len < 1)
		{
			return LTFALSE;
		}
	}


	char tokenSpace[5*(PARSE_MAXTOKENSIZE + 1)];
	char *pTokens[5];
	int nArgs;

	char* pCurPos = buf;
	char* pNextPos;

	DBOOL bMore = LTTRUE;
	while (bMore)
	{
		bMore = g_pLTClient->Parse(pCurPos, &pNextPos, tokenSpace, pTokens, &nArgs);
		if (nArgs < 2) break;

		if (_stricmp(pTokens[0], "WORLDNAME") == 0)
		{
			strncpy(pRetName, pTokens[1], nRetLen);
			return LTTRUE;
		}

		pCurPos = pNextPos;
	}
	
	return LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnModelKey
//
//	PURPOSE:	Handle weapon model keys
//
// --------------------------------------------------------------------------- //

void CGameClientShell::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs, int nTracker)
{
	if (m_weaponModel.GetHandle() == hObj)
	{
		m_weaponModel.OnModelKey(hObj, pArgs);
	}
	else
	{
		m_sfxMgr.OnModelKey(hObj, pArgs);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetDisconnectCode
//
//	PURPOSE:	Set the disconnection string...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetDisconnectCode(uint32 nCode, const char *pMsg)
{
	m_InterfaceMgr.SetDisconnectError(nCode);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DoActivate
//
//	PURPOSE:	Tell the server to do Activate
//
// --------------------------------------------------------------------------- //

void CGameClientShell::DoActivate()
{
	char* pActivateOverride = g_vtActivateOverride.GetStr();
	if (pActivateOverride && pActivateOverride[0] != ' ')
	{
		g_pLTClient->RunConsoleString(pActivateOverride);
		return;
	}

	LTRotation rRot;
	LTVector vU, vR, vF, vPos;

	g_pLTClient->GetObjectPos(m_CameraMgr.GetCameraObject(m_hPlayerCamera), &vPos);
	g_pLTClient->GetObjectRotation(m_CameraMgr.GetCameraObject(m_hPlayerCamera), &rRot);
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_ACTIVATE);
	g_pLTClient->WriteToMessageVector(hMessage, &vPos);
	g_pLTClient->WriteToMessageVector(hMessage, &vF);
	g_pLTClient->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetFacehugged
//
//	PURPOSE:	Setup the facehugger mode...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetFacehugged()
{
	// Set the time this we started the mode...
	m_fFacehugTime = g_pLTClient->GetTime();

	// Delete him if he already exists...
	if(m_hFacehugger)
	{
		g_pLTClient->DeleteObject(m_hFacehugger);
		m_hFacehugger = LTNULL;
	}


	// Now create the facehug model
	ObjectCreateStruct ocs;
	ocs.Clear();

	ocs.m_Flags = FLAG_VISIBLE | FLAG_REALLYCLOSE;
	ocs.m_ObjectType = OT_MODEL;

	SAFE_STRCPY(ocs.m_Filename, "models\\props\\facehugger_pv.abc");
	SAFE_STRCPY(ocs.m_SkinNames[0], "skins\\props\\facehugger_pv.dtx");

	m_hFacehugger = g_pLTClient->CreateObject(&ocs);


	// Play the proper animation
	if(m_hFacehugger)
	{
		LTAnimTracker *pTracker = LTNULL;
		g_pModelLT->GetMainTracker(m_hFacehugger, pTracker);

		LTBOOL bLoop = LTFALSE;
		HMODELANIM hAnim = g_pLTClient->GetAnimIndex(m_hFacehugger, "Attack");

		if(hAnim == INVALID_MODEL_ANIM)
		{
			hAnim = g_pLTClient->GetAnimIndex(m_hFacehugger, "Facehug");
			bLoop = LTTRUE;
		}

		if(!pTracker || (hAnim == INVALID_MODEL_ANIM))
		{
			ClearFacehugged();
			return;
		}

		g_pModelLT->SetCurAnim(pTracker, hAnim);
		g_pModelLT->SetLooping(pTracker, bLoop);
		g_pModelLT->SetPlaying(pTracker, LTTRUE);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateFacehugged
//
//	PURPOSE:	Update the facehugger mode...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateFacehugged()
{
	// Make sure we've got a facehugger at all...
	if(!m_hFacehugger)
		return;

	// See if the time has expired yet...
	if(g_pLTClient->GetTime() > (m_fFacehugTime + 5.0f))
	{
		// Make sure we're in the 3rd person death camera mode
		m_CameraMgr.SetCameraActive(m_h3rdPersonCamera);
		CameraMgrFX_3rdPersonToggle(m_h3rdPersonCamera, LTTRUE);

		// Remove the facehugger...
		ClearFacehugged();
	}
	else
	{
		// Get the current animation information of the facehugger...
		LTAnimTracker *pTracker = LTNULL;
		g_pModelLT->GetMainTracker(m_hFacehugger, pTracker);
		HMODELANIM hAnim;

		g_pModelLT->GetCurAnim(pTracker, hAnim);

		if(hAnim == g_pLTClient->GetAnimIndex(m_hFacehugger, "Attack"))
		{
			uint32 nFlags;
			g_pModelLT->GetPlaybackState(pTracker, nFlags);

			// If the attack animation is done... switch to the facehug animation...
			if(nFlags & MS_PLAYDONE)
			{
				hAnim = g_pLTClient->GetAnimIndex(m_hFacehugger, "Facehug");

				g_pModelLT->SetCurAnim(pTracker, hAnim);
				g_pModelLT->SetLooping(pTracker, LTTRUE);
				g_pModelLT->SetPlaying(pTracker, LTTRUE);
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ClearFacehugged
//
//	PURPOSE:	Get rid of the facehugger...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ClearFacehugged()
{
	// Make sure we've got a facehugger at all...
	if(!m_hFacehugger)
		return;

	// Delete the bugger...
	g_pLTClient->DeleteObject(m_hFacehugger);
	m_hFacehugger = LTNULL;
}


// --------------------------------------------------------------------------- //
// Called by the engine, saves all variables (console and member variables)
// related to demo playback.
// --------------------------------------------------------------------------- //
void LoadConVar(ClientDE *g_pLTClient, DStream *pStream, char *pVarName)
{
	float val;
	char cString[512];

	(*pStream) >> val;
	sprintf(cString, "%s %f", pVarName, val);
	g_pLTClient->RunConsoleString(cString);
}

void SaveConVar(ClientDE *g_pLTClient, DStream *pStream, char *pVarName, float defaultVal)
{
	HCONSOLEVAR hVar;
	float val;

	val = defaultVal;
	if(hVar = g_pLTClient->GetConsoleVar (pVarName))
	{
		val = g_pLTClient->GetVarValueFloat (hVar);
	}

	(*pStream) << val;
}

void CGameClientShell::DemoSerialize(DStream *pStream, DBOOL bLoad)
{
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

	if (bLoad)
	{
		pSettings->LoadDemoSettings(pStream);
	}
	else
	{
		pSettings->SaveDemoSettings(pStream);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LoadLeakFile
//
//	PURPOSE:	Loads a leak file and creates a line system for it.
//
// --------------------------------------------------------------------------- //

DBOOL LoadLeakFile(ClientDE *g_pLTClient, char *pFilename)
{
	FILE *fp;
	char line[256];
	HLOCALOBJ hObj;
	ObjectCreateStruct cStruct;
	DELine theLine;
	int nRead;

	fp = fopen(pFilename, "rt");
	if(fp)
	{
		INIT_OBJECTCREATESTRUCT(cStruct);
		cStruct.m_ObjectType = OT_LINESYSTEM;
		cStruct.m_Flags = FLAG_VISIBLE;
		hObj = g_pLTClient->CreateObject(&cStruct);
		if(!hObj)
		{
			fclose(fp);
			return LTFALSE;
		}

		while(fgets(line, 256, fp))
		{
			nRead = sscanf(line, "%f %f %f %f %f %f", 
				&theLine.m_Points[0].m_Pos.x, &theLine.m_Points[0].m_Pos.y, &theLine.m_Points[0].m_Pos.z, 
				&theLine.m_Points[1].m_Pos.x, &theLine.m_Points[1].m_Pos.y, &theLine.m_Points[1].m_Pos.z);

			// White
			theLine.m_Points[0].r = theLine.m_Points[0].g = theLine.m_Points[0].b = 1;
			theLine.m_Points[0].a = 1;
			
			// Read
			theLine.m_Points[1].r = 1;
			theLine.m_Points[1].g = theLine.m_Points[1].b = 0;
			theLine.m_Points[1].a = 1;

			g_pLTClient->AddLine(hObj, &theLine);
		}

		fclose(fp);
		return LTTRUE;
	}
	else
	{
		return LTFALSE;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ConnectToTcpIpAddress
//
//	PURPOSE:	Connects (joins) to the given tcp/ip address
//
// --------------------------------------------------------------------------- //

DBOOL ConnectToTcpIpAddress(CClientDE* g_pLTClient, char* sAddress)
{
	// Sanity checks...

/*	if (!g_pLTClient) return(LTFALSE);
	if (!sAddress) return(LTFALSE);


	// Try to connect to the given address...

	DBOOL db = NetStart_DoConsoleConnect(g_pLTClient, sAddress);

	if (!db)
	{
		if (strlen(sAddress) <= 0) g_pLTClient->CPrint("Unable to connect");
		else*/ g_pLTClient->CPrint("Unable to connect to %s", sAddress);
		return(LTFALSE);
/*	}


	// All done...

	if (strlen(sAddress) > 0) g_pLTClient->CPrint("Connected to %s", sAddress);
	else g_pLTClient->CPrint("Connected");

	return(LTTRUE);
*/
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DefaultModelHook
//
//	PURPOSE:	Default model hook function
//
// --------------------------------------------------------------------------- //

void DefaultModelHook (ModelHookData *pData, void *pUser)
{
	CGameClientShell* pShell = (CGameClientShell*) pUser;
	if (!pShell) return;
	
	DDWORD nUserFlags = 0;
	g_pLTClient->GetObjectUserFlags (pData->m_hObject, &nUserFlags);

	if (nUserFlags & USRFLG_GLOW)
	{
		if (pData->m_LightAdd)
		{
			*pData->m_LightAdd = LTVector(255.0f, 255.0f, 255.0f) * g_vtGlowRatio.GetFloat();
		}
	}
	else if (nUserFlags & USRFLG_MODELADD)
	{
		// Get the new color out of the upper 3 bytes of the 
		// user flags...

		LTFLOAT r = (LTFLOAT)(nUserFlags>>24);
		LTFLOAT g = (LTFLOAT)(nUserFlags>>16);
		LTFLOAT b = (LTFLOAT)(nUserFlags>>8);

		if (pData->m_LightAdd)
		{
			*pData->m_LightAdd = LTVector(r, g, b);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HookedWindowProc
//
//	PURPOSE:	Hook it real good
//
// --------------------------------------------------------------------------- //

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL                   0x020A
#endif

LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_MOUSEWHEEL)
	{
		CGameClientShell::OnMouseWheel(hWnd, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (UINT)(wParam));
	}
	else
	{
		switch(uMsg)
		{
			HANDLE_MSG(hWnd, WM_LBUTTONUP, CGameClientShell::OnLButtonUp);
			HANDLE_MSG(hWnd, WM_LBUTTONDOWN, CGameClientShell::OnLButtonDown);
			HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, CGameClientShell::OnLButtonDblClick);
			HANDLE_MSG(hWnd, WM_RBUTTONUP, CGameClientShell::OnRButtonUp);
			HANDLE_MSG(hWnd, WM_RBUTTONDOWN, CGameClientShell::OnRButtonDown);
			HANDLE_MSG(hWnd, WM_RBUTTONDBLCLK, CGameClientShell::OnRButtonDblClick);
			HANDLE_MSG(hWnd, WM_MOUSEMOVE, CGameClientShell::OnMouseMove);
			HANDLE_MSG(hWnd, WM_CHAR, CGameClientShell::OnChar);
		}
	}

	_ASSERT(g_pfnMainWndProc);
	return(CallWindowProc(g_pfnMainWndProc,hWnd,uMsg,wParam,lParam));
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnChar(HWND hWnd, char c, int rep)
{
	g_pInterfaceMgr->OnChar(c);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnLButtonUp(x,y);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	g_pMPMgr->OnLButtonDown(x,y);
	g_pInterfaceMgr->OnLButtonDown(x,y);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnLButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnLButtonDblClick(x,y);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnRButtonUp(x,y);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnRButtonDown(x,y);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnRButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnRButtonDblClick(x,y);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	g_pMPMgr->OnMouseMove(x,y);
	g_pInterfaceMgr->OnMouseMove(x,y);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnMouseWheel(HWND hwnd, int x, int y, UINT keyFlags)
{
	int delta = (int)keyFlags;

	g_pInterfaceMgr->OnMouseWheel(x,y,delta);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetMouseSensitivityScale
//
//	PURPOSE:	Control over mouse sensitivity scaling...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetMouseSensitivityScale(LTFLOAT fSensScale)
{
	// Get the mouse sensitivity setting
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("MouseSensitivity");
	LTFLOAT fSens = 1.0f;

	if(hVar)
		fSens = g_pLTClient->GetVarValueFloat(hVar);


	// Get the mouse device name
	char strDevice[128];
	memset (strDevice, 0, 128);
    LTRESULT result = g_pLTClient->GetDeviceName(DEVICETYPE_MOUSE, strDevice, 127);

	if(result == LT_OK)
	{
		// Get mouse x and y axis names
		char strXAxis[32];
		memset(strXAxis, 0, 32);
		char strYAxis[32];
		memset(strYAxis, 0, 32);

        LTBOOL bFoundXAxis = LTFALSE;
        LTBOOL bFoundYAxis = LTFALSE;

		DeviceObject* pList = g_pLTClient->GetDeviceObjects(DEVICETYPE_MOUSE);
		DeviceObject* ptr = pList;

		while(ptr)
		{
			if (ptr->m_ObjectType == CONTROLTYPE_XAXIS)
			{
				SAFE_STRCPY(strXAxis, ptr->m_ObjectName);
                bFoundXAxis = LTTRUE;
			}

			if (ptr->m_ObjectType == CONTROLTYPE_YAXIS)
			{
				SAFE_STRCPY(strYAxis, ptr->m_ObjectName);
                bFoundYAxis = LTTRUE;
			}

			ptr = ptr->m_pNext;
		}

		if(pList)
			g_pLTClient->FreeDeviceObjects(pList);

		if(bFoundXAxis && bFoundYAxis)
		{
			// Run the console strings
			char strConsole[64];
			LTFLOAT fTotalSens = (0.166f + (fSens / 2.75f)) * fSensScale * 0.001f;

			sprintf(strConsole, "scale \"%s\" \"%s\" %f", strDevice, strXAxis, fTotalSens);
			g_pLTClient->RunConsoleString (strConsole);

			sprintf(strConsole, "scale \"%s\" \"%s\" %f", strDevice, strYAxis, fTotalSens * 1.1f);
			g_pLTClient->RunConsoleString (strConsole);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HookWindow
//
//	PURPOSE:	HOOK IT!
//
// --------------------------------------------------------------------------- //

BOOL HookWindow()
{
	// Get the screen dimz
	HSURFACE hScreen = g_pInterface->GetScreenSurface();
	if(!hScreen)
		return FALSE;

	// Hook the window
	if(g_pInterface->GetEngineHook("HWND",(void **)&g_hMainWnd) != LT_OK)
	{
		TRACE("HookWindow - ERROR - could not get the engine window!\n");
		return FALSE;
	}

	// Get the window procedure
#ifdef STRICT
	g_pfnMainWndProc = (WNDPROC)GetWindowLong(g_hMainWnd,GWL_WNDPROC);
#else
	g_pfnMainWndProc = (FARPROC)GetWindowLong(g_hMainWnd,GWL_WNDPROC);
#endif 

	if(!g_pfnMainWndProc)
	{
		TRACE("HookWindow - ERROR - could not get the window procedure from the engine window!\n");
		return FALSE;
	}

	// Replace it with ours
	if(!SetWindowLong(g_hMainWnd,GWL_WNDPROC,(LONG)HookedWindowProc))
	{
		TRACE("HookWindow - ERROR - could not set the window procedure!\n");
		return FALSE;
	}

	// Clip the cursor if we're NOT in a window
	HCONSOLEVAR hVar = g_pInterface->GetConsoleVar("Windowed");
	BOOL bClip = TRUE;
	if(hVar)
	{
		float fVal = g_pInterface->GetVarValueFloat(hVar);
		if(fVal == 1.0f)
			bClip = FALSE;
	}

	if(bClip)
	{
		DDWORD dwScreenWidth = g_pGameClientShell->GetScreenWidth();
		DDWORD dwScreenHeight = g_pGameClientShell->GetScreenHeight();

		SetWindowLong(g_hMainWnd,GWL_STYLE,WS_VISIBLE);
		SetWindowPos(g_hMainWnd,HWND_TOPMOST,0,0,dwScreenWidth,dwScreenHeight,SWP_FRAMECHANGED);
		RECT wndRect;
		GetWindowRect(g_hMainWnd, &wndRect);
		ClipCursor(&wndRect);
	}

	return TRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	UnhookWindow
//
//	PURPOSE:	Unhook the window
//
// --------------------------------------------------------------------------- //

void UnhookWindow()
{
	if(g_pfnMainWndProc && g_hMainWnd)
	{
		SetWindowLong(g_hMainWnd, GWL_WNDPROC, (LONG)g_pfnMainWndProc);
		g_hMainWnd = 0;
		g_pfnMainWndProc = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleWeaponPickup()
//
//	PURPOSE:	Handle picking up weapon
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleWeaponPickup(DBYTE nWeaponID)
{
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (!pWeapon) return;

	LTVector vTintColor = g_pLayoutMgr->GetWeaponPickupColor();
	LTFLOAT	fTotalTime = g_pLayoutMgr->GetTintTime();
	LTFLOAT fRampDown = fTotalTime * 0.85f;
	LTFLOAT fRampUp   = fTotalTime * 0.10f;
	LTFLOAT fTintTime = fTotalTime * 0.05f;

	LTVector vCamPos;
	g_pCameraMgr->GetCameraPos(m_hPlayerCamera, vCamPos);

	LTRotation rRot;
	g_pCameraMgr->GetCameraRotation(m_hPlayerCamera, rRot);

	LTVector vU, vR, vF;
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

	FlashScreen(vTintColor, vCamPos, fTintTime, fRampUp, fRampDown, 1000.0f, LTTRUE);

	//tell the hud about the pickup
	g_pInterfaceMgr->GetHudMgr()->AddWeaponPickup(nWeaponID);

	// Auto change weapon here
	LTBOOL bAutoSwitch = GetConsoleInt("AutoswitchWeapons",1);

	if(bAutoSwitch)
	{
		// See if the new weapon is better then the current one...
		
		int nNewWeaponRank = GetPlayerStats()->GetWeaponIndex(nWeaponID);
		int nCurWeaponRank = GetPlayerStats()->GetWeaponIndex(m_weaponModel.GetWeaponId());

		if( nNewWeaponRank != WMGR_INVALID_ID )
		{
			if( nCurWeaponRank == WMGR_INVALID_ID || nNewWeaponRank > nCurWeaponRank )
			{
				// Time to change weapons...
				ChangeWeapon(nWeaponID);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleAmmoPickup()
//
//	PURPOSE:	Handle picking up ammo
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleAmmoPickup(DBYTE nAmmoPoolId, int nAmmoCount)
{
	AMMO_POOL* pAmmoPool = g_pWeaponMgr->GetAmmoPool(nAmmoPoolId);
	if (!pAmmoPool) return;

	int nNameId = pAmmoPool->nNameId;
	if (!nNameId) return;

	LTVector vTintColor = g_pLayoutMgr->GetAmmoPickupColor();
	LTFLOAT	fTotalTime = g_pLayoutMgr->GetTintTime();
	LTFLOAT fRampDown = fTotalTime * 0.85f;
	LTFLOAT fRampUp   = fTotalTime * 0.10f;
	LTFLOAT fTintTime = fTotalTime * 0.05f;


	LTVector vCamPos;
	g_pCameraMgr->GetCameraPos(m_hPlayerCamera, vCamPos);

	LTRotation rRot;
	g_pCameraMgr->GetCameraRotation(m_hPlayerCamera, rRot);

	LTVector vU, vR, vF;
	g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

	FlashScreen(vTintColor, vCamPos, fTintTime, fRampUp, fRampDown, 1000.0f, LTTRUE);

	//tell the hud about the pickup
	g_pInterfaceMgr->GetHudMgr()->AddAmmoPickup(nAmmoPoolId);
}

// --------------------------------------------------------------------------- //

void CGameClientShell::OnTextureLoad(StateChange** stateChange, const char* commandLine)
{
	m_ModeMGR.RegisterTexture(stateChange, commandLine);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartStoryObject
//
//	PURPOSE:	Starts our story object display
//
// --------------------------------------------------------------------------- //

void CGameClientShell::StartStoryObject(HMESSAGEREAD hMessage)
{
	if(m_pStoryElement)
	{
		delete m_pStoryElement;
		m_pStoryElement = LTNULL;
	}

	m_pStoryElement = new StoryElement;

	if(m_pStoryElement)
	{
		m_pStoryElement->m_hImage		= g_pLTClient->ReadFromMessageHString(hMessage);
		m_pStoryElement->m_hSoundFile	= g_pLTClient->ReadFromMessageHString(hMessage);
		m_pStoryElement->m_fDuration	= g_pLTClient->ReadFromMessageFloat(hMessage);
		LTFLOAT fTemp					= g_pLTClient->ReadFromMessageFloat(hMessage);
		m_pStoryElement->m_nStringID	= (int)g_pLTClient->ReadFromMessageFloat(hMessage);
		m_pStoryElement->m_nFontId		= g_pLTClient->ReadFromMessageByte(hMessage);
		m_pStoryElement->m_nYOffset		= (int)g_pLTClient->ReadFromMessageFloat(hMessage);
		m_pStoryElement->m_nXOffset		= (int)g_pLTClient->ReadFromMessageFloat(hMessage);
		m_pStoryElement->m_nTextWidth	= (int)g_pLTClient->ReadFromMessageFloat(hMessage);
		
		
		m_pStoryElement->m_fRangeSqr	= fTemp * fTemp;
		//call init
		m_pStoryElement->Init();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::EndStoryObject
//
//	PURPOSE:	Ends the story object
//
// --------------------------------------------------------------------------- //

void CGameClientShell::EndStoryObject()
{
	if(m_pStoryElement)
	{
		delete m_pStoryElement;
		m_pStoryElement = LTNULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartObjectTracker
//
//	PURPOSE:	Starts the object tracker for the motion detector
//
// --------------------------------------------------------------------------- //

void CGameClientShell::StartObjectTracker(HMESSAGEREAD hMessage)
{
	CHudMgr* pHudMgr = m_InterfaceMgr.GetHudMgr();

	if(pHudMgr)
		pHudMgr->SetTrackerObject(hMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateEMPEffect()
//
//	PURPOSE:	Update any EMP effect
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateEMPEffect()
{
	uint32 dwUserFlags;
	g_pLTClient->GetObjectUserFlags(g_pLTClient->GetClientObject(), &dwUserFlags);

	if(!m_bEMPEffect)
	{
		CharacterButes*	pButes = m_PlayerMovement.GetCharacterButes();

		uint32 nEMPFlags = pButes->m_nEMPFlags;

		if((dwUserFlags & USRFLG_CHAR_EMPEFFECT) && (nEMPFlags & EMP_FLAG_STUN))
		{
			//first time effect
			m_PlayerMovement.AllowMovement(LTFALSE);
			m_bEMPEffect = LTTRUE;

			SetMouseSensitivityScale(0.25f);
		}
	}
	else
	{
		if(!(dwUserFlags & USRFLG_CHAR_EMPEFFECT))
		{
			//time to reset
			m_PlayerMovement.AllowMovement(LTTRUE);
			m_bEMPEffect = LTFALSE;

			SetMouseSensitivityScale();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateStunEffect()
//
//	PURPOSE:	Update stun effect
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateStunEffect()
{
	uint32 dwUserFlags;
	g_pLTClient->GetObjectUserFlags(g_pLTClient->GetClientObject(), &dwUserFlags);

	if(!m_bStunEffect)
	{
		if(dwUserFlags & USRFLG_CHAR_STUNNED)
		{
			//first time effect
			m_PlayerMovement.AllowMovement(LTFALSE);
			m_bStunEffect = LTTRUE;

			SetMouseSensitivityScale(0.25f);
		}
	}
	else
	{
		if(!(dwUserFlags & USRFLG_CHAR_STUNNED))
		{
			//time to reset
			m_PlayerMovement.AllowMovement(LTTRUE);
			m_bStunEffect = LTFALSE;

			SetMouseSensitivityScale();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateFaceHugEffect()
//
//	PURPOSE:	Update any facehug related stuff
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateFaceHugEffect()
{
	uint32 dwUserFlags;
	g_pLTClient->GetObjectUserFlags(g_pLTClient->GetClientObject(), &dwUserFlags);

	if(!m_bFaceHugEffect)
	{
		if(dwUserFlags & USRFLG_CHAR_FACEHUG)
		{
			//first time effect
			m_PlayerMovement.AllowMovement(LTFALSE);
			m_bFaceHugEffect = LTTRUE;

			SetExternalCamera(LTTRUE);
//			CameraMgrFX_WidescreenToggle(m_h3rdPersonCamera);
		}
	}
	else
	{
		if(!(dwUserFlags & USRFLG_CHAR_FACEHUG))
		{
			//time to reset
			m_PlayerMovement.SetMovementState(CMS_IDLE);

			m_PlayerMovement.AllowMovement(LTTRUE);
			m_bFaceHugEffect = LTFALSE;

			SetExternalCamera(LTFALSE);
//			CameraMgrFX_WidescreenToggle(m_h3rdPersonCamera);
		}
		else
		{
			// Make sure the movement state is set properly
			if(m_PlayerMovement.GetMovementState() != CMS_FACEHUG_ATTACH)
				m_PlayerMovement.SetMovementState(CMS_FACEHUG_ATTACH);

			// Make sure the camera is in 3rd person (just in case)...
			SetExternalCamera(LTTRUE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateOrientationIndicator()
//
//	PURPOSE:	Update the orientation indicator...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateOrientationIndicator()
{
	// If we're not an alien, or we're in hunting mode... make sure these overlays are turned off...
	if(	!g_vtOrientOverlay.GetFloat() || (m_PlayerMovement.GetMovementState() != CMS_WALLWALKING) ||
		(VisionMode::g_pButeMgr->IsAlienHuntingVision(GetVisionModeMgr()->GetCurrentModeName())))
	{
		s_fOrientAngle = MATH_PI;
		GetVisionModeMgr()->SetOrientMode(LTFALSE);
		return;
	}


	// Turn the overlays on...
	GetVisionModeMgr()->SetOrientMode(LTTRUE);


	// Make sure the overlay is facing the correct direction
	HOBJECT hPlayer = g_pGameClientShell->GetPlayerMovement()->GetObject();

	if(hPlayer)
	{
		LTRotation rRot;
		LTVector vDir, vR, vU, vF;

		// Find out what direction we ARE facing
		g_pLTClient->GetObjectRotation(hPlayer, &rRot);
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);

		// Figure out what direction we want to compare with
		vDir.Init(0.0f, 1.0f, 0.0f);

		// Now determine a rotation given these values
		LTFLOAT fDot = vDir.Dot(vU);

		if(fDot > 1.0f) fDot = 1.0f;
		else if(fDot < -1.0f) fDot = -1.0f;

		LTFLOAT fAngle = (LTFLOAT)acos(-fDot);

		if(fAngle < s_fOrientAngle)
		{
			s_fOrientAngle -= g_vtOrientSpeed.GetFloat() * g_pLTClient->GetFrameTime();

			if(s_fOrientAngle < fAngle)
				s_fOrientAngle = fAngle;
		}
		else if(fAngle > s_fOrientAngle)
		{
			s_fOrientAngle += g_vtOrientSpeed.GetFloat() * g_pLTClient->GetFrameTime();

			if(s_fOrientAngle > fAngle)
				s_fOrientAngle = fAngle;
		}

		// Set the rotation of the overlays...
		GetVisionModeMgr()->SetOrientAngle(s_fOrientAngle);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateInfoString()
//
//	PURPOSE:	Display the info string in the center of the screen...
//
// ----------------------------------------------------------------------- //

inline DBOOL ObjListFilterFn_CharactersOnly(HOBJECT hTest, void *pUserData)
{
	// Always hit the world...
	if(g_pPhysicsLT->IsWorldObject(hTest) == LT_YES) return LTTRUE;


	// Test the flags to see if this object is visible
	uint32 nFlags;
	g_pLTClient->Common()->GetObjectFlags(hTest, OFT_Flags, nFlags);

	if(!(nFlags & FLAG_VISIBLE))
		return LTFALSE;


	// Test the user flags to see if this is a character
	uint32 nUserFlags;
	g_pLTClient->GetObjectUserFlags(hTest, &nUserFlags);

	if(!(nUserFlags & USRFLG_CHARACTER) || (nUserFlags & USRFLG_CHAR_CLOAKED))
		return LTFALSE;


	// Filters out objects for a raycast.  pUserData is a list of HOBJECTS terminated
	// with a NULL HOBJECT.
	HOBJECT *hList = (HOBJECT*)pUserData;

	while(hList && *hList)
	{
		if(hTest == *hList)
			return DFALSE;

		++hList;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::UpdateInfoString()
{
	// Check for cases that we don't want to do this...
	if(m_InterfaceMgr.GetGameState() != GS_PLAYING) return LTFALSE;
	
	// [KLS] 9/23/2001 Modified this so it will allow player's in clip mode
	// to get info on other players...

	if (m_ePlayerState != PS_ALIVE)
	{
	    if (!(m_ePlayerState == PS_OBSERVING && 
			  m_PlayerMovement.GetMovementState() == CMS_CLIPPING))
		{
			return LTFALSE;
		}
	}

	if((IsMultiplayerGame() && g_vtMPInfoString.GetFloat()) || g_vtInfoString.GetFloat())
	{
		// Do an intersect straight ahead from the first person camera...
		HOBJECT hFilterList[] = { g_pLTClient->GetClientObject(), m_PlayerMovement.GetObject(), DNULL };

		ClientIntersectInfo info;
		ClientIntersectQuery query;
		memset(&query, 0, sizeof(query));

		query.m_Flags = INTERSECT_OBJECTS;
		query.m_FilterFn = ObjListFilterFn_CharactersOnly;
		query.m_pUserData = hFilterList;

		g_pCameraMgr->GetCameraPos(m_hPlayerCamera, query.m_From, LTTRUE);

		LTRotation rRot;
		g_pCameraMgr->GetCameraRotation(m_hPlayerCamera, rRot, LTTRUE);

		LTVector vR, vU, vF;
		g_pMathLT->GetRotationVectors(rRot, vR, vU, vF);
		query.m_To = query.m_From + (vF * 5000.0f);


		if(g_pLTClient->IntersectSegment(&query, &info))
		{
			HOBJECT hTestObj = info.m_hObject;
			if(!hTestObj) return LTFALSE;


			// Find the list of FX related to characters
			CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);
			if(!pList) 
				return LTFALSE;

			for( CSpecialFXList::Iterator iter = pList->Begin();
				 iter != pList->End(); ++iter)
			{
				CCharacterFX *pFX = (CCharacterFX*)iter->pSFX;

				if(pFX && (pFX->GetServerObj() == hTestObj))
				{
					LTBOOL bDisplayInfo = LTTRUE;

					// See if we are filtering...
					if(m_pMPMgr && m_pMPMgr->ShowFriendlyNamesOnly())
					{
						// See if this player is friendly before allowing
						// the name to be displayed
						if(pFX->m_cs.nClientID)
							bDisplayInfo = m_pMPMgr->IsFriendly(pFX->m_cs.nClientID);
					}

					if(bDisplayInfo)
					{
					if(pFX->m_cs.nClientID)
						m_pMPMgr->SetInfoString(pFX->m_cs.nClientID);
					else
						m_pMPMgr->SetInfoString(pFX->GetInfoString());

					return LTTRUE;
					}
				}
			}
		}
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleWeaponKickBack()
//
//	PURPOSE:	Handle changing the player view when the weapon kicks back
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleWeaponKickBack()
{
	uint32 nFlags = (m_PlayerMovement.GetLocalControlFlags() | CM_KICK_BACK);

	m_PlayerMovement.SetLocalControlFlags(nFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SendFlareTossMessage()
//
//	PURPOSE:	Handle sending a flare toss message to the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SendFlareTossMessage()
{
	//get ammo info
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo("Flare_Pouch_Ammo");
	if(!pAmmo) return;
	uint32 nAmmoId		= pAmmo->nId;

	//check ammo amount
	if(GetPlayerStats()->GetAmmoCount((uint8)nAmmoId) > 0)
	{
		//set up variables for fire message
		LTVector vFlashPos(0,0,0);
		LTVector vFirePos;
		g_pLTClient->GetObjectPos(GetPlayerCameraObject(), &vFirePos);

		LTRotation rRot;
		g_pLTClient->GetObjectRotation(GetPlayerCameraObject(), &rRot);

		LTVector vF, vNull;
		g_pInterface->GetRotationVectors(&rRot, &vNull, &vNull, &vF);
		uint8 nRandomSeed	= GetRandom(2, 255);

		WEAPON* pWeap = g_pWeaponMgr->GetWeapon("Flare_Pouch_Weapon");
		if(!pWeap) return;
		uint32 nWeaponId	= pWeap->nId;

		BARREL* pBarrel = g_pWeaponMgr->GetBarrel("Flare_Pouch_Barrel");
		if(!pBarrel) return;
		uint32 nBarrelId	= pBarrel->nId;

		//send fire message
		HMESSAGEWRITE hWrite = g_pInterface->StartMessage(MID_WEAPON_FIRE);
		g_pInterface->WriteToMessageVector(hWrite, &vFlashPos);
		g_pInterface->WriteToMessageVector(hWrite, &vFirePos);
		g_pInterface->WriteToMessageVector(hWrite, &vF);
		g_pInterface->WriteToMessageByte(hWrite, nRandomSeed);
		g_pInterface->WriteToMessageByte(hWrite, (uint8)nWeaponId);
		g_pInterface->WriteToMessageByte(hWrite, (uint8)nBarrelId);
		g_pInterface->WriteToMessageByte(hWrite, (uint8)nAmmoId);
		g_pInterface->WriteToMessageByte(hWrite, LTFALSE);
		g_pInterface->WriteToMessageByte(hWrite, 0);
		g_pInterface->WriteToMessageObject(hWrite, LTNULL);
		g_pInterface->EndMessage2(hWrite, MESSAGE_NAGGLEFAST);
	}
}


Species CGameClientShell::GetPlayerSpecies()
{
	if (IsAlien(GetPlayerStats()->GetButeSet()))
		return Alien;
	else if (IsPredator(GetPlayerStats()->GetButeSet()))
		return Predator;

	return Marine;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandlePounceJump()
//
//	PURPOSE:	Handle sending a pounce jump message...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandlePounceJump()
{
	if(m_PlayerMovement.GetCharacterButes()->m_bCanPounce)
	{
		LTFLOAT fTime = g_pLTClient->GetTime();

		if(m_PlayerMovement.CanPounce() && m_InterfaceMgr.GetGameState() == GS_PLAYING )
		{
			// Set to true if we can pounce.
			LTBOOL bPounced = LTFALSE;

			// See if it is time to pounce
			if(fTime > m_fLastPounceTime + g_vtPounceDelay)
			{
				// Check the dims area to see if we have enough room to jump
//				if(m_PlayerMovement.CheckDimsSpace(m_PlayerMovement.GetDimsSetting(CM_DIMS_DEFAULT)))
//				{
					// Now force the weapon...
					WEAPON* pWep = g_pWeaponMgr->GetWeapon(m_PlayerMovement.GetCharacterButes()->m_szPounceWeapon);

					if(pWep)
					{
						m_weaponModel.ForceChangeWeapon(pWep->nId, LTFALSE);
						m_PlayerMovement.SetMovementState(CMS_PRE_POUNCE_JUMP);
					}

					m_fLastPounceTime = fTime;

					// We got the pounce off.
					bPounced = LTTRUE;
//				}
			}

			// Check if we got the pounce off.
			if( !bPounced )
			{
				// Play the failed pounce jump sound.
				;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CanSpringJump()
//
//	PURPOSE:	Is it time to spring jump...
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::CanSpringJump()
{
	LTFLOAT fTime = g_pLTClient->GetTime();

	LTFLOAT fDealy;

	switch(GetPlayerSpecies())
	{
	case (Alien):		fDealy = g_vtAlienSpringDelay;	break;
	case (Predator):	fDealy = g_vtPredatorSpringDelay;break;
	default:			fDealy = g_vtHumanSpringDelay;	break;
	}

	if(fTime > m_fLastSpringJumpTime + fDealy)
	{
		m_fLastSpringJumpTime = fTime;

		return LTTRUE;
	}

	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMediComp()
//
//	PURPOSE:	Handle starting the MediComp stuff...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMediComp()
{
	// Now force the weapon...
	WEAPON* pMedicomp	= g_pWeaponMgr->GetWeapon("Medicomp");
	WEAPON* pSift		= g_pWeaponMgr->GetWeapon("Energy_Sift");
	WEAPON* pCurWep		= m_weaponModel.GetWeapon(); 

	if(pMedicomp && pSift != pCurWep && pMedicomp != pCurWep)
	{
		m_weaponModel.ForceChangeWeapon(pMedicomp->nId);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleEnergySift()
//
//	PURPOSE:	Handle starting the Energy Sift stuff...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleEnergySift()
{
	// Now force the weapon...
	WEAPON* pMedicomp	= g_pWeaponMgr->GetWeapon("Medicomp");
	WEAPON* pSift		= g_pWeaponMgr->GetWeapon("Energy_Sift");
	WEAPON* pCurWep		= m_weaponModel.GetWeapon(); 

	if(pSift && pSift != pCurWep && pMedicomp != pCurWep)
	{
		m_weaponModel.ForceChangeWeapon(pSift->nId);
	}
}


void* CGameClientShell::GetAuthContext()
{
	CFolderMulti *pMulti = (CFolderMulti*)GetInterfaceMgr()->GetFolderMgr()->GetFolderFromID(FOLDER_ID_MULTI);
	return pMulti->GetGameSpyMgr()->GetAuthContext();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateSpeedMonitor()
//
//	PURPOSE:	Handle updating the server speed monitor...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateSpeedMonitor()
{
	LTFLOAT fLocalTime = g_pLTClient->GetTime();
	LTFLOAT fServerTime = g_pLTClient->GetGameTime();
	if(fServerTime - m_fInitialServerTime > 5.0f)
	{
		// time to do our check
		LTFLOAT fServerDelta	= fServerTime - m_fInitialServerTime;
		LTFLOAT fLocalDelta		= fLocalTime - m_fInitialLocalTime;

		if(fServerDelta / fLocalDelta < 0.98)
		{
			// possible cheater, increment cheat counter
			m_nSpeedCheatCounter++;

			if(m_nSpeedCheatCounter > 24)
			{
				// send the Message to the server
				HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_SPEED_CHEAT);
				g_pLTClient->EndMessage(hMsg);
			}
		}
		else
		{
			// reset the instance counter
			m_nSpeedCheatCounter = 0;
		}

		// reset the time counters
		m_fInitialServerTime	= fServerTime;
		m_fInitialLocalTime		= fLocalTime;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::MPChangeWeapon(uint8 nCommandId)
{
	DBYTE nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return;

	// Force a change to the approprite weapon... 

	CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
	if (pStats)
	{
		ChangeWeapon(nWeaponId, LTTRUE, LTFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ReadMPInterfaceUpdate()
//
//	PURPOSE:	Read in our MP interface update stuff
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ReadMPInterfaceUpdate(HMESSAGEREAD hMessage)
{
	// Read in how many ammo counts we need to update
	uint8 nCount = g_pLTClient->ReadFromMessageByte(hMessage);

	// Now adjust our inventory
	while(nCount)
	{
		uint8	nId		= g_pLTClient->ReadFromMessageByte(hMessage);
		LTFLOAT fAmt	= g_pLTClient->ReadFromMessageFloat(hMessage);

		m_InterfaceMgr.GetPlayerStats()->UpdateAmmo(nId, (uint32)fAmt, LTFALSE, LTFALSE);

		// Decrement
		nCount--;
	}

	// Read in our new max health
	LTFLOAT fValue = g_pLTClient->ReadFromMessageFloat(hMessage);
	m_InterfaceMgr.GetPlayerStats()->UpdateMaxHealth((uint32)fValue);

	// Now our normal health
	fValue = g_pLTClient->ReadFromMessageFloat(hMessage);
	m_InterfaceMgr.GetPlayerStats()->UpdateHealth((uint32)fValue);

	// Read in our new max armor
	fValue = g_pLTClient->ReadFromMessageFloat(hMessage);
	m_InterfaceMgr.GetPlayerStats()->UpdateMaxArmor((uint32)fValue);

	// Now our normal health
	fValue = g_pLTClient->ReadFromMessageFloat(hMessage);
	m_InterfaceMgr.GetPlayerStats()->UpdateArmor((uint32)fValue);

	// Update our air level
	fValue = g_pLTClient->ReadFromMessageFloat(hMessage);
	m_InterfaceMgr.GetPlayerStats()->UpdateAir(fValue);

	// Update our sc charge level
	fValue = g_pLTClient->ReadFromMessageFloat(hMessage);
	m_InterfaceMgr.GetPlayerStats()->UpdateCannonChargeLev((uint32)fValue);

	// Update our battery charge level
	fValue = g_pLTClient->ReadFromMessageFloat(hMessage);
	m_InterfaceMgr.GetPlayerStats()->UpdateBatteryLevel(fValue);
}
