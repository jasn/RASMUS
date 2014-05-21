.PHONY: clean all fast run
LDFLAGS=$(shell llvm-config --ldflags --libs core jit native engine interpreter)
CXXFLAGS=-std=c++11 -ggdb -I /usr/include/llvm-3.4/ -I /usr/include/llvm-c-3.4

OBJECTS=rasmus/parser.o rasmus/lexer.o rasmus/error.o rasmus/main.o rasmus/charRanges.o rasmus/firstParse.o rasmus/llvmCodeGen.o
GEN=rasmus/AST.hh rasmus/visitor.hh rasmus/nodetype.hh

all: rasmus/AST.hh stdlib.so rm

rasmus/error.o: rasmus/error.cc rasmus/error.hh rasmus/code.hh rasmus/lexer.hh rasmus/ASTBase.hh rasmus/nodetype.hh

rm: ${GEN} ${OBJECTS}
	$(CXX) ${OBJECTS} ${LDFLAGS} ${LDFLAGS} -o $@

run: rm stdlib.so
	./rm

clean:
	$(RM) ${GEN} rm stdlib.so ${OBJECTS}

${GEN}: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python2 ASTGen.py)

stdlib.so: stdlib/error.cc stdlib/print.cc stdlib/text.cc
	g++ -shared -fPIC $^ -o $@ -std=c++11
