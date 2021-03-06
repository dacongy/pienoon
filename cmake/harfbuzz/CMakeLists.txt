# Copyright 2014 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
cmake_minimum_required(VERSION 2.8.7)

project(harfbuzz)

set(harfbuzz_dir ${dependencies_harfbuzz_distr_dir})

if(WIN32)
# Include pregeneraged harfbuzz header files on Windows.
# On Windows, building harfbuzz requires prerequisites set up properly
# (e.g.ragel, auto tools). To make the build process easier, we commit
# pre-generated header files to our repository and include them only on Windows.
include_directories(${CMAKE_SOURCE_DIR}/external/include/harfbuzz)
else()
# Execute configuration script and make
# Need ragel, autoconf, automake, libtool, pkg-config and freetype to configure
# This process is required to generate header files
set(configure_cmd "./autogen.sh")
set(make_cmd "make")
execute_process(COMMAND ${configure_cmd} WORKING_DIRECTORY ${harfbuzz_dir}
                RESULT_VARIABLE autogen_result)
execute_process(COMMAND ${make_cmd} WORKING_DIRECTORY ${harfbuzz_dir}
                RESULT_VARIABLE make_result)
if (NOT ${autogen_result} EQUAL 0 OR NOT ${make_result} EQUAL 0)
    message(FATAL_ERROR
            "CMake encountered an error while configuring harfbuzz.\n"
            "You must have autoconf, automake, libtool, pkg-config and ragel\n"
            "installed to build on Linux/OSX.")
endif()
endif()

add_definitions(-DHB_NO_MT)

# Include freetype header files.
include_directories(${dependencies_freetype_distr_dir}/include)

# Include harfbuzz header files.
include_directories(${harfbuzz_dir}/src)
include_directories(${harfbuzz_dir}/src/hb-ucdn)

# Include harfbuzz source files.
set(libharfbuzz_SRCS
    ${harfbuzz_dir}/src/hb-blob.cc
    ${harfbuzz_dir}/src/hb-buffer-serialize.cc
    ${harfbuzz_dir}/src/hb-buffer.cc
    ${harfbuzz_dir}/src/hb-common.cc
    ${harfbuzz_dir}/src/hb-face.cc
    ${harfbuzz_dir}/src/hb-font.cc
    ${harfbuzz_dir}/src/hb-ft.cc
    ${harfbuzz_dir}/src/hb-ot-tag.cc
    ${harfbuzz_dir}/src/hb-set.cc
    ${harfbuzz_dir}/src/hb-shape.cc
    ${harfbuzz_dir}/src/hb-shape-plan.cc
    ${harfbuzz_dir}/src/hb-shaper.cc
    ${harfbuzz_dir}/src/hb-ucdn/ucdn.c
    ${harfbuzz_dir}/src/hb-ucdn.cc
    ${harfbuzz_dir}/src/hb-unicode.cc
    ${harfbuzz_dir}/src/hb-warning.cc
    ${harfbuzz_dir}/src/hb-ot-font.cc
    ${harfbuzz_dir}/src/hb-ot-layout.cc
    ${harfbuzz_dir}/src/hb-ot-map.cc
    ${harfbuzz_dir}/src/hb-ot-shape.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-arabic.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-default.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-hangul.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-hebrew.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-indic.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-indic-table.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-myanmar.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-sea.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-thai.cc
    ${harfbuzz_dir}/src/hb-ot-shape-complex-tibetan.cc
    ${harfbuzz_dir}/src/hb-ot-shape-normalize.cc
    ${harfbuzz_dir}/src/hb-ot-shape-fallback.cc )

# Add definitions to use OpenType backend UCDN Unicode functions
add_definitions(-DHAVE_OT)
add_definitions(-DHAVE_UCDN)

add_library(libharfbuzz ${libharfbuzz_SRCS})
