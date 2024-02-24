// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.cpp
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "ClientRes.h"
#include "WeaponStringDefs.h"
#include "VKDefs.h"
#include "SoundMgr.h"
#include "InterfaceResMgr.h"
#include "SCDefs.h"
#include "VarTrack.h"
#include "WinUtil.h"
#include "MissionMgr.h"
#include "ProfileMgr.h"
#include "ServerOptionMgr.h"
#include "CharacterFuncs.h"
#include "ClientButeMgr.h"  // For debug keys.
#include "MultiplayerClientMgr.h"
#include "FolderEscape.h"

#define AO_DEFAULT_ENABLED		(AO_MUSIC | AO_SOUND | AO_MOVIES | AO_LIGHTMAP | AO_FOG | AO_LINES | AO_MODELFULLBRITE | AO_JOYSTICK | AO_TRIPLEBUFFER)


extern CGameClientShell* g_pGameClientShell;
extern CObjectiveButeMgr* g_pObjectiveButeMgr;

CInterfaceMgr*	g_pInterfaceMgr = DNULL;

#define IM_SPLASH_SOUND		"Sounds\\Interface\\splash.wav"
#define IM_SPLASH_SCREEN	"Interface\\splash.pcx"

#define MAX_FRAME_DELTA		0.1f

#define ACTIVATE_FILTER (USRFLG_CAN_ACTIVATE|USRFLG_WELD_OBJECT|USRFLG_CUT_OBJECT|USRFLG_HACK_OBJECT|USRFLG_DETONATE_OBJECT)

const DWORD dwInit	= 0x773593FF;
const DWORD dwInitEnc = 0x5D398C86;

const DWORD dwMask1 = 0x2A0C1F79;
const DWORD dwMask2 = 0x18923D01;
const DWORD dwMask3 = 0x1C1EAD4D;


#ifdef _DEMO
	// Globals for demo use only
	uint8 g_nSplashCount;
#endif

DFLOAT		g_fSplashSndDuration = 0.0f;
DFLOAT		g_fFailScreenDuration = 0.0f;

DFLOAT		g_fFovXTan = 0.0f;
DFLOAT		g_fFovYTan = 0.0f;


DVector		g_vOverlayScale(0.02f, 0.02f, 1.0f);
DFLOAT		g_vOverlayDist = 10.0f;
DBYTE		nOverlayCount = 0;
DBOOL		bBlackScreen = DFALSE;
DFLOAT		fBlackScreenTime = 0.0f;


HOBJECT		hOverlays[NUM_OVERLAY_MASKS];
DFLOAT		g_vOverlayScaleMult[NUM_OVERLAY_MASKS];

HSURFACE	g_hSplash = DNULL;
DBOOL		bDidFadeOut = DFALSE;
LTFLOAT		fFadeInStart = 0.0f;

HDECOLOR	hDefaultTransColor = SETRGB_T(0,0,0);

VarTrack	g_vtDrawInterface;
VarTrack	g_vtObjectiveFlashCount;

VarTrack	g_vtDamageFlashTime;

VarTrack	g_vtMissionFailed;

VarTrack	g_vtCustomDifficulty;

VarTrack	g_vtSplashLockTime;

// NOLF Stuff
LTFLOAT      g_fBackDist = 200.0f;
LTVector	g_vBaseBackScale(0.8f, 0.6f, 1.0f);

VarTrack	g_vtDisableMovies;
VarTrack	g_vtDebugKeys;

#define MAX_INTERFACE_SFX	50
#define INVALID_ANI			((HMODELANIM)-1)

#define MOVIE_FOX_LOGO			0
#define MOVIE_SIERRA_LOGO		1
#define MOVIE_LITHTECH_LOGO		2
#define MOVIE_MONOLITH_LOGO		3
#define MOVIE_AVP_INTRO			4


const char* c_GameStateNames[] =
{
	"GS_UNDEFINED",
	"GS_PLAYING",
	"GS_LOADINGLEVEL",
	"GS_SPLASHSCREEN",
	"GS_DIALOGUE",
	"GS_MENU",
	"GS_FOLDER",
	"GS_PAUSED",
	"GS_FAILURE",
	"GS_MOVIE"
};

// NOLF
LTBOOL g_bLockFolder = LTFALSE;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CInterfaceMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CInterfaceMgr::CInterfaceMgr()
{
	g_pInterfaceMgr	= this;

	m_hCursor				= LTNULL;

	m_eGameState			= GS_UNDEFINED;
	m_eCurrOverlay			= OVM_NONE;
	m_dwAdvancedOptions		= AO_DEFAULT_ENABLED;
	m_dwOrignallyEnabled	= 0;
	m_nClearScreenCount		= 0;
	m_bClearScreenAlways	= DFALSE;
	m_fMenuSaveFOVx			= 0.0f;
	m_fMenuSaveFOVy			= 0.0f;
	m_bDrawPlayerStats		= DTRUE;
	m_bDrawCloseCaption		= DTRUE;
	m_bDrawFragCount		= DFALSE;
	m_bDrawInterface		= DTRUE;
	m_bGameMusicPaused		= DFALSE;
	m_bSwitchingModes		= DFALSE;
	m_hMenuMusic			= DNULL;
	m_szMenuLoop[0]			= LTNULL;
	m_hSplashSound			= DNULL;
	g_fSplashSndDuration	= 0.0f;
	m_hGamePausedSurface	= DNULL;
	m_nFailStringId			= DNULL;
	m_hFailBack				= DNULL;
	m_bMenuRectRestore		= DTRUE;
	m_hScubaSound			= DNULL;
	m_hFadeSurface			= DNULL;
	m_bScreenFade			= DFALSE;
	m_bFadeInitialized		= DFALSE;
	m_fTotalFadeTime		= 0.0f;
	m_fCurFadeTime			= 0.0f;
	m_bFadeIn				= DTRUE;
	m_bExitAfterFade		= DFALSE;
	m_bUseCursor			= DFALSE;
	//new for avp3
	m_hPauseBackground		= LTNULL;
	m_pWeaponChooser		= LTNULL;
	m_pCrosshairMgr			= LTNULL;
	m_pAutoTargetMgr		= LTNULL;
	m_nPrevMissionId		= -1;
	m_hLoading				= LTNULL;
	m_bDisplayObjectives	= LTFALSE;
	m_bObjectiveNotification= LTFALSE;
	m_bPredVisionTransition	= LTFALSE;
	m_bLoadFailed			= LTFALSE;
	m_bOldFogEnable			= LTTRUE;
	m_fAlertStartTime		= 0.0f;
	m_nObjectiveFlashCount	= 0;
	m_bDrawAlertSolid		= LTFALSE;

	m_hMovie			= LTNULL;
	m_nCurMovie			= 0;
	m_hBackground		= LTNULL;
	m_fSplashStartTime	= 0;


	m_eIS = IS_NONE;

	// NOLF
	m_bUseInterfaceCamera	= LTTRUE;

	memset(&m_rcMenuRestoreCamera, 0, sizeof(m_rcMenuRestoreCamera));

#ifdef _DEMO
	memset(m_hScreenShow, 0, 5*sizeof(HSURFACE));
#endif

	for (int i =0; i < NUM_OVERLAY_MASKS; i++)
	{
		hOverlays[i] = DNULL;
		g_vOverlayScaleMult[i] = 1.0f;
	}

	m_hObjectiveSound		= LTNULL;
	m_hObjectiveAlertSound	= LTNULL;

	m_hObjBackground		= LTNULL;
	m_hAlertImage			= LTNULL;
	
	for (i =0; i < 3; i++)
		m_hObjCheckBox[i] = LTNULL;

	m_bPlayedOutro			= LTFALSE;

	m_bCinematicSkipKey		= LTFALSE;
	m_fCinematicSkipDelay	= 0.0f;

	m_nLoadWorldCount = 0;
	m_nOldLoadWorldCount = 0;

	m_nDisconnectError = -1;
	m_nHudType			= -1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::~CInterfaceMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CInterfaceMgr::~CInterfaceMgr()
{
	SetMenuMusic(DFALSE);

	if (g_hSplash)
	{
		g_pClientDE->DeleteSurface(g_hSplash);
		g_hSplash = DNULL;
	}

	if (m_hSplashSound)
	{
		g_pClientDE->KillSound(m_hSplashSound);
		m_hSplashSound = DNULL;
	}

	if (m_hGamePausedSurface) 
	{
		g_pClientDE->DeleteSurface(m_hGamePausedSurface);
		m_hGamePausedSurface = DNULL;
	}

	if (m_hFailBack)
	{
		g_pClientDE->DeleteSurface(m_hFailBack);
		m_hFailBack = DNULL;
	}

#ifdef _DEMO
	if(m_hScreenShow[0]) g_pClientDE->DeleteSurface(m_hScreenShow[0]);
	if(m_hScreenShow[1]) g_pClientDE->DeleteSurface(m_hScreenShow[1]);
	if(m_hScreenShow[2]) g_pClientDE->DeleteSurface(m_hScreenShow[2]);
	if(m_hScreenShow[3]) g_pClientDE->DeleteSurface(m_hScreenShow[3]);
	if(m_hScreenShow[4]) g_pClientDE->DeleteSurface(m_hScreenShow[4]);
	memset(m_hScreenShow, 0, 5*sizeof(HSURFACE));
#endif

	if (m_hFadeSurface)
	{
		g_pClientDE->DeleteSurface(m_hFadeSurface);
		m_hFadeSurface = DNULL;
	}

	if (m_hLoading)
	{
		g_pClientDE->DeleteSurface(m_hLoading);
		m_hLoading = DNULL;
	}

	if(m_pAutoTargetMgr)
	{
		delete m_pAutoTargetMgr;
		m_pAutoTargetMgr = LTNULL;
	}

	if (m_hPauseBackground)
	{
		g_pClientDE->DeleteSurface(m_hPauseBackground);
		m_hPauseBackground = DNULL;
	}

	if(m_pCrosshairMgr)
	{
		delete m_pCrosshairMgr;
		m_pCrosshairMgr = LTNULL;
	}

	TermWepChooser();

	g_pInterfaceMgr = DNULL;

	//clean up any objective stuff that may be hanging around
	ClearObjectiveDisplay();
}
	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::Init()
{
	if (g_pClientDE->Cursor()->LoadCursorBitmapResource(MAKEINTRESOURCE(IDC_POINTER), m_hCursor) != LT_OK)
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Cursor!");
		return DFALSE;
	}
	
	if (g_pClientDE->Cursor()->SetCursor(m_hCursor) != LT_OK)
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Cursor!");
		return DFALSE;
	}

	g_vtDrawInterface.Init(g_pClientDE, "DrawInterface", NULL, 1.0f);
	g_vtObjectiveFlashCount.Init(g_pClientDE, "ObjectiveFlashCount", NULL, 5.0f);
	g_vtDamageFlashTime.Init(g_pClientDE, "DamageFlashTime", NULL, 2.0f);
	g_vtDebugKeys.Init(g_pClientDE, "DebugKeys", NULL, 1.0f);
	g_vtMissionFailed.Init(g_pLTClient, "MissionFailed", NULL, 1.0f);
	g_vtDisableMovies.Init(g_pLTClient, "NoMovies", NULL, 0.0f);
	g_vtCustomDifficulty.Init(g_pLTClient, "CustomDifficulty", NULL, 1.0f);

	g_vtSplashLockTime.Init(g_pLTClient, "SplashLockTime", NULL, 4.0f);

	ProcessAdvancedOptions();

	if(!m_hBackground)
	{
		m_hBackground = g_pLTClient->CreateSurfaceFromBitmap("Interface\\MP\\background.pcx");

		// Fill in the backgrounds
		HLTCOLOR hTransColor = SETRGB_T(0, 0, 0);
		g_pLTClient->FillRect(m_hBackground, LTNULL, SETRGB(144, 144, 224));
		g_pLTClient->OptimizeSurface(m_hBackground, hTransColor);
	}

	// read in the settings
	m_Settings.Init (g_pClientDE, g_pGameClientShell);


	if (!m_LayoutMgr.Init(g_pLTClient))
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize LayoutMgr!");
		return DFALSE;
	}


	if (!m_InterfaceResMgr.Init(g_pClientDE, g_pGameClientShell))
	{
		// If we couldn't init, something critical must have happened (like no render dlls)

		g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize InterfaceResMgr!");
		return DFALSE;
	}


	// NOLFs
    if (!m_FolderMgr.Init())
	{
		// If we couldn't init, something critical must have happened
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize FolderMgr!");
        return LTFALSE;
	}


	//initialize the crosshair manager
	if(!m_pCrosshairMgr)
	{
		m_pCrosshairMgr = new CCrosshairMgr;
		if(m_pCrosshairMgr)
			if(!m_pCrosshairMgr->Init())
			{
				g_pClientDE->ShutdownWithMessage("ERROR in CCrosshairMgr::Init():  Could not initialize Crosshair Manager!");
				return DFALSE;
			}
	}


	if(!m_pAutoTargetMgr)
	{
		//create a pointer to our menu manager object
		m_pAutoTargetMgr = new CAutoTargetMgr;

		//run the initialization routine
		if (!m_pAutoTargetMgr)
		{
			// If we couldn't init, something critical must have happened
			g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Auto-Target Manager!");
			return DFALSE;
		}

		m_pAutoTargetMgr->Init(TT_NORMAL);
	}



	m_messageMgr.Init();
	m_messageMgr.SetEnabled(DTRUE);

	if (!m_stats.Init())
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Player Stats!");
		return DFALSE;
	}


	// Create the main wnd (this is necessary to use the dialog wnd below)...

	if (!m_MainWnd.Init(0,"Main Window",DNULL,g_pClientDE->GetScreenSurface()))
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Main Wnd!");
		return DFALSE;
	}

	if (!m_DialogueWnd.InitFromBitmap(0,"Dialog Window", &m_MainWnd, "Interface\\DlgWnd\\DlgBack.pcx", 
		"Interface\\DlgWnd\\DecisionBack.pcx", "Interface\\DlgWnd\\dialogueframe", m_InterfaceResMgr.GetSmallFont(), 50, 50, LTWF_SIZEABLE,LTWS_CLOSED))
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Dialog Wnd!");
		return DFALSE;
	}

	m_DialogueWnd.SetImmediateDecisions(DTRUE);

	m_CursorPos.x = 0;
	m_CursorPos.y = 0;

	g_fFovXTan = (float)tan(MATH_DEGREES_TO_RADIANS(FOVX_NORMAL)/2);
	g_fFovYTan = (float)tan(MATH_DEGREES_TO_RADIANS(FOVY_NORMAL)/2);

	m_MessageBox.Init();
	m_Subtitle.Init();


	//initialize ProfileMgr with default profile
	g_pProfileMgr = new CProfileMgr;
	if( !g_pProfileMgr )
		return DFALSE;

	if (!g_pProfileMgr->Init()) 
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CProfileMgr::Init():  Could not initialize ProfileMgr!");
		delete g_pProfileMgr;
		g_pProfileMgr = NULL;
		return DFALSE;
	}


	//initialize ServerOptionMgr with default options
	g_pServerOptionMgr = new CServerOptionMgr;
	if( !g_pServerOptionMgr )
		return DFALSE;

	if (!g_pServerOptionMgr->Init()) 
	{
		g_pClientDE->ShutdownWithMessage("ERROR in CServerOptionMgr::Init():  Could not initialize ServerOptionMgr!");
		g_pServerOptionMgr->Term();
		delete g_pServerOptionMgr;
		return DFALSE;
	}

	m_LayoutMgr.GetSliderLeftSound(m_szSliderLeftSound,sizeof(m_szSliderLeftSound));
	m_LayoutMgr.GetSliderRightSound(m_szSliderRightSound,sizeof(m_szSliderRightSound));
	m_LayoutMgr.GetUnselectableSound(m_szUnselectableSound,sizeof(m_szUnselectableSound));
	m_LayoutMgr.GetEscapeSound(m_szEscapeSound,sizeof(m_szEscapeSound));

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Term()
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::Term()
{
	// NOLF
	m_FolderMgr.Term();

	m_LayoutMgr.Term();
	m_messageMgr.Term();
	m_stats.Term();
	m_Credits.Term();
	m_Subtitle.Clear();


	TermWepChooser();
	
	//	m_AmmoChooser.Term();
	m_InterfaceResMgr.Term();
	m_DialogueWnd.Term();
	m_MainWnd.Term();
	m_MessageBox.Term();

	if( g_pProfileMgr )
	{
		g_pProfileMgr->Term();
		delete g_pProfileMgr;
		g_pProfileMgr = NULL;
	}

	if (m_hCursor)
	{
		g_pLTClient->Cursor()->FreeCursor(m_hCursor);
		m_hCursor = LTNULL;
	}

	if ((m_dwOrignallyEnabled & AO_SOUND) && !(m_dwAdvancedOptions & AO_SOUND))
	{
		g_pClientDE->RunConsoleString("SoundEnable 1");
	}

	if ((m_dwOrignallyEnabled & AO_MUSIC) && !(m_dwAdvancedOptions & AO_MUSIC))
	{
		g_pClientDE->RunConsoleString("MusicEnable 1");
	}

	if (m_dwOrignallyEnabled & AO_MODELFULLBRITE)
	{
		g_pClientDE->RunConsoleString("ModelFullbrite 1");
	}

	if (m_dwOrignallyEnabled & AO_LIGHTMAP)
	{
		g_pClientDE->RunConsoleString("LightMap 1");
	}

	if(m_hObjectiveSound)
	{
		//kill the typing sound
		g_pLTClient->KillSound(m_hObjectiveSound);
		m_hObjectiveSound=LTNULL;
	}

	// Free the surfaces
	if(m_hBackground)
	{
		g_pLTClient->DeleteSurface(m_hBackground);
		m_hBackground = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ResetSharedSurfaces()
//
//	PURPOSE:	Makes sure shared surfaces are cleared or initted for specific
//				character types
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::ResetSharedSurfaces()
{
	// If this is a predator character... preload this graphic
	CharacterButes*	pButes = g_pGameClientShell->GetPlayerMovement()->GetCharacterButes();

	if(pButes && IsPredator(pButes->m_nId))
	{
		g_pInterfaceResMgr->GetSharedSurface("interface\\statusbar\\predator\\screenwipe.pcx");
	}
	else
	{
		g_pInterfaceResMgr->FreeSharedSurface("interface\\statusbar\\predator\\screenwipe.pcx");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering new world
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnEnterWorld(DBOOL bRestoringGame)
{
	m_stats.OnEnterWorld(bRestoringGame);

	// Clear the objective stuff...
	if(!bRestoringGame)
	{
		ClearObjectiveAlert();
		CreateObjectiveDisplay();

		ViewModeMGR* pVM = g_pGameClientShell->GetVisionModeMgr();

		if(pVM)
		{
			pVM->SetNetMode(LTFALSE);
			pVM->SetRailMode(LTFALSE);
			pVM->SetPredZoomMode(LTFALSE);
			pVM->SetOnFireMode(LTFALSE);
			pVM->SetEvacMode(LTFALSE);
			pVM->SetOrientMode(LTFALSE);
		}
	}

	m_bCinematicSkipKey = LTFALSE;

//	InitWepChooser();

	++m_nLoadWorldCount;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnExitWorld()
{
	m_stats.OnExitWorld();

	for (int i =0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (hOverlays[i])
			g_pClientDE->DeleteObject(hOverlays[i]);

		hOverlays[i] = DNULL;
	}

	m_Credits.Term();
	m_Subtitle.Clear();
	m_HudMgr.Reset();
	m_nHudType = -1;

	m_bFadeInitialized = DFALSE;
	m_bExitAfterFade = DFALSE;

	m_InterfaceResMgr.Clean();

	g_pGameClientShell->GetMusic()->Stop();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreUpdate()
//
//	PURPOSE:	Handle pre-updates
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PreUpdate()
{
	if (m_LoadingScreen.IsVisible())
		return LTTRUE;

	if (m_bClearScreenAlways)
	{
		g_pClientDE->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	}
	else if (m_nClearScreenCount)
	{
		g_pClientDE->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);
		m_nClearScreenCount--;

		// Clear the farz to the void color
		LTVector vVoidColor = g_pGameClientShell->GetVoidColor();
		g_pClientDE->ClearScreen(LTNULL, CLEARSCREEN_RENDER, &vVoidColor);
	}
	else
	{
		// Clear the screen to the void color
		LTVector vVoidColor = g_pGameClientShell->GetVoidColor();
		g_pClientDE->ClearScreen(LTNULL, CLEARSCREEN_RENDER, &vVoidColor);
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostUpdate()
//
//	PURPOSE:	Handle post-updates (return DTRUE to FLIP
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostUpdate()
{
	LTBOOL bCanFlipScreen = LTTRUE;

	// [KLS] Check for special case of loading a multiplayer game where we don't
	// want to render the screen until the player is in the world...

	if (g_pGameClientShell->GetGameType()->IsMultiplayer())
	{
		if (g_pGameClientShell->GetMultiplayerMgr()->IsLoadingDisplay() && 
			GS_PLAYING == m_eGameState && !g_pGameClientShell->IsPlayerInWorld())
		{
			bCanFlipScreen = LTFALSE;
		}
	}

	if (m_eGameState != GS_LOADINGLEVEL && bCanFlipScreen)
	{
		g_pClientDE->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Update()
//
//	PURPOSE:	Handle updating the interface
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::Update()
{
	// Update based on the game state...
	DBOOL bHandled = DFALSE;
	switch (m_eGameState)
	{
		case GS_PLAYING:
		{
			UpdatePlayingState();
			bHandled = DFALSE;  // Allow further processing
		}
		break;

		case GS_DIALOGUE:
		{
			UpdateDialogueState();
			bHandled = DFALSE;  // Allow further processing
		}
		break;

		case GS_LOADINGLEVEL:
		{
			UpdateLoadingLevelState();
			bHandled = DTRUE;
		}
		break;

		case GS_FOLDER:
		case GS_MENU :
		{
			UpdateFolderState();
			bHandled = DTRUE;
		}
		break;

		case GS_PAUSED:
		{
			UpdatePausedState();
			bHandled = DTRUE;
		}
		break;
		
		case GS_SPLASHSCREEN:
		{

			if(m_fSplashStartTime == 0)
			{
				// Mark the start time...
				m_fSplashStartTime = g_pLTClient->GetTime();
			}
			UpdateSplashScreenState();
			bHandled = DTRUE;
		}
		break;
		
		case GS_FAILURE:
		{
			UpdateFailureState();
			bHandled = DTRUE;
		}
		break;
		
		case GS_MOVIE:
		{
			UpdateMovieState();
            bHandled = LTTRUE;
		}
		break;
	}

	//in playing state, message box is drawn in InterfaceMgr::Draw(), 
	// otherwise draw it here
	if (m_eGameState != GS_PLAYING && m_MessageBox.IsVisible())
	{
        g_pLTClient->Start3D();
        g_pLTClient->StartOptimized2D();
        m_MessageBox.Draw(g_pLTClient->GetScreenSurface());
        g_pLTClient->EndOptimized2D();
        g_pLTClient->End3D();
	}

	// Handle the cinematic skip delay time
	if(!m_bCinematicSkipKey && (m_fCinematicSkipDelay > 0.0f))
		m_fCinematicSkipDelay -= g_pLTClient->GetFrameTime();

	return bHandled;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateWeaponStats()
//
//	PURPOSE:	Update the player's weapon stats display
//
// ----------------------------------------------------------------------- //
/*
void CInterfaceMgr::UpdateWeaponStats(DBYTE nWeaponId, DBYTE nAmmoId, DDWORD dwAmmo)
{
//	m_stats.UpdateAmmo(nWeaponId, nAmmoId, dwAmmo);
//	m_stats.UpdatePlayerWeapon(nWeaponId, nAmmoId);
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateFolderState()
//
//	PURPOSE:	Update folder state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateFolderState()
{
	g_pClientDE->Start3D();

	// NOLF
//	m_FolderMgr.UpdateInterfaceSFX();
	DrawSFX();

	g_pClientDE->StartOptimized2D();

	// NOLF
	if (m_eGameState == GS_FOLDER)
	{
		m_InterfaceResMgr.DrawFolder();
	}

	UpdateScreenFade();
	g_pClientDE->EndOptimized2D();
	g_pClientDE->End3D();

	if (!m_hMenuMusic)
	{
		SetMenuMusic(DTRUE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayingState()
//
//	PURPOSE:	Update playing state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePlayingState()
{
	//update the crosshair
	LTVector vCamPos;
	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	g_pLTClient->GetObjectPos(hCamera, &vCamPos);

	DRotation rCamRot;
	g_pLTClient->GetObjectRotation(hCamera, &rCamRot);

	DVector vU, vR, vF;
	g_pLTClient->GetRotationVectors(&rCamRot, &vU, &vR, &vF);

	UpdateActivate(vCamPos, vF);

	LTBOOL bIsDead = g_pGameClientShell->GetPlayerStats()->GetHealthCount()==0;

	if(m_pAutoTargetMgr)
	{
		if(!bIsDead)
			m_pAutoTargetMgr->UpdateTargets();
		else
			m_pAutoTargetMgr->ResetSounds();
	}

	m_Subtitle.Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateDialogueState()
//
//	PURPOSE:	Update dialogue state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateDialogueState()
{
	// Update the main window...

	m_MainWnd.Update(g_pClientDE->GetFrameTime());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateLoadingLevelState()
//
//	PURPOSE:	Update loading level state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateLoadingLevelState()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if ((g_pGameClientShell->IsInWorld() && (m_nOldLoadWorldCount != m_nLoadWorldCount)) &&
		hPlayerObj)
	{
		// Turn off the loading screen
		m_LoadingScreen.Hide();

		ChangeState(GS_PLAYING);
	}
	else if ((m_bLoadFailed) || (g_pLTClient->IsConnected() && IsKeyDown(VK_ESCAPE)))
	{
		m_LoadingScreen.Hide();

		// If they were in the middle of connecting and aborted, disconnect
		if (g_pLTClient->IsConnected())
			g_pLTClient->Disconnect();

		SwitchToFolder(FOLDER_ID_MAIN);
		ChangeState(GS_FOLDER);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePausedState()
//
//	PURPOSE:	Update paused state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePausedState()
{
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();

	if (m_hPauseBackground)
	{
		g_pClientDE->DrawSurfaceToSurface(hScreen, m_hPauseBackground, DNULL, 0, 0);
	}

	if (m_hGamePausedSurface)
	{
		DDWORD nSurfaceWidth = 0, nSurfaceHeight = 0;
		DDWORD nScreenWidth, nScreenHeight;

		g_pClientDE->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);
		g_pClientDE->GetSurfaceDims(m_hGamePausedSurface, &nSurfaceWidth, &nSurfaceHeight);

		g_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hGamePausedSurface, DNULL, 
												   ((int)(nScreenWidth - nSurfaceWidth)) / 2, 
												   (((int)(nScreenHeight - nSurfaceHeight)) / 2) + 70, hDefaultTransColor);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePausedState()
//
//	PURPOSE:	Update paused state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateSplashScreenState()
{
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();
	DDWORD nWidth = 0;
	DDWORD nHeight = 0;

	g_pClientDE->GetSurfaceDims(hScreen, &nWidth, &nHeight);

	DRect rcDst;
	rcDst.Init(0, 0, nWidth, nHeight);

	g_pClientDE->GetSurfaceDims(g_hSplash, &nWidth, &nHeight);

	DRect rcSrc;
	rcSrc.Init(0, 0, nWidth, nHeight);

	g_pClientDE->ScaleSurfaceToSurface(hScreen, g_hSplash, &rcDst, &rcSrc);

	g_pClientDE->Start3D();
	g_pClientDE->StartOptimized2D();

	UpdateScreenFade();

	g_pClientDE->EndOptimized2D();
	g_pClientDE->End3D();

	if(fFadeInStart == 0.0f)
		fFadeInStart = g_pLTClient->GetTime();

	if (!m_bScreenFade)
	{
		if (!bDidFadeOut)
		{
			if(g_pLTClient->GetTime() > (fFadeInStart+g_vtSplashLockTime.GetFloat()))
			{
				StartScreenFadeOut(0.5);
				bDidFadeOut = DTRUE;
			}
		}
	}
	else if (bDidFadeOut && m_fCurFadeTime <= 0.0f)
	{
#ifdef _DEMO
		NextSplashScreen();
#else
		ChangeState(GS_MOVIE);
#endif
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateFailureState()
//
//	PURPOSE:	Update failure state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateFailureState()
{
	LTFLOAT fTime = g_pClientDE->GetFrameTime();
	g_fFailScreenDuration += fTime > 0.1f ? 0.1f : fTime;
	HSURFACE hScreen = g_pClientDE->GetScreenSurface();
	DDWORD nScrnWidth = 0;
	DDWORD nScrnHeight = 0;

	g_pClientDE->GetSurfaceDims(hScreen, &nScrnWidth, &nScrnHeight);

	DRect rcDst;
	rcDst.left = rcDst.top = 0;
	rcDst.right = nScrnWidth;
	rcDst.bottom = nScrnHeight;

	HSTRING hStr = (m_nFailStringId == -1) ? g_pLTClient->CreateString("Mission Complete") : g_pClientDE->FormatString(m_nFailStringId);

	DDWORD nWidth = 0;
	DDWORD nHeight = 0;
	if (m_hFailBack)
		g_pClientDE->GetSurfaceDims(m_hFailBack, &nWidth, &nHeight);
	DRect rcSrc;
	rcSrc.left = rcSrc.top = 0;
	rcSrc.right = nWidth;
	rcSrc.bottom = nHeight;


	g_pLTClient->Start3D();
	g_pLTClient->StartOptimized2D();


	if (m_hFailBack)
		g_pClientDE->ScaleSurfaceToSurface(hScreen, m_hFailBack, &rcDst, &rcSrc);
	else
		g_pClientDE->FillRect(hScreen, &rcDst, SETRGB(25,0,0));

	DIntPt failPos = g_pLayoutMgr->GetFailStringPos();
	failPos.x = (int)( (float)failPos.x * ( (float)nScrnWidth / 640.0f ) );
	failPos.y = (int)( (float)failPos.y * ( (float)nScrnHeight / 480.0f ) );
	CLTGUIFont *pFont = GetCharacterFont();

#ifdef _DEMO
	static LTBOOL bDrawString = LTTRUE;

	if(hStr && pFont && bDrawString)
		pFont->Draw(hStr,hScreen,failPos.x,failPos.y,LTF_JUSTIFY_CENTER);
#else
	if(hStr && pFont)
		pFont->Draw(hStr,hScreen,failPos.x,failPos.y,LTF_JUSTIFY_CENTER);
#endif


	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D();


	g_pClientDE->FreeString(hStr);

	if(g_fFailScreenDuration >= g_pLayoutMgr->GetFailScreenDelay())
	{
#ifdef _DEMO
		static uint8 screenIndex = 0;

		if(screenIndex == 0)
		{
			if(!m_hScreenShow[0]) m_hScreenShow[0] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\StatusBar\\screen1.pcx");
			if(!m_hScreenShow[1]) m_hScreenShow[1] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\StatusBar\\screen2.pcx");
//			if(!m_hScreenShow[2]) m_hScreenShow[2] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\StatusBar\\screen3.pcx");
//			if(!m_hScreenShow[3]) m_hScreenShow[3] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\StatusBar\\screen4.pcx");
//			if(!m_hScreenShow[4]) m_hScreenShow[4] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\StatusBar\\screen5.pcx");
		}

		++screenIndex;

		// Don't draw the string
		bDrawString = LTFALSE;

		// nuke the old screen
		if(m_hFailBack)
		{
			g_pLTClient->DeleteSurface(m_hFailBack);
			m_hFailBack = LTNULL;
		}

		switch ((const uint8)screenIndex)
		{
		case 1:	m_hFailBack = m_hScreenShow[0]; m_hScreenShow[0]=LTNULL; g_fFailScreenDuration=0.0f; return;
		case 2:	m_hFailBack = m_hScreenShow[1]; m_hScreenShow[1]=LTNULL; g_fFailScreenDuration=0.0f; return;
//		case 3:	m_hFailBack = m_hScreenShow[2]; m_hScreenShow[2]=LTNULL; g_fFailScreenDuration=0.0f; return;
//		case 4:	m_hFailBack = m_hScreenShow[3]; m_hScreenShow[3]=LTNULL; g_fFailScreenDuration=0.0f; return;
//		case 5:	m_hFailBack = m_hScreenShow[4]; m_hScreenShow[4]=LTNULL; g_fFailScreenDuration=0.0f; return;
		default: screenIndex=0; bDrawString=LTTRUE;
		}
#endif

		// Kill the music and go back to the menus
		CMusic*	pMusic = g_pGameClientShell->GetMusic();

		if(pMusic)
			pMusic->TermLevel();

#ifdef _DEMO
		m_FolderMgr.ClearFolderHistory();

		SwitchToFolder(FOLDER_ID_MAIN);
		StartScreenFadeIn(0.5f);
#else
		m_FolderMgr.ClearFolderHistory();
		m_FolderMgr.AddToFolderHistory(FOLDER_ID_MAIN);
		m_FolderMgr.AddToFolderHistory(FOLDER_ID_SINGLE);

		SwitchToFolder(FOLDER_ID_LOAD);
		StartScreenFadeIn(0.5f);
#endif
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateMovieState()
//
//	PURPOSE:	Update movie state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateMovieState()
{
	ILTVideoMgr* pVideoMgr = g_pLTClient->VideoMgr();
	if (!pVideoMgr || !m_hMovie)
	{
		SwitchToFolder(FOLDER_ID_MAIN);
		StartScreenFadeIn(0.5f);
		return;
	}

	// Update and check if we're finished...

	if (pVideoMgr->UpdateVideo(m_hMovie) != LT_OK ||
		pVideoMgr->GetVideoStatus(m_hMovie) == LT_FINISHED)
	{
		NextMovie();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetMenuMusic()
//
//	PURPOSE:	Turns menu / load level music on or off
//
// ----------------------------------------------------------------------- //
// NOLF has this quite different
DBOOL CInterfaceMgr::SetMenuMusic(DBOOL bMusicOn)
{
	if (bMusicOn)
	{
		if (!m_bGameMusicPaused)
		{
			g_pClientDE->PauseMusic();
			m_bGameMusicPaused = DTRUE;
		}

		// Set up the music...
		char* pLoop = LTNULL;
		CBaseFolder *pFolder = m_FolderMgr.GetCurrentFolder();
		if (pFolder)
			pLoop = pFolder->GetAmbientSound();

		//if we have a loop and 
		//		either we're not playing anything or it's different than the current loop
		if (pLoop && (!m_hMenuMusic || stricmp(pLoop,m_szMenuLoop) != 0))
		{
			if (m_hMenuMusic) 
			{
				g_pClientDE->KillSound(m_hMenuMusic);
				m_hMenuMusic = DNULL;
			}

			DDWORD dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;

			m_hMenuMusic = g_pClientSoundMgr->PlaySoundLocal(pLoop, SOUNDPRIORITY_MISC_LOW, dwFlags);
			SAFE_STRCPY(m_szMenuLoop,pLoop);
		}

		return !!m_hMenuMusic;
	}
	else
	{
		if (m_bGameMusicPaused)
		{
			g_pClientDE->ResumeMusic();
			m_bGameMusicPaused = DFALSE;
		}

		if (m_hMenuMusic) 
		{
			g_pClientDE->KillSound(m_hMenuMusic);
			m_hMenuMusic = DNULL;
		}

		m_szMenuLoop[0] = LTNULL;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnMessage()
//
//	PURPOSE:	Handle interface messages
//
// ----------------------------------------------------------------------- //

static LTBOOL IC_ReadType1Param(uint8 nThing)
{
	switch(nThing)
	{
		case IC_AMMO_ID:
		case IC_HEALTH_ID:
		case IC_ARMOR_ID:
		case IC_AIRLEVEL_ID:
		case IC_RESET_INVENTORY_ID:
		case IC_MAX_HEALTH_ID:
		case IC_MAX_ARMOR_ID:
		case IC_FILL_ALL_CLIPS_ID:
		case IC_CANNON_CHARGE_ID:
		case IC_BATTERY_CHARGE_ID:
			return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

static LTBOOL IC_ReadType2Param(uint8 nThing)
{
	switch(nThing)
	{
		case IC_HEALTH_ID:
		case IC_ARMOR_ID:
		case IC_AIRLEVEL_ID:
		case IC_RESET_INVENTORY_ID:
		case IC_MAX_HEALTH_ID:
		case IC_MAX_ARMOR_ID:
		case IC_FILL_ALL_CLIPS_ID:
		case IC_CANNON_CHARGE_ID:
		case IC_BATTERY_CHARGE_ID:
		case IC_FILL_CLIP_ID:
			return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

static LTBOOL IC_ReadAmountParam(uint8 nThing)
{
	switch(nThing)
	{
		case IC_RESET_INVENTORY_ID:
		case IC_FILL_ALL_CLIPS_ID:
		case IC_FILL_CLIP_ID:
			return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

static LTBOOL IC_ReadNotifyParam(uint8 nThing)
{
	switch(nThing)
	{
		case IC_WEAPON_PICKUP_ID:
			return LTTRUE;
	}

	return FALSE;
}

// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::OnMessage(DBYTE messageID, HMESSAGEREAD hMessage)
{
	switch(messageID)
	{
		case MID_PLAYER_INFOCHANGE :
		{
			DBYTE nThing	= g_pClientDE->ReadFromMessageByte(hMessage);
			DBYTE nType1	= IC_ReadType1Param(nThing) ? g_pClientDE->ReadFromMessageByte(hMessage) : 0;
			DBYTE nType2	= IC_ReadType2Param(nThing) ? g_pClientDE->ReadFromMessageByte(hMessage) : 0;
			DFLOAT fAmount  = IC_ReadAmountParam(nThing) ? g_pClientDE->ReadFromMessageFloat(hMessage) : 0.0f;
			LTBOOL bNotify  = IC_ReadNotifyParam(nThing) ? g_pClientDE->ReadFromMessageByte(hMessage) : LTFALSE;

			UpdatePlayerStats(nThing, nType1, nType2, fAmount, bNotify);

			return DTRUE;
		}

		case MID_PLAYER_EXIT_LEVEL :
		{
			if(g_pGameClientShell->GetPlayerStats()->GetHealthCount()!=0)
				HandleExitLevel(hMessage);

			return DTRUE;
		}

		case MID_PLAYER_HEALTH_ADDED :
		{
			m_HudMgr.AddHealthPickup();
			return DTRUE;
		}

		case MID_PLAYER_MASK_ADDED :
		{
			m_HudMgr.AddMaskPickup();
			return DTRUE;
		}

		case MID_PLAYER_NIGHT_VISION_ADDED :
		{
			m_HudMgr.AddNightVisionPickup();
			return DTRUE;
		}

		case MID_PLAYER_CLOAK_ADDED :
		{
			m_HudMgr.AddCloakPickup();
			return DTRUE;
		}

		case MID_PLAYER_ARMOR_ADDED :
		{
			m_HudMgr.AddArmorPickup();
			return DTRUE;
		}

		case MID_PLAYER_RESET_COUNTER :
		{
			m_HudMgr.ResetCounter();
			return DTRUE;
		}

		case MID_PLAYER_RELOAD_SKINS :
		{
			g_pGameClientShell->GetWeaponModel()->ResetModelSkins();
			m_HudMgr.ResetCounter();
			return DTRUE;
		}

		case MID_PLAYER_CREDITS :
		{
            uint8 nMsg = g_pLTClient->ReadFromMessageByte(hMessage);
			switch (nMsg)
			{
				case 0:
					m_Credits.Term();
					break;
				case 1:
					m_Credits.Init();
					break;
				case 2:
					m_Credits.Init(CM_INTRO);
					break;
			}
			return DTRUE;
		}

		case MID_DISPLAY_TIMER :
		{
			DFLOAT fTime = g_pClientDE->ReadFromMessageFloat(hMessage);
			m_InterfaceTimer.SetTime(fTime);
			return DTRUE;
		}

		case SCM_PLAY_DIALOGUE:
		{
			char szAvatar[100];
			char szDecisions[200];

			DWORD dwID = hMessage->ReadDWord();
			hMessage->ReadStringFL(szAvatar, sizeof(szAvatar));
			BOOL bStayOpen = !!hMessage->ReadByte();
			hMessage->ReadStringFL(szDecisions, sizeof(szDecisions));
			
			m_DialogueWnd.DisplayText(dwID,szAvatar,bStayOpen,szDecisions);

			ChangeState(GS_DIALOGUE);
			return DTRUE;
		}

		case SCM_MISSION_FAILED:
		{
			MissionFailed(IDS_SP_MISSIONFAILED);
			return DTRUE;
		}

		case SCM_SHOW_BOSS_HEALTH:
		{
			LTBOOL bShow = g_pInterface->ReadFromMessageByte(hMessage);
			m_HudMgr.ShowBossHealth(bShow);
			return DTRUE;
		}

		case SCM_UPDATE_BOSS_HEALTH:
		{
			LTFLOAT fRatio = g_pInterface->ReadFromMessageFloat(hMessage);
			m_HudMgr.SetBossHealth(fRatio);
			return DTRUE;
		}

		case MID_PLAYER_OBJECTIVE_ADD:
		{
			// Ignore objectives if we are Insane!
			if(g_pGameClientShell->GetGameType()->GetDifficulty() != DIFFICULTY_PC_INSANE)
			{
				uint32 nId = g_pClientDE->ReadFromMessageWord(hMessage);
				m_stats.AddObjective(nId);
				SetAlert();
			}
			return DTRUE;
		}

		case MID_PLAYER_OBJECTIVE_OVERVIEW:
		{
			int nId = (int)g_pClientDE->ReadFromMessageDWord(hMessage);
			m_stats.AddOverview(nId);

			return DTRUE;
		}

		case MID_PLAYER_OBJECTIVE_RESET:
		{
			m_stats.ClearObjectiveData();
			return DTRUE;
		}

		case MID_PLAYER_OBJECTIVE_STATECHANGE:
		{
			// Ignore objectives if we are Insane!
			if(g_pGameClientShell->GetGameType()->GetDifficulty() != DIFFICULTY_PC_INSANE)
			{
				uint32 nId = g_pClientDE->ReadFromMessageWord(hMessage);
				ObjectiveState eState = (ObjectiveState)g_pClientDE->ReadFromMessageByte(hMessage);
				m_stats.UpdateObjective(nId, eState);
				SetAlert();
			}
			return DTRUE;
		}

		case MID_PREDATORMASK:
		{
			uint8 nFlags = g_pClientDE->ReadFromMessageByte(hMessage);

			LTBOOL bHas = (nFlags & 0x01);
			LTBOOL bMessage = (nFlags & 0x02);

			if(bMessage && g_pGameClientShell->GetGameType()->IsMultiplayer())
			{
				if(m_stats.HasMask() && !bHas)
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_LOST_MASK);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}
				else if(!m_stats.HasMask() && bHas)
				{
					HSTRING hStr = g_pLTClient->FormatString(IDS_MP_FOUND_MASK);
					g_pMessageMgr->AddMessage(g_pLTClient->GetStringData(hStr));
					g_pLTClient->FreeString(hStr);
				}
			}

			m_stats.SetHasMask(bHas);
			return DTRUE;
		}

		case MID_NIGHTVISION:
		{
			uint8 nFlags = g_pClientDE->ReadFromMessageByte(hMessage);

			LTBOOL bHas = (nFlags & 0x01);

			m_stats.SetHasNightVision(bHas);

			return DTRUE;
		}

		case MID_MEDI_FAIL:
		{
			// Play the medi fail sound here...
			g_pClientSoundMgr->PlaySoundLocal("MediComp_Fail");

			return DTRUE;
		}

		case MID_ADD_SKULL:
		{
			m_stats.AddToSkullCount();
			return DTRUE;
		}

		case MID_ADD_SKULL2:
		{
			m_stats.AddToSkullCount();
			// Play the pickup sound...
			g_pClientSoundMgr->PlaySoundLocal("pred_headpickup");

			return DTRUE;
		}

		case MID_SPEED_CHEAT:
		{
			if(m_eGameState == GS_PLAYING)
			{
				if( g_pGameClientShell->IsInWorld() )
				{
					g_pGameClientShell->ExitLevel();
				}

				ClearAllScreenBuffers();
				SwitchToFolder(FOLDER_ID_MAIN);
				StartScreenFadeIn(0.5f);
			}
			else if(m_eGameState == GS_FOLDER)
			{
				CFolderEscape * pFolderEscape = dynamic_cast<CFolderEscape *>(GetFolderMgr()->GetFolderFromID( FOLDER_ID_ESCAPE ));

				if( pFolderEscape && pFolderEscape->IsAborting() )
				{
					return DTRUE;
				}

				SwitchToFolder(FOLDER_ID_MAIN);
			}

			HSTRING hStr;
			hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_SYNC);
			ShowMessageBox(hStr, LTMB_OK, LTNULL, LTNULL);
			g_pLTClient->FreeString(hStr);

			return DTRUE;
		}

		default : break;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::OnEvent(DDWORD dwEventID, DDWORD dwParam)
{

	switch (dwEventID)
	{
		// Called when the renderer has switched into
		// the new mode but before it reloads all the textures
		// so you can display a loading screen.
		
		case LTEVENT_RENDERALMOSTINITTED :
		{
			if (m_bSwitchingModes)
			{
				ClearAllScreenBuffers();
				
				/*
				g_pClientDE->Start3D();
				g_pClientDE->StartOptimized2D();

				m_InterfaceResMgr.DrawMessage(m_InterfaceResMgr.GetSmallFont(),
					IDS_REINITIALIZING_RENDERER);
					
				g_pClientDE->EndOptimized2D();
				g_pClientDE->End3D();
				*/
				g_pClientDE->FlipScreen(0);
			
			}
		}
		break;

		// Client disconnected from server.  dwParam will 
		// be a error flag found in de_codes.h.

		case LTEVENT_DISCONNECT :
		{
			if(	g_pGameClientShell->IsMultiplayerGame() &&
				((m_eGameState == GS_PLAYING) || (m_eGameState == GS_LOADINGLEVEL) || (m_eGameState == GS_FOLDER)))
			{
				if(m_eGameState == GS_LOADINGLEVEL)
				{
					g_pInterfaceMgr->LoadFailed();
				}
				else if(m_eGameState == GS_PLAYING)
				{
					if( g_pGameClientShell->IsInWorld() )
					{
						g_pGameClientShell->ExitLevel();
					}

					ClearAllScreenBuffers();
					SwitchToFolder(FOLDER_ID_MAIN);
					StartScreenFadeIn(0.5f);
				}
				else if(m_eGameState == GS_FOLDER)
				{
					CFolderEscape * pFolderEscape = dynamic_cast<CFolderEscape *>(GetFolderMgr()->GetFolderFromID( FOLDER_ID_ESCAPE ));

					if( pFolderEscape && pFolderEscape->IsAborting() )
					{
						return DTRUE;
					}

					SwitchToFolder(FOLDER_ID_MAIN);
				}

				HSTRING hStr;

				switch(m_nDisconnectError)
				{
					case LT_DISCON_MISSINGFILE:		hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_MISSING);		break;
					case LT_DISCON_CONNECTLOST:		hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_LOST);		break;
					case LT_DISCON_CONNECTTERM:		hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_TERM);		break;
					case LT_DISCON_SERVERBOOT:		hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_KICK);		break;
					case LT_DISCON_TIMEOUT:			hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_TIMEOUT);		break;
					case LT_DISCON_WORLDCRC:		hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_WORLDCRC);	break;
					case LT_DISCON_MISCCRC:			hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_MISCCRC);		break;
					case LT_DISCON_PASSWORD:		hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_PASSWORD);	break;
					case LT_DISCON_USER:			hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_FULL);		break;

					case LT_DISCON_UNKNOWN:
					default:						hStr = g_pLTClient->FormatString(IDS_DISCONNECTED_FROM_SERVER);	break;
				}

				ShowMessageBox(hStr, LTMB_OK, LTNULL, LTNULL);
				g_pLTClient->FreeString(hStr);

			}
		}
		break;

		// Engine shutting down.  dwParam will be a error 
		// flag found in de_codes.h.

		case LTEVENT_SHUTDOWN :
		break;
		
		// The renderer was initialized.  This is called if 
		// you call SetRenderMode or if focus was lost and regained.

		case LTEVENT_RENDERINIT :
		{
			if (m_LoadingScreen.IsVisible())
			{
				// Hide it to get rid of any resources that may have been around
				m_LoadingScreen.Hide();
				// And then start it up again
				m_LoadingScreen.Show();
			}
		}
		break;

		// The renderer is being shutdown.  This happens when
		// ShutdownRender is called or if the app loses focus.

		case  LTEVENT_RENDERTERM :
		{
			// Stop drawing the loading screen
			if (m_LoadingScreen.IsVisible())
				m_LoadingScreen.Pause();
		}
		break;

		default :
		{
			DDWORD nStringID = IDS_UNSPECIFIEDERROR;
//			SwitchToFolder(FOLDER_ID_MAIN);
			//DoMessageBox(nStringID, TH_ALIGN_CENTER);
		}
		break;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayerStats()
//
//	PURPOSE:	Update the player's stats
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePlayerStats(DBYTE nThing, DBYTE nType1,
									  DBYTE nType2, DFLOAT fAmount, LTBOOL bNotify)
{
	switch (nThing)
	{
		case IC_WEAPON_PICKUP_ID :
		{
			m_stats.UpdateWeapon(nType1, DTRUE, bNotify);
			m_stats.UpdateAmmo(nType2, (DDWORD)fAmount, DTRUE, bNotify);
		}
		break;

		case IC_WEAPON_ID :
		{
			m_stats.UpdateWeapon(nType1);
		}
		break;

		case IC_AMMO_ID :
		{		
			m_stats.UpdateAmmo(nType2, (DDWORD)fAmount);
		}
		break;

		case IC_MAX_HEALTH_ID :
		{		
			m_stats.UpdateMaxHealth((DDWORD)fAmount);
			m_stats.SetResetHealth();
		}
		break;

		case IC_MAX_ARMOR_ID :
		{		
			m_stats.UpdateMaxArmor((DDWORD)fAmount);
			m_stats.SetResetArmor();
		}
		break;

		case IC_HEALTH_ID :
		{		
			m_stats.UpdateHealth((DDWORD)fAmount);
		}
		break;

		case IC_BATTERY_CHARGE_ID :
		{		
			m_stats.UpdateBatteryLevel(fAmount);
		}
		break;

		case IC_ARMOR_ID :
		{		
			m_stats.UpdateArmor((DDWORD)fAmount);
		}
		break;

		case IC_CANNON_CHARGE_ID :
		{		
			m_stats.UpdateCannonChargeLev((uint32)fAmount);
		}
		break;

		case IC_AIRLEVEL_ID :
		{	
			m_stats.UpdateAir(fAmount);
		}
		break;

		case IC_OUTOFAMMO_ID :
		{

		}
		break;

		case IC_RESET_INVENTORY_ID :
		{
			m_stats.ResetInventory();
		}
		break;

		case IC_FILL_ALL_CLIPS_ID :
		{
			uint32 nClips = g_pWeaponMgr->GetNumBarrels();

			for(uint32 i = 0; i < nClips; i++)
			{
				BARREL *pBarrel = g_pWeaponMgr->GetBarrel(i);

				if(pBarrel && pBarrel->nShotsPerClip)
					m_stats.UpdateClip((uint8)i,pBarrel->nShotsPerClip);
			}
		}

		case IC_FILL_CLIP_ID :
		{
			WEAPON* pWep = g_pWeaponMgr->GetWeapon(nType1);

			if(pWep)
			{
				// Do the primary...
				BARREL *pBarrel = g_pWeaponMgr->GetBarrel(pWep->aBarrelTypes[PRIMARY_BARREL]);

				if(pBarrel && pBarrel->nShotsPerClip)
					m_stats.UpdateClip(pBarrel->nId,pBarrel->nShotsPerClip);

				// Now try the alt barrel...
				pBarrel = g_pWeaponMgr->GetBarrel(pWep->aBarrelTypes[ALT_BARREL]);

				if(pBarrel && pBarrel->nShotsPerClip)
					m_stats.UpdateClip(pBarrel->nId,pBarrel->nShotsPerClip);
			}
		}

		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandleLoadNewLevel()
//
//	PURPOSE:	Handle loading a new level
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::HandleLoadNewLevel()
{
	// Be sure we are not in a vision mode...
	g_pGameClientShell->GetVisionModeMgr()->SetNormalMode();

	//get the mission data
	const MissionButes & prevMissionButes = g_pMissionMgr->GetMissionButes(m_nPrevMissionId);
	//exit this level
	g_pGameClientShell->ExitLevel();

	const int nStatus = HandleMissionStatus();

		// if all missions from all 3 species are complete
	if (_stricmp(prevMissionButes.m_szNextLevel, "menu") == 0)
	{
		if ((nStatus == 1) && (!m_bPlayedOutro))
		{
			g_pGameClientShell->LoadWorld("worlds\\singleplayer\\outro", LTNULL, LTNULL, LOAD_NEW_LEVEL);
			m_Credits.Init();
			m_bPlayedOutro = LTTRUE;
			return;
		}

		m_bPlayedOutro = LTFALSE;

		CreatePauseBackground();

#ifdef _DEMO
		MissionFailed(-1);
#else

		// Put NOLF folderMGR here
		g_pGameClientShell->GetCameraMgr()->SetCameraActive(g_pGameClientShell->GetPlayerCamera());

		SwitchToFolder(FOLDER_ID_MAIN);
		ChangeState(GS_FOLDER);

		StartScreenFadeIn(0.5f);
#endif
	}
	else
	{
		//load the new level
		g_pGameClientShell->GetGameType()->SetStartMission(LTFALSE);

		g_pGameClientShell->LoadWorld(const_cast<char*>(prevMissionButes.m_szNextLevel), LTNULL, LTNULL, LOAD_NEW_LEVEL);
		m_bPlayedOutro = LTFALSE;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandleMissionStatus()
//
//	PURPOSE:	Tracks the mission progression for each species in the
//				current user profile.
//
//	RETURNS:	1 if missions for all three species have been completed.
//				0 for all other cases.
//
// ----------------------------------------------------------------------- //
int CInterfaceMgr::HandleMissionStatus()
{
	DWORD dwVal;
	int nStatus = 0;

	// Record mission oompletion status
	const Species eSpecies = g_pGameClientShell->GetPlayerSpecies();
	const int nDifficulty = g_pGameClientShell->GetGameType()->GetDifficulty();
	const CampaignButes & campaign = g_pMissionMgr->GetCampaignButes(eSpecies);
	
	int nOffset = GetCampaignMission(campaign, g_pMissionMgr->GetNextLevelName(m_nPrevMissionId));
	if (_stricmp(g_pMissionMgr->GetNextLevelName(m_nPrevMissionId), "menu") == 0)
		nOffset = MAX_CAMPAIGN_MISSIONS;

	--nOffset;		// need this to start at 0 instead of 1
	if (nOffset >= 0)
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		if (pProfile)
		{
			switch (eSpecies)
			{
				case Alien:
				{
					// do nothing if the current difficulty is below the highest recorded.
					// Note: 8 and 9 are always overwritten (cheater, and incomplete)
					dwVal = DecodeStatus(pProfile->m_LevelStatus.Alien);
					nStatus = GetDigit(dwVal, nOffset);
					if ((nStatus <= 7) && (nDifficulty <= nStatus))
						break;

					dwVal = SetDigit(dwVal, nOffset, nDifficulty);
					pProfile->m_LevelStatus.Alien = EncodeStatus(dwVal);
					break;
				}
				case Marine:
				{
					dwVal = DecodeStatus(pProfile->m_LevelStatus.Marine);
					nStatus = GetDigit(dwVal, nOffset);
					if ((nStatus <= 7) && (nDifficulty <= nStatus))
						break;

					dwVal = SetDigit(dwVal, nOffset, nDifficulty);
					pProfile->m_LevelStatus.Marine = EncodeStatus(dwVal);
					break;
				}
				case Predator:
				{
					dwVal = DecodeStatus(pProfile->m_LevelStatus.Predator);
					nStatus = GetDigit(dwVal, nOffset);
					if ((nStatus <= 7) && (nDifficulty <= nStatus))
						break;

					dwVal = SetDigit(dwVal, nOffset, nDifficulty);
					pProfile->m_LevelStatus.Predator = EncodeStatus(dwVal);
					break;
				}
				default:
					break;
			}
			pProfile->Save();
		}
	}

	// Check to see if we won the whole game (all 3 species)
	nStatus = 0;
	if (nOffset == (campaign.m_nNumMissions - 1))
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		if (pProfile)
		{
			const DWORD dwAlienVal = DecodeStatus(pProfile->m_LevelStatus.Alien);
			const DWORD dwMarineVal = DecodeStatus(pProfile->m_LevelStatus.Marine);
			const DWORD dwPredatorVal = DecodeStatus(pProfile->m_LevelStatus.Predator);

			for (int i = 0; i < campaign.m_nNumMissions; i++)
			{
				// cheating (setting 8) won't get you the final movie
				if (GetDigit(dwAlienVal, i) <= 3)
					nStatus++;
				if (GetDigit(dwMarineVal, i) <= 3)
					nStatus++;
				if (GetDigit(dwPredatorVal, i) <= 3)
					nStatus++;
			}

			if (nStatus == (campaign.m_nNumMissions * 3))
				return 1;
		}
	}

	return 0;
}

DWORD CInterfaceMgr::SetDigit(DWORD dwVal, int nOffset, int nVal)
{
	// nOffset = the digit offset, starting least significant.  nVal = the value to set.
	int n1 = 1, n2 = 1;
	DWORD dwModVal;

	// base 10...
	for (int i = 0; i < nOffset; i++)
		n1 *= 10;

	n2 = n1 * 10;

	dwModVal = dwVal % n1;
	
	dwVal /= n2;
	dwVal *= n2;
	dwVal += (nVal * n1);
	dwVal += dwModVal;

	return dwVal;
}

DWORD CInterfaceMgr::GetDigit(DWORD dwVal, int nOffset)
{
	int n1 = 1, n2 = 1;

	for (int i = 0; i < nOffset; i++)
		n1 *= 10;

	n2 = n1 * 10;

	dwVal %= n2;			// preserve the digit & below
	dwVal /= n1;			// nuke the extra

	return dwVal;
}

DWORD CInterfaceMgr::EncodeStatus(DWORD dwVal)
{
	// Very simple encoder to foil the casual player who might edit the profile.
	int nKey = GetRandom(1, 3);

	switch (nKey)
	{
		case 0:
			return dwInitEnc;	// no missions unlocked
		case 1:
			dwVal ^= dwMask1;
			break;
		case 2:
			dwVal ^= dwMask2;
			break;
		case 3:
			dwVal ^= dwMask3;
			break;
		default:
			break;
	}

	// Set new key
	int nTop = dwVal / 1000000000;
	dwVal += (nKey - nTop) * 1000000000;

	return dwVal;
}

DWORD CInterfaceMgr::DecodeStatus(DWORD dwVal)
{
	int nKey = dwVal / 1000000000;
	dwVal += (1 - nKey) * 1000000000;

	switch (nKey)
	{
		case 0:
			return dwInit;
		case 1:
			dwVal ^= dwMask1;
			break;
		case 2:
			dwVal ^= dwMask2;
			break;
		case 3:
			dwVal ^= dwMask3;
			break;
		default:
			break;
	}

	return dwVal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandleExitLevel()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::HandleExitLevel(HMESSAGEREAD hMessage)
{
	m_nPrevMissionId = g_pClientDE->ReadFromMessageByte(hMessage);
	const MissionButes & prevMissionButes = g_pMissionMgr->GetMissionButes(m_nPrevMissionId);

	// Fade the screen out...

	if (prevMissionButes.m_fScreenFadeTime > 0.0f)
	{
		//start the fade and set some data
		StartScreenFadeOut(prevMissionButes.m_fScreenFadeTime);
		m_bExitAfterFade = LTTRUE;
		g_pGameClientShell->PauseGame(DTRUE);
	}
	else
	{
		HandleLoadNewLevel();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::OnCommandOn(int command)
{
	DBOOL bMultiplayer = g_pGameClientShell->IsMultiplayerGame();

	if (m_messageMgr.IsEditing()) return DTRUE;

	// Take appropriate action

	switch (command)
	{		
		case COMMAND_ID_SCOREDISPLAY :
		{
			if(!g_pGameClientShell->IsMultiplayerGame())
			{
				m_bDisplayObjectives=LTTRUE;
				SetObjectiveDrawData();
				CreateObjectiveDisplay();
				ClearObjectiveAlert();
			}
		}
		break;

		case COMMAND_ID_CROSSHAIRTOGGLE :
		{
			m_pCrosshairMgr->ToggleCrosshairOverride();
		}
		break;

		case COMMAND_ID_MESSAGE :
		{
			if (m_eGameState == GS_PLAYING)
			{
				if(g_pGameClientShell->GetMultiplayerMgr())
					if(g_pGameClientShell->GetMultiplayerMgr()->IsRulesDisplay())
						return LTFALSE;

				if(m_messageMgr.SetEditing(LTTRUE))
				{
					g_pGameClientShell->SetInputState(DFALSE);
				}
			}

			return DTRUE;
		}
		break;

		case COMMAND_ID_TEAMMESSAGE :
		{
			if (m_eGameState == GS_PLAYING)
			{
				if(g_pGameClientShell->GetMultiplayerMgr())
					if(g_pGameClientShell->GetMultiplayerMgr()->IsRulesDisplay())
						return LTFALSE;

				if(m_messageMgr.SetEditing(LTTRUE, EDIT_TYPE_TEAM))
				{
					g_pGameClientShell->SetInputState(DFALSE);
				}
			}

			return DTRUE;
		}
		break;

		case COMMAND_ID_PREV_WEAPON : 
		{
//			if (m_AmmoChooser.IsOpen())
//			{
//				m_AmmoChooser.Close();
//			}
			LTBOOL bIsDead = g_pGameClientShell->GetPlayerStats()->GetHealthCount()==0;

			if(bIsDead)
			{
				if(m_pWeaponChooser && m_pWeaponChooser->IsOpen())
				{
					m_pWeaponChooser->Close();
				}
				return LTTRUE;
			}

			if(m_pWeaponChooser && !g_pGameClientShell->IsCinematic())
			{
				if (m_pWeaponChooser->Open())
				{
					m_pWeaponChooser->PrevWeapon();
				}
			}

			// Special exosuit case...
			if(IsExosuit(g_pGameClientShell->GetPlayerStats()->GetButeSet()))
			{
				uint8 nWepIndex = g_pGameClientShell->GetWeaponModel()->PrevWeapon();

				g_pGameClientShell->GetWeaponModel()->ChangeWeapon(COMMAND_ID_WEAPON_BASE + nWepIndex);
			}

			return DTRUE;
			
		}
		break;
		
		case COMMAND_ID_NEXT_WEAPON : 
		{
//			if (m_AmmoChooser.IsOpen())
//			{
//				m_AmmoChooser.Close();
//			}
			LTBOOL bIsDead = g_pGameClientShell->GetPlayerStats()->GetHealthCount()==0;

			if(bIsDead)
			{
				if(m_pWeaponChooser && m_pWeaponChooser->IsOpen())
				{
					m_pWeaponChooser->Close();
				}
				return LTTRUE;
			}

			if(m_pWeaponChooser && !g_pGameClientShell->IsCinematic())
			{
				if (m_pWeaponChooser->Open())
				{
					m_pWeaponChooser->NextWeapon();
				}
			}

			// Special exosuit case...
			if(IsExosuit(g_pGameClientShell->GetPlayerStats()->GetButeSet()))
			{
				uint8 nWepIndex = g_pGameClientShell->GetWeaponModel()->NextWeapon();

				g_pGameClientShell->GetWeaponModel()->ChangeWeapon(COMMAND_ID_WEAPON_BASE + nWepIndex);
			}

			return DTRUE;
		}
		break;

		case COMMAND_ID_LAST_WEAPON : 
		{
			LTBOOL bIsDead = g_pGameClientShell->GetPlayerStats()->GetHealthCount()==0;

			if(bIsDead)
			{
				return LTTRUE;
			}

			g_pGameClientShell->GetWeaponModel()->LastWeapon();

			return DTRUE;
		}
		break;

/*		case COMMAND_ID_NEXT_AMMO : 
		{
			if (m_WeaponChooser.IsOpen())
			{
				m_WeaponChooser.Close();
			}
			if (m_AmmoChooser.Open())
			{
				m_AmmoChooser.NextAmmo();
			}
			return DTRUE;
		}
		break;
*/
		case COMMAND_ID_FIRING :
		case COMMAND_ID_ALT_FIRING:
		{

			if (IsChoosingWeapon())
			{
				if(m_pWeaponChooser)
				{
					DDWORD nCurrWeapon = m_pWeaponChooser->GetCurrentSelection();
					m_pWeaponChooser->Close();
					g_pGameClientShell->GetWeaponModel()->ChangeWeapon(g_pWeaponMgr->GetCommandId(nCurrWeapon));
				}
				return DTRUE;

			}
//			if (IsChoosingAmmo())
//			{
//				DBYTE nCurrAmmo = m_AmmoChooser.GetCurrentSelection();
//				m_AmmoChooser.Close();
//				g_pGameClientShell->GetWeaponModel()->ChangeAmmo(nCurrAmmo);
//				return DTRUE;
//
//			}

		}
		break;

		case COMMAND_ID_PREV_HUD : 
		{
#ifndef _DEMO
			if (m_eGameState == GS_PLAYING)
				m_stats.PrevLayout();
			return DTRUE;
#endif
		}
		break;

		case COMMAND_ID_NEXT_HUD : 
		{
#ifndef _DEMO
			if (m_eGameState == GS_PLAYING)
				m_stats.NextLayout();
			return DTRUE;
#endif
		}
		break;

		default :
		break;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::OnCommandOff(int command)
{
	// Only process if not editing a message...

	if (m_messageMgr.IsEditing()) return DTRUE;

	switch (command)
	{
		case COMMAND_ID_SCOREDISPLAY :
		{
			if(!g_pGameClientShell->IsMultiplayerGame())
			{
				m_bDisplayObjectives=LTFALSE;
				ClearObjectiveDisplay();
			}
		}
		break;

		default : break;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyDown()
//
//	PURPOSE:	Handle OnKeyDown messages
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::OnKeyDown(int key, int rep)
{
	// The message box is modal.
	if (m_MessageBox.IsVisible())
		return m_MessageBox.HandleKeyDown(key,rep);

#ifndef _DEMO

	// Is it a quick save or a quick load?
	if( key == g_pClientButeMgr->GetQuickLoadKey() )
	{
		CBaseFolder *pFolder = m_FolderMgr.GetCurrentFolder();
		int nFolderID;

		if(pFolder)
		{
			nFolderID = pFolder->GetFolderID();
		}

		LTBOOL bAllow = LTTRUE;

		// No quick loading from the custom controls!
		if(pFolder && (nFolderID == FOLDER_ID_CUST_CONTROLS))
			bAllow = LTFALSE;

		// No quick loading while a "modal" window is up!
		if(pFolder && pFolder->GetCapture())
			bAllow = LTFALSE;

		// Don't allow quick save and quick load from the config screen
		if(bAllow)
		{
			if( g_pGameClientShell->GetGameType()->GetDifficulty() == DIFFICULTY_PC_INSANE )
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_NO_QUICK_LOAD);
				m_messageMgr.AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);
			}
			else
			{
				g_pGameClientShell->QuickLoad();
			}

			return LTTRUE;
		}
	}
	else if( key == g_pClientButeMgr->GetQuickSaveKey() )
	{
		CBaseFolder *pFolder = m_FolderMgr.GetCurrentFolder();
		int nFolderID;

		if(pFolder)
		{
			nFolderID = pFolder->GetFolderID();
		}

		LTBOOL bAllow = LTTRUE;

		if(pFolder && (nFolderID == FOLDER_ID_CUST_CONTROLS))
			bAllow = LTFALSE;

		// Don't allow quick save and quick load from the config screen
		if(bAllow)
		{
			if( g_pGameClientShell->GetGameType()->GetDifficulty() == DIFFICULTY_PC_INSANE )
			{
				// No quick save allowed!
				HSTRING hStr = g_pLTClient->FormatString(IDS_NO_QUICK_SAVE);
				m_messageMgr.AddMessage(g_pLTClient->GetStringData(hStr));
				g_pLTClient->FreeString(hStr);
			}
			else
			{
				// Do quick save...
				if( g_pGameClientShell->QuickSave() )
				{
					// Inform the user.
					HSTRING hStr = g_pLTClient->FormatString(IDS_QUICKSAVING);
					char* pStr = g_pLTClient->GetStringData(hStr);
					g_pLTClient->CPrint(pStr);

					if( m_eGameState == GS_FOLDER )
					{
						ShowMessageBox(hStr,LTMB_OK,DNULL);

					// only reset cursor if we are in game..
					if(GetGameState() == GS_PLAYING || GetGameState() == GS_PAUSED)
						g_pInterfaceMgr->UseCursor(LTFALSE);
					}
					else
					{
						m_messageMgr.AddMessage(pStr);
					}

					g_pLTClient->FreeString(hStr);
				}
			}

			return LTTRUE;
		}
	}

#endif


	// Game state specific key handling.
	switch (m_eGameState)
	{
	case GS_PLAYING:
		{
			if(key == g_pClientButeMgr->GetCinematicSkipKey())
			{
				HCAMERA hCam = g_pGameClientShell->GetCinematicCamera();

				if (g_pCameraMgr->GetCameraActive(hCam))
				{
					m_bCinematicSkipKey = LTTRUE;
					m_fCinematicSkipDelay = 0.25f;

					LTVector vNull(0,0,0);

					// Skip cinematic here...
					HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_ACTIVATE);
					g_pLTClient->WriteToMessageVector(hMessage, &vNull);
					g_pLTClient->WriteToMessageVector(hMessage, &vNull);
					g_pLTClient->EndMessage(hMessage);

					return LTTRUE;
				}
			}
		}
		break;
	case GS_DIALOGUE :
		{
			CLTDecisionWnd* pWnd = m_DialogueWnd.GetDecisionWnd();
			if (pWnd && pWnd->IsInitialized() && pWnd->IsVisible() && pWnd->IsWindowEnabled())
			{
				if (key >= VK_1 && key <= VK_9)
				{			
					pWnd->SendMessage(LTWM_COMMAND, LTWC_SELECT_TEXT, (key - VK_1));
					return DTRUE;
				}
			}
			else
			{
				if (key == g_pClientButeMgr->GetCinematicSkipKey() )
				{
					m_DialogueWnd.OnLButtonDown(0, 0);
					return DTRUE;
				}
			}
			
			if (!m_DialogueWnd.IsVisible())
			{
				ChangeState(GS_PLAYING);
			}
			
			return DTRUE; // Can only do dialogue stuff
		}
		break;
		
	case GS_LOADINGLEVEL :
		{
			return DTRUE;
		}
		break;
		
		// NOLF
	case GS_FOLDER :
		{
			if (key != g_pClientButeMgr->GetEscapeKey() || !g_bLockFolder)
				m_FolderMgr.HandleKeyDown(key,rep);
            return LTTRUE;
		}
		break;
		
	case GS_PAUSED :
		{
			// They pressed a key - unpause the game
			ChangeState(GS_PLAYING);
			return DTRUE;
		}
		break;
		
	case GS_SPLASHSCREEN :
		{
			// Gotta lock for legal reasons...
#ifdef _DEMO
			NextSplashScreen();
#else
			if(g_pLTClient->GetTime() - m_fSplashStartTime > g_vtSplashLockTime.GetFloat())
			{
 				ChangeState(GS_MOVIE);
			}
#endif
			return DTRUE;
		}
		break;
		
		case GS_MOVIE :
		{
			// They pressed a key - end splash screen...
			NextMovie();
            return LTTRUE;
		}
		break;

	case GS_FAILURE :
		{
#ifdef _DEMO
			// just accelerate things
			if(g_fFailScreenDuration > 2.0f)
				g_fFailScreenDuration = 100.0f;
#else
			// They pressed a key - end failure screen...
			if (g_fFailScreenDuration > 1.0f)
			{
				m_FolderMgr.ClearFolderHistory();
				m_FolderMgr.AddToFolderHistory(FOLDER_ID_MAIN);
				m_FolderMgr.AddToFolderHistory(FOLDER_ID_SINGLE);

				SwitchToFolder(FOLDER_ID_LOAD);
				ChangeState(GS_FOLDER);
				StartScreenFadeIn(0.5f);
			}
#endif
			return DTRUE;
		}
		break;
		
	default : break;
	}


	// Are We Broadcasting a Message
	
	if (m_messageMgr.IsEditing())
	{
		m_messageMgr.HandleKeyDown(key, rep);
		return DTRUE;
	}
	

	// Other various catch-all keys.
	if( key == g_pClientButeMgr->GetPauseKey() )
	{
		if (g_pGameClientShell->IsMultiplayerGame() || m_eGameState != GS_PLAYING) return DFALSE;
			
		if (!g_pGameClientShell->IsGamePaused())
		{
			ChangeState(GS_PAUSED);
		}

		g_pGameClientShell->PauseGame(!g_pGameClientShell->IsGamePaused(), DTRUE);
		return DTRUE;
	}
	else if( key == g_pClientButeMgr->GetEscapeKey() )
	{
		//Get cinematic camera
		HCAMERA hCam = g_pGameClientShell->GetCinematicCamera();

		if (m_eGameState == GS_PLAYING && !g_pCameraMgr->GetCameraActive(hCam) && !IsFadingOut() && !IsFadingIn())
		{
			// close the chooser if it's open
			if(m_pWeaponChooser && m_pWeaponChooser->IsOpen())
			{
				m_pWeaponChooser->Close();
				return DTRUE;
			}

			// dont save unless our weapon is idle...
			if(	g_pGameClientShell->GetPlayerState() == PS_ALIVE && 
				(g_pGameClientShell->GetWeaponModel()->GetState() != W_IDLE && g_pGameClientShell->GetWeaponModel()->GetState() != W_SPINNING))
			{
				return LTTRUE;
			}

			CreatePauseBackground();
			
			// Put NOLF folderMGR here
			SwitchToFolder(FOLDER_ID_ESCAPE);
			ChangeState(GS_FOLDER);
		}

		return DTRUE;
	}

	// AI Debugging
#ifndef _FINAL
#ifndef _DEMO
	if( g_vtDebugKeys.GetFloat() > 0.0f )
	{
		char szTempStr[256];

		// Go through each debug key and see if it matches the key hit.
		for( int i = 0; i < g_pClientButeMgr->GetNumDebugKeys(); ++i )
		{
			if( g_pClientButeMgr->GetDebugKey(i) == key )
			{
				CString cstrDebugName = g_pClientButeMgr->GetDebugName(i);

				if( cstrDebugName.IsEmpty() )
					break;

				char * szVarName = cstrDebugName.GetBuffer(0);

				// Get the console variable associated with this debug key.
				HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(szVarName);

				// Initialize the variable if it does not exist.
				if( !hVar )
				{
					sprintf(szTempStr, "\"%s\" \"%2f\"", szVarName, 0.0f );
					g_pLTClient->RunConsoleString( szTempStr );

					hVar = g_pLTClient->GetConsoleVar(szVarName);
					ASSERT( hVar );

					if( !hVar )
						break;
				}

				// Increment the current debug level, recycling to zero if it is above max levels.
				LTFLOAT fCurrentLevel = g_pLTClient->GetVarValueFloat(hVar);
				++fCurrentLevel;

				if( int(fCurrentLevel) >= g_pClientButeMgr->GetNumDebugLevels(i) )
				{
					fCurrentLevel = 0.0f;
				}
				
				// Set the new debug level.
				sprintf(szTempStr, "\"%s\" \"%2f\"", szVarName, fCurrentLevel );
				g_pLTClient->RunConsoleString( szTempStr );

				g_pLTClient->CPrint("%s %2f", szVarName, fCurrentLevel );

				CString cstrConsole = g_pClientButeMgr->GetDebugString(i,DBYTE(fCurrentLevel));
				if( !cstrConsole.IsEmpty() )
				{
					g_pLTClient->RunConsoleString( cstrConsole.GetBuffer(0) );
#ifdef _DEBUG
					g_pLTClient->CPrint( cstrConsole.GetBuffer(0) );
#endif
				}

				// Display message
				CString cstrTitle = g_pClientButeMgr->GetDebugTitle(i,DBYTE(fCurrentLevel));
				if( !cstrTitle.IsEmpty() )
				{
					m_messageMgr.AddMessage( cstrTitle.GetBuffer() );
				}
				else
				{
					sprintf(szTempStr, "%s set to level %1.0f", szVarName, fCurrentLevel);
					m_messageMgr.AddMessage( szTempStr );
				}
			}
		}
	}
#endif
#endif

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyUp(int key)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::OnKeyUp(int key)
{
	// if it's the tilde (~) key then the console has been turned off
	// (this is the only event that causes this key to ever get processed)
	// so clear the back buffer to get rid of any part of the console still showing

	if (key == VK_TILDE)
	{
		AddToClearScreenCount();
		return DTRUE;
	}

	if (key == g_pClientButeMgr->GetCinematicSkipKey())
	{
		m_bCinematicSkipKey = LTFALSE;
		return DTRUE;
	}

	if (m_messageMgr.IsEditing())
	{
		m_messageMgr.HandleKeyUp(key);
		return DTRUE;
	}

	if (m_eGameState == GS_FOLDER)
	{
		m_FolderMgr.HandleKeyUp(key);
        return LTTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawPredTransition()
//
//	PURPOSE:	Handle drawing the predator transition
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DrawPredTransition(HCAMERA hCam)
{
	HSURFACE hOverlay = g_pInterfaceResMgr->GetSharedSurface("interface\\statusbar\\predator\\screenwipe.pcx");
	if(hOverlay)
	{
		g_pLTClient->SetSurfaceAlpha(hOverlay, 0.8f);
		uint32 cxOverlay, cyOverlay;
		g_pLTClient->GetSurfaceDims(hOverlay, &cxOverlay, &cyOverlay);

		//get the Camera react
		LTRect rc;
		g_pCameraMgr->GetCameraRect(hCam, rc, LTTRUE);

		//get screen dims
		uint32 cxScreen, cyScreen;
		HSURFACE hScreen = g_pLTClient->GetScreenSurface();
		g_pLTClient->GetSurfaceDims(hScreen, &cxScreen, &cyScreen);

		//fill rect with black top and bottom.
		LTRect rcFill;
		rcFill.top		= 0;
		rcFill.bottom	= cyScreen;
		rcFill.right	= rc.right + 75;
		rcFill.left		= rc.right - (int)(cyScreen * 0.8f);

		g_pLTClient->ScaleSurfaceToSurface(hScreen, hOverlay, &rcFill, LTNULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ClearCinematicBorders()
//
//	PURPOSE:	Handle clearing the cinematic boarders
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::HandleCinematicBorders(HCAMERA hCam)
{
	//get the Camera react
	LTRect rc;
	g_pCameraMgr->GetCameraRect(hCam, rc, LTTRUE);

	//get screen dims
	uint32 cxScreen, cyScreen;
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	g_pLTClient->GetSurfaceDims(hScreen, &cxScreen, &cyScreen);


	if(rc.top != 0 || rc.bottom != (int)cyScreen || 
		rc.left != 0 || rc.right != (int)cxScreen )
	{
		//fill rect with black top and bottom.
		LTRect rcFill;
		rcFill.top		= 0;
		rcFill.bottom	= rc.top;
		rcFill.left		= 0;
		rcFill.right	= rc.right;

		g_pLTClient->FillRect(hScreen, &rcFill, SETRGB(0,0,0));

		rcFill.top		= rc.bottom;
		rcFill.bottom	= cyScreen;

		g_pLTClient->FillRect(hScreen, &rcFill, SETRGB(0,0,0));

		//now fill the left and right
		rcFill.top		= rc.top;
		rcFill.bottom	= rc.bottom;
		rcFill.left		= 0;
		rcFill.right	= rc.left;

		g_pLTClient->FillRect(hScreen, &rcFill, SETRGB(0,0,0));

		rcFill.left		= rc.right;
		rcFill.right	= cxScreen;

		g_pLTClient->FillRect(hScreen, &rcFill, SETRGB(0,0,0));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandleCinematicBorders()
//
//	PURPOSE:	Handle clearing the cinematic boarders
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::HandleCinematicBorders()
{
	//Get cinematic camera
	HCAMERA hCam = g_pGameClientShell->GetPlayerCamera();

	if(g_pCameraMgr->GetCameraActive(hCam))
	{	
		HandleCinematicBorders(hCam);
	}
	else
	{
		hCam = g_pGameClientShell->GetCinematicCamera();

		if(g_pCameraMgr->GetCameraActive(hCam))
		{
			HandleCinematicBorders(hCam);
		}
		else
		{
			hCam = g_pGameClientShell->Get3rdPersonCamera();

			if(g_pCameraMgr->GetCameraActive(hCam))
			{
				HandleCinematicBorders(hCam);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Draw()
//
//	PURPOSE:	Draws any interface stuff that may need to be drawn
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::Draw()
{
	HandleCinematicBorders();

	// Check for interface drawing override...
	if (g_vtDrawInterface.GetFloat())
	{

		if (bBlackScreen)
		{
			g_pClientDE->ClearScreen(DNULL, CLEARSCREEN_SCREEN);
			if (fBlackScreenTime > 0.0f)
			{
				fBlackScreenTime -= g_pClientDE->GetFrameTime();
			}
			else
			{
				bBlackScreen = DFALSE;
				fBlackScreenTime = 0.0f;
			}
			return DFALSE;
		}

		// Find out if we're in multiplayer...

		DBOOL bMultiplayer = g_pGameClientShell->IsMultiplayerGame();
		PlayerState ePlayerState = g_pGameClientShell->GetPlayerState();

		//TEMP
		g_pGameClientShell->GetSFXMgr()->PostRenderDraw();

		// Draw the player stats (health,armor,ammo) if appropriate...
		
		if (m_bDrawInterface) 
		{
			m_stats.Draw(m_bDrawPlayerStats);
			m_pCrosshairMgr->DrawCrosshair();
		}


		// See about objective stuff if we are in first person.
		if(g_pGameClientShell->IsFirstPerson())
		{
			//draw the objectives
			if(m_bDisplayObjectives)
			{
				// Disable any story objects...
				if(g_pGameClientShell->IsStoryObjectActive())
					g_pGameClientShell->EndStoryObject();

				// Turn off the notification
				if(m_bObjectiveNotification)
					m_bObjectiveNotification = LTFALSE;

				// Draw the objective display
				DrawObjectives();
			}

			if(m_bObjectiveNotification)
			{
				DrawObjectiveNotification();
			}
			else
			{
				ClearObjectiveAlert();
			}
		}
		else
		{
			//Be sure the sound is off...
			if(m_hObjectiveAlertSound)
			{
				//kill the typing sound
				g_pLTClient->KillSound(m_hObjectiveAlertSound);
				m_hObjectiveAlertSound=LTNULL;
			}
			m_bDrawAlertSolid = LTTRUE;
		}

		if (IsChoosingWeapon())
		{
			LTBOOL bIsDead = g_pGameClientShell->GetPlayerStats()->GetHealthCount()==0;

			if(m_pWeaponChooser && !bIsDead)
			{
				m_pWeaponChooser->Draw();
			}
		}

		if (m_Credits.IsInited())
		{
			m_Credits.Update();
		}


		if (m_InterfaceTimer.GetTime() > 0.0f)
		{
			m_InterfaceTimer.Draw(g_pClientDE->GetScreenSurface());
		}


		if(m_bPredVisionTransition)
		{
			HCAMERA hCam = g_pGameClientShell->GetPlayerCamera();
			DrawPredTransition(hCam);
		}

		// Handle drawing the main window...

		m_MainWnd.Draw(m_MainWnd.GetSurface());


		if(m_bDrawCloseCaption)
			m_Subtitle.Draw();



		// Update the screen fade...

		UpdateScreenFade();
	}

	if (m_MessageBox.IsVisible())
	{
        g_pLTClient->Start3D();
        g_pLTClient->StartOptimized2D();
        m_MessageBox.Draw(g_pLTClient->GetScreenSurface());
        g_pLTClient->EndOptimized2D();
        g_pLTClient->End3D();
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreMovieState()
//
//	PURPOSE:	Initialize the movie state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreMovieState(GameState eCurState)
{
    if (eCurState == GS_MOVIE) return LTFALSE;

	g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);
	m_bUseInterfaceCamera = LTTRUE;

	m_nCurMovie = 0;
	NextMovie();


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostMovieState()
//
//	PURPOSE:	Handle leaving the movie state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostMovieState(GameState eNewState)
{
    if (eNewState == GS_MOVIE) return LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreFolderState()
//
//	PURPOSE:	Initialize the Folder state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PreFolderState(GameState eCurState)
{
	if (eCurState == GS_FOLDER || eCurState == GS_MENU) return DFALSE;

	m_InterfaceResMgr.Setup();

	// Pause the game...
	g_pGameClientShell->PauseGame(DTRUE, DTRUE);
	
	SetDrawInterface(DFALSE);
	ClearScreenAlways();

	UseCursor(DTRUE);

	// Make sure menus and folders are full screen...
	memset(&m_rcMenuRestoreCamera, 0, sizeof (DRect));

	DDWORD nWidth, nHeight;
	g_pClientDE->GetSurfaceDims(g_pClientDE->GetScreenSurface(), &nWidth, &nHeight);

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	if (hCamera /*&& !g_pGameClientShell->IsWidescreen()*/)
	{
		g_pClientDE->GetCameraRect(hCamera, &m_bMenuRectRestore, &m_rcMenuRestoreCamera.left, &m_rcMenuRestoreCamera.top, &m_rcMenuRestoreCamera.right, &m_rcMenuRestoreCamera.bottom);
		g_pClientDE->SetCameraRect(hCamera, DTRUE, 0, 0, (int)nWidth, (int)nHeight);
	}

	// NOLF
	m_bUseInterfaceCamera = LTTRUE;

	// Save off the fog info...
	m_bOldFogEnable = (LTBOOL) GetConsoleInt("FogEnable", 0);
	g_pLTClient->SetGlobalLightScale(&LTVector(1.0f,1.0f,1.0f));


	// No fog in menus...
	g_pClientDE->RunConsoleString ("+FogEnable 0");

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostFolderState()
//
//	PURPOSE:	Handle leaving the Folder state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostFolderState(GameState eNewState)
{
	if (eNewState == GS_FOLDER || eNewState == GS_MENU) return DFALSE;

	m_FolderMgr.ExitFolders();

	if (eNewState != GS_LOADINGLEVEL)
	{
		int nGameMode = GAMEMODE_NONE;
		g_pClientDE->GetGameMode(&nGameMode);
		if (nGameMode == GAMEMODE_NONE) return DFALSE;

		HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
		if (hCamera && (m_rcMenuRestoreCamera.left != 0 || m_rcMenuRestoreCamera.top != 0 || m_rcMenuRestoreCamera.right != 0 || m_rcMenuRestoreCamera.bottom != 0))
		{
			g_pClientDE->SetCameraRect(hCamera, m_bMenuRectRestore, m_rcMenuRestoreCamera.left, m_rcMenuRestoreCamera.top, m_rcMenuRestoreCamera.right, m_rcMenuRestoreCamera.bottom);
		}
	}

	ClearScreenAlways(DFALSE);
	AddToClearScreenCount();
	
	SetDrawInterface(DTRUE);
	SetMenuMusic(DFALSE);

	if(g_pGameClientShell->GetMultiplayerMgr()->AllowMouseControl())
		UseCursor(LTFALSE);
	else
		UseCursor(LTTRUE);
	
//	m_InterfaceResMgr.Clean();

	g_pClientDE->ClearInput();


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePauseState()
//
//	PURPOSE:	Initialize the Pause state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PrePauseState(GameState eCurState)
{
	if (eCurState == GS_PAUSED) return DFALSE;

	// Create the "paused" surface...
	
	if (!m_hGamePausedSurface)
	{
		m_hGamePausedSurface = g_pInterfaceResMgr->CreateSurfaceFromString (g_pInterfaceResMgr->GetLargeFont(),IDS_PAUSED,hDefaultTransColor,0,0);

		CreatePauseBackground();

		SetMenuOffset();
	}

	// NOLF
	m_bUseInterfaceCamera = LTTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPauseState()
//
//	PURPOSE:	Handle leaving the Pause state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostPauseState(GameState eNewState)
{
	if (eNewState == GS_PAUSED) return DFALSE;

	// Remove the "paused" surface...

	if (m_hGamePausedSurface) 
	{
		g_pClientDE->DeleteSurface(m_hGamePausedSurface);
		m_hGamePausedSurface = DNULL;
	}

	if (m_hPauseBackground) 
	{
		g_pClientDE->DeleteSurface(m_hPauseBackground);
		m_hPauseBackground = DNULL;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePlayingState()
//
//	PURPOSE:	Initialize the Playing state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PrePlayingState(GameState eCurState)
{
	if (eCurState == GS_PLAYING) return DFALSE;

	if (eCurState != GS_DIALOGUE)
	{
		// Unpause the game...

		g_pGameClientShell->PauseGame(DFALSE);
	}

	// NOLF
    m_bUseInterfaceCamera = LTFALSE;

	// Re-set the fog...
	char buf[30];
	sprintf(buf, "FogEnable %d", (int)m_bOldFogEnable);
	g_pLTClient->RunConsoleString(buf);


	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPlayingState()
//
//	PURPOSE:	Handle leaving the Playing state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostPlayingState(GameState eNewState)
{
	if (eNewState == GS_PLAYING) return DFALSE;

	// Save off the for info...
	m_bOldFogEnable = (LTBOOL) GetConsoleInt("FogEnable", 0);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreDialogueState()
//
//	PURPOSE:	Initialize the Dialogue state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PreDialogueState(GameState eCurState)
{
	if (eCurState == GS_DIALOGUE) return DFALSE;

	// Pause game (client only)...

	// g_pGameClientShell->PauseGame(DTRUE);

	// NOLF
	m_bUseInterfaceCamera = LTFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostDialogueState()
//
//	PURPOSE:	Handle leaving the Dialogue state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostDialogueState(GameState eNewState)
{
	if (eNewState == GS_DIALOGUE) return DFALSE;

	// Unpause the game...

	// g_pGameClientShell->PauseGame(DFALSE);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreLoadingLevelState()
//
//	PURPOSE:	Initialize the LoadingLevel state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PreLoadingLevelState(GameState eCurState)
{
	if (eCurState == GS_LOADINGLEVEL) return DFALSE;

	m_nOldLoadWorldCount = m_nLoadWorldCount;
	
	// Disable light scaling when not in the playing state...
	g_pLTClient->SetGlobalLightScale(&LTVector(1.0f,1.0f,1.0f));

	m_bUseInterfaceCamera = LTTRUE;
	m_LoadingScreen.Show();
	m_bLoadFailed = LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostLoadingLevelState()
//
//	PURPOSE:	Handle leaving the LoadingLevel state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostLoadingLevelState(GameState eNewState)
{
    if (eNewState == GS_LOADINGLEVEL) return LTFALSE;

	// Don't allow the loading state to go away until the loading screen has been hidden
	if (m_LoadingScreen.IsVisible())
		return LTFALSE;

	ClearAllScreenBuffers();
	m_bLoadFailed = LTFALSE;
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreSplashScreenState()
//
//	PURPOSE:	Initialize the SplashScreen state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PreSplashScreenState(GameState eCurState)
{
	if (eCurState == GS_SPLASHSCREEN) return DFALSE;

	// Since we're going to always go to the menu state next, load the 
	// surfaces here...

	m_InterfaceResMgr.Setup();

#ifdef _DEMO
	// Set the global counter...
	g_nSplashCount = 0;
	g_hSplash = g_pClientDE->CreateSurfaceFromBitmap("Interface\\fox_logo.pcx");
#else
	// Create the splash screen...
	g_hSplash = g_pClientDE->CreateSurfaceFromBitmap(IM_SPLASH_SCREEN);
	g_pClientSoundMgr->PlaySoundLocal(IM_SPLASH_SOUND, SOUNDPRIORITY_MISC_LOW);
#endif
	if (!g_hSplash) return DFALSE;

	g_pClientDE->ClearScreen(DNULL, CLEARSCREEN_SCREEN);

	// Fade into the splash screen...

	StartScreenFadeIn(0.5f);

	//NOLF
	m_bUseInterfaceCamera = LTTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostSplashScreenState()
//
//	PURPOSE:	Handle leaving the SplashScreen state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostSplashScreenState(GameState eNewState)
{
	if (eNewState == GS_SPLASHSCREEN) return DFALSE;

	if (g_hSplash)
	{
		g_pClientDE->DeleteSurface(g_hSplash);
		g_hSplash = DNULL;
	}

	// Stop splash screen sound (if playing)...

	if (m_hSplashSound)
	{
		g_pClientDE->KillSound(m_hSplashSound);
		m_hSplashSound = DNULL;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreFailureState()
//
//	PURPOSE:	Initialize the failure state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PreFailureState(GameState eCurState)
{
	if (eCurState == GS_FAILURE) return DFALSE;


	g_fFailScreenDuration = 0.0f;

	uint32 nButeSet = g_pGameClientShell->GetPlayerStats()->GetButeSet();
	char szStr[128];
	
	if(IsAlien(nButeSet))
		sprintf(szStr, "Interface\\StatusBar\\failure_alien.pcx");
	else if(IsPredator(nButeSet))
		sprintf(szStr, "Interface\\StatusBar\\failure_predator.pcx");
	else
		sprintf(szStr, "Interface\\StatusBar\\failure.pcx");

	if (m_hFailBack)
	{
		g_pClientDE->DeleteSurface(m_hFailBack);
		m_hFailBack = DNULL;
	}

	m_hFailBack = g_pClientDE->CreateSurfaceFromBitmap(szStr);


	g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);


	// Since we're going to always go to the menu state next, load the 
	// surfaces here...

	m_InterfaceResMgr.Setup();

	g_pClientDE->ClearInput();

	// NOLF
	m_bUseInterfaceCamera = LTTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostFailureState()
//
//	PURPOSE:	Handle leaving the failure state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PostFailureState(GameState eNewState)
{
	if (eNewState == GS_FAILURE) return DFALSE;
	g_pClientDE->DeleteSurface(m_hFailBack);
	m_hFailBack = DNULL;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ProcessAdvancedOptions
//
//	PURPOSE:	Process the advanced options
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::ProcessAdvancedOptions()
{
	// Check advanced options...

	HCONSOLEVAR hVar = g_pClientDE->GetConsoleVar("DisableMusic");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_MUSIC;
	}

	hVar = g_pClientDE->GetConsoleVar("DisableSound");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_SOUND;
	}

	hVar = g_pClientDE->GetConsoleVar("DisableMovies");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_MOVIES;
	}
	
	hVar = g_pClientDE->GetConsoleVar("DisableLightMap");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions &= ~AO_LIGHTMAP;
	}
	
	hVar = g_pClientDE->GetConsoleVar("DisableFog");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions &= ~AO_FOG;
	}
	
	hVar = g_pClientDE->GetConsoleVar("DisableLines");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions &= ~AO_LINES;
	}
	
	hVar = g_pClientDE->GetConsoleVar("DisableModelFB");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions &= ~AO_MODELFULLBRITE;
	}
	
	hVar = g_pClientDE->GetConsoleVar("DisableJoystick");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions &= ~AO_JOYSTICK;
	}

	hVar = g_pClientDE->GetConsoleVar("EnableOptSurf");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions |= AO_OPTIMIZEDSURFACES;
	}
	
	hVar = g_pClientDE->GetConsoleVar("EnableTripBuf");
	if (hVar && !g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions &= ~AO_TRIPLEBUFFER;
	}
	
	hVar = g_pClientDE->GetConsoleVar("EnableTJuncs");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))		
	{
		m_dwAdvancedOptions |= AO_TJUNCTIONS;
	}
	
	// Record the original state of sound and music

	hVar = g_pClientDE->GetConsoleVar("SoundEnable");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_SOUND;
	}
	
	hVar = g_pClientDE->GetConsoleVar("MusicEnable");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_MUSIC;
	}

	hVar = g_pClientDE->GetConsoleVar("ModelFullbrite");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_MODELFULLBRITE;
	}
	
	hVar = g_pClientDE->GetConsoleVar("LightMap");
	if (hVar && g_pClientDE->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_LIGHTMAP;
	}

	// Implement any advanced options here (before renderer is started)

	hVar = g_pClientDE->GetConsoleVar("SoundEnable");
	if (!hVar && (m_dwAdvancedOptions & AO_SOUND))
	{
		g_pClientDE->RunConsoleString("SoundEnable 1");
	}
	
	hVar = g_pClientDE->GetConsoleVar("MusicEnable");
	if (!hVar && (m_dwAdvancedOptions & AO_MUSIC))
	{
		g_pClientDE->RunConsoleString("MusicEnable 1");
	}

	if (m_dwAdvancedOptions & AO_OPTIMIZEDSURFACES)
	{
		g_pClientDE->RunConsoleString("OptimizeSurfaces 1");
	}
	
	if (!(m_dwAdvancedOptions & AO_TRIPLEBUFFER))
	{
		g_pClientDE->RunConsoleString("TripleBuffer 0");
	}
	
	if (m_dwAdvancedOptions & AO_TJUNCTIONS)
	{
		g_pClientDE->RunConsoleString("FixTJunc 1");
	}
	
	if (!(m_dwAdvancedOptions & AO_FOG))
	{
		g_pClientDE->RunConsoleString("FogEnable 0");
	}
	
	if (!(m_dwAdvancedOptions & AO_LINES))
	{
		g_pClientDE->RunConsoleString("DrawLineSystems 0");
	}
	
	if (!(m_dwAdvancedOptions & AO_MODELFULLBRITE))
	{
		g_pClientDE->RunConsoleString("ModelFullBrite 0");
	}
	
	if (!(m_dwAdvancedOptions & AO_JOYSTICK))
	{
		g_pClientDE->RunConsoleString("JoystickDisable 1");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ChangeState
//
//	PURPOSE:	Change the game state.  This allows us to do pre/post state
//				change handling
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::ChangeState(GameState eNewState)
{ 
	DebugChangeState(eNewState);
	
	GameState eCurState = m_eGameState;

	// First make sure we change change to the new state from the the
	// state we are currently in...

	if (PreChangeState(eCurState, eNewState))
	{
		// Make sure the client shell has no problems with us changing
		// to this state...

		if (g_pGameClientShell->PreChangeGameState(eNewState))
		{
			m_eGameState = eNewState; 
		
			if (g_pGameClientShell->PostChangeGameState(eCurState))
			{
				// State changed successfully...

				return DTRUE;
			}

			// State NOT Changed!

			m_eGameState = eCurState;
		}
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DebugChangeState
//
//	PURPOSE:	Print out debugging info about changing states
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DebugChangeState(GameState eNewState)
{ 
#ifdef _DEBUG
//#define _DEBUG_INTERFACE_MGR
#ifdef _DEBUG_INTERFACE_MGR
	g_pClientDE->CPrint("CInterfaceMgr::ChangeState() : %f", g_pClientDE->GetTime() );
	g_pClientDE->CPrint("  Old State: %s", c_GameStateNames[m_eGameState]);
	g_pClientDE->CPrint("  New State: %s", c_GameStateNames[eNewState]);
#endif
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreChangeState
//
//	PURPOSE:	Handle pre setting of game state
//
// ----------------------------------------------------------------------- //

DBOOL CInterfaceMgr::PreChangeState(GameState eCurState, GameState eNewState)
{ 
	// Do any clean up of the old (current) state...

	switch (eCurState)
	{
		case GS_PLAYING :
		{
			if (!PostPlayingState(eNewState)) return DFALSE;
		}
		break;

		case GS_DIALOGUE :
		{
			if (!PostDialogueState(eNewState)) return DFALSE;
		}
		break;

		case GS_LOADINGLEVEL :
		{
			if (!PostLoadingLevelState(eNewState)) return DFALSE;
		}
		break;

		case GS_FOLDER :
		{
			if (!PostFolderState(eNewState)) return DFALSE;
		}
		break;

		case GS_MENU :
		{
			if (!PostFolderState(eNewState)) return DFALSE;
		}
		break;

		case GS_PAUSED :
		{
			if (!PostPauseState(eNewState)) return DFALSE;
		}
		break;

		case GS_SPLASHSCREEN :
		{
			if (!PostSplashScreenState(eNewState)) return DFALSE;
		}
		break;

		case GS_FAILURE :
		{
			if (!PostFailureState(eNewState)) return DFALSE;
		}
		break;

		case GS_MOVIE :
		{
            if (!PostMovieState(eNewState)) return LTFALSE;
		}
		break;

		default : break;
	}

	// Do any initial setup for the new state...

	switch (eNewState)
	{
		case GS_PLAYING :
		{
			return PrePlayingState(eCurState);
		}
		break;

		case GS_DIALOGUE :
		{
			return PreDialogueState(eCurState);
		}
		break;

		case GS_LOADINGLEVEL :
		{
			return PreLoadingLevelState(eCurState);
		}
		break;

		case GS_FOLDER :
		{
			return PreFolderState(eCurState);
		}
		break;

		case GS_MENU :
		{
			return PreFolderState(eCurState);
		}
		break;

		case GS_PAUSED :
		{
			return PrePauseState(eCurState);
		}
		break;

		case GS_SPLASHSCREEN :
		{
			return PreSplashScreenState(eCurState);
		}
		break;

		case GS_FAILURE :
		{
			return PreFailureState(eCurState);
		}
		break;

		case GS_MOVIE :
		{
			return PreMovieState(eCurState);
		}
		break;

		default : break;
	}

	return DTRUE;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Save
//
//	PURPOSE:	Save the interface info
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	*hWrite << m_nPrevMissionId;

	m_stats.Save(hWrite);
	m_HudMgr.Save(hWrite);
	m_Settings.Save(hWrite);

	*hWrite << m_bObjectiveNotification;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	*hRead >> m_nPrevMissionId;

	m_stats.Load(hRead);
	m_HudMgr.Load(hRead);
	m_Settings.Load(hRead);

	*hRead >> m_bObjectiveNotification;

	if(m_bObjectiveNotification)
	{
		// Set this to false, SetAlert() will
		// reset it.
		m_bObjectiveNotification = LTFALSE;
		SetAlert();
	}

	// re-init the crosshairmgr
	if(m_pCrosshairMgr)
		m_pCrosshairMgr->Init();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetCurrentFolder()
//
//	PURPOSE:	Finds out what the current folder is
//				- returns FOLDER_ID_NONE if not in a folder state
//
// --------------------------------------------------------------------------- //

eFolderID CInterfaceMgr::GetCurrentFolder()
{
	if (m_eGameState != GS_FOLDER)
	{
		return FOLDER_ID_NONE;
	}
	return m_FolderMgr.GetCurrentFolderID();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SwitchToFolder
//
//	PURPOSE:	Go to the specified folder
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SwitchToFolder(eFolderID folderID)
{
	if (m_eGameState != GS_FOLDER)
	{
		if (m_eGameState == GS_SPLASHSCREEN && folderID == FOLDER_ID_MAIN)
		{
			StartScreenFadeIn(0.5);
		}

        if (!ChangeState(GS_FOLDER)) return LTFALSE;
	}

	m_FolderMgr.SetCurrentFolder(folderID);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ForceFolderUpdate
//
//	PURPOSE:	Force the current folder to update
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::ForceFolderUpdate(eFolderID folderID)
{
    if (m_eGameState != GS_FOLDER) return LTFALSE;

	return m_FolderMgr.ForceFolderUpdate(folderID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawSFX()
//
//	PURPOSE:	Renders the currently active special effects
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::DrawSFX()
{
	UpdateInterfaceSFX();

	return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::MissionFailed
//
//	PURPOSE:	Go to the mission failure state
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::MissionFailed(int nFailStringId)
{
	if(!g_vtMissionFailed.GetFloat() || (m_eGameState != GS_PLAYING))
		return;

#ifdef _DEMO
	g_pGameClientShell->ExitLevel();
#endif

	m_nFailStringId = nFailStringId;
	ChangeState(GS_FAILURE);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ScreenDimsChanged
//
//	PURPOSE:	Handle the screen dims changing
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::ScreenDimsChanged()
{ 
	m_InterfaceResMgr.ScreenDimsChanged();

	// Update the camera rect...

	DDWORD dwWidth = 640, dwHeight = 480;
	g_pClientDE->GetSurfaceDims(g_pClientDE->GetScreenSurface(), &dwWidth, &dwHeight);

	// This may need to be changed to support in-game cinematics...

	ResetMenuRestoreCamera(0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraRect (g_pGameClientShell->GetInterfaceCamera(), LTTRUE, 0, 0, dwWidth, dwHeight);

	//reset all the camera manager camera rects
	g_pCameraMgr->SetCameraRect( g_pGameClientShell->GetPlayerCamera(),		LTRect(0,0,0,0), LTTRUE);
	g_pCameraMgr->SetCameraRect( g_pGameClientShell->GetCinematicCamera(),	LTRect(0,0,0,0), LTTRUE);
	g_pCameraMgr->SetCameraRect( g_pGameClientShell->Get3rdPersonCamera(),	LTRect(0,0,0,0), LTTRUE);

	//new for AvP3
	ReloadHUD(m_stats.GetCurrentLayout(), LTTRUE);

	m_Subtitle.ScreenDimsChanged();

	g_pClientDE->RunConsoleString("RebindTextures");
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnLButtonUp
//
//	PURPOSE:	Mouse Handling
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::OnLButtonUp(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnLButtonUp(x,y);
		return;
	}

	if (m_eGameState == GS_FOLDER)
	{
		int relX = x - g_pInterfaceResMgr->GetXOffset();
		int relY = y - g_pInterfaceResMgr->GetYOffset();
		m_FolderMgr.OnLButtonUp(relX,relY);
	}
}

void CInterfaceMgr::OnLButtonDown(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnLButtonDown(x,y);
		return;
	}
	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnLButtonDown(relX,relY);
		}	break;

	case GS_FAILURE:
		{
#ifdef _DEMO
			// just accelerate things
			if(g_fFailScreenDuration > 2.0f)
				g_fFailScreenDuration = 100.0f;
#else
			// They pressed a key - end failure screen...
			if (g_fFailScreenDuration > 1.0f)
			{
				m_FolderMgr.ClearFolderHistory();
				m_FolderMgr.AddToFolderHistory(FOLDER_ID_MAIN);
				m_FolderMgr.AddToFolderHistory(FOLDER_ID_SINGLE);

				SwitchToFolder(FOLDER_ID_LOAD);
				ChangeState(GS_FOLDER);
				StartScreenFadeIn(0.5f);
			}
#endif
		}	break;

	case GS_SPLASHSCREEN:
		{
#ifdef _DEMO
			NextSplashScreen();
#else
			if(g_pLTClient->GetTime() - m_fSplashStartTime > g_vtSplashLockTime.GetFloat())
			{
 				ChangeState(GS_MOVIE);
			}
#endif
			break;
		}

	case GS_MOVIE:
		{
			NextMovie();
			break;
		}
	}
}


void CInterfaceMgr::OnLButtonDblClick(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}

	if (m_eGameState == GS_FOLDER)
	{
		int relX = x - g_pInterfaceResMgr->GetXOffset();
		int relY = y - g_pInterfaceResMgr->GetYOffset();
		m_FolderMgr.OnLButtonDblClick(relX,relY);
	}
}

void CInterfaceMgr::OnRButtonUp(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}

	if (m_eGameState == GS_FOLDER)
	{
		int relX = x - g_pInterfaceResMgr->GetXOffset();
		int relY = y - g_pInterfaceResMgr->GetYOffset();
		m_FolderMgr.OnRButtonUp(relX,relY);
	}
}

void CInterfaceMgr::OnRButtonDown(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}

	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnRButtonDown(relX,relY);

			break;
		}
	case GS_SPLASHSCREEN:
		{
#ifdef _DEMO
			NextSplashScreen();
#else
			if(g_pLTClient->GetTime() - m_fSplashStartTime > g_vtSplashLockTime.GetFloat())
			{
 				ChangeState(GS_MOVIE);
			}
#endif
			break;
		}
	case GS_MOVIE:
		{
			NextMovie();
			break;
		}
	case GS_FAILURE:
		{
#ifdef _DEMO
			// just accelerate things
			if(g_fFailScreenDuration > 2.0f)
				g_fFailScreenDuration = 100.0f;
#else
			// They pressed a key - end failure screen...
			if (g_fFailScreenDuration > 1.0f)
			{
				m_FolderMgr.ClearFolderHistory();
				m_FolderMgr.AddToFolderHistory(FOLDER_ID_MAIN);
				m_FolderMgr.AddToFolderHistory(FOLDER_ID_SINGLE);

				SwitchToFolder(FOLDER_ID_LOAD);
				ChangeState(GS_FOLDER);
				StartScreenFadeIn(0.5f);
			}
#endif
			break;
		}
	}
}

void CInterfaceMgr::OnRButtonDblClick(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}

	if (m_eGameState == GS_FOLDER)
	{
		int relX = x - g_pInterfaceResMgr->GetXOffset();
		int relY = y - g_pInterfaceResMgr->GetYOffset();
		m_FolderMgr.OnRButtonDblClick(relX,relY);
	}
}

void CInterfaceMgr::OnMouseMove(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnMouseMove(x,y);
		return;
	}

	if (m_eGameState == GS_FOLDER)
	{
		m_CursorPos.x = x;
		m_CursorPos.y = y;
		int relX = x - g_pInterfaceResMgr->GetXOffset();
		int relY = y - g_pInterfaceResMgr->GetYOffset();
		m_FolderMgr.OnMouseMove(relX, relY);
	}
}

void CInterfaceMgr::OnMouseWheel(int x, int y, int delta)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}

	if (m_eGameState == GS_FOLDER)
	{
		int relX = x - g_pInterfaceResMgr->GetXOffset();
		int relY = y - g_pInterfaceResMgr->GetYOffset();
		m_FolderMgr.OnMouseWheel(relX, relY, delta);
	}
}

void CInterfaceMgr::UseCursor(DBOOL bUseCursor)
{
	m_bUseCursor = bUseCursor;
	if (m_bUseCursor)
	{
		g_pClientDE->RunConsoleString("CursorCenter 0");
		g_pClientDE->Cursor()->SetCursorMode(CM_Hardware);
	}
	else
	{
		g_pClientDE->RunConsoleString ("CursorCenter 1");
		g_pClientDE->Cursor()->SetCursorMode(CM_None);

	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateOverlays()
//
//	PURPOSE:	Update the overlay used as a scope crosshair
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateOverlays()
{
	m_eCurrOverlay = OVM_NONE;
	if (!nOverlayCount)
		return;
	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	if (!hCamera) return;


	DVector vPos, vU, vR, vF, vTemp;
	DRotation rRot;

	g_pClientDE->GetObjectPos(hCamera, &vPos);
	g_pClientDE->GetObjectRotation(hCamera, &rRot);
	g_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
	VEC_MULSCALAR(vTemp, vF, g_vOverlayDist);
	VEC_ADD(vPos, vPos, vTemp);

	DFLOAT fFovX = MATH_DEGREES_TO_RADIANS(FOVX_NORMAL);
	DFLOAT fFovY = MATH_DEGREES_TO_RADIANS(FOVY_NORMAL);
	g_pClientDE->GetCameraFOV(hCamera, &fFovX, &fFovY);
	DFLOAT ratioX = (float)tan(fFovX/2) / g_fFovXTan;
	DFLOAT ratioY = (float)tan(fFovY/2) / g_fFovYTan;


	DBOOL bDrawnOne = DFALSE;
	for (int i = 0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (hOverlays[i])
		{
			VEC_COPY(vTemp,g_vOverlayScale);
			VEC_MULSCALAR(vTemp, vTemp, g_vOverlayScaleMult[i]);
			vTemp.x *= ratioX;
			vTemp.y *= ratioY;

			g_pClientDE->SetObjectScale(hOverlays[i], &vTemp);
			g_pClientDE->SetObjectPos(hOverlays[i], &vPos, DTRUE);
			if (!bDrawnOne && g_pGameClientShell->IsFirstPerson() )
			{
				m_eCurrOverlay = (eOverlayMask)i;
				bDrawnOne = DTRUE;
				DDWORD dwFlags = g_pInterface->GetObjectFlags(hOverlays[i]);
				g_pInterface->SetObjectFlags(hOverlays[i], dwFlags | FLAG_VISIBLE);
			}
			else
			{
				DDWORD dwFlags = g_pInterface->GetObjectFlags(hOverlays[i]);
				g_pInterface->SetObjectFlags(hOverlays[i], dwFlags & ~FLAG_VISIBLE);

			}
		}
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateScreenFade()
//
//	PURPOSE:	Update the screen fade
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateScreenFade()
{
	if (!m_bScreenFade || m_fTotalFadeTime < 0.1f) return;

	DRect rcSrc;
	rcSrc.Init(0, 0, 2, 2);

	HSURFACE hScreen = g_pClientDE->GetScreenSurface();
	DDWORD dwWidth = 640, dwHeight = 480;
	g_pClientDE->GetSurfaceDims(hScreen, &dwWidth, &dwHeight);

	HDECOLOR hTransColor = g_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DTRUE);

	
	// Initialize the surface if necessary...

	if (!m_bFadeInitialized)
	{
		if (m_hFadeSurface)
		{
			g_pClientDE->DeleteSurface(m_hFadeSurface);
		}

		m_hFadeSurface = g_pClientDE->CreateSurface(2, 2);
		if (!m_hFadeSurface) return;

		g_pClientDE->SetSurfaceAlpha(m_hFadeSurface, 1.0f);
		g_pClientDE->FillRect(m_hFadeSurface, &rcSrc, kNearBlack);
		g_pClientDE->OptimizeSurface(m_hFadeSurface, hTransColor);

		m_fCurFadeTime = m_fTotalFadeTime;
		m_bFadeInitialized = DTRUE;
	}


	// Assume we're done...

	DBOOL bDone = DTRUE;

	
	// Fade the screen if the surface is around...

	if (m_hFadeSurface)
	{
		DFLOAT fTimeDelta = m_bFadeIn ? m_fCurFadeTime : (m_fTotalFadeTime - m_fCurFadeTime);
		
		DFLOAT fLinearAlpha = (fTimeDelta / m_fTotalFadeTime);
		fLinearAlpha = fLinearAlpha < 0.0f ? 0.0f : (fLinearAlpha > 1.0f ? 1.0f : fLinearAlpha);

		DFLOAT fAlpha = 1.0f - WaveFn_SlowOn(1.0f - fLinearAlpha);
		fAlpha = fAlpha < 0.0f ? 0.0f : (fAlpha > 1.0f ? 1.0f : fAlpha);

		g_pClientDE->SetSurfaceAlpha(m_hFadeSurface, fAlpha);

		DRect rcDest;
		rcDest.Init(0, 0, dwWidth, dwHeight);

		g_pClientDE->ScaleSurfaceToSurfaceTransparent(hScreen, m_hFadeSurface, &rcDest, &rcSrc, hTransColor);

		m_fCurFadeTime -= (g_pClientDE->GetFrameTime() < MAX_FRAME_DELTA ? g_pClientDE->GetFrameTime() : MAX_FRAME_DELTA);

		bDone = m_bFadeIn ? (fAlpha <= 0.0f) : (fAlpha >= 1.0f);
	}

	// See if we're done...

	if (bDone)
	{
		if (m_hFadeSurface)
		{
			g_pClientDE->DeleteSurface(m_hFadeSurface);
			m_hFadeSurface = DNULL;
		}

		// If we faded in we're done...

		if (m_bFadeIn)
		{
			m_bFadeInitialized	= DFALSE;
			m_bScreenFade		= DFALSE;
		}
		else
		{
			// We need to keep the screen black until it is faded 
			// back in, so we'll just make sure the screen is clear 
			// (and not set m_bScreenFade so we'll be called to 
			// clear the screen every frame until we fade back in)...

			g_pClientDE->ClearScreen(DNULL, CLEARSCREEN_SCREEN);


			// Exit the level if necessary...

			if (m_bExitAfterFade)
			{
				//[KLS] 9/12/01 - Make sure we reset m_bExitAfterFade
				m_bExitAfterFade = LTFALSE;
				HandleLoadNewLevel();
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ReloadHUD()
//
//	PURPOSE:	Dump old and reload new hud
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::ReloadHUD(DDWORD nIndex, LTBOOL bForce)
{
//	m_HudMgr.Term();
	m_HudMgr.Init(nIndex, LTTRUE, bForce);
	m_HudMgr.NewWeapon(g_pGameClientShell->GetWeaponModel()->GetWeapon());
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CreatePauseBackground()
//
//	PURPOSE:	Take a SS of the curent screen and copy it to the
//				background member.
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::CreatePauseBackground()
{
	DDWORD nScreenWidth, nScreenHeight;

	HSURFACE hScreen = g_pClientDE->GetScreenSurface();
	g_pClientDE->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

	if (m_hPauseBackground) 
	{
		g_pClientDE->DeleteSurface(m_hPauseBackground);
		m_hPauseBackground = DNULL;
	}

	m_hPauseBackground = g_pClientDE->CreateSurface(nScreenWidth, nScreenHeight);

	g_pClientDE->DrawSurfaceToSurface(m_hPauseBackground, hScreen, DNULL, 0, 0);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetMenuOffset()
//
//	PURPOSE:	Sets the menu offset point for in-game menus
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::SetMenuOffset()
{
	DDWORD nScreenWidth, nScreenHeight;

	HSURFACE hScreen = g_pClientDE->GetScreenSurface();
	g_pClientDE->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

	m_ptMenuOffset.x = (nScreenWidth-640)/2;
	m_ptMenuOffset.y = (nScreenHeight-480)/2;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TermWepChooser()
//
//	PURPOSE:	Terminates the weapon chooser
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::TermWepChooser()
{
	if(m_pWeaponChooser)
	{
		m_pWeaponChooser->Term();

		delete m_pWeaponChooser;

		m_pWeaponChooser = DNULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::InitWepChooser()
//
//	PURPOSE:	Initializes the weapon chooser
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::InitWepChooser()
{
	//find out what character we are and chose the right
	//weapon chooser and HUD to new
	DDWORD nHudType = m_stats.GetHUDType();

	if(m_nHudType != nHudType)
	{
		// record the new hud type
		m_nHudType = nHudType;
	}
	else
	{
		// don't reload the hud
		return;
	}

	if(m_pWeaponChooser)
		TermWepChooser();


	switch (nHudType)
	{
	case (0):
		{} break;
	case (1):
		{
			if(_stricmp(g_pCharacterButeMgr->GetName(m_stats.GetButeSet()), "NoHands") == 0)
				break;
			//we are the marine
			m_pWeaponChooser = new CMarineWepChooser;

			//if all is good, initialize
			if(m_pWeaponChooser)
				m_pWeaponChooser->Init();
		} break;
	case (2):
		{} break;
	case (3):
		{
			//we are the marine
			m_pWeaponChooser = new CPredatorWepChooser;

			//if all is good, initialize
			if(m_pWeaponChooser)
				m_pWeaponChooser->Init();
		} break;
	}
}


/******************************************************************/
// NOLF stuff
void CInterfaceMgr::PlayInterfaceSound(InterfaceSound eIS, char *pSnd)
{
	//if the sound is still playing and the new one is lower priority, don't play it
	// (ALM 6/6/01) -- Took this out. It was causing sounds not to play in some cases
	// where it was supposed to. I'm assuming that's because it's not doing any check
	// to see if a sound is currently playing like it states in the above comment.
/*	if (eIS < m_eIS)
	{
		return;
	}
*/	if (!pSnd)
	{
		switch (eIS)
		{
		case IS_CHANGE:
			if (m_eGameState == GS_FOLDER)
			{
				pSnd = m_FolderMgr.GetCurrentFolder()->GetChangeSound();
			}
			break;
		case IS_SELECT:
			if (m_eGameState == GS_FOLDER)
			{
				pSnd = m_FolderMgr.GetCurrentFolder()->GetSelectSound();
			}
			break;
		case IS_ESCAPE:
			pSnd = m_szEscapeSound;
			break;
		case IS_LEFT:
			pSnd = m_szSliderLeftSound;
			break;
		case IS_RIGHT:
			pSnd = m_szSliderRightSound;
			break;
		case IS_NO_SELECT:
			pSnd = m_szUnselectableSound;
			break;

		};
	}
	PlayInterfaceSound(pSnd);
	m_eIS = eIS;
}


void CInterfaceMgr::PlayInterfaceSound(char *pSoundFile)
{
	if (pSoundFile)
	{
		g_pClientSoundMgr->PlaySoundLocal(pSoundFile, SOUNDPRIORITY_MISC_MEDIUM, 0);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CreateInterfaceBackground
//
//	PURPOSE:	Create the sprite used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::CreateInterfaceBackground()
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;

	char sprName[128] = "";

	BSCREATESTRUCT bcs;

    LTVector vPos, vU, vR, vF, vTemp, vScale;
    LTRotation rRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	vScale = g_vBaseBackScale * g_pLayoutMgr->GetBackSpriteScale();


	VEC_MULSCALAR(vTemp, vF, g_fBackDist);
	VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
    bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);

	g_pLayoutMgr->GetBackSprite(sprName,sizeof(sprName));

	bcs.Filename = sprName;
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;
	bcs.nType = OT_SPRITE;
	bcs.fInitialAlpha = 1.0f;
	bcs.fFinalAlpha = 1.0f;
	bcs.fLifeTime = 1000000.0f;

	if (m_BackSprite.Init(&bcs))
	{
        m_BackSprite.CreateObject(g_pLTClient);
	}

	// TESTING - ADD A LIGHT TO LIGHT UP MODELS IN THE INTERFACE...
/*
	if (!g_hLight)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType	= OT_LIGHT;
		createStruct.m_Flags		= FLAG_VISIBLE;

        g_pLTClient->GetObjectPos(hCamera, &createStruct.m_Pos);
		//vPos -= (vF * (g_fBackDist / 2.0f));
		//createStruct.m_Pos = vPos;

		createStruct.m_Pos.y += 30.0f;

        g_hLight = g_pLTClient->CreateObject(&createStruct);

        g_pLTClient->SetLightColor(g_hLight, 0.5f, 0.5f, 0.5f);
        g_pLTClient->SetLightRadius(g_hLight, 500.0f);
	}
*/
	// END TEST!!!
}

void CInterfaceMgr::UpdateInterfaceBackground()
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;


    LTVector vPos, vU, vR, vF, vTemp, vScale;
    LTRotation rRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vTemp, vF, g_fBackDist);
	VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
	VEC_ADD(vPos, vPos, vTemp);

    g_pLTClient->SetObjectPos(m_BackSprite.GetObject(), &vPos);

}

void CInterfaceMgr::RemoveInterfaceBackground()
{
	m_BackSprite.Term();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AddInterfaceSFX
//
//	PURPOSE:	Add a SFX object to the interface
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::AddInterfaceSFX(CSpecialFX* pSFX, ISFXType eType)
{
	if (!pSFX) return;

    uint32 index = m_InterfaceSFX.FindElement(pSFX);
	if (index >= m_InterfaceSFX.GetSize())
	{
		if (m_InterfaceSFX.GetSize() < MAX_INTERFACE_SFX)
		{
			m_InterfaceSFX.Add(pSFX);
		}
	}

	HOBJECT hObj = pSFX->GetObject();
	if (g_pLTClient->GetObjectType(hObj) == OT_MODEL)
	{
		char* pAniName = LTNULL;
		switch (eType)
		{
			case IFX_NORMAL :
				pAniName = "Interface";
			break;

			case IFX_ATTACH :
				pAniName = "Hand";
			break;

			case IFX_WORLD :
			default :
				pAniName = "World";
			break;
		}

		if (pAniName)
		{
			uint32 dwAni = g_pLTClient->GetAnimIndex(hObj, pAniName);
			if (dwAni != INVALID_ANI)
			{
				g_pLTClient->SetModelAnimation(hObj, dwAni);
			}
		}
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::RemoveInterfaceSFX
//
//	PURPOSE:	Remove a SFX object from the interface
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::RemoveInterfaceSFX(CSpecialFX* pSFX)
{
    uint32 index = m_InterfaceSFX.FindElement(pSFX);
	if (index < m_InterfaceSFX.GetSize())
	{
//      g_pLTClient->CPrint("removing SFX[%d]",index);
		m_InterfaceSFX.Remove(index);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RemoveAllInterfaceSFX
//
//	PURPOSE:	Remove the 3D objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::RemoveAllInterfaceSFX()
{
	while (m_InterfaceSFX.GetSize() > 0)
	{
		//m_InterfaceSFX[0]->Term();
		m_InterfaceSFX.Remove(0);
	}

	RemoveInterfaceBackground();
}

void CInterfaceMgr::UpdateModelAnimations(LTFLOAT fCurFrameDelta)
{
	for (uint32 i = 0; i < m_InterfaceSFX.GetSize(); i++)
	{
		if (g_pLTClient->GetObjectType(m_InterfaceSFX[i]->GetObject()) == OT_MODEL)
			g_pModelLT->UpdateMainTracker(m_InterfaceSFX[i]->GetObject(), fCurFrameDelta);
	}
}

void CInterfaceMgr::UpdateModelAnimations()
{
	static LTFLOAT LastFrameTime = CWinUtil::GetTime();

	LTFLOAT fCurTime = CWinUtil::GetTime();
	LTFLOAT fCurFrameDelta = fCurTime - LastFrameTime;
	LastFrameTime = fCurTime;

	for (uint32 i = 0; i < m_InterfaceSFX.GetSize(); i++)
	{
		if (g_pLTClient->GetObjectType(m_InterfaceSFX[i]->GetObject()) == OT_MODEL)
			g_pModelLT->UpdateMainTracker(m_InterfaceSFX[i]->GetObject(), fCurFrameDelta);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateInterfaceSFX
//
//	PURPOSE:	Update the 3D Objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateInterfaceSFX()
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;

    uint32 numSfx = m_InterfaceSFX.GetSize();
	HLOCALOBJ objs[MAX_INTERFACE_SFX + 2];
	//HLOCALOBJ objs[MAX_INTERFACE_SFX + 1];

	m_BackSprite.Update();
	objs[0] = m_BackSprite.GetObject();

	UpdateModelAnimations();

	int next = 1;
    for (uint32 i = 0; i < numSfx && next < MAX_INTERFACE_SFX; i++)
	{
		if (m_InterfaceSFX[i]->Update())
		{
			objs[next] = m_InterfaceSFX[i]->GetObject();

			next++;
		}
	}

    g_pLTClient->RenderObjects(hCamera, objs, next);

	// TESTING DYNAMIC LIGHT IN INTERFACE
	/*
	if (g_hLight)
	{
		objs[i+1] = g_hLight;
        g_pLTClient->RenderObjects(hCamera, objs, numSfx + 2);
	}
	else
	{
        g_pLTClient->RenderObjects(hCamera, objs, numSfx + 1);
	}
	*/
	// END TEST
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::NextMovie
//
//	PURPOSE:	Go to the next movie, if there is one
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::NextMovie()
{
	ILTVideoMgr* pVideoMgr = g_pLTClient->VideoMgr();
	if (!pVideoMgr) return;

	if (!(GetAdvancedOptions() & AO_MOVIES) || g_vtDisableMovies.GetFloat())
	{
		SwitchToFolder(FOLDER_ID_MAIN);
		StartScreenFadeIn(0.5f);
		return;
	}

	if (m_hMovie)
	{
		pVideoMgr->StopVideo(m_hMovie);
		m_hMovie = LTNULL;
	}


	while(1)
	{
		char* pMovie = GetCurrentMovie();

		if(!pMovie)
		{
			m_nCurMovie = 0;
			m_hMovie = LTNULL;

			SwitchToFolder(FOLDER_ID_MAIN);
			StartScreenFadeIn(0.5f);

			return;
		}
		else
		{
			m_nCurMovie++;

			if(pVideoMgr->StartOnScreenVideo(pMovie, PLAYBACK_FULLSCREEN, m_hMovie) == LT_OK)
				return;
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetCurrentMovie
//
//	PURPOSE:	Get the name of the current movie to play...
//
// --------------------------------------------------------------------------- //

char* CInterfaceMgr::GetCurrentMovie()
{
	char* pMovie = LTNULL;

	switch (m_nCurMovie)
	{
		case MOVIE_FOX_LOGO:		pMovie = "movies\\foxlogo.bik";		break;
		case MOVIE_SIERRA_LOGO:		pMovie = "movies\\sierralogo.bik";	break;
		case MOVIE_LITHTECH_LOGO:	pMovie = "movies\\ltlogo.bik";		break;
		case MOVIE_MONOLITH_LOGO:	pMovie = "movies\\lithlogo.bik";	break;
		case MOVIE_AVP_INTRO:		pMovie = "movies\\avpintro.bik";	break;

		default: break;
	}

	return pMovie;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::NextDemoScreen
//
//	PURPOSE:	Go to the next marketing screen, if there is one
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::NextDemoScreen()
{
/*
	if (g_hDemo)
	{
		g_pLTClient->DeleteSurface(g_hDemo);
		g_hDemo = LTNULL;
	}
	g_nDemo++;
	if (g_nDemo < NUM_DEMO_SCREENS)
	{
		g_hDemo = g_pLTClient->CreateSurfaceFromBitmap(g_szDemoScreens[g_nDemo]);
	}
	if (!g_hDemo)
	{
		m_bSeenDemoScreens = LTTRUE;
	}
*/
}


void CInterfaceMgr::ShowDemoScreens(LTBOOL bQuit)
{
/*
	m_bQuitAfterDemoScreens = bQuit;
	m_bSeenDemoScreens = LTFALSE;
	ChangeState(GS_DEMOSCREEN);
*/
}


void CInterfaceMgr::UpdateCursor()
{
/*
	LTBOOL bHWC = (GetConsoleInt("HardwareCursor",1) > 0);
	if (bHWC != m_bUseHardwareCursor)
		UseHardwareCursor(bHWC);
	if (m_bUseCursor && !m_bUseHardwareCursor)
	{
		g_pLTClient->Start3D();
		g_pLTClient->StartOptimized2D();
		int curX = m_CursorPos.x + g_pInterfaceResMgr->GetXOffset();
		int curY = m_CursorPos.y + g_pInterfaceResMgr->GetYOffset();

		g_pLTClient->DrawSurfaceToSurfaceTransparent(g_pLTClient->GetScreenSurface(), m_InterfaceResMgr.GetSurfaceCursor(), LTNULL,
												   curX, curY, hDefaultTransColor);

		g_pLTClient->EndOptimized2D();
		g_pLTClient->End3D();

	}
*/
}

LTIntPt CInterfaceMgr::GetCursorPos()
{
	LTIntPt point(g_pInterfaceResMgr->GetXOffset(), g_pInterfaceResMgr->GetYOffset());
	point.x += m_CursorPos.x;
	point.y += m_CursorPos.y;

	return m_CursorPos;
}

LTBOOL ActivateFilterFn(HOBJECT hObj, void *pUserData)
{
	// If you modify this, please update its copy in ObjectDLL\Player.cpp

	if(!hObj) return LTFALSE;
	if(g_pPhysicsLT->IsWorldObject(hObj) == LT_YES) return LTTRUE;

	// Ignore non-solid, non-activatable objects.
	if( GetActivateType(hObj) == AT_NOT_ACTIVATABLE )
	{
		const uint32 nFlags = g_pLTClient->GetObjectFlags(hObj);

		if( !(nFlags & FLAG_SOLID) )
			return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TestWeldRange()
//
//	PURPOSE:	Test for weld range
//
// ----------------------------------------------------------------------- //

XHairMode TestWeldRange(uint32 nRange)
{
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel("Blowtorch_Barrel_Weld");
	CharacterButes*	pButes = g_pGameClientShell->GetPlayerMovement()->GetCharacterButes();

	if(!pBarrel || ! pButes) return XHM_TARGETING;

	return ((uint32)pBarrel->nRangeSqr > nRange && IsHuman(pButes->m_nId))?XHM_MARINE_WELD:XHM_TARGETING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TestCutRange()
//
//	PURPOSE:	Test for cut range
//
// ----------------------------------------------------------------------- //

XHairMode TestCutRange(uint32 nRange)
{
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel("Blowtorch_Barrel_Cut");
	CharacterButes*	pButes = g_pGameClientShell->GetPlayerMovement()->GetCharacterButes();

	if(!pBarrel || ! pButes) return XHM_TARGETING;

	return ((uint32)pBarrel->nRangeSqr > nRange && IsHuman(pButes->m_nId))?XHM_MARINE_CUT:XHM_TARGETING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TestHackRange()
//
//	PURPOSE:	Test for cut range
//
// ----------------------------------------------------------------------- //

XHairMode TestHackRange(uint32 nRange)
{
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel("MarineHackingDevice_Barrel");
	CharacterButes*	pButes = g_pGameClientShell->GetPlayerMovement()->GetCharacterButes();

	if(!pBarrel || ! pButes) return XHM_TARGETING;

	LTBOOL bCharType = IsHuman(pButes->m_nId) || IsPredator(pButes->m_nId); 

	return ((uint32)pBarrel->nRangeSqr > nRange && bCharType)?XHM_MARINE_HACK:XHM_TARGETING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TestDetonateRange()
//
//	PURPOSE:	Test for cut range
//
// ----------------------------------------------------------------------- //

XHairMode TestDetonateRange(uint32 nRange)
{
	BARREL* pBarrel = g_pWeaponMgr->GetBarrel("Hotbomb_Barrel");
	CharacterButes*	pButes = g_pGameClientShell->GetPlayerMovement()->GetCharacterButes();

	if(!pBarrel || ! pButes) return XHM_TARGETING;

	return ((uint32)pBarrel->nRangeSqr > nRange && IsPredator(pButes->m_nId))?XHM_PRED_DETONATE:XHM_TARGETING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TestMarineActivatable()
//
//	PURPOSE:	Test for Marine activatable
//
// ----------------------------------------------------------------------- //

XHairMode TestMarineActivatable(CharacterButes*	pButes)
{
	if(!pButes) return XHM_TARGETING;

	return IsHuman(pButes->m_nId)?XHM_DEFAULT_ACTIVATE:XHM_TARGETING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TestPredatorActivatable()
//
//	PURPOSE:	Test for Marine activatable
//
// ----------------------------------------------------------------------- //

XHairMode TestPredatorActivatable(CharacterButes*	pButes)
{
	if(!pButes) return XHM_TARGETING;

	return IsPredator(pButes->m_nId)?XHM_DEFAULT_ACTIVATE:XHM_TARGETING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::TestExosuitActivatable()
//
//	PURPOSE:	Test for Marine activatable
//
// ----------------------------------------------------------------------- //

XHairMode TestExosuitActivatable(CharacterButes*	pButes)
{
	if(!pButes) return XHM_TARGETING;

	return IsExosuit(pButes->m_nId)?XHM_DEFAULT_ACTIVATE:XHM_TARGETING;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateActivate()
//
//	PURPOSE:	Can we activate the object in front of us
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateActivate(LTVector vPos, LTVector vDir)
{
	// Cast ray to see if there is an object to activate...
	uint32 nButeSet = g_pGameClientShell->GetPlayerStats()->GetButeSet();
	if(IsAlien(nButeSet) || (IsExosuit(nButeSet) && g_pGameClientShell->IsMultiplayerGame()))
	{
		m_pCrosshairMgr->SetActivateMode(XHM_TARGETING);
		return;
	}
			
	LTVector vDims, vTemp, vPos2;
	g_pLTClient->Physics()->GetObjectDims(g_pLTClient->GetClientObject(), &vDims);

	CharacterButes*	pButes = g_pGameClientShell->GetPlayerMovement()->GetCharacterButes();
	DFLOAT fDist = (vDims.x + vDims.z)/2.0f + pButes->m_fActivateDist;
	VEC_MULSCALAR(vTemp, vDir, fDist);
	VEC_ADD(vPos2, vPos, vTemp);
    
	IntersectQuery IQuery;
	memset(&IQuery, 0, sizeof(IQuery));

	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To = vPos2;
	IQuery.m_FilterFn  = ActivateFilterFn;
    
	IQuery.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS ;

	XHairMode eRval = XHM_TARGETING;

	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		switch(GetActivateType(IInfo.m_hObject))
		{
		case (AT_NORMAL):	eRval = XHM_DEFAULT_ACTIVATE;	break;
		case (AT_MARINE_PICKUP):eRval = TestMarineActivatable(pButes);	break;
		case (AT_PRED_PICKUP):	eRval = TestPredatorActivatable(pButes);break;
		case (AT_EXO_PICKUP):	eRval = TestExosuitActivatable(pButes);	break;
		case (AT_WELD):
			{
				if (g_pGameClientShell->GetGameType()->IsMultiplayer())
					eRval = TestWeldRange((uint32)(vPos-IInfo.m_Point).MagSqr());		
				break;
			}
		case (AT_CUT):		eRval = TestCutRange((uint32)(vPos-IInfo.m_Point).MagSqr());		break;
		case (AT_HACK):		eRval = TestHackRange((uint32)(vPos-IInfo.m_Point).MagSqr());		break;
		default:			eRval = XHM_TARGETING;			break;
		}
	}

	m_pCrosshairMgr->SetActivateMode(eRval);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnChar()
//
//	PURPOSE:	Handle OnChar messages
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnChar(char c)
{
	if (c < ' ') return;
	
	if (m_eGameState == GS_FOLDER)
	{
		m_FolderMgr.HandleChar(c);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::StartMission()
//
//	PURPOSE:	Handle game state change
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::StartMission(int nPrevMissionId)
{
	m_nPrevMissionId = nPrevMissionId;
	const MissionButes & prevMissionButes = g_pMissionMgr->GetMissionButes(m_nPrevMissionId);
	

	g_pGameClientShell->GetGameType()->Set(SP_STORY);
	g_pGameClientShell->GetGameType()->SetStartMission(LTTRUE);

	ChangeState(GS_LOADINGLEVEL);

	//load the new level
	if( g_pGameClientShell->LoadWorld( const_cast<char*>(prevMissionButes.m_szNextLevel) ) )
	{
		ChangeState(GS_PLAYING);
	}
	else
	{
		g_pInterfaceMgr->LoadFailed();
		SwitchToFolder(FOLDER_ID_MAIN);
		StartScreenFadeIn(0.5f);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::LoadCustomLevel()
//
//	PURPOSE:	Handles loading a custom level.
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::LoadCustomLevel(const char * szWorldName)
{
	ASSERT( szWorldName );
	if( !szWorldName || !szWorldName[0] )
		return LTFALSE;

	// Be sure our "previous" mission is invalid.
	m_nPrevMissionId = -1;

	// Set the difficulty level.
	int nDifficulty = int( g_vtCustomDifficulty.GetFloat() );
	nDifficulty = LTCLAMP(nDifficulty,DIFFICULTY_PC_EASY,DIFFICULTY_PC_INSANE);

	g_pGameClientShell->GetGameType()->SetDifficulty(nDifficulty);

	//clear out inventory and any mission related data
	m_stats.ResetInventory();

	// Be in story mode!
	g_pGameClientShell->GetGameType()->Set(SP_STORY);

	// act like we are starting a mission.
	g_pGameClientShell->GetGameType()->SetStartMission(LTTRUE);

	ChangeState(GS_LOADINGLEVEL);

	//load the new level
	if( g_pGameClientShell->LoadWorld( const_cast<char*>(szWorldName) ) )
	{
		ChangeState(GS_PLAYING);
	}
	else
	{
		g_pInterfaceMgr->LoadFailed();
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawObjectives()
//
//	PURPOSE:	Draw the objectives to the screen
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DrawObjectives()
{
	//get screen handle and dims
	HSURFACE	hScreen = g_pClientDE->GetScreenSurface();
	DDWORD		nHeight, nWidth;
	g_pClientDE->GetSurfaceDims (hScreen, &nWidth, &nHeight);

	//get the dispaly font
	CLTGUIFont* pFont = GetObjectiveFont();
	if(!pFont) return;

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//display the background
	g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hObjBackground, &LTRect(0,0,nWidth, nHeight), LTNULL);


	// Set the draw point and draw the overview here...

	LTIntPt pt;

	pt.x = (nWidth >> 1) - 275;
	pt.y = nHeight >> 3;

	int nID = m_stats.GetObjectiveOverview();

	if(nID != -1)
	{
		LITHFONTDRAWDATA DrawData;
		LITHFONTSAVEDATA SaveData;

		//clear the struct
		memset(&DrawData, 0, sizeof(LITHFONTDRAWDATA));

		//set drawdata elements
		DrawData.dwFlags		= LTF_DRAW_FORMATTED; 
		DrawData.byJustify		= LTF_JUSTIFY_LEFT;
		DrawData.dwFormatWidth	= 550;
		DrawData.fAlpha			= 1.0f;


		HSTRING	hStr = g_pLTClient->FormatString(nID);

		//Create the display string
		pFont->Draw(	g_pLTClient->GetStringData(hStr), 
						hScreen, 
						&DrawData,
						pt.x, pt.y, &SaveData);

		// Draw the border...
		LTIntPt ptSize(700, 5);
		LTIntPt ptBorder(nWidth >> 1, pt.y-25);

		Draw_Style1_Background(ptBorder, ptSize, 50, hScreen);

		LTIntPt p;
		p = pFont->GetTextExtentsFormat(hStr, DrawData.dwFormatWidth);
		g_pClientDE->FreeString(hStr);
		pt.y += p.y+50;

		ptBorder.y = pt.y - 25;
		Draw_Style1_Background(ptBorder, ptSize, 50, hScreen);
	}
	else
	{
		pt.y = nHeight >> 2;
	}

	
	// Reset the starting x coord and draw the objectives.

	pt.x = (nWidth >> 1) - 200;

	ObjectiveData* pData = LTNULL;
	LTBOOL bPlaySound=LTFALSE;
	for (int i=0; i<MAX_OBJECTIVES; i++)
	{
		pData = m_stats.GetObjectiveData(i);

		if(pData && pData->nId != 0)
		{
			if(pData->eState != OS_REMOVED)
			{
				ObjectiveButes bute = g_pObjectiveButeMgr->GetObjectiveButes(pData->nId);

				HSTRING	hStr = g_pLTClient->FormatString(bute.m_nObjectiveText);

				//Create the display string
				pFont->Draw(	g_pLTClient->GetStringData(hStr), 
								hScreen, 
								&m_DrawData,
								pt.x, pt.y, 
								&m_SaveData[i]);

				//draw the check-box
				g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
																m_hObjCheckBox[pData->eState], 
																LTNULL, pt.x-40, pt.y-10, 
																hTransColor);

				LTIntPt p;
				p = pFont->GetTextExtentsFormat(hStr, m_DrawData.dwFormatWidth);
				g_pClientDE->FreeString(hStr);
				pt.y += p.y+15;

				//check to see if this was typing
				if(m_SaveData[i].byLastState != LTF_STATE_DRAW_FINISHED)
					bPlaySound = LTTRUE;
			}
		}
		else
			break;
	}

	//now deal with the sound then return
	if(bPlaySound)
	{
		//ok we sould be playing the sound, lets see if we already are
		if(!m_hObjectiveSound)
			m_hObjectiveSound = g_pClientSoundMgr->PlaySoundLocal("Typing");
	}
	else
	{
		//ok we are done with the sound
		if(m_hObjectiveSound)
		{
			//kill the typing sound
			g_pLTClient->KillSound(m_hObjectiveSound);
			m_hObjectiveSound=LTNULL;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ClearObjectiveDisplay()
//
//	PURPOSE:	Clean up the objectives display stuff
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::ClearObjectiveDisplay()
{
	//clear the saved data
	memset(m_SaveData, 0, MAX_OBJECTIVES * sizeof(LITHFONTSAVEDATA));

	if(m_hObjectiveSound)
	{
		//kill the typing sound
		g_pLTClient->KillSound(m_hObjectiveSound);
		m_hObjectiveSound=LTNULL;
	}

	if(m_hObjBackground)
	{
		g_pClientDE->DeleteSurface(m_hObjBackground);
		m_hObjBackground = LTNULL;
	}

	//clean up the surfaces
	for(int i=0 ; i<3; i++)
	{
		g_pClientDE->DeleteSurface(m_hObjCheckBox[i]);
		m_hObjCheckBox[i] = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CreateObjectiveDisplay()
//
//	PURPOSE:	Create the objectives display stuff
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::CreateObjectiveDisplay()
{
	//set up the background and other stuff

	//set transparent color
	HDECOLOR hTransColor = SETRGB_T(0,0,0);

	//create the surface and draw the background
	m_hObjBackground = g_pClientDE->CreateSurfaceFromBitmap("Interface\\StatusBar\\Marine\\ObjectiveBackground.pcx");
	g_pClientDE->OptimizeSurface (m_hObjBackground, hTransColor);
	g_pClientDE->SetSurfaceAlpha (m_hObjBackground, 0.5f);

	//load up the checkboxes
	m_hObjCheckBox[0] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\check-off.pcx");
	m_hObjCheckBox[1] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\check-on.pcx");
	m_hObjCheckBox[2] = g_pClientDE->CreateSurfaceFromBitmap("Interface\\check-remove.pcx");

	for(int i=0 ; i<3; i++)
		g_pClientDE->OptimizeSurface (m_hObjCheckBox[i], hTransColor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetObjectiveDrawData()
//
//	PURPOSE:	Sets up the objectives draw data
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::SetObjectiveDrawData()
{
	//clear the struct
	memset(&m_DrawData, 0, sizeof(LITHFONTDRAWDATA));
	memset(&m_OVDrawData, 0, sizeof(LITHFONTDRAWDATA));

	//set drawdata elements
	m_DrawData.dwFlags			= LTF_DRAW_FORMATTED |LTF_DRAW_TIMED | LTF_TIMED_LETTERS; 
	m_DrawData.byJustify		= LTF_JUSTIFY_LEFT;
	m_DrawData.dwFormatWidth	= 400;
	m_DrawData.nLetterSpace		= 0;
	m_DrawData.nLineSpace		= 0;
	m_DrawData.fAlpha			= 1.0f;
	m_DrawData.fLetterDelay		= 0.01f;		// Time it takes to draw one letter (0.1 would draw 10 letters a second)

	m_OVDrawData.dwFlags		= LTF_DRAW_FORMATTED; 
	m_OVDrawData.byJustify		= LTF_JUSTIFY_CENTER;
	m_OVDrawData.dwFormatWidth	= 550;
	m_OVDrawData.fAlpha			= 1.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawObjectiveNotification()
//
//	PURPOSE:	Draw the objective notification to the screen
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DrawObjectiveNotification()
{
	static LTBOOL	bDraw = LTTRUE;
	static LTFLOAT	fFlashTime = g_pLTClient->GetTime();
	LTBOOL			bPlaySound=LTFALSE;

	//see if it is time to flash
	if(!m_bDrawAlertSolid)
	{
		if((g_pLTClient->GetTime() - fFlashTime) > 0.75f)
		{
			bDraw = !bDraw;
			fFlashTime = g_pLTClient->GetTime();
			m_nObjectiveFlashCount++;

			if(m_nObjectiveFlashCount > g_vtObjectiveFlashCount.GetFloat() && !bDraw)
			{
				//time's up!  Reset counter and stop flashing
				if(m_hObjectiveAlertSound)
				{
					//kill the typing sound
					g_pLTClient->KillSound(m_hObjectiveAlertSound);
					m_hObjectiveAlertSound=LTNULL;
				}
				m_bDrawAlertSolid = LTTRUE;
				g_pLTClient->SetSurfaceAlpha(m_hAlertImage, 0.5f);
			}
		}
	}

	if(bDraw || m_bDrawAlertSolid)
	{
		// Find out where to draw the image
		LTIntPt pt = GetAlertCoords();

		// Now draw it...
		HSURFACE	hScreen = g_pLTClient->GetScreenSurface();

		//set transparent color
		HDECOLOR hTransColor = SETRGB_T(0,0,0);

		//draw
		g_pLTClient->DrawSurfaceToSurfaceTransparent(	hScreen, 
														m_hAlertImage, 
														LTNULL,
														pt.x, 
														pt.y, 
														hTransColor);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawObjectiveNotification()
//
//	PURPOSE:	Draw the objective notification to the screen
//
// ----------------------------------------------------------------------- //

LTIntPt CInterfaceMgr::GetAlertCoords()
{
	//get screen handle and dims
	HSURFACE	hScreen = g_pClientDE->GetScreenSurface();
	DDWORD		nScreenHeight, nScreenWidth, nImageHeight, nImageWidth;
	g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	g_pLTClient->GetSurfaceDims (m_hAlertImage, &nImageWidth, &nImageHeight);

	LTIntPt ptRval;

	uint32 nBute = m_stats.GetButeSet();

	if(IsAlien(nBute))
	{
		ptRval.x = (nScreenWidth>>1) - (nImageWidth>>1);
		ptRval.y = 0;
	}
	else if(IsPredator(nBute))
	{
		ptRval.x = nScreenWidth - nImageWidth - 10;
		ptRval.y = nScreenHeight - nImageHeight - 10;
	}
	else if(IsExosuit(nBute))
	{
		ptRval.x = nScreenWidth - nImageWidth - 10;
		ptRval.y = 10;
	}
	else
	{
		ptRval.x = (nScreenWidth>>1) - (nImageWidth>>1);
		ptRval.y = nScreenHeight - nImageHeight - 10;
	}

	return ptRval;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetObjectiveFont()
//
//	PURPOSE:	Get the proper font for this character
//
// ----------------------------------------------------------------------- //

CLTGUIFont* CInterfaceMgr::GetObjectiveFont()
{
	return g_pInterfaceResMgr->GetObjectiveFont();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetObjectiveFont()
//
//	PURPOSE:	Get the proper font for this character
//
// ----------------------------------------------------------------------- //

CLTGUIFont* CInterfaceMgr::GetCharacterFont()
{
	//get the proper font
	uint32 nBute = m_stats.GetButeSet();

	if(IsAlien(nBute))
		return g_pInterfaceResMgr->GetAlienFont();
	else if(IsPredator(nBute))
		return g_pInterfaceResMgr->GetPredatorFont();

	return g_pInterfaceResMgr->GetMarineFont();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetAlert()
//
//	PURPOSE:	Start the alert
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::SetAlert()
{
	// Only do the alert if "ObjectiveMessages" is enabled and 
	// if we are not already displaying the objectives.
	LTBOOL bDoAlert = (LTBOOL)GetConsoleInt("ObjectiveMessages",1) && !m_bDisplayObjectives;

	// if we are alrady alerting, don't reset....
	if(m_bObjectiveNotification) return;

	if(bDoAlert)
	{
		// reset the alert flashing...
		m_bDrawAlertSolid = LTFALSE;

		//set the draw flag
		m_bObjectiveNotification = LTTRUE;

		//play the alert sound
		if(!m_hObjectiveAlertSound)
			m_hObjectiveAlertSound = g_pClientSoundMgr->PlaySoundLocal("Objective_Alert");

		//time stamp
		m_fAlertStartTime = g_pLTClient->GetTime();

		//load up the image
		uint32 nBute = m_stats.GetButeSet();

		if(IsAlien(nBute))
			m_hAlertImage = g_pInterfaceResMgr->GetSharedSurface("interface\\statusbar\\alien\\message_alien.pcx");
		else if(IsPredator(nBute))
			m_hAlertImage = g_pInterfaceResMgr->GetSharedSurface("interface\\statusbar\\predator\\message_predator.pcx");
		else
			m_hAlertImage = g_pInterfaceResMgr->GetSharedSurface("interface\\statusbar\\marine\\message_marine.pcx");

		//now set the alpha to full
		g_pLTClient->SetSurfaceAlpha(m_hAlertImage, 1.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ClearObjectiveDisplay()
//
//	PURPOSE:	Clean up the objectives display stuff
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::ClearObjectiveAlert()
{
	m_bObjectiveNotification	= LTFALSE;
	m_nObjectiveFlashCount		= 0;
	m_bDrawAlertSolid			= LTFALSE;

	if(m_hObjectiveAlertSound)
	{
		//kill the typing sound
		g_pLTClient->KillSound(m_hObjectiveAlertSound);
		m_hObjectiveAlertSound=LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HideLoadScreen()
//
//	PURPOSE:	Called externally to hide the loading screen
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::HideLoadScreen()
{
	m_LoadingScreen.Hide();
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetLoadLevelString()
//
//	PURPOSE:	Sets the name of the world to be displayed on the loading screen
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::SetLoadLevelString(HSTRING hWorld)
{
	m_LoadingScreen.SetWorldName(hWorld);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetLoadLevelPhoto()
//
//	PURPOSE:	Sets the photo of the world to be displayed on the loading screen
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::SetLoadLevelPhoto(char *pszPhoto)
{
	m_LoadingScreen.SetWorldPhoto(pszPhoto);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetDamageTintColor()
//
//	PURPOSE:	Figgure out the damage tint color
//
// ----------------------------------------------------------------------- //
LTVector CInterfaceMgr::GetDamageTintColor(DamageType dt)
{
	switch (dt)
	{
	case (DT_ACID):		// "green" damage types
	case (DT_POISON):	
		return LTVector(188, 190, 1) / LTVector(255, 255, 255);

	case(DT_ELECTRIC):	// "blue" damage types
	case(DT_FREEZE):	
		return LTVector(118, 200, 251) / LTVector(255, 255, 255);

	default:			// default to red "normal" damage
		return LTVector(190, 1, 1) / LTVector(255, 255, 255);
	}
}


#ifdef _DEMO

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::NextSplashScreen()
//
//	PURPOSE:	Change up the splash screen
//
// ----------------------------------------------------------------------- //


void CInterfaceMgr::NextSplashScreen()
{
	// Nuke the old screen
	if (g_hSplash)
	{
		g_pClientDE->DeleteSurface(g_hSplash);
		g_hSplash = DNULL;
	}

	switch ((const uint8)g_nSplashCount)
	{
	case (0):
		g_hSplash = g_pClientDE->CreateSurfaceFromBitmap("Interface\\sierra_logo.pcx");
		break;
	case (1):
		g_hSplash = g_pClientDE->CreateSurfaceFromBitmap("Interface\\mono_logo.pcx");
		break;
	case (2):
		g_hSplash = g_pClientDE->CreateSurfaceFromBitmap("Interface\\splash.pcx");
		break;
	case (3):
		SwitchToFolder(FOLDER_ID_MAIN);
		ChangeState(GS_FOLDER);
		StartScreenFadeIn(0.5f);
		return;
	}

	// Increment the counter
	++g_nSplashCount;

	// reset the fade
	StartScreenFadeIn(0.5f);
	bDidFadeOut = LTFALSE;
	fFadeInStart = g_pLTClient->GetTime();
}

#endif

// ----------------------------------------------------------------------- //
// Set the captioning object up with the data it needs to operate
// ----------------------------------------------------------------------- //
void CInterfaceMgr::SetCaption(const char* szFileName, LTVector vSpeakerPos, LTFLOAT fRadius, LTFLOAT fDuration)
{
	int nStringId = 0;
	char szTemp[128];

	SAFE_STRCPY(szTemp,szFileName);
	
	char *pStr = strrchr(szTemp,'.');
	*pStr = 0;
	pStr = strrchr(szTemp,'\\');
	pStr++;

#ifdef _DEBUG
	g_pLTClient->CPrint("set caption:%s",pStr);
#endif // _DEBUG

	nStringId = atoi(pStr);
	if (nStringId)
		m_Subtitle.Show(nStringId, vSpeakerPos, fRadius, fDuration);
	else
	{
		ASSERT(0);
		m_Subtitle.Clear();
	}
}

void CInterfaceMgr::SetCaption(int nStringId, LTVector vSpeakerPos, LTFLOAT fRadius, LTFLOAT fDuration)
{
	m_Subtitle.Show(nStringId, vSpeakerPos, fRadius, fDuration);
}

void CInterfaceMgr::StopCaptions()
{
	m_Subtitle.Clear();
}

// -------------------------------------------------------------------- //
//
// FUNCTION:	TeleType::Draw_Style1_Background()
//
// PURPOSE:		Draws the background
//
// -------------------------------------------------------------------- //

void CInterfaceMgr::Draw_Style1_Background(LTIntPt ptPos, LTIntPt ptSize, int nSteps, HSURFACE hSurf)
{
	// Calculate the width of each step...
	int nStepWidth = (int)((LTFLOAT)(ptSize.x / 2) / 30.0f);

	LTFLOAT fAlpha;
	LTFLOAT fAlphaStep = (0.01f - 1.0f) / 30.0f;

	int nLeftX = ptPos.x;
	int nRightX = ptPos.x - nStepWidth;

	// Go through each step and blit it to the screen...
	for(int i = 0; i < nSteps; i++)
	{
		fAlpha = 1.0f + ((LTFLOAT)i * fAlphaStep);

		fAlpha = (1.0f - fAlpha);
		fAlpha *= fAlpha;
		fAlpha = (1.0f - fAlpha);

		g_pLTClient->SetSurfaceAlpha(m_hBackground, fAlpha);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hBackground, &LTRect(nRightX, ptPos.y, nRightX + nStepWidth, ptPos.y + ptSize.y), LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hSurf, m_hBackground, &LTRect(nLeftX, ptPos.y, nLeftX + nStepWidth, ptPos.y + ptSize.y), LTNULL);

		nRightX += nStepWidth;
		nLeftX -= nStepWidth;
	}
}



// hides or shows cursor based on current game state
void CInterfaceMgr::UpdateCursorState()
{
	switch (m_eGameState)
	{
		case GS_PLAYING:
		case GS_LOADINGLEVEL:
		case GS_SPLASHSCREEN:
		case GS_PAUSED:
		case GS_FAILURE:
		case GS_MOVIE:
		case GS_UNDEFINED:
		{
			UseCursor(m_MessageBox.IsVisible());
			break;
		}

		case GS_DIALOGUE:
		case GS_MENU:
		case GS_FOLDER:
		default:
		{
			UseCursor(LTTRUE);
			break;
		}
	}
}
