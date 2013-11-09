.PHONY: clean all fast a s
all: rasmus/AST.py

clean:
	$(RM) rasmus/AST.py RASMUS

rasmus/AST.py: rasmus/AST.txt rasmus/ASTGen.py
	(cd rasmus && python ASTGen.py)

fast: rasmus/AST.py
	pypy ../pypy/rpython/bin/rpython --no-pdb --no-profile -O 0 --output RASMUS RASMUS.py

a: rasmus/AST.py
	pypy ../pypy/rpython/bin/rpython --view -a -t --no-profile -O 0 -s RASMUS.py


s: rasmus/AST.py
	pypy ../pypy/rpython/bin/rpython --no-pdb --no-profile -O 0 -s RASMUS.py

