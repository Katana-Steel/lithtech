// ------------------------------------------------------------------------- //
//
// FILE      : D S Y S _ I N T E R F A C E . H
//
// CREATED   : 11/12/99
//
// AUTHOR    :
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : Orig LithTech
//
// ------------------------------------------------------------------------- //

// This header defines the main system-dependent engine functions.

#ifndef __DSYS_INTERFACE_H__
#define __DSYS_INTERFACE_H__


// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "ltbasedefs.h"

#include "ltbasetypes.h"

#ifndef __VERSION_INFO_H__
#include "version_info.h"
#endif

#include <cstdarg>

#include <SDL.h>

// ------------------------------------------------------------------------- //
// Externs (Public C data)
// ------------------------------------------------------------------------- //
typedef struct {
    float x;
    float y;
} point;

// ------------------------------------------------------------------------- //
// Typedefs
// ------------------------------------------------------------------------- //
#define MAX_KEYBUFFER           100
#define SOUND_DRIVER_NAME_LEN    32
#define SOUND_DRIVER_NAME_ARG   "+soundso"
#define MAX_PATH               1024

// ------------------------------------------------------------------------- //
// Prototypes (Public C methods)
// ------------------------------------------------------------------------- //

class CClientMgr;
class CClassMgr;

// These are called in the startup code (in client.cpp and server_interface.cpp).
// They initialize the system-dependent modules.
// 0 = success
// 1 = couldn't load resource module (de_msg.dll).
int dsi_Init();
void dsi_Term();

// Called (in debug version) when any function uses RETURN_ERROR.
// (Good place to keep a breakpoint!)
void dsi_OnReturnError(int err);

// ClientDE implementations.
RMode* dsi_GetRenderModes();
void dsi_RelinquishRenderModes(RMode *pMode);
LTRESULT dsi_GetRenderMode(RMode *pMode);
LTRESULT dsi_SetRenderMode(RMode *pMode, const char* pName);
LTRESULT dsi_ShutdownRender(uint32 flags);

// Initializes the cshell and cres DLLs (copies them into a temp directory).
LTRESULT dsi_InitClientShellDE();
LTRESULT dsi_LoadServerObjects(CClassMgr *pInfo);

// Called when we run out of memory.  Shuts down everything
// and comes up with an error message.
void dsi_OnMemoryFailure();

void dsi_Sleep(uint32 ms);
void dsi_ServerSleep(uint32 ms);

// Client-only things.
void dsi_ClientSleep(uint32 ms);

LTBOOL dsi_IsInputEnabled();

uint16 dsi_NumKeyDowns();
uint16 dsi_NumKeyUps();
uint32 dsi_GetKeyDown(uint32 i);
uint32 dsi_GetKeyDownRep(uint32 i);
uint32 dsi_GetKeyUp(uint32 i);
void dsi_ClearKeyDowns();
void dsi_ClearKeyUps();
void dsi_ClearKeyMessages();

LTBOOL dsi_IsConsoleUp();
void dsi_SetConsoleUp(LTBOOL bUp);
LTBOOL dsi_IsClientActive();
void dsi_OnClientShutdown(const char *pMsg );

char* dsi_GetDefaultWorld();


// Sets up a message for a DRESULT.
LTRESULT dsi_SetupMessage(char *pMsg, int maxMsgLen, uint32 dResult, va_list marker);

// Puts an error message in the console if the renderer is initialized or
// a message box otherwise.
LTRESULT dsi_DoErrorMessage(char *pMessage);


bool dsi_IsConsoleEnabled();
void dsi_PrintToConsole(const char *pMsg, ...);	// Print to console.

void* dsi_GetInstanceHandle();	// Returns an HINSTANCE.
void* dsi_GetResourceModule();	// Returns an HINSTANCE.
void* dsi_GetMainWindow();		// Returns an HWND.

// Message box.
void dsi_MessageBox(char *pMsg, char *pTitle);

// System functions
LTRESULT GetOrCopyClientFile(char *pFilename, char *pOutName, int outNameLen, bool &bFileCopied);

// Get the version info of the executable.
// Returns LT_OK or an error.
LTRESULT dsi_GetVersionInfo(LTVersionInfo &info);

// ------------------------------------------------------------------------- //
// Class Definitions
// ------------------------------------------------------------------------- //
class ClientGlob {
    public:
        ClientGlob () {
            m_bIsConsoleUp		= false;
            m_bConsoleEnabled	= true;
            m_bInputEnabled		= true;

            m_pGameResources	= nullptr;
            m_pWorldName		= nullptr; }

    bool            m_bProcessWindowMessages;
/*
    jmp_buf         m_MemoryJmp;
    HWND            m_hMainWnd;

    HINSTANCE       m_hInstance;
*/
    SDL_Window      *m_window;

    char            *m_WndClassName;
    const char      *m_WndCaption;

    bool            m_bInitializingRenderer;
    bool            m_bBreakOnError; // Break in dsi_OnReturnError?
    bool            m_bClientActive;
    bool            m_bLostFocus;
    bool            m_bAppClosing;
    bool            m_bDialogUp;
    bool            m_bRendererShutdown;    // They called ShutdownRender so we shouldn't
                                            // reinitialize the renderer.

    bool            m_bHost;
    char            *m_pGameResources;

    const char      *m_pWorldName;
    char            m_CachePath[500];

    uint32          m_KeyDowns[MAX_KEYBUFFER];
    uint32          m_KeyUps[MAX_KEYBUFFER];
    SDL_Keycode		m_SDLDowns[SDL_NUM_SCANCODES];
    int				m_mousedown[2];
    int				m_mouserel[2];
    int				m_mousewheel;
    bool            m_KeyDownReps[MAX_KEYBUFFER];


    uint16          m_nKeyDowns;
    uint16          m_nKeyUps;

    bool            m_bIsConsoleUp;

    bool            m_bInputEnabled;

    char            m_ExitMessage[500];

    char            m_acSoundDriverName[ SOUND_DRIVER_NAME_LEN ];

    bool			m_bConsoleEnabled;
};

extern ClientGlob g_ClientGlob;
#endif  // __DSYS_INTERFACE_H__
