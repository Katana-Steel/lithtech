project(LIB_GenRegMgr)

add_library(${PROJECT_NAME} STATIC
	genregmgr.cpp)

include_directories(../lith
    ${CMAKE_SOURCE_DIR}/cppcodec
    ${CMAKE_SOURCE_DIR}/json/single_include)

