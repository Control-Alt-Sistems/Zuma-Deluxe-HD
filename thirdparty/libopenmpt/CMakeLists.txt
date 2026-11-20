# libopenmpt (release sources) built as a static library for ZumaHD.
# Only what the game needs: module playback through the C API.
# Vorbis-compressed samples in MO3 files use the ogg/vorbis targets of the parent project.

file(GLOB LIBOPENMPT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/plugins/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/plugins/dmo/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sounddsp/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/openmpt/soundfile_write/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_c.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_cxx.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_impl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_ext_impl.cpp"
)

add_library(libopenmpt STATIC ${LIBOPENMPT_SOURCES})

target_compile_features(libopenmpt PRIVATE cxx_std_17)

# the game includes <libopenmpt/libopenmpt.h>
target_include_directories(libopenmpt
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(libopenmpt
    PRIVATE
        LIBOPENMPT_BUILD
        MPT_WITH_OGG
        MPT_WITH_VORBIS
        MPT_WITH_VORBISFILE
)

target_link_libraries(libopenmpt PRIVATE ogg vorbis vorbisfile)

if(MSVC)
    # sources are UTF-8, some translation units are large
    target_compile_options(libopenmpt PRIVATE /utf-8 /bigobj /EHsc)
endif()

if(WIN32)
    # mpt::UUID uses UuidCreate
    target_link_libraries(libopenmpt PRIVATE rpcrt4)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(libopenmpt PRIVATE m)
endif()
