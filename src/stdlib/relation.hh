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
#ifndef __RELATION_HH__
#define __RELATION_HH__
#include "lib.h"
#include <stdlib/rm_object.hh>
#include <stdlib/refptr.hh>
#include <vector>
#include <stdlib/anyvalue.hh>

namespace rasmus {
namespace stdlib {

/* holds an attribute; 
   a schema consists of one or more such attributes
*/
struct Attribute {
	Type type;
	std::string name;
};

/* represents a schema e.g. for a tuple or a relation
 */
class Schema: public rm_object {
public:
	std::vector<Attribute> attributes;
	Schema(): rm_object(LType::schema) {};
};


/* a tuple consists of a schema and a set of values 
   with data corresponding to the schema.
 */
class Tuple: public rm_object {
public:
	RefPtr<rm_object> schema;
	std::vector<AnyValue> values;
	Tuple(): rm_object(LType::tuple) {};
};

/* a relation has a schema and some amount of tuples; 
   the tuples contain data corresponding to the schema
 */
class Relation: public rm_object {
public:
	std::vector<RefPtr<rm_object>> tuples;
	RefPtr<rm_object> schema;
	Relation(): rm_object(LType::relation) {};
};


void printRelationToStream(rm_object * ptr, std::ostream & out);
void saveRelationToStream(rm_object * o, std::ostream & outFile);
rm_object * loadRelationFromStream(std::istream & inFile);

void saveRelationToFile(rm_object * o, const char * name);
rm_object * loadRelationFromFile(const char * name);

} //namespace stdlib
} //namespace rasmus

#endif //__RELATION_HH__
