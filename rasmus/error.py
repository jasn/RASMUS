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
                    mainToken=None,
                    *ranges):
        lo = Ellipsis
        hi = 0
        if mainToken != None:
            lo = min(lo, mainToken.start)
            hi = max(hi, mainToken.length + mainToken.start)
        for r in ranges:
            lo = min(lo, r.lo)
            hi = max(hi, r.hi)

        line = bisect.bisect_left(self.lineStarts,lo) 
        print "%s:%d %serror%s %s"%(self.name, line, boldRed(), reset(), message)
        startOfLine = self.lineStarts[line-1]+1
        endOfLine = self.lineStarts[line]
        print "%s%s%s"%(green(),self.code[startOfLine:endOfLine],reset())
        if len(ranges) == 0 and mainToken != None:
            print "%s%s^%s%s"%(" "*(mainToken.start-startOfLine), blue(), "~"*(mainToken.length-1), reset())
        else:
            i = [" "]*(endOfLine - startOfLine)
            for r in ranges:
                for x in range(max(startOfLine, r.lo), min(endOfLine, r.hi)):
                    i[x-startOfLine] = "~";
            if mainToken != None:
                i[max(0, min(mainToken.start + (mainToken.length-1) / 2 - startOfLine, endOfLine-startOfLine-1))] = '^';
            print "%s%s%s"%(blue(), "".join(i), reset())
    
