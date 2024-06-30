cmake_minimum_required(VERSION 3.9.2)

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_COMPILER gcc)

project(pack.c
    VERSION 1.0.0
    LANGUAGES C)

    
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
file(GLOB LIB_SOURCES lib/*.c lib/**/*.c)
file(GLOB LIB_HEADERS lib/*.h lib/**/*.h)
add_library(LIB ${LIB_SOURCES})
target_sources(LIB INTERFACE ${LIB_HEADERS})
target_include_directories(LIB PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(LIB cwalk sqlite libzstd_static argparse)
link_libraries(LIB)

add_executable(pack pack.c)

add_executable(test4 test/test4.c)