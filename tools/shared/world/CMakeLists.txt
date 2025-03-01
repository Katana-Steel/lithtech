project(LIB_ToolWorld)

find_package(SDL2 REQUIRED)

add_definitions(-DDIRECT3D_VERSION=0x0700 -DCMAKE_GENERATOR_PLATFORM=Win32 -DWIN32 -D_WINDOWS -DDIRECTEDITOR_BUILD -DSTDLITH_MFC -DBDEFS_MFC -DPRE_FLOAT -DPREPROCESSOR_BUILD -DNO_PRAGMA_LIBS -DLT15_COMPAT -D_AFXDLL -D_WIN32_WINNT=0x501 -D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS -DUSE_ID3DXEFFECT)

set(libsources
	EditBrush.cpp
	EditObjects.cpp
	EditPlane.cpp
	EditPoly.cpp
	EditRegion.cpp
	TexturedPlane.cpp
	BasePoly.cpp
	PropList.cpp
	node_ops.cpp
	WorldNode.cpp
	s3tc_compress.cpp
	LTTexture.cpp
	PrefabMgr.cpp
	PrefabRef.cpp
	UVtoOPQ.cpp
	../../../runtime/shared/src/genericprop_setup.cpp
	../engine/conparse.cpp)

add_library(${PROJECT_NAME} STATIC ${libsources})

include_directories(../engine
	../../../sdk/inc
	../../../libs/cryptmgr
	../../../libs/butemgr
	../../../libs/stdlith
	../../../libs/lith
	../../../libs/ltamgr
	../../../libs/MFCStub
	../../../libs/dibmgr
	../../../libs/genregmgr
	../../../libs/RegMgr
	../../DEdit
	../../DEdit/Frames
	../../DEdit/Controls
	../../DEdit/Options
	../../DEdit/Dlgs
	../../DEdit/Docs
	../../DEdit/Misc
	../../DEdit/MRCEXT
	../../DEdit/Lightmap
	../../DEdit/draw
	../../DEdit/ModelView
	../../DEdit/RegionEdit
	../../DEdit/Project
	../../PreProcessor
	../../../runtime/kernel/src
	${SDL2_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
	LIB_ltamgr)