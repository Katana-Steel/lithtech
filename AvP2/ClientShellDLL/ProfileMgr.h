// ProfileMgr.h: interface for the CProfileMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef PROFILE_MGR_H
#define PROFILE_MGR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>			// For strings
#include <set>				// For the string comtainer
#include <iostream>			// For input and output
#include <fstream>			// For the files
#include <IO.h>				// Find first, find next, etc.

#include "ClientUtilities.h"
#include "ButeMgr.h"
#include "JoystickAxis.h"
#include "MissionMgr.h"

class CProfileMgr;
extern CProfileMgr* g_pProfileMgr;


const int NUM_DEVICES = 3;
const int MAX_NUM_COMMANDS = 25;
const int kMaxDeviceAxis = 10;

#define RESTORE_BINDINGS			0x01
#define RESTORE_MOUSE				0x02
#define RESTORE_KEYBOARD			0x04
#define RESTORE_JOYSTICK			0x08

//////////////////////////////////////////////////////////////////////

class CBindingData
{
	public:

		CBindingData()
			{ nStringID = 0; nAction = 0; memset(strTriggerName,0,sizeof(strTriggerName)); memset(strRealName,0,sizeof(strRealName)); nCode = 0;}

		uint32      nStringID;
		int			nAction;
		char		strTriggerName[NUM_DEVICES][64];
		char		strRealName[NUM_DEVICES][64];
		uint16		nCode;
};

//////////////////////////////////////////////////////////////////////

class CDeviceAxisData
{
	public:

		CDeviceAxisData()
			{ Init("",0,0.0f,0.0f); }

		void Init(char *sName, uint32 nType, float fRangeLow, float fRangeHigh)
			{ SAFE_STRCPY(m_sName,sName); m_nType = nType; m_fRangeLow = fRangeLow; m_fRangeHigh = fRangeHigh; }

		char	m_sName[INPUTNAME_LEN];
		uint32  m_nType;
		float	m_fRangeLow;
		float	m_fRangeHigh;
};

//////////////////////////////////////////////////////////////////////

struct LevelStatus
{
	// 9 = incomplete
	// 0 to 3 = complete (easy, medium, hard, insane)
	DWORD Alien;
	DWORD Marine;
	DWORD Predator;
};

typedef struct LevelStatus LevelStatus;

//////////////////////////////////////////////////////////////////////

class CUserProfile
{
public:

	CUserProfile();

	LTBOOL		Init(const std::string& profileName, LTBOOL bCreate);
	LTBOOL		RestoreDefaults(int nFlags = 0xFF);
	void		Term();

	//take bindings from profile and apply them to the game
	void		ApplyBindings(Species s = Unknown);
	void		ApplyMouse();
	void		ApplyKeyboard();
	void		ApplyJoystick();
	void		ApplyGame();
	void		ApplyMultiplayer();

	//read bindings from the game and save them in the profile
	void		SetBindings(Species s = Unknown);
	void		SetMouse();
	void		SetKeyboard();
	void		SetJoystick();
	void		SetGame();
	void		SetMultiplayer();

	void		Save();

	LTBOOL		IsInitted() {return m_bInitted;}

	CBindingData* FindBinding(int nType, int commandIndex);

public:

	std::string		m_sName;

	// Mouse
    LTBOOL			m_bInvertY;
    LTBOOL			m_bMouseLook;
	int				m_nInputRate;
	int				m_nSensitivity;

	// Keyboard
    LTBOOL          m_bAutoCenter;
	int				m_nNormalTurn;
	int	  			m_nFastTurn;
	int				m_nLookUp;

	// Joystick
	LTBOOL				m_bUseJoystick;
	CJoystickAxisLook	m_AxisLook;
	CJoystickAxisTurn	m_AxisTurn;
	CJoystickAxisStrafe m_AxisStrafe;
	CJoystickAxisMove	m_AxisMove;

	// Game
	LTFLOAT			m_fPickupDuration;
	LTBOOL			m_bAutoswitch;
	LTBOOL			m_bRunLock;
	LTBOOL			m_bHeadBob;
	LTBOOL			m_bHeadCanting;
	LTBOOL			m_bWeaponSway;
	LTBOOL			m_bObjectives;
	LTBOOL			m_bHints;
	LTBOOL			m_bSubtitles;
	LTBOOL			m_bOrientOverlay;

	// Multiplayer
	std::string		m_sPlayerName;
	uint8			m_nRace;
	uint8			m_nChar[4];
	LTFLOAT			m_fMessageDisplay;
	LTFLOAT			m_fMessageTime;
	uint8			m_nConnection;
	LTBOOL			m_bIgnoreTaunts;

	// Level completion status
	LevelStatus		m_LevelStatus;

private:

	void		LoadBindings();
	void		LoadMouse();
	void		LoadKeyboard();
	void		LoadJoystick();

private:

	int				m_nNumBindings[4];
	CBindingData	m_bindings[4][MAX_NUM_COMMANDS];


	LTBOOL		m_bInitted;
	CButeMgr	m_buteMgr;
	char*		m_pCryptKey;
};

//////////////////////////////////////////////////////////////////////

class CProfileMgr
{
	public:

		LTBOOL			Init();
		void			Term();

		void			GetProfileList(StringSet& profileList);

		//saves current profile and loads specified one (creating it if necessary)
		void			NewProfile(const std::string& profileName);
		void			DeleteProfile(const std::string& profileName);
		void			RenameProfile(const std::string& oldName,const std::string& newName);
		CUserProfile*	GetCurrentProfile() {return &m_profile;}
		const char*		GetCurrentProfileName() {return m_profile.m_sName.c_str();}

		void			SetSpecies(const std::string& speciesName);
		void			SetSpecies(Species species);

		Species			GetSpecies() {return m_eSpecies;}

		void			ClearBindings();
		uint32			GetControlType(uint32 deviceType, char *controlName);

		int				GetNumAxis() {return m_nNumDeviceAxis;}
		CDeviceAxisData* GetAxisData(int ndx);

		void		ApplyBandwidthThrottle();

	private:

		void			SetDeviceData();

		CUserProfile	m_profile;
		Species			m_eSpecies;

		// Information about each joystick axis
		int				m_nNumDeviceAxis;
		CDeviceAxisData m_aDeviceData[kMaxDeviceAxis];
};


#endif	// PROFILE_MGR_H
