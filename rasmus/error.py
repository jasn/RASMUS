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

def yellow():
    return '\033[33m'

def boldYellow():
    return '\033[33;1m'

def bisect_left(a, x):
    lo=0
    hi=len(a)
    while lo < hi:
        mid = (lo+hi)//2
        if a[mid] < x: lo = mid+1
        else: hi = mid
    return lo

class Error:
    def __init__(self, code):
        self.numberOfErrors = 0
        self.code = code

    def reportWarning(self, 
                    message,
                    mainToken=None,
                    ranges=[]):
        self.report("%swarning%s"%(boldYellow(), reset()), message, mainToken, ranges)

    def reportError(self, 
                    message,
                    mainToken=None,
                    ranges=[]):
        self.report("%serror%s"%(boldRed(), reset()), message, mainToken, ranges)

    def report(self, type, message, mainToken=None, ranges=[]):
        self.numberOfErrors += 1
        lo = 2147483648
        hi = 0
        if mainToken:
            lo = min(lo, mainToken.start)
            hi = max(hi, mainToken.length + mainToken.start)
        for r in ranges:
             lo = min(lo, r.lo)
             hi = max(hi, r.hi)
             
        line = bisect_left(self.code.lineStarts,lo)
        print "%s:%d %s %s"%(self.code.name, line, type, message)
        startOfLine = self.code.lineStarts[line-1]+1
        endOfLine = self.code.lineStarts[line]
        if startOfLine < 0 or endOfLine < startOfLine:
            print "%s%s%s"%(red(),"I am just making rpython happy",reset())
            return
        print "%s%s%s"%(green(),self.code.code[startOfLine:endOfLine],reset())
        if len(ranges) == 0 and mainToken:
            print "%s%s^%s%s"%(" "*(mainToken.start-startOfLine), blue(), "~"*(mainToken.length-1), reset())
        else:
             i = [" "]*(endOfLine - startOfLine)
             for r in ranges:
                 for x in range(max(startOfLine, r.lo), min(endOfLine, r.hi)):
                     i[x-startOfLine] = "~";
             if mainToken:
                 i[max(0, min(mainToken.start + (mainToken.length-1) / 2 - startOfLine, endOfLine-startOfLine-1))] = '^';
             print "%s%s%s"%(blue(), "".join(i), reset())
    
