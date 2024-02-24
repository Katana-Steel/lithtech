// ----------------------------------------------------------------------- //
//
// MODULE  : StarLightViewFX.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 02/04/98
//			  7/17/98 - Converted to client SFX.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "StarLightViewFX.h"
#include "ClientServerShared.h"
#include "CommandIDs.h"
#include "VarTrack.h"

#ifndef __GAME_CLIENT_SHELL_H__
#include "GameClientShell.h"
#endif

#ifndef VIEW_ORDER_ODDS_H
#include "ViewOrderOdds.h"
#endif

bool			StarLightViewFX::m_GlobalLight = true;			// Are we all in global light mode?
std::string		StarLightViewFX::m_Mode = "None";				// What mode are we in?

// -------------------------------------------------------------------- //

StarLightViewFX::StarLightViewFX() : CSpecialFX(), m_vColor(0,0,0), m_fStartTime(0.0f),
	m_fLifeTime(-1.0f), m_bUseServerPos(DFALSE), m_hLightAnim(INVALID_LIGHT_ANIM),
	m_vFogColor(0,0,0), m_MinFogNearZ(0), m_MinFogFarZ(5000),  m_FogNearZ(0), m_FogFarZ(5000), 
	m_ModeOn(false), m_bEnvWorld(LTTRUE), m_bEnvObj(LTTRUE)
{

}

StarLightViewFX::~StarLightViewFX()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Init()
//
//	PURPOSE:	Initialize data members
//
// ----------------------------------------------------------------------- //

// Stupid Function to make up for deficiencies in the global light class
LTVector GetGlobalLight()
{
	LTVector stupid;
	g_pClientDE->GetGlobalLightScale(&stupid);
	return stupid;
}

DBOOL StarLightViewFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead)
{
	STARLIGHTCREATESTRUCT light;

	light.hServerObj	= hServObj;

	// The uglyness of passing strings around!
	HSTRING hStr = hRead->ReadHString();
	light.m_ActiveMode	= std::string(g_pClientDE->GetStringData(hStr));
	g_pClientDE->FreeString(hStr);

	light.vColor		= hRead->ReadVector();
	light.m_vFogColor	= hRead->ReadVector();
	light.m_MinFogNearZ	= hRead->ReadDWord();
	light.m_MinFogFarZ	= hRead->ReadDWord();
	light.m_FogNearZ	= hRead->ReadDWord();
	light.m_FogFarZ		= hRead->ReadDWord();
	light.dwLightFlags	= hRead->ReadDWord();
	light.m_hLightAnim	= static_cast<HLIGHTANIM>(hRead->ReadDWord());
	light.m_vVertexTintColor = hRead->ReadVector();
	light.m_vModelAddColor = hRead->ReadVector();
	light.m_vAmbientColor = hRead->ReadVector();

	m_OldLight = GetGlobalLight();

	return Init(&light);
}

DBOOL StarLightViewFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) 
		return DFALSE;

	STARLIGHTCREATESTRUCT* pLight = (STARLIGHTCREATESTRUCT*)psfxCreateStruct;

	m_ActiveMode = pLight->m_ActiveMode;
	VEC_COPY(m_vColor, pLight->vColor);

	VEC_COPY(m_vFogColor, pLight->m_vFogColor);
	m_MinFogNearZ	= pLight->m_MinFogNearZ;
	m_MinFogFarZ	= pLight->m_MinFogFarZ;
	m_FogNearZ		= pLight->m_FogNearZ;
	m_FogFarZ		= pLight->m_FogFarZ;

	VEC_COPY(m_vVertexTintColor, pLight->m_vVertexTintColor);
	VEC_COPY(m_vModelAddColor, pLight->m_vModelAddColor);
	VEC_COPY(m_vAmbientColor, pLight->m_vAmbientColor);

	m_dwLightFlags	= pLight->dwLightFlags;
	m_hLightAnim	= pLight->m_hLightAnim;

	m_OldLight = GetGlobalLight();

	// store the default env map stuff
	m_bEnvWorld = (GetConsoleInt("EnvMapWorld", 0) == 1);
	m_bEnvObj = (GetConsoleInt("EnvMapEnable", 0) == 1);

	return DTRUE;
}

void StarLightViewFX::ResetEnvMap()
{
	// store the default env map stuff
	m_bEnvWorld = (GetConsoleInt("EnvMapWorld", 0) == 1);
	m_bEnvObj = (GetConsoleInt("EnvMapEnable", 0) == 1);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightViewFX::CreateObject
//
//	PURPOSE:	Create object associated with the light
//
// ----------------------------------------------------------------------- //

DBOOL StarLightViewFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	LAInfo info;
	m_pClientDE->GetLightAnimLT()->GetLightAnimInfo(m_hLightAnim, info);
	// These need to be played with I think
	info.m_iFrames[0] = info.m_iFrames[1] = LIGHTANIMFRAME_NONE;
	m_pClientDE->GetLightAnimLT()->SetLightAnimInfo(m_hLightAnim, info);
	
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightViewFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

DBOOL StarLightViewFX::Update()
{
	return DTRUE;
}

void StarLightViewFX::SetMode(const std::string& mode)
{
	m_Mode = mode;

	if (!m_pClientDE)
	{
		return;
	}

	CameraMgr *pCameraMgr = g_pGameClientShell->GetCameraMgr();
	HCAMERA hCamera = g_pGameClientShell->GetPlayerCamera();

	// This will be in the object
	if (m_ActiveMode == m_Mode)
	{
		// Enable
		// If ModeOn is used to keep us from doing this every frame.
		// Previous mode must set the world back to global light before we change
		if (!m_ModeOn && m_GlobalLight)
		{
			// Setup LightAnim info..
			LightAnimLT* pLightAnimLT = m_pClientDE->GetLightAnimLT();
			LAInfo info;
			pLightAnimLT->GetLightAnimInfo(m_hLightAnim, info);
	
			g_pClientDE->GetGlobalVertexTint(&m_OldVertexColor);
			g_pClientDE->SetGlobalVertexTint(&m_vVertexTintColor);


			info.m_iFrames[0] = info.m_iFrames[1] = 0;

			char buf[255];

			// turn off vFog since it is not compatible with
			// our normal distance fog.  It will get turned back on
			// when we call ResetGlobalFog()
			sprintf(buf, "vFog 0");
			g_pClientDE->RunConsoleString(buf);

			sprintf(buf, "FogEnable 1");
			g_pClientDE->RunConsoleString(buf);

			// Set the fog color...
			sprintf(buf, "FogR %d", (int)m_vFogColor.x);
			g_pClientDE->RunConsoleString(buf);

			sprintf(buf, "FogG %d", (int)m_vFogColor.y);
			g_pClientDE->RunConsoleString(buf);

			sprintf(buf, "FogB %d", (int)m_vFogColor.z);
			g_pClientDE->RunConsoleString(buf);

			// Now set the void color to match...
			g_pGameClientShell->SetVoidColor(m_vFogColor);


			CGameSettings* pSettings = g_pGameClientShell->GetInterfaceMgr()->GetSettings();
			int nNear = m_FogNearZ;
			int nFar = m_FogFarZ;

			if (pSettings)
			{
				LTFLOAT fScale = 1.0f;//(pSettings->GetFloatVar("FogPerformanceScale") / 100.0f);

				nNear = (int)(m_MinFogNearZ + ((m_FogNearZ - m_MinFogNearZ) * fScale));
				nFar = (int)(m_MinFogFarZ + ((m_FogFarZ - m_MinFogFarZ) * fScale));
			}


			// Set the Z values...
			sprintf(buf, "FogNearZ %d", nNear);
			g_pClientDE->RunConsoleString(buf);

			sprintf(buf, "FogFarZ %d", nFar);
			g_pClientDE->RunConsoleString(buf);

			sprintf(buf, "ModelAdd %d %d %d", (int)m_vModelAddColor.x, (int)m_vModelAddColor.y, (int)m_vModelAddColor.z);
			g_pClientDE->RunConsoleString(buf);

			sprintf(buf, "WMAmbient %d %d %d", (int)m_vAmbientColor.x, (int)m_vAmbientColor.y, (int)m_vAmbientColor.z);
			g_pClientDE->RunConsoleString(buf);

			// Disable env mapping in all modes
			g_pClientDE->RunConsoleString("EnvMapWorld 0");
			g_pClientDE->RunConsoleString("EnvMapEnable 0");

			m_ModeOn = true;
			m_GlobalLight = false;

			pLightAnimLT->SetLightAnimInfo(m_hLightAnim, info);
		}
	}
	else
	{
		// Disable if our mode was the one that was on, and no one else has changed the light
		if (m_ModeOn && !m_GlobalLight)
		{
			// Setup LightAnim info..
			LightAnimLT* pLightAnimLT = m_pClientDE->GetLightAnimLT();
			LAInfo info;
			pLightAnimLT->GetLightAnimInfo(m_hLightAnim, info);
	
			g_pClientDE->SetGlobalVertexTint(&m_OldVertexColor);

			g_pClientDE->RunConsoleString("ModelAdd 0 0 0");
			g_pClientDE->RunConsoleString("WMAmbient 0 0 0");
			info.m_iFrames[0] = info.m_iFrames[1] = LIGHTANIMFRAME_NONE;

			g_pGameClientShell->ResetGlobalFog();

			m_ModeOn = false;
			m_GlobalLight = true;

			// Restore the Env Mapping
			if(m_bEnvWorld) g_pClientDE->RunConsoleString("EnvMapWorld 1");
			if(m_bEnvObj) g_pClientDE->RunConsoleString("EnvMapEnable 1");

			pLightAnimLT->SetLightAnimInfo(m_hLightAnim, info);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	StarLightViewFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void StarLightViewFX::SetColor(DFLOAT fRedValue, DFLOAT fGreenValue, DFLOAT fBlueValue, LAInfo &info)
{
	if (fRedValue > 1.0f)       fRedValue = 1.0f;
    if (fRedValue < 0.0f)       fRedValue = 0.0f;
        
    if (fGreenValue > 1.0f)     fGreenValue = 1.0f;
    if (fGreenValue < 0.0f)     fGreenValue = 0.0f;
        
    if (fBlueValue > 1.0f)      fBlueValue = 1.0f;
    if (fBlueValue < 0.0f)      fBlueValue = 0.0f;

	m_pClientDE->SetLightColor(m_hObject, fRedValue, fGreenValue, fBlueValue);
    
	// Update the LightAnim.
	info.m_vLightColor.Init(fRedValue*255.0f, fGreenValue*255.0f, fBlueValue*255.0f);
}



//-------------------------------------------------------------------------------------------------
// SFX_LightFactory
//-------------------------------------------------------------------------------------------------

const SFX_StarLightViewFactory SFX_StarLightViewFactory::registerThis;

CSpecialFX* SFX_StarLightViewFactory::MakeShape() const
{
	return new StarLightViewFX();
}

