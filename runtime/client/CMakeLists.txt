project(EXE_Lithtech)

add_definitions(-DDE_LOCAL_SERVERBIND -DDE_CLIENT_COMPILE -DDSNDMGR_NO_MFC -DDIRECTENGINE_COMPILE -DMODEL_SUPPORT_ABC)
if(WIN32)
	add_definitions(-DUSE_ID3DXEFFECT)
	if(CMAKE_CL_64)
		link_directories("C:\\Program Files (x86)\\Microsoft DirectX SDK (August 2007)\\Lib\\x64")
	else(CMAKE_CL_64)
		link_directories("C:\\Program Files (x86)\\Microsoft DirectX SDK (August 2007)\\Lib\\x86")
	endif(CMAKE_CL_64)
endif(WIN32)

if (ENABLE_DXVK)
	include_directories(${DXVK_INCLUDE_DIRS})
	link_directories(${DXVK_LIB_DIRS})
endif()

if (ENABLE_OPENAL)
	add_definitions(-DUSE_OPENAL)
endif(ENABLE_OPENAL)

set(exec_src
	../controlfilemgr/controlfilemgr.cpp
	src/client_filemgr.cpp
	../kernel/src/icommandlineargs.cpp
	src/client_iltcommon.cpp
	../server/src/server_iltcommon.cpp
	../shared/src/shared_iltcommon.cpp
	src/client_iltmodel.cpp
	../shared/src/modellt_impl.cpp
	../server/src/server_iltmodel.cpp
	src/client_iltphysics.cpp
	../server/src/server_iltphysics.cpp
	../shared/src/shared_iltphysics.cpp
	../server/src/server_iltsoundmgr.cpp
	../sound/src/soundmgr.cpp
	../shared/src/transformlt_impl.cpp
	src/client_iltvideomgr.cpp
	../shared/src/interface_linkage.cpp
	../world/src/world_blind_object_data.cpp
	../world/src/world_blocker_data.cpp
	../world/src/world_blocker_math.cpp
	src/world_client_bsp.cpp
	../world/src/world_particle_blocker_data.cpp
	../server/src/world_server_bsp.cpp
	../world/src/world_shared_bsp.cpp
	../model/src/animtracker.cpp
	../shared/src/bdefs.cpp
	../shared/src/classbind.cpp
	../server/src/classmgr.cpp
	src/client_formatmgr.cpp
	src/client_graphmgr.cpp
	src/client_ticks.cpp
	src/clientde_impl.cpp
	src/clientmgr.cpp
	src/clientshell.cpp
	src/cmoveabstract.cpp
	src/cnet.cpp
	src/cobject.cpp
	../shared/src/collision.cpp
	../shared/src/compress.cpp
	../shared/src/concommand.cpp
	../shared/src/conparse.cpp
	../kernel/src/sys/sdl2/InputSDL2.cpp
	../kernel/src/ltdirectmusiccontrolfile.cpp
	src/console.cpp
	src/consolecommands.cpp
	src/cutil.cpp
	../world/src/de_mainworld.cpp
	../world/src/de_nodes.cpp
	../world/src/de_objects.cpp
	../shared/src/debuggeometry.cpp
	../kernel/src/debugging.cpp
	src/debuggraphmgr.cpp
	../shared/src/dhashtable.cpp
	../shared/src/dtxmgr.cpp
	../shared/src/engine_vars.cpp
	src/errorlog.cpp
	../shared/src/findobj.cpp
	../shared/src/ftclient.cpp
	../shared/src/ftserv.cpp
	../world/src/fullintersectline.cpp
	../server/src/game_serialize.cpp
	../shared/src/gamemath.cpp
	../shared/src/genericprop_setup.cpp
	../shared/src/genltstream.cpp
	../shared/src/geometry.cpp
	../shared/src/geomroutines.cpp
	../shared/src/impl_common.cpp
	../server/src/interlink.cpp
	../world/src/intersect_line.cpp
	../world/src/intersectsweptsphere.cpp
	../shared/src/leech.cpp
	../world/src/light_table.cpp
	../shared/src/lightmap_compress.cpp
	../shared/src/lightmap_planes.cpp
	src/linesystem.cpp
	../kernel/net/src/localdriver.cpp
	src/ltbenchmark_impl.cpp
	../../sdk/inc/lterror.cpp
	src/ltinfo_impl.cpp
	../kernel/mem/src/ltmemory.cpp
	../shared/src/ltmessage.cpp
	src/ltmessage_client.cpp
	../server/src/ltmessage_server.cpp
	../../sdk/inc/ltmodule.cpp
	../../sdk/inc/ltobjref.cpp
	../../sdk/inc/ltquatbase.cpp
	src/lttexturestring.cpp
	../shared/src/lttimer.cpp
	src/memorywatch.cpp
	../model/src/model.cpp
	../model/src/model_load.cpp
	../model/src/modelallocations.cpp
	../shared/src/motion.cpp
	../shared/src/moveobject.cpp
	../shared/src/moveplayer.cpp
	../kernel/net/src/netmgr.cpp
	../shared/src/nexus.cpp
	../shared/src/objectmgr.cpp
	../kernel/net/src/packet.cpp
	../shared/src/parse_world_info.cpp
	src/particlesystem.cpp
	../shared/src/pixelformat.cpp
	src/polygrid.cpp
	src/predict.cpp
	../shared/src/ratetracker.cpp
	../server/src/s_client.cpp
	../server/src/s_concommand.cpp
	../server/src/s_intersect.cpp
	../server/src/s_net.cpp
	../server/src/s_object.cpp
	../server/src/server_consolestate.cpp
	../server/src/server_extradata.cpp
	../server/src/server_filemgr.cpp
	../server/src/serverde_impl.cpp
	../server/src/serverevent.cpp
	../server/src/servermgr.cpp
	src/setupobject.cpp
	src/shellnet.cpp
	src/shelltransfer.cpp
	src/shellutil.cpp
	../server/src/smoveabstract.cpp
	../sound/src/soundbuffer.cpp
	../sound/src/sounddata.cpp
	../sound/src/soundinstance.cpp
	../server/src/soundtrack.cpp
	src/sprite.cpp
	../shared/src/spritecontrolimpl.cpp
	../shared/src/stacktrace.cpp
	../shared/src/stdlterror.cpp
	../shared/src/strtools.cpp
	src/texturestring.cpp
	../model/src/transformmaker.cpp
	../shared/src/version_info.cpp
	src/volumeeffect.cpp
	src/watermark.cpp
	../sound/src/wave.cpp
	../world/src/world_tree.cpp)

set(render_src )
set(libs )
if(WIN32)
	set(exec_src WIN32
	../kernel/io/src/sys/win/de_file.cpp
	#../model/src/sys/null/nullmodel_load.cpp
	../kernel/src/sys/win/lt_cursor_impl.cpp
	../kernel/src/sys/win/bindmgr.cpp
	../kernel/src/sys/win/binkvideomgrimpl.cpp
	../kernel/src/sys/win/client.cpp
	../kernel/src/sys/win/counter.cpp
	src/sys/win/customfontfilemgr.cpp
	../kernel/mem/src/sys/win/de_memory.cpp
	../kernel/src/sys/win/dshowvideomgrimpl.cpp
	../shared/src/sys/win/dstreamopenqueuemgr.cpp
	../kernel/src/sys/win/dsys_interface.cpp
	../kernel/src/sys/win/dutil.cpp
	../kernel/src/sys/win/input.cpp
	../kernel/src/sys/win/InputSim.cpp
	../kernel/src/sys/win/load_pcx.cpp
	../kernel/src/sys/win/lthread.cpp
	../kernel/src/sys/win/ltlibraryloader.cpp
	../kernel/src/sys/win/ltrendermgr_impl.cpp
	../kernel/src/sys/win/ltthread.cpp
	../kernel/src/sys/win/musicdriver.cpp
	../kernel/src/sys/win/streamsim.cpp
	../kernel/src/sys/win/stringmgr.cpp
	../kernel/src/sys/win/systeminfo.cpp
	../kernel/src/sys/win/text_mgr.cpp
	src/sys/win/texturestringimage.cpp
	../kernel/src/sys/win/timemgr.cpp
	../kernel/net/src/sys/win/udpdriver.cpp
	../kernel/src/sys/win/version_resource.cpp
	../kernel/src/sys/win/videomgr.cpp
	src/sys/win/winconsole_impl.cpp
	${exec_src})
else(WIN32)
	set(exec_src
	../kernel/mem/src/sys/linux/de_memory.cpp
	src/sys/linux/customfontfilemgr.cpp
	src/sys/linux/texturestringimage.cpp
	../kernel/io/src/sys/linux/linuxfile.cpp
	../kernel/net/src/sys/linux/linux_ltthread.cpp
	../kernel/net/src/sys/linux/udpdriver.cpp
	../kernel/src/sys/linux/lt_cursor_impl.cpp
	../kernel/src/sys/linux/dutil.cpp
	../kernel/src/sys/linux/bindmgr.cpp
	../kernel/src/sys/linux/timemgr.cpp
	../kernel/src/sys/linux/client.cpp
	../kernel/src/sys/linux/videomgr.cpp
	../kernel/src/sys/linux/linuxdsys.cpp
	../kernel/src/sys/linux/counter.cpp
	../kernel/src/sys/linux/load_pcx.cpp
	../kernel/src/sys/linux/ltlibraryloader.cpp
	../kernel/src/sys/linux/ltthread.cpp
	../kernel/src/sys/linux/lthread.cpp
	src/sys/linux/linuxconsole_impl.cpp
	../kernel/src/sys/linux/stringhelper.cpp
	../kernel/src/sys/linux/stringmgr.cpp
	../kernel/src/sys/linux/streamsim.cpp
	../kernel/src/sys/linux/musicdriver.cpp
	../kernel/src/sys/linux/input.cpp
    ${exec_src})
endif(WIN32)

if (ENABLE_OPENAL)
	set(exec_src ${exec_src}
	../sound/src/sys/openal/openal.cpp
    )
elseif (NOT WIN32)
	set(exec_src ${exec_src}
	../sound/src/sys/linux/sdl.cpp
    )
endif (ENABLE_OPENAL)

if (ENABLE_DIRECTMUSIC)
	set(exec_src ${exec_src}
	../kernel/src/sys/win/ltdirectmusic_impl.cpp
	../kernel/src/sys/win/ltdirectmusicloader.cpp
    )
else (ENABLE_DIRECTMUSIC)
	set(exec_src ${exec_src}
	../kernel/src/genericmusic_impl.cpp
    )
endif (ENABLE_DIRECTMUSIC)

if (WIN32 OR ENABLE_DXVK)
	set(exec_src ${exec_src}
	../model/src/sys/d3d/d3d_model_load.cpp
	../render_b/src/sys/d3d/d3ddrawprim.cpp
	../render_b/src/sys/d3d/d3dtexinterface.cpp
	../kernel/src/sys/win/interface_helpers.cpp
	../kernel/src/sys/win/render.cpp
	src/sys/win/winclientde_impl.cpp
    )

else()
	set(exec_src  ${exec_src}
	../shared/src/sys/linux/renderstruct.cpp
	../model/src/sys/linux/linuxmodel_load.cpp
	../render_b/src/sys/sdl/sdldrawprim.cpp
	../kernel/src/sys/linux/render.cpp
	src/sys/linux/linuxclientde_impl.cpp
    )
endif()

find_package(SDL2 REQUIRED)

include_directories(src
	../../sdk/inc
	../../sdk/inc/physics
	../../libs/stdlith
	../../libs/rezmgr
	../../libs/lith
	../../libs/RandomGen/src
	../../libs/LTGUIMgr
	../../tools/PreProcessor
	../shared/src
	../kernel/src
	../kernel/mem/src
	../world/src
	../kernel/io/src
	../kernel/net/src
	../model/src
	../client/src
	../sound/src
	../lithtemplate
	../physics/src
	../info/src
	../render_b/src
	../ui/src
	../controlfilemgr
	../server/src
	${SDL2_INCLUDE_DIRS})

if(WIN32)
	include_directories(../kernel/src/sys/win
		../shared/src/sys/win)
	set(libs ${libs}
		LIB_DShow
		winmm)
else(WIN32)
	include_directories(../kernel/src/sys/linux
		../shared/src/sys/linux)
	set(libs ${libs}
	    dl
		pthread)

endif(WIN32)

if (ENABLE_OPENAL)
	include_directories(
		${OPENAL_INCLUDEDIR}
		${MPG123_INCLUDEDIR})
	set(libs ${libs}
		${OPENAL_LIBRARIES}
		${MPG123_LIBRARIES})
endif (ENABLE_OPENAL)

if(ENABLE_DIRECTX)	# FIXME: set to find DirectX path
	include_directories(../../libs/dshow
		"C:\\Program Files (x86)\\Microsoft DirectX SDK (August 2007)\\Include")
		#if(CMAKE_CL_64)
		#	link_directories("C:\\Program Files (x86)\\Microsoft DirectX SDK (August 2007)\\Lib\\x64")
		#else(CMAKE_CL_64)

			# FIXME: Why is this not working??


		#endif(CMAKE_CL_64)
	set(libs ${libs}
			ddraw
			d3dx9
			d3d9
			DxErr
			dxguid
			dplayx
			dinput8
			strmiids)
endif(ENABLE_DIRECTX)

set(libs ${libs}
	LIB_StdLith
	LIB_ZLib
	LIB_LTMem
	LIB_RezMgr
	LIB_Lith
	LIB_Info
	LIB_ILTSound
	LIB_UI
	LIB_Random
    ${SDL2_LIBRARIES})


if(ENABLE_OPENGL)
	include_directories(../render_a/src/sys/ogl)
    add_definitions(-D__ENABLE_OGL)
	set(exec_src 
	../render_a/src/sys/ogl/common_init.cpp
	../render_a/src/sys/ogl/ogl_render.cpp
	../render_a/src/sys/ogl/ltvertexshadermgr.cpp
	../render_a/src/sys/ogl/ltpixelshadermgr.cpp
	../render_a/src/sys/ogl/rendererframestats.cpp
	${exec_src})
	set(libs ${libs}
		GL
		)
endif(ENABLE_OPENGL)

if(ENABLE_VULKAN)
	include_directories(../render_a/src/sys/vk)
    add_definitions(-D__ENABLE_VK)
endif(ENABLE_VULKAN)

if(ENABLE_D3D)
	include_directories(../render_a/src/sys/d3d)
	set(libs ${libs} LIB_D3DRender)
	if (ENABLE_DXVK)
		set(libs ${libs} dxvk_d3d9)
	endif()
endif(ENABLE_D3D)

add_executable(${PROJECT_NAME} ${exec_src})
set_target_properties(${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME Lithtech)
target_link_libraries(${PROJECT_NAME} ${libs})

if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-g -fpermissive")
endif(LINUX)
