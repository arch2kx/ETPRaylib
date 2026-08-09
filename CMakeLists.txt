cmake_minimum_required(VERSION 3.16)
project(EdenTreatyPandemonium)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 17)

# Download raylib automatically — no manual install required
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
# Suppress raylib's own examples/games from being built
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Native Wayland support (Linux only). Off by default - raylib/GLFW falls
# back to XWayland automatically, which works fine without any of this.
# Enable with: cmake -B build -DETP_USE_WAYLAND=ON
# Requires: wayland-protocols, libwayland-dev (or distro equivalent) and
# libxkbcommon-dev - see README. Do a clean rebuild after toggling this.
option(ETP_USE_WAYLAND "Build raylib with native Wayland support" OFF)
if(ETP_USE_WAYLAND AND UNIX AND NOT APPLE)
    set(USE_WAYLAND ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(raylib)

# --- macOS app icon: build a .icns from assets/mikaIcon.png -------------
# Runs at configure time using the built-in sips/iconutil tools. Delete
# build/icon.icns (or the whole build dir) and re-run cmake to regenerate
# it after changing the source image.
set(APP_ICON_ICNS "")
if(APPLE)
    set(ICON_SOURCE ${CMAKE_SOURCE_DIR}/assets/mikaIcon.png)
    set(ICONSET_DIR ${CMAKE_BINARY_DIR}/AppIcon.iconset)
    set(ICNS_FILE   ${CMAKE_BINARY_DIR}/icon.icns)

    if(EXISTS ${ICON_SOURCE})
        find_program(SIPS_EXE sips)
        find_program(ICONUTIL_EXE iconutil)
        if(SIPS_EXE AND ICONUTIL_EXE AND NOT EXISTS ${ICNS_FILE})
            file(MAKE_DIRECTORY ${ICONSET_DIR})
            foreach(SIZE 16 32 128 256 512)
                math(EXPR SIZE2X "${SIZE} * 2")
                execute_process(COMMAND ${SIPS_EXE} -z ${SIZE} ${SIZE} ${ICON_SOURCE}
                                 --out ${ICONSET_DIR}/icon_${SIZE}x${SIZE}.png
                                 OUTPUT_QUIET ERROR_QUIET)
                execute_process(COMMAND ${SIPS_EXE} -z ${SIZE2X} ${SIZE2X} ${ICON_SOURCE}
                                 --out ${ICONSET_DIR}/icon_${SIZE}x${SIZE}@2x.png
                                 OUTPUT_QUIET ERROR_QUIET)
            endforeach()
            execute_process(COMMAND ${ICONUTIL_EXE} -c icns ${ICONSET_DIR} -o ${ICNS_FILE})
            if(EXISTS ${ICNS_FILE})
                message(STATUS "Generated macOS app icon: ${ICNS_FILE}")
            else()
                message(WARNING "Failed to generate icon.icns from ${ICON_SOURCE}")
            endif()
        endif()
        if(EXISTS ${ICNS_FILE})
            set(APP_ICON_ICNS ${ICNS_FILE})
        endif()
    endif()
endif()

# --- Windows app icon: build a .ico from assets/mikaIcon.png ------------
# Requires ImageMagick's "magick" (or legacy "convert") on PATH. If it's
# not found, drop your own multi-resolution assets/mikaIcon.ico in place
# and this will pick it up instead. Re-run cmake after changing either.
set(APP_ICON_RC "")
if(WIN32)
    set(ICON_PNG_SOURCE ${CMAKE_SOURCE_DIR}/assets/mikaIcon.png)
    set(ICON_ICO_OVERRIDE ${CMAKE_SOURCE_DIR}/assets/mikaIcon.ico)
    set(ICO_FILE ${CMAKE_BINARY_DIR}/icon.ico)
    set(RC_FILE  ${CMAKE_BINARY_DIR}/icon.rc)

    if(NOT EXISTS ${ICO_FILE})
        if(EXISTS ${ICON_ICO_OVERRIDE})
            file(COPY ${ICON_ICO_OVERRIDE} DESTINATION ${CMAKE_BINARY_DIR})
            file(RENAME ${CMAKE_BINARY_DIR}/mikaIcon.ico ${ICO_FILE})
        elseif(EXISTS ${ICON_PNG_SOURCE})
            find_program(MAGICK_EXE NAMES magick convert)
            if(MAGICK_EXE)
                execute_process(
                    COMMAND ${MAGICK_EXE} ${ICON_PNG_SOURCE}
                            -define icon:auto-resize=256,128,64,48,32,16
                            ${ICO_FILE}
                    RESULT_VARIABLE MAGICK_RESULT
                    OUTPUT_QUIET ERROR_QUIET
                )
                if(MAGICK_RESULT EQUAL 0 AND EXISTS ${ICO_FILE})
                    message(STATUS "Generated Windows app icon: ${ICO_FILE}")
                else()
                    message(WARNING "Failed to generate icon.ico from ${ICON_PNG_SOURCE}")
                endif()
            else()
                message(STATUS "ImageMagick not found - skipping .ico generation. "
                                "Install ImageMagick, or place a ready-made icon at "
                                "assets/mikaIcon.ico, then re-run cmake.")
            endif()
        endif()
    endif()

    if(EXISTS ${ICO_FILE})
        file(WRITE ${RC_FILE} "IDI_ICON1 ICON \"${ICO_FILE}\"\n")
        set(APP_ICON_RC ${RC_FILE})
    endif()
endif()

add_executable(etp
    src/main.cpp
    src/player.cpp
    src/bullet.cpp
    src/enemy.cpp
    src/boss.cpp
    src/sniper.cpp
    src/ar.cpp
    src/game.cpp
    ${APP_ICON_ICNS}
    ${APP_ICON_RC}
)

target_link_libraries(etp raylib)

# Name the actual binary/app bundle "ETP" instead of the CMake target name
set_target_properties(etp PROPERTIES OUTPUT_NAME "ETP")

# Xcode 26 beta moved libc++ headers into the SDK only; the compiler's
# built-in search path (/usr/include/c++/v1) is empty. Point it at the SDK.
if(APPLE)
    execute_process(
        COMMAND xcrun --show-sdk-path
        OUTPUT_VARIABLE APPLE_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    target_compile_options(etp PRIVATE -isystem ${APPLE_SDK_PATH}/usr/include/c++/v1)

    # Bundle as a proper .app so macOS shows a Dock/Finder/desktop icon
    set_target_properties(etp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Eden Treaty Pandemonium"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.arch2kx.edentreatypandemonium"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
    if(APP_ICON_ICNS)
        set_source_files_properties(${APP_ICON_ICNS} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources")
        set_target_properties(etp PROPERTIES
            MACOSX_BUNDLE_ICON_FILE "icon.icns")
    endif()
endif()

# Hide the console window on Windows release builds
if(WIN32)
    set_target_properties(etp PROPERTIES WIN32_EXECUTABLE TRUE)
    if(MSVC)
        # WIN32_EXECUTABLE makes MSVC expect a WinMain entry point, but we
        # use a standard main(). Point the linker at mainCRTStartup instead.
        set_target_properties(etp PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
    endif()
endif()

# Copy assets next to the binary so relative paths work on all platforms
add_custom_command(TARGET etp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:etp>/assets
    COMMENT "Copying assets to build directory"
)
