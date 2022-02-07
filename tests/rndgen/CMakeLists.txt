project(Test_RandomGen)

set(exec_src
    main.cpp)

set(libs
	LIB_Random)

include_directories(${CMAKE_SOURCE_DIR}/libs/RandomGen/src)

add_executable(${PROJECT_NAME} ${exec_src})
set_target_properties(${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME testRandGen)
target_link_libraries(${PROJECT_NAME} ${libs})

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ../../OUT/testRegMgr)
