cmake_minimum_required(VERSION 3.20)

set(AQUAFS_VERSION v0.0.2)

project(aquafs)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/AquaZFS)

file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/src/**.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(aqua STATIC ${sources})
target_link_libraries(aqua aquaz)

add_definitions(-DGFLAGS=1)

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