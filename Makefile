.PHONY: clean all

all: rasmus/AST.py

clean:
	$(RM) rasmus/AST.py

rasmus/AST.py: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python ASTGen.py)

