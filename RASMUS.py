#!/usr/bin/python

import sys
import Lexer

def main():
    if len(sys.argv) != 2:
        exit(1)
    
    f = sys.argv[1]
    
    with open(f) as myFile:
        code = myFile.read()
        lexer = Lexer.Lexer(code)
        while True:
            TK_TYPE, start, length = lexer.getNext()
            if TK_TYPE == Lexer.TK_EOF:
                break
            print TK_TYPE

if __name__ == "__main__":
    main()
