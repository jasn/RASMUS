# Find the native LLVM includes and library
#
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_CFLAGS      - llvm compiler flags
#  LLVM_LFLAGS      - llvm linker flags
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.
#  LLVM_FOUND       - True if llvm found.

find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config DOC "llvm-config executable")

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
  OUTPUT_VARIABLE LLVM_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)


if (LLVM_CONFIG_EXECUTABLE)
  message(STATUS "LLVM llvm-config found at: ${LLVM_CONFIG_EXECUTABLE}, version ${LLVM_VERSION}")
else (LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "Could NOT find LLVM executable")
endif (LLVM_CONFIG_EXECUTABLE)


execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
  OUTPUT_VARIABLE LLVM_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (LLVM_VERSION VERSION_LESS 3.6)
execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core mcjit jit native engine interpreter
  OUTPUT_VARIABLE LLVM_MODULE_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
else()
execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core mcjit native engine interpreter
  OUTPUT_VARIABLE LLVM_MODULE_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
endif()

set(LLVM_CFLAGS "${LLVM_CFLAGS} -fexceptions -std=c++11")
set(LLVM_LFLAGS "${LLVM_LFLAGS} -lpthread -lz -ldl -lcurses")

 