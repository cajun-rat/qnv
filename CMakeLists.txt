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

# Generate ctags
set_source_files_properties(tags PROPERTIES GENERATED true)
add_custom_target(tags
    COMMAND ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_dependencies(qnv tags)

message("Build type is ${CMAKE_BUILD_TYPE}")
