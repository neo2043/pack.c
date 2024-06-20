cmake_minimum_required(VERSION 3.9.2)

set(CMAKE_C_STANDARD 23)

set(CMAKE_C_COMPILER gcc)

project(pack.c
    VERSION 1.0.0
    LANGUAGES C)

    
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
    
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/cwalk)
link_libraries(cwalk)

# Add vendor-datastruct library
file(GLOB DATASTRUCT_SOURCES vendor/datastructs/*.c)
file(GLOB DATASTRUCT_HEADERS vendor/datastructs/*.h)
add_library(datastructs ${DATASTRUCT_SOURCES})
target_sources(datastructs INTERFACE ${DATASTRUCT_HEADERS})
target_include_directories(datastructs PUBLIC ${PROJECT_SOURCE_DIR}/vendor/datastructs)
link_libraries(datastructs)

# Add libraries
file(GLOB SQLITE_SOURCES vendor/sqlite/*.c)
file(GLOB SQLITE_HEADERS vendor/sqlite/*.h)
add_library(sqlite ${SQLITE_SOURCES})
target_sources(sqlite INTERFACE ${SQLITE_HEADERS})
target_include_directories(sqlite PUBLIC ${PROJECT_SOURCE_DIR}/vendor/sqlite)
link_libraries(sqlite)

# Add libraries
file(GLOB LIB_SOURCES lib/*.c)
file(GLOB LIB_HEADERS lib/*.h)
add_library(LIB ${LIB_SOURCES})
target_sources(LIB INTERFACE ${LIB_HEADERS})
target_include_directories(LIB PUBLIC ${PROJECT_SOURCE_DIR}/lib)
link_libraries(LIB)

add_executable(pack pack.c)

if(WIN32)
    add_executable(test2 test/test2.c)
    add_executable(test3 test/test3.c)
    add_executable(test4 vendor/datastructs/tests.c)
elseif(UNIX)
    add_executable(test1 test/test1.c)
endif()
