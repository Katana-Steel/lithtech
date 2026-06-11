project(Test_CShell)
cmake_minimum_required(VERSION 3.16)

include(FindPkgConfig)
find_package(SDL2 REQUIRED)

set(exec_src
    src/shell.cpp)

set(libs
	${SDL2_LIBRARIES}
	LIB_Lith)

include_directories(../../sdk/inc
	${SDL2_INCLUDE_DIRS}
	../../libs/MFCStub
	../../libs/stdlith
	../../libs/ButeMgr
	../../libs/CryptMgr
	../../libs/lith
	../../libs/RegMgr
	../../libs/LTGUIMgr
	../../libs/ServerDir
)

add_library(${PROJECT_NAME} SHARED ${exec_src})

set_target_properties(${PROJECT_NAME}
        PROPERTIES OUTPUT_NAME CShell)

target_link_libraries(${PROJECT_NAME} ${libs})

