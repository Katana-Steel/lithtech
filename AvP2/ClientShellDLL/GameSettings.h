#ifndef __GameSettings_H
#define __GameSettings_H


// Values for 3 position options..

#define RS_LOW						0
#define	RS_MED						1
#define RS_HIGH						2


class CGameClientShell;

//***********************************************
//
//		Class Definition
//
//***********************************************

class CGameSettings
{

public:

	CGameSettings();
	~CGameSettings()	{}

	DBOOL		Init (CClientDE* pClientDE, CGameClientShell* pClientShell);

	DBOOL		GetBoolVar(char *pVar);
	void		SetBoolVar(char *pVar, DBOOL bVal);
	DBYTE		GetByteVar(char *pVar);
	void		SetByteVar(char *pVar, DBYTE nVal);
	float		GetFloatVar(char *pVar);
	void		SetFloatVar(char *pVar, float fVal);
	
	// misc access functions

	DBOOL		ScreenFlash() {return GetBoolVar("ScreenFlash");}
	
	// control access functions

	DBOOL		MouseLook()					{ return GetBoolVar("MouseLook"); }
	DBOOL		MouseInvertY()				{ return GetBoolVar("MouseInvertY");}
	void		SetMouseLook(DBOOL bVal)	{ SetBoolVar("MouseLook",bVal); }
	void		SetMouseInvertY(DBOOL bVal) { SetBoolVar("MouseInvertY",bVal); }
	
	float		MouseSensitivity()			{ return GetFloatVar("MouseSensitivity");}
	DBOOL		UseJoystick()				{ return GetBoolVar("UseJoystick"); }
	DBOOL		AutoCenter()				{ return GetBoolVar("AutoCenter"); }

	DBOOL		RunLock()					{ return GetBoolVar("RunLock"); }
	DBOOL		CrouchLock()				{ return GetBoolVar("CrouchLock"); }
	DBOOL		WallWalkLock()				{ return GetBoolVar("WallWalkLock"); }

	void		SetRunLock (DBOOL bRunLock)				{ SetBoolVar("RunLock",bRunLock); }
	void		SetCrouchLock (DBOOL bCrouchLock)		{ SetBoolVar("CrouchLock",bCrouchLock); }
	void		SetWallWalkLock (DBOOL bWallWalkLock)	{ SetBoolVar("WallWalkLock",bWallWalkLock); }

	// sound access functions

	DBOOL		MusicEnabled()				{ return GetBoolVar("MusicEnable"); }
	float		MusicVolume()				{ return GetFloatVar("MusicVolume");}
	DBOOL		SoundEnabled()				{ return GetBoolVar("SoundEnable"); }
	float		SoundVolume()				{ return GetFloatVar("soundvolume");}
	float		SFXVolume()					{ return GetFloatVar("sfxvolume");}
	float		SoundChannels()				{ return GetFloatVar("SoundChannels");}
	DBOOL		Sound16Bit()				{ return GetBoolVar("Sound16Bit"); }

	// top-level detail access functions

	DBOOL		Gore()						{ return GetBoolVar("Gore"); }

	// low-level detail access functions

	float		ModelLOD()					{ return GetFloatVar("ModelLOD");}
	DBYTE		Shadows()					{ return GetByteVar("MaxModelShadows"); }
	float		NumBulletHoles()			{ return GetFloatVar("BulletHoles");}
	float		TextureDetailSetting()		{ return GetFloatVar("TextureDetail");}
	float		DynamicLightSetting()		{ return GetFloatVar("DynamicLightSetting");}
	DBOOL		LightMap()					{ return GetBoolVar("LightMap");}
	DBYTE		SpecialFXSetting()			{ return GetByteVar("SpecialFX"); }
	DBOOL		EnvironmentMapping()		{ return GetBoolVar("EnvMapEnable"); }
	DBOOL		ModelFullBrights()			{ return GetBoolVar("ModelFullbrite"); }
	DBOOL		CloudMapLight()				{ return GetBoolVar("CloudMapLight"); }
	DBYTE		PlayerViewWeaponSetting()	{ return GetByteVar("PVWeapons"); }
	DBOOL		PolyGrids()					{ return GetBoolVar("PolyGrids"); }
	DBOOL		DrawSky()					{ return GetBoolVar("DrawSky"); }
	DBOOL		FogEnable()					{ return GetBoolVar("FogEnable"); }

	// read/write functions
	void		LoadDemoSettings(DStream *pStream);
	void		SaveDemoSettings(DStream *pStream);
	
	// utility functions
	void		SetLowDetail();
	void		SetMedDetail();
	void		SetHiDetail();

	// settings implementation functions

	DBOOL		ImplementRendererSetting();
	void		ImplementMusicSource();
	void		ImplementMusicVolume();
	void		ImplementSoundVolume();
	void		ImplementMouseSensitivity();
	void		ImplementDetailSettings();

	void		Save(HMESSAGEWRITE hWrite);
	void		Load(HMESSAGEREAD hRead);


private:

	CClientDE*			m_pClientDE;
	CGameClientShell*	m_pClientShell;
	RMode				CurrentRenderer;


	// Allow any gore (even display of the option in game)
	DBOOL		m_bAllowGore;

	HCONSOLEVAR m_hTmpVar;
	char		m_tmpStr[128];


};

inline DBOOL	CGameSettings::GetBoolVar(char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
		return (DBOOL)m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
		return DFALSE;
};


inline	void	CGameSettings::SetBoolVar(char *pVar, DBOOL bVal)
{
	sprintf (m_tmpStr, "+%s %d", pVar, bVal ? 1 : 0);
	m_pClientDE->RunConsoleString (m_tmpStr);
};

inline	DBYTE	CGameSettings::GetByteVar(char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
		return (DBYTE)m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
		return 0;
};

inline	void	CGameSettings::SetByteVar(char *pVar, DBYTE nVal)
{
	sprintf (m_tmpStr, "+%s %d", pVar, nVal);
	m_pClientDE->RunConsoleString (m_tmpStr);
}


inline	float	CGameSettings::GetFloatVar(char *pVar)
{
	m_hTmpVar = m_pClientDE->GetConsoleVar(pVar);
	if (m_hTmpVar)
		return m_pClientDE->GetVarValueFloat(m_hTmpVar);
	else
		return 0.0f;
};
inline	void	CGameSettings::SetFloatVar(char *pVar, float fVal)
{
	sprintf (m_tmpStr, "+%s %f", pVar, fVal);
	m_pClientDE->RunConsoleString (m_tmpStr);
}


#endif