cmake_minimum_required(VERSION 3.1)
project(Lithtech)

option(ENABLE_D3D "Enable Direct3D" ON)

add_definitions(-D__D3D -D__D3DREND)
add_definitions(-D_WINDOWS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)

add_subdirectory(libs/zlib)
add_subdirectory(libs/stdlith)

#add_subdirectory(runtime/render_a/src/sys/d3d)
add_subdirectory(libs/RegMgr)
add_subdirectory(libs/genregmgr)
add_subdirectory(libs/MFCStub)
add_subdirectory(libs/CryptMgr)
add_subdirectory(libs/ButeMgr)
add_subdirectory(libs/ltamgr)
add_subdirectory(libs/dibmgr)
add_subdirectory(libs/rezmgr)
add_subdirectory(libs/lith)

add_subdirectory(tools/shared/model)
add_subdirectory(tools/shared/world)

add_subdirectory(tools/DEdit)			# TOOLS_DEdit

