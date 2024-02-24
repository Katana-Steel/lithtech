/*******************************************************************************
;
;	MODULE:			LAUNCHERDLG (.CPP)
;
;	PURPOSE:		Launcher main dialog class
;
;	HISTORY:		11/15/2000  [kml]  This file was created
;
;	COMMENT:		Copyright (c) 2000-2002, Monolith Inc.
;
********************************************************************************/

#include "stdafx.h"
#include "Launcher.h"
#include "LauncherDlg.h"
//#include "MultiplayerDlg.h"
#include "OptionsDlg.h"
#include "DisplayDlg.h"
#include "DisplayMgr.h"
#include "RezFind.h"
#include "DetailSettingsDlg.h"
#include "ButtonEx.h"
#include "PlaySound.h"
#include <direct.h>
#include "Utils.h"
#include "CustomizeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////
// Button and image coordinates
/////////////////////////////////////////////

#define LD_CLOSE_X			503
#define LD_CLOSE_Y			6
#define LD_MINIMIZE_X		487
#define LD_MINIMIZE_Y		6
#define LD_PLAY_X			413
#define LD_PLAY_Y			25
#define LD_SERVER_X			413
#define LD_SERVER_Y			61
#define LD_DISPLAY_X		413
#define LD_DISPLAY_Y		97
#define LD_OPTIONS_X		413
#define LD_OPTIONS_Y		133
#define LD_UNINSTALL_X		413
#define LD_UNINSTALL_Y		169
#define LD_QUIT_X			413
#define LD_QUIT_Y			205
#define LD_COMPANYWEB_X		242
#define LD_COMPANYWEB_Y		205
#define LD_LITHTECHWEB_X	334
#define LD_LITHTECHWEB_Y	188
#define LD_SIERRAWEB_X		10
#define LD_SIERRAWEB_Y		205

#define LD_CUSTOMIZE_X		413
#define LD_CUSTOMIZE_Y		61

//#define LD_ANIMAIN_X		43
//#define LD_ANIMAIN_Y		82
//#define LD_ANIBUTTONS_X		387
//#define LD_ANIBUTTONS_Y		19

extern CDisplayMgr* g_pDisplayMgr;

//this should match the value in ClientShell\ProfileMgr.h
const float	kDefaultGamma	= 1.04f;


/////////////////////////////////////////////////////////////////////////////
// CLauncherDlg dialog

CLauncherDlg::CLauncherDlg(CWnd* pParent /*=NULL*/)
	: CMoveDialog(CLauncherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLauncherDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CMoveDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLauncherDlg)
//	DDX_Control(pDX, IDC_ANIMATE2, m_aniButtons);
	//DDX_Control(pDX, IDC_ANIMATE, m_aniMain);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLauncherDlg, CMoveDialog)
	//{{AFX_MSG_MAP(CLauncherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CLOSE, OnClose)
	ON_BN_CLICKED(IDC_MINIMIZE, OnMinimize)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	ON_BN_CLICKED(IDC_DISPLAY, OnDisplay)
	ON_BN_CLICKED(IDC_UNINSTALL, OnUninstall)
	ON_BN_CLICKED(IDC_QUIT, OnQuit)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_PUBLISHERWEB, OnPublisherweb)
	ON_BN_CLICKED(IDC_COMPANYWEB, OnCompanyWeb)
	ON_BN_CLICKED(IDC_LITHTECHWEB, OnLithtechWeb)
	ON_BN_CLICKED(IDC_SIERRAWEB, OnSierraWeb)
	ON_BN_CLICKED(IDC_CUSTOM, OnCustomize)
	//}}AFX_MSG_MAP
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnInitDialog
//
//	PURPOSE:	Where it all happens baby!
//
// ----------------------------------------------------------------------- //

BOOL CLauncherDlg::OnInitDialog()
{
	//***************************************************
	// Do this BEFORE calling CMoveDialog::OnInitDialog()
	//
	SetBackGround(IDB_BACKGROUND);

	//***************************************************

	CMoveDialog::OnInitDialog();

	CenterWindow();

	// Here's where we define what buttons go in our dialog
	AddButton(IDC_CLOSE,LD_CLOSE_X,LD_CLOSE_Y);
	AddButton(IDC_MINIMIZE,LD_MINIMIZE_X,LD_MINIMIZE_Y);
//	AddButton(IDC_SERVER,LD_SERVER_X,LD_SERVER_Y);
	AddButton(IDC_DISPLAY,LD_DISPLAY_X,LD_DISPLAY_Y);
	AddButton(IDC_OPTIONS,LD_OPTIONS_X,LD_OPTIONS_Y);
	AddButton(IDC_UNINSTALL,LD_UNINSTALL_X,LD_UNINSTALL_Y);
	AddButton(IDC_QUIT,LD_QUIT_X,LD_QUIT_Y);
	AddButton( IDC_COMPANYWEB, LD_COMPANYWEB_X, LD_COMPANYWEB_Y, IDC_WEBHAND );
//	AddButton( IDC_LITHTECHWEB, LD_LITHTECHWEB_X, LD_LITHTECHWEB_Y, IDC_WEBHAND );
	AddButton( IDC_SIERRAWEB, LD_SIERRAWEB_X, LD_SIERRAWEB_Y, IDC_WEBHAND );
	
	// Check for the custom directory before creating this button...
	
	if( DirExist( DIR_MODS ))
	{
		char szFiles[MAX_PATH] = {0};
		sprintf( szFiles, "%s\\*", DIR_MODS);

		CFileFind	cFileFinder;

		BOOL bModFound = false;
		BOOL bDirFound = cFileFinder.FindFile( szFiles );
		while( bDirFound )
		{
			bModFound = true;
			bDirFound = cFileFinder.FindNextFile();

			// Ignore the 'dots' directories and all files...

			if( !cFileFinder.IsDirectory() || cFileFinder.IsDots() )
			{
				bModFound = false;
			}

			if( bModFound )
				break;
		}

		// End the search...
		
		cFileFinder.Close();
		
		if( bModFound )
			AddButton( IDC_CUSTOM, LD_CUSTOMIZE_X, LD_CUSTOMIZE_Y );
	}

	// Figure out if we need to load the "Play" button or the "Install" button
	if(theApp.m_bInstalled)
	{
		CWnd* pWnd = GetDlgItem(IDC_PLAY);
		if(pWnd)
		{
			pWnd->SetWindowText("Play");
		}

		// Check for command-line params that automatically launch the game
		if(theApp.m_bGoMultiHost)
		{
			if(LaunchGame(ACTION_HOSTGAME))
			{
				EndDialog(ACTION_HOSTGAME);
				return TRUE;
			}
		}
		if(theApp.m_bGoMultiJoin)
		{
			if(LaunchGame(ACTION_JOINGAME))
			{
				EndDialog(ACTION_JOINGAME);
				return TRUE;
			}
		}
		if(theApp.m_bGoMultiFind)
		{
			if(LaunchGame(ACTION_FINDINTERNETSERVERS))
			{
				EndDialog(ACTION_FINDINTERNETSERVERS);
				return TRUE;
			}
		}
		if(theApp.m_bGoGame)
		{
			if(LaunchGame(ACTION_PLAY))
			{
				EndDialog(ACTION_PLAY);
				return TRUE;
			}
		}
		if(theApp.m_bGoServer)
		{
			if(LaunchGame(ACTION_STANDALONESERVER))
			{
				EndDialog(ACTION_STANDALONESERVER);
				return TRUE;
			}
		}
	}
	else
	{
		//GetDlgItem(IDC_MULTIPLAYER)->EnableWindow(FALSE);
//		GetDlgItem(IDC_SERVER)->EnableWindow(FALSE);
		GetDlgItem(IDC_DISPLAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_OPTIONS)->EnableWindow(FALSE);
		GetDlgItem(IDC_UNINSTALL)->EnableWindow(FALSE);
	}

	// Make sure the play button is default.
	SetDefID( IDC_PLAY );

	// We add this guy last because the images loaded by this function are
	// dependent on the text of the button -- which may have been altered above
	AddButton(IDC_PLAY,LD_PLAY_X,LD_PLAY_Y);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// Set window title
	SetWindowText(theApp.m_csAppName);

	// Set up our animation control
/*	if(!m_aniMain.Open(IDR_AVI_LOGO1))
	{
		MessageBox("AVI Open Error!");
	}
	m_aniMain.SetWindowPos(NULL,LD_ANIMAIN_X,LD_ANIMAIN_Y,0,0,SWP_NOZORDER | SWP_NOSIZE);
	m_aniMain.Play(0,-1,1);
	if(!m_aniButtons.Open(IDR_AVI_BUTTONS))
	{
		// Couldn't open the avi
		theApp.MessageBox(IDS_CANTOPENAVI, IDS_GAME_NAME, MB_OK | MB_ICONSTOP);
	}
	m_aniButtons.SetWindowPos(NULL,LD_ANIBUTTONS_X,LD_ANIBUTTONS_Y,0,0,SWP_NOZORDER | SWP_NOSIZE);
	m_aniButtons.Play(0,-1,-1);
*/

	// Play the intro sound
	PlaySound(IDR_INTRO);
		
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnPaint
//
//	PURPOSE:	WM_PAINT handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	//***************************************************
	// Call this BEFORE doing any painting on the dialog!
	//
	PaintBackGround(&dc);
	//***************************************************

	if (IsIconic())
	{
		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CMoveDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLauncherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnClose
//
//	PURPOSE:	Close button handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnClose() 
{
	// Close us
	OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnMinimize
//
//	PURPOSE:	Minimize button handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnMinimize() 
{
	// Minimize	us
	ShowWindow(SW_MINIMIZE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnPlay
//
//	PURPOSE:	Play/Install button handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnPlay() 
{
	// Launch the game or the installer
	if(theApp.m_bInstalled && !ExistProcess(theApp.m_csSetupExe, 2))
	{
		if(theApp.CD_VerifyGame())
		{
			if(LaunchGame(ACTION_PLAY))
			{
				EndDialog(ACTION_PLAY);
			}
		}
	}
	else
	{

#ifndef _FINAL
#define SELFINSTALL
#endif // _FINAL

#ifdef SELFINSTALL

		// In the debug version, don't actually launch the installer
		// just set up the registry
		CRegMgr32* pReg = &theApp.m_RegMgr;
		if(!pReg->CreateKey(HKEY_LOCAL_MACHINE,"SOFTWARE","Monolith Productions",(char *)GAME_NAME,(char *)(LPCTSTR)theApp.m_csAppVersion))
		{
			if(!pReg->OpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE","Monolith Productions",(char *)GAME_NAME,(char *)(LPCTSTR)theApp.m_csAppVersion))
			{
				theApp.MessageBox( IDS_DEBUG_REGCREATEERROR, IDS_GAME_NAME, MB_OK | MB_ICONSTOP );
				return;
			}
		}

		// Get the current directory
		char sDir[256];
		GetCurrentDirectory(255, sDir);
		CString csDir = sDir;
		if(csDir[csDir.GetLength()-1] != '\\')
		{
			csDir += '\\';
		}
		
		// Set the "InstallDir" field with the current directory just for shits...
		if(pReg->SetField("InstallDir",(char *)(LPCTSTR)csDir))
		{
			theApp.MessageBox( IDS_DEBUG_INSTALLSUCCESS, IDS_GAME_NAME, MB_OK | MB_ICONINFORMATION);
			//GetDlgItem(IDC_SERVER)->EnableWindow(TRUE);
			
			GetDlgItem(IDC_DISPLAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_OPTIONS)->EnableWindow(TRUE);
			GetDlgItem(IDC_UNINSTALL)->EnableWindow(TRUE);

			theApp.m_bInstalled = TRUE;
			Invalidate();

			// Swap the install button to say "Play"
			CButtonEx* pWnd = (CButtonEx*)GetDlgItem(IDC_PLAY);
			if(pWnd)
			{
				pWnd->LoadBitmaps("PlayU","PlayD","PlayF","PlayX");
				pWnd->Invalidate();
			}
		}
		else
		{
			theApp.MessageBox( IDS_DEBUG_REGCREATEERROR, IDS_GAME_NAME, MB_OK | MB_ICONSTOP );
		}

#else  // Retail install

		if(WinExec(theApp.m_csSetupExe,SW_SHOW) > 31)
		{
			// Just let it happen...
//			EndDialog(ACTION_INSTALL);

			// Okay, they're installing so we're done.  Force them to relaunch
			// the launcher since they really shouldn't be playing the game from
			// here anyway... :)

			OnQuit();
		}
		else
		{
			// Couldn't find the setup exe
			theApp.MessageBox(IDS_CANTLAUNCHSETUP, IDS_GAME_NAME, MB_OK | MB_ICONSTOP);
		}
#endif
	}
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnMultiplayer
//
//	PURPOSE:	Multiplayer button handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnMultiplayer() 
{
	// Get the button so we know where to position ourselves
	CWnd* wnd = GetDlgItem(IDC_MULTIPLAYER);
	
	// Get coordinates
	CRect rect;
	wnd->GetWindowRect(&rect);

	CPoint point;
	point.x = rect.right;
	point.y = rect.top;

	// Bring up the multiplayer dialog
	CMultiplayerDlg dlg;
	dlg.SetDialogPos(point.x,point.y);
	dlg.SetAnimTime(200);
	dlg.SetAnimSound(IDR_ANIMDLG);
	int nResponse = dlg.DoModal();
	switch(nResponse)
	{
		case ACTION_HOSTGAME:
		case ACTION_JOINGAME:
		case ACTION_FINDINTERNETSERVERS:
		case ACTION_STANDALONESERVER:
		{
			// Launch the game
			if(LaunchGame(nResponse))
			{
				EndDialog(nResponse);
			}
			break;
		}
	}
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnOptions
//
//	PURPOSE:	Options button handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnOptions() 
{
	// See if we need to display the warning for dumbass users
	if(!theApp.GetRegistryBool("OptionsWarning"))
	{
		if(theApp.MessageBox( IDS_OPTIONS_WARNING, IDS_GAME_NAME, MB_OKCANCEL | MB_ICONEXCLAMATION ) == IDCANCEL)
		{
			return;
		}

		theApp.m_RegMgr.SetField("OptionsWarning",1);
	}

	COptionsDlg dlg;
	dlg.DoModal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnDisplay
//
//	PURPOSE:	Display button handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnDisplay() 
{
	// See if we need to display the warning for complete retard users
	if(!theApp.GetRegistryBool("DisplayWarning"))
	{
		if(theApp.MessageBox( IDS_DISPLAY_WARNING, IDS_GAME_NAME, MB_OKCANCEL | MB_ICONEXCLAMATION ) == IDCANCEL)
		{
			return;
		}

		theApp.m_RegMgr.SetField("DisplayWarning",1);
	}

	CDisplayDlg dlg;
	if(dlg.DoModal() == IDOK)
	{
		theApp.m_bUseDisplaySettings = TRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnUninstall
//
//	PURPOSE:	Uninstall button handler
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnUninstall() 
{

#ifndef _FINAL

	// In a debug build, just remove the registry key
	CRegMgr32* pReg = &theApp.m_RegMgr;
	pReg->DeleteKey(HKEY_LOCAL_MACHINE,"SOFTWARE","Monolith Productions",(char*)GAME_NAME,(char *)(LPCTSTR)theApp.m_csAppVersion);
	theApp.MessageBox( IDS_DEBUG_UNINSTALLSUCCESS, IDS_GAME_NAME, MB_OK | MB_ICONINFORMATION);
	//GetDlgItem(IDC_MULTIPLAYER)->EnableWindow(FALSE);
	//GetDlgItem(IDC_SERVER)->EnableWindow(FALSE);
	GetDlgItem(IDC_DISPLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_OPTIONS)->EnableWindow(FALSE);
	GetDlgItem(IDC_UNINSTALL)->EnableWindow(FALSE);
	theApp.m_bInstalled = FALSE;
	Invalidate();

	// Swap the play button to say "Install"
	CButtonEx* pWnd = (CButtonEx*)GetDlgItem(IDC_PLAY);
	if(pWnd)
	{
		pWnd->LoadBitmaps("InstallU","InstallD","InstallF","InstallX");
		pWnd->Invalidate();
	}

#else

	// Make sure we use the install path so we can uninstall from anywhere...

	CRegMgr32* pReg = &theApp.m_RegMgr;
	if(pReg->OpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE","Monolith Productions",( char* )GAME_NAME,(char *)(LPCTSTR)theApp.m_csAppVersion))
	{
		// Get the installed directory
		char sDir[256];
		unsigned long nSize = sizeof(sDir);
		if(pReg->GetField("InstallDir",sDir,nSize))
		{
			CString csParam = ( sDir[0] ? 
								CString(sDir) + CString("\\setup.exe") :
								CString("setup.exe") );

			// Launch the uninstaller and exit our app
			STARTUPINFO			si;
			PROCESS_INFORMATION pi;
			GetStartupInfo(&si);
			if (CreateProcess(NULL, (char *)(LPCTSTR)csParam, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP,	NULL, NULL, &si, &pi))
			{
				EndDialog(ACTION_UNINSTALL);
				return;
			}
		}
	}

	// Couldn't uninstall the game...

	theApp.MessageBox(IDS_CANTUNINSTALL, IDS_GAME_NAME, MB_OK | MB_ICONSTOP);

#endif
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLauncherDlg::OnCompanyWeb
//
//  PURPOSE:	Bring the user to www.lith.com
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnCompanyWeb()
{
	CString	sURL;
	sURL.LoadString( IDS_COMPANYWEBPAGE );

	if ((UINT)ShellExecute(NULL, NULL, sURL, NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		theApp.MessageBox(IDS_LAUNCHBROWSERERROR, IDS_GAME_NAME , MB_OK | MB_ICONEXCLAMATION);
	}

}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLauncherDlg::OnPublisherweb
//
//  PURPOSE:	Bring the user to the publishers web page
//
// ----------------------------------------------------------------------- //
void CLauncherDlg::OnPublisherweb() 
{
	CString	sURL;
	sURL.LoadString( IDS_PUBLISHERWEBPAGE );

	if ((UINT)ShellExecute(NULL, NULL, sURL, NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		theApp.MessageBox(IDS_LAUNCHBROWSERERROR, IDS_GAME_NAME , MB_OK | MB_ICONEXCLAMATION);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLauncherDlg::OnLithtechWeb
//
//  PURPOSE:	Bring the user to the lithtech web page
//
// ----------------------------------------------------------------------- //
void CLauncherDlg::OnLithtechWeb() 
{
	CString	sURL;
	sURL.LoadString( IDS_LITHTECHWEBPAGE );

	if ((UINT)ShellExecute(NULL, NULL, sURL, NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		theApp.MessageBox(IDS_LAUNCHBROWSERERROR, IDS_GAME_NAME , MB_OK | MB_ICONEXCLAMATION);
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLauncherDlg::OnSierraWeb
//
//  PURPOSE:	Bring the user to the other publishers web page
//
// ----------------------------------------------------------------------- //
void CLauncherDlg::OnSierraWeb() 
{
	CString	sURL;
	sURL.LoadString( IDS_SIERRAWEBPAGE );

	if ((UINT)ShellExecute(NULL, NULL, sURL, NULL, NULL, SW_SHOWNORMAL) <= 32)
	{
		theApp.MessageBox(IDS_LAUNCHBROWSERERROR, IDS_GAME_NAME , MB_OK | MB_ICONEXCLAMATION);
	}
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLauncherDlg::OnServer
//
//  PURPOSE:	Launches the dedicated server app
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnServer()
{
	if( LaunchGame( ACTION_STANDALONESERVER ) )
	{
		EndDialog( ACTION_STANDALONESERVER );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnQuit
//
//	PURPOSE:	I'll give ya one big fat freakin guess.
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnQuit() 
{
	OnCancel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnCommand
//
//	PURPOSE:	Command handler
//
// ----------------------------------------------------------------------- //

BOOL CLauncherDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
//	if((LOWORD(wParam) == IDC_ANIMATE) && (HIWORD(wParam) == ACN_STOP))
//	{
//		m_aniMain.Close();
//		m_aniMain.Open(IDR_AVI_LOGO2);
//		m_aniMain.Play(0,-1,-1);
//	}
	return CMoveDialog::OnCommand(wParam, lParam);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnDestroy
//
//	PURPOSE:	Clean-up time... bitch.
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnDestroy() 
{
	CMoveDialog::OnDestroy();
	if(g_pDisplayMgr)
	{
		delete g_pDisplayMgr;
		g_pDisplayMgr = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::LaunchGame
//
//	PURPOSE:	This is the entire purpose of this godforsaken app.
//
// ----------------------------------------------------------------------- //

BOOL CLauncherDlg::LaunchGame(DWORD dwAction)
{
	if(dwAction == ACTION_STANDALONESERVER)
	{
		if(WinExec(theApp.m_csServerExe,SW_SHOW) > 31)
		{
			// Just let it happen...
			return TRUE;
		}
		else
		{
			theApp.MessageBox(IDS_CANTLAUNCHSERVER, IDS_GAME_NAME, MB_OK | MB_ICONSTOP);
			return FALSE;
		}
	}

	// See if we need to bring up the detail settings dialog box
	if((!theApp.m_bHasProfile) || (theApp.m_bRestoreDefaults))
	{
		CDetailSettingsDlg dlg;
		int nRet = dlg.DoModal();
		if(nRet == IDCANCEL)
		{
			return FALSE;
		}
		else
		{
			theApp.m_bUseDetailSettings = TRUE;
		}
	}

	char* sGameRez = GetGameRezFile();
	char* sSoundRez = GetSoundRezFile();

	if (!sGameRez || sGameRez[0] == '\0')
	{
		theApp.MessageBox(IDS_CANTFINDREZFILE, IDS_GAME_NAME, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	// Get the install path from the registry...

	CRegMgr32* pReg = &theApp.m_RegMgr;
	if(!pReg->OpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE","Monolith Productions",( char* )GAME_NAME,(char *)(LPCTSTR)theApp.m_csAppVersion))
	{
		theApp.MessageBox( IDS_CANTLAUNCHCLIENTEXE, IDS_GAME_NAME, MB_OK | MB_ICONSTOP );
		return FALSE;
	}

	// Get the installed directory
	char sDir[256];
	unsigned long nSize = sizeof(sDir);
	if(!pReg->GetField("InstallDir",sDir,nSize))
	{
		theApp.MessageBox( IDS_CANTLAUNCHCLIENTEXE, IDS_GAME_NAME, MB_OK | MB_ICONSTOP );
		return FALSE;
	}

#ifdef _FINAL
	SetCurrentDirectory(sDir);
#endif

	CString sNewCmdLine;

	// Set the window title...

	sNewCmdLine += " -windowtitle \"";
	sNewCmdLine += theApp.m_csAppName;
	sNewCmdLine += "\"";

	// Set rez files...

	// In order to ensure propper use of user created .rez files that are content
	// only, such as a map pack, specify the .rez files in the resources directory 
	// first.  Then our retail game .rez files and finialy the .rez files for the 
	// selected mod, if one was specified.

	
	// Add any user created .rez files found in the resources directory...

	char szFiles[MAX_PATH] = {0};
	sprintf( szFiles, "%s\\*.rez", DIR_RESOURCES);

	CFileFind	cFileFinder;

	BOOL bFound = cFileFinder.FindFile( szFiles );
	while( bFound )
	{
		bFound = cFileFinder.FindNextFile();

		// Ignore all directories...

		if( cFileFinder.IsDirectory() || cFileFinder.IsDots() )
			continue;
		
		// Add the .rez file to the command line...
		
		sNewCmdLine += " -rez ";
		sNewCmdLine += DIR_RESOURCES;
		sNewCmdLine += "\\";
		sNewCmdLine += cFileFinder.GetFileName();
	}

	// End the search...
	
	cFileFinder.Close();

	// Add our own retail game .rez files...

	sNewCmdLine += " -rez ";
	sNewCmdLine += sGameRez;

	if (sSoundRez && sSoundRez[0] != '\0')
	{
		sNewCmdLine += " -rez ";
		sNewCmdLine += sSoundRez;
	}

	sNewCmdLine += " -rez ";	// " -rez <game>2.rez"	(Disk 2 rez file)
	sNewCmdLine += theApp.m_csRezBase;
	sNewCmdLine += "2.rez";

	sNewCmdLine += " -rez ";	// " -rez <game>dll.rez" (DLL rez file)
	sNewCmdLine += theApp.m_csRezBase;
	sNewCmdLine += "dll.rez";

	sNewCmdLine += " -rez ";	// " -rez Sound.rez"	(sound rez file)
	sNewCmdLine += "Sound.rez";

	sNewCmdLine += " -rez ";	// " -rez <game>l.rez"  (language rez file)
	sNewCmdLine += theApp.m_csRezBase;
	sNewCmdLine += "l.rez";

	sNewCmdLine += " -rez custom";

	sNewCmdLine += " -rez ";	// " -rez <game>p.rez"	(patch rez file)
	sNewCmdLine += theApp.m_csRezBase;
	sNewCmdLine += "p.rez";

	sNewCmdLine += " -rez ";	// " -rez <game>p2.rez"  (patch2 rez file)
	sNewCmdLine += theApp.m_csRezBase;
	sNewCmdLine += "p2.rez";

	// Get the number of content addons installed.
	DWORD nContentNum = 0;
	if( pReg->GetField( "ContentNum", &nContentNum ))
	{
		char szCommandLineKey[256] = "";

		// Loop through each one, adding the newer, higher numbered ones, to the end
		// so they override the older ones.
		for( DWORD i = 0; i < nContentNum; i++ )
		{
			// Add any command line stuff added by an Update
			char szCommandLine[256] = "";
			sprintf( szCommandLineKey, "ContentCommandLine%d", i );
			DWORD nBufSize = sizeof( szCommandLine );
			if( pReg->GetField( szCommandLineKey, szCommandLine, nBufSize ) && 
				szCommandLine[0] )
			{
				sNewCmdLine += " ";
				sNewCmdLine += szCommandLine;
			}
		}
	}

	// Get the number of updates installed.  Make sure to do this after the content
	// addons so they override anything.
	DWORD nUpdateNum = 0;
	if( pReg->GetField( "UpdateNum", &nUpdateNum ))
	{
		char szCommandLineKey[256] = "";

		// Loop through each one, adding the newer, higher numbered ones, to the end
		// so they override the older ones.
		for( DWORD i = 0; i < nUpdateNum; i++ )
		{
			// Add any command line stuff added by an Update
			char szCommandLine[256] = "";
			sprintf( szCommandLineKey, "UpdateCommandLine%d", i );
			DWORD nBufSize = sizeof( szCommandLine );
			if( pReg->GetField( szCommandLineKey, szCommandLine, nBufSize ) && 
				szCommandLine[0] )
			{
				sNewCmdLine += " ";
				sNewCmdLine += szCommandLine;
			}
		}
	}

	// Now add all the .rez files associated with the selected mod, if we have one...

	char szSelectedMod[256] = {0};
	DWORD bufSize = sizeof(szSelectedMod);
	
	if( pReg->GetField( FIELD_SELECTEDMOD, szSelectedMod, bufSize ))
	{
		if( szSelectedMod[0] )
		{
			char szModDir[MAX_PATH] = {0};
			sprintf( szModDir, "%s\\%s", DIR_MODS, szSelectedMod );
			
			// Make sure the mod is still there...

			if( DirExist( szModDir ))
			{	
				// Set the mod name...

				sNewCmdLine += " +mod ";
				sNewCmdLine += szSelectedMod;

				// Search for all .rez files within the selected mod directory...

				sprintf( szFiles, "%s\\%s\\*.rez", DIR_MODS, szSelectedMod );
				
				BOOL bFound = cFileFinder.FindFile( szFiles );
				while( bFound )
				{
					bFound = cFileFinder.FindNextFile();

					// Ignore all directories...

					if( cFileFinder.IsDirectory() || cFileFinder.IsDots() )
						continue;
					
					// Add the .rez file to the command line...
					
					sNewCmdLine += " -rez ";
					sNewCmdLine += DIR_MODS;
					sNewCmdLine += "\\";
					sNewCmdLine += szSelectedMod;
					sNewCmdLine += "\\";
					sNewCmdLine += cFileFinder.GetFileName();
				}

				// End the search...
				
				cFileFinder.Close();
			}
			else
			{
				pReg->SetField( FIELD_SELECTEDMOD, "" );
			}
		}
	}


	// Set multiplayer stuff...
	switch(dwAction)
	{
		case ACTION_PLAY:
		{
			sNewCmdLine += " +multiplayer 0";
			break;
		}
		case ACTION_HOSTGAME:
		{
			sNewCmdLine += " +multiplayer 1";
			break;
		}
		case ACTION_JOINGAME:
		{
			sNewCmdLine += " +multijoin 1";
			break;
		}
		case ACTION_FINDINTERNETSERVERS:
		{
			sNewCmdLine += " +multifind 1";
			break;
		}
	}


	// Add the advanced options stuff as necessary...

	CString sAdv = theApp.GetAdvancedOptionsCommandLineString();

	if (!sAdv.IsEmpty())
	{
		sNewCmdLine += " ";
		sNewCmdLine += sAdv;
	}


	// Add display settings to the command-line if requested...

	if (theApp.m_bUseDisplaySettings && g_pDisplayMgr)
	{
		CString sDisplay = g_pDisplayMgr->GetDisplaySettingsString();

		if (!sDisplay.IsEmpty())
		{
			sNewCmdLine += " ";
			sNewCmdLine += sDisplay;
		}
	}


	if (theApp.m_bRestoreDefaults)
	{
		char szGamma[8] = "";
		sprintf(szGamma,"%0.2f",kDefaultGamma);
		sNewCmdLine += " +HardwareCursor 1";
		sNewCmdLine += " +VSyncOnFlip 1";
		sNewCmdLine += " +GammaR ";
		sNewCmdLine += szGamma;
		sNewCmdLine += " +GammaG ";
		sNewCmdLine += szGamma;
		sNewCmdLine += " +GammaB ";
		sNewCmdLine += szGamma;
	}



	// Add the 3D sound setting if necessary...

	/*CString sSnd = SndDlg_GetCommandLineParameters();

	if (!sSnd.IsEmpty())
	{
		sNewCmdLine += " ";
		sNewCmdLine += sSnd;
	}*/


	// Add any additional rez files...

	/*CString sAddRez = GetRezStringCommandLine();

	if (!sAddRez.IsEmpty())
	{
		sNewCmdLine += sAddRez;
	}*/


	// Add the detail setting if necessary...

	if (theApp.m_bUseDetailSettings)
	{
		CString sDetail = "";

		switch (theApp.m_dwDetailSetting)
		{
		case DETAIL_HIGH:
			//high detail == low performance
			sDetail = " +SetPerformanceLevel .DefaultLow";
			if (!theApp.m_bUseDisplaySettings)
			{
				sDetail += " +ScreenWidth 1024 +ScreenHeight 768 +BitDepth 32";
			}
			break;
		case DETAIL_MEDIUM:
			//high detail == low performance
			sDetail = " +SetPerformanceLevel .DefaultMid"; 
			if (!theApp.m_bUseDisplaySettings)
			{
				sDetail += " +ScreenWidth 800 +ScreenHeight 600 +BitDepth 32";
			}
			break;
		case DETAIL_LOW:
			//high detail == low performance
			sDetail = " +SetPerformanceLevel .DefaultHigh"; 
			if (!theApp.m_bUseDisplaySettings)
			{
				sDetail += " +ScreenWidth 640 +ScreenHeight 480 +BitDepth 32";
			}
			break;
		}


		sNewCmdLine += sDetail;
	}

	// Add the run/start specified command-line parameters...

	sNewCmdLine += " ";
	sNewCmdLine += theApp.m_lpCmdLine;


	// Add the user specified command-line parameters...

	sNewCmdLine += " ";
	sNewCmdLine += theApp.m_csCommands;


	// In debug builds, check if we should display the command line string...

	if (GetAsyncKeyState(VK_SHIFT) & 0x80000000)
	{
		int nRet = theApp.MessageBox(sNewCmdLine, "Command Line String", MB_OKCANCEL);
		if (nRet == IDCANCEL)
			return(FALSE);
	}

	// Time to run
	
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	GetStartupInfo(&si);

	//save it out to a file
	FILE* fp = fopen("launchcmds.txt", "w");
	if ( !fp ) 
	{
		theApp.MessageBox(IDS_CANTOPENCOMMANDFILE, IDS_GAME_NAME, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	fprintf(fp, sNewCmdLine);
	fclose(fp);

	if (!CreateProcess(NULL, "lithtech.exe -cmdfile launchcmds.txt", NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP,	NULL, NULL, &si, &pi))
	{
		theApp.MessageBox(IDS_CANTLAUNCHCLIENTEXE, IDS_GAME_NAME, MB_OK | MB_ICONEXCLAMATION);
		return(FALSE);
	}

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLauncherDlg::OnCustomize
//
//	PURPOSE:	Launch the customize dialog...
//
// ----------------------------------------------------------------------- //

void CLauncherDlg::OnCustomize( )
{
	if( !DirExist( DIR_MODS ))
	{
		theApp.MessageBox( IDS_NOCUSTOMDIR, IDS_GAME_NAME, MB_OK | MB_ICONEXCLAMATION );
		return;
	}

	CustomizeDlg dlg;
	dlg.DoModal();
}
HBRUSH CLauncherDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CMoveDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// Change dialog colors
	pDC->SetBkColor( RGB(0, 0, 0));
	pDC->SetTextColor( RGB(255, 255, 255));

	return hbr;
}
