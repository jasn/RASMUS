C = -1
def n():
    global C
    C += 1
    return C

OP_NOOP=n()
OP_ADD=n()
OP_INT_CONST=n()
