# CMakeList.txt : CMake project for CppBookTests2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Stores all cpp files in the src folder (not subfolders) into the variable SRC_FILES
file(GLOB SRC_FILES "src/*.cpp")

add_executable (HttpCmd ${SRC_FILES})
target_include_directories (HttpCmd PRIVATE "include")

if (MSVC)
    target_link_libraries (HttpCmd PRIVATE ws2_32)
    target_compile_definitions (HttpCmd PRIVATE WINDOWS)
    # If using MSVC, links to winsock2 and defines the macro WINDOWS
else ()
    target_compile_definitions (HttpCmd PRIVATE -DUNIX)
    # defines the macro UNIX
endif ()

add_subdirectory ("test")
