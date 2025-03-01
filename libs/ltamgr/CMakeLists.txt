project(LIB_ltamgr)

set(libsources
	ltabitfile.cpp
	ltacompressedfile.cpp
	ltaconverter.cpp
	ltafile.cpp
	ltafilebuffer.cpp
	ltahuffmantree.cpp
	ltaloadonlyalloc.cpp
	ltanode.cpp
	ltanodebuilder.cpp
	ltanodeiterator.cpp
	ltanodreader.cpp
	ltanodewriter.cpp
	ltareader.cpp
	ltautil.cpp
	ltawriter.cpp
	lzsswindow.cpp)

add_library(${PROJECT_NAME} STATIC ${libsources})

include_directories(../../sdk/inc)