// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The pyRASMUS development team
// 
// This file is part of pyRASMUS.
// 
// pyRASMUS is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// pyRASMUS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with pyRASMUS.  If not, see <http://www.gnu.org/licenses/>
#include "parser.hh"
#include <unordered_set>
#include <vector>
#include <iostream>
#include <cassert>

using lexer::TokenType;

namespace {
using namespace rasmus::frontend;

std::vector<const char *> at_names;

class RecoverException: public std::exception {
public:
	const TokenType id;
	RecoverException(TokenType id): id(id) {}
};


const std::unordered_set<TokenType> thingsThatMayComeAfterParseExp =
{TokenType::TK_RIGHTARROW, TokenType::TK_RPAREN, TokenType::TK_COMMA, TokenType::TK_FI,
 TokenType::TK_PIPE, TokenType::TK_COLON, TokenType::TK_END, TokenType::TK_IN, TokenType::TK_CHOICE,
 TokenType::TK_VAL, TokenType::TK_TWO_DOTS, TokenType::END_OF_FILE, TokenType::TK_SEMICOLON, TokenType::TK_BLOCKEND};

const std::unordered_set<TokenType> types=
{TokenType::TK_TYPE_BOOL,TokenType::TK_TYPE_INT, TokenType::TK_TYPE_TEXT,TokenType::TK_TYPE_ATOM, TokenType::TK_TYPE_TUP,TokenType::TK_TYPE_REL, TokenType::TK_TYPE_FUNC,TokenType::TK_TYPE_ANY};
	
class TheParser: public Parser {
public:
	std::shared_ptr<Lexer> lexer;
	std::shared_ptr<Error> error;
	bool interactiveMode;
	
	Token currentToken;

	std::vector<TokenType> recoverStack;
	
	TheParser(std::shared_ptr<Lexer> lexer, 
			  std::shared_ptr<Error> error,
			  bool interactiveMode): lexer(lexer), error(error), interactiveMode(interactiveMode) {
	}
	
	Token consumeToken() {
		Token token = currentToken;
		currentToken = lexer->getNext();
		return token;
	}

	template <typename T>
	inline void recover(TokenType id, T t) {
		recoverStack.push_back(id);
		try {
			t();
			assert(recoverStack.back() == id);
			recoverStack.pop_back();
		} catch(RecoverException e) {
			assert(recoverStack.back() == id);
			recoverStack.pop_back();
			if (e.id != id) throw;
		} catch(...) {
			assert(recoverStack.back() == id);
			recoverStack.pop_back();
			throw;
		}
	}

	
	void doRecover() __attribute__ ((noreturn)) {
		//Recover errors at the first token in list of tokens specified on the recovery stack
		std::unordered_set<TokenType> r(recoverStack.begin(), recoverStack.end());
		if (r.count(currentToken.id)) {
			throw RecoverException(currentToken.id);
		}
		consumeToken();
		while (r.count(currentToken.id) == 0) {
			if (currentToken.id == TokenType::END_OF_FILE) {
				throw RecoverException(TokenType::END_OF_FILE);
			}
			else if (currentToken.id == TokenType::INVALID)
				parseError("Invalid token");
			consumeToken();
		}
		throw RecoverException(currentToken.id);
	}

	void parseError(std::string err) {
		error->reportError(std::string("Parse error: ")+err, currentToken);
	}

    void unexpectedToken() __attribute__ ((noreturn)) {
        if (interactiveMode && currentToken.id == TokenType::END_OF_FILE)
            throw IncompleteInputException();
        parseError("Unexpected token");
        doRecover();
	}

    void assertToken(TokenType id) {
        if (interactiveMode &&
            currentToken.id == TokenType::END_OF_FILE &&
            id != TokenType::END_OF_FILE)
            throw IncompleteInputException();
        
        if (id != currentToken.id) {
            parseError(std::string("Expected ")+getTokenName(id)+" at");
			doRecover();
		}
	}

    Token assertTokenConsume(TokenType id) {
        Token token=currentToken;
        assertToken(id);
        consumeToken();
        return token;
	}


	Token parseType() {
		if (types.count(currentToken.id))
			return consumeToken();
		parseError("Expected Type");
		doRecover();
	}
	
	NodePtr parseAssignment() {
		Token nameToken = consumeToken();
		if (currentToken.id == TokenType::TK_ASSIGN) {
			Token t=consumeToken();
			return std::make_shared<AssignmentExp>(nameToken, t, parseOr());
		}
		return std::make_shared<VariableExp>(nameToken);
	}
	
	NodePtr parseIfExp() {
		std::shared_ptr<IfExp> n = std::make_shared<IfExp>(assertTokenConsume(TokenType::TK_IF));
		recover(TokenType::TK_IF, [&]() {
				recover(TokenType::TK_CHOICE, [&]() {
						NodePtr e=parseExp();
						Token t=assertTokenConsume(TokenType::TK_RIGHTARROW);
						n->choices.push_back(std::make_shared<Choice>(e, t,  parseExp()));
						while (currentToken.id == TokenType::TK_CHOICE) {
							consumeToken();
							recover(TokenType::TK_CHOICE, [&]() {
									NodePtr e=parseExp();
									Token t=assertTokenConsume(TokenType::TK_RIGHTARROW);
									n->choices.push_back(std::make_shared<Choice>(e, t,  parseExp()));
								});
						}
						assertToken(TokenType::TK_FI);
					});
			});
		n->fiToken = assertTokenConsume(TokenType::TK_FI);
		return n;
	}
  
	NodePtr parseForallExp() {
		Token t1=consumeToken();
		std::shared_ptr<ForallExp> n = std::make_shared<ForallExp>(t1, assertTokenConsume(TokenType::TK_LPAREN));
		recover(TokenType::TK_COMMA, [&]() {
				n->listExps.push_back(parseExp());
				while (currentToken.id == TokenType::TK_COMMA) {
					consumeToken();
					n->listExps.push_back(parseExp());
				}
				assertToken(TokenType::TK_RPAREN);
			});
		n->rparenToken = assertTokenConsume(TokenType::TK_RPAREN);
		if (currentToken.id == TokenType::TK_PIPE) {
			n->pipeToken = consumeToken();
			n->names.push_back(assertTokenConsume(TokenType::TK_NAME));
			while (currentToken.id == TokenType::TK_COMMA) {
				consumeToken();
				n->names.push_back(assertTokenConsume(TokenType::TK_NAME));
			}
		}
		n->colonToken = assertTokenConsume(TokenType::TK_COLON);
		n->exp = parseExp();



		std::shared_ptr<FuncExp> f = std::make_shared<FuncExp>(
			Token(TokenType::TK_FUNC, "func"),
			Token(TokenType::TK_LPAREN, "("));
			
		f->args.push_back(std::make_shared<FuncArg>(
							  Token(TokenType::TK_NAME, "#"),
							  Token(TokenType::TK_COLON, ":"),
							  Token(TokenType::TK_TYPE_TUP, "Tup")));
		for(size_t i = 0; i < n->listExps.size(); i++){

			if(at_names.size() <= i){
				char * name = new char[24];
				sprintf(name, "@(%zd)", i+1);
				at_names.push_back(name);
			}
			f->args.push_back(std::make_shared<FuncArg>(
								  Token(TokenType::TK_NAME, at_names[i]),
								  Token(TokenType::TK_COLON, ":"),
								  Token(TokenType::TK_TYPE_REL, "Rel")));		 
		}
		f->rparenToken = Token(TokenType::TK_RPAREN, ")");
		f->arrowToken = Token(TokenType::TK_RIGHTARROW, "->");
		f->lparen2Token = Token(TokenType::TK_LPAREN, "(");
		f->returnTypeToken = Token(TokenType::TK_TYPE_REL, "Rel");
		f->rparen2Token = Token(TokenType::TK_RPAREN, ")");
		f->body = n->exp;
		f->endToken = Token(TokenType::TK_END, "end");
		
		n->exp = f;
	
		return n;
	}

	NodePtr parseBuiltIn() {
		Token t1=consumeToken();
		std::shared_ptr<BuiltInExp> n = std::make_shared<BuiltInExp>(t1, assertTokenConsume(TokenType::TK_LPAREN));
		recover(TokenType::TK_RPAREN, [&]() {
				n->args.push_back(parseExp());
				while (currentToken.id == TokenType::TK_COMMA) {
					consumeToken();
					n->args.push_back(parseExp());
				}
				assertToken(TokenType::TK_RPAREN);
			});
		n->rparenToken = assertTokenConsume(TokenType::TK_RPAREN);
		return n;
	}
    
    NodePtr parseFuncExp() {
		Token t1=consumeToken();
		std::shared_ptr<FuncExp> n = std::make_shared<FuncExp>(t1,assertTokenConsume(TokenType::TK_LPAREN));
		recover(TokenType::TK_RPAREN, [&]() {
				if (currentToken.id != TokenType::TK_RPAREN) {
					Token t1=assertTokenConsume(TokenType::TK_NAME), t2=assertTokenConsume(TokenType::TK_COLON);
					n->args.push_back(std::make_shared<FuncArg>(t1, t2, parseType()));
					while (currentToken.id == TokenType::TK_COMMA) {
						consumeToken();
						Token t1=assertTokenConsume(TokenType::TK_NAME), t2=assertTokenConsume(TokenType::TK_COLON);
						n->args.push_back(std::make_shared<FuncArg>(t1, t2, parseType()));
					}
				}
				assertToken(TokenType::TK_RPAREN);
			});
        n->rparenToken = consumeToken();
		n->arrowToken = assertTokenConsume(TokenType::TK_RIGHTARROW);
		n->lparen2Token = assertTokenConsume(TokenType::TK_LPAREN);
		n->returnTypeToken = parseType();
		n->rparen2Token = assertTokenConsume(TokenType::TK_RPAREN);
		recover(TokenType::TK_END, [&]() {
				n->body = parseExp();
				assertToken(TokenType::TK_END);
			});
        n->endToken = assertTokenConsume(TokenType::TK_END);
		return n;
	}

    NodePtr parseTupExp() {
		Token t1=consumeToken();
		std::shared_ptr<TupExp> n = std::make_shared<TupExp>(t1, assertTokenConsume(TokenType::TK_LPAREN));
		recover(TokenType::TK_RPAREN, [&]() {
				if (currentToken.id != TokenType::TK_RPAREN) {
					Token t1=assertTokenConsume(TokenType::TK_NAME), t2=assertTokenConsume(TokenType::TK_COLON);
					n->items.push_back(std::make_shared<TupItem>(t1, t2, parseExp()));
					while (currentToken.id == TokenType::TK_COMMA) {
						consumeToken();
						Token t1=assertTokenConsume(TokenType::TK_NAME), t2=assertTokenConsume(TokenType::TK_COLON);
						n->items.push_back(std::make_shared<TupItem>(t1, t2, parseExp()));
					}
				}
				assertToken(TokenType::TK_RPAREN);
			});
		n->rparenToken = consumeToken();
        return n;
	}

    NodePtr parseBlockExp() {
		std::shared_ptr<BlockExp> n = std::make_shared<BlockExp>(consumeToken());
		recover(TokenType::TK_BLOCKEND, [&]() {
				recover(TokenType::TK_IN, [&]() {
						while (currentToken.id == TokenType::TK_VAL) {
							Token valToken = consumeToken();
							recover(TokenType::TK_VAL, [&]() {
									Token t1=assertTokenConsume(TokenType::TK_NAME);
									Token t2=assertTokenConsume(TokenType::TK_EQUAL);
									n->vals.push_back(std::make_shared<Val>(
														  valToken,
														  t1, t2,
														  parseExp()));
								});
						}
						assertToken(TokenType::TK_IN);
					});
				n->inToken = consumeToken();
				n->inExp = parseExp();
				assertToken(TokenType::TK_BLOCKEND);
			});
        n->blockendToken = consumeToken();
		return n;
	}

    NodePtr parseParenthesisExp() {
		NodePtr n = std::make_shared<InvalidExp>();
        consumeToken();
		recover(TokenType::TK_RPAREN, [&]() {
				n = parseExp();
				assertToken(TokenType::TK_RPAREN);
			});
        assertTokenConsume(TokenType::TK_RPAREN);
        return n;
	}

    NodePtr parseRelExp() {
		Token t=consumeToken();
		std::shared_ptr<RelExp> n = std::make_shared<RelExp>(t, assertTokenConsume(TokenType::TK_LPAREN));
		recover(TokenType::TK_RPAREN, [&]() {
				n->exp = parseExp();
				assertToken(TokenType::TK_RPAREN);
			});
        n->rparenToken = consumeToken();
        return n;
	}

    NodePtr parseAtExp() {
		Token t=consumeToken();
		std::shared_ptr<VariableExp> n = std::make_shared<VariableExp>(t);
        return n;
	}

	NodePtr parseUnset() {		
		Token t = consumeToken();
		assertToken(TokenType::TK_NAME);
		Token t2 = consumeToken();
		return std::make_shared<UnsetExp>(t, t2);
	}

    NodePtr parseBottomExp() {
		switch (currentToken.id) {
		case TokenType::TK_UNSET:
			return parseUnset();
		case TokenType::TK_IF:
			return parseIfExp();
		case TokenType::TK_LPAREN:
            return parseParenthesisExp();
		case TokenType::TK_TODAY:
		case TokenType::TK_CLOSE: {
			Token t=consumeToken();
            return std::make_shared<BuiltInExp>(t, Token());
		} case TokenType::TK_BANG:
		case TokenType::TK_BANGLT:
		case TokenType::TK_BANGGT:
            return parseForallExp();
		case TokenType::TK_ISATOM:
		case TokenType::TK_ISTUP:
		case TokenType::TK_ISREL:
		case TokenType::TK_ISFUNC:
		case TokenType::TK_ISANY:
		case TokenType::TK_SYSTEM:
		case TokenType::TK_WRITE:
		case TokenType::TK_OPEN:
		case TokenType::TK_HAS:
		case TokenType::TK_MAX:
		case TokenType::TK_MIN:
		case TokenType::TK_COUNT: 
		case TokenType::TK_ADD:
		case TokenType::TK_MULT:
		case TokenType::TK_DAYS:
		case TokenType::TK_BEFORE:
		case TokenType::TK_AFTER:
		case TokenType::TK_DATE:
		case TokenType::TK_ISBOOL:
		case TokenType::TK_ISINT:
		case TokenType::TK_ISTEXT:
		case TokenType::TK_PRINT:
            return parseBuiltIn();
		case TokenType::TK_PIPE: {
			Token t=consumeToken();
			NodePtr p=parseExp();
            return std::make_shared<LenExp>(t, p, assertTokenConsume(TokenType::TK_PIPE));
		}
		case TokenType::TK_MINUS:
		{
			Token t=consumeToken();
            return std::make_shared<UnaryOpExp>(t, parseExp());
		}
		case TokenType::TK_NAME:
            return parseAssignment();
		case TokenType::TK_FUNC:
            return parseFuncExp();
		case TokenType::TK_REL:
            return parseRelExp();
		case TokenType::TK_TUP:
            return parseTupExp();
		case TokenType::TK_ZERO:
		case TokenType::TK_ONE:
		case TokenType::TK_STDBOOL:
		case TokenType::TK_STDINT:
		case TokenType::TK_STDTEXT:
		case TokenType::TK_TEXT:
		case TokenType::TK_BADINT:
		case TokenType::TK_INT:
		case TokenType::TK_TRUE:
		case TokenType::TK_FALSE:
            return std::make_shared<ConstantExp>(consumeToken());
		case TokenType::TK_SHARP:
            return std::make_shared<VariableExp>(consumeToken());
		case TokenType::TK_BLOCKSTART:
            return parseBlockExp();
		case TokenType::TK_AT:
            return parseAtExp();
		case TokenType::TK_NOT: {
			Token t=consumeToken();
            return std::make_shared<UnaryOpExp>(t, parseExp());
		}
		default:
            unexpectedToken();
		}
	}

    NodePtr parseSubstringOrFuncInvocationExp() {
        NodePtr n = parseBottomExp();
		while (currentToken.id == TokenType::TK_LPAREN) {
            Token lparenToken = consumeToken();
			if (currentToken.id == TokenType::TK_RPAREN) {
				std::shared_ptr<FuncInvocationExp> f = std::make_shared<FuncInvocationExp>(n, lparenToken);
				f->rparenToken = assertTokenConsume(TokenType::TK_RPAREN);				
				n=f;
			} else {
				recover(TokenType::TK_RPAREN, [&]() {
						NodePtr e1 = parseExp();
						if (currentToken.id == TokenType::TK_TWO_DOTS) {
							Token t=consumeToken();
							NodePtr e2=parseExp();
							n = std::make_shared<SubstringExp>(n, lparenToken, e1, t, e2, assertTokenConsume(TokenType::TK_RPAREN));
						} else {
							std::shared_ptr<FuncInvocationExp> f = std::make_shared<FuncInvocationExp>(n, lparenToken);
							f->args.push_back(e1);
							while (currentToken.id == TokenType::TK_COMMA) {
								consumeToken();
								f->args.push_back(parseExp());
							}
							assertToken(TokenType::TK_RPAREN);
							f->rparenToken = assertTokenConsume(TokenType::TK_RPAREN);
							n=f;
						}
					});
				//TODO fix error recovery on RPAREN token, for TokenType::TK_TWO_DOTS this did not work under python either
			}
		}
		return n;
	}

    NodePtr parseOpExtendAndOverwriteExp() {
        NodePtr n = parseSubstringOrFuncInvocationExp();
		while (true) {
			switch(currentToken.id) {
			case TokenType::TK_OPEXTEND: {
				Token t=consumeToken();
				n = std::make_shared<BinaryOpExp>(t, n, parseSubstringOrFuncInvocationExp());
				break;
			}
			case TokenType::TK_SET_MINUS: { 
				Token t=consumeToken();
				n = std::make_shared<TupMinus>(n, t, assertTokenConsume(TokenType::TK_NAME));				
				break;
			}
			default:
				return n;
			}
		}
        return n;
	}
    
	NodePtr parseDotExp() {
        NodePtr n = parseOpExtendAndOverwriteExp();
		if (currentToken.id == TokenType::TK_ONE_DOT) {
			Token t=consumeToken();
            n = std::make_shared<DotExp>(n, t, assertTokenConsume(TokenType::TK_NAME));
		}
        return n;
	}

    NodePtr parseRenameAndProjectExp() {
        NodePtr n=parseDotExp();
		while (true) {
			switch(currentToken.id) {
			case TokenType::TK_LBRACKET: {
				std::shared_ptr<RenameExp> e=std::make_shared<RenameExp>(n, consumeToken());
				recover(TokenType::TK_RBRACKET, [&]() {
						Token t1=assertTokenConsume(TokenType::TK_NAME), t2=assertTokenConsume(TokenType::TK_LEFT_ARROW);
						e->renames.push_back(std::make_shared<RenameItem>(t1, t2, assertTokenConsume(TokenType::TK_NAME)));
						
						while (currentToken.id == TokenType::TK_COMMA) {
							consumeToken();
							Token t1=assertTokenConsume(TokenType::TK_NAME), t2=assertTokenConsume(TokenType::TK_LEFT_ARROW);
							e->renames.push_back(std::make_shared<RenameItem>(t1, t2, assertTokenConsume(TokenType::TK_NAME)));
						}
						assertToken(TokenType::TK_RBRACKET);
					});
				e->rbracketToken = assertTokenConsume(TokenType::TK_RBRACKET);
				n = e;
				break;
			}
			case TokenType::TK_PROJECT_PLUS:
			case TokenType::TK_PROJECT_MINUS: {
				std::shared_ptr<ProjectExp> e = std::make_shared<ProjectExp>(n, consumeToken());
				e->names.push_back(assertTokenConsume(TokenType::TK_NAME));
				while (currentToken.id == TokenType::TK_COMMA) {
					consumeToken();
					e->names.push_back(assertTokenConsume(TokenType::TK_NAME));
				}
				n=e;
				break;
			}
			case TokenType::TK_SELECT: {
				Token t=consumeToken();			
				
				recover(TokenType::TK_RPAREN, [&]() {
						std::shared_ptr<FuncExp> f = std::make_shared<FuncExp>(
							Token(TokenType::TK_FUNC, "func"),
							Token(TokenType::TK_LPAREN, "("));
						
						f->args.push_back(std::make_shared<FuncArg>(
											  Token(TokenType::TK_NAME, "#"),
											  Token(TokenType::TK_COLON, ":"),
											  Token(TokenType::TK_TYPE_TUP, "Tup")));
						f->rparenToken = Token(TokenType::TK_RPAREN, ")");
						f->arrowToken = Token(TokenType::TK_RIGHTARROW, "->");
						f->lparen2Token = Token(TokenType::TK_LPAREN, "(");
						f->returnTypeToken = Token(TokenType::TK_TYPE_BOOL, "Bool");
						f->rparen2Token = Token(TokenType::TK_RPAREN, ")");
						f->body = parseExp();
						f->endToken = Token(TokenType::TK_END, "end");
						assertTokenConsume(TokenType::TK_RPAREN);
						n = std::make_shared<BinaryOpExp>(t, n, f);
					});
				break;
			}
			default:
				return n;
			}
		}
        return n;
	}

    NodePtr parseMulDivModExp() {
		NodePtr n = parseRenameAndProjectExp();
        while (currentToken.id == TokenType::TK_DIV || currentToken.id == TokenType::TK_MUL ||
			   currentToken.id == TokenType::TK_MOD) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseRenameAndProjectExp());
		}
		return n;
	}

    NodePtr parsePlusMinusExp() {
        NodePtr n = parseMulDivModExp();
		while (currentToken.id == TokenType::TK_PLUS || currentToken.id == TokenType::TK_MINUS ||
			   /*currentToken.id == TokenType::TK_SET_MINUS ||*/
			   currentToken.id == TokenType::TK_CONCAT) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseMulDivModExp());
		}
        return n;
	}

    NodePtr parseCompareExp() {
        NodePtr n = parsePlusMinusExp();
        if (currentToken.id == TokenType::TK_EQUAL || currentToken.id == TokenType::TK_DIFFERENT ||
			currentToken.id == TokenType::TK_LESS || currentToken.id == TokenType::TK_GREATER ||
			currentToken.id == TokenType::TK_TILDE || currentToken.id == TokenType::TK_GREATEREQUAL ||
			currentToken.id == TokenType::TK_LESSEQUAL) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n,  parsePlusMinusExp());
		}
        return n;
	}
	
	NodePtr parseAnd() {
        NodePtr n = parseCompareExp();
        while (currentToken.id == TokenType::TK_AND) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseCompareExp());
		}
		return n;
	}

	NodePtr parseOr() {
        NodePtr n = parseAnd();
        while (currentToken.id == TokenType::TK_OR) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseOr());
		}
		return n;
	}

    NodePtr parseSequenceExp() {
        NodePtr n=std::make_shared<InvalidExp>();
		recover(TokenType::TK_SEMICOLON, [&]() {
				n=parseOr();
				if (thingsThatMayComeAfterParseExp.count(currentToken.id) == 0) 
					unexpectedToken();
			});
		
		if (currentToken.id != TokenType::TK_SEMICOLON) return n;
		
		std::shared_ptr<SequenceExp> s=std::make_shared<SequenceExp>();
		s->sequence.push_back(n);
		
        while (currentToken.id == TokenType::TK_SEMICOLON) {
            consumeToken();
			recover(TokenType::TK_SEMICOLON, [&]() {
					s->sequence.push_back(parseOr());
					if (thingsThatMayComeAfterParseExp.count(currentToken.id) == 0) 
						unexpectedToken();
				});
		}
        return s;
	}
	
    NodePtr parseExp() {
		return parseSequenceExp();
	}
	
    virtual NodePtr parse() override {
        NodePtr n = std::make_shared<InvalidExp>();
		currentToken = lexer->getNext();
		recover(TokenType::END_OF_FILE, [&](){
				n = parseExp();
				assertTokenConsume(TokenType::END_OF_FILE);
			});

		return n;
	}
};

} //nameless namespace

namespace rasmus {
namespace frontend {
std::shared_ptr<Parser> makeParser(std::shared_ptr<Lexer> lexer, 
							   std::shared_ptr<Error> error,
							   bool interactiveMode) {
	return std::make_shared<TheParser>(lexer, error, interactiveMode);
}
} //namespace frontend
} //namespace rasmus

