import os, bisect

def boldRed():
    return '\033[31;1m'

def red():
    return '\033[31m'

def reset():
    return '\x1b[0m'

def green():
    return '\033[32m'

def blue():
    return '\033[34m'

class Error:
    def __init__(self, code, name):
        self.name = os.path.basename(name)
        self.code = code
        self.lineStarts = [-1] + [ i for i in range(len(code)) if code[i] == '\n'] + [len(code)]


    def reportError(self, 
                    message,
                    mainToken,
                    r1=None,
                    r2=None): 
        line = bisect.bisect_left(self.lineStarts,mainToken.start) 
        print "%s:%d %serror%s %s"%(self.name, line, boldRed(), reset(), message)
        startOfLine = self.lineStarts[line-1]+1
        endOfLine = self.lineStarts[line]
        print "%s%s%s"%(green(),self.code[startOfLine:endOfLine],reset())
        if r1 == None:
            print "%s%s^%s%s"%(" "*(mainToken.start-startOfLine), blue(), "~"*(mainToken.length-1), reset())
        else:
            i = [" "]*(endOfLine - startOfLine)
            for x in range(max(startOfLine, r1.lo), min(endOfLine, r1.hi)):
                i[x-startOfLine] = "~";
            i[mainToken.start + mainToken.length / 2] = '^';
            print "%s%s%s"%(blue(), "".join(i), reset())
                    
        
    
