cmake_minimum_required(VERSION 2.8.8)
cmake_policy(SET CMP0043 NEW)
project(qnv)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-std=gnu++0x -Wall -Wextra -Werror")

# Widgets finds its own dependencies.
find_package(Qt5Widgets REQUIRED)

# Boost
find_package(Boost REQUIRED filesystem system random)
include_directories(${Boost_INCLUDE_DIRS})

qt5_wrap_ui(UI_MAINWINDOW mainwindow.ui)
add_executable(qnv main.cpp mainwindow.cpp notes.cpp ${UI_MAINWINDOW})
target_link_libraries(qnv ${Boost_LIBRARIES})
qt5_use_modules(qnv Widgets)

# Export compile_commands.json for clang-check
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
install(TARGETS qnv RUNTIME DESTINATION bin)

# Debian Packaging
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "phil@phil-wise.com")
set(CPACK_DEBIAN_PACKAGE_VERSION 1.0)
include(CPack)


message("Build type is ${CMAKE_BUILD_TYPE}")
