/*******************************************************************************
;
;	MODULE:			LAUNCHER (.H)
;
;	PURPOSE:		Launcher application main header file
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Inc.
;
********************************************************************************/

#if !defined(AFX_LAUNCHER_H__1E6712C7_B0F3_11D4_A2BA_00010229388A__INCLUDED_)
#define AFX_LAUNCHER_H__1E6712C7_B0F3_11D4_A2BA_00010229388A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// Includes
#include "resource.h"
#include "RegMgr32.h"

// Defines
#define ACTION_HOSTGAME				100
#define ACTION_JOINGAME				101
#define ACTION_FINDINTERNETSERVERS	102
#define ACTION_STANDALONESERVER		103
#define ACTION_INSTALL				104
#define ACTION_UNINSTALL			105
#define ACTION_PLAY					106

#define DETAIL_LOW					0
#define DETAIL_MEDIUM				1
#define DETAIL_HIGH					2

//#define DIR_CUSTOM					"Custom"
#define DIR_MODS					"Custom\\Mods"
#define DIR_RESOURCES				"Custom\\Resources"
#define FIELD_SELECTEDMOD			"SelectedMod"

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp:
// See Launcher.cpp for the implementation of this class
//

class CLauncherApp : public CWinApp
{
public:
	CLauncherApp();

	// General variables
	CRegMgr32 m_RegMgr;						// Registry manager object
	CBrush	m_BkBrush;						// Black background brush for edit and list boxes
	CFont	m_Font;							// Font for edit and list boxes
	char	m_chDrive;						// Which drive is the CDROM?

	BOOL	m_bInstalled;					// Is the game installed?
	BOOL	m_bRestoreDefaults;				// Should all default settings be restored?
	BOOL	m_bSaveCommands;				// Should the command-line commands be saved?
	BOOL	m_bUseDetailSettings;			// Should the detail settings be used?
	BOOL	m_bUseDisplaySettings;			// Should the display settings be used?
	BOOL	m_bDisableSound;				// Disables sound
	BOOL	m_bDisableMusic;				// Disables music
	BOOL	m_bDisableMovies;				// Disables movies
	BOOL	m_bDisableJoysticks;			// Disables Joysticks
	BOOL	m_bDisableTripleBuffering;		// Disables triple buffering
	BOOL	m_bDisableHardwareCursor;		// Disables the hardware cursor
	BOOL	m_bDisableHardwareSound;		// Disables hardware 3D sound
	BOOL	m_bDisableSoundFilters;			// Disables filtering of sounds in-game
	BOOL	m_bEnableScreenshots;			// Enables in-game screenshots

	BOOL	m_bGoMultiHost;					// Jump right into a multiplayer host game
	BOOL	m_bGoMultiJoin;					// Jump right into a multiplayer join game
	BOOL	m_bGoGame;						// Jump right into a single player game
	BOOL	m_bGoMultiFind;					// Jump right into find Internet servers
	BOOL	m_bGoServer;					// Jump right into the stand-alone server

	BOOL	m_bHasProfile;					// Has user already created a profile

	DWORD	m_dwNumLauncherRuns;			// How many times the launcher has been run
	DWORD	m_dwLastRandom;					// Last random value (for random splash screens)
	DWORD	m_dwDetailSetting;				// Which detail level does the user want?

	CString m_csAppName;					// Application name
	CString m_csAppCD1Check;				// "Please insert CD #1"
	CString m_csAppCD2Check;				// "Please insert CD #2"
	CString m_csRezBase;					// Base rez directory/file
	CString m_csSetupExe;					// Name of installer executable
	CString	m_csServerExe;					// Name of stand-alone server executable
	CString m_csLanguage;					// Language (for language rez files)
	CString m_csCDRoot;						// Root CDRom drive
	CString m_csCommands;					// Command-line commands specified by the user
	CString	m_csInstallDir;					// Directory where the game is installed
	CString m_csAppVersion;					// Application version

	// Graphical message box functions
	int		MessageBox(const char *szText, const char *szCaption, DWORD dwType = MB_OK);
	int		MessageBox(UINT nIDText, UINT nIDCaption, DWORD dwType = MB_OK);

	// Registry helpers
	BOOL	GetRegistryBool(char *szField,BOOL bDefault = FALSE);
	DWORD	GetRegistryDWord(char *szField, DWORD dwDefault = 0);
	BOOL	OpenRegistryKey();

	// CD functions
	BOOL	CD_VerifyGame();
	BOOL	CD_ExistGame(BOOL & bDisk2);
	BOOL	CD_ExistFile(const char* sFile);
	char	CD_GetDriveLetter();
	char	CD_GetDriveLetterWithGame(BOOL & bDisk2);
	char	CD_TestDriveLetterForGameDisc1(char chDrive, BOOL & bDisk2);
	char	CD_TestDriveLetterForGameDisc2(char chDrive);

	// General functions
	CString GetAdvancedOptionsCommandLineString();
	BOOL	ReadConfigFile(const char* sFile);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLauncherApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLauncherApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// Extern this guy so everyone can get access
extern CLauncherApp theApp;

extern COLORREF crHighlightText;
extern COLORREF crNormalText;



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LAUNCHER_H__1E6712C7_B0F3_11D4_A2BA_00010229388A__INCLUDED_)
