project(LIB_UI)

set(libsources
	src/cuibase.cpp
	src/cuibase_impl.cpp
	src/cuibitmapfont.cpp
	src/cuibutton.cpp
	src/cuibutton_impl.cpp
	src/cuicheck.cpp
	src/cuicheck_impl.cpp
	src/cuidropdownlist.cpp
	src/cuidropdownlist_impl.cpp
	src/cuifont_impl.cpp
	src/cuifontmanager.cpp
	src/cuiformattedpolystring.cpp
	src/cuiformattedpolystring_impl.cpp
	src/cuiinterval.cpp
	src/cuiinterval_impl.cpp
	src/cuilinklist.cpp
	src/cuilist.cpp
	src/cuilist_impl.cpp
	src/cuimessagequeue.cpp
	src/cuioption.cpp
	src/cuioption_impl.cpp
	src/cuipolystring.cpp
	src/cuipolystring_impl.cpp
	src/cuipolytex.cpp
	src/cuiprogress.cpp
	src/cuiprogress_impl.cpp
	src/cuiredblack.cpp
	src/cuirenderstate.cpp
	src/cuislider.cpp
	src/cuislider_impl.cpp
	src/cuistaticimage.cpp
	src/cuistaticimage_impl.cpp
	src/cuistatictext.cpp
	src/cuistatictext_impl.cpp
	src/cuiwidget.cpp
	src/cuiwidget_impl.cpp
	src/cuiwidgetmanager.cpp
	src/cuiwindow.cpp
	src/cuiwindow_impl.cpp)

if(ENABLE_D3D)
	set(libsources ${libsources}
		src/sys/d3d/cuidebug.cpp)
	include_directories(sys/d3d)
endif(ENABLE_D3D)

if(LINUX)
find_package(SDL2 REQUIRED)
endif(LINUX)

include_directories(src
	../../sdk/inc
	../../libs/stdlith
	../shared/src
	../kernel/src
	../kernel/mem/src
	${SDL2_INCLUDE_DIRS})

if(WIN32)
	set(libsources ${libsources}
		src/sys/win/cuivectorfont.cpp)
    include_directories (../shared/src/sys/win
	    ../kernel/src/sys/win)
endif(WIN32)

if(UNIX)
	set(libsources ${libsources}
		src/sys/linux/cuivectorfont.cpp)
    include_directories (../shared/src/sys/linux
        ../kernel/src/sys/linux)
endif(UNIX)


add_library(${PROJECT_NAME} STATIC ${libsources})

if(LINUX)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-fpermissive -fPIC")
endif(LINUX)
