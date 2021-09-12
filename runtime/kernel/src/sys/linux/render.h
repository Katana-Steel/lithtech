// This file defines DirectEngine's interface to the renderer and holds
// the global RenderStruct used to talk to it.
#ifndef __RENDER_H__
#define __RENDER_H__
struct RenderStruct;
struct RMode;
//  Functions that used to be extracted from the render DLL with GetProcAddress
//  but are now staticly linked.
extern RMode* rdll_GetSupportedModes();
extern void rdll_FreeModeList(RMode *pModes);
void rdll_RenderDLLSetup(RenderStruct *pStruct);
/*
#ifdef __ENABLE_VK
#include "vk_render.h"
#endif
*/
#ifdef __ENABLE_OGL
#include "ogl_render.h"
#endif

//  End.
#ifndef __RENDERSTRUCT_H__
#include "renderstruct.h"
#endif
extern RenderStruct g_Render;
extern RMode        g_RMode;                // The current (or last successful) config for the renderer.
class CClientMgr;
// Render initialization status codes.
#define R_OK                    0
#define R_CANTLOADLIBRARY       1
#define R_INVALIDRENDERDLL      2
#define R_INVALIDRENDEROPTIONS  3
// Returns the RenderStruct it's using to talk to the driver.
// Better be initialized...
inline RenderStruct* r_GetRenderStruct() {return &g_Render;}
inline LTBOOL r_IsRenderInitted() {return g_Render.m_bInitted;}
// Called right at the beginning by the client.. initializes the RenderStruct
// data members.
void r_InitRenderStruct(bool bFullClear);
// Only implemented for client.exe.. initializes the renderer.
LTRESULT r_InitRender(RMode *pMode, const char* window_name);
// surfaceHandling
//    0 = leave surfaces alone
//    1 = backup surfaces
//    2 = delete surfaces
LTRESULT r_TermRender(int surfaceHandling, bool bUnLoadDLL);

//this will run through and release any textures that have a valid file pointer so that they can
//be recreated later on demand
void r_TerminateAllRecreatableTextureData();

#ifdef USE_DXVK
//this should be called to access texture data of a texture. If it has no image and the shared texture
//is properly setup, it will load the image data and bind it to the shared texture
TextureData*	r_GetTextureData(SharedTexture *pTexture);

//this should be called to access information on a texture. It will ensure that it is filled out.
//The values are invalid if it returns false
bool			r_GetTextureInfo(SharedTexture *pTexture, uint32& nWidth, uint32& nHeight, PFormat& Format);

//this will load the texture and bind it to the device. The texture data of the shared texture
//will be valid until it is bound to the device, at which point it is possible that it will
//be freed
LTRESULT r_LoadSystemTexture(SharedTexture *pSharedTexture);

//frees the associated texture data and cleans up references to it
void r_UnloadSystemTexture(TextureData *pTexture);
#endif

//this will bind the texture to the device. Note that the texture data is not guaranteed to be valid
//after this call since the renderer can free it to save memory
void r_BindTexture(SharedTexture *pTexture, bool bTextureChanged);
//unbinds the texture from the device
void r_UnbindTexture(SharedTexture *pTexture, bool bUnloadEngineData);
// Called by the renderer and ILTClient::ProcessAttachments.
LTObject* r_ProcessAttachment(LTObject *pParent, Attachment *pAttachment);

struct SysCache 
{
    uint32  m_CurMem;   // How much memory currently used?

    // All the loaded TextureDatas, in an MRU (most recently used are at
    // the start of the list).
    LTList  m_List; 
};

extern SysCache g_SysCache;
#endif  // __RENDER_H__
