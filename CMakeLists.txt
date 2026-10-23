cmake_minimum_required(VERSION 3.21)

project(
    WorldMapSelectorForController
    VERSION 1.0.0
    LANGUAGES CXX RC
)

# The DLL's VERSIONINFO resource, stamped from the project version (the version gate reads it).
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
    @ONLY
)

# Keep the package manifest aligned with the version used for plugin metadata
# and release archive names.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json" vcpkgManifest)
string(JSON vcpkgVersion GET "${vcpkgManifest}" "version-string")
if(NOT "${vcpkgVersion}" STREQUAL "${PROJECT_VERSION}")
    message(FATAL_ERROR
        "Project version ${PROJECT_VERSION} does not match vcpkg manifest version ${vcpkgVersion}.")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENABLE_SKYRIM_SE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_AE ON CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

# Debug symbols (rule 43) and no build-machine paths in the DLL (rule 45). /d1trimfile is set for every
# target in this tree, CommonLibSSE NG's included, so __FILE__ / std::source_location carry relative paths.
if(MSVC)
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" PROJECT_NATIVE_DIR)
    add_compile_options("/d1trimfile:${PROJECT_NATIVE_DIR}")
endif()

add_subdirectory(extern/CommonLibVR)

include("${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibVR/cmake/CommonLibSSE.cmake")

find_package(minhook CONFIG REQUIRED)
find_path(SIMPLEINI_INCLUDE_DIR SimpleIni.h REQUIRED)

add_library(
    ${PROJECT_NAME}
    SHARED
        src/ClassicMessageBox.cpp
        src/ClassicMessageBox.h
        src/Config.cpp
        src/DevBench/DevBenchAPI.cpp
        src/DevBench/DevBenchAPI.h
        src/DevBenchTool.cpp
        src/DevBenchTool.h
        src/Diagnostics.cpp
        src/Hooks.cpp
        src/Hooks.h
        src/MapChooser.cpp
        src/MapChooserInput.cpp
        src/MapChooserInput.h
        src/MapPanel.cpp
        src/MapPanel.h
        src/PreciseSlider.h
        src/SettingsPage.cpp
        src/SettingsPage.h
        src/SKSEMenuFramework.h
        src/Toggle.h
        src/MapMenuEvents.cpp
        src/MapMenuKeyHint.cpp
        src/MapMenuKeyHint.h
        src/MapMarkerOverride.cpp
        src/MapSelection.cpp
        src/MapSwitchFlow.cpp
        src/MapSwitchFlow.h
        src/PluginVersion.cpp
        src/RoutedMarkerOverride.cpp
        src/RoutedMarkerOverride.h
        src/Strings.cpp
        src/Strings.h
        src/Utilities.cpp
        src/Utilities.h
        src/WorldspaceCatalog.cpp
        src/WorldspaceOverride.cpp
        src/plugin.cpp
        "${CMAKE_CURRENT_BINARY_DIR}/version.rc"
)

# These are the target settings normally supplied by add_commonlibsse_plugin.
# PluginVersion.cpp keeps the explicit supported-runtime list and its generated
# SKSE metadata under project source control.
target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        __CMAKE_COMMONLIBSSE_PLUGIN=1
        WMS_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        WMS_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        WMS_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        WMS_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
)
target_link_libraries(${PROJECT_NAME} PUBLIC CommonLibSSE::CommonLibSSE)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "${CommonLibSSE_INCLUDE_DIRS}"
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        "${SIMPLEINI_INCLUDE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        minhook::minhook
)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
        "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/PCH.h"
)

# A full PDB beside the DLL (rule 43); /PDBALTPATH keeps the build folder out of the DLL's debug record (rule 45).
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zi)
    target_link_options(${PROJECT_NAME} PRIVATE /DEBUG:FULL /OPT:REF /OPT:ICF /PDBALTPATH:%_PDB%)
endif()

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/dist/SKSE/Plugins/WorldMapSelectorForController.ini"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/WorldMapSelectorForController.ini"
)

set(packagePluginDirectory "SKSE/Plugins")
set(packageLicenseDirectory "${packagePluginDirectory}/WorldMapSelectorForController/Licenses")

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${packagePluginDirectory}" COMPONENT ReleasePackage
    LIBRARY DESTINATION "${packagePluginDirectory}" COMPONENT ReleasePackage
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/dist/SKSE/Plugins/WorldMapSelectorForController.ini"
    DESTINATION "${packagePluginDirectory}"
    COMPONENT ReleasePackage
)

# The eleven language files for the chooser's text (rule 66).
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist/Interface"
    DESTINATION "."
    COMPONENT ReleasePackage
)

function(install_license sourcePath outputName)
    if(NOT EXISTS "${sourcePath}")
        message(FATAL_ERROR "Required license file was not found: ${sourcePath}")
    endif()

    install(
        FILES "${sourcePath}"
        DESTINATION "${packageLicenseDirectory}"
        RENAME "${outputName}.txt"
        COMPONENT ReleasePackage
    )
endfunction()

install_license("${CMAKE_CURRENT_SOURCE_DIR}/UNLICENSE" "WorldMapSelector-Unlicense")
install_license("${CMAKE_CURRENT_SOURCE_DIR}/src/DevBench/DevBenchAPI.LICENSE.txt" "DevBenchAPI-MIT")
install_license("${CMAKE_CURRENT_SOURCE_DIR}/LICENSING.md" "WorldMapSelector-Licensing")
install_license("${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibVR/COPYING" "CommonLibSSE-NG-GPL-3.0")
install_license("${CMAKE_CURRENT_SOURCE_DIR}/extern/CommonLibVR/EXCEPTIONS.md" "CommonLibSSE-NG-Exceptions")

set(vcpkgShareDirectory "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share")
install_license("${vcpkgShareDirectory}/directxmath/copyright" "directxmath")
install_license("${vcpkgShareDirectory}/directxtk/copyright" "directxtk")
install_license("${vcpkgShareDirectory}/fmt/copyright" "fmt")
install_license("${vcpkgShareDirectory}/minhook/copyright" "minhook")
install_license("${vcpkgShareDirectory}/nlohmann-json/copyright" "nlohmann-json")
install_license("${vcpkgShareDirectory}/rapidcsv/copyright" "rapidcsv")
install_license("${vcpkgShareDirectory}/simpleini/copyright" "simpleini")
install_license("${vcpkgShareDirectory}/spdlog/copyright" "spdlog")
install_license("${vcpkgShareDirectory}/toml11/copyright" "toml11")
install_license("${vcpkgShareDirectory}/xbyak/copyright" "xbyak")

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME "WorldMapSelectorForController-${PROJECT_VERSION}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${PROJECT_NAME};ReleasePackage;/")
include(CPack)
