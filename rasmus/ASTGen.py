import StringIO
o=file("AST.hh","w")
o.write("#ifndef __AST_HH__\n")
o.write("#define __AST_HH__\n")
o.write("#include \"ASTBase.hh\"\n")

v = StringIO.StringIO()

v.write("class Visitor {\n")
v.write("public:\n")
v.write("\tvirtual ~Visitor() {};\n")
v.write("\tvoid visit(NodePtr n) {n->visit(this, n);}\n")
v.write("\n")
v.write("\ttemplate <typename IT>\n")
v.write("\tvoid visitAll(IT a, IT b) {for (IT t=a; t !=b; ++t) (*t)->visit(this, *t);}\n")
v.write("\n")
v.write("\ttemplate <typename T>\n")
v.write("\tvoid visitAll(const T & v) {visitAll(v.begin(), v.end());}\n")
v.write("\n")
v.write("\tvoid visitAll(std::initializer_list<NodePtr> v) {visitAll(v.begin(), v.end());}\n")

v2 = StringIO.StringIO()

for line in file('AST.txt'):
    line = line.strip().split("|")
    className = line[0]
    args = []
    if len(line) > 1:
        args = line[1].split(",")
    if args and not args[0]: args=[]
    rem = line[2:]
    o.write("class %s: public Node {\n"%className)
    o.write("public:\n")
    targs = []
    for arg in args:
        if arg.endswith("Token"):
            targs.append(("Token", arg))
        else:
            targs.append(("NodePtr", arg))

    trem=[]
    for r in rem:
        d=None
        t=None
        r = r.split("=", 1)
        if len(r) == 2: d=r[1]
        r=r[0]
        r = r.rsplit(" ", 1)
        if len(r) == 2: t,r=r
        else: r=r[0]
        if t == "NodeList": t="std::vector<NodePtr>"
        if t == "TokenList": t="std::vector<Token>"
        if not t:
            if r.endswith("Token"): t="Token"
            else: t="NodePtr"
        #if t == "NodePtr" and d==None: //TODO invalidExp
        trem.append((t,r,d))

    for t,n in targs:
        o.write("\t%s %s;\n"%(t,n))

    for t,n,d in trem:
        o.write("\t%s %s;\n"%(t,n))
        
    o.write("\t%s("%className)
    first=True
    for t,n in targs:
        if first: first=False
        else: o.write(", ")
        o.write("%s %s"%(t,n))
    o.write(")")
    if not first:
        o.write(" : ")
        first=True
        for t,n in targs:
            if first: first=False
            else: o.write(", ")
            o.write("%s(%s)"%(n, n))
    for t,n,d in trem:
        if not d: continue
        o.write("%s(%s);\n"%(n,d))
    o.write(" {}\n")
    # o.write("\t\tNode.__init__(self)\n")
    # for arg in args:
    #     if not arg: continue
    #     o.write("\t\tself.%s=%s\n"%(arg,arg))
    # for var in rem:
    #     o.write("\t\tself.%s\n"%var)
    
    v.write("\tvirtual void visit(std::shared_ptr<%s> node) = 0;\n"%className)
    o.write("\tinline void visit(Visitor * v, NodePtr self) override;\n")
    v2.write("void %s::visit(Visitor * v, NodePtr self) {v->visit(std::static_pointer_cast<%s>(self));}\n"%(className,className));
        
    o.write("};\n")
    o.write("\n")
v.write("};\n")
o.write(v.getvalue())
o.write(v2.getvalue())

o.write("#endif //__AST_HH__\n")
