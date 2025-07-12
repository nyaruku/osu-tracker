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

option(CPR_BUILD_TESTS    "Build CPR test suite" OFF)
option(CPR_BUILD_EXAMPLES "Build CPR examples"   OFF)
option(CPR_BUI "Build CPR examples"   OFF)

# Windows doesn't have curl installed by default
if (WIN32)
  set(CPR_USE_SYSTEM_CURL OFF CACHE BOOL "" FORCE)
else()
  set(CPR_USE_SYSTEM_CURL ON  CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG        1.12.0
)
FetchContent_MakeAvailable(cpr)


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



if (UNIX AND NOT APPLE)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(glfw)
  
  find_package(OpenGL REQUIRED)
  find_package(X11 REQUIRED)
endif()


set(PROJECT_SOURCES
  main.cpp
  header/ui/ui.h
  header/ui/extc.h
  header/ui/draw.h
  includes/nuklear/nuklear.h
  includes/nuklear/stb_image.h
  
)

# Platform‑specific UI bridge
if (WIN32)
  list(APPEND ${PROJECT_SOURCES} header/ui/ui_windows.c includes/nuklear/nuklear_gdi.h)
else()
  list(APPEND ${PROJECT_SOURCES} includes/nuklear/glfw3.h includes/nuklear/glfw3native.h header/ui/ui_linux.c includes/nuklear/nuklear_glfw_gl2.h)
endif()


if (WIN32)
  # Use the WIN32 keyword so the console window is hidden in Release builds.
  add_executable(osu-tracker WIN32 ${PROJECT_SOURCES} res/resources.rc)
else()
  add_executable(osu-tracker ${PROJECT_SOURCES})
endif()

# Platform‑specific compile definitions & link libraries
# No Mac-os lol
if (WIN32)
  target_compile_definitions(osu-tracker PRIVATE 
    OSU_TRACKER_PLATFORM="windows" 
    _WIN32 
    CURL_STATICLIB 
    OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")

  target_link_libraries(osu-tracker PRIVATE  
    cpr::cpr 
    zlib
    user32 gdi32 Msimg32 crypt32 advapi32)
else()
  target_compile_definitions(osu-tracker PRIVATE 
    OSU_TRACKER_PLATFORM="linux" 
    __linux__ 
    OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_SYSTEM_PROCESSOR}")

  target_link_libraries(osu-tracker PRIVATE 
    cpr::cpr 
    zlib
    glfw 
    OpenGL::GL 
    ${X11_LIBRARIES}
  )
endif()

target_include_directories(osu-tracker SYSTEM PRIVATE ${BASE_INCLUDE_PATH})

add_executable(update update.cpp lib/miniz.c)

target_include_directories(update SYSTEM PRIVATE ${BASE_INCLUDE_PATH})

target_link_libraries(update PRIVATE zlib)

foreach(_t osu-tracker update)
  set_target_properties(${_t} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/osu-tracker"  # keep assets & exe together
  )
endforeach()

# Build and compiler definitions
# Could be simplified more, but cmake is clusterfuck
add_compile_definitions(OSU_TRACKER_CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DEBUG_BUILD)
  
  if (MSVC)
    # Nothing yet
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-changes-meaning")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-unused-function")
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
	
# CMAKE Info
add_compile_definitions(OSU_TRACKER_CMAKE_GENERATOR="${CMAKE_GENERATOR}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_STANDARD="C++ ${CMAKE_CXX_STANDARD}")
add_compile_definitions(OSU_TRACKER_CMAKE_VERSION="${CMAKE_VERSION}")
add_compile_definitions(OSU_TRACKER_CMAKE_MINIMUM_REQUIRED_VERSION="${CMAKE_MINIMUM_REQUIRED_VERSION}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}")
add_compile_definitions(OSU_TRACKER_CMAKE_SYSTEM_NAME="${CMAKE_SYSTEM_NAME}")

# Auto Update Control
add_compile_definitions(OSU_TRACKER_VERSION_SIGNED="40")
add_compile_definitions(OSU_TRACKER_UPDATE_EQUAL=0) # Debug flag to download update on same signed version
add_compile_definitions(OSU_TRACKER_UPDATE_MASTER=0) # Debug flag to check master branch

# OSU_TRACKER PROJECT STATIC STRINGS
add_compile_definitions(OSU_TRACKER_NAME="osu! Tracker")
add_compile_definitions(OSU_TRACKER_VERSION="v0.4.0")
add_compile_definitions(OSU_TRACKER_TITLE="osu! Tracker v0.4.0")
add_compile_definitions(OSU_TRACKER_RELEASE_TYPE="Beta")
add_compile_definitions(OSU_TRACKER_CREATOR="Railgun")
add_compile_definitions(OSU_TRACKER_PROFILE="https://osu.ppy.sh/users/13817114")
add_compile_definitions(OSU_TRACKER_GITHUB="https://github.com/nyaruku/osu-tracker")
add_compile_definitions(OSU_TRACKER_DISCORD="https://discord.gg/FN6vauFTGA")

# LIBRARY INFO
add_compile_definitions(CROW_REPOSITORY_URL="https://github.com/CrowCpp/Crow/tree/e8b125159ec5e61384187e36bbf6d14f1fad504f")
add_compile_definitions(LIBCURL_REPOSITORY_URL="https://github.com/curl/curl")
add_compile_definitions(ASIO_REPOSITORY_URL="https://github.com/chriskohlhoff/asio")
add_compile_definitions(CPR_REPOSITORY_URL="https://github.com/libcpr/cpr")
add_compile_definitions(ZLIB_REPOSITORY_URL="https://github.com/madler/zlib")
add_compile_definitions(NLOHMANNJSON_REPOSITORY_URL="https://github.com/nlohmann/json")
