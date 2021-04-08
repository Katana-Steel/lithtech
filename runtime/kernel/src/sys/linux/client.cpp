#include "bdefs.h"

#include "timemgr.h"
#include "clientmgr.h"
#include "sysdebugging.h"
#include "render.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <vector>
#include <SDL_syswm.h>

//holder for command line argument mgr interface.
#include "icommandlineargs.h"
static ICommandLineArgs *command_line_args;
define_holder(ICommandLineArgs, command_line_args);

extern char g_SSFile[];
extern LTBOOL g_bNullRender;
extern int32 g_CV_CursorCenter;
extern int32 g_nConsoleLines;
extern LTBOOL g_bConsoleEnable;
extern int32 g_bShowRunningTime;
extern LTBOOL g_CV_HighPriority;
extern int32 g_CV_PlayDemoReps;
extern int32 g_CV_NoDefaultEngineRez;

int32 g_CurRunIteration=0;

ClientGlob g_ClientGlob;
uint32 g_EngineStartMS;

static bool StartClient(ClientGlob *pGlob)
{
    pGlob->m_bHost = command_line_args->FindArgDash("host") != NULL;
    pGlob->m_pWorldName = command_line_args->FindArgDash("world");

#ifdef USE_ABSTRACT_SOUND_INTERFACES

    //  =======================================================================

    const char* pcSoundDriverName = command_line_args->FindArg(SOUND_DRIVER_NAME_ARG);
    if (pcSoundDriverName != LTNULL) {
        for (int i = 0; i < SOUND_DRIVER_NAME_LEN && pcSoundDriverName[ i ] != 0; i++) {
            pGlob->m_acSoundDriverName[ i ] = pcSoundDriverName[ i ];
        }
    }
    else {
        pGlob->m_acSoundDriverName[0] = 0;
    }

    //  =======================================================================

#endif  // USE_ABSTRACT_SOUND_INTERFACES

    std::vector<const char*> resTree{};
    resTree.reserve(MAX_RESTREES);
    // Add the default engine resource...
    if (!g_CV_NoDefaultEngineRez)
    {
        resTree.push_back("engine.rez");
    }

    resTree.push_back("game.rez");
    resTree.push_back("game2.rez");
    resTree.push_back("sound.rez");

    if (command_line_args->FindArgDash("noinput"))
    {
        pGlob->m_bInputEnabled = false;
    }

	//the configuration files that we need to load (they are loaded in order, so the later in
	//the lists override the earlier in the lists)
    static const uint32 knMaxConfigFiles = 16;
    std::vector<const char*> pszConfigFiles{};
    pszConfigFiles.reserve(knMaxConfigFiles);

	//see what the name of the autoexec configuration file is
    const char* pszAutoExecFileName = command_line_args->FindArgDash("config");
    pszConfigFiles.push_back( (pszAutoExecFileName) ? pszAutoExecFileName : "autoexec.cfg");

	//see what the name of the display configuration file is
    const char* pszDisplayFileName = command_line_args->FindArgDash("display");
    pszConfigFiles.push_back( (pszDisplayFileName) ? pszDisplayFileName : "display.cfg");

    uint32 initStartTime = timeGetTime();
    if (g_pClientMgr->Init(resTree.data(), resTree.size(), pszConfigFiles.size(), pszConfigFiles.data()) != LT_OK) {
        return false;
    }

    char strVersion[50];
    memset(strVersion,0,sizeof(strVersion));

    g_pClientMgr->m_VersionInfo.GetString(strVersion, sizeof(strVersion));
    DebugOut("LithTech build %s initialized in %.2f seconds.\n",
        strVersion, (float)(timeGetTime() - initStartTime) / 1000.0f);


    return true;
}

int RunClientApp()
{
    //should set AllocHook here, maybe?

    ClientGlob *pGlob = &g_ClientGlob;
    memset(pGlob, 0, sizeof(*pGlob));
	pGlob->m_bConsoleEnabled = true;
    pGlob->m_bInputEnabled = true;

    if (command_line_args->FindArgDash("DebugStructBanks")) {
        g_bDebugStructBanks = LTTRUE;
    }

    // todo check/set outOfMemory breakpoint

    const char *pArg = command_line_args->FindArgDash("workingdir");
    // if chdir fails it returns non-zero making the expression true
    if (pArg && chdir(pArg)) {
        return -1;
    }

    int status = dsi_Init();
    if(status != 0) {
        if(status == 1) {
            std::cout << "ERROR: unable to load ltmsg\n";
        } else {
            std::cout << "ERROR: unable to initialize LithTech Engine\n";
        }
        dsi_Term();
        return -1;
    }

    // initialize ClientMgr
    cm_Init();
    pGlob->m_bClientActive = true;

    pGlob->m_WndCaption = command_line_args->FindArgDash("windowtitle");
    if(pGlob->m_WndCaption == nullptr){
        pGlob->m_WndCaption = "LithTech";
    }

    pGlob->m_bBreakOnError = command_line_args->FindArgDash("breakonerror") != NULL;

    SDL_Rect screenRect;
    SDL_GetDisplayBounds(0, &screenRect);

	pGlob->m_window = SDL_CreateWindow(pGlob->m_WndCaption, SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
    if(!pGlob->m_window){
        std::cout << "Can't setup minimal SDL2 window\n" << SDL_GetError() << '\n';

        g_pClientMgr->Term();
        delete g_pClientMgr;
        g_pClientMgr = nullptr;

        dsi_Term();
        return -1;
    }
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(pGlob->m_window, &info);

	bool bOutOfMemory = false;
    bool bPrevHighPriority = false;
    SDL_Rect wndRect;
    if(StartClient(pGlob)) {
        pGlob->m_bProcessWindowMessages = true;
        // MainLoop
        LTRESULT dResult = LT_OK;
        for(;;)
        {
            dResult = g_pClientMgr->Update();
            if(dResult != LT_OK)
                break;

            // Give our process high priority?
            if (g_CV_HighPriority != bPrevHighPriority) {
                if (g_CV_HighPriority) {
                    dsi_PrintToConsole("Setting process to high priority");
                    setpriority(PRIO_PROCESS, 0, -10);
                }
                else {
                    dsi_PrintToConsole("Setting process to normal priority");
                    setpriority(PRIO_PROCESS, 0, 0);
                }
            }

            if (g_bShowRunningTime) {
                dsi_PrintToConsole("Running for %.1f seconds", (float)(SDL_GetTicks() - g_EngineStartMS) / 1000.0f);
            }

            if (g_CV_CursorCenter && !g_ClientGlob.m_bLostFocus) {
                SDL_GetWindowSize(pGlob->m_window, &wndRect.w,&wndRect.h);
                SDL_GetWindowPosition(pGlob->m_window, &wndRect.x,&wndRect.y);
#if 0
                SDL_WarpMouseGlobal((screenRect.x - screenRect.w) / 2, (screenRect.h - screenRect.y) / 2);
#else
                SDL_WarpMouseInWindow(pGlob->m_window, wndRect.w/2, wndRect.h/2);
#endif
                SDL_SetRelativeMouseMode(SDL_TRUE);
            }
			else
			{
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}

        }
    }

    if (pGlob->m_ExitMessage[0]) {
        r_TermRender(2, true);
        std::cout << pGlob->m_ExitMessage << '\n';
    }
    pGlob->m_bProcessWindowMessages = LTFALSE;

    g_pClientMgr->Term();
    delete g_pClientMgr;
    g_pClientMgr = nullptr;

    if(pGlob->m_window) {
        SDL_DestroyWindow(pGlob->m_window);
    }

    if (bOutOfMemory) {
        std::cout << "Error: Out Of Memory\n";
    }

    return 0;
}

bool SetupArgs(int argc, char *argv[])
{
    command_line_args->Init(argc, argv);

    return true;
}

int main(int p1, char *p2[])
{
	SDL_Init(SDL_INIT_EVERYTHING);
    LTMemInit();

    g_EngineStartMS = timeGetTime();
    if(! SetupArgs(p1, p2) )
    {
        return -1;
    }
    int ret = RunClientApp();
    if (g_CV_PlayDemoReps > 1)
    {
        g_CurRunIteration++;
        while(g_CV_PlayDemoReps > g_CurRunIteration)
        {
            g_CurRunIteration++;
            ret = RunClientApp();
        }

    }

    SDL_Quit();
    return ret;
}
