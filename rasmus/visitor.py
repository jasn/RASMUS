class Visitor:
    def visit(self, node):
        return node.visit(self)

    def visitRenameItem(self, node):
        pass

    def visitChoice(self, node):
        pass

    def visitTupItem(self, node):
        pass

    def visitFuncArg(self, node):
        pass

    def visitVal(self, node):
        pass

    def visitAll(self, nodes):
        for node in nodes:
            node.visit(self)

