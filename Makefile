.PHONY: clean all fast a s
CXXFLAGS=-std=c++11 -ggdb

all: rasmus/AST.hh stdlib.so rm

rm: rasmus/parser.o rasmus/lexer.o rasmus/error.o rasmus/main.o rasmus/charRanges.o
	$(CXX) $^ -o $@

clean:
	$(RM) rasmus/AST.py RASMUS stdlib.so rasmus/parser.o rasmus/lexer.o rasmus/error.o rasmus/main.o rm

rasmus/AST.hh: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python2 ASTGen.py)


stdlib.so: stdlib/error.cc stdlib/print.cc stdlib/text.cc
	g++ -shared -fPIC $^ -o $@ -std=c++11
