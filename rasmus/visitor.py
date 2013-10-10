class Visitor:
    def visit(self, node):
        node.visit(self)

    def visitAll(self, nodes):
        for node in nodes:
            node.visit(self)

