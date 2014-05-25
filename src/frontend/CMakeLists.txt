add_custom_command(
  OUTPUT AST.hh visitor.hh nodetype.hh
  COMMAND python2 ${CMAKE_SOURCE_DIR}/tools/ASTGen.py ${CMAKE_CURRENT_SOURCE_DIR}/AST.txt 
  MAIN_DEPENDENCY AST.txt 
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/ASTGen.py)

add_library(frontend
  AST.hh
  visitor.hh
  nodetype.hh
  ASTBase.hh
  charRanges.cc
  firstParse.cc
  code.hh
  error.cc
  error.hh
  lexer.cc
  lexer.hh
  llvmCodeGen.cc
  llvmCodeGen.hh
  parser.cc
  parser.hh)

target_link_libraries(frontend ${LLVM_LFLAGS} ${LLVM_MODULE_LIBS} -lncurses )
