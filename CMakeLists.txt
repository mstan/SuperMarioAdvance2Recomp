cmake_minimum_required(VERSION 3.20)
project(SuperMarioWorldRecomp VERSION 0.0.6 LANGUAGES C CXX)
set(GAME_BUILD_VERSION "${PROJECT_VERSION}" CACHE STRING "Packaged game version")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
if(BUILD_TESTING)
    add_executable(sma2_restoration_patch_test
        tests/restoration_patch_test.cpp src/smw_restoration_patch.cpp)
    target_include_directories(sma2_restoration_patch_test PRIVATE src)
    add_test(NAME sma2_restoration_patch COMMAND sma2_restoration_patch_test)
endif()

set(GBARECOMP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/gbarecomp"
    CACHE PATH "Path to this game's gbarecomp worktree")
set(RECOMP_UI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/recomp-ui"
    CACHE PATH "Path to the shared recomp-ui checkout")
set(GBARECOMP_RUNTIME_UI_ROOT "${RECOMP_UI_ROOT}"
    CACHE PATH "recomp-ui root used by the live GBA runtime adapter")
set(GBARECOMP_ENABLE_MODS ON CACHE BOOL
    "Enable Super Mario World's built-in mod catalog" FORCE)
if(NOT EXISTS "${GBARECOMP_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR
        "GBARECOMP_ROOT is not a gbarecomp checkout:\n"
        "  ${GBARECOMP_ROOT}\n"
        "Configure with -DGBARECOMP_ROOT=<path> if the local worktree link is absent.")
endif()

set(GBARECOMP_ENABLE_HQ2X ON CACHE BOOL "Include the optional HQx display mod" FORCE)
add_subdirectory("${GBARECOMP_ROOT}" gbarecomp_build EXCLUDE_FROM_ALL)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/recompiled.cpp")
    message(FATAL_ERROR
        "Stale monolithic generated/recompiled.cpp found. "
        "Run tools/regen.ps1 with the current gbarecomp worktree.")
endif()
file(GLOB GAME_GENERATED_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/*.cpp")

add_executable(SuperMarioWorldRecomp
    src/main.cpp
    src/smw_extended_view.cpp
    src/mods/smw_adaptive_view_plugin.cpp
    src/mods/smw_restoration_plugin.cpp
    src/smw_restoration_patch.cpp
    ${GAME_GENERATED_SOURCES})
gbarecomp_stage_hq2x(SuperMarioWorldRecomp)
if(TARGET gbarecomp_hqx)
    target_sources(SuperMarioWorldRecomp PRIVATE src/mods/hqx_plugin.cpp)
endif()

set(GAME_RECOMPILED_SOURCES ${GAME_GENERATED_SOURCES})
list(FILTER GAME_RECOMPILED_SOURCES INCLUDE REGEX
    "/recompiled_[0-9]+\\.cpp$")
if(GAME_RECOMPILED_SOURCES)
    set_source_files_properties(${GAME_RECOMPILED_SOURCES} PROPERTIES
        COMPILE_OPTIONS "-O1;-g0")
endif()

target_include_directories(SuperMarioWorldRecomp PRIVATE
    "${GBARECOMP_ROOT}/src/armv4t"
    "${GBARECOMP_ROOT}/src/gba"
    "${GBARECOMP_ROOT}/src/runtime"
    "${GBARECOMP_ROOT}/src/debug")

target_link_libraries(SuperMarioWorldRecomp PRIVATE
    "-Wl,--start-group"
    gbarecomp_runtime
    gbarecomp_debug
    gbarecomp_gba
    gbarecomp_armv4t
    "-Wl,--end-group")

target_compile_definitions(SuperMarioWorldRecomp PRIVATE
    GBARECOMP_DEFAULT_DEBUG_PORT=19877
    GBARECOMP_WINDOW_TITLE="SuperMarioWorldRecomp"
    GBARECOMP_DEFAULT_GAME_CONFIG="game.toml")
set_property(SOURCE src/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS
    GBARECOMP_GAME_VERSION="${GAME_BUILD_VERSION}")

option(GBAGAME_RECOMP_UI "Build the recomp-ui pre-boot launcher" ON)
if(GBAGAME_RECOMP_UI)
    include(${RECOMP_UI_ROOT}/recomp_ui.cmake)
    add_library(game_launcher_ui STATIC src/game_launcher_boot.cpp)
    target_include_directories(game_launcher_ui PRIVATE
        "${GBARECOMP_ROOT}/src/runtime"
        "${GBARECOMP_ROOT}/src/gba"
        src)
    target_compile_definitions(game_launcher_ui PRIVATE
        GBARECOMP_ENABLE_MODS=1)
    recomp_target_launcher_ui(game_launcher_ui CONSOLE gba)
    find_package(SDL2 REQUIRED)
    target_link_libraries(game_launcher_ui PUBLIC SDL2::SDL2)
    if(WIN32)
        target_link_libraries(game_launcher_ui PUBLIC opengl32)
    endif()
    target_link_libraries(SuperMarioWorldRecomp PRIVATE game_launcher_ui)
    set_property(SOURCE src/main.cpp APPEND PROPERTY
        COMPILE_DEFINITIONS GBAGAME_RECOMP_UI)
endif()

# Ship the game-owned adaptive-view and optional HQ2x packages beside the
# executable. User-installed .gbamod archives join this same catalog.
set(SMW_PRELOADED_MODS "${CMAKE_CURRENT_SOURCE_DIR}/mods/preloaded")
if(EXISTS "${SMW_PRELOADED_MODS}/packages")
    add_custom_command(TARGET SuperMarioWorldRecomp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SMW_PRELOADED_MODS}"
            "$<TARGET_FILE_DIR:SuperMarioWorldRecomp>/mods"
        COMMENT "Preloading the Super Mario World mod catalog")
endif()

if(MSVC)
    target_link_options(SuperMarioWorldRecomp PRIVATE /STACK:16777216)
elseif(MINGW OR WIN32)
    target_link_options(SuperMarioWorldRecomp PRIVATE -Wl,--stack,16777216)
endif()
