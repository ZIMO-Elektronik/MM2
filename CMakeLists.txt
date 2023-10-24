cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY https://github.com/ZIMO-Elektronik/CMakeModules
  GIT_TAG v0.2.1
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  MM2
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES CXX)

set(MM2_RX_DEQUE_SIZE
    8uz-1uz
    CACHE STRING "Size of the receiver deque of mm2::rx::CrtpBase")

add_library(MM2 INTERFACE)
add_library(MM2::MM2 ALIAS MM2)

target_compile_features(MM2 INTERFACE cxx_std_23)

target_compile_definitions(MM2 INTERFACE MM2_RX_DEQUE_SIZE=${MM2_RX_DEQUE_SIZE})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  target_include_directories(MM2 INTERFACE include)
else()
  target_include_directories(MM2 SYSTEM INTERFACE include)
endif()

target_common_warnings(MM2 INTERFACE)

if(NOT TARGET ZTL::ZTL)
  cpmaddpackage("gh:ZIMO-Elektronik/ZTL@0.18.0")
endif()

target_link_libraries(MM2 INTERFACE ZTL::ZTL)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

if(BUILD_TESTING
   AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME
   AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(examples)
  add_subdirectory(tests)
endif()
