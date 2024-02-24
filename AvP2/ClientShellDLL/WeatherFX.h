// ----------------------------------------------------------------------- //
//
// MODULE  : WeatherFX.h
//
// PURPOSE : Weather special fx class - Definition
//
// CREATED : 3/23/99
//
// ----------------------------------------------------------------------- //

#ifndef __WEATHER_FX_H__
#define __WEATHER_FX_H__

#include "VolumeBrushFX.h"
#include "LineSystemFX.h"
#include "ParticleSystemFX.h"
#include "SurfaceMgr.h"
#include "BaseScaleFX.h"

#define NUM_SPLASH_SPRITES		100


struct WFXCREATESTRUCT : public VBCREATESTRUCT
{
	WFXCREATESTRUCT::WFXCREATESTRUCT();

	uint32	dwFlags;
	LTFLOAT	fViewDist;
};

inline WFXCREATESTRUCT::WFXCREATESTRUCT()
{
	dwFlags		= 0;
	fViewDist	= 0.0f;
}


class CWeatherFX : public CVolumeBrushFX
{
	public :

		CWeatherFX() : CVolumeBrushFX() 
		{
			m_bFirstUpdate	= LTTRUE;
			m_fArea			= 1.0;
			m_dwFlags		= 0;
			m_fFloorY		= 0.0f;
			m_eSurfaceType	= ST_UNKNOWN;
			m_fViewDist		= 1000.0f;

			m_vRainVel.Init();
			m_vSnowVel.Init();
			m_vRainPos.Init();
			m_vPos.Init();
			m_vDims.Init();
		}

		virtual LTBOOL Update();
		virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
		virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual LTBOOL CreateObject(CClientDE* pClientDE);

		void	DoSplash(LSLineStruct* pLine); 

	protected :

		LTBOOL		m_bFirstUpdate;
		uint32		m_dwFlags;
		LTFLOAT		m_fFloorY;
		LTFLOAT		m_fViewDist;
		double		m_fArea;

		SurfaceType	m_eSurfaceType;

		LTVector		m_vRainVel;
		LTVector		m_vSnowVel;
		LTVector		m_vRainPos;
		LTVector		m_vPos;
		LTVector		m_vDims;

		CLineSystemFX		m_Rain;
		CParticleSystemFX	m_Snow;

		CBaseScaleFX	m_Splash[NUM_SPLASH_SPRITES];

		LTBOOL	CreateSnow();
		LTBOOL	CreateRain();
		void	CreateSplashSprites();
};

//-------------------------------------------------------------------------------------------------
// SFX_WeatherFactory
//-------------------------------------------------------------------------------------------------
#ifndef C_SPECIAL_FX_FACTORY_H
#include "CSpecialFXFactory.h"
#endif

#ifndef __SFX_MSG_IDS_H__
#include "SFXMsgIds.h"
#endif

class SFX_WeatherFactory : public CSpecialFXFactory
{
	SFX_WeatherFactory() : CSpecialFXFactory(SFX_WEATHER_ID) {;}
	static const SFX_WeatherFactory registerThis;

	virtual CSpecialFX* MakeShape() const;
};

#endif // __WEATHER_FX_H__