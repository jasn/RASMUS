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

namespace {

class RecoverException: public std::exception {
public:
	const TokenId id;
	RecoverException(TokenId id): id(id) {}
};


const std::unordered_set<TokenId> thingsThatMayComeAfterParseExp =
{TK_RIGHTARROW, TK_RPAREN, TK_COMMA, TK_FI,
 TK_PIPE, TK_COLON, TK_END, TK_IN, TK_CHOICE,
 TK_VAL, TK_TWO_DOTS, TK_EOF, TK_SEMICOLON, TK_BLOCKEND};

const std::unordered_set<TokenId> types=
{TK_TYPE_BOOL,TK_TYPE_INT, TK_TYPE_TEXT,TK_TYPE_ATOM, TK_TYPE_TUP,TK_TYPE_REL, TK_TYPE_FUNC,TK_TYPE_ANY};
	
class TheParser: public Parser {
public:
	std::shared_ptr<Lexer> lexer;
	std::shared_ptr<Error> error;
	bool interactiveMode;
	
	Token currentToken;

	std::vector<TokenId> recoverStack;
	
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
	inline void recover(TokenId id, T t) {
		recoverStack.push_back(id);
		try {
			t();
			recoverStack.pop_back();
		} catch(RecoverException e) {
			recoverStack.pop_back();
			if (e.id != id) throw;
		};
	}

	
	void doRecover() __attribute__ ((noreturn)) {
		//Recover errors at the first token in list of tokens specified on the recovery stack
		std::unordered_set<TokenId> r(recoverStack.begin(), recoverStack.end());
		if (r.count(currentToken.id)) 
			throw RecoverException(currentToken.id);
		consumeToken();
		while (r.count(currentToken.id) == 0) {
			if (currentToken.id == TK_EOF)
				throw RecoverException(TK_EOF);
			else if (currentToken.id == TK_ERR)
				parseError("Invalid token");
			consumeToken();
		}
		throw RecoverException(currentToken.id);
	}

	void parseError(std::string err) {
		error->reportError(std::string("Parse error: ")+err, currentToken);
	}

    void unexpectedToken() __attribute__ ((noreturn)) {
        if (interactiveMode && currentToken.id == TK_EOF)
            throw IncompleteInputException();
        parseError("Unexpected token");
        doRecover();
	}

    void assertToken(TokenId id) {
        if (interactiveMode &&
            currentToken.id == TK_EOF &&
            id != TK_EOF)
            throw IncompleteInputException();
        
        if (id != currentToken.id) {
            parseError(std::string("Expected ")+getTokenName(id)+" at");
			doRecover();
		}
	}

    Token assertTokenConsume(TokenId id) {
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
		if (currentToken.id == TK_ASSIGN) {
			Token t=consumeToken();
			return std::make_shared<AssignmentExp>(nameToken, t, parseCompareExp());
		}
		return std::make_shared<VariableExp>(nameToken);
	}
	
	NodePtr parseIfExp() {
		std::shared_ptr<IfExp> n = std::make_shared<IfExp>(assertTokenConsume(TK_IF));
		recover(TK_IF, [&]() {
			recover(TK_CHOICE, [&]() {
				NodePtr e=parseExp();
				Token t=assertTokenConsume(TK_RIGHTARROW);
				n->choices.push_back(std::make_shared<Choice>(e, t,  parseExp()));
				while (currentToken.id == TK_CHOICE) {
					consumeToken();
					recover(TK_CHOICE, [&]() {
						NodePtr e=parseExp();
						Token t=assertTokenConsume(TK_RIGHTARROW);
						n->choices.push_back(std::make_shared<Choice>(e, t,  parseExp()));
					});
				}
				assertToken(TK_FI);
			});
		});
		n->fiToken = assertTokenConsume(TK_FI);
		return n;
	}
  
	NodePtr parseForallExp() {
		Token t1=consumeToken();
		std::shared_ptr<ForallExp> n = std::make_shared<ForallExp>(t1, assertTokenConsume(TK_LPAREN));
		recover(TK_COMMA, [&]() {
			n->listExps.push_back(parseExp());
			while (currentToken.id == TK_COMMA) {
				consumeToken();
				n->listExps.push_back(parseExp());
			}
			assertToken(TK_RPAREN);
		});
		n->rparenToken = assertTokenConsume(TK_RPAREN);
		if (currentToken.id == TK_PIPE) {
			n->pipeToken = consumeToken();
			n->names.push_back(assertTokenConsume(TK_NAME));
			while (currentToken.id == TK_COMMA) {
				consumeToken();
				n->names.push_back(assertTokenConsume(TK_NAME));
			}
		}
		n->colonToken = assertTokenConsume(TK_COLON);
		n->exp = parseExp();
		return n;
	}

	NodePtr parseBuiltIn() {
		Token t1=consumeToken();
		std::shared_ptr<BuiltInExp> n = std::make_shared<BuiltInExp>(t1, assertTokenConsume(TK_LPAREN));
		recover(TK_RPAREN, [&]() {
			n->args.push_back(parseExp());
			while (currentToken.id == TK_COMMA) {
				consumeToken();
				n->args.push_back(parseExp());
			}
			assertToken(TK_RPAREN);
		});
		n->rparenToken = assertTokenConsume(TK_RPAREN);
		return n;
	}
    
    NodePtr parseFuncExp() {
		Token t1=consumeToken();
		std::shared_ptr<FuncExp> n = std::make_shared<FuncExp>(t1,assertTokenConsume(TK_LPAREN));
		recover(TK_RPAREN, [&]() {
            if (currentToken.id != TK_RPAREN) {
				Token t1=assertTokenConsume(TK_NAME), t2=assertTokenConsume(TK_COLON);
				n->args.push_back(std::make_shared<FuncArg>(t1, t2, parseType()));
                while (currentToken.id == TK_COMMA) {
                    consumeToken();
					Token t1=assertTokenConsume(TK_NAME), t2=assertTokenConsume(TK_COLON);
					n->args.push_back(std::make_shared<FuncArg>(t1, t2, parseType()));
				}
			}
            assertToken(TK_RPAREN);
		});
        n->rparenToken = consumeToken();
		n->arrowToken = assertTokenConsume(TK_RIGHTARROW);
		n->lparen2Token = assertTokenConsume(TK_LPAREN);
		n->returnTypeToken = parseType();
		n->rparen2Token = assertTokenConsume(TK_RPAREN);
		recover(TK_END, [&]() {
            n->body = parseExp();
			assertToken(TK_END);
		});
        n->endToken = assertTokenConsume(TK_END);
		return n;
	}

    NodePtr parseTupExp() {
		Token t1=consumeToken();
		std::shared_ptr<TupExp> n = std::make_shared<TupExp>(t1, assertTokenConsume(TK_LPAREN));
		recover(TK_RPAREN, [&]() {
            if (currentToken.id != TK_RPAREN) {
				Token t1=assertTokenConsume(TK_NAME), t2=assertTokenConsume(TK_COLON);
                n->items.push_back(std::make_shared<TupItem>(t1, t2, parseExp()));
                while (currentToken.id == TK_COMMA) {
                    consumeToken();
					Token t1=assertTokenConsume(TK_NAME), t2=assertTokenConsume(TK_COLON);
					n->items.push_back(std::make_shared<TupItem>(t1, t2, parseExp()));
				}
			}
			assertToken(TK_RPAREN);
		});
		n->rparenToken = consumeToken();
        return n;
	}

    NodePtr parseBlockExp() {
		std::shared_ptr<BlockExp> n = std::make_shared<BlockExp>(consumeToken());
		recover(TK_BLOCKEND, [&]() {
			recover(TK_IN, [&]() {
                while (currentToken.id == TK_VAL) {
                    Token valToken = consumeToken();
					recover(TK_VAL, [&]() {
						Token t1=assertTokenConsume(TK_NAME);
						Token t2=assertTokenConsume(TK_EQUAL);
						n->vals.push_back(std::make_shared<Val>(
											 valToken,
											 t1, t2,
											 parseExp()));
					});
				}
				assertToken(TK_IN);
			});
            n->inToken = consumeToken();
            n->inExp = parseExp();
            assertToken(TK_BLOCKEND);
		});
        n->blockendToken = consumeToken();
		return n;
	}

    NodePtr parseParenthesisExp() {
		NodePtr n = std::make_shared<InvalidExp>();
        consumeToken();
		recover(TK_RPAREN, [&]() {
            n = parseExp();
            assertToken(TK_RPAREN);
		});
        assertTokenConsume(TK_RPAREN);
        return n;
	}

    NodePtr parseRelExp() {
		Token t=consumeToken();
		std::shared_ptr<RelExp> n = std::make_shared<RelExp>(t, assertTokenConsume(TK_LPAREN));
		recover(TK_RPAREN, [&]() {
            n->exp = parseExp();
			assertToken(TK_RPAREN);
		});
        n->rparenToken = consumeToken();
        return n;
	}

    NodePtr parseAtExp() {
		Token t=consumeToken();
		std::shared_ptr<AtExp> n = std::make_shared<AtExp>(t, assertTokenConsume(TK_LPAREN));
		recover(TK_RPAREN, [&]() {
            n->exp = parseExp();
            assertToken(TK_RPAREN);
		});
        n->rparenToken = consumeToken();
        return n;
	}

    NodePtr parseBottomExp() {
		switch (currentToken.id) {
		case TK_IF:
			return parseIfExp();
		case TK_LPAREN:
            return parseParenthesisExp();
		case TK_TODAY:
		case TK_CLOSE: {
			Token t=consumeToken();
            return std::make_shared<BuiltInExp>(t, Token());
		} case TK_BANG:
		case TK_BANGLT:
		case TK_BANGGT:
            return parseForallExp();
		case TK_ISATOM:
		case TK_ISTUP:
		case TK_ISREL:
		case TK_ISFUNC:
		case TK_ISANY:
		case TK_SYSTEM:
		case TK_WRITE:
		case TK_OPEN:
		case TK_HAS:
		case TK_MAX:
		case TK_MIN:
		case TK_COUNT: 
		case TK_ADD:
		case TK_MULT:
		case TK_DAYS:
		case TK_BEFORE:
		case TK_AFTER:
		case TK_DATE:
		case TK_ISBOOL:
		case TK_ISINT:
		case TK_ISTEXT:
		case TK_PRINT:
            return parseBuiltIn();
		case TK_PIPE: {
			Token t=consumeToken();
			NodePtr p=parseExp();
            return std::make_shared<LenExp>(t, p, assertTokenConsume(TK_PIPE));
		}
		case TK_MINUS:
		{
			Token t=consumeToken();
            return std::make_shared<UnaryOpExp>(t, parseExp());
		}
		case TK_NAME:
            return parseAssignment();
		case TK_FUNC:
            return parseFuncExp();
		case TK_REL:
            return parseRelExp();
		case TK_TUP:
            return parseTupExp();
		case TK_ZERO:
		case TK_ONE:
		case TK_STDBOOL:
		case TK_STDINT:
		case TK_STDTEXT:
		case TK_TEXT:
		case TK_INT:
		case TK_TRUE:
		case TK_FALSE:
            return std::make_shared<ConstantExp>(consumeToken());
		case TK_SHARP:
            return std::make_shared<VariableExp>(consumeToken());
		case TK_BLOCKSTART:
            return parseBlockExp();
		case TK_AT:
            return parseAtExp();
		case TK_NOT: {
			Token t=consumeToken();
            return std::make_shared<UnaryOpExp>(t, parseExp());
		}
		default:
            unexpectedToken();
		}
	}
				

    NodePtr parseSubstringOrFuncInvocationExp() {
        NodePtr n = parseBottomExp();
		if (currentToken.id == TK_LPAREN) {
            Token lparenToken = consumeToken();
			if (currentToken.id == TK_RPAREN) {
				std::shared_ptr<FuncInvocationExp> f = std::make_shared<FuncInvocationExp>(n, lparenToken);
				f->rparenToken = assertTokenConsume(TK_RPAREN);				
				n=f;
			} else {
				recover(TK_RPAREN, [&]() {
                    NodePtr e1 = parseExp();
                    if (currentToken.id == TK_TWO_DOTS) {
						Token t=consumeToken();
						NodePtr e2=parseExp();
                        n = std::make_shared<SubstringExp>(n, lparenToken, e1, t, e2, assertTokenConsume(TK_RPAREN));
					} else {
						std::shared_ptr<FuncInvocationExp> f = std::make_shared<FuncInvocationExp>(n, lparenToken);
						f->args.push_back(e1);
                        while (currentToken.id == TK_COMMA) {
                            consumeToken();
                            f->args.push_back(parseExp());
						}
						assertToken(TK_RPAREN);
						f->rparenToken = assertTokenConsume(TK_RPAREN);
						n=f;
					}
				});
				//TODO fix error recovery on RPAREN token, for TK_TWO_DOTS this did not work under python either
			}
		}
		return n;
	}

    NodePtr parseRenameExp() {
        NodePtr n =parseSubstringOrFuncInvocationExp();
        while (currentToken.id == TK_LBRACKET) {
			std::shared_ptr<RenameExp> e=std::make_shared<RenameExp>(n, consumeToken());
			recover(TK_RBRACKET, [&]() {
				Token t1=assertTokenConsume(TK_NAME), t2=assertTokenConsume(TK_LEFT_ARROW);
				e->renames.push_back(std::make_shared<RenameItem>(t1, t2, assertTokenConsume(TK_NAME)));

                while (currentToken.id == TK_COMMA) {
                    consumeToken();
					Token t1=assertTokenConsume(TK_NAME), t2=assertTokenConsume(TK_LEFT_ARROW);
                    e->renames.push_back(std::make_shared<RenameItem>(t1, t2, assertTokenConsume(TK_NAME)));
				}
				assertToken(TK_RBRACKET);
			});
            e->rbracketToken = assertTokenConsume(TK_RBRACKET);
			n = e;
		}
        return n;
	}

    NodePtr parseDotExp() {
        NodePtr n = parseRenameExp();
		if (currentToken.id == TK_ONE_DOT) {
			Token t=consumeToken();
            n = std::make_shared<DotExp>(n, t, assertTokenConsume(TK_NAME));
		}
        return n;
	}

    NodePtr parseOpExtendAndOverwriteExp() {
        NodePtr n = parseDotExp();
        while (currentToken.id == TK_OPEXTEND) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseDotExp());
		}
        return n;
	}

    NodePtr parseConcatExp() {
        NodePtr n = parseOpExtendAndOverwriteExp();
        while (currentToken.id == TK_CONCAT) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseOpExtendAndOverwriteExp());
		}
		return n;
	}
	
    NodePtr parseProjectionExp() {
        NodePtr n = parseConcatExp();
		if (currentToken.id == TK_PROJECT_PLUS || currentToken.id == TK_PROJECT_MINUS) {
			std::shared_ptr<ProjectExp> e = std::make_shared<ProjectExp>(n, consumeToken());
            e->names.push_back(assertTokenConsume(TK_NAME));
            while (currentToken.id == TK_COMMA) {
                consumeToken();
                e->names.push_back(assertTokenConsume(TK_NAME));
			}
			n=e;
		}
        return n;
	}

    NodePtr parseMulDivModAndExp() {
        NodePtr n = parseProjectionExp();
        while (currentToken.id == TK_DIV || currentToken.id == TK_MUL ||
			   currentToken.id == TK_MOD || currentToken.id == TK_AND) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseProjectionExp());
		}
		return n;
	}

    NodePtr parsePlusMinusOrExp() {
        NodePtr n = parseMulDivModAndExp();
		while (currentToken.id == TK_PLUS || currentToken.id == TK_MINUS ||
			   currentToken.id == TK_OR || currentToken.id == TK_SET_MINUS) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseMulDivModAndExp());
		}
        return n;
	}

    NodePtr parseSelectExp() {
        NodePtr n = parsePlusMinusOrExp();
        while (currentToken.id == TK_QUESTION) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parsePlusMinusOrExp());
		}
        return n;
	}

    NodePtr parseCompareExp() {
        NodePtr n = parseSelectExp();
        if (currentToken.id == TK_EQUAL || currentToken.id == TK_DIFFERENT ||
			currentToken.id == TK_LESS || currentToken.id == TK_GREATER ||
			currentToken.id == TK_TILDE) {
			Token t=consumeToken();
            n = std::make_shared<BinaryOpExp>(t, n, parseSelectExp());
		}
        return n;
	}

    NodePtr parseSequenceExp() {
        NodePtr n=std::make_shared<InvalidExp>();
		recover(TK_SEMICOLON, [&]() {
            n=parseCompareExp();
			if (thingsThatMayComeAfterParseExp.count(currentToken.id) == 0) 
				unexpectedToken();
		});
		
		if (currentToken.id != TK_SEMICOLON) return n;
		
		std::shared_ptr<SequenceExp> s=std::make_shared<SequenceExp>();
		s->sequence.push_back(n);
		
        while (currentToken.id == TK_SEMICOLON) {
            consumeToken();
			recover(TK_SEMICOLON, [&]() {
				s->sequence.push_back(parseCompareExp());
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
		recover(TK_EOF, [&](){
			n = parseExp();
		});
		assertTokenConsume(TK_EOF);
		return n;
	}
};

} //nameless namespace

std::shared_ptr<Parser> parser(std::shared_ptr<Lexer> lexer, 
							   std::shared_ptr<Error> error,
							   bool interactiveMode) {
	return std::make_shared<TheParser>(lexer, error, interactiveMode);
}

