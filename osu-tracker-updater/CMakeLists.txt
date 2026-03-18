add_executable(update update.cpp ${BASE_LIB_PATH}/miniz.c)

target_include_directories(update SYSTEM PRIVATE ${BASE_INCLUDE_PATH})

if (WIN32)
    target_link_libraries(update PRIVATE zlib ws2_32)
else()
    target_link_libraries(update PRIVATE z)
endif()

set_target_properties(update PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/osu-tracker"
)

