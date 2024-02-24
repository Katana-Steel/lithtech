# Microsoft Developer Studio Project File - Name="ClientShellDLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ClientShellDLL - Win32 Demo Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ClientShellDLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ClientShellDLL.mak" CFG="ClientShellDLL - Win32 Demo Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ClientShellDLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Demo Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Final" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Source/ClientShellDLL", FFAAAAAA"
# PROP Scc_LocalPath "."
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /I "$(LT2_DIR)\lithshared\wonapi" /D "NDEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\CShell.dll"
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
InputPath=.\release\CShell.dll
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy release\cshell.dll $(AVP2_BUILD_DIR)\cshell.dll

# End Custom Build

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /I "$(LT2_DIR)\lithshared\wonapi" /D "_DEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"debug\CShell.dll" /pdbtype:sept /libpath:"$(LT2_DIR)\lithshared\libs\debug\\"
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
InputPath=.\debug\CShell.dll
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy debug\cshell.dll $(AVP2_BUILD_DIR)\cshell.dll

# End Custom Build

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ClientShellDLL___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "ClientShellDLL___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ClientShellDLL___Win32_Demo_Release"
# PROP Intermediate_Dir "ClientShellDLL___Win32_Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "NDEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP3BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "NDEBUG" /D "_DEMO" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /D "_FINAL" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Demo_Release/ClientShellDLL.bsc"
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\CShell.dll"
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:"Demo_Release/CShell.pdb" /machine:I386 /out:"demo\release\CShell.dll"
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
InputPath=.\demo\release\CShell.dll
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy demo\release\cshell.dll $(AVP2_BUILD_DIR)\cshell.dll

# End Custom Build

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ClientShellDLL___Win32_Demo_Debug"
# PROP BASE Intermediate_Dir "ClientShellDLL___Win32_Demo_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ClientShellDLL___Win32_Demo_Debug"
# PROP Intermediate_Dir "ClientShellDLL___Win32_Demo_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "_DEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP3BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FR /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "_DEBUG" /D "_DEMO" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /D "_FINAL" /FR"Demo_Debug/" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Demo_Debug/ClientShellDLL.bsc"
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"debug\CShell.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:"Demo_Debug/CShell.pdb" /debug /machine:I386 /out:"demo\debug\CShell.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
InputPath=.\demo\debug\CShell.dll
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy demo\debug\cshell.dll $(AVP2_BUILD_DIR)\cshell.dll

# End Custom Build

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ClientShellDLL___Win32_Final"
# PROP BASE Intermediate_Dir "ClientShellDLL___Win32_Final"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ClientShellDLL___Win32_Final"
# PROP Intermediate_Dir "ClientShellDLL___Win32_Final"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /I "$(LT2_DIR)\lithshared\wonapi" /D "NDEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ClientRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /I "$(LT2_DIR)\lithshared\wonapi" /D "_FINAL" /D "NDEBUG" /D "_CLIENTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Final/ClientShellDLL.bsc"
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\CShell.dll"
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:"Final/CShell.pdb" /machine:I386 /out:"final\CShell.dll"
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
InputPath=.\final\CShell.dll
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\cshell.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy final\cshell.dll $(AVP2_BUILD_DIR)\cshell.dll

# End Custom Build

!ENDIF 

# Begin Target

# Name "ClientShellDLL - Win32 Release"
# Name "ClientShellDLL - Win32 Debug"
# Name "ClientShellDLL - Win32 Demo Release"
# Name "ClientShellDLL - Win32 Demo Debug"
# Name "ClientShellDLL - Win32 Final"
# Begin Group "Source"

# PROP Default_Filter "*.cpp"
# Begin Group "Interface Source"

# PROP Default_Filter ""
# Begin Group "Menu Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BaseFolder.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderCampaignLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderCredits.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderCustomControls.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderCustomLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderEscape.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderGame.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderHost.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderHostConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderHostMaps.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderHostOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderJoin.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderJoinInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderJoystick.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderMain.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderMouse.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderMulti.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderPerformance.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderPlayerJoin.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSave.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSetupDM.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSetupEvac.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSetupHunt.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSetupOverrun.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSetupSurvivor.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSetupTeamDM.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderSingle.cpp
# End Source File
# Begin Source File

SOURCE=.\FrameCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageBoxCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\SliderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticTextCtrl.cpp
# End Source File
# End Group
# Begin Group "Managers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AutoTargetMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CrosshairMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\HudMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\InterfaceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\InterfaceResMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\InterfaceSurfMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\InterfaceTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\JoystickAxis.cpp
# End Source File
# Begin Source File

SOURCE=.\LayoutMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\PerformanceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ProfileMgr.cpp
# End Source File
# End Group
# Begin Group "HUD Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MotionDetector.cpp
# End Source File
# Begin Source File

SOURCE=.\Subtitle.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponChooser.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Credits.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyFixes.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadingScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\LTDecisionWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\LTDialogueWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\LTMaskedWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\LTTextWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\LtWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageBox.cpp
# End Source File
# Begin Source File

SOURCE=.\TeleType.cpp
# End Source File
# End Group
# Begin Group "Misc Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\AssertMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Callback0.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraMgrFX.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterMovement.cpp
# End Source File
# Begin Source File

SOURCE=.\CheatMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\client_physics.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CommonUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\CycleCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DamageTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\FrameRateMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\GameClientShell.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSettings.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\GameType.cpp
# End Source File
# Begin Source File

SOURCE=.\HeadBobMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\IKChain.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\LTString.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\MemoryUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MouseMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MPClientMgrChart.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiplayerClientMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\MultiplayerMgrDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\Music.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeController.cpp
# End Source File
# Begin Source File

SOURCE=.\OverlayMGR.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\PlayerMovement.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerStats.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ratetracker.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveGameData.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ServerOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Sparam.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StoryElement.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\TemplateList.cpp
# End Source File
# Begin Source File

SOURCE=.\TextureModeMGR.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewModeMGR.cpp
# End Source File
# Begin Source File

SOURCE=.\VKDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponModel.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\WinUtil.cpp
# End Source File
# End Group
# Begin Group "ButeMgr Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\CharacterButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSoundMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\FXButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\GameButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\GlobalMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\MissionMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ModelButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ObjectivesButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\PickupButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerOptionMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\VisionModeButeMGR.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponMgr.cpp
# End Source File
# End Group
# Begin Group "Special FX Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BaseLineSystemFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseParticleSystemFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BasePolyDrawFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseScaleFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BodyPropFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BulletTrailFX.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraFX.cpp
# End Source File
# Begin Source File

SOURCE=.\CharacterFX.cpp
# End Source File
# Begin Source File

SOURCE=.\DeathFX.cpp
# End Source File
# Begin Source File

SOURCE=.\DebrisFX.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebugLine.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugLineFX.cpp
# End Source File
# Begin Source File

SOURCE=.\DripperFX.cpp
# End Source File
# Begin Source File

SOURCE=.\DynamicLightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ExplosionFX.cpp
# End Source File
# Begin Source File

SOURCE=.\FireFX.cpp
# End Source File
# Begin Source File

SOURCE=.\FlashLight.cpp
# End Source File
# Begin Source File

SOURCE=.\FogVolumeFX.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\FXStructs.cpp
# End Source File
# Begin Source File

SOURCE=.\GibFX.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LaserBeam.cpp
# End Source File
# Begin Source File

SOURCE=.\LeashFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LensFlareFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LightningFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LineSystemFX.cpp
# End Source File
# Begin Source File

SOURCE=.\MarkSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiDebrisFX.cpp
# End Source File
# Begin Source File

SOURCE=.\MuzzleFlashFX.cpp
# End Source File
# Begin Source File

SOURCE=.\MuzzleFlashParticleFX.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeLinesFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleExplosionFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleShowerFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSystemFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleTrailFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleTrailSegmentFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PickupObjectFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyDebrisFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyGridFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyLineFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PredTargetFx.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectileFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PVFXMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomSparksFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchLightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXFactories.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedFXStructs.cpp
# End Source File
# Begin Source File

SOURCE=.\ShellCasingFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SmokeFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundFX.cpp
# End Source File
# Begin Source File

SOURCE=.\sprinklesfx.cpp
# End Source File
# Begin Source File

SOURCE=.\SpriteControlFX.cpp
# End Source File
# Begin Source File

SOURCE=.\StarLightViewerFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SteamFX.cpp
# End Source File
# Begin Source File

SOURCE=.\TracerFX.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeBrush.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponFX.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponSoundFX.cpp
# End Source File
# Begin Source File

SOURCE=.\WeatherFX.cpp
# End Source File
# End Group
# End Group
# Begin Group "Headers"

# PROP Default_Filter "*.h"
# Begin Group "Interface Headers"

# PROP Default_Filter ""
# Begin Group "HUD Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MotionDetector.h
# End Source File
# Begin Source File

SOURCE=.\Subtitle.h
# End Source File
# Begin Source File

SOURCE=.\WeaponChooser.h
# End Source File
# End Group
# Begin Group "Menu Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BaseFolder.h
# End Source File
# Begin Source File

SOURCE=.\FolderCampaignLevel.h
# End Source File
# Begin Source File

SOURCE=.\FolderCommands.h
# End Source File
# Begin Source File

SOURCE=.\FolderCredits.h
# End Source File
# Begin Source File

SOURCE=.\FolderCustomControls.h
# End Source File
# Begin Source File

SOURCE=.\FolderCustomLevel.h
# End Source File
# Begin Source File

SOURCE=.\FolderDisplay.h
# End Source File
# Begin Source File

SOURCE=.\FolderEscape.h
# End Source File
# Begin Source File

SOURCE=.\FolderGame.h
# End Source File
# Begin Source File

SOURCE=.\FolderHost.h
# End Source File
# Begin Source File

SOURCE=.\FolderHostConfig.h
# End Source File
# Begin Source File

SOURCE=.\FolderHostMaps.h
# End Source File
# Begin Source File

SOURCE=.\FolderHostOptions.h
# End Source File
# Begin Source File

SOURCE=.\FolderJoin.h
# End Source File
# Begin Source File

SOURCE=.\FolderJoinInfo.h
# End Source File
# Begin Source File

SOURCE=.\FolderJoystick.h
# End Source File
# Begin Source File

SOURCE=.\FolderKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\FolderLoad.h
# End Source File
# Begin Source File

SOURCE=.\FolderMain.h
# End Source File
# Begin Source File

SOURCE=.\FolderMouse.h
# End Source File
# Begin Source File

SOURCE=.\FolderMulti.h
# End Source File
# Begin Source File

SOURCE=.\FolderOptions.h
# End Source File
# Begin Source File

SOURCE=.\FolderPerformance.h
# End Source File
# Begin Source File

SOURCE=.\FolderPlayer.h
# End Source File
# Begin Source File

SOURCE=.\FolderPlayerJoin.h
# End Source File
# Begin Source File

SOURCE=.\FolderProfile.h
# End Source File
# Begin Source File

SOURCE=.\FolderSave.h
# End Source File
# Begin Source File

SOURCE=.\FolderSetupDM.h
# End Source File
# Begin Source File

SOURCE=.\FolderSetupEvac.h
# End Source File
# Begin Source File

SOURCE=.\FolderSetupHunt.h
# End Source File
# Begin Source File

SOURCE=.\FolderSetupOverrun.h
# End Source File
# Begin Source File

SOURCE=.\FolderSetupSurvivor.h
# End Source File
# Begin Source File

SOURCE=.\FolderSetupTeamDM.h
# End Source File
# Begin Source File

SOURCE=.\FolderSingle.h
# End Source File
# Begin Source File

SOURCE=.\FrameCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MessageBoxCtrl.h
# End Source File
# Begin Source File

SOURCE=.\SliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\StaticTextCtrl.h
# End Source File
# End Group
# Begin Group "Manager Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AutoTargetMgr.h
# End Source File
# Begin Source File

SOURCE=.\CrosshairMgr.h
# End Source File
# Begin Source File

SOURCE=.\FolderMgr.h
# End Source File
# Begin Source File

SOURCE=.\HudMgr.h
# End Source File
# Begin Source File

SOURCE=.\InterfaceMgr.h
# End Source File
# Begin Source File

SOURCE=.\InterfaceResMgr.h
# End Source File
# Begin Source File

SOURCE=.\InterfaceSurfMgr.h
# End Source File
# Begin Source File

SOURCE=.\InterfaceTimer.h
# End Source File
# Begin Source File

SOURCE=.\JoystickAxis.h
# End Source File
# Begin Source File

SOURCE=.\LayoutMgr.h
# End Source File
# Begin Source File

SOURCE=.\PerformanceMgr.h
# End Source File
# Begin Source File

SOURCE=.\ProfileMgr.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\KeyFixes.h
# End Source File
# Begin Source File

SOURCE=.\LoadingScreen.h
# End Source File
# Begin Source File

SOURCE=.\LTAlphaWnd.h
# End Source File
# Begin Source File

SOURCE=.\LTDecisionWnd.h
# End Source File
# Begin Source File

SOURCE=.\LTDialogueWnd.h
# End Source File
# Begin Source File

SOURCE=.\LTMaskedWnd.h
# End Source File
# Begin Source File

SOURCE=.\LTTextWnd.h
# End Source File
# Begin Source File

SOURCE=.\LtWnd.h
# End Source File
# Begin Source File

SOURCE=.\MessageBox.h
# End Source File
# Begin Source File

SOURCE=.\TeleType.h
# End Source File
# End Group
# Begin Group "ButeMgr Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\CharacterButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\ClientButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\ClientSoundMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\FXButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GameButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\GameButes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GlobalMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\HierarchicalButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MissionMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ModelButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ObjectivesButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\PickupButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\ServerOptionMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceMgr.h
# End Source File
# Begin Source File

SOURCE=.\VisionModeButeMGR.h
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponMgr.h
# End Source File
# End Group
# Begin Group "Misc Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\AssertMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Callback.h
# End Source File
# Begin Source File

SOURCE=..\Shared\callback0.h
# End Source File
# Begin Source File

SOURCE=..\Shared\callback1.h
# End Source File
# Begin Source File

SOURCE=..\Shared\callback2.h
# End Source File
# Begin Source File

SOURCE=..\Shared\callback3.h
# End Source File
# Begin Source File

SOURCE=..\Shared\callback4.h
# End Source File
# Begin Source File

SOURCE=..\Shared\callback5.h
# End Source File
# Begin Source File

SOURCE=.\CameraMgr.h
# End Source File
# Begin Source File

SOURCE=.\CameraMgrDefs.h
# End Source File
# Begin Source File

SOURCE=.\CameraMgrFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterAlignment.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterMovement.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterMovementDefs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CheatDefs.h
# End Source File
# Begin Source File

SOURCE=.\CheatMgr.h
# End Source File
# Begin Source File

SOURCE=.\client_physics.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ClientServerShared.h
# End Source File
# Begin Source File

SOURCE=.\ClientUtilities.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponUtils.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CommandIDs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CommonUtilities.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ContainerCodes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CSDefs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DamageTypes.h
# End Source File
# Begin Source File

SOURCE=.\DynArray.h
# End Source File
# Begin Source File

SOURCE=.\FastList.h
# End Source File
# Begin Source File

SOURCE=.\FrameRateMgr.h
# End Source File
# Begin Source File

SOURCE=.\GameClientShell.h
# End Source File
# Begin Source File

SOURCE=.\GameSettings.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Globals.h
# End Source File
# Begin Source File

SOURCE=.\HeadBobMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\IKChain.h
# End Source File
# Begin Source File

SOURCE=..\Shared\LTString.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MemoryUtils.h
# End Source File
# Begin Source File

SOURCE=.\MessageMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ModelDefs.h
# End Source File
# Begin Source File

SOURCE=.\MouseMgr.h
# End Source File
# Begin Source File

SOURCE=.\MPClientMgrChart.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MsgIDs.h
# End Source File
# Begin Source File

SOURCE=.\MultiplayerClientMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MultiplayerMgrDefs.h
# End Source File
# Begin Source File

SOURCE=.\Music.h
# End Source File
# Begin Source File

SOURCE=.\NodeController.h
# End Source File
# Begin Source File

SOURCE=.\OverlayMGR.h
# End Source File
# Begin Source File

SOURCE=..\Shared\PlayerMovement.h
# End Source File
# Begin Source File

SOURCE=.\PlayerStats.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ratetracker.h
# End Source File
# Begin Source File

SOURCE=.\SaveGameData.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SCDefs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ServerOptions.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedMovement.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundTypes.h
# End Source File
# Begin Source File

SOURCE=.\Sparam.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Stdafx.h
# End Source File
# Begin Source File

SOURCE=.\StoryElement.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceFunctions.h
# End Source File
# Begin Source File

SOURCE=..\Shared\TemplateList.h
# End Source File
# Begin Source File

SOURCE=.\TextureModeMGR.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Timer.h
# End Source File
# Begin Source File

SOURCE=.\VarTrack.h
# End Source File
# Begin Source File

SOURCE=.\ViewModeMGR.h
# End Source File
# Begin Source File

SOURCE=.\ViewOrderOdds.h
# End Source File
# Begin Source File

SOURCE=.\VKDefs.h
# End Source File
# Begin Source File

SOURCE=.\WeaponModel.h
# End Source File
# Begin Source File

SOURCE=.\WeaponStringDefs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\\WinUtil.h
# End Source File
# End Group
# Begin Group "Special FX Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BaseLineSystemFX.h
# End Source File
# Begin Source File

SOURCE=.\BaseParticleSystemFX.h
# End Source File
# Begin Source File

SOURCE=.\BasePolyDrawFX.h
# End Source File
# Begin Source File

SOURCE=.\BaseScaleFX.h
# End Source File
# Begin Source File

SOURCE=.\BodyPropFX.h
# End Source File
# Begin Source File

SOURCE=.\BulletTrailFX.h
# End Source File
# Begin Source File

SOURCE=.\CameraFX.h
# End Source File
# Begin Source File

SOURCE=.\CharacterFX.h
# End Source File
# Begin Source File

SOURCE=.\CSpecialFXFactory.h
# End Source File
# Begin Source File

SOURCE=.\DeathFX.h
# End Source File
# Begin Source File

SOURCE=.\DebrisFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisTypes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebugLine.h
# End Source File
# Begin Source File

SOURCE=.\DebugLineFX.h
# End Source File
# Begin Source File

SOURCE=.\DripperFX.h
# End Source File
# Begin Source File

SOURCE=.\DynamicLightFX.h
# End Source File
# Begin Source File

SOURCE=.\ExplosionFX.h
# End Source File
# Begin Source File

SOURCE=.\FireFX.h
# End Source File
# Begin Source File

SOURCE=.\FlashLight.h
# End Source File
# Begin Source File

SOURCE=.\FogVolumeFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\FXStructs.h
# End Source File
# Begin Source File

SOURCE=.\GibFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GibTypes.h
# End Source File
# Begin Source File

SOURCE=.\GraphFX.h
# End Source File
# Begin Source File

SOURCE=.\LaserBeam.h
# End Source File
# Begin Source File

SOURCE=.\LeashFX.h
# End Source File
# Begin Source File

SOURCE=.\LensFlareFX.h
# End Source File
# Begin Source File

SOURCE=.\LightFX.h
# End Source File
# Begin Source File

SOURCE=.\LightningFX.h
# End Source File
# Begin Source File

SOURCE=.\LineSystemFX.h
# End Source File
# Begin Source File

SOURCE=.\MarkSFX.h
# End Source File
# Begin Source File

SOURCE=.\MultiDebrisFX.h
# End Source File
# Begin Source File

SOURCE=.\MuzzleFlashFX.h
# End Source File
# Begin Source File

SOURCE=.\MuzzleFlashParticleFX.h
# End Source File
# Begin Source File

SOURCE=.\NodeLinesFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleExplosionFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleShowerFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSystemFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleTrailFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleTrailSegmentFX.h
# End Source File
# Begin Source File

SOURCE=.\PickupObjectFX.h
# End Source File
# Begin Source File

SOURCE=.\PolyDebrisFX.h
# End Source File
# Begin Source File

SOURCE=.\PolyGridFX.h
# End Source File
# Begin Source File

SOURCE=.\PolyLineFX.h
# End Source File
# Begin Source File

SOURCE=.\PredTargetFx.h
# End Source File
# Begin Source File

SOURCE=.\ProjectileFX.h
# End Source File
# Begin Source File

SOURCE=.\PVFXMgr.h
# End Source File
# Begin Source File

SOURCE=.\RandomSparksFX.h
# End Source File
# Begin Source File

SOURCE=.\SearchLightFX.h
# End Source File
# Begin Source File

SOURCE=.\SFXMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SFXMsgIds.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedBaseFXStructs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedFXStructs.h
# End Source File
# Begin Source File

SOURCE=.\ShellCasingFX.h
# End Source File
# Begin Source File

SOURCE=.\SmokeFX.h
# End Source File
# Begin Source File

SOURCE=.\SoundFX.h
# End Source File
# Begin Source File

SOURCE=.\SpecialFX.h
# End Source File
# Begin Source File

SOURCE=.\SpecialFXList.h
# End Source File
# Begin Source File

SOURCE=.\sprinklesfx.h
# End Source File
# Begin Source File

SOURCE=.\SpriteControlFX.h
# End Source File
# Begin Source File

SOURCE=.\StarLightViewFX.h
# End Source File
# Begin Source File

SOURCE=.\SteamFX.h
# End Source File
# Begin Source File

SOURCE=.\TracerFX.h
# End Source File
# Begin Source File

SOURCE=.\VolumeBrushFX.h
# End Source File
# Begin Source File

SOURCE=.\WeaponFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponFXTypes.h
# End Source File
# Begin Source File

SOURCE=.\WeaponSoundFX.h
# End Source File
# Begin Source File

SOURCE=.\WeatherFX.h
# End Source File
# End Group
# End Group
# Begin Group "Libs_Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\LithFontMgr_LT2.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\ltguimgr_LT2.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\MFCStub.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\CryptMgr.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\StdLith.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\ButeMgrNoMFC.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\LithTrackMgrClient.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\GameSpyClientMgr.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\WONAPI.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Lt2\lithshared\libs\Debug\GameSpyClientMgrDemo.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Libs_Release"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\StdLith.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\CryptMgr.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\LithFontMgr_LT2.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\ltguimgr_LT2.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\MFCStub.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\ButeMgrNoMFC.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\LithTrackMgrClient.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\GameSpyClientMgr.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\WONAPI.lib"

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Lt2\lithshared\libs\Release\GameSpyClientMgrDemo.lib

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Demo Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
