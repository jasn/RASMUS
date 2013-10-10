class VariableExp:
	def __init__(self, token):
		self.token=token
		pass
	def visit(self, visitor):
		return visitor.visitVariableExp(self)

class AssignmentExp:
	def __init__(self, nameToken, assignToken, valueExp):
		self.nameToken=nameToken
		self.assignToken=assignToken
		self.valueExp=valueExp
		pass
	def visit(self, visitor):
		return visitor.visitAssignmentExp(self)

class Choice:
	def __init__(self, condition, arrowToken, value):
		self.condition=condition
		self.arrowToken=arrowToken
		self.value=value
		pass
	def visit(self, visitor):
		return visitor.visitChoice(self)

class IfExp:
	def __init__(self, ifToken):
		self.ifToken=ifToken
		self.choices=[]
		self.fiToken=None
		pass
	def visit(self, visitor):
		return visitor.visitIfExp(self)

class ForallExp:
	def __init__(self, typeToken, lparenToken):
		self.typeToken=typeToken
		self.lparenToken=lparenToken
		self.listExps=[]
		self.rparenToken=None
		self.pipeToken=None
		self.colonToken=None
		self.expr=None
		self.names=[]
		pass
	def visit(self, visitor):
		return visitor.visitForallExp(self)

class FuncArg:
	def __init__(self, nameToken, colonToken, typeToken):
		self.nameToken=nameToken
		self.colonToken=colonToken
		self.typeToken=typeToken
		pass
	def visit(self, visitor):
		return visitor.visitFuncArg(self)

class FuncExp:
	def __init__(self, funcToken, lparenToken):
		self.funcToken=funcToken
		self.lparenToken=lparenToken
		self.args=[]
		self.rparenToken=None
		self.arrowToken=None
		self.lparenToken2=None
		self.rparenToken2=None
		self.returnTypeToken=None
		self.body=None
		self.endToken
		pass
	def visit(self, visitor):
		return visitor.visitFuncExp(self)

class TupItem:
	def __init__(self, nameToken, colonToken, exp):
		self.nameToken=nameToken
		self.colonToken=colonToken
		self.exp=exp
		pass
	def visit(self, visitor):
		return visitor.visitTupItem(self)

class TupExp:
	def __init__(self, tupToken, lparenToken):
		self.tupToken=tupToken
		self.lparenToken=lparenToken
		self.rparenToken=None
		self.items=[]
		pass
	def visit(self, visitor):
		return visitor.visitTupExp(self)

class Val:
	def __init__(self, valToken, nameToken, equalToken, exp):
		self.valToken=valToken
		self.nameToken=nameToken
		self.equalToken=equalToken
		self.exp=exp
		pass
	def visit(self, visitor):
		return visitor.visitVal(self)

class BlockExp:
	def __init__(self, blockstartToken):
		self.blockstartToken=blockstartToken
		self.vals=[]
		self.inToken=None
		self.inExp=None
		self.blockendToken=None
		pass
	def visit(self, visitor):
		return visitor.visitBlockExp(self)

class BuiltInExp:
	def __init__(self, nameToken, lparenToken):
		self.nameToken=nameToken
		self.lparenToken=lparenToken
		self.args=[]
		self.rparenToken=None
		pass
	def visit(self, visitor):
		return visitor.visitBuiltInExp(self)

class AtExp:
	def __init__(self, atToken, lparenToken):
		self.atToken=atToken
		self.lparenToken=lparenToken
		self.exp=None
		self.rparenToken=None
		pass
	def visit(self, visitor):
		return visitor.visitAtExp(self)

class ConstantExp:
	def __init__(self, token):
		self.token=token
		pass
	def visit(self, visitor):
		return visitor.visitConstantExp(self)

class UnaryOpExp:
	def __init__(self, token, exp):
		self.token=token
		self.exp=exp
		pass
	def visit(self, visitor):
		return visitor.visitUnaryOpExp(self)

class RelExp:
	def __init__(self, relToken, lparenToken):
		self.relToken=relToken
		self.lparenToken=lparenToken
		self.exp=None
		self.rparenToken=None
		pass
	def visit(self, visitor):
		return visitor.visitRelExp(self)

class LenExp:
	def __init__(self, leftPipeToken, exp, rightPipeToken):
		self.leftPipeToken=leftPipeToken
		self.exp=exp
		self.rightPipeToken=rightPipeToken
		pass
	def visit(self, visitor):
		return visitor.visitLenExp(self)

class FuncInvocationExp:
	def __init__(self, funcExp, lparenToken):
		self.funcExp=funcExp
		self.lparenToken=lparenToken
		self.args=[]
		self.rparenToken=None
		pass
	def visit(self, visitor):
		return visitor.visitFuncInvocationExp(self)

class SubstringExp:
	def __init__(self, stringExp, lparenToken, fromExp, dotdotToken, toExp):
		self.stringExp=stringExp
		self.lparenToken=lparenToken
		self.fromExp=fromExp
		self.dotdotToken=dotdotToken
		self.toExp=toExp
		self.rparenToken=None
		pass
	def visit(self, visitor):
		return visitor.visitSubstringExp(self)

class RenameItem:
	def __init__(self, fromName, arrowToken, toName):
		self.fromName=fromName
		self.arrowToken=arrowToken
		self.toName=toName
		pass
	def visit(self, visitor):
		return visitor.visitRenameItem(self)

class RenameExp:
	def __init__(self, lhs, lbracketToken):
		self.lhs=lhs
		self.lbracketToken=lbracketToken
		self.renames=[]
		self.rbracketToken=None
		pass
	def visit(self, visitor):
		return visitor.visitRenameExp(self)

class DotExp:
	def __init__(self, lhs, token, nameToken):
		self.lhs=lhs
		self.token=token
		self.nameToken=nameToken
		pass
	def visit(self, visitor):
		return visitor.visitDotExp(self)

class ProjectExp:
	def __init__(self, lhs, projectionToken):
		self.lhs=lhs
		self.projectionToken=projectionToken
		self.names=[]
		pass
	def visit(self, visitor):
		return visitor.visitProjectExp(self)

class BinaryOpExp:
	def __init__(self, token, lhs, rhs):
		self.token=token
		self.lhs=lhs
		self.rhs=rhs
		pass
	def visit(self, visitor):
		return visitor.visitBinaryOpExp(self)

class SequenceExp:
	def __init__(self, ):
		self.sequence=[]
		pass
	def visit(self, visitor):
		return visitor.visitSequenceExp(self)

