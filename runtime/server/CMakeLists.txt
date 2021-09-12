project(DLL_Server)

add_definitions(-DDE_SERVER_COMPILE -DDIRECTENGINE_COMPILE -DSTDLITH_ALLOC_OVERRIDE -DLT15_COMPAT -DNO_PRAGMA_LIBS)

if (ENABLE_DXVK)
	include_directories(${DXVK_INCLUDE_DIRS})
endif()

set(libsources
	src/server_iltcommon.cpp
	../shared/src/shared_iltcommon.cpp
	../shared/src/modellt_impl.cpp
	src/server_iltmodel.cpp
	src/server_iltphysics.cpp
	../shared/src/shared_iltphysics.cpp
	src/server_iltsoundmgr.cpp
	../shared/src/transformlt_impl.cpp
	src/world_server_bsp.cpp
	../world/src/world_shared_bsp.cpp
	../shared/src/interface_linkage.cpp
	../model/src/animtracker.cpp
	../shared/src/bdefs.cpp
	../shared/src/classbind.cpp
	src/classmgr.cpp
	../shared/src/collision.cpp
	../shared/src/compress.cpp
	../shared/src/concommand.cpp
	../shared/src/conparse.cpp
	../world/src/de_mainworld.cpp
	../world/src/de_nodes.cpp
	../world/src/de_objects.cpp
	../kernel/src/debugging.cpp
	../shared/src/dhashtable.cpp
	../shared/src/engine_vars.cpp
	../shared/src/findobj.cpp
	../shared/src/ftserv.cpp
	../world/src/fullintersectline.cpp
	src/game_serialize.cpp
	../shared/src/gamemath.cpp
	../shared/src/genericprop_setup.cpp
	../shared/src/genltstream.cpp
	../shared/src/geometry.cpp
	../shared/src/geomroutines.cpp
	../shared/src/impl_common.cpp
	src/interlink.cpp
	../world/src/intersect_line.cpp
	../world/src/intersectsweptsphere.cpp
	../shared/src/leech.cpp
	../world/src/light_table.cpp
	../shared/src/lightmap_planes.cpp
	../kernel/net/src/localdriver.cpp
	../shared/src/ltmessage.cpp
	src/ltmessage_server.cpp
	../../sdk/inc/ltmodule.cpp
	../../sdk/inc/ltobjref.cpp
	../../sdk/inc/ltquatbase.cpp
	../shared/src/lttimer.cpp
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
	../shared/src/ratetracker.cpp
	src/s_client.cpp
	src/s_concommand.cpp
	src/s_intersect.cpp
	src/s_net.cpp
	src/s_object.cpp
	src/server_consolestate.cpp
	src/server_extradata.cpp
	src/server_filemgr.cpp
	../kernel/src/server_interface.cpp
	src/server_loaderthread.cpp
	src/serverde_impl.cpp
	src/serverevent.cpp
	src/servermgr.cpp
	src/sloaderthread.cpp
	src/smoveabstract.cpp
	../sound/src/sounddata.cpp
	src/soundtrack.cpp
	../shared/src/spritecontrolimpl.cpp
	../shared/src/stacktrace.cpp
	../shared/src/stdlterror.cpp
	../shared/src/strtools.cpp
	../model/src/transformmaker.cpp
	../sound/src/wave.cpp
	../world/src/world_blind_object_data.cpp
	../world/src/world_blocker_data.cpp
	../world/src/world_blocker_math.cpp
	../world/src/world_particle_blocker_data.cpp
	../world/src/world_tree.cpp)

if(WIN32)
	set(libsources ${libsources}
		../kernel/net/src/sys/win/udpdriver.cpp
		../model/src/sys/d3d/d3d_model_load.cpp
		../kernel/src/sys/win/bindmgr.cpp
		../kernel/src/sys/win/dutil.cpp
		../kernel/src/sys/win/stringmgr.cpp
		../kernel/src/sys/win/counter.cpp
		../kernel/io/src/sys/win/de_file.cpp
		../kernel/mem/src/sys/win/de_memory.cpp
		../kernel/src/sys/win/dsys_interface.cpp
		../kernel/src/sys/win/lthread.cpp
		../kernel/src/sys/win/ltlibraryloader.cpp
		../kernel/src/sys/win/ltthread.cpp
		../kernel/src/sys/win/streamsim.cpp
		../kernel/src/sys/win/systeminfo.cpp
		../kernel/src/sys/win/timemgr.cpp)
else(WIN32)
	set(libsources ${libsources}
		../kernel/net/src/sys/linux/udpdriver.cpp
		../kernel/src/sys/linux/bindmgr.cpp
		../kernel/src/sys/linux/dutil.cpp
		../kernel/src/sys/linux/stringmgr.cpp
		../kernel/src/sys/linux/counter.cpp
		../kernel/io/src/sys/linux/linuxfile.cpp
		../kernel/mem/src/sys/linux/de_memory.cpp
		../kernel/mem/src/sys/linux/linuxundata.cpp
		../kernel/src/sys/linux/linuxdsys.cpp
		../kernel/src/sys/linux/lthread.cpp
		../kernel/src/sys/linux/ltlibraryloader.cpp
		../kernel/src/sys/linux/ltthread.cpp
		../kernel/src/sys/linux/streamsim.cpp
		../kernel/src/sys/linux/timemgr.cpp)
endif(WIN32)

include_directories(src
	../../sdk/inc
	../../sdk/inc/physics
	../../libs/stdlith
	../../libs/rezmgr
	../../libs/lith
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
	../controlfilemgr)


if(NOT MSVC)	# FIXME: this is a hack for geomroutines.h:309
	include_directories(../../tools/PreProcessor)
endif(NOT MSVC)

if(WIN32)
	include_directories(../kernel/src/sys/win
		../shared/src/sys/win)
else(WIN32)
	find_package(SDL2 REQUIRED)
	include_directories(../kernel/src/sys/linux
		../shared/src/sys/linux
		${SDL2_INCLUDE_DIRS})
endif(WIN32)

if(ENABLE_D3D)
	include_directories(../render_a/src/sys/d3d)
endif(ENABLE_D3D)

if(ENABLE_DIRECTX)	# FIXME: set to find DirectX path
	include_directories("C:\\Program Files (x86)\\Microsoft DirectX SDK (August 2007)\\Include")
endif(ENABLE_DIRECTX)

add_library(${PROJECT_NAME} SHARED ${libsources})

set_target_properties(${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME Server)

target_link_libraries(${PROJECT_NAME}
	LIB_RezMgr
	LIB_Info
	LIB_Lith
	LIB_StdLith
	LIB_LTMem)

if(WIN32)
	target_link_libraries(${PROJECT_NAME}
		winmm
		ws2_32)
endif(WIN32)

if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fpermissive -fPIC")
endif(LINUX)
