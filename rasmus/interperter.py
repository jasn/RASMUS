from opcodes import *

class Stack:
    def __init__(self):
        self.stack = []
        
    def push_int(self, val):
        self.stack.append(val)

    def pop_int(self):
        return self.stack.pop()

    
class Interperter:
    def __init__(self, codeStore):
        self.stack = Stack()
        self.method = 0
        self.store = codeStore
        self.pc = 0

    def readUInt8(self):
        b = ord(self.store.methods[self.method][self.pc])
        self.pc += 1
        return b
    
    def readInt32(self):
        v = ord(self.store.methods[self.method][self.pc])
        v += ord(self.store.methods[self.method][self.pc+1]) << 8 
        v += ord(self.store.methods[self.method][self.pc+2]) << 16
        v += ord(self.store.methods[self.method][self.pc+3]) << 24
        self.pc += 4
        return v

    def run(self):
        while self.pc < len(self.store.methods[self.method]):
            code = self.readUInt8()
            if code == OP_NOOP:
                pass
            elif code == OP_INT_CONST:
                self.stack.push_int(self.readInt32())
            elif code == OP_ADD:
                self.stack.push_int(self.stack.pop_int() + self.stack.pop_int())
            elif code == OP_MINUS:
                x = self.stack.pop_int()
                self.stack.push_int(self.stack.pop_int() - x)
            elif code == OP_PRINT:
                print self.stack.pop_int()
            elif code == OP_HALT:
                return
                
                
                
            
