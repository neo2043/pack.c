cmake_minimum_required(VERSION 3.9.2)

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_COMPILER clang)

project(pack.c
    VERSION 1.0.0
    LANGUAGES C)

# set(CMAKE_BUILD_TYPE "Release")

find_program(SCCACHE sccache REQUIRED)
set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
    
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/cwalk)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/argparse)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/zstd/build/cmake zstd)
target_include_directories(zstd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/zstd/lib/)

# Add libraries
set(SQLITE_DEFAULT_FOREIGN_KEYS ON)
file(GLOB SQLITE_SOURCES vendor/sqlite/*.c)
file(GLOB SQLITE_HEADERS vendor/sqlite/*.h)
add_library(sqlite ${SQLITE_SOURCES})
target_sources(sqlite INTERFACE ${SQLITE_HEADERS})
target_include_directories(sqlite PUBLIC ${PROJECT_SOURCE_DIR}/vendor/sqlite)

# Add libraries
file(GLOB DATASTRUCT_SOURCES vendor/datastructs/*.c)
file(GLOB DATASTRUCT_HEADERS vendor/datastructs/*.h)
add_library(datastructs ${DATASTRUCT_SOURCES})
target_sources(datastructs INTERFACE ${DATASTRUCT_HEADERS})
target_include_directories(datastructs PUBLIC ${PROJECT_SOURCE_DIR}/vendor/datastructs)

# Add libraries
file(GLOB LIB_SOURCES lib/*.c lib/**/*.c)
file(GLOB LIB_HEADERS lib/*.h lib/**/*.h)
add_library(LIB ${LIB_SOURCES})
target_sources(LIB INTERFACE ${LIB_HEADERS})
target_include_directories(LIB PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(LIB cwalk sqlite datastructs libzstd_static argparse pthread)
link_libraries(LIB)

add_executable(pack pack.c)
file(GLOB SOURCE_FILES RELATIVE ${PROJECT_SOURCE_DIR}/test test/*.c)
foreach (BIN_SOURCE ${SOURCE_FILES})
    string(REPLACE ".c" "" BIN ${BIN_SOURCE})
    target_compile_definitions(LIB PRIVATE DEBUG)
    add_executable(${BIN} test/${BIN_SOURCE})
    target_link_libraries(${BIN} LIB)
endforeach ()
