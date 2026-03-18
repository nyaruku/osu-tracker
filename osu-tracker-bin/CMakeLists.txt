add_custom_target(copy_web_assets ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory  "${CMAKE_BINARY_DIR}/osu-tracker/www"
    COMMAND ${CMAKE_COMMAND} -E copy            "${CMAKE_CURRENT_SOURCE_DIR}/Httpdocs/base.html" "${CMAKE_BINARY_DIR}/osu-tracker/www/base.html"
    COMMAND ${CMAKE_COMMAND} -E copy_directory  "${CMAKE_CURRENT_SOURCE_DIR}/Httpdocs/pages"    "${CMAKE_BINARY_DIR}/osu-tracker/www"
    COMMAND ${CMAKE_COMMAND} -E copy_directory  "${CMAKE_CURRENT_SOURCE_DIR}/Httpdocs/static"   "${CMAKE_BINARY_DIR}/osu-tracker/www/static"
    COMMAND ${CMAKE_COMMAND} -E copy_directory  "${CMAKE_CURRENT_SOURCE_DIR}/Httpdocs/template" "${CMAKE_BINARY_DIR}/osu-tracker/www/template"
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/debug.env")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/debug.env" DESTINATION "${CMAKE_BINARY_DIR}/osu-tracker/")
endif()

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Txt" DESTINATION "${CMAKE_BINARY_DIR}/osu-tracker/")

set(IMGUI_SOURCES
    ${BASE_INCLUDE_PATH}/imgui/imgui.cpp
    ${BASE_INCLUDE_PATH}/imgui/imgui_draw.cpp
    ${BASE_INCLUDE_PATH}/imgui/imgui_widgets.cpp
    ${BASE_INCLUDE_PATH}/imgui/imgui_tables.cpp
    ${BASE_INCLUDE_PATH}/imgui/imgui_stdlib.cpp
    ${BASE_INCLUDE_PATH}/imgui/imgui_impl_glfw.cpp
    ${BASE_INCLUDE_PATH}/imgui/imgui_impl_opengl2.cpp
)

if (WIN32)
    add_executable(osu-tracker main.cpp ${IMGUI_SOURCES} Resources/Icons/resources.rc)
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
else()
    add_executable(osu-tracker main.cpp ${IMGUI_SOURCES})
    find_package(ZLIB REQUIRED)
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
endif()

target_include_directories(osu-tracker SYSTEM PRIVATE ${BASE_INCLUDE_PATH})
target_include_directories(osu-tracker SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(osu-tracker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/osu-tracker"
)

add_dependencies(osu-tracker copy_web_assets)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(osu-tracker PRIVATE DEBUG_BUILD)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(osu-tracker PRIVATE RELEASE_BUILD)
endif()

target_compile_definitions(osu-tracker PRIVATE
    # Web server
    OSU_TRACKER_WEBSERVER_ENABLE=1
    OSU_TRACKER_WEBSERVER_HOST="127.0.0.1"
    OSU_TRACKER_WEBSERVER_PORT=10727
    CROW_STATIC_DIRECTORY="www/static/"
    CROW_ENFORCE_WS_SPEC

    # Settings
    OSU_TRACKER_VERSION_SIGNED="50"
    OSU_TRACKER_UPDATE_MASTER=0
    OSU_TRACKER_UPDATE_EQUAL=0
    OSU_TRACKER_API_TIMEOUT=5000

    # Project strings
    OSU_TRACKER_NAME="osu! Tracker"
    OSU_TRACKER_VERSION="v0.5.0"
    OSU_TRACKER_TITLE="osu! Tracker v0.5.0"
    OSU_TRACKER_RELEASE_TYPE="Beta"

    # CMake info
    OSU_TRACKER_CMAKE_GENERATOR="${CMAKE_GENERATOR}"
    OSU_TRACKER_CMAKE_CXX_STANDARD="C++ ${CMAKE_CXX_STANDARD}"
    OSU_TRACKER_CMAKE_VERSION="${CMAKE_VERSION}"
    OSU_TRACKER_CMAKE_MINIMUM_REQUIRED_VERSION="${CMAKE_MINIMUM_REQUIRED_VERSION}"
    OSU_TRACKER_CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
    OSU_TRACKER_CMAKE_CXX_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}"
    OSU_TRACKER_CMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
    OSU_TRACKER_CMAKE_SYSTEM_NAME="${CMAKE_SYSTEM_NAME}"
    OSU_TRACKER_CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)
