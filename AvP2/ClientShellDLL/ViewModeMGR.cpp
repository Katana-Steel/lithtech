// ----------------------------------------------------------------------- //
//
// MODULE  : ViewModeMGR.cpp
//
// PURPOSE : Controls the objects that contribute to a view. This object
//			 handles the decisions of what modes are available and what 
//			 objects need to do what for that mode.
//
// CREATED : 06/08/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"

#include "msgids.h"
#include "VarTrack.h"
#include "CharacterFuncs.h"


#ifndef VIEW_MODE_MGR_H
#include "ViewModeMGR.h"
#endif

#ifndef TEXTURE_MODE_MGR_H
#include "TextureModeMGR.h"
#endif

#ifndef __COMMAND_IDS_H__
#include "CommandIDs.h"
#endif

#ifndef OVERLAY_MGR_H
#include "OverlayMGR.h"
#endif

#ifndef __GAME_CLIENT_SHELL_H__
#include "GameClientShell.h"
#endif

#ifndef VIEW_ORDER_ODDS_H
#include "ViewOrderOdds.h"
#endif

#ifndef VISION_MODE_BUTE_MGR_H
#include "VisionModeButeMGR.h"
#endif

#ifndef __STARLIGHT_VIEW_FX_H__
#include "StarLightViewFX.h"
#endif

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarNVMinChargeLevel;

ILTMessage & operator<<(ILTMessage & out, const std::string & x)
{
	// Write the size of the string.
	out.WriteWord(x.size());

	// Write each byte of the string.
	for( std::string::const_iterator iter = x.begin();
	     iter != x.end(); ++iter )
	{
		out.WriteByte(*iter);
	}

	// Write the terminating null, just to be safe.
	out.WriteByte(0);
	return out;
}

ILTMessage & operator>>(ILTMessage & in,  std::string & x)
{
	// Remove anything that was in the string.
	x.clear();

	// Read the size of the string, and each byte of the string.
	const uint16 string_size = in.ReadWord();

	x.reserve(string_size);
	for(int i = 0; i < string_size; ++i)
	{
		x += in.ReadByte();
	}

	// Read the terminating null (was not needed, just there for safety).
	in.ReadByte();

	return in;
}


//-----------------------------------------------------------------------------
// Define data local to this class.
//-----------------------------------------------------------------------------
struct ViewModeData
{
	typedef std::vector<StarLightViewFX*>	StarLightList;

	StarLightList							m_StarLightList;
	TextureModeMGR							m_TextureModeMGR;
	OverlayMGR								m_OverlayMGR;
	std::string								m_CurrentModeName;
	std::string								m_InitialModeName;
	std::string								m_CurrentVisionSet;
	LTBOOL									m_bMenuMode;

	ViewModeData();
};

ViewModeData::ViewModeData() :	m_TextureModeMGR(Views::START_MODE), 
								m_CurrentModeName(Views::START_MODE),
								m_InitialModeName(Views::START_MODE),
								m_CurrentVisionSet(Views::START_SET),
								// m_CurrentModeName
								// m_CurrentVisionSet
								m_bMenuMode(LTFALSE)
{
}

//-----------------------------------------------------------------------------
// Local function declarations
//-----------------------------------------------------------------------------
void SetVisionMode(ViewModeData* data, std::string mode);
void UpdateStarLight(ViewModeData* data);
void SetStarLightMode(ViewModeData* data, std::string mode);

//-----------------------------------------------------------------------------
// ViewModeMGR
//-----------------------------------------------------------------------------
ViewModeMGR::ViewModeMGR()
{
	m_pData = new ViewModeData();
	m_LastModeName = "Normal";
	m_hLoopingVMSound = LTNULL;
	m_bCineMode = LTFALSE;
}

ViewModeMGR::~ViewModeMGR()
{
	// Delete the FX members
	for (ViewModeData::StarLightList::iterator iter = m_pData->m_StarLightList.begin(); 
		iter != m_pData->m_StarLightList.end(); ++iter)
	{
		delete (*iter);
	}

	if( m_pData )
	{
		delete m_pData;
		m_pData = NULL;
	}
}

const std::string ViewModeMGR::GetCurrentModeName()
{	
	return m_pData->m_CurrentModeName; 
}

const std::string ViewModeMGR::GetLastModeName()
{	
	return m_LastModeName; 
}

const LTVector ViewModeMGR::GetVertexTint()
{	
	for (ViewModeData::StarLightList::iterator iter = m_pData->m_StarLightList.begin(); iter != m_pData->m_StarLightList.end(); ++iter)
	{
		if((*iter)->GetActiveMode() == m_pData->m_CurrentModeName)
			return (*iter)->GetVertexTint();
	}
	return LTVector(1,1,1);
}

void ViewModeMGR::ResetEnvMap()
{	
	for (ViewModeData::StarLightList::iterator iter = m_pData->m_StarLightList.begin(); iter != m_pData->m_StarLightList.end(); ++iter)
	{
		(*iter)->ResetEnvMap();
	}
}

void ViewModeMGR::Init()
{
	m_pData->m_CurrentVisionSet = g_pCharacterButeMgr->GetVisionSet(g_pGameClientShell->GetPlayerStats()->GetButeSet());


	SetVisionMode(m_pData, m_pData->m_InitialModeName);
	SetVisionSound();

	m_LastModeName = "Normal";

	g_cvarNVMinChargeLevel.Init(g_pLTClient, "NVMinChargeLevel", LTNULL, 50.0f);
}

void ViewModeMGR::RegisterTexture(StateChange** stateChange, const char* commandLine)
{
	m_pData->m_TextureModeMGR.RegisterTexture(stateChange, commandLine);
}

void ViewModeMGR::SetNormalMode()
{
	SetVisionMode(m_pData, "Normal");
	SetVisionSound();
}

LTBOOL ViewModeMGR::CanChangeMode()
{
	LTBOOL rVal = LTTRUE;

	if(IsPredator(g_pGameClientShell->GetPlayerMovement()->GetCharacterButes()))
	{
		if(!g_pGameClientShell->GetPlayerStats()->HasMask())
			rVal = LTFALSE;
	}

	if(IsHuman(g_pGameClientShell->GetPlayerMovement()->GetCharacterButes()))
	{
		if(!g_pGameClientShell->GetPlayerStats()->HasNightVision())
			rVal = LTFALSE;
	}

	return rVal;
}

void ViewModeMGR::SetVisionSound()
{
	//see if we are in normal mode
	if(_stricmp("normal", m_pData->m_CurrentModeName.c_str())==0)
	{
		//turn loop sound off
		if(m_hLoopingVMSound)
		{
			g_pLTClient->KillSound(m_hLoopingVMSound);
			m_hLoopingVMSound = LTNULL;
		}

		return;
	}

	switch(g_pGameClientShell->GetPlayerMovement()->GetCharacterButes()->m_eCharacterClass)
	{
		case PREDATOR:
			{
				//turn on the sound
				if(!m_hLoopingVMSound)
					m_hLoopingVMSound = g_pClientSoundMgr->PlaySoundLocal("pred_vision_loop");
			}
			break;

		case ALIEN:
			{
				//turn on the sound
				if(!m_hLoopingVMSound)
					m_hLoopingVMSound = g_pClientSoundMgr->PlaySoundLocal("alien_vision_loop");
			}
			break;

		default:
	
			break;
	}
}

LTBOOL ViewModeMGR::NextMode()
{
	std::string nextMode = VisionMode::g_pButeMgr->GetNextMode(m_pData->m_CurrentVisionSet, m_pData->m_CurrentModeName);

	if(VisionMode::g_pButeMgr->IsMarineNightVision(nextMode))
	{
		//see if we got enuf juice to turn on
		if(g_pInterfaceMgr->GetPlayerStats()->GetBatteryLevel() < g_cvarNVMinChargeLevel.GetFloat())
		{
			//no can do, not enuf juice to turn on
			return LTFALSE;
		}
	}

	SetVisionMode(m_pData, nextMode);
	SetVisionSound();

	CameraMgrFX_VisionCycleCreate(g_pGameClientShell->GetPlayerCamera());

	return LTTRUE;
}

LTBOOL ViewModeMGR::PrevMode()
{
	std::string prevMode = VisionMode::g_pButeMgr->GetPrevMode(m_pData->m_CurrentVisionSet, m_pData->m_CurrentModeName);

	if(VisionMode::g_pButeMgr->IsMarineNightVision(prevMode))
	{
		//see if we got enuf juice to turn on
		if(g_pInterfaceMgr->GetPlayerStats()->GetBatteryLevel() < g_cvarNVMinChargeLevel.GetFloat())
		{
			//no can do, not enuf juice to turn on
			return LTFALSE;
		}
	}

	SetVisionMode(m_pData, prevMode);
	SetVisionSound();

	CameraMgrFX_VisionCycleCreate(g_pGameClientShell->GetPlayerCamera());

	return LTTRUE;
}

void ViewModeMGR::Update(LTBOOL bAlive)
{
	// If we are dead then just update the overlay and get out...
	if(!bAlive)
	{
		m_pData->m_OverlayMGR.Update();
		return;
	}

	// Start out with no state
	static bool bEMPEffect = false;

	uint32 dwFlags;
	g_pLTClient->GetObjectUserFlags(g_pLTClient->GetClientObject(), &dwFlags);

	if(dwFlags & USRFLG_CHAR_EMPEFFECT)
	{
		if(!bEMPEffect)
		{	
			//first time, set flag and modes
			bEMPEffect = true;

			m_LastModeName = m_pData->m_CurrentModeName;
			SetVisionMode(m_pData, "Normal");
			SetVisionSound();
		}
		//exit
		return;
	}
	else
	{
		if(bEMPEffect)
		{
			//re-set back to prev mode
			SetVisionMode(m_pData, m_LastModeName);
			SetVisionSound();
			m_LastModeName = "Normal";
		}
		//be sure to set this to flase
		bEMPEffect = false;
	}

	LTBOOL marineMode = VisionMode::g_pButeMgr->IsMarineNightVision(m_pData->m_CurrentModeName);

	if(marineMode)
	{
		//check for sufficient battery power
		if(g_pInterfaceMgr->GetPlayerStats()->GetBatteryLevel() == 0.0f)
		{
			//oops outta juice
			SetVisionMode(m_pData, "Normal");
			SetVisionSound();

			//play the change sound
			g_pClientSoundMgr->PlaySoundLocal("vision_switch_off");

			return;
		}
	}

	// If we are a predator see that we only use the overlay when cloaked
	if(IsPredator(g_pInterfaceMgr->GetPlayerStats()->GetButeSet()))
	{
		m_pData->m_OverlayMGR.SetNormalOverlayVisible(dwFlags & USRFLG_CHAR_CLOAKED);
	}


	// Update things that need them
	m_pData->m_OverlayMGR.Update();
	UpdateStarLight(m_pData);
}

void ViewModeMGR::LevelExit()
{
	SetVisionMode(m_pData, Views::START_MODE);
	SetVisionSound();

	m_pData->m_InitialModeName = Views::START_MODE;

	// Delete the FX members
	for (ViewModeData::StarLightList::iterator iter = m_pData->m_StarLightList.begin(); 
		iter != m_pData->m_StarLightList.end(); ++iter)
	{
		delete (*iter);
	}
	m_pData->m_StarLightList.clear();
}

void ViewModeMGR::SetNetMode(LTBOOL bOn)
{
	m_pData->m_OverlayMGR.SetNetMode(bOn);
}

void ViewModeMGR::SetRailMode(LTBOOL bOn)
{
	SetVisionSound();
	m_pData->m_OverlayMGR.SetRailMode(bOn);
}

LTBOOL ViewModeMGR::GetRailMode()
{
	return m_pData->m_OverlayMGR.GetRailMode();
}

void ViewModeMGR::SetPredZoomMode(LTBOOL bOn)
{
	m_pData->m_OverlayMGR.SetPredZoomMode(bOn);
}

void ViewModeMGR::SetOnFireMode(LTBOOL bOn)
{
	m_pData->m_OverlayMGR.SetOnFireMode(bOn);
}

void ViewModeMGR::SetEvacMode(LTBOOL bOn)
{
	m_pData->m_OverlayMGR.SetEvacMode(bOn);
}

void ViewModeMGR::SetEvacRotation(LTRotation rRot)
{
	m_pData->m_OverlayMGR.SetEvacRotation(rRot);
}

void ViewModeMGR::SetEvacAlpha(LTFLOAT fAlpha)
{
	m_pData->m_OverlayMGR.SetEvacAlpha(fAlpha);
}

void ViewModeMGR::SetOrientMode(LTBOOL bOn)
{
	m_pData->m_OverlayMGR.SetOrientMode(bOn);
}

void ViewModeMGR::SetOrientAngle(LTFLOAT fAngle)
{
	m_pData->m_OverlayMGR.SetOrientAngle(fAngle);
}

void ViewModeMGR::MakeStarlightObject(HMESSAGEREAD hMessage)
{
	StarLightViewFX *pStarLight = new StarLightViewFX();
	pStarLight->Init(LTNULL, hMessage);
	pStarLight->CreateObject(g_pLTClient);
	pStarLight->SetMode("Normal");
	m_pData->m_StarLightList.push_back(pStarLight);
}

void ViewModeMGR::SetMenuMode(LTBOOL bOn)
{
	if (bOn)
	{
		if( !m_pData->m_bMenuMode )
		{
			m_pData->m_bMenuMode = LTTRUE;

			m_LastModeName = m_pData->m_CurrentModeName;
			SetVisionMode(m_pData, "Normal");
			SetVisionSound();
		}
	}
	else
	{
		if( m_pData->m_bMenuMode )
		{
			m_pData->m_bMenuMode = LTFALSE;

			SetVisionMode(m_pData, m_LastModeName);
			SetVisionSound();
		}
	}
}

void ViewModeMGR::SetCineMode(LTBOOL bOn)
{
	if(bOn && _stricmp("normal", m_pData->m_CurrentModeName.c_str())!=0 )
	{
		SetMenuMode(bOn);
		m_bCineMode = bOn;
	}
	else
	{
		if(bOn == m_bCineMode) return;

		SetMenuMode(bOn);
		m_bCineMode = bOn;
	}
}

void SetVisionMode(ViewModeData* data, std::string mode)
{
	data->m_CurrentModeName = mode;
	data->m_OverlayMGR.SetMode(mode);
	data->m_TextureModeMGR.SetMode(mode);

	// Now set the textures to point to the structures of this new mode
	g_pGameClientShell->GetSFXMgr()->SetMode(mode);
	g_pGameClientShell->GetWeaponModel()->ResetModelSkins();

	const LTBOOL bAllowHud = VisionMode::g_pButeMgr->AllowHud(mode);

	//set HUD enabled
	g_pGameClientShell->GetInterfaceMgr()->GetHudMgr()->SetMDDraw(bAllowHud);

	// Invert the screen..
	LTBOOL bIsInverted = VisionMode::g_pButeMgr->IsInverted(mode);
	WriteConsoleInt("InvertHack", (int)bIsInverted);

	// Invert the screen..
	LTBOOL bDrawSky = VisionMode::g_pButeMgr->ShouldDrawSky(mode);
	WriteConsoleInt("DrawSky", (int)bDrawSky);

	// Turn on/off lighting
	LTBOOL bIsFullBright = VisionMode::g_pButeMgr->IsFullBright(mode);
	WriteConsoleInt("LMFullBright", (int)bIsFullBright);

	// Starlight gets a turn
	SetStarLightMode(data, mode);

	// Send message to server
	HMESSAGEWRITE hMessage = g_pClientDE->StartMessage(MID_PLAYER_CLIENTMSG);
	g_pClientDE->WriteToMessageByte(hMessage, CP_VISIONMODE);

	if(VisionMode::g_pButeMgr->IsMarineNightVision(mode))
		g_pClientDE->WriteToMessageByte(hMessage, VM_NIGHTV_ON);
	else if(VisionMode::g_pButeMgr->IsPredatorHeatVision(mode))
		g_pClientDE->WriteToMessageByte(hMessage, VM_PRED_HEAT);
	else if(VisionMode::g_pButeMgr->IsPredatorElecVision(mode))
		g_pClientDE->WriteToMessageByte(hMessage, VM_PRED_ELEC);
	else if(VisionMode::g_pButeMgr->IsPredatorNavVision(mode))
		g_pClientDE->WriteToMessageByte(hMessage, VM_PRED_NAV);
	else
		g_pClientDE->WriteToMessageByte(hMessage, VM_NIGHTV_OFF);

	g_pClientDE->EndMessage(hMessage);
}

void UpdateStarLight(ViewModeData* data)
{
	for (ViewModeData::StarLightList::iterator iter = data->m_StarLightList.begin(); 
		iter != data->m_StarLightList.end(); ++iter)
	{
		(*iter)->Update();
	}
}

void SetStarLightMode(ViewModeData* data, std::string mode)
{
	// Always go in here first so that other variables can get setup...
	// just in case they've changed.
	if(g_pGameClientShell->IsInWorld())
	{
		g_pGameClientShell->ResetGlobalFog();
	}


	// First time turns off the old one
	for (ViewModeData::StarLightList::iterator iter = data->m_StarLightList.begin(); 
		iter != data->m_StarLightList.end(); ++iter)
	{
		(*iter)->SetMode(mode);
	}

	// second one turns on the new
	for (iter = data->m_StarLightList.begin(); 
		iter != data->m_StarLightList.end(); ++iter)
	{
		(*iter)->SetMode(mode);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Save()
//
//	PURPOSE:	Save the flashlight info...
//
// ----------------------------------------------------------------------- //

void ViewModeMGR::Save(HMESSAGEWRITE hWrite)
{
	// Save the vision mode we should load into.
	if( !m_pData->m_bMenuMode )
		*hWrite << m_pData->m_CurrentModeName;
	else
		*hWrite << m_LastModeName;

	*hWrite << m_LastModeName;
	*hWrite << m_bCineMode;
	*hWrite << m_pData->m_bMenuMode;

	m_pData->m_OverlayMGR.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Load()
//
//	PURPOSE:	Load the flashlight info...
//
// ----------------------------------------------------------------------- //

void ViewModeMGR::Load(HMESSAGEREAD hRead)
{
	// Load the saved current mode into the initial
	// mode so that when ViewModeMgr::Init is called,
	// it will set the mode to what was the current mode.
	*hRead >> m_pData->m_InitialModeName;

	*hRead >> m_LastModeName;
	*hRead >> m_bCineMode;
	*hRead >> m_pData->m_bMenuMode;

	m_pData->m_OverlayMGR.Load(hRead);
}
