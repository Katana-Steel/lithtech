project(LIB_OGLRender)

cmake_minimum_required(VERSION 3.1)

include(FindPkgConfig)

pkg_check_modules(SDL2 REQUIRED sdl2)


add_library(${PROJECT_NAME} SHARED
    common_init.cpp
    ltpixelshadermgr.cpp
    ltvertexshadermgr.cpp
    ogl_render.cpp
    rendererframestats.cpp)

include_directories(.
    ${SDL2_INCLUDE_DIRS}
	../../../../../sdk/inc
	../../../../../sdk/inc/physics
	../../../../../libs/stdlith
	../../../../shared/src
	../../../../shared/src/sys/linux
	../../../../kernel/src
	../../../../kernel/mem/src
	../../../../kernel/src/sys/linux
	../../../../kernel/io/src
	../../../../client/src
	../../../../world/src
	../../../../model/src
	../../../../lithtemplate)



if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS -std=c++11 ${CMAKE_CXX_FLAGS})
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fpermissive -fPIC")
endif(CMAKE_COMPILER_IS_GNUCXX)

target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    GLX
    GL)