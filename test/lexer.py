import unittest
from rasmus.lexer import *

class TestLexer(unittest.TestCase):
    def do(self, code, *classes):
        l = Lexer(code)
        ll = []
        while True:
            (t, _, _) = l.getNext()
            if t == TK_EOF: break
            ll.append(t)
        self.assertEqual(ll, list(classes))
    def test_tup(self): self.do("tup", TK_TUP)
    def test_rel(self): self.do("rel", TK_REL)
    def test_func(self): self.do("func", TK_FUNC)
    def test_end(self): self.do("end", TK_END)
    def test_at(self): self.do("@", TK_AT)
    def test_sharp(self): self.do("#", TK_SHARP)
    def test_not(self): self.do("not nott", TK_NOT, TK_NAME)
    def test_minus(self): self.do("-", TK_MINUS)
    def test_and(self): self.do("and", TK_AND)
    def test_or(self): self.do("or", TK_OR)
    def test_truefalse(self): self.do("false-true truee", TK_FALSE, TK_MINUS, TK_TRUE, TK_NAME)
    def test_paren(self): self.do("((++ )+)" , TK_LPAREN, TK_BLOCKSTART, TK_PLUS, TK_RPAREN, TK_BLOCKEND)
    def test_val(self): self.do("val", TK_VAL)
    def test_in(self): self.do("in", TK_IN)
    def test_name(self): self.do("namee", TK_NAME)
    def test_colon(self): self.do(":", TK_COLON)
    def test_int(self): self.do("1234", TK_INT)
    def test_text(self): self.do("\"hello\"", TK_TEXT)
    def test_zero(self): self.do("zero", TK_ZERO)
    def test_one(self): self.do("one", TK_ONE)
    def test_stdbool(self): self.do("?-Bool", TK_STDBOOL)
    def test_stdint(self): self.do("?-Int", TK_STDINT)
    def test_stdtext(self): self.do("?-Text", TK_STDTEXT)
    def test_comma(self): self.do(",", TK_COMMA)
    def test_arrow(self): self.do("--> <-", TK_MINUS, TK_RIGHTARROW, TK_LEFT_ARROW)
    def test_type_bool(self): self.do("Bool", TK_TYPE_BOOL)
    def test_type_int(self): self.do("Int", TK_TYPE_INT)
    def test_type_text(self): self.do("Text", TK_TYPE_TEXT)
    def test_type_atom(self): self.do("Atom", TK_TYPE_ATOM)
    def test_type_tup(self): self.do("Tup", TK_TYPE_TUP)
    def test_type_rel(self): self.do("Rel", TK_TYPE_REL)
    def test_type_func(self): self.do("Func", TK_TYPE_FUNC)
    def test_type_any(self): self.do("Any", TK_TYPE_ANY)
    def test_operators(self): self.do("+*/++-\\", TK_PLUS, TK_MUL, TK_DIV, TK_CONCAT, TK_MINUS, TK_SET_MINUS)
    def test_mod(self): self.do("mod", TK_MOD)
    def test_comp(self): self.do("< <> <= >= = >", TK_LESS, TK_DIFFERENT, TK_LESSEQUAL, TK_GREATEREQUAL, TK_EQUAL, TK_GREATER)
    def test_semicolon(self): self.do(";", TK_SEMICOLON)
    def test_pipe(self): self.do("|", TK_PIPE)
    def test_dot(self): self.do("...", TK_TWO_DOTS, TK_ONE_DOT)
    def test_question(self): self.do("?", TK_QUESTION)
    def test_project(self): self.do("||+|--", TK_PIPE, TK_PROJECT_PLUS, TK_PROJECT_MINUS, TK_MINUS)
    def test_bracket(self): self.do("[]", TK_LBRACKET, TK_RBRACKET)
    def test_buildin(self): self.do("max min count add mult days before after today date open close write system has", TK_MAX, TK_MIN, TK_COUNT, TK_ADD, TK_MULT, TK_DAYS, TK_BEFORE, TK_AFTER, TK_TODAY, TK_DATE, TK_OPEN, TK_CLOSE, TK_WRITE, TK_SYSTEM, TK_HAS)
    def test_if(self): self.do("if iff fi", TK_IF, TK_NAME, TK_FI)
    def test_choice(self): self.do("&", TK_CHOICE)
    def test_bang(self): self.do("!!<<!>", TK_BANG, TK_BANGLT, TK_LESS, TK_BANGGT)
    def test_tilde(self): self.do("~", TK_TILDE)
    def test_is(self): self.do("is-Booll is-Int is-Text is-Atom is-Tup is-Rel is-Func is-Any",
                               TK_ISBOOL, TK_NAME, TK_ISINT, TK_ISTEXT, TK_ISATOM, TK_ISTUP, TK_ISREL, TK_ISFUNC, TK_ISANY)
    def test_error(self): self.do("hat %%Kat hat", TK_NAME, TK_ERR, TK_NAME)
3
