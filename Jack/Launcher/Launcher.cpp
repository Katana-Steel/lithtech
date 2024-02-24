/****************************************************************************
;
;	MODULE:			Launcher (.cpp)
;
;	PURPOSE:		Launcher application main header file
;
;	HISTORY:		11/16/2000 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2000-2002, Monolith Productions, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "LauncherDlg.h"
#include "MessageBoxDlg.h"
#include "RezFind.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

COLORREF crHighlightText = RGB(255, 255, 0);
COLORREF crNormalText = RGB(192, 160, 32);

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp

BEGIN_MESSAGE_MAP(CLauncherApp, CWinApp)
	//{{AFX_MSG_MAP(CLauncherApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp construction

CLauncherApp::CLauncherApp()
{
	m_bDisableSound = FALSE;

	m_bDisableMusic = FALSE;
	m_bDisableMovies = FALSE;
	m_bDisableJoysticks = FALSE;
	m_bDisableTripleBuffering = FALSE;
	m_bDisableHardwareCursor = FALSE;
	m_bDisableHardwareSound = TRUE;
	m_bDisableSoundFilters = TRUE;
	m_bEnableScreenshots = FALSE;
	m_bRestoreDefaults = FALSE;
	m_bSaveCommands = FALSE;
	m_dwNumLauncherRuns = 0;
	m_bHasProfile = FALSE;
	m_chDrive = 0;
	m_dwLastRandom = 1;
	m_bUseDisplaySettings = FALSE;
	m_bUseDetailSettings = FALSE;
	m_dwDetailSetting = DETAIL_MEDIUM;
	m_bGoMultiHost = FALSE;
	m_bGoMultiJoin = FALSE;
	m_bGoGame = FALSE;
	m_bGoMultiFind = FALSE;
	m_bGoServer = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLauncherApp object

CLauncherApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CLauncherApp initialization

BOOL CLauncherApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Load some strings
	m_csAppName.LoadString(IDS_GAME_NAME);
	m_csAppName.LoadString(IDS_GAME_NAME);
	m_csAppCD1Check.Format( "\\%s", LAUNCHER_EXE );
	m_csAppCD2Check = "\\DATA\\GAME2.REZ";
	m_csRezBase.LoadString(IDS_REZBASE);
	m_csSetupExe.LoadString(IDS_SETUPEXE);
	m_csServerExe.LoadString(IDS_SERVEREXE);
	m_csLanguage.LoadString(IDS_LANGUAGE);
	m_csAppVersion.LoadString(IDS_APPVERSION);

	// Check if we are already running...

	char sPath[128];
	char sName[128];
	char sExt[32];
	char sExe[128];
	GetModuleFileName(NULL, sPath, 125);
	_splitpath(sPath, NULL, NULL, sName, sExt);
	sprintf(sExe, "%s%s", sName, sExt);
	if(ExistProcess(sExe, 2))
	{
		return FALSE;
	}

	char sDir[128] = { "" };
	GetCurrentDirectory(125, sDir);
	if(strlen(sDir) > 3) strcat(sDir, "\\");

	sprintf(sExe, "%sLITHTECH.EXE", sDir);

	if(ExistProcess(sExe, 1))
	{
		return FALSE;
	}

	// Set the CD-ROM root directory string...
	BOOL bBogus;
	m_csCDRoot.Format("%c:\\", CD_GetDriveLetterWithGame(bBogus));

	// Load the registry settings
	m_bInstalled = FALSE;
	m_RegMgr.Init();
	if(OpenRegistryKey())
	{
		char p[MAX_PATH];
		DWORD dwSize = sizeof(char) * MAX_PATH;
		memset(p,0,dwSize);
		if(m_RegMgr.GetField("InstallDir",p,dwSize) && p[0])
		{
			// We're already installed
			m_bInstalled = TRUE;
			m_csInstallDir = p;
		}

		memset(p,0,dwSize);
		if(m_RegMgr.GetField("ProfileName",p,dwSize) && p[0])
		{
			// We have already created a profile
			m_bHasProfile = TRUE;
		}

		m_dwLastRandom = GetRegistryDWord("Last Random");
		m_dwNumLauncherRuns = GetRegistryDWord("Num Launcher Runs");
		m_bDisableSound = GetRegistryBool("Disable Sound");
		m_bDisableMusic = GetRegistryBool("Disable Music");
		m_bDisableMovies = GetRegistryBool("Disable Movies");
		m_bDisableJoysticks = GetRegistryBool("Disable Joysticks");
		m_bDisableTripleBuffering = GetRegistryBool("Disable Triple Buffering");
		m_bDisableHardwareCursor = GetRegistryBool("Disable Hardware Cursor");
		m_bDisableHardwareSound = GetRegistryBool("Disable Hardware Sound");
		m_bDisableSoundFilters = GetRegistryBool("Disable Sound Filters");
		m_bEnableScreenshots = GetRegistryBool("Enable Screenshots");

		// Check for a command string
		m_bSaveCommands = GetRegistryBool("Save Commands");
		if(m_bSaveCommands)
		{
			dwSize = sizeof(char) * MAX_PATH;
			memset(p,0,dwSize);
			if(m_RegMgr.GetField("Commands",p,dwSize) && p[0])
			{
				m_csCommands = p;
			}
		}

#ifdef _FINAL
		//change to the install directory to find stuff
		if (m_bInstalled)
		{
			SetCurrentDirectory(m_csInstallDir);
		}
#endif

		// Find rez files
		FindRezFiles(CD_GetDriveLetterWithGame(bBogus),m_csRezBase);

		// Check for command line params
		CString csCmdLine = m_lpCmdLine;
		csCmdLine.MakeUpper();

/*
		if(csCmdLine.Find("-GOMULTIHOST") != -1)
			m_bGoMultiHost = TRUE;
		if(csCmdLine.Find("-GOMULTIJOIN") != -1)
			m_bGoMultiJoin = TRUE;
		if(csCmdLine.Find("-GOMULTIFIND") != -1)
			m_bGoMultiFind = TRUE;
		if(csCmdLine.Find("-GOSERVER") != -1)
			m_bGoServer = TRUE;
		if(csCmdLine.Find("-GOGAME") != -1)
			m_bGoGame = TRUE;
		// Check for GameSpy command line params
		if(csCmdLine.Find("CONNECT") != -1)
			m_bGoMultiHost = TRUE;
*/
		if(csCmdLine.Find("+JOIN ") != -1)
			m_bGoMultiJoin = TRUE;

		CString sModCommand = "+MOD ";
		int nIndex = csCmdLine.Find( sModCommand );
		if( nIndex != -1 )
		{
			CString sModName = csCmdLine.Right( csCmdLine.GetLength( ) - ( nIndex + sModCommand.GetLength( )));
			sModName.Trim( );
			int nEnd = sModName.Find( " " );
			if( nEnd == -1 )
				nEnd = sModName.GetLength( );
			sModName = sModName.Left( nEnd );
			theApp.m_RegMgr.SetField( FIELD_SELECTEDMOD, const_cast<char*>(( const char* )sModName ) );
		}
	}

	// Make our custom brush and font
	m_BkBrush.CreateSolidBrush((RGB(0,0,0)));

	LOGFONT lf;
	memset(&lf,0,sizeof(LOGFONT));
	lf.lfHeight = 20;
	strcpy(lf.lfFaceName,"Arial Narrow");
	m_Font.CreateFontIndirect(&lf);

	CLauncherDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();

	m_dwNumLauncherRuns++;

	// Write out some registry stuff
	if(m_bInstalled)
	{
		m_RegMgr.SetField("Num Launcher Runs", m_dwNumLauncherRuns);
		m_RegMgr.SetField("Last Random", m_dwLastRandom);
		m_RegMgr.SetField("Disable Sound", m_bDisableSound);
		m_RegMgr.SetField("Disable Music", m_bDisableMusic);
		m_RegMgr.SetField("Disable Movies", m_bDisableMovies);
		m_RegMgr.SetField("Disable Joysticks", m_bDisableJoysticks);
		m_RegMgr.SetField("Disable Triple Buffering", m_bDisableTripleBuffering);
		m_RegMgr.SetField("Disable Hardware Cursor", m_bDisableHardwareCursor);
		m_RegMgr.SetField("Disable Hardware Sound", m_bDisableHardwareSound);
		m_RegMgr.SetField("Disable Sound Filters", m_bDisableSoundFilters);
		m_RegMgr.SetField("Enable Screenshots", m_bEnableScreenshots);

		// Set the language - done by the installer
		//m_RegMgr.SetField("Language", (char *)(LPCTSTR)m_csLanguage);
	}

	// Clean up
	m_RegMgr.Term();
	m_BkBrush.DeleteObject();
	m_Font.DeleteObject();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::MessageBox
//
//	PURPOSE:	Graphical MessageBox function
//
// ----------------------------------------------------------------------- //
int CLauncherApp::MessageBox(const char *szText, const char *szCaption, DWORD dwType)
{
	CMessageBoxDlg dlg;
	dlg.Init(szText,szCaption,dwType);
	return dlg.DoModal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::MessageBox
//
//	PURPOSE:	Graphical MessageBox function
//
// ----------------------------------------------------------------------- //
int CLauncherApp::MessageBox(UINT nIDText, UINT nIDCaption, DWORD dwType)
{
	// Load the text and caption strings from the resource file
	CString csText,csCaption;
	if(!csText.LoadString(nIDText))
	{
		csText.Format("ERROR - Could not load string id: %d",nIDText);
		MessageBox(csText,"ERROR");
	}

	if(!csCaption.LoadString(nIDCaption))
	{
		csText.Format("ERROR - Could not load string id: %d",nIDCaption);
		MessageBox(csText,"ERROR");
	}
	
	return MessageBox(csText,csCaption,dwType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::GetRegistryBool
//
//	PURPOSE:	Gets a BOOL value from the registry under the
//				key that's already open
//
// ----------------------------------------------------------------------- //
BOOL CLauncherApp::GetRegistryBool(char *szField,BOOL bDefault /* = FALSE */)
{
	DWORD dwResult;
	if (m_RegMgr.GetField(szField,&dwResult))
	{
		return (dwResult == 1);
	}
	
	return bDefault;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::GetRegistryDWord
//
//	PURPOSE:	Gets a DWORD value from the registry under the
//				key that's already open
//
// ----------------------------------------------------------------------- //
DWORD CLauncherApp::GetRegistryDWord(char *szField, DWORD dwDefault /* = 0 */)
{
	DWORD dwResult = 0;
	if (m_RegMgr.GetField(szField,&dwResult))
	{
		return dwResult;
	}
	return dwDefault;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::CD_VerifyGame
//
//	PURPOSE:	Verifies that the correct CD is in the drive
//
// ----------------------------------------------------------------------- //
BOOL CLauncherApp::CD_VerifyGame()
{
	// Check if the game CD is already in the drive...
	BOOL bDisc2 = FALSE;
	{
		CWaitCursor wc;

		bool bCDExists = !!CD_ExistGame(bDisc2);
		if( bCDExists )
		{
			return(TRUE);
		}
	}

	// Ask the user to insert the game CD...
	BOOL bContinue = TRUE;
	while(bContinue)
	{
		DWORD dwMsg;
		if(bDisc2)
		{
			dwMsg = IDS_INSERTCD2;
		}
		else
		{
			dwMsg = IDS_INSERTCD;
		}

		int nRet = MessageBox(dwMsg, IDS_GAME_NAME, MB_OKCANCEL | MB_ICONEXCLAMATION);

		if(nRet == IDOK)
		{
			CWaitCursor wc;
			if(CD_ExistGame(bDisc2))
			{
				return(TRUE);
			}
		}
		else
		{
			return(FALSE);
		}
	}


	// If we get here, we couldn't verify the CD...
	return(FALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::CD_ExistGame
//
//	PURPOSE:	Looks for the game CD
//
// ----------------------------------------------------------------------- //
BOOL CLauncherApp::CD_ExistGame(BOOL & bDisc2)
{
	// Use the new function to search across multiple drives...
	char chDrive = CD_GetDriveLetterWithGame(bDisc2);

	// Update the rez files if necessary...
	char* sGameRez = GetGameRezFile();

	if(!sGameRez || sGameRez[0] == '\0')
	{
		FindRezFiles(chDrive,m_csRezBase);
	}

	// If we get here, it's the game cd...
	return(TRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::CD_ExistFile
//
//	PURPOSE:	Determines if the given file exists
//
// ----------------------------------------------------------------------- //
BOOL CLauncherApp::CD_ExistFile(const char* sFile)
{
	if(!sFile) return(FALSE);
	if(sFile[0] == '\0') return(FALSE);

	OFSTRUCT ofs;

	HFILE hFile = OpenFile(sFile, &ofs, OF_EXIST);

	return(hFile != HFILE_ERROR);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::CD_GetDriveLetter
//
//	PURPOSE:	Gets a CD ROM drive letter
//
// ----------------------------------------------------------------------- //
char CLauncherApp::CD_GetDriveLetter()
{
	// Look for a registry override setting...
	char sDir[256];
	char sDrive[32];
	DWORD bufsize = 30;

	sDrive[0] = '\0';
	if(m_RegMgr.GetField("CdRom Drive", sDrive, bufsize))
	{
		if(sDrive[0] > 20)
		{
			char cdDrive = sDrive[0];
			sprintf(sDir, "%c:\\", cdDrive);
			if(GetDriveType(sDir) == DRIVE_CDROM) return(cdDrive);
		}
	}

	// Try to find a CD ROM drive...
	GetCurrentDirectory(255, sDir);
	sDir[3] = '\0';

	if(GetDriveType(sDir) == DRIVE_CDROM)
	{
		return(sDir[0]);
	}
	else
	{
		GetLogicalDriveStrings(255,sDir);

		char *pDir = sDir;

		while (*pDir)
		{
			if(GetDriveType(pDir) == DRIVE_CDROM)
			{
				return(*pDir);
			}

			pDir++;
			if (pDir)
				pDir += 3;
		}
	}

	// If we get here, we didn't find one...
	return(0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::CD_GetDriveLetterWithGame
//
//	PURPOSE:	Gets the CD ROM drive letter with the game on it
//
// ----------------------------------------------------------------------- //

char CLauncherApp::CD_GetDriveLetterWithGame(BOOL & bDisc2)
{
	// Check if we've already found the cd-rom drive...
	if(m_chDrive != 0)
	{
		return(m_chDrive);
	}

	bDisc2 = FALSE;

	// Look for a registry override setting...
	char sDrive[32];
	DWORD bufsize = 30;

	sDrive[0] = '\0';
	if(m_RegMgr.GetField("CdRom Drive", sDrive, bufsize))
	{
		if(sDrive[0] > 20)
		{
			char chDrive = sDrive[0];
	
			char cTest = CD_TestDriveLetterForGameDisc1(chDrive, bDisc2);
			if(cTest)
			{
				return (cTest);
			}
		}
	}

	// Try to find a CD ROM drive with the game EXE on it...
	char sDir[256] = "";
	GetLogicalDriveStrings(255,sDir);

	char *pDir = sDir;

	while (*pDir)
	{
		if(GetDriveType(pDir) == DRIVE_CDROM)
		{
			char cTest = CD_TestDriveLetterForGameDisc1(*pDir, bDisc2);
			if(cTest)
			{
				return (cTest);
			}
		}
		
		pDir++;
		if (pDir)
			pDir += 3;
	}

	// If we get here, we didn't find one...
	return(0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::CD_TestDriveLetterForGameDisc1
//
//	PURPOSE:	Tests a potential CD ROM drive to see if it contains
//				the game disc 1
//
// ----------------------------------------------------------------------- //
char CLauncherApp::CD_TestDriveLetterForGameDisc1(char chDrive, BOOL & bDisc2)
{
	char sDir[256];
	sprintf(sDir, "%c:\\", chDrive);
	if(GetDriveType(sDir) == DRIVE_CDROM)
	{
		char sBuf[256];
		sprintf(sBuf, "%c:%s", chDrive, m_csAppCD1Check);

		if(CD_ExistFile(sBuf))
		{
			m_chDrive = chDrive;
			return(chDrive);
		}	
	}

	// Check and see if they left disk 2 in by mistake...
	if(CD_TestDriveLetterForGameDisc2(chDrive))
	{
		bDisc2 = TRUE;
	}

	return (0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::CD_TestDriveLetterForGameDisc2
//
//	PURPOSE:	Tests a potential CD ROM drive to see if it contains
//				the game disc 2
//
// ----------------------------------------------------------------------- //
char CLauncherApp::CD_TestDriveLetterForGameDisc2(char chDrive)
{
	char sDir[256];
	sprintf(sDir, "%c:\\", chDrive);
	if(GetDriveType(sDir) == DRIVE_CDROM)
	{
		char sBuf[256];
		sprintf(sBuf, "%c:%s", chDrive, m_csAppCD2Check);

		if(CD_ExistFile(sBuf))
		{
			return(chDrive);
		}	
	}

	return (0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherApp::GetAdvancedOptionsCommandLineString()
//
//	PURPOSE:	Assembles the command line string based on the
//				advanced options
//
// ----------------------------------------------------------------------- //
CString CLauncherApp::GetAdvancedOptionsCommandLineString()
{
	CString sAdv;

	if(m_bDisableMusic)
	{
		sAdv += " +DisableMusic 1";
	}
	else
	{
		sAdv += " +DisableMusic 0";
	}

	if(m_bDisableSound)
	{
		sAdv += " +DisableSound 1";
	}
	else
	{
		sAdv += " +DisableSound 0";
	}

	if(m_bDisableMovies)
	{
		sAdv += " +DisableMovies 1";
	}

	if(m_bDisableJoysticks)
	{
		sAdv += " +DisableJoystick 1";
	}

	if(m_bDisableTripleBuffering)
	{
		sAdv += " +DisableTripBuf 1";
	}
	else
	{
		sAdv += " +DisableTripBuf 0";
	}

	if(m_bDisableHardwareCursor)
	{
		sAdv += " +DisableHardwareCursor 1";
	}
	else
	{
		sAdv += " +DisableHardwareCursor 0";
	}
	


	if(m_bDisableHardwareSound)
	{
		sAdv += " +DisableHardwareSound 1";
	}
	else
	{
		sAdv += " +DisableHardwareSound 0";
	}

	if(m_bDisableSoundFilters)
	{
		sAdv += " +DisableSoundFilters 1";
	}
	else
	{
		sAdv += " +DisableSoundFilters 0";
	}

	if(m_bEnableScreenshots)
	{
		sAdv += " +lockablebackbuffer 1";
	}
	else
	{
		sAdv += " +lockablebackbuffer 0";
	}


	if(m_bRestoreDefaults)
	{
		sAdv += " +RestoreDefaults 1";
	}

	return(sAdv);
}


BOOL CLauncherApp::OpenRegistryKey()
{
	return(m_RegMgr.OpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE","Monolith Productions",(char*)GAME_NAME,(char *)(LPCTSTR)m_csAppVersion));
}

