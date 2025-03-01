project(LIB_ToolModel)

find_package(SDL2 REQUIRED)

set(libsources
	model.cpp
	model_ops.cpp)

include_directories(../engine
	../../runtime/render_a/src/sys/d3d
	../../../sdk/inc
	../../../libs/stdlith
	../../../libs/ltamgr
	${SDL2_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC ${libsources})