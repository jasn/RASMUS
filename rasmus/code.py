class Code:
    def __init__(self, code, name):
        self.name = name
        self.setCode(code)
    
    def addCode(self, code):
        self.setCode(self.code + code)
    
    def setCode(self, code):
        self.code = code
        self.lineStarts = [-1] + [ i for i in range(len(code)) if code[i] == '\n'] + [len(code)]
