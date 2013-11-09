o=file("AST.py","w")
o.write("class Node:\n")
o.write("\tdef __init__(self):\n")
o.write("\t\tself.tainted = False\n")
o.write("\t\tself.charRange = None\n")
o.write("\t\tself.type = None\n")
o.write("\n")

for line in file('AST.txt'):
    line = line.strip().split("|")
    className = line[0]
    args = []
    if len(line) > 1:
        args = line[1].split(",")
    rem = line[2:]
    o.write("class %s(Node):\n"%className)
    o.write("\tdef __init__(self, %s):\n"%(", ".join(args)))
    o.write("\t\tNode.__init__(self)\n")
    for arg in args:
        if not arg: continue
        o.write("\t\tself.%s=%s\n"%(arg,arg))
    for var in rem:
        o.write("\t\tself.%s\n"%var)
    o.write("\tdef visit(self, visitor):\n")
    o.write("\t\treturn visitor.visit%s(self)\n"%className.split("(")[0])
    o.write("\n")
