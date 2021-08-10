# CMakeList.txt : CMake project for CppBookTests2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Stores all cpp files in the src folder (not subfolders) into the variable SRC_FILES
file(GLOB SRC_FILES "src/*.cpp")

add_executable (HttpCmd ${SRC_FILES})

# If this doesn't work, you may need to set the root directory manually:
# set (OPENSSL_ROOT_DIR "directory")
find_package (OpenSSL REQUIRED)
message ("OpenSSL Libs: ${OPENSSL_LIBRARIES}")
message ("OpenSSL Inc: ${OPENSSL_INCLUDE_DIR}")

function (link_target TARGET_NAME)
    target_include_directories (${TARGET_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/HttpProject/include")
    if (MSVC)
        target_link_libraries (${TARGET_NAME} PRIVATE ws2_32)
        target_compile_definitions (${TARGET_NAME} PRIVATE WINDOWS)
        # If using MSVC, links to winsock2 and defines the macro WINDOWS
    else ()
        target_compile_definitions (${TARGET_NAME} PRIVATE -DUNIX)
        # defines the macro UNIX, the -D just specifies that this is a 
        # preprocessor definition
    endif ()
    target_link_libraries (${TARGET_NAME} PRIVATE OpenSSL::SSL)
endfunction ()

if (MSVC)
    add_compile_options (/W4 /WX)
else ()
    add_compile_options (-Wall -Werror)
endif ()

link_target (HttpCmd)


add_subdirectory ("test")
