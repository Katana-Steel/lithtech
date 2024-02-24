// ----------------------------------------------------------------------- //
//
// MODULE  : FogVolumeFX.cpp
//
// PURPOSE : Volumetric fog controller
//
// CREATED : 8/27/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "fogvolumefx.h"
#include "clientservershared.h"
#include "vartrack.h"

// ----------------------------------------------------------------------- //

static VarTrack		g_vtFogVolDimsRamp;
static VarTrack		g_vtFogVolColorRamp;
static VarTrack		g_vtFogVolDensityRamp;

// ----------------------------------------------------------------------- //

LTBOOL CFogVolumeFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	// Make sure everything is okie dokie
	if(!CSpecialFX::Init(hServObj, hRead)) return LTFALSE;
	if(!hRead) return LTFALSE;

	FOGVOLCREATESTRUCT fogVol;

	// Setup the FX create structure
	fogVol.hServerObj		= hServObj;

	g_pLTClient->ReadFromMessageVector(hRead, &fogVol.vDims);
	g_pLTClient->ReadFromMessageVector(hRead, &fogVol.vColor);

	fogVol.fDensity			= g_pLTClient->ReadFromMessageFloat(hRead);
	fogVol.fMinOpacity		= g_pLTClient->ReadFromMessageFloat(hRead);
	fogVol.fMaxOpacity		= g_pLTClient->ReadFromMessageFloat(hRead);

	fogVol.nRes				= g_pLTClient->ReadFromMessageByte(hRead);
	fogVol.nShape			= g_pLTClient->ReadFromMessageByte(hRead);
	fogVol.nGradient		= g_pLTClient->ReadFromMessageByte(hRead);
	fogVol.nDensityFn		= g_pLTClient->ReadFromMessageByte(hRead);

	fogVol.bFogWorld		= (LTBOOL)g_pLTClient->ReadFromMessageByte(hRead);
	fogVol.bFogModels		= (LTBOOL)g_pLTClient->ReadFromMessageByte(hRead);

	// Return the value from the structured init
	return Init(&fogVol);
}

// ----------------------------------------------------------------------- //

LTBOOL CFogVolumeFX::Init(SFXCREATESTRUCT *psfx)
{
	// Do the base FX initialization (this should fill in the ServerObj variable for us)
	if(!CSpecialFX::Init(psfx)) return LTFALSE;

	// Cast it to a point that we want
	FOGVOLCREATESTRUCT *pFX = (FOGVOLCREATESTRUCT*)psfx;
	m_Fog = *pFX;

	m_vDims = m_Fog.vDims;
	m_vColor = m_Fog.vColor;
	m_fDensity = m_Fog.fDensity;

	m_vLastPos.Init(0.0f, 0.0f, 0.0f);
	m_dwLastUserFlags = 0;

	m_bNeedsUpdates = LTTRUE;

	// Make sure the variable trackers are initted
	if(!g_vtFogVolDimsRamp.IsInitted())
		g_vtFogVolDimsRamp.Init(g_pLTClient, "FogVolDimsRamp", LTNULL, 10.0f);
	if(!g_vtFogVolColorRamp.IsInitted())
		g_vtFogVolColorRamp.Init(g_pLTClient, "FogVolColorRamp", LTNULL, 0.5f);
	if(!g_vtFogVolDensityRamp.IsInitted())
		g_vtFogVolDensityRamp.Init(g_pLTClient, "FogVolDensityRamp", LTNULL, 0.5f);


	char buf[32];

	// Setup the initial values of the fog parameter so we don't do
	// any ramping on the initial creation
	sprintf(buf, "vfp_density %f", m_fDensity);
	g_pLTClient->RunConsoleString(buf);

	sprintf(buf, "vfp_dimsx %f", m_vDims.x);
	g_pLTClient->RunConsoleString(buf);
	sprintf(buf, "vfp_dimsy %f", m_vDims.y);
	g_pLTClient->RunConsoleString(buf);
	sprintf(buf, "vfp_dimsz %f", m_vDims.z);
	g_pLTClient->RunConsoleString(buf);

	sprintf(buf, "vfp_colorr %f", m_vColor.x);
	g_pLTClient->RunConsoleString(buf);
	sprintf(buf, "vfp_colorg %f", m_vColor.y);
	g_pLTClient->RunConsoleString(buf);
	sprintf(buf, "vfp_colorb %f", m_vColor.z);
	g_pLTClient->RunConsoleString(buf);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CFogVolumeFX::Update()
{
	// If the server object is invalid, just return
	if(!m_hServerObject)
	{
		g_pLTClient->RunConsoleString("vfp_fog 0");
		return LTFALSE;
	}

	// Get the user flags of the server object
	uint32 dwUserFlags = 0;
	g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

	// Make sure we're supposed to be rendering this fog
	if(!(dwUserFlags & USRFLG_VISIBLE))
	{
		g_pLTClient->RunConsoleString("vfp_fog 0");
		g_pLTClient->RunConsoleString("vfp_fogdebug 0");
	}
	else if(!(m_dwLastUserFlags & USRFLG_VISIBLE))
	{
		g_pLTClient->RunConsoleString("vfp_fog 1");
		g_pLTClient->RunConsoleString("vfp_fogdebug 1");
		m_bNeedsUpdates = LTTRUE;
	}

	m_dwLastUserFlags = dwUserFlags;


	char buf[32];

	// If the position of the server object has changed... update our
	// fog volume location
	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	if(vPos != m_vLastPos)
	{
		sprintf(buf, "vfp_posx %f", vPos.x);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_posy %f", vPos.y);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_posz %f", vPos.z);
		g_pLTClient->RunConsoleString(buf);

		m_vLastPos = vPos;
	}


	// Get the time between frames
	LTFLOAT fFrameTime = g_pLTClient->GetFrameTime();


	// If the dims are different... make sure we fit to the right size
	if(!m_vDims.Equals(m_Fog.vDims))
	{
		// If we need to go larger... do so
		if(m_vDims.x < m_Fog.vDims.x)
		{
			m_vDims.x += fFrameTime * g_vtFogVolDimsRamp.GetFloat();

			if(m_vDims.x > m_Fog.vDims.x)
				m_vDims.x = m_Fog.vDims.x;
		}
		// If we need to go smaller... do that instead
		else
		{
			m_vDims.x -= fFrameTime * g_vtFogVolDimsRamp.GetFloat();

			if(m_vDims.x < m_Fog.vDims.x)
				m_vDims.x = m_Fog.vDims.x;
		}

		// If we need to go larger... do so
		if(m_vDims.y < m_Fog.vDims.y)
		{
			m_vDims.y += fFrameTime * g_vtFogVolDimsRamp.GetFloat();

			if(m_vDims.y > m_Fog.vDims.y)
				m_vDims.y = m_Fog.vDims.y;
		}
		// If we need to go smaller... do that instead
		else
		{
			m_vDims.y -= fFrameTime * g_vtFogVolDimsRamp.GetFloat();

			if(m_vDims.y < m_Fog.vDims.y)
				m_vDims.y = m_Fog.vDims.y;
		}

		// If we need to go larger... do so
		if(m_vDims.z < m_Fog.vDims.z)
		{
			m_vDims.z += fFrameTime * g_vtFogVolDimsRamp.GetFloat();

			if(m_vDims.z > m_Fog.vDims.z)
				m_vDims.z = m_Fog.vDims.z;
		}
		// If we need to go smaller... do that instead
		else
		{
			m_vDims.z -= fFrameTime * g_vtFogVolDimsRamp.GetFloat();

			if(m_vDims.z < m_Fog.vDims.z)
				m_vDims.z = m_Fog.vDims.z;
		}

		sprintf(buf, "vfp_dimsx %f", m_vDims.x);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_dimsy %f", m_vDims.y);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_dimsz %f", m_vDims.z);
		g_pLTClient->RunConsoleString(buf);
	}



	// If the dims are different... make sure we fit to the right size
	if(!m_vColor.Equals(m_Fog.vColor))
	{
		// If we need to go larger... do so
		if(m_vColor.x < m_Fog.vColor.x)
		{
			m_vColor.x += fFrameTime * g_vtFogVolColorRamp.GetFloat();

			if(m_vColor.x > m_Fog.vColor.x)
				m_vColor.x = m_Fog.vColor.x;
		}
		// If we need to go smaller... do that instead
		else
		{
			m_vColor.x -= fFrameTime * g_vtFogVolColorRamp.GetFloat();

			if(m_vColor.x < m_Fog.vColor.x)
				m_vColor.x = m_Fog.vColor.x;
		}

		// If we need to go larger... do so
		if(m_vColor.y < m_Fog.vColor.y)
		{
			m_vColor.y += fFrameTime * g_vtFogVolColorRamp.GetFloat();

			if(m_vColor.y > m_Fog.vColor.y)
				m_vColor.y = m_Fog.vColor.y;
		}
		// If we need to go smaller... do that instead
		else
		{
			m_vColor.y -= fFrameTime * g_vtFogVolColorRamp.GetFloat();

			if(m_vColor.y < m_Fog.vColor.y)
				m_vColor.y = m_Fog.vColor.y;
		}

		// If we need to go larger... do so
		if(m_vColor.z < m_Fog.vColor.z)
		{
			m_vColor.z += fFrameTime * g_vtFogVolColorRamp.GetFloat();

			if(m_vColor.z > m_Fog.vColor.z)
				m_vColor.z = m_Fog.vColor.z;
		}
		// If we need to go smaller... do that instead
		else
		{
			m_vColor.z -= fFrameTime * g_vtFogVolColorRamp.GetFloat();

			if(m_vColor.z < m_Fog.vColor.z)
				m_vColor.z = m_Fog.vColor.z;
		}

		sprintf(buf, "vfp_colorr %f", m_vColor.x);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_colorg %f", m_vColor.y);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_colorb %f", m_vColor.z);
		g_pLTClient->RunConsoleString(buf);
	}



	// If the density is different... make sure to move towards it
	if(m_fDensity != m_Fog.fDensity)
	{
		// If we need to go larger... do so
		if(m_fDensity < m_Fog.fDensity)
		{
			m_fDensity += fFrameTime * g_vtFogVolDensityRamp.GetFloat();

			if(m_fDensity > m_Fog.fDensity)
				m_fDensity = m_Fog.fDensity;
		}
		// If we need to go smaller... do that instead
		else
		{
			m_fDensity -= fFrameTime * g_vtFogVolDensityRamp.GetFloat();

			if(m_fDensity < m_Fog.fDensity)
				m_fDensity = m_Fog.fDensity;
		}

		sprintf(buf, "vfp_density %f", m_fDensity);
		g_pLTClient->RunConsoleString(buf);
	}


	// If the fog needs updates... go through and set all the console variables again
	if(m_bNeedsUpdates)
	{
		sprintf(buf, "vfp_minopacity %f", m_Fog.fMinOpacity);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_maxopacity %f", m_Fog.fMaxOpacity);
		g_pLTClient->RunConsoleString(buf);

		sprintf(buf, "vfp_mapres %d", m_Fog.nRes);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_shape %d", m_Fog.nShape);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_gradient %d", m_Fog.nGradient);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_densityfn %d", m_Fog.nDensityFn);
		g_pLTClient->RunConsoleString(buf);

		sprintf(buf, "vfp_fogworld %d", (int)m_Fog.bFogWorld);
		g_pLTClient->RunConsoleString(buf);
		sprintf(buf, "vfp_fogmodels %d", (int)m_Fog.bFogModels);
		g_pLTClient->RunConsoleString(buf);

		// Make sure we only go into these settings when we need to
		m_bNeedsUpdates = LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

LTBOOL CFogVolumeFX::OnServerMessage(HMESSAGEREAD hRead)
{
	if(!hRead) return LTFALSE;

	g_pLTClient->ReadFromMessageVector(hRead, &m_Fog.vDims);
	g_pLTClient->ReadFromMessageVector(hRead, &m_Fog.vColor);

	m_Fog.fDensity			= g_pLTClient->ReadFromMessageFloat(hRead);
	m_Fog.fMinOpacity		= g_pLTClient->ReadFromMessageFloat(hRead);
	m_Fog.fMaxOpacity		= g_pLTClient->ReadFromMessageFloat(hRead);

	m_Fog.nRes				= g_pLTClient->ReadFromMessageByte(hRead);
	m_Fog.nShape			= g_pLTClient->ReadFromMessageByte(hRead);
	m_Fog.nGradient			= g_pLTClient->ReadFromMessageByte(hRead);
	m_Fog.nDensityFn		= g_pLTClient->ReadFromMessageByte(hRead);

	m_Fog.bFogWorld			= (LTBOOL)g_pLTClient->ReadFromMessageByte(hRead);
	m_Fog.bFogModels		= (LTBOOL)g_pLTClient->ReadFromMessageByte(hRead);

	m_bNeedsUpdates			= LTTRUE;

	return LTTRUE;
}

//-------------------------------------------------------------------------------------------------
// SFX_FogVolumeFactory
//-------------------------------------------------------------------------------------------------

const SFX_FogVolumeFactory SFX_FogVolumeFactory::registerThis;

CSpecialFX* SFX_FogVolumeFactory::MakeShape() const
{
	return new CFogVolumeFX();
}

