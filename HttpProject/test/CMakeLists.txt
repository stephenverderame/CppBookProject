cmake_minimum_required (VERSION 3.8)

function(cp_dir source dest)
	make_directory (${dest})
	file (GLOB source_files RELATIVE "${source}" "${source}/*") #Files are specified relative to ${source}
	foreach (templateFile ${source_files})
		set (src_path ${source}/${templateFile})
		message (${src_path})
		if (NOT IS_DIRECTORY ${src_path})
			configure_file (${src_path} ${dest}/${templateFile} @ONLY)
		else ()
			cp_dir (${src_path} ${dest}/${templateFile})
		endif ()
	endforeach ()
endfunction ()

function (make_test NAME)
	set (BOOLEAN_ARGS "")
	set (ONEVALUE_ARGS "")
	set (MULTIVALUE_ARGS "SOURCES")
	cmake_parse_arguments(
		MK_TEST
		"${BOOLEAN_ARGS}"
		"${ONEVALUE_ARGS}"
		"${MULTIVALUE_ARGS}"
		${ARGN}
	)

	add_executable (${NAME} ${MK_TEST_SOURCES})

	add_dependencies (${NAME} gtest gmock) # Ensure this target is compiled after gtest and gmock
	link_target (${NAME})
	target_link_libraries (${NAME} PRIVATE GTest::gtest_main PRIVATE GTest::gmock)
	add_test (NAME ${NAME} COMMAND ${NAME} WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/HttpProject/test")

endfunction()

set (SOURCE_DIR ${PROJECT_SOURCE_DIR}/HttpProject/src)
set (TEST_SOURCES "${SOURCE_DIR}/Networking.cpp" 
	"${SOURCE_DIR}/SSLSocket.cpp" "${SOURCE_DIR}/Address.cpp")

make_test (SocketTest SOURCES "SocketTest.cpp" "${SOURCE_DIR}/Networking.cpp" 
	"${SOURCE_DIR}/SSLSocket.cpp" "${SOURCE_DIR}/Address.cpp")

make_test (ChunkedEncodingTest SOURCES "ChunkedTest.cpp" "${SOURCE_DIR}/Networking.cpp" 
	"${SOURCE_DIR}/SSLSocket.cpp" "${SOURCE_DIR}/Address.cpp")

make_test (QueryTest SOURCES "QueryTest.cpp" "${SOURCE_DIR}/Networking.cpp" 
	"${SOURCE_DIR}/SSLSocket.cpp" "${SOURCE_DIR}/Address.cpp")

cp_dir ("${CMAKE_CURRENT_SOURCE_DIR}/data" "${CMAKE_CURRENT_BINARY_DIR}/data")
# MSVC doesn't seem to support the WORKING_DIRECTORY flag on add_test
# so this copies any test data to the build directory