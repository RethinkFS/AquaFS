cmake_minimum_required(VERSION 3.20)

set(AQUAFS_VERSION v0.0.2)

project(aquafs)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(AQUAFS_IS_PLUGIN OFF)
else ()
    set(AQUAFS_IS_PLUGIN ON)
endif ()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/AquaZFS)

file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/src/**.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(aqua STATIC ${sources})
target_link_libraries(aqua aquaz)

find_package(gflags REQUIRED)
add_definitions(-DGFLAGS=1)
include_directories(${GFLAGS_INCLUDE_DIR})

add_definitions(-DNUMA)
add_definitions(-DWITH_TERARKDB=1 -DOS_LINUX=1 -DROCKSDB_PLATFORM_POSIX=1)

file(GLOB programs ${CMAKE_CURRENT_SOURCE_DIR}/programs/*.cc)
foreach (program ${programs})
    cmake_path(GET program STEM name)
    add_executable(${name} ${program})
    target_link_libraries(${name} aqua)
endforeach ()

#file(GLOB programs_db ${CMAKE_CURRENT_SOURCE_DIR}/db/*.cc)
#foreach (program ${programs_db})
#    cmake_path(GET program STEM name)
#    add_executable(${name} ${program})
#    target_link_libraries(${name} aqua rocksdb)
#endforeach ()

# is subdirectory
if (AQUAFS_IS_PLUGIN)
    message("CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
    # Exporting as RocksDB plugin
    file(GLOB_RECURSE AquaFS_SOURCES_local
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/db/*.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/AquaZFS/base/**.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/AquaZFS/fs/**.cc
            )
    file(GLOB_RECURSE AquaFS_HEADERS_local
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/db/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/AquaZFS/base/**.h
            ${CMAKE_CURRENT_SOURCE_DIR}/AquaZFS/fs/**.h
            )
    set(AquaFS_LIBS_local "zbd" "gflags" "uring" "zstd" "lz4" "snappy" "bz2" "z" "numa")
    list(TRANSFORM AquaFS_SOURCES_local REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "")
    list(TRANSFORM AquaFS_HEADERS_local REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "")
    if (AQUAFS_EXPORT_PROMETHEUS)
        list(APPEND AquaFS_LIBS_local prometheus-cpp-pull prometheus-cpp-core)
    else ()
        list(REMOVE_ITEM AquaFS_SOURCES_local "AquaZFS/fs/metrics_prometheus.cc")
        list(REMOVE_ITEM AquaFS_HEADERS_local "AquaZFS/fs/metrics_prometheus.h")
    endif ()
    set(AquaFS_LIBS ${AquaFS_LIBS_local} PARENT_SCOPE)
    set(AquaFS_CMAKE_EXE_LINKER_FLAGS "-u aquafs_filesystems_reg" PARENT_SCOPE)
    set(AquaFS_INCLUDE_PATHS "fs" PARENT_SCOPE)
    set(AquaFS_HEADERS ${AquaFS_HEADERS_local} PARENT_SCOPE)
    set(AquaFS_SOURCES ${AquaFS_SOURCES_local} PARENT_SCOPE)
    #    message(STATUS "AquaFS_HEADERS_local = ${AquaFS_HEADERS_local}, AquaFS_SOURCES_local = ${AquaFS_SOURCES_local}")
    #    message(STATUS "AquaFS_HEADERS = ${AquaFS_HEADERS}, AquaFS_SOURCES = ${AquaFS_SOURCES}")
endif ()