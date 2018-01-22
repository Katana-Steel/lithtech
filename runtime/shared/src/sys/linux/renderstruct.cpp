#include "bdefs.h"
#include "render.h"
#include "consolecommands.h"
#include "dsys.h"
#include "clientmgr.h"
#include "sysclientde_impl.h"


#ifdef LT_VK_AVAILABLE
VKRenderStruct vkrdr;
void vkReadConsoleVariables()
{
  vkrdr.ReadConsoleVariables();
}
#endif
OGlRenderStruct oglrdr;
void oglReadConsoleVariables()
{
  oglrdr.ReadConsoleVariables();
}

void r_InitRenderStruct(bool bFullClear)
{
    char filename[MAX_PATH];
    memset(filename, 0, MAX_PATH);

    const char* renderlib = "libOglrender.so";
    bool copied;
    LTRESULT dResult = GetOrCopyClientFile(renderlib, filename, MAX_PATH, copied);
    if (dResult != LTTRUE) {
        g_pClientMgr->SetupError(LT_ERRORCOPYINGFILE, renderlib);
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_ERRORCOPYINGFILE, renderlib);
    }

    auto status = bm_BindModule(filename, copied, g_Render);

    RenderStructInit rsi{0,bFullClear, '\0'};
    g_Render.Init(&rsi);
}

int RenderStruct::Init(RenderStructInit * pInit)
{
#ifdef LT_VK_AVAILABLE
	ci_string render(command_line_args->FindArgDash("render"));
	if(render == ci_string("vulkan"))
	{
	    this->m_pRender = &vkrdr;
            this->ReadConsoleVariables = &(vkReadConsoleVariables);

        } else
#endif
        {
            this->m_pRender = &oglrdr;
            this->ReadConsoleVariables = &(oglReadConsoleVariables);
        }
	pInit->m_RendererVersion = LTRENDER_VERSION;
	return 0;
}

HRENDERCONTEXT RenderStruct::CreateContext()
{
    return this->m_pRender->CreateContext();
}

bool RenderStruct::LoadWorldData(ILTStream *pStream)
{
    return this->m_pRender->LoadWorldData(pStream);
}
void           RenderStruct::DeleteContext(HRENDERCONTEXT hContext)
{
    return this->m_pRender->DeleteContext(hContext);
}
int            RenderStruct::RenderScene(SceneDesc *pScene)
{
    return this->m_pRender->RenderScene(pScene);
}
void           RenderStruct::RenderCommand(uint32 argc, const char** argv)
{
    return this->m_pRender->RenderCommand(argc, argv);
}
void           RenderStruct::SwapBuffers(uint flags )
{
    return this->m_pRender->SwapBuffers(flags);
}
void           RenderStruct::MakeCubicEnvMap(const char* pszPrefix, uint32 nSize, const SceneDesc& InSceneDesc)
{
    return this->m_pRender->MakeCubicEnvMap(pszPrefix, nSize, InSceneDesc);
}
void           RenderStruct::GetRenderInfo(RenderInfoStruct* pRenderInfo)
{
    return this->m_pRender->GetRenderInfo(pRenderInfo);
}
CRenderObject* RenderStruct::CreateRenderObject(CRenderObject::RENDER_OBJECT_TYPES ObjectType)
{
    return this->m_pRender->CreateRenderObject(ObjectType);
}
bool           RenderStruct::DestroyRenderObject(CRenderObject* pObject)
{
    return this->m_pRender->DestroyRenderObject(pObject);
}
bool           RenderStruct::SetLightGroupColor(uint32 nID, const LTVector &vColor)
{
    return this->m_pRender->SetLightGroupColor(nID,vColor);
}
LTRESULT       RenderStruct::SetOccluderEnabled(uint32 nID, bool bEnabled)
{
    return this->m_pRender->SetOccluderEnabled(nID,bEnabled);
}
LTRESULT       RenderStruct::GetOccluderEnabled(uint32 nID, bool *pEnabled)
{
    return this->m_pRender->GetOccluderEnabled(nID, pEnabled);
}
uint32         RenderStruct::GetTextureEffectVarID(const char* pszEffectGroup, uint32 nStage)
{
    return this->m_pRender->GetTextureEffectVarID(pszEffectGroup, nStage);
}
bool           RenderStruct::SetTextureEffectVar(uint32 nVarID, uint32 nVar, float fValue)
{
    return this->m_pRender->SetTextureEffectVar(nVarID,nVar,fValue);
}
bool           RenderStruct::IsObjectGroupEnabled(uint32 nGroup)
{
    return this->m_pRender->IsObjectGroupEnabled(nGroup);
}
void           RenderStruct::SetObjectGroupEnabled(uint32 nGroup, bool bEnable)
{
    return this->m_pRender->SetObjectGroupEnabled(nGroup, bEnable);
}
void           RenderStruct::SetAllObjectGroupEnabled()
{
    return this->m_pRender->SetAllObjectGroupEnabled();
}
bool           RenderStruct::AddGlowRenderStyleMapping(const char* pszSource, const char* pszMapTo)
{
    return this->m_pRender->AddGlowRenderStyleMapping(pszSource, pszMapTo);
}
bool           RenderStruct::SetGlowDefaultRenderStyle(const char* pszFile)
{
    return this->m_pRender->SetGlowDefaultRenderStyle(pszFile);
}
bool           RenderStruct::SetNoGlowRenderStyle(const char* pszFile)
{
    return this->m_pRender->SetNoGlowRenderStyle(pszFile);
}
