
// This module implements all the dsi_interface functions.

#include <cstdarg>
#include <iostream>

#include "SDL_timer.h"
#include "bdefs.h"
#include "render.h"
#include "stdlterror.h"
#include "stringmgr.h"
#include "sysfile.h"
#include "de_objects.h"
#include "servermgr.h"
#include "classbind.h"
#include "bindmgr.h"
#include "console.h"
#include "linuxdsys.h"
#include "clientmgr.h"

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

//IInstanceHandleClient
static IInstanceHandleClient *instance_handle_client;
define_holder(IInstanceHandleClient, instance_handle_client);


void dsi_OnReturnError(int err)
{
}

static LTBOOL dsi_LoadResourceModule()
{
return LTTRUE;      // DAN - temporary
}

static void dsi_UnloadResourceModule()
{
}


LTRESULT dsi_SetupMessage(char *pMsg, int maxMsgLen, LTRESULT dResult, va_list marker)
{
    char msg[1000];
    vsnprintf(msg, 999, pMsg, marker);
    if (strlen(msg) < 1) {
        vsnprintf(msg, 999, "%s", marker);
    }
    auto &&o = (dResult == LT_OK) ? std::cout : std::cerr;;
    switch(dResult){
    case LT_OK:
        o << "Info  : ";
        break;
    case LT_ERRORLOADINGRENDERDLL:
        o << "Render: ";
        break;
    case LT_ERRORCOPYINGFILE:
        o << "Library not found: ";
        break;
    case LT_NOGAMERESOURCES:
        o << "NoGameResources found ";
        break;
    case LT_CANTLOADGAMERESOURCES:
        o << "GameResourceMissing: ";
        break;
    default:
        o << "Error : ";
        break;
    }
    o << msg << '\n';
    return LT_OK;      // DAN - temporary
}


int dsi_Init()
{
	dm_Init();	// Memory manager.
	str_Init();	// String manager.
	df_Init();	// File manager.
//	obj_Init();	// Object manager.
//	packet_Init();
return 0;      // DAN - temporary
}

void dsi_Term()
{
//	packet_Term();
//	obj_Term();
	df_Term();
	str_Term();
	dm_Term();
	return;
}

void* dsi_GetResourceModule()
{
return NULL;      // DAN - temporary
}

LTRESULT _GetOrCopyFile(char *pTempPath, char *pFilename, char *pOutName, int outNameLen)
{
    return LTTRUE;      // DAN - temporary
}


LTRESULT dsi_LoadServerObjects(CClassMgr *pInfo)
{
	const char* pGameServerObjectName = "./libObject.lto";

    //load the GameServer shared object
    int version;
    int status = cb_LoadModule(pGameServerObjectName, false, pInfo->m_ClassModule, &version);

    //check for errors.
    if (status == CB_CANTFINDMODULE)
	{
        return LT_INVALIDOBJECTDLL;
    }
    else if (status == CB_NOTCLASSMODULE)
	{
        return LT_INVALIDOBJECTDLL;
    }
    else if (status == CB_VERSIONMISMATCH)
	{
		return LT_INVALIDOBJECTDLLVERSION;
	}

/*
	    // Get sres.dll.
	bFileCopied = false;
    if ((GetOrCopyFile("sres.dll", fileName, sizeof(fileName),bFileCopied) != LT_OK)
        || (bm_BindModule(fileName, bFileCopied, pClassMgr->m_hServerResourceModule) != BIND_NOERROR))
    {
		cb_UnloadModule( pClassMgr->m_ClassModule );

        sm_SetupError(LT_ERRORCOPYINGFILE, "sres.dll");
        RETURN_ERROR_PARAM(1, LoadServerObjects, LT_ERRORCOPYINGFILE, "sres.dll");
    }

    //let the dll know it's instance handle.
    if (instance_handle_server != NULL)
	{
        instance_handle_server->SetInstanceHandle( pClassMgr->m_ClassModule.m_hModule );
    }
*/

	//cb_LoadModule(fileName, false, pInfo->m_ClassModule, &version);

	/*
	pInfo->m_hShellModule = (ShellModule*)malloc(sizeof(ShellModule));
	pInfo->m_hShellModule->m_hModule = NULL;
	pInfo->m_CreateShellFn =
		pInfo->m_hShellModule->m_CreateFn = (CreateShellFn) CreateServerShell;
	pInfo->m_DeleteShellFn =
		pInfo->m_hShellModule->m_DeleteFn = (DeleteShellFn) DeleteServerShell;
	*/

	return LT_OK;
}

void dsi_Sleep(uint32 ms)
{
// Several possible implementations:
// poll (requires sys/poll.h)
//	poll(NULL, 0, ms);
// select (requires sys/time.h, sys/types.h, unistd.h)
//	timeval timeout;
//	timeout.tv_sec = 0;
//	timeout.tv_usec = ms*1000;
//	select(0, NULL, NULL, NULL, &timeout);
// SIGALRM (requires sys/time.h, unistd.h)
//	itimerval timerconfig;
//	memset(&timerconfig, 0, sizeof(timerconfig));
//	timerconfig.it_value.tv_usec = ms*1000;
//	setitimer(ITIMER_REAL, &timerconfig, NULL);
//	pause();

// the simplest implementation using sdl2:
    SDL_Delay(ms);
}

void dsi_ServerSleep(uint32 ms)
{ dsi_Sleep(ms); }

extern int32 g_ScreenWidth, g_ScreenHeight;	// Console variables.

static void dsi_GetDLLModes(const char *pDLLName, RMode **pMyList)
{
	RMode *pMyMode;
	RMode *pListHead, *pCur;

	pListHead = rdll_GetSupportedModes();

	// Copy the mode list.
	pCur = pListHead;
	while (pCur)
	{
		LT_MEM_TRACK_ALLOC(pMyMode = (RMode*)dalloc(sizeof(RMode)),LT_MEM_TYPE_MISC);
		memcpy(pMyMode, pCur, sizeof(RMode));

		pMyMode->m_pNext = *pMyList;
		*pMyList = pMyMode;

		pCur = pCur->m_pNext;
	}

	rdll_FreeModeList(pListHead);
}


RMode* dsi_GetRenderModes()
{
	RMode *pList = LTNULL;
	dsi_GetDLLModes("integrated", &pList);
	return pList;
}

void dsi_RelinquishRenderModes(RMode *pMode)
{
    RMode *pCur = pMode;
    RMode *pNext = nullptr;
    while (pCur)
    {
        pNext = pCur->m_pNext;
        dfree(pCur);
        pCur = pNext;
    }
}

LTRESULT dsi_GetRenderMode(RMode *pMode)
{
    pMode->m_BitDepth = 32;
    pMode->m_Width = g_ScreenWidth;
    pMode->m_Height = g_ScreenHeight;
    pMode->m_Description[0] = 0;
    pMode->m_InternalName[0] = 0;
    pMode->m_bHWTnL = true;
    pMode->m_pNext = nullptr;
    return LT_OK;
}

LTRESULT dsi_SetRenderMode(RMode *pMode, const char *pName)
{
    RMode currentMode;
    char message[256];

    if (r_TermRender(1, false) != LT_OK) {
        dsi_SetupMessage(message, sizeof(message)-1, LT_UNABLETORESTOREVIDEO, LTNULL);
        dsi_OnClientShutdown(message);
        RETURN_ERROR(0, SetRenderMode, LT_UNABLETORESTOREVIDEO);
    }

    memcpy(&currentMode, &g_RMode, sizeof(RMode));

    // Try to set the new mode.
    if (r_InitRender(pMode, pName) != LT_OK) {
        // Ok, try to restore the old mode.
        if (r_InitRender(&currentMode, pName) != LT_OK) {
            //dsi_SetupMessage(message, sizeof(message)-1, LT_UNABLETORESTOREVIDEO, LTNULL);
            //dsi_OnClientShutdown(message);
            RETURN_ERROR(0, SetRenderMode, LT_UNABLETORESTOREVIDEO);
        }

        RETURN_ERROR(1, SetRenderMode, LT_KEPTSAMEMODE);
    }

    g_ClientGlob.m_bRendererShutdown = false;
    return LT_OK;
}

LTRESULT dsi_ShutdownRender(uint32 flags)
{
return LT_OK;      // DAN - temporary
}

LTRESULT _GetOrCopyClientFile(char *pTempPath, const char *pFilename, char *pOutName, int outNameLen)
{
    std::string fname{"./"};
    fname += pFilename;
    if(fname.length() < outNameLen)
        memcpy(pOutName, fname.c_str(), fname.length());
    return LTTRUE;      // DAN - temporary
}

LTRESULT GetOrCopyClientFile(const char *pFilename, char *pOutName, int outNameLen, bool &bFileCopied)
{
    bFileCopied = false;
    char pTempPath[_MAX_PATH*2] = {'/','t', 'm', 'p', '/', 0};
    return _GetOrCopyClientFile(pTempPath, pFilename, pOutName, outNameLen);
}

LTRESULT dsi_InitClientShellDE()
{
	g_pClientMgr->m_hShellModule = nullptr;
	g_pClientMgr->m_hClientResourceModule = nullptr;
    char filename[MAX_PATH];
    memset(filename, 0, MAX_PATH);

    const char* clientlib = "libCShell.so";
    const char* reslib = "libCRes.so";
    bool copied;
    LTRESULT dResult = GetOrCopyClientFile(clientlib, filename, MAX_PATH, copied);
    if (dResult != LTTRUE) {
        g_pClientMgr->SetupError(LT_ERRORCOPYINGFILE, clientlib);
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_ERRORCOPYINGFILE, clientlib);
    }

    auto status = bm_BindModule(filename, copied, g_pClientMgr->m_hShellModule);
    if (status == BIND_CANTFINDMODULE) {
        g_pClientMgr->SetupError(LT_MISSINGSHELLDLL, clientlib);
        RETURN_ERROR(1, InitClientShellDE, LT_MISSINGSHELLDLL);
    }

	// have the user's cshell and the clientMgr exchange info
    if (i_client_shell == nullptr )
    {
        g_pClientMgr->SetupError(LT_INVALIDSHELLDLL, clientlib);
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_INVALIDSHELLDLL, clientlib);
	}

    copied = false;
    memset(filename, 0, MAX_PATH);
    dResult = GetOrCopyClientFile( reslib, filename, MAX_PATH, copied );
    if (dResult != LTTRUE) 
	{
        g_pClientMgr->SetupError(LT_ERRORCOPYINGFILE, reslib);
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_ERRORCOPYINGFILE, reslib);
    }

    //load the DLL.
    status = bm_BindModule(filename, copied, g_pClientMgr->m_hClientResourceModule);

    //check if it was loaded.
    if (status == BIND_CANTFINDMODULE) {
        //unload the loaded cshell 
        bm_UnbindModule(g_pClientMgr->m_hShellModule);
        g_pClientMgr->m_hShellModule = nullptr;

        g_pClientMgr->SetupError(LT_INVALIDSHELLDLL, reslib);
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_INVALIDSHELLDLL, reslib);
    }

    if (instance_handle_client != nullptr) {
        instance_handle_client->SetInstanceHandle(g_pClientMgr->m_hShellModule);
    }

	return LT_OK;
}

void dsi_OnMemoryFailure()
{
}

// Client-only functions.
void dsi_ClientSleep(uint32 ms)
{
	dsi_ServerSleep(ms);
}

LTBOOL dsi_IsInputEnabled()
{
return LTTRUE;      // DAN - temporary
}

uint16 dsi_NumKeyDowns()
{
return 0;      // DAN - temporary
}

uint16 dsi_NumKeyUps()
{
return 0;     // DAN - temporary
}

uint32 dsi_GetKeyDown(uint32 i)
{
return 0;     // DAN - temporary
}

uint32 dsi_GetKeyDownRep(uint32 i)
{
return 0;     // DAN - temporary
}

uint32 dsi_GetKeyUp(uint32 i)
{
return 0;     // DAN - temporary
}

int dsi_GetSDLDown(uint32 i)
{
	return g_ClientGlob.m_SDLDowns[i];
}

int dsi_GetSDLUp(uint32 i)
{
	return g_ClientGlob.m_SDLUps[i];
}

void dsi_ClearKeyDowns()
{
	size_t i;
	g_ClientGlob.m_nKeyDowns=0;
	for (i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		g_ClientGlob.m_SDLDowns[i] = 0;
	}
}

void dsi_ClearKeyUps()
{
	size_t i;
	g_ClientGlob.m_nKeyUps=0;
	for (i = 0; i < SDL_NUM_SCANCODES; i++)
	{
		g_ClientGlob.m_SDLUps[i] = 0;
	}
}

void dsi_ClearKeyMessages()
{
}

LTBOOL dsi_IsConsoleUp()
{
    return LTFALSE;
}

void dsi_SetConsoleUp(LTBOOL bUp)
{
}

LTBOOL dsi_IsClientActive()
{
	return 1;
}

void dsi_OnClientShutdown(const char *pMsg )
{
    if(pMsg == nullptr || pMsg[0] == '\0')
        return;
    ClientGlob *pGlob = &g_ClientGlob;
    strncpy(pGlob->m_ExitMessage, pMsg, sizeof(pGlob->m_ExitMessage));
    pGlob->m_ExitMessage[sizeof(pGlob->m_ExitMessage)-1] = '\0';
}

char* dsi_GetDefaultWorld()
{
return NULL;     // DAN - temporary
}


#include "server_interface.h"

extern CServerMgr *g_pServerMgr;

bool dsi_IsConsoleEnabled()
{
    return g_ClientGlob.m_bConsoleEnabled; // RKJ STUB temporary
}

void dsi_PrintToConsole(const char *pMsg, ...) {
    va_list marker;
    char msg[1000];

    if (g_pServerMgr && g_pServerMgr->m_pServerAppHandler) {
        va_start(marker, pMsg);
        vsnprintf(msg, 999, pMsg, marker);
        va_end(marker);

        g_pServerMgr->m_pServerAppHandler->ConsoleOutputFn(msg);
    }
}

void* dsi_GetInstanceHandle()
{
return NULL;     // DAN - temporary
}

void* dsi_GetMainWindow()
{
    return (void*)g_ClientGlob.m_window;
}

LTRESULT dsi_DoErrorMessage(char *pMessage)
{
return LT_OK;      // DAN - temporary
}

void dsi_MessageBox(char *pMessage, char *pTitle)
{
}

LTRESULT dsi_GetVersionInfo(LTVersionInfo &info)
{
	// the current linux version
	info.m_MajorVersion = 0;
	info.m_MinorVersion = 1;
	return LT_OK;
}
