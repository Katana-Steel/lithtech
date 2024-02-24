// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGridFX.cpp
//
// PURPOSE : PolyGrid special FX - Implementation
//
// CREATED : 10/13/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PolyGridFX.h"
#include "Plasma.h"
#include "cpp_client_de.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "GameSettings.h"

static char g_SinTable[256];
static DBOOL g_bSinTableInitted=DFALSE;

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::Init
//
//	PURPOSE:	Create the poly grid
//
// ----------------------------------------------------------------------- //

DBOOL CPolyGridFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	PGCREATESTRUCT pg;

	pg.hServerObj = hServObj;
	pg.vDims = hRead->ReadVector();
	D_WORD wColor = hRead->ReadWord();
	Color255WordToVector(wColor, &(pg.vColor1));
	wColor = hRead->ReadWord();
	Color255WordToVector(wColor, &(pg.vColor2));
	pg.fXScaleMin = hRead->ReadFloat(); 
	pg.fXScaleMax = hRead->ReadFloat(); 
	pg.fYScaleMin = hRead->ReadFloat(); 
	pg.fYScaleMax = hRead->ReadFloat(); 
	pg.fXScaleDuration = hRead->ReadFloat();
	pg.fYScaleDuration = hRead->ReadFloat();
	pg.fXPan = hRead->ReadFloat();
	pg.fYPan = hRead->ReadFloat();
	pg.fFrequency = hRead->ReadFloat();
	pg.fAlpha = hRead->ReadFloat();
	pg.hstrSurfaceSprite = hRead->ReadHString();
	pg.dwNumPolies = (DDWORD)hRead->ReadWord();
	pg.bAdditive = (DBOOL)hRead->ReadByte();
	pg.bMultiply = (DBOOL)hRead->ReadByte();
	pg.nPlasmaType = hRead->ReadByte();
	pg.nRingRate[0] = hRead->ReadByte();
	pg.nRingRate[1] = hRead->ReadByte();
	pg.nRingRate[2] = hRead->ReadByte();
	pg.nRingRate[3] = hRead->ReadByte();

	return Init(&pg);
}

DBOOL CPolyGridFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return DFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	PGCREATESTRUCT* pg = (PGCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vDims, pg->vDims);
	VEC_COPY(m_vColor1, pg->vColor1);
	VEC_COPY(m_vColor2, pg->vColor2);
	m_fXScaleMin		= pg->fXScaleMin; 
	m_fXScaleMax		= pg->fXScaleMax; 
	m_fYScaleMin		= pg->fYScaleMin; 
	m_fYScaleMax		= pg->fYScaleMax; 
	m_fXScaleDuration	= pg->fXScaleDuration;
	m_fYScaleDuration	= pg->fYScaleDuration;
	m_fXPan				= pg->fXPan;
	m_fYPan				= pg->fYPan;
	m_fFrequency		= pg->fFrequency;
	m_hstrSurfaceSprite = pg->hstrSurfaceSprite;
	m_dwNumPolies		= pg->dwNumPolies;

	m_fAlpha			= pg->fAlpha;
	m_nPlasmaType		= pg->nPlasmaType;
	m_bAdditive			= pg->bAdditive;
	m_bMultiply			= pg->bMultiply;

	m_nRingRate[0]		= pg->nRingRate[0];
	m_nRingRate[1]		= pg->nRingRate[1];
	m_nRingRate[2]		= pg->nRingRate[2];
	m_nRingRate[3]		= pg->nRingRate[3];

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::CreateObject
//
//	PURPOSE:	Create object associated the poly grid
//
// ----------------------------------------------------------------------- //

DBOOL CPolyGridFX::CreateObject(CClientDE *pClientDE)
{
	ColorRamp colorRamps[] = 
	{
		ColorRamp(DVector(m_vColor1.x, m_vColor1.y, m_vColor1.z), 0),
		ColorRamp(DVector(m_vColor2.x, m_vColor2.y, m_vColor2.z), 256)
	};

	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	if (!g_pGameClientShell) return DFALSE;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return DFALSE;

	DVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);

	DRotation rRot;
	pClientDE->GetObjectRotation(m_hServerObject, &rRot);

	// Setup the PolyGrid...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_POLYGRID;
	createStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	DDWORD dwSize = 2;
	if (m_dwNumPolies > 4)
	{
		dwSize = (DDWORD)sqrt((DFLOAT)(m_dwNumPolies/2)) + 1;

		if (m_nPlasmaType == PLASMA_FOUR_RING)
		{
			// Make sure dwSize is a factor of 2 if using four ring plasma...

			if (2 <= dwSize && dwSize < 4) dwSize = 2;
			else if (4 <= dwSize && dwSize <= 6) dwSize = 4;
			else if (6 < dwSize && dwSize <= 12) dwSize = 8;
			else dwSize = 16;
		}
	}

	
	// Adjust the size of the polygrid based on the current polygrids and
	// special fx detail setting...

	if (m_bUseGlobalSettings)
	{
		DBYTE nVal = pSettings->SpecialFXSetting();
		DBOOL bOn  = pSettings->PolyGrids();

		if (!bOn || nVal == RS_LOW)
		{
			dwSize = 4;		// Smallest possible
		}
		else if (bOn && nVal == RS_MED)
		{
			dwSize /= 2;	// Half normal size
		}
	}


	m_hObject = m_pClientDE->CreateObject(&createStruct);
	m_pClientDE->SetupPolyGrid(m_hObject, dwSize, dwSize, DFALSE);
	
	// Set alpha value...

	DFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fAlpha);


	DVector vMin, vMax, vScale;
	VEC_SUB(vMin, vPos, m_vDims);
	VEC_ADD(vMax, vPos, m_vDims);

	m_pClientDE->FitPolyGrid(m_hObject, &vMin, &vMax, &vPos, &vScale);

	if (m_hstrSurfaceSprite)
	{
		char* pSpriteName = m_pClientDE->GetStringData(m_hstrSurfaceSprite);
		if (pSpriteName && pSpriteName[0])
		{
			m_pClientDE->SetPolyGridTexture(m_hObject, pSpriteName);
			m_pClientDE->SetPolyGridTextureInfo(m_hObject, 0, 0, m_fXScaleMin, m_fYScaleMin);
		}
	}

	m_pClientDE->SetObjectPos(m_hObject, &vPos);
	m_pClientDE->SetObjectScale(m_hObject, &vScale);

	SetPolyGridPalette(colorRamps, sizeof(colorRamps)/sizeof(colorRamps[0]));
	PrecalculatePlasma();

	
	// Set blend modes if applicable...

	DDWORD dwFlags;
	g_pInterface->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags);

	DBOOL bFog = DTRUE;
	if (m_bAdditive)
	{
		dwFlags |= FLAG2_ADDITIVE;
		bFog = DFALSE;
	}
	else if (m_bMultiply)
	{
		dwFlags |= FLAG2_MULTIPLY;
		bFog = DFALSE;
	}
	g_pInterface->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags);

	// Testing...
	//if (!bFog)
	//{
	//	g_pInterface->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	//	g_pInterface->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags | FLAG_FOGDISABLE);
	//}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::Update
//
//	PURPOSE:	Update the grid
//
// ----------------------------------------------------------------------- //

DBOOL CPolyGridFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return DFALSE;


	if(IsWaitingForRemove())
		return DFALSE;


	// Set the flags of the polygrid based on the the server object...

	DDWORD dwUserFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

	DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

	if (dwUserFlags & USRFLG_VISIBLE)
	{
		m_pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
	}
	else  // We're hidden, no need to update...
	{
		m_pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
		return DTRUE;
	}

	
	// Don't update if not drawn :)

	if (!(m_pClientDE->GetObjectFlags(m_hObject) & FLAG_WASDRAWN) && !m_bAlwaysUpdate) 
	{
		return DTRUE;
	}

	
	// Update the position of the polygrid to reflect the position of the
	// server object...

	DVector vPos;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);


	// If we're not using polygrids (or special fx are set to the lowest
	// detail setting), don't update...

	if (m_bUseGlobalSettings)
	{
		CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
		if (!pSettings) return DTRUE;

		DBYTE nVal = pSettings->SpecialFXSetting();
		DBOOL bOn  = pSettings->PolyGrids();

		if (!bOn || nVal == RS_LOW)
		{
			return DTRUE;
		}
	}

		
	// Update the plasma based on the type of plasma...

	switch (m_nPlasmaType)
	{
		case PLASMA_FOUR_RING :
			UpdateFourRingPlasma();
		break;

		case PLASMA_NORMAL:
		default :
			 UpdatePlasma();
		break;
	}


	if (m_hstrSurfaceSprite)
	{
		UpdateSurface();
	}


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::UpdateSurface
//
//	PURPOSE:	Update the polygrid's surface fx
//
// ----------------------------------------------------------------------- //

void CPolyGridFX::UpdateSurface()
{
	if (!m_pClientDE || !m_hstrSurfaceSprite) return;

	DFLOAT fXPan, fYPan, fXScale, fYScale;
	m_pClientDE->GetPolyGridTextureInfo(m_hObject, &fXPan, &fYPan, &fXScale, &fYScale);

	DFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

	// Scale in X direction...

	if (m_fXScaleDuration > 0.0f)
	{
		if (m_bScalingXUp)
		{
			fXScale += fDeltaTime * (m_fXScaleMax - m_fXScaleMin) / m_fXScaleDuration;
			if (fXScale > m_fXScaleMax) 
			{
				fXScale = m_fXScaleMax;
				m_bScalingXUp = DFALSE;
			}
		}
		else
		{
			fXScale -= fDeltaTime * (m_fXScaleMax - m_fXScaleMin) / m_fXScaleDuration;
			if (fXScale < m_fXScaleMin) 
			{
				fXScale = m_fXScaleMin;
				m_bScalingXUp = DTRUE;
			}
		}
	}

	// Scale in Y direction...

	if (m_fYScaleDuration > 0.0f)
	{
		if (m_bScalingYUp)
		{
			fYScale += fDeltaTime * (m_fYScaleMax - m_fYScaleMin) / m_fYScaleDuration;
			if (fYScale > m_fYScaleMax) 
			{
				fYScale = m_fYScaleMax;
				m_bScalingYUp = DFALSE;
			}
		}
		else
		{
			fYScale -= fDeltaTime * (m_fYScaleMax - m_fYScaleMin) / m_fYScaleDuration;
			if (fYScale < m_fYScaleMin) 
			{
				fYScale = m_fYScaleMin;
				m_bScalingYUp = DTRUE;
			}
		}
	}

	// Now update the pan...
	fXPan += (fDeltaTime * m_fXPan);
	fYPan += (fDeltaTime * m_fYPan);

	m_pClientDE->SetPolyGridTextureInfo(m_hObject, fXPan, fYPan, fXScale, fYScale);
}



// ----------------------------------------------------------------------- //
// Precalculates the lookup tables for plasma FX.
// ----------------------------------------------------------------------- //

void CPolyGridFX::PrecalculatePlasma()
{
	DDWORD width, height;
	int halfWidth, halfHeight, x, y;
	char *pData;
	PGColor *pColorTable;
	float scale, val, maxDistSqr, testDist, t;
	int i;


	// Create the (scaled) sin table.
	if(!g_bSinTableInitted)
	{
		scale = (MATH_CIRCLE / 255.0f) * 3.0f;
		for(i=0; i < 256; i++)
		{
			val = (float)i * scale;
			g_SinTable[i] = (char)(sin(val) * 128.0f);
		}
	
		g_bSinTableInitted = DTRUE;
	}


	// Fill in the distance grid.
	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &width, &height, &pColorTable);


	m_DistanceGrid = (DBYTE*)malloc(width*height);
	halfWidth = width >> 1;
	halfHeight = height >> 1;
	maxDistSqr = (float)(halfWidth*halfWidth + halfHeight*halfHeight);
	for(y=0; y < (int)height; y++)
	{
		for(x=0; x < (int)width; x++)
		{
			testDist = (float)((x-halfWidth)*(x-halfWidth) + (y-halfHeight)*(y-halfHeight));
			t = testDist / maxDistSqr;
			m_DistanceGrid[y*height+x] = (DBYTE)(t * 255.0f);
		}
	}
}


// ----------------------------------------------------------------------- //
// Inits the palette for the grid.  Pass in a ramp list for it to interpolate thru.
// ----------------------------------------------------------------------- //

void CPolyGridFX::SetPolyGridPalette(ColorRamp *pRamps, int nRamps)
{
	float t;
	DDWORD width, height;
	char *pData;
	PGColor *pColorTable, color1, color2;
	int ramp, i, index1, index2;

	// Randomize the poly grid values.
	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &width, &height, &pColorTable);

	// Make the color table go from white to black.
	for(ramp=0; ramp < nRamps-1; ramp++)
	{
		index1 = pRamps[ramp].m_Index;
		index2 = pRamps[ramp+1].m_Index;
		VEC_COPY(color1, pRamps[ramp].m_Color);
		VEC_COPY(color2, pRamps[ramp+1].m_Color);

		for(i=index1; i < index2; i++)
		{
			t = (float)(i - index1) / (index2 - index1);
			VEC_LERP(pColorTable[i], color1, color2, t);
			pColorTable[i].a = 255.0f;
		}
	}
}


// ----------------------------------------------------------------------- //
// Updates the plasma effect.
// ----------------------------------------------------------------------- //

void CPolyGridFX::UpdatePlasma()
{
	DDWORD width, height, x, y;
	char *pData, *pCur;
	PGColor *pColorTable;
	DBYTE count, *pDistanceGrid;

	m_fCount += m_fFrequency * m_pClientDE->GetFrameTime();
	count = (DBYTE)m_fCount;

	// Randomize the poly grid values.
	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &width, &height, &pColorTable);
	for(y=0; y < height; y++)
	{
		pCur = pData + (y*width);
		
		pDistanceGrid = &m_DistanceGrid[y*width];
		x = width;
		while(x--)
		{
			*pCur = g_SinTable[(DBYTE)((*pDistanceGrid) + count)];
			++pDistanceGrid;
			++pCur;
		}
	}
}

float g_Counts[4] = {0.0f, 1.0f, 4.0f, 3.0f};
long g_Offsets[4][2] =
{
	6, 10,
	13, 3,
	17, 1,
	31, 22
};

// ----------------------------------------------------------------------- //
// Updates the plasma effect.
// ----------------------------------------------------------------------- //

void CPolyGridFX::UpdateFourRingPlasma()
{

	DDWORD dwWidth, dwHeight;
	long width, height, x, y, xMask, yMask, yOffsets[4];
	char *pData, *pCur;
	PGColor *pColorTable;
	DBYTE val[4], counts[4], *pDistanceGrid;
	long i;

	for(i=0; i < 4; i++)
	{
		g_Counts[i] += ((float)m_nRingRate[i]) * m_pClientDE->GetFrameTime();
		counts[i] = (DBYTE)g_Counts[i];
	}

	m_pClientDE->GetPolyGridInfo(m_hObject, &pData, &dwWidth, &dwHeight, &pColorTable);
	width = (long)dwWidth;
	height = (long)dwHeight;

	xMask = width - 1;
	yMask = height - 1;

	for(y=0; y < height; y++)
	{
		pCur = pData + (y*width);
		//pDistanceGrid = &m_DistanceGrid[y*width];
		pDistanceGrid = m_DistanceGrid;

		yOffsets[0] = ((y+g_Offsets[0][1]) & yMask)*width;
		yOffsets[1] = ((y+g_Offsets[1][1]) & yMask)*width;
		yOffsets[2] = ((y+g_Offsets[2][1]) & yMask)*width;
		yOffsets[3] = ((y+g_Offsets[3][1]) & yMask)*width;

		x = width;
		while(x--)
		{
			val[0] = pDistanceGrid[yOffsets[0] + ((x+g_Offsets[0][0]) & xMask)];
			val[1] = pDistanceGrid[yOffsets[1] + ((x+g_Offsets[1][0]) & xMask)];
			val[2] = pDistanceGrid[yOffsets[2] + ((x+g_Offsets[2][0]) & xMask)];
			val[3] = pDistanceGrid[yOffsets[3] + ((x+g_Offsets[3][0]) & xMask)];

			*pCur = (
				g_SinTable[(DBYTE)(val[0] + counts[0])] + 
				g_SinTable[(DBYTE)(val[1] + counts[1])] + 
				g_SinTable[(DBYTE)(val[2] + counts[2])] + 
				g_SinTable[(DBYTE)(val[3] + counts[3])] 
					) >> 2;
			
			//++pDistanceGrid;
			++pCur;
		}
	}
}

//-------------------------------------------------------------------------------------------------
// SFX_PolyGridFactory
//-------------------------------------------------------------------------------------------------

const SFX_PolyGridFactory SFX_PolyGridFactory::registerThis;

CSpecialFX* SFX_PolyGridFactory::MakeShape() const
{
	return new CPolyGridFX();
}

