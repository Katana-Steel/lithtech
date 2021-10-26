cmake_minimum_required(VERSION 3.1)
project(Lithtech)

if(NOT WIN32)
  find_package(PkgConfig)
  pkg_check_modules(DXVK dxvk-native)
  if(DXVK_FOUND)
    set(GL OFF)
    set(VK ON)
  else()
    set(GL ON)
    set(VK OFF)
  endif()
endif()

option(BUILD_ENGINE "Build engine" ON)
option(BUILD_NOLF2 "Build No One Lives Forever 2 game code" ON)
option(BUILD_TRON "Build Tron 2.0 game code" OFF)
option(BUILD_BLOOD2 "Build Blood II game code" OFF)
option(BUILD_FEAR "Build F.E.A.R. game code" OFF)
option(BUILD_NOLF "Build No One Lives Forever game code" OFF)
option(BUILD_SHOGO "Build Shogo game code" OFF)
option(BUILD_TOOLS "Build tools" OFF)
if(WIN32)
option(ENABLE_D3D "Enable Direct3D" ON)
option(ENABLE_DIRECTX "Enable DirectX" ON)
option(ENABLE_DIRECTSHOW "Enable DirectShow" ON)
option(ENABLE_OPENAL "Use OpenAL as audio backend" OFF)
option(ENABLE_DIRECTMUSIC "Use DirectMusic as music backend" OFF)
else(WIN32)
option(ENABLE_OPENGL "Enable OpenGL 3.3" ${GL})
option(ENABLE_VULKAN "Enable Vulkan" OFF)
option(ENABLE_DXVK "Enable DXVK" ${VK})
option(ENABLE_OPENAL "Use OpenAL as audio backend" ON)
option(ENABLE_DIRECTMUSIC OFF)
endif(WIN32)
option(USE_INTERNAL_ZLIB "Use bundled zlib" ON)			# eventually want default to be OFF
option(USE_INTERNAL_FREETYPE "Use bundled freetype" ON)	# eventually want default to be OFF

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/OUT")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

if (ENABLE_DXVK)
	option(ENABLE_D3D "Enable Direct3D" ON)
	pkg_check_modules(DXVK dxvk-native)
	add_definitions(-DUSE_DXVK)
	set(DXVK_INCLUDE_DIRS ${DXVK_INCLUDEDIR}/native/windows ${DXVK_INCLUDEDIR}/native/directx)
	set(DXVK_LIB_DIRS ${DXVK_LIBDIR})
endif()

if (ENABLE_OPENAL AND NOT WIN32)
	pkg_check_modules(OPENAL REQUIRED openal)
	pkg_check_modules(MPG123 REQUIRED libmpg123)
endif()

if(ENABLE_D3D)
    add_definitions(-D__D3D -D__D3DREND)
endif(ENABLE_D3D)


if(WIN32)
    add_definitions(-D_WINDOWS)
else(WIN32)
    add_definitions(-D_LINUX -D__LINUX)
    set(LINUX 1)
endif(WIN32)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif(MSVC)

if(DEBUG)
    add_definitions(-D_DEBUG)
else(DEBUG)
    add_definitions(-D_FINAL)
endif(DEBUG)


if(USE_INTERNAL_ZLIB)
    #set zlib vars
    add_subdirectory(libs/zlib)							# LIB_ZLib
else(USE_INTERNAL_ZLIB)
    find_package(ZLIB)
endif(USE_INTERNAL_ZLIB)

# move to Lib_UI CMakeLists.txt?
if(USE_INTERNAL_FREETYPE)
    #set freetype vars
else(USE_INTERNAL_FREETYPE)
    find_package(Freetype)
endif(USE_INTERNAL_FREETYPE)


add_subdirectory(libs/lith)								# LIB_Lith
add_subdirectory(libs/stdlith)							# LIB_StdLith
add_subdirectory(libs/ltmem)							# LIB_LTMem
add_subdirectory(libs/rezmgr)							# LIB_RezMgr

if(LINUX)
    set(JSON_BuildTests off)
    add_subdirectory(libs/DynRes)
    add_subdirectory(json)
endif(LINUX)

if(BUILD_ENGINE)
    add_subdirectory(runtime/ui)						# LIB_UI
    add_subdirectory(runtime/info)						# LIB_Info
    add_subdirectory(runtime/sound)						# LIB_ILTSound
    add_subdirectory(runtime/server)					# DLL_Server
    add_subdirectory(runtime/client)					# EXE_Lithtech

	if(ENABLE_D3D)
		add_subdirectory(runtime/render_a/src/sys/d3d)	# LIB_D3DRender
	endif(ENABLE_D3D)
	 if(WIN32)
        if(NOT ENABLE_OPENAL)
            add_subdirectory(runtime/sound/src/sys/s_dx8)	# DLL_SndDrvDX8
        endif(NOT ENABLE_OPENAL)

        if(ENABLE_DIRECTSHOW)
            add_subdirectory(libs/dshow)					# LIB_DShow
        endif(ENABLE_DIRECTSHOW)

        add_subdirectory(runtime/winbuild/ltmsg)		# DLL_LTMsg
    endif(WIN32)
endif(BUILD_ENGINE)

if(BUILD_NOLF2)
    add_subdirectory(libs/MFCStub)						# LIB_MFCStub
    add_subdirectory(libs/ButeMgr)						# LIB_ButeMgr
    add_subdirectory(libs/CryptMgr)						# LIB_CryptMgr
    add_subdirectory(libs/RegMgr)						# LIB_RegMgr
    add_subdirectory(libs/LTGUIMgr)						# LIB_LTGUIMgr
    add_subdirectory(libs/ServerDir)					# ServerDir
    add_subdirectory(libs/WONAPI)						# LIB_WONAPI
    add_subdirectory(NOLF2/ClientFxDLL)					# ClientFXDLL
    add_subdirectory(NOLF2/ClientRes/TO2)				# ClientRes
    add_subdirectory(NOLF2/ClientShellDLL/TO2)			# ClientShellDLL
    add_subdirectory(NOLF2/ObjectDLL/TO2)				# ObjectDLL
    add_subdirectory(NOLF2/ServerRes/TO2)				# ServerRes
endif(BUILD_NOLF2)

if(BUILD_FEAR)
    if(NOT BUILD_NOLF2)
        add_subdirectory(libs/MFCStub)						# LIB_MFCStub
        add_subdirectory(libs/ButeMgr)						# LIB_ButeMgr
        add_subdirectory(libs/CryptMgr)						# LIB_CryptMgr
        add_subdirectory(libs/RegMgr)						# LIB_RegMgr
        add_subdirectory(libs/LTGUIMgr)						# LIB_LTGUIMgr
        add_subdirectory(libs/ServerDir)					# ServerDir
        add_subdirectory(libs/WONAPI)						# LIB_WONAPI
    endif(NOT BUILD_NOLF2)
    add_subdirectory(FEAR/ClientFxDLL)					# ClientFXDLL
#	add_subdirectory(NOLF2/ClientRes/TO2)				# ClientRes
    add_subdirectory(FEAR/ClientShellDLL)				# ClientShellDLL
#	add_subdirectory(NOLF2/ObjectDLL/TO2)				# ObjectDLL
#	add_subdirectory(NOLF2/ServerRes/TO2)				# ServerRes
endif(BUILD_FEAR)

if(BUILD_TRON)
    if(NOT BUILD_NOLF2)
        add_subdirectory(libs/MFCStub)
        add_subdirectory(libs/ButeMgr)
        add_subdirectory(libs/CryptMgr)
        add_subdirectory(libs/RegMgr)
        add_subdirectory(libs/LTGUIMgr)
        add_subdirectory(libs/ServerDir)
        add_subdirectory(libs/WONAPI)
        add_subdirectory(NOLF2/ClientFxDLL)		# ClientFXDLL
    endif(NOT BUILD_NOLF2)
    add_subdirectory(NOLF2/ClientRes/TRON)		# ClientRes
    add_subdirectory(NOLF2/ClientShellDLL/TRON)	# ClientShellDLL
    add_subdirectory(NOLF2/ObjectDLL/TRON)		# ObjectDLL
    add_subdirectory(NOLF2/ServerRes/TRON)		# ServerRes
endif(BUILD_TRON)

if(BUILD_TOOLS)
    add_subdirectory(tools/DtxView)			# TOOLS_DtxView
    add_subdirectory(tools/LithRez)			# TOOLS_LithRez
endif(BUILD_TOOLS)

if(NOT WIN32)
add_subdirectory(tests/RegMgr)
add_subdirectory(tests/CShell)
add_subdirectory(tests/DynRes)
add_subdirectory(tests/DynResDLL)
endif(NOT WIN32)
