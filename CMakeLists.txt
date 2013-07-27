project(RecentlyUsedSync)
cmake_minimum_required(VERSION 2.6)

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
link_directories(${GTK2_LIBRARY_DIRS})
# Add other flags to the compiler

set(CMAKE_INSTALL_PREFIX `kde4-config --prefix`)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${GTK2_CFLAGS_OTHER})
include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} . ${GTK2_INCLUDE_DIRS})


set(RecentlyUsedSync_SRCS main.cpp krecentdocument.cpp)

add_executable(RecentlyUsedSync ${RecentlyUsedSync_SRCS})
target_link_libraries(RecentlyUsedSync ${QT_QTCORE_LIBRARY} ${KDE4_KIO_LIBS} ${GTK2_LIBRARIES})
