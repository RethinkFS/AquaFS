cmake_minimum_required(VERSION 3.20)

set(AQUAFS_VERSION v0.0.2)

project(aquafs)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(AQUAFS_IS_PLUGIN OFF)
else ()
    set(AQUAFS_IS_PLUGIN ON)
    add_definitions(-DAQUAFS_IS_PLUGIN=1)
endif ()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/AquaZFS)

if (NOT AQUAFS_IS_PLUGIN)
    if (AQUAFS_RAID_URING)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/liburing4cpp/include)
        set(AquaFS_COMPILE_FLAGS_local "${AquaFS_COMPILE_FLAGS_local} -DAQUAFS_RAID_URING=1 -Wno-non-virtual-dtor -Wno-shadow")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-fcoroutines)
        endif ()
        link_libraries("uring" "fmt" "Threads::Threads")
    endif ()

    if (USE_RTTI)
        add_definitions(-DUSE_RTTI=1)
    endif ()

    if (AQUAFS_DETECT_READ_OFFLINE)
        add_definitions(-DAQUAFS_DETECT_READ_OFFLINE=1)
    endif ()

    if (AQUAFS_EXPORT_METRICS)
        add_definitions(-DAQUAFS_EXPORT_METRICS=1)
    endif ()

    if (AQUAFS_SIM_DELAY)
        add_definitions(-DAQUAFS_SIM_DELAY=1)
    endif ()
endif ()

file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/src/**.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(aqua STATIC ${sources})
target_link_libraries(aqua aquaz)

find_package(gflags REQUIRED)
add_definitions(-DGFLAGS=1)
include_directories(${GFLAGS_INCLUDE_DIR})

add_definitions(-DNUMA)
add_definitions(-DWITH_TERARKDB=1 -DOS_LINUX=1 -DROCKSDB_PLATFORM_POSIX=1)

find_package(PkgConfig REQUIRED)
#find_package(FUSE3 REQUIRED)
pkg_search_module(fuse3 REQUIRED fuse3)
add_definitions(-D_FILE_OFFSET_BITS=64)

file(GLOB programs
        ${CMAKE_CURRENT_SOURCE_DIR}/programs/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/programs/*.c
        )
foreach (program ${programs})
    cmake_path(GET program STEM name)
    add_executable(${name} ${program})
    target_link_libraries(${name} aqua fuse3)
endforeach ()

#file(GLOB programs_db ${CMAKE_CURRENT_SOURCE_DIR}/db/*.cc)
#foreach (program ${programs_db})
#    cmake_path(GET program STEM name)
#    add_executable(${name} ${program})
#    target_link_libraries(${name} aqua rocksdb)
#endforeach ()

# is subdirectory
if (AQUAFS_IS_PLUGIN)
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
    set(AquaFS_LIBS_local "zbd")
    set(AquaFS_COMPILE_FLAGS_local "")
    set(AquaFS_INCLUDE_PATHS_local ${CMAKE_CURRENT_SOURCE_DIR})
    if (AQUAFS_RAID_URING)
        set(liburing4cpp_INC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/liburing4cpp/include)
        list(APPEND AquaFS_INCLUDE_PATHS_local ${liburing4cpp_INC_PATH})
        # include headers
        file(GLOB liburing4cpp_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/liburing4cpp/include/liburing/*.hpp)
        # append and remove prefix for file items
        foreach (file ${liburing4cpp_HEADERS})
            get_filename_component(file_name ${file} NAME)
            list(APPEND AquaFS_HEADERS_local "liburing4cpp/include/liburing/${file_name}")
        endforeach ()
        set(AquaFS_COMPILE_FLAGS_local "${AquaFS_COMPILE_FLAGS_local} -DAQUAFS_RAID_URING=1 -Wno-non-virtual-dtor -Wno-shadow")
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            set(AquaFS_COMPILE_FLAGS_local "${AquaFS_COMPILE_FLAGS_local} -fcoroutines")
        endif ()
        list(APPEND AquaFS_LIBS_local "uring" "fmt" "Threads::Threads")
    endif ()

    if (USE_RTTI)
        set(AquaFS_COMPILE_FLAGS_local "${AquaFS_COMPILE_FLAGS_local} -DUSE_RTTI=1")
    endif ()

    if (AQUAFS_DETECT_READ_OFFLINE)
        set(AquaFS_COMPILE_FLAGS_local "${AquaFS_COMPILE_FLAGS_local} -DAQUAFS_DETECT_READ_OFFLINE=1")
    endif ()

    if (AQUAFS_EXPORT_METRICS)
        set(AquaFS_COMPILE_FLAGS_local "${AquaFS_COMPILE_FLAGS_local} -DAQUAFS_EXPORT_METRICS=1")
    endif ()

    if (AQUAFS_SIM_DELAY)
        set(AquaFS_COMPILE_FLAGS_local "${AquaFS_COMPILE_FLAGS_local} -DAQUAFS_SIM_DELAY=1")
    endif ()
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
    set(AquaFS_COMPILE_FLAGS ${AquaFS_COMPILE_FLAGS_local} PARENT_SCOPE)
    set(AquaFS_INCLUDE_PATHS ${AquaFS_INCLUDE_PATHS_local} PARENT_SCOPE)
endif ()