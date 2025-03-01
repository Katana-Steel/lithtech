project(LIB_RezMgr)

add_library(${PROJECT_NAME} STATIC
	rezfile.cpp
	rezhash.cpp
	rezmgr.cpp
	rezutils.cpp)

include_directories(../../sdk/inc
	../lith)
	
target_link_libraries(${PROJECT_NAME}
	LIB_Lith)

if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fpermissive -fPIC")
endif(LINUX)
