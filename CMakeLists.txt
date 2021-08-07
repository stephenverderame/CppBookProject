# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_VERSION 17)

project ("HttpProject")

macro(external_add filename pkname)
	configure_file(${filename} "${CMAKE_SOURCE_DIR}/external/${pkname}-download/CMakeLists.txt")
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/${pkname}-download")
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/external/${pkname}-download")
	set(${pkname}_SUBDIRS "${CMAKE_SOURCE_DIR}/external/${pkname}-src"
	"${CMAKE_SOURCE_DIR}/external/${pkname}-build")
endmacro() 

#Install googletest
external_add(GTestCMakeLists.txt.in gtest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${gtest_SUBDIRS})


enable_testing()
include (CTest)

add_subdirectory ("HttpProject")
