.PHONY: clean all fast a s
LDFLAGS=$(shell llvm-config --ldflags --libs core jit native engine interpreter)
CXXFLAGS=-std=c++11 -ggdb -I /usr/include/llvm-3.4/ -I /usr/include/llvm-c-3.4

all: rasmus/AST.hh stdlib.so rm

rm: rasmus/parser.o rasmus/lexer.o rasmus/error.o rasmus/main.o rasmus/charRanges.o rasmus/firstParse.o rasmus/llvmCodeGen.o
	$(CXX) $^ ${LDFLAGS} ${LDFLAGS} -o $@

clean:
	$(RM) rasmus/AST.py RASMUS stdlib.so rasmus/parser.o rasmus/lexer.o rasmus/error.o rasmus/main.o rm  rasmus/charRanges.o rasmus/firstParse.o rasmus/llvmCodeGen.o

rasmus/AST.hh: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python2 ASTGen.py)


stdlib.so: stdlib/error.cc stdlib/print.cc stdlib/text.cc
	g++ -shared -fPIC $^ -o $@ -std=c++11
