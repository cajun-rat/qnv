cmake_minimum_required(VERSION 2.8.8)
project(qnv)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# Tell CMake to run the UI compiler when necessary
SET(CMAKE_AUTOUIC ON) 

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-std=gnu++0x -Wall -Wextra -Werror")

# Widgets finds its own dependencies.
find_package(Qt5Widgets REQUIRED)

# Boost
find_package(Boost REQUIRED filesystem system random)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(qnv main.cpp mainwindow.cpp notes.cpp mainwindow.ui)
target_link_libraries(qnv ${Boost_LIBRARIES})
qt5_use_modules(qnv Widgets)
