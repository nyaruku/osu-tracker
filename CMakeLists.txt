cmake_minimum_required(VERSION 3.15)
project(osu-tracker LANGUAGES C CXX)

set(CMAKE_BUILD_PARALLEL_LEVEL 16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(BASE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/includes")
set(BASE_LIB_PATH     "${CMAKE_SOURCE_DIR}/lib")

include_directories(${BASE_INCLUDE_PATH})

include(FetchContent)

set(CPR_BUILD_TESTS           OFF CACHE BOOL "" FORCE)
set(CPR_BUILD_EXAMPLES        OFF CACHE BOOL "" FORCE)
set(CPR_FORCE_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
set(CPR_USE_SYSTEM_CURL       OFF CACHE BOOL "" FORCE)

if (WIN32)
  set(CPR_USE_SYSTEM_CURL  OFF CACHE BOOL "" FORCE)
  set(CURL_USE_STATIC_LIBS ON  CACHE BOOL "" FORCE)
  set(BUILD_CURL_EXE       OFF CACHE BOOL "" FORCE)
  set(ENABLE_CURL_STATIC   ON  CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS    OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING        OFF CACHE BOOL "" FORCE)
  set(ENABLE_MANUAL        OFF CACHE BOOL "" FORCE)
  set(ENABLE_EXAMPLES      OFF CACHE BOOL "" FORCE)
  set(ENABLE_DOCS          OFF CACHE BOOL "" FORCE)
  set(ENABLE_TESTING       OFF CACHE BOOL "" FORCE)
else()
  set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.12.0
)
FetchContent_MakeAvailable(cpr)

if (WIN32)
  set(ZLIB_USE_STATIC_LIBS "ON")
  FetchContent_Declare(
    zlib
    URL https://github.com/madler/zlib.git
    GIT_TAG 1.3.1
  )
  FetchContent_MakeAvailable(zlib)
endif()

set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if (UNIX AND NOT APPLE)
  set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
  set(GLFW_BUILD_X11     ON CACHE BOOL "" FORCE)
  find_package(X11 REQUIRED)
endif()

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /RTC1")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   /Od /Zi /RTC1")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG    "${CMAKE_EXE_LINKER_FLAGS_DEBUG}    /DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -pedantic")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   -g -O0 -Wall -Wextra -pedantic")
  endif()
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
    set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   /O2 /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE    "${CMAKE_EXE_LINKER_FLAGS_RELEASE}    /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-changes-meaning")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-unused-function")
  endif()
endif()

add_subdirectory(osu-tracker-bin)
add_subdirectory(osu-tracker-updater)
