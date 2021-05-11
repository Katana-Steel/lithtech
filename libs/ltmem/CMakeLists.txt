project(LIB_LTMem)

if(LINUX)
find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
endif(LINUX)

add_library(${PROJECT_NAME} STATIC
	ltmem.cpp
	ltmemdebug.cpp
	ltmemheap.cpp
	ltmemstats.cpp
	ltmemtrack.cpp
	stdafx.cpp)

include_directories(../../sdk/inc
	../../runtime/shared/src
	../../runtime/kernel/src
	../stdlith
	../../runtime/kernel/mem/src
	${SDL2_INCLUDE_DIRS})

if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fpermissive -fPIC")
endif(LINUX)
