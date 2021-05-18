#include "bdefs.h"
#include "ogl_render.h"
#include "de_objects.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <vector>

namespace {
    inline float intToFloatRange(uint8 c) {
        return float(c)/255.0f;
    }

    inline void RenderObject(LTObject *obj)
    {
        return;
    }
}

OGlRenderStruct::OGlRenderStruct()
{
}

bool OGlRenderStruct::LoadWorldData(ILTStream *pStream)
{
    return false; // STUB we currently want to fail loading data;
}

HRENDERCONTEXT OGlRenderStruct::CreateContext()
{
    auto win = g_ClientGlob.m_window;
    auto cleanUp = SDL_GL_GetCurrentContext();
    hContext = SDL_GL_CreateContext(win);
    SDL_GL_MakeCurrent(win, hContext);
    glLoadIdentity();
    glFrustum(
        -1.0, 1.0,
        -1.0, 1.0,
        0.01, 1
    );
    glViewport(0, 0, m_Width, m_Height);
    if(cleanUp)
        SDL_GL_DeleteContext(cleanUp);
    return (HRENDERCONTEXT)hContext;
}

void OGlRenderStruct::DeleteContext(HRENDERCONTEXT){
    SDL_GL_DeleteContext(hContext);
}

int OGlRenderStruct::RenderScene(SceneDesc *pScene) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(pScene->m_Pos.x, pScene->m_Pos.y, pScene->m_Pos.z);
    auto &&rot = pScene->m_Rotation.m_Quat;
//    glRotatef(rot[3], rot[0], rot[1], rot[2]);
    std::vector<LTObject*> objs;
    objs.resize(pScene->m_ObjectListSize);
    memcpy(objs.data(), pScene->m_pObjectList, sizeof(LTObject*)*pScene->m_ObjectListSize);
    for(LTObject *obj : objs) {
        RenderObject(obj);
    }
    glPopMatrix();
    this->SwapBuffers(0);
    return 0;
}

void OGlRenderStruct::SwapBuffers(uint flags ){
    SDL_GL_SwapWindow(g_ClientGlob.m_window);
}

void            OGlRenderStruct::RenderCommand(uint32 argc, const char** argv){}
void            OGlRenderStruct::MakeCubicEnvMap(const char* pszPrefix, uint32 nSize, const SceneDesc& InSceneDesc){}
void            OGlRenderStruct::ReadConsoleVariables(){}
void            OGlRenderStruct::GetRenderInfo(RenderInfoStruct* pRenderInfo){}
CRenderObject*  OGlRenderStruct::CreateRenderObject(CRenderObject::RENDER_OBJECT_TYPES ObjectType){return nullptr;}
bool            OGlRenderStruct::DestroyRenderObject(CRenderObject* pObject){return true;}
bool            OGlRenderStruct::SetLightGroupColor(uint32 nID, const LTVector &vColor){return true;}
LTRESULT        OGlRenderStruct::SetOccluderEnabled(uint32 nID, bool bEnabled){return LT_OK;}
LTRESULT        OGlRenderStruct::GetOccluderEnabled(uint32 nID, bool *pEnabled){ *pEnabled=false; return LT_OK; }
uint32	        OGlRenderStruct::GetTextureEffectVarID(const char* pszEffectGroup, uint32 nStage){return 0;}
bool            OGlRenderStruct::SetTextureEffectVar(uint32 nVarID, uint32 nVar, float fValue){return true;}
bool            OGlRenderStruct::IsObjectGroupEnabled(uint32 nGroup){return false;}
void            OGlRenderStruct::SetObjectGroupEnabled(uint32 nGroup, bool bEnable){}
void            OGlRenderStruct::SetAllObjectGroupEnabled(){}
bool            OGlRenderStruct::AddGlowRenderStyleMapping(const char* pszSource, const char* pszMapTo){return true;}
bool            OGlRenderStruct::SetGlowDefaultRenderStyle(const char* pszFile){return true;}
bool            OGlRenderStruct::SetNoGlowRenderStyle(const char* pszFile){return true;}
