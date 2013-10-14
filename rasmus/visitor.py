class Visitor:
    def visit(self, node):
        return node.visit(self)

    def visitAll(self, nodes):
        for node in nodes:
            node.visit(self)

