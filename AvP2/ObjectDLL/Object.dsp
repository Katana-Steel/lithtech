# Microsoft Developer Studio Project File - Name="Object" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Object - Win32 Demo Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak" CFG="Object - Win32 Demo Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Object - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Demo Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Final" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Libraries/Riot (C++)/Object", HKXAAAAA"
# PROP Scc_LocalPath "c:\proj\de\build\object"
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Object - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "NDEBUG" /D "_OBJECTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FD /c
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
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\Object.lto"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
InputPath=.\release\Object.lto
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy release\object.lto $(AVP2_BUILD_DIR)\object.lto

# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "_DEBUG" /D "_OBJECTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /Fr /FD /c
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
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"libcmt.lib" /out:"debug\Object.lto" /pdbtype:sept /libpath:"$(LT2_DIR)\lithshared\libs\debug\\"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
InputPath=.\debug\Object.lto
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy debug\object.lto $(AVP2_BUILD_DIR)\object.lto

# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Object___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "Object___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Object___Win32_Demo_Release"
# PROP Intermediate_Dir "Object___Win32_Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AVP3BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "NDEBUG" /D "_DEMO" /D "_OBJECTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /D "_FINAL" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Demo_Release/Object.bsc"
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\Object.lto"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:"Demo_Release/Object.pdb" /machine:I386 /out:"demo\release\Object.lto"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
InputPath=.\demo\release\Object.lto
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy demo\release\object.lto $(AVP2_BUILD_DIR)\object.lto

# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Object___Win32_Demo_Debug"
# PROP BASE Intermediate_Dir "Object___Win32_Demo_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Object___Win32_Demo_Debug"
# PROP Intermediate_Dir "Object___Win32_Demo_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AVP3BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /Fr /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /Zi /Od /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "_DEBUG" /D "_DEMO" /D "_OBJECTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /D "_FINAL" /Fr"Demo_Debug/" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Demo_Debug/Object.bsc"
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"debug\Object.lto" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:"Demo_Debug/Object.pdb" /debug /machine:I386 /out:"demo\debug\Object.lto" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map
# Begin Custom Build
InputPath=.\demo\debug\Object.lto
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy demo\debug\object.lto $(AVP2_BUILD_DIR)\object.lto

# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Object___Win32_Final"
# PROP BASE Intermediate_Dir "Object___Win32_Final"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Object___Win32_Final"
# PROP Intermediate_Dir "Object___Win32_Final"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "NDEBUG" /D "_OBJECTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I "..\ServerRes" /I "..\ClientShellDLL" /I "..\ObjectDLL" /I "..\shared" /I "$(LT2_DIR)\sdk\inc" /I "$(LT2_DIR)\sdk\inc\compat" /I "$(LT2_DIR)\lithshared\incs" /I "$(LT2_DIR)\lithshared\stl" /D "_FINAL" /D "NDEBUG" /D "_OBJECTBUILD" /D "WIN32" /D "_WINDOWS" /D "_AVP2BUILD" /D "LT15_COMPAT" /D "NO_PRAGMA_LIBS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Final/Object.bsc"
LINK32=xilink6.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\Object.lto"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:"Final/Object.pdb" /machine:I386 /out:"final\Object.lto"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
InputPath=.\final\Object.lto
SOURCE="$(InputPath)"

"$(AVP2_BUILD_DIR)\object.lto" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy final\object.lto $(AVP2_BUILD_DIR)\object.lto

# End Custom Build

!ENDIF 

# Begin Target

# Name "Object - Win32 Release"
# Name "Object - Win32 Debug"
# Name "Object - Win32 Demo Release"
# Name "Object - Win32 Demo Debug"
# Name "Object - Win32 Final"
# Begin Group "Source"

# PROP Default_Filter "cpp;c"
# Begin Group "AI Source Files"

# PROP Default_Filter ""
# Begin Group "AIStrategy Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\AIStrategyAdvance.cpp
# End Source File
# Begin Source File

SOURCE=.\AIStrategyApproachTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\AIStrategyEngageTarget.cpp
# End Source File
# Begin Source File

SOURCE=.\AIStrategyFollowPath.cpp
# End Source File
# Begin Source File

SOURCE=.\AIStrategyMove.cpp
# End Source File
# Begin Source File

SOURCE=.\AIStrategyRetreat.cpp
# End Source File
# Begin Source File

SOURCE=.\AIStrategySetFacing.cpp
# End Source File
# End Group
# Begin Group "AI Goal Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIAlarm.cpp
# End Source File
# Begin Source File

SOURCE=.\AIAlienAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\AIAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\AIChase.cpp
# End Source File
# Begin Source File

SOURCE=.\AICombatGoal.cpp
# End Source File
# Begin Source File

SOURCE=.\AICower.cpp
# End Source File
# Begin Source File

SOURCE=.\AIInvestigate.cpp
# End Source File
# Begin Source File

SOURCE=.\AILurk.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeCheck.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPatrol.cpp
# End Source File
# Begin Source File

SOURCE=.\AIRetreat.cpp
# End Source File
# Begin Source File

SOURCE=.\AIScript.cpp
# End Source File
# Begin Source File

SOURCE=.\AISnipe.cpp
# End Source File
# End Group
# Begin Group "AINode Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AINode.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeAlarm.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeCover.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeLadder.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodePatrol.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeSnipe.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeWallWalk.cpp
# End Source File
# End Group
# Begin Group "AI Action Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIActionMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\AIActionMoveTo.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AI.cpp
# End Source File
# Begin Source File

SOURCE=.\AIAlien.cpp
# End Source File
# Begin Source File

SOURCE=.\AIButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AICorporate.cpp
# End Source File
# Begin Source File

SOURCE=.\AIDunyaExosuit.cpp
# End Source File
# Begin Source File

SOURCE=.\AIEisenberg.cpp
# End Source File
# Begin Source File

SOURCE=.\AIExosuit.cpp
# End Source File
# Begin Source File

SOURCE=.\AIMarine.cpp
# End Source File
# Begin Source File

SOURCE=.\AIMiser.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPath.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPathMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPredator.cpp
# End Source File
# Begin Source File

SOURCE=.\AIQueen.cpp
# End Source File
# Begin Source File

SOURCE=.\AIRykov.cpp
# End Source File
# Begin Source File

SOURCE=.\AIRykovExosuit.cpp
# End Source File
# Begin Source File

SOURCE=.\AIScriptCommands.cpp
# End Source File
# Begin Source File

SOURCE=.\AISense.cpp
# End Source File
# Begin Source File

SOURCE=.\AISenseHear.cpp
# End Source File
# Begin Source File

SOURCE=.\AISenseMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AISenseSee.cpp
# End Source File
# Begin Source File

SOURCE=.\AIState.cpp
# End Source File
# Begin Source File

SOURCE=.\AITarget.cpp
# End Source File
# Begin Source File

SOURCE=.\AIUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\AIVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\AIVolumeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AvoidancePath.cpp
# End Source File
# Begin Source File

SOURCE=.\BidMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Goal.cpp
# End Source File
# Begin Source File

SOURCE=.\ObstacleMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\Sense.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleAI.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleAIAlien.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleNode.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleNodeMgr.cpp
# End Source File
# End Group
# Begin Group "ButeMgr Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIAnimButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\AnimationButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AttachButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterButeMgr.cpp
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

SOURCE=.\MusicMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ObjectivesButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\PickupButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\PropTypeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerAssetMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerSoundMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SpawnButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponMgr.cpp
# End Source File
# End Group
# Begin Group "Misc Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Shared\AssertMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Attachments.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Callback0.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterAnimation.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterAnimationDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\CharacterHitBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CharacterMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterMovement.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientDeathSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientLightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CommonUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\Controller.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DamageTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\DebrisFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisTypes.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebugLine.cpp
# End Source File
# Begin Source File

SOURCE=.\DebugLineSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\DialogueWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Editable.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\FXStructs.cpp
# End Source File
# Begin Source File

SOURCE=.\GameServerShell.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\GameType.cpp
# End Source File
# Begin Source File

SOURCE=.\HHWeaponModel.cpp
# End Source File
# Begin Source File

SOURCE=.\Key.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyData.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyFramer.cpp
# End Source File
# Begin Source File

SOURCE=.\keyframer_light.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\LTString.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\MemoryUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiplayerMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeLine.cpp
# End Source File
# Begin Source File

SOURCE=.\object_list.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectiveMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectRemover.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\PlugFactoryTemplate.h
# End Source File
# Begin Source File

SOURCE=.\PolyGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\PredDisk.cpp
# End Source File
# Begin Source File

SOURCE=.\PredNet.cpp
# End Source File
# Begin Source File

SOURCE=.\Projectile.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectileTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomSpawner.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ratetracker.cpp
# End Source File
# Begin Source File

SOURCE=.\Scanner.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchLight.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerMark.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedFXStructs.cpp
# End Source File
# Begin Source File

SOURCE=.\Sparam.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprinkles.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\TemplateList.cpp
# End Source File
# Begin Source File

SOURCE=.\turret.cpp
# End Source File
# Begin Source File

SOURCE=.\TurretSense.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapons.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldModelDebris.cpp
# End Source File
# End Group
# Begin Group "Game Object Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Alarm.cpp
# End Source File
# Begin Source File

SOURCE=.\AmmoBox.cpp
# End Source File
# Begin Source File

SOURCE=.\Beetle.cpp
# End Source File
# Begin Source File

SOURCE=.\BodyProp.cpp
# End Source File
# Begin Source File

SOURCE=.\Breakable.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\Character.cpp
# End Source File
# Begin Source File

SOURCE=.\CinematicTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\Civilian.cpp
# End Source File
# Begin Source File

SOURCE=.\DebrisSpawner.cpp
# End Source File
# Begin Source File

SOURCE=.\Destructible.cpp
# End Source File
# Begin Source File

SOURCE=.\DestructibleModel.cpp
# End Source File
# Begin Source File

SOURCE=.\Door.cpp
# End Source File
# Begin Source File

SOURCE=.\Dripper.cpp
# End Source File
# Begin Source File

SOURCE=.\Egg.cpp
# End Source File
# Begin Source File

SOURCE=.\EvacZone.cpp
# End Source File
# Begin Source File

SOURCE=.\EventCounter.cpp
# End Source File
# Begin Source File

SOURCE=.\ExitTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\Explosion.cpp
# End Source File
# Begin Source File

SOURCE=.\Fire.cpp
# End Source File
# Begin Source File

SOURCE=.\FogVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\GameBase.cpp
# End Source File
# Begin Source File

SOURCE=.\GameStartPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\Group.cpp
# End Source File
# Begin Source File

SOURCE=.\HackableLock.cpp
# End Source File
# Begin Source File

SOURCE=.\HackableWorldmodel.cpp
# End Source File
# Begin Source File

SOURCE=.\HingedDoor.cpp
# End Source File
# Begin Source File

SOURCE=.\LaserTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\LightGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\Lightning.cpp
# End Source File
# Begin Source File

SOURCE=.\MDTrackerItem.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiSpawner.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectiveEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\PickupObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Plant.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerObj.cpp
# End Source File
# Begin Source File

SOURCE=.\PredShoulderCannon.cpp
# End Source File
# Begin Source File

SOURCE=.\Prop.cpp
# End Source File
# Begin Source File

SOURCE=.\PropType.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingDoor.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingWorldModel.cpp
# End Source File
# Begin Source File

SOURCE=.\ScaleSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenShake.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundFXObj.cpp
# End Source File
# Begin Source File

SOURCE=.\Spawner.cpp
# End Source File
# Begin Source File

SOURCE=.\SpectatorPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\SpriteControl.cpp
# End Source File
# Begin Source File

SOURCE=.\StarlightView.cpp
# End Source File
# Begin Source File

SOURCE=.\Steam.cpp
# End Source File
# Begin Source File

SOURCE=.\StoryObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Striker.cpp
# End Source File
# Begin Source File

SOURCE=.\Switch.cpp
# End Source File
# Begin Source File

SOURCE=.\TeleportPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\TorchableLock.cpp
# End Source File
# Begin Source File

SOURCE=.\TorchableLockCounter.cpp
# End Source File
# Begin Source File

SOURCE=.\TrailNode.cpp
# End Source File
# Begin Source File

SOURCE=.\TranslucentWorldModel.cpp
# End Source File
# Begin Source File

SOURCE=.\Trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\TriggerSound.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeBrush.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeBrushTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldProperties.cpp
# End Source File
# End Group
# End Group
# Begin Group "Headers"

# PROP Default_Filter "h;hpp"
# Begin Group "AI Header Files"

# PROP Default_Filter ""
# Begin Group "AIStrategy Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIStrategy.h
# End Source File
# Begin Source File

SOURCE=.\AIStrategyAdvance.h
# End Source File
# Begin Source File

SOURCE=.\AIStrategyApproachTarget.h
# End Source File
# Begin Source File

SOURCE=.\AIStrategyEngageTarget.h
# End Source File
# Begin Source File

SOURCE=.\AIStrategyFollowPath.h
# End Source File
# Begin Source File

SOURCE=.\AIStrategyMove.h
# End Source File
# Begin Source File

SOURCE=.\AIStrategyRetreat.h
# End Source File
# Begin Source File

SOURCE=.\AIStrategySetFacing.h
# End Source File
# End Group
# Begin Group "AI Goal Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIAlarm.h
# End Source File
# Begin Source File

SOURCE=.\AIAlienAttack.h
# End Source File
# Begin Source File

SOURCE=.\AIAttack.h
# End Source File
# Begin Source File

SOURCE=.\AIChase.h
# End Source File
# Begin Source File

SOURCE=.\AICombatGoal.h
# End Source File
# Begin Source File

SOURCE=.\AICower.h
# End Source File
# Begin Source File

SOURCE=.\AIIdle.h
# End Source File
# Begin Source File

SOURCE=.\AIInvestigate.h
# End Source File
# Begin Source File

SOURCE=.\AILurk.h
# End Source File
# Begin Source File

SOURCE=.\AINodeCheck.h
# End Source File
# Begin Source File

SOURCE=.\AIPatrol.h
# End Source File
# Begin Source File

SOURCE=.\AIRetreat.h
# End Source File
# Begin Source File

SOURCE=.\AIScript.h
# End Source File
# Begin Source File

SOURCE=.\AISnipe.h
# End Source File
# End Group
# Begin Group "AINode Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AINode.h
# End Source File
# Begin Source File

SOURCE=.\AINodeAlarm.h
# End Source File
# Begin Source File

SOURCE=.\AINodeCover.h
# End Source File
# Begin Source File

SOURCE=.\AINodeGroup.h
# End Source File
# Begin Source File

SOURCE=.\AINodeLadder.h
# End Source File
# Begin Source File

SOURCE=.\AINodeMgr.h
# End Source File
# Begin Source File

SOURCE=.\AINodePatrol.h
# End Source File
# Begin Source File

SOURCE=.\AINodeSnipe.h
# End Source File
# Begin Source File

SOURCE=.\AINodeVolume.h
# End Source File
# Begin Source File

SOURCE=.\AINodeWallWalk.h
# End Source File
# End Group
# Begin Group "AI Action Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIAction.h
# End Source File
# Begin Source File

SOURCE=.\AIActionMisc.h
# End Source File
# Begin Source File

SOURCE=.\AIActionMoveTo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AI.h
# End Source File
# Begin Source File

SOURCE=.\AIAlien.h
# End Source File
# Begin Source File

SOURCE=.\AIButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\AICorporate.h
# End Source File
# Begin Source File

SOURCE=.\AIDunyaExosuit.h
# End Source File
# Begin Source File

SOURCE=.\AIEisenberg.h
# End Source File
# Begin Source File

SOURCE=.\AIExosuit.h
# End Source File
# Begin Source File

SOURCE=.\AIMarine.h
# End Source File
# Begin Source File

SOURCE=.\AIMiser.h
# End Source File
# Begin Source File

SOURCE=.\AIPath.h
# End Source File
# Begin Source File

SOURCE=.\AIPlayer.h
# End Source File
# Begin Source File

SOURCE=.\AIPredator.h
# End Source File
# Begin Source File

SOURCE=.\AIQueen.h
# End Source File
# Begin Source File

SOURCE=.\AIRykov.h
# End Source File
# Begin Source File

SOURCE=.\AIRykovExosuit.h
# End Source File
# Begin Source File

SOURCE=.\AIScriptCommands.h
# End Source File
# Begin Source File

SOURCE=.\AISense.h
# End Source File
# Begin Source File

SOURCE=.\AISenseHear.h
# End Source File
# Begin Source File

SOURCE=.\AISenseMgr.h
# End Source File
# Begin Source File

SOURCE=.\AISenseSee.h
# End Source File
# Begin Source File

SOURCE=.\AIState.h
# End Source File
# Begin Source File

SOURCE=.\AITarget.h
# End Source File
# Begin Source File

SOURCE=.\AIUtils.h
# End Source File
# Begin Source File

SOURCE=.\AIVolume.h
# End Source File
# Begin Source File

SOURCE=.\AStar.h
# End Source File
# Begin Source File

SOURCE=.\AvoidancePath.h
# End Source File
# Begin Source File

SOURCE=.\BidMgr.h
# End Source File
# Begin Source File

SOURCE=.\Goal.h
# End Source File
# Begin Source File

SOURCE=.\ObstacleMgr.h
# End Source File
# Begin Source File

SOURCE=.\ParseUtils.h
# End Source File
# Begin Source File

SOURCE=.\Sense.h
# End Source File
# Begin Source File

SOURCE=.\SimpleAI.h
# End Source File
# Begin Source File

SOURCE=.\SimpleAIAlien.h
# End Source File
# Begin Source File

SOURCE=.\SimpleNode.h
# End Source File
# Begin Source File

SOURCE=.\SimpleNodeMgr.h
# End Source File
# End Group
# Begin Group "ButeMgr Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIAnimButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\AnimationButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\AttachButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterButeMgr.h
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

SOURCE=..\Shared\GlobalMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MissionMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ModelButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\MusicMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ObjectivesButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\PickupButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\PropTypeMgr.h
# End Source File
# Begin Source File

SOURCE=.\ServerAssetMgr.h
# End Source File
# Begin Source File

SOURCE=.\ServerSoundMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundMgr.h
# End Source File
# Begin Source File

SOURCE=.\SpawnButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponMgr.h
# End Source File
# End Group
# Begin Group "Misc Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AIPathMgr.h
# End Source File
# Begin Source File

SOURCE=.\AIVolumeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\AssertMgr.h
# End Source File
# Begin Source File

SOURCE=.\Attachments.h
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

SOURCE=..\Shared\CharacterAlignment.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterAnimation.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterAnimationDefs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterFuncs.h
# End Source File
# Begin Source File

SOURCE=.\CharacterHitBox.h
# End Source File
# Begin Source File

SOURCE=.\CharacterMgr.h
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

SOURCE=.\ClientDeathSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientLightFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ClientServerShared.h
# End Source File
# Begin Source File

SOURCE=.\ClientSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CommandIDs.h
# End Source File
# Begin Source File

SOURCE=.\CommandMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CommonUtilities.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ContainerCodes.h
# End Source File
# Begin Source File

SOURCE=.\Controller.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CSDefs.h
# End Source File
# Begin Source File

SOURCE=.\CVarTrack.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DamageTypes.h
# End Source File
# Begin Source File

SOURCE=.\DebrisFuncs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisTypes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebugLine.h
# End Source File
# Begin Source File

SOURCE=.\DebugLineSystem.h
# End Source File
# Begin Source File

SOURCE=.\DialogueWindow.h
# End Source File
# Begin Source File

SOURCE=.\DisplayTimer.h
# End Source File
# Begin Source File

SOURCE=.\DynArray.h
# End Source File
# Begin Source File

SOURCE=.\Editable.h
# End Source File
# Begin Source File

SOURCE=.\fsm.h
# End Source File
# Begin Source File

SOURCE=..\Shared\FXStructs.h
# End Source File
# Begin Source File

SOURCE=.\GameServerShell.h
# End Source File
# Begin Source File

SOURCE=..\Shared\gametexmgr_codes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GameType.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GibTypes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Globals.h
# End Source File
# Begin Source File

SOURCE=.\HHWeaponModel.h
# End Source File
# Begin Source File

SOURCE=..\Shared\HierarchicalButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\Key.h
# End Source File
# Begin Source File

SOURCE=.\KeyData.h
# End Source File
# Begin Source File

SOURCE=.\KeyFramer.h
# End Source File
# Begin Source File

SOURCE=.\keyframer_light.h
# End Source File
# Begin Source File

SOURCE=..\Shared\LTString.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MemoryUtils.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ModelDefs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MsgIDs.h
# End Source File
# Begin Source File

SOURCE=.\MultiplayerMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MultiplayerMgrDefs.h
# End Source File
# Begin Source File

SOURCE=.\NodeLine.h
# End Source File
# Begin Source File

SOURCE=.\ObjectiveMgr.h
# End Source File
# Begin Source File

SOURCE=.\ObjectMsgs.h
# End Source File
# Begin Source File

SOURCE=.\ObjectRemover.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.h
# End Source File
# Begin Source File

SOURCE=.\PolyGrid.h
# End Source File
# Begin Source File

SOURCE=.\PredDisk.h
# End Source File
# Begin Source File

SOURCE=.\PredNet.h
# End Source File
# Begin Source File

SOURCE=.\Projectile.h
# End Source File
# Begin Source File

SOURCE=.\ProjectileTypes.h
# End Source File
# Begin Source File

SOURCE=.\Range.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ratetracker.h
# End Source File
# Begin Source File

SOURCE=..\Shared\RCMessage.h
# End Source File
# Begin Source File

SOURCE=.\Scanner.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SCDefs.h
# End Source File
# Begin Source File

SOURCE=.\SearchLight.h
# End Source File
# Begin Source File

SOURCE=.\ServerMark.h
# End Source File
# Begin Source File

SOURCE=.\ServerUtilities.h
# End Source File
# Begin Source File

SOURCE=.\SFXFuncs.h
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

SOURCE=..\Shared\SharedMovement.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundTypes.h
# End Source File
# Begin Source File

SOURCE=.\Sparam.h
# End Source File
# Begin Source File

SOURCE=.\Sprinkles.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Stdafx.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceFunctions.h
# End Source File
# Begin Source File

SOURCE=..\Shared\TemplateList.h
# End Source File
# Begin Source File

SOURCE=.\Turret.h
# End Source File
# Begin Source File

SOURCE=.\TurretSense.h
# End Source File
# Begin Source File

SOURCE=.\Weapon.h
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponFXTypes.h
# End Source File
# Begin Source File

SOURCE=.\Weapons.h
# End Source File
# Begin Source File

SOURCE=.\WorldModelDebris.h
# End Source File
# End Group
# Begin Group "Game Object Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Alarm.h
# End Source File
# Begin Source File

SOURCE=.\AmmoBox.h
# End Source File
# Begin Source File

SOURCE=.\Beetle.h
# End Source File
# Begin Source File

SOURCE=.\BodyProp.h
# End Source File
# Begin Source File

SOURCE=.\Breakable.h
# End Source File
# Begin Source File

SOURCE=.\Camera.h
# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=.\CinematicTrigger.h
# End Source File
# Begin Source File

SOURCE=.\Civilian.h
# End Source File
# Begin Source File

SOURCE=.\DebrisSpawner.h
# End Source File
# Begin Source File

SOURCE=.\Destructible.h
# End Source File
# Begin Source File

SOURCE=.\DestructibleModel.h
# End Source File
# Begin Source File

SOURCE=.\Door.h
# End Source File
# Begin Source File

SOURCE=.\Dripper.h
# End Source File
# Begin Source File

SOURCE=.\Egg.h
# End Source File
# Begin Source File

SOURCE=.\EvacZone.h
# End Source File
# Begin Source File

SOURCE=.\EventCounter.h
# End Source File
# Begin Source File

SOURCE=.\ExitTrigger.h
# End Source File
# Begin Source File

SOURCE=.\Explosion.h
# End Source File
# Begin Source File

SOURCE=.\Fire.h
# End Source File
# Begin Source File

SOURCE=.\FogVolume.h
# End Source File
# Begin Source File

SOURCE=.\GameBase.h
# End Source File
# Begin Source File

SOURCE=.\GameStartPoint.h
# End Source File
# Begin Source File

SOURCE=.\Group.h
# End Source File
# Begin Source File

SOURCE=.\HackableLock.h
# End Source File
# Begin Source File

SOURCE=.\HackableWorldmodel.h
# End Source File
# Begin Source File

SOURCE=.\HingedDoor.h
# End Source File
# Begin Source File

SOURCE=.\LaserTrigger.h
# End Source File
# Begin Source File

SOURCE=.\LightGroup.h
# End Source File
# Begin Source File

SOURCE=.\Lightning.h
# End Source File
# Begin Source File

SOURCE=.\LightObject.h
# End Source File
# Begin Source File

SOURCE=.\MDTrackerItem.h
# End Source File
# Begin Source File

SOURCE=.\MultiSpawner.h
# End Source File
# Begin Source File

SOURCE=.\MusicVolume.h
# End Source File
# Begin Source File

SOURCE=.\ObjectiveEvent.h
# End Source File
# Begin Source File

SOURCE=.\PickupObject.h
# End Source File
# Begin Source File

SOURCE=.\Plant.h
# End Source File
# Begin Source File

SOURCE=.\PlayerObj.h
# End Source File
# Begin Source File

SOURCE=.\PredShoulderCannon.h
# End Source File
# Begin Source File

SOURCE=.\Prop.h
# End Source File
# Begin Source File

SOURCE=.\PropType.h
# End Source File
# Begin Source File

SOURCE=.\RandomSpawner.h
# End Source File
# Begin Source File

SOURCE=.\RotatingDoor.h
# End Source File
# Begin Source File

SOURCE=.\RotatingWorldModel.h
# End Source File
# Begin Source File

SOURCE=.\ScaleSprite.h
# End Source File
# Begin Source File

SOURCE=.\ScreenShake.h
# End Source File
# Begin Source File

SOURCE=.\SoundFX.h
# End Source File
# Begin Source File

SOURCE=.\SoundFXObj.h
# End Source File
# Begin Source File

SOURCE=.\Spawner.h
# End Source File
# Begin Source File

SOURCE=.\SpectatorPoint.h
# End Source File
# Begin Source File

SOURCE=.\SpriteControl.h
# End Source File
# Begin Source File

SOURCE=.\StarlightView.h
# End Source File
# Begin Source File

SOURCE=.\Steam.h
# End Source File
# Begin Source File

SOURCE=.\StoryObject.h
# End Source File
# Begin Source File

SOURCE=.\Striker.h
# End Source File
# Begin Source File

SOURCE=.\Switch.h
# End Source File
# Begin Source File

SOURCE=.\TeleportPoint.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Timer.h
# End Source File
# Begin Source File

SOURCE=.\TorchableLock.h
# End Source File
# Begin Source File

SOURCE=.\TorchableLockCounter.h
# End Source File
# Begin Source File

SOURCE=.\TrailNode.h
# End Source File
# Begin Source File

SOURCE=.\TranslucentWorldModel.h
# End Source File
# Begin Source File

SOURCE=.\Trigger.h
# End Source File
# Begin Source File

SOURCE=.\TriggerSound.h
# End Source File
# Begin Source File

SOURCE=.\VolumeBrush.h
# End Source File
# Begin Source File

SOURCE=.\VolumeBrushTypes.h
# End Source File
# Begin Source File

SOURCE=.\WorldProperties.h
# End Source File
# End Group
# End Group
# Begin Group "Libs_Release"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\MFCStub.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\CryptMgr.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\ButeMgrNoMFC.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\StdLith.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\LithTrackMgr.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\GameSpyMgr.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Release\WONAPI.lib"
# End Source File
# Begin Source File

SOURCE=..\..\..\Lt2\lithshared\libs\Release\GameSpyMgrDemo.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Libs_Debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\CryptMgr.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\StdLith.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\ButeMgrNoMFC.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\MFCStub.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\LithTrackMgr.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="$(LT2_DIR)\lithshared\libs\Debug\GameSpyMgr.lib"

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Lt2\lithshared\libs\Debug\GameSpyMgrDemo.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Demo Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
