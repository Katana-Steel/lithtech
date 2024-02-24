// ----------------------------------------------------------------------- //
//
// MODULE  : WeatherFX.cpp
//
// PURPOSE : Weather special FX - Implementation
//
// CREATED : 3/23/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeatherFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ParticleSystemFX.h"
#include "SFXMgr.h"
#include "GameClientShell.h"
#include "physics_lt.h"
#include "ClientWeaponUtils.h"
#include "ClientButeMgr.h"
#include "GameButes.h"
#include "SurfaceFunctions.h"
#include "ContainerCodes.h"

extern CGameClientShell* g_pGameClientShell;
extern CClientButeMgr* g_pClientButeMgr;

#define BASE_AREA	(1000.0*1000.0)				// Base surface area for particle/line density
#define MAX_SPLASH_VIEW_DIST		2000.0f		// Max distance from camera to create splashes
#define RAIN_MIN_ALPHA_SCALE_DIST	4000.0f		// Distance from camera for rain to be at min alpha
#define RAIN_FULL_ALPHA_SCALE_DIST	2000.0f		// Distance from camera for rain to be normal alpha
#define	RAIN_MIN_ALPHA				0.4f
#define	RAIN_MAX_ALPHA				0.9f


static void HandleRemoveLine(void *pUserData, LSLineStruct* pLine); 

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeatherFX::Init
//
//	PURPOSE:	Init the weather fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeatherFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	WFXCREATESTRUCT weather;

	weather.hServerObj	= hServObj;
	weather.bFogEnable	= (LTBOOL)hRead->ReadByte();
	weather.fMinFogFarZ	= hRead->ReadFloat();
	weather.fMinFogNearZ= hRead->ReadFloat();
	weather.fFogFarZ	= hRead->ReadFloat();
	weather.fFogNearZ	= hRead->ReadFloat();
	weather.vFogColor = hRead->ReadVector();
	weather.vTintColor = hRead->ReadVector();
	weather.vLightAdd = hRead->ReadVector();

	weather.fViscosity = hRead->ReadFloat();
	weather.fFriction = hRead->ReadFloat();
	weather.vCurrent = hRead->ReadVector();

	weather.dwFlags		= hRead->ReadDWord();
	weather.fViewDist	= hRead->ReadFloat();

	return Init(&weather);
}

LTBOOL CWeatherFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CVolumeBrushFX::Init(psfxCreateStruct) || !g_pClientButeMgr) return LTFALSE;

	WFXCREATESTRUCT* pWFX = (WFXCREATESTRUCT*)psfxCreateStruct;

	m_dwFlags	= pWFX->dwFlags;
	m_fViewDist	= pWFX->fViewDist;

	m_vRainVel.y = g_pClientButeMgr->GetWeatherAttributeFloat(WEATHER_BUTE_RAINVEL);
	m_vSnowVel.y = g_pClientButeMgr->GetWeatherAttributeFloat(WEATHER_BUTE_SNOWVEL);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeatherFX::CreateObject
//
//	PURPOSE:	Create object(s) associated with weather fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeatherFX::CreateObject(CClientDE *pClientDE)
{
	if (!CVolumeBrushFX::CreateObject(pClientDE) || !g_pClientButeMgr) return LTFALSE;

	// Calculate the bottom of the brush.  This is where all "splash"
	// effects will be created...(yes this assumes the ground is flat).

	PhysicsLT* pPhysics = g_pLTClient->Physics();

	g_pLTClient->GetObjectPos(m_hServerObject, &m_vPos);
	pPhysics->GetObjectDims(m_hServerObject, &m_vDims);

	m_fFloorY = (m_vPos.y - m_vDims.y) + 1.0f;
	m_fArea = (m_vDims.x*2.0f)*(m_vDims.z*2.0f);

	
	// Create the fx...

	if (m_dwFlags & WFLAG_SNOW)
	{
		if (!CreateSnow()) return LTFALSE;
	}

	if (m_dwFlags & WFLAG_RAIN)
	{
		if (!CreateRain()) return LTFALSE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeatherFX::CreateSnow
//
//	PURPOSE:	Create snow
//
// ----------------------------------------------------------------------- //

LTBOOL CWeatherFX::CreateSnow()
{
	// Create snow particle system(s)...

	if (!g_pLTClient || !m_hServerObject) return LTFALSE;

	LTVector vColor1(255, 255, 255), vColor2(255, 255, 255);
	
	PhysicsLT* pPhysics = g_pLTClient->Physics();

	// Calculate how long the particles should stay alive...

	double fVelY = double(m_vSnowVel.y);
	LTFLOAT fLifetime = fabs(fVelY) > 0.01 ? (LTFLOAT) (m_vDims.y*2.0f / fabs(fVelY)) : 0.0f;

	char* pBute = WEATHER_BUTE_SNOWLIGHT;
	if (m_dwFlags & WFLAG_NORMAL_SNOW)
	{
		pBute = WEATHER_BUTE_SNOWNORMAL;
	}
	else if (m_dwFlags & WFLAG_HEAVY_SNOW)
	{
		pBute = WEATHER_BUTE_SNOWHEAVY;
	}

	LTFLOAT fFlakesPerSec = g_pClientButeMgr->GetWeatherAttributeFloat(pBute);

	// Adjust the number of flakes based on our area...

	fFlakesPerSec *= (LTFLOAT)(m_fArea / BASE_AREA);

	// Create all particles at the top of the brush...

	LTVector vPos, vDims;
	vDims = m_vDims;
	vPos  = m_vPos;

	vPos.y += vDims.y;
	vDims.y = 0.0f;

	CString str = g_pClientButeMgr->GetWeatherAttributeString(WEATHER_BUTE_SNOWPARTICLE);
	if (str.GetLength() < 1) return LTFALSE;

	PSCREATESTRUCT ps;

	ps.fParticlesPerSecond  = fFlakesPerSec;
	ps.fParticleRadius		= g_pClientButeMgr->GetWeatherAttributeFloat(WEATHER_BUTE_SNOWPARTICLERAD);
	ps.hServerObj			= m_hServerObject;
	ps.vPos					= vPos;
	ps.vColor1				= vColor1;
	ps.vColor2				= vColor2;
	ps.vMinVel				= m_vSnowVel;
	ps.vMaxVel				= m_vSnowVel;
	ps.dwFlags				= 0;
	ps.fBurstWait			= 0.0f;
	ps.vDims				= vDims;
	ps.fParticleLifetime	= fLifetime;
	ps.fGravity				= 0.0f;
	ps.fRotationVelocity	= 0.0f;
	ps.hstrTextureName		= g_pLTClient->CreateString((char *)(LPCSTR)str);
	ps.fViewDist			= m_fViewDist;

	if (!m_Snow.Init(&ps) || !m_Snow.CreateObject(g_pLTClient))
	{
		return LTFALSE;
	}

	m_Snow.Update();

	LTFLOAT r, g, b, a;
	g_pLTClient->GetObjectColor(m_Snow.GetObject(), &r, &g, &b, &a);
	g_pLTClient->SetObjectColor(m_Snow.GetObject(), r, g, b, 0.9f);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeatherFX::CreateRain
//
//	PURPOSE:	Create rain
//
// ----------------------------------------------------------------------- //

LTBOOL CWeatherFX::CreateRain()
{
	// Create rain line system(s)...

	if (!g_pLTClient || !m_hServerObject) return LTFALSE;

	LTVector vStartColor(0.375f, 0.375f, 0.375f), vEndColor(0.375f, 0.375f, 0.375f);
	
	PhysicsLT* pPhysics = g_pLTClient->Physics();


	// Calculate how long the lines should stay alive...

	double fVelY = double(m_vRainVel.y);
	LTFLOAT fLifetime = fabs(fVelY) > 0.01 ? (LTFLOAT) (m_vDims.y*2.0f / fabs(fVelY)) : 0.0f;


	char* pDropsBute = WEATHER_BUTE_RAINLIGHT;

	if (m_dwFlags & WFLAG_NORMAL_RAIN)
	{
		pDropsBute = WEATHER_BUTE_RAINNORMAL;
	}
	else if (m_dwFlags & WFLAG_HEAVY_RAIN)
	{
		pDropsBute = WEATHER_BUTE_RAINHEAVY;
	}

	LTFLOAT fDropsPerSec = g_pClientButeMgr->GetWeatherAttributeFloat(pDropsBute);


	// Adjust the number of drops based on our area...

	fDropsPerSec *= (LTFLOAT)(m_fArea / BASE_AREA);


	// Create all lines at the top of the brush...

	LTVector vDims = m_vDims;
	m_vRainPos = m_vPos;

	m_vRainPos.y += vDims.y;
	vDims.y = 0.0f;

	LSCREATESTRUCT ls;

	ls.hServerObj		= m_hServerObject;
	ls.vPos				= m_vRainPos;
	ls.vStartColor		= vStartColor;
	ls.vEndColor		= vEndColor;
	ls.vMinVel			= m_vRainVel;
	ls.vMaxVel			= m_vRainVel;
	ls.fStartAlpha		= 0.2f;
	ls.fEndAlpha		= 0.75f;
	ls.fBurstWait		= 0.001f;
	ls.fBurstWaitMin	= 1.0f;
	ls.fBurstWaitMax	= 1.0f;
	ls.fLinesPerSecond	= fDropsPerSec;
	ls.fLineLength		= 150.0f;
	ls.vDims			= vDims;
	ls.fLineLifetime	= fLifetime;
	ls.fViewDist		= m_fViewDist;

	if (!m_Rain.Init(&ls) || !m_Rain.CreateObject(g_pLTClient))
	{
		return LTFALSE;
	}

	m_Rain.Update();
	m_Rain.SetRemoveLineFn(HandleRemoveLine, (void*)this);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeatherFX::CreateSplashSprites
//
//	PURPOSE:	Create the sprites used when the rain impacts
//
// ----------------------------------------------------------------------- //

void CWeatherFX::CreateSplashSprites()
{
	if (m_eSurfaceType == ST_UNKNOWN) return;

	// Setup the splash sprite...

	BSCREATESTRUCT sc;

	CString str;
	LTFLOAT fLifetime = 0.0;
	uint32 dwFlags	 = FLAG_NOLIGHT;
	char* pFilename  = DNULL;

	if (m_eSurfaceType == ST_LIQUID)
	{
		LTVector vUp(0, 1, 0);
		g_pLTClient->AlignRotation(&(sc.rRot), &vUp, DNULL);
	
		fLifetime = 1.0f;
		dwFlags |= FLAG_ROTATEABLESPRITE;

		str = g_pClientButeMgr->GetWeatherAttributeString(WEATHER_BUTE_RAINRING);
		if (str.GetLength() < 1) return;
		
		pFilename = (char *)(LPCSTR)str;
	}
	else  // Not a liquid
	{
		fLifetime = 0.05f;
		dwFlags |= FLAG_SPRITEBIAS;

		str = g_pClientButeMgr->GetWeatherAttributeString(WEATHER_BUTE_RAINSPLASH);
		if (str.GetLength() < 1) return;
		
		pFilename = (char *)(LPCSTR)str;
	}
	
	VEC_COPY(sc.vPos, m_vPos);

	sc.dwFlags			= dwFlags;
	sc.fLifeTime		= fLifetime;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.Filename			= pFilename;
	sc.nType			= OT_SPRITE;	

	LTFLOAT fStartScale, fEndScale;

	for (int i=0; i < NUM_SPLASH_SPRITES; i++)
	{
		if (m_eSurfaceType == ST_LIQUID)
		{
			fStartScale = GetRandom(0.05f, 0.1f);
			fEndScale   = GetRandom(0.2f, 0.3f);
		}
		else
		{
			fStartScale = GetRandom(0.1f, 0.30f);
			fEndScale   = GetRandom(0.2f, 0.35f);
		}

		VEC_SET(sc.vInitialScale, fStartScale, fStartScale, 1.0f);
		VEC_SET(sc.vFinalScale, fEndScale, fEndScale, 1.0f);

		if (!m_Splash[i].Init(&sc) || !m_Splash[i].CreateObject(g_pLTClient))
		{
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	HandleRemoveLine
//
//	PURPOSE:	Handle a rain 'drop' being removed
//
// ----------------------------------------------------------------------- //

void HandleRemoveLine(void *pUserData, LSLineStruct* pLine)
{
	if (!pUserData || !pLine) return;

	// Depending on the detail settings, create more or less splashes...

	//static int s_nNum = 0;
	//if (++s_nNum % 2 == 0)
	//{
		CWeatherFX* pWeather = (CWeatherFX*)pUserData;
		pWeather->DoSplash(pLine);
	//}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeatherFX::DoSplash
//
//	PURPOSE:	Handle a rain 'drop' splash
//
// ----------------------------------------------------------------------- //

void CWeatherFX::DoSplash(LSLineStruct* pLine)
{
	if (!pLine || !pLine->hDELine) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr || !g_pLTClient || m_eSurfaceType == ST_UNKNOWN) return;


	// Get the splash position...

	DELine line;
	g_pLTClient->GetLineInfo(pLine->hDELine, &line);
	LTVector vPos = m_vRainPos + ((line.m_Points[0].m_Pos + line.m_Points[1].m_Pos) / 2.0f);
	vPos.y = m_fFloorY;


	// Get the camera's position...If the camera is too far away from
	// the sprite being added, don't add it :)

	HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
	if (!hCamera) return;

	LTVector vCamPos, vDist;
	g_pLTClient->GetObjectPos(hCamera, &vCamPos);
	vDist = vCamPos - vPos;

	if (vDist.MagSqr() > (MAX_SPLASH_VIEW_DIST*MAX_SPLASH_VIEW_DIST))
	{
		return;
	}


	// Show the sprite by moving one of the splash sprites to this position
	// and showing it...

	HOBJECT hObj = DNULL;

	for (int i=0; i < NUM_SPLASH_SPRITES; i++)
	{
		hObj = m_Splash[i].GetObject();

		if (hObj)
		{
			uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);
			if (!(dwFlags & FLAG_VISIBLE))
			{
				g_pLTClient->SetObjectPos(hObj, &vPos);
				g_pLTClient->SetObjectFlags(hObj, dwFlags | FLAG_VISIBLE);
				m_Splash[i].Reset();
				break;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeatherFX::Update
//
//	PURPOSE:	Update the weather fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeatherFX::Update()
{
	// Make sure we're supposed to be here...

	if (IsWaitingForRemove() || !m_hServerObject) return LTFALSE;


	// Determine what type of surface this brush is on top of.

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = LTFALSE;

		ClientIntersectQuery iQuery;
		ClientIntersectInfo  iInfo;
		memset(&iInfo, 0, sizeof(iInfo));

		iQuery.m_Flags	 = INTERSECT_OBJECTS;
		iQuery.m_From    = m_vPos;

		iQuery.m_To   = m_vPos;
		iQuery.m_To.y = m_fFloorY - 100;

		if (g_pLTClient->IntersectSegment(&iQuery, &iInfo))
		{		
			m_eSurfaceType = GetSurfaceType(iInfo);
		
			// Create splash sprites if necessary...

			if (m_dwFlags & WFLAG_RAIN)
			{
				CreateSplashSprites();
			}
		}
	}
	
	
	// Update Snow...

	if (m_dwFlags & WFLAG_SNOW)
	{
		m_Snow.Update();
	}
	
	
	// Update Rain...

	if (m_dwFlags & WFLAG_RAIN)
	{
		m_Rain.Update();

		// Get the camera's position...Make rain systems far away from the
		// camera more transparent...
/*
		HOBJECT hCamera = g_pGameClientShell->GetPlayerCameraObject();
		if (!hCamera) return LTFALSE;

		LTVector vCamPos, vDist, vPos;
		g_pLTClient->GetObjectPos(m_Rain.GetObject(), &vPos);
		g_pLTClient->GetObjectPos(hCamera, &vCamPos);
		vCamPos.y = vPos.y;  // Only wory about X and Z
		vDist = vCamPos - vPos;

		LTFLOAT fDistSqr			= vDist.MagSqr();
		LTFLOAT fFullAlphaSqr	= (RAIN_FULL_ALPHA_SCALE_DIST*RAIN_FULL_ALPHA_SCALE_DIST);
		LTFLOAT fMinAlphaSqr		= (RAIN_MIN_ALPHA_SCALE_DIST*RAIN_MIN_ALPHA_SCALE_DIST);

		LTFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(m_Rain.GetObject(), &r, &g, &b, &a);
		
		if (fDistSqr <= fFullAlphaSqr)
		{
			a = RAIN_MAX_ALPHA;  // Full alpha
		}
		else  // Calculate new alpha
		{
			LTFLOAT fDistOffset	= fDistSqr - fFullAlphaSqr;
			LTFLOAT fTotalDist	= fMinAlphaSqr - fFullAlphaSqr;

			a = RAIN_MAX_ALPHA - ((RAIN_MAX_ALPHA - RAIN_MIN_ALPHA) * (fDistOffset / fTotalDist));
			a = a < RAIN_MIN_ALPHA ? RAIN_MIN_ALPHA : a;
		}

		g_pLTClient->SetObjectColor(m_Rain.GetObject(), r, g, b, a);
*/
		// Update Splash fx...

		HOBJECT hObj = DNULL;

		for (int i=0; i < NUM_SPLASH_SPRITES; i++)
		{
			hObj = m_Splash[i].GetObject();
			
			if (hObj)
			{
				uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);
				
				if (dwFlags & FLAG_VISIBLE)
				{
					if (!m_Splash[i].Update())
					{
						// If the sprite is done, hide it...

						g_pLTClient->SetObjectFlags(hObj, dwFlags & ~FLAG_VISIBLE);
					}
				}
			}
		}
	}

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_WeatherFactory
//-------------------------------------------------------------------------------------------------

const SFX_WeatherFactory SFX_WeatherFactory::registerThis;

CSpecialFX* SFX_WeatherFactory::MakeShape() const
{
	return new CWeatherFX();
}

