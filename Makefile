.PHONY: clean all fast a s
all: rasmus/AST.py

clean:
	$(RM) rasmus/AST.py RASMUS

rasmus/AST.py: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python ASTGen.py)
