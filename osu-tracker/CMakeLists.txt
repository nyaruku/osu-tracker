# THIS IS A PILE OF JUNK
cmake_minimum_required(VERSION 3.15)
project(osu-tracker LANGUAGES C CXX)

# Threads to use
set(CMAKE_BUILD_PARALLEL_LEVEL 16) # got a new cpu :3

set(CMAKE_CXX_STANDARD 20) # Set the C++ standard to C++20
set(CMAKE_C_STANDARD 23) # Set the C standard to C23

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Base directories
set(BASE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/includes")

# Add the include directories for the project
include_directories(${BASE_INCLUDE_PATH})

# CMAKE
option(USE_STATIC_LIB "Use static library" ON)

file(COPY ${CMAKE_SOURCE_DIR}/www DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/osu-tracker/")
file(COPY ${CMAKE_SOURCE_DIR}/tracker_txt DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/osu-tracker/")

# Sources
file(GLOB_RECURSE CPR_SOURCES CONFIGURE_DEPENDS lib/cpr/*.cpp)
file(GLOB_RECURSE ZLIB_SOURCES CONFIGURE_DEPENDS lib/zlib/*.c)
file(GLOB_RECURSE MINIZ_SOURCES CONFIGURE_DEPENDS lib/miniz.c)

# Headers
file(GLOB_RECURSE CPR_HEADERS CONFIGURE_DEPENDS lib/cpr/*.h)
file(GLOB_RECURSE CURL_HEADERS CONFIGURE_DEPENDS lib/curl/lib/*.h)
file(GLOB_RECURSE ZLIB_HEADERS CONFIGURE_DEPENDS lib/zlib/*.h)
file(GLOB_RECURSE MINIZ_HEADERS CONFIGURE_DEPENDS includes/miniz.h)
file(GLOB_RECURSE NUKLEAR_HEADER CONFIGURE_DEPENDS includes/nuklear/nuklear.h)
file(GLOB_RECURSE NUKLEAR_HEADER_WIN CONFIGURE_DEPENDS includes/nuklear/nuklear_gdi.h)
file(GLOB_RECURSE NUKLEAR_HEADER_LINUX CONFIGURE_DEPENDS includes/nuklear/nuklear_glfw_gl2.h)
file(GLOB_RECURSE NUKLEAR_HEADER_GLFW CONFIGURE_DEPENDS includes/nuklear/glfw3.h)
file(GLOB_RECURSE NUKLEAR_HEADER_GLFW_NATIVE CONFIGURE_DEPENDS includes/nuklear/glfw3native.h)

# No MacOS support, cant test
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	add_compile_definitions(OSU_TRACKER_PLATFORM="windows")
	add_compile_definitions(_WIN32)
	add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")

	# Static libcurl, libcrypto and libssl (windows only)
	# Using system curl and zlib on linux
	set(LIBCURL_LIBRARY "${CMAKE_SOURCE_DIR}/lib/static/libcurl.lib")
	set(LIBCRYPTO_LIBRARY "${CMAKE_SOURCE_DIR}/lib/static/libcrypto_static.lib")
	set(LIBSSL_LIBRARY "${CMAKE_SOURCE_DIR}/lib/static/libssl_static.lib")
	add_executable(osu-tracker main.cpp
		${CPR_SOURCES}
		${CPR_HEADERS}
		${CURL_HEADERS}
		${ZLIB_SOURCES}
		${ZLIB_HEADERS}
		${NUKLEAR_HEADER}
		${NUKLEAR_HEADER_WIN}
		header/ui/ui_windows.c
	)
	target_sources(osu-tracker PRIVATE "${CMAKE_SOURCE_DIR}/res/resources.rc")
	add_executable(update update.cpp
		${ZLIB_SOURCES}
		${ZLIB_HEADERS}
		${MINIZ_SOURCES}
		${MINIZ_HEADERS}
	)
	target_link_libraries(update)
	# static libcurl
	add_definitions(-DCURL_STATICLIB)
	# static link for tracker
	target_link_libraries(osu-tracker PRIVATE user32.lib gdi32.lib Msimg32.lib crypt32 advapi32 ${LIBCRYPTO_LIBRARY} ${LIBSSL_LIBRARY} ${LIBCURL_LIBRARY})

	# Include bundled libraries
	include_directories(
		${CMAKE_SOURCE_DIR}/lib/cpr/cpr
		${CMAKE_SOURCE_DIR}/lib/cpr/include
		${CMAKE_SOURCE_DIR}/lib/curl/include
		${CMAKE_SOURCE_DIR}/lib/zlib
	)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_compile_definitions(OSU_TRACKER_PLATFORM="linux")
	add_compile_definitions(__linux__)
	set(LIBGLFW3 "${CMAKE_SOURCE_DIR}/lib/static/libglfw3.a")
	add_compile_definitions(OSU_TRACKER_CMAKE_CXX_COMPILER_ARCHITECTURE_ID="${CMAKE_SYSTEM_PROCESSOR}")
	find_package(CURL REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(X11 REQUIRED)
	add_executable(osu-tracker main.cpp
		${CPR_SOURCES}
		${CPR_HEADERS}
		${NUKLEAR_HEADER}
		${NUKLEAR_HEADER_LINUX}
		header/ui/ui_linux.c
	)
	add_executable(update update.cpp
		${MINIZ_SOURCES}
		${MINIZ_HEADERS}
	)
	target_link_libraries(update PRIVATE ${ZLIB_LIBRARIES})
	
	# Include system headers
	include_directories(${CURL_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${X11_INCLUDE_DIR})
	target_link_libraries(osu-tracker PRIVATE ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${X11_LIBRARIES} ${LIBGLFW3} m GL)
	include_directories(
		${CMAKE_SOURCE_DIR}/lib/cpr/cpr
		${CMAKE_SOURCE_DIR}/lib/cpr/include
	)
endif()
	
set_target_properties(osu-tracker PROPERTIES OUTPUT_NAME "osu-tracker" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/osu-tracker")
set_target_properties(update PROPERTIES OUTPUT_NAME "update" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/osu-tracker")
	
# CMAKE BUILD AND COMPILER DEFINITIONS
add_compile_definitions(OSU_TRACKER_CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_compile_definitions(DEBUG_BUILD)
	if(MSVC)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		# Linux (GCC/Clang) specific optimizations
		#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-changes-meaning")
		#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-unused-function -D_POSIX_C_SOURCE=200809L")
	endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_definitions(RELEASE_BUILD)
	if(MSVC)
		# Windows (MSVC) specific optimizations
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /GL")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		# Linux (GCC/Clang) specific optimizations
		#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-changes-meaning")
		#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-unused-function -D_POSIX_C_SOURCE=200809L")
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