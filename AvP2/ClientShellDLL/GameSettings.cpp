#include "stdafx.h"
#include "GameClientShell.h"
#include "client_de.h"
#include "GameSettings.h"
#include "stdio.h"
#include "windows.h"
#include "ClientRes.h"
#include "SoundTypes.h"

CGameSettings::CGameSettings()
{
	m_pClientDE = DNULL;
	m_pClientShell = DNULL;
	m_bAllowGore = TRUE;
}

//////////////////////////////////////////////////////////////////
//
//	INIT THE SETTINGS...
//
//////////////////////////////////////////////////////////////////

DBOOL CGameSettings::Init (CClientDE* pClientDE, CGameClientShell* pClientShell)
{
	if (!pClientDE || !pClientShell) return DFALSE;

	m_pClientDE = pClientDE;
	m_pClientShell = pClientShell;

	// check if gore is allowed

//	if (TextHelperCheckStringID(m_pClientDE, IDS_ALLOW_NO_GORE, "TRUE")) m_bAllowGore = FALSE;
//	else m_bAllowGore = TRUE;



	
	DDWORD dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();
	if (!(dwAdvancedOptions & AO_LIGHTMAP))
	{
		SetFloatVar("LightMap",0.0f);
	}

	if (!(dwAdvancedOptions & AO_MODELFULLBRITE))
	{
		SetFloatVar("ModelFullbrite",0.0f);
	}
	if (dwAdvancedOptions & AO_MUSIC)
	{
		m_pClientDE->RunConsoleString("musicenable 1");
	}
	else
	{
		m_pClientDE->RunConsoleString("musicenable 0");
	}
	if (dwAdvancedOptions & AO_SOUND)
	{
		m_pClientDE->RunConsoleString("soundenable 1");
	}
	else
	{
		m_pClientDE->RunConsoleString("soundenable 0");
	}

	if (dwAdvancedOptions & AO_JOYSTICK ||  UseJoystick())
	{
		char strJoystick[128];
		memset (strJoystick, 0, 128);
		DRESULT result = m_pClientDE->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
		if (result == LT_OK) 
		{
			char strConsole[256];
			sprintf (strConsole, "EnableDevice \"%s\"", strJoystick);
			m_pClientDE->RunConsoleString (strConsole);
			m_pClientDE->RunConsoleString("UseJoystick 1");
		}
		else
		{
			m_pClientDE->RunConsoleString("UseJoystick 0");
		}

	}
	else
	{
		m_pClientDE->RunConsoleString("UseJoystick 0");
	}

	// implement settings that need implementing

	ImplementMouseSensitivity();
	ImplementDetailSettings();


	ImplementMusicSource();
	ImplementMusicVolume();

	// hack to keep sound volume reasonable
	if (SFXVolume() > 90.0f) 
		SetFloatVar("sfxvolume",90.0f);

	ImplementSoundVolume();

	g_pGameClientShell->InitSound();
	
	return DTRUE;
}







//////////////////////////////////////////////////////////////////
//
//	IMPLEMENT CURRENT RENDERER SETTING
//
//////////////////////////////////////////////////////////////////

DBOOL CGameSettings::ImplementRendererSetting()
{
	if (!m_pClientDE || !m_pClientShell) return DFALSE;

	// make sure the active mode isn't what we're trying to set...

	RMode current;
	memset (&current, 0, sizeof (RMode));
	if (m_pClientDE->GetRenderMode (&current) != DE_OK) return DFALSE;

	if (strcmp (CurrentRenderer.m_RenderDLL, current.m_RenderDLL) == 0 && strcmp (CurrentRenderer.m_Description, current.m_Description) == 0)
	{
		if (CurrentRenderer.m_Width == current.m_Width && CurrentRenderer.m_Height == current.m_Height)
		{
			return DTRUE;
		}
	}

	// attempt to set the render mode

	g_pInterfaceMgr->SetSwitchingRenderModes(DTRUE);
	DRESULT hResult = m_pClientDE->SetRenderMode(&CurrentRenderer);
	g_pInterfaceMgr->SetSwitchingRenderModes(DFALSE);

	if (hResult == LT_KEPTSAMEMODE || hResult == DE_UNABLETORESTOREVIDEO)
	{
		if (hResult == LT_KEPTSAMEMODE)
		{
			// reset the structure
			m_pClientDE->GetRenderMode (&CurrentRenderer);
		}
		return DFALSE;
	}

	m_pClientDE->GetRenderMode (&current);
	if (stricmp (CurrentRenderer.m_Description, current.m_Description) != 0 ||
		stricmp (CurrentRenderer.m_RenderDLL, current.m_RenderDLL) != 0 ||
		CurrentRenderer.m_Width != current.m_Width || CurrentRenderer.m_Height != current.m_Height)
	{
		// SetRenderMode() returned success, but it really didn't work
		// reset the structure
		m_pClientDE->GetRenderMode (&CurrentRenderer);
		return DFALSE;
	}

	// adjust the screen and camera rects

	DDWORD nScreenWidth = 0;
	DDWORD nScreenHeight = 0;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	m_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	int nLeft = 0;
	int nTop = 0;
	int nRight = (int)nScreenWidth;
	int nBottom = (int)nScreenHeight;
	DBOOL bFullScreen = DTRUE;
	m_pClientDE->SetCameraRect (m_pClientShell->GetPlayerCameraObject(), bFullScreen, nLeft, nTop, nRight, nBottom);
	m_pClientDE->SetCameraRect (m_pClientShell->GetInterfaceCamera(), bFullScreen, nLeft, nTop, nRight, nBottom);
	g_pInterfaceMgr->ResetMenuRestoreCamera (0, 0, nRight, nBottom);
	g_pInterfaceMgr->ScreenDimsChanged();

	// make sure the structure is completely filled in

	m_pClientDE->GetRenderMode (&CurrentRenderer);
			

	return DTRUE;
}

void CGameSettings::ImplementMusicSource()
{
	if (!m_pClientDE || !m_pClientShell) return;

    uint32 dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

    LTBOOL bPlay = MusicEnabled() && (dwAdvancedOptions & AO_MUSIC);

	if (bPlay)
	{
		if (m_pClientShell->GetMusic()->Init(m_pClientDE))
		{
			m_pClientShell->GetMusic()->Play();
		}
	}
	else if (!MusicEnabled())
	{
		m_pClientShell->GetMusic()->Stop();
	}
	else if (!(dwAdvancedOptions & AO_MUSIC))
	{
		m_pClientDE->RunConsoleString("musicenable 0");
		m_pClientShell->GetMusic()->Term();
	}

}

void CGameSettings::ImplementMusicVolume()
{
	if (!m_pClientDE) return;

	float fMusicVolume = MusicVolume();

	g_pGameClientShell->GetMusic()->SetMenuVolume((long)fMusicVolume);

	// Set the sound volume for the sound effects we play for the music.
	(( ILTClientSoundMgr* )m_pClientDE->SoundMgr( ))->SetVolumeByType(( uint16 )fMusicVolume, 
		USERSOUNDTYPE_MUSIC);
}


void CGameSettings::ImplementSoundVolume()
{
	if (!m_pClientDE) return;

	float fSoundVolume = SFXVolume();

	(( ILTClientSoundMgr* )m_pClientDE->SoundMgr( ))->SetVolumeByType((uint16)fSoundVolume, 
		USERSOUNDTYPE_SOUNDEFFECT);
}

void CGameSettings::ImplementMouseSensitivity()
{
/*	if (!m_pClientDE) return;

	float nMouseSensitivity = GetFloatVar("MouseSensitivity");

	// get the mouse device name

	char strDevice[128];
	memset (strDevice, 0, 128);
	DRESULT result = m_pClientDE->GetDeviceName (DEVICETYPE_MOUSE, strDevice, 127);
	if (result == LT_OK)
	{
		// get mouse x- and y- axis names

		char strXAxis[32];
		memset (strXAxis, 0, 32);
		char strYAxis[32];
		memset (strYAxis, 0, 32);

		DBOOL bFoundXAxis = DFALSE;
		DBOOL bFoundYAxis = DFALSE;
		
		DeviceObject* pList = m_pClientDE->GetDeviceObjects (DEVICETYPE_MOUSE);
		DeviceObject* ptr = pList;
		while (ptr)
		{
			if (ptr->m_ObjectType == CONTROLTYPE_XAXIS)
			{
				SAFE_STRCPY(strXAxis, ptr->m_ObjectName);
				bFoundXAxis = DTRUE;
			}

			if (ptr->m_ObjectType == CONTROLTYPE_YAXIS)
			{
				SAFE_STRCPY(strYAxis, ptr->m_ObjectName);
				bFoundYAxis = DTRUE;
			}

			ptr = ptr->m_pNext;
		}
		if (pList) m_pClientDE->FreeDeviceObjects (pList);
		
		if (bFoundXAxis && bFoundYAxis)
		{
			// run the console string

			char strConsole[64];
			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strXAxis, 0.00125f + ((float)nMouseSensitivity * 0.001125f));
			m_pClientDE->RunConsoleString (strConsole);
			sprintf (strConsole, "scale \"%s\" \"%s\" %f", strDevice, strYAxis, 0.00125f + ((float)nMouseSensitivity * 0.001125f));
			m_pClientDE->RunConsoleString (strConsole);
		}
	}
*/
}


void CGameSettings::ImplementDetailSettings()
{
	if (!m_pClientDE || !m_pClientShell) return;

	DDWORD dwAdvancedOptions = g_pInterfaceMgr->GetAdvancedOptions();

	if (!(dwAdvancedOptions & AO_FOG))
	{
		m_pClientDE->RunConsoleString ("+FogEnable 0");
	}

	
	if (!(dwAdvancedOptions & AO_LIGHTMAP))
	{
		m_pClientDE->RunConsoleString ("+LightMap 0");
	}


	if (!(dwAdvancedOptions & AO_MODELFULLBRITE)) 
	{
		m_pClientDE->RunConsoleString ("+ModelFullbrite 0");
	}

}


namespace
{
	const int numDemoSettings = 22;
	char demoSettings[numDemoSettings][32] =
	{
		"MouseLook",
		"MouseInvertY",
		"MouseSensitivity",
		"InputRate",
		"JoyLook",
		"JoyInvertY",
		"LookSpring",
		"RunLock",
		"Gore",
		"ModelLOD",
		"MaxModelShadows",
		"BulletHoles",
		"TextureDetail",
		"DynamicLightSetting",
		"LightMap",
		"SpecialFX",
		"EnvMapEnable",
		"ModelFullbrite",
		"CloudMapLight",
		"PVWeapons",
		"PolyGrids",
	};
};


void CGameSettings::LoadDemoSettings(DStream *pStream)
{
	for(int i=0; i <= numDemoSettings; i++)
	{		
		float temp;
		(*pStream) >> temp;
		SetFloatVar(demoSettings[i],temp);
	}
}


void CGameSettings::SaveDemoSettings(DStream *pStream)
{
	for(int i=0; i <= numDemoSettings; i++)
	{		
		float temp = GetFloatVar(demoSettings[i]);
		(*pStream) << temp;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameSettings::Save
//
//	PURPOSE:	Save the interface info
//
// --------------------------------------------------------------------------- //

void CGameSettings::Save(HMESSAGEWRITE hWrite)
{
	ASSERT( hWrite );
	if( !hWrite )
		return;

	LTBOOL bCL = CrouchLock();		
	LTBOOL bWW = WallWalkLock();
	
	*hWrite << bCL;
	*hWrite << bWW;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameSettings::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CGameSettings::Load(HMESSAGEREAD hRead)
{
	ASSERT( hRead );
	if( !hRead )
		return;

	LTBOOL bCL;
	LTBOOL bWW;
	
	*hRead >> bCL;
	*hRead >> bWW;

	SetCrouchLock (bCL);
	SetWallWalkLock (bWW);
}