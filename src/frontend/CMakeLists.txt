add_custom_command(
  OUTPUT AST.hh visitor.hh nodetype.hh
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/ASTGen.py ${CMAKE_CURRENT_SOURCE_DIR}/AST.txt 
  MAIN_DEPENDENCY AST.txt 
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/ASTGen.py)

add_library(frontend
  AST.hh
  nodetype.hh
  visitor.hh

  ASTBase.hh
  callback.cc
  callback.hh
  charRanges.cc
  charRanges.hh
  code.hh
  error.cc
  error.hh
  firstParse.cc
  firstParse.hh
  interpreter.cc
  interpreter.hh
  lexer.cc
  lexer.hh
  llvmCodeGen.cc
  llvmCodeGen.hh
  parser.cc
  parser.hh
  astPrinter.hh
  astPrinter.cc
  tokenizer.hh
  tokenizer.cc

  strongType.hh
  strongType.cc
)

if (WIN32)
   set(winllvmlib C:/Users/Jesper/Desktop/llvm-3.5.0.src/build2/lib)
   set(winllvm "-L${winllvmlib} -lpthread -lz")
   target_link_libraries(frontend -L${LLVM_LIBRARY_DIR} ${LLVM_MODULE_LIBS} ${winllvm} -lm -limagehlp stdlib)
   message(${winllvm})
   message("remember to adjust winllvmlib variable")
else()
   target_link_libraries(frontend -L${LLVM_LIBRARY_DIR} ${LLVM_MODULE_LIBS} ${LLVM_LFLAGS} -lm stdlib)   
endif (WIN32)



