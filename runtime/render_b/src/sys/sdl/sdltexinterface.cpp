#include "bdefs.h"
#include "ilttexinterface.h"
#include "sdltexinterface.h"
#include "render.h"

#include <SDL2/SDL.h>

// interface database
define_interface(CSysTexInterface, ILTTexInterface);

// SharedTexture == SDL_Surface
struct SharedTexture
{
    SDL_Surface *tex;
    uint64_t ref;

    SharedTexture(uint32 w=16, uint32 h=16) : ref(0)
    {
        tex = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
    }

    ~SharedTexture()
    {
        SDL_FreeSurface(tex);
        tex = nullptr; // just incase
    }
};
// Find texture in memory if it exists and return its handle.
// If it does not exist, this function should return an error.
LTRESULT CSysTexInterface::FindTextureFromName(HTEXTURE &hTexture, const char *pFilename) 
{
    hTexture = new SharedTexture;
    return LT_OK;
}

LTRESULT CSysTexInterface::CreateTextureFromName(HTEXTURE &hTexture, const char *pFilename)
{
    hTexture = new SharedTexture;
    return LT_OK;
}

LTRESULT CSysTexInterface::CreateTextureFromData(HTEXTURE &hTexture, ETextureType eTextureType, uint32 TextureFlags, 
	uint8 *pData, uint32 nWidth, uint32 nHeight, uint32 nAutoGenMipMaps) 
{
    hTexture = new SharedTexture(nWidth, nHeight);
    hTexture->tex->pixels = pData;
    return LT_OK;
}

LTRESULT CSysTexInterface::GetTextureData(const HTEXTURE hTexture, const uint8* &pData, uint32 &nPitch, uint32& nWidth, uint32& nHeight, ETextureType& eType) 
{
    return LT_ERROR;
}

LTRESULT CSysTexInterface::FlushTextureData (const HTEXTURE hTexture, ETextureMod eChanged, uint32 nMipMap) 
{
	r_BindTexture(hTexture, true);		 // Bind it to the renderer.
	return LT_OK;
}

LTRESULT CSysTexInterface::GetTextureType(const HTEXTURE hTexture, ETextureType &eTextureType)
{
    return LT_ERROR;
}

LTRESULT CSysTexInterface::GetTextureDims(const HTEXTURE hTexture, uint32 &nWidth, uint32 &nHeight) 
{
    if (!hTexture)
        return LT_ERROR;
    nWidth = hTexture->tex->w;
    nHeight = hTexture->tex->h;
    return LT_OK;
}

bool CSysTexInterface::ReleaseTextureHandle(const HTEXTURE hTexture)
{
    if(hTexture)
        if (--(hTexture->ref) < 1) {
            delete hTexture;
            return true;
        }
    return false;
}

uint32 CSysTexInterface::AddRefTextureHandle(const HTEXTURE hTexture)
{
    if(hTexture)
        return ++hTexture->ref;
    return 0;
}
