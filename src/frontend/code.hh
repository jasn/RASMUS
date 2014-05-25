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
#ifndef __code_hh__
#define __code_hh__
#include <string>
#include <vector>

class Code {
public:
  std::vector<ssize_t> lineStarts;
  std::string code;
  std::string name;
  
  Code(std::string code, const std::string & name): name(name) {
    set(std::move(code) );
  }

  void add(std::string extra) {
    set(code + extra);
  }
  
  void set(std::string c) {
    code = std::move(c);
    lineStarts.clear();
    lineStarts.push_back(-1);
    for (size_t i=0; i < code.size(); ++i) {
      if (code[i] == '\n') lineStarts.push_back(i);
    }
    lineStarts.push_back(code.size());
  }
  
};

#endif //__code_hh__
