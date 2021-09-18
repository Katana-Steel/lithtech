#include "bdefs.h"
#include "dtxmgr.h"
#include "sysstreamsim.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif

#ifndef __CUIVECTORFONT_H__
#include "cuivectorfont.h"
#endif

#ifndef __LTSYSOPTIM_H__
#include "ltsysoptim.h"
#endif

#include "LTFontParams.h"

#include "iltclient.h"
#include "interface_helpers.h"

#include <string>

// get the ILTTexInterface from the interface database
static ILTTexInterface *pTexInterface = NULL;
define_holder(ILTTexInterface, pTexInterface);

//ILTClient game interface
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

class InstalledFontFace
{
public:

	InstalledFontFace()
	{
		m_nHeight = 0;
		m_ttf_buffer = nullptr;
	}

	~InstalledFontFace()
	{
		Term();
	}

	// Intialize the font face, from optional font file.
	bool Init(char const* pszFontFile, char const* pszFontFace, int nHeight, LTFontParams* fontParams);
	void Term();

	char const* GetFontFace() { return m_sFontFace.c_str(); }
	int				GetHeight() { return m_nHeight; }
	unsigned char* GetBuffer() { return m_ttf_buffer; }

private:

	std::string		m_sFontFace;
	int				m_nHeight;
	unsigned char* m_ttf_buffer;
};

bool InstalledFontFace::Init(char const* pszFontFile, char const* pszFontFace, int nHeight, LTFontParams* fontParams)
{
	unsigned long file_len;
	ILTStream *pStream;

	// Check inputs.
	if (!pszFontFace || !pszFontFace[0])
	{
		ASSERT(!"InstalledFontFace::Init:  Invalid parameters.");
		return false;
	}

	// Start fresh.
	Term();

	// If they specified a font file, install it.
	if (pszFontFile && pszFontFile[0])
	{
		ilt_client->OpenFile(pszFontFile, &pStream);
		if(!pStream)
		{
			return false;
		}

		file_len = pStream->GetLen();

		m_ttf_buffer = new unsigned char[file_len];

		pStream->Read(m_ttf_buffer, file_len);
		pStream->Release();
	}

	m_sFontFace = pszFontFace;
	m_nHeight = nHeight;

	return true;
}

void InstalledFontFace::Term()
{
	if (m_ttf_buffer)
	{
		delete[] m_ttf_buffer;
		m_ttf_buffer = NULL;
	}

	m_sFontFace = "";
	m_nHeight = 0;
}


//	--------------------------------------------------------------------------
// create a proportional bitmap font from a TrueType resource

CUIVectorFont::CUIVectorFont()
{
}

CUIVectorFont::~CUIVectorFont()
{
	Term();
}

bool CUIVectorFont::Init(
	char const* pszFontFile,
	char const* pszFontFace,
	uint32 pointSize,
	uint8  asciiStart,
	uint8  asciiEnd,
	LTFontParams* fontParams
)
{
	char szChars[256];
	int i;
	for (i = asciiStart; i <= asciiEnd; i++)
	{
		szChars[i - asciiStart] = i;
	}
	szChars[i - asciiStart] = 0;

	bool bOk = Init(pszFontFile, pszFontFace, pointSize, szChars, fontParams);

	return bOk;
}


// create a proportional font from TTF, and string
bool CUIVectorFont::Init(char const* pszFontFile,
	char const* pszFontFace,
	uint32 pointSize,
	char const* pszCharacters,
	LTFontParams* fontParams
)
{
	// Check inputs.
	if (!pszCharacters || !pszCharacters[0] || !pszFontFace || !pszFontFace[0])
		return false;

	// Start fresh.
	Term();

	// set the font defaults
	m_Proportional = true;

	InstalledFontFace installedFontFace;
	if (!installedFontFace.Init(pszFontFile, pszFontFace, pointSize, fontParams))
		return false;

	// when the font is created, this can be
	// slightly different than pointSize
	m_PointSize = pointSize;

	// Create a Texture and a Font table
	bool bOk = CreateFontTextureAndTable(installedFontFace, pszCharacters, true);
	if (!bOk)
	{
		return false;
	}

	// font is valid
	m_Valid = true;
	return true;
}

void CUIVectorFont::Term()
{
	// free any used resources
	if (m_bAllocatedTable && m_pFontTable)
	{
		delete[] m_pFontTable;
		m_pFontTable = NULL;
	}

	if (m_bAllocatedMap && m_pFontMap)
	{
		delete[] m_pFontMap;
		m_pFontMap = NULL;
	}

	// release the HTEXTURE
	if (m_Texture)
	{
		pTexInterface->ReleaseTextureHandle(m_Texture);
		m_Texture = NULL;
	}

	m_CharTexWidth = 0;
	m_CharTexHeight = 0;

	// font is no longer valid
	m_Valid = false;
}

// Spacing between each character in font map.
const int kCharSpacing = 2;

inline int GetPowerOfTwo(int nValue)
{
	int nPowerOfTwo = 32;
	while (nPowerOfTwo < nValue)
	{
		nPowerOfTwo *= 2;
	}

	return nPowerOfTwo;
}

struct glyph_metrics_t
{
	unsigned int width;
	unsigned int height;
	int origin_x;
	int origin_y;
};

struct size2d_t
{
	unsigned int cx;
	unsigned int cy;
};

static void GetTextureSizeFromCharSizes(glyph_metrics_t const* pGlyphMetrics, size2d_t const& sizeMaxGlyphSize,
	int nLen, size2d_t& sizeTexture)
{
	// Get the total area of the pixels of all the characters.  We use the largest glyph size
	// rather than the exact values because this is just a rough
	// guess and if we overestimate in width, we just get a shorter texture.
	int nTotalPixelArea = (sizeMaxGlyphSize.cx + kCharSpacing) * nLen *
		(sizeMaxGlyphSize.cy + kCharSpacing);

	// Use the square root of the area guess at the width.
	int nRawWidth = static_cast<int>(sqrtf(static_cast<float>(nTotalPixelArea)) + 0.5f);

	// Englarge the width to the nearest power of two and use that as our final width.
	sizeTexture.cx = GetPowerOfTwo(nRawWidth);

	// Start the height off as one row.
	int nRawHeight = sizeMaxGlyphSize.cy + kCharSpacing;

	// To find the height, keep putting characters into the rows until we reach the bottom.
	int nXOffset = 0;
	for (int nGlyph = 0; nGlyph < nLen; nGlyph++)
	{
		// Get this character's width.
		int nCharWidth = pGlyphMetrics[nGlyph].width;

		// See if this width fits in the current row.
		int nNewXOffset = nXOffset + nCharWidth + kCharSpacing;
		if (nNewXOffset < sizeTexture.cx)
		{
			// Still fits in the current row.
			nXOffset = nNewXOffset;
		}
		else
		{
			// Doesn't fit in the current row.  Englarge by one row
			// and start at the left again.
			nXOffset = 0;
			nRawHeight += sizeMaxGlyphSize.cy + kCharSpacing;
		}
	}

	// Enlarge the height to the nearest power of two and use that as our final height.
	sizeTexture.cy = GetPowerOfTwo(nRawHeight);
}

static int MulDiv(int number, int numerator, int denominator)
{
    return (int)(((long)number * numerator + (denominator >> 1)) / denominator);
}

static void CopyGlyphBitmapToPixelData(unsigned char* bitmap, uint8* pPixelData,
	unsigned int w0, unsigned int h0, unsigned int w1)
{
	uint16* pData;
	unsigned int i, j;
	uint32 nVal;

	for (i = 0; i < h0; i++)
	{
		for (j = 0; j < w0; j++)
		{
			nVal = MulDiv(bitmap[(w0 * i) + j], 15, 255);
			pData = (uint16*)(pPixelData + (w1 * i * 2) + (j * 2));
			*pData = (*pData & 0x0FFF) | ((nVal & 0x0F) << 12);
		}
	}
}

static bool GetGlyphSizes(char const* pszChars, int nLen,
	glyph_metrics_t* pGlyphMetrics, size2d_t& sizeMaxGlyphSize, stbtt_fontinfo* info, float scale, int ascent)
{
	// Get the individual widths of all chars in font, indexed by character values.
	int ix0, ix1, iy0, iy1;
	memset(pGlyphMetrics, 0, sizeof(glyph_metrics_t) * nLen);
	sizeMaxGlyphSize.cx = sizeMaxGlyphSize.cy = 0;
	for (int nGlyph = 0; nGlyph < nLen; nGlyph++)
	{
		// Get the character for this glyph.
		char nChar = pszChars[nGlyph];
		glyph_metrics_t& glyphMetrics = pGlyphMetrics[nGlyph];

		stbtt_GetCodepointBitmapBox(info, nChar, scale, scale, &ix0, &iy0, &ix1, &iy1);

		glyphMetrics.origin_x = ix0;
		glyphMetrics.origin_y = -iy0;
		glyphMetrics.width = ix1 - ix0;
		glyphMetrics.height = iy1 - iy0;

		if (glyphMetrics.width > sizeMaxGlyphSize.cx)
		{
			sizeMaxGlyphSize.cx = glyphMetrics.width;
		}

		// The glyph bitmap will be offset into the texture character slot in the
		// y direction.  The maximum it will take up in the font texture needs to include
		// the amount it's offset.

		if (ascent + iy1 > sizeMaxGlyphSize.cy)
		{
			sizeMaxGlyphSize.cy = ascent + iy1;
		}
	}

	return true;
}

bool CUIVectorFont::CreateFontTextureAndTable(InstalledFontFace& installedFontFace,
	char const* pszChars, bool bMakeMap)
{
	bool bOk = true;
	int i;
	unsigned char* temp_bitmap;
	stbtt_packedchar* chars;
	unsigned char* ttf;
	stbtt_fontinfo info;

	// sanity check
	if (!pTexInterface)
	{
		DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable: No Interface"));
		return false;
	}

	// Check inputs.
	if (!pszChars || !pszChars[0])
	{
		DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable:  Invalid parameters"));
		return false;
	}

	// Get the number of characters to put in font.
	int nLen = (int)strlen(pszChars);
	int offset;

	// This will hold the size of the texture used for rendering.
	size2d_t sizeTexture;

	// This will hold the sizes of each glyph in the font.  Index 0 of aGlyphSizes is for index 0 of pszChars.
	glyph_metrics_t aGlyphMetrics[256];
	size2d_t sizeMaxGlyphSize;

	float f_ascent;
	float conv, f_gap, scale;

	int i_ascent, i_descent, i_gap;
	int advance, left;

	ttf = installedFontFace.GetBuffer();
	offset = stbtt_GetFontOffsetForIndex(ttf, 0);
	stbtt_InitFont(&info, ttf, offset);
	scale = stbtt_ScaleForPixelHeight(&info, (float)installedFontFace.GetHeight());

	if (bOk)
	{
		stbtt_GetFontVMetrics(&info, &i_ascent, &i_descent, &i_gap);
		f_ascent = (float)i_ascent * scale;

		// Get the sizes of each glyph.
		bOk = GetGlyphSizes(pszChars, nLen, aGlyphMetrics,
			sizeMaxGlyphSize, &info, scale, (int)f_ascent);
	}

	if (bOk)
	{

		// Get the size of the default character.
		size2d_t sizeTextExtent;
		//~ GetTextExtentPoint32(hDC, " ", 1, &sizeTextExtent);
		//RKNSTUB
		stbtt_GetCodepointHMetrics(&info, 32, &advance, &left);
		conv = (float)advance * scale;
		sizeTextExtent.cx = 32;
		sizeTextExtent.cy = 32;
		m_DefaultCharScreenWidth = static_cast<uint8>(conv);
		m_DefaultCharScreenHeight = static_cast<uint8>(installedFontFace.GetHeight());
		m_DefaultVerticalSpacing = (uint32)(((float)m_DefaultCharScreenHeight / 4.0f) + 0.5f);

		// Get the average info on the characters.  The width isn't used
		// for proportional fonts, so using an average is ok.
		m_CharTexWidth = (uint8)25; //RKNSTUB
		m_CharTexHeight = static_cast<uint8>(installedFontFace.GetHeight());

		// Get the size our font texture should be to hold all the characters.
		GetTextureSizeFromCharSizes(aGlyphMetrics, sizeMaxGlyphSize, nLen, sizeTexture);

		// allocate the font table.  This contains the texture offsets for
		// the characters.
		LT_MEM_TRACK_ALLOC(m_pFontTable = new uint16[nLen * 3], LT_MEM_TYPE_UI);
		bOk = m_bAllocatedTable = (m_pFontTable != NULL);
	}

	if (bOk)
	{
		// allocate the font map.  This contains the character mappings.
		if (bMakeMap)
		{
			LT_MEM_TRACK_ALLOC(m_pFontMap = new uint8[256], LT_MEM_TYPE_UI);
			bOk = m_bAllocatedMap = (m_pFontMap != NULL);
		}
	}

	// This will be filled in with the pixel data of the font.
	uint8* pPixelData = NULL;

	// Calculate the pixeldata pitch.
	int nPixelDataPitch =  (((( uint32 )16 * sizeTexture.cx + 7) / 8 + 3) & ~3 );
	int nPixelDataSize = nPixelDataPitch * sizeTexture.cy;

	if (bOk)
	{
		// Allocate an array to copy the font into.
		LT_MEM_TRACK_ALLOC(pPixelData = new uint8[nPixelDataSize], LT_MEM_TYPE_UI);
		bOk = (pPixelData != NULL);
		if (!bOk)
			DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable:  Failed to create pixeldata."));
	}

	if (bOk)
	{
		// set the whole font texture to pure white, with alpha of 0.  When
		// we copy the glyph from the bitmap to the pixeldata, we just
		// affect the alpha, which allows the font to antialias with any color.
		uint16* pData = (uint16*)pPixelData;
		uint16* pPixelDataEnd = (uint16*)(pPixelData + nPixelDataSize);
		while (pData < pPixelDataEnd)
		{
			pData[0] = 0x0FFF;
			pData++;
		}

		// This will hold the UV offset for the font texture.
		size2d_t sizeOffset;
		sizeOffset.cx = sizeOffset.cy = 0;

		// Iterate over the characters.
		for (int nGlyph = 0; nGlyph < nLen; nGlyph++)
		{
			// Get this character's width.
			char nChar = pszChars[nGlyph];
			glyph_metrics_t& glyphMetrics = aGlyphMetrics[nGlyph];
			int nCharWidthWithSpacing = glyphMetrics.width + kCharSpacing;

			int nCharRightSide = sizeOffset.cx + nCharWidthWithSpacing;
			if (nCharRightSide >= sizeTexture.cx)
			{
				// Doesn't fit in the current row.  Go to the next row.
				sizeOffset.cx = 0;
				sizeOffset.cy += sizeMaxGlyphSize.cy + kCharSpacing;
			}

			// Fill in the font character map if we have one.
			if (m_pFontMap)
				m_pFontMap[(uint8)nChar] = nGlyph;

			// Char width.
			m_pFontTable[nGlyph * 3] = nCharWidthWithSpacing;

			// X Offset.
			m_pFontTable[nGlyph * 3 + 1] = (uint16)sizeOffset.cx;

			// Y Offset.
			m_pFontTable[nGlyph * 3 + 2] = (uint16)sizeOffset.cy;

			int rw, rh, rx, ry;
			temp_bitmap = stbtt_GetCodepointBitmap(&info, 0.0f,
				stbtt_ScaleForPixelHeight(&info,
				(float)installedFontFace.GetHeight()), nChar, &rw, &rh, &rx, &ry);

			// Find pointer to region within the pixel data to copy the glyph
			// and copy the glyph into the pixeldata.
			int y_offset = (sizeOffset.cy + (int)f_ascent - glyphMetrics.origin_y) * sizeTexture.cx * 2;
			if (y_offset  >= nPixelDataSize || y_offset < 0)
			{
				y_offset = 0;
			}
			uint8* texture_offset = (pPixelData + (y_offset) + (m_pFontTable[nGlyph * 3 + 1] * 2));

			CopyGlyphBitmapToPixelData(temp_bitmap, texture_offset, rw, rh, sizeTexture.cx);
			free(temp_bitmap);

			// Update to the next offset for the next character.
			sizeOffset.cx += nCharWidthWithSpacing;
		}
	}

	if (bOk)
	{
		// turn pixeldata into a texture
		pTexInterface->CreateTextureFromData(
			m_Texture,
			TEXTURETYPE_ARGB4444,
			TEXTUREFLAG_PREFER16BIT | TEXTUREFLAG_PREFER4444,
			pPixelData,
			sizeTexture.cx,
			sizeTexture.cy);
		if (!m_Texture)
		{
			DEBUG_PRINT(1, ("CUIVectorFont::CreateFontTextureAndTable:  Couldn't create texture."));
			bOk = false;
		}
	}

	// Don't need pixel data any more.
	if (pPixelData)
	{
		delete[] pPixelData;
		pPixelData = NULL;
	}

	// Clean up if we had an error.
	if (!bOk)
	{
		if (m_pFontTable)
		{
			delete[] m_pFontTable;
			m_pFontTable = NULL;
		}

		if (m_pFontMap)
		{
			delete[] m_pFontMap;
			m_pFontMap = NULL;
		}

		if (m_Texture)
		{
			pTexInterface->ReleaseTextureHandle(m_Texture);
			m_Texture = NULL;
		}
	}

	return bOk;
}
