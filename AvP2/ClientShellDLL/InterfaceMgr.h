// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.h
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// ----------------------------------------------------------------------- //

#ifndef __INTERFACE_MGR_H__
#define __INTERFACE_MGR_H__

#include "iltclient.h"
#include "iltcursor.h"

#include "PlayerStats.h"
#include "GameSettings.h"
#include "MessageMgr.h"
#include "InterfaceResMgr.h"
#include "LayoutMgr.h"
#include "LTWnd.h"
#include "LTDialogueWnd.h"
#include "WeaponChooser.h"
#include "InterfaceTimer.h"
#include "LoadingScreen.h"
#include "MessageBox.h"
#include "AutoTargetMgr.h"
#include "Overlays.h"
#include "HudMgr.h"
#include "Subtitle.h"
#include "FolderMgr.h"
#include "CrosshairMgr.h"
#include "ObjectivesButeMgr.h"
#include "DamageTypes.h"
#include "iltvideomgr.h"
#include "credits.h"

#define AO_MUSIC				(1<<0)
#define AO_SOUND				(1<<1)
#define AO_MOVIES				(1<<2)
#define AO_LIGHTMAP				(1<<3)
#define AO_FOG					(1<<4)
#define AO_LINES				(1<<5)
#define AO_MODELFULLBRITE		(1<<6)
#define AO_JOYSTICK				(1<<7)
#define AO_OPTIMIZEDSURFACES	(1<<8)
#define AO_TRIPLEBUFFER			(1<<9)
#define AO_TJUNCTIONS			(1<<10)

// Game states

enum GameState 
{
	GS_UNDEFINED=0,
	GS_PLAYING,
	GS_LOADINGLEVEL,
	GS_SPLASHSCREEN,
	GS_DIALOGUE,
	GS_MENU,
	GS_FOLDER,
	GS_PAUSED,
	GS_FAILURE,
	GS_MOVIE
};


// NOLF
enum ISFXType
{
	IFX_NORMAL,
	IFX_WORLD,
	IFX_ATTACH
};

enum InterfaceSound
{
	IS_NONE,
	IS_CHANGE,
	IS_SELECT,
	IS_ESCAPE,
	IS_LEFT,
	IS_RIGHT,
	IS_NO_SELECT,
};

const HLTCOLOR kWhite		= SETRGB(255,255,255);
const HLTCOLOR kLtGray		= SETRGB(128, 128, 128);
const HLTCOLOR kGray		= SETRGB(64,64,64);
const HLTCOLOR kBlack		= SETRGB(0,0,0);
const HLTCOLOR kNearBlack	= SETRGB(10,10,10);
const HLTCOLOR kTransBlack	= SETRGB_T(0,0,0);
const HLTCOLOR kSolid		= 0x7FFFFFFF;

class CInterfaceMgr;
extern CInterfaceMgr* g_pInterfaceMgr;

class CInterfaceMgr
{
	public :

		CInterfaceMgr();
		virtual ~CInterfaceMgr();
	
		LTBOOL	Init();
		void	Term();

		void	ResetSharedSurfaces();

		void	OnEnterWorld(LTBOOL bRestoringGame=LTFALSE);
		void	OnExitWorld();

		LTBOOL	OnCommandOn(int command);
		LTBOOL	OnCommandOff(int command);
		LTBOOL	OnKeyDown(int key, int rep);
		LTBOOL	OnKeyUp(int nKey);
		LTBOOL	OnMessage(DBYTE messageID, HMESSAGEREAD hMessage);
		LTBOOL	OnEvent(DDWORD dwEventID, DDWORD dwParam);

		void	StartScreenFadeIn(DFLOAT fFadeTime)	 { m_bFadeInitialized = LTFALSE; m_bScreenFade = LTTRUE; m_fTotalFadeTime = fFadeTime; m_bFadeIn = LTTRUE; }
		void	StartScreenFadeOut(DFLOAT fFadeTime) { m_bFadeInitialized = LTFALSE; m_bScreenFade = LTTRUE; m_fTotalFadeTime = fFadeTime; m_bFadeIn = LTFALSE; }

		LTBOOL	IsFadingIn()		{ return (m_bScreenFade && m_bFadeIn); }
		LTBOOL	IsFadingOut()		{ return (m_bScreenFade && !m_bFadeIn); }
		void	GetFadeTimes(LTFLOAT &fCurrent, LTFLOAT &fTotal)	{ fCurrent = m_fCurFadeTime; fTotal = m_fTotalFadeTime; }

		//mouse messages
		void OnLButtonUp(int x, int y);
		void OnLButtonDown(int x, int y);
		void OnLButtonDblClick(int x, int y);
		void OnRButtonUp(int x, int y);
		void OnRButtonDown(int x, int y);
		void OnRButtonDblClick(int x, int y);
		void OnMouseMove(int x, int y);
		void OnMouseWheel(int x, int y, int delta);
		void OnChar(char c);


		LTBOOL	PreUpdate();
		LTBOOL	Update();
		LTBOOL	PostUpdate();

		LTBOOL	Draw();

		LTBOOL		ChangeState(GameState eNewState);
		void		DebugChangeState(GameState eNewState);
		GameState	GetGameState() const { return m_eGameState; }

		void	MissionFailed(int nFailStringId);

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);

		void	SetDrawInterface(LTBOOL bDraw)			{ m_bDrawInterface = bDraw; }
		void	SetSwitchingRenderModes(LTBOOL bSwitch)	{ SetMenuOffset(); m_bSwitchingModes = bSwitch; }

		void	ResetMenuRestoreCamera(int nLeft, int nTop, int nRight, int nBottom);

		void	ClearScreenAlways(LTBOOL bYes = LTTRUE)	{ m_bClearScreenAlways = bYes; }
		void	AddToClearScreenCount()					{ m_nClearScreenCount = 3; }
		void	ZeroClearScreenCount()					{ m_nClearScreenCount = 0; }
		void	ClearAllScreenBuffers();

		LTBOOL	SetMenuMusic(LTBOOL bMusicOn);

		CGameSettings*	GetSettings()		{ return &m_Settings; }
		CPlayerStats*	GetPlayerStats()	{ return &m_stats; }
		CMessageMgr*	GetMessageMgr()		{ return &m_messageMgr; }
		CHudMgr*		GetHudMgr()			{ return &m_HudMgr;	} 
		CCrosshairMgr*	GetCrosshairMgr()	{ return m_pCrosshairMgr; }
		CAutoTargetMgr*	GetAutoTargetMgr()	{ return m_pAutoTargetMgr; }
		CWeaponChooser*	GetWeaponChooser()	{ return m_pWeaponChooser; }
		CCredits*		GetCredits()		{ return &m_Credits; }

		//new for avp3
		DIntPt			GetMenuOffset()		{ return m_ptMenuOffset; }

		DDWORD			GetAdvancedOptions() const { return m_dwAdvancedOptions; }
		
		void		EnableCrosshair(LTBOOL bOn)		 { m_pCrosshairMgr->EnableCrosshair(bOn); }
		LTBOOL		IsCrosshairEnabled()			 { return m_pCrosshairMgr->CrosshairEnabled(); }
		LTBOOL		IsCrosshairOn()					 { return m_pCrosshairMgr->CrosshairOn(); }
		void		DrawPlayerStats(LTBOOL bOn=DTRUE) { m_bDrawPlayerStats = bOn; }
		void		DrawCloseCaption(LTBOOL bOn=DTRUE) { m_bDrawCloseCaption = bOn; }

		// Caption functions
		void SetCaption(const char* szFileName, LTVector vSpeakerPos, LTFLOAT fRadius=0.0f, LTFLOAT fDuration=-1.0f);
		void SetCaption(int nStringId, LTVector vSpeakerPos, LTFLOAT fRadius=0.0f, LTFLOAT fDuration=-1.0f);
		void StopCaptions();


		LTBOOL		IsChoosingWeapon()				 { return m_pWeaponChooser?m_pWeaponChooser->IsOpen():LTFALSE;}
//		LTBOOL		IsChoosingAmmo()				 { return m_AmmoChooser.IsOpen();}

		void		UpdatePlayerStats(DBYTE nThing, DBYTE nType1, DBYTE nType2, DFLOAT fAmount, LTBOOL bNotify);
//		void		UpdateWeaponStats(DBYTE nWeaponId, DBYTE nAmmoId, DDWORD dwAmmo);

		void		ToggleInterfaceAdjust()			 {  }

		void		ScreenDimsChanged();

		LTIntPt		GetCursorPos();
		LTBOOL		IsCursorUsed() {return m_bUseCursor;}
		void		UseCursor(LTBOOL bUseCursor);
		void		UpdateCursorState(); // hides or shows cursor based on current game state

		void			UpdateOverlays();
		eOverlayMask	GetCurrentOverlay()			 { return m_eCurrOverlay; }

		void			BeginZoom();
		void			EndZoom();

//		void			BeginUnderwater();
//		void			EndUnderwater();

		void		HideLoadScreen();
		// Inform the interface mgr that loading failed
		void		LoadFailed() { m_bLoadFailed = LTTRUE; }
		void		SetLoadLevelString(HSTRING hWorld);
		void		SetLoadLevelPhoto(char *pszPhoto);


		void		ShowMessageBox(HSTRING hString, eMBType eType, MBCallBackFn pFn, void *pData = DNULL, LTBOOL bLargeFont = LTTRUE, 
									LTBOOL bDefaultReturn = LTTRUE, LTBOOL bSolidBack = LTFALSE) 
								{m_MessageBox.Show(hString, eType, pFn, pData, bLargeFont, bDefaultReturn, bSolidBack);}

		//new for AvP3
		void		ReloadHUD(DDWORD nIndex, LTBOOL bForce=LTFALSE);
		void		DrawHud(){m_HudMgr.UpdateHud();m_HudMgr.DrawHud();}
		HSURFACE	GetPauseBackground() { return m_hPauseBackground; }
		void		SetMenuOffset();
		void		TermWepChooser();
		void		InitWepChooser();
		void		InitTargetingMgr(int nType) { m_pAutoTargetMgr->Init((TargetingType)nType);};
		LTBOOL		IsAlienTargeting() { return m_pAutoTargetMgr? m_pAutoTargetMgr->GetTargetingType()==TT_ALIEN:LTFALSE; }

		void		PlayInterfaceSound(InterfaceSound eIS, char* pSnd = LTNULL);
		CFolderMgr*	GetFolderMgr()		{ return &m_FolderMgr; }

		// NOLF
		eFolderID	GetCurrentFolder();
		LTBOOL		SwitchToFolder(eFolderID folderID);
		LTBOOL		ForceFolderUpdate(eFolderID folderID);
		LTBOOL		DrawSFX();

		void		CreateInterfaceBackground();
		void		UpdateInterfaceBackground();
		void		RemoveInterfaceBackground();
		void		AddInterfaceSFX(CSpecialFX* pSFX, ISFXType eType);
		void		RemoveInterfaceSFX(CSpecialFX* pSFX);
		void		RemoveAllInterfaceSFX();
		void		UpdateModelAnimations(LTFLOAT fCurFrameDelta);
		void		UpdateModelAnimations();
		void		UpdateInterfaceSFX();

		void		NextDemoScreen();
		void		ShowDemoScreens(LTBOOL bQuit);
		void		UpdateCursor();
		LTBOOL		UseInterfaceCamera() {return m_bUseInterfaceCamera;}

		LTBOOL		LoadCustomLevel(const char * szWorldName);

		void		StartMission(int nPrevMissionId);
		uint32		GetPrevMissionIndex(){ return m_nPrevMissionId; }
		void		SetPredVisionTransition(LTBOOL bOn) { m_bPredVisionTransition = bOn; }

		CLTGUIFont* GetCharacterFont();
		CLTGUIFont*	GetObjectiveFont();
	
		CLoadingScreen* GetLoadingScreen() { return &m_LoadingScreen; }

		DWORD		SetDigit(DWORD dwVal, int nOffset, int nVal);
		DWORD		GetDigit(DWORD dwVal, int nOffset);
		DWORD		EncodeStatus(DWORD dwVal);
		DWORD		DecodeStatus(DWORD dwVal);

		LTBOOL		IsCinematicSkipKeyDown()	{ return m_bCinematicSkipKey; }
		LTBOOL		CinematicSkipDelay()		{ return (m_fCinematicSkipDelay > 0.0f); }

		void		SetDisconnectError(uint32 nError)	{ m_nDisconnectError = nError; }


	private :

		CInterfaceResMgr m_InterfaceResMgr;		// manages shared resources
		CLayoutMgr		m_LayoutMgr;			// bute mgr for layout info

		CLTWnd			m_MainWnd;				// Main window
		CLTDialogueWnd	m_DialogueWnd;			// Dialogue window

		CPlayerStats	m_stats;				// Player statistics (health, ammo, armor, etc.)
		CMessageMgr		m_messageMgr;			// Message display/sending mgr
		CGameSettings	m_Settings;
		CWeaponChooser*	m_pWeaponChooser;		// Next/previous weapon interface
		CCrosshairMgr*	m_pCrosshairMgr;		// Crosshair Manager
		CInterfaceTimer m_InterfaceTimer;		// Main interface timer
		CMessageBox		m_MessageBox;			// Used for simple dialog boxes

		//new for AvP3
		CHudMgr			m_HudMgr;				// HUD manager interface
		DIntPt			m_ptMenuOffset;			// Offset for menu display
		CAutoTargetMgr*	m_pAutoTargetMgr;		// Auto-Targeting Manager
		GameState		m_eGameState;			// Current game state

		DDWORD		m_dwAdvancedOptions;		// Advanced options
		DDWORD		m_dwOrignallyEnabled;		// Advanced options that were originally enabled

		DBYTE		m_nClearScreenCount;		// How many frames to clear the screen
		LTBOOL		m_bClearScreenAlways;		// Should we always clear the screen?

		LTBOOL		m_bDrawPlayerStats;			// Draw the player stats display?
		LTBOOL		m_bDrawCloseCaption;		// Draw the close caption display?
		LTBOOL		m_bDrawFragCount;			// Draw the frag count?
		LTBOOL		m_bDrawInterface;			// Draw the interface?

		LTBOOL		m_bGameMusicPaused;			// Is the game muisc paused?
		HSOUNDDE	m_hMenuMusic;				// Handle to music playing while menu is up
		HSOUNDDE	m_hSplashSound;				// Handle to sound played when splash screen is up
		LTFLOAT		m_fSplashEnd;				// when do we leave splash screen

		char		m_szMenuLoop[128];

		HSURFACE	m_hGamePausedSurface;		// "Game Paused" message
		HSURFACE	m_hPauseBackground;			// "Game Paused" background surface
		
		DRect		m_rcMenuRestoreCamera;		// Camera rect to restore after leaving menus
		LTBOOL		m_bMenuRectRestore;			// Was the camera rect full-screen before going to the menus?
	
		DFLOAT		m_fMenuSaveFOVx;			// Fov before entering menu		
		DFLOAT		m_fMenuSaveFOVy;			// Fov before entering menu

		LTBOOL		m_bSwitchingModes;			// Switching render modes?

		DIntPt		m_CursorPos;

		int			m_nFailStringId;			// id of the string to display on the mission failed screen
		HSURFACE	m_hFailBack;				// background of Mission Failure screen

#ifdef _DEMO
		HSURFACE	m_hScreenShow[5];			// Our Demo screen show
		void		NextSplashScreen();			// Change up the splash screen
#endif

		HSOUNDDE	m_hScubaSound;				// sound looping while scuba gear is on

		HSURFACE	m_hBackground;				// The surface for the background
		HSURFACE	m_hFadeSurface;				// Used to do screen fading
		LTBOOL		m_bFadeInitialized;			// Have we initialized everything
		LTBOOL		m_bScreenFade;				// Should we fade the screen
		DFLOAT		m_fTotalFadeTime;			// How long to do the fade
		DFLOAT		m_fCurFadeTime;				// Current fade time
		LTBOOL		m_bFadeIn;					// Should we fade in (or out)
		LTBOOL		m_bExitAfterFade;			// Exit the level after the fade
		HSURFACE	m_hLoading;					// Loading surface

		HLTCURSOR	m_hCursor;
		LTBOOL		m_bUseCursor;

		uint32		m_nPrevMissionId;

		LTBOOL		m_bDisplayObjectives;		// Should we draw the objectives?
		LTBOOL		m_bObjectiveNotification;	// Should we draw the notification?

		LITHFONTSAVEDATA	m_SaveData[MAX_OBJECTIVES];			// Saved data for the draw routine
		LITHFONTDRAWDATA	m_DrawData;							// Draw data

		LITHFONTDRAWDATA	m_OVDrawData;			// Draw data

		LTFLOAT		m_fAlertStartTime;			// When did the alert start.
		uint8		m_nObjectiveFlashCount;		// How many times have we flashed.
		LTBOOL		m_bDrawAlertSolid;			// Time to just draw it solid.

		HLTSOUND	m_hObjectiveSound;
		HLTSOUND	m_hObjectiveAlertSound;

		HSURFACE	m_hAlertImage;
		HSURFACE	m_hObjBackground;
		HSURFACE	m_hObjCheckBox[3];

		LTBOOL		m_bCinematicSkipKey;		// Are we pressing the cinematic skip key?
		LTFLOAT		m_fCinematicSkipDelay;		// The time we need to delay after a cinematic skip.

		uint32		m_nDisconnectError;			// The error type upon disconnect


		// NOLF stuff
		CFolderMgr				m_FolderMgr;
		CSubtitle				m_Subtitle;

		eOverlayMask			m_eCurrOverlay;	// Currently displayed overlay
		CBaseScaleFX			m_BackSprite;
		CMoArray<CSpecialFX *>	m_InterfaceSFX;

		LTBOOL		m_bUseInterfaceCamera;

		CLoadingScreen m_LoadingScreen;			// The loading screen object/thread
		LTBOOL		m_bLoadFailed;

		LTBOOL		m_bPredVisionTransition;	// Should we draw the predator vision transition effect

		LTBOOL	m_bOldFogEnable; // Saved in-game fog enable info...


		LTBOOL	PreChangeState(GameState eCurState, GameState eNewState);

		LTBOOL	PrePlayingState(GameState eCurState);
		void	UpdatePlayingState();
		LTBOOL	PostPlayingState(GameState eNewState);

		LTBOOL	PreDialogueState(GameState eCurState);
		void	UpdateDialogueState();
		LTBOOL	PostDialogueState(GameState eNewState);

		LTBOOL	PreLoadingLevelState(GameState eCurState);
		void	UpdateLoadingLevelState();
		LTBOOL	PostLoadingLevelState(GameState eNewState);

		LTBOOL	PrePauseState(GameState eCurState);
		void	UpdatePausedState();
		LTBOOL	PostPauseState(GameState eNewState);

		LTBOOL	PreSplashScreenState(GameState eCurState);
		void	UpdateSplashScreenState();
		LTBOOL	PostSplashScreenState(GameState eNewState);

		LTBOOL	PreFolderState(GameState eCurState);
		void	UpdateFolderState();
		LTBOOL	PostFolderState(GameState eNewState);

		LTBOOL	PreFailureState(GameState eCurState);
		void	UpdateFailureState();
		LTBOOL	PostFailureState(GameState eNewState);

        LTBOOL  PreMovieState(GameState eCurState);
		void	UpdateMovieState();
        LTBOOL  PostMovieState(GameState eNewState);

		void	ProcessAdvancedOptions();

		void	Draw_Style1_Background(LTIntPt ptPos, LTIntPt ptSize, int nSteps, HSURFACE hSurf);

		void	NextMovie();
		char*	GetCurrentMovie();

		HVIDEO	m_hMovie;
		int		m_nCurMovie;

		void		UpdateScreenFade();
		void		CreatePauseBackground();
		void		ResetMenuOffset(){m_ptMenuOffset.x=0;m_ptMenuOffset.y=0;}
		void		UpdateActivate(LTVector vPos, LTVector vDir);
		void		HandleExitLevel(HMESSAGEREAD hMessage);
		void		HandleLoadNewLevel();
		void		DrawObjectives();
		void		ClearObjectiveDisplay();
		void		CreateObjectiveDisplay();
		void		SetObjectiveDrawData();
		void		DrawObjectiveNotification();
		void		SetAlert();
		LTIntPt		GetAlertCoords();
		void		ClearObjectiveAlert();
		void		HandleCinematicBorders(HCAMERA hCam);
		void		HandleCinematicBorders();
		void		DrawPredTransition(HCAMERA hCam);
		void		DoDamageScreenFlash(DamageType dt);
		LTVector	GetDamageTintColor(DamageType dt);

		void		PlayInterfaceSound(char *pSoundFile);

		int			HandleMissionStatus();

		InterfaceSound m_eIS;
		char		m_szSliderLeftSound[64];
		char		m_szSliderRightSound[64];
		char		m_szUnselectableSound[64];
		char		m_szEscapeSound[64];

		LTBOOL		m_bPlayedOutro;
		CCredits	m_Credits;				// Display credits

		// Used for tracking when a new level is loaded..
		uint32	m_nLoadWorldCount;
		uint32	m_nOldLoadWorldCount;

		// Used for splash screen...
		LTFLOAT		m_fSplashStartTime;

		// used for weapon chooser
		uint32		m_nHudType;
};


inline void CInterfaceMgr::ResetMenuRestoreCamera(int nLeft, int nTop, int nRight, int nBottom)
{ 
	if (m_rcMenuRestoreCamera.right != 0 && m_rcMenuRestoreCamera.bottom != 0)
	{
		m_rcMenuRestoreCamera.left = nLeft;
		m_rcMenuRestoreCamera.top = nTop;
		m_rcMenuRestoreCamera.right = nRight;
		m_rcMenuRestoreCamera.bottom = nBottom;
	} 
}

// ----------------------------------------------------------------------- //
// Clears the screen a few times so the backbuffer(s) get cleared.
// ----------------------------------------------------------------------- //
inline void CInterfaceMgr::ClearAllScreenBuffers()
{
	for (int i=0; i < 4; i++)
	{
		g_pClientDE->ClearScreen(DNULL, CLEARSCREEN_SCREEN);
		g_pClientDE->FlipScreen(0);
	}
}


#endif // __INTERFACE_MGR_H__