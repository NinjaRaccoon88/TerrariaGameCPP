cmake_minimum_required(VERSION 3.1)
project(fastNoiseSIMD)

# sets MY_SOURCES to be a list of all the source files
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/FastNoiseSIMD/*.cpp")

add_library(fastNoiseSIMD)

# Separate the AVX2 file and give it special compile flags
foreach(SRC IN LISTS MY_SOURCES)
    if(SRC MATCHES "FastNoiseSIMD_avx2.cpp$")
        set(AVX2_SRC ${SRC})
        list(REMOVE_ITEM MY_SOURCES ${SRC})
    endif()
endforeach()

target_sources(fastNoiseSIMD PRIVATE ${MY_SOURCES})

# Compile AVX2 file with AVX2 enabled
if(MSVC)
    set_source_files_properties(${AVX2_SRC} PROPERTIES COMPILE_FLAGS "/arch:AVX2")
else()
    set_source_files_properties(${AVX2_SRC} PROPERTIES COMPILE_FLAGS "-mavx2")
endif()

target_sources(fastNoiseSIMD PRIVATE ${AVX2_SRC})
target_include_directories(fastNoiseSIMD PUBLIC "FastNoiseSIMD")