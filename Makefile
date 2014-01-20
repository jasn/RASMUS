.PHONY: clean all fast a s
all: rasmus/AST.py stdlib.so

clean:
	$(RM) rasmus/AST.py RASMUS

rasmus/AST.py: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python ASTGen.py)

stdlib.so: stdlib/error.cc
	g++ -shared -fPIC stdlib/error.cc -o stdlib.so
