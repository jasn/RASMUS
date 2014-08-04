project("rasmus")
cmake_minimum_required(VERSION 2.8)

option(RASMUS_TESTS "Build with testing support" ON)
option(RASMUS_STATIC "Build rasmus static" OFF)

find_package(Qt4 4.4.3 COMPONENTS QtCore QtGui)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
find_package(LLVM REQUIRED)
find_package(Readline REQUIRED)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
include_directories(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src ${LLVM_INCLUDE_DIR})
link_directories(${LLVM_LIBRARY_DIR})
add_subdirectory(src)
if (RASMUS_TESTS)
   enable_testing()
   add_subdirectory(test)
endif (RASMUS_TESTS)

