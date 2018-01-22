#include "bdefs.h"
#include "ogl_render.h"
#include "iclientshell.h"
#include "iltclient.h"
#include <GL/gl.h>

OGlRenderStruct::OGlRenderStruct()
{
}

bool OGlRenderStruct::LoadWorldData(ILTStream *pStream)
{
    return false; // STUB we currently want to fail loading data;
}

HRENDERCONTEXT OGlRenderStruct::CreateContext()
{
    SDL_Window *_win;
    g_pLTClient->GetEngineHook("SDL_Window", (void**)&_win);
    SDL_GLContext cx = SDL_GL_CreateContext(_win);
    return (HRENDERCONTEXT)cx;
}

void OGlRenderStruct::DeleteContext(HRENDERCONTEXT hContext){
    SDL_GL_DeleteContext((SDL_GLContext)hContext);
}

int OGlRenderStruct::RenderScene(SceneDesc *pScene){
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
        glColor4f(0.5,0.5,1.0,1.0);
        glVertex3f(-0.5,-0.5,0.1);
        glVertex3f(0.0, 0.5,0.1);
        glVertex3f(0.5,-0.5,0.1);
    glEnd();
    return 0;
}

void OGlRenderStruct::SwapBuffers(uint flags ){
    SDL_Window *_win;
    g_pLTClient->GetEngineHook("SDL_Window", (void**)&_win);
    SDL_GL_SwapWindow(_win);    
}

void            OGlRenderStruct::RenderCommand(uint32 argc, const char** argv){}
void			OGlRenderStruct::MakeCubicEnvMap(const char* pszPrefix, uint32 nSize, const SceneDesc& InSceneDesc){}
void            OGlRenderStruct::ReadConsoleVariables(){}
void            OGlRenderStruct::GetRenderInfo(RenderInfoStruct* pRenderInfo){}
CRenderObject*  OGlRenderStruct::CreateRenderObject(CRenderObject::RENDER_OBJECT_TYPES ObjectType){return nullptr;}
bool            OGlRenderStruct::DestroyRenderObject(CRenderObject* pObject){return true;}
bool            OGlRenderStruct::SetLightGroupColor(uint32 nID, const LTVector &vColor){return true;}
LTRESULT        OGlRenderStruct::SetOccluderEnabled(uint32 nID, bool bEnabled){return LT_OK;}
LTRESULT        OGlRenderStruct::GetOccluderEnabled(uint32 nID, bool *pEnabled){ *pEnabled=false; return LT_OK; }
uint32	        OGlRenderStruct::GetTextureEffectVarID(const char* pszEffectGroup, uint32 nStage){return 0;}
bool			OGlRenderStruct::SetTextureEffectVar(uint32 nVarID, uint32 nVar, float fValue){return true;}
bool	        OGlRenderStruct::IsObjectGroupEnabled(uint32 nGroup){return false;}
void			OGlRenderStruct::SetObjectGroupEnabled(uint32 nGroup, bool bEnable){}
void			OGlRenderStruct::SetAllObjectGroupEnabled(){}
bool			OGlRenderStruct::AddGlowRenderStyleMapping(const char* pszSource, const char* pszMapTo){return true;}
bool			OGlRenderStruct::SetGlowDefaultRenderStyle(const char* pszFile){return true;}
bool			OGlRenderStruct::SetNoGlowRenderStyle(const char* pszFile){return true;}
