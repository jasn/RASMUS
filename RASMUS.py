#!/usr/bin/python
import rasmus.lexer
import rasmus.parser
import rasmus.error
import rasmus.jsonPrinter
import rasmus.llvmCodeGen
import rasmus.charRanges
import rasmus.firstParse
import rasmus.code
import rasmus.AST
import os
#import readline
from llvm.ee import ExecutionEngine, TargetData, GenericValue
from llvm.core import Type
import ctypes



def run_file(path):
    myFile = open(path)
    code = rasmus.code.Code(myFile.read(), "hat")
    e = rasmus.error.Error(code)
    p = rasmus.parser.Parser(e, code)
    AST = p.parse()
    rasmus.charRanges.CharRanges().visit(AST)
    rasmus.firstParse.FirstParse(e, code).visit(AST)
    return 0

def hat(hat):
    return True

def run_terminal():
    code = rasmus.code.Code("", "Interpreted")
    e = rasmus.error.Error(code)
    typeChecker = rasmus.firstParse.FirstParse(e, code)
    #outerLus = [{}]
    theCode = ""
    incomplete = ""
    codegen = rasmus.llvmCodeGen.LLVMCodeGen(e, code)
    llvm_executor = ExecutionEngine.new(codegen.module)

    dll = ctypes.cdll.LoadLibrary( os.getcwd()+"/stdlib.so")
    
    INTP = ctypes.POINTER(ctypes.c_size_t)
    ptr = ctypes.cast(ctypes.addressof(dll.emit_type_error), INTP)[0]
    llvm_executor.add_global_mapping(codegen.typeErr, ptr)
    ptr = ctypes.cast(ctypes.addressof(dll.emit_arg_cnt_error), INTP)[0]
    llvm_executor.add_global_mapping(codegen.argCntErr, ptr)

    ptr = ctypes.cast(ctypes.addressof(dll.doPrint), INTP)[0]
    llvm_executor.add_global_mapping(codegen.doPrint, ptr)

    ptr = ctypes.cast(ctypes.addressof(dll.interactiveWrapper), INTP)[0]
    llvm_executor.add_global_mapping(codegen.interactiveWrapper, ptr)
    
    sequenceExpNode = rasmus.AST.SequenceExp()    
    while True:      
        try:
            if incomplete:
                line = raw_input(".... ")
            else:
                line = raw_input(">>>> ")
        except EOFError:
            print
            break
        newCode = theCode + incomplete + line
        code.setCode(newCode)
        p = rasmus.parser.Parser(e, code, True, len(theCode))
        try:
            errorsPrior = e.numberOfErrors
            AST = p.parse()
            if not isinstance(AST, rasmus.AST.InvalidExp):
                AST=AST.exp
                
                nameToken = rasmus.lexer.Token(rasmus.lexer.TK_PRINT, 0, 0)
                printStmt = rasmus.AST.BuiltInExp(nameToken, None)
                printStmt.args.append(AST)

                AST = printStmt
                rasmus.charRanges.CharRanges().visit(AST)
                sequenceExpNode.sequence.append(AST)
                #typeChecker.setLus(outerLus)
                typeChecker.visit(AST)
                #rasmus.jsonPrinter.JSONPrinter(code, sys.stdout).visit(AST)
                errorsNow = e.numberOfErrors
                if errorsNow == errorsPrior:
                    #outerLus = typeChecker.getLus()
                    theCode = newCode + ";\n"
                    codegen.visitOuter(AST)

                    codeStr = ctypes.c_char_p(newCode)
                    ptr = ctypes.cast(ctypes.addressof(codeStr), INTP)[0]
                    llvm_executor.run_function(codegen.function, [GenericValue.pointer(ptr)])
                else:
                    sequenceExpNode.sequence.pop()
                incomplete = ""
        except rasmus.parser.IncompleteInputException:
            incomplete += line + "\n"    
    return 0

def entry_point(argv):
    if len(argv) == 1:
        return run_terminal()

    if len(argv) == 2:
        return run_file(argv[1])

    return 1

def target(*args):
    return entry_point, None

if __name__ == "__main__":
    import sys
    entry_point(sys.argv)

