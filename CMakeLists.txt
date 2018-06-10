cmake_minimum_required(VERSION 2.8.8)
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

# Widgets finds its own dependencies.
find_package(Qt5Widgets REQUIRED)

# Boost
find_package(Boost REQUIRED filesystem system random)
include_directories(${Boost_INCLUDE_DIRS})

# clang-check and clang-format
find_program(CLANG_FORMAT NAMES clang-format-3.8 clang-format)
find_program(CLANG_TIDY NAMES clang-tidy-6.0 clang-tidy-3.8 clang-tidy)


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



# Debian Packaging
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "phil@phil-wise.com")
set(CPACK_PACKAGE_VERSION 1.1)
include(CPack)

# Export compile_commands.json for clang-check
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


message("Build type is ${CMAKE_BUILD_TYPE}")
