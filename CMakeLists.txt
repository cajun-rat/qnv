cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0043 NEW)
project(qnv)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
endif(CMAKE_COMPILER_IS_GNUCXX)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(Boost_USE_STATIC_LIBS   ON)
endif()

# Widgets finds its own dependencies.
find_package(Qt5Widgets REQUIRED)

# Boost
find_package(Boost REQUIRED filesystem system random)
include_directories(${Boost_INCLUDE_DIRS})

# clang-check and clang-format
find_program(CLANG_FORMAT NAMES clang-format)
find_program(CLANG_TIDY NAMES clang-tidy)


if(CLANG_FORMAT)
    function(qnv_clang_format)
        file(RELATIVE_PATH SUBDIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
        foreach(FILE ${ARGN})
            string(REPLACE "/" "_" TARGETNAME "qnv_clang_format-${SUBDIR}-${FILE}")
            add_custom_target(${TARGETNAME}
                COMMAND ${CLANG_FORMAT} -i -style=Google ${FILE}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                VERBATIM)
            add_dependencies(format ${TARGETNAME})
        endforeach()
    endfunction()
else()
    message(WARNING "clang-format not found, skipping")
    function(qnv_clang_format)
    endfunction()
endif()

add_custom_target(format)

add_subdirectory(src)

set(CPACK_PACKAGE_VERSION 1.1)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Debian Packaging
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "phil@phil-wise.com")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Bundle
    set(prefix "${PROJECT_NAME}.app/Contents")
    set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
    set(INSTALL_CMAKE_DIR "${prefix}/Resources")
    set(CPACK_BUNDLE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/src/icons.ico)


    macro(install_qt5_plugin _qt_plugin_name _qt_plugins_var _prefix)
        get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
        if(EXISTS "${_qt_plugin_path}")
            get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
            get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
            get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
            set(_qt_plugin_dest "${_prefix}/PlugIns/${_qt_plugin_type}")
            install(FILES "${_qt_plugin_path}"
                    DESTINATION "${_qt_plugin_dest}")
            set(${_qt_plugins_var}
                    "${${_qt_plugins_var}};\$ENV{DEST_DIR}\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
        else()
            message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
        endif()
    endmacro()


    install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS ${prefix})
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
            "[Paths]\nPlugins = ${_qt_plugin_dir}\n")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
            DESTINATION "${INSTALL_CMAKE_DIR}")

    set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")

    set(DIRS ${CMAKE_BINARY_DIR})
    list(APPEND DIRS "${Qt5Widgets_DIR}/../..")
    list(APPEND DIRS "${Qt5Core_DIR}/../..")

    include(InstallRequiredSystemLibraries)

    message(STATUS "APPS: ${APPS}")
    message(STATUS "QT_PLUGINS: ${QT_PLUGINS}")
    message(STATUS "DIRS: ${DIRS}")

    install(CODE "include(BundleUtilities)
        fixup_bundle(\"${APPS}\" \"${QT_PLUGINS}\" \"${DIRS}\")")

    set(CPACK_GENERATOR "DragNDrop")
else()
    message(WARNING "Unrecognised CMAKE_SYSTEM_NAME:${CMAKE_SYSTEM_NAME}. Disabling packaging via cpack.")
endif()


include(CPack)



# Export compile_commands.json for clang-check
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


message("Build type is ${CMAKE_BUILD_TYPE}")
