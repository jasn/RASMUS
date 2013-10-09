#!/usr/bin/python
import sys
import rasmus.lexer
import rasmus.parser
import rasmus.error

def main():
    if len(sys.argv) != 2:
        exit(1)
    
    f = sys.argv[1]
    
    with open(f) as myFile:
        code = myFile.read()
        e = error.Error(code, f)
        p = parser.Parser(e, code)
        p.parse()

if __name__ == "__main__":
    main()
