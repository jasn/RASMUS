#!/usr/bin/python
import sys
import rasmus.lexer
import rasmus.parser
import rasmus.error
import rasmus.jsonPrinter
import rasmus.llvmCodeGen
import rasmus.charRanges

def main():
    if len(sys.argv) != 2:
        exit(1)
    
    f = sys.argv[1]
    
    with open(f) as myFile:
        code = myFile.read()
        e = rasmus.error.Error(code, f)
        p = rasmus.parser.Parser(e, code)
        AST = p.parse()
        rasmus.charRanges.CharRanges().visit(AST)
        rasmus.llvmCodeGen.LLVMCodeGen(code, sys.stdout).generate(AST)
        #rasmus.jsonPrinter.JSONPrinter(code, sys.stdout).visit(AST)
        sys.stdout.write("\n")

if __name__ == "__main__":
    main()
