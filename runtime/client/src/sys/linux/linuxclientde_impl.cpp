#include "bdefs.h"

#include "clientmgr.h"
#include "iltclient.h"
#include "console.h"
#include "bindmgr.h"
#include "linuxclientde_impl.h"
#include "renderstruct.h"
#include "render.h"

//the ILTClient game interface
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

static LTRESULT
cis_GetEngineHook(const char *name, void **pData)
{
    if(stricmp("hwnd", name) == 0)
    {
		*pData = g_ClientGlob.m_window;
        return LT_OK;
    } else if(stricmp("sdl_window", name) == 0)
    {
		*pData = g_ClientGlob.m_window;
        return LT_OK;
    } else if(stricmp(name, "cshell_hinstance") == 0)
	{
		return bm_GetInstanceHandle(g_pClientMgr->m_hShellModule, pData);
	} else if(stricmp("cres_hinstance", name) == 0)
    {
		return bm_GetInstanceHandle(g_pClientMgr->m_hClientResourceModule, pData);
    }

    return LT_ERROR;
}

void cis_Term()
{}

bool cis_RendererIsHere(RenderStruct *pStruct)
{return false;}
bool cis_RendererGoingAway()
{return false;}

static HSURFACE cis_CreateSurface(uint32 w, uint32 h)
{
    return (HSURFACE)SDL_CreateRGBSurface(0, w, h,32,0,0,0,0);
}

LTRESULT cis_DeleteSurface(HSURFACE hSurface)
{
    SDL_FreeSurface((SDL_Surface*)hSurface);
    return LT_OK; // rkj STUB
}

HSURFACE
cis_CreateSurfaceFromPcx(LoadedBitmap *pBitmap)
{
    return cis_CreateSurface(32, 32);
}

static HSURFACE
cis_CreateSurfaceFromBitmap(const char* pBitmap)
{
    return cis_CreateSurfaceFromPcx(nullptr);
}

static HSURFACE cis_GetScreenSurface()
{
    return nullptr;
}

static void cis_GetSurfaceDimentions(HSURFACE surface, uint32 *x, uint32 *y)
{
    if(!surface)
        return;
    SDL_Surface *s = (SDL_Surface*)surface;
    *x = s->w;
    *y = s->h;
}
static LTRESULT cis_SetSurfaceAlpha(HSURFACE surface, float alpha)
{
    SDL_SetSurfaceAlphaMod((SDL_Surface*)surface, (0xff && (255.0f * alpha)));
    return LT_OK;
}

static LTRESULT cis_GetSurfaceAlpha(HSURFACE surface, float &alpha)
{
    uint8 nAlpha=0;
    SDL_GetSurfaceAlphaMod((SDL_Surface*)surface, &nAlpha);
    alpha = (float)nAlpha / 255.0f;
    return LT_OK;
}

static HLTCOLOR cis_CreateColor(float r, float g, float b, bool bTransparent)
{
	if(bTransparent)
	{
		return SETRGB_FT(r,g,b);
	}
	else
	{
		return SETRGB_F(r,g,b);
	}
}

static LTRESULT cis_FillRect(HSURFACE surface, LTRect *rc, HLTCOLOR color)
{
    if(!surface)
        return LT_ERROR;
    SDL_Surface *s = (SDL_Surface *)surface;
    rc->Init(0, 0, s->w, s->h);
    return LT_OK;
}

static LTRESULT cis_OptimizeSurface(HSURFACE surface, HLTCOLOR trans)
{
    return LT_OK;
}

static LTRESULT
cis_DrawSurfaceOnSurface(HSURFACE dest, HSURFACE src, LTRect *rc, int px, int py, HLTCOLOR transparent)
{
    return LT_OK;
}

static LTRESULT
cis_RenderObjects(HLOCALOBJ cam, HLOCALOBJ *objs, int count, float fTime)
{
    if(g_pClientMgr->Render((CameraInstance*)cam, DRAWMODE_OBJECTLIST, objs, count, fTime))
        return LT_OK;
    RETURN_ERROR(1, RenderObjects, LT_ERROR);
}

static LTRESULT
cis_ClearScreen(LTRect*,uint32,LTRGB*)
{
    return LT_OK;
}

static LTRESULT
cis_FlipScreen(uint32)
{
    return LT_OK;
}

/* Start3D
StartOptimized2D
EndOptimized2D  should be 3 functions */
static LTRESULT
cis_GraphicStub()
{
    return LT_OK; // because I cheat.... and It's rarely checked.
}
static LTRESULT
cis_End3D(uint32)
{
    r_GetRenderStruct()->DeleteContext(nullptr);
    return LT_OK; // because I cheat.... and It's rarely checked.
}

static LTRESULT
cis_Start3D()
{
    r_GetRenderStruct()->CreateContext();
    return LT_OK;
}

void cis_Init()
{
    ilt_client->GetEngineHook = cis_GetEngineHook;
    ilt_client->CreateSurfaceFromBitmap = cis_CreateSurfaceFromBitmap;
    ilt_client->DeleteSurface = cis_DeleteSurface;
    ilt_client->GetScreenSurface = cis_GetScreenSurface;
    ilt_client->CreateSurface = cis_CreateSurface;
    ilt_client->GetSurfaceDims = cis_GetSurfaceDimentions;
    ilt_client->SetSurfaceAlpha = cis_SetSurfaceAlpha;
    ilt_client->GetSurfaceAlpha = cis_GetSurfaceAlpha;
    ilt_client->OptimizeSurface = cis_OptimizeSurface;
    ilt_client->FillRect = cis_FillRect;
    ilt_client->SetupColor1 = cis_CreateColor;
    ilt_client->SetupColor2 = cis_CreateColor;
    ilt_client->ClearScreen = cis_ClearScreen;
    ilt_client->FlipScreen = cis_FlipScreen;
    ilt_client->Start3D = cis_Start3D;// ();
    ilt_client->StartOptimized2D = cis_GraphicStub;//();
    ilt_client->DrawSurfaceToSurfaceTransparent = cis_DrawSurfaceOnSurface;//(ilt_client->GetScreenSurface(), m_hSurfCursor, LTNULL,
    ilt_client->EndOptimized2D = cis_GraphicStub;//();
    ilt_client->End3D = cis_End3D;//(END3D_CANDRAWCONSOLE);

    ilt_client->RenderObjects = cis_RenderObjects;

}
