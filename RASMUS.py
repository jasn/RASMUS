#!/usr/bin/python
import rasmus.lexer
import rasmus.parser
import rasmus.error
import rasmus.jsonPrinter
#import rasmus.llvmCodeGen
import rasmus.charRanges
import rasmus.firstParse

def entry_point(argv):
    if len(argv) != 2:
        return 1
    
    f = argv[1]
    myFile = open(f)
    code = myFile.read()
    e = rasmus.error.Error(code, f)
    p = rasmus.parser.Parser(e, code)
    AST = p.parse()
    rasmus.charRanges.CharRanges().visit(AST)
    rasmus.firstParse.FirstParse(e, code).visit(AST)
    rasmus.jsonPrinter.JSONPrinter(code, open("ast.json","w")).visit(AST)

    #rasmus.llvmCodeGen.LLVMCodeGen(code, sys.stdout).generate(AST)
    #sys.stdout.write("\n")
    return 0

def target(*args):
    return entry_point, None

if __name__ == "__main__":
    import sys
    entry_point(sys.argv)

