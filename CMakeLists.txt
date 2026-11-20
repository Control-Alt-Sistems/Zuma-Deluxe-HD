cmake_minimum_required(VERSION 4.0...4.4)
project(
    ZumaHD
    LANGUAGES C CXX
    VERSION 0.1.0
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# deps exapt
set(EXPAT_BUILD_TESTS OFF)
set(EXPAT_BUILD_DOCS OFF)
set(EXPAT_BUILD_TOOLS OFF)

# deps sdl
add_subdirectory(thirdparty/expat/expat)
add_subdirectory(thirdparty/sdl2)

# deps ogg / vorbis 
set(INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(INSTALL_PKG_CONFIG_MODULE OFF CACHE BOOL "" FORCE)
set(INSTALL_CMAKE_PACKAGE_MODULE OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/ogg)

set(OGG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/ogg/include" CACHE PATH "" FORCE)
set(OGG_LIBRARY ogg CACHE STRING "" FORCE)

# vorbis 1.3.7 declares cmake_minimum_required(2.8.12), which CMake 4 rejects
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(thirdparty/vorbis)
unset(CMAKE_POLICY_VERSION_MINIMUM)

# deps libopenmpt (music)
add_subdirectory(thirdparty/libopenmpt)

# project conf
FILE(GLOB_RECURSE SOURCES RELATIVE ${PROJECT_SOURCE_DIR} "src/*.c")

set(MINIAUDIO_LIBVORBIS_DIR "${PROJECT_SOURCE_DIR}/thirdparty/miniaudio/extras/decoders/libvorbis")

add_executable(
    ZumaHD
    ${SOURCES}
    "${MINIAUDIO_LIBVORBIS_DIR}/miniaudio_libvorbis.c"
)

target_include_directories(
    ZumaHD PUBLIC

    "${PROJECT_SOURCE_DIR}/src"
)

# SYSTEM: warnings from these headers are not ours to fix
target_include_directories(
    ZumaHD SYSTEM PUBLIC

    "${PROJECT_SOURCE_DIR}/thirdparty/stb"
    "${PROJECT_SOURCE_DIR}/thirdparty/miniaudio"
    "${MINIAUDIO_LIBVORBIS_DIR}"
)

# compiler warnings, only for the game sources (not miniaudio_libvorbis.c or the deps)
option(ZUMAHD_WARNINGS_AS_ERRORS "Treat compiler warnings in the game sources as errors" OFF)

if(MSVC)
    set(ZUMAHD_WARNING_FLAGS /W4)
    if(ZUMAHD_WARNINGS_AS_ERRORS)
        list(APPEND ZUMAHD_WARNING_FLAGS /WX)
    endif()
else()
    set(ZUMAHD_WARNING_FLAGS -Wall -Wextra)
    if(ZUMAHD_WARNINGS_AS_ERRORS)
        list(APPEND ZUMAHD_WARNING_FLAGS -Werror)
    endif()
endif()

set_source_files_properties(${SOURCES} PROPERTIES COMPILE_OPTIONS "${ZUMAHD_WARNING_FLAGS}")

target_link_libraries(
    ZumaHD PRIVATE
    SDL2::SDL2
    expat
    vorbisfile
    libopenmpt
)

# miniaudio: no device (SDL plays the sound), no encoders/generators
target_compile_definitions(
    ZumaHD PRIVATE
    MA_NO_DEVICE_IO
    MA_NO_ENCODING
    MA_NO_GENERATION
)

# miniaudio backend deps (Windows/macOS need nothing extra)
if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(ZumaHD PRIVATE Threads::Threads m ${CMAKE_DL_LIBS})
endif()

# copy assets
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/content
    $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# SDL2 links with /NODEFAULTLIB, so MSVC /RTC checks must be off (CMP0184 moved them out of CMAKE_C_FLAGS_DEBUG)
set_target_properties(SDL2 PROPERTIES MSVC_RUNTIME_CHECKS "")
