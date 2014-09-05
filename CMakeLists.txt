project("rasmus")
cmake_minimum_required(VERSION 2.8.8)

set(CMAKE_INSTALL_PREFIX "/usr/")

option(RASMUS_TESTS "Build with testing support" ON)
option(RASMUS_STATIC "Build rasmus static" OFF)

find_package(Qt4 4.4.3 COMPONENTS QtCore QtGui)
if (QT_FOUND)
   include(${QT_USE_FILE})
endif (QT_FOUND)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
find_package(LLVM REQUIRED)
find_package(Readline)
find_package(PythonInterp REQUIRED)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${LLVM_CFLAGS} -std=c++11 -Wall -Wextra")
include_directories(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src ${LLVM_INCLUDE_DIR})
link_directories(${LLVM_LIBRARY_DIR})

if (APPLE)
  set(MACOSX_BUNDLE_INCON_FILE OSX/rasmus.icns)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/QSX/rasmus.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif (APPLE)

add_subdirectory(src)
if (RASMUS_TESTS)
   enable_testing()
   add_subdirectory(test)
endif (RASMUS_TESTS)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rasmus interpreter")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Jakob Truelsen <antialize@gmail.com>")
set(CPACK_PACKAGE_NAME rasmus)
set(CPACK_PACKAGE_VERSION 2.0)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.19), llvm-3.4, libqt4-core, libqt4-gui")


if (APPLE)
  set(DIRS ${QT_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/src/stdlib)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/rasmus.app")
  INSTALL(CODE "
    include(BundleUtilities)
    message(WARNING \"${APPS}\")
    fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
    " COMPONENTS Runtime)

   set(CPACK_BINARY_DRAGNDROP ON)
endif (APPLE)

include(CPack)
