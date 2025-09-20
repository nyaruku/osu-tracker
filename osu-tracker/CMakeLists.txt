# THIS IS A PILE OF JUNK
cmake_minimum_required(VERSION 3.15)
project(osu-tracker LANGUAGES C CXX)

# Threads to use
set(CMAKE_BUILD_PARALLEL_LEVEL 16) # got a new cpu :3

set(CMAKE_CXX_STANDARD 20) # Set the C++ standard to C++20
set(CMAKE_C_STANDARD 11) # Set the C standard to C11

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Base directories
set(BASE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/includes")

# Add the include directories
include_directories(${BASE_INCLUDE_PATH})

file(COPY ${CMAKE_SOURCE_DIR}/www         DESTINATION "${CMAKE_BINARY_DIR}/osu-tracker/")
file(COPY ${CMAKE_SOURCE_DIR}/tracker_txt DESTINATION "${CMAKE_BINARY_DIR}/osu-tracker/")

# Dependencies
option(USE_STATIC_LIB "Use static library" ON)

include(FetchContent)

# Ensure static CPR
set(CPR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CPR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CPR_FORCE_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)

# Windows doesn't have curl installed by default
if (WIN32)
  set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
  # Tell CPR to build curl statically | IMPORTANT FOR WINDOWS
  set(CURL_USE_STATIC_LIBS ON CACHE BOOL "" FORCE)
  set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
  set(ENABLE_CURL_STATIC ON CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(ENABLE_MANUAL OFF CACHE BOOL "" FORCE)
  set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(ENABLE_DOCS OFF CACHE BOOL "" FORCE)
  set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  # idk how many of these actually do something
else()
  set(CPR_USE_SYSTEM_CURL ON  CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.12.0
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Unix like already have zlib most of the time
if (WIN32)
  set(ZLIB_USE_STATIC_LIBS "ON")
  #set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)
  FetchContent_Declare(
    zlib
    URL https://github.com/madler/zlib.git
    GIT_TAG 1.3.1
  )
  FetchContent_MakeAvailable(zlib)
endif()

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

if (UNIX AND NOT APPLE)
  set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
  set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
  find_package(X11 REQUIRED)
endif()

find_package(OpenGL REQUIRED)

set(PROJECT_SOURCES
  header/ui/ui.h
        header/ui/draw.h
  includes/imgui/imgui_single_file.h
  includes/imgui/imgui_impl_glfw.cpp
  includes/imgui/imgui_impl_opengl2.cpp
)

add_executable(update update.cpp lib/miniz.c)
if (WIN32)
  # Use the WIN32 keyword so the console window is hidden in Release builds.
  add_executable(osu-tracker main.cpp ${PROJECT_SOURCES} res/resources.rc)
  add_executable(osu-tracker-test test.cpp ${PROJECT_SOURCES} res/resources.rc)

  target_compile_definitions(osu-tracker PRIVATE 
    OSU_TRACKER_PLATFORM="windows" 
    _WIN32 
    CURL_STATICLIB 
    OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}"
  )
  target_link_libraries(osu-tracker PRIVATE  
    cpr::cpr
    zlib
    ws2_32
    glfw
    OpenGL::GL
    user32 gdi32 Msimg32 crypt32 advapi32
  )
  target_compile_definitions(osu-tracker-test PRIVATE 
    OSU_TRACKER_PLATFORM="windows" 
    _WIN32 
    CURL_STATICLIB 
    OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}"
  )
  target_link_libraries(osu-tracker-test PRIVATE  
    cpr::cpr
    zlib
    ws2_32
    glfw
    OpenGL::GL
    user32 gdi32 Msimg32 crypt32 advapi32
  )
else()
  add_executable(osu-tracker main.cpp ${PROJECT_SOURCES})
  add_executable(osu-tracker-test test.cpp ${PROJECT_SOURCES})

  find_package(ZLIB REQUIRED)  # Find zlib on Linux
  target_compile_definitions(osu-tracker PRIVATE 
    OSU_TRACKER_PLATFORM="linux" 
    __linux__ 
    OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_SYSTEM_PROCESSOR}"
  )
  target_link_libraries(osu-tracker PRIVATE 
    cpr::cpr 
    ZLIB::ZLIB
    glfw 
    OpenGL::GL 
    ${X11_LIBRARIES}
  )
  target_compile_definitions(osu-tracker-test PRIVATE 
    OSU_TRACKER_PLATFORM="linux" 
    __linux__ 
    OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_SYSTEM_PROCESSOR}")
  target_link_libraries(osu-tracker-test PRIVATE 
    cpr::cpr 
    ZLIB::ZLIB
    glfw 
    OpenGL::GL 
    ${X11_LIBRARIES}
  )
endif()

target_include_directories(osu-tracker SYSTEM PRIVATE ${BASE_INCLUDE_PATH})
target_include_directories(update SYSTEM PRIVATE ${BASE_INCLUDE_PATH})
target_include_directories(osu-tracker-test SYSTEM PRIVATE ${BASE_INCLUDE_PATH})

if (WIN32)
  target_link_libraries(update PRIVATE zlib)
else()
  target_link_libraries(update PRIVATE z)
endif()

# TESTS
set_target_properties(osu-tracker-test PROPERTIES FOLDER "Tests")
target_link_libraries(osu-tracker-test PRIVATE Catch2::Catch2WithMain)

foreach(_t osu-tracker update osu-tracker-test)
  set_target_properties(${_t} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/osu-tracker"  # keep assets & exe together
  )
endforeach()

# Build and compiler definitions
# Could be simplified more, but cmake is clusterfuck
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG_BUILD)
  if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /RTC1")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Od /Zi /RTC1")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -pedantic")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -Wall -Wextra -pedantic")
  endif()
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_definitions(RELEASE_BUILD)
  if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-changes-meaning")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-unused-function")
  endif()
endif()

# WEB SERVER DEFINITIONS 
add_compile_definitions(OSU_TRACKER_ENABLE_WEBSERVER=1) # Legacy flag due to webserver was an optional feature
add_compile_definitions(OSU_TRACKER_WEBSERVER_IP="127.0.0.1")
add_compile_definitions(OSU_TRACKER_WEBSERVER_PORT=10727)
    
# Settings
add_compile_definitions(OSU_TRACKER_VERSION_SIGNED="50")
add_compile_definitions(OSU_TRACKER_UPDATE_MASTER=0) # Debug flag to check master branch
add_compile_definitions(OSU_TRACKER_UPDATE_EQUAL=0) # Debug flag to download update on same signed version

# OSU_TRACKER PROJECT STATIC STRINGS
add_compile_definitions(OSU_TRACKER_NAME="osu! Tracker")
add_compile_definitions(OSU_TRACKER_VERSION="v0.5.0")
add_compile_definitions(OSU_TRACKER_TITLE="osu! Tracker v0.5.0")
add_compile_definitions(OSU_TRACKER_RELEASE_TYPE="Beta")

# CMAKE Info
add_compile_definitions(OSU_TRACKER_CMAKE_GENERATOR="${CMAKE_GENERATOR}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_STANDARD="C++ ${CMAKE_CXX_STANDARD}")
add_compile_definitions(OSU_TRACKER_CMAKE_VERSION="${CMAKE_VERSION}")
add_compile_definitions(OSU_TRACKER_CMAKE_MINIMUM_REQUIRED_VERSION="${CMAKE_MINIMUM_REQUIRED_VERSION}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}")
add_compile_definitions(OSU_TRACKER_CMAKE_SYSTEM_NAME="${CMAKE_SYSTEM_NAME}")
add_compile_definitions(OSU_TRACKER_CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}")
