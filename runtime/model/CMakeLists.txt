project(LIB_Model)

find_package(SDL2 REQUIRED)

set(libsources
	src/animtracker.cpp
	src/model.cpp
	src/model_load.cpp
	src/model_ops.cpp
	src/modelallocations.cpp
	src/transformmaker.cpp)

if(ENABLE_D3D)
	set(libsources ${libsources}
		src/sys/d3d/d3d_model_load.cpp)
	include_directories(sys/d3d)
endif(ENABLE_D3D)

include_directories(src
	../../sdk/inc
	../../libs/stdlith
	../shared/src
	../kernel/src
	../kernel/mem/src
	../lithtemplate
	../world/src
	../kernel/src/sys/win
	../shared/src/sys/win
	../../tools/shared/model
	${SDL2_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC ${libsources})