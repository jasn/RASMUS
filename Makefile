.PHONY: clean all fast a s
all: rasmus/AST.py stdlib.so

clean:
	$(RM) rasmus/AST.py RASMUS

rasmus/AST.py: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python2 ASTGen.py)

stdlib.so: stdlib/error.cc stdlib/print.cc
	g++ -shared -fPIC $^ -o $@
