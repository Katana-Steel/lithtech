
@echo off

REM This file can be different on each person's machine doing a build.  It 
REM sets the environment variables that MakeDist.bat and SrcBackup.bat use.

set PROJ_DIR=C:\PROJ
set LT2_DIR=%PROJ_DIR%\LT2

set AVP2_DIR=%PROJ_DIR%\AVP2
set AVP2_BUILD_DIR=%AVP2_DIR%\AVP2

set MSDEV_PATH=c:\program files\microsoft visual studio\common\msdev98\bin
set SS_PATH=c:\program files\microsoft visual studio\vss\win32
set PATH=%PATH%;"%MSDEV_PATH%";"%SS_PATH%"
