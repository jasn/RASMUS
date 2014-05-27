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
#ifndef __callback_hh__
#define __callback_hh__
#include <frontend/code.hh>
#include <frontend/lexer.hh>
#include <frontend/ASTBase.hh>

enum class MsgType {
	error, warning, info
};

class Callback {
public:
	virtual ~Callback() {}
	virtual void report(MsgType type, 
						std::shared_ptr<Code> code,
						std::string message,
						Token mainToken,
						std::initializer_list<CharRange> ranges) = 0;
	virtual void report(MsgType type, std::string message) = 0;

	virtual void print(Type type, std::string repr) = 0;
};

class TerminalCallback: public Callback {
public:
	virtual void report(MsgType type, 
						std::shared_ptr<Code> code,
						std::string message,
						Token mainToken,
						std::initializer_list<CharRange> ranges) override;
	
	virtual void report(MsgType type, std::string message) override;

	virtual void print(Type type, std::string repr) override;
};


#endif //__callback_hh__

