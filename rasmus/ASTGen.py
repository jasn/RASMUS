import StringIO
nt=file("nodetype.hh","w")
nt.write("#ifndef __nodetype_HH__\n")
nt.write("#define __nodetype_HH__\n")
nt.write("enum class NodeType {\n")

o=file("AST.hh","w")
o.write("#ifndef __AST_HH__\n")
o.write("#define __AST_HH__\n")
o.write("#include \"ASTBase.hh\"\n")

v=file("visitor.hh", "w")
v.write("#ifndef __VISITOR_HH__\n")
v.write("#define __VISITOR_HH__\n")
v.write("#include \"AST.hh\"\n")
v.write("\n")
v.write("template <typename child_t, typename RT>\n")
v.write("class VisitorCRTP {\n")
v.write("public:\n")
v.write("\ttemplate <typename IT>\n")
v.write("\tvoid visitAll(IT a, IT b) {for (IT t=a; t !=b; ++t) visitNode(*t);}\n")
v.write("\n")
v.write("\ttemplate <typename T>\n")
v.write("\tvoid visitAll(const T & v) {visitAll(v.begin(), v.end());}\n")
v.write("\n")
v.write("\tvoid visitAll(std::initializer_list<NodePtr> v) {visitAll(v.begin(), v.end());}\n")
v.write("\n")
v.write("\tRT visitNode(NodePtr node) {\n")
v.write("\t\tswitch(node->nodeType) {\n")

for line in file('AST.txt'):
    line = line.strip().split("|")
    className = line[0]
    args = []
    if len(line) > 1:
        args = line[1].split(",")
    if args and not args[0]: args=[]
    rem = line[2:]
    nt.write("\t%s,\n"%className)
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
        if t == "NodePtr" and d == None: d="std::make_shared<InvalidExp>()"
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

    o.write(" : Node(NodeType::%s)"%className)
    for t,n in targs:
        o.write(", %s(%s)"%(n, n))
        
    for t,n,d in trem:
        if not d: continue
        o.write(", %s(%s)"%(n,d))
        
    o.write(" {}\n")
    o.write("};\n")
    o.write("\n")

    v.write("\t\tcase NodeType::%s:\n"%className)
    v.write("\t\t\treturn static_cast<child_t*>(this)->visit(std::static_pointer_cast<%s>(node));\n"%className)

o.write("#endif //__AST_HH__\n")
nt.write("};\n#endif //__nodetype_hh__\n")

v.write("\t\t}\n");
v.write("\t}\n");
v.write("};\n#endif //__visitor_hh__\n")
