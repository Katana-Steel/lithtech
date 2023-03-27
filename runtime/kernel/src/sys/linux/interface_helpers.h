#ifdef USE_DXVK
#include "../win/interface_helpers.h"
#else
// This module implements lots of helpful rectangle and 2d clipping
// routines used for interface stuff.
#ifndef __INTERFACE_HELPERS_H__
#define __INTERFACE_HELPERS_H__
#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif
#ifndef __RENDER_H__
#include "render.h"
#endif
// Surface flags.
#define SURFFLAG_SCREEN         (1<<0)  // This is the screen surface..
#define SURFFLAG_OPTIMIZED      (1<<1)  // This surface is 'optimized' by the renderer.
#define SURFFLAG_OPTIMIZEDIRTY  (1<<2)  // This surface has been changed and needs to be
                                        // reoptimized.
// These are used to simplify routines.
class Pixel16 {}; // stub
class Pixel32 {}; // stub
// The font structure.
class LTFont : public CGLLNode {}; // stub
// The surface structure.. 
class CisSurface : public CGLLNode {}; // stub
        // Used when restarting the renderer to backup the surface.
        // All the surface info you always wanted but were afraid to ask for.
        // Is it the screen or a normal surface, etc...
        // If this is NO_OPTIMIZED_TRANSPARENCY
        // User data..
        // Surface alpha value (only used if optimized).
#define MAX_WARP_POINTS 10
#define WARP_FIXED              int32
#define WARP_FIXED_SHIFT        16
#define FLOAT_TO_WARP_FIXED(x)  (WARP_FIXED)(x * (float)(1<<WARP_FIXED_SHIFT))
typedef struct
{
    short       m_DestX;
    WARP_FIXED  m_SourceX, m_SourceY;   // Fixed point source X and Y.
} WarpCoords;

// These take the base number and shift them left by 3 WITH sign extending.
typedef LTRESULT (DrawWarpFn)(CisSurface *pDest, CisSurface *pSrc,
    WarpCoords *pLeftCoords, WarpCoords *pRightCoords, uint32 minY, uint32 maxY);

// ----------------------------------------------------------------- //
// Functions implemented in clientde_impl_sys.cpp.
// ----------------------------------------------------------------- //
//gone now, use IClientFormatMgr interface.
//FormatMgr* cis_GetFormatMgr();
// ----------------------------------------------------------------- //
// Helper functions.
// ----------------------------------------------------------------- //
// Note: this clips a little differently than ClipRectsNonScaled.  It first
// CLAMPS the source coordinates to be inside the source rectangle, then clips
// the destination rectangle.
inline LTBOOL cis_IsScreenSurface(CisSurface *pSurface) { return LTFALSE; } // stub
inline void cis_SetDirty(CisSurface *pSurface) {} // stub
inline void* cis_LockSurface(CisSurface *pSurface, long &pitch, LTBOOL bSetDirty=LTFALSE) { return nullptr; } // stub
        //pitch = pSurface->m_Pitch;
inline void cis_UnlockSurface(CisSurface *pSurface) {} // stub
inline LTBOOL IsColorTransparent(HLTCOLOR hColor) { return LTFALSE; } // stub
#endif  // __INTERFACE_HELPERS_H__
#endif  // USE_DXVK
