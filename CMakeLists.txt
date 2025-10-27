cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.10.0
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  MM2
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES CXX)

set(MM2_RX_DEQUE_SIZE
    7uz
    CACHE STRING "Size of the receiver deque of decoder")

add_library(MM2 INTERFACE)
add_library(MM2::MM2 ALIAS MM2)

target_compile_features(MM2 INTERFACE cxx_std_23)

target_compile_definitions(MM2 INTERFACE MM2_RX_DEQUE_SIZE=${MM2_RX_DEQUE_SIZE})

if(PROJECT_IS_TOP_LEVEL)
  target_include_directories(MM2 INTERFACE include)
  target_common_warnings(MM2 INTERFACE)
  target_common_errors(MM2 INTERFACE)
else()
  target_include_directories(MM2 SYSTEM INTERFACE include)
endif()

if(NOT TARGET ZTL::ZTL)
  cpmaddpackage("gh:ZIMO-Elektronik/ZTL@0.23.1")
endif()

target_link_libraries(MM2 INTERFACE ZTL::ZTL)

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
  add_subdirectory(examples)
  file(
    DOWNLOAD
    "https://github.com/ZIMO-Elektronik/.github/raw/master/data/.clang-format"
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
  file(GLOB_RECURSE SRC examples/*.[ch]pp include/*.[ch]pp tests/*.[ch]pp)
  add_clang_format_target(MM2Format OPTIONS -i FILES ${SRC})
endif()

if(BUILD_TESTING
   AND PROJECT_IS_TOP_LEVEL
   AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(tests)
endif()
