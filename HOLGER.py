#!/usr/bin/env python

import sys
import rasmus.lexer
import rasmus.parser
import rasmus.error
import rasmus.AST
import rasmus.jsonPrinter
import rasmus.charRanges
import rasmus.firstParse
import rasmus.evaluator
import readline

def main():

    code = ""
    sequenceExpNode = rasmus.AST.SequenceExp()

    e = rasmus.error.Error(code, "Interpreted")
    typeChecker = rasmus.firstParse.FirstParse(e, code)
    outerLus = [{}]
    incomplete = ""

    while True:       
        if incomplete:
            line = raw_input(">>   ")
        else:
            line = raw_input(">>>> ")

        newCode = code + incomplete + line
        e.setCode(newCode)
        p = rasmus.parser.Parser(e, newCode, True, len(code))
        try:
            errorsPrior = e.numberOfErrors
            AST = p.parse()
            if not isinstance(AST, rasmus.AST.InvalidExp):
                AST=AST.exp
                rasmus.charRanges.CharRanges().visit(AST)
                sequenceExpNode.sequence.append(AST)
                typeChecker.setLus(outerLus)
                typeChecker.setCode(newCode)
                typeChecker.visit(AST)
                errorsNow = e.numberOfErrors
                if errorsNow == errorsPrior:
                    outerLus = typeChecker.getLus()
                    code = newCode + ";\n"
                    print rasmus.evaluator.Evaluator().visit(AST)
                else:
                    sequenceExpNode.sequence.pop()
                
            incomplete = ""

            # eval + print
        except rasmus.parser.IncompleteInputException:
            incomplete += line + "\n"    

if __name__ == "__main__":
    main()
