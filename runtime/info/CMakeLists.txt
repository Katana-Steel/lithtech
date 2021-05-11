project(LIB_Info)

set(libsources
	src/outputredir.cpp)

if(WIN32)
  set(libsources ${libsources}
	src/sys/win32/outputredir_win32.cpp)
else(WIN32)
  set(libsources ${libsources}
	src/sys/linux/outputredir_linux.cpp)
endif(WIN32)

add_library(${PROJECT_NAME} STATIC
    ${libsources})

include_directories(src
	../../sdk/inc
	../kernel/src
	../shared/src
	${SDL2_INCLUDE_DIRS})

set_target_properties(${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME libltinfo)

if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fpermissive -fPIC")
endif(LINUX)
