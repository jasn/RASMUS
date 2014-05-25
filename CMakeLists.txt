project("rasmus")
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

find_package(LLVM REQUIRED)
find_package(Readline REQUIRED)


SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11" )

include_directories(src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src ${LLVM_INCLUDE_DIR})
link_directories(${LLVM_LIBRARY_DIR})

add_subdirectory(src)




# include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${LLVM_INCLUDE_DIR} ${Readline_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)

# add_executable(
#   rm
#   AST.hh
#   visitor.hh
#   nodetype.hh
#   src/ASTBase.hh
#   src/charRanges.cc
#   src/firstParse.cc
#   src/code.hh
#   src/error.cc
#   src/error.hh
#   src/lexer.cc
#   src/lexer.hh
#   src/llvmCodeGen.cc
#   src/llvmCodeGen.hh
#   src/main.cc
#   src/parser.cc
#   src/parser.hh)


# target_link_libraries(rm ${LLVM_LFLAGS} ${LLVM_MODULE_LIBS} ${Readline_LIBRARY} -lncurses -pthread)

# add_library(stdlib SHARED stdlib/error.cc stdlib/inner.hh stdlib/lib.h stdlib/print.cc stdlib/text.cc stdlib/relation.cc)

# add_library(test SHARED src/test/unit.hh src/test/llsunit.cc src/test/log.cc src/test/log.hh src/test/logstream.hh)
#message("${LLVM_INCLUDE_DIRS}")