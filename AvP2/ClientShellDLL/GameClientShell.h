// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.h
//
// PURPOSE : Game Client Shell - Definition
//
// CREATED : 9/18/97
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_CLIENT_SHELL_H__
#define __GAME_CLIENT_SHELL_H__

// ----------------------------------------------------------------------- //

#include "cpp_clientshell_de.h"
#include "ClientServerShared.h"
#include "SFXMgr.h"
#include "Music.h"
#include "WeaponModel.h"
#include "GlobalMgr.h"
#include "InterfaceMgr.h"
#include "FlashLight.h"
#include "SharedMovement.h"
#include "PlayerMovement.h"
#include "CameraMgrFX.h"
#include "GameType.h"
#include "HeadBobMgr.h"
#include "ViewModeMGR.h"
#include "FrameRateMgr.h"


// ----------------------------------------------------------------------- //

#define FOVX_NORMAL		75.0f
#define FOVY_NORMAL		60.0f

// ----------------------------------------------------------------------- //
// Error values

#define MP_HOST_ERROR_INVALID_OPTIONS		100
#define MP_HOST_ERROR_NO_MAPS				101
#define MP_HOST_ERROR_STARTGAME_FAILED		102
#define MP_HOST_ERROR_UNKNOWN				103

#define MP_JOIN_ERROR_INVALID_SERVER		200
#define MP_JOIN_ERROR_GAME_FULL				201
#define MP_JOIN_ERROR_STARTGAME_FAILED		202
#define MP_JOIN_ERROR_WRONG_VERSION			203
#define MP_JOIN_ERROR_UNKNOWN				204

// ----------------------------------------------------------------------- //

class CServerOptions;
class CGameTexMgr;
class StoryElement;
class MultiplayerClientMgr;
class CGameSpyServer;

typedef std::list<HSURFACE>	ImageList;

// ----------------------------------------------------------------------- //

class CGameClientShell : public CClientShellDE
{
	public:

		// --------------------------------------------------------------------------------- //
		// Construction and destruction

		CGameClientShell();
		~CGameClientShell();


		// --------------------------------------------------------------------------------- //
		// Console program handling functions

		void		HandleRecord(int argc, char **argv);
		void		HandlePlaydemo(int argc, char **argv);
		void		HandleExitLevel(int argc, char **argv);

		void		DemoSerialize(DStream *pStream, LTBOOL bLoad);


		// --------------------------------------------------------------------------------- //
		// Level and game setup functions

		LTBOOL		CanSaveGame()					const	{ return (m_bInWorld); }

		LTBOOL		LoadGame(char* pWorld, char* pObjectsFile);
		LTBOOL		SaveGame(char* pObjectsFile);

		LTBOOL		QuickSave();
		LTBOOL		QuickLoad();

		void		ExitLevel();

		LTBOOL		LoadWorld(char* pWorldFile, char* pCurWorldSaveFile = LTNULL,
							char* pRestoreWorldFile = LTNULL, uint8 nFlags = LOAD_NEW_GAME);

		LTBOOL		GetNiceWorldName(char* pWorldFile, char* pRetName, int nRetLen);
		char*		GetCurrentWorldName()					{ return m_strCurrentWorldName; }

		void		CSPrint(char* msg, ...);

		uint32			GetLastSaveLoadVersion() const { return m_nLastSaveLoadVersion; }
		uint32			GetCurrentSaveLoadVersion() const { return m_nCurrentSaveLoadVersion; }

		// --------------------------------------------------------------------------------- //
		// Game state information functions

		LTBOOL		IsFirstUpdate()					const	{ return m_bFirstUpdate; }

		void		PauseGame(LTBOOL bPause, LTBOOL bPauseSound = LTFALSE);
		LTBOOL		IsGamePaused()					const	{ return m_bGamePaused; }

		LTBOOL		PreChangeGameState(GameState eNewState);
		LTBOOL		PostChangeGameState(GameState eOldState);


		// --------------------------------------------------------------------------------- //
		// Misc interface related functions

		void		MinimizeMainWindow()					{ m_bMainWindowMinimized = LTTRUE; }
		LTBOOL		IsMainWindowMinimized()					{ return m_bMainWindowMinimized; }
		void		RestoreMainWindow()						{ m_bMainWindowMinimized = LTFALSE; }

		void		SetInputState(LTBOOL bAllowInput);

		int			GetScreenWidth();
		int			GetScreenHeight();

		LTBOOL		IsStoryObjectActive()					{ return m_pStoryElement?LTTRUE:LTFALSE; }


		// --------------------------------------------------------------------------------- //
		// Sound related functions

		void		InitSound();
		LTBOOL		SoundInited()			const	{ return ( m_resSoundInit == LT_OK ) ? LTTRUE : LTFALSE; }

		HSOUNDDE	PlaySoundLocal(char *szSound, LTBOOL bLoop = LTFALSE, LTBOOL bStream = LTFALSE, LTBOOL bGetHandle = LTFALSE);
		void		PlayTauntSound(uint32 nClientID);


		// --------------------------------------------------------------------------------- //
		// Multiplayer specific functions

		LTRESULT	HostMultiplayerGame(CServerOptions *pOptions);
		LTRESULT	JoinMultiplayerGame(CGameSpyServer *pServer, const char *szPassword = LTNULL);
		LTRESULT	JoinMultiplayerGameAtIP(const char *szIP, uint16 port, const char *szPassword = LTNULL);
		LTBOOL		LauncherServerApp( CServerOptions& serverOptions );

		LTBOOL		IsMultiplayerGame();

		MultiplayerClientMgr*	GetMultiplayerMgr()			{ return m_pMPMgr; }
		void*		GetAuthContext();

		// --------------------------------------------------------------------------------- //
		// Specific joystick input functions

		LTBOOL		IsJoystickEnabled();
		LTBOOL		EnableJoystick();


		// --------------------------------------------------------------------------------- //
		// Specific mouse input functions

		static void	OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
		static void	OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void	OnLButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void	OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
		static void	OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void	OnRButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
		static void	OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
		static void OnMouseWheel(HWND hwnd, int x, int y, UINT keyFlags);
		static void OnChar(HWND hWnd, char c, int rep);

		void		SetMouseSensitivityScale(LTFLOAT fSensScale = 1.0f);


		// --------------------------------------------------------------------------------- //
		// Access to the main player movement class for this client

		PlayerMovement*	GetPlayerMovement()		{ return &m_PlayerMovement; }


		// --------------------------------------------------------------------------------- //
		// Access to camera information

		CameraMgr*	GetCameraMgr()				{ return g_pCameraMgr; }

		HLOCALOBJ	GetInterfaceCamera() const	{ return m_hInterfaceCamera; }
		void		CreateInterfaceCamera();
		void		DeleteInterfaceCamera();

		HCAMERA		GetPlayerCamera()			{ return m_hPlayerCamera; }
		HCAMERA		Get3rdPersonCamera()		{ return m_h3rdPersonCamera; }
		HCAMERA		GetCinematicCamera()		{ return m_hCinematicCamera; }

		HOBJECT		GetPlayerCameraObject()		{ return g_pCameraMgr->GetCameraObject(m_hPlayerCamera); }
		HOBJECT		Get3rdPersonCameraObject()	{ return g_pCameraMgr->GetCameraObject(m_h3rdPersonCamera); }
		HOBJECT		GetCinematicCameraObject()	{ return g_pCameraMgr->GetCameraObject(m_hCinematicCamera); }

		LTBOOL		IsFirstPerson()				{ return g_pCameraMgr->GetCameraActive(m_hPlayerCamera); }
		LTBOOL		Is3rdPerson()				{ return g_pCameraMgr->GetCameraActive(m_h3rdPersonCamera); }
		LTBOOL		IsCinematic()				{ return g_pCameraMgr->GetCameraActive(m_hCinematicCamera); }

		void		SetExternalCamera(LTBOOL bExternal = LTTRUE);

		void		RenderCamera(LTBOOL bDrawInterface = LTTRUE);
		LTBOOL		HandleCameraEffect(ArgList *pArgList);


		// --------------------------------------------------------------------------------- //
		// Misc game object or message control functions

		void		ChangeWeapon(uint8 nWeaponId, LTBOOL bForce=LTFALSE, LTBOOL bPlaySelSound=LTTRUE);//, uint8 nAmmoId, uint32 dwAmmo);
		void		MPChangeWeapon(uint8 nCommandId);
		void		ReadMPInterfaceUpdate(HMESSAGEREAD hMessage);
		void		HandleWeaponKickBack();

		void		HandleWeaponPickup(uint8 nWeaponId);
		void		HandleAmmoPickup(uint8 nAmmoPoolId, int nAmmoCount);

		void		ShakeScreen(LTVector vAmount);
		void		FlashScreen(LTVector vTintColor, LTVector vPos, LTFLOAT fTime, LTFLOAT fRampUp,
								LTFLOAT fRampDown, LTFLOAT fScaleDist, LTBOOL bFirstPersonOnly = LTFALSE);

		void		EndStoryObject();
		void		ResetGlobalFog();

		void		DoActivate();

		void		SetFacehugged();
		void		UpdateFacehugged();
		void		ClearFacehugged();
		LTBOOL		IsFirstLand() {if(m_bFirstLand){m_bFirstLand=LTFALSE;return LTTRUE;}return LTFALSE;}
		CFlashLightPlayer* GetFlashlight() { return &m_FlashLight; }


		// --------------------------------------------------------------------------------- //
		// Misc data settings and retrieval

		LTFLOAT		GetFrameTime()				{ return m_fFrameTime; }
		LTVector	GetVoidColor()				{ return m_vVoidColor; }
		void		SetVoidColor(LTVector v)	{ m_vVoidColor = v; }
		FRState		GetFrameRateState()			{ return m_FrameRateMgr.GetState(); }

		Species		GetPlayerSpecies();
		PlayerState	GetPlayerState()	const	{ return m_ePlayerState; }
		uint32		GetPlayerFlags()	const	{ return m_dwPlayerFlags; }

		LTBOOL		IsPlayerDead()		const	{ return (m_ePlayerState == PS_DEAD || m_ePlayerState == PS_DYING); }
		LTBOOL		IsPlayerInWorld();
		LTBOOL		IsInWorld()			const	{ return m_bInWorld; }
		LTBOOL		CanSpringJump();

		LTBOOL		IsFacehugged()				{ return (m_hFacehugger != LTNULL); }
		void		HideShowAttachments(HOBJECT hObj);
		void		SetSFXandMusicVolume(LTFLOAT fSFX, LTFLOAT fMusic) { m_fSFXVolume=fSFX; m_fMusicVolume=fMusic; }


		// --------------------------------------------------------------------------------- //
		// Misc manager/API retrieval

		GameType*		GetGameType()			{ return &m_GameType; }
		CWeaponModel*	GetWeaponModel()		{ return &m_weaponModel; }
		CPlayerStats*	GetPlayerStats()		{ return m_InterfaceMgr.GetPlayerStats(); }
		CGameSettings*	GetGameSettings()		{ return m_InterfaceMgr.GetSettings(); }
		CInterfaceMgr*	GetInterfaceMgr()		{ return &m_InterfaceMgr; }
		ViewModeMGR*	GetVisionModeMgr()		{ return &m_ModeMGR; }
		CSFXMgr*		GetSFXMgr()				{ return &m_sfxMgr; }
		CMusic*			GetMusic()				{ return &m_Music; }


	protected:

		// --------------------------------------------------------------------------------- //
		// ***************** LITHTECH!! *******************
		//
		// These are functions called from Lithtech which we handle in the game client shell

		uint32		OnEngineInitialized(RMode *pMode, DGUID *pAppGuid);
		void		OnEngineTerm();
		void		OnEvent(uint32 dwEventID, uint32 dwParam);
		LTRESULT	OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos);
		LTRESULT	OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot);
		LTRESULT	OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag);
		void		PreLoadWorld(char *pWorldName);
		void		BeginPeerToPeerAuth();
		void		OnPeerToPeerAuthPacket(void* theData, unsigned long theLegnth);
		void		OnEnterWorld();
		void		OnExitWorld();
		void		PreUpdate();
		void		Update();
		void		PostUpdate();
		void		UpdatePlaying();
		void		OnCommandOn(int command);
		void		OnCommandOff(int command);
		void		OnKeyDown(int key, int rep);
		void		OnKeyUp(int key);
		void		SpecialEffectNotify(HLOCALOBJ hObj, HMESSAGEREAD hMessage);
		void		OnObjectRemove(HLOCALOBJ hObj);
		void		OnMessage(uint8 messageID, HMESSAGEREAD hMessage);
		void		OnModelKey(HLOCALOBJ hObj, ArgList *pArgs, int nTracker);

		void		SetDisconnectCode(uint32 nCode, const char *pMsg);

		// Called when a shared texture is loaded so special effect modes can be attached to it.
		virtual void OnTextureLoad(StateChange** stateChange, const char* commandLine);


	private:

		// --------------------------------------------------------------------------------- //
		// Private helper functions...

		void	FirstUpdate();

		void	MirrorSConVar(char *pSVarName, char *pCVarName);
		void	MirrorSConVarScale(char *pMinSVarName, char *pSVarName, char *pCVarName, LTFLOAT fScale);

		// Player updating functions

		void	UpdatePlayer();

		// Special effect and object update handling functions

		void	UpdateWeaponModel();
		void	UpdateSoundReverb();

		void 	UpdateEMPEffect();
		void 	UpdateStunEffect();
		void	UpdateFaceHugEffect();

		void	UpdateOrientationIndicator();

		LTBOOL	UpdateInfoString();

		void 	SendFlareTossMessage();

		// Game object message handling functions

		void	ChangeWeapon(HMESSAGEREAD hMessage);
		void	ForceChangeWeapon(HMESSAGEREAD hMessage);

		void	StartStoryObject(HMESSAGEREAD hMessage);
		void	StartObjectTracker(HMESSAGEREAD hMessage);

		// Misc message handling functions

		void	HandlePlayerStateChange(HMESSAGEREAD hMessage);
		void	HandlePlayerDamage(HMESSAGEREAD hMessage);
		void	HandleExitLevel(HMESSAGEREAD hMessage);
		void	HandleServerError(HMESSAGEREAD hMessage);

		void	HandleMediComp();
		void	HandleEnergySift();
		void	HandlePounceJump();
		void	HandleRespawn();

		// Level and game setup functions

		void	StartLevel();
		LTBOOL	DoLoadWorld(char* pWorldFile, char* pCurWorldSaveFile = LTNULL,
					        char* pRestoreWorldFile = LTNULL, uint8 nFlags = LOAD_NEW_GAME,
							char *pRecordFile = LTNULL, char *pPlaydemoFile = LTNULL);

		// Load Save functionality...

		void	AutoSave(HMESSAGEREAD hMessage);

		void	BuildClientSaveMsg(HMESSAGEWRITE hMessage);
		void	UnpackClientSaveMsg(HMESSAGEREAD hRead);

		// Speed Hack security monitor
		void	UpdateSpeedMonitor();


	private:

		// Misc managers and APIs

		PlayerMovement			m_PlayerMovement;			// Movement control of the player character

		CGlobalMgr				m_GlobalMgr;				// Contains global attribute mgrs
		CInterfaceMgr			m_InterfaceMgr;				// Interface manager
		CHeadBobMgr				m_HeadBobMgr;				// Handle head/weapon bob/cant
		CSFXMgr					m_sfxMgr;					// Special FX management...
		ViewModeMGR				m_ModeMGR;					// Texture render modes management
		CFrameRateMgr			m_FrameRateMgr;				// Frame Rate Manager


		// Variables used in controlling all aspects of camera objects

		HLOCALOBJ				m_hInterfaceCamera;			// The camera used in the interface
		CameraMgr				m_CameraMgr;				// Client camera manager
		HCAMERA					m_hPlayerCamera;			// The camera that gets rendered from the player
		CAMERAEFFECTINSTANCE	*m_hPlayerCameraHeight;		// The special effect that handles the camera height
		CAMERAEFFECTINSTANCE	*m_hPlayerCameraAiming;		// The special effect that handles the camera aiming

		HCAMERA					m_h3rdPersonCamera;			// The camera that gets rendered from the player
		CAMERAEFFECTINSTANCE	*m_h3rdPersonCameraHeight;	// The special effect that handles the camera height
		CAMERAEFFECTINSTANCE	*m_h3rdPersonCameraAiming;	// The special effect that handles the camera aiming

		HCAMERA					m_hCinematicCamera;			// The camera used to render cinematics and other external views

		LTVector				m_vVoidColor;				// The color to render the clipping void


		// Player information

		uint32					m_dwPlayerFlags;			// What is the player doing
		PlayerState				m_ePlayerState;				// What is the state of the player

		LTBOOL					m_bPlayerPosSet;			// Has the server sent us the player pos?

		LTBOOL					m_bEMPEffect;				// Are we under EMP effect?
		LTBOOL					m_bStunEffect;				// Are we under stun effect?
		LTBOOL					m_bFaceHugEffect;			// Are we udner Facehug effect?
		LTFLOAT					m_fLastPounceTime;			// The time we last pounced.
		LTFLOAT					m_fLastSpringJumpTime;		// The time we last spring jumped.

		LTFLOAT					m_fEarliestRespawnTime;
		LTFLOAT					m_fForceRespawnTime;


		// Client and game information

		LTBOOL					m_bFirstUpdate;				// Is this the first update
		LTBOOL					m_bInWorld;					// Are we in a world

		GameType				m_GameType;					// The current game type (ie. single player, deathmatch, etc)

		MultiplayerClientMgr	*m_pMPMgr;					// The multiplayer manager

		uint32					m_nLastSaveLoadVersion;  // The save/load version of the file being loaded (or last loaded). 
		static uint32			m_nCurrentSaveLoadVersion;  // The save/load version that will be stored with any current save file.

		// Game object data

		CFlashLightPlayer		m_FlashLight;				// Flash light
		CWeaponModel			m_weaponModel;				// Current weapon model
		StoryElement			*m_pStoryElement;			// The currently viewed StoryObject


	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...
	
		LTFLOAT					m_fFrameTime;				// Current frame delta

		LTRESULT				m_resSoundInit;				// Was sound initialized ok?

		LTBOOL					m_bStartedLevel;

		LTBOOL					m_bRestoringGame;			// Are we restoring a saved game

		LTBOOL					m_bMainWindowMinimized;		// Is the main window minimized?


		// Panning sky...

		LTBOOL					m_bPanSky;					// Should we pan the sky
		LTFLOAT					m_fPanSkyOffsetX;			// How much do we pan in X/frame
		LTFLOAT					m_fPanSkyOffsetZ;			// How much do we pan in Z/frame
		LTFLOAT					m_fPanSkyScaleX;			// How much do we scale the texture in X
		LTFLOAT					m_fPanSkyScaleZ;			// How much do we scale the texutre in Z
		LTFLOAT					m_fCurSkyXOffset;			// What is the current x offset
		LTFLOAT					m_fCurSkyZOffset;			// What is the current z offset


		CMusic					m_Music;					// Music helper variable
		char					m_strCurrentWorldName[256];	// Current world that's running
		LTBOOL					m_bGamePaused;				// Is the game paused?
		LTBOOL					m_bMainWindowFocus;			// Focus


		LTBOOL					m_bQuickSave;
		ImageList				m_StoryImageList;
		LTBOOL					m_bFirstLand;

		LTFLOAT					m_fMusicVolume;
		LTFLOAT					m_fSFXVolume;


		// Reverb parameters...

		LTBOOL					m_bUseReverb;
		float					m_fReverbLevel;
		float					m_fNextSoundReverbTime;
		LTVector				m_vLastReverbPos;


		// Facehug specific variables

		HLOCALOBJ				m_hFacehugger;
		LTFLOAT					m_fFacehugTime;

		// Speed Hack security monitor variables
		LTFLOAT					m_fInitialServerTime;
		LTFLOAT					m_fInitialLocalTime;
		uint8					m_nSpeedCheatCounter;
};

// ----------------------------------------------------------------------- //

inline int CGameClientShell::GetScreenWidth()
{
	uint32 dwWidth = 640, dwHeight = 480;
	g_pClientDE->GetSurfaceDims(g_pClientDE->GetScreenSurface(), &dwWidth, &dwHeight);

	return (int)dwWidth;
}

// ----------------------------------------------------------------------- //

inline int CGameClientShell::GetScreenHeight()
{
	uint32 dwWidth = 640, dwHeight = 480;
	g_pClientDE->GetSurfaceDims(g_pClientDE->GetScreenSurface(), &dwWidth, &dwHeight);

	return (int)dwHeight;
}

// ----------------------------------------------------------------------- //

inline HSOUNDDE CGameClientShell::PlaySoundLocal(char *szSound, LTBOOL bLoop, LTBOOL bStream, LTBOOL bGetHandle)
{
	uint32 dwFlags = bLoop ? PLAYSOUND_LOOP : 0;

	return g_pClientSoundMgr->PlaySoundLocal(szSound, SOUNDPRIORITY_MISC_HIGH, dwFlags);
}

// ----------------------------------------------------------------------- //

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //

#endif  // __GAME_CLIENT_SHELL_H__

