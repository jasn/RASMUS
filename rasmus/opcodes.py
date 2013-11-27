from struct import Struct

C = -1
def n():
    global C
    C += 1
    return C

OP_NOOP=n()
OP_ADD=n()
OP_MINUS=n()
OP_MUL=n()
OP_DIV=n()
OP_OR=n()
OP_AND=n()
OP_INT_CONST=n()
OP_PRINT=n()
OP_HALT=n()

class CodeStore:
    def __init__(self):
        self.methods = [""]
    
