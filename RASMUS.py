#!/usr/bin/python
import sys
import rasmus.lexer
import rasmus.parser
import rasmus.error
import rasmus.xmlPrinter

def main():
    if len(sys.argv) != 2:
        exit(1)
    
    f = sys.argv[1]
    
    with open(f) as myFile:
        code = myFile.read()
        e = rasmus.error.Error(code, f)
        p = rasmus.parser.Parser(e, code)
        AST = p.parse()
        xmlPrinter = rasmus.xmlPrinter.XmlPrinter(code)
        xmlPrinter.visit(AST)

if __name__ == "__main__":
    main()
