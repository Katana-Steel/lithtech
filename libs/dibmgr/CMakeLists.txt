project(LIB_dibmgr)

add_definitions(-D_AFXDLL -D_WIN32_WINNT=0x501)

set(libsources
	dib.cpp
	dibmgr.cpp
	dibpal.cpp)

add_library(${PROJECT_NAME} STATIC ${libsources})

include_directories(../../sdk/inc)