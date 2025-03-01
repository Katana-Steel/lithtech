project(LIB_ButeMgr)

add_library(${PROJECT_NAME} STATIC
	arange.cpp
	avector.cpp
	butemgr.cpp
	stdafx.cpp)

add_definitions(-D_AFXDLL -D_WIN32_WINNT=0x501)
if(MSVC)
	add_definitions(-D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)	# VC14 deprecated stdext::hash_*
endif(MSVC)
if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
endif(LINUX)

include_directories(.
    ../CryptMgr
	../stdlith
	../lith
	../MFCStub)
